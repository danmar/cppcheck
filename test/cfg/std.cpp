
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
    (void)std::abs(i);

    double d;
    const std::complex<double> dc(d,d);
    // cppcheck-suppress uninitvar
    (void)std::proj(dc);

    // cppcheck-suppress uninitvar
    (void)std::isalnum(i);
    // cppcheck-suppress uninitvar
    (void)std::isalpha(i);
    // cppcheck-suppress uninitvar
    (void)std::iscntrl(i);
    // cppcheck-suppress uninitvar
    (void)std::isdigit(i);
    // cppcheck-suppress uninitvar
    (void)std::isgraph(i);
    // cppcheck-suppress uninitvar
    (void)std::islower(i);
    // cppcheck-suppress uninitvar
    (void)std::isprint(i);
    // cppcheck-suppress uninitvar
    (void)std::isspace(i);
    // cppcheck-suppress uninitvar
    (void)std::isupper(i);
    // cppcheck-suppress uninitvar
    (void)std::isxdigit(i);

    const struct tm *tm;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress obsoleteFunctionsasctime
    (void)std::asctime(tm);
}
