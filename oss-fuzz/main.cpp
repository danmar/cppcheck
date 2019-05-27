
#include "cppcheck.h"
#include "type2.h"


class CppcheckExecutor : public ErrorLogger {
private:
    CppCheck cppcheck;

public:
    CppcheckExecutor()
        : ErrorLogger()
        , cppcheck(*this, false) {
        cppcheck.settings().addEnabled("all");
        cppcheck.settings().inconclusive = true;
    }

    void run(const std::string &code) {
        cppcheck.check("test.cpp", code);
    }

    void reportOut(const std::string &outmsg) { }
    void reportErr(const ErrorLogger::ErrorMessage &msg) {}
    void reportProgress(const std::string& filename,
                        const char stage[],
                        const unsigned int value) {}
};


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    if (dataSize < 10000) {
        const std::string code = generateCode2(data, dataSize);
        //std::ofstream fout("code.cpp");
        //fout << code;
        //fout.close();

        CppcheckExecutor cppcheckExecutor;
        cppcheckExecutor.run(code);
    }
    return 0;
}


