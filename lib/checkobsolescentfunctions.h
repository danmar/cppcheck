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
#ifndef checkobsoletefunctionsH
#define checkobsoletefunctionsH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include <string>
#include <map>


/// @addtogroup Checks
/// @{

/**
 * @brief Using obsolete functions that are always insecure to use.
 */

class CPPCHECKLIB CheckObsoleteFunctions : public Check {
public:
    /** This constructor is used when registering the CheckObsoleteFunctions */
    CheckObsoleteFunctions() : Check(myName()) {
        initObsoleteFunctions();
    }

    /** This constructor is used when running checks. */
    CheckObsoleteFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
        initObsoleteFunctions();
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckObsoleteFunctions checkObsoleteFunctions(tokenizer, settings, errorLogger);
        checkObsoleteFunctions.obsoleteFunctions();
    }

    /** Check for obsolete functions */
    void obsoleteFunctions();

private:
    /* function name / error message */
    std::map<std::string, std::string> _obsoleteStandardFunctions;
    std::map<std::string, std::string> _obsoletePosixFunctions;
    std::map<std::string, std::string> _obsoleteC99Functions;

    /** init obsolete functions list ' */
    void initObsoleteFunctions();

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckObsoleteFunctions c(0, settings, errorLogger);

        for (std::map<std::string, std::string>::const_iterator it = _obsoleteStandardFunctions.begin(); it != _obsoleteStandardFunctions.end(); ++it)
            c.reportError(0, Severity::style, "obsoleteFunctions" + it->first, it->second);
        for (std::map<std::string, std::string>::const_iterator it = _obsoleteC99Functions.begin(); it != _obsoleteC99Functions.end(); ++it)
            c.reportError(0, Severity::style, "obsoleteFunctions" + it->first, it->second);
        for (std::map<std::string, std::string>::const_iterator it = _obsoletePosixFunctions.begin(); it != _obsoletePosixFunctions.end(); ++it)
            c.reportError(0, Severity::style, "obsoleteFunctions" + it->first, it->second);
    }

    static std::string myName() {
        return "Obsolete functions";
    }

    std::string classInfo() const {
        std::string info = "Warn if any of these obsolete functions are used:\n";
        for (std::map<std::string, std::string>::const_iterator it = _obsoleteStandardFunctions.begin(); it != _obsoleteStandardFunctions.end(); ++it)
            info += "- " + it->first + "\n";
        for (std::map<std::string, std::string>::const_iterator it = _obsoleteC99Functions.begin(); it != _obsoleteC99Functions.end(); ++it)
            info += "- " + it->first + "\n";
        for (std::map<std::string, std::string>::const_iterator it = _obsoletePosixFunctions.begin(); it != _obsoletePosixFunctions.end(); ++it)
            info += "- " + it->first + "\n";
        return info;
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkobsoletefunctionsH
