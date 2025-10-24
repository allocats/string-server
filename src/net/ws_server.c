#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <unistd.h>
#endif /* ifndef _GNU_SOURCE */

#include "ws_server.h"

#include "ws_common.h"
#include "ws_connection.h"
#include "ws_epoll.h"
#include "ws_net.h"

#include "../http/ws_http.h"
#include "../utils/ws_macros.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>

extern volatile sig_atomic_t running; 

__attribute__ ((hot))
void ws_start_server(const char* address, const u16 port) {
    i32 server_fd = ws_create_tcp_server(address, port);
    i32 efd = ws_epoll_init_server(server_fd);

    ArenaAllocator arena = {0};
    init_arena(&arena, 16384);

    struct epoll_event events[MAX_EVENTS] = {0};
    ws_Connection connections[MAX_EVENTS] = {0};
    b32 connection_slots[MAX_EVENTS] = {0};

    for (i32 i = 0; i < MAX_EVENTS; i++) {
        connections[i].buffer = arena_alloc(&arena, WS_BUFFER_SIZE);
        connections[i].buffer_size = WS_BUFFER_SIZE;
    }

    while (LIKELY(running == 1)) {
        i32 nfds = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (UNLIKELY(nfds == -1)) {
            if (LIKELY(errno == EINTR)) continue;
            WS_ERR_CLOSE_AND_EXIT("Epoll_wait() failed", server_fd, 1);
        }

        for (i32 i = 0; i < nfds; i++) {
            struct epoll_event current_event = events[i];

            ws_Connection* current_conn = current_event.data.ptr;
            i32 current_fd = current_event.data.fd;

            if (current_fd == server_fd) {
                i32 client_fd = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (UNLIKELY(client_fd == -1)) {
                    if (LIKELY(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) continue;
                    perror("Failed to accept client");
                    continue;
                }

                i32 flag = 1;
                setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
                
                ws_Connection* conn = ws_find_slot(client_fd, connections, connection_slots, MAX_EVENTS);
                if (UNLIKELY(!conn)) {
                    close(client_fd);
                    continue;
                }

                struct epoll_event ev = {
                    .events = EPOLLIN | EPOLLRDHUP | EPOLLET,
                    .data.ptr = conn
                };

                if (UNLIKELY(epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &ev) == -1)) {
                    WS_ERR_CLOSE_AND_EXIT("Failed to add client to epoll", client_fd, 1);
                }

                conn -> state = WS_READING;

                continue;
            } 

            if (current_event.events & EPOLLIN) {
                if (LIKELY(current_conn -> state == WS_READING)) {
                    ssize_t n = ws_read(current_conn);

                    if (UNLIKELY(n == 1)) {
                        continue;
                    } else if (UNLIKELY(n == -1)) {
                        ws_epoll_remove(efd, current_fd);
                        ws_free_connection(current_event.data.ptr, connections, connection_slots);
                        continue;
                    }

                    const ws_Asset* asset = ws_parse_request(current_conn);
                    if (UNLIKELY(!asset)) {
                        ws_epoll_remove(efd, current_fd);
                        ws_free_connection(current_event.data.ptr, connections, connection_slots);
                        continue;
                    }

                    n = ws_write(current_conn, asset -> response, asset -> size);
                    if (LIKELY(n != -1)) {
                        current_conn -> state = WS_DONE;
                    } else {
                        struct epoll_event ev = {
                            .events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
                            .data.ptr = current_conn
                        };

                        epoll_ctl(efd, EPOLL_CTL_MOD, current_conn -> fd, &ev);
                        current_conn -> state = WS_RESPOND;
                    }
                }
            }

            if (current_event.events & EPOLLOUT) {
                if (LIKELY(current_conn -> state == WS_RESPOND)) {
                    const ws_Asset* asset = ws_parse_request(current_conn);
                    if (UNLIKELY(!asset)) {
                        ws_epoll_remove(efd, current_fd);
                        ws_free_connection(current_event.data.ptr, connections, connection_slots);
                        continue;
                    }

                    if (UNLIKELY(ws_write(current_conn, asset -> response, asset -> size) != -1)) {
                        ws_epoll_remove(efd, current_fd);
                        ws_free_connection(current_event.data.ptr, connections, connection_slots);
                        continue;
                    }
                }
            }

            if (current_event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                ws_epoll_remove(efd, current_fd);
                ws_free_connection(current_event.data.ptr, connections, connection_slots);
                continue;
            }

            if (current_conn -> state == WS_DONE) {
                ws_epoll_remove(efd, current_fd);
                ws_free_connection(current_event.data.ptr, connections, connection_slots);
            }
        }
    } 

    arena_free(&arena);
    close(server_fd);
} 
