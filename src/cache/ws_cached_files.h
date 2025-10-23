#ifndef WS_CACHED_FILES_H
#define WS_CACHED_FILES_H

#include "../utils/ws_types.h"

static const char response_index_html[] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: 255\r\n\r\n" "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><link rel=\"stylesheet\" href=\"styles.css\"><title>hello from string server :3</title></head><body><h1>Testing</h1></body></html>";
static const size_t length_index_html = 361;

static const char response_styles_css[] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/css\r\nContent-Length: 14\r\n\r\n" "h1{color:red;}";
static const size_t length_styles_css = 106;

#endif // !WS_CACHED_FILES_H
