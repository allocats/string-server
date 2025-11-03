#include "ws_connection.h"

#include "../../utils/ws_debug.h"
#include "../../utils/ws_macros.h"

#include "ws_connection_types.h"

#include <sys/socket.h>
#include <unistd.h>

static u32 last_index = 0;

ws_Connection* ws_find_slot(i32 fd, ws_Connection* restrict conns, b32* restrict slots, u32 max) {
    for (u32 i = last_index; i < max; i++) {
        if (LIKELY(slots[i] == false)) {
            slots[i] = true;

            ws_Connection* conn = &conns[i];
            conn -> fd = fd;
            conn -> state = WS_READING;
            conn -> bytes_transferred = 0;

            return conn;
        }
    }

    for (u32 i = 0; i < last_index; i++) {
        if (slots[i] == false) {
            slots[i] = true;

            ws_Connection* conn = &conns[i];
            conn -> fd = fd;
            conn -> state = WS_READING;
            conn -> bytes_transferred = 0;

            return conn;
        }
    }

    return NULL;
}

__attribute__ ((always_inline)) inline
void ws_free_connection(ws_Connection* restrict ptr, ws_Connection* restrict conns, b32* restrict slots) {
    if (UNLIKELY(!ptr)) return;

    u32 index = ptr - conns;
    last_index = index;
    slots[index] = false;

    ptr -> bytes_transferred = 0;

    close(ptr -> fd);

    ws_debug_log(
        "Closed client"
    );
}
