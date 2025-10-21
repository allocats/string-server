#include "net/ws_server.h"

#include <sys/wait.h>
#include <unistd.h>

#define WORKER_COUNT 12
#define ADDRESS "127.0.0.1"
#define PORT 8080

int main(void) {
    for (int i = 0; i < WORKER_COUNT; i++) {
        if (fork() == 0) {
            ws_start_server(ADDRESS, PORT);
        } 
    }

    while (wait(NULL) > 0);
}
