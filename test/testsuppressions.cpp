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

#include "cppcheck.h"
#include "settings.h"
#include "testsuite.h"
#include "cppcheckexecutor.h"
#include "threadexecutor.h"

#include <sstream>

extern std::ostringstream errout;

class TestSuppressions : public TestFixture
{
public:
    TestSuppressions() : TestFixture("TestSuppressions")
    { }

private:

    void run()
    {
        TEST_CASE(suppressionsBadId1);
        TEST_CASE(suppressionsDosFormat);     // Ticket #1836
        TEST_CASE(suppressionsFileNameWithColon);    // Ticket #1919 - filename includes colon
        TEST_CASE(suppressionsGlob);
        TEST_CASE(suppressionsFileNameWithExtraPath);
        TEST_CASE(suppressionsSettings);
        TEST_CASE(suppressionsMultiFile);
    }

    void suppressionsBadId1()
    {
        Suppressions suppressions;
        std::istringstream s("123");
        ASSERT_EQUALS("Failed to add suppression. Invalid id \"123\"", suppressions.parseFile(s));
    }

    void suppressionsDosFormat()
    {
        Suppressions suppressions;
        std::istringstream s("abc\r\ndef\r\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed("abc", "test.cpp", 1));
        ASSERT_EQUALS(true, suppressions.isSuppressed("def", "test.cpp", 1));
    }

    void suppressionsFileNameWithColon()
    {
        Suppressions suppressions;
        std::istringstream s("errorid:c:\\foo.cpp\nerrorid:c:\\bar.cpp:12");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "c:\\foo.cpp", 1111));
        ASSERT_EQUALS(false, suppressions.isSuppressed("errorid", "c:\\bar.cpp", 10));
        ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "c:\\bar.cpp", 12));
    }

    void suppressionsGlob()
    {
        // Check for syntax errors in glob
        {
            Suppressions suppressions;
            std::istringstream s("errorid:**.cpp\n");
            ASSERT_EQUALS("Failed to add suppression. Syntax error in glob.", suppressions.parseFile(s));
        }

        // Check that globbing works
        {
            Suppressions suppressions;
            std::istringstream s("errorid:x*.cpp\nerrorid:y?.cpp\nerrorid:test.c*");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp.cpp", 1));
            ASSERT_EQUALS(false, suppressions.isSuppressed("errorid", "abc.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "ya.cpp", 1));
            ASSERT_EQUALS(false, suppressions.isSuppressed("errorid", "y.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "test.c", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "test.cpp", 1));
        }

        // Check that both a filename match and a glob match apply
        {
            Suppressions suppressions;
            std::istringstream s("errorid:x*.cpp\nerrorid:xyz.cpp:1\nerrorid:a*.cpp:1\nerrorid:abc.cpp:2");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp", 2));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "abc.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "abc.cpp", 2));
        }
    }

    void suppressionsFileNameWithExtraPath()
    {
        // Ticket #2797
        Suppressions suppressions;
        suppressions.addSuppression("errorid", "./a.c", 123);
        ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "a.c", 123));
    }

    // Check the suppression
    void checkSuppression(const char code[], const std::string &suppression = "")
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings._inlineSuppressions = true;
        if (!suppression.empty())
        {
            std::string r = settings.nomsg.addSuppressionLine(suppression);
            ASSERT_EQUALS("", r);
        }

        CppCheck cppCheck(*this, true);
        cppCheck.settings(settings);
        cppCheck.check("test.cpp", code);

        reportUnmatchedSuppressions(cppCheck.settings().nomsg.getUnmatchedGlobalSuppressions());
    }

    void checkSuppressionThreads(const char code[], const std::string &suppression = "")
    {
        errout.str("");
        output.str("");

        std::vector<std::string> filenames;
        std::map<std::string, long> filesizes;
        filenames.push_back("test.cpp");

        Settings settings;
        settings._jobs = 1;
        settings._inlineSuppressions = true;
        if (!suppression.empty())
        {
            std::string r = settings.nomsg.addSuppressionLine(suppression);
            ASSERT_EQUALS("", r);
        }
        ThreadExecutor executor(filenames, filesizes, settings, *this);
        for (unsigned int i = 0; i < filenames.size(); ++i)
            executor.addFileContent(filenames[i], code);

        executor.check();

        reportUnmatchedSuppressions(settings.nomsg.getUnmatchedGlobalSuppressions());
    }

    // Check the suppression for multiple files
    void checkSuppression(const char *names[], const char *codes[], const std::string &suppression = "")
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings._inlineSuppressions = true;
        if (!suppression.empty())
            settings.nomsg.addSuppressionLine(suppression);

        CppCheck cppCheck(*this, true);
        cppCheck.settings(settings);
        for (int i = 0; names[i] != NULL; ++i)
            cppCheck.check(names[i], codes[i]);

        reportUnmatchedSuppressions(cppCheck.settings().nomsg.getUnmatchedGlobalSuppressions());
    }

    void runChecks(void (TestSuppressions::*check)(const char[], const std::string &))
    {
        // check to make sure the appropriate error is present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        // suppress uninitvar globally
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "uninitvar");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar globally, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar");
        ASSERT_EQUALS("[*]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress uninitvar for this file only
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "uninitvar:test.cpp");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress all for this file only
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "*:test.cpp");
        ASSERT_EQUALS("", errout.str());

        // suppress all for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "*:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: *\n", errout.str());

        // suppress uninitvar for this file and line
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "uninitvar:test.cpp:3");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar for this file and line, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp:3");
        ASSERT_EQUALS("[test.cpp:3]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    /* cppcheck-suppress uninitvar */\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    /* cppcheck-suppress uninitvar */\n"
                       "\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    b++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:4]: (information) Unmatched suppression: uninitvar\n", errout.str());
    }

    void suppressionsSettings()
    {
        runChecks(&TestSuppressions::checkSuppression);
        if (ThreadExecutor::isEnabled())
            runChecks(&TestSuppressions::checkSuppressionThreads);
    }

    void suppressionsMultiFile()
    {
        const char *names[] = {"abc.cpp", "xyz.cpp", NULL};
        const char *codes[] =
        {
            "void f() {\n"
            "}\n",
            "void f() {\n"
            "    int a;\n"
            "    a++;\n"
            "}\n",
        };

        // suppress uninitvar for this file and line
        checkSuppression(names, codes, "uninitvar:xyz.cpp:3");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestSuppressions)
