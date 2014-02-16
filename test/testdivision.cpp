/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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


// Check for dangerous division..
// such as "svar / uvar". Treating "svar" as unsigned data is not good


#include "tokenize.h"
#include "checkother.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestDivision : public TestFixture {
public:
    TestDivision() : TestFixture("TestDivision") {
    }

private:
    void check(const char code[], bool inconclusive = false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("warning");
        settings.inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for unsigned divisions..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkUnsignedDivision();
    }

    void run() {
        TEST_CASE(division1);
        TEST_CASE(division2);
        TEST_CASE(division4);
        TEST_CASE(division5);
        TEST_CASE(division6);
        TEST_CASE(division7);
        TEST_CASE(division8);
        TEST_CASE(division9);
        TEST_CASE(division10);
        TEST_CASE(division11);  // no error when using "unsigned char" (it is promoted)
    }

    void division1() {
        check("void f() {\n"
              "    int ivar = -2;\n"
              "    unsigned int uvar = 2;\n"
              "    return ivar / uvar;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int ivar = -2;\n"
              "    unsigned int uvar = 2;\n"
              "    return ivar / uvar;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:5]: (warning, inconclusive) Division with signed and unsigned operators. The result might be wrong.\n", errout.str());
    }

    void division2() {
        check("void f()\n"
              "{\n"
              "    int ivar = -2;\n"
              "    unsigned int uvar = 2;\n"
              "    return uvar / ivar;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:5]: (warning, inconclusive) Division with signed and unsigned operators. The result might be wrong.\n", errout.str());
    }

    void division4() {
        check("void f1()\n"
              "{\n"
              "    int i1;\n"
              "}\n"
              "\n"
              "void f2(unsigned int i1)\n"
              "{\n"
              "    unsigned int i2;\n"
              "    result = i2 / i1;"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f1()\n"
              "{\n"
              "    unsigned int num = 0;\n"
              "}\n"
              "\n"
              "void f2(int X)\n"
              "{\n"
              "    X = X / z;"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void division5() {
        check("void foo()\n"
              "{\n"
              "    unsigned int val = 32;\n"
              "    val = val / (16);\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void division6() {
        check("void foo()\n"
              "{\n"
              "    unsigned int val = 32;\n"
              "    int i = val / -2; }");
        ASSERT_EQUALS("[test.cpp:4]: (error) Unsigned division. The result will be wrong.\n", errout.str());
    }

    void division7() {
        check("void foo()\n"
              "{\n"
              "    unsigned int val = 32;\n"
              "    int i = -96 / val; }");
        ASSERT_EQUALS("[test.cpp:4]: (error) Unsigned division. The result will be wrong.\n", errout.str());
    }

    void division8() {
        check("void foo(int b)\n"
              "{\n"
              "    if (b > 0)\n"
              "    {\n"
              "         unsigned int a;\n"
              "         unsigned int c = a / b;\n"
              "    }\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void foo(int b)\n"
              "{\n"
              "    if (b < 0)\n"
              "    {\n"
              "         unsigned int a;\n"
              "         unsigned int c = a / b;\n"
              "    }\n"
              "}", true);
        TODO_ASSERT_EQUALS("[test.cpp:6]: (warning) Division with signed and unsigned operators. The result might be wrong.\n",
                           "", errout.str());

        check("void a(int i) { }\n"
              "int foo( unsigned int sz )\n"
              "{\n"
              "    register unsigned int i=1;\n"
              "    return i/sz;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void division9() {
        check("void f()\n"
              "{\n"
              "    int ivar = -2;\n"
              "    unsigned long uvar = 2;\n"
              "    return ivar / uvar;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:5]: (warning, inconclusive) Division with signed and unsigned operators. The result might be wrong.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int ivar = -2;\n"
              "    unsigned long long uvar = 2;\n"
              "    return ivar / uvar;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:5]: (warning, inconclusive) Division with signed and unsigned operators. The result might be wrong.\n", errout.str());
    }

    void division10() {
        // Ticket: #2932 - don't segfault
        check("i / i", true);
        ASSERT_EQUALS("", errout.str());
    }

    void division11() {
        check("void f(int x, unsigned char y) {\n"
              "    int z = x / y;\n"  // no error, y is promoted
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestDivision)
