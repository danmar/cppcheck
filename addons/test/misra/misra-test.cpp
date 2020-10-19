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
    auto misra_21_1_C a, b; // 12.3
    a = b;
}

static void test_misra_12_3()
{
    // this is not a 12.3
    new QMap<int, void*>();
}

static void test_misra_13_4()
{
    // this is not a 13.4
    std::function<void(std::vector<uint8_t>)> CommTestReceived= [=] (std::vector<uint8_t> payload)
    {
        this->commTestReceived(payload);
    };

    // this is a 13.4 violation
    a [ x ] = a[ x = y ];
}


