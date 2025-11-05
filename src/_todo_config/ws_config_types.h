#pragma once
#ifndef WS_CONFIG_TYPES_H
#define WS_CONFIG_TYPES_H

#include "../utils/ws_types.h"

typedef struct {
    const char* addr;
    const u16 port;
    const u8 _padding[2];

    const u16 workers;
    const u16 connections;
} ws_Config;

typedef struct {
    char* buffer;
    char* end;
    char* curr;
} ws_Lexer;

#endif // !WS_CONFIG_TYPES_H
