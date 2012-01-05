/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include <fstream>
#include <algorithm>

#ifndef _WIN32
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#endif

class TestFileLister: public TestFixture {
public:
    TestFileLister()
        :TestFixture("TestFileLister")
    {}

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

    void isDirectory() {
        ASSERT_EQUALS(false, FileLister::isDirectory("readme.txt"));
        ASSERT_EQUALS(true, FileLister::isDirectory("lib"));
    }

#ifndef _WIN32
    void absolutePath() {
        char current_dir[PATH_MAX];
        getcwd(current_dir, sizeof(current_dir));

        std::string absolute_path = FileLister::getAbsolutePath(".");
        ASSERT_EQUALS(current_dir, absolute_path);
    }
#endif

    void recursiveAddFiles() {
        // Recursively add add files..
        std::vector<std::string> filenames;
        std::map<std::string, long> filesizes;
        FileLister::recursiveAddFiles(filenames, filesizes, ".");

        // Ensure a size entry is present for each listed file
        for (std::vector<std::string>::const_iterator i = filenames.begin(); i != filenames.end(); ++i) {
            ASSERT(filesizes.find(*i) != filesizes.end());
        }

        // In case there are leading "./"..
        for (unsigned int i = 0; i < filenames.size(); ++i) {
            if (filenames[i].compare(0,2,"./") == 0)
                filenames[i].erase(0,2);
        }

        // Make sure source files are added..
        ASSERT(std::find(filenames.begin(), filenames.end(), "cli/main.cpp") != filenames.end());
        ASSERT(std::find(filenames.begin(), filenames.end(), "lib/token.cpp") != filenames.end());
        ASSERT(std::find(filenames.begin(), filenames.end(), "lib/tokenize.cpp") != filenames.end());
        ASSERT(std::find(filenames.begin(), filenames.end(), "test/testfilelister.cpp") != filenames.end());

        // Make sure headers are not added..
        ASSERT(std::find(filenames.begin(), filenames.end(), "lib/tokenize.h") == filenames.end());
    }
};

REGISTER_TEST(TestFileLister)

