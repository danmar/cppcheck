
struct S { int x; };

int globalvar;

void callfunc1() {
    int x = 16;
    scanf("%i\n", &x);
    // cppcheck-suppress verificationDivByZero
    return 100000 / x;
}

void g1() {
    // cppcheck-suppress verificationDivByZero
    return 100000 / globalvar;
}

void pointer1(int *p) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / *p;
}

void pointer2(int *p) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / p[32];
}

void float1(float f) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / (int)f;
}

void float2(float f) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / f;
}

void stdmap(std::map<int,int> &data) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / data[43];
}

void struct1(struct S *s) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / s->x;
}
