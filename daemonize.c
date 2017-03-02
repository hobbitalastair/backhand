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
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "lib.h"

#define NAME "daemonize"

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
        fprintf(stderr, "usage: %s <child> [<child arguments> ...]\n", NAME);
        return EINVAL;
    }

    if (!daemonize()) {
        fprintf(stderr, "%s: daemonizing failed: %s\n", NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    exec_fatal(NAME, count - 1, &args[1]);
}
