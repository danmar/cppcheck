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
#include "checknonreentrantfunctions.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestNonReentrantFunctions : public TestFixture
{
public:
    TestNonReentrantFunctions() : TestFixture("TestNonReentrantFunctions")
    { }

private:

    void run()
    {
        TEST_CASE(test_crypt);
    }

    void check(const char code[])
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.posix = true;
        settings.addEnabled("portability");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Assign variable ids
        tokenizer.setVarId();

        // Fill function list
        tokenizer.fillFunctionList();

        // Check for non reentrant functions..
        CheckNonReentrantFunctions checkNonReentrantFunctions(&tokenizer, &settings, this);
        checkNonReentrantFunctions.nonReentrantFunctions();
    }

    void test_crypt()
    {
        check("void f(char *pwd)\n"
              "{\n"
              "    char *cpwd;"
              "    crypt(pwd, cpwd);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Found non reentrant function 'crypt'. For threadsafe applications it is recommended to use the reentrant replacement function 'crypt_r'\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char *pwd = getpass(\"Password:\");"
              "    char *cpwd;"
              "    crypt(pwd, cpwd);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Found non reentrant function 'crypt'. For threadsafe applications it is recommended to use the reentrant replacement function 'crypt_r'\n", errout.str());

        check("int f()\n"
              "{\n"
              "    int crypt = 0;"
              "    return crypt;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestNonReentrantFunctions)


