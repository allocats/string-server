#include "arena.h"

#include <assert.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#define UNLIKELY(x) __builtin_expect(x, 0)
#define LIKELY(x) __builtin_expect(x, 1)

#define SSE2_CHUNK(p, n) (p + (16 * n))

void* arena_realloc_sse2(ArenaAllocator* arena, void* ptr, const size_t old_size, const size_t new_size) {
    if (UNLIKELY(new_size <= old_size)) {
        return ptr;
    }

    void* result = arena_alloc(arena, new_size);
    char* new_ptr = (char*) result;
    const char* old_ptr = (char*) ptr;
    size_t copy_size = old_size;

    while (copy_size > 0 && ((uintptr_t) new_ptr & 15)) {
        *new_ptr++ = *old_ptr++;
        copy_size--;
    }

    while (copy_size >= 64) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 0)));
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 1), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 1)));
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 2), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 2)));
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 3), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 3)));

        new_ptr += 64;
        old_ptr += 64;
        copy_size -= 64;
    }

    while (copy_size >= 48) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 0)));
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 1), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 1)));
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 2), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 2)));

        new_ptr += 48;
        old_ptr += 48;
        copy_size -= 48;
    }

    while (copy_size >= 32) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 0)));
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 1), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 1)));

        new_ptr += 32;
        old_ptr += 32;
        copy_size -= 32;
    }

    while (copy_size >= 16) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), _mm_load_si128((const __m128i*) SSE2_CHUNK(old_ptr, 0)));

        new_ptr += 16;
        old_ptr += 16;
        copy_size -= 16;
    }

    const __m128i zeros = _mm_setzero_si128();
    size_t zero_size = new_size - old_size;
    while (zero_size >= 64) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), zeros);
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 1), zeros);
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 2), zeros);
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 3), zeros);

        new_ptr += 64;
        zero_size -= 64;
    }

    while (zero_size >= 48) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), zeros);
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 1), zeros);
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 2), zeros);

        new_ptr += 48;
        zero_size -= 48;
    }

    while (zero_size >= 32) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), zeros);
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 1), zeros);

        new_ptr += 32;
        zero_size -= 32;
    }

    while (zero_size >= 16) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(new_ptr, 0), zeros);

        new_ptr += 16;
        zero_size -= 16;
    }

    while (zero_size > 0) {
        *new_ptr++ = 0;
        zero_size--;
    }

    return result;
}

void* arena_memset_sse2(void* ptr, const int value, size_t len) {
    char* p = (char*) ptr;
    const char char_value = (char) value;

    while (len > 0 && ((uintptr_t) p & 15)) {
        *p++ = char_value;
        len--;
    }

    __m128i byte_value = _mm_set1_epi8(char_value);

    while (len >= 64) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 0), byte_value);
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 1), byte_value);
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 2), byte_value);
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 3), byte_value);

        p += 64;
        len -= 64;
    }

    while (len >= 48) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 0), byte_value);
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 1), byte_value);
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 2), byte_value);

        p += 48;
        len -= 48;
    }

    while (len >= 32) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 0), byte_value);
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 1), byte_value);

        p += 32;
        len -= 32;
    }

    while (len >= 16) {
        _mm_store_si128((__m128i*) SSE2_CHUNK(p, 0), byte_value);

        p += 16;
        len -= 16;
    }

    while (len > 0) {
        *p++ = char_value;
        len--;
    }

    return ptr;
}

void* arena_memcpy_sse2(void* dest, const void* src, size_t len) {
    char* d = dest;
    const char* s = src;


    while (len >= 64) {
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 0), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 0)));
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 1), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 1)));
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 2), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 2)));
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 3), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 3)));

        len -= 64;
        d += 64;
        s += 64;
    }

    while (len >= 48) {
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 0), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 0)));
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 1), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 1)));
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 2), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 2)));

        len -= 48;
        d += 48;
        s += 48;
    }

    while (len >= 32) {
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 0), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 0)));
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 1), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 1)));

        len -= 32;
        d += 32;
        s += 32;
    }

    while (len >= 16) {
        _mm_storeu_si128((__m128i*) SSE2_CHUNK(d, 0), _mm_loadu_si128((const __m128i*) SSE2_CHUNK(s, 0)));

        len -= 16;
        d += 16;
        s += 16;
    }

    while (len--) {
        *d++ = *s++;
    }

    return dest;
}
