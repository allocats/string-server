#!/usr/bin/env bash

SRC_DIR=${1:-"www"}

if [ ! -d "$SRC_DIR" ]; then
    echo "Error: Directory $SRC_DIR not found!"
    echo "Usage: $0 <www_directory>"
    exit 1
fi

OUTPUT_FILE="src/cache/ws_cached_files.h"
LOOKUP_FILE="src/cache/ws_lookup_table.h"

echo "Generating cached files from $SRC_DIR..."

cat > "$OUTPUT_FILE" << 'EOF'
#pragma once
#ifndef WS_CACHED_FILES_H
#define WS_CACHED_FILES_H

#include "../utils/ws_types.h"

EOF

cat > "$LOOKUP_FILE" << 'EOF'
#pragma once
#ifndef WS_LOOKUP_TABLE_H
#define WS_LOOKUP_TABLE_H

#include "ws_cached_files.h"

#include "../assets/ws_assets_types.h"

const ws_Asset WS_ASSETS[] = {
EOF

file_count=0

minify_html() {
    sed 's/<!--.*-->//g' | \
    sed 's/>[[:space:]]\+</></g' | \
    sed 's/^[[:space:]]\+//g' | \
    sed 's/[[:space:]]\+$//g' | \
    tr -s ' ' | \
    sed 's/[[:space:]]*=[[:space:]]*/=/g' | \
    tr -d '\n' | tr -d '\r'
}

minify_css() {
    # sed 's/\/\*.*\*\///g' | \
    tr -d '\n' | tr -d '\r' | \
    sed 's/[[:space:]]\+/ /g' | \
    sed 's/[[:space:]]*{[[:space:]]*/{/g' | \
    sed 's/[[:space:]]*}[[:space:]]*/}/g' | \
    sed 's/[[:space:]]*;[[:space:]]*/;/g' | \
    sed 's/[[:space:]]*:[[:space:]]*/:/g' | \
    sed 's/[[:space:]]*,[[:space:]]*/,/g' | \
    sed 's/^[[:space:]]\+//g' | \
    sed 's/[[:space:]]\+$//g'
}

minify_js() {
    sed 's/\/\*.*\*\///g' | \
    sed 's/\/\/.*$//g' | \
    sed 's/"[^"]*"/STRING_PLACEHOLDER_&/g' | \ 
    tr -d '\n' | tr -d '\r' | \
    sed 's/[[:space:]]\+/ /g' | \
    sed 's/[[:space:]]*{[[:space:]]*/{/g' | \
    sed 's/[[:space:]]*}[[:space:]]*/}/g' | \
    sed 's/[[:space:]]*;[[:space:]]*/;/g' | \
    sed 's/[[:space:]]*([[:space:]]*(/g' | \
    sed 's/[[:space:]]*)[[:space:]]*/)/g' | \
    sed 's/^[[:space:]]\+//g' | \
    sed 's/[[:space:]]\+$//g'
}

escape_for_asm() {
    sed 's/\\/\\\\/g; s/"/\\"/g'
}

djb2_hash() {
    local string="$1"
    local hash=5381
    local i
    
    for (( i=0; i<${#string}; i++ )); do
        local char=$(printf '%d' "'${string:$i:1}")
        hash=$(( (hash << 5) + hash + char ))
        hash=$(( hash & 0xFFFFFFFF ))
    done
    
    echo $hash
}

fnv1a_hash() {
    local string="$1"
    local hash=2166136261
    local char
    local i

    for (( i=0; i<${#string}; i++ )); do
        # local char=$(printf '%d' "${string:$i:1}")
        printf -v char '%d' "'${string:$i:1}"
        (( hash^=char ))
        (( hash=(hash*16777619)))
        (( hash &= 0xFFFFFFFF ))
    done

    echo $hash
}

for file in $(find "$SRC_DIR" -name "*.html" -o -name "*.css" -o -name "*.js" | sort); do
    rel_path=${file#$SRC_DIR}
    rel_path=${rel_path#/} 
    
    safe_name=$(echo "$rel_path" | tr '/' '_' | tr '.' '_' | tr '-' '_')
    
    original_size=$(wc -c < "$file")

    header="HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n"
    
    echo ""
    echo "Processing: /$rel_path ($original_size bytes)"

    
    case "$file" in
        *.html)
            content=$(cat "$file" | minify_html)
            content_len=${#content}
            content=$(echo "$content" | escape_for_asm)
            header+="Content-Type: text/html\r\nContent-Length: ${content_len}\r\n\r\n"
            ;;
        *.css)
            content=$(cat "$file" | minify_css)
            content_len=${#content}
            content=$(echo "$content" | escape_for_asm)
            header+="Content-Type: text/css\r\nContent-Length: ${content_len}\r\n\r\n"
            ;;
        *.js)
            content=$(cat "$file" | minify_js)
            content_len=${#content}
            content=$(echo "$content" | escape_for_asm)
            header+="Content-Type: text/javascript\r\nContent-Length: ${content_len}\r\n\r\n"
            ;;
        *)
            content=$(cat "$file")
            content_len=${#content}
            content=$(echo "$content" | escape_for_asm)
            ;;
    esac

    # hash=$(djb2_hash "${rel_path}")
    hash=$(fnv1a_hash "${rel_path}")

    echo "Hash for /${rel_path}: 0x$(printf '%x', $hash) (Input: ${rel_path}))"
    
    minified_size=${#content}
    savings=$((original_size - minified_size))
    percentage=$(( savings * 100 / original_size ))

    total_length=$(( ${#content} + ${#header} ))
    
    echo "  Minified: $minified_size bytes (saved $savings bytes, ${percentage}%)"

    echo "static const char response_$safe_name[] = \"$header\" \"$content\";" >> "$OUTPUT_FILE"
    echo "static const size_t length_$safe_name = $total_length;" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"

    echo "  {" >> "$LOOKUP_FILE"
    echo "      .hash = 0x$(printf '%x' $hash)," >> "$LOOKUP_FILE"
    echo "      .size = sizeof(response_$safe_name) - 1," >> "$LOOKUP_FILE"
    echo "      .response = response_$safe_name" >> "$LOOKUP_FILE"
    echo "  }," >> "$LOOKUP_FILE"
    
    ((file_count++))
done

echo "};" >> "$LOOKUP_FILE"
echo "" >> "$LOOKUP_FILE"
echo "const u32 WS_ASSETS_COUNT = $file_count;" >> "$LOOKUP_FILE"
echo "" >> "$LOOKUP_FILE"

# CLOSING IF

echo "#endif // !WS_CACHED_FILES_H" >> "$OUTPUT_FILE"
echo "#endif // !WS_LOOKUP_TABLE_H" >> "$LOOKUP_FILE"

echo ""
echo "Generated $file_count cached files:"
echo "  - $OUTPUT_FILE (file data)"
echo "  - $LOOKUP_FILE (lookup table)"
echo ""
