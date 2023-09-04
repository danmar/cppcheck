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
#ifndef checkassertH
#define checkassertH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "tokenize.h"

#include <string>

class ErrorLogger;
class Scope;
class Settings;
class Token;

/// @addtogroup Checks
/// @{

/**
 * @brief Checking for side effects in assert statements
 */

class CPPCHECKLIB CheckAssert : public Check {
public:
    CheckAssert() : Check(myName()) {}

    CheckAssert(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** run checks, the token list is not simplified */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override {
        CheckAssert checkAssert(&tokenizer, tokenizer.getSettings(), errorLogger);
        checkAssert.assertWithSideEffects();
    }

    void assertWithSideEffects();

protected:
    void checkVariableAssignment(const Token* assignTok, const Scope *assertionScope);
    static bool inSameScope(const Token* returnTok, const Token* assignTok);

private:
    void sideEffectInAssertError(const Token *tok, const std::string& functionName);
    void assignmentInAssertError(const Token *tok, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override {
        CheckAssert c(nullptr, settings, errorLogger);
        c.sideEffectInAssertError(nullptr, "function");
        c.assignmentInAssertError(nullptr, "var");
    }

    static std::string myName() {
        return "Assert";
    }

    std::string classInfo() const override {
        return "Warn if there are side effects in assert statements (since this cause different behaviour in debug/release builds).\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkassertH
