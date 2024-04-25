/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "cppcheck.h"
#include "type2.h"

#ifdef NO_FUZZ
#include <cstdlib>
#include <fstream>
#include <sstream>
#endif

enum class Color;

class DummyErrorLogger : public ErrorLogger {
public:
    void reportOut(const std::string& /*outmsg*/, Color /*c*/) override {}
    void reportErr(const ErrorMessage& /*msg*/) override {}
    void reportProgress(const std::string& /*filename*/,
                        const char /*stage*/[],
                        const std::size_t /*value*/) override {} // FN
};

static DummyErrorLogger s_errorLogger;

static void doCheck(const std::string& code)
{
    CppCheck cppcheck(s_errorLogger, false, nullptr);
    cppcheck.settings().addEnabled("all");
    cppcheck.settings().certainty.setEnabled(Certainty::inconclusive, true);
    cppcheck.check("test.cpp", code);
}

#ifndef NO_FUZZ
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    if (dataSize < 10000) {
        const std::string code = generateCode2(data, dataSize);
        doCheck(code);
    }
    return 0;
}
#else
int main(int argc, char * argv[])
{
    if (argc != 2)
        return EXIT_FAILURE;

    std::ifstream f(argv[1]);
    if (!f.is_open())
        return EXIT_FAILURE;

    std::ostringstream oss;
    oss << f.rdbuf();

    if (!f.good())
        return EXIT_FAILURE;

    const std::string code = oss.str();
    doCheck(code);

    return EXIT_SUCCESS;
}
#endif


