int buffer_overflow() { int x[10]; return x[100]; }
int division_by_zero() { return 100 / 0; }
int no_return() {}
int null_pointer() { int *p = 0; return *p; }
int *pointer_arithmetic() { static int buf[10]; return buf + 100; }
int shift_overrun(int x) { return x << 123; }
int shift_negative() { return -1 << 1; }
int signed_int_overrun() { int x = ~0; return x * 2; }
void string_literal() { *((char *)"hello") = 0; }
int uninit() { int x; return x + 2; }
