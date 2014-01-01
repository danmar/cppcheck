#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "cppcheck.h"

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
        printf("%s\n", msg.toString(true).c_str());
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

    char code[4096] = {0};
    unencode(data, code);

    if (strlen(code) > 1000) {
        puts("Content-type: text/html\r\n\r\n");
        puts("<html><body>For performance reasons the code must be shorter than 1000 chars.</body></html>");
        return EXIT_SUCCESS;
    }

    FILE *logfile = fopen("democlient.log", "at");
    if (logfile != NULL) {
        fprintf(logfile, "===========================================================\n%s\n", code);
        fclose(logfile);
    }

    puts("Content-type: text/html\r\n\r\n");
    puts("<html><body><pre>");

    CppcheckExecutor cppcheckExecutor;
    cppcheckExecutor.run(code);

    puts("</pre>Done!</body></html>");

    return EXIT_SUCCESS;
}
