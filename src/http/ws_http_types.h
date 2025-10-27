#pragma once
#ifndef WS_HTTP_TYPES_H
#define WS_HTTP_TYPES_H

typedef enum {
    WS_HTTP_PARSE_METHOD
} ws_HttpParseState;

typedef enum {
    WS_HTTP_PARSE_INCOMPLETE,
    WS_HTTP_PARSE_ERROR,
    WS_HTTP_PARSE_OK,
} ws_HttpParseResult;

#endif // !WS_HTTP_TYPES_H
