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


#define UNIT_TESTING
#include "tokenize.h"
#include "checkother.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestCharVar : public TestFixture
{
public:
    TestCharVar() : TestFixture("TestCharVar")
    { }

private:


    void run()
    {
        TEST_CASE( array_index );
        TEST_CASE( bitop1 );
        TEST_CASE( bitop2 );
    }

    void check( const char code[] )
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize( istr, "test.cpp" );
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check char variable usage..
        CheckOther checkOther( &tokenizer, this );
        checkOther.CheckCharVariable();
    }

    void array_index()
    {
        check( "void foo()\n"
               "{\n"
               "    unsigned char ch = 0x80;\n"
               "    buf[ch] = 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );

        check( "void foo()\n"
               "{\n"
               "    char ch = 0x80;\n"
               "    buf[ch] = 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Warning - using char variable as array index\n"), errout.str() );

        check( "void foo(char ch)\n"
               "{\n"
               "    buf[ch] = 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:3]: Warning - using char variable as array index\n"), errout.str() );
    }


    void bitop1()
    {
        check( "void foo()\n"
               "{\n"
               "    char ch;\n"
               "    result = a | ch;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Warning - using char variable in bit operation\n"), errout.str() );
    }

    void bitop2()
    {
        check( "void foo()\n"
               "{\n"
               "    char ch;\n"
               "    func(&ch);\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }
};

REGISTER_TEST( TestCharVar )

