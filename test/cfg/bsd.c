// Test library configuration for bsd.cfg
//
// Usage:
// $ cppcheck --library=bsd --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/bsd.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

// #9323, #9331
void verify_timercmp(struct timeval t)
{
    (void)timercmp(&t, &t, <);
    (void)timercmp(&t, &t, <=);
    (void)timercmp(&t, &t, ==);
    (void)timercmp(&t, &t, !=);
    (void)timercmp(&t, &t, >=);
    (void)timercmp(&t, &t, >);
}

// False negative: #9346
void uninitvar_timercmp(struct timeval t)
{
    struct timeval uninit;
    (void)timercmp(&t, &uninit, <);
    (void)timercmp(&uninit, &t, <=);
    (void)timercmp(&uninit, &uninit, ==);
}

void nullPointer_timercmp(struct timeval t)
{
    struct timeval *p=0;
    // cppcheck-suppress nullPointer
    (void)timercmp(&t, p, <);
    // cppcheck-suppress nullPointer
    (void)timercmp(p, &t, <=);
    // cppcheck-suppress nullPointer
    (void)timercmp(p, p, ==);
}

// size_t strlcat(char *dst, const char *src, size_t size);
void uninitvar_strlcat(char *Ct, const char *S, size_t N)
{
    char *ct;
    char *s;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)strlcat(ct,s,n1);
    // cppcheck-suppress uninitvar
    (void)strlcat(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)strlcat(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)strlcat(Ct,S,n2);

    // no warning is expected for
    (void)strlcat(Ct,S,N);
}

void bufferAccessOutOfBounds(void)
{
    uint16_t uint16Buf[4];
    // cppcheck-suppress bufferAccessOutOfBounds
    arc4random_buf(uint16Buf, 9);
    // valid
    arc4random_buf(uint16Buf, 8);
}

void ignoredReturnValue(void)
{
    // cppcheck-suppress ignoredReturnValue
    arc4random();
    // cppcheck-suppress ignoredReturnValue
    arc4random_uniform(10);
}

void invalidFunctionArg()
{
    // cppcheck-suppress invalidFunctionArg
    (void) arc4random_uniform(1);
    // valid
    (void) arc4random_uniform(2);
}

void nullPointer(void)
{
    // cppcheck-suppress nullPointer
    arc4random_buf(NULL, 5);
}

void uninitvar(void)
{
    uint32_t uint32Uninit;

    // cppcheck-suppress uninitvar
    (void) arc4random_uniform(uint32Uninit);
}
