#ifndef WS_NET_H
#define WS_NET_H

#include <stdint.h>

int ws_create_tcp_server(const char* address, const uint16_t port);
int ws_make_nonblocking(const int fd);

#endif // !define WS_NET_H
