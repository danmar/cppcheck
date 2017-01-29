// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2016 Cppcheck team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "options.h"
#include "testsuite.h"


class TestOptions: public TestFixture {
public:
    TestOptions()
        :TestFixture("TestOptions") {
    }


private:
    void run() {
        TEST_CASE(which_test);
        TEST_CASE(which_test_method);
        TEST_CASE(no_test_method);
        TEST_CASE(not_quiet);
        TEST_CASE(quiet);
        TEST_CASE(multiple_testcases);
        TEST_CASE(invalid_switches);
    }


    void which_test() const {
        const char* argv[] = {"./test_runner", "TestClass"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS("TestClass", args.which_test());
    }


    void which_test_method() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS("TestClass::TestMethod", args.which_test());
    }


    void no_test_method() const {
        const char* argv[] = {"./test_runner"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS("", args.which_test());
    }


    void not_quiet() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-v"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS(false, args.quiet());
    }


    void quiet() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-q"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS(true, args.quiet());
    }




    void multiple_testcases() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "Ignore::ThisOne"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS("TestClass::TestMethod", args.which_test());
    }


    void invalid_switches() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-a", "-v", "-q"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS("TestClass::TestMethod", args.which_test());
        ASSERT_EQUALS(true, args.quiet());
    }
};

REGISTER_TEST(TestOptions)
