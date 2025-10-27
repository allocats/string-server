#pragma once
#ifndef WS_CONNECTION_TYPES_H
#define WS_CONNECTION_TYPES_H

#include "../../utils/ws_types.h"

#define WS_BUFFER_SIZE 4096

typedef enum {
    WS_READING,
    WS_RESPOND,
    WS_DONE,
} ws_State;

typedef struct __attribute__ ((aligned(32))) {
    char* buffer;
    u32 buffer_size;
    u32 bytes_read;
    ws_State state;
    i32 fd;
    u8 _padding[8];
} ws_Connection;

#endif // !WS_CONNECTION_TYPES_H
