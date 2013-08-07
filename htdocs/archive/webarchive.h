#include "validatexml.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_RECORDS   1000
#define MAX_LINE_LEN  0xffff
#define MAX_NAME_LEN  32

static void unencode(const char *src, char *dest)
{
    for (; *src; src++, dest++) {
        if (*src == '+')
            *dest = ' ';
        else if (*src == '%') {
            int code;
            if (sscanf(src+1, "%2x", &code) != 1)
                code = '?';
            if (code == '%' && isxdigit(src[3]) && isxdigit(src[4])) {
                src += 2;
                sscanf(src+1, "%2x", &code);
            }
            *dest = code;
            src += 2;
        } else
            *dest = *src;
    }
    *dest = '\0';
}

int readdata(char ** const data, int sz)
{
    FILE *f = fopen("data.txt", "rt");
    if (!f)
        return 0;  // failed

    char line[MAX_LINE_LEN] = {0};
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

int getversion(const char *data)
{
    const char *name = getname(data);
    if (name == NULL)
        return 0; // invalid string => 0
    data = data + strlen("name=") + strlen(name);
    if (strncmp(data,"&version=",9) != 0)
        return 1;  // no version => 1
    data = data + 9;
    int ret = 0;
    while (isdigit(*data)) {
        ret = ret * 10 + *data - '0';
        data++;
    }
    if (*data != '\0' && *data != '&')
        return -1;  // invalid version => -1
    return ret;
}

void sortdata(char ** const data, int sz)
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

void generatepage(const char msg[])
{
    puts("Content-type: text/html\r\n\r\n");
    puts("<html>");
    puts("<head><script type=\"text/javascript\">");
    puts("function ok() { window.location = \"http://cppcheck.sourceforge.net/archive/\"; }");
    puts("</script></head>");
    puts("<body>");
    puts(msg);
    puts("<br /><input type=\"button\" value=\"OK\" onclick=\"ok()\"></body></html>");
}


const char *validate_name_version(const char *data)
{
    int i = 0;

    // name
    if (strncmp(data,"name=",5) != 0)
        return "invalid query string: must start with 'name='";
    i += 5;
    if (!isalnum(data[i]))
        return "invalid query string: no name / invalid character in name";
    while (isalnum(data[i]))
        i++;
    if (i > 35)
        return "invalid query string: max name size is 32";

    // version
    if (strncmp(&data[i], "&version=", 9) != 0)
        return "invalid query string: 'version=' not seen at the expected location";
    i += strlen("&version=");
    if (!isdigit(data[i]))
        return "invalid query string: version must consist of digits 0-9";
    while (isdigit(data[i]))
        i++;

    // end
    if (data[i] != '\0')
        return "invalid query";

    return NULL;
}

const char *validate_name_version_data(const char *data)
{
    int i = 0;

    // name
    if (strncmp(data,"name=",5) != 0)
        return "invalid query string: must start with 'name='";
    i += 5;
    if (!isalnum(data[i]))
        return "invalid query string: no name / invalid character in name";
    while (isalnum(data[i]))
        i++;
    if (i > 35)
        return "invalid query string: max name size is 32";

    // version
    if (strncmp(&data[i], "&version=", 9) != 0)
        return "invalid query string: 'version=' not seen at the expected location";
    i += strlen("&version=");
    if (!isdigit(data[i]))
        return "invalid query string: version must consist of digits 0-9";
    while (isdigit(data[i]))
        i++;

    // filedata
    if (strncmp(data+i, "&data=", 6) != 0)
        return "invalid query string: 'data=' not seen at the expected location";
    i += 6;

    // validate xml
    char xmldata[strlen(data+i)];
    memset(xmldata, 0, strlen(data+i));
    unencode(data+i, xmldata);
    const int badline = validatexml(xmldata);
    if (badline >= 1) {
        static char buf[256];
        sprintf(buf, "Invalid query: Invalid XML at line %i\n", badline);
        return buf;
    }

    // No error
    return NULL;
}
