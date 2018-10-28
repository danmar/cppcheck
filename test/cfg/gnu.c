
// Test library configuration for gnu.cfg
//
// Usage:
// $ cppcheck --check-library --library=gnu --enable=information --enable=style --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/gnu.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef __CYGWIN__
#include <sys/epoll.h>
#endif

void ignoreleak(void)
{
    char *p = (char *)malloc(10);
    __builtin_memset(&(p[0]), 0, 10);
    // cppcheck-suppress memleak
}

void uninitvar__builtin_memset(void)
{
    void *s;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)__builtin_memset(s,c,n);
}

void bufferAccessOutOfBounds__builtin_memset(void)
{
    uint8_t buf[42];
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)__builtin_memset(buf,0,1000);
}

void bufferAccessOutOfBounds()
{
    char buf[2];
    // This is valid
    sethostname(buf, 2);
    // cppcheck-suppress bufferAccessOutOfBounds
    sethostname(buf, 4);
}

void leakReturnValNotUsed()
{
    // cppcheck-suppress unreadVariable
    char* ptr = (char*)strdupa("test");
    // cppcheck-suppress ignoredReturnValue
    strdupa("test");
    // cppcheck-suppress unreadVariable
    char* ptr2 = (char*)strndupa("test", 1);
    // cppcheck-suppress ignoredReturnValue
    strndupa("test", 1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcasestr("test", NULL);

    // cppcheck-suppress knownConditionTrueFalse
    // cppcheck-suppress duplicateExpression
    if (42 == __builtin_expect(42, 0))
        return;
}

#ifndef __CYGWIN__
int nullPointer_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    // no warning is expected
    (void)epoll_ctl(epfd, op, fd, event);

    // No nullpointer warning is expected in case op is set to EPOLL_CTL_DEL
    //   EPOLL_CTL_DEL
    //          Remove (deregister) the target file descriptor fd from the
    //          epoll instance referred to by epfd.  The event is ignored and
    //          can be NULL.
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}
#endif
