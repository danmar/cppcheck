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

#include "filelister.h"
#include "path.h"
#include "pathmatch.h"
#include "fixture.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <utility>

class TestFileLister : public TestFixture {
public:
    TestFileLister() : TestFixture("TestFileLister") {}

private:
    void run() override {
        TEST_CASE(recursiveAddFiles);
        TEST_CASE(recursiveAddFilesEmptyPath);
        TEST_CASE(excludeFile1);
        TEST_CASE(excludeFile2);
    }

    // TODO: generate file list instead
    static std::string findBaseDir() {
        std::string basedir;
        while (!Path::isDirectory(Path::join(basedir, ".github"))) {
            basedir += "../";
        }
        return basedir;
    }

    void recursiveAddFiles() const {
        const std::string adddir = findBaseDir() + ".";

        // Recursively add add files..
        std::map<std::string, std::size_t> files;
        std::vector<std::string> masks;
        PathMatch matcher(masks);
        std::string err = FileLister::recursiveAddFiles(files, adddir, matcher);
        ASSERT_EQUALS("", err);

        ASSERT(!files.empty());

#ifdef _WIN32
        std::string dirprefix;
        if (adddir != ".")
            dirprefix = adddir + "/";
#else
        const std::string dirprefix = adddir + "/";
#endif

        // Make sure source files are added..
        ASSERT(files.find(dirprefix + "cli/main.cpp") != files.end());
        ASSERT(files.find(dirprefix + "lib/token.cpp") != files.end());
        ASSERT(files.find(dirprefix + "lib/tokenize.cpp") != files.end());
        ASSERT(files.find(dirprefix + "gui/main.cpp") != files.end());
        ASSERT(files.find(dirprefix + "test/testfilelister.cpp") != files.end());

        // Make sure headers are not added..
        ASSERT(files.find(dirprefix + "lib/tokenize.h") == files.end());
    }

    void recursiveAddFilesEmptyPath() const {
        std::map<std::string, std::size_t> files;
        const std::string err = FileLister::recursiveAddFiles(files, "", PathMatch({}));
        ASSERT_EQUALS("no path specified", err);
    }

    void excludeFile1() const {
        const std::string basedir = findBaseDir();

        std::map<std::string, std::size_t> files;
        std::vector<std::string> ignored{"lib/token.cpp"};
        PathMatch matcher(ignored);
        std::string err = FileLister::recursiveAddFiles(files, basedir + "lib/token.cpp", matcher);
        ASSERT_EQUALS("", err);
        ASSERT(files.empty());
    }

    void excludeFile2() const {
        const std::string basedir = findBaseDir();

        std::map<std::string, std::size_t> files;
        std::vector<std::string> ignored;
        PathMatch matcher(ignored);
        std::string err = FileLister::recursiveAddFiles(files, basedir + "lib/token.cpp", matcher);
        ASSERT_EQUALS("", err);
        ASSERT_EQUALS(1, files.size());
        ASSERT_EQUALS(basedir + "lib/token.cpp", files.begin()->first);
    }

    // TODO: test errors
};

REGISTER_TEST(TestFileLister)
