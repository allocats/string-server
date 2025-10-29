#pragma once
#ifndef WS_HTTP_H
#define WS_HTTP_H

#include "ws_http_types.h"

#include "../core/connection/ws_connection_types.h"
#include "../assets/ws_assets.h"

#define WS_HTTP_MAX_URI_LEN 1024
#define WS_HTTP_MAX_METHOD_LEN 8

const ws_Asset* ws_parse_request(ws_Connection* conn, ws_HttpParseResult* status);

#endif // !WS_HTTP_H
