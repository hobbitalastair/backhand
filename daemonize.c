/* daemonize.c
 *
 * Background a child process.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/* We use a statically allocated buffer to avoid needing to malloc a new
 * buffer of the appropriate size.
 */
#define ARG_BUF_SIZE 256

bool daemonize() {
    /* Daemonize the current process.
     *
     * This boils down to forking twice and calling "setsid" in the middle;
     * there is a default implementation (daemon) available in the libc, but
     * it's not standardised so just roll our own.
     *
     * This returns true if the daemonize process succeeded, otherwise it
     * returns false.
     */

    pid_t result = fork();
    if (result == -1) return false;
    if (result > 0) exit(EXIT_SUCCESS);

    if (setsid() == -1) return false;

    result = fork();
    if (result > 0) {
        exit(EXIT_SUCCESS);
    }
    return result != -1;
}

int main(int count, char** args) {
    if (count < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return EINVAL;
    } else if (count > ARG_BUF_SIZE) {
        fprintf(stderr, "Too many arguments\n");
        return E2BIG;
    }

    if (!daemonize()) {
        perror("daemonize: failed to daemonize");
        return EXIT_FAILURE;
    }

    /* Exec the child process */
    char* arg_buf[ARG_BUF_SIZE] = {0};
    for (unsigned int i = 0; i < count; i++) arg_buf[i] = args[i + 1];
    execv(arg_buf[0], arg_buf); /* Shouldn't return */
    perror("daemonize: failed to execute child");
    return EXIT_FAILURE;
}
