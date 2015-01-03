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
    void initObsoleteFunctions() {
        // Obsolete posix functions, which messages suggest only one alternative and doesn't contain additional information.
        const struct {
            const char* bad;
            const char* good;
        } posix_stdmsgs[] = {
            {"bsd_signal", "sigaction"},
            {"gethostbyaddr", "getnameinfo"},
            {"gethostbyname", "getaddrinfo"},
            {"bcmp", "memcmp"},
            {"bzero", "memset"},
            {"ecvt", "sprintf"},
            {"fcvt", "sprintf"},
            {"gcvt", "sprintf"},
            {"getwd", "getcwd"},
            {"index", "strchr"}, // See #2334 (using the Qt Model/View function 'index')
            {"rindex", "strrchr"},
            {"pthread_attr_getstackaddr", "pthread_attr_getstack"},
            {"pthread_attr_setstackaddr", "pthread_attr_setstack"},
            {"vfork", "fork"},
            {"wcswcs", "wcsstr"},
            {"rand_r", "rand"},
            {"utime", "utimensat"},
            {"asctime_r", "strftime"},
            {"ctime_r", "strftime"}
        };

        for (std::size_t i = 0; i < (sizeof(posix_stdmsgs) / sizeof(*posix_stdmsgs)); ++i) {
            _obsoletePosixFunctions[posix_stdmsgs[i].bad] = "Obsolete function '" + std::string(posix_stdmsgs[i].bad) + "' called. It is recommended to use the function '" + posix_stdmsgs[i].good + "' instead.";
        }

        _obsoletePosixFunctions["usleep"] = "Obsolete function 'usleep' called. It is recommended to use the 'nanosleep' or 'setitimer' function instead.\n"
                                            "The obsolete function 'usleep' is called. POSIX.1-2001 declares usleep() function obsolete and POSIX.1-2008 removes it. It is recommended that new applications use the 'nanosleep' or 'setitimer' function.";

        _obsoletePosixFunctions["bcopy"] = "Obsolete function 'bcopy' called. It is recommended to use the 'memmove' or 'memcpy' function instead.";

        _obsoletePosixFunctions["ftime"] = "Obsolete function 'ftime' called. It is recommended to use time(), gettimeofday() or clock_gettime() instead.";

        _obsoletePosixFunctions["getcontext"] = "Obsolete function 'getcontext' called. Due to portability issues, applications are recommended to be rewritten to use POSIX threads.";
        _obsoletePosixFunctions["makecontext"] = "Obsolete function 'makecontext' called. Due to portability issues, applications are recommended to be rewritten to use POSIX threads.";
        _obsoletePosixFunctions["swapcontext"] = "Obsolete function 'swapcontext' called. Due to portability issues, applications are recommended to be rewritten to use POSIX threads.";

        _obsoletePosixFunctions["scalbln"] = "Obsolete function 'scalb' called. It is recommended to use 'scalbln', 'scalblnf' or 'scalblnl' instead.";

        _obsoletePosixFunctions["ualarm"] = "Obsolete function 'ualarm' called. It is recommended to use 'timer_create', 'timer_delete', 'timer_getoverrun', 'timer_gettime' or 'timer_settime' instead.";

        _obsoletePosixFunctions["tmpnam"] = "Obsolete function 'tmpnam' called. It is recommended to use 'tmpfile', 'mkstemp' or 'mkdtemp' instead.";

        _obsoletePosixFunctions["tmpnam_r"] = "Obsolete function 'tmpnam_r' called. It is recommended to use 'tmpfile', 'mkstemp' or 'mkdtemp' instead.";

        _obsoleteStandardFunctions["gets"] = "Obsolete function 'gets' called. It is recommended to use the function 'fgets' instead.\n"
                                             "The obsolete function 'gets' is called. With 'gets' you'll get a buffer overrun if the input data exceeds the size of the buffer. It is recommended to use the function 'fgets' instead.";
        _obsoleteC99Functions["alloca"] = "Obsolete function 'alloca' called. In C99 and later it is recommended to use a variable length array instead.\n"
                                          "The obsolete function 'alloca' is called. In C99 and later it is recommended to use a variable length array or a dynamically allocated array instead. The function 'alloca' is dangerous for many reasons (http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).";
        _obsoleteC99Functions["asctime"] = "Obsolete function 'asctime' called. It is recommended to use the function 'strftime' instead.";
        // ctime is obsolete - it's not threadsafe. but there is no good replacement.
        //_obsoleteC99Functions["ctime"] = "Obsolete function 'ctime' called. It is recommended to use the function 'strftime' instead.";
    }

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckObsoleteFunctions c(0, settings, errorLogger);

        std::map<std::string,std::string>::const_iterator it(_obsoletePosixFunctions.begin()), itend(_obsoletePosixFunctions.end());
        for (; it!=itend; ++it) {
            c.reportError(0, Severity::style, "obsoleteFunctions"+it->first, it->second);
        }
    }

    static std::string myName() {
        return "Obsolete functions";
    }

    std::string classInfo() const {
        std::string info = "Warn if any of these obsolete functions are used:\n";
        std::map<std::string,std::string>::const_iterator it(_obsoletePosixFunctions.begin()), itend(_obsoletePosixFunctions.end());
        for (; it!=itend; ++it) {
            info += "- " + it->first + "\n";
        }
        return info;
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkobsoletefunctionsH
