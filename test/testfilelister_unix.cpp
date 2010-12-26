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

#define private public

#include "filelister_unix.h"

class TestFileLister: public TestFixture
{
public:
    TestFileLister()
        :TestFixture("TestFileLister")
    {}

private:
    void run()
    {
        TEST_CASE(test_recursiveAddFiles2);
    }

    void test_recursiveAddFiles2()
    {
        std::vector<std::string> relative, absolute;
        FileListerUnix ful;
        ful.recursiveAddFiles2(relative, absolute, ".");

        ASSERT(relative.size() != 0);
        ASSERT_EQUALS(relative.size(), absolute.size());

        for (std::vector<std::string>::const_iterator r = relative.begin(), r_end = relative.end(),
             a = absolute.begin(), a_end = absolute.end();
             r != r_end && a != a_end;
             ++r, ++a
             )
        {
            static const size_t start_at_relative = std::string("./").size();
            static const size_t start_at_absolute = std::string("./").size() + a->size() - r->size();

            ASSERT_EQUALS(r->substr(start_at_relative), a->substr(start_at_absolute));
        }
    }
};

REGISTER_TEST(TestFileLister)

