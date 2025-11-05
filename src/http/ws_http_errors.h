#pragma once
#ifndef WS_HTTP_ERRORS_H
#define WS_HTTP_ERRORS_H

#include "../assets/ws_assets_types.h"

static const ws_Asset WS_HTTP_ERROR_400 = {
    .hash = 0x0,
    .size = 79,
    .response = "HTTP/1.1 400 Bad Request\r\n"
                "Content-Length: 11\r\n"
                // "Connection: keep-alive\r\n"
                "Connection: close\r\n"
                "\r\n"
                "Bad Request!",
    .type = WS_ASSET_IN_MEMORY,
};

static const ws_Asset WS_HTTP_ERROR_404 = {
    .hash = 0x0,
    .size = 73,
    .response = "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: 9\r\n"
                // "Connection: keep-alive\r\n"
                "Connection: close\r\n"
                "\r\n"
                "Not Found",
    .type = WS_ASSET_IN_MEMORY,
};

#endif // !WS_HTTP_ERRORS_H
