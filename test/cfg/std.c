
// Test library configuration for std.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/std.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h> // frexp
#include <wchar.h>
#if  defined(__STD_UTF_16__) || defined(__STD_UTF_32__)
#include <uchar.h>
#endif
#include <ctype.h>
#include <wctype.h>
#include <fenv.h>
#include <setjmp.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

void bufferAccessOutOfBounds(void)
{
    char a[5];
    fgets(a,5,stdin);
    // cppcheck-suppress bufferAccessOutOfBounds
    fgets(a,6,stdin);
    sprintf(a, "ab%s", "cd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    sprintf(a, "ab%s", "cde");
    // TODO cppcheck-suppress redundantCopy
    snprintf(a, 5, "abcde%i", 1);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    snprintf(a, 6, "abcde%i", 1);
    // TODO cppcheck-suppress redundantCopy
    strcpy(a,"abcd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    strcpy(a, "abcde");
    // cppcheck-suppress bufferAccessOutOfBounds
    strcpy_s(a, 10, "abcdefghij");
    // TODO cppcheck-suppress redundantCopy
    strncpy(a,"abcde",5);
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    strncpy(a,"abcde",6);
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    strncpy(a,"a",6);
    // TODO cppcheck-suppress redundantCopy
    strncpy(a,"abcdefgh",4);
    // valid call
    strncpy_s(a,5,"abcd",5);
    // string will be truncated, error is returned, but no buffer overflow
    strncpy_s(a,5,"abcde",6);
    // TODO cppcheck-suppress bufferAccessOutOfBounds
    strncpy_s(a,5,"a",6);
    strncpy_s(a,5,"abcdefgh",4);
    // valid call
    strncat_s(a,5,"1",2);
    // cppcheck-suppress bufferAccessOutOfBounds
    strncat_s(a,10,"1",2);
    // TODO cppcheck-suppress bufferAccessOutOfBounds
    strncat_s(a,5,"1",5);
    fread(a,1,5,stdin);
    // cppcheck-suppress bufferAccessOutOfBounds
    fread(a,1,6,stdin);
    fwrite(a,1,5,stdout);
    // cppcheck-suppress bufferAccessOutOfBounds
    fread(a,1,6,stdout);
}

void bufferAccessOutOfBounds_libraryDirectionConfiguration(void)
{
    // This tests whether the argument to isdigit() is configured with direction "in". This allows
    // Cppcheck to report the error without marking it as inconclusive.
    char arr[10];
    char c = 'A';
    (void)isdigit(c);
    // cppcheck-suppress arrayIndexOutOfBounds
    // cppcheck-suppress unreadVariable
    arr[c] = 'x';
}

// memory leak

void ignoreleak(void)
{
    char *p = (char *)malloc(10);
    memset(&(p[0]), 0, 10);
    // TODO cppcheck-suppress memleak
}

// null pointer

void nullpointer(int value)
{
    int res = 0;
    FILE *fp;
    wchar_t *pWcsUninit;

#ifndef __CYGWIN__
    // cppcheck-suppress nullPointer
    clearerr(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    feof(0);
#endif
    // cppcheck-suppress nullPointer
    (void)fgetc(0);
    // cppcheck-suppress nullPointer
    fclose(0);
#ifndef __CYGWIN__
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    ferror(0);
#endif
    // cppcheck-suppress nullPointer
    (void)ftell(0);
    // cppcheck-suppress nullPointer
    puts(0);
    // cppcheck-suppress nullPointer
    fp=fopen(0,0);
    fclose(fp);
    fp = 0;
    // No FP
    fflush(0);
    fp = freopen(0,"abc",stdin);
    fclose(fp);
    fp = NULL;
    // cppcheck-suppress nullPointer
    fputc(0,0);
    // cppcheck-suppress nullPointer
    fputs(0,0);
    // cppcheck-suppress nullPointer
    fgetpos(0,0);
    // cppcheck-suppress nullPointer
    frexp(1.0,0);
    // cppcheck-suppress nullPointer
    fsetpos(0,0);
    // cppcheck-suppress nullPointer
    itoa(123,0,10);
    putchar(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strchr(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcschr(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strlen(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcslen(0);
    // cppcheck-suppress nullPointer
    strcpy(0,0);
    // cppcheck-suppress nullPointer
    wcscpy(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcsspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcscspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcoll(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcscoll(0,0);
    // cppcheck-suppress nullPointer
    strcat(0,0);
    // cppcheck-suppress nullPointer
    wcscat(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcmp(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcscmp(0,0);
    // cppcheck-suppress nullPointer
    strcpy_s(0,1,1);
    // cppcheck-suppress nullPointer
    strcpy_s(1,1,0);
    // cppcheck-suppress nullPointer
    strncpy(0,0,1);
    // cppcheck-suppress nullPointer
    strncpy_s(0,1,1,1);
    // cppcheck-suppress nullPointer
    strncpy_s(1,1,0,1);
    // cppcheck-suppress nullPointer
    wcsncpy(0,0,1);
    // cppcheck-suppress nullPointer
    strncat(0,0,1);
    // cppcheck-suppress nullPointer
    strncat_s(0,1,1,1);
    // cppcheck-suppress nullPointer
    strncat_s(1,1,0,1);
    // cppcheck-suppress nullPointer
    wcsncat(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strncmp(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcsncmp(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strstr(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcsstr(0,0);
    // cppcheck-suppress nullPointer
    strtoul(0,0,0);
    // cppcheck-suppress nullPointer
    wcstoul(0,0,0);
    // cppcheck-suppress nullPointer
    strtoull(0,0,0);
    // cppcheck-suppress nullPointer
    wcstoull(0,0,0);
    // cppcheck-suppress nullPointer
    strtol(0,0,0);
    // cppcheck-suppress nullPointer
    wcstol(0,0,0);

    // #6100 False positive nullPointer - calling mbstowcs(NULL,)
    res += mbstowcs(0,"",0);
    res += wcstombs(0,L"",0);

    strtok(NULL,"xyz");
    wcstok(NULL,L"xyz",&pWcsUninit);

    strxfrm(0,"foo",0);
    // TODO: error message (#6306 and http://trac.cppcheck.net/changeset/d11eb4931aea51cf2cb74faccdcd2a3289b818d6/)
    strxfrm(0,"foo",42);
    wcsxfrm(0,L"foo",0);
    // TODO: error message when arg1==NULL and arg3!=0 #6306: https://trac.cppcheck.net/ticket/6306#comment:2
    wcsxfrm(0,L"foo",42);

    snprintf(NULL, 0, "someformatstring"); // legal
    // cppcheck-suppress nullPointer
    snprintf(NULL, 42, "someformatstring"); // not legal

    scanf("%i", &res);
    // cppcheck-suppress nullPointer
    scanf("%i", NULL);
    wscanf(L"%i", &res);
    // cppcheck-suppress nullPointer
    wscanf(L"%i", NULL);
}

void nullpointerMemchr1(char *p, char *s)
{
    p = memchr(s, 'p', strlen(s));
    (void)p;
}

void nullpointerMemchr2(char *p, char *s)
{
    p = memchr(s, 0, strlen(s));
    (void)p;
}

void nullPointer_memchr(char *p)
{
    char *s = 0;
    // cppcheck-suppress nullPointer
    p = memchr(s, 0, strlen(s));
    (void)p;
}

void nullPointer_memcmp(char *p)
{
    // cppcheck-suppress nullPointer
    (void)memcmp(p, 0, 123);
}

void nullPointer_wmemcmp(wchar_t *p)
{
    // cppcheck-suppress nullPointer
    (void)wmemcmp(p, 0, 123);
}

// uninit pointers

void uninitvar_abs(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)abs(i);
}

void uninitvar_clearerr(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    clearerr(fp);
}

void uninitvar_fclose(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    fclose(fp);
}

void uninitvar_fopen(void)
{
    const char *filename, *mode;
    FILE *fp;
    // cppcheck-suppress uninitvar
    fp = fopen(filename, "rt");
    fclose(fp);
    // cppcheck-suppress uninitvar
    fp = fopen("filename.txt", mode);
    fclose(fp);
}

void uninitvar_feof(void)
{
    FILE *fp;
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress uninitvar
    feof(fp);

    // cppcheck-suppress uninitvar
    (void)feof(fp);
}

void uninitvar_ferror(void)
{
    FILE *fp;
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress uninitvar
    ferror(fp);

    // cppcheck-suppress uninitvar
    (void)ferror(fp);
}

void uninitvar_fflush(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    fflush(fp);
}

void uninitvar_fgetc(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    (void)fgetc(fp);
}

void uninitvar_fgetpos(void)
{
    FILE *fp;
    fpos_t pos;
    fpos_t *ppos;
    // cppcheck-suppress uninitvar
    fgetpos(fp,&pos);

    fp = fopen("filename","rt");
    // cppcheck-suppress uninitvar
    fgetpos(fp,ppos);
    fclose(fp);
}

void uninitvar_fsetpos(void)
{
    FILE *fp;
    fpos_t pos;
    fpos_t *ppos;
    // cppcheck-suppress uninitvar
    fsetpos(fp,&pos);

    fp = fopen("filename","rt");
    // cppcheck-suppress uninitvar
    fsetpos(fp,ppos);
    fclose(fp);
}

void uninitvar_fgets(void)
{
    FILE *fp;
    char buf[10];
    char *str;
    int n;

    fgets(buf,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(str,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(buf,10,fp);

    // cppcheck-suppress uninitvar
    fgets(buf,n,stdin);
}

void uninitvar_fputc(void)
{
    int i;
    FILE *fp;

    fputc('a', stdout);

    // cppcheck-suppress uninitvar
    fputc(i, stdout);

    // cppcheck-suppress uninitvar
    fputc('a', fp);
}

void uninitvar_fputs(void)
{
    const char *s;
    FILE *fp;

    fputs("a", stdout);

    // cppcheck-suppress uninitvar
    fputs(s, stdout);

    // cppcheck-suppress uninitvar
    fputs("a", fp);
}

void uninitvar_ftell(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    (void)ftell(fp);
}

void uninitvar_puts(void)
{
    const char *s;
    // cppcheck-suppress uninitvar
    puts(s);
}

void uninitvar_putchar(void)
{
    char c;
    // cppcheck-suppress uninitvar
    putchar(c);
}

void uninitvar_cproj(void) // #6939
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)cprojf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cproj(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cprojl(ldc);
}

void uninitvar_creal(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)crealf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)creal(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)creall(ldc);
}

void uninitvar_acos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)acosf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)acos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)acosl(ld);
}

void uninitvar_acosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)acoshf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)acosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)acoshl(ld);
}

void uninitvar_asctime(void)
{
    const struct tm *tm;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress asctimeCalled
    (void)asctime(tm);
}

void uninitvar_asctime_s(void)
{
    const struct tm *tm;
    char buf[26];
    // cppcheck-suppress uninitvar
    // cppcheck-suppress asctime_sCalled
    asctime_s(buf, sizeof(buf), tm);
}

void uninitvar_assert(void)
{
    int i;
    // cppcheck-suppress checkLibraryNoReturn
    // cppcheck-suppress uninitvar
    assert(i);
}

void uninitvar_sqrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)sqrtf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)sqrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)sqrtl(ld);
}

void uninitvar_csqrt(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)csqrtf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)csqrt(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)csqrtl(ldc);
}

