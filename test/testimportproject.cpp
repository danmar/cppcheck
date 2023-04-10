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

#include "importproject.h"
#include "settings.h"
#include "fixture.h"

#include <list>
#include <map>
#include <sstream> // IWYU pragma: keep
#include <string>
#include <vector>

class TestImporter : public ImportProject {
public:
    using ImportProject::importCompileCommands;
    using ImportProject::importCppcheckGuiProject;

    bool sourceFileExists(const std::string & /*file*/) override {
        return true;
    }
};


class TestImportProject : public TestFixture {
public:
    TestImportProject() : TestFixture("TestImportProject") {}

private:

    void run() override {
        TEST_CASE(setDefines);
        TEST_CASE(setIncludePaths1);
        TEST_CASE(setIncludePaths2);
        TEST_CASE(setIncludePaths3); // macro names are case insensitive
        TEST_CASE(importCompileCommands1);
        TEST_CASE(importCompileCommands2); // #8563, #9567
        TEST_CASE(importCompileCommands3); // check with existing trailing / in directory
        TEST_CASE(importCompileCommands4); // only accept certain file types
        TEST_CASE(importCompileCommands5); // Windows/CMake/Ninja generated comile_commands.json
        TEST_CASE(importCompileCommands6); // Windows/CMake/Ninja generated comile_commands.json with spaces
        TEST_CASE(importCompileCommands7); // linux: "/home/danielm/cppcheck 2"
        TEST_CASE(importCompileCommands8); // Windows: "C:\Users\danielm\cppcheck"
        TEST_CASE(importCompileCommands9);
        TEST_CASE(importCompileCommands10); // #10887: include path with space
        TEST_CASE(importCompileCommands11); // include path order
        TEST_CASE(importCompileCommandsArgumentsSection); // Handle arguments section
        TEST_CASE(importCompileCommandsNoCommandSection); // gracefully handles malformed json
        TEST_CASE(importCppcheckGuiProject);
        TEST_CASE(ignorePaths);
    }

    void setDefines() const {
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

    void setIncludePaths1() const {
        ImportProject::FileSettings fs;
        std::list<std::string> in(1, "../include");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        fs.setIncludePaths("abc/def/", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("abc/include/", fs.includePaths.front());
    }

    void setIncludePaths2() const {
        ImportProject::FileSettings fs;
        std::list<std::string> in(1, "$(SolutionDir)other");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        variables["SolutionDir"] = "c:/abc/";
        fs.setIncludePaths("/home/fred", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void setIncludePaths3() const { // macro names are case insensitive
        ImportProject::FileSettings fs;
        std::list<std::string> in(1, "$(SOLUTIONDIR)other");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        variables["SolutionDir"] = "c:/abc/";
        fs.setIncludePaths("/home/fred", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void importCompileCommands1() const {
        const char json[] = R"([{
                                   "directory": "/tmp",
                                   "command": "gcc -DTEST1 -DTEST2=2 -o /tmp/src.o -c /tmp/src.c",
                                   "file": "/tmp/src.c"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("TEST1=1;TEST2=2", importer.fileSettings.cbegin()->defines);
    }

    void importCompileCommands2() const {
        // Absolute file path
#ifdef _WIN32
        const char json[] = R"([{
                                   "directory": "C:/foo",
                                   "command": "gcc -c /bar.c",
                                   "file": "/bar.c"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("C:/bar.c", importer.fileSettings.cbegin()->filename);
#else
        const char json[] = R"([{
                                   "directory": "/foo",
                                   "command": "gcc -c bar.c",
                                   "file": "/bar.c"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/bar.c", importer.fileSettings.cbegin()->filename);
#endif
    }

    void importCompileCommands3() const {
        const char json[] = R"([{
                                    "directory": "/tmp/",
                                    "command": "gcc -c src.c",
                                    "file": "src.c"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.cbegin()->filename);
    }

    void importCompileCommands4() const {
        const char json[] = R"([{
                                    "directory": "/tmp/",
                                    "command": "gcc -c src.mm",
                                    "file": "src.mm"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
    }

    void importCompileCommands5() const {
        const char json[] =
            R"([{
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomylib\\CMakeFiles\\mylib.dir\\src\\foobar\\mylib.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp"
             },
             {
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\myapp\\src -Imyapp -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomyapp\\CMakeFiles\\myapp.dir\\src\\main.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp"
             }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/src/", importer.fileSettings.cbegin()->includePaths.front());
    }

    void importCompileCommands6() const {
        const char json[] =
            R"([{
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src -I\"C:\\Users\\dan\\git\\test-cppcheck\\mylib\\second src\" /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomylib\\CMakeFiles\\mylib.dir\\src\\foobar\\mylib.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp"
            },
            {
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\myapp\\src -Imyapp -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomyapp\\CMakeFiles\\myapp.dir\\src\\main.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp"
             }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/src/", importer.fileSettings.cbegin()->includePaths.front());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/second src/", importer.fileSettings.cbegin()->includePaths.back());
    }


    void importCompileCommands7() const {
        // cmake -DFILESDIR="/some/path" ..
        const char json[] =
            R"([{
                "directory": "/home/danielm/cppcheck 2/b/lib",
                "command": "/usr/bin/c++  -DFILESDIR=\\\"/some/path\\\" -I\"/home/danielm/cppcheck 2/b/lib\" -isystem \"/home/danielm/cppcheck 2/externals\" \"/home/danielm/cppcheck 2/lib/astutils.cpp\"",
                "file": "/home/danielm/cppcheck 2/lib/astutils.cpp"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("FILESDIR=\"/some/path\"", importer.fileSettings.cbegin()->defines);
        ASSERT_EQUALS(1, importer.fileSettings.cbegin()->includePaths.size());
        ASSERT_EQUALS("/home/danielm/cppcheck 2/b/lib/", importer.fileSettings.cbegin()->includePaths.front());
        TODO_ASSERT_EQUALS("/home/danielm/cppcheck 2/externals/",
                           "/home/danielm/cppcheck 2/b/lib/",
                           importer.fileSettings.cbegin()->includePaths.back());
    }

    void importCompileCommands8() const {
        // cmake -DFILESDIR="C:\Program Files\Cppcheck" -G"NMake Makefiles" ..
        const char json[] =
            R"([{
              "directory": "C:/Users/danielm/cppcheck/build/lib",
              "command": "C:\\PROGRA~2\\MICROS~2\\2017\\COMMUN~1\\VC\\Tools\\MSVC\\1412~1.258\\bin\\Hostx64\\x64\\cl.exe  /nologo /TP -DFILESDIR=\"\\\"C:\\Program Files\\Cppcheck\\\"\" -IC:\\Users\\danielm\\cppcheck\\build\\lib -IC:\\Users\\danielm\\cppcheck\\lib -c C:\\Users\\danielm\\cppcheck\\lib\\astutils.cpp",
              "file": "C:/Users/danielm/cppcheck/lib/astutils.cpp"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr)); // Do not crash
    }

