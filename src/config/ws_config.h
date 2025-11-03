#pragma once
#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include "ws_config_types.h"

#define WS_CONFIG_PATH "whisker.config"

ws_Config* ws_load_config(void);

#endif // !WS_CONFIG_H
