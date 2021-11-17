#include <stdio.h>

int main() {
    printf("Hello world!\n");

    int* pI = (int*)malloc(sizeof(int));
    *pI=2;
    free(pI);
    printf("Now freeing a pointer twice...\n");
    free(pI);
    printf("Did you notice?\n");
    return 0;
}
