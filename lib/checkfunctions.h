/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Cppcheck team.
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
#ifndef checkfunctionsH
#define checkfunctionsH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include <string>


/// @addtogroup Checks
/// @{

/**
 * @brief Check for functions which should not be used
 */

class CPPCHECKLIB CheckFunctions : public Check {
public:
    /** This constructor is used when registering the CheckFunctions */
    CheckFunctions() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckFunctions checkFunctions(tokenizer, settings, errorLogger);
        checkFunctions.check();
    }

    /** Check for functions that should not be used */
    void check();

private:
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckFunctions c(0, settings, errorLogger);

        for (std::map<std::string, Library::WarnInfo>::const_iterator i = settings->library.functionwarn.cbegin(); i != settings->library.functionwarn.cend(); ++i) {
            c.reportError(0, Severity::style, i->first+"Called", i->second.message);
        }
    }

    static std::string myName() {
        return "Check function usage";
    }

    std::string classInfo() const {
        return "Warn if a function is called whose usage is discouraged\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkfunctionsH
