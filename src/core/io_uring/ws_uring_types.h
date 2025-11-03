#pragma once
#ifndef WS_URING_TYPES_H
#define WS_URING_TYPES_H

#include "../../utils/ws_types.h"
#include "../connection/ws_connection_types.h"

typedef enum {
    WS_IO_EVENT_ACCEPT,
    WS_IO_EVENT_CLIENT,
} ws_IoEventType;

typedef struct __attribute__ ((aligned(16))) {
    union {
        i32 server_fd;
        ws_Connection *conn;
    };
    ws_IoEventType type;
    u8 _padding[4];
} ws_IoEvent;

typedef enum __attribute__ ((packed)) {
    WS_SENDFILE_NEW,
    WS_SENDFILE_TO_PIPE,
    WS_SENDFILE_TO_SOCKET,
    WS_SENDFILE_OK,
    WS_SENDFILE_ERR,
} ws_SendfileState;

typedef struct __attribute__ ((aligned(32))) {
    i32 in_fd;
    i32 out_fd;
    i32 pipe[2];
    u32 file_offset;
    u32 bytes_to_transfer;
    u32 bytes_in_pipe;
    ws_SendfileState state; 
    // u8 _padding[1];
} ws_SendfileCtx;

#endif // !WS_URING_TYPES_H
