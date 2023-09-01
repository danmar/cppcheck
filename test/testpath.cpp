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

#include "path.h"
#include "fixture.h"
#include "helpers.h"

#include <string>
#include <vector>

class TestPath : public TestFixture {
public:
    TestPath() : TestFixture("TestPath") {}

private:

    void run() override {
        TEST_CASE(removeQuotationMarks);
        TEST_CASE(acceptFile);
        TEST_CASE(getCurrentPath);
        TEST_CASE(getCurrentExecutablePath);
        TEST_CASE(isAbsolute);
        TEST_CASE(getRelative);
        TEST_CASE(is_c);
        TEST_CASE(is_cpp);
        TEST_CASE(get_path_from_filename);
        TEST_CASE(join);
        TEST_CASE(isDirectory);
        TEST_CASE(isFile);
    }

    void removeQuotationMarks() const {
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

    void acceptFile() const {
        ASSERT(Path::acceptFile("index.cpp"));
        ASSERT(Path::acceptFile("index.invalid.cpp"));
        ASSERT(Path::acceptFile("index.invalid.Cpp"));
        ASSERT(Path::acceptFile("index.invalid.C"));
        ASSERT(Path::acceptFile("index.invalid.C++"));
        ASSERT(Path::acceptFile("index.")==false);
        ASSERT(Path::acceptFile("index")==false);
        ASSERT(Path::acceptFile("")==false);
        ASSERT(Path::acceptFile("C")==false);

        // don't accept any headers
        ASSERT_EQUALS(false, Path::acceptFile("index.h"));
        ASSERT_EQUALS(false, Path::acceptFile("index.hpp"));
    }

    void getCurrentPath() const {
        ASSERT_EQUALS(true, Path::isAbsolute(Path::getCurrentPath()));
    }

    void getCurrentExecutablePath() const {
        ASSERT_EQUALS(false, Path::getCurrentExecutablePath("").empty());
    }

    void isAbsolute() const {
#ifdef _WIN32
        ASSERT_EQUALS(true, Path::isAbsolute("C:\\foo\\bar"));
        ASSERT_EQUALS(true, Path::isAbsolute("C:/foo/bar"));
        ASSERT_EQUALS(true, Path::isAbsolute("\\\\foo\\bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo\\bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo/bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo.cpp"));
        ASSERT_EQUALS(false, Path::isAbsolute("C:foo.cpp"));
        ASSERT_EQUALS(false, Path::isAbsolute("C:foo\\bar.cpp"));
        ASSERT_EQUALS(false, Path::isAbsolute("bar.cpp"));
        TODO_ASSERT_EQUALS(true, false, Path::isAbsolute("\\"));
#else
        ASSERT_EQUALS(true, Path::isAbsolute("/foo/bar"));
        ASSERT_EQUALS(true, Path::isAbsolute("/"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo/bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo.cpp"));
#endif
    }

    void getRelative() const {
        const std::vector<std::string> basePaths = {
            "", // Don't crash with empty paths
            "C:/foo",
            "C:/bar/",
            "C:/test.cpp"
        };

        ASSERT_EQUALS("x.c", Path::getRelativePath("C:/foo/x.c", basePaths));
        ASSERT_EQUALS("y.c", Path::getRelativePath("C:/bar/y.c", basePaths));
        ASSERT_EQUALS("foo/y.c", Path::getRelativePath("C:/bar/foo/y.c", basePaths));
        ASSERT_EQUALS("C:/test.cpp", Path::getRelativePath("C:/test.cpp", basePaths));
        ASSERT_EQUALS("C:/foobar/test.cpp", Path::getRelativePath("C:/foobar/test.cpp", basePaths));
    }

    void is_c() const {
        ASSERT(Path::isC("index.cpp")==false);
        ASSERT(Path::isC("")==false);
        ASSERT(Path::isC("c")==false);
        ASSERT(Path::isC("index.c"));
        ASSERT(Path::isC("C:\\foo\\index.c"));

        // In unix .C is considered C++
#ifdef _WIN32
        ASSERT_EQUALS(true, Path::isC("C:\\foo\\index.C"));
#else
        ASSERT_EQUALS(false, Path::isC("C:\\foo\\index.C"));
#endif
    }

    void is_cpp() const {
        ASSERT(Path::isCPP("index.c")==false);

        // In unix .C is considered C++
#ifdef _WIN32
        ASSERT_EQUALS(false, Path::isCPP("index.C"));
#else
        ASSERT_EQUALS(true, Path::isCPP("index.C"));
#endif
        ASSERT(Path::isCPP("index.cpp"));
        ASSERT(Path::isCPP("C:\\foo\\index.cpp"));
        ASSERT(Path::isCPP("C:\\foo\\index.Cpp"));
    }

    void get_path_from_filename() const {
        ASSERT_EQUALS("", Path::getPathFromFilename("index.h"));
        ASSERT_EQUALS("/tmp/", Path::getPathFromFilename("/tmp/index.h"));
        ASSERT_EQUALS("a/b/c/", Path::getPathFromFilename("a/b/c/index.h"));
        ASSERT_EQUALS("a/b/c/", Path::getPathFromFilename("a/b/c/"));
    }

    void join() const {
        ASSERT_EQUALS("a", Path::join("a", ""));
        ASSERT_EQUALS("a", Path::join("", "a"));
        ASSERT_EQUALS("a/b", Path::join("a", "b"));
        ASSERT_EQUALS("a/b", Path::join("a/", "b"));
        ASSERT_EQUALS("/b", Path::join("a", "/b"));
    }

    void isDirectory() const {
        ScopedFile file("testpath.txt", "", "testpath");
        ScopedFile file2("testpath2.txt", "");
        ASSERT_EQUALS(false, Path::isDirectory("testpath.txt"));
        ASSERT_EQUALS(true, Path::isDirectory("testpath"));
        ASSERT_EQUALS(false, Path::isDirectory("testpath/testpath.txt"));
        ASSERT_EQUALS(false, Path::isDirectory("testpath2.txt"));
    }

    void isFile() const {
        ScopedFile file("testpath.txt", "", "testpath");
        ScopedFile file2("testpath2.txt", "");
        ASSERT_EQUALS(false, Path::isFile("testpath"));
        ASSERT_EQUALS(false, Path::isFile("testpath.txt"));
        ASSERT_EQUALS(true, Path::isFile("testpath/testpath.txt"));
        ASSERT_EQUALS(true, Path::isFile("testpath2.txt"));
    }
};

REGISTER_TEST(TestPath)
