#include <stdio.h>

int main() {
    printf("Hello world!\n");

    int* pI;
    pI = (int*)malloc(sizeof(int));
    printf("Let's leak a pointer to int\n");
    *pI = 303;

    return 0;
}
