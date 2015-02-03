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

#include "testsuite.h"
#include "filelister.h"
#include "settings.h"
#include <fstream>

#ifndef _WIN32
#include <vector>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#endif

class TestFileLister: public TestFixture {
public:
    TestFileLister()
        :TestFixture("TestFileLister") {
    }

private:
    void run() {
        // bail out if the tests are not executed from the base folder
        {
            std::ifstream fin("test/testfilelister.cpp");
            if (!fin.is_open())
                return;
        }

        TEST_CASE(isDirectory);
#ifndef _WIN32
        TEST_CASE(absolutePath);
#endif
        TEST_CASE(recursiveAddFiles);
    }

    void isDirectory() const {
        ASSERT_EQUALS(false, FileLister::isDirectory("readme.txt"));
        ASSERT_EQUALS(true, FileLister::isDirectory("lib"));
    }

#ifndef _WIN32
    void absolutePath() const {
        std::vector<char> current_dir;
#ifdef PATH_MAX
        current_dir.resize(PATH_MAX);
#else
        current_dir.resize(1024);
#endif
        while (getcwd(&current_dir[0], current_dir.size()) == nullptr && errno == ERANGE) {
            current_dir.resize(current_dir.size() + 1024);
        }

        std::string absolute_path = FileLister::getAbsolutePath(".");
        ASSERT_EQUALS(&current_dir[0], absolute_path);
    }
#endif

    void recursiveAddFiles() const {
        // Recursively add add files..
        std::map<std::string, std::size_t> files;
        std::set<std::string> extra;
        FileLister::recursiveAddFiles(files, ".", extra);

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
};

REGISTER_TEST(TestFileLister)
