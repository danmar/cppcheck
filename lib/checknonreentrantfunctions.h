/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#ifndef CheckNonReentrantFunctionsH
#define CheckNonReentrantFunctionsH
//---------------------------------------------------------------------------

#include "check.h"
#include <string>
#include <list>


/// @addtogroup Checks
/// @{

/**
 * @brief Using non reentrant functions that can be replaced by their reentrant versions
 */

class CheckNonReentrantFunctions : public Check
{
public:
    /** This constructor is used when registering the CheckNonReentrantFunctions */
    CheckNonReentrantFunctions() : Check(myName())
    {
        initNonReentrantFunctions();
    }

    /** This constructor is used when running checks. */
    CheckNonReentrantFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    {
        initNonReentrantFunctions();
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckNonReentrantFunctions checkNonReentrantFunctions(tokenizer, settings, errorLogger);
        checkNonReentrantFunctions.nonReentrantFunctions();
    }

    /** Check for non reentrant functions */
    void nonReentrantFunctions();

private:

    /* function name / error message */
    std::list< std::pair< const std::string, const std::string> > _nonReentrantFunctions;

    /** init obsolete functions list ' */
    void initNonReentrantFunctions()
    {
        _nonReentrantFunctions.push_back(std::make_pair("crypt","Found the non reentrant function 'crpyt'. For threadsafe applications it is recommended to use the reentrant replacement function 'crypt_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("getlogin","Found the non reentrant function 'getlogin'. For threadsafe applications it is recommended to use the reentrant replacement function 'getlogin_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("ttyname","Found the non reentrant function 'ttyname'. For threadsafe applications it is recommended to use the reentrant replacement function 'ttyname_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("asctime","Found the non reentrant function 'asctime'. For threadsafe applications it is recommended to use the reentrant replacement function 'asctime_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("ctime","Found the non reentrant function 'ctime'. For threadsafe applications it is recommended to use the reentrant replacement function 'ctime_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("gmtime","Found the non reentrant function 'gmtime'. For threadsafe applications it is recommended to use the reentrant replacement function 'gmtime_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("localtime","Found the non reentrant function 'localtime'. For threadsafe applications it is recommended to use the reentrant replacement function 'localtime_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("getgrgid","Found the non reentrant function 'getgrgid'. For threadsafe applications it is recommended to use the reentrant replacement function 'getgrgid_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("getgrnam","Found the non reentrant function 'getgrnam'. For threadsafe applications it is recommended to use the reentrant replacement function 'getgrnam_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("getpwnam","Found the non reentrant function 'getpwnam'. For threadsafe applications it is recommended to use the reentrant replacement function 'getpwnam_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("getpwuid","Found the non reentrant function 'getpwuid'. For threadsafe applications it is recommended to use the reentrant replacement function 'getpwuid_r'"));
        _nonReentrantFunctions.push_back(std::make_pair("rand","Found the non reentrant function 'rand'. For threadsafe applications it is recommended to use the reentrant replacement function 'rand_r'"));
    }

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
    {
        CheckNonReentrantFunctions c(0, settings, errorLogger);

        std::list< std::pair<const std::string, const std::string> >::const_iterator it(_nonReentrantFunctions.begin()), itend(_nonReentrantFunctions.end());
        for (; it!=itend; ++it)
        {
            c.reportError(0, Severity::style, "nonreentrantFunctions"+it->first, it->second);
        }
    }

    std::string myName() const
    {
        return "Non reentrant functions";
    }

    std::string classInfo() const
    {
        std::string info = "Warn if any of these non reentrant functions are used:\n";
        std::list< std::pair<const std::string, const std::string> >::const_iterator it(_nonReentrantFunctions.begin()), itend(_nonReentrantFunctions.end());
        for (; it!=itend; ++it)
        {
            info += "* " + it->first + "\n";
        }
        return info;
    }
};
/// @}
//---------------------------------------------------------------------------
#endif

