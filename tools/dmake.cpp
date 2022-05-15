/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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
#include <fstream> // IWYU pragma: keep
#include <iostream>
#include <string>
#include <vector>

#include "../cli/filelister.h"
#include "../lib/pathmatch.h"

static std::string builddir(std::string filename)
{
    if (filename.compare(0,4,"lib/") == 0)
        filename = "$(libcppdir)" + filename.substr(3);
    return filename;
}

static std::string objfile(std::string cppfile)
{
    cppfile.erase(cppfile.rfind('.'));
    return builddir(cppfile + ".o");
}

static std::string objfiles(const std::vector<std::string> &files)
{
    std::string allObjfiles;
    for (const std::string &file : files) {
        if (file != files.front())
            allObjfiles += std::string(14, ' ');
        allObjfiles += objfile(file);
        if (file != files.back())
            allObjfiles += " \\\n";
    }
    return allObjfiles;
}

static void getDeps(const std::string &filename, std::vector<std::string> &depfiles)
{
    static const std::vector<std::string> externalfolders{"externals", "externals/picojson", "externals/simplecpp", "externals/tinyxml2" };

    // Is the dependency already included?
    if (std::find(depfiles.begin(), depfiles.end(), filename) != depfiles.end())
        return;

    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        /*
         * Recursively search for includes in other directories.
         * Files are searched according to the following priority:
         * [test, tools] -> cli -> lib -> externals
         */
        if (filename.compare(0, 4, "cli/") == 0)
            getDeps("lib" + filename.substr(filename.find('/')), depfiles);
        else if (filename.compare(0, 5, "test/") == 0)
            getDeps("cli" + filename.substr(filename.find('/')), depfiles);
        else if (filename.compare(0, 6, "tools/") == 0)
            getDeps("cli" + filename.substr(filename.find('/')), depfiles);
        else if (filename.compare(0, 4, "lib/") == 0) {
            for (const std::string & external : externalfolders)
                getDeps(external + filename.substr(filename.find('/')), depfiles);
        }
        return;
    }
    if (filename.find(".c") == std::string::npos)
        depfiles.push_back(filename);

    std::string path(filename);
    if (path.find('/') != std::string::npos)
        path.erase(1 + path.rfind('/'));

    std::string line;
    while (std::getline(f, line)) {
        std::string::size_type pos1 = line.find("#include \"");
        char rightBracket = '\"';
        if (pos1 == std::string::npos) {
            pos1 = line.find("#include <");
            rightBracket = '>';
            if (pos1 == std::string::npos)
                continue;
        }

        pos1 += 10;

        std::string::size_type pos2 = line.find(rightBracket, pos1);
        std::string hfile = path + line.substr(pos1, pos2 - pos1);

        if (hfile.find("/../") != std::string::npos)    // TODO: Ugly fix
            hfile.erase(0, 4 + hfile.find("/../"));
        getDeps(hfile, depfiles);
    }
}

static void compilefiles(std::ostream &fout, const std::vector<std::string> &files, const std::string &args)
{
    for (const std::string &file : files) {
        bool external(file.compare(0,10,"externals/") == 0);
        fout << objfile(file) << ": " << file;
        std::vector<std::string> depfiles;
        getDeps(file, depfiles);
        std::sort(depfiles.begin(), depfiles.end());
        for (const std::string &depfile : depfiles)
            fout << " " << depfile;
        fout << "\n\t$(CXX) " << args << " $(CPPFLAGS) $(CPPFILESDIR) $(CXXFLAGS)" << (external?" -w":"") << " $(UNDEF_STRICT_ANSI) -c -o " << objfile(file) << " " << builddir(file) << "\n\n";
    }
}

static std::string getCppFiles(std::vector<std::string> &files, const std::string &path, bool recursive)
{
    std::map<std::string,size_t> filemap;
    const std::set<std::string> extra;
    const std::vector<std::string> masks;
    const PathMatch matcher(masks);
    std::string err = FileLister::addFiles(filemap, path, extra, recursive, matcher);
    if (!err.empty())
        return err;

    // add *.cpp files to the "files" vector..
    for (const std::pair<const std::string&, size_t> file : filemap) {
        if (file.first.find(".cpp") != std::string::npos)
            files.push_back(file.first);
    }
    return "";
}


static void makeConditionalVariable(std::ostream &os, const std::string &variable, const std::string &defaultValue)
{
    os << "ifndef " << variable << '\n'
       << "    " << variable << '=' << defaultValue << '\n'
       << "endif\n"
       << "\n";
}

