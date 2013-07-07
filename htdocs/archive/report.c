
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_RECORDS 1000

int readdata(char * * const data, int sz)
{
    FILE *f = fopen("data.txt", "rt");
    if (!f)
        return 0;  // failed

    char line[10000] = {0};
    int i = 0;
    while (i < sz && fgets(line,sizeof(line)-2,f)) {
        if (strncmp(line, "name=", 5) == 0) {
            data[i] = malloc(strlen(line));
            strcpy(data[i], line);
            i++;
        }
    }
    fclose(f);

    return 1;  // success
}

const char * getname(const char *data) {
    static char name[32];
    if (strncmp(data,"name=",5) != 0)
        return NULL;
    int i = 0;
    while (i < sizeof(name) && data[i+5] && data[i+5] != '&') {
        name[i] = data[i+5];
        i++;
    }
    if (i >= sizeof(name))
        return NULL;
    while (i < sizeof(name))
        name[i++] = 0;
    return name;
}

int main() {
    char *data[MAX_RECORDS] = {0};

    // read
    if (!readdata(data, MAX_RECORDS)) {
        printf("Internal error: failed to load data");
    }

    // sort
    for (int i = 1; i < MAX_RECORDS && data[i]; i++) {
        if (strcmp(data[i-1],data[i]) > 0) {
            char *p = data[i-1];
            data[i-1] = data[i];
            data[i] = p;
            if (i > 1)
                i--;
        }
    }

    // output
    printf("Content-type: text/html\r\n\r\n");
    printf("<html><meta http-equiv=\"Pragma\" content=\"no-cache\"><body>\n");
    printf("<table>\n");
    for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
        const char *name = getname(data[i]);
        printf("<tr><td>%s</td><td>Delete this</td></tr>\n", name);
    }
    printf("<table>\n");
    printf("</body></html>");

    return 0;
}

