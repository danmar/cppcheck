/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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



#include "tokenize.h"
#include "CheckClass.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestConstructors : public TestFixture
{
public:
    TestConstructors() : TestFixture("TestConstructors")
    { }

private:
    void check( const char code[] )
    {
        // Tokenize..
        tokens = tokens_back = NULL;
        std::istringstream istr(code);
        TokenizeCode( istr );
        SimplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        CheckConstructors();

        DeallocateTokens();
    }

    void run()
    {
        TEST_CASE( simple1 );
        TEST_CASE( simple2 );
        TEST_CASE( simple3 );
        TEST_CASE( simple4 );

        TEST_CASE( initvar_with_this );     // BUG 2190300
        TEST_CASE( initvar_if );            // BUG 2190290
        TEST_CASE( initvar_operator_eq );   // BUG 2190376
        TEST_CASE( initvar_same_classname );    // BUG 2208157
    }


    void simple1()
    {
        check( "class Fred\n"
           "{\n"
           "public:\n"
           "    int i;\n"
           "};\n" );
        std::string actual( errout.str() );
        std::string expected( "[test.cpp:1] The class 'Fred' has no constructor\n" );
        ASSERT_EQUALS( expected, actual );
    }


    void simple2()
    {
        check( "class Fred\n"
           "{\n"
           "public:\n"
           "    Fred() { }\n"
           "    int i;\n"
           "};\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4] Uninitialized member variable 'Fred::i'\n"), errout.str() );
    }


    void simple3()
    {
        check( "class Fred\n"
           "{\n"
           "public:\n"
           "    Fred();\n"
           "    int i;\n"
           "};\n"
           "Fred::Fred()\n"
           "{ }\n" );
        ASSERT_EQUALS( std::string("[test.cpp:7] Uninitialized member variable 'Fred::i'\n"), errout.str() );
    }


    void simple4()
    {
        check( "class Fred\n"
           "{\n"
           "public:\n"
           "    Fred();\n"
           "    Fred(int _i);\n"
           "    int i;\n"
           "};\n"
           "Fred::Fred()\n"
           "{ }\n"
           "Fred::Fred(int _i)\n"
           "{\n"
           "    i = _i;\n"
           "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:8] Uninitialized member variable 'Fred::i'\n"), errout.str() );
    }


    void initvar_with_this()
    {
        check( "class Fred\n"
               "{\n"
               "public:\n"
               "    Fred()\n"
               "    { this->i = 0; }\n"
               "    int i;\n"
               "};\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void initvar_if()
    {
        check( "class Fred\n"
               "{\n"
               "public:\n"
               "    Fred()\n"
               "    {\n"
               "        if (true)\n"
               "            i = 0;\n"
               "        else\n"
               "            i = 1;\n"
               "    }\n"
               "    int i;\n"
               "};\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void initvar_operator_eq()
    {
        // Bug 2190376 - False positive, Uninitialized member variable with operator=

        check( "class Fred\n"
               "{\n"
               "private:\n"
               "    int i;\n"
               "\n"
               "public:\n"
               "    Fred()\n"
               "    { i = 0; }\n"
               "\n"
               "    Fred(const Fred &fred)\n"
               "    { *this = fred; }\n"
               "\n"
               "    const Fred & operator=(const Fred &fred)\n"
               "    { i = fred.i; return *this; }\n"
               "};\n" );

        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), err );
    }

    void initvar_same_classname()
    {
        // Bug 2208157 - False positive: Uninitialized variable, same class name

        check( "void func1()\n"
               "{\n"
               "    class Fred\n"
               "    {\n"
               "        int a;\n"
               "        Fred() { a = 0; }\n"
               "    };\n"
               "}\n"
               "\n"
               "void func2()\n"
               "{\n"
               "    class Fred\n"
               "    {\n"
               "        int b;\n"
               "        Fred() { b = 0; }\n"
               "    };\n"
               "}\n" );

        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), err );
    }


};

REGISTER_TEST( TestConstructors )
