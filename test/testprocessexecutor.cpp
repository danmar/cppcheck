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

#include "processexecutor.h"
#include "redirect.h"
#include "settings.h"
#include "fixture.h"
#include "helpers.h"
#include "timer.h"
#include "library.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class TestProcessExecutor : public TestFixture {
public:
    TestProcessExecutor() : TestFixture("TestProcessExecutor") {}

private:
    Settings settings = settingsBuilder().library("std.cfg").build();

    static std::string fprefix()
    {
        return "process";
    }

    struct CheckOptions
    {
        CheckOptions() = default;
        SHOWTIME_MODES showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        const char* plistOutput = nullptr;
        std::vector<std::string> filesList;
    };

    /**
     * Execute check using n jobs for y files which are have
     * identical data, given within data.
     */
    void check(unsigned int jobs, int files, int result, const std::string &data, const CheckOptions& opt = make_default_obj{}) {
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> filemap;
        if (opt.filesList.empty()) {
            for (int i = 1; i <= files; ++i) {
                std::ostringstream oss;
                oss << fprefix() << "_" << i << ".cpp";
                filemap[oss.str()] = data.size();
            }
        }
        else {
            for (const auto& f : opt.filesList)
            {
                filemap[f] = data.size();
            }
        }

        Settings s = settings;
        s.jobs = jobs;
        s.showtime = opt.showtime;
        if (opt.plistOutput)
            s.plistOutput = opt.plistOutput;
        // TODO: test with settings.project.fileSettings;
        ProcessExecutor executor(filemap, s, s.nomsg, *this);
        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filemap.size());
        for (std::map<std::string, std::size_t>::const_iterator i = filemap.cbegin(); i != filemap.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, data));

        ASSERT_EQUALS(result, executor.check());
    }

    void run() override {
#if !defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
        TEST_CASE(deadlock_with_many_errors);
        TEST_CASE(many_threads);
        TEST_CASE(many_threads_showtime);
        TEST_CASE(many_threads_plist);
        TEST_CASE(no_errors_more_files);
        TEST_CASE(no_errors_less_files);
        TEST_CASE(no_errors_equal_amount_files);
        TEST_CASE(one_error_less_files);
        TEST_CASE(one_error_several_files);
        TEST_CASE(markup);
#endif // !WIN32
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

    // #11249 - reports TSAN errors
    void many_threads_showtime() {
        SUPPRESS;
        check(16, 100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions, $.showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY));
    }

    void many_threads_plist() {
        const char plistOutput[] = "plist_process/";
        ScopedFile plistFile("dummy", "", plistOutput);

        check(16, 100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions, $.plistOutput = plistOutput));
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

    void markup() {
        const Settings settingsOld = settings;
        settings.library.mMarkupExtensions.emplace(".cp1");
        settings.library.mProcessAfterCode.emplace(".cp1", true);

        const std::vector<std::string> files = {
            fprefix() + "_1.cp1", fprefix() + "_2.cpp", fprefix() + "_3.cp1", fprefix() + "_4.cpp"
        };

        // the checks are not executed on the markup files => expected result is 2
        check(2, 4, 2,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}",
              dinit(CheckOptions, $.filesList = files));
        // TODO: order of "Checking" and "checked" is affected by thread
        /*TODO_ASSERT_EQUALS("Checking " + fprefix() + "_2.cpp ...\n"
                           "1/4 files checked 25% done\n"
                           "Checking " + fprefix() + "_4.cpp ...\n"
                           "2/4 files checked 50% done\n"
                           "Checking " + fprefix() + "_1.cp1 ...\n"
                           "3/4 files checked 75% done\n"
                           "Checking " + fprefix() + "_3.cp1 ...\n"
                           "4/4 files checked 100% done\n",
                           "Checking " + fprefix() + "_1.cp1 ...\n"
                           "1/4 files checked 25% done\n"
                           "Checking " + fprefix() + "_2.cpp ...\n"
                           "2/4 files checked 50% done\n"
                           "Checking " + fprefix() + "_3.cp1 ...\n"
                           "3/4 files checked 75% done\n"
                           "Checking " + fprefix() + "_4.cpp ...\n"
                           "4/4 files checked 100% done\n",
                           output.str());*/
        settings = settingsOld;
    }

    // TODO: test clang-tidy
    // TODO: test whole program analysis
};

REGISTER_TEST(TestProcessExecutor)
