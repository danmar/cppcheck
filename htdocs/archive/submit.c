#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webarchive.h"

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

const char *validate(const char *data)
{
    int i;
    if (strncmp(data,"name=",5) != 0)
        return "invalid query string: must start with 'name='";
    i = 5;
    while (isalnum(data[i]))
        i++;
    if (i == 5)
        return "invalid query string: no name";
    if (i > 35)
        return "invalid query string: max name size is 32";
    if (data[i] == '\0')
        return NULL;
    if (data[i] != '&')
        return "invalid query string: only alphanumeric characters are allowed in the name";
    if (strncmp(data+i,"&data=",6)!=0)
        return "invalid query string";
    i += 6;

    // TODO: check XML data..

    return NULL;
}

int main()
{
    const char *query_string = getenv("QUERY_STRING");
    if (query_string == NULL) {
        printf("Content-type: text/plain\r\n\r\n");
        printf("empty/invalid data\n");
    } else if (strlen(query_string) > 1024) {
        printf("Content-type: text/plain\r\n\r\n");
        printf("data size limit exceeded (1024)\n");
    } else if (NULL != validate(query_string)) {
        printf("Content-type: text/plain\r\n\r\n");
        printf("%s\n", validate(query_string));
    } else {
        char data[4096] = {0};
        unencode(query_string, data);

        printf("Content-type: text/plain\r\n\r\n");

        if (NULL != validate(data)) {
            printf("%s\n", validate(data));
        } else {
            char *olddata[MAX_RECORDS] = {0};
            olddata[0] = data;
            readdata(&olddata[1], MAX_RECORDS-1);
            sortdata(olddata, MAX_RECORDS);

            FILE *f = fopen("data.txt", "wt");
            for (int i = 0; i < MAX_RECORDS && olddata[i]; i++)
                fprintf(f, "%s\n", olddata[i]);
            fclose(f);
            printf("saved\n");
        }
    }

    return EXIT_SUCCESS;
}
