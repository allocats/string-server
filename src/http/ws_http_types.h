#pragma once
#ifndef WS_HTTP_TYPES_H
#define WS_HTTP_TYPES_H

typedef enum {
    WS_HTTP_METHOD_GET,
    WS_HTTP_METHOD_POST,
    WS_HTTP_METHOD_INVALID
} ws_HttpMethod;

// typedef enum {
//     WS_HTTP_PARSE_METHOD,
//     WS_HTTP_PARSE_URI,
//     WS_HTTP_PARSE_VERSION,
//     WS_HTTP_PARSE_HEADERS,
//     WS_HTTP_PARSE_BODY,
//     WS_HTTP_PARSE_DONE,
// } ws_HttpParseState;

typedef enum {
    WS_HTTP_PARSE_INCOMPLETE,
    WS_HTTP_PARSE_ERROR,
    WS_HTTP_PARSE_OK,
} ws_HttpParseResult;

#endif // !WS_HTTP_TYPES_H
