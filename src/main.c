#include "net/ws_server.h"

#include "utils/ws_types.h"

#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define WORKER_COUNT 12
#define ADDRESS "0.0.0.0"
#define PORT 8080

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    (void) sig;
    running = 0;
}

i32 main(void) {
    signal(SIGINT, sigint_handler);

    for (i32 i = 0; i < WORKER_COUNT; i++) {
        if (fork() == 0) {
            ws_start_server(ADDRESS, PORT);
            exit(0);
        } 
    }

    while (wait(NULL) > 0);
    return 0;
}
