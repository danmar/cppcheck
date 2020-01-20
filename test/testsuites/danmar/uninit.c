
// make USE_Z3=yes
// ./cppcheck --verify --inline-suppr --enable=information test/testsuites/danmar-verify/uninit.c

#include <string.h>

int array1() {
    int a[10];
    a[0] = 0;
    // cppcheck-suppress verificationUninit
    return a[2];
}

int array2() {
    int a[10][10];
    a[0][0] = 0;
    // cppcheck-suppress verificationUninit
    return a[2][3];
}

int local1() {
    int x;
    // cppcheck-suppress verificationUninit
    // cppcheck-suppress uninitvar
    return x;
}

int pointer1(int *p) {
    // cppcheck-suppress verificationUninit
    return *p;
}

int pointer2(char *p) {
    // cppcheck-suppress verificationUninitArg
    return strlen(p);
}

