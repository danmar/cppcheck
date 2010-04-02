/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef testsuiteH
#define testsuiteH

#include <sstream>
#include "errorlogger.h"

class Token;

class TestFixture : public ErrorLogger
{
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;
    static size_t fails_counter;
    static size_t todos_counter;

protected:
    std::string classname;
    std::string testToRun;

    virtual void run() = 0;

    bool runTest(const char testname[]);

    void assertEquals(const char *filename, int linenr, const std::string &expected, const std::string &actual);

    // the vars expected and actual need to be of type double, in order to avoid overflow of unsigned int
    // e.g: ASSERT_EQUALS(-100.0, MathLib::toDoubleNumber("-1.0E+2")); whould not work without this.
    void assertEquals(const char *filename, int linenr, double expected, double actual);

    void todoAssertEquals(const char *filename, int linenr, const std::string &expected, const std::string &actual);
    void todoAssertEquals(const char *filename, int linenr, unsigned int expected, unsigned int actual);
    void assertThrowFail(const char *filename, int linenr);

public:
    virtual void reportOut(const std::string &outmsg);
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);
    virtual void reportStatus(unsigned int /*index*/, unsigned int /*max*/) {}
    void run(const std::string &str);

    TestFixture(const std::string &_name);
    virtual ~TestFixture() { }

    static void printTests();
    static size_t runTests(const char cmd[]);
};


#define TEST_CASE( NAME )  if ( runTest(#NAME) ) NAME ();
#define ASSERT_EQUALS( EXPECTED , ACTUAL )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_THROW( CMD, EXCEPTION ) try { CMD ; assertThrowFail(__FILE__, __LINE__); } catch (EXCEPTION &) { } catch (...) { assertThrowFail(__FILE__, __LINE__); }
#define TODO_ASSERT_EQUALS( EXPECTED , ACTUAL ) todoAssertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance; }

#endif

