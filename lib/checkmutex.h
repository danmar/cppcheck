/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#ifndef CheckMutexH
#define CheckMutexH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
class Variable;

/// @addtogroup Checks
/// @{

/** @brief %Check input output operations. */
class CPPCHECKLIB CheckMutex : public Check {
public:
    /** @brief This constructor is used when registering CheckMutex */
    CheckMutex() : Check(myName())
    { }

    /** @brief This constructor is used when running checks. */
    CheckMutex(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckMutex checkMutex(tokenizer, settings, errorLogger);

    }

    /** @brief Run checks on the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckMutex checkMutex(tokenizer, settings, errorLogger);

    //    checkMutex.checkCoutCerrMisusage();
        checkMutex.checkMutexUsage();
    //    checkMutex.invalidScanf();
    }


    /** @brief %Check usage of files*/
    void checkMutexUsage();

private:
    void checkFunction(const Token* tok);
    // Reporting errors..
    void checkMutexUsageError(const Token* tok, const std::string& functionName);
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckMutex c(0, settings, errorLogger);
    }

    std::string myName() const {
        return "mutex";
    }

    std::string classInfo() const {
        return "Check input/output operations.\n"
               "* Bad usage of the function 'sprintf' (overlapping data)\n"
               "* Use a file that has been closed\n"
               "* File input/output without positioning results in undefined behaviour\n"
               "* Read to a file that has only been opened for writing (or vice versa)\n"
               "* Using fflush() on an input stream\n"
               "* Invalid usage of output stream. For example: std::cout << std::cout;'\n"
               "* Wrong number of arguments given to 'printf' or 'scanf;'\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif
