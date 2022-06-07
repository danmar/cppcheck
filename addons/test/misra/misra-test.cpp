// #8441
class C {
    int a;
    int b;
    C(void) : a(1), b(1) { c; }
};

class misra_21_1_C {
    public:
       misra_21_1_C operator=(const misra_21_1_C &);
};

class C2 {
public:
	C2(void);
private:
	void* f;
};
C2::C2(void) : f(NULL) {}

static void test_misra_21_1_crash(void)
{
    auto misra_21_1_C a, b; // 12.3
    a = b;
}
