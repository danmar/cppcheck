/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "color.h"
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

    void reportOut(const std::string &outmsg, Color) OVERRIDE {
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


