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
#ifndef checkconditionH
#define checkconditionH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "errortypes.h"

#include <string>

class Settings;
class ErrorLogger;
class Tokenizer;

/// @addtogroup Checks
/// @{

/**
 * @brief Check for condition mismatches
 */

class CPPCHECKLIB CheckCondition : public Check {
public:
    /** This constructor is used when registering the CheckAssignIf */
    CheckCondition() : Check("Condition") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Match conditions with assignments and other conditions:\n"
               "- Mismatching assignment and comparison => comparison is always true/false\n"
               "- Mismatching lhs and rhs in comparison => comparison is always true/false\n"
               "- Detect usage of | where & should be used\n"
               "- Duplicate condition and assignment\n"
               "- Detect matching 'if' and 'else if' conditions\n"
               "- Mismatching bitand (a &= 0xf0; a &= 1; => a = 0)\n"
               "- Opposite inner condition is always false\n"
               "- Identical condition after early exit is always false\n"
               "- Condition that is always true/false\n"
               "- Mutual exclusion over || always evaluating to true\n"
               "- Comparisons of modulo results that are always true/false.\n"
               "- Known variable values => condition is always true/false\n"
               "- Invalid test for overflow. Some mainstream compilers remove such overflow tests when optimising code.\n"
               "- Suspicious assignment of container/iterator in condition => condition is always true.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkconditionH
