/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include "checkuncommentedmethod.h"
#include "platform.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <string>

class TestCommentedMethods : public TestFixture {
public:
    TestCommentedMethods() : TestFixture("TestCommentedMethods") {}

private:
    Settings settings;

    void run() OVERRIDE {
        settings.severity.enable(Severity::style);

        TEST_CASE(callback1);
        TEST_CASE(callback2);

    }

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    void check_(const char* file, int line, const char code[], Settings::PlatformType platform = Settings::Native) {
        // Clear the error buffer..
        errout.str("");

        settings.platform(platform);

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, "test.cpp"), file, line);

        // Check for unused functions..
        CheckUncommentedMethod checkUncommentedMethod(&tokenizer, &settings, this);
        checkUncommentedMethod.parseTokens(tokenizer,  "someFile.c", &settings);
        // check() returns error if and only if errout is not empty.
        if ((checkUncommentedMethod.check)(this, settings)) {
            ASSERT(errout.str() != "");
        } else {
            ASSERT_EQUALS("", errout.str());
        }
    }

    void callback1() {
        check("void f1()\n"
              "{\n"
              "    void (*f)() = cond ? f1 : NULL;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void callback2() { // #8677
        check("class C {\n"
              "public:\n"
              "    void callback();\n"
              "    void start();\n"
              "};\n"
              "\n"
              "void C::callback() {}\n" // <- not unused
              "\n"
              "void C::start() { ev.set<C, &C::callback>(this); }");
        ASSERT_EQUALS("[test.cpp:9]: (style) The function 'start' is never used.\n", errout.str());
    }
};

REGISTER_TEST(TestUnusedFunctions)
