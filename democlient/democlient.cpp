#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "cppcheck.h"
#include "version.h"

static void unencode(const char *src, char *dest)
{
    for (; *src; src++, dest++) {
        if (*src == '+')
            *dest = ' ';
        else if (*src == '%') {
            unsigned int code;
            if (std::sscanf(src+1, "%2x", &code) != 1)
                code = '?';
            *dest = code;
            src += 2;
        } else
            *dest = *src;
    }
    *dest = '\0';
}

static FILE *logfile = nullptr;

class CppcheckExecutor : public ErrorLogger {
private:
    const std::time_t stoptime;
    CppCheck cppcheck;

public:
    CppcheckExecutor()
        : ErrorLogger()
        , stoptime(std::time(nullptr)+2U)
        , cppcheck(*this, false) {
        cppcheck.settings().addEnabled("all");
        cppcheck.settings().inconclusive = true;
    }

    void run(const char code[]) {
        cppcheck.check("test.cpp", code);
    }

    void reportOut(const std::string &outmsg) { }
    void reportErr(const ErrorLogger::ErrorMessage &msg) {
        const std::string s = msg.toString(true);

        std::cout << s << std::endl;

        if (logfile != nullptr)
            std::fprintf(logfile, "%s\n", s.c_str());
    }

    void reportProgress(const std::string& filename,
                        const char stage[],
                        const unsigned int value) {
        if (std::time(nullptr) >= stoptime) {
            std::cout << "Time to analyse the code exceeded 2 seconds. Terminating.\n\n";
            cppcheck.terminate();
        }
    }
};


int main()
{
    std::cout << "Content-type: text/html\r\n\r\n"
              << "<!DOCTYPE html>\n";

    char data[4096] = {0};

    const char *query_string = std::getenv("QUERY_STRING");
    if (query_string)
        std::strncpy(data, query_string, sizeof(data)-2);

    const char *lenstr = std::getenv("CONTENT_LENGTH");
    if (lenstr) {
        int len = std::min(1 + std::atoi(lenstr), (int)(sizeof(data) - 2));
        std::fgets(data, len, stdin);
    }

    if (data[4000] != '\0') {
        std::cout << "<html><body>For performance reasons the code must be shorter than 1000 chars.</body></html>";
        return EXIT_SUCCESS;
    }

    const char *pdata = data;
    if (std::strncmp(pdata, "code=", 5)==0)
        pdata += 5;

    char code[4096] = {0};
    unencode(pdata, code);

    logfile = std::fopen("democlient.log", "at");
    if (logfile != nullptr)
        std::fprintf(logfile, "===========================================================\n%s\n", code);

    std::cout << "<html><body>Cppcheck " CPPCHECK_VERSION_STRING "<pre>";

    CppcheckExecutor cppcheckExecutor;
    cppcheckExecutor.run(code);

    std::fclose(logfile);

    std::cout << "</pre>Done!</body></html>";

    return EXIT_SUCCESS;
}
