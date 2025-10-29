#include "ws_http.h"

#include "../utils/ws_debug.h"
#include "../utils/ws_macros.h"

#include "ws_http_errors.h"
#include "ws_http_types.h"

// #include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
*
*   Little endian, need to add a check and support for big vs little endian
*
*/

#define WS_HTTP_METHOD_GET_U32  0x20544547u 
#define WS_HTTP_METHOD_POST_U32 0x54534F50u

static inline ws_HttpMethod ws_http_method_lookup(const char* p, u32 len) {
    if (UNLIKELY(len < 3 || len > WS_HTTP_MAX_METHOD_LEN)) {
        return WS_HTTP_METHOD_INVALID;
    }

    u32 word = *(u32*) p;

    switch (word) {
        case WS_HTTP_METHOD_GET_U32:
            return WS_HTTP_METHOD_GET;

        case WS_HTTP_METHOD_POST_U32:
            return WS_HTTP_METHOD_POST;

        default:
            return WS_HTTP_METHOD_INVALID;
    }
}

__attribute__ ((hot))
const ws_Asset* ws_parse_request(ws_Connection* restrict conn, ws_HttpParseResult* restrict status) {
    if (UNLIKELY(conn -> bytes_transferred > conn -> buffer_size)) {
        *status = WS_HTTP_PARSE_ERROR;
        return &WS_HTTP_ERROR_400;
    }

    u32 bytes_read = conn -> bytes_transferred;
    char* buffer = conn -> buffer;
    char* end = buffer + bytes_read;
    char* p = buffer;

    ws_debug_log(
        "Request received: %s\n",
        p
    );

    /*
    *
    *   Replace this in the future, memmem is slow
    *
    */
    // const char* header_end = memmem(buffer, bytes_read, "\r\n\r\n", 4);
    // if (UNLIKELY(header_end == NULL)) {
    //     *status = WS_HTTP_PARSE_INCOMPLETE; 
    //     return NULL;
    // }

    const ws_Asset* asset = NULL;
    ws_HttpMethod method; 

    const char* method_start = p;
    while (p < end) {
        if (*p++ == '/') break;
    }

    if (UNLIKELY(p >= end)) {
        *status = WS_HTTP_PARSE_ERROR;
        return &WS_HTTP_ERROR_400;
    }

    u32 len = p - method_start - 2;
    method = ws_http_method_lookup(method_start, len);
    if (UNLIKELY(method == WS_HTTP_METHOD_INVALID)) {
        *status = WS_HTTP_PARSE_ERROR;
        return &WS_HTTP_ERROR_400;
    }

    const char* uri_start = p;
    while (p < end) {
        const char c = *p;
        if (UNLIKELY(c == ' ' || c == '?' || c == '\r' || c == '\n')) break;
        p++;
    }

    if (UNLIKELY(p >= end)) {
        *status = WS_HTTP_PARSE_ERROR;
        return &WS_HTTP_ERROR_400;
    }

    u32 hash = ws_hash_fnv1a(uri_start, p - uri_start);
    asset = ws_lookup_asset(hash); 

    if (UNLIKELY(!asset)) {
        *status = WS_HTTP_PARSE_ERROR;
        return &WS_HTTP_ERROR_404;
    }

    switch (method) {
        case WS_HTTP_METHOD_GET: {
            *status = WS_HTTP_PARSE_OK;
            return asset;
        }

        case WS_HTTP_METHOD_POST: {
            *status = WS_HTTP_PARSE_OK;
            return asset;
        }

        default: {
            *status = WS_HTTP_PARSE_OK;
            return asset;
        }
    }

}
