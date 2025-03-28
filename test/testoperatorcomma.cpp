/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "checkoperatorcomma.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class TestOpComma : public TestFixture {
public:
    TestOpComma() : TestFixture("TestOpComma") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::style).build();

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        runChecks<CheckOpComma>(tokenizer, this);
    }

    void run() override {
        TEST_CASE(commaInIfCondition);
        TEST_CASE(commaInWhileCondition);
        TEST_CASE(commaInForCondition);
        TEST_CASE(commaInForConditionalExp1);
        TEST_CASE(commaInForConditionalExp2);
        TEST_CASE(commaInForConditionalExp3);
    }

    void commaInIfCondition() {
        check("void f() {\n"
              "    int a; a = 100;\n"
              "    if(a == 100, a != 100) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) There is an suspicious comma expression used as a condition.\n", errout_str());
    }

    void commaInWhileCondition() {
        check("void f() {\n"
              "    int a; a = 100;\n"
              "    while(a == 100, a != 100) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) There is an suspicious comma expression used as a condition.\n", errout_str());
    }

    void commaInForCondition() {
        check("void f() {\n"
              "    int a;\n"
              "    for(a = 0;a == 100, a != 100;a++) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) There is an suspicious comma expression used as a condition.\n", errout_str());
    }

    void commaInForConditionalExp1() {
        check("void f() {\n"
              "    int a;int z;\n"
              "    z = (a == 100, a != 100)?100 : 200;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) There is an suspicious comma expression used as a condition.\n", errout_str());
    }

    void commaInForConditionalExp2() {
        check("void f() {\n"
              "    int a;int z;\n"
              "    z = (a == 100, a != 100) && a == 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) There is an suspicious comma expression used as a condition.\n", errout_str());
    }

    void commaInForConditionalExp3() {
        check("void f() {\n"
              "    int a;int z;\n"
              "    z = (a == 100, a != 100) || a == 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) There is an suspicious comma expression used as a condition.\n", errout_str());
    }
};

REGISTER_TEST(TestOpComma)
