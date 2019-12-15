#include <stdlib.h>
#include <string.h>

// Copy string

void f() {
    char* buf = (char*) malloc(9);
    strcpy(buf, "Too big to fit");
}

int main() {
    f();
    return 0;
}
