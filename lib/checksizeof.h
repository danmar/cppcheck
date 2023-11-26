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
#ifndef checksizeofH
#define checksizeofH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Settings;
class Tokenizer;

/// @addtogroup Checks
/// @{

/** @brief checks on usage of sizeof() operator */

class CPPCHECKLIB CheckSizeof : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckSizeof() : Check("Sizeof") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer& tokenizer, ErrorLogger* errorLogger) override;

    void getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const override;

    std::string classInfo() const override {
        return "sizeof() usage checks\n"
               "- sizeof for array given as function argument\n"
               "- sizeof for numeric given as function argument\n"
               "- using sizeof(pointer) instead of the size of pointed data\n"
               "- look for 'sizeof sizeof ..'\n"
               "- look for calculations inside sizeof()\n"
               "- look for function calls inside sizeof()\n"
               "- look for suspicious calculations with sizeof()\n"
               "- using 'sizeof(void)' which is undefined\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checksizeofH
