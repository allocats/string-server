#pragma once
#ifndef WS_DEBUG_H
#define WS_DEBUG_H

#include <assert.h>
#include <stdio.h>

/*
*
*   Use this define to enable/disable debugging fn's across the project 
*
*/

#define WS_DEBUG_MODE

#ifdef WS_DEBUG_MODE
    #define ws_debug_assert(cond) (assert((cond)))
    #define ws_debug_log(fmt, ...) \
        fprintf(stderr, "[DEBUG]: " fmt "\n", ##__VA_ARGS__)
#else 
    #define ws_debug_assert(cond)   ((void) 0);
    #define ws_debug_log(fmt, ...)  ((void) 0) 
#endif // WS_DEBUG_MODE

#endif // !WS_DEBUG_H
