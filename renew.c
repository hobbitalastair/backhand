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

#define ARG_BUF_SIZE 256
#define BUCKET_SIZE 20
#define BUCKET_COST_DIVISOR 2

bool spawn(char* arg_buf[ARG_BUF_SIZE]) {
    /* Spawn a child with arguments in arg_buf.
     *
     * arg_buf[0] is assumed to be the program name.
     * Returns true if the child exited with a return code of 0, false
     * otherwise.
     */

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

    bool waiting = true;
    bool success = false;
    while (waiting) {
        int status;
        pid_t result = wait(&status);
        if (result == -1 && errno == ECHILD) {
            waiting = false;
        } else if (result == child_pid) {
            waiting = false;
            success = WEXITSTATUS(status) == EXIT_SUCCESS;
            if (!success) {
                fprintf(stderr, "renew: child returned with status %d\n",
                        WEXITSTATUS(status));
            }
        }
    }

    return success;
}

void wait_bucket(time_t bucket_size, time_t* bucket, time_t* old_time) {
    /* wait_bucket implements a "leaky bucket" rate limiting algorithm.
     *
     * The algorithm works by maintaining a "bucket" which fills with
     * time; however each call removes time from the bucket, blocking until
     * there is enough time available to remove.
     */

    time_t elapsed = time(NULL) - *old_time;
    *bucket += elapsed;

    /* Calculate the cost to call this function */
    time_t cost = bucket_size / BUCKET_COST_DIVISOR;

    if (*bucket < cost) {
        /* Wait until the cost is paid, emptying the bucket */
        cost -= *bucket;
        *bucket = 0;
        while (cost > 0) {
            cost = sleep(cost);
        }
    } else {
        /* Lower the bucket, ensure that it is capped, and save the time */
        *bucket -= cost;
        if (*bucket > bucket_size) {
            *bucket = bucket_size;
        }
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

    char* arg_buf[ARG_BUF_SIZE] = {0};
    for (unsigned int i = 0; i < count; i++) {
        arg_buf[i] = args[i + 1];
    }

    /* old_time and bucket can both be initialised to 0 since the elapsed time
     * since the epoch will be enough to initially fill the bucket...
     */
    time_t old_time = 0;
    time_t bucket = 0;
    while (true) {
        wait_bucket(BUCKET_SIZE, &bucket, &old_time);
        spawn(arg_buf);
    }
}
