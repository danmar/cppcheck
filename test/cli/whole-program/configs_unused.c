
typedef struct { // tests misra system Level analysis [misra-c2012-2.3]
    int x; // cppcheck-suppress unusedStructMember
} X;

static void func(void) {};// tests whole project analysis [unusedFunction]

int main(void) {
    #ifdef A
    X x = {5};
    func();
    return x.x;
    
    #elif defined(B)
    return 0;
    #endif
    
}