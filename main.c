/*
 * odz — a DEFLATE-class compressor
 *
 * Format v2: "ODZ\x02" | original_size(u64 LE) | blocks...
 * Each block: flags(u8) | raw_size(u32 LE) | [compressed_size(u32 LE)] | data
 *
 * Compression pipeline: LZ77 hash-chain → Huffman → bitstream
 * Processes input in 1 MB blocks for bounded memory usage.
 *
 * Build: cmake --build . --config Release
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libodzip.h"

static void die(const char *m) { fprintf(stderr, "odz: error: %s\n", m); exit(1); }

static int progress_cb(uint64_t processed, uint64_t total, void *userdata) {
    (void)userdata;
    fprintf(stderr, "\r  %llu / %llu bytes  (%.1f%%)",
            (unsigned long long)processed,
            (unsigned long long)total,
            total > 0 ? 100.0 * processed / total : 100.0);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr,
            "odz — LZ77+Huffman compressor (format v%d)\n"
            "usage:\n"
            "  %s c <input> <output>   compress\n"
            "  %s d <input> <output>   decompress\n",
            ODZ_FORMAT_VERSION, argv[0], argv[0]);
        return 2;
    }

    char mode = argv[1][0];

    FILE *fin = fopen(argv[2], "rb");
    if (!fin) die("cannot open input file");

    FILE *fout = fopen(argv[3], "wb");
    if (!fout) { fclose(fin); die("cannot open output file"); }

    odz_options_t opts = { .progress = progress_cb, .userdata = NULL };
    int rc;

    if (mode == 'c') {
        rc = odz_compress(fin, fout, &opts);
    } else if (mode == 'd') {
        rc = odz_decompress(fin, fout, &opts);
    } else {
        die("mode must be 'c' or 'd'");
    }

    fprintf(stderr, "\n");

    if (rc != ODZ_OK) die(odz_strerror(rc));

    fclose(fin);
    fclose(fout);
    return 0;
}
