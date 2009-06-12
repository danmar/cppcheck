/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


//---------------------------------------------------------------------------
#ifndef checksecurityH
#define checksecurityH
//---------------------------------------------------------------------------

#include "check.h"

class CheckSecurity : public Check
{
public:
    /** This constructor is used when registering the CheckClass */
    CheckSecurity() : Check()
    { }

    /** This constructor is used when running checks.. */
    CheckSecurity(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckSecurity checkSecurity(tokenizer, settings, errorLogger);
        checkSecurity.readnum();
        checkSecurity.gui();
    }

    /** Reading a number from a stream/FILE */
    void readnum();

    /** Reading Form/GUI data */
    void gui();

private:
    void unvalidatedInput(const Token *tok);

    void getErrorMessages()
    {
        std::cout << "===security===" << "\n";
        unvalidatedInput(0);
    }

    std::string name() const
    {
        return "Security";
    }

    std::string classInfo() const
    {
        return "This is an unfinnished check that will detect unvalidated input.\n";
    }
};

//---------------------------------------------------------------------------
#endif

