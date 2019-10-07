
// Test library configuration for openssl.cfg
//
// Usage:
// $ cppcheck --check-library --library=openssl --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/openssl.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <openssl/ssl.h>
#include <openssl/bio.h>

void validCode(BIO * bio)
{
    BIO_printf(bio, "%d\n", 1);
}

void invalidPrintfArgType_test(BIO * bio)
{
    // cppcheck-suppress invalidPrintfArgType_sint
    BIO_printf(bio, "%d\n", 5U);
}
