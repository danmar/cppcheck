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

#include <cstring>
#include "testsuite.h"
#include "token.h"

extern std::ostringstream errout;
class TestTOKEN : public TestFixture
{
public:
    TestTOKEN() : TestFixture("TestTOKEN")
    { }

private:

    void run()
    {
        TEST_CASE( nextprevious );

    }

    void nextprevious()
    {
        TOKEN *token = new TOKEN;
        token->str( "1" );
        token->insertToken( "2" );
        token->next()->insertToken( "3" );
        TOKEN *last = token->next()->next();
        ASSERT_EQUALS( token->str(), "1" );
        ASSERT_EQUALS( token->next()->str(), "2" );
        ASSERT_EQUALS( token->next()->next()->str(), "3" );
        if( last->next() )
            ASSERT_EQUALS( "Null was expected", "" );

        ASSERT_EQUALS( last->str(), "3" );
        ASSERT_EQUALS( last->previous()->str(), "2" );
        ASSERT_EQUALS( last->previous()->previous()->str(), "1" );
        if( token->previous() )
            ASSERT_EQUALS( "Null was expected", "" );
    }


};

REGISTER_TEST( TestTOKEN )
