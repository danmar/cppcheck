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

#include "cppcheck.h"
#include "type2.h"

enum class Color;

class DummyErrorLogger : public ErrorLogger {
public:
    void reportOut(const std::string&, Color) override {}
    void reportErr(const ErrorMessage&) override {}
    void reportProgress(const std::string&,
                        const char[],
                        const std::size_t) override {}
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    if (dataSize < 10000) {
        const std::string code = generateCode2(data, dataSize);

        DummyErrorLogger errorLogger;
        CppCheck cppcheck(errorLogger, false, nullptr);
        cppcheck.settings().addEnabled("all");
        cppcheck.settings().certainty.setEnabled(Certainty::inconclusive, true);
        cppcheck.check("test.cpp", code);
    }
    return 0;
}


