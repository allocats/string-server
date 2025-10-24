#ifndef WS_MACROS_H
#define WS_MACROS_H

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

#define UNUSED(x) ((void)(x))

#endif // !WS_MACROS_H
