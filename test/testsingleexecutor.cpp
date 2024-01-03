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
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cppcheck.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "redirect.h"
#include "settings.h"
#include "singleexecutor.h"
#include "timer.h"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class TestSingleExecutorBase : public TestFixture {
protected:
    TestSingleExecutorBase(const char * const name, bool useFS) : TestFixture(name), useFS(useFS) {}

private:
    Settings settings = settingsBuilder().library("std.cfg").build();
    bool useFS;

    std::string fprefix() const
    {
        if (useFS)
            return "singlefs";
        return "single";
    }

    static std::string zpad3(int i)
    {
        if (i < 10)
            return "00" + std::to_string(i);
        if (i < 100)
            return "0" + std::to_string(i);
        return std::to_string(i);
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

    void check(int files, int result, const std::string &data, const CheckOptions& opt = make_default_obj{}) {
        errout.str("");

        std::list<FileSettings> fileSettings;

        std::list<std::pair<std::string, std::size_t>> filelist;
        if (opt.filesList.empty()) {
            for (int i = 1; i <= files; ++i) {
                std::string f_s = fprefix() + "_" + zpad3(i) + ".cpp";
                filelist.emplace_back(f_s, data.size());
                if (useFS) {
                    FileSettings fs;
                    fs.filename = std::move(f_s);
                    fileSettings.emplace_back(std::move(fs));
                }
            }
        }
        else {
            for (const auto& f : opt.filesList)
            {
                filelist.emplace_back(f, data.size());
                if (useFS) {
                    FileSettings fs;
                    fs.filename = f;
                    fileSettings.emplace_back(std::move(fs));
                }
            }
        }

        Settings s = settings;
        s.showtime = opt.showtime;
        s.quiet = opt.quiet;
        if (opt.plistOutput)
            s.plistOutput = opt.plistOutput;
        s.clangTidy = opt.clangTidy;

        bool executeCommandCalled = false;
        std::string exe;
        std::vector<std::string> args;
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        CppCheck cppcheck(*this, true, [&executeCommandCalled, &exe, &args](std::string e,std::vector<std::string> a,std::string,std::string&){
            executeCommandCalled = true;
            exe = std::move(e);
            args = std::move(a);
            return EXIT_SUCCESS;
        });
        cppcheck.settings() = s;

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (std::list<std::pair<std::string, std::size_t>>::const_iterator i = filelist.cbegin(); i != filelist.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, data));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        SingleExecutor executor(cppcheck, filelist, fileSettings, s, s.nomsg, *this);
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
        TEST_CASE(many_files);
        TEST_CASE(many_files_showtime);
        TEST_CASE(many_files_plist);
        TEST_CASE(no_errors_more_files);
        TEST_CASE(no_errors_less_files);
        TEST_CASE(no_errors_equal_amount_files);
        TEST_CASE(one_error_less_files);
        TEST_CASE(one_error_several_files);
        TEST_CASE(clangTidy);
        TEST_CASE(showtime_top5_file);
        TEST_CASE(showtime_top5_summary);
        TEST_CASE(showtime_file);
        TEST_CASE(showtime_summary);
        TEST_CASE(showtime_file_total);
        TEST_CASE(suppress_error_library);
        TEST_CASE(unique_errors);
    }

    void many_files() {
        check(100, 100,
              "int main()\n"
              "{\n"
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions,
                         $.quiet = false));
        std::string expected;
        for (int i = 1; i <= 100; ++i) {
            expected += "Checking " + fprefix() + "_" + zpad3(i) + ".cpp ...\n";
            expected += std::to_string(i) + "/100 files checked " + std::to_string(i) + "% done\n";
        }
        ASSERT_EQUALS(expected, output_str());
    }

    void many_files_showtime() {
        SUPPRESS;
        check(100, 100,
              "int main()\n"
              "{\n"
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions, $.showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY));
    }

    void many_files_plist() {
        const std::string plistOutput = "plist_" + fprefix() + "/";
        ScopedFile plistFile("dummy", "", plistOutput);

        check(100, 100,
              "int main()\n"
              "{\n"
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions, $.plistOutput = plistOutput.c_str()));
    }

    void no_errors_more_files() {
        check(3, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_less_files() {
        check(1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_equal_amount_files() {
        check(2, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void one_error_less_files() {
        check(1, 1,
              "int main()\n"
              "{\n"
              "  {int i = *((int*)0);}\n"
              "  return 0;\n"
              "}");
    }

    void one_error_several_files() {
        check(20, 20,
              "int main()\n"
              "{\n"
              "  {int i = *((int*)0);}\n"
              "  return 0;\n"
              "}");
    }

    void clangTidy() {
        // TODO: we currently only invoke it with ImportProject::FileSettings
        if (!useFS)
            return;

#ifdef _WIN32
        constexpr char exe[] = "clang-tidy.exe";
#else
        constexpr char exe[] = "clang-tidy";
#endif

        const std::string file = fprefix() + "_001.cpp";
        check(1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}",
              dinit(CheckOptions,
                    $.quiet = false,
                        $.clangTidy = true,
                        $.executeCommandCalled = true,
                        $.exe = exe,
                        $.args = {"-quiet", "-checks=*,-clang-analyzer-*,-llvm*", file, "--"}));
        ASSERT_EQUALS("Checking " + file + " ...\n", output_str());
    }

// TODO: provide data which actually shows values above 0

    void showtime_top5_file() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_TOP5_FILE));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // for each file: top5 results + overall + empty line
        ASSERT_EQUALS((5 + 1 + 1) * 2, cppcheck::count_all_of(output_s, '\n'));
    }

    void showtime_top5_summary() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_TOP5_SUMMARY));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // once: top5 results + overall + empty line
        ASSERT_EQUALS(5 + 1 + 1, cppcheck::count_all_of(output_s, '\n'));
        // should only report the top5 once
        ASSERT(output_s.find("1 result(s)") == std::string::npos);
        ASSERT(output_s.find("2 result(s)") != std::string::npos);
    }

    void showtime_file() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_FILE));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        ASSERT_EQUALS(2, cppcheck::count_all_of(output_s, "Overall time:"));
    }

    void showtime_summary() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // should only report the actual summary once
        ASSERT(output_s.find("1 result(s)") == std::string::npos);
        ASSERT(output_s.find("2 result(s)") != std::string::npos);
    }

    void showtime_file_total() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = SHOWTIME_MODES::SHOWTIME_FILE_TOTAL));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        ASSERT(output_s.find("Check time: " + fprefix() + "_" + zpad3(1) + ".cpp: ") != std::string::npos);
        ASSERT(output_s.find("Check time: " + fprefix() + "_" + zpad3(2) + ".cpp: ") != std::string::npos);
    }

    void suppress_error_library() {
        SUPPRESS;
        const Settings settingsOld = settings;
        const char xmldata[] = R"(<def format="2"><markup ext=".cpp" reporterrors="false"/></def>)";
        settings = settingsBuilder().libraryxml(xmldata, sizeof(xmldata)).build();
        check(1, 0,
              "int main()\n"
              "{\n"
              "  int i = *((int*)0);\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        settings = settingsOld;
    }

    void unique_errors() {
        SUPPRESS;
        ScopedFile inc_h(fprefix() + ".h",
                         "inline void f()\n"
                         "{\n"
                         "  (void)*((int*)0);\n"
                         "}");
        check(2, 2,
              "#include \"" + inc_h.name() + "\"");
        // these are not actually made unique by the implementation. That needs to be done by the given ErrorLogger
        ASSERT_EQUALS("[" + inc_h.name() + ":3]: (error) Null pointer dereference: (int*)0\n", errout.str());
    }

    // TODO: test whole program analysis
    // TODO: test unique errors
};

class TestSingleExecutorFiles : public TestSingleExecutorBase {
public:
    TestSingleExecutorFiles() : TestSingleExecutorBase("TestSingleExecutorFiles", false) {}
};

class TestSingleExecutorFS : public TestSingleExecutorBase {
public:
    TestSingleExecutorFS() : TestSingleExecutorBase("TestSingleExecutorFS", true) {}
};

REGISTER_TEST(TestSingleExecutorFiles)
REGISTER_TEST(TestSingleExecutorFS)
