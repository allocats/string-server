#include "ws_http.h"

#include "../utils/ws_macros.h"

#include "ws_http_errors.h"

// #include <errno.h>
// #include <stdio.h>
// #include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define WS_IS_GET(n) (n[0] == 'G' && n[1] == 'E' && n[2] == 'T')
#define WS_IS_POST(n) (n[0] == 'P' && n[1] == 'O' && n[2] == 'S' && n[3] == 'T')

__attribute__ ((hot))
const ws_Asset* ws_parse_request(ws_Connection* restrict conn, ws_HttpParseResult* restrict status) {
    char* buffer = conn -> buffer;
    char* end = buffer + conn -> bytes_read;
    char* p = buffer;

    if (LIKELY(WS_IS_GET(buffer))) {
        p += 4;

        while (p < end) {
            if (*p++ == '/') break;
        }

        const char* start = p;

        while (p < end) {
            char c = *p;
            if (UNLIKELY(c == ' ' || c == '?' || c == '\r' || c == '\n')) break;
            p++;
        }

        u32 len = p - start;
        u32 hash = ws_hash_fnv1a(start, len);

        const ws_Asset* asset = ws_lookup_asset(hash);
        if (UNLIKELY(!asset)) {
            *status = WS_HTTP_PARSE_ERROR;
            return &WS_HTTP_ERROR_404;
        }

        *status = WS_HTTP_PARSE_OK;
        return asset;
    }

    *status = WS_HTTP_PARSE_ERROR;
    return &WS_HTTP_ERROR_400;
}

// __attribute__ ((hot))
// ws_Asset* ws_new_parse_request(ws_Connection* restrict conn, ws_HttpParseResult* restrict status) {
//     char* p = conn -> buffer;
//     ws_HttpParseState state = conn -> parse_state;
//
//     return NULL;
// }
