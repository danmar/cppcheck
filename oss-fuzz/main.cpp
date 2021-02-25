
#include "cppcheck.h"
#include "type2.h"


class CppcheckExecutor : public ErrorLogger {
private:
    CppCheck cppcheck;

public:
    CppcheckExecutor()
        : ErrorLogger()
        , cppcheck(*this, false, nullptr) {
        cppcheck.settings().addEnabled("all");
        cppcheck.settings().certainty.setEnabled(Certainty::inconclusive, true);
    }

    void run(const std::string &code) {
        cppcheck.check("test.cpp", code);
    }

    void reportOut(const std::string &outmsg) OVERRIDE {
        (void)outmsg;
    }
    void reportErr(const ErrorMessage &msg) OVERRIDE {
        (void)msg;
    }
    void reportProgress(const std::string& filename,
                        const char stage[],
                        const std::size_t value) OVERRIDE {
        (void)filename;
        (void)stage;
        (void)value;
    }
    void bughuntingReport(const std::string &str) OVERRIDE {
        (void)str;
    }
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


