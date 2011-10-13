/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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


#include "tokenize.h"
#include "check64bit.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class Test64BitPortability : public TestFixture {
public:
    Test64BitPortability() : TestFixture("Test64BitPortability")
    { }

private:


    void run() {
        TEST_CASE(novardecl);
        TEST_CASE(functionpar);
        TEST_CASE(structmember);
        TEST_CASE(ptrcompare);
        TEST_CASE(ptrarithmetic);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("portability");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Check char variable usage..
        Check64BitPortability check64BitPortability(&tokenizer, &settings, this);
        check64BitPortability.pointerassignment();
    }

    void novardecl() {
        // if the variable declarations can't be seen then skip the warning
        check("void foo()\n"
              "{\n"
              "    a = p;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void functionpar() {
        check("int foo(int *p)\n"
              "{\n"
              "    int a = p;\n"
              "    return a + 4;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning an address value to the integer (int/long/etc) type is not portable\n", errout.str());

        check("int foo(int p[])\n"
              "{\n"
              "    int a = p;\n"
              "    return a + 4;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning an address value to the integer (int/long/etc) type is not portable\n", errout.str());

        check("int foo(int p[])\n"
              "{\n"
              "    int *a = p;\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    int *p = x;\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning an integer (int/long/etc) to a pointer is not portable\n", errout.str());
    }

    void structmember() {
        check("struct Foo { int *p };\n"
              "void f(struct Foo *foo) {\n"
              "    int i = foo->p;\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning an address value to the integer (int/long/etc) type is not portable\n", "", errout.str());
    }

    void ptrcompare() {
        // Ticket #2892
        check("void foo(int *p) {\n"
              "    int a = (p != NULL);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void ptrarithmetic() {
        // #3073
        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = p + x;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = x + p;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = x * x;\n"
              "}\n");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("void foo(int *start, int *end) {\n"
              "    int len;\n"
              "    int len = end + 10 - start;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(Test64BitPortability)

