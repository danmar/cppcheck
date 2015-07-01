/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#ifndef checknonreentrantfunctionsH
#define checknonreentrantfunctionsH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include <string>
#include <map>


/// @addtogroup Checks
/// @{

/**
 * @brief Using non reentrant functions that can be replaced by their reentrant versions
 */

class CPPCHECKLIB CheckNonReentrantFunctions : public Check {
public:
    /** This constructor is used when registering the CheckNonReentrantFunctions */
    CheckNonReentrantFunctions() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckNonReentrantFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckNonReentrantFunctions checkNonReentrantFunctions(tokenizer, settings, errorLogger);
        checkNonReentrantFunctions.nonReentrantFunctions();
    }

    /** Check for non reentrant functions */
    void nonReentrantFunctions();

private:

    static std::string generateErrorMessage(const std::string& function);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const;

    static std::string myName() {
        return "Non reentrant functions";
    }

    std::string classInfo() const;
};
/// @}
//---------------------------------------------------------------------------
#endif // checknonreentrantfunctionsH
