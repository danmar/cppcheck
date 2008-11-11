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
extern bool ShowAll;

class TestUnusedPrivateFunction : public TestFixture
{
public:
    TestUnusedPrivateFunction() : TestFixture("TestUnusedPrivateFunction")
    { }

private:
    void run()
    {
        TEST_CASE( test1 );
    }


    void check( const char code[] )
    {
        // Tokenize..
        tokens = tokens_back = NULL;
        std::istringstream istr(code);
        Tokenizer tokenizer;
        tokenizer.TokenizeCode( istr );

        // Clear the error buffer..
        errout.str("");

        // Check for unused private functions..
        CheckClass checkClass;
        checkClass.CheckUnusedPrivateFunctions();

        tokenizer.DeallocateTokens();
    }



    void test1()
    {
        check( "class Fred\n"
               "{\n"
               "private:\n"
               "    unsigned int f()\n"
               "    { }\n"
               "};\n" );
        // Todo: This should be detected.
        ASSERT_EQUALS( std::string(""), errout.str() );
    }
};

REGISTER_TEST( TestUnusedPrivateFunction )

