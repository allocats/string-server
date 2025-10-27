#pragma once
#ifndef WS_ASSETS_H
#define WS_ASSETS_H

#include "../utils/ws_types.h"

typedef struct __attribute__ ((aligned(32))) {
    const size_t size;
    const char* response;
    const u32 hash;
    u8 _padding[12];
} ws_Asset;

const ws_Asset* ws_lookup_asset(u32 hash);

u32 ws_hash_djb2(char* s, u32 len);
u32 ws_hash_fnv1a(const char* s, u32 len);

#endif // !WS_ASSETS_H
