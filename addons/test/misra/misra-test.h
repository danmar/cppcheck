#ifndef MISRA_TEST_H
#define MISRA_TEST_H
struct misra_h_s { int foo; };
bool test(char *); // 8.2
bool test(char *a); // OK
int misra_8_2_no_fp(int a);
#endif // MISRA_TEST_H
