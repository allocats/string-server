#ifndef WS_EPOLL_H
#define WS_EPOLL_H

#include "ws_common.h"

i32 ws_epoll_init_server(const i32 fd);
i32 ws_epoll_add_fd(const i32 efd, const i32 fd, const u32 flags);
i32 ws_epoll_remove(const i32 efd, const i32 fd);

#endif // !WS_EPOLL_H
