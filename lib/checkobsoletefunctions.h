/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#ifndef CheckObsoleteFunctionsH
#define CheckObsoleteFunctionsH
//---------------------------------------------------------------------------

#include "check.h"
#include <string>
#include <list>


/// @addtogroup Checks
/// @{

/**
 * @brief Using obsolete functions that are always insecure to use.
 */

class CheckObsoleteFunctions : public Check
{
public:
    /** This constructor is used when registering the CheckObsoleteFunctions */
    CheckObsoleteFunctions() : Check()
    {
        initObsoleteFunctions();
    }

    /** This constructor is used when running checks. */
    CheckObsoleteFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(tokenizer, settings, errorLogger)
    {
        initObsoleteFunctions();
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckObsoleteFunctions checkObsoleteFunctions(tokenizer, settings, errorLogger);
        checkObsoleteFunctions.obsoleteFunctions();
    }

    /** Check for obsolete functions */
    void obsoleteFunctions();

private:
    /* function name / error message */
    std::list< std::pair< const std::string, const std::string> > _obsoleteFunctions;

    /** init obsolete functions list ' */
    void initObsoleteFunctions()
    {
        _obsoleteFunctions.push_back(std::make_pair("bsd_signal","Found obsolete function 'bsd_signal'. It is recommended that new applications use the 'sigaction' function"));

        _obsoleteFunctions.push_back(std::make_pair("gethostbyaddr","Found obsolete function 'gethostbyaddr'. It is recommended that new applications use the 'getaddrinfo' function"));
        _obsoleteFunctions.push_back(std::make_pair("gethostbyname","Found obsolete function 'gethostbyname'. It is recommended that new applications use the 'getnameinfo' function"));

        _obsoleteFunctions.push_back(std::make_pair("usleep","Found obsolete function 'usleep'. It is recommended that new applications use the 'nanosleep' or 'setitimer' function"));

        _obsoleteFunctions.push_back(std::make_pair("bcmp","Found obsolete function 'bcmp'. It is recommended that new applications use the 'memcmp' function"));
        _obsoleteFunctions.push_back(std::make_pair("bcopy","Found obsolete function 'bcopy'. It is recommended that new applications use the 'memmove' function"));
        _obsoleteFunctions.push_back(std::make_pair("bzero","Found obsolete function 'bzero'. It is recommended that new applications use the 'memset' function"));

        _obsoleteFunctions.push_back(std::make_pair("ecvt","Found obsolete function 'ecvt'. It is recommended that new applications use the 'sprintf' function"));
        _obsoleteFunctions.push_back(std::make_pair("fcvt","Found obsolete function 'fcvt'. It is recommended that new applications use the 'sprintf' function"));
        _obsoleteFunctions.push_back(std::make_pair("gcvt","Found obsolete function 'gcvt'. It is recommended that new applications use the 'sprintf' function"));

        _obsoleteFunctions.push_back(std::make_pair("ftime","Found obsolete function 'ftime'. It is recommended that new applications use the 'ftime' function. Realtime applications should use ''clock_gettime'' to determine the current time"));

        _obsoleteFunctions.push_back(std::make_pair("getcontext","Found obsolete function 'getcontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads"));
        _obsoleteFunctions.push_back(std::make_pair("makecontext","Found obsolete function 'makecontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads"));
        _obsoleteFunctions.push_back(std::make_pair("swapcontext","Found obsolete function 'swapcontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads"));

        _obsoleteFunctions.push_back(std::make_pair("getwd","Found obsolete function 'getwd'. It is recommended that new applications use the 'getcwd' function"));

        _obsoleteFunctions.push_back(std::make_pair("index","Found obsolete function 'index'. It is recommended to use the function 'strchr' instead"));
        _obsoleteFunctions.push_back(std::make_pair("rindex","Found obsolete function 'rindex'. It is recommended to use the function 'strrchr' instead"));

        _obsoleteFunctions.push_back(std::make_pair("pthread_attr_getstackaddr","Found obsolete function 'pthread_attr_getstackaddr'.It is recommended that new applications use the 'pthread_attr_getstack' function"));
        _obsoleteFunctions.push_back(std::make_pair("pthread_attr_setstackaddr","Found obsolete function 'pthread_attr_setstackaddr'.It is recommended that new applications use the 'pthread_attr_setstack' function"));

        _obsoleteFunctions.push_back(std::make_pair("scalbln","Found obsolete function 'scalb'.It is recommended to use either 'scalbln', 'scalblnf' or 'scalblnl' instead of this function"));

        _obsoleteFunctions.push_back(std::make_pair("ualarm","Found obsolete function 'ualarm'.It is recommended to use either 'timer_create', 'timer_delete', 'timer_getoverrun', 'timer_gettime', or 'timer_settime' instead of this function"));

        _obsoleteFunctions.push_back(std::make_pair("vfork","Found obsolete function 'vfork'. It is recommended to use the function 'fork' instead"));

        _obsoleteFunctions.push_back(std::make_pair("wcswcs","Found obsolete function 'wcswcs'. It is recommended to use the function 'wcsstr' instead"));

    }

    void getErrorMessages()
    {
        std::list< std::pair<const std::string, const std::string> >::const_iterator it(_obsoleteFunctions.begin()), itend(_obsoleteFunctions.end());
        for (; it!=itend; ++it)
        {
            reportError(0, Severity::style, "obsoleteFunctions"+it->first, it->second);
        }
    }

    std::string name() const
    {
        return "Obsolete functions";
    }

    std::string classInfo() const
    {
        std::string info = "Warn if any of these obsolete functions are used:\n";
        std::list< std::pair<const std::string, const std::string> >::const_iterator it(_obsoleteFunctions.begin()), itend(_obsoleteFunctions.end());
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

