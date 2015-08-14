
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
#include <ctime>
#include <cctype>
#include <complex>

void bufferAccessOutOfBounds(void)
{
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

void uninitvar(void)
{
    int i;
    // cppcheck-suppress uninitvar
    std::abs(i);

    double d;
    const std::complex<double> dc(d,d);
    // cppcheck-suppress uninitvar
    std::proj(dc);

    // cppcheck-suppress uninitvar
    std::isalnum(i);
    // cppcheck-suppress uninitvar
    std::isalpha(i);
    // cppcheck-suppress uninitvar
    std::iscntrl(i);
    // cppcheck-suppress uninitvar
    std::isdigit(i);
    // cppcheck-suppress uninitvar
    std::isgraph(i);
    // cppcheck-suppress uninitvar
    std::islower(i);
    // cppcheck-suppress uninitvar
    std::isprint(i);
    // cppcheck-suppress uninitvar
    std::isspace(i);
    // cppcheck-suppress uninitvar
    std::isupper(i);
    // cppcheck-suppress uninitvar
    std::isxdigit(i);

    const struct tm *tm;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress obsoleteFunctionsasctime
    std::asctime(tm);
}
