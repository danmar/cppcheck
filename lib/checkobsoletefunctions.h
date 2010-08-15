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
    { }

    /** This constructor is used when running checks. */
    CheckObsoleteFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckObsoleteFunctions checkObsoleteFunctions(tokenizer, settings, errorLogger);
        checkObsoleteFunctions.obsoleteFunctions();
    }

    /** Check for obsolete functions */
    void obsoleteFunctions();

private:
    /** Report Error : Using obsolete function 'bsd_signal' */
    void obsoleteFunctionbsd_signal(const Token *tok);

    /** Report Error : Using obsolete function 'gethostbyaddr' */
    void obsoleteFunctiongethostbyaddr(const Token*);

    /** Report Error : Using obsolete function 'gethostbyname' */
    void obsoleteFunctiongethostbyname(const Token*);

    /** Report Error : Using obsolete function 'usleep' */
    void obsoleteFunctionusleep(const Token*);

    /** Report Error : Using obsolete function 'bcmp' */
    void obsoleteFunctionbcmp(const Token *tok);

    /** Report Error : Using obsolete function 'bcopy' */
    void obsoleteFunctionbcopy(const Token *tok);

    /** Report Error : Using obsolete function 'bzero' */
    void obsoleteFunctionbzero(const Token *tok);

    /** Report Error : Using obsolete function 'ecvt' */
    void obsoleteFunctionecvt(const Token *tok);

    /** Report Error : Using obsolete function 'fcvt' */
    void obsoleteFunctionfcvt(const Token *tok);

    /** Report Error : Using obsolete function 'gcvt' */
    void obsoleteFunctiongcvt(const Token *tok);

    /** Report Error : Using obsolete function 'ftime' */
    void obsoleteFunctionftime(const Token *tok);

    /** Report Error : Using obsolete function 'getcontext' */
    void obsoleteFunctiongetcontext(const Token *tok);

    /** Report Error : Using obsolete function 'makecontext' */
    void obsoleteFunctionmakecontext(const Token *tok);

    /** Report Error : Using obsolete function 'swapcontext' */
    void obsoleteFunctionswapcontext(const Token *tok);

    /** Report Error : Using obsolete function 'getwd' */
    void obsoleteFunctiongetwd(const Token *tok);

    /** Report Error : Using obsolete function 'index' */
    void obsoleteFunctionindex(const Token *tok);

    /** Report Error : Using obsolete function 'pthread_attr_getstackaddr' */
    void obsoleteFunctionpthread_attr_getstackaddr(const Token *tok);

    /** Report Error : Using obsolete function 'pthread_attr_setstackaddr' */
    void obsoleteFunctionpthread_attr_setstackaddr(const Token *tok);

    /** Report Error : Using obsolete function 'rindex' */
    void obsoleteFunctionrindex(const Token *tok);

    /** Report Error : Using obsolete function 'scalb' */
    void obsoleteFunctionscalb(const Token *tok);

    /** Report Error : Using obsolete function 'ualarm' */
    void obsoleteFunctionualarm(const Token *tok);

    /** Report Error : Using obsolete function 'vfork' */
    void obsoleteFunctionvfork(const Token *tok);

    /** Report Error : Using obsolete function 'wcswcs' */
    void obsoleteFunctionwcswcs(const Token *tok);

    void getErrorMessages()
    {
        obsoleteFunctionbsd_signal(0);
        obsoleteFunctiongethostbyaddr(0);
        obsoleteFunctiongethostbyname(0);
        obsoleteFunctionusleep(0);
        obsoleteFunctionbcmp(0);
        obsoleteFunctionbcopy(0);
        obsoleteFunctionbzero(0);
        obsoleteFunctionecvt(0);
        obsoleteFunctionfcvt(0);
        obsoleteFunctiongcvt(0);
        obsoleteFunctionftime(0);
        obsoleteFunctiongetcontext(0);
        obsoleteFunctionmakecontext(0);
        obsoleteFunctionswapcontext(0);
        obsoleteFunctiongetwd(0);
        obsoleteFunctionindex(0);
        obsoleteFunctionpthread_attr_getstackaddr(0);
        obsoleteFunctionpthread_attr_setstackaddr(0);
        obsoleteFunctionrindex(0);
        obsoleteFunctionscalb(0);
        obsoleteFunctionualarm(0);
        obsoleteFunctionvfork(0);
        obsoleteFunctionwcswcs(0);
    }

    std::string name() const
    {
        return "Obsolete functions";
    }

    std::string classInfo() const
    {
        return "Warn if any of these obsolete functions are used:\n"
               "* bsd_signal\n"
               "* gethostbyaddr\n"
               "* gethostbyname\n"
               "* usleep\n"
               "* bcmp\n"
               "* bcopy\n"
               "* bzero\n"
               "* ecvt\n"
               "* fcvt\n"
               "* gcvt\n"
               "* ftime\n"
               "* getcontext\n"
               "* makecontext\n"
               "* swapcontext\n"
               "* getwd\n"
               "* index\n"
               "* pthread_attr_getstackaddr\n"
               "* pthread_attr_setstackaddr\n"
               "* rindex\n"
               "* scalb\n"
               "* ualarm\n"
               "* vfork\n"
               "* wcswcs\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif

