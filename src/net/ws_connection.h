#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include "ws_common.h"

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

ws_Connection* ws_find_slot(i32 fd, ws_Connection* conns, b32* slots, u32 max);
void ws_free_connection(ws_Connection* ptr, ws_Connection* conns, b32* slots);

#endif // !WS_CONNECTION_H
