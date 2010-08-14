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
// Obsolete functions
//---------------------------------------------------------------------------

#include "checkobsoletefunctions.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace
{
CheckObsoleteFunctions instance;
}

void CheckObsoleteFunctions::obsoleteFunctions()
{
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->strAt(1) == "bsd_signal" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionbsd_signal(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "gethostbyaddr" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctiongethostbyaddr(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "gethostbyname" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctiongethostbyname(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "usleep" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionusleep(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "bcmp" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionbcmp(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "bcopy" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionbcopy(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "bzero" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionbzero(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "ecvt" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionecvt(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "fcvt" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionfcvt(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "gcvt" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctiongcvt(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "ftime" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionftime(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "getcontext" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctiongetcontext(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "makecontext" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionmakecontext(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "swapcontext" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionswapcontext(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "getwd" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctiongetwd(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "index" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionindex(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "pthread_attr_getstackaddr" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionpthread_attr_getstackaddr(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "pthread_attr_setstackaddr" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionpthread_attr_setstackaddr(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "rindex" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionrindex(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "scalb" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionscalb(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "ualarm" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionualarm(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "vfork" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionvfork(tok->tokAt(1));
        }
        else if (tok->strAt(1) == "wcswcs" && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
        {
            obsoleteFunctionwcswcs(tok->tokAt(1));
        }
    }
}
//---------------------------------------------------------------------------


void CheckObsoleteFunctions::obsoleteFunctionbsd_signal(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionbsd_signal", "Found obsolete function 'bsd_signal'. It is recommended that new applications use the 'sigaction' function");
}

void CheckObsoleteFunctions::obsoleteFunctiongethostbyaddr(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctiongethostbyaddr", "Found obsolete function 'gethostbyaddr'. It is recommended that new applications use the 'getaddrinfo' function");
}

void CheckObsoleteFunctions::obsoleteFunctiongethostbyname(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctiongethostbyname", "Found obsolete function 'gethostbyname'. It is recommended that new applications use the 'getnameinfo' function");
}

void CheckObsoleteFunctions::obsoleteFunctionusleep(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionusleep", "Found obsolete function 'usleep'. It is recommended that new applications use the 'nanosleep' or 'setitimer' function");
}

void CheckObsoleteFunctions::obsoleteFunctionbcmp(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionbcmp", "Found obsolete function 'bcmp'. It is recommended that new applications use the 'memcmp' function");
}

void CheckObsoleteFunctions::obsoleteFunctionbcopy(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionbcopy", "Found obsolete function 'bcopy'. It is recommended that new applications use the 'memmove' function");
}

void CheckObsoleteFunctions::obsoleteFunctionbzero(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionbzero", "Found obsolete function 'bzero'. It is recommended that new applications use the 'memset' function");
}

void CheckObsoleteFunctions::obsoleteFunctionecvt(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionecvt", "Found obsolete function 'ecvt'. It is recommended that new applications use the 'sprintf' function");
}

void CheckObsoleteFunctions::obsoleteFunctionfcvt(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionfcvt", "Found obsolete function 'fcvt'. It is recommended that new applications use the 'sprintf' function");
}

void CheckObsoleteFunctions::obsoleteFunctiongcvt(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctiongcvt", "Found obsolete function 'gcvt'. It is recommended that new applications use the 'sprintf' function");
}

void CheckObsoleteFunctions::obsoleteFunctionftime(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionftime", "Found obsolete function 'ftime'. It is recommended that new applications use the 'ftime' function. Realtime applications should use ''clock_gettime'' to determine the current time");
}

void CheckObsoleteFunctions::obsoleteFunctiongetcontext(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctiongetcontext", "Found obsolete function 'getcontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads");
}

void CheckObsoleteFunctions::obsoleteFunctionmakecontext(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionmakecontext", "Found obsolete function 'makecontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads");
}

void CheckObsoleteFunctions::obsoleteFunctionswapcontext(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionswapcontext", "Found obsolete function 'swapcontext'. Due to portability issues with this function, applications are recommended to be rewritten to use POSIX threads");
}

void CheckObsoleteFunctions::obsoleteFunctiongetwd(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctiongetwd", "Found obsolete function 'getwd'. It is recommended that new applications use the 'getcwd' function");
}

void CheckObsoleteFunctions::obsoleteFunctionindex(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionindex", "Found obsolete function 'index'. It is recommended to use the function 'strchr' instead");
}

void CheckObsoleteFunctions::obsoleteFunctionpthread_attr_getstackaddr(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionpthread_attr_getstackaddr", "Found obsolete function 'pthread_attr_getstackaddr'.It is recommended that new applications use the 'pthread_attr_getstack' function");
}

void CheckObsoleteFunctions::obsoleteFunctionpthread_attr_setstackaddr(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionpthread_attr_setstackaddr", "Found obsolete function 'pthread_attr_setstackaddr'.It is recommended that new applications use the 'pthread_attr_setstack' function");
}

void CheckObsoleteFunctions::obsoleteFunctionrindex(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionrindex", "Found obsolete function 'rindex'. It is recommended to use the function 'strrchr' instead");
}

void CheckObsoleteFunctions::obsoleteFunctionscalb(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionscalb", "Found obsolete function 'scalb'.It is recommended to use either 'scalbln', 'scalblnf()' or 'scalblnl' instead of this function");
}

void CheckObsoleteFunctions::obsoleteFunctionualarm(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionualarm", "Found obsolete function 'ualarm'.It is recommended to use either 'timer_create', 'timer_delete', 'timer_getoverrun', 'timer_gettime', or 'timer_settime' instead of this function");
}

void CheckObsoleteFunctions::obsoleteFunctionvfork(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionvfork", "Found obsolete function 'vfork'. It is recommended to use the function 'fork' instead");
}

void CheckObsoleteFunctions::obsoleteFunctionwcswcs(const Token *tok)
{
    reportError(tok, Severity::style, "obsoleteFunctionwcswcs", "Found obsolete function 'wcswcs'. It is recommended to use the function 'wcsstr' instead");
}


