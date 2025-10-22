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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>

extern volatile sig_atomic_t running; 

void ws_start_server(const char* address, const u16 port) {
    i32 server_fd = ws_create_tcp_server(address, port);
    i32 efd = ws_epoll_init_server(server_fd);

    ArenaAllocator arena = {0};
    init_arena(&arena, 0);

    struct epoll_event events[MAX_EVENTS] = {0};
    ws_Connection connections[MAX_EVENTS] = {0};
    b32 connection_slots[MAX_EVENTS] = {0};

    for (i32 i = 0; i < MAX_EVENTS; i++) {
        connections[i].buffer = arena_alloc(&arena, WS_BUFFER_SIZE);
        connections[i].buffer_size = WS_BUFFER_SIZE;
    }

    while (running == 1) {
        i32 nfds = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) continue;
            WS_ERR_CLOSE_AND_EXIT("Epoll_wait() failed", server_fd, 1);
        }

        for (i32 i = 0; i < nfds; i++) {
            struct epoll_event current_event = events[i];

            ws_Connection* current_conn = current_event.data.ptr;
            i32 current_fd = current_event.data.fd;

            if (current_fd == server_fd) {
                i32 client_fd = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (client_fd == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) continue;
                    perror("Failed to accept client");
                    continue;
                }
                
                ws_Connection* conn = ws_find_slot(client_fd, connections, connection_slots, MAX_EVENTS);
                if (!conn) {
                    continue;
                }

                struct epoll_event ev = {
                    .events = EPOLLIN | EPOLLRDHUP | EPOLLET,
                    .data.ptr = conn
                };

                if (epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                    WS_ERR_CLOSE_AND_EXIT("Failed to add client to epoll", client_fd, 1);
                }

                conn -> state = WS_READING;

                #ifdef WS_DEBUG_LOGS
                printf("LOG: Accepted client %d\n", client_fd);
                #endif /* ifdef WS_DEBUG_LOGS */

                continue;
            } 

            if (current_event.events & EPOLLIN) {
                if (current_conn -> state == WS_READING) {
                    ssize_t n = ws_debug_read(current_conn);
                    #ifdef WS_DEBUG_LOGS
                    printf("LOG: Read %zu bytes\n", n);
                    #endif /* ifdef WS_DEBUG_LOGS */

                    n = ws_debug_response(current_conn);
                    if (n != -1) {
                        #ifdef WS_DEBUG_LOGS
                        printf("LOG: Sent %zu bytes\n", n);
                        #endif

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
                if (current_conn -> state == WS_RESPOND) {
                    if (ws_debug_response(current_conn) != -1) {
                        #ifdef WS_DEBUG_LOGS
                        printf("LOG: Sent response\n");
                        #endif /* ifdef WS_DEBUG_LOGS */
                    }
                }
            }

            if (current_event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                current_conn->state = WS_DONE;
            }

            if (current_conn -> state == WS_DONE) {
                #ifdef WS_DEBUG_LOGS
                ws_Connection* conn = current_event.data.ptr;
                i32 debug_fd = conn -> fd; 
                #endif /* ifdef WS_DEBUG_LOGS */

                if (ws_epoll_remove(efd, current_fd) == -1) {
                    // handle errors, check errno
                }

                ws_free_connection(current_event.data.ptr, connections, connection_slots);

                #ifdef WS_DEBUG_LOGS
                printf("LOG: Closed client %d\n", debug_fd);
                #endif /* ifdef WS_DEBUG_LOGS */
            }
        }
    } 

    arena_free(&arena);
    close(server_fd);
} 
