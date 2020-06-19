/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "config.h"
#include "exprengine.h"
#include "settings.h"
#include "tokenize.h"
#include "testsuite.h"

class TestBughuntingChecks : public TestFixture {
public:
    TestBughuntingChecks() : TestFixture("TestBughuntingChecks") {
    }

private:
    void run() OVERRIDE {
#ifdef USE_Z3
        TEST_CASE(uninit);
#endif
    }

    void check(const char code[]) {
        Settings settings;
        settings.platform(cppcheck::Platform::Unix64);
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        errout.str("");
        ExprEngine::runChecks(this, &tokenizer, &settings);
    }

    void uninit() {
        check("void foo() { int x; x = x + 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that 'x' is initialized\n", errout.str());

        check("void foo() { int x; int y = x + 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that 'x' is initialized\n", errout.str());

        check("void foo() { int x; x++; }");
        TODO_ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that 'x' is initialized\n", "", errout.str());
    }
};

REGISTER_TEST(TestBughuntingChecks)
