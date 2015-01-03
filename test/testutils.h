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

#ifndef TestUtilsH
#define TestUtilsH

#include "settings.h"
#include "tokenize.h"

class Token;

class givenACodeSampleToTokenize {
private:
    Settings _settings;
    Tokenizer _tokenizer;

public:
    explicit givenACodeSampleToTokenize(const char sample[], bool createOnly = false, bool cpp = true)
        : _tokenizer(&_settings, 0) {
        std::istringstream iss(sample);
        if (createOnly)
            _tokenizer.list.createTokens(iss, cpp ? "test.cpp" : "test.c");
        else
            _tokenizer.tokenize(iss, cpp ? "test.cpp" : "test.c");
    }

    const Token* tokens() const {
        return _tokenizer.tokens();
    }
};


class SimpleSuppressor : public ErrorLogger {
public:
    SimpleSuppressor(Settings &settings, ErrorLogger *next)
        : _settings(settings), _next(next) {
    }
    virtual void reportOut(const std::string &outmsg) {
        _next->reportOut(outmsg);
    }
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg) {
        if (!msg._callStack.empty() && !_settings.nomsg.isSuppressed(msg._id, msg._callStack.begin()->getfile(), msg._callStack.begin()->line))
            _next->reportErr(msg);
    }
private:
    Settings &_settings;
    ErrorLogger *_next;
};

#endif // TestUtilsH
