#include "ws_assets.h"

#include "../cache/ws_lookup_table.h"

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
