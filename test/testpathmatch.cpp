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
        TEST_CASE(pathiterator);
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
        ASSERT(!emptyMatcher.match("d:/home/user/code/src/"));
    }

    // Test PathMatch containing "src/"
    void onemaskemptypath() const {
        ASSERT(!srcMatcher.match(""));
    }

    void onemasksamepath() const {
        ASSERT(srcMatcher.match("src/"));
    }

    void onemasksamepathdifferentslash() const {
        PathMatch srcMatcher2({"src\\"}, basepath);
        ASSERT(srcMatcher2.match("src/"));
    }

    void onemasksamepathdifferentcase() const {
        PathMatch match({"sRc/"}, basepath, PathMatch::Mode::icase);
        ASSERT(match.match("srC/"));
    }

    void onemasksamepathwithfile() const {
        ASSERT(srcMatcher.match("src/file.txt"));
    }

    void onemaskshorterpath() const {
        const std::string longerExclude("longersrc/");
        const std::string shorterToMatch("src/");
        ASSERT(shorterToMatch.length() < longerExclude.length());
        PathMatch match({longerExclude});
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
        ASSERT(srcMatcher.match("d:/tmp/src/"));
    }

    void onemasklongerpath2() const {
        ASSERT(srcMatcher.match("src/module/"));
    }

    void onemasklongerpath3() const {
        ASSERT(srcMatcher.match("project/src/module/"));
    }

    void onemaskcwd() const {
        ASSERT(srcMatcher.match("./src"));
    }

    void twomasklongerpath1() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(!match.match("project/"));
    }

    void twomasklongerpath2() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(match.match("project/src/"));
    }

    void twomasklongerpath3() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(match.match("project/module/"));
    }

    void twomasklongerpath4() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(match.match("project/src/module/"));
    }

    // Test PathMatch containing "foo.cpp"
    void filemask1() const {
        ASSERT(fooCppMatcher.match("foo.cpp"));
    }

    void filemaskdifferentcase() const {
        PathMatch match({"foo.cPp"}, basepath, PathMatch::Mode::icase);
        ASSERT(match.match("fOo.cpp"));
    }

    void filemask2() const {
        ASSERT(fooCppMatcher.match("../foo.cpp"));
    }

    void filemask3() const {
        ASSERT(fooCppMatcher.match("src/foo.cpp"));
    }

    void filemaskcwd() const {
        ASSERT(fooCppMatcher.match("./lib/foo.cpp"));
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

    void mixedallmatch() const { // #13570
        // when trying to match a directory against a directory entry it erroneously modified a local variable also used for file matching
        PathMatch match({ "tests/", "file.c" });
        ASSERT(match.match("tests/"));
        ASSERT(match.match("lib/file.c"));
    }

    void glob() const {
        PathMatch match({"test?.cpp"});
        ASSERT(match.match("test1.cpp"));
        ASSERT(match.match("src/test1.cpp"));
        ASSERT(match.match("test1.cpp/src"));
        ASSERT(!match.match("test1.c"));
        ASSERT(!match.match("test.cpp"));
    }

    void globstar1() const {
        PathMatch match({"src/**/foo.c"});
        ASSERT(match.match("src/lib/foo/foo.c"));
        ASSERT(match.match("src/lib/foo/bar/foo.c"));
        ASSERT(!match.match("src/lib/foo/foo.cpp"));
        ASSERT(!match.match("src/lib/foo/bar/foo.cpp"));
    }

    void globstar2() const {
        PathMatch match({"./src/**/foo.c"});
        ASSERT(match.match("src/lib/foo/foo.c"));
        ASSERT(match.match("src/lib/foo/bar/foo.c"));
        ASSERT(!match.match("src/lib/foo/foo.cpp"));
        ASSERT(!match.match("src/lib/foo/bar/foo.cpp"));
    }

    void pathiterator() const {
        using PathIterator = PathMatch::PathIterator;
        ASSERT_EQUALS("/hello/world", PathIterator("/hello/universe/.", "../world//").read());
        ASSERT_EQUALS("/", PathIterator("//./..//.///.", "../../..///").read());
        ASSERT_EQUALS("/foo/bar", PathIterator(nullptr, "/foo/bar/.").read());
        ASSERT_EQUALS("/foo/bar", PathIterator("/foo/bar/.", nullptr).read());
        ASSERT_EQUALS("/foo/bar", PathIterator("/foo", "bar").read());
        ASSERT_EQUALS("", PathIterator("", "").read());
        ASSERT_EQUALS("", PathIterator("", nullptr).read());
        ASSERT_EQUALS("", PathIterator(nullptr, "").read());
        ASSERT_EQUALS("", PathIterator(nullptr, nullptr).read());
        ASSERT_EQUALS("c:/windows/system32", PathIterator("C:", "Windows\\System32\\Drivers\\..\\.", true).read());
        ASSERT_EQUALS("C:", PathIterator("C:\\Program Files\\", "..").read());
    }
};

REGISTER_TEST(TestPathMatch)
