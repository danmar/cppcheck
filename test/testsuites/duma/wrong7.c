#include <stdio.h>

int main() {
    int *p;

    p = (int*) malloc( sizeof(int) * 10 );
    printf("Now writing before our allocated array\n");
    p[-1] ^= 0x0F; /* bash before */
    printf("... and now after our allocated array\n");
    p[10] ^= 0x0F; /* bash after */
    printf("Did you notice?\n");
    free(p);
}
