/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#include "checkexceptionsafety.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestExceptionSafety : public TestFixture
{
public:
    TestExceptionSafety() : TestFixture("TestExceptionSafety")
    { }

private:

    void run()
    {
        TEST_CASE(destructors);
        TEST_CASE(newnew);
        TEST_CASE(switchnewnew);
        TEST_CASE(realloc);
        TEST_CASE(deallocThrow);
    }

    void check(const std::string &code, const std::string &autodealloc = "")
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code.c_str());
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        Settings settings;
        settings.addEnabled("all");
        std::istringstream istr2(autodealloc.c_str());
        settings.autoDealloc(istr2);
        CheckExceptionSafety checkExceptionSafety(&tokenizer, &settings, this);
        checkExceptionSafety.runSimplifiedChecks(&tokenizer, &settings, this);
    }

    void destructors()
    {
        check("x::~x()\n"
              "{\n"
              "    throw e;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Throwing exception in destructor\n", errout.str());
    }

    void newnew()
    {
        check("C::C() : a(new A), b(new B) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Upon exception there is memory leak: a\n", errout.str());

        check("C::C() : a(new A), b(new B) { }", "A\nB\n");
        ASSERT_EQUALS("", errout.str());

        check("C::C()\n"
              "{\n"
              "    a = new A;\n"
              "    b = new B;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Upon exception there is memory leak: a\n", errout.str());

        check("void a()\n"
              "{\n"
              "    A *a1 = new A;\n"
              "    A *a2 = new A;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Upon exception there is memory leak: a1\n", errout.str());

        check("void a()\n"
              "{\n"
              "    A *a1 = new A;\n"
              "    A *a2 = new (std::nothrow) A;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void a()\n"
              "{\n"
              "    A *a1 = new A;\n"
              "    delete a1;\n"
              "    A *a2 = new A;\n"
              "    delete a2;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void a()\n"
              "{\n"
              "    A *a = new A;\n"
              "    B *b = new B;\n"
              "}\n", "A\n");
        ASSERT_EQUALS("", errout.str());

        // passing pointer to unknown function.. the pointer may be added to some list etc..
        check("void f()\n"
              "{\n"
              "    A *a1 = new A;\n"
              "    add(a1);\n"
              "    A *a2 = new A;\n"
              "}\n", "");
        ASSERT_EQUALS("", errout.str());
    }

    void switchnewnew()
    {
        check("int *f(int x)\n"
              "{\n"
              "    int *p = 0;\n"
              "    switch(x)\n"
              "    {\n"
              "        case 1:\n"
              "            p = new int(10);\n"
              "            break;\n"
              "        case 2:\n"
              "            p = new int(100);\n"
              "            break;\n"
              "    };\n"
              "    return p;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc()
    {
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "    void a()\n"
              "    {\n"
              "        delete p;\n"
              "        p = new int[123];\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (style) Upon exception p becomes a dead pointer\n", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "    void a()\n"
              "    {\n"
              "        delete p;\n"
              "        p = new (std::nothrow) int[123];\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "    void a()\n"
              "    {\n"
              "        try {\n"
              "            delete p;\n"
              "            p = new int[123];\n"
              "        } catch (...) { p = 0; }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void deallocThrow()
    {
        check("int * p;\n"
              "void f(int x)\n"
              "{\n"
              "    delete p;\n"
              "    if (x)\n"
              "        throw 123;\n"
              "    p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Throwing exception in invalid state, p points at deallocated memory\n", errout.str());
    }
};

REGISTER_TEST(TestExceptionSafety)

