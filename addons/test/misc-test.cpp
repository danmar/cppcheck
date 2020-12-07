// To test:
// ~/cppcheck/cppcheck --dump misc-test.cpp && python ../misc.py -verify misc-test.cpp.dump

#include <string>
#include <vector>

// Warn about string concatenation in array initializers..
const char *a[] = {"a" "b"}; // stringConcatInArrayInit
const char *b[] = {"a","b" "c"}; // stringConcatInArrayInit
#define MACRO "MACRO"
const char *c[] = { MACRO "text" }; // stringConcatInArrayInit


// Function is implicitly virtual
class base {
    virtual void dostuff(int);	
};

class derived : base {
	void dostuff(int); // implicitlyVirtual
};


// Pass struct to ellipsis function
struct {int x;int y;} s;
void ellipsis(int x, ...);
void foo(std::vector<std::string> v) {
    ellipsis(321, s); // ellipsisStructArg
    ellipsis(321, v[0]); // ellipsisStructArg
}
