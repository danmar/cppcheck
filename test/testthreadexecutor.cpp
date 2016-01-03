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


#include "cppcheck.h"
#include "testsuite.h"
#include "threadexecutor.h"
#include "cppcheckexecutor.h"

#include <map>
#include <string>


class TestThreadExecutor : public TestFixture {
public:
    TestThreadExecutor() : TestFixture("TestThreadExecutor") {
    }

private:
    Settings settings;

    /**
     * Execute check using n jobs for y files which are have
     * identical data, given within data.
     */
    void check(unsigned int jobs, int files, int result, const std::string &data) {
        errout.str("");
        output.str("");
        if (!ThreadExecutor::isEnabled()) {
            // Skip this check on systems which don't use this feature
            return;
        }

        std::map<std::string, std::size_t> filemap;
        for (int i = 1; i <= files; ++i) {
            std::ostringstream oss;
            oss << "file_" << i << ".cpp";
            filemap[oss.str()] = 1;
        }

        settings.jobs = jobs;
        ThreadExecutor executor(filemap, settings, *this);
        for (std::map<std::string, std::size_t>::const_iterator i = filemap.begin(); i != filemap.end(); ++i)
            executor.addFileContent(i->first, data);

        ASSERT_EQUALS(result, executor.check());
    }

    void run() {
        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(deadlock_with_many_errors);
        TEST_CASE(many_threads);
        TEST_CASE(no_errors_more_files);
        TEST_CASE(no_errors_less_files);
        TEST_CASE(no_errors_equal_amount_files);
        TEST_CASE(one_error_less_files);
        TEST_CASE(one_error_several_files);
    }

    void deadlock_with_many_errors() {
        std::ostringstream oss;
        oss << "int main()\n"
            << "{\n";
        for (int i = 0; i < 500; i++)
            oss << "  {char *a = malloc(10);}\n";

        oss << "  return 0;\n"
            << "}\n";
        check(2, 3, 3, oss.str());
    }

    void many_threads() {
        check(16, 100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_more_files() {
        check(2, 3, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_less_files() {
        check(2, 1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_equal_amount_files() {
        check(2, 2, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void one_error_less_files() {
        check(2, 1, 1,
              "int main()\n"
              "{\n"
              "  {char *a = malloc(10);}\n"
              "  return 0;\n"
              "}");
    }

    void one_error_several_files() {
        check(2, 20, 20,
              "int main()\n"
              "{\n"
              "  {char *a = malloc(10);}\n"
              "  return 0;\n"
              "}");
    }
};

REGISTER_TEST(TestThreadExecutor)