int main(int argc, char **argv)
{
    const bool release(argc >= 2 && std::string(argv[1]) == "--release");

    // Get files..
    std::vector<std::string> libfiles;
    std::string err = getCppFiles(libfiles, "lib/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> extfiles;
    err = getCppFiles(extfiles, "externals/", true);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> clifiles;
    err = getCppFiles(clifiles, "cli/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> testfiles;
    err = getCppFiles(testfiles, "test/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> toolsfiles;
    err = getCppFiles(toolsfiles, "tools/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    if (libfiles.empty() && clifiles.empty() && testfiles.empty()) {
        std::cerr << "No files found. Are you in the correct directory?" << std::endl;
        return EXIT_FAILURE;
    }

    // QMAKE - lib/lib.pri
    {
        std::ofstream fout1("lib/lib.pri");
        if (fout1.is_open()) {
            fout1 << "# no manual edits - this file is autogenerated by dmake\n\n";
            fout1 << "include($$PWD/pcrerules.pri)\n";
            fout1 << "include($$PWD/../externals/externals.pri)\n";
            fout1 << "INCLUDEPATH += $$PWD\n";
            fout1 << "HEADERS += ";
            for (const std::string &libfile : libfiles) {
                std::string fname(libfile.substr(4));
                if (fname.find(".cpp") == std::string::npos)
                    continue;   // shouldn't happen
                fname.erase(fname.find(".cpp"));
                fout1 << "$${PWD}/" << fname << ".h";
                if (libfile != libfiles.back())
                    fout1 << " \\\n" << std::string(11, ' ');
            }
            fout1 << "\n\nSOURCES += ";
            for (const std::string &libfile : libfiles) {
                fout1 << "$${PWD}/" << libfile.substr(4);
                if (libfile != libfiles.back())
                    fout1 << " \\\n" << std::string(11, ' ');
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

    // use match compiler..
    fout << "# use match compiler\n";
    fout << "ifeq ($(SRCDIR),build)\n"
         << "    $(warning Usage of SRCDIR to activate match compiler is deprecated. Use MATCHCOMPILER=yes instead.)\n"
         << "    MATCHCOMPILER:=yes\n"
         << "endif\n";
    // TODO: bail out when matchcompiler.py fails (i.e. invalid PYTHON_INTERPRETER specified)
    // TODO: handle "PYTHON_INTERPRETER="
    fout << "ifeq ($(MATCHCOMPILER),yes)\n"
         << "    # Find available Python interpreter\n"
         << "    ifeq ($(PYTHON_INTERPRETER),)\n"
         << "        PYTHON_INTERPRETER := $(shell which python3)\n"
         << "    endif\n"
         << "    ifeq ($(PYTHON_INTERPRETER),)\n"
         << "        PYTHON_INTERPRETER := $(shell which python)\n"
         << "    endif\n"
         << "    ifeq ($(PYTHON_INTERPRETER),)\n"
         << "        $(error Did not find a Python interpreter)\n"
         << "    endif\n"
         << "    ifdef VERIFY\n"
         << "        matchcompiler_S := $(shell $(PYTHON_INTERPRETER) tools/matchcompiler.py --verify)\n"
         << "    else\n"
         << "        matchcompiler_S := $(shell $(PYTHON_INTERPRETER) tools/matchcompiler.py)\n"
         << "    endif\n"
         << "    libcppdir:=build\n"
         << "else\n"
         << "    libcppdir:=lib\n"
         << "endif\n\n";

    // explicit files dir..
    fout << "ifdef FILESDIR\n"
         << "    CPPFILESDIR=-DFILESDIR=\\\"$(FILESDIR)\\\"\n"
         << "else\n"
         << "    CPPFILESDIR=\n"
         << "endif\n\n";

    // enable backtrac
    fout << "RDYNAMIC=-rdynamic\n";

    // The _GLIBCXX_DEBUG doesn't work in cygwin or other Win32 systems.
    fout << "# Set the CPPCHK_GLIBCXX_DEBUG flag. This flag is not used in release Makefiles.\n"
         << "# The _GLIBCXX_DEBUG define doesn't work in Cygwin or other Win32 systems.\n"
         << "ifndef COMSPEC\n"
         << "    ifdef ComSpec\n"
         << "        #### ComSpec is defined on some WIN32's.\n"
         << "        WINNT=1\n"
         << "\n"
         << "        ifneq (,$(findstring /cygdrive/,$(PATH)))\n"
         << "            CYGWIN=1\n"
         << "        endif # CYGWIN\n"
         << "    endif # ComSpec\n"
         << "endif # COMSPEC\n"
         << "\n"
         << "ifdef WINNT\n"
         << "    #### Maybe Windows\n"
         << "    ifndef CPPCHK_GLIBCXX_DEBUG\n"
         << "        CPPCHK_GLIBCXX_DEBUG=\n"
         << "    endif # !CPPCHK_GLIBCXX_DEBUG\n"
         << "\n"
         << "    ifeq ($(MSYSTEM),MINGW32 MINGW64)\n"
         << "        LDFLAGS=-lshlwapi\n"
         << "    else\n"
         << "        RDYNAMIC=-lshlwapi\n"
         << "    endif\n"
         << "else # !WINNT\n"
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
         << "endif # WINNT\n"
         << "\n";

    // tinymxl2 requires __STRICT_ANSI__ to be undefined to compile under CYGWIN.
    fout << "ifdef CYGWIN\n"
         << "    # Set the UNDEF_STRICT_ANSI flag to address compile time warnings\n"
         << "    # with tinyxml2 and Cygwin.\n"
         << "    UNDEF_STRICT_ANSI=-U__STRICT_ANSI__\n"
         << "    \n"
         << "    # Increase stack size for Cygwin builds to avoid segmentation fault in limited recursive tests.\n"
         << "    CXXFLAGS+=-Wl,--stack,8388608\n"
         << "endif # CYGWIN\n"
         << "\n";

    // skip "-D_GLIBCXX_DEBUG" if clang, since it breaks the build
    makeConditionalVariable(fout, "CXX", "g++");
    fout << "ifeq (clang++, $(findstring clang++,$(CXX)))\n"
         << "    CPPCHK_GLIBCXX_DEBUG=\n"
         << "endif\n";

    // Makefile settings..
    if (release) {
        makeConditionalVariable(fout, "CXXFLAGS", "-std=c++0x -O2 -DNDEBUG -Wall -Wno-sign-compare");
    } else {
        // TODO: add more compiler warnings.
        // -Wlogical-op       : doesn't work on older GCC
        // -Wsign-conversion  : too many warnings
        // -Wunreachable-code : some GCC versions report lots of warnings
        makeConditionalVariable(fout, "CXXFLAGS",
                                "-pedantic "
                                "-Wall "
                                "-Wextra "
                                "-Wcast-qual "
//                                "-Wconversion "  // danmar: gives fp. for instance: unsigned int sizeof_pointer = sizeof(void *);
                                "-Wno-deprecated-declarations "
                                "-Wfloat-equal "
//                                "-Wlogical-op "
                                "-Wmissing-declarations "
                                "-Wmissing-format-attribute "
                                "-Wno-long-long "
//                                "-Woverloaded-virtual "  // danmar: we get fp when overloading analyseWholeProgram()
                                "-Wpacked "
                                "-Wredundant-decls "
                                "-Wundef "
                                "-Wno-shadow "
//                                "-Wsign-conversion "
//                                "-Wsign-promo "
                                "-Wno-missing-field-initializers "
                                "-Wno-missing-braces "
//                                "-Wunreachable-code "
                                "-Wno-sign-compare "  // danmar: I don't like this warning, it's very rarely a bug
                                "-Wno-multichar "
                                "$(CPPCHK_GLIBCXX_DEBUG) "
                                "-g");
    }

    fout << "ifeq (g++, $(findstring g++,$(CXX)))\n"
         << "    override CXXFLAGS += -std=c++0x\n"
         << "else ifeq (clang++, $(findstring clang++,$(CXX)))\n"
         << "    override CXXFLAGS += -std=c++0x\n"
         << "else ifeq ($(CXX), c++)\n"
         << "    ifeq ($(shell uname -s), Darwin)\n"
         << "        override CXXFLAGS += -std=c++0x\n"
         << "    endif\n"
         << "endif\n"
         << "\n";

    fout << "ifeq ($(HAVE_RULES),yes)\n"
         << "    override CXXFLAGS += -DHAVE_RULES -DTIXML_USE_STL $(shell pcre-config --cflags)\n"
         << "    ifdef LIBS\n"
         << "        LIBS += $(shell pcre-config --libs)\n"
         << "    else\n"
         << "        LIBS=$(shell pcre-config --libs)\n"
         << "    endif\n"
         << "endif\n\n";

    makeConditionalVariable(fout, "PREFIX", "/usr");
    makeConditionalVariable(fout, "INCLUDE_FOR_LIB", "-Ilib -isystem externals -isystem externals/picojson -isystem externals/simplecpp -isystem externals/tinyxml2");
    makeConditionalVariable(fout, "INCLUDE_FOR_CLI", "-Ilib -isystem externals/simplecpp -isystem externals/tinyxml2");
    makeConditionalVariable(fout, "INCLUDE_FOR_TEST", "-Ilib -Icli -isystem externals/simplecpp -isystem externals/tinyxml2");

    fout << "BIN=$(DESTDIR)$(PREFIX)/bin\n\n";
    fout << "# For 'make man': sudo apt-get install xsltproc docbook-xsl docbook-xml on Linux\n";
    fout << "DB2MAN?=/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/manpages/docbook.xsl\n";
    fout << "XP=xsltproc -''-nonet -''-param man.charmap.use.subset \"0\"\n";
    fout << "MAN_SOURCE=man/cppcheck.1.xml\n\n";

    fout << "\n###### Object Files\n\n";
    fout << "LIBOBJ =      " << objfiles(libfiles) << "\n\n";
    fout << "EXTOBJ =      " << objfiles(extfiles) << "\n\n";
    fout << "CLIOBJ =      " << objfiles(clifiles) << "\n\n";
    fout << "TESTOBJ =     " << objfiles(testfiles) << "\n\n";

    fout << ".PHONY: run-dmake tags\n\n";
    fout << "\n###### Targets\n\n";
    fout << "cppcheck: $(LIBOBJ) $(CLIOBJ) $(EXTOBJ)\n";
    fout << "\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIBS) $(LDFLAGS) $(RDYNAMIC)\n\n";
    fout << "all:\tcppcheck testrunner\n\n";
    fout << "testrunner: $(TESTOBJ) $(LIBOBJ) $(EXTOBJ) cli/threadexecutor.o cli/cmdlineparser.o cli/cppcheckexecutor.o cli/filelister.o\n";
    fout << "\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIBS) $(LDFLAGS) $(RDYNAMIC)\n\n";
    fout << "test:\tall\n";
    fout << "\t./testrunner\n\n";
    fout << "check:\tall\n";
    fout << "\t./testrunner -q\n\n";
    fout << "checkcfg:\tcppcheck validateCFG\n";
    fout << "\t./test/cfg/runtests.sh\n\n";
    fout << "dmake:\ttools/dmake.o cli/filelister.o $(libcppdir)/pathmatch.o $(libcppdir)/path.o $(libcppdir)/utils.o externals/simplecpp/simplecpp.o\n";
    fout << "\t$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)\n\n";
    fout << "run-dmake: dmake\n";
    fout << "\t./dmake\n\n";
    fout << "clean:\n";
    fout << "\trm -f build/*.cpp build/*.o lib/*.o cli/*.o test/*.o tools/*.o externals/*/*.o testrunner dmake cppcheck cppcheck.exe cppcheck.1\n\n";
    fout << "man:\tman/cppcheck.1\n\n";
    fout << "man/cppcheck.1:\t$(MAN_SOURCE)\n\n";
    fout << "\t$(XP) $(DB2MAN) $(MAN_SOURCE)\n\n";
    fout << "tags:\n";
    fout << "\tctags -R --exclude=doxyoutput --exclude=test/cfg --exclude=test/synthetic cli externals gui lib test\n\n";
    fout << "install: cppcheck\n";
    fout << "\tinstall -d ${BIN}\n";
    fout << "\tinstall cppcheck ${BIN}\n";
    fout << "\tinstall htmlreport/cppcheck-htmlreport ${BIN}\n";
    fout << "ifdef FILESDIR\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}/addons\n";
    fout << "\tinstall -m 644 addons/*.py ${DESTDIR}${FILESDIR}/addons\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}/cfg\n";
    fout << "\tinstall -m 644 cfg/*.cfg ${DESTDIR}${FILESDIR}/cfg\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}/platforms\n";
    fout << "\tinstall -m 644 platforms/*.xml ${DESTDIR}${FILESDIR}/platforms\n";
    fout << "else\n";
    fout << "\t$(error FILESDIR must be set!)\n";
    fout << "endif\n";
    fout << "\n";
    fout << "uninstall:\n";
    fout << "\t@if test -d ${BIN}; then \\\n";
    fout << "\t  files=\"cppcheck cppcheck-htmlreport\"; \\\n";
    fout << "\t  echo '(' cd ${BIN} '&&' rm -f $$files ')'; \\\n";
    fout << "\t  ( cd ${BIN} && rm -f $$files ); \\\n";
    fout << "\tfi\n";
    fout << "ifdef FILESDIR \n";
    fout << "\t@if test -d ${DESTDIR}${FILESDIR}; then \\\n";
    fout << "\t  echo rm -rf ${DESTDIR}${FILESDIR}; \\\n";
    fout << "\t  rm -rf ${DESTDIR}${FILESDIR}; \\\n";
    fout << "\tfi\n";
    fout << "endif\n";
    fout << "ifdef CFGDIR \n";
    fout << "\t@if test -d ${DESTDIR}${CFGDIR}; then \\\n";
    fout << "\t  files=\"`cd cfg 2>/dev/null && ls`\"; \\\n";
    fout << "\t  if test -n \"$$files\"; then \\\n";
    fout << "\t    echo '(' cd ${DESTDIR}${CFGDIR} '&&' rm -f $$files ')'; \\\n";
    fout << "\t    ( cd ${DESTDIR}${CFGDIR} && rm -f $$files ); \\\n";
    fout << "\t  fi; \\\n";
    fout << "\tfi\n";
    fout << "endif\n\n";
    fout << "# Validation of library files:\n";
    fout << "ConfigFiles := $(wildcard cfg/*.cfg)\n";
    fout << "ConfigFilesCHECKED := $(patsubst %.cfg,%.checked,$(ConfigFiles))\n";
    fout << ".PHONY: validateCFG\n";
    fout << "%.checked:%.cfg\n";
    fout << "\txmllint --noout --relaxng cfg/cppcheck-cfg.rng $<\n";
    fout << "validateCFG: ${ConfigFilesCHECKED}\n";
    fout << "\txmllint --noout cfg/cppcheck-cfg.rng\n\n";
    fout << "# Validation of platforms files:\n";
    fout << "PlatformFiles := $(wildcard platforms/*.xml)\n";
    fout << "PlatformFilesCHECKED := $(patsubst %.xml,%.checked,$(PlatformFiles))\n";
    fout << ".PHONY: validatePlatforms\n";
    fout << "%.checked:%.xml\n";
    fout << "\txmllint --noout --relaxng platforms/cppcheck-platforms.rng $<\n";
    fout << "validatePlatforms: ${PlatformFilesCHECKED}\n\n";
    fout << "# Validate XML output (to detect regressions)\n";
    fout << "/tmp/errorlist.xml: cppcheck\n";
    fout << "\t./cppcheck --errorlist >$@\n";
    fout << "/tmp/example.xml: cppcheck\n";
    fout << "\t./cppcheck --xml --enable=all --inconclusive --max-configs=1 samples 2>/tmp/example.xml\n";
    fout << "createXMLExamples:/tmp/errorlist.xml /tmp/example.xml\n";
    fout << ".PHONY: validateXML\n";
    fout << "validateXML: createXMLExamples\n";
    fout << "\txmllint --noout cppcheck-errors.rng\n";
    fout << "\txmllint --noout --relaxng cppcheck-errors.rng /tmp/errorlist.xml\n";
    fout << "\txmllint --noout --relaxng cppcheck-errors.rng /tmp/example.xml\n";
    fout << "\ncheckCWEEntries: /tmp/errorlist.xml\n";
    // TODO: handle "PYTHON_INTERPRETER="
    fout << "\t$(eval PYTHON_INTERPRETER := $(if $(PYTHON_INTERPRETER),$(PYTHON_INTERPRETER),$(shell which python3)))\n";
    fout << "\t$(eval PYTHON_INTERPRETER := $(if $(PYTHON_INTERPRETER),$(PYTHON_INTERPRETER),$(shell which python)))\n";
    fout << "\t$(eval PYTHON_INTERPRETER := $(if $(PYTHON_INTERPRETER),$(PYTHON_INTERPRETER),$(error Did not find a Python interpreter)))\n";
    fout << "\t$(PYTHON_INTERPRETER) tools/listErrorsWithoutCWE.py -F /tmp/errorlist.xml\n";
    fout << ".PHONY: validateRules\n";
    fout << "validateRules:\n";
    fout << "\txmllint --noout rules/*.xml\n";

    fout << "\n###### Build\n\n";

    compilefiles(fout, libfiles, "${INCLUDE_FOR_LIB}");
    compilefiles(fout, clifiles, "${INCLUDE_FOR_CLI}");
    compilefiles(fout, testfiles, "${INCLUDE_FOR_TEST}");
    compilefiles(fout, extfiles, "");
    compilefiles(fout, toolsfiles, "${INCLUDE_FOR_LIB}");

    return 0;
}
