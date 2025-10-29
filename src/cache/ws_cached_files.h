#pragma once
#ifndef WS_CACHED_FILES_H
#define WS_CACHED_FILES_H

#include "../utils/ws_types.h"

static const char response_index_html[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nContent-Length: 478\r\n\r\n" "<!doctypehtml><html lang=\"en\"><meta charset=\"UTF-8\"><meta name=\"viewport\"content=\"width=device-width,initial-scale=1\"><link rel=\"stylesheet\"href=\"styles.css\"><title>hello from string server</title><h1 id=\"demo\">Welcome to string server!</h1><p>Thank you for stopping by my passion/curiosity project. I will be pursuing this project for a long time and hopefully turn this into a fully functional production ready system :) <a href=\"\">Blah</a></p><script src=\"index.js\"></script>";

static const char response_index_js[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/javascript\r\nContent-Length: 54\r\n\r\n" "document.getElementById(\"demo\").style.fontSize=\"50px\";";

static const char response_styles_css[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/css\r\nContent-Length: 13\r\n\r\n" "h1{color:red}";

#endif // !WS_CACHED_FILES_H
