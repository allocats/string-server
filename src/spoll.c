#include "spoll.h"

#include <sys/epoll.h>

inline bool epoll_add(int efd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        return false;
    }

    return true;
}
 
inline bool epoll_remove(int efd, int fd) {
    return epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL) == -1 ? false : true;
}
