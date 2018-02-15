
// Test platform configuration for avr8
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/platforms/avr8.c
// =>
// No warnings about unmatched suppressions, etc. exitcode=0
//

#include <stdint.h>

void test()
{
    signed char charVar = INT8_MAX;
    // cppcheck-suppress redundantAssignment
    charVar = 0;
    // cppcheck-suppress shiftTooManyBitsSigned
    charVar << 7;
    // cppcheck-suppress shiftTooManyBits
    charVar << 8;

    char ucharVar = UINT8_MAX;
    ucharVar << 7;
    // cppcheck-suppress shiftTooManyBits
    ucharVar << 8;

    short shortVar = INT16_MAX;
    shortVar << 14;
    // cppcheck-suppress shiftTooManyBitsSigned
    shortVar << 15;
    // cppcheck-suppress shiftTooManyBits
    shortVar << 16;

    unsigned short ushortVar = UINT16_MAX;
    ushortVar << 15;
    // cppcheck-suppress shiftTooManyBits
    ushortVar << 16;

    int intVar = INT16_MAX;
    // cppcheck-suppress redundantAssignment
    intVar = 0;
    intVar << 14;
    // cppcheck-suppress shiftTooManyBitsSigned
    intVar << 15;
    // cppcheck-suppress shiftTooManyBits
    intVar << 16;

    unsigned int uintVar = UINT16_MAX;
    uintVar << 15;
    // cppcheck-suppress shiftTooManyBits
    uintVar << 16;

    long longVar = INT32_MAX;
    // cppcheck-suppress redundantAssignment
    longVar = 0;
    longVar << 30;
    // cppcheck-suppress shiftTooManyBitsSigned
    longVar << 31;
    // cppcheck-suppress shiftTooManyBits
    longVar << 32;

    unsigned long ulongVar = UINT32_MAX;
    ulongVar << 31;
    // cppcheck-suppress shiftTooManyBits
    ulongVar << 32;
}
