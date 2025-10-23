#ifndef WS_HTTP_H
#define WS_HTTP_H

#include "../net/ws_connection.h"
#include "../assets/ws_assets.h"

#define WS_DEBUG_RESPONSE "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK"
#define WS_DEBUG_RESPONSE_LEN strlen(WS_DEBUG_RESPONSE)

const ws_Asset* ws_parse_request(ws_Connection* conn);
// void ws_parse_request(ws_Connection* conn);

ssize_t ws_read(ws_Connection* conn);
ssize_t ws_write(ws_Connection* conn, const char* buffer, const size_t len);

#endif // !WS_HTTP_H
