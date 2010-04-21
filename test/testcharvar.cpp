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
#include "checkother.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestCharVar : public TestFixture
{
public:
    TestCharVar() : TestFixture("TestCharVar")
    { }

private:


    void run()
    {
        TEST_CASE(array_index);
        TEST_CASE(bitop1);
        TEST_CASE(bitop2);
        TEST_CASE(return1);
        TEST_CASE(assignChar);
        TEST_CASE(and03);
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkCharVariable();
    }

    void array_index()
    {
        check("void foo()\n"
              "{\n"
              "    unsigned char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Warning - using char variable as array index\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    signed char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Warning - using char variable as array index\n", errout.str());

        check("void foo(char ch)\n"
              "{\n"
              "    buf[ch] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Warning - using char variable as array index\n", errout.str());
    }


    void bitop1()
    {
        check("void foo()\n"
              "{\n"
              "    int result = 0;\n"
              "    char ch;\n"
              "    result = a | ch;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Warning - using char variable in bit operation\n", errout.str());
    }

    void bitop2()
    {
        check("void foo()\n"
              "{\n"
              "    char ch;\n"
              "    func(&ch);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void return1()
    {
        check("void foo()\n"
              "{\n"
              "    char c;\n"
              "    return &c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void assignChar()
    {
        check("void foo()\n"
              "{\n"
              "    char c;\n"
              "    c = c & 0x123;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void and03()
    {
        check("void foo()\n"
              "{\n"
              "    char c;\n"
              "    int i = c & 0x03;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestCharVar)

