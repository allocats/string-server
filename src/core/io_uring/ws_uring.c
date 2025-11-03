#define _GNU_SOURCE
#include "ws_uring.h"

#include "../../utils/ws_debug.h"
#include "../../utils/ws_macros.h"
#include "ws_uring_types.h"

#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define WS_PIPE_SIZE 65536

static __attribute__ ((always_inline)) inline 
ws_IoEvent* ws_uring_alloc_event(void) {
    ws_IoEvent* event = &WS_IO_EVENTS[WS_IO_EVENT_INDEX];
    WS_IO_EVENT_INDEX = (WS_IO_EVENT_INDEX + 1) & (MAX_IO_EVENTS - 1);
    return event;
}

void ws_uring_add_accept(struct io_uring* ring, i32 server_fd) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        io_uring_submit(ring);
        sqe = io_uring_get_sqe(ring);
        
        if (UNLIKELY(!sqe)) {
            fprintf(stderr, "Failed to get SQE for accpet\n");
            return;
        }
    }

    WS_ACCEPT_EVENT.server_fd = server_fd;
    io_uring_prep_multishot_accept(sqe, server_fd, NULL, NULL, 0);
    io_uring_sqe_set_data(sqe, &WS_ACCEPT_EVENT);
}

void ws_uring_add_read_request(struct io_uring* ring, ws_Connection* conn) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        io_uring_submit(ring);
        sqe = io_uring_get_sqe(ring);
        
        if (UNLIKELY(!sqe)) {
            fprintf(stderr, "Failed to get SQE for read\n");
            return;
        }
    }

    ws_IoEvent* event = ws_uring_alloc_event();
    event -> type = WS_IO_EVENT_CLIENT;
    event -> conn = conn;

    u32 bytes_read = conn -> bytes_transferred;

    io_uring_prep_recv(
        sqe,
        conn -> fd, 
        conn -> buffer + bytes_read, 
        conn -> buffer_size - bytes_read, 
        0
    );
    io_uring_sqe_set_data(sqe, event);
}

void ws_uring_add_write_response(struct io_uring* ring, ws_Connection* conn, const ws_Asset* asset) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        io_uring_submit(ring);
        sqe = io_uring_get_sqe(ring);
        
        if (UNLIKELY(!sqe)) {
            fprintf(stderr, "Failed to get SQE for write\n");
            return;
        }
    }

    ws_IoEvent* event = ws_uring_alloc_event();
    event -> type = WS_IO_EVENT_CLIENT;
    event -> conn = conn;

    u32 bytes_sent = conn -> bytes_transferred;

    io_uring_prep_send(
        sqe, 
        conn -> fd, 
        asset -> response + bytes_sent, 
        asset -> size - bytes_sent, 
        0 
    );

    io_uring_sqe_set_data(sqe, event);
}

void ws_uring_add_write_header(
    struct io_uring* ring, 
    ws_Connection* conn, 
    const ws_Asset* asset
) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        io_uring_submit(ring);
        sqe = io_uring_get_sqe(ring);
        
        if (UNLIKELY(!sqe)) {
            fprintf(stderr, "Failed to get SQE for write\n");
            return;
        }
    }

    ws_IoEvent* event = ws_uring_alloc_event();
    event -> type = WS_IO_EVENT_CLIENT;
    event -> conn = conn;

    u32 bytes_sent = conn -> bytes_transferred;

    ws_debug_log(
        "header=%u", bytes_sent
    );

    io_uring_prep_send(
        sqe, 
        conn -> fd, 
        asset -> response + bytes_sent, 
        asset -> header_len - bytes_sent, 
        0 
    );

    io_uring_sqe_set_data(sqe, event);
}

inline __attribute__ ((always_inline))
void ws_sendfile_init_ctx(ws_SendfileCtx* ctx, i32 in_fd, i32 out_fd, usize n_bytes) {
    if (UNLIKELY(pipe(ctx -> pipe) == -1)) {
        ctx -> state = WS_SENDFILE_ERR;
        return;
    }

    ctx -> in_fd = in_fd;
    ctx -> out_fd = out_fd;
    ctx -> bytes_to_transfer = n_bytes;
    ctx -> file_offset = 0;
    ctx -> bytes_in_pipe = 0;
    ctx -> state = WS_SENDFILE_TO_PIPE;
}

void ws_sendfile_to_pipe(struct io_uring* ring, ws_SendfileCtx* ctx, ws_Connection* conn) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        io_uring_submit(ring);
        sqe = io_uring_get_sqe(ring);

        if (UNLIKELY(!sqe)) {
            ctx -> state = WS_SENDFILE_ERR;
            fprintf(stderr, "Failed to get SQE for write\n");
            return;
        }
    }

    ws_IoEvent* event = ws_uring_alloc_event();
    event -> type = WS_IO_EVENT_CLIENT;
    event -> conn = conn;

    usize remaining = ctx -> bytes_to_transfer - ctx -> file_offset;
    usize chunk = remaining > WS_PIPE_SIZE ? WS_PIPE_SIZE : remaining;

    io_uring_prep_splice(
        sqe, 
        ctx -> in_fd, 
        ctx -> file_offset, 
        ctx -> pipe[1], 
        -1, 
        chunk,
        0//SPLICE_F_MOVE 
    );

    io_uring_sqe_set_data(sqe, event);
}

void ws_sendfile_to_socket(struct io_uring* ring, ws_SendfileCtx* ctx, ws_Connection* conn) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        io_uring_submit(ring);
        sqe = io_uring_get_sqe(ring);

        if (UNLIKELY(!sqe)) {
            ctx -> state = WS_SENDFILE_ERR;
            fprintf(stderr, "Failed to get SQE for write\n");
            return;
        }
    }

    ws_IoEvent* event = ws_uring_alloc_event();
    event -> type = WS_IO_EVENT_CLIENT;
    event -> conn = conn;

    io_uring_prep_splice(
        sqe, 
        ctx -> pipe[0], 
        -1, 
        ctx -> out_fd, 
        -1, 
        ctx -> bytes_in_pipe, 
        0//SPLICE_F_MOVE | SPLICE_F_MORE
    );

    io_uring_sqe_set_data(sqe, event);
}
