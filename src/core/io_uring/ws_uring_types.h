#pragma once
#ifndef WS_URING_TYPES_H
#define WS_URING_TYPES_H

#include "../../utils/ws_types.h"
#include "../connection/ws_connection_types.h"

typedef enum {
    WS_IO_EVENT_ACCEPT,
    WS_IO_EVENT_CLIENT,
} ws_IoEventType;

typedef struct {
    ws_IoEventType type;
    union {
        i32 server_fd;
        ws_Connection *conn;
    };
} ws_IoEvent;

#endif // !WS_URING_TYPES_H
