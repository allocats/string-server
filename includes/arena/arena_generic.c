#include "arena.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#define UNLIKELY(x) __builtin_expect(x, 0)
#define LIKELY(x) __builtin_expect(x, 1)

void* arena_realloc_generic(ArenaAllocator* arena, void* ptr, size_t old_size, size_t new_size) {
    if (UNLIKELY(new_size <= old_size)) return ptr;

    void* result = arena_alloc(arena, new_size);
    char* new_ptr = (char*) result;
    char* old_ptr = (char*) ptr;

    size_t n = old_size / 8;
    size_t current = 0;

    for (size_t i = 0; i < n; i += 8) {
        new_ptr[i] = old_ptr[i];
        new_ptr[i + 1] = old_ptr[i + 1];
        new_ptr[i + 2] = old_ptr[i + 2];
        new_ptr[i + 3] = old_ptr[i + 3];
        new_ptr[i + 4] = old_ptr[i + 4];
        new_ptr[i + 5] = old_ptr[i + 5];
        new_ptr[i + 6] = old_ptr[i + 6];
        new_ptr[i + 7] = old_ptr[i + 7];

        current += 8;
    }

    while (current < old_size) {
        new_ptr[current] = old_ptr[current];
    }

    n = (new_size - old_size) / 8;

    for (size_t i = 0; i < n; i += 8) {
        new_ptr[i] = 0;
        new_ptr[i + 1] = 0;
        new_ptr[i + 2] = 0;
        new_ptr[i + 3] = 0;
        new_ptr[i + 4] = 0;
        new_ptr[i + 5] = 0;
        new_ptr[i + 6] = 0;
        new_ptr[i + 7] = 0;

        current += 8;
    }

    while (current < new_size) {
        new_ptr[current] = 0;
    }

    return result;
}

void* arena_memset_generic(void* ptr, int value, size_t len) {
    char* p = (char*) ptr;
    char byte_value = (char) value;

    size_t current = 0;
    size_t n = len / 8;

    for (size_t i = 0; i < n; i += 8) {
        p[i] = byte_value;
        p[i + 1] = byte_value; 
        p[i + 2] = byte_value; 
        p[i + 3] = byte_value; 
        p[i + 4] = byte_value; 
        p[i + 5] = byte_value; 
        p[i + 6] = byte_value; 
        p[i + 7] = byte_value; 

        current += 8;
    }

    while (current < len) {
        *(p + current) = byte_value;
        current++;
    }

    return ptr;
} 

void* arena_memcpy_generic(void* dest, const void* src, size_t len) {
    char* d = dest;
    const char* s = src;

    size_t current = 0;
    size_t n = len / 8;

    for (size_t i = 0; i < n; i += 8) {
        d[i] = s[i];
        d[i + 1] = s[i + 1];
        d[i + 2] = s[i + 2];
        d[i + 3] = s[i + 3];
        d[i + 4] = s[i + 4];
        d[i + 5] = s[i + 5];
        d[i + 6] = s[i + 6];
        d[i + 7] = s[i + 7];

        current += 8;
    }

    while (current < len) {
        *(d + current) = *(s + current);
        current++;
    }

    return dest;
}
