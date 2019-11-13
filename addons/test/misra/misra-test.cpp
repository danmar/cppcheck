// #8441
class C {
    int a;
    int b;
    C() : a(1), b(1) { c; }
};

class misra_21_1_C {
    public:
       misra_21_1_C operator=(const misra_21_1_C &);
};

static bool test_misra_21_1_crash(const C* cc)
{
    auto misra_21_1_C a, b;
    a = b;
}