void uninitvar_sinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)sinhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)sinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)sinhl(ld);
}

void uninitvar_sin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)sinf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)sin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)sinl(ld);
}

void uninitvar_csin(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)csinf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)csin(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)csinl(ldc);
}

void uninitvar_csinh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)csinhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)csinh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)csinhl(ldc);
}

void uninitvar_asin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)asinf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)asin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)asinl(ld);
}

void uninitvar_casin(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)casinf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)casin(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)casinl(ldc);
}

void uninitvar_asinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)asinhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)asinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)asinhl(ld);
}

void uninitvar_casinh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)casinhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)casinh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)casinhl(ldc);
}

void uninitvar_wcsftime(wchar_t* ptr)
{
    size_t maxsize;
    wchar_t* format;
    struct tm* timeptr;
    // cppcheck-suppress uninitvar
    (void)wcsftime(ptr, maxsize, format, timeptr);
}

void uninitvar_tan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)tanf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)tan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)tanl(ld);
}

void uninitvar_ctan(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ctanf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ctan(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ctanl(ldc);
}

void uninitvar_tanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)tanhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)tanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)tanhl(ld);
}

void uninitvar_ctanh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ctanhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ctanh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ctanhl(ldc);
}

void uninitvar_feclearexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)feclearexcept(i);
}

void uninitvar_fegetexceptflag(fexcept_t* flagp)
{
    int excepts;
    // cppcheck-suppress uninitvar
    (void)fegetexceptflag(flagp, excepts);
}

void uninitvar_feraiseexcept(void)
{
    int excepts;
    // cppcheck-suppress uninitvar
    (void)feraiseexcept(excepts);
}

void uninitvar_fesetenv(void)
{
    fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)fesetenv(envp);
}

void uninitvar_fesetround(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)fesetround(i);
}

void uninitvar_fetestexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)fetestexcept(i);
}

void uninitvar_feupdateenv(void)
{
    fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)feupdateenv(envp);
}

void uninitvar_atan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)atanf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)atan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)atanl(ld);
}

void uninitvar_catan(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)catanf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)catan(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)catanl(ldc);
}

void uninitvar_tgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)tgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)tgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)tgammal(ld);
}

void uninitvar_trunc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)truncf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)trunc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)truncl(ld);
}

void uninitvar_atanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)atanhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)atanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)atanhl(ld);
}

void uninitvar_catanh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)catanhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)catanh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)catanhl(ldc);
}

void uninitvar_atan2(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)atan2f(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)atan2(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)atan2l(ld1,ld2);
}

void uninitvar_atof(void)
{
    char * c;
    // cppcheck-suppress uninitvar
    (void)atof(c);
}

void uninitvar_atol(void)
{
    char * c;
    // cppcheck-suppress uninitvar
    (void)atoi(c);

    // cppcheck-suppress uninitvar
    (void)atol(c);

    // cppcheck-suppress uninitvar
    (void)atoll(c);
}

void uninitvar_calloc(void)
{
    size_t nitems;
    size_t size;
    // cppcheck-suppress uninitvar
    int * p = (int*) calloc(nitems, size);
    free(p);
}

void uninitvar_ceil(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)ceilf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)ceil(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)ceill(ld);
}

void uninitvar_copysign(void)
{
    float f1, f2;
    // cppcheck-suppress uninitvar
    (void)copysignf(f1, f2);

    double d1, d2;
    // cppcheck-suppress uninitvar
    (void)copysign(d1, d2);

    long double ld1, ld2;
    // cppcheck-suppress uninitvar
    (void)copysignl(ld1, ld2);
}

void uninitvar_cbrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)cbrtf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)cbrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)cbrtl(ld);
}

void uninitvar_cos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)cosf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)cos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)cosl(ld);
}

