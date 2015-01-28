
// Test library configuration for std.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --inline-suppr cfg/test/std.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <string.h>
#include <stdio.h>

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
  fsetpos(0,0);
  // cppcheck-suppress nullPointer
  itoa(123,0,10);
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
