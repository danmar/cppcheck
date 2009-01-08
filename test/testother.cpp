/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
#include "../src/checkother.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestOther : public TestFixture
{
public:
    TestOther() : TestFixture("TestOther")
    { }

private:


    void run()
    {
        TEST_CASE(delete1);
        TEST_CASE(delete2);
        
        TEST_CASE(sprintf1);    // Dangerous usage of sprintf
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, this);
        checkOther.WarningRedundantCode();
    }

    void delete1()
    {
        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "    {\n"
              "        delete p;\n"
              "        p = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void delete2()
    {
        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "    {\n"
              "        delete p;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: Redundant condition. It is safe to deallocate a NULL pointer\n"), errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "        delete p;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: Redundant condition. It is safe to deallocate a NULL pointer\n"), errout.str());
    }
    
    
    
    void sprintfUsage(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, this);
        checkOther.InvalidFunctionUsage();        
    }
    
    void sprintf1()
    {
        sprintfUsage( "void foo()\n"
                      "{\n"
                      "    char buf[100];\n"
                      "    sprintf(buf,\"%s\",buf);\n"
                      "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Overlapping data buffer buf\n"), errout.str());
    }
};

REGISTER_TEST(TestOther)

