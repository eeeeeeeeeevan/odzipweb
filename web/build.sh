#!/bin/sh
set -e

SRCDIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTDIR="$(cd "$(dirname "$0")" && pwd)"

emcc \
    -O2 -flto \
    -s EXPORTED_FUNCTIONS='["_odz_wasm_compress","_odz_wasm_decompress","_odz_wasm_strerror","_odz_wasm_free","_malloc","_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","getValue","UTF8ToString","HEAPU8"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MAXIMUM_MEMORY=512MB \
    -s MODULARIZE=1 \
    -s EXPORT_NAME=OdzipModule \
    -s ENVIRONMENT=web \
    -I"$SRCDIR" \
    "$SRCDIR/odz_util.c" \
    "$SRCDIR/bitstream.c" \
    "$SRCDIR/huffman.c" \
    "$SRCDIR/lz_hashchain.c" \
    "$SRCDIR/compress.c" \
    "$SRCDIR/decompress.c" \
    "$OUTDIR/wasm.c" \
    -o "$OUTDIR/odz.js"

echo "done"
