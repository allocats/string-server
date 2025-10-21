/*
 *
 *  Inspired by tsoding's implementation at https://github.com/tsoding/arena
 *  Original work licensed under MIT License
 *
 *  Usage:
 *
 *      #include "arena.h"
 *
 *      Add arena.c to compilation, with -mavx2 for AVX2 support
 *
 *      Use init_arena() to initialise your arena: 
 *          The size is multiplied by sizeof(uintptr_t)
 *          Pass in 0 for default capacity of 4 * 1024 * sizeof(uintptr_t) = 32768 bytes on 64 bit
 *
 */

#ifndef ARENA_H
#define ARENA_H 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define ARENA_DEFAULT_CAPACITY (4 * 1024) 

#define arena_array(arena, type, count) \
    (type*) arena_alloc(arena, sizeof(type) * (count)) 

#define arena_array_zero(arena, type, count) \
    (type*) arena_memset(arena_alloc(arena, sizeof(type) * (count)), 0, sizeof(type) * (count)) 

typedef struct ArenaBlock {
    struct ArenaBlock* next;
    size_t usage;
    size_t capacity;
    uintptr_t data[];
} ArenaBlock;

typedef struct {
    ArenaBlock* start;
    ArenaBlock* end;
    size_t default_capacity;
} ArenaAllocator;

size_t align_size(size_t size);

void init_arena(ArenaAllocator* arena, size_t default_capacity);

void* arena_alloc(ArenaAllocator* arena, const size_t size);
void* arena_realloc(ArenaAllocator* arena, void* ptr, const size_t old_size, const size_t new_size);
void* arena_memset(void* ptr, const int value, size_t len);
void* arena_memcpy(void* dest, const void* src, size_t len);
char* arena_strdup(ArenaAllocator* arena, const char* str);

void arena_reset(ArenaAllocator* arena);
void arena_free(ArenaAllocator* arena); 

size_t total_capacity(const ArenaAllocator* arena);
size_t total_usage(const ArenaAllocator* arena); 

#ifdef __cplusplus 
}
#endif

#endif // !ARENA_H
