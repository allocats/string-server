#define _GNU_SOURCE

#include "snet.h"
#include "sconn.h"
#include "spoll.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_WORKERS 12
#define MAX_EVENTS 8192

#define SERVER_ERROR(msg) \
    perror(msg); \
    exit(1)

void run_worker(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        SERVER_ERROR("Failed to make socket");
    }

    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        close(server_fd);
        SERVER_ERROR("Failed to set setsockopt() ADDR");
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        close(server_fd);
        SERVER_ERROR("Failed to set setsockopt() PORT");
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (bind(server_fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        close(server_fd);
        SERVER_ERROR("Failed to bind");
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        close(server_fd);
        SERVER_ERROR("Failed to listen");
    }

    if (make_non_blocking(server_fd) == false) {
        close(server_fd);
        SERVER_ERROR("Failed to set non blocking socket");
    }

    int efd = epoll_create1(EPOLL_CLOEXEC);
    if (efd == -1) {
        close(server_fd);
        SERVER_ERROR("Failed to spawn epoll for server fd");
    }

    struct epoll_event ev = { 
        .events = EPOLLIN, 
        .data.fd = server_fd 
    };

    if (epoll_ctl(efd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        close(server_fd);
        SERVER_ERROR("Failed to add server to epoll");
    }  

    struct epoll_event events[MAX_EVENTS] = {0};
    sconnection conns[MAX_EVENTS] = {0};

    while (1) {
        int fd_count = epoll_wait(efd, events, MAX_EVENTS, -1);

        if (fd_count == -1) {
            if (errno == EINTR) continue;
            close(server_fd);
            SERVER_ERROR("Epoll wait failed");
        }

        for (int i = 0; i < fd_count; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_size = sizeof(client_addr);

                int client_fd = accept4(server_fd, &client_addr, &client_addr_size, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (client_fd == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    if (errno == EINTR) continue;

                    perror("Failed to accept client");
                    continue;
                }

                if (epoll_add(efd, client_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP) == false) {
                    perror("Failed to add client to epoll");
                    close(client_fd);
                    continue;
                }

                conns[client_fd].fd = client_fd;
                conns[client_fd].state = STATE_READ_HEADER;
                conns[client_fd].addr = client_addr;
                conns[client_fd].addr_size = client_addr_size;

                continue;
            }

            if (events[i].events & EPOLLIN) {
                if (handle_read(&conns[fd]) == false) {
                    epoll_remove(efd, fd);
                    free(conns[fd].buffer);
                    conns[fd].buffer = NULL;
                    close(conns[fd].fd);
                    memset(&conns[fd], 0, sizeof(conns[fd]));
                    continue;
                }
            }

            if (events[i].events & EPOLLOUT) {
                if (handle_write(&conns[fd]) == false) {
                    epoll_remove(efd, fd);
                    free(conns[fd].buffer);
                    conns[fd].buffer = NULL;
                    close(conns[fd].fd);
                    memset(&conns[fd], 0, sizeof(conns[fd]));
                    continue;
                }
            }

            if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                if (epoll_remove(efd, fd) == false) {
                    free(conns[fd].buffer);
                    close(conns[fd].fd);
                    memset(&conns[fd], 0, sizeof(conns[fd]));
                    close(server_fd);
                    SERVER_ERROR("Failed to remove client from epoll");
                }

                free(conns[fd].buffer);
                conns[fd].buffer = NULL;
                close(conns[fd].fd);
                memset(&conns[fd], 0, sizeof(conns[fd]));
            }
        }
    }

    close(server_fd);
}

int main(void) {
    int port = 8080;
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (fork() == 0) {
            run_worker(port);
            exit(0);
        }
    }

    while (wait(NULL) > 0);
}
