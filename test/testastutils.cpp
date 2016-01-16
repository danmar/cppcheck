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
#include "tokenizer.h"
#include "testsuite.h"


class TestAstUtils : public TestFixture {
public:
    TestAstUtils() : TestFixture("TestAstUtils") {
    }

private:

    void run() {
        TEST_CASE(isReturnScope);
    }

    Tokenizer tokenize(const char code[], const char filename[]) {
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        return tokenizer;
    }

    void isReturnScope() const {
        const Tokenizer &t1 = tokenize("void f() { if (a) { return; } }");
        ASSERT_EQUALS(true, ::isReturnScope(Token::findsimplematch(t1.tokens(),"} }")));

    }
};

REGISTER_TEST(TestAstUtils)
