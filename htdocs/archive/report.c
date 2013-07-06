
#include <stdio.h>

int main() {
    char line[4096] = {0};
    printf("Content-type: text/plain\n\n");

    FILE *f = fopen("data.txt", "rt");
    if (f) {
        int c;
        while (EOF != (c = fgetc(f)))
            printf("%c", c);
        fclose(f);
    } else {
        printf("failed to open data\n");
    }

    return 0;
}

