
// Test library configuration for gtk.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --inconclusive --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --library=gtk test/cfg/gtk.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <gtk/gtk.h>

void validCode(int argInt)
{
    // if G_UNLIKELY is not defined this results in a syntax error
    if G_UNLIKELY(argInt == 1) {
    } else if (G_UNLIKELY(argInt == 2)) {
    }

    if G_LIKELY(argInt == 0) {
    } else if (G_LIKELY(argInt == -1)) {
    }
}
