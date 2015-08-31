
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

void uninitvar_ceil(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::ceil(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::ceil(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::ceil(ld);
}

void uninitvar_copysign(void)
{
    float f1, f2;
    // cppcheck-suppress uninitvar
    (void)std::copysign(f1, f2);

    double d1, d2;
    // cppcheck-suppress uninitvar
    (void)std::copysign(d1, d2);

    long double ld1, ld2;
    // cppcheck-suppress uninitvar
    (void)std::copysign(ld1, ld2);
}

void uninitvar_cbrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::cbrt(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::cbrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::cbrt(ld);
}

void uninitvar_cos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::cos(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::cos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::cos(ld);
}

void uninitvar_clearerr(void)
{
    FILE * stream;
    // cppcheck-suppress uninitvar
    std::clearerr(stream);
}

void uninitvar_cosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::cosh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::cosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::cosh(ld);
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

void uninitvar_ctime(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::ctime(tp);
}

void uninitvar_difftime(void)
{
    time_t t1,t2;
    // cppcheck-suppress uninitvar
    (void)std::difftime(t1, t2);
}

void uninitvar_div(void)
{
    int num;
    int denom;
    // cppcheck-suppress uninitvar
    (void)std::div(num,denom);
}

void uninitvar_exit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    std::exit(i);
}

void uninitvar_erf(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::erf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::erf(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::erf(ld);
}

void uninitvar_erfc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::erfc(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::erfc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::erfc(ld);
}

void uninitvar_exp(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::exp(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::exp(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::exp(ld);
}

void uninitvar_exp2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::exp2(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::exp2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::exp2(ld);
}

void uninitvar_expm1(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::expm1(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::expm1(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::expm1(ld);
}

void uninitvar_fabs(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::fabs(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::fabs(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::fabs(ld);
}

void uninitvar_fdim(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fdim(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fdim(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fdim(ld1,ld2);
}

void uninitvar_fclose(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fclose(stream);
}

void uninitvar_ferror(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ferror(stream);
}

void uninitvar_feof(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::feof(stream);
}

void uninitvar_fflush(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fflush(stream);
}

void uninitvar_fgetc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgetc(stream);
}

void uninitvar_fgetwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgetwc(stream);
}

void uninitvar_fgetpos(void)
{
    FILE* stream;
    fpos_t *ptr;
    // cppcheck-suppress uninitvar
    (void)std::fgetpos(stream,ptr);
}

void uninitvar_floor(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::floor(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::floor(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::floor(ld);
}

void uninitvar_fma(void)
{
    // cppcheck-suppress unassignedVariable
    float f1,f2,f3;
    // cppcheck-suppress uninitvar
    (void)std::fma(f1,f2,f3);

    // cppcheck-suppress unassignedVariable
    double d1,d2,d3;
    // cppcheck-suppress uninitvar
    (void)std::fma(d1,d2,d3);

    // cppcheck-suppress unassignedVariable
    long double ld1,ld2,ld3;
    // cppcheck-suppress uninitvar
    (void)std::fma(ld1,ld2,ld3);
}

void uninitvar_fmax(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fmax(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmax(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fmax(ld1,ld2);
}

void uninitvar_fmin(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fmin(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmin(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fmin(ld1,ld2);
}

void uninitvar_fmod(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fmod(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmod(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fmod(ld1,ld2);
}

void uninitar_fopen(void)
{
    char *filename;
    char *mode;
    // cppcheck-suppress uninitvar
    FILE * fp = std::fopen(filename, mode);
    fclose(fp);
}

void uninitar_fprintf(void)
{
    FILE *stream;
    char *format;
    int argument;
    // cppcheck-suppress uninitvar
    (void)std::fprintf(stream, format, argument);
}

void uninitar_vfprintf(void)
{
    FILE *stream;
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfprintf(stream, format, arg);
}

void uninitar_vfwprintf(void)
{
    FILE *stream;
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfwprintf(stream, format, arg);
}

void uninitvar_fputc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputc(c,stream);
}

void uninitvar_fputwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputwc(c,stream);
}

void uninitvar_fputs(void)
{
    char *string;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputs(string,stream);
}

void uninitvar_fputws(void)
{
    wchar_t *string;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputws(string,stream);
}

void uninitvar_fread(void)
{
    void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fread(ptr,size,nobj,stream);
}

void uninitvar_free(void)
{
    // cppcheck-suppress unassignedVariable
    void *block;
    // cppcheck-suppress uninitvar
    std::free(block);
}

void uninitvar_freopen(void)
{
    char *filename;
    char *mode;
    FILE *stream;
    // cppcheck-suppress uninitvar
    FILE * p = std::freopen(filename,mode,stream);
    free(p);
}

void uninitvar_frexp(void)
{
    float f1;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)std::frexp(f1,i1);

    double d1;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)std::frexp(d1,i2);

    long double ld1;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)std::frexp(ld1,i3);
}

void uninitvar_hypot(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::hypot(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::hypot(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::hypot(ld1,ld2);
}

void uninitvar_fscanf(void)
{
    FILE *stream;
    char *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fscanf(stream,format,i);
}
