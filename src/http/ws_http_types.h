#pragma once
#ifndef WS_HTTP_TYPES_H
#define WS_HTTP_TYPES_H

typedef enum {
    WS_HTTP_METHOD_GET,
    WS_HTTP_METHOD_POST,
    WS_HTTP_METHOD_INVALID
} ws_HttpMethod;

typedef enum {
    WS_HTTP_PARSE_INCOMPLETE,
    WS_HTTP_PARSE_ERROR,
    WS_HTTP_PARSE_OK,
} ws_HttpParseResult;

typedef struct __attribute__ ((aligned(16))) {
    const char* key;
    const char* value;
} ws_HttpParams;

#endif // !WS_HTTP_TYPES_H
