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
#ifndef checkvaargtH
#define checkvaargtH
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
 * @brief Checking for misusage of variable argument lists
 */

class CPPCHECKLIB CheckVaarg : public Check {
public:
    CheckVaarg() : Check("Vaarg") {}

    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

private:
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Check for misusage of variable argument lists:\n"
               "- Wrong parameter passed to va_start()\n"
               "- Reference passed to va_start()\n"
               "- Missing va_end()\n"
               "- Using va_list before it is opened\n"
               "- Subsequent calls to va_start/va_copy()\n";
    }
};

/// @}

//---------------------------------------------------------------------------
#endif // checkvaargtH
