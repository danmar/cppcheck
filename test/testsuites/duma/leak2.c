#include <stdio.h>

int main() {
    printf("Hello world!\n");

    int* pI;
    pI = (int*)malloc(10*sizeof(int));

    printf("Let's leak a pointer to an array of 10 ints\n");
    int i=0;
    for (i=0; i<9; i++) {
        pI[i] = 303+i;
    }
    int j=0;
    for (j=0; j<9; j++) {
        if (pI[j] != 303+j) printf("  Something strange is happening...\n");
    }

    return 0;
}
