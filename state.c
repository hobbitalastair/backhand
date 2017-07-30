/* state.c
 *
 * Provide a simple "state" which can accessed atomically.
 *
 * Given a filename and a new state, update the existing state, returning 0 if
 * the state changed, and 1 otherwise.
 * If the file does not exist, create it, otherwise use the existing file.
 * Return 2 on error.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define EXIT_CHANGED 0
#define EXIT_UNCHANGED 1
#define EXIT_FAILED 2

void fatal_lock(char* name, int fd) {
    /* Lock the given fd.
     *
     * The whole file is locked, and the process will wait until the lock is
     * available. If we fail to lock the file for some reason, exit() will be
     * called. Locks are released on program exit.
     */

    struct flock fl = {
        .l_type=F_RDLCK,
        .l_start=0,
        .l_whence=SEEK_SET,
        .l_len=0
    };

    /* Acquire a lock */
    int result = -1;
    do {
        result = fcntl(fd, F_SETLKW, &fl);
    } while (result == -1 && errno == EINTR);

    if (result != 0) {
        fprintf(stderr, "%s: locking failed: %s\n", name, strerror(errno));
        exit(EXIT_FAILED);
    }
}

int fd_compare(char* name, int fd, char* state) {
    /* Return EXIT_UNCHANGED if the contents of the fd is equal to the given
     * state, EXIT_CHANGED if not, or EXIT_FAILURE on error.
     */

    while (1) {
        char c = 0;
        ssize_t result = read(fd, &c, 1);
        if (result == 1) {
            /* This needs to check for '\0' as strings in a file might contain
             * more than one null.
             */
            if (state[0] == '\0' || state[0] != c) {
                return EXIT_CHANGED;
            }
            state++;
        }
        if (result == 0) {
            if (state[0] == '\0') {
                return EXIT_UNCHANGED;
            }
            return EXIT_CHANGED;
        }
        if (result == -1 && errno != EINTR) {
            fprintf(stderr, "%s: read failed: %s\n", name, strerror(errno));
            return EXIT_FAILED;
        }
    }
}

void fatal_set(char* name, int fd, char* state) {
    /* Write the given value back into the file.
     *
     * This calls exit() on failure.
     */

    /* Note that we only want result == len; if this is not the case then we
     * can't be sure of success...
     *
     * FIXME: Due to our locking method we just use the really simple
     *        truncate and replace method here - but if we fail for any reason
     *        we bail, leaving a broken file.
     */
    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "%s: seek failed: %s\n", name, strerror(errno));
        exit(EXIT_FAILED);
    }
    if (ftruncate(fd, 0) == -1) {
        fprintf(stderr, "%s: truncate failed: %s\n", name, strerror(errno));
        exit(EXIT_FAILED);
    }
    size_t len = strlen(state);
    if (write(fd, state, len) != len) {
        fprintf(stderr, "%s: write failed: %s\n", name, strerror(errno));
        exit(EXIT_FAILED);
    }
}

int main(int count, char** args) {
    char* name = __FILE__;
    if (count > 0) name = args[0];
    if (count != 3) {
        fprintf(stderr, "usage: %s <lock file> <state>\n", name);
        return EINVAL;
    }
    char* op = args[2];

    int fd = open(args[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "%s: open failed: %s\n", name, strerror(errno));
        return EXIT_FAILED;
    }

    /* Lock the file; this is an attempt to avoid race conditions where
     * multiple calls overwrite the other's results.
     * The lock is released when the process exits.
     */
    fatal_lock(name, fd);

    /* Update the value */
    int ret = fd_compare(name, fd, op);
    if (ret == EXIT_CHANGED) {
        fatal_set(name, fd, op);
    }
    return ret;
}
