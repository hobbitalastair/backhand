/* bk-stop.c
 *
 * Stop a process managed by bk-escort by connecting to the given socket and
 * waiting for the socket to be closed by the other end.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include "config.h"

int main(int count, char** args) {
    char* name = "";
    if (count > 0) name = args[0];
    if (count != 2) {
        fprintf(stderr, "usage: %s <socket>\n", name);
        return EINVAL;
    }
    char* path = args[1];
    if (strlen(path) >= SOCK_PATHLEN) {
        fprintf(stderr, "%s: \"%s\" too long (max %ld bytes)\n", name, path,
                strlen(path));
        return EINVAL;
    }

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "%s: socket(): %s\n", name, strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_un conn;
    conn.sun_family = AF_UNIX;
    strcpy(conn.sun_path, path);
    size_t len = strlen(path) + sizeof(conn.sun_family);
    if (connect(sock, (struct sockaddr*)(&conn), len) == -1) {
        fprintf(stderr, "%s: connect(): %s\n", name, strerror(errno));
        return EXIT_FAILURE;
    }

    char c = 0;
    read(sock, &c, 1);
    close(sock);
}
