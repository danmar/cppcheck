/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include "testsuite.h"
#include "options.h"

#include <iostream>
#include <cstdio>
#include <list>

std::ostringstream errout;
std::ostringstream output;
std::ostringstream warnings;

/**
 * TestRegistry
 **/

class TestRegistry {
private:
    std::list<TestFixture *> _tests;

public:
    static TestRegistry &theInstance() {
        static TestRegistry testreg;
        return testreg;
    }

    void addTest(TestFixture *t) {
        _tests.push_back(t);
    }

    const std::list<TestFixture *> &tests() const {
        return _tests;
    }
};




/**
 * TestFixture
 **/

std::ostringstream TestFixture::errmsg;
unsigned int       TestFixture::countTests;

std::size_t TestFixture::fails_counter = 0;
std::size_t TestFixture::todos_counter = 0;
std::size_t TestFixture::succeeded_todos_counter = 0;
std::set<std::string> TestFixture::missingLibs;

TestFixture::TestFixture(const std::string &_name)
    :classname(_name)
    ,gcc_style_errors(false)
    ,quiet_tests(false)
{
    TestRegistry::theInstance().addTest(this);
}


bool TestFixture::prepareTest(const char testname[])
{
    // Check if tests should be executed
    if (testToRun.empty() || testToRun == testname) {
        // Tests will be executed - prepare them
        ++countTests;
        if (quiet_tests) {
            std::putchar('.'); // Use putchar to write through redirection of std::cout/cerr
            std::fflush(stdout);
        } else {
            std::cout << classname << "::" << testname << std::endl;
        }
        _lib = Library();
        currentTest = classname + "::" + testname;
        return true;
    }
    return false;
}

static std::string writestr(const std::string &str, bool gccStyle = false)
{
    std::ostringstream ostr;
    if (gccStyle)
        ostr << '\"';
    for (std::string::const_iterator i = str.begin(); i != str.end(); ++i) {
        if (*i == '\n') {
            ostr << "\\n";
            if ((i+1) != str.end() && !gccStyle)
                ostr << std::endl;
        } else if (*i == '\t')
            ostr << "\\t";
        else if (*i == '\"')
            ostr << "\\\"";
        else
            ostr << *i;
    }
    if (!str.empty() && !gccStyle)
        ostr << std::endl;
    else if (gccStyle)
        ostr << '\"';
    return ostr.str();
}

void TestFixture::assert_(const char *filename, unsigned int linenr, bool condition) const
{
    if (!condition) {
        ++fails_counter;
        if (gcc_style_errors) {
            errmsg << filename << ':' << linenr << ": Assertion failed." << std::endl;
        } else {
            errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl << "_____" << std::endl;
        }
    }
}

void TestFixture::assertEquals(const char *filename, unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg) const
{
    if (expected != actual) {
        ++fails_counter;
        if (gcc_style_errors) {
            errmsg << filename << ':' << linenr << ": Assertion failed. "
                   << "Expected: "
                   << writestr(expected, true)
                   << ". Actual: "
                   << writestr(actual, true)
                   << '.'
                   << std::endl;
            if (!msg.empty())
                errmsg << msg << std::endl;
        } else {
            errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl
                   << "Expected:" << std::endl
                   << writestr(expected) << std::endl
                   << "Actual:" << std::endl
                   << writestr(actual) << std::endl;
            if (!msg.empty())
                errmsg << "Hint:" << std::endl << msg << std::endl;
            errmsg << "_____" << std::endl;
        }
    }
}
void TestFixture::assertEquals(const char *filename, unsigned int linenr, const char expected[], const std::string& actual, const std::string &msg) const
{
    assertEquals(filename, linenr, std::string(expected), actual, msg);
}
void TestFixture::assertEquals(const char *filename, unsigned int linenr, const char expected[], const char actual[], const std::string &msg) const
{
    assertEquals(filename, linenr, std::string(expected), std::string(actual), msg);
}
void TestFixture::assertEquals(const char *filename, unsigned int linenr, const std::string& expected, const char actual[], const std::string &msg) const
{
    assertEquals(filename, linenr, expected, std::string(actual), msg);
}

void TestFixture::assertEquals(const char *filename, unsigned int linenr, long long expected, long long actual, const std::string &msg) const
{
    assertEquals(filename, linenr, MathLib::toString(expected), MathLib::toString(actual), msg);
}

void TestFixture::assertEqualsDouble(const char *filename, unsigned int linenr, double expected, double actual, const std::string &msg) const
{
    assertEquals(filename, linenr, MathLib::toString(expected), MathLib::toString(actual), msg);
}

