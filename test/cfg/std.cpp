
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
#include <cassert>

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

void uninitvar_abs(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::abs(i);
}

void uninitvar_isalnum(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isalnum(i);
}

void uninitvar_isalpha(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isalpha(i);
}

void uninitvar_iscntrl(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::iscntrl(i);
}

void uninitvar_isdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isdigit(i);
}

void uninitvar_isgraph(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isgraph(i);
}

void uninitvar_islower(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::islower(i);
}

void uninitvar_isprint(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isprint(i);
}

void uninitvar_isspace(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isspace(i);
}

void uninitvar_isupper(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isupper(i);
}

void uninitvar_isxdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isxdigit(i);
}

void uninitvar_proj(void)
{
    double d;
    const std::complex<double> dc(d,d);
    // cppcheck-suppress uninitvar
    (void)std::proj(dc);
}

void uninitvar_acos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::acos(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::acos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::acos(ld);
}

void uninitvar_acosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::acosh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::acosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::acosh(ld);
}

void uninitvar_asctime(void)
{
    const struct tm *tm;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress obsoleteFunctionsasctime
    (void)std::asctime(tm);
}

void uninitvar_sqrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::sqrt(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::sqrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::sqrt(ld);
}

void uninitvar_sinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::sinh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::sinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::sinh(ld);
}

void uninitvar_sin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::sin(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::sin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::sin(ld);
}

void uninitvar_asin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::asin(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::asin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::asin(ld);
}

void uninitvar_asinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::asinh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::asinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::asinh(ld);
}
