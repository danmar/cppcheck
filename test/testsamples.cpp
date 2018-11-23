/*
* Cppcheck - A tool for static C/C++ code analysis
* Copyright (C) 2007-2018 Cppcheck team.
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

#include "cppcheckexecutor.h"
#include "errorlogger.h"
#include "cppcheck.h"
#include "filelister.h"
#include "path.h"
#include "pathmatch.h"
#include "redirect.h"
#include "testsuite.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


class TestSamples : public TestFixture {
public:
    TestSamples() : TestFixture("TestSamples") {
    }

private:

    void run() override {
        TEST_CASE(runSamples);
        TEST_CASE(runConsoleCodePageTranslationOnWindows);
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
            if (i->first.find("memleak") != std::string::npos)
                continue;
            CLEAR_REDIRECT_ERROUT;
            char* path = new char[i->first.size() + 1];
            strcpy(path, i->first.c_str());
            const char * const argv[] = {
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
                std::ifstream ifs(expected_filename);
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

    class CppCheckExecutor2 : public CppCheckExecutor {
    public:
        void settings(const Settings &set) {
            setSettings(set);
        }

    };

    void runConsoleCodePageTranslationOnWindows() const {
        REDIRECT;

        std::vector<std::string> msgs = {
            "ASCII",     // first entry should be using only ASCII
            "kääk",
            "Português"
            //      "日本語",
            //      "한국어",
            //      "Русский",
            //      "中文",
        };

        Settings set1;
        Settings setXML;
        setXML.xml = true;
        setXML.xml_version = 2;
        CppCheckExecutor2 exec;
        exec.settings(set1);
        CppCheckExecutor2 execXML;
        execXML.settings(setXML);

        for (std::vector<std::string>::const_iterator i = msgs.begin(); i != msgs.end(); ++i) {
            CLEAR_REDIRECT_OUTPUT;
            CLEAR_REDIRECT_ERROUT;

            exec.reportOut(*i);

            ErrorLogger::ErrorMessage errMessage;
            errMessage.setmsg(*i);

            // no xml option
            exec.reportInfo(errMessage);

#ifdef _WIN32
            // expect changes through code page translation except for the 'ASCII' case
            if (i == msgs.begin()) {
                ASSERT_EQUALS(*i + "\n", GET_REDIRECT_OUTPUT);
                ASSERT_EQUALS(*i + "\n", GET_REDIRECT_ERROUT);
            } else {
                ASSERT(*i + "\n" != GET_REDIRECT_OUTPUT);
                ASSERT(*i + "\n" != GET_REDIRECT_ERROUT);
            }
#else
            // do not expect any code page translation
            ASSERT_EQUALS(*i + "\n", GET_REDIRECT_OUTPUT);
            ASSERT_EQUALS(*i + "\n", GET_REDIRECT_ERROUT);
#endif

            CLEAR_REDIRECT_ERROUT;
            // possible change of msg for xml option
            // with ErrorLogger::ErrorMessage::fixInvalidChars(), plus additional XML formatting
            execXML.reportInfo(errMessage);
            // undo the effects of "ErrorLogger::ErrorMessage::fixInvalidChars()"
            // replacing octal constants with characters
            std::string myErr;
            std::string myErrOrg = GET_REDIRECT_ERROUT;
            std::string::const_iterator from = myErrOrg.begin();
            while (from != myErrOrg.end()) {
                if (*from == '\\') {
                    ++from;
                    unsigned c;
                    // expect three digits
                    std::istringstream es(std::string(from, from + 3));
                    es >> std::oct >> c;
                    ++from;
                    ++from;
                    myErr.push_back(c);
                } else {
                    myErr.push_back(*from);
                }
                ++from;
            }

            ASSERT(std::string::npos != myErr.find(*i));
        }
    }
};

REGISTER_TEST(TestSamples)
