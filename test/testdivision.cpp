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


// Check for dangerous division..
// such as "svar / uvar". Treating "svar" as unsigned data is not good


#include "../src/tokenize.h"
#include "../src/checkother.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestDivision : public TestFixture
{
public:
    TestDivision() : TestFixture("TestDivision")
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

        Settings settings;
        settings._showAll = true;
        settings._checkCodingStyle = true;

        // Check for unsigned divisions..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.CheckUnsignedDivision();
    }

    void run()
    {
        TEST_CASE(division1);
        TEST_CASE(division2);
        TEST_CASE(division3);
        TEST_CASE(division4);
        TEST_CASE(division5);
        TEST_CASE(division6);
        TEST_CASE(division7);
    }

    void division1()
    {
        check("void f()\n"
              "{\n"
              "    int ivar = -2;\n"
              "    unsigned int uvar = 2;\n"
              "    return ivar / uvar;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all style) Warning: Division with signed and unsigned operators\n"), errout.str());
    }

    void division2()
    {
        check("void f()\n"
              "{\n"
              "    int ivar = -2;\n"
              "    unsigned int uvar = 2;\n"
              "    return uvar / ivar;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all style) Warning: Division with signed and unsigned operators\n"), errout.str());
    }

    void division3()
    {
        check("typedef int s32;\n"
              "typedef unsigned int u32;\n"
              "void f()\n"
              "{\n"
              "    s32 ivar = -2;\n"
              "    u32 uvar = 2;\n"
              "    return uvar / ivar;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:7]: (all style) Warning: Division with signed and unsigned operators\n"), errout.str());
    }

    void division4()
    {
        check("void f1()\n"
              "{\n"
              "    int i1;\n"
              "}\n"
              "\n"
              "void f2(unsigned int i1)\n"
              "{\n"
              "    unsigned int i2;\n"
              "    result = i2 / i1;}\n"
             );
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void division5()
    {
        check("#define USER_HASH (16)\n"
              "void foo()\n"
              "{\n"
              "    unsigned int val = 32;\n"
              "    val = val / USER_HASH;}\n"
             );
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void division6()
    {
        check("void foo()\n"
              "{\n"
              "    unsigned int val = 32;\n"
              "    int i = val / -2; }\n"
             );
        ASSERT_EQUALS(std::string("[test.cpp:4]: (error) Unsigned division. The result will be wrong.\n"), errout.str());
    }

    void division7()
    {
        check("void foo()\n"
              "{\n"
              "    unsigned int val = 32;\n"
              "    int i = -96 / val; }\n"
             );
        ASSERT_EQUALS(std::string("[test.cpp:4]: (error) Unsigned division. The result will be wrong.\n"), errout.str());
    }
};

REGISTER_TEST(TestDivision)


