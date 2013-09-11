#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webarchive.h"

const char *validate(const char *data)
{
    int i;

    // name1
    if (strncmp(data,"name1=",6) != 0)
        return "invalid query string: must start with 'name1='";
    i = 6;
    while (isalnum(data[i]))
        i++;
    if (i == 6)
        return "invalid query string: no name1";
    if (i > 36)
        return "invalid query string: max name1 size is 30";

    // name2
    const int i1 = i;
    if (strncmp(data+i,"&name2=",7) != 0)
        return "invalid query string: no name2";
    i += 7;
    if (!isalnum(data[i]))
        return "invalid query string: empty name2";
    while (isalnum(data[i]))
        i++;
    if (i - i1 > 37)
        return "invalid query string: max name2 size is 30";

    if (data[i] != '\0')
        return "invalid query string: invalid char in name2";

    return NULL;
}

int main()
{
    const char *query_string = getenv("QUERY_STRING");
    if (query_string == NULL) {
        generatepage("Internal error: empty/invalid data");
    } else if (NULL != validate(query_string)) {
        generatepage(validate(query_string));
    } else {

        char *data[MAX_RECORDS] = {0};
        if (!readdata(data, MAX_RECORDS)) {
            generatepage("access failed, try again");
            return EXIT_SUCCESS;
        }
        sortdata(data, MAX_RECORDS);

        // Get name1 and name2..
        char buf[strlen(query_string)];
        strcpy(buf, query_string);
        const char * const name1 = strstr(buf, "name1=") + 6;
        const char * const name2 = strstr(buf, "name2=") + 6;
        for (int i = 0; buf[i] != '\0'; i++) {
            if (buf[i] == '&')
                buf[i] = '\0';
        }
        int index = -1;
        for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
            if (strcmp(name1, getname(data[i])) == 0) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            generatepage("file not found");
            return EXIT_SUCCESS;
        }

        FILE *f = fopen("data.txt", "wt");
        if (f == NULL) {
            generatepage("failed to rename file (access denied), try again");
            return EXIT_SUCCESS;
        }

        for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
            if (i == index)
                fprintf(f, "name=%s%s\n", name2, data[i]+5+strlen(name1));
            else
                fprintf(f, "%s\n", data[i]);
        }

        fclose(f);

        generatepage("file renamed");
    }

    return EXIT_SUCCESS;
}
