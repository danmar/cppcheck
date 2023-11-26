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
#ifndef checkboolH
#define checkboolH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Settings;
class Tokenizer;

/// @addtogroup Checks
/// @{


/** @brief checks dealing with suspicious usage of boolean type (not for evaluating conditions) */

class CPPCHECKLIB CheckBool : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckBool() : Check("Boolean") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Boolean type checks\n"
               "- using increment on boolean\n"
               "- comparison of a boolean expression with an integer other than 0 or 1\n"
               "- comparison of a function returning boolean value using relational operator\n"
               "- comparison of a boolean value with boolean value using relational operator\n"
               "- using bool in bitwise expression\n"
               "- pointer addition in condition (either dereference is forgot or pointer overflow is required to make the condition false)\n"
               "- Assigning bool value to pointer or float\n"
               "- Returning an integer other than 0 or 1 from a function with boolean return value\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkboolH
