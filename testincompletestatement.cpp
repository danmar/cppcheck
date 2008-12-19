/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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

class TestIncompleteStatement : public TestFixture
{
public:
    TestIncompleteStatement() : TestFixture("TestIncompleteStatement")
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
        checkOther.CheckIncompleteStatement();
    }

    void run()
    {
        TEST_CASE( test1 );
        TEST_CASE( test2 );
    }

    void test1()
    {
        check( "void foo()\n"
               "{\n"
               "    const char def[] =\n"
               "#ifdef ABC\n"
               "    \"abc\";\n"
               "#else\n"
               "    \"not abc\";\n"
               "#endif\n"
               "}\n" );

        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void test2()
    {
        // Todo: remove the ';' before the string

        check( "void foo()\n"
               "{\n"
               "    ;\"abc\";\n"
               "}\n" );

        ASSERT_EQUALS( std::string("[test.cpp:3]: Redundant code: Found a statement that begins with string constant\n"), errout.str() );
    }
};

REGISTER_TEST( TestIncompleteStatement )


