/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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

// Generate Makefile for cppcheck

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "../cli/filelister.h"

static std::string builddir(std::string filename);
static std::string objfile(std::string cppfile);
static void getDeps(const std::string &filename, std::vector<std::string> &depfiles);


std::string builddir(std::string filename)
{
    if (filename.compare(0,4,"lib/") == 0)
        filename = "$(SRCDIR)" + filename.substr(3);
    return filename;
}

std::string objfile(std::string cppfile)
{
    cppfile.erase(cppfile.rfind("."));
    return builddir(cppfile + ".o");
}

void getDeps(const std::string &filename, std::vector<std::string> &depfiles)
{
    // Is the dependency already included?
    if (std::find(depfiles.begin(), depfiles.end(), filename) != depfiles.end())
        return;

    std::ifstream f(filename.c_str());
    if (! f.is_open()) {
        if (filename.compare(0, 4, "cli/") == 0 || filename.compare(0, 5, "test/") == 0)
            getDeps("lib" + filename.substr(filename.find("/")), depfiles);
        return;
    }
    if (filename.find(".c") == std::string::npos)
        depfiles.push_back(filename);

    std::string path(filename);
    if (path.find("/") != std::string::npos)
        path.erase(1 + path.rfind("/"));

    std::string line;
    while (std::getline(f, line)) {
        std::string::size_type pos1 = line.find("#include \"");
        if (pos1 == std::string::npos)
            continue;
        pos1 += 10;

        std::string::size_type pos2 = line.find("\"", pos1);
        std::string hfile(path + line.substr(pos1, pos2 - pos1));
        if (hfile.find("/../") != std::string::npos)    // TODO: Ugly fix
            hfile.erase(0, 4 + hfile.find("/../"));
        getDeps(hfile, depfiles);
    }
}

static void compilefiles(std::ostream &fout, const std::vector<std::string> &files, const std::string &args)
{
    for (unsigned int i = 0; i < files.size(); ++i) {
        fout << objfile(files[i]) << ": " << files[i];
        std::vector<std::string> depfiles;
        depfiles.push_back("lib/cxx11emu.h");
        getDeps(files[i], depfiles);
        for (unsigned int dep = 0; dep < depfiles.size(); ++dep)
            fout << " " << depfiles[dep];
        fout << "\n\t$(CXX) " << args << " $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -std=c++0x -c -o " << objfile(files[i]) << " " << builddir(files[i]) << "\n\n";
    }
}

static void getCppFiles(std::vector<std::string> &files, const std::string &path)
{
    std::map<std::string,size_t> filemap;
    FileLister::recursiveAddFiles(filemap, path);

    // add *.cpp files to the "files" vector..
    for (std::map<std::string,size_t>::const_iterator it = filemap.begin(); it != filemap.end(); ++it) {
        if (it->first.find(".cpp") != std::string::npos)
            files.push_back(it->first);
    }
}


static void makeConditionalVariable(std::ostream &os, const std::string &variable, const std::string &defaultValue)
{
    os << "ifndef " << variable << '\n'
       << "    " << variable << '=' << defaultValue << '\n'
       << "endif\n"
       << "\n";
}

static std::string getLibName(const std::string &path)
{
    // path can be e.g. "externals/foo/foo.cpp" then returned
    // library name is "FOO".
    std::string libName = path.substr(path.find('/')+1);
    libName = libName.substr(0, libName.find('/'));
    std::transform(libName.begin(), libName.end(),libName.begin(), ::toupper);
    return libName;
}

static void makeExtObj(std::ostream &fout, const std::vector<std::string> &externalfiles)
{
    bool start = true;
    std::ostringstream libNames;
    std::string libName;
    for (unsigned int i = 0; i < externalfiles.size(); ++i) {
        if (start) {
            libName = getLibName(externalfiles[i]);
            fout << "ifndef " << libName << "\n";
            fout << "    " << libName << " = " << objfile(externalfiles[i]);
            libNames << "EXTOBJ += $(" << libName << ")\n";
            start = false;
        } else {
            fout << std::string(14, ' ') << objfile(externalfiles[i]);
        }

        if (i+1 >= externalfiles.size() || libName != getLibName(externalfiles[i+1])) {
            // This was the last file for this library
            fout << "\nendif\n\n\n";
            start = true;
        } else {
            // There are more files for this library
            fout << " \\\n";
        }
    }

    fout << libNames.str();
}

