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
#ifndef checkassertH
#define checkassertH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

/// @addtogroup Checks
/// @{

/**
 * @brief Checking for side effects in assert statements
 */

class CPPCHECKLIB CheckAssert : public Check {
public:
    CheckAssert() : Check(myName()) {
    }

    CheckAssert(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    virtual void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckAssert check(tokenizer, settings, errorLogger);
        check.assertWithSideEffects();
    }

    void assertWithSideEffects();

protected:
    void checkVariableAssignment(const Token* tmp);
    static bool inSameScope(const Token* returnTok, const Token* assignTok);

private:
    void sideEffectInAssertError(const Token *tok, const std::string& functionName);
    void assignmentInAssertError(const Token *tok, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckAssert c(0, settings, errorLogger);
        c.sideEffectInAssertError(0, "function");
        c.assignmentInAssertError(0, "var");
    }

    static std::string myName() {
        return "Assert";
    }

    std::string classInfo() const {
        return "Warn if there are side effects in assert statements (since this cause different behaviour in debug/release builds).\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkassertH
