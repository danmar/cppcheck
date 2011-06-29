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

#include "settings.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestSettings : public TestFixture
{
public:
    TestSettings() : TestFixture("TestSettings")
    { }

private:

    void run()
    {
        TEST_CASE(suppressionsBadId1);
        TEST_CASE(suppressionsDosFormat);     // Ticket #1836
        TEST_CASE(suppressionsFileNameWithColon);    // Ticket #1919 - filename includes colon
        TEST_CASE(suppressionsGlob);
        TEST_CASE(suppressionsFileNameWithExtraPath);
    }

    void suppressionsBadId1()
    {
        Settings::Suppressions suppressions;
        std::istringstream s("123");
        ASSERT_EQUALS("Failed to add suppression. Invalid id \"123\"", suppressions.parseFile(s));
    }

    void suppressionsDosFormat()
    {
        Settings::Suppressions suppressions;
        std::istringstream s("abc\r\ndef\r\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed("abc", "test.cpp", 1));
        ASSERT_EQUALS(true, suppressions.isSuppressed("def", "test.cpp", 1));
    }

    void suppressionsFileNameWithColon()
    {
        Settings::Suppressions suppressions;
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
            Settings::Suppressions suppressions;
            std::istringstream s("errorid:**.cpp\n");
            ASSERT_EQUALS("Failed to add suppression. Syntax error in glob.", suppressions.parseFile(s));
        }

        // Check that globbing works
        {
            Settings::Suppressions suppressions;
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
            Settings::Suppressions suppressions;
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
        Settings::Suppressions suppressions;
        suppressions.addSuppression("errorid", "./a.c", 123);
        ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "a.c", 123));
    }
};

REGISTER_TEST(TestSettings)
