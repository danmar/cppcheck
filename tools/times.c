#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *replace(char *str, char before, char after)
{
    while (strchr(str,before))
        *strchr(str,before) = after;
    return str;
}

int main()
{
    FILE *f = fopen("times.log", "rt");
    if (!f)
        return 1;

    char lines[0xffff][64] = {0};

    int n = 0;
    float mintime=0.0f, maxtime=0.0f;
    char rev[10] = {0};
    char line[128] = {0};
    while (fgets(line,sizeof(line),f) && n < (sizeof(lines)/sizeof(*lines))) {
        replace(line,'\r','\0');
        replace(line,'\n','\0');
        if (strncmp(line,"HEAD is now at ", 15) == 0) {
            if (rev[0])
                sprintf(lines[n++],"%s\t%.1f\t%.1f", rev, mintime, maxtime);
            strncpy(rev, line+15, 7);
            mintime = 0.0f;
            maxtime = 0.0f;
        }
        if (strncmp(line,"Overall time:",13)==0) {
            float time = atof(line+14);
            if (mintime < 0.1f || time < mintime)
                mintime = time;
            if (time > maxtime)
                maxtime = time;
        }
    }

    while (n > 0)
        printf("%s\n", lines[--n]);

    fclose(f);
    return 0;
}
