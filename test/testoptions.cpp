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

#include "options.h"
#include "fixture.h"

#include <functional>
#include <set>
#include <string>


class TestOptions : public TestFixture {
public:
    TestOptions()
        : TestFixture("TestOptions") {}


private:
    void run() override {
        TEST_CASE(which_test);
        TEST_CASE(which_test_method);
        TEST_CASE(no_test_method);
        TEST_CASE(not_quiet);
        TEST_CASE(quiet);
        TEST_CASE(not_help);
        TEST_CASE(help);
        TEST_CASE(help_long);
        TEST_CASE(multiple_testcases);
        TEST_CASE(multiple_testcases_ignore_duplicates);
        TEST_CASE(invalid_switches);
    }


    void which_test() const {
        const char* argv[] = {"./test_runner", "TestClass"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT(std::set<std::string> {"TestClass"} == args.which_test());
    }


    void which_test_method() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT(std::set<std::string> {"TestClass::TestMethod"} == args.which_test());
    }


    void no_test_method() const {
        const char* argv[] = {"./test_runner"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT(std::set<std::string> {""} == args.which_test());
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

    void not_help() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-v"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS(false, args.help());
    }


    void help() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-h"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS(true, args.help());
    }


    void help_long() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "--help"};
        options args(sizeof argv / sizeof argv[0], argv);
        ASSERT_EQUALS(true, args.help());
    }

    void multiple_testcases() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "TestClass::AnotherTestMethod"};
        options args(sizeof argv / sizeof argv[0], argv);
        std::set<std::string> expected {"TestClass::TestMethod", "TestClass::AnotherTestMethod"};
        ASSERT(expected == args.which_test());
    }

    void multiple_testcases_ignore_duplicates() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "TestClass"};
        options args(sizeof argv / sizeof argv[0], argv);
        std::set<std::string> expected {"TestClass"};
        ASSERT(expected == args.which_test());
    }

    void invalid_switches() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-a", "-v", "-q"};
        options args(sizeof argv / sizeof argv[0], argv);
        std::set<std::string> expected {"TestClass::TestMethod"};
        ASSERT(expected == args.which_test());
        ASSERT_EQUALS(true, args.quiet());
    }
};

REGISTER_TEST(TestOptions)
