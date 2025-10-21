#include "ws_common.h"
#include "ws_net.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int ws_make_nonblocking(const int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("Fcntl() failed");
        _exit(1);
    }

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int ws_create_tcp_server(const char* address, const uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Failed to create socket\n");
        _exit(1);
    }

    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        WS_ERR_CLOSE_AND_EXIT("Failed to set socket option: SO_REUSEADDR", fd, 1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        WS_ERR_CLOSE_AND_EXIT("Failed to set socket option: SO_REUSEPORT", fd, 1);
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &addr.sin_addr);

    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        WS_ERR_CLOSE_AND_EXIT("Failed to bind socket", fd, 1);
    }

    if (listen(fd, SOMAXCONN) == -1) {
        WS_ERR_CLOSE_AND_EXIT("Failed to start listening", fd, 1);
    }

    return fd;
}
