/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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
#include "pathmatch.h"
#include "testsuite.h"

#include <cstddef>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <utility>

class TestFileLister : public TestFixture {
public:
    TestFileLister()
        : TestFixture("TestFileLister") {}

private:
    void run() override {
        // bail out if the tests are not executed from the base folder
        {
            std::ifstream fin("test/testfilelister.cpp");
            if (!fin.is_open())
                return;
        }

        TEST_CASE(isDirectory);
        TEST_CASE(recursiveAddFiles);
        TEST_CASE(fileExists);
    }

    void isDirectory() const {
        ASSERT_EQUALS(false, FileLister::isDirectory("readme.txt"));
        ASSERT_EQUALS(true, FileLister::isDirectory("lib"));
    }

    void recursiveAddFiles() const {
        // Recursively add add files..
        std::map<std::string, std::size_t> files;
        std::vector<std::string> masks;
        PathMatch matcher(masks);
        std::string err = FileLister::recursiveAddFiles(files, ".", matcher);
        ASSERT(err.empty());

        // In case there are leading "./"..
        for (std::map<std::string, std::size_t>::iterator i = files.begin(); i != files.end();) {
            if (i->first.compare(0,2,"./") == 0) {
                files[i->first.substr(2)] = i->second;
                files.erase(i++);
            } else
                ++i;
        }

        // Make sure source files are added..
        ASSERT(files.find("cli/main.cpp") != files.end());
        ASSERT(files.find("lib/token.cpp") != files.end());
        ASSERT(files.find("lib/tokenize.cpp") != files.end());
        ASSERT(files.find("test/testfilelister.cpp") != files.end());

        // Make sure headers are not added..
        ASSERT(files.find("lib/tokenize.h") == files.end());
    }

    void fileExists() const {
        ASSERT_EQUALS(false, FileLister::fileExists("lib"));
        ASSERT_EQUALS(true, FileLister::fileExists("readme.txt"));
    }
};

REGISTER_TEST(TestFileLister)
