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
#ifndef checkfunctionsH
#define checkfunctionsH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Settings;
class Tokenizer;

/// @addtogroup Checks
/// @{

/**
 * @brief Check for bad function usage
 */

class CPPCHECKLIB CheckFunctions : public Check {
public:
    /** This constructor is used when registering the CheckFunctions */
    CheckFunctions() : Check("Check function usage") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Check function usage:\n"
               "- missing 'return' in non-void function\n"
               "- return value of certain functions not used\n"
               "- invalid input values for functions\n"
               "- Warn if a function is called whose usage is discouraged\n"
               "- memset() third argument is zero\n"
               "- memset() with a value out of range as the 2nd parameter\n"
               "- memset() with a float as the 2nd parameter\n"
               "- copy elision optimization for returning value affected by std::move\n"
               "- use memcpy()/memset() instead of for loop\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkfunctionsH
