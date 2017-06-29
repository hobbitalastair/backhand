/* bk-escort.c
 *
 * Escort process for looking after long-lived daemons.
 * This provides a unix domain socket for shutting down the process, and will
 * restart the process when it is supposed to be running.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define SOCK_PATHLEN 92 /* Maximum path length for the socket */
#define SLEEP_INTERVAL 1 /* Number of seconds to sleep between retries */
#define CHILD_TIMEOUT 10 /* Time before killing a child that exits slowly */

/* We store a flag for detecting which signal handlers have been called */
volatile sig_atomic_t sigchld;
volatile sig_atomic_t sigalrm;
volatile sig_atomic_t sigterm;
volatile sig_atomic_t sigint;

void handle_signal(int signum) {
    /* Handle any incoming signals */
    if (signum == SIGCHLD) sigchld += 1;
    if (signum == SIGALRM) sigalrm = 1;
    if (signum == SIGTERM) sigterm = 1;
    if (signum == SIGINT) sigint = 1;
}

sigset_t init_signals(char* name) {
    /* Mask all signals and register signal handlers.
     *
     * We need to be careful about which signals we recieve and how we handle
     * them to avoid dying by surprise, so just mask them all to start with.
     * We can then unmask any we want to handle when we call pselect().
     *
     * Returns a full signal mask minus the signals we handle.
     */

    sigset_t mask;
    int ret = sigfillset(&mask);
    if (ret != -1) ret = sigprocmask(SIG_SETMASK, &mask, NULL);
    if (ret == -1) {
        fprintf(stderr, "%s: setting the signal mask failed: %s\n", name,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sigaction action;
    action.sa_handler = &handle_signal;
    action.sa_flags = SA_NOCLDSTOP;
    ret = sigemptyset(&action.sa_mask);
    if (ret != -1) ret = sigaction(SIGCHLD, &action, NULL);
    if (ret != -1) ret = sigaction(SIGALRM, &action, NULL);
    if (ret != -1) ret = sigaction(SIGTERM, &action, NULL);
    if (ret != -1) ret = sigaction(SIGINT, &action, NULL);
    if (ret == -1) {
        fprintf(stderr, "%s: sigaction(): %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    ret = sigfillset(&mask);
    if (ret != -1) ret = sigdelset(&mask, SIGCHLD);
    if (ret != -1) ret = sigdelset(&mask, SIGALRM);
    if (ret != -1) ret = sigdelset(&mask, SIGTERM);
    if (ret != -1) ret = sigdelset(&mask, SIGINT);
    if (ret == -1) {
        fprintf(stderr, "%s: setting the signal mask failed: %s\n", name,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    return mask;
}

int init_socket(char* name, char* path) {
    /* Initialise a local socket bound to "path", calling exit() on failure */
    if (strlen(path) >= SOCK_PATHLEN) {
        fprintf(stderr, "%s: \"%s\" too long (max %ld bytes)\n", name, path,
                strlen(path));
        exit(EINVAL);
    }

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "%s: socket(): %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
        fprintf(stderr, "%s: fcntl(): %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un bound_sock;
    bound_sock.sun_family = AF_UNIX;
    strcpy(bound_sock.sun_path, path);
    size_t len = strlen(path) + sizeof(bound_sock.sun_family);
    if (bind(sock, (struct sockaddr*)(&bound_sock), len) == -1) {
        fprintf(stderr, "%s: bind(): %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(sock, 1) == -1) {
        fprintf(stderr, "%s: listen(): %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return sock;
}

pid_t launch(char* name, int sock, int count, char** args) {
    /* Launch the child process described with args 2 and onwards.
     *
     * Returns a -1 on failure, and the pid of the child otherwise.
     */

    fprintf(stderr, "%s: launching child %s\n", name, args[2]);

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "%s: fork(): %s\n", name, strerror(errno));
        return -1;
    } else if (pid == 0) {
        /* Clean up */
        close(sock);
        sigset_t child_mask;
        int ret = sigemptyset(&child_mask);
        if (ret != -1) ret = sigprocmask(SIG_SETMASK, &child_mask, NULL);
        if (ret == -1) {
            fprintf(stderr, "%s: setting the signal mask failed: %s\n", name,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* Exec child */
        size_t i;
        for (i = 0; i < count - 2; i ++) {
            args[i] = args[i + 2];
        }
        args[i] = (char*)NULL;
        execv(args[0], args);
        fprintf(stderr, "%s: execv(): %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return pid;
}

int main(int count, char** args) {
    char* name = "";
    if (count > 0) name = args[0];
    if (count < 3) {
        fprintf(stderr, "usage: %s <socket> [<child> ...]\n", name);
        return EINVAL;
    }

    sigset_t mask = init_signals(name);
    int sock = init_socket(name, args[1]);
    pid_t pid = launch(name, sock, count, args);
    if (pid == -1) return EXIT_FAILURE;

    /* Main event loop.
     *
     * We need to deal with signals and new connections here.
     * Key signals are SIGCHLD, SIGALRM, and SIGTERM.
     * We should never call exit() here; retry failures instead.
     */
    bool keep_alive = true; /* keep_alive -> restart dead child */
    int quit_request = 0; /* 1 if we should terminate the child and return */
    int conn = -1; /* Connection which requested termination */
    while (1) {
        /* We use pselect here to avoid races - we want to ensure that we
         * block until either a signal arrives or a connection appears, which
         * needs some kind of select() either for a signalfd, a
         * "self-pipe trick" implementation, or pselect, which only removes the
         * mask when getting a signal would also result in aborting the call.
         *
         * Fortunately if there are no sockets to read from, pselect appears to
         * block until interrupted by a signal. This is sufficient for us to
         * use it as a pause() alternative when we have closed the socket.
         */
        fd_set fds;
        FD_ZERO(&fds);
        if (sock != -1) FD_SET(sock, &fds);
        if (pselect(sock + 1, &fds, NULL, NULL, NULL, &mask) == -1 &&
                errno != EINTR) {
            fprintf(stderr, "%s: pselect(): %s\n", name, strerror(errno));
            sleep(SLEEP_INTERVAL);
        }

        while (sigchld > 0) {
            sigchld --;

            int status;
            pid_t child = waitpid(-1, &status, WNOHANG);
            if (child == pid) {
                /* Handle our special child */
                if (WIFEXITED(status)) {
                    fprintf(stderr, "%s: child exited with status %d\n", name,
                            WEXITSTATUS(status));
                }
                if (WIFSIGNALED(status)) {
                    fprintf(stderr, "%s: child died from signal %d\n", name,
                            WTERMSIG(status));
                }

                if (keep_alive) {
                    pid_t newpid = launch(name, sock, count, args);
                    if (newpid == -1) {
                        sleep(SLEEP_INTERVAL);
                    } else {
                        pid = newpid;
                    }
                } else {
                    if (conn != -1) {
                        /* We write a single byte to the buffer to confirm that
                         * we have finished with the child.
                         */
                        while (write(conn, "\0", 1) == -1 && errno == EINTR);
                        close(conn);
                    }
                    exit(EXIT_SUCCESS);
                }
            }
        }

        if (sock != -1) {
            /* Note that this needs to go *after* handling the children, as we
             * rely on launching the child when sock is the only open file.
             * However, this is not true when we accept a connection, so we
             * have to ensure that keep_alive is unset by the time we process
             * any children.
             */
            struct sockaddr_un remote;
            socklen_t addrlen = sizeof(struct sockaddr_un);
            int new_conn = accept(sock, (struct sockaddr*)(&remote), &addrlen);
            if (new_conn != -1) {
                conn = new_conn;
                quit_request = 1;
            } else if (errno != EINTR &&
                    errno != EAGAIN &&
                    errno != EWOULDBLOCK) {
                fprintf(stderr, "%s: accept(): %s\n", name, strerror(errno));
                sleep(SLEEP_INTERVAL);
            }
        }

        if (sigalrm || ((sigterm || sigint) && !keep_alive)) {
            /* Kill the child */
            fprintf(stderr, "%s: killing child\n", name);
            sigalrm = 0;
            kill(pid, SIGKILL);
        }

        if (sigterm || sigint || quit_request) {
            /* Stop the child, and close the socket for incoming requests */
            fprintf(stderr, "%s: terminating child\n", name);
            quit_request = sigint = sigterm = 0;

            keep_alive = false;
            kill(pid, SIGTERM);
            alarm(CHILD_TIMEOUT);

            unlink(args[1]);
            close(sock);
            sock = -1;
        }
    }
}

