// Test library configuration for bsd.cfg
//
// Usage:
// $ cppcheck --library=bsd --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/bsd.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>

// size_t strlcat(char *dst, const char *src, size_t size);
void uninitvar_strlcat(char *Ct, const char *S, size_t N)
{
    char *ct;
    char *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)strlcat(ct,s,n);
    // cppcheck-suppress uninitvar
    (void)strlcat(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)strlcat(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)strlcat(Ct,S,n);

    // no warning is expected for
    (void)strlcat(Ct,S,N);
}
