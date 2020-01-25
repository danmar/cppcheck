// make USE_Z3=yes
// ./cppcheck --verify --inline-suppr --enable=information test/testsuites/danmar-verify/divbyzero.cpp

#include <stdio.h>
#include <map>

struct S { int x; };

int globalvar;

void dostuff();

int callfunc1() {
    int x = 16;
    scanf("%i\n", &x);
    // cppcheck-suppress verificationDivByZero
    return 100000 / x;
}

int float1(float f) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / (int)f;
}

float float2(float f) {
    // cppcheck-suppress verificationDivByZeroFloat
    return 100000 / f;
}

int functionCall() {
#ifdef __clang__
    return 0;
#else
    // cppcheck-suppress verificationDivByZero
    return 100000 / unknown_function();
#endif
}

int globalVar1() {
    // cppcheck-suppress verificationDivByZero
    return 100000 / globalvar;
}

int globalVar2() {
    globalvar = 123;
    dostuff();
    // cppcheck-suppress verificationDivByZero
    return 100000 / globalvar;
}

int pointer1(int *p) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / *p;
}

int pointer2(int *p) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / p[32];
}

int stdmap(std::map<int,int> &data) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / data[43];
}

int struct1(struct S *s) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / s->x;
}

int trycatch() {
    int x = 0;
    try {
        dostuff();
        x = 1;
    } catch (...) {}
    // cppcheck-suppress verificationDivByZero
    return 100000 / x;
}
