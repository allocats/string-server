#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <stdint.h>

#define MAX_EVENTS 4096

void ws_start_server(const char* address, const uint16_t port);

#endif // !WS_SERVER_H
