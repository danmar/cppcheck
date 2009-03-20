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
#include "../src/checkclass.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestUnusedPrivateFunction : public TestFixture
{
public:
    TestUnusedPrivateFunction() : TestFixture("TestUnusedPrivateFunction")
    { }

private:
    void run()
    {
        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);
        TEST_CASE(test4);

        // [ 2236547 ] False positive --style unused function, called via pointer
        TEST_CASE(func_pointer);
    }


    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for unused private functions..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.privateFunctions();
    }



    void test1()
    {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int f();\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "unsigned int Fred::f()\n"
              "{ }\n");

        ASSERT_EQUALS(std::string("[test.cpp:4]: (style) Unused private function 'Fred::f'\n"), errout.str());
    }


    void test2()
    {
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "\n"
              "    void a() const\n"
              "    { b(); }\n"
              "private:\n"
              "    void b( ) const\n"
              "    { }\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{ }\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void test3()
    {
        check("class A {\n"
              "public:\n"
              "    A() { }\n"
              "    ~A();\n"
              "private:\n"
              "    void B() { }\n"
              "};\n"
              "\n"
              "A::~A()\n"
              "{ B(); }\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void test4()
    {
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "private:\n"
              "    bool _owner;\n"
              "    void b() { }\n"
              "};\n"
              "\n"
              "A::A() : _owner(false)\n"
              "{ b(); }\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }







    void func_pointer()
    {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    typedef void (*testfp)();\n"
              "\n"
              "    testfp get()\n"
              "    {\n"
              "        return test;\n"
              "    }\n"
              "\n"
              "    static void test()\n"
              "    { }\n"
              "\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{}\n");

        ASSERT_EQUALS(std::string("[test.cpp:6]: (style) Unused private function 'Fred::get'\n"), errout.str());
    }



};

REGISTER_TEST(TestUnusedPrivateFunction)

