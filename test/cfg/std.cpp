
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
#include <csetjmp>
#include <cmath>
#include <csignal>
#include <iostream>
#include <iomanip>
#include <cinttypes>
#include <istream>
#include <fstream>
#include <vector>
#include <cstdarg>

void returnValue_std_isgreater(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::isgreater(4,2) == 0) {}
    // @todo support floats
    if (std::isgreater(4.0f,2.0f) == 0) {}
}

void returnValue_std_isgreaterequal(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::isgreaterequal(4,2) == 0) {}
    // @todo support floats
    if (std::isgreaterequal(4.0f,2.0f) == 0) {}
}

void returnValue_std_isless(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::isless(4,2) == 0) {}
    // @todo support floats
    if (std::isless(4.0f,2.0f) == 0) {}
}

void returnValue_std_islessequal(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::islessequal(4,2) == 0) {}
    // @todo support floats
    if (std::islessequal(4.0f,2.0f) == 0) {}
}

void returnValue_std_islessgreater(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::islessgreater(4,2) == 0) {}
    // cppcheck-suppress knownConditionTrueFalse
    if (std::islessgreater(2,4) == 0) {}

    if (std::islessgreater(4.0f,2.0f) == 0) {} // @todo support floats
    if (std::islessgreater(2.0f,4.0f) == 0) {} // @todo support floats
}

void bufferAccessOutOfBounds(void)
{
    char a[5];
    std::strcpy(a,"abcd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    std::strcpy(a, "abcde");
    // TODO cppcheck-suppress redundantCopy
    std::strncpy(a,"abcde",5);
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    std::strncpy(a,"abcde",6);
}

void uninitvar_abs(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::abs(i);
}

void uninivar_imaxabs(void)
{
    intmax_t i;
    // cppcheck-suppress uninitvar
    (void)std::imaxabs(i);
    // cppcheck-suppress uninitvar
    (void)imaxabs(i);
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
    // TODO cppcheck-suppress uninitvar
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
    (void)std::acoshf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::acosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::acoshl(ld);
}

void uninitvar_asctime(void)
{
    const struct tm *tm;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress asctimeCalled
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
    (void)std::asinhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::asinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::asinhl(ld);
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
    (void)std::tgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::tgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::tgammal(ld);
}

void uninitvar_trunc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::truncf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::trunc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::truncl(ld);
}

void uninitvar_atanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::atanhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::atanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::atanhl(ld);
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

void uninitvar_atof(void)
{
    char * c;
    // cppcheck-suppress uninitvar
    (void)std::atof(c);
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
    (void)std::copysignf(f1, f2);

    double d1, d2;
    // cppcheck-suppress uninitvar
    (void)std::copysign(d1, d2);

    long double ld1, ld2;
    // cppcheck-suppress uninitvar
    (void)std::copysignl(ld1, ld2);
}

void uninitvar_cbrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::cbrtf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::cbrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::cbrtl(ld);
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

void uninitvar_imaxdiv(void)
{
    intmax_t numer;
    intmax_t denom;
    // cppcheck-suppress uninitvar
    (void)std::imaxdiv(numer,denom);
    // cppcheck-suppress uninitvar
    (void)imaxdiv(numer,denom);
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
    (void)std::erff(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::erf(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::erfl(ld);
}

void uninitvar_erfc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::erfcf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::erfc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::erfcl(ld);
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
    (void)std::exp2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::exp2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::exp2l(ld);
}

void uninitvar_expm1(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::expm1f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::expm1(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::expm1l(ld);
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
    (void)std::fdimf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fdim(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fdiml(ld1,ld2);
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
    float f1,f2,f3;
    // cppcheck-suppress uninitvar
    (void)std::fmaf(f1,f2,f3);

    double d1,d2,d3;
    // cppcheck-suppress uninitvar
    (void)std::fma(d1,d2,d3);

    long double ld1,ld2,ld3;
    // cppcheck-suppress uninitvar
    (void)std::fmal(ld1,ld2,ld3);
}

void uninitvar_fmax(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fmaxf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmax(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fmaxl(ld1,ld2);
}

void uninitvar_fmin(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fminf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmin(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fminl(ld1,ld2);
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

void uninitar_fprintf(FILE *Stream, char *Format, int Argument)
{
    FILE *stream;
    char *format;
    int argument;
    // cppcheck-suppress uninitvar
    (void)std::fprintf(stream, format, argument);
    // cppcheck-suppress uninitvar
    (void)std::fprintf(stream, Format, Argument);
    // cppcheck-suppress uninitvar
    (void)std::fprintf(Stream, format, Argument);
    // cppcheck-suppress uninitvar
    (void)std::fprintf(Stream, Format, argument);

    // no warning is expected
    (void)std::fprintf(Stream, Format, Argument);
}

void uninitar_vfprintf(FILE *Stream, const char *Format, va_list Arg)
{
    FILE *stream;
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfprintf(stream, format, arg);
    // cppcheck-suppress uninitvar
    (void)std::vfprintf(stream, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)std::vfprintf(Stream, format, Arg);

    // no warning is expected
    (void)std::vfprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vfprintf(Stream, Format, arg);
}

void uninitar_vfwprintf(FILE *Stream, wchar_t *Format, va_list Arg)
{
    FILE *stream;
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfwprintf(stream, format, arg);
    // cppcheck-suppress uninitvar
    (void)std::vfwprintf(stream, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)std::vfwprintf(Stream, format, Arg);

    // no warning is expected
    (void)std::vfwprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vfwprintf(Stream, Format, arg);
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
    (void)std::hypotf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::hypot(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::hypotl(ld1,ld2);
}

void uninitvar_fscanf(void)
{
    FILE *stream;
    char *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fscanf(stream,format,i);
}

void uninitvar_vfscanf(void)
{
    FILE *stream;
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfscanf(stream,format,arg);
}

void uninitvar_vfwscanf(void)
{
    FILE *stream;
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfwscanf(stream,format,arg);
}

void uninitvar_fseek(void)
{
    FILE* stream;
    long int offset;
    int origin;
    // cppcheck-suppress uninitvar
    (void)std::fseek(stream,offset,origin);
}

void uninitvar_fsetpos(void)
{
    FILE* stream;
    fpos_t *ptr;
    // cppcheck-suppress uninitvar
    (void)std::fsetpos(stream,ptr);
}

void uninitvar_fgets(void)
{
    char *buffer;
    int n;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgets(buffer,n,stream);
}

void uninitvar_fgetws(void)
{
    wchar_t *buffer;
    int n;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgetws(buffer,n,stream);
}

void uninitvar_ftell(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ftell(stream);
}

void uninitvar_fwide(void)
{
    FILE *stream;
    int mode;
    // cppcheck-suppress uninitvar
    (void)std::fwide(stream,mode);
}

void uninitvar_fwrite(void)
{
    void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fwrite(ptr,size,nobj,stream);
}

void uninitvar_mblen(void)
{
    char *string;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)std::mblen(string,size);
}

void uninitvar_mbtowc(void)
{
    wchar_t* pwc;
    char* pmb;
    size_t max;
    // cppcheck-suppress uninitvar
    (void)std::mbtowc(pwc,pmb,max);
}

void uninitvar_mbrlen(const char* p, size_t m, mbstate_t* s)
{
    char* pmb;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(pmb,max,ps);
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(pmb,m,s);
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(p,max,s);
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(p,m,ps);
    // no warning is expected
    (void)std::mbrlen(p,m,s);
}

void uninitvar_btowc(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::btowc(c);
}

void uninitvar_mbsinit(void)
{
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)std::mbsinit(ps);
}

void uninitvar_mbstowcs(void)
{
    wchar_t *ws;
    char *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::mbstowcs(ws,s,n);
}

void uninitvar_mbsrtowcs(void)
{
    wchar_t* dest;
    const char* src;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)std::mbsrtowcs(dest,&src,max,ps);
}

void uninitvar_wctob(void)
{
    wint_t wc;
    // cppcheck-suppress uninitvar
    (void)std::wctob(wc);
}

void uninitvar_wctomb(void)
{
    char *s;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)std::wctomb(s,wc);
}

void uninitvar_wcstombs(void)
{
    char *mbstr;
    wchar_t *wcstr;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcstombs(mbstr,wcstr,n);
}

void uninitvar_getc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::getc(stream);
}

void uninitvar_getwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::getwc(stream);
}

void uninitvar_ungetc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ungetc(c,stream);
}

void uninitvar_ungetwc(void)
{
    wint_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ungetwc(c,stream);
}

void uninitvar_getenv(void)
{
    char *name;
    // cppcheck-suppress uninitvar
    (void)std::getenv(name);
}

void uninitvar_gmtime(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::gmtime(tp);
}

void uninitvar_iswalnum(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswalnum(i);
}

void uninitvar_iswalpha(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswalpha(i);
}

void uninitvar_isblank(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isblank(i);
}

void uninitvar_iswblank(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswblank(i);
}

void uninitvar_iswcntrl(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswcntrl(i);
}

void uninitvar_iswctype(void)
{
    wint_t c;
    wctype_t desc;
    // cppcheck-suppress uninitvar
    (void)std::iswctype(c,desc);
}

void uninitvar_iswdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswdigit(i);
}

void uninitvar_iswgraph(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswgraph(i);
}

void uninitvar_iswlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswlower(i);
}

void uninitvar_iswprint(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswprint(i);
}

void uninitvar_ispunct(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::ispunct(i);
}

void uninitvar_iswpunct(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswpunct(i);
}

void uninitvar_iswspace(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswspace(i);
}

void uninitvar_iswupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswupper(i);
}

void uninitvar_iswxdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswxdigit(i);
}

void uninitvar_towctrans(void)
{
    wint_t c;
    wctrans_t desc;
    // cppcheck-suppress uninitvar
    (void)std::towctrans(c,desc);
}

void uninitvar_towlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::towlower(i);
}

void uninitvar_towupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::towupper(i);
}

void uninitvar_wctrans(void)
{
    char* property;
    // cppcheck-suppress uninitvar
    (void)std::wctrans(property);
}

void uninitvar_wctype(void)
{
    char* property;
    // cppcheck-suppress uninitvar
    (void)std::wctype(property);
}

void uninitvar_labs(void)
{
    long int li;
    // cppcheck-suppress uninitvar
    (void)std::labs(li);

    long long int lli;
    // cppcheck-suppress uninitvar
    (void)std::llabs(lli);
}

void uninitvar_ldexp(void)
{
    float fd;
    int e1;
    // cppcheck-suppress uninitvar
    (void)std::ldexp(fd,e1);

    double dc;
    int e2;
    // cppcheck-suppress uninitvar
    (void)std::ldexp(dc,e2);

    long double ldc;
    int e3;
    // cppcheck-suppress uninitvar
    (void)std::ldexp(ldc,e3);
}

void uninitvar_lgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::lgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::lgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::lgammal(ld);
}

void uninitvar_rint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::rintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::rint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::rintl(ld);
}

void uninitvar_lrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::lrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::lrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::lrintl(ld);
}

void uninitvar_llrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::llrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::llrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::llrintl(ld);
}

void uninitvar_lround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::lroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::lround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::lroundl(ld);
}

void uninitvar_llround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::llroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::llround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::llroundl(ld);
}

void uninitvar_srand(void)
{
    unsigned int seed;
    // cppcheck-suppress uninitvar
    (void)std::srand(seed);
}

void uninitvar_ldiv(void)
{
    long int l1;
    long int l2;
    // cppcheck-suppress uninitvar
    (void)std::ldiv(l1,l2);

    long long int ll1;
    long long int ll2;
    // cppcheck-suppress uninitvar
    (void)std::lldiv(ll1,ll2);
}

void uninitvar_localtime(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::localtime(tp);
}

void uninitvar_log(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log(ld);
}

void uninitvar_fpclassify(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::fpclassify(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::fpclassify(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::fpclassify(ld);
}

void uninitvar_isfinite(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::isfinite(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::isfinite(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::isfinite(ld);
}

void uninitvar_isgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::isgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::isgreater(ld1,ld2);
}

void uninitvar_isgreaterequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::isgreaterequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isgreaterequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::isgreaterequal(ld1,ld2);
}

void uninitvar_isinf(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::isinf(d);
}

void uninitvar_logb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::logbf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::logb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::logbl(ld);
}

void uninitvar_isless(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::isless(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isless(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::isless(ld1,ld2);
}

void uninitvar_islessequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::islessequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::islessequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::islessequal(ld1,ld2);
}

void uninitvar_islessgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::islessgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::islessgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::islessgreater(ld1,ld2);
}

void uninitvar_nan(void)
{
    char *tagp;
    // cppcheck-suppress uninitvar
    (void)std::nanf(tagp);
    // cppcheck-suppress uninitvar
    (void)std::nan(tagp);
    // cppcheck-suppress uninitvar
    (void)std::nanl(tagp);
}

void uninitvar_isnan(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::isnan(d);
}

void uninitvar_isnormal(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::isnormal(d);
}

void uninitvar_isunordered(void)
{
    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isunordered(d1,d2);
}

void uninitvar_ilogb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::ilogb(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::ilogb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::ilogb(ld);
}

void uninitvar_log10(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log10(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log10(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log10(ld);
}

void uninitvar_log1p(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log1pf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log1p(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log1pl(ld);
}

void uninitvar_log2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log2l(ld);
}

void uninitvar_nearbyint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::nearbyintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::nearbyint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::nearbyintl(ld);
}

void uninitvar_nextafter(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::nextafterf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::nextafter(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::nextafterl(ld1,ld2);
}

void uninitvar_nexttoward(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::nexttowardf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::nexttoward(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::nexttowardl(ld1,ld2);
}

void uninitvar_longjmp(void)
{
    jmp_buf env;
    int val;
    // cppcheck-suppress uninitvar
    (void)std::longjmp(env,val);
}

void uninitvar_malloc(void)
{
    size_t size;
    // cppcheck-suppress uninitvar
    int *p = (int*)std::malloc(size);
    free(p);
}

void uninitvar_memchr(void)
{
    void *cs;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memchr(cs,c,n);
}

void uninitvar_wmemchr(void)
{
    wchar_t *cs;
    wchar_t c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemchr(cs,c,n);
}

void uninitvar_memcmp(void)
{
    void *s1;
    void *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memcmp(s1,s2,n);
}

void uninitvar_wmemcmp(void)
{
    wchar_t *s1;
    wchar_t *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemcmp(s1,s2,n);
}

void uninitvar_memcpy(void)
{
    void *ct;
    void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memcpy(ct,cs,n);
}

void uninitvar_wmemcpy(void)
{
    wchar_t *cs;
    wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemcpy(cs,c,n);
}

void uninitvar_memmove(void)
{
    void *ct;
    void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memmove(ct,cs,n);
}

void uninitvar_wmemmove(void)
{
    wchar_t *cs;
    wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemmove(cs,c,n);
}

void uninitvar_memset(void)
{
    void *s;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memset(s,c,n);
}

void uninitvar_wmemset(void)
{
    wchar_t *cs;
    wchar_t  c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemset(cs,c,n);
}

void uninitvar_mktime(void)
{
    struct tm *tp;
    // cppcheck-suppress uninitvar
    (void)std::mktime(tp);
}

void uninivar_modf(void)
{
    float f1;
    float *f2;
    // cppcheck-suppress uninitvar
    (void)std::modf(f1,f2);

    double d1;
    double *d2;
    // cppcheck-suppress uninitvar
    (void)std::modf(d1,d2);

    long double ld1;
    long double *ld2;
    // cppcheck-suppress uninitvar
    (void)std::modf(ld1,ld2);
}

void uninivar_perror(void)
{
    char *string;
    // cppcheck-suppress uninitvar
    (void)std::perror(string);
}

void uninitvar_pow(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::pow(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::pow(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::pow(ld1,ld2);
}

void uninitvar_remainder(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::remainderf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::remainder(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::remainderl(ld1,ld2);
}

void uninitvar_remquo(void)
{
    float f1,f2;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)std::remquof(f1,f2,i1);

    double d1,d2;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)std::remquo(d1,d2,i2);

    long double ld1,ld2;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)std::remquol(ld1,ld2,i3);
}

void uninivar_printf(char *Format, int Argument)
{
    char * format;
    int argument;
    // no warning is expected
    (void)std::printf("x");
    // cppcheck-suppress uninitvar
    (void)std::printf(format,argument);
    // cppcheck-suppress uninitvar
    (void)std::printf(Format,argument);
    // cppcheck-suppress uninitvar
    (void)std::printf(format,Argument);
    // cppcheck-suppress uninitvar
    (void)std::printf(format,1);

    // no warning is expected
    (void)std::printf(Format,Argument);
}

void uninivar_vprintf(char *Format, va_list Arg)
{
    char * format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vprintf(format,arg);
    // cppcheck-suppress uninitvar
    (void)std::vprintf(format,Arg);

    // no warning is expected
    (void)std::vprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vprintf(Format,arg);
}

void uninivar_vwprintf(wchar_t *Format, va_list Arg)
{
    wchar_t * format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vwprintf(format,arg);
    // cppcheck-suppress uninitvar
    (void)std::vwprintf(format,Arg);

    // no warning is expected
    (void)std::vwprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vwprintf(Format,arg);
}

void uninivar_bsearch(void)
{
    void* key;
    void* base;
    size_t num;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)std::bsearch(key,base,num,size,(int(*)(const void*,const void*)) strcmp);
}

void uninitvar_qsort(void)
{
    void *base;
    size_t n;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)std::qsort(base,n,size, (int(*)(const void*,const void*)) strcmp);
}

void uninitvar_putc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::putc(c,stream);
}

void uninitvar_putwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::putc(c,stream);
}

void uninitvar_putchar(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::putchar(c);
}

void uninitvar_putwchar(void)
{
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)std::putwchar(c);
}

void uninitvar_puts(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)std::puts(s);
}

void uninitvar_realloc(void)
{
    void *block;
    size_t newsize;
    // cppcheck-suppress uninitvar
    void *p = std::realloc(block, newsize);
    free(p);
}

void uninitvar_remove(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)std::remove(s);
}

void uninitvar_rename(void)
{
    char *s1;
    char *s2;
    // cppcheck-suppress uninitvar
    (void)std::rename(s1,s2);
}

void uninitvar_rewind(void)
{
    FILE *f;
    // cppcheck-suppress uninitvar
    (void)std::rewind(f);
}

void uninitvar_round(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::roundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::round(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::roundl(ld);
}

void uninivar_scalbn(void)
{
    float f;
    int i1;
    // cppcheck-suppress uninitvar
    (void)std::scalbnf(f,i1);

    double d;
    int i2;
    // cppcheck-suppress uninitvar
    (void)std::scalbn(d,i2);

    long double ld;
    int i3;
    // cppcheck-suppress uninitvar
    (void)std::scalbnl(ld,i3);
}

void uninivar_scalbln(void)
{
    float f;
    long int i1;
    // cppcheck-suppress uninitvar
    (void)std::scalblnf(f,i1);

    double d;
    long int i2;
    // cppcheck-suppress uninitvar
    (void)std::scalbln(d,i2);

    long double ld;
    long int i3;
    // cppcheck-suppress uninitvar
    (void)std::scalblnl(ld,i3);
}

void uninitvar_signbit(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::signbit(d);
}

void uninivar_signal(void)
{
    int i;
    // cppcheck-suppress uninitvar
    std::signal(i, exit);
}

void uninivar_raise(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::raise(i);
}

void uninivar_scanf(void)
{
    char *format;
    char str[42];
    // cppcheck-suppress uninitvar
    (void)std::scanf(format, str);
}

void uninivar_vsscanf(void)
{
    char *s;
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vsscanf(s,format,arg);
}

void uninivar_vswscanf(void)
{
    wchar_t *s;
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vswscanf(s,format,arg);
}

void uninivar_vscanf(void)
{
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vscanf(format,arg);
}

void uninivar_vwscanf(void)
{
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vwscanf(format,arg);
}

void uninivar_setbuf(void)
{
    FILE *stream;
    char *buf;
    // cppcheck-suppress uninitvar
    (void)std::setbuf(stream,buf);
}

void uninivar_setvbuf(void)
{
    FILE *stream;
    char *buf;
    int mode;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)std::setvbuf(stream,buf,mode,size);
}

void uninitvar_strcat(char *dest, const char * const source)
{
    char *deststr;
    char *srcstr;
    // cppcheck-suppress uninitvar
    (void)std::strcat(deststr,srcstr);
    // cppcheck-suppress uninitvar
    (void)std::strcat(dest,srcstr);
    // cppcheck-suppress uninitvar
    (void)std::strcat(deststr,source);

    // no warning shall be shown for
    (void)std::strcat(dest,source);
}

void uninitvar_wcscat(wchar_t *dest, const wchar_t * const source)
{
    wchar_t *deststr;
    wchar_t *srcstr;
    // cppcheck-suppress uninitvar
    (void)std::wcscat(deststr,srcstr);
    // cppcheck-suppress uninitvar
    (void)std::wcscat(dest,srcstr);
    // cppcheck-suppress uninitvar
    (void)std::wcscat(deststr,source);

    // no warning shall be shown for
    (void)std::wcscat(dest,source);
}

void uninivar_wcrtomb(void)
{
    char *s;
    wchar_t wc;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)std::wcrtomb(s,wc,ps);
}

void uninivar_strchr(void)
{
    char *cs;
    int c;
    // cppcheck-suppress uninitvar
    (void)std::strchr(cs,c);
}

void uninivar_wcschr(void)
{
    wchar_t *cs;
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)std::wcschr(cs,c);
}

void uninivar_strcmp(void)
{
    char *str1;
    char *str2;
    // cppcheck-suppress uninitvar
    (void)std::strcmp(str1,str2);
}

void uninivar_wcscmp(void)
{
    wchar_t *str1;
    wchar_t *str2;
    // cppcheck-suppress uninitvar
    (void)std::wcscmp(str1,str2);
}

void uninivar_strcpy(void)
{
    char *str1;
    char *str2;
    // cppcheck-suppress uninitvar
    (void)std::strcpy(str1,str2);
}

void uninivar_wcscpy(void)
{
    wchar_t *str1;
    wchar_t *str2;
    // cppcheck-suppress uninitvar
    (void)std::wcscpy(str1,str2);
}

void uninivar_strftime(void)
{
    char *s;
    size_t max;
    char *fmt;
    struct tm *p;
    // cppcheck-suppress uninitvar
    (void)std::strftime(s,max,fmt,p);
}

void uninivar_strlen(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)std::strlen(s);
}

void uninivar_wcslen(void)
{
    wchar_t *s;
    // cppcheck-suppress uninitvar
    (void)std::wcslen(s);
}

void uninivar_strncpy(void)
{
    char *s;
    char *ct;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::strncpy(s,ct,n);
}

void uninivar_strpbrk(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strpbrk(cs,ct);
}

void uninivar_strncat(char *Ct, char *S, size_t N)
{
    char *ct;
    char *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::strncat(ct,s,n);
    // cppcheck-suppress uninitvar
    (void)std::strncat(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)std::strncat(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)std::strncat(Ct,S,n);

    // no warning is expected for
    (void)std::strncat(Ct,S,N);
}

void uninivar_wcsncat(wchar_t *Ct, wchar_t *S, size_t N)
{
    wchar_t *ct;
    wchar_t *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(ct,s,n);
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(Ct,S,n);

    // no warning is expected for
    (void)std::wcsncat(Ct,S,N);
}

void uninivar_strncmp(char *Ct, char *S, size_t N)
{
    char *ct;
    char *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::strncmp(ct,s,n);
    // cppcheck-suppress uninitvar
    (void)std::strncmp(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)std::strncmp(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)std::strncmp(Ct,S,n);

    // no warning is expected for
    (void)std::strncmp(Ct,S,N);
}

void uninivar_wcsncmp(wchar_t *Ct, wchar_t *S, size_t N)
{
    wchar_t *ct;
    wchar_t *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(ct,s,n);
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(Ct,S,n);

    // no warning is expected for
    (void)std::wcsncmp(Ct,S,N);
}

void uninivar_strstr(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strstr(cs,ct);
}

void uninivar_wcsstr(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcsstr(cs,ct);
}

void uninivar_strspn(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strspn(cs,ct);
}

void uninivar_strxfrm(void)
{
    char *ds;
    char *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::strxfrm(ds,ss,n);
}

void uninivar_wcsxfrm(void)
{
    wchar_t *ds;
    wchar_t *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcsxfrm(ds,ss,n);
}

void uninivar_wcsspn(void)
{
    wchar_t *ds;
    wchar_t *ss;
    // cppcheck-suppress uninitvar
    (void)std::wcsspn(ds,ss);
}

void uninivar_setlocale(void)
{
    int category;
    char* locale;
    // cppcheck-suppress uninitvar
    (void)std::setlocale(category,locale);
}
void uninivar_strerror(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::strerror(i);
}

void uninivar_strcspn(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strcspn(cs,ct);
}

void uninivar_wcscspn(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcscspn(cs,ct);
}

void uninivar_wcspbrk(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcspbrk(cs,ct);
}

void uninivar_wcsncpy(void)
{
    wchar_t *cs;
    wchar_t *ct;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcsncpy(cs,ct,n);
}

void uninivar_strcoll(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strcoll(cs,ct);
}

void uninivar_wcscoll(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcscoll(cs,ct);
}

void uninivar_strrchr(void)
{
    char * str;
    int c;
    // cppcheck-suppress uninitvar
    (void)std::strrchr(str,c);
}

void uninivar_wcsrchr(void)
{
    wchar_t* ws;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)std::wcsrchr(ws,wc);
}

void uninivar_wcsrtombs(void)
{
    char *dst;
    const wchar_t * p;;
    size_t len;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)std::wcsrtombs(dst,&p,len,ps);
}

void uninivar_strtok(void)
{
    char *s;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strtok(s,ct);
}

void uninivar_strtoimax(void)
{
    const char *s;
    char **endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)std::strtoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::strtoumax(s,endp,base);
}

void uninivar_strtof(void)
{
    const char *s;
    char **endp;
    // cppcheck-suppress uninitvar
    (void)std::strtof(s,endp);
    // cppcheck-suppress uninitvar
    (void)std::strtod(s,endp);
    // cppcheck-suppress uninitvar
    (void)std::strtold(s,endp);
}

void uninivar_strtol(void)
{
    const char *s;
    char **endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)std::strtol(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::strtoll(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::strtoul(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::strtoull(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::strtoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)strtoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::strtoumax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)strtoumax(s,endp,base);
}

void uninitvar_time(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::time(tp);
}

void uninitvar_tmpnam(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)std::tmpnam(s);
}

void uninivar_tolower(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::tolower(c);
}

void uninivar_toupper(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::toupper(c);
}

void uninivar_wcstof(void)
{
    const wchar_t *s;
    wchar_t **endp;
    // cppcheck-suppress uninitvar
    (void)std::wcstof(s,endp);
    // cppcheck-suppress uninitvar
    (void)std::wcstod(s,endp);
    // cppcheck-suppress uninitvar
    (void)std::wcstold(s,endp);
}

void uninivar_stoX(void)
{
    std::string str;
    std::wstring wstr;
    size_t* idx;
    // cppcheck-suppress uninitvar
    (void)std::stod(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stod(wstr,idx);
    // cppcheck-suppress uninitvar
    (void)std::stof(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stof(wstr,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoi(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoi(wstr,idx);
    // cppcheck-suppress uninitvar
    (void)std::stol(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stol(wstr,idx);
    // cppcheck-suppress uninitvar
    (void)std::stold(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stold(wstr,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoll(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoll(wstr,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoul(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoul(wstr,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoull(str,idx);
    // cppcheck-suppress uninitvar
    (void)std::stoull(wstr,idx);
}

void uninivar_to_string(void)
{
    int i;
    long l;
    long long ll;
    unsigned u;
    unsigned long ul;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::to_string(i);
    // cppcheck-suppress uninitvar
    (void)std::to_string(l);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ll);
    // cppcheck-suppress uninitvar
    (void)std::to_string(u);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ul);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ull);
    // cppcheck-suppress uninitvar
    (void)std::to_string(f);
    // cppcheck-suppress uninitvar
    (void)std::to_string(d);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ld);
}

void uninivar_to_wstring(void)
{
    int i;
    long l;
    long long ll;
    unsigned u;
    unsigned long ul;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(i);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(l);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ll);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(u);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ul);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ull);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(f);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(d);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ld);
}

void uninivar_mbrtowc(void)
{
    wchar_t* pwc;
    const char* pmb;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)std::mbrtowc(pwc,pmb,max,ps);
}

void uninivar_wcstok(void)
{
    wchar_t *s;
    const wchar_t *ct;
    wchar_t **ptr;
    // cppcheck-suppress uninitvar
    (void)std::wcstok(s,ct,ptr);
}

void uninivar_wcstoimax(void)
{
    const wchar_t *s;
    wchar_t ** endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)std::wcstoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::wcstoumax(s,endp,base);
}

void uninivar_wcstol(void)
{
    const wchar_t *s;
    wchar_t ** endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)std::wcstol(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::wcstoll(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::wcstoul(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::wcstoull(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::wcstoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)wcstoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)std::wcstoumax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)wcstoumax(s,endp,base);
}

void uninitvar_wprintf(wchar_t *Format, int Argument)
{
    const wchar_t *format;
    int argument;
    // cppcheck-suppress uninitvar
    (void)std::wprintf(format,argument);
    // cppcheck-suppress uninitvar
    (void)std::wprintf(format);
    // cppcheck-suppress uninitvar
    (void)std::wprintf(Format,argument);
    // cppcheck-suppress uninitvar
    (void)std::wprintf(format,Argument);
    // no warning is expected
    (void)std::wprintf(Format,Argument);
    (void)std::wprintf(Format);
}

void uninitvar_sprintf(void)
{
    char *s;
    const char *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::sprintf(s,format,i);
}

void uninitvar_swprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    // cppcheck-suppress uninitvar
    (void)std::swprintf(s,n,format);
}

void uninitvar_vsprintf(void)
{
    char *s;
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vsprintf(s,format,arg);
}

void uninitvar_vswprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vswprintf(s,n,format,arg);
}

void uninivar_fwprintf(void)
{
    FILE* stream;
    const wchar_t* format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fwprintf(stream,format,i);
}

void uninivar_snprintf(char *S, size_t N, char *Format, int Int)
{
    size_t n;
    char *format;
    int i;
    char *s;
    // cppcheck-suppress uninitvar
    (void)std::snprintf(s,n,format,i);
    // cppcheck-suppress uninitvar
    (void)std::snprintf(S,n,Format,Int); // n is uninitialized
    // cppcheck-suppress uninitvar
    (void)std::snprintf(S,N,format,Int); // format is uninitialized
    // cppcheck-suppress uninitvar
    (void)std::snprintf(S,N,Format,i); // i is uninitialized
    // TODO cppcheck-suppress uninitvar
    (void)std::snprintf(s,N,Format,Int);

    // no warning is expected for
    (void)std::snprintf(S,N,Format,Int);
}

void uninivar_vsnprintf(char *S, size_t N, char *Format, va_list Arg)
{
    char *s;
    size_t n;
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(s,n,format,arg);
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(s,N,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(S,n,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(S,N,format,Arg);

    // no warning is expected for
    (void)std::vsnprintf(S,N,Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vsnprintf(S,N,Format,arg);
}

void uninivar_wscanf(void)
{
    wchar_t *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::wscanf(format);
    // cppcheck-suppress uninitvar
    (void)std::wscanf(format,&i);
}

void uninivar_sscanf(void)
{
    char *string;
    const char * format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::sscanf(string,format);
    // cppcheck-suppress uninitvar
    (void)std::sscanf(string,format,&i);
}

void uninivar_fwscanf(void)
{
    FILE* stream;
    wchar_t* format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fwscanf(stream,format);
    // cppcheck-suppress uninitvar
    (void)std::fwscanf(stream,format,&i);
}

void uninivar_swscanf(void)
{
    wchar_t* s;
    wchar_t* format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::swscanf(s,format);
    // cppcheck-suppress uninitvar
    (void)std::swscanf(s,format,&i);
}

void uninitvar_system(void)
{
    char *c;
    // cppcheck-suppress uninitvar
    (void)std::system(c);
}

void uninitvar_setw(void)
{
    int i;
    // cppcheck-suppress uninitvar
    std::cout << std::setw(i);
}

void uninitvar_setiosflags(void)
{
    std::ios_base::fmtflags mask;
    // TODO cppcheck-suppress uninitvar
    std::cout << std::setiosflags(mask); // #6987 - false negative
}

void uninitvar_resetiosflags(void)
{
    std::ios_base::fmtflags mask;
    // TODO cppcheck-suppress uninitvar
    std::cout << std::resetiosflags(mask); // #6987 - false negative
}

void uninitvar_setfill(void)
{
    char c;
    // cppcheck-suppress uninitvar
    std::cout << std::setfill(c);

    wchar_t wc;
    // cppcheck-suppress uninitvar
    std::wcout << std::setfill(wc);
}

void uninitvar_setprecision(void)
{
    int p;
    // cppcheck-suppress uninitvar
    std::cout << std::setprecision(p);
}

void uninitvar_setbase(void)
{
    int p;
    // cppcheck-suppress uninitvar
    std::cout << std::setbase(p);
}

void uninitvar_find(std::string s)
{
    // testing of size_t find (const string& str, size_t pos = 0)
    size_t pos;
    // cppcheck-suppress uninitvar
    (void)s.find("find",pos); // #6991

    // testing of  size_t find (const char* s, size_t pos = 0) const;
    char *pc;
    // cppcheck-suppress uninitvar
    (void)s.find(pc,0);
    // cppcheck-suppress uninitvar
    (void)s.find(pc,pos);
    // cppcheck-suppress uninitvar
    (void)s.find("test",pos);

    // testing of size_t find (char c, size_t pos = 0) const;
    char c;
    // cppcheck-suppress uninitvar
    (void)s.find(c,pos);

    // testing of size_t find (const char* pc, size_t pos, size_t n) const;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)s.find(pc,pos,n); // #6991
    // cppcheck-suppress uninitvar
    (void)s.find("test",pos,n);
    // cppcheck-suppress uninitvar
    (void)s.find("test",1,n);
    // cppcheck-suppress uninitvar
    (void)s.find("test",pos,1);
    // cppcheck-suppress uninitvar
    (void)s.find(pc,1,1);
}

void uninivar_ifstream_read(std::ifstream &f)
{
    int size;
    char buffer[10];
    // cppcheck-suppress uninitvar
    f.read(buffer, size);
}

void uninivar_istream_read(std::istream &f)
{
    int size;
    char buffer[10];
    // cppcheck-suppress uninitvar
    f.read(buffer, size);
}

void uninitvar_string_compare(std::string &teststr, std::wstring &testwstr)
{
    char *pStrUninit;
    // cppcheck-suppress uninitvar
    (void)teststr.compare(pStrUninit);

    wchar_t *pWStrUninit;
    // cppcheck-suppress uninitvar
    (void)testwstr.compare(pWStrUninit);
}

void invalidFunctionArgBool_abs(bool b, double x, double y)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::abs(true); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::abs(b); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::abs(x<y); // #5635
}

void ignoredReturnValue_abs(int i)
{
    // cppcheck-suppress ignoredReturnValue
    std::abs(i);
    // cppcheck-suppress ignoredReturnValue
    std::abs(-199);
}

void ignoredReturnValue_string_compare(std::string teststr, std::wstring testwstr)
{
    // cppcheck-suppress ignoredReturnValue
    teststr.compare("test");
    // cppcheck-suppress ignoredReturnValue
    testwstr.compare(L"wtest");
}

void ignoredReturnValue_locale_global(const std::locale& loc)
{
    // no ignoredReturnValue shall be shown for
    std::locale::global(loc);
}

void ignoredReturnValue_make_pair()
{
    // cppcheck-suppress ignoredReturnValue
    std::make_pair(1, 2);
}

void nullPointer_ifstream_read(std::ifstream &f)
{
    // cppcheck-suppress nullPointer
    f.read(NULL, 10);
}

void nullPointer_istream_read(std::istream &f)
{
    // cppcheck-suppress nullPointer
    f.read(NULL, 10);
}

void nullPointer_asctime(void)
{
    struct tm *tm = 0;
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)std::asctime(tm);
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)std::asctime(0);
}

void nullPointer_wcsftime(size_t maxsize)
{
    wchar_t* ptr = 0;
    wchar_t* format = 0;
    struct tm* timeptr = 0;
    // cppcheck-suppress nullPointer
    (void)std::wcsftime(ptr,maxsize,format,timeptr);
    // cppcheck-suppress nullPointer
    (void)std::wcsftime(0,maxsize,0,0);
}

void nullPointer_fegetenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fegetenv(envp);
    // cppcheck-suppress nullPointer
    (void)std::fegetenv(0);
}

void nullPointer_fegetexceptflag(int excepts)
{
    fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fegetexceptflag(flagp,excepts);
    // cppcheck-suppress nullPointer
    (void)std::fegetexceptflag(0,excepts);
}

void nullPointer_feholdexcept(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::feholdexcept(envp);
    // cppcheck-suppress nullPointer
    (void)std::feholdexcept(0);
}

void nullPointer_fesetenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fesetenv(envp);
    // cppcheck-suppress nullPointer
    (void)std::fesetenv(0);
}

void nullPointer_fesetexceptflag(int excepts)
{
    fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fesetexceptflag(flagp,excepts);
    // cppcheck-suppress nullPointer
    (void)std::fesetexceptflag(0,excepts);
}

void nullPointer_feupdateenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::feupdateenv(envp);
    // cppcheck-suppress nullPointer
    (void)std::feupdateenv(0);
}

void nullPointer_atexit(void)
{
    // cppcheck-suppress nullPointer
    (void)std::atexit(0);
}

void nullPointer_atof(void)
{
    char * c = 0;
    // cppcheck-suppress nullPointer
    (void)std::atof(c);
    // cppcheck-suppress nullPointer
    (void)std::atof(0);
}

void nullPointer_memcmp(char *p)
{
    // cppcheck-suppress nullPointer
    (void)std::memcmp(p, 0, 123);
}

void nullPointer_wmemcmp(wchar_t *p)
{
    // cppcheck-suppress nullPointer
    (void)std::wmemcmp(p, 0, 123);
}

///////////////////////////////////////////////////////////////////////
//  <algorithm>
///////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <list>

#define pred    [](int i){return i==0;}


void stdalgorithm(const std::list<int> &ints1, const std::list<int> &ints2)
{
    // <!-- InputIterator std::find(InputIterator first, InputIterator last, T val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::find(ints1.begin(), ints2.end(), 123);
    // cppcheck-suppress mismatchingContainers
    if (std::find(ints1.begin(), ints1.end(), 123) == ints2.end()) {}

    // <!-- InputIterator std::find_if(InputIterator first, InputIterator last, UnaryPredicate val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::find_if(ints1.begin(), ints2.end(), pred);
    // cppcheck-suppress mismatchingContainers
    if (std::find_if(ints1.begin(), ints1.end(), pred) == ints2.end()) {}

    // <!-- InputIterator std::find_if_not(InputIterator first, InputIterator last, UnaryPredicate val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::find_if_not(ints1.begin(), ints2.end(), pred);
    // cppcheck-suppress mismatchingContainers
    if (std::find_if_not(ints1.begin(), ints1.end(), pred) == ints2.end()) {}

    // <!-- bool std::all_of(InputIterator first, InputIterator last, UnaryPredicate pred) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::all_of(ints1.begin(), ints2.end(), pred);

    // <!-- bool std::any_of(InputIterator first, InputIterator last, UnaryPredicate pred) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::any_of(ints1.begin(), ints2.end(), pred);

    // <!-- bool std::none_of(InputIterator first, InputIterator last, UnaryPredicate pred) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::none_of(ints1.begin(), ints2.end(), pred);

    // <!-- difference_type std::count(InputIterator first, InputIterator last, T val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::count(ints1.begin(), ints2.end(), 123);

    // <!-- difference_type std::count_if(InputIterator first, InputIterator last, UnaryPredicate val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::count_if(ints1.begin(), ints2.end(), pred);

    // <!-- Function std::for_each(InputIterator first, InputIterator last, Function func) -->
    // cppcheck-suppress mismatchingContainers
    std::for_each(ints1.begin(), ints2.end(), [](int i) {});

}


void getline()
{
    // #837
    std::ifstream in("test1.txt");

    char cBuf[10];
    // cppcheck-suppress bufferAccessOutOfBounds
    in.getline(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.read(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.readsome(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.get(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.get(cBuf, 100, 'a');
    // cppcheck-suppress bufferAccessOutOfBounds
    in.getline(cBuf, 100, 'a');

    in.close();
}

void stdstring()
{
    std::string s;
    // cppcheck-suppress ignoredReturnValue
    s.size();
}

void stdvector()
{
    int uninit;
    std::vector<int> v;
    // cppcheck-suppress ignoredReturnValue
    v.size();
    // cppcheck-suppress ignoredReturnValue
    v.capacity();
    // cppcheck-suppress uselessCallsEmpty
    // cppcheck-suppress ignoredReturnValue
    v.empty();
    // cppcheck-suppress ignoredReturnValue
    v.max_size();
    // cppcheck-suppress uninitvar
    v.push_back(uninit);
    // cppcheck-suppress uninitvar
    v.reserve(uninit);
    // cppcheck-suppress invalidFunctionArg
    v.reserve(-1);
    // no warning is expected for capacity 0 as it simply has no effect
    v.reserve(0);
    // cppcheck-suppress uninitvar
    v.resize(uninit);
    // cppcheck-suppress invalidFunctionArg
    v.resize(-1);

    v.clear();
    v.shrink_to_fit();

    // no warning is expected for pop_back()
    v.push_back(42);
    v.pop_back();

    v.push_back(42);
    // cppcheck-suppress ignoredReturnValue
    v.back();
    // cppcheck-suppress ignoredReturnValue
    v.front();
}
