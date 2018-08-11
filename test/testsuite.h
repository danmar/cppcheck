/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "config.h"
#include "errorlogger.h"

#include <cstddef>
#include <set>
#include <sstream>

class options;

class TestFixture : public ErrorLogger {
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;
    static std::size_t fails_counter;
    static std::size_t todos_counter;
    static std::size_t succeeded_todos_counter;
    static std::set<std::string> missingLibs;
    bool mVerbose;
    std::string mTemplateFormat;
    std::string mTemplateLocation;

protected:
    std::string testToRun;
    bool quiet_tests;

    virtual void run() = 0;

    bool prepareTest(const char testname[]);

    void assert_(const char * const filename, const unsigned int linenr, const bool condition) const;

    void assertEquals(const char * const filename, const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = emptyString) const;
    void assertEqualsWithoutLineNumbers(const char * const filename, const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = emptyString) const;
    void assertEquals(const char * const filename, const unsigned int linenr, const char expected[], const std::string& actual, const std::string &msg = emptyString) const;
    void assertEquals(const char * const filename, const unsigned int linenr, const char expected[], const char actual[], const std::string &msg = emptyString) const;
    void assertEquals(const char * const filename, const unsigned int linenr, const std::string& expected, const char actual[], const std::string &msg = emptyString) const;
    void assertEquals(const char * const filename, const unsigned int linenr, const long long expected, const long long actual, const std::string &msg = emptyString) const;
    void assertEqualsDouble(const char * const filename, const unsigned int linenr, const double expected, const double actual, const double tolerance, const std::string &msg = emptyString) const;

    void todoAssertEquals(const char * const filename, const unsigned int linenr, const std::string &wanted,
                          const std::string &current, const std::string &actual) const;
    void todoAssertEquals(const char * const filename, const unsigned int linenr, const long long wanted,
                          const long long current, const long long actual) const;
    void assertThrow(const char * const filename, const unsigned int linenr) const;
    void assertThrowFail(const char * const filename, const unsigned int linenr) const;
    void assertNoThrowFail(const char * const filename, const unsigned int linenr) const;
    void complainMissingLib(const char * const libname) const;
    std::string deleteLineNumber(const std::string &message) const;

    void setVerbose(bool v) {
        mVerbose = v;
    }

    void setMultiline() {
        mTemplateFormat = "{file}:{line}:{severity}:{message}";
        mTemplateLocation = "{file}:{line}:note:{info}";
    }

    void processOptions(const options& args);
public:
    virtual void reportOut(const std::string &outmsg) override;
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg) override;
    void run(const std::string &str);
    const std::string classname;

    explicit TestFixture(const char * const _name);
    virtual ~TestFixture() { }

    static std::size_t runTests(const options& args);
};

extern std::ostringstream errout;
extern std::ostringstream output;

#define TEST_CASE( NAME )  if ( prepareTest(#NAME) ) { setVerbose(false); NAME(); }
#define ASSERT( CONDITION )  assert_(__FILE__, __LINE__, CONDITION)
#define ASSERT_EQUALS( EXPECTED , ACTUAL )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_WITHOUT_LINENUMBERS( EXPECTED , ACTUAL )  assertEqualsWithoutLineNumbers(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_DOUBLE( EXPECTED , ACTUAL, TOLERANCE )  assertEqualsDouble(__FILE__, __LINE__, EXPECTED, ACTUAL, TOLERANCE)
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
#define LOAD_LIB_2( LIB, NAME ) { if (((LIB).load("./testrunner", "cfg/" NAME).errorcode != Library::OK) && ((LIB).load("./bin/testrunner", "bin/cfg/" NAME).errorcode != Library::OK)) { complainMissingLib(NAME); return; } }
#endif

#endif
