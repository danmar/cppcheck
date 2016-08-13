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

#include <string>
#include "testsuite.h"
#include "importproject.h"

class TestImportProject : public TestFixture {
public:
    TestImportProject() : TestFixture("TestImportProject") {
    }

private:

    void run() {
        TEST_CASE(setDefines);
        TEST_CASE(setIncludePaths);
    }

    void setDefines() {
        ImportProject::FileSettings fs;

        fs.setDefines("A");
        ASSERT_EQUALS("A=1", fs.defines);

        fs.setDefines("A;B;");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        fs.setDefines("A;;B;");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        fs.setDefines("A;;B");
        ASSERT_EQUALS("A=1;B=1", fs.defines);
    }

    void setIncludePaths() {
        ImportProject::FileSettings fs;
        std::list<std::string> in;
        in.push_back("../include");
        fs.setIncludePaths("abc/def/", in);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("abc/include/", fs.includePaths.front());
    }
};

REGISTER_TEST(TestImportProject)
