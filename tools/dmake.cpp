/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

// Generate Makefile for cppcheck

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../src/filelister.h"

std::string objfile(std::string cppfile)
{
    cppfile.erase(cppfile.rfind("."));
    return cppfile + ".o";
}

void getDeps(const std::string &filename, std::vector<std::string> &depfiles)
{
    std::ifstream f(filename.c_str());
    if (! f.is_open())
        return;
    if (filename.find(".c") == std::string::npos)
        depfiles.push_back(filename);

    std::string path(filename);
    if (path.find("/") != std::string::npos)
        path.erase(1 + path.rfind("/"));

    std::string line;
    while (std::getline(f, line))
    {
        std::string::size_type pos1 = line.find("#include \"");
        if (pos1 == std::string::npos)
            continue;
        pos1 += 10;

        std::string::size_type pos2 = line.find("\"", pos1);
        std::string hfile(path + line.substr(pos1, pos2 - pos1));
        if (hfile.find("/../") != std::string::npos)	// TODO: Ugly fix
            hfile.erase(0, 4 + hfile.find("/../"));
        if (std::find(depfiles.begin(), depfiles.end(), hfile) != depfiles.end())
            continue;
        getDeps(hfile, depfiles);
    }
}

int main()
{
    // Get files..
    std::vector<std::string> srcfiles;
    FileLister::RecursiveAddFiles(srcfiles, "src/", true);
    if (srcfiles.empty())
    {
        std::cout << "No source files found." << std::endl;
        exit(1);
    }
    std::vector<std::string> testfiles;
    FileLister::RecursiveAddFiles(testfiles, "test/", true);

    std::ofstream fout("Makefile");

    // more warnings.. -Wfloat-equal -Wcast-qual -Wsign-conversion -Wlogical-op
    fout << "CXXFLAGS=-Wall -Wextra -pedantic -g\n";
    fout << "COMPILER=g++\n";
    fout << "BIN=${DESTDIR}/usr/bin\n\n";

    fout << "\n###### Object Files\n\n";
    fout << "OBJECTS =     " << objfile(srcfiles[0]);
    for (unsigned int i = 1; i < srcfiles.size(); ++i)
        fout << " \\" << std::endl << std::string(14, ' ') << objfile(srcfiles[i]);
    fout << "\n\n";
    fout << "TESTOBJ =     " << objfile(testfiles[0]);
    for (unsigned int i = 1; i < testfiles.size(); ++i)
        fout << " \\" << std::endl << std::string(14, ' ') << objfile(testfiles[i]);
    for (unsigned int i = 0; i < srcfiles.size(); ++i)
    {
        if (srcfiles[i] != "src/main.cpp")
            fout << " \\" << std::endl << std::string(14, ' ') << objfile(srcfiles[i]);
    }
    fout << "\n\n";


    fout << "\n###### Targets\n\n";
    fout << "cppcheck:\t$(OBJECTS)\n";
    fout << "\t$(COMPILER) $(CXXFLAGS) -o cppcheck $(OBJECTS)\n\n";
    fout << "all:\tcppcheck\ttestrunner\ttools\n\n";
    fout << "testrunner:\t$(TESTOBJ)\n";
    fout << "\t$(COMPILER) $(CXXFLAGS) -o testrunner $(TESTOBJ)\n\n";
    fout << "test:\tall\n";
    fout << "\t./testrunner\n\n";
    fout << "tools:\ttools/errmsg\ttools/dmake\n\n";
    fout << "tools/errmsg:\ttools/errmsg.cpp\n";
    fout << "\t$(COMPILER) $(CXXFLAGS) -o tools/errmsg tools/errmsg.cpp\n\n";
    fout << "tools/dmake:\ttools/dmake.cpp\tsrc/filelister.cpp\tsrc/filelister.h\n";
    fout << "\t$(COMPILER) $(CXXFLAGS) -o tools/dmake tools/dmake.cpp src/filelister.cpp\n\n";
    fout << "clean:\n";
    fout << "\trm -f src/*.o test/*.o testrunner cppcheck tools/dmake tools/errmsg\n\n";
    fout << "install:\tcppcheck\n";
    fout << "\tinstall -d ${BIN}\n";
    fout << "\tinstall cppcheck ${BIN}\n\n";

    fout << "\n###### Build\n\n";

    for (unsigned int i = 0; i < srcfiles.size(); ++i)
    {
        fout << objfile(srcfiles[i]) << ": " << srcfiles[i];
        std::vector<std::string> depfiles;
        getDeps(srcfiles[i], depfiles);
        for (unsigned int dep = 0; dep < depfiles.size(); ++dep)
            fout << " " << depfiles[dep];
        fout << "\n\t$(COMPILER) $(CXXFLAGS) -c -o " << objfile(srcfiles[i]) << " " << srcfiles[i] << "\n\n";
    }

    for (unsigned int i = 0; i < testfiles.size(); ++i)
    {
        fout << objfile(testfiles[i]) << ": " << testfiles[i];
        std::vector<std::string> depfiles;
        getDeps(testfiles[i], depfiles);
        for (unsigned int dep = 0; dep < depfiles.size(); ++dep)
            fout << " " << depfiles[dep];
        fout << "\n\t$(COMPILER) $(CXXFLAGS) -c -o " << objfile(testfiles[i]) << " " << testfiles[i] << "\n\n";
    }

    fout << "src/errorlogger.h:\ttools/errmsg\n";
    fout << "\ttools/errmsg\n";
    fout << "\tmv errorlogger.h src/\n\n";

    return 0;
}


