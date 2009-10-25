/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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



// Check for unused variables..

#include "testsuite.h"
#include "tokenize.h"
#include "checkother.h"

#include <sstream>
extern std::ostringstream errout;

class TestUnusedVar : public TestFixture
{
public:
    TestUnusedVar() : TestFixture("TestUnusedVar")
    { }

private:
    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkStructMemberUsage();
    }

    void run()
    {
        TEST_CASE(structmember1);
        TEST_CASE(structmember2);
        TEST_CASE(structmember3);
        TEST_CASE(structmember4);
        TEST_CASE(structmember5);
        TEST_CASE(structmember6);
    }

    void structmember1()
    {
        check("struct abc\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) struct or union member 'abc::a' is never used\n"
                                  "[test.cpp:4]: (style) struct or union member 'abc::b' is never used\n"
                                  "[test.cpp:5]: (style) struct or union member 'abc::c' is never used\n"), errout.str());
    }

    void structmember2()
    {
        check("struct ABC\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    int a = abc.a;\n"
              "    int b = abc.b;\n"
              "    int c = abc.c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember3()
    {
        check("struct ABC\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n"
              "\n"
              "static struct ABC abc[] = { {1, 2, 3} };\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    int a = abc[0].a;\n"
              "    int b = abc[0].b;\n"
              "    int c = abc[0].c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember4()
    {
        check("struct ABC\n"
              "{\n"
              "    const int a;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    ABC abc;\n"
              "    if (abc.a == 2);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember5()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    void reset()\n"
              "    {\n"
              "        a = 1;\n"
              "        b = 2;\n"
              "    }\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct AB ab;\n"
              "    ab.reset();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember6()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(char *buf)\n"
              "{\n"
              "    struct AB *ab = (struct AB *)&buf[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(char *buf)\n"
              "{\n"
              "    struct AB *ab = (AB *)&buf[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestUnusedVar)


