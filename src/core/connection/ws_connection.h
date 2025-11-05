#pragma once
#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include "ws_connection_types.h"
#include "../io_uring/ws_uring.h"

ws_Connection* ws_find_slot(i32 fd, ws_Connection* conns, b32* slots, u32 max);
void ws_free_connection(
    ws_Connection* ptr, 
    ws_Connection* conns, 
    b32* slots, 
    ws_SendfileCtx* ctxs
);

#endif // !WS_CONNECTION_H
