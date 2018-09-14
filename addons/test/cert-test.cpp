// To test:
// ~/cppcheck/cppcheck --dump cert-test.cpp && python ../cert.py -verify cert-test.cpp.dump

#include <cstdlib>

class msc30TestClass {
public:
    static int rand();
};

void msc30(msc30TestClass & testClass)
{
    unsigned int num = rand(); // cert-MSC30-c
    num = std::rand(); // cert-MSC30-c
    num = msc30TestClass::rand();
    num = unknownClass::rand();
    num = testClass.rand();
    num = unknownClass.rand();
    int rand = 5;
    int a = rand;
}
