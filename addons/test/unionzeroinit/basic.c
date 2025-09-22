#include <stdint.h>

union bad_union_0 {
    char c;
    int64_t i64;
    void *p;
};

typedef union {
    char c;
    int i;
} bad_union_1;

extern void e(union bad_union_0 *);

void
foo(void)
{
    union { int i; char c; } good0 = {0};
    union { int i; char c; } good1 = {};

    union { char c; int i; } bad0 = {0};
    union bad_union_0 bad1 = {0};
    e(&bad1);
    bad_union_1 bad2 = {0};
}
