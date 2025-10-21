#include "ws_http.h"

#include <string.h>
#include <unistd.h>

int ws_debug_response(ws_Connection* conn) {
    const char* response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "OK";

    return write(conn -> fd, response, strlen(response));
}

int ws_debug_read(ws_Connection* conn) {
    return read(conn -> fd, conn -> buffer, conn -> buffer_size);
}
