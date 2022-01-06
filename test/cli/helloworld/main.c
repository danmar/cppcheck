#include <stdio.h>

int main(void) {
    (void)printf("Hello world!\n");
    x = 3 / 0; // ERROR
    return 0;
}

#ifdef SOME_CONFIG
void foo();
#endif
