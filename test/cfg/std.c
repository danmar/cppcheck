
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
#include <fenv.h>

void bufferAccessOutOfBounds(void)
{
    char a[5];
    fgets(a,5,stdin);
    // cppcheck-suppress bufferAccessOutOfBounds
    fgets(a,6,stdin);
    sprintf(a, "ab%s", "cd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress redundantCopy
    sprintf(a, "ab%s", "cde");
    // cppcheck-suppress redundantCopy
    snprintf(a, 5, "abcde%i", 1);
    // cppcheck-suppress redundantCopy
    snprintf(a, 6, "abcde%i", 1);   //TODO: cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress redundantCopy
    strcpy(a,"abcd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress redundantCopy
    strcpy(a, "abcde");
    // cppcheck-suppress redundantCopy
    strncpy(a,"abcde",5);
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress redundantCopy
    strncpy(a,"abcde",6);
    fread(a,1,5,stdin);
    // cppcheck-suppress bufferAccessOutOfBounds
    fread(a,1,6,stdin);
    fwrite(a,1,5,stdout);
    // cppcheck-suppress bufferAccessOutOfBounds
    fread(a,1,6,stdout);
}

// memory leak

void ignoreleak(void)
{
    char *p = (char *)malloc(10);
    memset(&(p[0]), 0, 10);
    // cppcheck-suppress memleak
}

// null pointer

void nullpointer(int value)
{
    int res = 0;
    FILE *fp;

    // cppcheck-suppress nullPointer
    clearerr(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    feof(0);
    // cppcheck-suppress nullPointer
    fgetc(0);
    // cppcheck-suppress nullPointer
    fclose(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    ferror(0);
    // cppcheck-suppress nullPointer
    ftell(0);
    // cppcheck-suppress nullPointer
    puts(0);
    // cppcheck-suppress nullPointer
    fp=fopen(0,0);
    fclose(fp);
    fp = 0;
    // No FP
    fflush(0);
    // No FP
    // cppcheck-suppress redundantAssignment
    fp = freopen(0,"abc",stdin);
    fclose(fp);
    fp = 0;
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
    strlen(0);
    // cppcheck-suppress nullPointer
    strcpy(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcoll(0,0);
    // cppcheck-suppress nullPointer
    strcat(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcmp(0,0);
    // cppcheck-suppress nullPointer
    strncpy(0,0,1);
    // cppcheck-suppress nullPointer
    strncat(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strncmp(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strstr(0,0);
    // cppcheck-suppress nullPointer
    strtoul(0,0,0);
    // cppcheck-suppress nullPointer
    strtoull(0,0,0);
    // cppcheck-suppress nullPointer
    strtol(0,0,0);

    // #6100 False positive nullPointer - calling mbstowcs(NULL,)
    res += mbstowcs(0,"",0);
    // cppcheck-suppress unreadVariable
    res += wcstombs(0,L"",0);

    strtok(NULL,"xyz");

    strxfrm(0,"foo",0);
    // TODO: error message
    strxfrm(0,"foo",42);

    snprintf(NULL, 0, "someformatstring"); // legal
    // cppcheck-suppress nullPointer
    snprintf(NULL, 42, "someformatstring"); // not legal
}

void nullpointerMemchr1(char *p, char *s)
{
    // cppcheck-suppress uselessAssignmentPtrArg
    p = memchr(s, 'p', strlen(s));
}

void nullpointerMemchr2(char *p, char *s)
{
    // cppcheck-suppress uselessAssignmentPtrArg
    p = memchr(s, 0, strlen(s));
}

void nullpointerMemchr3(char *p)
{
    char *s = 0;
    // cppcheck-suppress nullPointer
    // cppcheck-suppress uselessAssignmentPtrArg
    p = memchr(s, 0, strlen(s));
}

void nullpointerMemcmp(char *p)
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    memcmp(p, 0, 123);
}


// uninit pointers

void uninit_clearerr(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    clearerr(fp);
}

void uninit_fclose(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    fclose(fp);
}

void uninit_fopen(void)
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

void uninit_feof(void)
{
    FILE *fp;
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress uninitvar
    feof(fp);
}

void uninit_ferror(void)
{
    FILE *fp;
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress uninitvar
    ferror(fp);
}

void uninit_fflush(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    fflush(fp);
}

void uninit_fgetc(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    fgetc(fp);
}

void uninit_fgetpos(void)
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

void uninit_fsetpos(void)
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

void uninit_fgets(void)
{
    FILE *fp;
    char buf[10];
    char *str;

    fgets(buf,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(str,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(buf,10,fp);
}

void uninit_fputc(void)
{
    int i;
    FILE *fp;

    fputc('a', stdout);

    // cppcheck-suppress uninitvar
    fputc(i, stdout);

    // cppcheck-suppress uninitvar
    fputc('a', fp);
}

void uninit_fputs(void)
{
    const char *s;
    FILE *fp;

    fputs("a", stdout);

    // cppcheck-suppress uninitvar
    fputs(s, stdout);

    // cppcheck-suppress uninitvar
    fputs("a", fp);
}

void uninit_ftell(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    ftell(fp);
}

void uninit_puts(void)
{
    const char *s;
    // cppcheck-suppress uninitvar
    puts(s);
}

void uninit_putchar(void)
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
    // cppcheck-suppress obsoleteFunctionsasctime
    (void)asctime(tm);
}

#if 0
void uninitvar_assert(void)
{
    int i;
    // cppcheck-suppress uninitvar
    assert(i);
}
#endif

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

void ignoreretrn(void)
{
    char szNumbers[] = "2001 60c0c0 -1101110100110100100000 0x6fffff";
    char * pEnd;
    strtol(szNumbers,&pEnd,10);
}
