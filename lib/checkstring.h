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
#ifndef checkstringH
#define checkstringH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

/// @addtogroup Checks
/// @{


/** @brief Detect misusage of C-style strings and related standard functions */

class CPPCHECKLIB CheckString : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckString() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckString(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckString checkString(tokenizer, settings, errorLogger);

        // Checks
        checkString.strPlusChar();
        checkString.checkSuspiciousStringCompare();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckString checkString(tokenizer, settings, errorLogger);

        // Checks
        checkString.checkIncorrectStringCompare();
        checkString.checkAlwaysTrueOrFalseStringCompare();
        checkString.sprintfOverlappingData();
    }

    /** @brief str plus char (unusual pointer arithmetic) */
    void strPlusChar();

    /** @brief %Check for using bad usage of strncmp and substr */
    void checkIncorrectStringCompare();

    /** @brief %Check for comparison of a string literal with a char* variable */
    void checkSuspiciousStringCompare();

    /** @brief %Check for suspicious code that compares string literals for equality */
    void checkAlwaysTrueOrFalseStringCompare();

    /** @brief %Check for overlapping source and destination passed to sprintf() */
    void sprintfOverlappingData();

private:
    void sprintfOverlappingDataError(const Token *tok, const std::string &varname);
    void strPlusCharError(const Token *tok);
    void incorrectStringCompareError(const Token *tok, const std::string& func, const std::string &string);
    void incorrectStringBooleanError(const Token *tok, const std::string& string);
    void alwaysTrueFalseStringCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void alwaysTrueStringVariableCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void suspiciousStringCompareError(const Token* tok, const std::string& var);
    void suspiciousStringCompareError_char(const Token* tok, const std::string& var);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckString c(0, settings, errorLogger);

        c.sprintfOverlappingDataError(0, "varname");
        c.strPlusCharError(0);
        c.incorrectStringCompareError(0, "substr", "\"Hello World\"");
        c.suspiciousStringCompareError(0, "foo");
        c.suspiciousStringCompareError_char(0, "foo");
        c.incorrectStringBooleanError(0, "\"Hello World\"");
        c.alwaysTrueFalseStringCompareError(0, "str1", "str2");
        c.alwaysTrueStringVariableCompareError(0, "varname1", "varname2");
    }

    static std::string myName() {
        return "String";
    }

    std::string classInfo() const {
        return "Detect misusage of C-style strings:\n"
               "- overlapping buffers passed to sprintf as source and destination\n"
               "- incorrect length arguments for 'substr' and 'strncmp'\n"
               "- suspicious condition (runtime comparison of string literals)\n"
               "- suspicious condition (string literals as boolean)\n"
               "- suspicious comparison of a string literal with a char* variable\n"
               "- suspicious comparison of '\\0' with a char* variable\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkstringH
