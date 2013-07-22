
#include "validatexml.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void skipspaces(const char xmldata[], int *pos, int *linenr)
{
    const char *p = &xmldata[*pos];
    while (isspace(*p) || *p == '\r' || *p == '\n') {
        if (strncmp(p,"\r\n",2)==0)
            ++p;
        if (*p == '\r' || *p == '\n')
            ++(*linenr);
        ++p;
    }
    *pos = p - xmldata;
}

int validatexml(const char xmldata[])
{
    if (strncmp(xmldata,"<?xml version=\"1.0\"?>",21)!=0)
        return 1;
    int linenr = 1;
    char elementNames[10][64];  // only 10 element levels handled
    int level = 0;
    for (int pos = 21; xmldata[pos]; pos++) {
        if (strncmp(&xmldata[pos], "\r\n", 2)==0) {
            ++linenr;
            ++pos;
        } else if (xmldata[pos]=='\r' || xmldata[pos]=='\n') {
            ++linenr;
        } else if (xmldata[pos] == '<') {
            ++pos;
            skipspaces(xmldata,&pos,&linenr);
            if (xmldata[pos] == '/') {
                if (level <= 0) {
                    return linenr;
                }
                --level;
                int len = strlen(elementNames[level]);
                if (strncmp(&xmldata[pos+1],elementNames[level],len)!=0 || xmldata[pos+1+len]!='>')
                    return linenr;
                pos += 1 + len;
            } else {
                if (level > 8)
                    return linenr;
                if (!isalpha(xmldata[pos]))
                    return linenr;
                memset(elementNames[level], 0, 64);
                for (int i = 0; i < 64; i++) {
                    if ((xmldata[pos+i]>='a' && xmldata[pos+i]<='z') || xmldata[pos+i] == '-')
                        elementNames[level][i] = xmldata[pos+i];
                    else {
                        pos += i;
                        break;
                    }
                }

                if (!strchr("> \r\n", xmldata[pos]))
                    return linenr;

                level++;

                while (xmldata[pos] != '>') {
                    skipspaces(xmldata,&pos,&linenr);
                    if ((xmldata[pos] >= 'a') && xmldata[pos] <= 'z') {
                        while (((xmldata[pos] >= 'a') && xmldata[pos] <= 'z') || xmldata[pos] == '-')
                            ++pos;
                        if (xmldata[pos++] != '=')
                            return linenr;
                        if (xmldata[pos++] != '\"')
                            return linenr;
                        while (isalnum(xmldata[pos]) || strchr(":-.,",xmldata[pos]))
                            ++pos;
                        if (xmldata[pos++] != '\"')
                            return linenr;
                        if (!strchr("> \r\n", xmldata[pos]))
                            return linenr;
                    } else if (xmldata[pos] != '>') {
                        return linenr;
                    }
                }
            }
        } else if (xmldata[pos] == '>') {
            return linenr;
        }
    }

    if (level != 0)
        return linenr;

    return -1;
}

