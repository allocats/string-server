#ifndef SPOLL_H
#define SPOLL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool epoll_add(int efd, int fd, uint32_t events);
bool epoll_remove(int efd, int fd);

#endif // !SPOLL_H
