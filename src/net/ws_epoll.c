/*
*
* DEPRECATED FOR NOW
*
*/

#include "ws_common.h"
#include "ws_epoll.h"

#include <sys/epoll.h>
#include <sys/socket.h>

i32 ws_epoll_init_server(const i32 fd) {
    i32 efd = epoll_create1(SOCK_CLOEXEC);

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

inline i32 ws_epoll_add_fd(const i32 efd, const i32 fd, const u32 flags) {
    struct epoll_event ev = { .events = flags, .data.fd = fd };
    return epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
}

inline i32 ws_epoll_remove(const i32 efd, const i32 fd) {
    return epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
}
