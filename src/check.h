/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef checkH
#define checkH

#include "token.h"
#include "tokenize.h"
#include "settings.h"
#include "errorlogger.h"

#include <list>
#include <iostream>

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

    /** This constructor is used when running checks.. */
    Check(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : _tokenizer(tokenizer), _settings(settings), _errorLogger(errorLogger)
    { }

    virtual ~Check()
    {
        instances().remove(this);
    }

    /** get instances of this */
    static std::list<Check *> &instances()
    {
        static std::list<Check *> _instances;
        return _instances;
    }

    /** run checks, the token list is not simplified */
    virtual void runChecks(const Tokenizer *, const Settings *, ErrorLogger *)
    { }

    /** run checks, the token list is simplified */
    virtual void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) = 0;

    /** get error messages */
    virtual void getErrorMessages() = 0;

    /** class name */
    virtual std::string name() const = 0;

    /** get information about this class */
    virtual std::string classInfo() const = 0;

protected:
    const Tokenizer * const _tokenizer;
    const Settings * const _settings;
    ErrorLogger * const _errorLogger;

    /** report an error */
    void reportError(const Token *tok, const Severity::e severity, const std::string &id, const std::string &msg)
    {
        std::list<const Token *> callstack;
        callstack.push_back(tok);
        reportError(callstack, severity, id, msg);
    }

    /** report an error */
    void reportError(const std::list<const Token *> &callstack, const Severity::e severity, const std::string &id, std::string msg)
    {
        // No errorLogger => just report the message to stdout
        if (_errorLogger == NULL)
        {
            std::cout << "(" << Severity::stringify(severity) << ") " << msg << std::endl;
            return;
        }

        // If the verbose flag hasn't been given, don't show verbose information
        if (!_settings || !_settings->_verbose)
        {
            std::string::size_type pos = msg.find("\n");
            if (pos != std::string::npos)
                msg.erase(pos);
        }

        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        for (std::list<const Token *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it)
        {
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.line = (*it)->linenr();
            loc.file = _tokenizer->file(*it);
            locationList.push_back(loc);
        }

        _errorLogger->reportErr(ErrorLogger::ErrorMessage(locationList, Severity::stringify(severity), msg, id));
    }

private:
    /** compare the names of Check classes */
    bool operator<(const Check *other) const
    {
        return (name() < other->name());
    }

};



#endif

