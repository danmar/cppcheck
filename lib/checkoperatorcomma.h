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

#ifndef checkoperatorcommaH
#define checkoperatorcommaH

#include "check.h"

class CPPCHECKLIB CheckOpComma : public Check {
public:
    CheckOpComma() : Check(myName()) {}
private:
    CheckOpComma(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}
    static std::string myName() {
        return "CheckOpComma";
    }
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;
    std::string classInfo() const override {
        return "warn if there is any suspicious comma expression used as a condition, which might be a bad coding style\n";
    }

    void assertWithSuspiciousComma();
    void assertWithSuspiciousCommaError(const Token *tok);
};
#endif