void uninitvar_ccos(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ccosf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ccos(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ccosl(ldc);
}

void uninitvar_cosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)coshf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)cosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)coshl(ld);
}

void uninitvar_ccosh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ccoshf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ccosh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ccoshl(ldc);
}

void uninitvar_ctime(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)ctime(tp);
}

void uninitvar_difftime(void)
{
    time_t t1,t2;
    // cppcheck-suppress uninitvar
    (void)difftime(t1, t2);
}

void uninitvar_div(void)
{
    int num;
    int denom;
    // cppcheck-suppress uninitvar
    (void)div(num,denom);
}

void uninitvar_exit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    exit(i);
}

void uninitvar_erf(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)erff(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)erf(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)erfl(ld);
}

void uninitvar_erfc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)erfcf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)erfc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)erfcl(ld);
}

void uninitvar_carg(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cargf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)carg(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cargl(ldc);
}

void uninitvar_exp(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)expf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)exp(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)expl(ld);
}

void uninitvar_cexp(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cexpf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cexp(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cexpl(ldc);
}

void uninitvar_cimag(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cimagf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cimag(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cimagl(ldc);
}

void uninitvar_exp2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)exp2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)exp2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)exp2l(ld);
}

void uninitvar_expm1(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)expm1f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)expm1(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)expm1l(ld);
}

void uninitvar_fabs(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)fabsf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)fabs(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)fabsl(ld);
}

void uninitvar_fdim(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fdimf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fdim(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fdiml(ld1,ld2);
}

void uninitvar_fgetwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fgetwc(stream);
}

void uninitvar_floor(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)floorf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)floor(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)floorl(ld);
}

void uninitvar_fma(void)
{
    float f1,f2,f3;
    // cppcheck-suppress uninitvar
    (void)fmaf(f1,f2,f3);

    double d1,d2,d3;
    // cppcheck-suppress uninitvar
    (void)fma(d1,d2,d3);

    long double ld1,ld2,ld3;
    // cppcheck-suppress uninitvar
    (void)fmal(ld1,ld2,ld3);
}

void uninitvar_fmax(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fmaxf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fmax(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fmaxl(ld1,ld2);
}

void uninitvar_fmin(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fminf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fmin(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fminl(ld1,ld2);
}

void uninitvar_fmod(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fmodf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fmod(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fmodl(ld1,ld2);
}

void uninitvar_fprintf(FILE *Stream, char *Format, int Argument)
{
    FILE *stream;
    char *format1, *format2;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)fprintf(stream, format1, argument1);
    // cppcheck-suppress uninitvar
    (void)fprintf(stream, Format, Argument);
    // cppcheck-suppress uninitvar
    (void)fprintf(Stream, format2, Argument);
    // cppcheck-suppress uninitvar
    (void)fprintf(Stream, Format, argument2);

    // no warning is expected
    (void)fprintf(Stream, Format, Argument);
}

void uninitvar_vfprintf(FILE *Stream, const char *Format, va_list Arg)
{
    FILE *stream;
    char *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfprintf(stream, format1, arg);
    // cppcheck-suppress uninitvar
    (void)vfprintf(stream, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)vfprintf(Stream, format2, Arg);

    // no warning is expected
    (void)vfprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vfprintf(Stream, Format, arg);
}

void uninitvar_vfwprintf(FILE *Stream, wchar_t *Format, va_list Arg)
{
    FILE *stream;
    wchar_t *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfwprintf(stream, format1, arg);
    // cppcheck-suppress uninitvar
    (void)vfwprintf(stream, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)vfwprintf(Stream, format2, Arg);

    // no warning is expected
    (void)vfwprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vfwprintf(Stream, Format, arg);
}

void uninitvar_fputwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fputwc(c,stream);
}

void uninitvar_fputws(void)
{
    wchar_t *string;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fputws(string,stream);
}

void uninitvar_fread(void)
{
    void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fread(ptr,size,nobj,stream);
}

void uninitvar_free(void)
{
    // cppcheck-suppress unassignedVariable
    void *block;
    // cppcheck-suppress uninitvar
    free(block);
}

void uninitvar_freopen(void)
{
    char *filename;
    char *mode;
    FILE *stream;
    // cppcheck-suppress uninitvar
    FILE * p = freopen(filename,mode,stream);
    free(p);
}

void uninitvar_frexp(void)
{
    float f1;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)frexpf(f1,i1);

    double d1;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)frexp(d1,i2);

    long double ld1;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)frexpl(ld1,i3);
}

void uninitvar_hypot(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)hypotf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)hypot(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)hypotl(ld1,ld2);
}

void uninitvar_fscanf(void)
{
    FILE *stream;
    char *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)fscanf(stream,format,i);
}

void uninitvar_vfscanf(void)
{
    FILE *stream;
    char * format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfscanf(stream,format,arg);
}

void uninitvar_vfwscanf(void)
{
    FILE *stream;
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfwscanf(stream,format,arg);
}

void uninitvar_fseek(void)
{
    FILE* stream;
    long int offset;
    int origin;
    // cppcheck-suppress uninitvar
    (void)fseek(stream,offset,origin);
}

void uninitvar_fgetws(void)
{
    wchar_t *buffer;
    int n;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fgetws(buffer,n,stream);
}

void uninitvar_fwide(void)
{
    FILE *stream;
    int mode;
    // cppcheck-suppress uninitvar
    (void)fwide(stream,mode);
}

void uninitvar_fwrite(void)
{
    void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fwrite(ptr,size,nobj,stream);
}

void uninitvar_mblen(void)
{
    char *string;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)mblen(string,size);
}

void uninitvar_mbtowc(void)
{
    wchar_t* pwc;
    char* pmb;
    size_t max;
    // cppcheck-suppress uninitvar
    (void)mbtowc(pwc,pmb,max);
}

void uninitvar_mbrlen(const char* p, size_t m, mbstate_t* s)
{
    char* pmb;
    size_t max1, max2;
    mbstate_t* ps1, *ps2;
    // cppcheck-suppress uninitvar
    (void)mbrlen(pmb,max1,ps1);
    // cppcheck-suppress uninitvar
    (void)mbrlen(pmb,m,s);
    // cppcheck-suppress uninitvar
    (void)mbrlen(p,max2,s);
    // cppcheck-suppress uninitvar
    (void)mbrlen(p,m,ps2);
    // no warning is expected
    (void)mbrlen(p,m,s);
}

void uninitvar_btowc(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)btowc(c);
}

void uninitvar_mbsinit(void)
{
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)mbsinit(ps);
}

void uninitvar_mbstowcs(void)
{
    wchar_t *ws;
    char *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)mbstowcs(ws,s,n);
}

void uninitvar_mbsrtowcs(void)
{
    wchar_t* dest;
    const char* src;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)mbsrtowcs(dest,&src,max,ps);
}

void uninitvar_wctob(void)
{
    wint_t wc;
    // cppcheck-suppress uninitvar
    (void)wctob(wc);
}

void uninitvar_wctomb(void)
{
    char *s;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)wctomb(s,wc);
}

void uninitvar_wcstombs(void)
{
    char *mbstr;
    wchar_t *wcstr;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wcstombs(mbstr,wcstr,n);
}

void uninitvar_getc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)getc(stream);
}

void uninitvar_getwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)getwc(stream);
}

void uninitvar_ungetc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)ungetc(c,stream);
}

void uninitvar_ungetwc(void)
{
    wint_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)ungetwc(c,stream);
}

void uninitvar_getenv(void)
{
    char *name;
    // cppcheck-suppress uninitvar
    (void)getenv(name);
}

