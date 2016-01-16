/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

#include "astutils.h"
#include "token.h"
#include "settings.h"
#include "tokenize.h"
#include "testsuite.h"


class TestAstUtils : public TestFixture {
public:
    TestAstUtils() : TestFixture("TestAstUtils") {
    }

private:

    void run() {
        TEST_CASE(isReturnScope);
    }

    bool isReturnScope(const char code[], const char filename[]="test.cpp") {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        return ::isReturnScope(Token::findsimplematch(tokenizer.tokens(),"}"));
    }

    void isReturnScope() {
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return; } }"));

    }
};

REGISTER_TEST(TestAstUtils)
