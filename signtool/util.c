#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "util.h"

int writefile(const char *path, const uint8_t *data, size_t size) {
    FILE *f = fopen(path, "wb");
    if(!f) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
        return 0;
    }

    size_t res = fwrite(data, 1, size, f);
    fclose(f);
    if(res != size) {
        fprintf(stderr, "Failed to write to file %s: short write (%u vs %u)\n", path, res, size);
        return 0;
    }
    return 1;
}

int readfile(const char *path, uint8_t *buf, size_t size) {
    FILE *f = fopen(path, "rb");
    if(!f) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
        return 0;
    }

    size_t res, total = 0;
    while((res = fread(buf+total, 1, size - total, f)) > 0 && total < size) {
        total += res;
    }
    fclose(f);

    if(total != size) {
        fprintf(stderr, "Failed to read file %s: short read (%u vs %u)\n", path, total, size);
        return 0;
    }
    return 1;
}

void print_hex(const uint8_t *buf, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        static const char *hex = "0123456789abcdef";
        printf("%c", hex[buf[i] >> 4]);
        printf("%c", hex[buf[i] & 0xF]);
    }
}

int sha256sum(const char *fname, uint8_t hash[SHA256_BLOCK_SIZE]) {
    FILE *f = fopen(fname, "rb");
    if(!f) {
        fprintf(stderr, "Failed to open file %s\n", fname);
        return 0;
    }

    SHA256_CTX ctx;
    sha256_init(&ctx);

    uint8_t buf[2048];
    size_t res, total = 0;
    while((res = fread(buf, 1, sizeof(buf), f)) > 0) {
        sha256_update(&ctx, buf, res);
        total += res;
    }
    fclose(f);
    sha256_final(&ctx, hash);
}