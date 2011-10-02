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
#include <vector>
#include "testsuite.h"
#include "pathmatch.h"

class TestPathMatch : public TestFixture
{
public:
    TestPathMatch() : TestFixture("TestPathMatch")
    { }

private:

    void run()
    {
        TEST_CASE(emptymaskemptyfile);
        TEST_CASE(emptymaskpath1);
        TEST_CASE(emptymaskpath2);
        TEST_CASE(emptymaskpath3);
        TEST_CASE(onemaskemptypath);
        TEST_CASE(onemasksamepath);
        TEST_CASE(onemasksamepathdifferentcase);
        TEST_CASE(onemasksamepathwithfile);
        TEST_CASE(onemaskdifferentdir1);
        TEST_CASE(onemaskdifferentdir2);
        TEST_CASE(onemaskdifferentdir3);
        TEST_CASE(onemaskdifferentdir4);
        TEST_CASE(onemasklongerpath1);
        TEST_CASE(onemasklongerpath2);
        TEST_CASE(onemasklongerpath3);
        TEST_CASE(filemask1);
        TEST_CASE(filemaskdifferentcase);
        TEST_CASE(filemask2);
        TEST_CASE(filemask3);
        TEST_CASE(filemaskpath1);
        TEST_CASE(filemaskpath2);
        TEST_CASE(filemaskpath3);
        TEST_CASE(filemaskpath4);
    }

    void emptymaskemptyfile()
    {
        std::vector<std::string> masks;
        PathMatch match(masks);
        ASSERT(!match.Match(""));
    }

    void emptymaskpath1()
    {
        std::vector<std::string> masks;
        PathMatch match(masks);
        ASSERT(!match.Match("src/"));
    }

    void emptymaskpath2()
    {
        std::vector<std::string> masks;
        PathMatch match(masks);
        ASSERT(!match.Match("../src/"));
    }

    void emptymaskpath3()
    {
        std::vector<std::string> masks;
        PathMatch match(masks);
        ASSERT(!match.Match("/home/user/code/src/"));
    }

    void onemaskemptypath()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(!match.Match(""));
    }

    void onemasksamepath()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(match.Match("src/"));
    }

    void onemasksamepathdifferentcase()
    {
        std::vector<std::string> masks;
        masks.push_back("sRc/");
        PathMatch match(masks);
        ASSERT(match.Match("srC/", false));
    }

    void onemasksamepathwithfile()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(match.Match("src/file.txt"));
    }

    void onemaskdifferentdir1()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(!match.Match("srcfiles/file.txt"));
    }

    void onemaskdifferentdir2()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(!match.Match("proj/srcfiles/file.txt"));
    }

    void onemaskdifferentdir3()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(!match.Match("proj/mysrc/file.txt"));
    }

    void onemaskdifferentdir4()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(!match.Match("proj/mysrcfiles/file.txt"));
    }

    void onemasklongerpath1()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(match.Match("/tmp/src/"));
    }

    void onemasklongerpath2()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(match.Match("src/module/"));
    }

    void onemasklongerpath3()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        PathMatch match(masks);
        ASSERT(match.Match("project/src/module/"));
    }

    void twomasklongerpath1()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(!match.Match("project/"));
    }

    void twomasklongerpath2()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(match.Match("project/src/"));
    }

    void twomasklongerpath3()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(match.Match("project/module/"));
    }

    void twomasklongerpath4()
    {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(match.Match("project/src/module/"));
    }

    void filemask1()
    {
        std::vector<std::string> masks;
        masks.push_back("foo.cpp");
        PathMatch match(masks);
        ASSERT(match.Match("foo.cpp"));
    }

    void filemaskdifferentcase()
    {
        std::vector<std::string> masks;
        masks.push_back("foo.cPp");
        PathMatch match(masks);
        ASSERT(match.Match("fOo.cpp", false));
    }

    void filemask2()
    {
        std::vector<std::string> masks;
        masks.push_back("foo.cpp");
        PathMatch match(masks);
        ASSERT(match.Match("../foo.cpp"));
    }

    void filemask3()
    {
        std::vector<std::string> masks;
        masks.push_back("foo.cpp");
        PathMatch match(masks);
        ASSERT(match.Match("src/foo.cpp"));
    }

    void filemaskpath1()
    {
        std::vector<std::string> masks;
        masks.push_back("src/foo.cpp");
        PathMatch match(masks);
        ASSERT(match.Match("src/foo.cpp"));
    }

    void filemaskpath2()
    {
        std::vector<std::string> masks;
        masks.push_back("src/foo.cpp");
        PathMatch match(masks);
        ASSERT(match.Match("proj/src/foo.cpp"));
    }

    void filemaskpath3()
    {
        std::vector<std::string> masks;
        masks.push_back("src/foo.cpp");
        PathMatch match(masks);
        ASSERT(!match.Match("foo.cpp"));
    }

    void filemaskpath4()
    {
        std::vector<std::string> masks;
        masks.push_back("src/foo.cpp");
        PathMatch match(masks);
        ASSERT(!match.Match("bar/foo.cpp"));
    }
};

REGISTER_TEST(TestPathMatch)
