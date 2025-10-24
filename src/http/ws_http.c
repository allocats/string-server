#include "ws_http.h"

#include "../assets/ws_assets.h"
#include "../utils/ws_macros.h"

#include <asm-generic/errno.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// void ws_send_error(ws_Connection* conn, i32 code) {
//     const char* response;
//     size_t size;
//
//     switch (code) {
//         default: 
//             break;
//     }
// }

#define WS_IS_GET(n) (n[0] == 'G' && n[1] == 'E' && n[2] == 'T')
#define WS_IS_POST(n) (n[0] == 'P' && n[1] == 'O' && n[2] == 'S' && n[3] == 'T')

// void ws_parse_request(ws_Connection* conn) {
__attribute__ ((hot))
const ws_Asset* ws_parse_request(ws_Connection* conn) {
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
            return NULL;
        }

        return asset;
    }

    return NULL;
}


/*
*
* -1 = error
*  0 = ok
*  1 = eagain
*
*/
ssize_t ws_read(ws_Connection* conn) {
    i32 fd = conn -> fd;
    char* buffer = conn -> buffer;
    size_t buffer_size = conn -> buffer_size;
    size_t bytes_read = conn -> bytes_read;

    ssize_t n = recv(fd, buffer + bytes_read, buffer_size - bytes_read, 0);

    if (LIKELY(n > 0)) {
        bytes_read += n;

        // size_t search_start = (bytes_read > (size_t) n + 3) ? (bytes_read - n - 3) : 0;
        // if (LIKELY(memmem(buffer + search_start, buffer_size - search_start, "\r\n\r\n", 4) != NULL)) {
        //     conn -> bytes_read = bytes_read;
        //     return 0;
        // }

        if (LIKELY(bytes_read < buffer_size)) {
            conn -> bytes_read = bytes_read;
            return 0;
        }

        conn -> bytes_read = bytes_read;
        return -1;
    } else if (n == 0) {
        return -1;
    } else {
        if (LIKELY(errno == EAGAIN || errno == EWOULDBLOCK)) {
            return 1;
        }
        return -1;
    }
}

ssize_t ws_write(ws_Connection* conn, const char* buffer, const size_t len) {
    ssize_t total_sent = 0;

    while (total_sent < (const ssize_t) len) {
        ssize_t bytes_sent = send(conn -> fd, buffer + total_sent, len - total_sent, MSG_NOSIGNAL);

        if (bytes_sent < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) return total_sent;
            return -1;
        }

        if (bytes_sent == 0) return total_sent;

        total_sent += bytes_sent;
    }

    return total_sent;
}
