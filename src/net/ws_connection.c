#include "ws_connection.h"
#include "ws_server.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

ws_Connection* ws_find_slot(i32 fd, ws_Connection* conns, b32* slots, u32 max) {
    for (u32 i = 0; i < max; i++) {
        if (slots[i] == false) {
            slots[i] = true;

            ws_Connection* conn = &conns[i];
            conn -> fd = fd;
            conn -> state = WS_READING;

            return conn;
        }
    }

    return NULL;
}

void ws_free_connection(ws_Connection* ptr, ws_Connection* conns, b32* slots) {
    if (!ptr) return;

    u32 index = ptr - conns;
    assert(index < MAX_EVENTS && "Index out of range!\n");

    slots[index] = false;
    close(ptr -> fd);
}
