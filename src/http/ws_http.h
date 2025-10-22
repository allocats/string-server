#ifndef WS_HTTP_H
#define WS_HTTP_H

#include "../net/ws_connection.h"

i32 ws_debug_response(ws_Connection* conn);
i32 ws_debug_read(ws_Connection* conn);

#endif // !WS_HTTP_H
