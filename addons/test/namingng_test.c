#include <stddef.h>
#include <inttypes.h>

uint32_t ui32Good (int a)
{
    uint32_t ui32good;
    int32_t i32good;
    uint32_t badui32;
    int32_t badi32;

    uint32_t a;  // Short
    return 5;
}

uint16_t ui16Good (int a)
{
    return 5;
}

uint16_t ui16bad_underscore (int a)
{
    return 5;
}

uint32_t u32Bad (int a)
{
    uint32_t ui32good;
    int32_t i32good;
    uint32_t badui32;
    int32_t badi32;
    int * intpointer=NULL;
    int ** intppointer=NULL;
    int *** intpppointer=NULL;
    return 5;
}

uint16_t Badui16 (int a)
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
