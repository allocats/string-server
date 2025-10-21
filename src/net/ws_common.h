#ifndef WS_COMMON_H
#define WS_COMMON_H

#define WS_DEBUG_LOGS

#include "../../includes/arena/arena.h"

#include <stdio.h>
#include <unistd.h>

#define WS_ERR_CLOSE_AND_EXIT(msg, fd, status) \
    perror(msg); \
    close(fd); \
    _exit(status)

#endif // !WS_COMMON_H
