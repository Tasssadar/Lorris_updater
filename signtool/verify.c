#include "sha256.h"
#include "verify.h"
#include "util.h"

int signtool_sha256sum(const char *fn, uint8_t hash[32]) {
    return sha256sum(fn, hash);   
}

int signtool_verify(const uint8_t hash[SHA256_BLOCK_SIZE], const char *sign_file, const uint8_t public_key[ECC_BYTES+1]) {
    #if SHA256_BLOCK_SIZE != ECC_BYTES
      #error "The ecc curve size is wrong!"
    #endif

    uint8_t signature[ECC_BYTES*2];
    if(!readfile(sign_file, signature, sizeof(signature))) {
        return 0;
    }

    if(ecdsa_verify(public_key, hash, signature) == 0) {
        return 0;
    }
    return 1;
}