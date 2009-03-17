/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
        TEST_CASE(iterator2);
        TEST_CASE(STLSize);
        TEST_CASE(STLSizeNoErr);
        TEST_CASE(erase);
        TEST_CASE(eraseBreak);
        TEST_CASE(eraseReturn);
        TEST_CASE(eraseGoto);
        TEST_CASE(eraseAssign);

        TEST_CASE(pushback1);
        TEST_CASE(invalidcode);
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

    void iterator2()
    {
        check("void foo()\n"
              "{\n"
              "    it = foo.begin();\n"
              "    while (it != bar.end())\n"
              "    {\n"
              "        ++it;\n"
              "    }\n"
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

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "        if (ii == foo.size())\n"
                  "        {\n"
                  "        }\n"
                  "        else\n"
                  "        {\n"
                  "            foo[ii] = 0;\n"
                  "        }\n"
                  "    }\n"
                  "}\n");
            // TODO ASSERT_EQUALS(std::string(""), errout.str());
        }
    }






    void checkErase(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        CheckStl checkStl(&tokenizer, this);
        checkStl.erase();
    }


    void erase()
    {
        checkErase("void f()\n"
                   "{\n"
                   "    for (it = foo.begin(); it != foo.end(); ++it)\n"
                   "    {\n"
                   "        foo.erase(it);\n"
                   "    }\n"
                   "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dangerous usage of erase\n", errout.str());
    }

    void eraseBreak()
    {
        checkErase("void f()\n"
                   "{\n"
                   "    for (it = foo.begin(); it != foo.end(); ++it)\n"
                   "    {\n"
                   "        foo.erase(it);\n"
                   "        break;\n"
                   "    }\n"
                   "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseReturn()
    {
        checkErase("void f()\n"
                   "{\n"
                   "    for (it = foo.begin(); it != foo.end(); ++it)\n"
                   "    {\n"
                   "        foo.erase(it);\n"
                   "        return;\n"
                   "    }\n"
                   "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseGoto()
    {
        checkErase("void f()\n"
                   "{\n"
                   "    for (it = foo.begin(); it != foo.end(); ++it)\n"
                   "    {\n"
                   "        foo.erase(it);\n"
                   "        goto abc;\n"
                   "    }\n"
                   "bar:\n"
                   "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseAssign()
    {
        checkErase("void f()\n"
                   "{\n"
                   "    for (it = foo.begin(); it != foo.end(); ++it)\n"
                   "    {\n"
                   "        foo.erase(it);\n"
                   "        it = foo.begin();\n"
                   "    }\n"
                   "}\n");
        ASSERT_EQUALS("", errout.str());
    }






    void checkPushback(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        CheckStl checkStl(&tokenizer, this);
        checkStl.pushback();
    }


    void pushback1()
    {
        checkPushback("void f()\n"
                      "{\n"
                      "    std::vector<int>::const_iterator it = foo.begin();\n"
                      "    foo.push_back(123);\n"
                      "    *it;\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) After push_back or push_front, the iterator 'it' may be invalid\n", errout.str());
    }

    void invalidcode()
    {
        check("void f()\n"
              "{\n"
              "    for ( \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestStl)
