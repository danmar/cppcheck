/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

std::string objfile(std::string cppfile)
{
    cppfile.erase(cppfile.rfind("."));
    return cppfile + ".o";
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
        if (hfile.find("/../") != std::string::npos)	// TODO: Ugly fix
            hfile.erase(0, 4 + hfile.find("/../"));
        getDeps(hfile, depfiles);
    }
}

static void compilefiles(std::ostream &fout, const std::vector<std::string> &files, const std::string &args)
{
    for (unsigned int i = 0; i < files.size(); ++i) {
        fout << objfile(files[i]) << ": " << files[i];
        std::vector<std::string> depfiles;
        getDeps(files[i], depfiles);
        for (unsigned int dep = 0; dep < depfiles.size(); ++dep)
            fout << " " << depfiles[dep];
        fout << "\n\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) " << args << " -c -o " << objfile(files[i]) << " " << files[i] << "\n\n";
    }
}

static void getCppFiles(std::vector<std::string> &files, const std::string &path)
{
    std::map<std::string,long> filesizes;
    FileLister::recursiveAddFiles(files, filesizes, path);
    // only get *.cpp files..
    for (std::vector<std::string>::iterator it = files.begin(); it != files.end();) {
        if (it->find(".cpp") == std::string::npos)
            it = files.erase(it);
        else
            ++it;
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
            fout << "ifndef " << libName << std::endl;
            fout << "    " << libName << " = " << objfile(externalfiles[i]);
            libNames << "EXTOBJ += $(" << libName << ")" << std::endl;
            start = false;
        } else {
            fout << std::string(14, ' ') << objfile(externalfiles[i]);
        }

        if (i+1 >= externalfiles.size() || libName != getLibName(externalfiles[i+1])) {
            // This was the last file for this library
            fout << std::endl << "endif" << std::endl;
            fout << "\n\n";
            start = true;
        } else {
            // There are more files for this library
            fout << " \\" << std::endl;
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
            fout1 << "INCLUDEPATH += ../externals/tinyxml\n";
            fout1 << "HEADERS += $${BASEPATH}check.h \\\n";
            for (unsigned int i = 0; i < libfiles.size(); ++i) {
                std::string fname(libfiles[i].substr(4));
                if (fname.find(".cpp") == std::string::npos)
                    continue;   // shouldn't happen
                fname.erase(fname.find(".cpp"));
                fout1 << std::string(11, ' ') << "$${BASEPATH}" << fname << ".h";
                if (i < libfiles.size() - 1)
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

    // Makefile settings..
    if (release) {
        makeConditionalVariable(fout, "CXXFLAGS", "-O2 -DNDEBUG -Wall");
    } else {
        // TODO: add more compiler warnings.
        // -Wlogical-op      : doesn't work on older GCC
        // -Wconversion      : too many warnings
        // -Wsign-conversion : too many warnings

        // The _GLIBCXX_DEBUG doesn't work in cygwin
        makeConditionalVariable(fout, "CXXFLAGS",
                                "-pedantic "
                                "-Wall "
                                "-Wextra "
                                "-Wabi "
                                "-Wcast-qual "
                                "-Wfloat-equal "
                                "-Winline "
                                "-Wmissing-declarations "
                                "-Wmissing-format-attribute "
                                "-Wno-long-long "
                                "-Woverloaded-virtual "
                                "-Wpacked "
                                "-Wredundant-decls "
                                "-Wshadow "
                                "-Wsign-promo "
//                                "-Wunreachable-code "
//                                "-Wsign-conversion "
//                                "-Wconversion "
                                "-D_GLIBCXX_DEBUG "
                                "-g");
    }

    fout << "ifeq ($(HAVE_RULES),yes)\n"
         << "    CXXFLAGS += -DHAVE_RULES\n"
         << "    ifdef LIBS\n"
         << "        LIBS += -lpcre\n"
         << "    else\n"
         << "        LIBS=-lpcre\n"
         << "    endif\n"
         << "endif\n\n";

    makeConditionalVariable(fout, "CXX", "g++");
    makeConditionalVariable(fout, "PREFIX", "/usr");
    makeConditionalVariable(fout, "INCLUDE_FOR_LIB", "-Ilib");
    makeConditionalVariable(fout, "INCLUDE_FOR_CLI", "-Ilib -Iexternals -Iexternals/tinyxml");
    makeConditionalVariable(fout, "INCLUDE_FOR_TEST", "-Ilib -Icli -Iexternals -Iexternals/tinyxml");

    fout << "BIN=$(DESTDIR)$(PREFIX)/bin\n\n";
    fout << "# For 'make man': sudo apt-get install xsltproc docbook-xsl docbook-xml on Linux\n";
    fout << "DB2MAN=/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/manpages/docbook.xsl\n";
    fout << "XP=xsltproc -''-nonet -''-param man.charmap.use.subset \"0\"\n";
    fout << "MAN_SOURCE=man/cppcheck.1.xml\n\n";

    fout << "\n###### Object Files\n\n";
    fout << "LIBOBJ =      " << objfile(libfiles[0]);
    for (unsigned int i = 1; i < libfiles.size(); ++i)
        fout << " \\" << std::endl << std::string(14, ' ') << objfile(libfiles[i]);
    fout << "\n\n";
    fout << "CLIOBJ =      " << objfile(clifiles[0]);
    for (unsigned int i = 1; i < clifiles.size(); ++i)
        fout << " \\" << std::endl << std::string(14, ' ') << objfile(clifiles[i]);
    fout << "\n\n";
    fout << "TESTOBJ =     " << objfile(testfiles[0]);
    for (unsigned int i = 1; i < testfiles.size(); ++i)
        fout << " \\" << std::endl << std::string(14, ' ') << objfile(testfiles[i]);
    fout << "\n\n";

    makeExtObj(fout, externalfiles);

    fout << "\n###### Targets\n\n";
    fout << "cppcheck: $(LIBOBJ) $(CLIOBJ) $(EXTOBJ)\n";
    fout << "\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o cppcheck $(CLIOBJ) $(LIBOBJ) $(EXTOBJ) $(LIBS)\n\n";
    fout << "all:\tcppcheck testrunner\n\n";
    fout << "testrunner: $(TESTOBJ) $(LIBOBJ) $(EXTOBJ) cli/threadexecutor.o cli/cmdlineparser.o cli/cppcheckexecutor.o cli/filelister.o cli/pathmatch.o\n";
    fout << "\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o testrunner $(TESTOBJ) $(LIBOBJ) cli/threadexecutor.o cli/cppcheckexecutor.o cli/cmdlineparser.o cli/filelister.o cli/pathmatch.o $(EXTOBJ) $(LIBS)\n\n";
    fout << "test:\tall\n";
    fout << "\t./testrunner\n\n";
    fout << "check:\tall\n";
    fout << "\t./testrunner -g -q\n\n";
    fout << "dmake:\ttools/dmake.cpp\n";
    fout << "\t$(CXX) -o dmake tools/dmake.cpp cli/filelister.cpp lib/path.cpp -Ilib\n\n";
    fout << "clean:\n";
#ifdef _WIN32
    fout << "\tdel lib\\*.o\n\tdel cli\\*.o\n\tdel test\\*.o\n\tdel *.exe\n";
#else
    fout << "\trm -f lib/*.o cli/*.o test/*.o externals/tinyxml/*.o testrunner cppcheck cppcheck.1\n\n";
    fout << "man:\tman/cppcheck.1\n\n";
    fout << "man/cppcheck.1:\t$(MAN_SOURCE)\n\n";
    fout << "\t$(XP) $(DB2MAN) $(MAN_SOURCE)\n\n";
    fout << "tags:\n";
    fout << "\tctags -R --exclude=doxyoutput .\n\n";
    fout << "install: cppcheck\n";
    fout << "\tinstall -d ${BIN}\n";
    fout << "\tinstall cppcheck ${BIN}\n\n";
#endif

    fout << "\n###### Build\n\n";

    compilefiles(fout, libfiles, "${INCLUDE_FOR_LIB}");
    compilefiles(fout, clifiles, "${INCLUDE_FOR_CLI}");
    compilefiles(fout, testfiles, "${INCLUDE_FOR_TEST}");

    return 0;
}

