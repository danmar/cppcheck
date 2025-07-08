/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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
#ifdef _WIN32
    const std::string basepath{"C:\\test"};
#else
    const std::string basepath{"/test"};
#endif
    const PathMatch emptyMatcher{{}, basepath};
    const PathMatch srcMatcher{{"src/"}, basepath};
    const PathMatch fooCppMatcher{{"foo.cpp"}, basepath};
    const PathMatch srcFooCppMatcher{{"src/foo.cpp"}, basepath};

    void run() override {
        TEST_CASE(emptymaskemptyfile);
        TEST_CASE(emptymaskpath1);
        TEST_CASE(emptymaskpath2);
        TEST_CASE(emptymaskpath3);
        TEST_CASE(onemaskemptypath);
        TEST_CASE(onemasksamepath);
        TEST_CASE(onemasksamepathdifferentslash);
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
        TEST_CASE(onemaskcwd);
        TEST_CASE(twomasklongerpath1);
        TEST_CASE(twomasklongerpath2);
        TEST_CASE(twomasklongerpath3);
        TEST_CASE(twomasklongerpath4);
        TEST_CASE(filemask1);
        TEST_CASE(filemaskdifferentcase);
        TEST_CASE(filemask2);
        TEST_CASE(filemask3);
        TEST_CASE(filemaskcwd);
        TEST_CASE(filemaskpath1);
        TEST_CASE(filemaskpath2);
        TEST_CASE(filemaskpath3);
        TEST_CASE(filemaskpath4);
        TEST_CASE(mixedallmatch);
        TEST_CASE(glob);
        TEST_CASE(globstar1);
        TEST_CASE(globstar2);
    }

    // Test empty PathMatch
    void emptymaskemptyfile() const {
        ASSERT(!emptyMatcher.match("", basepath));
    }

    void emptymaskpath1() const {
        ASSERT(!emptyMatcher.match("src/", basepath));
    }

    void emptymaskpath2() const {
        ASSERT(!emptyMatcher.match("../src/", basepath));
    }

    void emptymaskpath3() const {
        ASSERT(!emptyMatcher.match("/home/user/code/src/", basepath));
        ASSERT(!emptyMatcher.match("d:/home/user/code/src/", basepath));
    }

    // Test PathMatch containing "src/"
    void onemaskemptypath() const {
        ASSERT(!srcMatcher.match("", basepath));
    }

    void onemasksamepath() const {
        ASSERT(srcMatcher.match("src/", basepath));
    }

    void onemasksamepathdifferentslash() const {
        PathMatch srcMatcher2({"src\\"}, basepath);
        ASSERT(srcMatcher2.match("src/", basepath));
    }

    void onemasksamepathdifferentcase() const {
        PathMatch match({"sRc/"}, basepath, PathMatch::Mode::icase);
        ASSERT(match.match("srC/", basepath));
    }

    void onemasksamepathwithfile() const {
        ASSERT(srcMatcher.match("src/file.txt", basepath));
    }

    void onemaskshorterpath() const {
        const std::string longerExclude("longersrc/");
        const std::string shorterToMatch("src/");
        ASSERT(shorterToMatch.length() < longerExclude.length());
        PathMatch match({longerExclude}, basepath);
        ASSERT(match.match(longerExclude, basepath));
        ASSERT(!match.match(shorterToMatch, basepath));
    }

    void onemaskdifferentdir1() const {
        ASSERT(!srcMatcher.match("srcfiles/file.txt", basepath));
    }

    void onemaskdifferentdir2() const {
        ASSERT(!srcMatcher.match("proj/srcfiles/file.txt", basepath));
    }

    void onemaskdifferentdir3() const {
        ASSERT(!srcMatcher.match("proj/mysrc/file.txt", basepath));
    }

    void onemaskdifferentdir4() const {
        ASSERT(!srcMatcher.match("proj/mysrcfiles/file.txt", basepath));
    }

    void onemasklongerpath1() const {
        ASSERT(srcMatcher.match("/tmp/src/", basepath));
        ASSERT(srcMatcher.match("d:/tmp/src/", basepath));
    }

    void onemasklongerpath2() const {
        ASSERT(srcMatcher.match("src/module/", basepath));
    }

    void onemasklongerpath3() const {
        ASSERT(srcMatcher.match("project/src/module/", basepath));
    }

    void onemaskcwd() const {
        ASSERT(!srcMatcher.match("./src", basepath));
    }

    void twomasklongerpath1() const {
        PathMatch match({ "src/", "module/" }, basepath);
        ASSERT(!match.match("project/", basepath));
    }

    void twomasklongerpath2() const {
        PathMatch match({ "src/", "module/" }, basepath);
        ASSERT(match.match("project/src/", basepath));
    }

    void twomasklongerpath3() const {
        PathMatch match({ "src/", "module/" }, basepath);
        ASSERT(match.match("project/module/", basepath));
    }

    void twomasklongerpath4() const {
        PathMatch match({ "src/", "module/" }, basepath);
        ASSERT(match.match("project/src/module/", basepath));
    }

    // Test PathMatch containing "foo.cpp"
    void filemask1() const {
        ASSERT(fooCppMatcher.match("foo.cpp", basepath));
    }

    void filemaskdifferentcase() const {
        PathMatch match({"foo.cPp"}, basepath, PathMatch::Mode::icase);
        ASSERT(match.match("fOo.cpp", basepath));
    }

    void filemask2() const {
        ASSERT(fooCppMatcher.match("../foo.cpp", basepath));
    }

    void filemask3() const {
        ASSERT(fooCppMatcher.match("src/foo.cpp", basepath));
    }

    void filemaskcwd() const {
        ASSERT(fooCppMatcher.match("./lib/foo.cpp", basepath));
    }

    // Test PathMatch containing "src/foo.cpp"
    void filemaskpath1() const {
        ASSERT(srcFooCppMatcher.match("src/foo.cpp", basepath));
    }

    void filemaskpath2() const {
        ASSERT(srcFooCppMatcher.match("proj/src/foo.cpp", basepath));
    }

    void filemaskpath3() const {
        ASSERT(!srcFooCppMatcher.match("foo.cpp", basepath));
    }

    void filemaskpath4() const {
        ASSERT(!srcFooCppMatcher.match("bar/foo.cpp", basepath));
    }

    void mixedallmatch() const { // #13570
        // when trying to match a directory against a directory entry it erroneously modified a local variable also used for file matching
        PathMatch match({ "tests/", "file.c" }, basepath);
        ASSERT(match.match("tests/", basepath));
        ASSERT(match.match("lib/file.c", basepath));
    }

    void glob() const {
        PathMatch match({"test?.cpp"}, basepath);
        ASSERT(match.match("test1.cpp", basepath));
        ASSERT(match.match("src/test1.cpp", basepath));
        ASSERT(!match.match("test1.c", basepath));
        ASSERT(!match.match("test.cpp", basepath));
        ASSERT(!match.match("test1.cpp/src", basepath));
    }

    void globstar1() const {
        PathMatch match({"src/**/foo.c"}, basepath);
        ASSERT(match.match("src/lib/foo/foo.c", basepath));
        ASSERT(match.match("src/lib/foo/bar/foo.c", basepath));
        ASSERT(!match.match("src/lib/foo/foo.cpp", basepath));
        ASSERT(!match.match("src/lib/foo/bar/foo.cpp", basepath));
    }

    void globstar2() const {
        PathMatch match({"./src/**/foo.c"}, basepath);
        ASSERT(match.match("src/lib/foo/foo.c", basepath));
        ASSERT(match.match("src/lib/foo/bar/foo.c", basepath));
        ASSERT(!match.match("src/lib/foo/foo.cpp", basepath));
        ASSERT(!match.match("src/lib/foo/bar/foo.cpp", basepath));
    }
};

REGISTER_TEST(TestPathMatch)
