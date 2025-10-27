#pragma once
#ifndef WS_SOCKETS_H
#define WS_SOCKETS_H

#include "../../utils/ws_types.h"

i32 ws_create_tcp_server(const char* address, const u16 port);
i32 ws_make_nonblocking(const i32 fd);

#endif // !define WS_SOCKETS_H
