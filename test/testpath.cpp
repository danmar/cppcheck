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

#include <string>
#include "testsuite.h"
#include "path.h"

class TestPath : public TestFixture {
public:
    TestPath() : TestFixture("TestPath")
    { }

private:

    void run() {
        TEST_CASE(simplify_path);
    }

    void simplify_path() {
        // Path::simplifyPath()
        ASSERT_EQUALS("index.h", Path::simplifyPath("index.h"));
        ASSERT_EQUALS("index.h", Path::simplifyPath("./index.h"));
        ASSERT_EQUALS("/index.h", Path::simplifyPath("/index.h"));
        ASSERT_EQUALS("/path/", Path::simplifyPath("/path/"));
        ASSERT_EQUALS("/", Path::simplifyPath("/"));
        ASSERT_EQUALS("../index.h", Path::simplifyPath("../index.h"));
        ASSERT_EQUALS("/index.h", Path::simplifyPath("/path/../index.h"));
        ASSERT_EQUALS("/index.h", Path::simplifyPath("/path/../other/../index.h"));
        ASSERT_EQUALS("/index.h", Path::simplifyPath("/path/../other///././../index.h"));
        ASSERT_EQUALS("../path/index.h", Path::simplifyPath("../path/other/../index.h"));
        ASSERT_EQUALS("a/index.h", Path::simplifyPath("a/../a/index.h"));
        ASSERT_EQUALS("a/..", Path::simplifyPath("a/.."));
        ASSERT_EQUALS("../../src/test.cpp", Path::simplifyPath("../../src/test.cpp"));
        ASSERT_EQUALS("../../../src/test.cpp", Path::simplifyPath("../../../src/test.cpp"));

        // Path::removeQuotationMarks()
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("index.cpp"));
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("\"index.cpp"));
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("index.cpp\""));
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("\"index.cpp\""));
        ASSERT_EQUALS("path to/index.cpp", Path::removeQuotationMarks("\"path to\"/index.cpp"));
        ASSERT_EQUALS("path to/index.cpp", Path::removeQuotationMarks("\"path to/index.cpp\""));
        ASSERT_EQUALS("the/path to/index.cpp", Path::removeQuotationMarks("the/\"path to\"/index.cpp"));
        ASSERT_EQUALS("the/path to/index.cpp", Path::removeQuotationMarks("\"the/path to/index.cpp\""));
    }
};

REGISTER_TEST(TestPath)
