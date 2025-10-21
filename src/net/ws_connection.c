#include "ws_connection.h"

#include <stdint.h>
#include <string.h>

ws_Connection* ws_find_slot(ArenaAllocator* arena, int fd, ws_Connection* conns, bool* slots, uint32_t max) {
    for (uint32_t i = 0; i < max; i++) {
        if (slots[i] == false) {
            slots[i] = true;

            ws_Connection* conn = &conns[i];

            conn -> fd = fd;
            conn -> state = WS_READING;

            if (!conn -> buffer) {
                conn -> buffer = arena_alloc(arena, 8192 / sizeof(uintptr_t)); // Arena allocates n * sizeof(uintptr_t)
                conn -> buffer_size = 8192;
            } 

            memset(conn -> buffer, 0, conn -> buffer_size);

            return conn;
        }
    }

    return NULL;
}

void ws_free_connection(ws_Connection* ptr, ws_Connection* conns, bool* slots) {
    if (!ptr) return;

    int index = ptr - conns;
    slots[index] = false;

    close(ptr -> fd);
}
