#ifndef SCONN_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    STATE_READ_HEADER,
    STATE_READ_PAYLOAD,
    STATE_WRITE_RESPONSE
} sconn_state;

typedef struct {
    int fd;
    sconn_state state;
    char* buffer;
    uint32_t payload_size;
    uint32_t net_len;
    struct sockaddr_in addr;
    socklen_t addr_size;
} sconnection;

size_t read_exact(int fd, void* buf, size_t len);
size_t write_exact(int fd, void* buf, size_t len);

bool handle_read(sconnection* conn);
bool handle_write(sconnection* conn);

#endif // !SCONN_H
