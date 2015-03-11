/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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


class TestPathMatch : public TestFixture {
public:
    TestPathMatch()
        : TestFixture("TestPathMatch")
        , emptyMatcher(std::vector<std::string>())
        , srcMatcher(std::vector<std::string>(1, "src/"))
        , fooCppMatcher(std::vector<std::string>(1, "foo.cpp"))
        , srcFooCppMatcher(std::vector<std::string>(1, "src/foo.cpp")) {
    }

private:
    const PathMatch emptyMatcher;
    const PathMatch srcMatcher;
    const PathMatch fooCppMatcher;
    const PathMatch srcFooCppMatcher;

    void run() {
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
        TEST_CASE(twomasklongerpath1);
        TEST_CASE(twomasklongerpath2);
        TEST_CASE(twomasklongerpath3);
        TEST_CASE(twomasklongerpath4);
        TEST_CASE(filemask1);
        TEST_CASE(filemaskdifferentcase);
        TEST_CASE(filemask2);
        TEST_CASE(filemask3);
        TEST_CASE(filemaskpath1);
        TEST_CASE(filemaskpath2);
        TEST_CASE(filemaskpath3);
        TEST_CASE(filemaskpath4);
    }

    // Test empty PathMatch
    void emptymaskemptyfile() const {
        ASSERT(!emptyMatcher.Match(""));
    }

    void emptymaskpath1() const {
        ASSERT(!emptyMatcher.Match("src/"));
    }

    void emptymaskpath2() const {
        ASSERT(!emptyMatcher.Match("../src/"));
    }

    void emptymaskpath3() const {
        ASSERT(!emptyMatcher.Match("/home/user/code/src/"));
    }

    // Test PathMatch containing "src/"
    void onemaskemptypath() const {
        ASSERT(!srcMatcher.Match(""));
    }

    void onemasksamepath() const {
        ASSERT(srcMatcher.Match("src/"));
    }

    void onemasksamepathdifferentcase() const {
        std::vector<std::string> masks(1, "sRc/");
        PathMatch match(masks, false);
        ASSERT(match.Match("srC/"));
    }

    void onemasksamepathwithfile() const {
        ASSERT(srcMatcher.Match("src/file.txt"));
    }

    void onemaskdifferentdir1() const {
        ASSERT(!srcMatcher.Match("srcfiles/file.txt"));
    }

    void onemaskdifferentdir2() const {
        ASSERT(!srcMatcher.Match("proj/srcfiles/file.txt"));
    }

    void onemaskdifferentdir3() const {
        ASSERT(!srcMatcher.Match("proj/mysrc/file.txt"));
    }

    void onemaskdifferentdir4() const {
        ASSERT(!srcMatcher.Match("proj/mysrcfiles/file.txt"));
    }

    void onemasklongerpath1() const {
        ASSERT(srcMatcher.Match("/tmp/src/"));
    }

    void onemasklongerpath2() const {
        ASSERT(srcMatcher.Match("src/module/"));
    }

    void onemasklongerpath3() const {
        ASSERT(srcMatcher.Match("project/src/module/"));
    }

    void twomasklongerpath1() const {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(!match.Match("project/"));
    }

    void twomasklongerpath2() const {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(match.Match("project/src/"));
    }

    void twomasklongerpath3() const {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(match.Match("project/module/"));
    }

    void twomasklongerpath4() const {
        std::vector<std::string> masks;
        masks.push_back("src/");
        masks.push_back("module/");
        PathMatch match(masks);
        ASSERT(match.Match("project/src/module/"));
    }

    // Test PathMatch containing "foo.cpp"
    void filemask1() const {
        ASSERT(fooCppMatcher.Match("foo.cpp"));
    }

    void filemaskdifferentcase() const {
        std::vector<std::string> masks(1, "foo.cPp");
        PathMatch match(masks, false);
        ASSERT(match.Match("fOo.cpp"));
    }

    void filemask2() const {
        ASSERT(fooCppMatcher.Match("../foo.cpp"));
    }

    void filemask3() const {
        ASSERT(fooCppMatcher.Match("src/foo.cpp"));
    }

    // Test PathMatch containing "src/foo.cpp"
    void filemaskpath1() const {
        ASSERT(srcFooCppMatcher.Match("src/foo.cpp"));
    }

    void filemaskpath2() const {
        ASSERT(srcFooCppMatcher.Match("proj/src/foo.cpp"));
    }

    void filemaskpath3() const {
        ASSERT(!srcFooCppMatcher.Match("foo.cpp"));
    }

    void filemaskpath4() const {
        ASSERT(!srcFooCppMatcher.Match("bar/foo.cpp"));
    }
};

REGISTER_TEST(TestPathMatch)
