/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "pathmatch.h"
#include "fixture.h"

#include <string>
#include <vector>


class TestPathMatch : public TestFixture {
public:
    TestPathMatch() : TestFixture("TestPathMatch") {}

private:
    const PathMatch emptyMatcher{std::vector<std::string>()};
    const PathMatch srcMatcher{std::vector<std::string>(1, "src/")};
    const PathMatch fooCppMatcher{std::vector<std::string>(1, "foo.cpp")};
    const PathMatch srcFooCppMatcher{std::vector<std::string>(1, "src/foo.cpp")};

    void run() override {
        TEST_CASE(emptymaskemptyfile);
        TEST_CASE(emptymaskpath1);
        TEST_CASE(emptymaskpath2);
        TEST_CASE(emptymaskpath3);
        TEST_CASE(onemaskemptypath);
        TEST_CASE(onemasksamepath);
        TEST_CASE(onemasksamepathdifferentcase);
        TEST_CASE(onemasksamepathwithfile);
        TEST_CASE(onemaskshorterpath);
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
        ASSERT(!emptyMatcher.match(""));
    }

    void emptymaskpath1() const {
        ASSERT(!emptyMatcher.match("src/"));
    }

    void emptymaskpath2() const {
        ASSERT(!emptyMatcher.match("../src/"));
    }

    void emptymaskpath3() const {
        ASSERT(!emptyMatcher.match("/home/user/code/src/"));
    }

    // Test PathMatch containing "src/"
    void onemaskemptypath() const {
        ASSERT(!srcMatcher.match(""));
    }

    void onemasksamepath() const {
        ASSERT(srcMatcher.match("src/"));
    }

    void onemasksamepathdifferentcase() const {
        std::vector<std::string> masks(1, "sRc/");
        PathMatch match(masks, false);
        ASSERT(match.match("srC/"));
    }

    void onemasksamepathwithfile() const {
        ASSERT(srcMatcher.match("src/file.txt"));
    }

    void onemaskshorterpath() const {
        const std::string longerExclude("longersrc/");
        const std::string shorterToMatch("src/");
        ASSERT(shorterToMatch.length() < longerExclude.length());
        PathMatch match(std::vector<std::string>(1, longerExclude));
        ASSERT(match.match(longerExclude));
        ASSERT(!match.match(shorterToMatch));
    }

    void onemaskdifferentdir1() const {
        ASSERT(!srcMatcher.match("srcfiles/file.txt"));
    }

    void onemaskdifferentdir2() const {
        ASSERT(!srcMatcher.match("proj/srcfiles/file.txt"));
    }

    void onemaskdifferentdir3() const {
        ASSERT(!srcMatcher.match("proj/mysrc/file.txt"));
    }

    void onemaskdifferentdir4() const {
        ASSERT(!srcMatcher.match("proj/mysrcfiles/file.txt"));
    }

    void onemasklongerpath1() const {
        ASSERT(srcMatcher.match("/tmp/src/"));
    }

    void onemasklongerpath2() const {
        ASSERT(srcMatcher.match("src/module/"));
    }

    void onemasklongerpath3() const {
        ASSERT(srcMatcher.match("project/src/module/"));
    }

    void twomasklongerpath1() const {
        std::vector<std::string> masks = { "src/", "module/" };
        PathMatch match(masks);
        ASSERT(!match.match("project/"));
    }

    void twomasklongerpath2() const {
        std::vector<std::string> masks = { "src/", "module/" };
        PathMatch match(masks);
        ASSERT(match.match("project/src/"));
    }

    void twomasklongerpath3() const {
        std::vector<std::string> masks = { "src/", "module/" };
        PathMatch match(masks);
        ASSERT(match.match("project/module/"));
    }

    void twomasklongerpath4() const {
        std::vector<std::string> masks = { "src/", "module/" };
        PathMatch match(masks);
        ASSERT(match.match("project/src/module/"));
    }

    // Test PathMatch containing "foo.cpp"
    void filemask1() const {
        ASSERT(fooCppMatcher.match("foo.cpp"));
    }

    void filemaskdifferentcase() const {
        std::vector<std::string> masks(1, "foo.cPp");
        PathMatch match(masks, false);
        ASSERT(match.match("fOo.cpp"));
    }

    void filemask2() const {
        ASSERT(fooCppMatcher.match("../foo.cpp"));
    }

    void filemask3() const {
        ASSERT(fooCppMatcher.match("src/foo.cpp"));
    }

    // Test PathMatch containing "src/foo.cpp"
    void filemaskpath1() const {
        ASSERT(srcFooCppMatcher.match("src/foo.cpp"));
    }

    void filemaskpath2() const {
        ASSERT(srcFooCppMatcher.match("proj/src/foo.cpp"));
    }

    void filemaskpath3() const {
        ASSERT(!srcFooCppMatcher.match("foo.cpp"));
    }

    void filemaskpath4() const {
        ASSERT(!srcFooCppMatcher.match("bar/foo.cpp"));
    }
};

REGISTER_TEST(TestPathMatch)