void uninitvar_gets(void)
{
    char *buffer;
    // cppcheck-suppress getsCalled
    // cppcheck-suppress uninitvar
    (void)gets(buffer);
}

void uninitvar_gmtime(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)gmtime(tp);
}

void uninitvar_isalnum(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isalnum(i);
}

void uninitvar_iswalnum(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswalnum(i);
}

void uninitvar_isalpha(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isalpha(i);
}

void uninitvar_iswalpha(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswalpha(i);
}

void uninitvar_isblank(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isblank(i);
}

void uninitvar_iswblank(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswblank(i);
}

void uninitvar_iscntrl(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)iscntrl(i);
}

void uninitvar_iswcntrl(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswcntrl(i);
}

void uninitvar_iswctype(void)
{
    wint_t c;
    wctype_t desc;
    // cppcheck-suppress uninitvar
    (void)iswctype(c,desc);
}

void uninitvar_isdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isdigit(i);
}

void uninitvar_iswdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswdigit(i);
}

void uninitvar_isgraph(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isgraph(i);
}

void uninitvar_iswgraph(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswgraph(i);
}

void uninitvar_islower(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)islower(i);
}

void uninitvar_iswlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswlower(i);
}

void uninitvar_isprint(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isprint(i);
}

void uninitvar_iswprint(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswprint(i);
}

void uninitvar_ispunct(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)ispunct(i);
}

void uninitvar_iswpunct(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswpunct(i);
}

void uninitvar_isspace(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isspace(i);
}

void uninitvar_iswspace(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswspace(i);
}

void uninitvar_isupper(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isupper(i);
}

void uninitvar_iswupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswupper(i);
}

void uninitvar_isxdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isxdigit(i);
}

void uninitvar_iswxdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswxdigit(i);
}

void uninitvar_towctrans(void)
{
    wint_t c;
    wctrans_t desc;
    // cppcheck-suppress uninitvar
    (void)towctrans(c,desc);
}

void uninitvar_towlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)towlower(i);
}

void uninitvar_towupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)towupper(i);
}

void uninitvar_wctrans(void)
{
    char* property;
    // cppcheck-suppress uninitvar
    (void)wctrans(property);
}

void uninitvar_wctype(void)
{
    char* property;
    // cppcheck-suppress uninitvar
    (void)wctype(property);
}

void ignorereturn(void)
{
    char szNumbers[] = "2001 60c0c0 -1101110100110100100000 0x6fffff";
    char * pEnd;
    strtol(szNumbers,&pEnd,10);
}

void uninitvar_cabs(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cabsf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cabs(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cabsl(ldc);
}

void uninitvar_cacos(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cacosf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cacos(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cacosl(ldc);
}

void uninitvar_cacosh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cacoshf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cacosh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cacoshl(ldc);
}

void uninitvar_labs(void)
{
    long int li;
    // cppcheck-suppress uninitvar
    (void)labs(li);

    long long int lli;
    // cppcheck-suppress uninitvar
    (void)llabs(lli);
}

void uninitvar_ldexp(void)
{
    float f;
    int e1;
    // cppcheck-suppress uninitvar
    (void)ldexpf(f,e1);

    double d;
    int e2;
    // cppcheck-suppress uninitvar
    (void)ldexp(d,e2);

    long double ld;
    int e3;
    // cppcheck-suppress uninitvar
    (void)ldexpl(ld,e3);
}

void uninitvar_lgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)lgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)lgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)lgammal(ld);
}

void uninitvar_rint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)rintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)rint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)rintl(ld);
}

void uninitvar_lrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)lrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)lrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)lrintl(ld);
}

void uninitvar_llrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)llrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)llrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)llrintl(ld);
}

void uninitvar_lround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)lroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)lround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)lroundl(ld);
}

void uninitvar_llround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)llroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)llround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)llroundl(ld);
}

void uninitvar_srand(void)
{
    unsigned int seed;
    // cppcheck-suppress uninitvar
    (void)srand(seed);
}

void uninitvar_ldiv(void)
{
    long int l1;
    long int l2;
    // cppcheck-suppress uninitvar
    (void)ldiv(l1,l2);

    long long int ll1;
    long long int ll2;
    // cppcheck-suppress uninitvar
    (void)lldiv(ll1,ll2);
}

void uninitvar_localtime(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)localtime(tp);
}

void uninitvar_log(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)logf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)logl(ld);
}

void uninitvar_clog(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)clogf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)clog(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)clogl(ldc);
}

void uninitvar_conj(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)conjf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)conj(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)conjl(ldc);
}

void uninitvar_fpclassify(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)fpclassify(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)fpclassify(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)fpclassify(ld);
}

void uninitvar_isfinite(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)isfinite(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)isfinite(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)isfinite(ld);
}

void uninitvar_isgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)isgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)isgreater(ld1,ld2);
}

void uninitvar_isgreaterequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)isgreaterequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isgreaterequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)isgreaterequal(ld1,ld2);
}

void uninitvar_isinf(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)isinf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)isinf(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)isinf(ld);
}

void uninitvar_logb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)logbf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)logb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)logbl(ld);
}

void uninitvar_isless(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)isless(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isless(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)isless(ld1,ld2);
}

void uninitvar_islessequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)islessequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)islessequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)islessequal(ld1,ld2);
}

void uninitvar_islessgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)islessgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)islessgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)islessgreater(ld1,ld2);
}

void uninitvar_nan(void)
{
    char *tagp;
    // cppcheck-suppress uninitvar
    (void)nanf(tagp);
    // cppcheck-suppress uninitvar
    (void)nan(tagp);
    // cppcheck-suppress uninitvar
    (void)nanl(tagp);
}

void uninitvar_isnan(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)isnan(d);
}

void uninitvar_isnormal(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)isnormal(d);
}

void uninitvar_isunordered(void)
{
    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isunordered(d1,d2);
}

void uninitvar_ilogb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)ilogbf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)ilogb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)ilogbl(ld);
}

void uninitvar_log10(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)log10f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log10(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)log10l(ld);
}

void uninitvar_log1p(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)log1pf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log1p(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)log1pl(ld);
}

void uninitvar_log2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)log2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)log2l(ld);
}

void uninitvar_nearbyint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)nearbyintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)nearbyint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)nearbyintl(ld);
}

void uninitvar_nextafter(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)nextafterf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)nextafter(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)nextafterl(ld1,ld2);
}

void uninitvar_nexttoward(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)nexttowardf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)nexttoward(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)nexttowardl(ld1,ld2);
}

void uninitvar_longjmp(void)
{
    jmp_buf env;
    int val;
    // cppcheck-suppress uninitvar
    (void)longjmp(env,val);
}

void uninitvar_malloc(void)
{
    size_t size;
    // cppcheck-suppress uninitvar
    int *p = (int*)malloc(size);
    free(p);
}

void uninitvar_alloca(void)
{
    size_t size;
    // cppcheck-suppress allocaCalled
    // cppcheck-suppress uninitvar
    (void)alloca(size);
}

void uninitvar_memchr(void)
{
    void *cs;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memchr(cs,c,n);
}

void uninitvar_wmemchr(void)
{
    wchar_t *cs;
    wchar_t c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemchr(cs,c,n);
}

void uninitvar_memcmp(void)
{
    void *s1;
    void *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memcmp(s1,s2,n);
}

void uninitvar_wmemcmp(void)
{
    wchar_t *s1;
    wchar_t *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemcmp(s1,s2,n);
}

void uninitvar_memcpy(void)
{
    void *ct;
    void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memcpy(ct,cs,n);
}

void uninitvar_wmemcpy(void)
{
    wchar_t *cs;
    wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemcpy(cs,c,n);
}

void uninitvar_memmove(void)
{
    void *ct;
    void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memmove(ct,cs,n);
}

void uninitvar_wmemmove(void)
{
    wchar_t *cs;
    wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemmove(cs,c,n);
}

void uninitvar_memset(void)
{
    void *s;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memset(s,c,n);
}

void uninitvar_wmemset(void)
{
    wchar_t *cs;
    wchar_t  c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemset(cs,c,n);
}

void uninitvar_mktime(void)
{
    struct tm *tp;
    // cppcheck-suppress uninitvar
    (void)mktime(tp);

    struct tmx *tpx;
    // cppcheck-suppress uninitvar
    (void)mkxtime(tpx);
}

void uninitvar_modf(void)
{
    float f1;
    float *f2;
    // cppcheck-suppress uninitvar
    (void)modff(f1,f2);

    double d1;
    double *d2;
    // cppcheck-suppress uninitvar
    (void)modf(d1,d2);

    long double ld1;
    long double *ld2;
    // cppcheck-suppress uninitvar
    (void)modfl(ld1,ld2);
}

void uninitvar_perror(void)
{
    char *string;
    // cppcheck-suppress uninitvar
    (void)perror(string);
}

void uninitvar_pow(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)powf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)pow(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)powl(ld1,ld2);
}

void uninitvar_cpow(void)
{
    float complex f1,f2;
    // cppcheck-suppress uninitvar
    (void)cpowf(f1,f2);

    double complex d1,d2;
    // cppcheck-suppress uninitvar
    (void)cpow(d1,d2);

    long double complex ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)cpowl(ld1,ld2);
}

void uninitvar_remainder(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)remainderf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)remainder(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)remainderl(ld1,ld2);
}

void uninitvar_remquo(void)
{
    float f1,f2;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)remquof(f1,f2,i1);

    double d1,d2;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)remquo(d1,d2,i2);

    long double ld1,ld2;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)remquol(ld1,ld2,i3);
}

void uninitvar_printf(char *Format, int Argument)
{
    char * format_1, * format_2, * format_3;
    int argument1, argument2;
    // no warning is expected
    (void)printf("x");
    // cppcheck-suppress uninitvar
    (void)printf(format_1,argument1);
    // cppcheck-suppress uninitvar
    (void)printf(Format,argument2);
    // cppcheck-suppress uninitvar
    (void)printf(format_2,Argument);
    // cppcheck-suppress uninitvar
    (void)printf(format_3,1);

    // no warning is expected
    (void)printf(Format,Argument);
}

void uninitvar_vprintf(char *Format, va_list Arg)
{
    char * format1, *format2;
    va_list arg1, arg2;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vprintf(format1,arg1);
    // cppcheck-suppress uninitvar
    (void)vprintf(format2,Arg);

    // no warning is expected
    (void)vprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vprintf(Format,arg2);
}

void uninitvar_vwprintf(wchar_t *Format, va_list Arg)
{
    wchar_t * format1, * format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vwprintf(format1,arg);
    // cppcheck-suppress uninitvar
    (void)vwprintf(format2,Arg);

    // no warning is expected
    (void)vwprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vwprintf(Format,arg);
}

void uninitvar_bsearch(void)
{
    void* key;
    void* base;
    size_t num;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)bsearch(key,base,num,size,(int(*)(const void*,const void*)) strcmp);
}

void uninitvar_qsort(void)
{
    void *base;
    size_t n;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)qsort(base,n,size, (int(*)(const void*,const void*)) strcmp);
}

void uninitvar_putc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)putc(c,stream);
}

void uninitvar_putwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)putc(c,stream);
}

void uninitvar_putwchar(void)
{
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)putwchar(c);
}

void uninitvar_realloc(void)
{
    void *block;
    size_t newsize;
    // cppcheck-suppress uninitvar
    void *p = realloc(block, newsize);
    free(p);
}

void uninitvar_remove(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)remove(s);
}

void uninitvar_rename(void)
{
    char *s1;
    char *s2;
    // cppcheck-suppress uninitvar
    (void)rename(s1,s2);
}

void uninitvar_rewind(void)
{
    FILE *f;
    // cppcheck-suppress uninitvar
    (void)rewind(f);
}

void uninitvar_round(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)roundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)round(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)roundl(ld);
}

void uninitvar_scalbn(void)
{
    float f;
    int i1;
    // cppcheck-suppress uninitvar
    (void)scalbnf(f,i1);

    double d;
    int i2;
    // cppcheck-suppress uninitvar
    (void)scalbn(d,i2);

    long double ld;
    int i3;
    // cppcheck-suppress uninitvar
    (void)scalbnl(ld,i3);
}

void uninitvar_scalbln(void)
{
    float f;
    long int i1;
    // cppcheck-suppress uninitvar
    (void)scalblnf(f,i1);

    double d;
    long int i2;
    // cppcheck-suppress uninitvar
    (void)scalbln(d,i2);

    long double ld;
    long int i3;
    // cppcheck-suppress uninitvar
    (void)scalblnl(ld,i3);
}

void uninitvar_signbit(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)signbit(d);
}

void uninitvar_signal(void)
{
    int i;
    // cppcheck-suppress uninitvar
    signal(i, exit);
}

void uninitvar_raise(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)raise(i);
}

void uninitvar_scanf(void)
{
    char *format;
    char str[42];
    // cppcheck-suppress uninitvar
    (void)scanf(format, str);
}

void uninitvar_vsscanf(void)
{
    char *s;
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vsscanf(s,format,arg);
}

void uninitvar_vswscanf(void)
{
    wchar_t *s;
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vswscanf(s,format,arg);
}

void uninitvar_vscanf(void)
{
    char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vscanf(format,arg);
}

void uninitvar_vwscanf(void)
{
    wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vwscanf(format,arg);
}

void uninitvar_setbuf(void)
{
    FILE *stream;
    char *buf;
    // cppcheck-suppress uninitvar
    (void)setbuf(stream,buf);
}

void uninitvar_setvbuf(void)
{
    FILE *stream;
    char *buf;
    int mode;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)setvbuf(stream,buf,mode,size);
}

void uninitvar_strcat(char *dest, const char * const source)
{
    char *deststr1, *deststr2;
    char *srcstr;
    // cppcheck-suppress uninitvar
    (void)strcat(deststr1,srcstr);
    // cppcheck-suppress uninitvar
    (void)strcat(dest,srcstr);
    // cppcheck-suppress uninitvar
    (void)strcat(deststr2,source);

    // no warning shall be shown for
    (void)strcat(dest,source);
}

void bufferAccessOutOfBounds_strcat(char *dest, const char * const source)
{
    char buf4[4] = {0};
    const char * const srcstr3 = "123";
    const char * const srcstr4 = "1234";
    // @todo #8599 cppcheck-suppress bufferAccessOutOfBounds
    (void)strcat(buf4,srcstr4); // off by one issue: strcat is appends \0' at the end

    // no warning shall be shown for
    (void)strcat(dest,source);
    (void)strcat(buf4,srcstr3); // strcat appends '\0' at the end
    (void)strcat(dest,srcstr4); // Cppcheck does not know the length of 'dest'
}

void uninitvar_wcscat(wchar_t *dest, const wchar_t * const source)
{
    wchar_t *deststr_1, *deststr_2;
    wchar_t *srcstr_1, *srcstr_2;
    // cppcheck-suppress uninitvar
    (void)wcscat(deststr_1,srcstr_1);
    // cppcheck-suppress uninitvar
    (void)wcscat(dest,srcstr_2);
    // cppcheck-suppress uninitvar
    (void)wcscat(deststr_2,source);

    // no warning shall be shown for
    (void)wcscat(dest,source);
}

void uninitvar_wcrtomb(void)
{
    char *s;
    wchar_t wc;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)wcrtomb(s,wc,ps);
}

void uninitvar_strchr(void)
{
    char *cs;
    int c;
    // cppcheck-suppress uninitvar
    (void)strchr(cs,c);
}

void invalidFunctionArg_strchr(char *cs, int c)
{
    // cppcheck-suppress invalidFunctionArg
    (void)strchr(cs,-1);

    // No warning shall be issued for
    (void)strchr(cs, 0);
    (void)strchr(cs, 255);

    // cppcheck-suppress invalidFunctionArg
    (void)strchr(cs, 256);
}

void uninitvar_wcschr(void)
{
    wchar_t *cs;
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)wcschr(cs,c);
}

void uninitvar_strcmp(void)
{
    char *str1;
    char *str2;
    // cppcheck-suppress uninitvar
    (void)strcmp(str1,str2);
}

void uninitvar_wcscmp(void)
{
    wchar_t *str1;
    wchar_t *str2;
    // cppcheck-suppress uninitvar
    (void)wcscmp(str1,str2);
}

void uninitvar_strcpy(void)
{
    char *str1;
    char *str2;
    // cppcheck-suppress uninitvar
    (void)strcpy(str1,str2);
}

void uninitvar_strcpy_s(char * strDest)
{
    char *strUninit1;
    char *strUninit2;
    // cppcheck-suppress uninitvar
    (void)strcpy_s(strUninit1, 1, "a");
    // cppcheck-suppress uninitvar
    (void)strcpy_s(strDest, 1, strUninit2);
}

void uninitvar_wcscpy(void)
{
    wchar_t *str1;
    wchar_t *str2;
    // cppcheck-suppress uninitvar
    (void)wcscpy(str1,str2);
}

void uninitvar_strftime(void)
{
    char *s;
    size_t max;
    char *fmt;
    struct tm *p;
    // cppcheck-suppress uninitvar
    (void)strftime(s,max,fmt,p);

    struct tmx *px;
    // cppcheck-suppress uninitvar
    (void)strfxtime(s,max,fmt,px);
}

void uninitvar_strlen(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)strlen(s);
}

void uninitvar_wcslen(void)
{
    wchar_t *s;
    // cppcheck-suppress uninitvar
    (void)wcslen(s);
}

void uninitvar_strncpy(void)
{
    char *s;
    char *ct;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)strncpy(s,ct,n);
}

void uninitvar_strncpy_s(char *Ct, size_t N1, char *S, size_t N2)
{
    char dest[42];
    char *s;
    size_t n1;
    size_t n2;
    size_t n3;
    size_t n4;

    // cppcheck-suppress uninitvar
    (void)strncpy_s(dest,n1,s,n2);
    // cppcheck-suppress uninitvar
    (void)strncpy_s(Ct,n3,S,N2);
    // cppcheck-suppress uninitvar
    (void)strncpy_s(Ct,N1,s,N2);
    // cppcheck-suppress uninitvar
    (void)strncpy_s(Ct,N1,S,n4);

    // no warning is expected for
    (void)strncpy_s(Ct,N1,S,N2);
    (void)strncpy_s(dest,N1,S,N2);
}

void uninitvar_strpbrk(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)strpbrk(cs,ct);
}

void uninitvar_strncat(char *Ct, char *S, size_t N)
{
    char *ct_1, *ct_2;
    char *s;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)strncat(ct_1,s,n1);
    // cppcheck-suppress uninitvar
    (void)strncat(ct_2,S,N);
    // cppcheck-suppress uninitvar
    (void)strncat(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)strncat(Ct,S,n2);

    // no warning is expected for
    (void)strncat(Ct,S,N);
}

// errno_t strcat_s(char *restrict dest, rsize_t destsz, const char *restrict src); // since C11
void uninitvar_strcat_s(char *Ct, size_t N, char *S)
{
    char *ct_1, *ct_2;
    char *s;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)strcat_s(ct_1,n1,s);
    // cppcheck-suppress uninitvar
    (void)strcat_s(ct_2,N,S);
    // cppcheck-suppress uninitvar
    (void)strcat_s(Ct,N,s);
    // cppcheck-suppress uninitvar
    (void)strcat_s(Ct,n2,S);

    // no warning is expected for
    (void) strcat_s(Ct,N,S);
}

// errno_t wcscat_s(wchar_t *restrict dest, rsize_t destsz, const wchar_t *restrict src); // since C11
void uninitvar_wcscat_s(wchar_t *Ct, size_t N, wchar_t *S)
{
    wchar_t *ct_1, *ct_2;
    wchar_t *s;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)wcscat_s(ct_1,n1,s);
    // cppcheck-suppress uninitvar
    (void)wcscat_s(ct_2,N,S);
    // cppcheck-suppress uninitvar
    (void)wcscat_s(Ct,N,s);
    // cppcheck-suppress uninitvar
    (void)wcscat_s(Ct,n2,S);

    // no warning is expected for
    (void) wcscat_s(Ct,N,S);
}

void uninitvar_strncat_s(char *Ct, size_t N1, char *S, size_t N2)
{
    char *ct_1, *ct_2;
    char *s;
    size_t n1;
    size_t n2;
    size_t n3;
    size_t n4;

    // cppcheck-suppress uninitvar
    (void)strncat_s(ct_1,n1,s,n2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(ct_2,N1,S,N2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(Ct,n3,S,N2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(Ct,N1,s,N2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(Ct,N1,S,n4);

    // no warning is expected for
    (void)strncat_s(Ct,N1,S,N2);
}

void uninitvar_wcsncat(wchar_t *Ct, wchar_t *S, size_t N)
{
    wchar_t *ct_1, *ct_2;
    wchar_t *s;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)wcsncat(ct_1,s,n1);
    // cppcheck-suppress uninitvar
    (void)wcsncat(ct_2,S,N);
    // cppcheck-suppress uninitvar
    (void)wcsncat(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)wcsncat(Ct,S,n2);

    // no warning is expected for
    (void)wcsncat(Ct,S,N);
}

void uninitvar_strncmp(char *Ct, char *S, size_t N)
{
    char *ct;
    char *s;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)strncmp(ct,s,n1);
    // cppcheck-suppress uninitvar
    (void)strncmp(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)strncmp(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)strncmp(Ct,S,n2);

    // no warning is expected for
    (void)strncmp(Ct,S,N);
}

void uninitvar_wcsncmp(wchar_t *Ct, wchar_t *S, size_t N)
{
    wchar_t *ct;
    wchar_t *s;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)wcsncmp(ct,s,n1);
    // cppcheck-suppress uninitvar
    (void)wcsncmp(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)wcsncmp(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)wcsncmp(Ct,S,n2);

    // no warning is expected for
    (void)wcsncmp(Ct,S,N);
}

void uninitvar_strstr(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)strstr(cs,ct);
}

void uninitvar_wcsstr(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcsstr(cs,ct);
}

void uninitvar_strspn(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)strspn(cs,ct);
}

void uninitvar_strxfrm(void)
{
    char *ds;
    char *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)strxfrm(ds,ss,n);
}

void uninitvar_wcsxfrm(void)
{
    wchar_t *ds;
    wchar_t *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wcsxfrm(ds,ss,n);
}

