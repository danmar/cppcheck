#include <stdlib.h>

// Simple for loop

void f() {
    char* buf = (char*) malloc(9);
    int i;
    for (i = 0; i < 12; i++) {
        buf[i] = 's';
    }
}

int main() {
    f();
    return 0;
}
