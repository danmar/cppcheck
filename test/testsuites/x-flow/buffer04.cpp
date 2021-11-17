#include <stdlib.h>

// Nested loops

void f() {
    char* buf = (char*) malloc(9);
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 6; j++) {
            buf[i*j] = 's';
        }
    }
}

int main() {
    f();
    return 0;
}
