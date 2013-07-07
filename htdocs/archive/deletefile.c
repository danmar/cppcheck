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
    if (data[i] != '\0')
        return "invalid delete command";

    return NULL;
}

int main()
{
    const char *query_string = getenv("QUERY_STRING");
    if (query_string == NULL) {
        printf("Content-type: text/plain\r\n\r\n");
        puts("empty/invalid data");
    } else if (NULL != validate(query_string)) {
        printf("Content-type: text/plain\r\n\r\n");
        puts(validate(query_string));
    } else {

        char *data[MAX_RECORDS] = {0};
        if (!readdata(data, MAX_RECORDS)) {
            printf("Content-type: text/plain\r\n\r\n");
            puts("failed to delete file, try again");
        }
        sortdata(data, MAX_RECORDS);

        char name[32] = {0};
        strcpy(name, getname(query_string));
        int index = -1;
        for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
            if (strcmp(name, getname(data[i])) == 0) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            puts("Content-type: text/plain\r\n\r\n");
            puts("file not found");
            return EXIT_SUCCESS;
        }

        if (index >= 0) {
            int deleted = 0;
            FILE *f = fopen("data.txt", "wt");
            if (f != NULL) {
                for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
                    if (i != index)
                        fprintf(f, "%s\n", data[i]);
                    else
                        deleted = 1;
                }
                fclose(f);

                puts("Content-type: text/plain\r\n\r\n");
                puts(deleted ? "file deleted" : "failed to delete file");
            } else {
                puts("Content-type: text/plain\r\n\r\n");
                puts("failed to delete file, try again");
            }
        }
    }

    return EXIT_SUCCESS;
}

