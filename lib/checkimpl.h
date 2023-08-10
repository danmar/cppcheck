/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#ifndef checkimplH
#define checkimplH

#include "errortypes.h"
#include "vfvalue.h"

#include <list>
#include <string>

class Settings;
class ErrorLogger;
class Tokenizer;
class Check;
class Token;

class CheckImpl
{
public:
    /** This constructor is used when running checks. */
    CheckImpl(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : mTokenizer(tokenizer), mSettings(settings), mErrorLogger(errorLogger) {}

    CheckImpl(const CheckImpl &) = delete;
    Check& operator=(const CheckImpl &) = delete;

protected:
    const Tokenizer* const mTokenizer{};
    const Settings* const mSettings{};
    ErrorLogger* const mErrorLogger{};

    /** report an error */
    void reportError(const Token *tok, const Severity severity, const std::string &id, const std::string &msg) {
        reportError(tok, severity, id, msg, CWE(0U), Certainty::normal);
    }

    /** report an error */
    void reportError(const Token *tok, const Severity severity, const std::string &id, const std::string &msg, const CWE &cwe, Certainty certainty) {
        const std::list<const Token *> callstack(1, tok);
        reportError(callstack, severity, id, msg, cwe, certainty);
    }

    /** report an error */
    void reportError(const std::list<const Token *> &callstack, Severity severity, const std::string &id, const std::string &msg) {
        reportError(callstack, severity, id, msg, CWE(0U), Certainty::normal);
    }

    /** report an error */
    void reportError(const std::list<const Token *> &callstack, Severity severity, const std::string &id, const std::string &msg, const CWE &cwe, Certainty certainty);

    void reportError(const ErrorPath &errorPath, Severity severity, const char id[], const std::string &msg, const CWE &cwe, Certainty certainty);

    ErrorPath getErrorPath(const Token* errtok, const ValueFlow::Value* value, std::string bug) const;

    /**
     * Use WRONG_DATA in checkers when you check for wrong data. That
     * will call this method
     */
    bool wrongData(const Token *tok, const char *str);

    void logChecker(const char id[]);
};

#endif // checkimplH
