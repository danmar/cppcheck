
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
#include <cwchar>
#include <cfenv>

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

void uninitvar_wcsftime(wchar_t* ptr)
{
    size_t maxsize;
    wchar_t* format;
    struct tm* timeptr;
    // cppcheck-suppress uninitvar
    (void)std::wcsftime(ptr, maxsize, format, timeptr);
}

void uninitvar_tan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::tan(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::tan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::tan(ld);
}

void uninitvar_tanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::tanh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::tanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::tanh(ld);
}

void uninitvar_atan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::atan(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::atan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::atan(ld);
}

void uninitvar_tgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::tgamma(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::tgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::tgamma(ld);
}

void uninitvar_trunc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::trunc(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::trunc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::trunc(ld);
}

void uninitvar_atanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::atanh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::atanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::atanh(ld);
}

void uninitvar_atan2(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::atan2(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::atan2(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::atan2(ld1,ld2);
}

void uninitvar_atol(void)
{
    char * c;
    // cppcheck-suppress uninitvar
    (void)std::atoi(c);

    // cppcheck-suppress uninitvar
    (void)std::atol(c);

    // cppcheck-suppress uninitvar
    (void)std::atoll(c);
}

void uninitvar_feraiseexcept(void)
{
    int excepts;
    // cppcheck-suppress uninitvar
    (void)std::feraiseexcept(excepts);
}

void uninitvar_fesetexceptflag(fexcept_t* flagp)
{
    int excepts;
    // cppcheck-suppress uninitvar
    (void)std::fesetexceptflag(flagp, excepts);
}

void uninitvar_feclearexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::feclearexcept(i);
}

void uninitvar_fesetenv(void)
{
    fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)std::fesetenv(envp);
}

void uninitvar_fesetround(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fesetround(i);
}

void uninitvar_fetestexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fetestexcept(i);
}

void uninitvar_feupdateenv(void)
{
    fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)std::feupdateenv(envp);
}
