// #10431
#ifdef X
struct S { int i; }; // cppcheck-suppress unusedStructMember
#else
struct S {};
#endif
