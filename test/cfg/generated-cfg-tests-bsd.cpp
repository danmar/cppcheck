// auto generated tests from cfg/bsd.cfg
//
// Generated by command:
// ./generate_cfg_tests cfg/bsd.cfg > generated-cfg-tests-bsd.cpp
//
// Recommended cppcheck command line:
// $ cppcheck --enable=warning,information --inline-suppr --platform=unix64 generated-cfg-tests-bsd.cpp
// => 'unmatched suppression' warnings are false negatives.
//

void test__fts_open__noreturn() {
  int x = 1;
  if (cond) { x=100; fts_open(arg1, arg2, arg3); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__fts_open__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_open(x, arg2, arg3);
}

void test__fts_open__arg2__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_open(arg1, x, arg3);
}

void test__fts_open__arg3__notnull() {
  // cppcheck-suppress nullPointer
  fts_open(arg1, arg2, NULL);
}

void test__fts_open__arg3__notuninit() {
  int x[10];
  // cppcheck-suppress uninitvar
  fts_open(arg1, arg2, x);
}

void test__fts_read__noreturn() {
  int x = 1;
  if (cond) { x=100; fts_read(arg1); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__fts_read__arg1__notnull() {
  // cppcheck-suppress nullPointer
  fts_read(NULL);
}

void test__fts_read__arg1__notuninit() {
  int x[10];
  // cppcheck-suppress uninitvar
  fts_read(x);
}

void test__readpassphrase__noreturn() {
  int x = 1;
  if (cond) { x=100; readpassphrase(arg1, arg2); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__readpassphrase__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  readpassphrase(x, arg2);
}

void test__readpassphrase__arg2__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  readpassphrase(arg1, x);
}

void test__fts_set__noreturn() {
  int x = 1;
  if (cond) { x=100; fts_set(arg1, arg2, arg3); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__fts_set__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_set(x, arg2, arg3);
}

void test__fts_set__arg2__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_set(arg1, x, arg3);
}

void test__fts_set__arg3__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_set(arg1, arg2, x);
}

void test__fts_set_clientptr__noreturn() {
  int x = 1;
  if (cond) { x=100; fts_set_clientptr(arg1, arg2); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__fts_set_clientptr__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_set_clientptr(x, arg2);
}

void test__fts_set_clientptr__arg2__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_set_clientptr(arg1, x);
}

void test__fts_get_clientptr__noreturn() {
  int x = 1;
  if (cond) { x=100; fts_get_clientptr(arg1); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__fts_get_clientptr__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_get_clientptr(x);
}

void test__fts_get_stream__noreturn() {
  int x = 1;
  if (cond) { x=100; fts_get_stream(arg1); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__fts_get_stream__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_get_stream(x);
}

void test__fts_close__noreturn() {
  int x = 1;
  if (cond) { x=100; fts_close(arg1); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__fts_close__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  fts_close(x);
}

void test__readpassphrase__noreturn() {
  int x = 1;
  if (cond) { x=100; readpassphrase(arg1, arg2, arg3, arg4); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__readpassphrase__leakignore() {
  char *p = malloc(10); *p=0;
  readpassphrase(p, arg2, arg3, arg4);
  // cppcheck-suppress memleak
}

void test__readpassphrase__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  readpassphrase(x, arg2, arg3, arg4);
}

void test__readpassphrase__arg2__notnull() {
  // cppcheck-suppress nullPointer
  readpassphrase(arg1, NULL, arg3, arg4);
}

void test__readpassphrase__arg3__notnull() {
  // cppcheck-suppress nullPointer
  readpassphrase(arg1, arg2, NULL, arg4);
}

void test__readpassphrase__arg3__notuninit() {
  int x[10];
  // cppcheck-suppress uninitvar
  readpassphrase(arg1, arg2, x, arg4);
}

void test__readpassphrase__arg4__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  readpassphrase(arg1, arg2, arg3, x);
}

void test__setfib__noreturn() {
  int x = 1;
  if (cond) { x=100; setfib(arg1); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__setfib__arg1__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  setfib(x);
}

void test__strtonum__noreturn() {
  int x = 1;
  if (cond) { x=100; strtonum(arg1, arg2, arg3, arg4); }
  // cppcheck-suppress shiftTooManyBits
  x = 1 << x;
}

void test__strtonum__leakignore() {
  char *p = malloc(10); *p=0;
  strtonum(p, arg2, arg3, arg4);
  // cppcheck-suppress memleak
}

void test__strtonum__arg1__notnull() {
  // cppcheck-suppress nullPointer
  strtonum(NULL, arg2, arg3, arg4);
}

void test__strtonum__arg1__notuninit() {
  int x[10];
  // cppcheck-suppress uninitvar
  strtonum(x, arg2, arg3, arg4);
}

void test__strtonum__arg2__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  strtonum(arg1, x, arg3, arg4);
}

void test__strtonum__arg3__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  strtonum(arg1, arg2, x, arg4);
}

void test__strtonum__arg4__notuninit() {
  int x;
  // cppcheck-suppress uninitvar
  strtonum(arg1, arg2, arg3, x);
}


int main() {
    test__fts_open__noreturn();
    test__fts_open__arg1__notuninit();
    test__fts_open__arg2__notuninit();
    test__fts_open__arg3__notnull();
    test__fts_open__arg3__notuninit();
    test__fts_read__noreturn();
    test__fts_read__arg1__notnull();
    test__fts_read__arg1__notuninit();
    test__readpassphrase__noreturn();
    test__readpassphrase__arg1__notuninit();
    test__readpassphrase__arg2__notuninit();
    test__fts_set__noreturn();
    test__fts_set__arg1__notuninit();
    test__fts_set__arg2__notuninit();
    test__fts_set__arg3__notuninit();
    test__fts_set_clientptr__noreturn();
    test__fts_set_clientptr__arg1__notuninit();
    test__fts_set_clientptr__arg2__notuninit();
    test__fts_get_clientptr__noreturn();
    test__fts_get_clientptr__arg1__notuninit();
    test__fts_get_stream__noreturn();
    test__fts_get_stream__arg1__notuninit();
    test__fts_close__noreturn();
    test__fts_close__arg1__notuninit();
    test__readpassphrase__noreturn();
    test__readpassphrase__leakignore();
    test__readpassphrase__arg1__notuninit();
    test__readpassphrase__arg2__notnull();
    test__readpassphrase__arg3__notnull();
    test__readpassphrase__arg3__notuninit();
    test__readpassphrase__arg4__notuninit();
    test__setfib__noreturn();
    test__setfib__arg1__notuninit();
    test__strtonum__noreturn();
    test__strtonum__leakignore();
    test__strtonum__arg1__notnull();
    test__strtonum__arg1__notuninit();
    test__strtonum__arg2__notuninit();
    test__strtonum__arg3__notuninit();
    test__strtonum__arg4__notuninit();
    return 0;
}
