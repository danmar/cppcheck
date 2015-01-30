
// Test library configuration for posix.cfg
//
// Usage:
// $ cppcheck --check-library --library=posix --enable=information --error-exitcode=1 --inline-suppr cfg/test/posix.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <unistd.h>

void f(char *p) {
    isatty (0);
    mkdir (p, 0);
    getcwd (0, 0);
    // cppcheck-suppress nullPointer
    readdir (0);
}

