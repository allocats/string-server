#pragma once
#ifndef WS_HTTP_ERRORS_H
#define WS_HTTP_ERRORS_H

#include "../assets/ws_assets.h"

static const ws_Asset WS_HTTP_ERROR_400 = {
    .hash = 0x0,
    .size = 75,
    .response = "HTTP/1.1 400 Bad Request\r\n"
                "Content-Length: 11\r\n"
                // "Connection: keep-alive\r\n"
                "Connection: close\r\n"
                "\r\n"
                "Bad Request"
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
};

#endif // !WS_HTTP_ERRORS_H
