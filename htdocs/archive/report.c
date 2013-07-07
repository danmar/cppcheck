
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "webarchive.h"

#define MAX_RECORDS 1000

int main() {
    char *data[MAX_RECORDS] = {0};

    // read
    if (!readdata(data, MAX_RECORDS)) {
        printf("Internal error: failed to load data");
    }

    // sort
    sortdata(data,MAX_RECORDS);

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

