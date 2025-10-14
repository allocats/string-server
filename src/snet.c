#include "snet.h"

#include <fcntl.h>
#include <stdbool.h>

inline bool make_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return false; 
    return true;
}
