/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
    givenACodeSampleToTokenize(const char sample[], bool createOnly = false)
        : _tokenizer(&_settings, 0) {
        std::istringstream iss(sample);
        if (createOnly)
            _tokenizer.list.createTokens(iss);
        else
            _tokenizer.tokenize(iss, "test.cpp");
    }

    const Token* tokens() const {
        return _tokenizer.tokens();
    }
};

#endif//ndef TestUtilsH
