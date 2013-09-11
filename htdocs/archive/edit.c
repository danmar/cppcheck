#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webarchive.h"

int main()
{
    const char *query_string = getenv("QUERY_STRING");
    if (query_string == NULL) {
        generatepage("Internal error: invalid request");
        return EXIT_SUCCESS;
    }

    if (NULL != validate_name_version(query_string)) {
        generatepage(validate_name_version(query_string));
        return EXIT_SUCCESS;
    }

    char *data[MAX_RECORDS] = {0};
    if (!readdata(data, MAX_RECORDS)) {
        generatepage("Failed to read file data");
        return EXIT_SUCCESS;
    }

    char name[MAX_NAME_LEN] = {0};
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

    int version = getversion(data[index]);
    if (version < 1)
        version = 1;

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
    puts("  <form action=\"http://cppcheck.sourceforge.net/cgi-bin/setfiledata.cgi\" method=\"get\">");
    printf("    <textarea name=\"name\" cols=\"30\" rows=\"1\" readonly>%s</textarea>\n",name);
    printf("    <textarea name=\"version\" cols=\"30\" rows=\"1\" readonly>%i</textarea><br>\n",1+version);
    printf("    <textarea name=\"data\" cols=\"60\" rows=\"20\" maxsize=\"512\">%s</textarea><br>\n",olddata);
    puts("    <input type=\"submit\" value=\"Save\">");
    puts("  </form>");
    puts("</body>");
    puts("</html>");

    return EXIT_SUCCESS;
}
