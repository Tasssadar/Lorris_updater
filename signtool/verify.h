#ifndef VERIFY_H
#define VERIFY_H

#define ECC_CURVE secp256r1
#include "../easy-ecc/ecc.h"

#ifdef __cplusplus
extern "C"
{
#endif

int signtool_sha256sum(const char *fn, uint8_t hash[32]);
int signtool_verify(const uint8_t hash[32], const char *sign_file, const uint8_t public_key[ECC_BYTES+1]);

#ifdef __cplusplus
}
#endif

#endif