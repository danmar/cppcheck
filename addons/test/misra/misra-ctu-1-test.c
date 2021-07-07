// Test with command:
// ./cppcheck --addon=misra --inline-suppr addons/test/misra/misra-ctu-*-test.c

#include "misra-ctu-test.h"

// cppcheck-suppress misra-c2012-5.6
typedef int MISRA_5_6_VIOLATION;