int main(int argc, char **argv)
{
    const bool release(argc >= 2 && std::string(argv[1]) == "--release");

    // Get files..
    std::vector<std::string> libfiles;
    getCppFiles(libfiles, "lib/");

    std::vector<std::string> clifiles;
    getCppFiles(clifiles, "cli/");

    std::vector<std::string> testfiles;
    getCppFiles(testfiles, "test/");

    std::vector<std::string> toolsfiles;
    getCppFiles(toolsfiles, "tools/");

    if (libfiles.empty() && clifiles.empty() && testfiles.empty()) {
        std::cerr << "No files found. Are you in the correct directory?" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> externalfiles;
    getCppFiles(externalfiles, "externals/");


    // QMAKE - lib/lib.pri
    {
        std::ofstream fout1("lib/lib.pri");
        if (fout1.is_open()) {
            fout1 << "# no manual edits - this file is autogenerated by dmake\n\n";
            fout1 << "include($$PWD/pcrerules.pri)\n";
            fout1 << "BASEPATH = ../externals/tinyxml/\n";
            fout1 << "include($$PWD/../externals/tinyxml/tinyxml.pri)\n";
            fout1 << "BASEPATH = ../lib/\n";
            fout1 << "INCLUDEPATH += ../externals/tinyxml\n";
            fout1 << "HEADERS += $${BASEPATH}check.h \\\n";
            for (unsigned int i = 0; i < libfiles.size(); ++i) {
                std::string fname(libfiles[i].substr(4));
                if (fname.find(".cpp") == std::string::npos)
                    continue;   // shouldn't happen
                fname.erase(fname.find(".cpp"));
                fout1 << std::string(11, ' ') << "$${BASEPATH}" << fname << ".h";
                if (i + 1 < testfiles.size())
                    fout1 << " \\\n";
            }
            fout1 << "\n\nSOURCES += ";
            for (unsigned int i = 0; i < libfiles.size(); ++i) {
                fout1 << "$${BASEPATH}" << libfiles[i].substr(4);
                if (i < libfiles.size() - 1)
                    fout1 << " \\\n" << std::string(11, ' ');
            }
            fout1 << "\n";
        }
    }

    // QMAKE - test/testfiles.pri
    {
        std::ofstream fout1("test/testfiles.pri");
        if (fout1.is_open()) {
            fout1 << "# no manual edits - this file is autogenerated by dmake\n\n";
            fout1 << "INCLUDEPATH += ../externals/tinyxml\n";
            fout1 << "\n\nSOURCES += ";
            for (unsigned int i = 0; i < testfiles.size(); ++i) {
                const std::string filename(testfiles[i].substr(5));
                // Include only files containing tests in this listing.
                // I.e. filenames beginning with "test".
                if (filename.compare(0, 4, "test") == 0) {
                    fout1 << "$${BASEPATH}/" << filename;
                    if (i + 1 < testfiles.size())
                        fout1 << " \\\n" << std::string(11, ' ');
                }
            }
            fout1 << "\n";
        }
    }

    static const char makefile[] = "Makefile";
    std::ofstream fout(makefile, std::ios_base::trunc);
    if (!fout.is_open()) {
        std::cerr << "An error occurred while trying to open "
                  << makefile
                  << ".\n";
        return EXIT_FAILURE;
    }

    fout << "# This file is generated by tools/dmake, do not edit.\n\n";
    fout << "# To compile with rules, use 'make HAVE_RULES=yes'\n";
    makeConditionalVariable(fout, "HAVE_RULES", "no");

    // compiled patterns..
    fout << "# folder where lib/*.cpp files are located\n";
    makeConditionalVariable(fout, "SRCDIR", "lib");
    fout << "ifeq ($(SRCDIR),build)\n"
         << "    ifdef VERIFY\n"
         << "        matchcompiler_S := $(shell python tools/matchcompiler.py --verify)\n"
         << "    else\n"
         << "        matchcompiler_S := $(shell python tools/matchcompiler.py)\n"
         << "    endif\n"
         << "endif\n\n";

    // explicit cfg dir..
    fout << "ifdef CFGDIR\n"
         << "    CFG=-DCFGDIR=\\\"$(CFGDIR)\\\"\n"
         << "else\n"
         << "    CFG=\n"
         << "endif\n\n";

    // enable backtrac
    fout << "RDYNAMIC=-rdynamic\n";

    // The _GLIBCXX_DEBUG doesn't work in cygwin or other Win32 systems.
    fout << "# Set the CPPCHK_GLIBCXX_DEBUG flag. This flag is not used in release Makefiles.\n"
         << "# The _GLIBCXX_DEBUG define doesn't work in Cygwin or other Win32 systems.\n"
         << "ifndef COMSPEC\n"
         << "    ifdef ComSpec\n"
         << "        #### ComSpec is defined on some WIN32's.\n"
         << "        COMSPEC=$(ComSpec)\n"
         << "    endif # ComSpec\n"
         << "endif # COMSPEC\n"
         << "\n"
         << "ifdef COMSPEC\n"
         << "    #### Maybe Windows\n"
         << "    ifndef CPPCHK_GLIBCXX_DEBUG\n"
         << "        CPPCHK_GLIBCXX_DEBUG=\n"
         << "    endif # !CPPCHK_GLIBCXX_DEBUG\n"
         << "\n"
         << "    ifeq ($(MSYSTEM),MINGW32)\n"
         << "        LDFLAGS=-lshlwapi\n"
         << "    else\n"
         << "        RDYNAMIC=-lshlwapi\n"
         << "    endif\n"
         << "else # !COMSPEC\n"
         << "    uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')\n"
         << "\n"
         << "    ifeq ($(uname_S),Linux)\n"
         << "        ifndef CPPCHK_GLIBCXX_DEBUG\n"
         << "            CPPCHK_GLIBCXX_DEBUG=-D_GLIBCXX_DEBUG\n"
         << "        endif # !CPPCHK_GLIBCXX_DEBUG\n"
         << "    endif # Linux\n"
         << "\n"
         << "    ifeq ($(uname_S),GNU/kFreeBSD)\n"
         << "        ifndef CPPCHK_GLIBCXX_DEBUG\n"
         << "            CPPCHK_GLIBCXX_DEBUG=-D_GLIBCXX_DEBUG\n"
         << "        endif # !CPPCHK_GLIBCXX_DEBUG\n"
         << "    endif # GNU/kFreeBSD\n"
         << "\n"
         << "endif # COMSPEC\n"
         << "\n";

    // tinymxl2 requires __STRICT_ANSI__ to be undefined to compile under CYGWIN.
    fout << "# Set the UNDEF_STRICT_ANSI flag to address compile time warnings\n"
         << "# with tinyxml2 and Cygwin.\n"
         << "ifdef COMSPEC\n"
         << "    uname_S := $(shell uname -s)\n"
         << "\n"
         << "    ifneq (,$(findstring CYGWIN,$(uname_S)))\n"
         << "        UNDEF_STRICT_ANSI=-U__STRICT_ANSI__\n"
         << "    endif # CYGWIN\n"
         << "endif # COMSPEC\n"
         << "\n";

    // Makefile settings..
    if (release) {
        makeConditionalVariable(fout, "CXXFLAGS", "-O2 -include lib/cxx11emu.h  -DNDEBUG -Wall");
    } else {
        // TODO: add more compiler warnings.
        // -Wlogical-op       : doesn't work on older GCC
        // -Wsign-conversion  : too many warnings
        // -Wunreachable-code : some GCC versions report lots of warnings
        makeConditionalVariable(fout, "CXXFLAGS",
                                "-include lib/cxx11emu.h "
                                "-pedantic "
                                "-Wall "
                                "-Wextra "
                                "-Wabi "
                                "-Wcast-qual "
                                "-Wconversion "
                                "-Wfloat-equal "
                                "-Winline "
//                                "-Wlogical-op "
                                "-Wmissing-declarations "
                                "-Wmissing-format-attribute "
                                "-Wno-long-long "
                                "-Woverloaded-virtual "
                                "-Wpacked "
                                "-Wredundant-decls "
                                "-Wshadow "
//                                "-Wsign-conversion "
                                "-Wsign-promo "
                                "-Wno-missing-field-initializers "
                                "-Wno-missing-braces "
//                                "-Wunreachable-code "
                                "-Wno-sign-compare "  // danmar: I don't like this warning, it's very rarelly a bug
                                "$(CPPCHK_GLIBCXX_DEBUG) "
                                "-g");
    }

    fout << "ifeq ($(HAVE_RULES),yes)\n"
         << "    CXXFLAGS += -DHAVE_RULES -DTIXML_USE_STL $(shell pcre-config --cflags)\n"
         << "    ifdef LIBS\n"
         << "        LIBS += $(shell pcre-config --libs)\n"
         << "    else\n"
         << "        LIBS=$(shell pcre-config --libs)\n"
         << "    endif\n"
         << "endif\n\n";

    makeConditionalVariable(fout, "CXX", "g++");
    makeConditionalVariable(fout, "PREFIX", "/usr");
    makeConditionalVariable(fout, "INCLUDE_FOR_LIB", "-Ilib -Iexternals/tinyxml");
    makeConditionalVariable(fout, "INCLUDE_FOR_CLI", "-Ilib -Iexternals/tinyxml");
    makeConditionalVariable(fout, "INCLUDE_FOR_TEST", "-Ilib -Icli -Iexternals/tinyxml");

    fout << "BIN=$(DESTDIR)$(PREFIX)/bin\n\n";
    fout << "# For 'make man': sudo apt-get install xsltproc docbook-xsl docbook-xml on Linux\n";
    fout << "DB2MAN=/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/manpages/docbook.xsl\n";
    fout << "XP=xsltproc -''-nonet -''-param man.charmap.use.subset \"0\"\n";
    fout << "MAN_SOURCE=man/cppcheck.1.xml\n\n";

    fout << "\n###### Object Files\n\n";
    fout << "LIBOBJ =      " << objfile(libfiles[0]);
    for (size_t i = 1; i < libfiles.size(); ++i)
        fout << " \\\n" << std::string(14, ' ') << objfile(libfiles[i]);
    fout << "\n\n";
    fout << "CLIOBJ =      " << objfile(clifiles[0]);
    for (size_t i = 1; i < clifiles.size(); ++i)
        fout << " \\\n" << std::string(14, ' ') << objfile(clifiles[i]);
    fout << "\n\n";
    fout << "TESTOBJ =     " << objfile(testfiles[0]);
    for (size_t i = 1; i < testfiles.size(); ++i)
        fout << " \\\n" << std::string(14, ' ') << objfile(testfiles[i]);
    fout << "\n\n";

    makeExtObj(fout, externalfiles);

    fout << ".PHONY: dmake\n\n";
    fout << "\n###### Targets\n\n";
    fout << "cppcheck: $(LIBOBJ) $(CLIOBJ) $(EXTOBJ)\n";
    fout << "\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++0x -o cppcheck $(CLIOBJ) $(LIBOBJ) $(EXTOBJ) $(LIBS) $(LDFLAGS) $(RDYNAMIC)\n\n";
    fout << "all:\tcppcheck testrunner\n\n";
    fout << "testrunner: $(TESTOBJ) $(LIBOBJ) $(EXTOBJ) cli/threadexecutor.o cli/cmdlineparser.o cli/cppcheckexecutor.o cli/filelister.o cli/pathmatch.o\n";
    fout << "\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++0x -o testrunner $(TESTOBJ) $(LIBOBJ) cli/threadexecutor.o cli/cppcheckexecutor.o cli/cmdlineparser.o cli/filelister.o cli/pathmatch.o $(EXTOBJ) $(LIBS) $(LDFLAGS) $(RDYNAMIC)\n\n";
    fout << "test:\tall\n";
    fout << "\t./testrunner\n\n";
    fout << "check:\tall\n";
    fout << "\t./testrunner -g -q\n\n";
    fout << "dmake:\ttools/dmake.o cli/filelister.o lib/path.o\n";
    fout << "\t$(CXX) $(CXXFLAGS) -std=c++0x -o dmake tools/dmake.o cli/filelister.o lib/path.o -Ilib $(LDFLAGS)\n";
    fout << "\t./dmake\n\n";
    fout << "reduce:\ttools/reduce.o externals/tinyxml/tinyxml2.o $(LIBOBJ)\n";
    fout << "\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++0x -g -o reduce tools/reduce.o -Ilib -Iexternals/tinyxml $(LIBOBJ) $(LIBS) externals/tinyxml/tinyxml2.o $(LDFLAGS) $(RDYNAMIC)\n\n";
    fout << "clean:\n";
    fout << "\trm -f build/*.o lib/*.o cli/*.o test/*.o tools/*.o externals/tinyxml/*.o testrunner reduce dmake cppcheck cppcheck.1\n\n";
    fout << "man:\tman/cppcheck.1\n\n";
    fout << "man/cppcheck.1:\t$(MAN_SOURCE)\n\n";
    fout << "\t$(XP) $(DB2MAN) $(MAN_SOURCE)\n\n";
    fout << "tags:\n";
    fout << "\tctags -R --exclude=doxyoutput .\n\n";
    fout << "install: cppcheck\n";
    fout << "\tinstall -d ${BIN}\n";
    fout << "\tinstall cppcheck ${BIN}\n";
    fout << "\tinstall htmlreport/cppcheck-htmlreport ${BIN}\n";
    fout << "ifdef CFGDIR \n";
    fout << "\tinstall -d ${DESTDIR}${CFGDIR}\n";
    fout << "\tinstall -m 644 cfg/* ${DESTDIR}${CFGDIR}\n";
    fout << "endif\n\n";

    fout << "\n###### Build\n\n";

    compilefiles(fout, libfiles, "${INCLUDE_FOR_LIB}");
    compilefiles(fout, clifiles, "${INCLUDE_FOR_CLI}");
    compilefiles(fout, testfiles, "${INCLUDE_FOR_TEST}");
    compilefiles(fout, externalfiles, "${INCLUDE_FOR_LIB}");
    compilefiles(fout, toolsfiles, "${INCLUDE_FOR_LIB}");

    return 0;
}
