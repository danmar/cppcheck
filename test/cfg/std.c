
// Test library configuration for std.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --inline-suppr cfg/test/std.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>
#include <stdio.h>
#include <tgmath.h> // frexp

void strcpy_ok(char *a, char *b) {
    strcpy(a,b);
}

void strcpy_bad() {
  char a[10];
  // cppcheck-suppress bufferAccessOutOfBounds
  strcpy(a, "hello world!");
}


// null pointer

void nullpointer(int value){
  int res = 0;
  FILE *fp;
    
  // cppcheck-suppress nullPointer
  clearerr(0);
  // cppcheck-suppress nullPointer
  feof(0);
  // cppcheck-suppress nullPointer
  fgetc(0);
  // cppcheck-suppress nullPointer
  fclose(0);
  // cppcheck-suppress nullPointer
  ferror(0);
  // cppcheck-suppress nullPointer
  ftell(0);
  // cppcheck-suppress nullPointer
  puts(0);
  // cppcheck-suppress nullPointer
  fp=fopen(0,0);
  fclose(fp); fp = 0;
  // No FP
  fflush(0);
  // No FP
  fp = freopen(0,"abc",stdin);
  fclose(fp); fp = 0;
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
  // cppcheck-suppress nullPointer
  strchr(0,0);
  // cppcheck-suppress nullPointer
  strlen(0);
  // cppcheck-suppress nullPointer
  strcpy(0,0);
  // cppcheck-suppress nullPointer
  strspn(0,0);
  // cppcheck-suppress nullPointer
  strcspn(0,0);
  // cppcheck-suppress nullPointer
  strcoll(0,0);
  // cppcheck-suppress nullPointer
  strcat(0,0);
  // cppcheck-suppress nullPointer
  strcmp(0,0);
  // cppcheck-suppress nullPointer
  strncpy(0,0,1);
  // cppcheck-suppress nullPointer
  strncat(0,0,1);
  // cppcheck-suppress nullPointer
  strncmp(0,0,1);
  // cppcheck-suppress nullPointer
  strstr(0,0);
  // cppcheck-suppress nullPointer
  strtoul(0,0,0);
  // cppcheck-suppress nullPointer
  strtoull(0,0,0);
  // cppcheck-suppress nullPointer
  strtol(0,0,0);

  // #6100 False positive nullPointer - calling mbstowcs(NULL,)
  res += mbstowcs(0,value,0);
  res += wcstombs(0,value,0);

  strtok(NULL,"xyz");

  strxfrm(0,"foo",0);
  // TODO: error message
  strxfrm(0,"foo",42);
}

void nullpointerMemchr1(char *p, char *s) {
  p = memchr (s, 'p', strlen(s));
}

void nullpointerMemchr2(char *p, char *s) {
  p = memchr (s, 0, strlen(s));
}

void nullpointerMemchr3(char *p) {
  char *s = 0;
  // cppcheck-suppress nullPointer
  p = memchr (s, 0, strlen(s));
}

void nullpointerMemcmp(char *p) {
  // cppcheck-suppress nullPointer
  memcmp(p, 0, 123);
}


// uninit pointers

void uninit_clearerr(void) {
    FILE *fp;
    // cppcheck-suppress uninitvar
    clearerr(fp);
}

void uninit_fclose(void) {
    FILE *fp;
    // cppcheck-suppress uninitvar
    fclose(fp);
}

void uninit_fopen(void) {
    const char *filename, *mode;
    FILE *fp;
    // cppcheck-suppress uninitvar
    fp = fopen(filename, "rt");
    fclose(fp);
    // cppcheck-suppress uninitvar
    fp = fopen("filename.txt", mode);
    fclose(fp);
}

void uninit_feof(void) {
    FILE *fp;
    // cppcheck-suppress uninitvar
    feof(fp);
}

void uninit_ferror(void) {
    FILE *fp;
    // cppcheck-suppress uninitvar
    ferror(fp);
}

void uninit_fflush(void) {
    FILE *fp;
    // cppcheck-suppress uninitvar
    fflush(fp);
}

void uninit_fgetc(void) {
    FILE *fp;
    // cppcheck-suppress uninitvar
    fgetc(fp);
}

void uninit_fgetpos(void) {
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

void uninit_fsetpos(void) {
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

void uninit_fgets(void) {
    FILE *fp;
    char buf[10];
    char *str;

    fgets(buf,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(str,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(buf,10,fp);
}

void uninit_fputc(void) {
    int i;
    FILE *fp;

    fputc('a', stdout);

    // cppcheck-suppress uninitvar
    fputc(i, stdout);

    // cppcheck-suppress uninitvar
    fputc('a', fp);
}

void uninit_fputs(void) {
    const char *s;
    FILE *fp;

    fputs("a", stdout);

    // cppcheck-suppress uninitvar
    fputs(s, stdout);

    // cppcheck-suppress uninitvar
    fputs("a", fp);
}

void uninit_ftell(void) {
    FILE *fp;
    // cppcheck-suppress uninitvar
    ftell(fp);
}

void uninit_puts(void) {
    const char *s;
    // cppcheck-suppress uninitvar
    puts(s);
}

void uninit_putchar(void) {
    char c;
    // cppcheck-suppress uninitvar
    putchar(c);
}
