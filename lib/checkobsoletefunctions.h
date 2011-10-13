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

class CheckObsoleteFunctions : public Check {
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
    void initObsoleteFunctions() {
        _obsoletePosixFunctions["bsd_signal"] = "Found obsolete function 'bsd_signal'. It is recommended that new applications use the 'sigaction' function";

        _obsoletePosixFunctions["gethostbyaddr"] = "Found obsolete function 'gethostbyaddr'. It is recommended that new applications use the 'getnameinfo' function";
        _obsoletePosixFunctions["gethostbyname"] = "Found obsolete function 'gethostbyname'. It is recommended that new applications use the 'getaddrinfo' function";

        _obsoletePosixFunctions["usleep"] = "Found obsolete function 'usleep'. It is recommended that new applications use the 'nanosleep' or 'setitimer' function\n"
                                            "Found obsolete function 'usleep'. POSIX.1-2001 declares usleep() function obsolete and POSIX.1-2008 removes it. It is recommended that new applications use the 'nanosleep' or 'setitimer' function.";

        _obsoletePosixFunctions["bcmp"]="Found obsolete function 'bcmp'. It is recommended that new applications use the 'memcmp' function";
        _obsoletePosixFunctions["bcopy"]="Found obsolete function 'bcopy'. It is recommended that new applications use the 'memmove' or 'memcpy' functions";
        _obsoletePosixFunctions["bzero"]="Found obsolete function 'bzero'. It is recommended that new applications use the 'memset' function";

        _obsoletePosixFunctions["ecvt"]="Found obsolete function 'ecvt'. It is recommended that new applications use the 'sprintf' function";
        _obsoletePosixFunctions["fcvt"]="Found obsolete function 'fcvt'. It is recommended that new applications use the 'sprintf' function";
        _obsoletePosixFunctions["gcvt"]="Found obsolete function 'gcvt'. It is recommended that new applications use the 'sprintf' function";

        _obsoletePosixFunctions["ftime"]="Found obsolete function 'ftime'.\n"
                                         "It is recommended that new applications use time(), gettimeofday(), or clock_gettime() instead. "
                                         "For high-resolution timing on Windows, QueryPerformanceCounter() and QueryPerformanceFrequency may be used.";

        _obsoletePosixFunctions["getcontext"] = "Found obsolete function 'getcontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads";
        _obsoletePosixFunctions["makecontext"] = "Found obsolete function 'makecontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads";
        _obsoletePosixFunctions["swapcontext"] = "Found obsolete function 'swapcontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads";

        _obsoletePosixFunctions["getwd"] = "Found obsolete function 'getwd'. It is recommended that new applications use the 'getcwd' function";

        // See #2334 (using the Qt Model/View function 'index')
        _obsoletePosixFunctions["index"] ="Found obsolete function 'index'. It is recommended to use the function 'strchr' instead";

        _obsoletePosixFunctions["rindex"] = "Found obsolete function 'rindex'. It is recommended to use the function 'strrchr' instead";

        _obsoletePosixFunctions["pthread_attr_getstackaddr"] = "Found obsolete function 'pthread_attr_getstackaddr'.It is recommended that new applications use the 'pthread_attr_getstack' function";
        _obsoletePosixFunctions["pthread_attr_setstackaddr"] = "Found obsolete function 'pthread_attr_setstackaddr'.It is recommended that new applications use the 'pthread_attr_setstack' function";

        _obsoletePosixFunctions["scalbln"] = "Found obsolete function 'scalb'.It is recommended to use either 'scalbln', 'scalblnf' or 'scalblnl' instead of this function";

        _obsoletePosixFunctions["ualarm"] = "Found obsolete function 'ualarm'.It is recommended to use either 'timer_create', 'timer_delete', 'timer_getoverrun', 'timer_gettime', or 'timer_settime' instead of this function";

        _obsoletePosixFunctions["vfork"] = "Found obsolete function 'vfork'. It is recommended to use the function 'fork' instead";

        _obsoletePosixFunctions["wcswcs"] = "Found obsolete function 'wcswcs'. It is recommended to use the function 'wcsstr' instead";

        _obsoleteStandardFunctions["gets"] = "Found obsolete function 'gets'. It is recommended to use the function 'fgets' instead\n"
                                             "Found obsolete function 'gets'. With gets you'll get buffer overruns if the input data too big for the buffer. It is recommended to use the function 'fgets' instead.";
        _obsoleteC99Functions["alloca"] = "Found obsolete function 'alloca'. It is recommended to use a variable length array.\nFound obsolete function 'alloca'. It is recommended to use a variable length array or a dynamically allocated array. The function 'alloca' is dangerous for many reasons (http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).";

    }

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) {
        CheckObsoleteFunctions c(0, settings, errorLogger);

        std::map<std::string,std::string>::const_iterator it(_obsoletePosixFunctions.begin()), itend(_obsoletePosixFunctions.end());
        for (; it!=itend; ++it) {
            c.reportError(0, Severity::style, "obsoleteFunctions"+it->first, it->second);
        }
    }

    std::string myName() const {
        return "Obsolete functions";
    }

    std::string classInfo() const {
        std::string info = "Warn if any of these obsolete functions are used:\n";
        std::map<std::string,std::string>::const_iterator it(_obsoletePosixFunctions.begin()), itend(_obsoletePosixFunctions.end());
        for (; it!=itend; ++it) {
            info += "* " + it->first + "\n";
        }
        return info;
    }
};
/// @}
//---------------------------------------------------------------------------
#endif

