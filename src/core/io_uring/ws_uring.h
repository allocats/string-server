#pragma once
#ifndef WS_URING_H
#define WS_URING_H

#include "ws_uring_types.h"

#include "../../assets/ws_assets.h"
#include "../../utils/ws_types.h"

#include <liburing.h>

#define RING_SIZE 1024 
#define MAX_IO_EVENTS RING_SIZE * 4

static ws_IoEvent WS_ACCEPT_EVENT = {
    .type = WS_IO_EVENT_ACCEPT,
};

static ws_IoEvent WS_IO_EVENTS[MAX_IO_EVENTS];
static u32 WS_IO_EVENT_INDEX = 0;

void ws_uring_add_accept(struct io_uring* ring, i32 server_fd);
void ws_uring_add_read_request(struct io_uring* ring, ws_Connection* conn);
void ws_uring_add_write_response(struct io_uring* ring, ws_Connection* conn, const ws_Asset* asset); 
void ws_uring_add_write_header(struct io_uring* ring, ws_Connection* conn, const ws_Asset* asset); 

// Sendfile, caller is responsible for tracking the state
void ws_sendfile_init_ctx(ws_SendfileCtx* ctx, i32 in_fd, i32 out_fd, usize n_bytes);
void ws_sendfile_to_pipe(struct io_uring* ring, ws_SendfileCtx* ctx, ws_Connection* conn);
void ws_sendfile_to_socket(struct io_uring* ring, ws_SendfileCtx* ctx, ws_Connection* conn);
void ws_sendfile_close_pipe(ws_SendfileCtx* ctx);

#endif // !WS_URING_H
