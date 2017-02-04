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

    bool isReturnScope(const char code[], int offset) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const tok = (offset < 0)
                                  ? tokenizer.list.back()->tokAt(1+offset)
                                  : tokenizer.tokens()->tokAt(offset);
        return ::isReturnScope(tok);
    }

    void isReturnScope() {
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return; } }", -2));
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return (ab){0}; } }", -2)); // #7103
        ASSERT_EQUALS(false, isReturnScope("void f() { if (a) { return (ab){0}; } }", -4)); // #7103
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -4)); // #7144
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -2)); // #7144
    }
};

REGISTER_TEST(TestAstUtils)
