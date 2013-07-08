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
        return "invalid edit command";

    return NULL;
}

int main()
{
    const char *query_string = getenv("QUERY_STRING");
    if (query_string == NULL) {
        generatepage("Internal error: invalid request");
    } else if (NULL != validate(query_string)) {
        generatepage(validate(query_string));
    } else {
        char *data[MAX_RECORDS] = {0};
        if (!readdata(data, MAX_RECORDS)) {
            generatepage("Failed to read file data");
            return EXIT_SUCCESS;
        }

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
            generatepage("File not found");
            return EXIT_SUCCESS;
        }

        const char *olddata = strstr(data[index], "&data=");
        if (olddata) {
            char *temp = malloc(strlen(olddata+6));
            unencode(olddata+6, temp);
            olddata = temp;
        } else
            olddata = "";

        puts("Content-type: text/html\r\n\r\n");
        puts("<html>");
        puts("<body>");
        puts("  <form action=\"http://cppcheck.sf.net/cgi-bin/setfiledata.cgi\" method=\"get\">");
        printf("    <textarea name=\"name\" cols=\"30\" rows=\"1\" readonly>abcd</textarea><br>\n",name);
        printf("    <textarea name=\"data\" cols=\"60\" rows=\"20\" maxsize=\"512\">123</textarea><br>\n",olddata);
        puts("    <input type=\"submit\" value=\"Save\">");
        puts("  </form>");
        puts("</body>");
        puts("</html>");
    }

    return EXIT_SUCCESS;
}

