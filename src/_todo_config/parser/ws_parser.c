#include "../ws_config.h"

#include "ws_char_map.h"

#include "../../../includes/arena/arena.h"
#include "../../utils/ws_macros.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static ArenaAllocator config_arena = {0};

static void ws_config_error(const char* msg) {
    perror(msg);
    exit(1);
}

static void ws_lexer_init(ws_Lexer* lexer) {
    i32 fd = open(WS_CONFIG_PATH, O_RDONLY);
    if (UNLIKELY(fd == -1)) {
        ws_config_error("Unable to open config file");
    }

    struct stat st;
    if (UNLIKELY(fstat(fd, &st) == -1)) {
        close(fd);
        ws_config_error("Unable to stat config file");
    }

    size_t file_size = st.st_size;
    size_t bytes_read = 0;
    size_t n = 0;

    char* buffer = arena_alloc(&config_arena, file_size + 1);
    if (!buffer) {
        close(fd);
        ws_config_error("Failed to allocate memory for buffer while parsing config");
    }

    while (bytes_read < file_size) {
        n = read(fd, buffer + bytes_read, file_size - bytes_read);
        if (UNLIKELY(n == -1)) {
            if (errno == EINTR) continue;

            close(fd);
            ws_config_error("Failed to read config file");
        }

        bytes_read += n;
    }

    buffer[file_size] = 0;
    close(fd);

    lexer -> buffer = buffer;
    lexer -> end = buffer + file_size;
    lexer -> curr = buffer; 
}


#define WS_LEXER_ADVANCE(curr, end) \
    curr = ++curr >= end ? end : curr; \
    if (UNLIKELY(curr == end)) ws_config_error("Sudden EOF in config")


static void ws_parse_config(ws_Lexer* lexer, ws_Config* config) {
    char* curr = lexer -> curr;
    char* end = lexer -> end;

    while (WS_IS_WHITESPACE(*curr)) {
        WS_LEXER_ADVANCE(curr, end);
    }
}

ws_Config* ws_load_config(void) {
    init_arena(&config_arena, 1024);

    ws_Config* config = arena_alloc(&config_arena, sizeof(*config));
    if (UNLIKELY(!config)) {
        ws_config_error("Failed to allocate memory for config");
    }

    ws_Lexer lexer = {0};
    ws_lexer_init(&lexer);
    ws_parse_config(&lexer, config);

    return config;
}
