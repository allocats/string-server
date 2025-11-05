#define _GNU_SOURCE

#include "utils/ws_debug.h"
#ifdef WS_DEBUG_MODE
#include "assets/ws_assets_types.h"
#include "core/connection/ws_connection_types.h"
#include "core/io_uring/ws_uring_types.h"
#endif

#include "core/server/ws_server.h"

#include "utils/ws_debug.h"
#include "utils/ws_types.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <unistd.h>

#define ADDRESS "0.0.0.0"
#define WORKER_COUNT 12
#define PORT 8080

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    (void) sig;
    running = 0;
}

void pin_to_cpu(int cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity");
    }
}

i32 main(void) {
    signal(SIGINT, sigint_handler);

    ws_debug_assert(sizeof(ws_Connection) == 32);
    ws_debug_assert(sizeof(ws_Asset) == 32);
    ws_debug_assert(sizeof(ws_IoEvent) == 16);
    ws_debug_assert(sizeof(ws_SendfileCtx) == 32);

    for (i32 i = 0; i < WORKER_COUNT; i++) {
        if (fork() == 0) {
            pin_to_cpu(i);
            ws_start_server(ADDRESS, PORT);
            exit(0);
        } 
    }

    while (wait(NULL) > 0);
    return 0;
}
