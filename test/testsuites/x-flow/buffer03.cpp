#include <stdlib.h>

// Reallocation

void f() {
    char* buf = (char*) malloc(20);
    buf[6] = 'x';
    buf = (char*) realloc(buf, 9);
    int i = 0;
    while (i < 12) {
        buf[i] = 's';
        i++;
    }
}

int main() {
    f();
    return 0;
}
