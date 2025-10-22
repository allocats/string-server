#ifndef WS_NET_H
#define WS_NET_H

#include "ws_common.h"

i32 ws_create_tcp_server(const char* address, const u16 port);
i32 ws_make_nonblocking(const i32 fd);

#endif // !define WS_NET_H
