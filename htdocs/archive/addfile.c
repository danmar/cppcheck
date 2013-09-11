#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webarchive.h"

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
        generatepage("Internal error: empty/invalid data");
    } else if (strlen(query_string) > MAX_LINE_LEN) {
        char errmsg[100] = {0};
        sprintf(errmsg, "Internal error: data size limit exceeded (%i)", MAX_LINE_LEN);
        generatepage(errmsg);
    } else if (NULL != validate(query_string)) {
        generatepage(validate(query_string));
    } else {
        char data[MAX_LINE_LEN] = {0};
        unencode(query_string, data);

        if (NULL != validate(data)) {
            generatepage(validate(data));
        } else {
            char *olddata[MAX_RECORDS] = {0};
            olddata[0] = data;
            if (!readdata(&olddata[1], MAX_RECORDS-1)) {
                generatepage("Failed to add file (access denied). Try again.");
                return EXIT_SUCCESS;
            }
            sortdata(olddata, MAX_RECORDS);

            FILE *f = fopen("data.txt", "wt");
            if (f == NULL) {
                generatepage("Failed to add file (access denied). Try again.");
                return EXIT_SUCCESS;
            }
            for (int i = 0; i < MAX_RECORDS && olddata[i]; i++)
                fprintf(f, "%s\n", olddata[i]);
            fclose(f);
            generatepage("saved.");
        }
    }

    return EXIT_SUCCESS;
}
