#include "ws_http.h"

#include "../assets/ws_assets.h"

#include <errno.h>
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
const ws_Asset* ws_parse_request(ws_Connection* conn) {
    char* buffer = conn -> buffer;
    char* end = buffer + conn -> bytes_read;
    char* p = buffer;

    if (WS_IS_GET(buffer)) {
        p += 4;

        while (*p != '/' && p < end) p++;
        p++;

        char* start = p;
        while (*p != ' ' && p < end) p++;

        u32 len = p - start;

        u32 hash = ws_hash_djb2(start, len);
        const ws_Asset* asset = ws_lookup_asset(hash);

        if (!asset) {
            return NULL;
        }

        return asset;
    }

    return NULL;
}

ssize_t ws_read(ws_Connection* conn) {
    const ssize_t max = (const ssize_t) conn -> buffer_size;
    ssize_t total_read = 0;

    while (total_read < max) {
        ssize_t bytes_read = recv(conn -> fd, conn -> buffer + total_read, max - total_read, 0);

        if (bytes_read < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) return total_read;
            return -1;
        }

        if (bytes_read == 0) return total_read;

        total_read += bytes_read;

        // For now handle as error, grow buffer later on
        if (total_read >= max) return -1;
    }

    return total_read;
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
