
// Test library configuration for gnu.cfg
//
// Usage:
// $ cppcheck --check-library --library=gnu --enable=information --enable=style --error-exitcode=1 --inline-suppr test/cfg/gnu.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>
#include <unistd.h>

void leakReturnValNotUsed() {
   // cppcheck-suppress unreadVariable
   char* ptr = strdupa("test");
   // cppcheck-suppress ignoredReturnValue
   strdupa("test");
   // cppcheck-suppress unreadVariable
   char* ptr2 = strndupa("test", 1);
   // cppcheck-suppress ignoredReturnValue
   strndupa("test", 1);
}
