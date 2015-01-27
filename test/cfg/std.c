
// Test library configuration for std.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --inline-suppr cfg/test/std.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>

void strcpy_ok(char *a, char *b) {
    strcpy(a,b);
}

void strcpy_bad() {
  char a[10];
  // cppcheck-suppress bufferAccessOutOfBounds
  strcpy(a, "hello world!");
}