void uninitvar_wcsspn(void)
{
    wchar_t *ds;
    wchar_t *ss;
    // cppcheck-suppress uninitvar
    (void)wcsspn(ds,ss);
}

void uninitvar_setlocale(void)
{
    int category;
    char* locale;
    // cppcheck-suppress uninitvar
    (void)setlocale(category,locale);
}

void uninitvar_strerror(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)strerror(i);
}

void uninitvar_strcspn(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)strcspn(cs,ct);
}

void uninitvar_wcscspn(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcscspn(cs,ct);
}

void uninitvar_wcspbrk(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcspbrk(cs,ct);
}

void uninitvar_wcsncpy(void)
{
    wchar_t *cs;
    wchar_t *ct;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wcsncpy(cs,ct,n);
}

void uninitvar_strcoll(void)
{
    char *cs;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)strcoll(cs,ct);
}

void uninitvar_wcscoll(void)
{
    wchar_t *cs;
    wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcscoll(cs,ct);
}

void uninitvar_strrchr(void)
{
    char * str;
    int c;
    // cppcheck-suppress uninitvar
    (void)strrchr(str,c);
}

void uninitvar_wcsrchr(void)
{
    wchar_t* ws;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)wcsrchr(ws,wc);
}

void uninitvar_wcsrtombs(void)
{
    char *dst;
    const wchar_t * p;;
    size_t len;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)wcsrtombs(dst,&p,len,ps);
}

void uninitvar_strtok(void)
{
    char *s;
    char *ct;
    // cppcheck-suppress uninitvar
    (void)strtok(s,ct);
}

void uninitvar_strtoimax(void)
{
    const char *s;
    char **endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)strtoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)strtoumax(s,endp,base);
}

void uninitvar_strtof(void)
{
    const char *s;
    char **endp;
    // cppcheck-suppress uninitvar
    (void)strtof(s,endp);
    // cppcheck-suppress uninitvar
    (void)strtod(s,endp);
    // cppcheck-suppress uninitvar
    (void)strtold(s,endp);
}

void uninitvar_strtol(void)
{
    const char *s;
    char **endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)strtol(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)strtoll(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)strtoul(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)strtoull(s,endp,base);
}

void uninitvar_time(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)time(tp);
}

void uninitvar_tmpnam(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)tmpnam(s);
}

void uninitvar_tolower(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)tolower(c);
}

void uninitvar_toupper(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)toupper(c);
}

void uninitvar_wcstof(void)
{
    const wchar_t *s;
    wchar_t **endp;
    // cppcheck-suppress uninitvar
    (void)wcstof(s,endp);
    // cppcheck-suppress uninitvar
    (void)wcstod(s,endp);
    // cppcheck-suppress uninitvar
    (void)wcstold(s,endp);
}

void uninitvar_mbrtowc(void)
{
    wchar_t* pwc;
    const char* pmb;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)mbrtowc(pwc,pmb,max,ps);
}

void uninitvar_wcstok(void)
{
    wchar_t *s;
    const wchar_t *ct;
    wchar_t **ptr;
    // cppcheck-suppress uninitvar
    (void)wcstok(s,ct,ptr);
}

void uninitvar_wcstoimax(void)
{
    const wchar_t *s;
    wchar_t ** endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)wcstoimax(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)wcstoumax(s,endp,base);
}

void uninitvar_wcstol(void)
{
    const wchar_t *s;
    wchar_t ** endp;
    int base;
    // cppcheck-suppress uninitvar
    (void)wcstol(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)wcstoll(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)wcstoul(s,endp,base);
    // cppcheck-suppress uninitvar
    (void)wcstoull(s,endp,base);
}

void uninitvar_wprintf(wchar_t *Format, int Argument)
{
    const wchar_t *format1, *format2, *format3;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)wprintf(format1,argument1);
    // cppcheck-suppress uninitvar
    (void)wprintf(format2);
    // cppcheck-suppress uninitvar
    (void)wprintf(Format,argument2);
    // cppcheck-suppress uninitvar
    (void)wprintf(format3,Argument);
    // no warning is expected
    (void)wprintf(Format,Argument);
    (void)wprintf(Format);
}

void uninitvar_sprintf(char *S, char *Format, int Argument)
{
    char *s1, *s2;
    const char *format1, *format2;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)sprintf(s1,format1,argument1);
    // cppcheck-suppress uninitvar
    (void)sprintf(s2,Format,Argument);
    // cppcheck-suppress uninitvar
    (void)sprintf(S,format2,Argument);
    // cppcheck-suppress uninitvar
    (void)sprintf(S,Format,argument2);

    // no warning is expected for
    (void)sprintf(S,Format,Argument);
}

void uninitvar_swprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    // cppcheck-suppress uninitvar
    (void)swprintf(s,n,format);
}

void uninitvar_vsprintf(void)
{
    char *s;
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vsprintf(s,format,arg);
}

void valid_vsprintf_helper(const char * format, ...)
{
    char buffer[2];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    printf(buffer);
    va_end(args);
}

void valid_vsprintf()
{
    // buffer will contain "2\0" => no bufferAccessOutOfBounds
    // cppcheck-suppress checkLibraryFunction
    // cppcheck-suppress checkLibraryNoReturn
    valid_vsprintf_helper("%1.0f", 2.0f);
}

void uninitvar_vswprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vswprintf(s,n,format,arg);
}

void uninitvar_fwprintf(void)
{
    FILE* stream;
    const wchar_t* format;
    int i;
    // cppcheck-suppress uninitvar
    (void)fwprintf(stream,format,i);
}

void uninitvar_snprintf(char *S, size_t N, char *Format, int Int)
{
    size_t n1,n2;
    char *format;
    int i1, i2;
    char *s1, *s2;
    // cppcheck-suppress uninitvar
    (void)snprintf(s1,n1,format,i1);
    // cppcheck-suppress uninitvar
    (void)snprintf(S,n2,Format,Int); // n is uninitialized
    // cppcheck-suppress uninitvar
    (void)snprintf(S,N,format,Int); // format is uninitialized
    // cppcheck-suppress uninitvar
    (void)snprintf(S,N,Format,i2); // i is uninitialized
    // cppcheck-suppress uninitvar
    (void)snprintf(s2,N,Format,Int);

    // no warning is expected for
    (void)snprintf(S,N,Format,Int);
}

void uninitvar_vsnprintf(char *S, size_t N, char *Format, va_list Arg)
{
    char *s1, *s2;
    size_t n1, n2;
    char *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vsnprintf(s1,n1,format1,arg);
    // cppcheck-suppress uninitvar
    (void)vsnprintf(s2,N,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)vsnprintf(S,n2,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)vsnprintf(S,N,format2,Arg);

    // no warning is expected for
    (void)vsnprintf(S,N,Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vsnprintf(S,N,Format,arg);
}

void uninitvar_wscanf(void)
{
    wchar_t *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)wscanf(format);
    // cppcheck-suppress uninitvar
    (void)wscanf(format,&i);
}

void uninitvar_sscanf(void)
{
    char *string;
    const char * format;
    int i;
    // cppcheck-suppress uninitvar
    (void)sscanf(string,format);
    // cppcheck-suppress uninitvar
    (void)sscanf(string,format,&i);
}

void uninitvar_fwscanf(void)
{
    FILE* stream;
    wchar_t* format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)fwscanf(stream,format1);
    // cppcheck-suppress uninitvar
    (void)fwscanf(stream,format2,&i);
}

void uninitvar_swscanf(void)
{
    wchar_t* s;
    wchar_t* format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)swscanf(s,format1);
    // cppcheck-suppress uninitvar
    (void)swscanf(s,format2,&i);
}

