#include <stdio.h>

int main(int argc, char *argv[])
{
#ifdef 0
    // cppcheck-suppress zerodiv
    int i = 1/0;
    printf("i = %d\n", i);
#endif
    return 0;
}
