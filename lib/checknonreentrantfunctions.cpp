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
// Find non reentrant functions
//---------------------------------------------------------------------------

#include "checknonreentrantfunctions.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckNonReentrantFunctions instance;
}

namespace {
    const std::set<std::string> _nonReentrantFunctions = make_container< std::set<std::string> > ()
            << "localtime" << "gmtime" << "strtok" << "gethostbyname" << "gethostbyaddr" << "getservbyname"
            << "getservbyport" << "crypt" << "ttyname" << "gethostbyname2"
            << "getprotobyname" << "getnetbyname" << "getnetbyaddr" << "getrpcbyname" << "getrpcbynumber" << "getrpcent"
            << "ctermid" << "readdir" << "getlogin" << "getpwent" << "getpwnam" << "getpwuid" << "getspent"
            << "fgetspent" << "getspnam" << "getgrnam" << "getgrgid" << "getnetgrent" << "tempnam" << "fgetpwent"
            << "fgetgrent" << "ecvt" << "gcvt" << "getservent" << "gethostent" << "getgrent" << "fcvt" ;
}

std::string CheckNonReentrantFunctions::generateErrorMessage(const std::string& function)
{
    return std::string("Non reentrant function '") + function + "' called. " +
           "For threadsafe applications it is recommended to use the reentrant replacement function '" + function + "_r'.";
}

void CheckNonReentrantFunctions::nonReentrantFunctions()
{
    if (!_settings->standards.posix || !_settings->isEnabled("portability"))
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // Look for function invocations
            if (tok->varId() != 0 || !tok->isName() || tok->strAt(1) != "(")
                continue;

            // Check for non-reentrant function name
            std::set<std::string>::const_iterator it = _nonReentrantFunctions.find(tok->str());
            if (it == _nonReentrantFunctions.end())
                continue;

            const Token *prev = tok->previous();
            if (prev) {
                // Ignore function definitions, class members or class definitions
                if (prev->str() == ".")
                    continue;

                // Check for "std" or global namespace, ignore other namespaces
                if (_tokenizer->isCPP() && prev->str() == "::" && prev->previous() && prev->previous()->str() != "std" && prev->previous()->isName())
                    continue;
            }

            // Only affecting multi threaded code, therefore this is "portability"
            reportError(tok, Severity::portability, "nonreentrantFunctions" + *it, generateErrorMessage(*it));
        }
    }
}
//---------------------------------------------------------------------------

void CheckNonReentrantFunctions::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckNonReentrantFunctions c(0, settings, errorLogger);

    std::set<std::string>::const_iterator it(_nonReentrantFunctions.begin()), itend(_nonReentrantFunctions.end());
    for (; it!=itend; ++it) {
        c.reportError(0, Severity::portability, "nonreentrantFunctions"+*it, generateErrorMessage(*it));
    }
}

std::string CheckNonReentrantFunctions::classInfo() const
{
    std::string info = "Warn if any of these non reentrant functions are used:\n";
    std::set<std::string>::const_iterator it(_nonReentrantFunctions.begin()), itend(_nonReentrantFunctions.end());
    for (; it!=itend; ++it) {
        info += "- " + *it + "\n";
    }
    return info;
}
