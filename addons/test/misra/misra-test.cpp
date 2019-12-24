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

class C2 {
public:
	C2();
private:
	void* f;
};
C2::C2() : f(NULL) {}

static bool test_misra_21_1_crash()
{
    auto misra_21_1_C a, b;
    a = b;
}
