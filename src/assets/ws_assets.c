#include "ws_assets.h"

#include "../cache/ws_lookup_table.h"
#include "../utils/ws_debug.h"
#include "ws_assets_types.h"

#include <fcntl.h>
#include <unistd.h>

__attribute__((always_inline)) inline 
const ws_Asset* ws_lookup_asset(u32 hash) {
    for (u32 i = 0; i < WS_ASSETS_COUNT; i++) {
        if (WS_ASSETS[i].hash == hash) {
            return &WS_ASSETS[i];
        }
    }
    return NULL;
}
__attribute__((always_inline)) inline 
u32 ws_hash_fnv1a(const char* s, u32 len) {
    u32 hash = 2166136261u;
    for (u32 i = 0; i < len; i++) {
        hash ^= s[i];
        hash *= 16777619u;
    }
    return hash;
} 

__attribute__((always_inline)) inline 
u32 ws_hash_djb2(char* s, u32 len) {
    u32 hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + s[i];
    }
    return hash;
}

i32 ws_assets_load(void) {
    for (u32 i = 0; i < WS_ASSETS_COUNT; i++) {
        if (WS_ASSETS[i].type == WS_ASSET_FILE) {
            ws_Asset* asset = &WS_ASSETS[i];

            asset -> fd = open(asset -> response + asset -> header_len, O_RDONLY);
            if (asset -> fd == -1) {
                return -1;
            }

            ws_debug_log(
                "Opening:%s on fd=%u", 
                asset -> response + asset -> header_len, asset -> fd
            );

        }
    }

    return 0;
}

i32 ws_assets_close(void) {
    for (u32 i = 0; i < WS_ASSETS_COUNT; i++) {
        if (WS_ASSETS[i].type == WS_ASSET_FILE) {
            ws_Asset* asset = &WS_ASSETS[i];

            i32 ret = close(asset -> fd);
            if (ret == -1) {
                return -1;
            }
        }
    }

    return 0;
}
