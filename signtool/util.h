#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#include "sha256.h"

int writefile(const char *path, const uint8_t *data, size_t size);
int readfile(const char *path, uint8_t *buf, size_t size);
void print_hex(const uint8_t *buf, size_t size);
int sha256sum(const char *fname, uint8_t hash[SHA256_BLOCK_SIZE]);

#endif