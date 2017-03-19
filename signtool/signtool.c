#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#ifdef __unix__
#include <sys/stat.h>
#endif

#include "sha256.h"
#include "util.h"
#include "verify.h"

static int makekey(const char * argv[]) {
    const char *fname_priv = argv[0];
    const char *fname_pub = argv[1];

    uint8_t public[ECC_BYTES+1];
    uint8_t private[ECC_BYTES];
    int res = ecc_make_key(public, private);
    if(res == 0) {
        fprintf(stderr, "Failed to generate ecc key!\n");
        return -1;
    }

    if(!writefile(fname_priv, private, sizeof(private))) {
        return -1;
    }
#ifdef __unix__
    chmod(fname_priv, 0600);
#endif
    printf("Written private key to %s\n", fname_priv);

    if(!writefile(fname_pub, public, sizeof(public))) {
        return -1;
    }
    printf("Written public key to %s\n", fname_pub);

    printf("\nconst uint8_t pubkey[ECC_BYTES+1] = {");
    for(size_t i = 0; i < sizeof(public); ++i) {
        static const char *hex = "0123456789abcdef";
        if(i%17 == 0) {
            printf("\n    ");
        }
        printf("0x%c", hex[public[i] >> 4]);
        printf("%c,", hex[public[i] & 0xF]);
    }
    printf("\n};\n");

    return 0;
}

static int sha256sum_info(const char *fname, uint8_t hash[SHA256_BLOCK_SIZE]) {
    if(!sha256sum(fname, hash)) {
        return 0;
    }

    printf("File %s's hash: ", fname);
    print_hex(hash, SHA256_BLOCK_SIZE);
    printf("\n");
    return 1;
}

static int sign(const char * argv[]) {
    const char *fname = argv[0];
    const char *fname_priv = argv[1];
    const char *fname_sign = argv[2];

    #if SHA256_BLOCK_SIZE != ECC_BYTES
      #error "The ecc curve size is wrong!"
    #endif
    uint8_t hash[SHA256_BLOCK_SIZE];
    if(!sha256sum_info(fname, hash)) {
        return -1;
    }

    uint8_t private[ECC_BYTES];
    if(!readfile(fname_priv, private, sizeof(private))) {
        return -1;
    }

    uint8_t signature[ECC_BYTES*2];
    if(ecdsa_sign(private, hash, signature) == 0) {
        fprintf(stderr, "Failed to generate signature!\n");
        return -1;
    }

    if(!writefile(fname_sign, signature, sizeof(signature))) {
        return -1;
    }

    printf("Saved signature to %s\n", fname_sign);
    return 0;
}

static int verify(const char * argv[]) {
    const char *fname = argv[0];
    const char *fname_pub = argv[1];
    const char *fname_sign = argv[2];

    #if SHA256_BLOCK_SIZE != ECC_BYTES
      #error "The ecc curve size is wrong!"
    #endif
    uint8_t hash[SHA256_BLOCK_SIZE];
    if(!sha256sum_info(fname, hash)) {
        return -1;
    }

    uint8_t public[ECC_BYTES+1];
    if(!readfile(fname_pub, public, sizeof(public))) {
        return -1;
    }

    uint8_t signature[ECC_BYTES*2];
    if(!readfile(fname_sign, signature, sizeof(signature))) {
        return -1;
    }

    if(ecdsa_verify(public, hash, signature) == 0) {
        fprintf(stderr, "The signature is not valid!\n");
        return -1;
    }
    printf("This signature is valid.\n");
    return 0;
}

static void printhelp(const char *progname) {
    printf("Usage: %s COMMAND [ARGS]\n", progname);
    printf("Commands:\n");
    printf("    makekey PRIVATE_KEY_FILE_DEST PUBLIC_KEY_FILE_DEST\n");
    printf("    sign FILENAME PRIVATE_KEY_FILE SIGNATURE_FILE_DEST\n");
    printf("    verify FILENAME PUBLIC_KEY SIGNATURE_FILE\n");
}

static int wrongargcount(const char *progname) {
    fprintf(stderr, "Wrong arguments\n\n");
    printhelp(progname);
    return -1;
}

int main(int argc, const char * argv[]) {
    if(argc < 2) {
        fprintf(stderr, "No command\n\n");
        printhelp(argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "makekey") == 0) {
        if(argc != 4) {
            return wrongargcount(argv[0]);
        }
        return makekey(argv+2);
    } else if(strcmp(argv[1], "sign") == 0) {
        if(argc != 5) {
            return wrongargcount(argv[0]);
        }
        return sign(argv+2);
    } else if(strcmp(argv[1], "verify") == 0) {
        if(argc != 5) {
            return wrongargcount(argv[0]);
        }
        return verify(argv+2);
    } else {
        fprintf(stderr, "Unknown command: %s\n\n", argv[1]);
        printhelp(argv[0]);
        return -1;
    }
}