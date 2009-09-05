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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "testsuite.h"
#include "../src/cppcheck.h"

#include <map>
#include <string>

extern std::ostringstream errout;

class TestCppcheck : public TestFixture
{
public:
    TestCppcheck() : TestFixture("TestCppcheck")
    { }

private:

    void check(const std::string &data)
    {
        errout.str("");
        CppCheck cppCheck(*this);
        cppCheck.addFile("file.cpp", data);
        cppCheck.check();
    }

    void run()
    {
        TEST_CASE(linenumbers);
        // TEST_CASE(linenumbers2);

        TEST_CASE(xml);

        TEST_CASE(include);
        TEST_CASE(templateFormat);
    }

    void linenumbers()
    {
        const char filedata[] = "void f()\n"
                                "{\n"
                                "  char *foo = new char[10];\n"
                                "  delete [] foo;\n"
                                "  foo[3] = 0;\n"
                                "}\n";
        check(filedata);

        // Compare results..
        ASSERT_EQUALS("[file.cpp:5]: (error) Dereferencing 'foo' after it is deallocated / released\n", errout.str());
    }

    void linenumbers2()
    {
        const char filedata[] = "void f()\n"
                                "{\n"
                                "  char *string;\n"
                                "  string = new char[20];\n"
                                "  string = new char[30];\n"
                                "  delete [] string;\n"
                                "}\n";
        check(filedata);

        // Compare results..
        ASSERT_EQUALS("[file.cpp:5]: (error) Memory leak: string\n", errout.str());
    }


    void xml()
    {
        // Test the errorlogger..
        ErrorLogger::ErrorMessage errmsg;
        errmsg._msg = "ab<cd>ef";
        ASSERT_EQUALS("<error id=\"\" severity=\"\" msg=\"ab&lt;cd&gt;ef\"/>", errmsg.toXML());
    }


    void include()
    {
        ErrorLogger::ErrorMessage errmsg;
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.file = "ab/cd/../ef.h";
        errmsg._callStack.push_back(loc);
        ASSERT_EQUALS("<error file=\"ab/ef.h\" line=\"0\" id=\"\" severity=\"\" msg=\"\"/>", errmsg.toXML());
        ASSERT_EQUALS("[ab/ef.h:0]: ", errmsg.toText());
    }

    void templateFormat()
    {
        ErrorLogger::ErrorMessage errmsg;
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.file = "some/{file}file.cpp";
        loc.line = 10;
        errmsg._callStack.push_back(loc);
        errmsg._id = "testId";
        errmsg._severity = "testSeverity";
        errmsg._msg = "long testMessage";
        ASSERT_EQUALS("<error file=\"some/{file}file.cpp\" line=\"10\" id=\"testId\" severity=\"testSeverity\" msg=\"long testMessage\"/>", errmsg.toXML());
        ASSERT_EQUALS("[some/{file}file.cpp:10]: (testSeverity) long testMessage", errmsg.toText());
        ASSERT_EQUALS("testId-some/{file}file.cpp,testSeverity.10?{long testMessage}", errmsg.toText("{id}-{file},{severity}.{line}?{{message}}"));
    }
};

REGISTER_TEST(TestCppcheck)
