/* renew.c
 *
 * Keep alive a child process, by restarting it whenever it returns.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* We use a statically allocated buffer to avoid needing to malloc a new
 * buffer of the appropriate size.
 */
#define ARG_BUF_SIZE 256

/* We define some constants that control the leaky bucket algorithm.
 * See wait_bucket() for details on the algorithm
 *
 * BUCKET_SIZE controls the size of the bucket (in seconds).
 * BUCKET_COST contains the cost of each call to wait_bucket (or in our case,
 * each process respawn) in seconds.
 */
#define BUCKET_SIZE 20
#define BUCKET_COST 10

bool spawn(char* arg_buf[ARG_BUF_SIZE]) {
    /* Spawn a child with arguments in arg_buf, then wait for the child to
     * return.
     *
     * arg_buf[0] is assumed to be the program name.
     * Returns true if the child exited with a return code of 0, false
     * otherwise.
     */

    /* Fork off the child process */
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("renew: error when forking");
        return false;
    } else if (child_pid == 0) {
        execv(arg_buf[0], arg_buf);
        perror("renew: failed to execv child");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "renew: launched child %s\n", arg_buf[0]);

    /* Wait until the *correct* child returns */
    int status;
    while (true) {
        pid_t result = wait(&status);
        if (result == -1 && errno == ECHILD) {
            return false;
        } else if (result == child_pid) {
            if (WEXITSTATUS(status) != EXIT_SUCCESS) {
                fprintf(stderr, "renew: child returned with status %d\n",
                        WEXITSTATUS(status));
                return false;
            }
            return true;
        }
    }
}

void wait_bucket(time_t* bucket, time_t* old_time) {
    /* wait_bucket implements a "leaky bucket" rate limiting algorithm.
     *
     * The algorithm works by maintaining a "bucket" which fills with
     * time; however each call removes time from the bucket, blocking until
     * there is enough time available to remove.
     */

    time_t cost = BUCKET_COST;
    time_t elapsed = time(NULL) - *old_time;
    *bucket += elapsed;

    if (*bucket < cost) {
        /* Wait until the cost is paid, emptying the bucket */
        cost -= *bucket;
        *bucket = 0;
        while (cost > 0) cost = sleep(cost);
    } else {
        /* Lower the bucket, ensure that it is capped, and save the time */
        *bucket -= cost;
        if (*bucket > BUCKET_SIZE) *bucket = BUCKET_SIZE;
    }

    *old_time = time(NULL);
}

int main(int count, char** args) {
    if (count < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return EINVAL;
    } else if (count > ARG_BUF_SIZE) {
        fprintf(stderr, "Too many arguments\n");
        return E2BIG;
    }

    /* Make a copy of the arguments in the right format for execv */
    char* arg_buf[ARG_BUF_SIZE] = {0};
    for (unsigned int i = 0; i < count; i++) arg_buf[i] = args[i + 1];

    /* old_time and bucket can both be initialised to 0 since the elapsed time
     * since the epoch will be enough to initially fill the bucket...
     */
    time_t old_time = 0;
    time_t bucket = 0;
    while (true) {
        wait_bucket(&bucket, &old_time);
        spawn(arg_buf);
    }
}