    void importCompileCommands9() const {
        // IAR output (https://sourceforge.net/p/cppcheck/discussion/general/thread/608af51e0a/)
        const char json[] =
            R"([{
              "arguments" : [
                 "powershell.exe -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass -File d:\\Projekte\\xyz\\firmware\\app\\xyz-lib\\build.ps1 -IAR -COMPILER_PATH \"c:\\Program Files (x86)\\IAR Systems\\Embedded Workbench 9.0\" -CONTROLLER CC1310F128 -LIB LIB_PERMANENT -COMPILER_DEFINES \"CC1310_HFXO_FREQ=24000000 DEBUG\""
              ],
              "directory" : "d:\\Projekte\\xyz\\firmware\\app",
              "type" : "PRE",
              "file": "1.c"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
    }

    void importCompileCommands10() const { // #10887
        const char json[] =
            R"([{
               "file": "/home/danielm/cppcheck/1/test folder/1.c" ,
               "directory": "",
               "arguments": [
                   "iccavr.exe",
                   "-I",
                   "/home/danielm/cppcheck/test folder"
               ]
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const ImportProject::FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("/home/danielm/cppcheck/test folder/", fs.includePaths.front());
    }

    void importCompileCommands11() const { // include path order
        const char json[] =
            R"([{
               "file": "1.c" ,
               "directory": "/x",
               "arguments": [
                   "cc",
                   "-I",
                   "def",
                   "-I",
                   "abc"
               ]
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const ImportProject::FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("/x/def/", fs.includePaths.front());
        ASSERT_EQUALS("/x/abc/", fs.includePaths.back());
    }

    void importCompileCommandsArgumentsSection() const {
        const char json[] = "[ { \"directory\": \"/tmp/\","
                            "\"arguments\": [\"gcc\", \"-c\", \"src.c\"],"
                            "\"file\": \"src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.cbegin()->filename);
    }

    void importCompileCommandsNoCommandSection() const {
        const char json[] = "[ { \"directory\": \"/tmp/\","
                            "\"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(false, importer.importCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
    }

    void importCppcheckGuiProject() const {
        const char xml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<project version=\"1\">\n"
                           "    <root name=\".\"/>\n"
                           "    <builddir>out1</builddir>\n"
                           "    <analyze-all-vs-configs>true</analyze-all-vs-configs>\n"
                           "    <includedir>\n"
                           "        <dir name=\"lib/\"/>\n"
                           "    </includedir>\n"
                           "    <paths>\n"
                           "        <dir name=\"cli/\"/>\n"
                           "    </paths>\n"
                           "    <exclude>\n"
                           "        <path name=\"gui/temp/\"/>\n"
                           "    </exclude>\n"
                           "</project>\n";
        std::istringstream istr(xml);
        Settings s;
        TestImporter project;
        ASSERT_EQUALS(true, project.importCppcheckGuiProject(istr, &s));
        ASSERT_EQUALS(1, project.guiProject.pathNames.size());
        ASSERT_EQUALS("cli/", project.guiProject.pathNames[0]);
        ASSERT_EQUALS(1, s.includePaths.size());
        ASSERT_EQUALS("lib/", s.includePaths.front());
    }

    void ignorePaths() const {
        ImportProject::FileSettings fs1, fs2;
        fs1.filename = "foo/bar";
        fs2.filename = "qwe/rty";
        TestImporter project;
        project.fileSettings = {fs1, fs2};

        project.ignorePaths({"*foo", "bar*"});
        ASSERT_EQUALS(2, project.fileSettings.size());

        project.ignorePaths({"foo/*"});
        ASSERT_EQUALS(1, project.fileSettings.size());
        ASSERT_EQUALS("qwe/rty", project.fileSettings.front().filename);

        project.ignorePaths({ "*e/r*" });
        ASSERT_EQUALS(0, project.fileSettings.size());
    }
};

REGISTER_TEST(TestImportProject)
