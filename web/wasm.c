#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <emscripten.h>

#include "libodzip.h"
typedef uint8_t uint8; typedef uint64_t uint64; typedef size_t size;
typedef struct __zip_inst {
    int err;      
    void* data;     
    size size;     
} zip_instance;

static zip_instance res;

EMSCRIPTEN_KEEPALIVE
zip_instance* odz_wasm_compress (const uint8* in, size in_len) 
{
    res.err = 0;
    res.data = NULL;
    res.size = 0;

    FILE* fin = fmemopen((void *)in, in_len, "rb");
    if (!fin) { res.err = ODZ_ERR_IO; return &res; }
   
    // maybe
    size cap = in_len + in_len/4+4096;
    uint8* obuf = malloc(cap);
    if (!obuf) { fclose(fin); res.err = ODZ_ERR_OOM; return &res; }
    FILE* fout = fmemopen(obuf, cap, "wb");

    if (!fout) { fclose(fin); free(obuf); res.err = ODZ_ERR_IO; return &res; }
    // int odz_compress(FILE *in, FILE *out, const odz_options_t *opts)
    int rc = odz_compress(fin, fout, NULL);
    long written = ftell(fout);
    fclose(fin);
    fclose(fout);

    if (rc != ODZ_OK || written < 0) {
        free(obuf);
        res.err = rc ? rc : -1;
        // res.err = (rc != NULL) ? rc : ODZ_ERR_IO;
        return &res;
    }
    res.data = obuf;
    res.size = (size) written;
    return &res;
}

EMSCRIPTEN_KEEPALIVE
zip_instance* odz_wasm_decompress (const uint8* in, size in_len) 
{
    res.err  = 0;
    res.data = NULL;
    res.size = 0;
    // if magic byte nothere duh
    if (in_len < 12) { res.err = ODZ_ERR_FORMAT; return &res; }
    if (in[0] != 'O' || in[1] != 'D' || in[2] != 'Z') {
        res.err = ODZ_ERR_FORMAT;
        return &res;
    }

    uint64 orig = 0;
    for (int i = 0; i < 8; i++) orig |= (uint64)in[4 + i] << (8 * i);

    if (orig > (256u << 20)) { res.err = ODZ_ERR_OOM; return &res; }

    FILE* fin = fmemopen((void *)in, in_len, "rb");
    if (!fin) { res.err = ODZ_ERR_IO; return &res; }

    size out_cap = (size)orig;
    // whatever dude
    uint8* obuf = (uint8*)malloc(out_cap);
    if (!obuf) { fclose(fin); res.err = ODZ_ERR_OOM; return &res; }

    FILE* fout = fmemopen(obuf, out_cap, "wb");
    if (!fout) { fclose(fin); free(obuf); res.err = ODZ_ERR_IO; return &res; }

    int rc = odz_decompress(fin, fout, NULL);
    fclose(fin);
    fclose(fout);

    if (rc != ODZ_OK) {
        free(obuf);
        res.err = rc;
        return &res;
    }

    res.data = obuf;
    res.size = (size)orig;
    return &res;
}

EMSCRIPTEN_KEEPALIVE
const char* odz_wasm_strerror (int err)
{
    return odz_strerror(err);
}

EMSCRIPTEN_KEEPALIVE
void odz_wasm_free (void* ptr)
{
    free(ptr);
}
