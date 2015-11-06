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
// Obsolete functions
//---------------------------------------------------------------------------

#include "checkobsolescentfunctions.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckObsoleteFunctions instance;
}

void CheckObsoleteFunctions::obsoleteFunctions()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const bool cStandard = _settings->standards.c >= Standards::C99 ;

    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->isName() && tok->varId()==0 && (!tok->function() || !tok->function()->hasBody()) && tok->strAt(1) == "(" &&
                tok->strAt(-1) != "." && (!Token::Match(tok->tokAt(-2), "%name% ::") || Token::simpleMatch(tok->tokAt(-2), "std ::"))) {

                std::map<std::string,std::string>::const_iterator it = _obsoleteStandardFunctions.find(tok->str());
                if (it != _obsoleteStandardFunctions.end()) {
                    // If checking an old code base it might be uninteresting to update obsolete functions.
                    reportError(tok, Severity::style, "obsoleteFunctions"+it->first, it->second);
                } else {
                    if (_settings->standards.posix) {
                        it = _obsoletePosixFunctions.find(tok->str());
                        if (it != _obsoletePosixFunctions.end()) {
                            // If checking an old code base it might be uninteresting to update obsolete functions.
                            reportError(tok, Severity::style, "obsoleteFunctions"+it->first, it->second);
                        }
                    }
                    if (cStandard) {
                        // alloca : this function is obsolete in C but not in C++ (#4382)
                        it = _obsoleteC99Functions.find(tok->str());
                        if (it != _obsoleteC99Functions.end() && !(tok->str() == "alloca" && _tokenizer->isCPP())) {
                            reportError(tok, Severity::style, "obsoleteFunctions"+it->first, it->second);
                        }
                    }
                }
            }
        }
    }
}

void CheckObsoleteFunctions::initObsoleteFunctions()
{
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
//---------------------------------------------------------------------------
