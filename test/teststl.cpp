/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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
#include "../src/checkstl.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestStl : public TestFixture
{
public:
    TestStl() : TestFixture("TestStl")
    { }

private:

    void run()
    {
        TEST_CASE(iterator1);
        TEST_CASE(STLSize);
        TEST_CASE(STLSizeNoErr);
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        CheckStl checkStl(&tokenizer, this);
        checkStl.iterators();
        checkStl.stlOutOfBounds();
    }


    void iterator1()
    {
        check("void foo()\n"
              "{\n"
              "    for (it = foo.begin(); it != bar.end(); ++it)\n"
              "    { }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Same iterator is used with both foo and bar\n", errout.str());
    }


    void STLSize()
    {
        check("void foo()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
              "    {\n"
              "       foo[ii] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:6]: (error) When ii == size(), foo [ ii ] is out of bounds\n"), errout.str());
    }

    void STLSizeNoErr()
    {
        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii < foo.size(); ++ii)\n"
                  "    {\n"
                  "       foo[ii] = 0;\n"
                  "    }\n"
                  "}\n");
            ASSERT_EQUALS(std::string(""), errout.str());
        }

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "    }\n"
                  "}\n");
            ASSERT_EQUALS(std::string(""), errout.str());
        }
    }
};

REGISTER_TEST(TestStl)
