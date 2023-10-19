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
#include "importproject.h"
#include "timer.h"
#include "library.h"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class TestProcessExecutorBase : public TestFixture {
public:
    TestProcessExecutorBase(const char * const name, bool useFS) : TestFixture(name), useFS(useFS) {}

private:
    Settings settings = settingsBuilder().library("std.cfg").build();
    bool useFS;

    std::string fprefix() const
    {
        if (useFS)
            return "processfs";
        return "process";
    }

    struct CheckOptions
    {
        CheckOptions() = default;
        bool quiet = true;
        SHOWTIME_MODES showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        const char* plistOutput = nullptr;
        std::vector<std::string> filesList;
        bool clangTidy = false;
        bool executeCommandCalled = false;
        std::string exe;
        std::vector<std::string> args;
    };

    /**
     * Execute check using n jobs for y files which are have
     * identical data, given within data.
     */
    void check(unsigned int jobs, int files, int result, const std::string &data, const CheckOptions& opt = make_default_obj{}) {
        errout.str("");
        output.str("");

        Settings s = settings;

        std::map<std::string, std::size_t> filemap;
        if (opt.filesList.empty()) {
            for (int i = 1; i <= files; ++i) {
                std::string f_s = fprefix() + "_" + std::to_string(i) + ".cpp";
                filemap[f_s] = data.size();
                if (useFS) {
                    ImportProject::FileSettings fs;
                    fs.filename = std::move(f_s);
                    s.project.fileSettings.emplace_back(std::move(fs));
                }
            }
        }
        else {
            for (const auto& f : opt.filesList)
            {
                filemap[f] = data.size();
                if (useFS) {
                    ImportProject::FileSettings fs;
                    fs.filename = f;
                    s.project.fileSettings.emplace_back(std::move(fs));
                }
            }
        }

        s.jobs = jobs;
        s.showtime = opt.showtime;
        s.quiet = opt.quiet;
        if (opt.plistOutput)
            s.plistOutput = opt.plistOutput;

        bool executeCommandCalled = false;
        std::string exe;
        std::vector<std::string> args;
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        auto executeFn = [&executeCommandCalled, &exe, &args](std::string e,std::vector<std::string> a,std::string,std::string&){
            executeCommandCalled = true;
            exe = std::move(e);
            args = std::move(a);
            return EXIT_SUCCESS;
        };

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filemap.size());
        for (std::map<std::string, std::size_t>::const_iterator i = filemap.cbegin(); i != filemap.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, data));

        // clear files list so only fileSettings are used
        if (useFS)
            filemap.clear();

        ProcessExecutor executor(filemap, s, s.nomsg, *this, executeFn);
        ASSERT_EQUALS(result, executor.check());
        ASSERT_EQUALS(opt.executeCommandCalled, executeCommandCalled);
        ASSERT_EQUALS(opt.exe, exe);
        ASSERT_EQUALS(opt.args.size(), args.size());
        for (int i = 0; i < args.size(); ++i)
        {
            ASSERT_EQUALS(opt.args[i], args[i]);
        }
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
        TEST_CASE(clangTidy);
        TEST_CASE(showtime_top5_file);
        TEST_CASE(showtime_top5_summary);
        TEST_CASE(showtime_file);
        TEST_CASE(showtime_summary);
        TEST_CASE(showtime_file_total);
