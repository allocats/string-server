#ifndef WS_LOOKUP_TABLE_H
#define WS_LOOKUP_TABLE_H

#include "ws_cached_files.h"

#include "../assets/ws_assets.h"
#include "../utils/ws_types.h"

const ws_Asset WS_ASSETS[] = {
  {
      .hash = 0xaf538a40,
      .size = sizeof(response_index_html) - 1,
      .response = response_index_html
  },
  {
      .hash = 0xe7547ca0,
      .size = sizeof(response_styles_css) - 1,
      .response = response_styles_css
  },
};

const u32 WS_ASSETS_COUNT = 2;

#endif // !WS_LOOKUP_TABLE_H
