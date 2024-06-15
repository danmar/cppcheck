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

#include "filesettings.h"
#include "fixture.h"

class TestFileSettings : public TestFixture {
public:
    TestFileSettings() : TestFixture("TestFileSettings") {}

private:
    void run() override {
        TEST_CASE(test);
    }

    void test() const {
        {
            const FileWithDetails p{"file.cpp"};
            ASSERT_EQUALS("file.cpp", p.path());
            ASSERT_EQUALS("file.cpp", p.spath());
            ASSERT_EQUALS(0, p.size());
        }
        {
            const FileWithDetails p{"file.cpp", 123};
            ASSERT_EQUALS("file.cpp", p.path());
            ASSERT_EQUALS("file.cpp", p.spath());
            ASSERT_EQUALS(123, p.size());
        }
        {
            const FileWithDetails p{"in/file.cpp"};
            ASSERT_EQUALS("in/file.cpp", p.path());
            ASSERT_EQUALS("in/file.cpp", p.spath());
            ASSERT_EQUALS(0, p.size());
        }
        {
            const FileWithDetails p{"in\\file.cpp"};
            ASSERT_EQUALS("in\\file.cpp", p.path());
            ASSERT_EQUALS("in/file.cpp", p.spath());
            ASSERT_EQUALS(0, p.size());
        }
        {
            const FileWithDetails p{"in/../file.cpp"};
            ASSERT_EQUALS("in/../file.cpp", p.path());
            ASSERT_EQUALS("file.cpp", p.spath());
            ASSERT_EQUALS(0, p.size());
        }
        {
            const FileWithDetails p{"in\\..\\file.cpp"};
            ASSERT_EQUALS("in\\..\\file.cpp", p.path());
            ASSERT_EQUALS("file.cpp", p.spath());
            ASSERT_EQUALS(0, p.size());
        }
    }
};

REGISTER_TEST(TestFileSettings)
