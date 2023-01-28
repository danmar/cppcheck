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

#include "redirect.h"
#include "settings.h"
#include "fixture.h"
#include "helpers.h"
#include "threadexecutor.h"
#include "timer.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class TestThreadExecutor : public TestFixture {
public:
    TestThreadExecutor() : TestFixture("TestThreadExecutor") {}

private:
    Settings settings;

    /**
     * Execute check using n jobs for y files which are have
     * identical data, given within data.
     */
    void check(unsigned int jobs, int files, int result, const std::string &data, SHOWTIME_MODES showtime = SHOWTIME_MODES::SHOWTIME_NONE) {
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> filemap;
        for (int i = 1; i <= files; ++i) {
            std::ostringstream oss;
            oss << "file_" << i << ".cpp";
            filemap[oss.str()] = data.size();
        }

        settings.jobs = jobs;
        settings.showtime = showtime;
        ThreadExecutor executor(filemap, settings, *this);
        std::vector<ScopedFile> scopedfiles;
        scopedfiles.reserve(filemap.size());
        for (std::map<std::string, std::size_t>::const_iterator i = filemap.cbegin(); i != filemap.cend(); ++i)
            scopedfiles.emplace_back(i->first, data);

        ASSERT_EQUALS(result, executor.check());
    }

    void run() override {
        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(deadlock_with_many_errors);
        TEST_CASE(many_threads);
        TEST_CASE(many_threads_showtime);
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

    // #11249 - reports TSAN errors - only applies to threads not processes though
    void many_threads_showtime() {
        REDIRECT;
        check(16, 100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}", SHOWTIME_MODES::SHOWTIME_SUMMARY);
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
