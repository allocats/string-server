#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include "ws_common.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    WS_READING,
    WS_PARSING,
    WS_RESPOND,
    WS_DONE,
} ws_State;

typedef struct {
    ws_State state;
    char* buffer;
    size_t buffer_size;
    int fd;
} ws_Connection;

ws_Connection* ws_find_slot(ArenaAllocator* arena, int fd, ws_Connection* conns, bool* slots, uint32_t max);
void ws_free_connection(ws_Connection* ptr, ws_Connection* conns, bool* slots);

#endif // !WS_CONNECTION_H
