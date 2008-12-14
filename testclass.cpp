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
#include "checkclass.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestClass : public TestFixture
{
public:
    TestClass() : TestFixture("TestClass")
    { }

private:

    void run()
    {
        TEST_CASE( virtualDestructor1 );	// Base class not found => no error
        TEST_CASE( virtualDestructor2 );    // Base class doesn't have a destructor
        TEST_CASE( virtualDestructor3 );	// Base class has a destructor, but it's not virtual
    }

    // Check that base classes have virtual destructors
    void checkVirtualDestructor(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize( istr, "test.cpp" );
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        CheckClass checkClass( &tokenizer, settings, this );
        checkClass.virtualDestructor();
    }


    void virtualDestructor1()
    {
		// Base class not found

        checkVirtualDestructor("class Derived : public Base { };");
        ASSERT_EQUALS( std::string(""), errout.str() );

        checkVirtualDestructor("class Derived : Base { };");
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void virtualDestructor2()
    {
		// Base class doesn't have a destructor

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : public Base { };");
        ASSERT_EQUALS( std::string("[test.cpp:1]: Base class Base doesn't have a virtual destructor\n"), errout.str() );

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : Base { };");
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void virtualDestructor3()
    {
		// Base class has a destructor, but it's not virtual

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { };");
        ASSERT_EQUALS( std::string("[test.cpp:1]: The destructor for the base class Base is not virtual\n"), errout.str() );

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { };");
        ASSERT_EQUALS( std::string("[test.cpp:1]: The destructor for the base class Base is not virtual\n"), errout.str() );
    }
};

REGISTER_TEST( TestClass )
