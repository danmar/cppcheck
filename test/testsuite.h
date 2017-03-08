/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include <set>
#include "errorlogger.h"

class options;

class TestFixture : public ErrorLogger {
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;
    static std::size_t fails_counter;
    static std::size_t todos_counter;
    static std::size_t succeeded_todos_counter;
    static std::set<std::string> missingLibs;

protected:
    std::string classname;
    std::string testToRun;
    bool quiet_tests;

    virtual void run() = 0;

    bool prepareTest(const char testname[]);

    void assert_(const char *filename, unsigned int linenr, bool condition) const;

    void assertEquals(const char *filename, unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = emptyString) const;
    void assertEquals(const char *filename, unsigned int linenr, const char expected[], const std::string& actual, const std::string &msg = emptyString) const;
    void assertEquals(const char *filename, unsigned int linenr, const char expected[], const char actual[], const std::string &msg = emptyString) const;
    void assertEquals(const char *filename, unsigned int linenr, const std::string& expected, const char actual[], const std::string &msg = emptyString) const;
    void assertEquals(const char *filename, unsigned int linenr, long long expected, long long actual, const std::string &msg = emptyString) const;
    void assertEqualsDouble(const char *filename, unsigned int linenr, double expected, double actual, const std::string &msg = emptyString) const;

    void todoAssertEquals(const char *filename, unsigned int linenr, const std::string &wanted,
                          const std::string &current, const std::string &actual) const;
    void todoAssertEquals(const char *filename, unsigned int linenr, long long wanted,
                          long long current, long long actual) const;
    void assertThrow(const char *filename, unsigned int linenr) const;
    void assertThrowFail(const char *filename, unsigned int linenr) const;
    void assertNoThrowFail(const char *filename, unsigned int linenr) const;
    void complainMissingLib(const char* libname) const;

    void processOptions(const options& args);
public:
    virtual void reportOut(const std::string &outmsg);
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);
    void run(const std::string &str);

    explicit TestFixture(const char* _name);
    virtual ~TestFixture() { }

    static std::size_t runTests(const options& args);
};

extern std::ostringstream errout;
extern std::ostringstream output;

#define TEST_CASE( NAME )  if ( prepareTest(#NAME) ) { NAME(); }
#define ASSERT( CONDITION )  assert_(__FILE__, __LINE__, CONDITION)
#define ASSERT_EQUALS( EXPECTED , ACTUAL )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_DOUBLE( EXPECTED , ACTUAL )  assertEqualsDouble(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_MSG( EXPECTED , ACTUAL, MSG )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL, MSG)
#define ASSERT_THROW( CMD, EXCEPTION ) try { CMD ; assertThrowFail(__FILE__, __LINE__); } catch (const EXCEPTION&) { } catch (...) { assertThrowFail(__FILE__, __LINE__); }
#define ASSERT_NO_THROW( CMD ) try { CMD ; } catch (...) { assertNoThrowFail(__FILE__, __LINE__); }
#define TODO_ASSERT_THROW( CMD, EXCEPTION ) try { CMD ; } catch (const EXCEPTION&) { } catch (...) { assertThrow(__FILE__, __LINE__); }
#define TODO_ASSERT( CONDITION ) { bool condition=CONDITION; todoAssertEquals(__FILE__, __LINE__, true, false, condition); }
#define TODO_ASSERT_EQUALS( WANTED , CURRENT , ACTUAL ) todoAssertEquals(__FILE__, __LINE__, WANTED, CURRENT, ACTUAL)
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance_##CLASSNAME; }

#ifdef _WIN32
#define LOAD_LIB_2( LIB, NAME ) { if (((LIB).load("./testrunner", "../cfg/" NAME).errorcode != Library::OK) && ((LIB).load("./testrunner", "cfg/" NAME).errorcode != Library::OK)) { complainMissingLib(NAME); return; } }
#else
#define LOAD_LIB_2( LIB, NAME ) { if ((LIB).load("./testrunner", "cfg/" NAME).errorcode != Library::OK) { complainMissingLib(NAME); return; } }
#endif

#endif
