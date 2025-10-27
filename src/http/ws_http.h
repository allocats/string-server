#pragma once
#ifndef WS_HTTP_H
#define WS_HTTP_H

#include "../net/ws_connection.h"
#include "../assets/ws_assets.h"

typedef enum {
    WS_HTTP_PARSE_METHOD
} ws_HttpParseState;

typedef enum {
    WS_HTTP_PARSE_INCOMPLETE,
    WS_HTTP_PARSE_ERROR,
    WS_HTTP_PARSE_OK,
} ws_HttpParseResult;

const ws_Asset* ws_parse_request(ws_Connection* conn, ws_HttpParseResult* status);

#endif // !WS_HTTP_H
