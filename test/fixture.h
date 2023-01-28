/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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


#ifndef fixtureH
#define fixtureH

#include "color.h"
#include "config.h"
#include "errorlogger.h"

#include <cstddef>
#include <set>
#include <sstream>
#include <string>

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
    std::string mTestname;

protected:
    std::string exename;
    std::string testToRun;
    bool quiet_tests;

    virtual void run() = 0;

    bool prepareTest(const char testname[]);
    std::string getLocationStr(const char * const filename, const unsigned int linenr) const;

    bool assert_(const char * const filename, const unsigned int linenr, const bool condition) const;

    template<typename T, typename U>
    bool assertEquals(const char* const filename, const unsigned int linenr, const T& expected, const U& actual, const std::string& msg = emptyString) const {
        if (expected != actual) {
            std::ostringstream expectedStr;
            expectedStr << expected;
            std::ostringstream actualStr;
            actualStr << actual;

            assertEqualsFailed(filename, linenr, expectedStr.str(), actualStr.str(), msg);
        }
        return expected == actual;
    }

    //Helper function to be called when an assertEquals assertion fails.
    //Writes the appropriate failure message to errmsg and increments fails_counter
    void assertEqualsFailed(const char* const filename, const unsigned int linenr, const std::string& expected, const std::string& actual, const std::string& msg) const;

    bool assertEquals(const char * const filename, const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = emptyString) const;
    void assertEqualsWithoutLineNumbers(const char * const filename, const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = emptyString) const;
    bool assertEquals(const char * const filename, const unsigned int linenr, const char expected[], const std::string& actual, const std::string &msg = emptyString) const;
    bool assertEquals(const char * const filename, const unsigned int linenr, const char expected[], const char actual[], const std::string &msg = emptyString) const;
    bool assertEquals(const char * const filename, const unsigned int linenr, const std::string& expected, const char actual[], const std::string &msg = emptyString) const;
    bool assertEquals(const char * const filename, const unsigned int linenr, const long long expected, const long long actual, const std::string &msg = emptyString) const;
    void assertEqualsDouble(const char * const filename, const unsigned int linenr, const double expected, const double actual, const double tolerance, const std::string &msg = emptyString) const;

    void todoAssertEquals(const char * const filename, const unsigned int linenr, const std::string &wanted,
                          const std::string &current, const std::string &actual) const;
    void todoAssertEquals(const char * const filename, const unsigned int linenr, const char wanted[],
                          const char current[], const std::string &actual) const;
    void todoAssertEquals(const char * const filename, const unsigned int linenr, const long long wanted,
                          const long long current, const long long actual) const;
    void assertThrow(const char * const filename, const unsigned int linenr) const;
    void assertThrowFail(const char * const filename, const unsigned int linenr) const;
    void assertNoThrowFail(const char * const filename, const unsigned int linenr) const;
    static void complainMissingLib(const char * const libname);
    static std::string deleteLineNumber(const std::string &message);

    void setVerbose(bool v) {
        mVerbose = v;
    }

    void setMultiline() {
        mTemplateFormat = "{file}:{line}:{severity}:{message}";
        mTemplateLocation = "{file}:{line}:note:{info}";
    }

    void processOptions(const options& args);
public:
    void reportOut(const std::string &outmsg, Color c = Color::Reset) override;
    void reportErr(const ErrorMessage &msg) override;
    void run(const std::string &str);
    static void printHelp();
    const std::string classname;

    explicit TestFixture(const char * const _name);
    ~TestFixture() override {}

    static std::size_t runTests(const options& args);
};

extern std::ostringstream errout;
extern std::ostringstream output;

#define TEST_CASE( NAME )  do { if (prepareTest(#NAME)) { setVerbose(false); NAME(); } } while (false)
#define ASSERT( CONDITION )  if (!assert_(__FILE__, __LINE__, (CONDITION))) return
#define ASSERT_LOC( CONDITION, FILE_, LINE_ )  assert_(FILE_, LINE_, (CONDITION))
#define CHECK_EQUALS( EXPECTED, ACTUAL )  assertEquals(__FILE__, __LINE__, (EXPECTED), (ACTUAL))
#define ASSERT_EQUALS( EXPECTED, ACTUAL )  if (!assertEquals(__FILE__, __LINE__, (EXPECTED), (ACTUAL))) return
#define ASSERT_EQUALS_WITHOUT_LINENUMBERS( EXPECTED, ACTUAL )  assertEqualsWithoutLineNumbers(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_DOUBLE( EXPECTED, ACTUAL, TOLERANCE )  assertEqualsDouble(__FILE__, __LINE__, EXPECTED, ACTUAL, TOLERANCE)
#define ASSERT_EQUALS_MSG( EXPECTED, ACTUAL, MSG )  assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL, MSG)
#define ASSERT_THROW( CMD, EXCEPTION ) do { try { CMD; assertThrowFail(__FILE__, __LINE__); } catch (const EXCEPTION&) {} catch (...) { assertThrowFail(__FILE__, __LINE__); } } while (false)
#define ASSERT_THROW_EQUALS( CMD, EXCEPTION, EXPECTED ) do { try { CMD; assertThrowFail(__FILE__, __LINE__); } catch (const EXCEPTION&e) { assertEquals(__FILE__, __LINE__, EXPECTED, e.errorMessage); } catch (...) { assertThrowFail(__FILE__, __LINE__); } } while (false)
#define ASSERT_NO_THROW( CMD ) do { try { CMD; } catch (...) { assertNoThrowFail(__FILE__, __LINE__); } } while (false)
#define TODO_ASSERT_THROW( CMD, EXCEPTION ) do { try { CMD; } catch (const EXCEPTION&) {} catch (...) { assertThrow(__FILE__, __LINE__); } } while (false)
#define TODO_ASSERT( CONDITION ) do { const bool condition=(CONDITION); todoAssertEquals(__FILE__, __LINE__, true, false, condition); } while (false)
#define TODO_ASSERT_EQUALS( WANTED, CURRENT, ACTUAL ) todoAssertEquals(__FILE__, __LINE__, WANTED, CURRENT, ACTUAL)
#define EXPECT_EQ( EXPECTED, ACTUAL ) assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance_ ## CLASSNAME; }

#define LOAD_LIB_2( LIB, NAME ) do { \
        if (((LIB).load(exename.c_str(), NAME).errorcode != Library::ErrorCode::OK)) { \
            complainMissingLib(NAME); \
            return; \
        } \
} while (false)

#endif // fixtureH
