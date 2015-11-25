#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "cppcheck.h"
#include "version.h"

static void unencode(const char *src, char *dest)
{
    for (; *src; src++, dest++) {
        if (*src == '+')
            *dest = ' ';
        else if (*src == '%') {
            int code;
            if (sscanf(src+1, "%2x", &code) != 1)
                code = '?';
            *dest = code;
            src += 2;
        } else
            *dest = *src;
    }
    *dest = '\0';
}


class CppcheckExecutor : public ErrorLogger {
private:
    const std::time_t stoptime;
    CppCheck cppcheck;

public:
    CppcheckExecutor()
        : ErrorLogger()
        , stoptime(std::time(NULL)+2U)
        , cppcheck(*this,false) {
        cppcheck.settings().addEnabled("all");
        cppcheck.settings().inconclusive = true;
    }

    void run(const char code[]) {
        cppcheck.check("test.c", code);
    }

    void reportOut(const std::string &outmsg) { }
    void reportErr(const ErrorLogger::ErrorMessage &msg) {
        const std::string s = msg.toString(true);

        printf("%s\n", s.c_str());

        FILE *logfile = fopen("democlient.log", "at");
        if (logfile != NULL) {
            fprintf(logfile, "%s\n", s.c_str());
            fclose(logfile);
        }
    }

    void reportProgress(const
                        std::string &filename,
                        const char stage[],
                        const unsigned int value) {
        if (std::time(NULL) >= stoptime) {
            printf("time to analyse the "
                   "code is more than 1 "
                   "second. terminating."
                   "\n\n");
            cppcheck.terminate();
        }
    }
};


int main()
{
    char data[4096] = {0};

    const char *query_string = getenv("QUERY_STRING");
    if (query_string)
        std::strncpy(data, query_string, sizeof(data)-2);

    const char *lenstr = getenv("CONTENT_LENGTH");
    if (lenstr) {
        int len = std::min(1 + atoi(lenstr), (int)(sizeof(data) - 2));
        fgets(data, len, stdin);
    }

    if (data[4000] != '\0') {
        puts("Content-type: text/html\r\n\r\n");
        puts("<html><body>For performance reasons the code must be shorter than 1000 chars.</body></html>");
        return EXIT_SUCCESS;
    }

    const char *pdata = data;
    if (std::strncmp(pdata, "code=", 5)==0)
        pdata += 5;

    char code[4096] = {0};
    unencode(pdata, code);

    FILE *logfile = fopen("democlient.log", "at");
    if (logfile != NULL) {
        fprintf(logfile, "===========================================================\n%s\n", code);
        fclose(logfile);
    }

    puts("Content-type: text/html\r\n\r\n");
    puts("<html><body>Cppcheck version " CPPCHECK_VERSION_STRING "<pre>");

    CppcheckExecutor cppcheckExecutor;
    cppcheckExecutor.run(code);

    puts("</pre>Done!</body></html>");

    return EXIT_SUCCESS;
}
