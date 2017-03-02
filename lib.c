/* lib.c
 *
 * Provide some common, shared functions.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* We use a statically allocated buffer to avoid needing to malloc a new
 * buffer of the appropriate size.
 */
#define ARG_BUF_SIZE 256

void exec_fatal(char* name, int count, char** args) {
    /* Execute the given set of arguments.
     *
     * The program to execute is assumed to be the item in "args" at index 0,
     * and "name" is used when printing errors.
     *
     * Call exit(EXIT_FAILURE) if we fail to exec.
     */

    if (count >= ARG_BUF_SIZE) {
        fprintf(stderr, "%s: too many arguments\n", name);
        exit(EXIT_FAILURE);
    }

    char* arg_buf[ARG_BUF_SIZE] = {0};
    for (int i = 0; i < count; i++) arg_buf[i] = args[i];
    execv(arg_buf[0], arg_buf); /* Should not return */
    fprintf(stderr, "%s: failed to execute child %s: %s\n", name, arg_buf[0],
            strerror(errno));
    exit(EXIT_FAILURE);
}
