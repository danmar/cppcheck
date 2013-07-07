#include <string.h>
#include <ctype.h>

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
            int len = strlen(line);
            while (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == '\t' || line[len-1] == ' ')
                line[--len] = '\0';
            data[i] = malloc(len);
            strcpy(data[i], line);
            i++;
        }
    }
    fclose(f);

    return 1;  // success
}

const char * getname(const char *data)
{
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

void sortdata(char * * const data, int sz)
{
    for (int i = 1; i < sz && data[i]; i++) {
        if (strcmp(data[i-1], data[i]) > 0) {
            char *p = data[i-1];
            data[i-1] = data[i];
            data[i] = p;
            if (i >= 2)
                i -= 2;
        }
    }
}
