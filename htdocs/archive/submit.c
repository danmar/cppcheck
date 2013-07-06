#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void unencode(const char *src, char *dest)
{
    for (; *src; src++, dest++) {
        if (*src == '+')
            *dest = ' ';
        else if (*src == '%') {
            int code;
            if (sscanf(src+1, "%2x", &code) != 1)
                code = '?';
            *dest = code;
            src += 2;
        } else
            *dest = *src;
    }
    *dest = '\0';
}

int main()
{
    const char *query_string = getenv("QUERY_STRING");
    if (query_string == NULL) {
        printf("Content-type: text/plain\n\n");
        printf("empty/invalid data\n");
    } else if (strlen(query_string) > 1024) {
        printf("Content-type: text/plain\n\n");
        printf("data size limit exceeded (1024)\n");
    } else {
        char data[4096] = {0};
        unencode(query_string, data);

        FILE *f = fopen("data.txt", "a");
        fprintf(f,"%s\n",data);
        fclose(f);

        printf("Content-type: text/plain\n\n");
        printf("saved\n");
    }

    return EXIT_SUCCESS;
}


