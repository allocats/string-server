#include "ws_uring.h"

#include "../../utils/ws_macros.h"

#include <liburing.h>
#include <stdio.h>

static inline ws_IoEvent* ws_uring_alloc_event(void) {
    ws_IoEvent* event = &WS_IO_EVENTS[WS_IO_EVENT_INDEX];
    WS_IO_EVENT_INDEX = (WS_IO_EVENT_INDEX + 1) & (MAX_IO_EVENTS - 1);
    return event;
}

void ws_uring_add_accept(struct io_uring* ring, i32 server_fd) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        fprintf(stderr, "Failed to get SQE for accpet\n");
        return;
    }

    WS_ACCEPT_EVENT.server_fd = server_fd;
    io_uring_prep_multishot_accept(sqe, server_fd, NULL, NULL, 0);
    io_uring_sqe_set_data(sqe, &WS_ACCEPT_EVENT);
}

void ws_uring_add_read(struct io_uring* ring, ws_Connection* conn) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        fprintf(stderr, "Failed to get SQE for read\n");
        return;
    }

    ws_IoEvent* event = ws_uring_alloc_event();
    event -> type = WS_IO_EVENT_CLIENT;
    event -> conn = conn;

    conn -> state = WS_READING;

    io_uring_prep_recv(sqe, 
                       conn -> fd, 
                       conn -> buffer + conn -> bytes_read, 
                       conn -> buffer_size - conn -> bytes_read, 
                       0);
    io_uring_sqe_set_data(sqe, event);
}

void ws_uring_add_write(struct io_uring* ring, ws_Connection* conn, const ws_Asset* asset) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (UNLIKELY(!sqe)) {
        fprintf(stderr, "Failed to get SQE for write\n");
        return;
    }

    ws_IoEvent* event = ws_uring_alloc_event();
    event -> type = WS_IO_EVENT_CLIENT;
    event -> conn = conn;

    conn -> state = WS_RESPOND;

    io_uring_prep_send(sqe, conn -> fd, asset -> response, asset -> size, MSG_ZEROCOPY);
    io_uring_sqe_set_data(sqe, event);
}
