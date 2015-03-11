/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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


class TestNonReentrantFunctions : public TestFixture {
public:
    TestNonReentrantFunctions() : TestFixture("TestNonReentrantFunctions") {
    }

private:

    void run() {
        TEST_CASE(test_crypt);
        TEST_CASE(test_namespace_handling);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.standards.posix = true;
        settings.addEnabled("portability");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check for non reentrant functions..
        CheckNonReentrantFunctions checkNonReentrantFunctions(&tokenizer, &settings, this);
        checkNonReentrantFunctions.nonReentrantFunctions();
    }

    void test_crypt() {
        check("void f(char *pwd)\n"
              "{\n"
              "    char *cpwd;"
              "    crypt(pwd, cpwd);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'crypt' called. For threadsafe applications it is recommended to use the reentrant replacement function 'crypt_r'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char *pwd = getpass(\"Password:\");"
              "    char *cpwd;"
              "    crypt(pwd, cpwd);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'crypt' called. For threadsafe applications it is recommended to use the reentrant replacement function 'crypt_r'.\n", errout.str());

        check("int f()\n"
              "{\n"
              "    int crypt = 0;"
              "    return crypt;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void test_namespace_handling() {
        check("int f()\n"
              "{\n"
              "    time_t t = 0;"
              "    std::localtime(&t);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'localtime' called. For threadsafe applications it is recommended to use the reentrant replacement function 'localtime_r'.\n", errout.str());

        // Passed as function argument
        check("int f()\n"
              "{\n"
              "    printf(\"Magic guess: %d\n\", getpwent());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'getpwent' called. For threadsafe applications it is recommended to use the reentrant replacement function 'getpwent_r'.\n", errout.str());

        // Pass return value
        check("int f()\n"
              "{\n"
              "    time_t t = 0;"
              "    struct tm *foo = localtime(&t);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'localtime' called. For threadsafe applications it is recommended to use the reentrant replacement function 'localtime_r'.\n", errout.str());

        // Access via global namespace
        check("int f()\n"
              "{\n"
              "    ::getpwent();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'getpwent' called. For threadsafe applications it is recommended to use the reentrant replacement function 'getpwent_r'.\n", errout.str());

        // Be quiet on function definitions
        check("int getpwent()\n"
              "{\n"
              "    return 123;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Be quiet on other namespaces
        check("int f()\n"
              "{\n"
              "    foobar::getpwent();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Be quiet on class member functions
        check("int f()\n"
              "{\n"
              "    foobar.getpwent();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestNonReentrantFunctions)
