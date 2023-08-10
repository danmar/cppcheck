/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

//---------------------------------------------------------------------------
#ifndef checkioH
#define checkioH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class Settings;
class ErrorLogger;
class Tokenizer;

/// @addtogroup Checks
/// @{

/** @brief %Check input output operations. */
class CPPCHECKLIB CheckIO : public Check {
    friend class TestIO;

public:
    /** @brief This constructor is used when registering CheckIO */
    CheckIO() : Check("IO using format string") {}

private:
    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Check format string input/output operations.\n"
               "- Bad usage of the function 'sprintf' (overlapping data)\n"
               "- Missing or wrong width specifiers in 'scanf' format string\n"
               "- Use a file that has been closed\n"
               "- File input/output without positioning results in undefined behaviour\n"
               "- Read to a file that has only been opened for writing (or vice versa)\n"
               "- Repositioning operation on a file opened in append mode\n"
               "- The same file can't be open for read and write at the same time on different streams\n"
               "- Using fflush() on an input stream\n"
               "- Invalid usage of output stream. For example: 'std::cout << std::cout;'\n"
               "- Wrong number of arguments given to 'printf' or 'scanf;'\n";
    }

    // for testing
    static void checkWrongPrintfScanfArguments(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkioH
