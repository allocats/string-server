#pragma once
#ifndef WS_CONNECTION_TYPES_H
#define WS_CONNECTION_TYPES_H

#include "../../http/ws_http_types.h"
#include "../../utils/ws_types.h"

#define WS_DEFAULT_BUFFER_SIZE 4096

typedef enum {
    WS_READING,
    WS_RESPOND,
    WS_DONE,
} ws_State;

/*
* 
*   Keep structs aligned to and <= 32 bytes to fit two into a single cache line 
*   combined with arrays, leads to predictable access, 
*   resulting in a significant improvement to cache performance
*   and overall server performance
*
*   Could try to adopt SoA over AoS, will look into it and test it further
*
*/
typedef struct __attribute__ ((aligned(32))) {
    char* buffer;
    u32 buffer_size;
    u32 bytes_read;
    ws_State state;
    i32 fd;
    u8 _padding[8];
} ws_Connection;

#endif // !WS_CONNECTION_TYPES_H
