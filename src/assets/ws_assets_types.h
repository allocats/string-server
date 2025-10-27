#pragma once
#ifndef WS_ASSET_TYPES_H
#define WS_ASSET_TYPES_H

#include "../utils/ws_types.h"

typedef struct __attribute__ ((aligned(32))) {
    const size_t size;
    const char* response;
    const u32 hash;
    u8 _padding[12];
} ws_Asset;

#endif // !WS_ASSET_TYPES_H
