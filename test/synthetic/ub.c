void alias() { int x; int *ip=&x; float *fp = (float *)ip; }
int buffer_overflow() { int x[10]={0}; return x[100]; }
int dead_pointer(int a) { int *p=&a; if (a) { int x=0; p = &x; } return *p; }
int division_by_zero() { return 100 / 0; }
int float_to_int() { double d=1E100; return (int)d; }
void negative_size(int sz) { if (sz < 0) { int buf[sz]; } }
int no_return() {}
int null_pointer() { int *p = 0; return *p; }
int *pointer_arithmetic() { static int buf[10]; return buf + 100; }
unsigned char pointer_to_u8() { static int buf[10]; return (int*)buf; }
int pointer_subtraction() { char a[10]; char b[10]; return b-a; }
int pointer_comparison() { char a[10]; char b[10]; return b<a; }
int shift_overrun(int x) { return x << 123; }
int shift_negative() { return -1 << 1; }
int int_overflow() { int intmax = (~0U) >> 1; return intmax * 2; }
void string_literal() { *((char *)"hello") = 0; }
int uninit() { int x; return x + 2; }
