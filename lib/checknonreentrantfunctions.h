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
#ifndef checknonreentrantfunctionsH
#define checknonreentrantfunctionsH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include <string>
#include <map>


/// @addtogroup Checks
/// @{

/**
 * @brief Using non reentrant functions that can be replaced by their reentrant versions
 */

class CPPCHECKLIB CheckNonReentrantFunctions : public Check {
public:
    /** This constructor is used when registering the CheckNonReentrantFunctions */
    CheckNonReentrantFunctions() : Check(myName()) {
        initNonReentrantFunctions();
    }

    /** This constructor is used when running checks. */
    CheckNonReentrantFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
        initNonReentrantFunctions();
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckNonReentrantFunctions checkNonReentrantFunctions(tokenizer, settings, errorLogger);
        checkNonReentrantFunctions.nonReentrantFunctions();
    }

    /** Check for non reentrant functions */
    void nonReentrantFunctions();

private:

    /* function name / error message */
    std::map<std::string,std::string> _nonReentrantFunctions;

    /** init nonreentrant functions list ' */
    void initNonReentrantFunctions() {
        static const char * const non_reentrant_functions_list[] = {
            "localtime", "gmtime", "strtok", "gethostbyname", "gethostbyaddr", "getservbyname"
            , "getservbyport", "crypt", "ttyname", "gethostbyname2"
            , "getprotobyname", "getnetbyname", "getnetbyaddr", "getrpcbyname", "getrpcbynumber", "getrpcent"
            , "ctermid", "readdir", "getlogin", "getpwent", "getpwnam", "getpwuid", "getspent"
            , "fgetspent", "getspnam", "getgrnam", "getgrgid", "getnetgrent", "tempnam", "fgetpwent"
            , "fgetgrent", "ecvt", "gcvt", "getservent", "gethostent", "getgrent", "fcvt"
        };

        // generate messages
        for (unsigned int i = 0; i < (sizeof(non_reentrant_functions_list) / sizeof(char *)); ++i) {
            std::string strMsg("Non reentrant function '");
            strMsg+=non_reentrant_functions_list[i];
            strMsg+= "' called. For threadsafe applications it is recommended to use the reentrant replacement function '";
            strMsg+=non_reentrant_functions_list[i];
            strMsg+="_r'.";
            _nonReentrantFunctions[non_reentrant_functions_list[i]] = strMsg;
        }
    }

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckNonReentrantFunctions c(0, settings, errorLogger);

        std::map<std::string,std::string>::const_iterator it(_nonReentrantFunctions.begin()), itend(_nonReentrantFunctions.end());
        for (; it!=itend; ++it) {
            c.reportError(0, Severity::portability, "nonreentrantFunctions"+it->first, it->second);
        }
    }

    static std::string myName() {
        return "Non reentrant functions";
    }

    std::string classInfo() const {
        std::string info = "Warn if any of these non reentrant functions are used:\n";
        std::map<std::string,std::string>::const_iterator it(_nonReentrantFunctions.begin()), itend(_nonReentrantFunctions.end());
        for (; it!=itend; ++it) {
            info += "- " + it->first + "\n";
        }
        return info;
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checknonreentrantfunctionsH
