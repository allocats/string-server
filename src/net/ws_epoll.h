#ifndef WS_EPOLL_H
#define WS_EPOLL_H

#include <stdint.h>

int ws_epoll_init_server(const int fd);
int ws_epoll_add_fd(const int efd, const int fd, const uint32_t flags);
int ws_epoll_remove(const int efd, const int fd);

#endif // !WS_EPOLL_H
