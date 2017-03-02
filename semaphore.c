/* semaphore.c
 *
 * Provide a simple "semaphore" which can be incremented and decremented.
 *
 * Given a filename, and a +/- operation, return 0 if the state changed (+ 
 * incremented the count from 0, or - decremented the count), and 1 otherwise.
 * If the file does not exist, create it, otherwise use the existing file.
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

#define INC '+'
#define DEC '-'
#define EXIT_CHANGED 0
#define EXIT_UNCHANGED 1
#define EXIT_FAILED 2

#define BUF_SIZE 10 /* Enough for roughly 10^9 + operations */

void fatal_lock(int fd) {
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
        perror("semaphore: failed to lock file");
        exit(EXIT_FAILED);
    }
}

int fatal_get(int fd) {
    /* Return the current value stored in the given fd.
     *
     * This calls exit() on failure.
     */

    char buf[BUF_SIZE] = {0};

    ssize_t result = read(fd, &buf, BUF_SIZE);
    errno = 0;
    if (result == -1 || errno != 0) {
        perror("semaphore: failed to read from file");
        exit(EXIT_FAILED);
    } else if (result >= BUF_SIZE) {
        fprintf(stderr, "semaphore: too many characters\n");
        exit(EXIT_FAILED);
    }
    
    /* Read the value stored in the given fd */
    int total = 0;
    unsigned int offset = 0;
    unsigned int current;
    while (buf[offset] != '\0') {
        current = (buf[offset] - '0');
        if (current > 9 || current < 0) {
            fprintf(stderr,
                    "semaphore: unexpected character ('%lc' in '%s')\n",
                    buf[offset], buf);
            exit(EXIT_FAILED);
        }
        total *= 10;
        total += current;
        offset++;
    }

    return total;
}

void fatal_set(int fd, int value) {
    /* Write the given value back into the file.
     *
     * This calls exit() on failure.
     */

    char buf[BUF_SIZE] = {0};

    /* Write the value to the buffer */
    if (snprintf(buf, BUF_SIZE-1, "%u", value) >= BUF_SIZE) {
        fprintf(stderr, "semaphore: too many increments\n");
        exit(EXIT_FAILED);
    }

    /* Write the result to a file.
     *
     * Note that we only want result == len; if this is not the case then we
     * can't be sure of success...
     */
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("semaphore: failed to seek");
        exit(EXIT_FAILED);
    }
    if (ftruncate(fd, 0) == -1) {
        perror("semaphore: failed to truncate");
        exit(EXIT_FAILED);
    }
    size_t len = strlen(buf);
    if (write(fd, buf, len) != len) {
        perror("semaphore: failed to write to file");
        exit(EXIT_FAILED);
    }
}

int main(int count, char** args) {
    /* args[0] == program name, args[1] == file name, args[2] == +/- operand */
    if (count != 3) {
        fprintf(stderr, "Unexpected number of arguments\n");
        return EINVAL;
    }

    char op = args[2][0];
    if ((op != INC && op != DEC) || args[2][1] != '\0') {
        fprintf(stderr, "Expected '+' or '-', got '%s'\n", args[2]);
        return EINVAL;
    }

    int fd = open(args[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("semaphore: failed to open file");
        return EXIT_FAILED;
    }

    /* Lock the file; this is an attempt to avoid race conditions where
     * multiple calls overwrite the other's results.
     * The lock is released when the process exits.
     */
    fatal_lock(fd);

    /* Update the value */
    int value = fatal_get(fd);
    int ret = EXIT_UNCHANGED;
    if (op == INC) {
        if (value == 0) ret = EXIT_CHANGED;
        value++;
    } else if (op == DEC) {
        value--;
        if (value < 0) value = 0;
        if (value == 0) ret = EXIT_CHANGED;
    }
    fatal_set(fd, value);

    return ret;
}
