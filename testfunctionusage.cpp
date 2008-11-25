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


#define UNIT_TESTING
#include "tokenize.h"
#include "testsuite.h"
#include "CheckFunctionUsage.h"
#include <sstream>

extern std::ostringstream errout;

class TestFunctionUsage : public TestFixture
{
public:
    TestFunctionUsage() : TestFixture("TestFunctionUsage")
    { }

private:


    void run()
    {
        TEST_CASE( incondition );
        TEST_CASE( return1 );
        TEST_CASE( callback1 );
        TEST_CASE( else1 );
    }

    void check( const char code[] )
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize( istr, "test.cpp" );

        // Clear the error buffer..
        errout.str("");

        // Check for unused functions..
        CheckFunctionUsage checkFunctionUsage(this);
        checkFunctionUsage.parseTokens( tokenizer );
        checkFunctionUsage.check();
    }

    void incondition()
    {
        check( "int f1()\n"
               "{\n"
               "    if (f1())\n"
               "    { }\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void return1()
    {
        check( "int f1()\n"
               "{\n"
               "    return f1();\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void callback1()
    {
        check( "void f1()\n"
               "{\n"
               "    void (*f)() = cond ? f1 : NULL;\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void else1()
    {
        check( "void f1()\n"
               "{\n"
               "    if (cond) ;\n"
               "    else f1();\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }
};

REGISTER_TEST( TestFunctionUsage )

