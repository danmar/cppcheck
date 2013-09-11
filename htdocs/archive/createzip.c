#include "webarchive.h"
#include "miniz.c"

#include <stdio.h>
#include <stdlib.h>

#define ALL_ZIP   "all.zip"

int main()
{
    mz_zip_archive zip_archive = {0};

    FILE *f = fopen("data.txt", "rt");
    if (f == NULL) {
        generatepage("failed to load data");
        return EXIT_FAILURE;
    }

    int first = 1;
    char line[MAX_LINE_LEN] = {0};
    while (fgets(line,sizeof(line)-2,f)) {
        char data[MAX_LINE_LEN] = {0};
        unencode(line,data);
        const char *xmldata = strstr(data, "&data=");
        xmldata = xmldata ? (xmldata + 6) : "";

        char name[MAX_NAME_LEN+20];
        if (strstr(xmldata, "<rule>"))
            sprintf(name, "archive/%s.rule", getname(line));
        else
            sprintf(name, "archive/%s.cfg", getname(line));

        if (first == 1) {
            first = 0;

            if (!mz_zip_writer_init_file(&zip_archive, ALL_ZIP, 0)) {
                generatepage("internal error: init_file failed");
                return EXIT_FAILURE;
            }
            if (!mz_zip_writer_add_mem_ex(&zip_archive, name, xmldata, strlen(xmldata), NULL, 0U, MZ_BEST_COMPRESSION, 0, 0)) {
                generatepage("internal error: add_mem_ex failed");
                return EXIT_FAILURE;
            }
            if (!mz_zip_writer_finalize_archive(&zip_archive)) {
                generatepage("internal error: finalize_archive failed");
                return EXIT_FAILURE;
            }
            if (!mz_zip_writer_end(&zip_archive)) {
                generatepage("internal error: writer_end failed");
                return EXIT_FAILURE;
            }
        } else if (!mz_zip_add_mem_to_archive_file_in_place(ALL_ZIP, name, xmldata, strlen(xmldata), NULL, 0U, MZ_BEST_COMPRESSION)) {
            generatepage("failed to add data");
            return EXIT_FAILURE;
        }
    }
    fclose(f);

    f = fopen("all.zip","rb");
    if (!f) {
        generatepage("internal error: failed to load zip");
        return EXIT_FAILURE;
    }
    puts("Content-type: application/zip\r\n\r");
    int c;
    while ((c = fgetc(f)) != EOF)
        putc(c,stdout);

    return EXIT_SUCCESS;
}
