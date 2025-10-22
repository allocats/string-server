#include "ws_connection.h"

#include <stdbool.h>
#include <string.h>

ws_Connection* ws_find_slot(i32 fd, ws_Connection* conns, b32* slots, u32 max) {
    for (u32 i = 0; i < max; i++) {
        if (slots[i] == false) {
            slots[i] = true;

            ws_Connection* conn = &conns[i];
            conn -> fd = fd;
            conn -> state = WS_READING;

            // if (conn -> buffer[0] != 0) {
            //     memset(conn -> buffer, 0, conn -> buffer_size);
            // }

            return conn;
        }
    }

    return NULL;
}

void ws_free_connection(ws_Connection* ptr, ws_Connection* conns, b32* slots) {
    if (!ptr) return;

    u32 index = ptr - conns;
    slots[index] = false;

    close(ptr -> fd);
}