void TestFixture::todoAssertEquals(const char *filename, unsigned int linenr,
                                   const std::string &wanted,
                                   const std::string &current,
                                   const std::string &actual) const
{
    if (wanted == actual) {
        if (gcc_style_errors) {
            errmsg << filename << ':' << linenr << ": Assertion succeeded unexpectedly. "
                   << "Result: " << writestr(wanted, true) << "." << std::endl;
        } else {
            errmsg << "Assertion succeeded unexpectedly in " << filename << " at line " << linenr << std::endl
                   << "Result:" << std::endl << writestr(wanted) << std::endl << "_____" << std::endl;
        }
        ++succeeded_todos_counter;
    } else {
        assertEquals(filename, linenr, current, actual);
        ++todos_counter;
    }
}

void TestFixture::todoAssertEquals(const char *filename, unsigned int linenr, long long wanted, long long current, long long actual) const
{
    std::ostringstream wantedStr, currentStr, actualStr;
    wantedStr << wanted;
    currentStr << current;
    actualStr << actual;
    todoAssertEquals(filename, linenr, wantedStr.str(), currentStr.str(), actualStr.str());
}

void TestFixture::assertThrowFail(const char *filename, unsigned int linenr) const
{
    ++fails_counter;
    if (gcc_style_errors) {
        errmsg << filename << ':' << linenr << " Assertion failed. "
               << "The expected exception was not thrown" << std::endl;
    } else {
        errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl
               << "The expected exception was not thrown" << std::endl << "_____" << std::endl;
    }
}

void TestFixture::complainMissingLib(const char* libname) const
{
    missingLibs.insert(libname);
}

void TestFixture::run(const std::string &str)
{
    testToRun = str;
    if (quiet_tests) {
        std::cout << '\n' << classname << ':';
    }
    if (quiet_tests) {
        REDIRECT;
        run();
    } else
        run();
}

void TestFixture::warn(const char msg[]) const
{
    warnings << "Warning: " << currentTest << " " << msg << std::endl;
}

void TestFixture::warnUnsimplified(const std::string& unsimplified, const std::string& simplified)
{
    warn(("Unsimplified code in test case. It looks like this test "
          "should either be cleaned up or moved to TestTokenizer or "
          "TestSimplifyTokens instead.\nactual=" + unsimplified + "\nexpected=" + simplified).c_str());
}

void TestFixture::processOptions(const options& args)
{
    quiet_tests = args.quiet();
    gcc_style_errors = args.gcc_style_errors();
}

std::size_t TestFixture::runTests(const options& args)
{
    std::string classname(args.which_test());
    std::string testname("");
    if (classname.find("::") != std::string::npos) {
        testname = classname.substr(classname.find("::") + 2);
        classname.erase(classname.find("::"));
    }

    countTests = 0;
    errmsg.str("");

    const std::list<TestFixture *> &tests = TestRegistry::theInstance().tests();

    for (std::list<TestFixture *>::const_iterator it = tests.begin(); it != tests.end(); ++it) {
        if (classname.empty() || (*it)->classname == classname) {
            (*it)->processOptions(args);
            (*it)->run(testname);
        }
    }

    const std::string &w(warnings.str());
    if (!w.empty()) {
        std::cout << "\n\n" << w;
    }
    std::cout << "\n\nTesting Complete\nNumber of tests: " << countTests << std::endl;
    std::cout << "Number of todos: " << todos_counter;
    if (succeeded_todos_counter > 0)
        std::cout << " (" << succeeded_todos_counter << " succeeded)";
    std::cout << std::endl;
    // calling flush here, to do all output before the error messages (in case the output is buffered)
    std::cout.flush();

    std::cerr << "Tests failed: " << fails_counter << std::endl << std::endl;
    std::cerr << errmsg.str();

    if (!missingLibs.empty()) {
        std::cerr << "Missing libraries: ";
        for (std::set<std::string>::const_iterator i = missingLibs.cbegin(); i != missingLibs.cend(); ++i)
            std::cerr << *i << "  ";
        std::cerr << std::endl << std::endl;
    }
    std::cerr.flush();
    return fails_counter;
}

void TestFixture::reportOut(const std::string & outmsg)
{
    output << outmsg << std::endl;
}

void TestFixture::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    const std::string errormessage(msg.toString(false));
    if (errout.str().find(errormessage) == std::string::npos)
        errout << errormessage << std::endl;
}
