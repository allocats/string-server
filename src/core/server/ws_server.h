#pragma once
#ifndef WS_SERVER_H
#define WS_SERVER_H

#include "../../utils/ws_types.h"

#define MAX_EVENTS 4096

void ws_start_server(const char* address, const u16 port);

#endif // !WS_SERVER_H
