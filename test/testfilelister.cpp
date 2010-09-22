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

#include <string>
#include "testsuite.h"

#if defined(_WIN32)
#include "../lib/filelister_win32.h"
#else // POSIX-style system
#include "../lib/filelister_unix.h"
#endif

class TestFileLister : public TestFixture
{
public:
    TestFileLister() : TestFixture("TestFileLister")
    { }

private:

    void run()
    {
        TEST_CASE(simplify_path);
    }

    void simplify_path()
    {
        ASSERT_EQUALS("index.h", getFileLister()->simplifyPath("index.h"));
        ASSERT_EQUALS("/index.h", getFileLister()->simplifyPath("/index.h"));
        ASSERT_EQUALS("/path/", getFileLister()->simplifyPath("/path/"));
        ASSERT_EQUALS("/", getFileLister()->simplifyPath("/"));
        ASSERT_EQUALS("./index.h", getFileLister()->simplifyPath("./index.h"));
        ASSERT_EQUALS("../index.h", getFileLister()->simplifyPath("../index.h"));
        ASSERT_EQUALS("/index.h", getFileLister()->simplifyPath("/path/../index.h"));
        ASSERT_EQUALS("/index.h", getFileLister()->simplifyPath("/path/../other/../index.h"));
        ASSERT_EQUALS("/index.h", getFileLister()->simplifyPath("/path/../other///././../index.h"));
        ASSERT_EQUALS("../path/index.h", getFileLister()->simplifyPath("../path/other/../index.h"));
        ASSERT_EQUALS("a/index.h", getFileLister()->simplifyPath("a/../a/index.h"));
        ASSERT_EQUALS("a/..", getFileLister()->simplifyPath("a/.."));
        ASSERT_EQUALS("../../src/test.cpp", getFileLister()->simplifyPath("../../src/test.cpp"));
        ASSERT_EQUALS("../../../src/test.cpp", getFileLister()->simplifyPath("../../../src/test.cpp"));
    }
};

REGISTER_TEST(TestFileLister)
