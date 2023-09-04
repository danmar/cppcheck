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
#ifndef checkboostH
#define checkboostH
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


/** @brief %Check Boost usage */
class CPPCHECKLIB CheckBoost : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckBoost() : Check(myName()) {}

    /** This constructor is used when running checks. */
    CheckBoost(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override {
        if (!tokenizer.isCPP())
            return;

        CheckBoost checkBoost(&tokenizer, tokenizer.getSettings(), errorLogger);
        checkBoost.checkBoostForeachModification();
    }

    /** @brief %Check for container modification while using the BOOST_FOREACH macro */
    void checkBoostForeachModification();

private:
    void boostForeachError(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override {
        CheckBoost c(nullptr, settings, errorLogger);
        c.boostForeachError(nullptr);
    }

    static std::string myName() {
        return "Boost usage";
    }

    std::string classInfo() const override {
        return "Check for invalid usage of Boost:\n"
               "- container modification during BOOST_FOREACH\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkboostH
