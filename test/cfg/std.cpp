
// Test library configuration for std.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/std.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <cstring>
#include <cstdio>
#include <cstdlib>

void bufferAccessOutOf(void) {
  char a[5];
  std::strcpy(a,"abcd");
  // cppcheck-suppress bufferAccessOutOfBounds
  // cppcheck-suppress redundantCopy
  std::strcpy(a, "abcde");
  // cppcheck-suppress redundantCopy
  std::strncpy(a,"abcde",5);
  // cppcheck-suppress bufferAccessOutOfBounds
  // cppcheck-suppress redundantCopy
  std::strncpy(a,"abcde",6);
}
