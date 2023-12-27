// Command:
// ../../cppcheck --addon=namingng_test.json --enable=style --inline-suppr --suppress=unusedVariable --suppress=constVariablePointer --suppress=unreadVariable --error-exitcode=1 namingng_test.c
// Expected return code is 0

#include <stddef.h>
#include <stdint.h>

uint32_t ui32Good (int abc)
{
    uint32_t ui32good;
    int32_t i32good;
    uint32_t badui32; // cppcheck-suppress namingng-mismatch
    int32_t badi32;

    uint32_t a;  // cppcheck-suppress namingng-mismatch; Short name
    return 5;
}

uint16_t ui16Good (int a)
{
    return 5;
}

uint16_t ui16bad_underscore (int a) // cppcheck-suppress namingng-mismatch
{
    return 5;
}

uint32_t u32Bad (int a) // cppcheck-suppress namingng-mismatch
{
    uint32_t ui32good;
    int32_t i32good;
    uint32_t badui32; // cppcheck-suppress namingng-mismatch
    int32_t badi32;
    int * intpointer=NULL;
    int ** intppointer=NULL;
    int *** intpppointer=NULL;
    return 5;
}

uint16_t Badui16 (int a) // cppcheck-suppress namingng-mismatch
{
    return 5;
}

void * Pointer()
{
    return NULL;
}

void ** PPointer()
{
    return NULL;
}
