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


// The preprocessor that c++check uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "testsuite.h"
#include "tokenize.h"

class TestTokenizer : public TestFixture
{
public:
    TestTokenizer() : TestFixture("TestTokenizer")
    { }

private:

    void run()
    {
        TEST_CASE( multiline );
    }


    bool cmptok(const char *expected[], const TOKEN *actual)
    {
        unsigned int i = 0;
        for (; expected[i] && actual; ++i, actual = actual->next)
        {
            if ( strcmp( expected[i], actual->str ) != 0)
                return false;
        }
        return (expected[i] == NULL && actual == NULL);
    }


    void multiline()
    {
        const char filedata[] = "#define str \"abc\" \\\n"
                                "            \"def\"\n";

        // tokenize..
        tokens = tokens_back = NULL;
        std::istringstream istr(filedata);
        TokenizeCode(istr, 0);

        // Expected result..
        const char *expected[] = 
        {
            "def",
            "str",
            ";",
            0
        };

        // Compare..
        ASSERT_EQUALS( true, cmptok(expected, tokens) );

        DeallocateTokens();
    }

};

REGISTER_TEST( TestTokenizer )
