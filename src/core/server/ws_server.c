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

__attribute__ ((always_inline)) static inline
void ws_handle_accept(
    struct io_uring* restrict ring, 
    i32 res, 
    ws_Connection* connections,
    b32* connection_slots
) {
    if (UNLIKELY(res < 0)) {
        perror("Accept failed\n");
        return;
    }

    ws_Connection* conn = ws_find_slot(res, connections, connection_slots, MAX_CONNECTIONS);
    if (UNLIKELY(!conn)) {
        ws_debug_log(
            "[Line %d]: Connection pool is full!", 
            __LINE__
        );

        close(res);
        return;
    }

    conn -> fd = res;
    conn -> state = WS_READING;

    ws_uring_add_read_request(ring, conn);

    ws_debug_log(
        "[Line %d] Added client, fd: %d. Reading request", 
        __LINE__, conn -> fd
    );
}

__attribute__ ((always_inline)) static inline
void ws_handle_read(
    struct io_uring* restrict ring, 
    i32 res, 
    ws_Connection* conn, 
    ws_Connection* connections,
    b32* connection_slots,
    ws_SendfileCtx* sendfile_ctxs
) {
    if (UNLIKELY(res <= 0)) {
        ws_free_connection(conn, connections, connection_slots, sendfile_ctxs);

        ws_debug_log(
            "[Line %d] Closing client %d", 
            __LINE__, conn -> fd
        );

        return;
    }

    conn -> bytes_transferred += res;

    ws_debug_log(
        "[Line %d] Parsing %u bytes",
        __LINE__, conn -> bytes_transferred
    );

    ws_HttpParseResult result;
    const ws_Asset* asset = ws_parse_request(conn, &result);

    conn -> asset = asset;

    if (LIKELY(result == WS_HTTP_PARSE_OK)) {
        conn -> state = WS_RESPOND;
        conn -> bytes_transferred = 0;

        if (LIKELY(asset -> type == WS_ASSET_IN_MEMORY)) {
            ws_uring_add_write_response(ring, conn, asset);

            ws_debug_log(
                "READ: Memory, sending response"
            );
        } else {
            ws_uring_add_write_header(ring, conn, asset);

            ws_debug_log(
                "READ: File, sending header"
            );
        }

        ws_debug_log(
            "[Line %d] Write to client %d", 
            __LINE__, conn -> fd
        );

        return;
    } else if (LIKELY(result == WS_HTTP_PARSE_ERROR)) {
        conn -> state = WS_DONE;
        conn -> bytes_transferred = 0;

        ws_uring_add_write_response(ring, conn, asset);

        ws_debug_log(
            "[Line %d] Bad request for client %d", 
            __LINE__, conn -> fd
        );

        return;
    } else {
        ws_uring_add_read_request(ring, conn);

        ws_debug_log(
            "[Line %d] Incomplete request for client %d", 
            __LINE__, conn -> fd
        );

        return;
    }
}

