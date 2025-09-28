/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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
#ifndef checkunionzeroinitH
#define checkunionzeroinitH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;
struct UnionMember;

/// @addtogroup Checks
/// @{

/**
 * @brief Check for error-prone zero initialization of unions.
 */

class CPPCHECKLIB CheckUnionZeroInit : public Check {
    friend class TestUnionZeroInit;

public:
    /** This constructor is used when registering the CheckUnionZeroInit */
    CheckUnionZeroInit() : Check(myName()) {}

private:
    /** This constructor is used when running checks. */
    CheckUnionZeroInit(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** Check for error-prone zero initialization of unions. */
    void check();

    void unionZeroInitError(const Token *tok, const UnionMember &largestMember);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static std::string generateTestMessage(const Tokenizer &tokenizer, const Settings &settings);

    static std::string myName() {
        return "CheckUnionZeroInit";
    }

    std::string classInfo() const override {
        return "Check for error-prone zero initialization of unions.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkunionzeroinitH
