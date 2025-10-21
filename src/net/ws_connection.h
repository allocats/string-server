#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include "ws_common.h"

#include <stdint.h>

typedef enum {
    WS_READING,
    WS_PARSING,
    WS_RESPOND
} ws_State;

typedef struct {
    ws_State state;
    char* buffer;
    uint32_t length;
    uint32_t net_length;
    int fd;
} ws_Connection;

ws_Connection* ws_create_conn(ArenaAllocator* arena, const int fd);

#endif // !WS_CONNECTION_H
