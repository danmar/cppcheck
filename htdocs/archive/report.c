#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "webarchive.h"

#define MAX_RECORDS 1000

void listAll(char **data)
{
    puts("Content-type: text/html\r\n\r\n");
    puts("<input type=\"button\" onclick=\"addFile()\" value=\"Add file\"/>");
    puts("<br /><table border=\"1\"><tr><td><table>");
    for (int i = 0; i < MAX_RECORDS && data[i]; i++) {
        const char *name = getname(data[i]);
        int version = getversion(data[i]);
        if (version < 0)
            version = time(0);
        if (i > 0)
            printf("<tr height=\"1\"><td colspan=\"2\" bgcolor=\"gray\"></td></tr>");
        printf("<tr><td width=\"200\">%s</td>", name);
        printf("<td><input type=\"button\" onclick=\"editFile(\'%s\','%i')\" value=\"Edit\"/>", name, version);
        printf("<input type=\"button\" onclick=\"renameFile(\'%s\','%i')\" value=\"Rename\"/>", name, version);
        if (strcmp(name,"gtk")==0 || strcmp(name,"windows")==0)
            printf("<font color=\"gray\">Delete</font>");
        else
            printf("<input type=\"button\" onclick=\"deleteFile(\'%s\','%i')\" value=\"Delete\"/>&nbsp;</td>", name, version);
        printf("</tr>\n");
    }
    puts("</table></td></tr></table>");
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
    if (query_string == NULL || *query_string == '\0') {
        listAll(data);
    } else if (strncmp(query_string, "name=", 5) == 0 && getname(query_string) != NULL) {
        char name[MAX_NAME_LEN] = {0};
        strcpy(name, getname(query_string));
        listOne(data,name);
    } else {
        puts("Content-type: text/plain\r\n\r\n");
        puts("Invalid query");
    }

    return 0;
}
