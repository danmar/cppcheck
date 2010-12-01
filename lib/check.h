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

#ifndef checkH
#define checkH

#include "token.h"
#include "tokenize.h"
#include "settings.h"
#include "errorlogger.h"

#include <list>
#include <iostream>
#include <set>

/// @addtogroup Core
/// @{

/**
 * @brief Interface class that cppcheck uses to communicate with the checks.
 * All checking classes must inherit from this class
 */
class Check
{
public:
    /** This constructor is used when registering the CheckClass */
    Check()
        : _tokenizer(0), _settings(0), _errorLogger(0)
    {
        instances().push_back(this);
        instances().sort();
    }

    /** This constructor is used when running checks. */
    Check(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : _tokenizer(tokenizer), _settings(settings), _errorLogger(errorLogger)
    { }

    virtual ~Check()
    {
#ifndef DJGPP
        instances().remove(this);
#endif
    }

    /** List of registered check classes. This is used by Cppcheck to run checks and generate documentation */
    static std::list<Check *> &instances()
    {
        static std::list<Check *> _instances;
        return _instances;
    }

    /**
     * analyse code - must be thread safe
     * @param tokens The tokens to analyse
     * @param result container where results are stored
     */
    virtual void analyse(const Token *tokens, std::set<std::string> &result) const
    {
        // suppress compiler warnings
        (void)tokens;
        (void)result;
    }

    /**
     * Save analysis data - the caller ensures thread safety
     * @param data The data where the results are saved
     */
    virtual void saveAnalysisData(const std::set<std::string> &data) const
    {
        // suppress compiler warnings
        (void)data;
    }

    /** run checks, the token list is not simplified */
    virtual void runChecks(const Tokenizer *, const Settings *, ErrorLogger *)
    { }

    /** run checks, the token list is simplified */
    virtual void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) = 0;

    /** get error messages */
    virtual void getErrorMessages() = 0;

    /** class name, used to generate documentation */
    virtual std::string name() const = 0;

    /** get information about this class, used to generate documentation */
    virtual std::string classInfo() const = 0;

    /**
     * Write given error to errorlogger or to out stream in xml format.
     * This is for for printout out the error list with --errorlist
     * @param errmsg Error message to write
     */
    static void reportError(const ErrorLogger::ErrorMessage &errmsg)
    {
        std::cout << errmsg.toXML(true, false) << std::endl;
    }

protected:
    const Tokenizer * const _tokenizer;
    const Settings * const _settings;
    ErrorLogger * const _errorLogger;

    /** report an error */
    void reportError(const Token *tok, const Severity::SeverityType severity, const std::string &id, const std::string &msg)
    {
        std::list<const Token *> callstack;
        if (tok)
            callstack.push_back(tok);
        reportError(callstack, severity, id, msg);
    }

    /** report an error */
    void reportError(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, std::string msg)
    {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        for (std::list<const Token *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it)
        {
            // --errorlist can provide null values here
            if (!(*it))
                continue;

            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.line = (*it)->linenr();
            loc.setfile(_tokenizer->file(*it));
            locationList.push_back(loc);
        }

        ErrorLogger::ErrorMessage errmsg(locationList, severity, msg, id);
        if (_tokenizer && _tokenizer->getFiles() && !_tokenizer->getFiles()->empty())
            errmsg.file0 = _tokenizer->getFiles()->at(0);
        if (_errorLogger)
            _errorLogger->reportErr(errmsg);
        else
            reportError(errmsg);
    }



private:
    /** compare the names of Check classes, used when sorting the Check descendants */
    bool operator<(const Check *other) const
    {
        return (name() < other->name());
    }

    /** disabled assignment operator */
    void operator=(const Check &);

};

/// @}

#endif

