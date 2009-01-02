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



// Check for unused variables..

#define UNIT_TESTING
#include "testsuite.h"
#include "tokenize.h"
#include "checkother.h"

#include <sstream>
extern std::ostringstream errout;

class TestUnusedVar : public TestFixture
{
public:
    TestUnusedVar() : TestFixture("TestUnusedVar")
    { }

private:
    void check( const char code[] )
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize( istr, "test.cpp" );
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        CheckOther checkOther( &tokenizer, this );
        checkOther.CheckStructMemberUsage();
    }

    void run()
    {
        TEST_CASE( structmember1 );
        TEST_CASE( structmember2 );
        TEST_CASE( structmember3 );

        TEST_CASE( localvar1 );
        TEST_CASE( localvar2 );
        TEST_CASE( localvar3 );
        TEST_CASE( localvar4 );
    }

    void structmember1()
    {
        check( "struct abc\n"
               "{\n"
               "    int a;\n"
               "    int b;\n"
               "    int c;\n"
               "};\n" );
        ASSERT_EQUALS( std::string("[test.cpp:2]: struct member 'abc::a' is never read\n"
                                   "[test.cpp:3]: struct member 'abc::b' is never read\n"
                                   "[test.cpp:4]: struct member 'abc::c' is never read\n"), errout.str() );
    }

    void structmember2()
    {
        check( "struct ABC\n"
               "{\n"
               "    int a;\n"
               "    int b;\n"
               "    int c;\n"
               "};\n"
               "\n"
               "void foo()\n"
               "{\n"
               "    struct ABC abc;\n"
               "    int a = abc.a;\n"
               "    int b = abc.b;\n"
               "    int c = abc.c;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void structmember3()
    {
        check( "struct ABC\n"
               "{\n"
               "    int a;\n"
               "    int b;\n"
               "    int c;\n"
               "};\n"
               "\n"
               "static struct ABC abc[] = { {1, 2, 3} };\n"
               "\n"
               "void foo()\n"
               "{\n"
               "    int a = abc[0].a;\n"
               "    int b = abc[0].b;\n"
               "    int c = abc[0].c;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }





    void functionVariableUsage( const char code[] )
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize( istr, "test.cpp" );
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        CheckOther checkOther( &tokenizer, this );
        checkOther.functionVariableUsage();
    }

    void localvar1()
    {
        functionVariableUsage( "void foo()\n"
                               "{\n"
                               "    int i = 0;\n"
                               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:2]: Variable 'i' is assigned a value that is never used\n"), errout.str() );
    }

    void localvar2()
    {
        functionVariableUsage( "void foo()\n"
                               "{\n"
                               "    int i;\n"
                               "    return i;\n"
                               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:2]: Variable 'i' is not assigned a value\n"), errout.str() );
    }

    void localvar3()
    {
        functionVariableUsage( "void foo()\n"
                               "{\n"
                               "    int i;\n"
                               "    if ( abc )\n"
                               "        ;\n"
                               "    else i = 0;\n"
                               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:2]: Variable 'i' is assigned a value that is never used\n"), errout.str() );
    }

    void localvar4()
    {
        functionVariableUsage( "void foo()\n"
                               "{\n"
                               "    int i = 0;\n"
                               "    f(i);\n"
                               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


};

REGISTER_TEST( TestUnusedVar )


