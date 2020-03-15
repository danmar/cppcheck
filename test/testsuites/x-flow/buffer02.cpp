#include <stdlib.h>

// Simple while loop

void f() {
    char* buf = (char*) malloc(9);
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
