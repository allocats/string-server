#pragma once
#ifndef WS_LOOKUP_TABLE_H
#define WS_LOOKUP_TABLE_H

#include "ws_cached_files.h"

#include "../assets/ws_assets_types.h"

static ws_Asset WS_ASSETS[] = {
  {
      .hash = 0xefe35522,
      .size = sizeof(response_index_html) - 1,
      .type = WS_ASSET_IN_MEMORY,
      .response = response_index_html
  },
  {
      .hash = 0xebd1114,
      .size = 108055,
      .header_len = 98,
      .type = WS_ASSET_FILE,
      .response = response_index_js
  },
  {
      .hash = 0x9876146e,
      .size = sizeof(response_styles_css) - 1,
      .type = WS_ASSET_IN_MEMORY,
      .response = response_styles_css
  },
};

const u32 WS_ASSETS_COUNT = 3;

#endif // !WS_LOOKUP_TABLE_H
