/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
 * @brief Check for bad function usage
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

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckFunctions checkFunctions(tokenizer, settings, errorLogger);

        // Checks
        checkFunctions.checkIgnoredReturnValue();

        // --check-library : functions with nonmatching configuration
        checkFunctions.checkLibraryMatchFunctions();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckFunctions checkFunctions(tokenizer, settings, errorLogger);

        checkFunctions.checkProhibitedFunctions();
        checkFunctions.invalidFunctionUsage();
        checkFunctions.checkMathFunctions();
    }

    /** Check for functions that should not be used */
    void checkProhibitedFunctions();

    /**
    * @brief Invalid function usage (invalid input value / overlapping data)
    *
    * %Check that given function parameters are valid according to the standard
    * - wrong radix given for strtol/strtoul
    * - overlapping data when using sprintf/snprintf
    * - wrong input value according to library
    */
    void invalidFunctionUsage();

    /** @brief %Check for ignored return values. */
    void checkIgnoredReturnValue();

    /** @brief %Check for parameters given to math function that do not make sense*/
    void checkMathFunctions();

    /** @brief --check-library: warn for unconfigured function calls */
    void checkLibraryMatchFunctions();

private:
    void invalidFunctionArgError(const Token *tok, const std::string &functionName, int argnr, const std::string &validstr);
    void invalidFunctionArgBoolError(const Token *tok, const std::string &functionName, int argnr);
    void ignoredReturnValueError(const Token* tok, const std::string& function);
    void mathfunctionCallWarning(const Token *tok, const unsigned int numParam = 1);
    void mathfunctionCallWarning(const Token *tok, const std::string& oldexp, const std::string& newexp);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckFunctions c(nullptr, settings, errorLogger);

        for (std::map<std::string, Library::WarnInfo>::const_iterator i = settings->library.functionwarn.cbegin(); i != settings->library.functionwarn.cend(); ++i) {
            c.reportError(nullptr, Severity::style, i->first+"Called", i->second.message);
        }

        c.invalidFunctionArgError(nullptr, "func_name", 1, "1-4");
        c.invalidFunctionArgBoolError(nullptr, "func_name", 1);
        c.ignoredReturnValueError(nullptr, "malloc");
        c.mathfunctionCallWarning(nullptr);
        c.mathfunctionCallWarning(nullptr, "1 - erf(x)", "erfc(x)");
    }

    static std::string myName() {
        return "Check function usage";
    }

    std::string classInfo() const {
        return "Check function usage:\n"
               "- return value of certain functions not used\n"
               "- invalid input values for functions\n"
               "- Warn if a function is called whose usage is discouraged\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkfunctionsH
