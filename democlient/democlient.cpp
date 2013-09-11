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
        printf("%s\n", ErrorLogger::ErrorMessage::getXMLHeader(2).c_str());
        cppcheck.check("test.c", code);
        printf("%s\n", ErrorLogger::ErrorMessage::getXMLFooter(2).c_str());
        printf("\n\n");
    }

    void reportOut(const std::string &outmsg) { }
    void reportErr(const ErrorLogger::ErrorMessage &msg) {
        const std::string str(msg.toXML(true,2U));
        printf("%s\n", str.c_str());
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

    const char *lenstr = getenv("CONTENT_LENGTH");
    if (lenstr) {
        int len = std::min(1 + atoi(lenstr), (int)(sizeof(data) - 2));
        fgets(data, len, stdin);
    } else {
        const char *s = getenv("QUERY_STRING");
        std::strncpy(data, s?s:"", sizeof(data)-2);
    }

    char code[4096] = {0};
    unencode(data, code);

    FILE *logfile = fopen("democlient.log", "at");
    if (logfile != NULL) {
        fprintf(logfile, "===========================================================\n%s\n", code);
        fclose(logfile);
    }

    printf("Content-type: text/plain\n\n");

    CppcheckExecutor cppcheckExecutor;
    cppcheckExecutor.run(code);

    return EXIT_SUCCESS;
}
