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
    }

    void check( const char code[] )
    {
        // Tokenize..
        Tokenizer tokenizer;
        tokenizer._files.push_back( "test.cpp" );
        std::istringstream istr(code);
        tokenizer.TokenizeCode( istr );

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        CheckFunctionUsage checkFunctionUsage(this);
        checkFunctionUsage.parseTokens( tokenizer );
        checkFunctionUsage.check();
    }

    void incondition()
    {
        check( "int f1()\n"
               "{\n"
               "    f2();\n"
               "}\n"
               "\n"
               "void f2()\n"
               "{\n"
               "    if (f1())\n"
               "    { }\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }
};

REGISTER_TEST( TestFunctionUsage )

