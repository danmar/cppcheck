/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include <string>

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;

/// @addtogroup Checks
/// @{


/** @brief %Check Boost usage */
class CPPCHECKLIB CheckBoost : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckBoost() : Check(myName()) {}

private:
    /** This constructor is used when running checks. */
    CheckBoost(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief %Check for container modification while using the BOOST_FOREACH macro */
    void checkBoostForeachModification();

    void boostForeachError(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

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
