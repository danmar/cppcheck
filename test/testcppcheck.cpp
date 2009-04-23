/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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


// The preprocessor that c++check uses is a bit special. Instead of generating
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
        ASSERT_EQUALS("[file.cpp:5]: (error) Using 'foo' after it is deallocated / released\n", errout.str());
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
};

REGISTER_TEST(TestCppcheck)
