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

#include "check.h"
#include "color.h"
#include "config.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"

#include <cstddef>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

class options;
class Tokenizer;

class TestFixture : public ErrorLogger {
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;
    static std::size_t fails_counter;
    static std::size_t todos_counter;
    static std::size_t succeeded_todos_counter;
    bool mVerbose{};
    std::string mTemplateFormat;
    std::string mTemplateLocation;
    std::string mTestname;

protected:
    std::string exename;
    std::string testToRun;
    bool quiet_tests{};

    virtual void run() = 0;

    bool prepareTest(const char testname[]);
    virtual void prepareTestInternal() {}
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
    static std::string deleteLineNumber(const std::string &message);

    void setVerbose(bool v) {
        mVerbose = v;
    }

    void setTemplateFormat(const std::string &templateFormat);

    void setMultiline() {
        setTemplateFormat("multiline");
    }

    void processOptions(const options& args);

    template<typename T>
    static T& getCheck()
    {
        for (Check *check : Check::instances()) {
            //cppcheck-suppress [constVariablePointer, useStlAlgorithm] - TODO: fix constVariable FP
            if (T* c = dynamic_cast<T*>(check))
                return *c;
        }
        throw std::runtime_error("instance not found");
    }

    template<typename T>
    static void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
    {
        T& check = getCheck<T>();
        check.runChecks(tokenizer, errorLogger);
    }

    class SettingsBuilder
    {
    public:
        explicit SettingsBuilder(const TestFixture &fixture) : fixture(fixture) {}
        SettingsBuilder(const TestFixture &fixture, Settings settings) : fixture(fixture), settings(std::move(settings)) {}

        SettingsBuilder& severity(Severity::SeverityType sev, bool b = true) {
            if (REDUNDANT_CHECK && settings.severity.isEnabled(sev) == b)
                throw std::runtime_error("redundant setting: severity");
            settings.severity.setEnabled(sev, b);
            return *this;
        }

        SettingsBuilder& certainty(Certainty cert, bool b = true) {
            if (REDUNDANT_CHECK && settings.certainty.isEnabled(cert) == b)
                throw std::runtime_error("redundant setting: certainty");
            settings.certainty.setEnabled(cert, b);
            return *this;
        }

        SettingsBuilder& clang() {
            if (REDUNDANT_CHECK && settings.clang)
                throw std::runtime_error("redundant setting: clang");
            settings.clang = true;
            return *this;
        }

        SettingsBuilder& checkLibrary() {
            if (REDUNDANT_CHECK && settings.checkLibrary)
                throw std::runtime_error("redundant setting: checkLibrary");
            settings.checkLibrary = true;
            return *this;
        }

        SettingsBuilder& checkUnusedTemplates(bool b = true) {
            if (REDUNDANT_CHECK && settings.checkUnusedTemplates == b)
                throw std::runtime_error("redundant setting: checkUnusedTemplates");
            settings.checkUnusedTemplates = b;
            return *this;
        }

        SettingsBuilder& debugwarnings(bool b = true) {
            if (REDUNDANT_CHECK && settings.debugwarnings == b)
                throw std::runtime_error("redundant setting: debugwarnings");
            settings.debugwarnings = b;
            return *this;
        }

        SettingsBuilder& c(Standards::cstd_t std) {
            // TODO: CLatest and C11 are the same - handle differently
            //if (REDUNDANT_CHECK && settings.standards.c == std)
            //    throw std::runtime_error("redundant setting: standards.c");
            settings.standards.c = std;
            return *this;
        }

        SettingsBuilder& cpp(Standards::cppstd_t std) {
            // TODO: CPPLatest and CPP20 are the same - handle differently
            //if (REDUNDANT_CHECK && settings.standards.cpp == std)
            //    throw std::runtime_error("redundant setting: standards.cpp");
            settings.standards.cpp = std;
            return *this;
        }

        SettingsBuilder& library(const char lib[]);

        SettingsBuilder& libraryxml(const char xmldata[], std::size_t len);

        SettingsBuilder& platform(cppcheck::Platform::Type type);

        SettingsBuilder& checkConfiguration() {
            if (REDUNDANT_CHECK && settings.checkConfiguration)
                throw std::runtime_error("redundant setting: checkConfiguration");
            settings.checkConfiguration = true;
            return *this;
        }

        SettingsBuilder& checkHeaders(bool b = true) {
            if (REDUNDANT_CHECK && settings.checkHeaders == b)
                throw std::runtime_error("redundant setting: checkHeaders");
            settings.checkHeaders = b;
            return *this;
        }

        Settings build() {
            return std::move(settings);
        }
    private:
        const TestFixture &fixture;
        Settings settings;

        const bool REDUNDANT_CHECK = false;
    };

    SettingsBuilder settingsBuilder() const {
        return SettingsBuilder(*this);
    }

    SettingsBuilder settingsBuilder(Settings settings) const {
        return SettingsBuilder(*this, std::move(settings));
    }

public:
    void reportOut(const std::string &outmsg, Color c = Color::Reset) override;
    void reportErr(const ErrorMessage &msg) override;
    void run(const std::string &str);
    static void printHelp();
    const std::string classname;

    explicit TestFixture(const char * const _name);

    static std::size_t runTests(const options& args);
};

// TODO: fix these
// NOLINTNEXTLINE(readability-redundant-declaration)
extern std::ostringstream errout;
// NOLINTNEXTLINE(readability-redundant-declaration)
extern std::ostringstream output;

// TODO: most asserts do not actually assert i.e. do not return
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
#define ASSERT_THROW_EQUALS_2( CMD, EXCEPTION, EXPECTED ) do { try { CMD; assertThrowFail(__FILE__, __LINE__); } catch (const EXCEPTION&e) { assertEquals(__FILE__, __LINE__, EXPECTED, e.what()); } catch (...) { assertThrowFail(__FILE__, __LINE__); } } while (false)
#define ASSERT_NO_THROW( CMD ) do { try { CMD; } catch (...) { assertNoThrowFail(__FILE__, __LINE__); } } while (false)
#define TODO_ASSERT_THROW( CMD, EXCEPTION ) do { try { CMD; } catch (const EXCEPTION&) {} catch (...) { assertThrow(__FILE__, __LINE__); } } while (false)
#define TODO_ASSERT( CONDITION ) do { const bool condition=(CONDITION); todoAssertEquals(__FILE__, __LINE__, true, false, condition); } while (false)
#define TODO_ASSERT_EQUALS( WANTED, CURRENT, ACTUAL ) todoAssertEquals(__FILE__, __LINE__, WANTED, CURRENT, ACTUAL)
#define EXPECT_EQ( EXPECTED, ACTUAL ) assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance_ ## CLASSNAME; }

#define LOAD_LIB_2_EXE( LIB, NAME, EXE ) do { if (((LIB).load((EXE), NAME).errorcode != Library::ErrorCode::OK)) throw std::runtime_error("library '" + std::string(NAME) + "' not found"); } while (false)
#define LOAD_LIB_2( LIB, NAME ) LOAD_LIB_2_EXE(LIB, NAME, exename.c_str())

#define PLATFORM( P, T ) do { std::string errstr; assertEquals(__FILE__, __LINE__, true, P.set(cppcheck::Platform::toString(T), errstr, {exename}), errstr); } while (false)

#endif // fixtureH
