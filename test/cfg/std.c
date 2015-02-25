
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

void bufferAccessOutOf(void) {
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

void ignoreleak(void) {
    char *p = (char *)malloc(10);
    memset(&(p[0]), 0, 10);
    // cppcheck-suppress memleak
}

// null pointer

void nullpointer(int value){
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
  fclose(fp); fp = 0;
  // No FP
  fflush(0);
  // No FP
  // cppcheck-suppress redundantAssignment
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
  // cppcheck-suppress ignoredReturnValue
  // cppcheck-suppress nullPointer
  strtoul(0,0,0);
  // cppcheck-suppress ignoredReturnValue
  // cppcheck-suppress nullPointer
  strtoull(0,0,0);
  // cppcheck-suppress ignoredReturnValue
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

void nullpointerMemchr1(char *p, char *s) {
  // cppcheck-suppress uselessAssignmentPtrArg
  p = memchr (s, 'p', strlen(s));
}

void nullpointerMemchr2(char *p, char *s) {
  // cppcheck-suppress uselessAssignmentPtrArg
  p = memchr (s, 0, strlen(s));
}

void nullpointerMemchr3(char *p) {
  char *s = 0;
  // cppcheck-suppress nullPointer
  // cppcheck-suppress uselessAssignmentPtrArg
  p = memchr (s, 0, strlen(s));
}

void nullpointerMemcmp(char *p) {
  // cppcheck-suppress ignoredReturnValue
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
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress uninitvar
    feof(fp);
}

void uninit_ferror(void) {
    FILE *fp;
    // cppcheck-suppress ignoredReturnValue
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

void ignoreretrn(void) {
  char szNumbers[] = "2001 60c0c0 -1101110100110100100000 0x6fffff";
  char * pEnd;
  strtol (szNumbers,&pEnd,10);
}
