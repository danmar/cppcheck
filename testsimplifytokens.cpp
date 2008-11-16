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



#include "testsuite.h"
#include "tokenize.h"
#include <sstream>

class TestSimplifyTokens : public TestFixture
{
public:
    TestSimplifyTokens() : TestFixture("TestSimplifyTokens")
    { }


private:

    void run()
    {
        TEST_CASE( cast0 );
        TEST_CASE( sizeof1 );
    }

    static std::string tok(const char code[])
    {
        std::istringstream istr(code);
        Tokenizer tokenizer;
        tokenizer.TokenizeCode( istr );
        tokenizer.SimplifyTokenList();

        std::string ret;
        for ( const TOKEN *tok = tokenizer.tokens(); tok; tok = tok->next )
        {
            ret += std::string(tok->str) + " ";
        }

        return ret;
    }

    void cast0()
    {
        const char code1[] = " if ( p == (char *)0 ) ";
        const char code2[] = " if ( p == 0 ) ";
        ASSERT_EQUALS( tok(code1), tok(code2) );
    }

    void sizeof1()
    {
        const char code1[] = " struct ABC *abc = malloc(sizeof(*abc)); ";
        const char code2[] = " struct ABC *abc = malloc(100); ";
        ASSERT_EQUALS( tok(code1), tok(code2) );
    }
};

REGISTER_TEST( TestSimplifyTokens )
