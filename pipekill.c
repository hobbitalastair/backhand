/* pipekill.c
 *
 * Read signal names from a pipe and send them to a child process.
 * Exit when the child exits.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "lib.h"

#define NAME "pipekill"

/* We need the fifo path to clean it up in a signal handler, so store it in
 * a global.
 * The cleanup signal handler also handles killing the child process, so store
 * that.
 */
char *fifo;
volatile sig_atomic_t child_pid = -1;

void cleanup(int signum) {
    /* Cleanup by removing the fifo */
    if (child_pid > 0 && signum != SIGCHLD) kill(child_pid, SIGTERM);
    unlink(fifo);
    exit(EXIT_SUCCESS);
}

int main(int count, char** args) {
    if (count < 3) {
        fprintf(stderr, "usage: %s <pipe_path> <child> ...\n", NAME);
        return EINVAL;
    }
    fifo = args[1];

    /* FIXME: This is racy, since the fifo could be deleted before we try to
     *        open it, we can't tell if there is actually a process waiting on
     *        the other side if one does exist, and we may never get to unlink
     *        the fifo (eg if we are interupted by a SIGTERM).
     */

    if (mkfifo(fifo, 0660) == -1) {
        int err = errno;
        fprintf(stderr, "%s: mkfifo '%s' failed: %s\n", NAME, fifo,
                strerror(err));
        return err;
    }

    /* We want to be notified when the child dies so we can clean up.
     * This needs to be registered after creating the fifo but before creating
     * the child to ensure that we catch the event.
     * We are also want to know when we exit, and cleanup as required for that.
     */
    signal(SIGCHLD, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGINT, cleanup);

    child_pid = fork();
    if (child_pid == -1) {
        fprintf(stderr, "%s: fork failed: %s\n", NAME, strerror(errno));
        return EXIT_FAILURE;
    } else if (child_pid == 0) {
        exec_fatal(NAME, count - 2, &args[2]);
    }

    int fd = open(fifo, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "%s: failed to open fifo at '%s': %s\n", NAME, fifo,
                strerror(errno));
        return EXIT_FAILURE;
    }

    while (true) {
        char buf[5] = {0};
        ssize_t bytes_read = read(fd, buf, 5);
        if (bytes_read > 0) kill(child_pid, SIGTERM);
    }
}
