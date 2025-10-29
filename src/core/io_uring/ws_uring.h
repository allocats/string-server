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
void ws_uring_add_read(struct io_uring* ring, ws_Connection* conn);
void ws_uring_add_write(struct io_uring* ring, ws_Connection* conn, const ws_Asset* asset); 

#endif // !WS_URING_H
