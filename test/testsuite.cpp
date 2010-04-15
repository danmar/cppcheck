/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

#include <iostream>
#include <list>

std::ostringstream errout;
std::ostringstream output;

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

    void addTest(TestFixture *t)
    {
        _tests.push_back(t);
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

size_t TestFixture::fails_counter = 0;
size_t TestFixture::todos_counter = 0;

TestFixture::TestFixture(const std::string &_name) : classname(_name)
{
    TestRegistry::theInstance().addTest(this);
}


bool TestFixture::runTest(const char testname[])
{
    if (testToRun.empty() || testToRun == testname)
    {
        ++countTests;
        std::cout << classname << "::" << testname << "\n";
        return true;
    }
    return false;
}

static std::string writestr(const std::string &str)
{
    std::ostringstream ostr;
    ostr << "\"";
    for (unsigned int i = 0; i < str.length(); ++i)
    {
        char ch = str[i];
        if (ch == '\n')
            ostr << "\\n";
        else if (ch == '\t')
            ostr << "\\t";
        else if (ch == '\"')
            ostr << "\\\"";
        else
            ostr << std::string(1, ch);
    }
    ostr << "\"";
    return ostr.str();
}

void TestFixture::assert(const char *filename, int linenr, bool condition)
{
    if (!condition)
    {
        ++fails_counter;
        errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl;
    }
}

void TestFixture::assertEquals(const char *filename, int linenr, const std::string &expected, const std::string &actual)
{
    if (expected != actual)
    {
        ++fails_counter;

        errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl
               << "Expected:" << std::endl
               << writestr(expected) << std::endl
               << "Actual:" << std::endl
               << writestr(actual) << std::endl;
    }
}

void TestFixture::assertEquals(const char *filename, int linenr, double expected, double actual)
{
    std::ostringstream ostr1;
    ostr1 << expected;
    std::ostringstream ostr2;
    ostr2 << actual;
    assertEquals(filename, linenr, ostr1.str(), ostr2.str());
}

void TestFixture::todoAssertEquals(const char *filename, int linenr, const std::string &expected, const std::string &actual)
{
    if (expected == actual)
        assertEquals(filename, linenr, "TODO assertion", "The assertion succeeded");
    else
        ++todos_counter;
}

void TestFixture::todoAssertEquals(const char *filename, int linenr, unsigned int expected, unsigned int actual)
{
    std::ostringstream ostr1;
    ostr1 << expected;
    std::ostringstream ostr2;
    ostr2 << actual;
    todoAssertEquals(filename, linenr, ostr1.str(), ostr2.str());
}

void TestFixture::assertThrowFail(const char *filename, int linenr)
{
    ++fails_counter;

    errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl
           << "The expected exception was not thrown" << std::endl;
}

void TestFixture::printTests()
{
    const std::list<TestFixture *> &tests = TestRegistry::theInstance().tests();

    for (std::list<TestFixture *>::const_iterator it = tests.begin(); it != tests.end(); ++it)
    {
        std::cout << (*it)->classname << std::endl;
    }
}

void TestFixture::run(const std::string &str)
{
    testToRun = str;
    run();
}

size_t TestFixture::runTests(const char cmd[])
{
    std::string classname(cmd ? cmd : "");
    std::string testname("");
    if (classname.find("::") != std::string::npos)
    {
        testname = classname.substr(classname.find("::") + 2);
        classname.erase(classname.find("::"));
    }

    countTests = 0;
    errmsg.str("");

    const std::list<TestFixture *> &tests = TestRegistry::theInstance().tests();

    for (std::list<TestFixture *>::const_iterator it = tests.begin(); it != tests.end(); ++it)
    {
        if (classname.empty() || (*it)->classname == classname)
        {
            (*it)->run(testname);
        }
    }

    std::cout << "\n\nTesting Complete\nNumber of tests: " << countTests << "\n";
    std::cout << "Number of todos: " << todos_counter << "\n";

    std::cerr << errmsg.str();

    return fails_counter;
}

void TestFixture::reportOut(const std::string & outmsg)
{
    output << outmsg << std::endl;
}

void TestFixture::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    errout << msg.toText() << std::endl;
}
