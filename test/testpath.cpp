/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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
#include "standards.h"

#include <set>
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
        TEST_CASE(get_path_from_filename);
        TEST_CASE(join);
        TEST_CASE(isDirectory);
        TEST_CASE(isFile);
        TEST_CASE(sameFileName);
        TEST_CASE(getFilenameExtension);
        TEST_CASE(identify);
        TEST_CASE(is_header_2);
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
        ASSERT(Path::acceptFile("index.c"));
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

        const std::set<std::string> extra = { ".extra", ".header" };
        ASSERT(Path::acceptFile("index.c", extra));
        ASSERT(Path::acceptFile("index.cpp", extra));
        ASSERT(Path::acceptFile("index.extra", extra));
        ASSERT(Path::acceptFile("index.header", extra));
        ASSERT(Path::acceptFile("index.h", extra)==false);
        ASSERT(Path::acceptFile("index.hpp", extra)==false);
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

    void sameFileName() const {
        ASSERT(Path::sameFileName("test", "test"));

        // case sensitivity cases
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
        ASSERT(Path::sameFileName("test", "Test"));
        ASSERT(Path::sameFileName("test", "TesT"));
        ASSERT(Path::sameFileName("test.h", "test.H"));
        ASSERT(Path::sameFileName("test.hh", "test.Hh"));
        ASSERT(Path::sameFileName("test.hh", "test.hH"));
#else
        ASSERT(!Path::sameFileName("test", "Test"));
        ASSERT(!Path::sameFileName("test", "TesT"));
        ASSERT(!Path::sameFileName("test.h", "test.H"));
        ASSERT(!Path::sameFileName("test.hh", "test.Hh"));
        ASSERT(!Path::sameFileName("test.hh", "test.hH"));
#endif
    }

    void getFilenameExtension() const {
        ASSERT_EQUALS("", Path::getFilenameExtension("test"));
        ASSERT_EQUALS("", Path::getFilenameExtension("Test"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.h"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("Test.h"));
        ASSERT_EQUALS("", Path::getFilenameExtension("test", true));
        ASSERT_EQUALS("", Path::getFilenameExtension("Test", true));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.h", true));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("Test.h", true));

        // case sensitivity cases
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.H"));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.Hh"));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.hH"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.H", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.Hh", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.hH", true));
#else
        ASSERT_EQUALS(".H", Path::getFilenameExtension("test.H"));
        ASSERT_EQUALS(".Hh", Path::getFilenameExtension("test.Hh"));
        ASSERT_EQUALS(".hH", Path::getFilenameExtension("test.hH"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.H", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.Hh", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.hH", true));
#endif
    }


    void identify() const {
        Standards::Language lang;
        bool header;

        ASSERT_EQUALS(Standards::Language::None, Path::identify(""));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("c"));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("cpp"));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("h"));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("hpp"));

        // TODO: what about files starting with a "."?
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".c"));
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".cpp"));
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".h"));
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".hpp"));

        // C
        ASSERT_EQUALS(Standards::Language::C, Path::identify("index.c"));
        ASSERT_EQUALS(Standards::Language::C, Path::identify("index.cl"));
        ASSERT_EQUALS(Standards::Language::C, Path::identify("C:\\foo\\index.c"));
        ASSERT_EQUALS(Standards::Language::C, Path::identify("/mnt/c/foo/index.c"));

        // In unix .C is considered C++
#ifdef _WIN32
        ASSERT_EQUALS(Standards::Language::C, Path::identify("C:\\foo\\index.C"));
#endif

        lang = Path::identify("index.c", &header);
        ASSERT_EQUALS(Standards::Language::C, lang);
        ASSERT_EQUALS(false, header);

        // C++
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.cpp"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.cxx"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.cc"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.c++"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.tpp"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.txx"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.ipp"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.ixx"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("C:\\foo\\index.cpp"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("C:\\foo\\index.Cpp"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("/mnt/c/foo/index.cpp"));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("/mnt/c/foo/index.Cpp"));

        // TODO: check for case-insenstive filesystem instead
        // In unix .C is considered C++
#if !defined(_WIN32) && !(defined(__APPLE__) && defined(__MACH__))
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.C"));
#else
        ASSERT_EQUALS(Standards::Language::C, Path::identify("index.C"));
#endif

        lang = Path::identify("index.cpp", &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(false, header);

        // headers
        lang = Path::identify("index.h", &header);
        ASSERT_EQUALS(Standards::Language::C, lang);
        ASSERT_EQUALS(true, header);

        lang = Path::identify("index.hpp", &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);
        lang = Path::identify("index.hxx", &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);
        lang = Path::identify("index.h++", &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);
        lang = Path::identify("index.hh", &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);

        ASSERT_EQUALS(Standards::Language::None, Path::identify("index.header"));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("index.htm"));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("index.html"));
    }

    void is_header_2() const {
        ASSERT(Path::isHeader2("index.h"));
        ASSERT(Path::isHeader2("index.hpp"));
        ASSERT(Path::isHeader2("index.hxx"));
        ASSERT(Path::isHeader2("index.h++"));
        ASSERT(Path::isHeader2("index.hh"));

        ASSERT(Path::isHeader2("index.c")==false);
        ASSERT(Path::isHeader2("index.cpp")==false);
        ASSERT(Path::isHeader2("index.header")==false);
        ASSERT(Path::isHeader2("index.htm")==false);
        ASSERT(Path::isHeader2("index.html")==false);
    }
};

REGISTER_TEST(TestPath)
