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
#ifndef checkstringH
#define checkstringH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "tokenize.h"

#include <string>

class ErrorLogger;
class Settings;
class Token;

/// @addtogroup Checks
/// @{


/** @brief Detect misusage of C-style strings and related standard functions */

class CPPCHECKLIB CheckString : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckString() : Check(myName()) {}

    /** @brief This constructor is used when running checks. */
    CheckString(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override {
        CheckString checkString(&tokenizer, tokenizer.getSettings(), errorLogger);

        // Checks
        checkString.strPlusChar();
        checkString.checkSuspiciousStringCompare();
        checkString.stringLiteralWrite();
        checkString.overlappingStrcmp();
        checkString.checkIncorrectStringCompare();
        checkString.sprintfOverlappingData();
        checkString.checkAlwaysTrueOrFalseStringCompare();
    }

    /** @brief undefined behaviour, writing string literal */
    void stringLiteralWrite();

    /** @brief str plus char (unusual pointer arithmetic) */
    void strPlusChar();

    /** @brief %Check for using bad usage of strncmp and substr */
    void checkIncorrectStringCompare();

    /** @brief %Check for comparison of a string literal with a char* variable */
    void checkSuspiciousStringCompare();

    /** @brief %Check for suspicious code that compares string literals for equality */
    void checkAlwaysTrueOrFalseStringCompare();

    /** @brief %Check for overlapping strcmp() */
    void overlappingStrcmp();

    /** @brief %Check for overlapping source and destination passed to sprintf() */
    void sprintfOverlappingData();

private:
    void stringLiteralWriteError(const Token *tok, const Token *strValue);
    void sprintfOverlappingDataError(const Token *funcTok, const Token *tok, const std::string &varname);
    void strPlusCharError(const Token *tok);
    void incorrectStringCompareError(const Token *tok, const std::string& func, const std::string &string);
    void incorrectStringBooleanError(const Token *tok, const std::string& string);
    void alwaysTrueFalseStringCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void alwaysTrueStringVariableCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void suspiciousStringCompareError(const Token* tok, const std::string& var, bool isLong);
    void suspiciousStringCompareError_char(const Token* tok, const std::string& var);
    void overlappingStrcmpError(const Token* eq0, const Token *ne0);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override {
        CheckString c(nullptr, settings, errorLogger);
        c.stringLiteralWriteError(nullptr, nullptr);
        c.sprintfOverlappingDataError(nullptr, nullptr, "varname");
        c.strPlusCharError(nullptr);
        c.incorrectStringCompareError(nullptr, "substr", "\"Hello World\"");
        c.suspiciousStringCompareError(nullptr, "foo", false);
        c.suspiciousStringCompareError_char(nullptr, "foo");
        c.incorrectStringBooleanError(nullptr, "\"Hello World\"");
        c.incorrectStringBooleanError(nullptr, "\'x\'");
        c.alwaysTrueFalseStringCompareError(nullptr, "str1", "str2");
        c.alwaysTrueStringVariableCompareError(nullptr, "varname1", "varname2");
        c.overlappingStrcmpError(nullptr, nullptr);
    }

    static std::string myName() {
        return "String";
    }

    std::string classInfo() const override {
        return "Detect misusage of C-style strings:\n"
               "- overlapping buffers passed to sprintf as source and destination\n"
               "- incorrect length arguments for 'substr' and 'strncmp'\n"
               "- suspicious condition (runtime comparison of string literals)\n"
               "- suspicious condition (string/char literals as boolean)\n"
               "- suspicious comparison of a string literal with a char\\* variable\n"
               "- suspicious comparison of '\\0' with a char\\* variable\n"
               "- overlapping strcmp() expression\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkstringH
