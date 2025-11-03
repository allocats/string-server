#pragma once
#ifndef WS_CHAR_MAP_H
#define WS_CHAR_MAP_H

#include "../../utils/ws_types.h"

#define WS_IS_NUMBER(c)     (char_map[(unsigned char)(c)] & 1)
#define WS_IS_ALPHA(c)      (char_map[(unsigned char)(c)] & 2)
#define WS_IS_WHITESPACE(c) (char_map[(unsigned char)(c)] & 4)
#define WS_IS_IDENTIFIER(c) (char_map[(unsigned char)(c)] & 8)
#define WS_IS_DELIM(c)      (char_map[(unsigned char)(c)] & 16)

static const u8 char_map[256] = {
    ['0' ... '9'] = 1,

    ['a' ... 'z'] = 2,
    ['A' ... 'Z'] = 2,

    ['_'] = 2,
    ['-'] = 2,
    ['/'] = 2,
    ['.'] = 2,
    ['*'] = 2,
    ['='] = 2,
    ['?'] = 2,

    [' '] = 4, 
    ['\f'] = 4,
    ['\n'] = 4,
    ['\r'] = 4,
    ['\t'] = 4, 
    ['\v'] = 4,

    [':'] = 8,

    ['['] = 16,
    [']'] = 16,
    ['{'] = 16,
    ['}'] = 16,
};

#endif // !WS_CHAR_MAP_H
