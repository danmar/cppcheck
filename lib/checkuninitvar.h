/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#ifndef checkuninitvarH
#define checkuninitvarH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"

class Token;

/// @addtogroup Checks
/// @{


/** @brief Checking for uninitialized variables */

class CheckUninitVar : public Check
{
public:
    /** @brief This constructor is used when registering the CheckUninitVar */
    CheckUninitVar() : Check(myName())
    { }

    /** @brief This constructor is used when running checks. */
    CheckUninitVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        (void)tokenizer;
        (void)settings;
        (void)errorLogger;
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckUninitVar checkUninitVar(tokenizer, settings, errorLogger);
        checkUninitVar.executionPaths();
    }

    /**
     * @brief Uninitialized variables: analyse functions to see how they work with uninitialized variables
     * @param tokens [in] the token list
     * @param func [out] names of functions that don't handle uninitialized variables well. the function names are added to the set. No clearing is made.
     */
    void analyse(const Token * tokens, std::set<std::string> &func) const;

    /** Save analysis results */
    void saveAnalysisData(const std::set<std::string> &data) const;

    /** @brief new type of check: check execution paths */
    void executionPaths();

    void uninitstringError(const Token *tok, const std::string &varname, bool strncpy_);
    void uninitdataError(const Token *tok, const std::string &varname);
    void uninitvarError(const Token *tok, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
    {
        CheckUninitVar c(0, settings, errorLogger);

        // error
        c.uninitstringError(0, "varname", true);
        c.uninitdataError(0, "varname");
        c.uninitvarError(0, "varname");
    }

    std::string myName() const
    {
        return "Uninitialized variables";
    }

    std::string classInfo() const
    {
        return "Uninitialized variables\n"
               "* using uninitialized variables and data\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif

