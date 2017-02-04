/*
* Cppcheck - A tool for static C/C++ code analysis
* Copyright (C) 2007-2016 Cppcheck team.
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
#include "testsuite.h"
#include "cppcheckexecutor.h"
#include "path.h"
#include "pathmatch.h"
#include "redirect.h"
#include <fstream>
#include <cstring>
#include <algorithm>


class TestSamples : public TestFixture {
public:
    TestSamples() : TestFixture("TestSamples") {
    }

private:

    void run() {
        TEST_CASE(runSamples);
    }

    void runSamples() const {
        REDIRECT;

        std::map<std::string, std::size_t> files;
        const std::vector<std::string> masks;
        const PathMatch matcher(masks);
#ifdef _WIN32
        FileLister::recursiveAddFiles(files, "..\\samples", matcher);
#else
        FileLister::recursiveAddFiles(files, "samples", matcher);
#endif
        for (std::map<std::string, std::size_t>::const_iterator i = files.begin(); i != files.end(); ++i) {
            CLEAR_REDIRECT_ERROUT;
            char* path = new char[i->first.size() + 1];
            strcpy(path, i->first.c_str());
            const char* argv[] = {
#ifdef _WIN32
                ".\\..\\testrunner",
#else
                "./testrunner",
#endif
                "--enable=style,warning,performance,portability", "--inconclusive", "-rp", "-f", "-q", path
            };
            std::string filename = i->first.substr(i->first.find_last_of("/\\")+1);
            if (filename == "good.cpp" || filename == "good.c") {
                CppCheckExecutor exec;
                exec.check(7, argv);
                ASSERT_EQUALS_MSG("", GET_REDIRECT_ERROUT, i->first);
            } else if (filename == "bad.cpp" || filename == "bad.c") {
                CppCheckExecutor exec;
                exec.check(7, argv);
                std::string expected_filename = Path::getPathFromFilename(i->first) + "out.txt";
                std::ifstream ifs(expected_filename.c_str());
                std::string expected((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                std::string actual = GET_REDIRECT_ERROUT;
                // We need some uniformization to make this work on Unix and Windows
                std::replace(actual.begin(), actual.end(), '/', '\\'); // Uniformize slashes.
                while (actual.find("..\\") != std::string::npos)
                    actual.erase(actual.find("..\\"), 3); // Remove '..\'
                ASSERT_EQUALS_MSG(expected, actual, i->first);
            }
            delete[] path;
        }
    }
};

REGISTER_TEST(TestSamples)
