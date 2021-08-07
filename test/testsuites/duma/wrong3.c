#include <stdio.h>

int main() {
    printf("Hello world!\n");

    int* pI = (int*)malloc(sizeof(int));
    int j;
    printf("Now reading uninitialized memory\n");
    j = *pI+2;
    printf("Did you notice? (value was %i)\n",j);
    free(pI);
    printf("(No memory leak here)\n");

    int* pJ;
    printf("Now writing to uninitialized pointer\n");
    *pJ = j;
    printf("Did you notice?\n");

    // valgrind reports 8, but that's ok
    printf("There should be 2 errors in this run\n");
    return 0;
}