__attribute__ ((always_inline)) static inline
void ws_handle_respond(
    struct io_uring* restrict ring, 
    i32 res, 
    ws_Connection* conn, 
    ws_Connection* connections,
    b32* connection_slots,
    ws_SendfileCtx* sendfile_ctxs
) {
    if (UNLIKELY(res < 0)) {
        ws_debug_log(
            "[Line %d] Closing client %d", 
            __LINE__, conn -> fd
        );

        ws_free_connection(conn, connections, connection_slots, sendfile_ctxs);
        return;
    }

    const ws_Asset* asset = conn -> asset;
    conn -> bytes_transferred += res;

    if (asset -> type == WS_ASSET_IN_MEMORY) {
        if (UNLIKELY(conn -> bytes_transferred < asset -> size)) {
            ws_debug_log(
                "[Line %d] Partial write for client %d", 
                __LINE__, conn -> fd
            );

            ws_uring_add_write_response(ring, conn, conn -> asset); 
            return;
        }

        conn -> bytes_transferred = 0;
        conn -> state = WS_READING;

        ws_uring_add_read_request(ring, conn);

        ws_debug_log(
            "[Line %d] Moving client %d to reading state", 
            __LINE__, conn -> fd
        );

        return;
    }

    ws_SendfileCtx* ctx = &sendfile_ctxs[conn - connections];

    if (UNLIKELY(conn -> bytes_transferred < asset -> header_len)) {
        ws_uring_add_write_header(ring, conn, asset);
        return;
    }     

    switch (ctx -> state) {
        case WS_SENDFILE_NEW: {
            conn -> bytes_transferred = 0;

            ws_sendfile_init_ctx(ctx, asset -> fd, conn -> fd, asset -> size);

            if (UNLIKELY(ctx -> state == WS_SENDFILE_ERR)) {
                ctx -> state = WS_SENDFILE_NEW;

                // TODO: Send server error
                ws_free_connection(conn, connections, connection_slots, sendfile_ctxs);
                return;
            }

            ctx -> state = WS_SENDFILE_TO_PIPE;
            ws_sendfile_to_pipe(ring, ctx, conn);
            return;
        }

        case WS_SENDFILE_TO_PIPE: {
            ws_debug_log(
                "SPLICE TO_PIPE completed: res=%d, file_offset was=%u, fd=%d", 
                res, ctx -> file_offset, conn -> fd
            );

            ctx -> file_offset += res;
            ctx -> bytes_in_pipe = res;
            ctx -> state = WS_SENDFILE_TO_SOCKET;

            ws_sendfile_to_socket(ring, ctx, conn);

            return;
        }

        case WS_SENDFILE_TO_SOCKET: {
            ws_debug_log(
                "SPLICE_TO_SOCKET done res=%d, bytes_in_pipe was=%u, file_offset=%u/%u", 
                res, ctx -> bytes_in_pipe, ctx -> file_offset, ctx -> bytes_to_transfer
            );

            ctx -> bytes_in_pipe -= res;

            if (ctx -> file_offset < ctx -> bytes_to_transfer) {
                ctx -> state = WS_SENDFILE_TO_PIPE;
                ws_sendfile_to_pipe(ring, ctx, conn);
                return;
            }

            ctx -> state = WS_SENDFILE_NEW;
            ws_sendfile_close_pipe(ctx);
            
            conn -> bytes_transferred = 0;
            conn -> state = WS_READING;

            ws_uring_add_read_request(ring, conn);

            return;
        }

        case WS_SENDFILE_OK: {
            ctx -> state = WS_SENDFILE_NEW;
            ws_sendfile_close_pipe(ctx);

            conn -> bytes_transferred = 0;
            conn -> state = WS_READING;

            ws_uring_add_read_request(ring, conn);

            return;
        }

        case WS_SENDFILE_ERR: {
            ctx -> state = WS_SENDFILE_NEW;
            ws_free_connection(conn, connections, connection_slots, sendfile_ctxs);
            return;
        }
    }
}

__attribute__ ((hot))
void ws_start_server(const char* address, const u16 port) {
    if (ws_assets_load() == -1) {
        fprintf(stderr, "Failed to load assets. Closing");
        return;
    }

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

    ws_SendfileCtx sendfile_ctxs[MAX_CONNECTIONS] = {0};
    ws_Connection connections[MAX_CONNECTIONS] = {0};
    b32 connection_slots[MAX_CONNECTIONS] = {0};

    for (i32 i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i].buffer = arena_alloc(&arena, WS_DEFAULT_BUFFER_SIZE);
        connections[i].buffer_size = WS_DEFAULT_BUFFER_SIZE;
        arena_memset(connections[i].buffer, 0, WS_DEFAULT_BUFFER_SIZE);

        sendfile_ctxs[i].state = WS_SENDFILE_NEW;
    }

    while (LIKELY(running == 1)) {
        struct io_uring_cqe* cqe;

        i32 ret = io_uring_wait_cqe(&ring, &cqe); 
        if (UNLIKELY(ret < 0)) {
            if (errno == -EINTR) continue;
            WS_ERR_CLOSE_AND_EXIT("io_uring_wait_cqe failed", server_fd, 1);
        } 

        ws_IoEvent* event = (ws_IoEvent*) io_uring_cqe_get_data(cqe);
        if (UNLIKELY(!event)) {
            io_uring_cqe_seen(&ring, cqe);
            continue;
        }

        i32 res = cqe -> res;

        switch (event -> type) {
            case WS_IO_EVENT_ACCEPT: {
                ws_handle_accept(&ring, res, connections, connection_slots);
                break;
            }

            case WS_IO_EVENT_CLIENT: {
                ws_Connection* conn = event -> conn;

                switch (conn -> state) {
                    case WS_READING: {
                        ws_handle_read(
                            &ring, 
                            res, 
                            conn, 
                            connections, 
                            connection_slots,
                            sendfile_ctxs
                        );
                        break;
                    }

                    case WS_RESPOND: {
                        ws_handle_respond(
                            &ring, 
                            res, 
                            conn, 
                            connections, 
                            connection_slots,
                            sendfile_ctxs
                        );
                        break;
                    }

                    case WS_DONE: {
                        ws_debug_log(
                            "[Line %d] Client is done, closing %d", 
                            __LINE__, conn -> fd
                        );

                        ws_free_connection(
                            conn, 
                            connections, 
                            connection_slots, 
                            sendfile_ctxs
                        );
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
    ws_assets_close();
} 
