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

#ifndef helpersH
#define helpersH

#include "color.h"
#include "errorlogger.h"
#include "settings.h"
#include "suppressions.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstdio>
#include <fstream>
#include <list>
#include <sstream> // IWYU pragma: keep
#include <string>
#include <utility>

class Token;

class givenACodeSampleToTokenize {
private:
    Tokenizer tokenizer;
    static const Settings settings;

public:
    explicit givenACodeSampleToTokenize(const char sample[], bool createOnly = false, bool cpp = true)
        : tokenizer(&settings, nullptr) {
        std::istringstream iss(sample);
        if (createOnly)
            tokenizer.list.createTokens(iss, cpp ? "test.cpp" : "test.c");
        else
            tokenizer.tokenize(iss, cpp ? "test.cpp" : "test.c");
    }

    const Token* tokens() const {
        return tokenizer.tokens();
    }
};


class SimpleSuppressor : public ErrorLogger {
public:
    SimpleSuppressor(Settings &settings, ErrorLogger *next)
        : settings(settings), next(next) {}
    void reportOut(const std::string &outmsg, Color /*c*/ = Color::Reset) override {
        next->reportOut(outmsg);
    }
    void reportErr(const ErrorMessage &msg) override {
        if (!msg.callStack.empty() && !settings.nomsg.isSuppressed(msg.toSuppressionsErrorMessage()))
            next->reportErr(msg);
    }
private:
    Settings &settings;
    ErrorLogger *next;
};

class ScopedFile {
public:
    ScopedFile(std::string name, const std::string &content) : mName(std::move(name)) {
        std::ofstream of(mName);
        of << content;
    }

    ~ScopedFile() {
        remove(mName.c_str());
    }
private:
    std::string mName;
};

#endif // helpersH
