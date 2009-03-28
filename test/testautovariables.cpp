/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki, Gianluca Scacco
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
#include "../src/checkautovariables.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestAutoVariables : public TestFixture
{
public:
    TestAutoVariables() : TestFixture("TestAutoVariables")
    { }

private:



    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Assign variable ids
        tokenizer.setVarId();

        // Fill function list
        tokenizer.fillFunctionList();

        // Clear the error buffer..
        errout.str("");

        // Check for buffer overruns..
        Settings settings;
        settings._showAll = true;
        CheckAutoVariables checkAutoVariables(&tokenizer, &settings, this);
        checkAutoVariables.autoVariables();
    }

    void run()
    {
        TEST_CASE(testautovar);
        TEST_CASE(testautovararray);
    }



    void testautovar()
    {
        check("void func1(int **res)\n"
              "{\n"
              "    int num=2;"
              "res=&num;");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (error) Wrong assignement of an auto-variable to an effective parameter of a function\n"), errout.str());
    }
    void testautovararray()
    {
        check("void func1(int* arr[2])\n"
              "{\n"
              "    int num=2;"
              "arr[0]=&num;");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (error) Wrong assignement of an auto-variable to an effective parameter of a function\n"), errout.str());
    }
};

REGISTER_TEST(TestAutoVariables)


