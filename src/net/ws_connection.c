#include "ws_connection.h"

ws_Connection* ws_create_conn(ArenaAllocator* arena, const int fd) {
    ws_Connection* conn = arena_alloc(arena, sizeof(*conn));

    conn -> state = WS_READING;
    conn -> fd = fd;

    return conn;
}