#endif // !WIN32
    }

    void deadlock_with_many_errors() {
        std::ostringstream oss;
        oss << "int main()\n"
            << "{\n";
        for (int i = 0; i < 500; i++)
            oss << "  {int i = *((int*)0);}\n";

        oss << "  return 0;\n"
            << "}\n";
        check(2, 3, 3, oss.str());
    }

    // TODO: check the output
    void many_threads() {
        check(16, 100, 100,
              "int main()\n"
              "{\n"
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}");
    }

    // #11249 - reports TSAN errors
    void many_threads_showtime() {
        SUPPRESS;
        check(16, 100, 100,
              "int main()\n"
              "{\n"
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions, $.showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY));
    }

    void many_threads_plist() {
        const std::string plistOutput = "plist_" + fprefix() + "/";
        ScopedFile plistFile("dummy", "", plistOutput);

        check(16, 100, 100,
              "int main()\n"
              "{\n"
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions, $.plistOutput = plistOutput.c_str()));
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
              "  {int i = *((int*)0);}\n"
              "  return 0;\n"
              "}");
    }

    void one_error_several_files() {
        check(2, 20, 20,
              "int main()\n"
              "{\n"
              "  {int i = *((int*)0);}\n"
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
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}",
              dinit(CheckOptions,
                    $.quiet = false,
                        $.filesList = files));
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

    void clangTidy() {
        // TODO: we currently only invoke it with ImportProject::FileSettings
        if (!useFS)
            return;

#ifdef _WIN32
        const char exe[] = "clang-tidy.exe";
#else
        const char exe[] = "clang-tidy";
#endif
        (void)exe;

        const std::string file = fprefix() + "_1.cpp";
        // TODO: the invocation cannot be checked as the code is called in the forked process
        check(2, 1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}",
              dinit(CheckOptions,
                    $.quiet = false,
                        $.clangTidy = true /*,
                                              $.executeCommandCalled = true,
                                              $.exe = exe,
                                              $.args = {"-quiet", "-checks=*,-clang-analyzer-*,-llvm*", file, "--"}*/));
        ASSERT_EQUALS("Checking " + file + " ...\n", output.str());
    }

    // TODO: provide data which actually shows values above 0

    // TODO: should this be logged only once like summary?
    void showtime_top5_file() {
        REDIRECT; // should not cause TSAN failures as the showtime logging is synchronized
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_TOP5_FILE));
        // for each file: top5 results + overall + empty line
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // for each file: top5 results + overall + empty line
        TODO_ASSERT_EQUALS(static_cast<long long>(5 + 1 + 1) * 2, 0, cppcheck::count_all_of(output_s, '\n'));
    }

    void showtime_top5_summary() {
        REDIRECT;
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_TOP5_SUMMARY));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // once: top5 results + overall + empty line
        TODO_ASSERT_EQUALS(5 + 1 + 1, 2, cppcheck::count_all_of(output_s, '\n'));
        // should only report the top5 once
        ASSERT(output_s.find("1 result(s)") == std::string::npos);
        TODO_ASSERT(output_s.find("2 result(s)") != std::string::npos);
    }

    void showtime_file() {
        REDIRECT; // should not cause TSAN failures as the showtime logging is synchronized
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_FILE));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        TODO_ASSERT_EQUALS(2, 0, cppcheck::count_all_of(output_s, "Overall time:"));
    }

    void showtime_summary() {
        REDIRECT; // should not cause TSAN failures as the showtime logging is synchronized
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // should only report the actual summary once
        ASSERT(output_s.find("1 result(s)") == std::string::npos);
        TODO_ASSERT(output_s.find("2 result(s)") != std::string::npos);
    }

    void showtime_file_total() {
        REDIRECT; // should not cause TSAN failures as the showtime logging is synchronized
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_FILE_TOTAL));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        TODO_ASSERT(output_s.find("Check time: " + fprefix() + "_1.cpp: ") != std::string::npos);
        TODO_ASSERT(output_s.find("Check time: " + fprefix() + "_2.cpp: ") != std::string::npos);
    }

    // TODO: test whole program analysis
};

class TestProcessExecutorFiles : public TestProcessExecutorBase {
public:
    TestProcessExecutorFiles() : TestProcessExecutorBase("TestProcessExecutorFiles", false) {}
};

class TestProcessExecutorFS : public TestProcessExecutorBase {
public:
    TestProcessExecutorFS() : TestProcessExecutorBase("TestProcessExecutorFS", true) {}
};

REGISTER_TEST(TestProcessExecutorFiles)
REGISTER_TEST(TestProcessExecutorFS)
