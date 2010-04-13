/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#ifndef CheckDangerousFunctionsH
#define CheckDangerousFunctionsH
//---------------------------------------------------------------------------

#include "check.h"

/// @addtogroup Checks
/// @{

/**
 * @brief Using dangerous functions that are always insecure to use.
 */

class CheckDangerousFunctions : public Check
{
public:
    /** This constructor is used when registering the CheckDangerousFunctions */
    CheckDangerousFunctions() : Check()
    { }

    /** This constructor is used when running checks. */
    CheckDangerousFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckDangerousFunctions checkDangerousFunctions(tokenizer, settings, errorLogger);
        if (settings->_checkCodingStyle)
        {
            checkDangerousFunctions.dangerousFunctions();
        }
    }

    /** Check for dangerous functions */
    void dangerousFunctions();

private:
    /** Report Error : Using dangerous function 'mktemp' */
    void dangerousFunctionmktemp(const Token *tok);
    /** Report Error : Using dangerous function 'gets' */
    void dangerousFunctiongets(const Token *tok);
    /** Report Error : Using dangerous function 'scanf' */
    void dangerousFunctionscanf(const Token *tok);

    void getErrorMessages()
    {
        dangerousFunctionmktemp(0);
        dangerousFunctiongets(0);
        dangerousFunctionscanf(0);
    }

    std::string name() const
    {
        return "Deprecated functions";
    }

    std::string classInfo() const
    {
        return "Warn if any of these deprecated functions are used:\n"
               "* mktemp\n"
               "* gets\n"
               "* scanf\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif

