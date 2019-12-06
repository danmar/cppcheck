
// Test library configuration for openssl.cfg
//
// Usage:
// $ cppcheck --check-library --library=openssl --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/openssl.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <string.h>

void valid_code(BIO * bio)
{
    BIO_printf(bio, "%d\n", 1);
}

// Example for encrypting a string using IDEA (from https://www.openssl.org/docs/man1.1.1/man3/EVP_CIPHER_CTX_new.html)
int valid_code_do_crypt(char *outfile)
{
    unsigned char outbuf[1024];
    int outlen, tmplen;
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};
    char intext[] = "Some Crypto Text";
    EVP_CIPHER_CTX *ctx;
    FILE *out;

    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    if (!EVP_EncryptUpdate(ctx, outbuf, &outlen, intext, strlen(intext))) {
        /* Error */
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    if (!EVP_EncryptFinal_ex(ctx, outbuf + outlen, &tmplen)) {
        /* Error */
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    outlen += tmplen;
    EVP_CIPHER_CTX_free(ctx);

    out = fopen(outfile, "wb");
    if (out == NULL) {
        /* Error */
        return 0;
    }
    fwrite(outbuf, 1, outlen, out);
    fclose(out);
    return 1;
}

void invalidPrintfArgType_test(BIO * bio)
{
    // cppcheck-suppress invalidPrintfArgType_sint
    BIO_printf(bio, "%d\n", 5U);
}

void EVP_CIPHER_CTX_new_test()
{
    EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
    printf("%p", ctx);
    // cppcheck-suppress resourceLeak
}
