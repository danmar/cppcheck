#include <stdio.h>

struct Test {
    int a;
    char st[10];
};

int main() {
    printf("Hello world!\n");

    struct Test ar[10];
    struct Test b;
    printf("Let's index out of bounds \n");
    ar[10].a=10;
    printf("Did you notice?\n");

    printf("There should be 1 error in this run\n");
    return 0;
}