void uninitvar_system(void)
{
    char *c;
    // cppcheck-suppress uninitvar
    (void)system(c);
}

void uninitvar_zonetime(void)
{
    time_t *tp;
    int zone;
    // cppcheck-suppress uninitvar
    (void)zonetime(tp,zone);
}

void uninitvar_itoa(void)
{
    int value;
    char * str;
    int base;
    // cppcheck-suppress uninitvar
    (void)itoa(value,str,base);
}

#ifdef __STD_UTF_16__
void uninitvar_c16rtomb(void)
{
    char * pmb;
    char16_t c16;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)c16rtomb(pmb,c16,ps);
}

void uninitvar_mbrtoc16(void)
{
    char16_t * pc16;
    char * pmb;
    size_t max;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)mbrtoc16(pc16,pmb,max,ps);
}
#endif // __STD_UTF_16__

#ifdef __STD_UTF_32__
void uninitvar_c32rtomb(void)
{
    char * pmb;
    char32_t c32;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)c32rtomb(pmb,c32,ps);
}

void uninitvar_mbrtoc32(void)
{
    char32_t * pc32;
    char * pmb;
    size_t max;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)mbrtoc32(pc32,pmb,max,ps);
}
#endif // __STD_UTF_32__

void invalidFunctionArgBool_abs(bool b, double x, double y)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)abs(true); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)abs(b); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)abs(x<y); // #5635
}

void invalidFunctionArg(char c)
{
    // cppcheck-suppress asctime_sCalled
    // cppcheck-suppress invalidFunctionArg
    asctime_s(1, 24, 1);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isalnum(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isalnum(-1);
    /* no warning for*/
    (void)isalnum(c);
    (void)isalnum(0);
    (void)isalnum(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isalpha(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isalpha(-1);
    /* no warning for*/
    (void)isalpha(c);
    (void)isalpha(0);
    (void)isalpha(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isblank(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isblank(-1);
    /* no warning for*/
    (void)isblank(c);
    (void)isblank(0);
    (void)isblank(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)iscntrl(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)iscntrl(-1);
    /* no warning for*/
    (void)iscntrl(c);
    (void)iscntrl(0);
    (void)iscntrl(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isdigit(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isdigit(-1);
    /* no warning for*/
    (void)isdigit(c);
    (void)isdigit(0);
    (void)isdigit(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isgraph(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isgraph(-1);
    /* no warning for*/
    (void)isgraph(c);
    (void)isgraph(0);
    (void)isgraph(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)islower(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)islower(-1);
    /* no warning for*/
    (void)islower(c);
    (void)islower(0);
    (void)islower(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isupper(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isupper(-1);
    /* no warning for*/
    (void)isupper(c);
    (void)isupper(0);
    (void)isupper(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isprint(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isprint(-1);
    /* no warning for*/
    (void)isprint(c);
    (void)isprint(0);
    (void)isprint(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)ispunct(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)ispunct(-1);
    /* no warning for*/
    (void)ispunct(c);
    (void)ispunct(0);
    (void)ispunct(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isspace(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isspace(-1);
    /* no warning for*/
    (void)isspace(c);
    (void)isspace(0);
    (void)isspace(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isxdigit(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isxdigit(-1);
    /* no warning for*/
    (void)isxdigit(c);
    (void)isxdigit(0);
    (void)isxdigit(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)tolower(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)tolower(-1);
    /* no warning for*/
    (void)tolower(c);
    (void)tolower(0);
    (void)tolower(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)toupper(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)toupper(-1);
    /* no warning for*/
    (void)toupper(c);
    (void)toupper(0);
    (void)toupper(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)strcpy_s(1,0,"a");
}

void invalidFunctionArgString(char c)
{
    /* cppcheck-suppress invalidFunctionArgStr */
    (void)atoi(&c);
    char x = 'x';
    /* cppcheck-suppress invalidFunctionArgStr */
    (void)strlen(&x);

    char y = '\0';
    (void)strlen(&y);

    // #5225
    char str[80] = "hello worl";
    char d='d';
    /* cppcheck-suppress invalidFunctionArgStr */
    (void)strcat(str,&d);
}

void ignoredReturnValue_abs(int i)
{
    // cppcheck-suppress ignoredReturnValue
    abs(i);
    // cppcheck-suppress ignoredReturnValue
    abs(-100);
}

void nullPointer_asctime(void)
{
    struct tm *tm = 0;
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)asctime(tm);
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)asctime(0);
}

void nullPointer_asctime_s(void)
{
    struct tm *tm = 0;
    char * buf = NULL;
    // cppcheck-suppress asctime_sCalled
    // cppcheck-suppress nullPointer
    asctime_s(buf, 26, 1);
    // cppcheck-suppress asctime_sCalled
    // cppcheck-suppress nullPointer
    asctime_s(1, 26, tm);
}

void nullPointer_wcsftime(size_t maxsize)
{
    wchar_t* ptr = 0;
    wchar_t* format = 0;
    struct tm* timeptr = 0;
    // cppcheck-suppress nullPointer
    (void)wcsftime(ptr,maxsize,format,timeptr);
    // cppcheck-suppress nullPointer
    (void)wcsftime(0,maxsize,0,0);
}

void nullPointer_fegetenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)fegetenv(envp);
    // cppcheck-suppress nullPointer
    (void)fegetenv(0);
}

void nullPointer_fegetexceptflag(int excepts)
{
    fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)fegetexceptflag(flagp,excepts);
    // cppcheck-suppress nullPointer
    (void)fegetexceptflag(0,excepts);
}

void nullPointer_feholdexcept(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)feholdexcept(envp);
    // cppcheck-suppress nullPointer
    (void)feholdexcept(0);
}

void nullPointer_fesetenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)fesetenv(envp);
    // cppcheck-suppress nullPointer
    (void)fesetenv(0);
}

void nullPointer_fesetexceptflag(int excepts)
{
    fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)fesetexceptflag(flagp,excepts);
    // cppcheck-suppress nullPointer
    (void)fesetexceptflag(0,excepts);
}

void nullPointer_feupdateenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)feupdateenv(envp);
    // cppcheck-suppress nullPointer
    (void)feupdateenv(0);
}

void nullPointer_atexit(void)
{
    // cppcheck-suppress nullPointer
    (void)atexit(0);
}

void nullPointer_atof(void)
{
    char * c = 0;
    // cppcheck-suppress nullPointer
    (void)atof(c);
    // cppcheck-suppress nullPointer
    (void)atof(0);
}

void invalidPrintfArgType_printf(void)
{
    int i = 0;
    // cppcheck-suppress invalidPrintfArgType_float
    printf("%f",i);

    // #7016
    uint8_t n = 7;
    // TODO cppcheck-suppress invalidPrintfArgType_uint
    printf("%"PRIi16"\n", n);
}


#define AssertAlwaysTrue(C)  if (C) {}

void valueFlow(void)
{
    const char abc[] = "abc";
    const int three = 3, minusThree = -3;
    const int c0='0', ca='a', blank=' ', tab='\t';
    const wint_t wblank=L' ', wtab=L'\t', w0=L'0';

    // When adding functions below, please sort alphabetically.

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(strlen(abc) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(abs(three) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(abs(minusThree) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isblank(blank) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isblank(tab) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isblank(c0) == 0);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isdigit(c0) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isdigit(ca) == 0);

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswblank(wblank) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswblank(wtab) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswblank(w0) == 0);

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswdigit(w0) == 0);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswdigit(wtab) == 1);

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(labs(three) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(labs(minusThree) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(llabs(three) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(llabs(minusThree) == 3);
}
