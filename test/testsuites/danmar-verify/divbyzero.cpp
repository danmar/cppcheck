
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

void stdmap(std::map<int,int> &data) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / data[43];
}

struct S { int x; };
void struct1(struct S *s) {
    // cppcheck-suppress verificationDivByZero
    return 100000 / s->x;
}
