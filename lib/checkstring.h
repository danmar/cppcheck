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
#ifndef checkstringH
#define checkstringH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Settings;
class Tokenizer;

/// @addtogroup Checks
/// @{


/** @brief Detect misusage of C-style strings and related standard functions */

class CPPCHECKLIB CheckString : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckString() : Check("String") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Detect misusage of C-style strings:\n"
               "- overlapping buffers passed to sprintf as source and destination\n"
               "- incorrect length arguments for 'substr' and 'strncmp'\n"
               "- suspicious condition (runtime comparison of string literals)\n"
               "- suspicious condition (string/char literals as boolean)\n"
               "- suspicious comparison of a string literal with a char\\* variable\n"
               "- suspicious comparison of '\\0' with a char\\* variable\n"
               "- overlapping strcmp() expression\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkstringH
