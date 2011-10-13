/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include "redirect.h"

class options;

class Token;

class TestFixture : public ErrorLogger {
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;
    static size_t fails_counter;
    static size_t todos_counter;

protected:
    std::string classname;
    std::string testToRun;
    bool gcc_style_errors;
    bool quiet_tests;

    virtual void run() = 0;

    bool runTest(const char testname[]);

    void assert_(const char *filename, int linenr, bool condition);

    void assertEquals(const char *filename, int linenr, const std::string &expected, const std::string &actual, const std::string &msg = "");
    void assertEquals(const char *filename, int linenr, long long expected, long long actual, const std::string &msg="");
    void assertEqualsDouble(const char *filename, int linenr, double expected, double actual, const std::string &msg="");

    void todoAssertEquals(const char *filename, int linenr, const std::string &wanted,
                          const std::string &current, const std::string &actual);
    void todoAssertEquals(const char *filename, int linenr, unsigned int wanted,
                          unsigned int current, unsigned int actual);
    void assertThrowFail(const char *filename, int linenr);
    void processOptions(const options& args);
public:
    virtual void reportOut(const std::string &outmsg);
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);
    virtual void reportStatus(unsigned int /*fileindex*/, unsigned int /*filecount*/, long /*sizedone*/, long /*sizetotal*/) {}
    void run(const std::string &str);

    TestFixture(const std::string &_name);
    virtual ~TestFixture() { }

    static void printTests();
    static size_t runTests(const options& args);
};

#define TEST_CASE( NAME )  if ( runTest(#NAME) ) { if (quiet_tests) { REDIRECT; NAME(); } else { NAME ();} }
#define ASSERT( CONDITION )  assert_(__FILE__, __LINE__, CONDITION)
#define ASSERT_EQUALS( EXPECTED , ACTUAL )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_DOUBLE( EXPECTED , ACTUAL )  assertEqualsDouble(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_MSG( EXPECTED , ACTUAL, MSG )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL, MSG)
#define ASSERT_THROW( CMD, EXCEPTION ) try { CMD ; assertThrowFail(__FILE__, __LINE__); } catch (EXCEPTION &) { } catch (...) { assertThrowFail(__FILE__, __LINE__); }
#define TODO_ASSERT_EQUALS( WANTED , CURRENT , ACTUAL ) todoAssertEquals(__FILE__, __LINE__, WANTED, CURRENT, ACTUAL)
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance; }

#endif

