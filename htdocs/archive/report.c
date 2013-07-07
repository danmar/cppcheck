#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "webarchive.h"

#define MAX_RECORDS 1000

void listAll(char **data)
{
    puts("Content-type: text/html\r\n\r\n");
    puts("<html><meta http-equiv=\"Pragma\" content=\"no-cache\"><body>\n");
    puts("<table>\n");
    for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
        const char *name = getname(data[i]);
        printf("<tr><td>%s</td><td>Delete this</td></tr>\n", name);
    }
    puts("<table>\n");
    puts("</body></html>");
}

void listOne(char **data, const char name[])
{
    int index = -1;
    for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
        if (strcmp(getname(data[i]), name)==0) {
            index = i;
            break;
        }
    }

    puts("Content-type: text/plain\r\n\r\n");
    puts((index == -1) ? "Not found" : data[index]);
}

int main()
{
    char *data[MAX_RECORDS] = {0};

    // read
    if (!readdata(data, MAX_RECORDS)) {
        puts("Content-type: text/html\r\n\r\n");
        puts("Internal error: failed to load data");
        return 0;
    }

    // sort
    sortdata(data,MAX_RECORDS);

    const char *query_string = getenv("QUERY_STRING");
    if (query_string == NULL) {
        listAll(data);
    } else if (strncmp(query_string, "name=", 5) == 0 && getname(query_string) != NULL) {
        char name[32] = {0};
        strcpy(name, getname(query_string));
        listOne(data,name);
    } else {
        puts("Content-type: text/plain\r\n\r\n");
        puts("Invalid query");
    }

    return 0;
}
