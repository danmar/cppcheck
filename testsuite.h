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




#include <sstream>
#include "errorlogger.h"

class TOKEN;

class TestFixture : public ErrorLogger
{
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;

protected:
    std::string classname;

    virtual void run()
    { }

    bool runTest(const char testname[]);
    void assertEquals(const char *filename, int linenr, const std::string &expected, const std::string &actual);
    void assertEquals(const char *filename, int linenr, unsigned int expected, unsigned int actual);

public:
    virtual void reportErr( const std::string &errmsg);

    virtual void reportOut( const std::string &outmsg); 

    TestFixture(const std::string &_name);
    virtual ~TestFixture() { }

    static void printTests();
    static void runTests(const char cmd[]);
};


#define TEST_CASE( NAME )  if ( runTest(#NAME) ) NAME ();
#define ASSERT_EQUALS( EXPECTED , ACTUAL )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL);
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance; }

