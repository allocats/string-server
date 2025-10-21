#include "ws_common.h"
#include "ws_epoll.h"

#include <sys/epoll.h>
#include <sys/socket.h>

int ws_epoll_init_server(const int fd) {
    int efd = epoll_create1(SOCK_CLOEXEC);

    if (efd == -1) {
        WS_ERR_CLOSE_AND_EXIT("Failed to create epoll fd", fd, 1);
    }

    struct epoll_event ev = {
        .events = EPOLLIN,
        .data.fd = fd
    };

    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        WS_ERR_CLOSE_AND_EXIT("Failed to add server socket to epoll", fd, 1);
    }

    return efd;
}

inline int ws_epoll_add_fd(const int efd, const int fd, const uint32_t flags) {
    struct epoll_event ev = { .events = flags, .data.fd = fd };
    return epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
}

inline int ws_epoll_remove(const int efd, const int fd) {
    return epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
}
