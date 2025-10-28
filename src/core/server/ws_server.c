#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <unistd.h>
#endif /* ifndef _GNU_SOURCE */

#include "ws_server.h"
#include "ws_sockets.h"

#include "../common/ws_common.h"
#include "../connection/ws_connection.h"
#include "../io_uring/ws_uring.h"

#include "../../http/ws_http.h"
#include "../../utils/ws_debug.h"
#include "../../utils/ws_macros.h"

#include <liburing.h>
#include <stdio.h>

extern volatile sig_atomic_t running; 

__attribute__ ((hot))
void ws_start_server(const char* address, const u16 port) {
    struct io_uring ring;

    if (io_uring_queue_init(RING_SIZE, &ring, 0) < 0) {
        perror("io_uring_queue_init");
        _exit(1);
    }

    i32 server_fd = ws_create_tcp_server(address, port);
    ws_uring_add_accept(&ring, server_fd);
    io_uring_submit(&ring);

    ArenaAllocator arena = {0};
    init_arena(&arena, 16384);

    ws_Connection connections[MAX_CONNECTIONS] = {0};
    b32 connection_slots[MAX_CONNECTIONS] = {0};

    for (i32 i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i].buffer = arena_alloc(&arena, WS_DEFAULT_BUFFER_SIZE);
        connections[i].buffer_size = WS_DEFAULT_BUFFER_SIZE;
    }

    while (LIKELY(running == 1)) {
        struct io_uring_cqe* cqe;

        int ret = io_uring_wait_cqe(&ring, &cqe); 
        if (UNLIKELY(ret < 0)) {
            WS_ERR_CLOSE_AND_EXIT("io_uring_wait_cqe failed", server_fd, 1);
        } 

        ws_IoEvent* event = (ws_IoEvent*) io_uring_cqe_get_data(cqe);
        if (UNLIKELY(!event)) {
            io_uring_cqe_seen(&ring, cqe);
            continue;
        }

        int res = cqe -> res;

        switch (event -> type) {
            case WS_IO_EVENT_ACCEPT: {
                if (UNLIKELY(res < 0)) {
                    perror("Accept failed\n");
                    break;
                }

                ws_Connection* conn = ws_find_slot(res, connections, connection_slots, MAX_CONNECTIONS);
                if (UNLIKELY(!conn)) {
                    ws_debug_log(
                        "[Line %d]: Connection pool is full!", 
                        __LINE__
                    );

                    close(res);
                    break;
                }

                conn -> fd = res;
                ws_uring_add_read(&ring, conn);

                ws_debug_log(
                    "[Line %d] Added client, fd: %d. Reading request", 
                    __LINE__, conn -> fd
                );

                break;
            }

            case WS_IO_EVENT_CLIENT: {
                ws_Connection* conn = event -> conn;

                switch (conn -> state) {
                    case WS_READING: {
                        if (UNLIKELY(res <= 0)) {
                            ws_free_connection(conn, connections, connection_slots);

                            ws_debug_log(
                                "[Line %d] Closing client %d", 
                                __LINE__, conn -> fd
                            );
                            
                            break;
                        }

                        conn -> bytes_read += res;

                        ws_debug_log(
                            "[Line %d] Parsing %u bytes",
                            __LINE__, conn -> bytes_read
                        );

                        ws_HttpParseResult result;
                        const ws_Asset* asset = ws_parse_request(conn, &result);

                        if (LIKELY(result == WS_HTTP_PARSE_OK)) {
                            conn -> state = WS_RESPOND;
                            ws_uring_add_write(&ring, conn, asset);

                            ws_debug_log(
                                "[Line %d] Sent %zu bytes to client %d", 
                                __LINE__, asset -> size, conn -> fd
                            );

                            break;
                        } else if (LIKELY(result == WS_HTTP_PARSE_ERROR)) {
                            ws_uring_add_write(&ring, conn, asset);
                            conn -> state = WS_DONE;

                            ws_debug_log(
                                "[Line %d] Bad request for client %d", 
                                __LINE__, conn -> fd
                            );

                            break;
                        } else {
                            ws_uring_add_read(&ring, conn);

                            ws_debug_log(
                                "[Line %d] Incomplete request for client %d", 
                                __LINE__, conn -> fd
                            );

                            break;
                        }
                    }

                    case WS_RESPOND: {
                        if (UNLIKELY(res < 0)) {
                            ws_debug_log(
                                "[Line %d] Closing client %d", 
                                __LINE__, conn -> fd
                            );

                            ws_free_connection(conn, connections, connection_slots);
                            break;
                        }

                        conn -> bytes_read = 0;
                        conn -> state = WS_READING;
                        ws_uring_add_read(&ring, conn);

                        ws_debug_log(
                            "[Line %d] Moving client %d to reading state", 
                            __LINE__, conn -> fd
                        );

                        break;
                    }

                    case WS_DONE: {
                        ws_debug_log(
                            "[Line %d] Client is done, closing %d", 
                            __LINE__, conn -> fd
                        );

                        ws_free_connection(conn, connections, connection_slots);
                        break;
                    }
                }
                
                break;
            }
        }

        io_uring_cqe_seen(&ring, cqe);
        io_uring_submit(&ring);
    } 

    io_uring_queue_exit(&ring);
    arena_free(&arena);
    close(server_fd);
} 
