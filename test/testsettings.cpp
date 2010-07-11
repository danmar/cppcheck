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
    }

    void suppressionsBadId1()
    {
        Settings::Suppressions suppressions;
        std::istringstream s("123");
        ASSERT_EQUALS(false, suppressions.parseFile(s));
    }

    void suppressionsDosFormat()
    {
        Settings::Suppressions suppressions;
        std::istringstream s("abc\r\ndef\r\n");
        ASSERT_EQUALS(true, suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed("abc", "test.cpp", 1));
        ASSERT_EQUALS(true, suppressions.isSuppressed("def", "test.cpp", 1));
    }
};

REGISTER_TEST(TestSettings)
