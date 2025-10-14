#include "sconn.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

size_t read_exact(int fd, void* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, (char*) buf + total, len - total);

        if (n < 0) {
            if (errno == EINTR) {
                fprintf(stderr, "EINTR, retrying\n");
                continue;
            }

            if (errno == EAGAIN) {
                return total;
            }

            return -1;
        }

        if (n == 0) {
            return total;
        }

        total += n;
    }
    return total;
}

size_t write_exact(int fd, void* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, (char*) buf + total, len - total);

        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) return total;
            return -1;
        }

        if (n == 0) return total;

        total += n;
    }

    return total;
}

bool handle_read(sconnection* conn) {
    if (conn -> state == STATE_READ_HEADER) {
        ssize_t n = read_exact(conn -> fd, &conn -> net_len, sizeof(conn -> net_len));
        if (n == -1) {
            fprintf(stderr, "Failed to read payload header for fd: %d\n", conn -> fd);
            return false;
        }

        if (n == 0) return false;

        if (n < (ssize_t) sizeof(conn -> net_len)) {
            return true;
        }

        conn -> payload_size = ntohl(conn -> net_len);
        conn -> buffer = calloc(1, conn -> payload_size + 1);
        conn -> state = STATE_READ_PAYLOAD;
    }

    if (conn -> state == STATE_READ_PAYLOAD) {
        ssize_t n = read_exact(conn -> fd, conn -> buffer, conn -> payload_size);
        if (n == -1) {
            fprintf(stderr, "Failed to read payload for fd: %d\n", conn -> fd);
            return false;
        }

        conn -> state = STATE_WRITE_RESPONSE;
    }

    return true;
}

bool handle_write(sconnection* conn) {
    if (conn -> state == STATE_WRITE_RESPONSE) {
        ssize_t n = write_exact(conn -> fd, conn -> buffer, conn -> payload_size);
        if (n == -1) {
            fprintf(stderr, "Failed to send payload for fd: %d\n  Buffer: %s\n", conn -> fd, conn -> buffer);
            return false;
        }

        free(conn -> buffer);
        conn -> buffer = NULL;
        conn -> state = STATE_READ_HEADER;
        conn -> payload_size = 0;
        conn -> net_len = 0;
    }

    return true;
}
