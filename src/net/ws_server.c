#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* ifndef _GNU_SOURCE */

#include "ws_server.h"

#include "ws_common.h"
#include "ws_connection.h"
#include "ws_epoll.h"
#include "ws_net.h"

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>

void ws_start_server(const char* address, const uint16_t port) {
    int server_fd = ws_create_tcp_server(address, port);
    int efd = ws_epoll_init_server(server_fd);

    ArenaAllocator arena = {0};
    init_arena(&arena, 0);

    struct epoll_event events[MAX_EVENTS];

    uint32_t n_connections = 0;
    bool arena_needs_reset = false;

    while (1) {
        int nfds = epoll_wait(efd, events, MAX_EVENTS, 0);
        if (nfds == -1) {
            if (errno == EINTR) continue;
            WS_ERR_CLOSE_AND_EXIT("Epoll_wait() failed", server_fd, 1);
        }

        for (int i = 0; i < nfds; i++) {
            struct epoll_event* current_event = &events[i];
            int current_fd = current_event -> data.fd;

            if (current_fd == server_fd) {
                int client_fd = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (client_fd == -1) {

                    // handle errno 
                    
                    perror("Failed to accept client");
                    continue;
                }

                if (ws_epoll_add_fd(efd, client_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP) == -1) {
                    WS_ERR_CLOSE_AND_EXIT("Failed to add client to epoll", client_fd, 1);
                }

                printf("LOG: Accepted client %d\n", client_fd);

                current_event -> data.ptr = ws_create_conn(&arena, client_fd);
                n_connections++;
                continue;
            } 

            if (current_event -> events & EPOLLIN) {
                // handle read
            }

            if (current_event -> events & EPOLLOUT) {
                // handle write
            }

            if (current_event -> events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                if (ws_epoll_remove(efd, current_fd) == -1) {
                    // handle errors, check errno
                }
                n_connections--;
                arena_needs_reset = true;

                printf("LOG: Closed client %d\n", current_fd);
            }
        }

        if (n_connections == 0 && arena_needs_reset) {
            arena_reset(&arena);
            printf("LOG: Resetting arena\n");
            arena_needs_reset = false;
        }
    } 
} 
