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

#include "tokenize.h"
#include "checkother.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestOther : public TestFixture
{
public:
    TestOther() : TestFixture("TestOther")
    { }

private:


    void run()
    {
        // TODO TEST_CASE( delete1 );
    }

    void check( const char code[] )
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize( istr, "test.cpp" );

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        CheckOther checkOther( &tokenizer, this );
        checkOther.WarningRedundantCode();
    }

    void delete1()
    {
        check( "void foo()\n"
               "{\n"
               "    if (p)\n"
               "    {\n"
               "        delete p;\n"
               "        abc = 123;\n"
               "    }\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }
};

REGISTER_TEST( TestOther )

