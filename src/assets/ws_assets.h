#pragma once
#ifndef WS_ASSETS_H
#define WS_ASSETS_H

#include "ws_assets_types.h"

const ws_Asset* ws_lookup_asset(u32 hash);

u32 ws_hash_djb2(char* s, u32 len);
u32 ws_hash_fnv1a(const char* s, u32 len);

#endif // !WS_ASSETS_H
