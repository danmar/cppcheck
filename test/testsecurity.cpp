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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



#include "../src/tokenize.h"
#include "../src/checksecurity.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestSecurity : public TestFixture
{
public:
    TestSecurity() : TestFixture("TestSecurity")
    { }

private:

    void run()
    {
        TEST_CASE(stdin1);
        TEST_CASE(gui);
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        CheckSecurity checkSecurity(&tokenizer, 0, this);
        checkSecurity.readnum();
    }


    void stdin1()
    {
        check("void foo()\n"
              "{\n"
              "    int i;\n"
              "    std::cin >> i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (security) Unvalidated input\n", errout.str());
    }





    void checkGui(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        CheckSecurity checkSecurity(&tokenizer, 0, this);
        checkSecurity.gui();
    }


    void gui()
    {
        // Reading the value of a textbox and converting it into an int without any validation..
        checkGui("void onok()\n"
                 "{\n"
                 "    TEdit *editAbc;\n"
                 "    abc = atoi(editAbc->Text.c_str());\n"
                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (security) Unvalidated input\n", errout.str());
    }
};

REGISTER_TEST(TestSecurity)
