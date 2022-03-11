#include <stdio.h>

int main(int argc, char *argv[])
{
#ifdef INCLUDE_ZERO_DIV
    // cppcheck-suppress zerodiv
    int i = 1/0;
    printf("i = %d\n", i);
#endif
    return 0;
}
