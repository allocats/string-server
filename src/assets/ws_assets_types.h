#pragma once
#ifndef WS_ASSET_TYPES_H
#define WS_ASSET_TYPES_H

#include "../utils/ws_types.h"

typedef enum __attribute__ ((packed)) {
    WS_ASSET_IN_MEMORY,
    WS_ASSET_FILE,
} ws_AssetType;

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
    const u32 size;
    const u32 hash;
    const u16 header_len;
    const u8 flags;
    const ws_AssetType type;
    i32 fd;
    u8 _padding[4];
} ws_Asset;

#endif // !WS_ASSET_TYPES_H
