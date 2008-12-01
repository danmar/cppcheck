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

#include <iostream>
#include <list>

std::ostringstream errout;

/**
 * TestRegistry
 **/

class TestRegistry
{
private:
    std::list<TestFixture *> _tests;

public:
    static TestRegistry &theInstance()
    {
        static TestRegistry testreg;
        return testreg;
    }

    void addTest( TestFixture *t )
    {
        _tests.push_back( t );
    }

    const std::list<TestFixture *> &tests() const
    {
        return _tests;
    }
};




/**
 * TestFixture
 **/

std::ostringstream TestFixture::errmsg;
unsigned int       TestFixture::countTests;

TestFixture::TestFixture(const std::string &_name) : classname(_name)
{
    TestRegistry::theInstance().addTest(this);
}


bool TestFixture::runTest(const char testname[])
{
    countTests++;
    std::cout << classname << "::" << testname << "\n";
    return true;
}

void TestFixture::assertEquals(const char *filename, int linenr, const std::string &expected, const std::string &actual)
{
    if ( expected != actual )
    {
        errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl
               << "Expected:" << std::endl
               << expected << std::endl
               << "Actual:" << std::endl
               << actual << std::endl;
    }
}

void TestFixture::assertEquals(const char *filename, int linenr, unsigned int expected, unsigned int actual)
{
    std::ostringstream ostr1;
    ostr1 << expected;
    std::ostringstream ostr2;
    ostr2 << actual;
    assertEquals( filename, linenr, ostr1.str(), ostr2.str() );
}

void TestFixture::printTests()
{
    const std::list<TestFixture *> &tests = TestRegistry::theInstance().tests();

    for ( std::list<TestFixture *>::const_iterator it = tests.begin(); it != tests.end(); ++it )
    {
        std::cout << (*it)->classname << std::endl;
    }
}

void TestFixture::runTests(const char cmd[])
{
    std::string classname(cmd ? cmd : "");
    std::string testname("");
    if ( classname.find("::") != std::string::npos )
    {
        testname = classname.substr( classname.find("::") + 2 );
        classname.erase( classname.find("::") );
    }

    countTests = 0;
    errmsg.str("");

    const std::list<TestFixture *> &tests = TestRegistry::theInstance().tests();

    for ( std::list<TestFixture *>::const_iterator it = tests.begin(); it != tests.end(); ++it )
    {
        if ( classname.empty() || (*it)->classname == classname )
            (*it)->run();
    }

    std::cout << "\n\nTesting Complete\nNumber of tests: " << countTests << "\n";

    std::cerr << errmsg.str();
}


void TestFixture::reportErr( const std::string &errmsg)
{
    errout << errmsg << std::endl;
}

void TestFixture::reportOut( const std::string &outmsg)
{
    // These can probably be ignored
}
