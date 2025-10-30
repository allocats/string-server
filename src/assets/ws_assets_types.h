#pragma once
#ifndef WS_ASSET_TYPES_H
#define WS_ASSET_TYPES_H

#include "../utils/ws_types.h"

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
    const char* response;
    const size_t size;
    const u32 hash;
    u8 _padding[12];
} ws_Asset;

#endif // !WS_ASSET_TYPES_H
