/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "cppcheck.h"
#include "mathlib.h"

class CppcheckExecutor : public ErrorLogger {
private:
    CppCheck cppcheck;
    std::string pattern;
    bool foundLine;
    std::time_t stopTime;

public:
    CppcheckExecutor(const char *defines, std::size_t linenr, bool hang)
        : ErrorLogger()
        , cppcheck(*this,false)
        , foundLine(false)
        , stopTime(0) {

        if (!hang)
            pattern = ":" + MathLib::longToString(linenr) + "]";

        if (defines)
            cppcheck.settings().userDefines = defines;
        cppcheck.settings().addEnabled("all");
        cppcheck.settings().inconclusive = true;
        cppcheck.settings()._force = true;
    }

    bool run(const char filename[], unsigned int maxtime) {
        foundLine = false;
        stopTime = std::time(0) + maxtime;
        cppcheck.check(filename);
        return foundLine;
    }

    void reportOut(const std::string &/*outmsg*/) { }
    void reportErr(const ErrorLogger::ErrorMessage &msg) {
        if (!pattern.empty() && msg.toString(false).find(pattern) != std::string::npos) {
            foundLine = true;
            cppcheck.terminate();
        }
    }
    void reportProgress(const std::string &filename, const char stage[], const std::size_t value) {
        if (std::time(0) > stopTime) {
            if (pattern.empty())
                foundLine = true;
            else
                std::cerr << "timeout. You might want to use a longer --maxtime timeout" << std::endl;
            std::cout << "terminate" << std::endl;
            cppcheck.terminate();
        }
    }
};

struct ReduceSettings {
    const char *filename;
    std::size_t linenr;
    bool hang;
    unsigned int maxtime;
    const char *defines;
};

static bool test(const ReduceSettings &settings, const std::vector<std::string> &filedata, const std::size_t line1, const std::size_t line2)
{
    std::string path(settings.filename);
    if (path.find_first_of("\\/") != std::string::npos)
        path = path.erase(1 + path.find_last_of("\\/"));
    else
        path.clear();

    const std::string tempfilename(path + "__temp__" + std::strrchr(settings.filename,'.'));
    std::ofstream fout(tempfilename.c_str());
    for (std::size_t i = 0; i < filedata.size(); i++)
        fout << ((i>=line1 && i<=line2) ? "" : filedata[i]) << std::endl;
    fout.close();

    CppcheckExecutor cppcheck(settings.defines, settings.linenr, settings.hang);
    return cppcheck.run(tempfilename.c_str(), settings.maxtime);
}

static bool test(const ReduceSettings &settings, const std::vector<std::string> &filedata, const std::size_t line)
{
    return test(settings, filedata, line, line);
}

static void printstr(const std::vector<std::string> &filedata, int i1, int i2)
{
    std::cout << filedata.size();
    for (int i = i1; i < i2; ++i)
        std::cout << i << ":" << filedata[i] << std::endl;
}

static std::vector<std::string> readfile(const std::string &filename)
{
    std::vector<std::string> filedata;
    std::ifstream fin(filename.c_str());
    std::string line;
    bool blockComment = false;
    while (std::getline(fin,line)) {
        // replace various space characters with space
        for (std::string::size_type pos = 0; pos < line.size(); ++pos) {
            if (line[pos] & 0x80 || std::isspace(line[pos]))
                line[pos] = ' ';
        }

        // remove block comments TODO: Handle /* inside strings
        if (blockComment) {
            if (line.find("*/") == std::string::npos)
                line.clear();
            else {
                if (line.find("*/") + 2U == line.size())
                    line.clear();
                else
                    line = line.substr(line.find("*/") + 2U);
                blockComment = false;
            }
        }
        while (!blockComment && line.find("/*") != std::string::npos) {
            std::string::size_type pos = line.find("/*");
            if (line.find("*/",pos) == std::string::npos) {
                blockComment = true;
                if (pos==0)
                    line.clear();
                else
                    line = line.substr(0,pos);
            } else {
                blockComment = false;
                line = line.erase(pos, 2U + line.find("*/", pos) - pos);
            }
        }

        // Remove // comments
        if (line.find("//") != std::string::npos)
            line = line.substr(0, line.find("//"));

        // empty line
        if (line.find_first_not_of(" ") == std::string::npos)
            line.clear();
        else {
            const std::string::size_type pos = line.find_first_not_of(" ");

            // remove spaces before leading #
            if (line[pos]=='#')
                line = line.substr(pos);
        }

        // remove trailing spaces
        while (!line.empty() && std::isspace(line[line.size()-1U]))
            line = line.substr(0, line.size() - 1U);

        filedata.push_back(line);
    }
    return filedata;
}

static bool removeMacrosInGlobalScope(const ReduceSettings &settings, std::vector<std::string> &filedata)
{
    bool changed = false;

    // Remove macros in global scope..
    for (std::size_t i = 0; i < filedata.size(); i++) {
        const std::string line = (i==0U&&filedata.empty()) ? std::string(";") : filedata[i];
        if (line.empty())
            continue;

        const char startChar = line[0];
        const char endChar   = line[line.size() - 1U];

        bool decl = bool(!std::isspace(startChar) && (endChar=='}' || endChar==';'));
        while (decl) {
            decl = false; // might be set to true below
            std::size_t pos = i + 1U;
            while (pos < filedata.size() && filedata[pos].empty())
                ++pos;
            if (pos >= filedata.size())
                break;
            const std::string &s = filedata[pos];  // possible macro : make sure it matches '[A-Z0-9_]+\(.*\)'
            std::string::size_type si = 0;
            while (si < s.size() && ((s[si]>='A' && s[si]<='Z') || (i>0 && s[si]>='0' && s[si]<='9') || s[si]=='_'))
                si++;
            while (si < s.size() && std::isspace(s[si]))
                si++;
            if (si == 0U || si >= s.size() || s[si] != '(')
                break;
            si++;
            unsigned int parlevel = 1;
            while (si < s.size() && parlevel >= 1U) {
                if (s[si] == '(')
                    ++parlevel;
                else if (s[si] == ')')
                    --parlevel;
                si++;
            }
            if (!(parlevel == 0U && si == s.size()))
                break;
            if (test(settings, filedata, pos)) {
                decl = true;
                filedata[pos].clear();
                std::cout << "removed declaration at line " << pos << std::endl;
                changed = true;
            } else {
                std::cout << "kept declaration at line " << pos << std::endl;
            }
        }
    }

    return changed;
}

static bool removeBlocksOfCode(const ReduceSettings &settings, std::vector<std::string> &filedata)
{
    bool changed = false;

    // Remove blocks of code..
    for (std::size_t i = 0; i < filedata.size(); i++) {
        const std::string line = (i==0U&&filedata.empty()) ? std::string(";") : filedata[i];
        if (line.empty())
            continue;

        const char startChar = line[0];
        const char endChar   = line[line.size() - 1U];

        // some kind of single line declaration
        if (std::isalpha(startChar) && endChar==';') {
            if (test(settings, filedata, i)) {
                filedata[i].clear();
                std::cout << "removed declaration at line " << i << std::endl;
                changed = true;
            } else {
                std::cout << "kept declaration at line " << i << std::endl;
            }
        }

        // remove a function body below a '}'
        bool decl = bool(!std::isspace(startChar) && (endChar=='}' || endChar==';'));
        while (decl) {
            decl = false; // might be set to true below
            std::size_t pos = ++i;
            while (pos < filedata.size() && filedata[pos].empty())
                ++pos;
            if ((pos+2U < filedata.size()) && (std::isalpha(filedata[pos].at(0)))) {

                // does code block start with "{"
                std::size_t pos2 = pos;

                // struct X { ..
                if (filedata[pos].find_first_of("();}") == std::string::npos && filedata[pos].at(filedata[pos].size()-1U) == '{') {

                }

                // function declaration ..
                else {
                    if (filedata[pos].find("(") != std::string::npos && filedata[pos].find(")")==std::string::npos) {
                        ++pos2;
                        while (pos2+2U < filedata.size() && !filedata[pos2].empty() && filedata[pos2].find_first_of("(){}") == std::string::npos)
                            ++pos2;
                        if (filedata[pos2].find_first_of("({}")!=std::string::npos || filedata[pos2].find(")") == std::string::npos)
                            break;
                    }
                    pos2++;
                    if (pos2 < filedata.size() && !filedata[pos2].empty() && filedata[pos2].at(filedata[pos2].find_first_not_of(" ")) == ':') {
                        pos2++;
                        while (pos2 < filedata.size() && !filedata[pos2].empty() && filedata[pos2].at(filedata[pos2].find_first_not_of(" ")) == ',')
                            pos2++;
                    }
                    if (pos2+2U >= filedata.size() || filedata[pos2] != "{")
                        break;
                }
                pos2++;

                // find end of block..
                int level = 0;
                while ((pos2 < filedata.size()) && (filedata[pos2].empty() || std::isspace(filedata[pos2].at(0)) || (std::isalpha(filedata[pos2].at(0)) && filedata[pos2].at(filedata[pos2].size()-1U) == ':') || filedata[pos2].compare(0,3,"#if")==0 || filedata[pos2].compare(0,3,"#el")==0 || filedata[pos2]=="#endif")) {
                    if (filedata[pos2].compare(0,3,"#if") == 0)
                        ++level;
                    else if (filedata[pos2] == "#endif")
                        --level;
                    ++pos2;
                }
                if (level != 0)
                    break;

                // does block of code end with a '}'
                if ((pos2 < filedata.size()) && (filedata[pos2] == "}" || filedata[pos2] == "};")) {
                    if (test(settings, filedata, pos, pos2)) {
                        for (i = pos; i <= pos2; i++)
                            filedata[i].clear();
                        std::cout << "removed block of code at lines " << pos << "-" << pos2 << std::endl;
                        decl = true;
                        changed = true;
                    } else {
                        std::cout << "kept block of code at lines " << pos << "-" << pos2 << std::endl;
                    }
                }
            }
        }
    }
    return changed;
}

static bool removeClassAndStructMembers(const ReduceSettings &settings, std::vector<std::string> &filedata)
{
    bool changed = false;

    // remove class and struct members
    for (std::size_t i = 0; i + 2U < filedata.size(); i++) {
        if ((filedata[i].compare(0,6,"class ")==0 || filedata[i].compare(0,7,"struct ")==0) && filedata[i].find(";")==std::string::npos && filedata[i+1]=="{") {
            bool decl = true;
            for (std::size_t pos = i+2U; pos < filedata.size(); pos++) {
                const std::string line = filedata[pos];
                if (line.empty())
                    continue;

                // count { and }
                unsigned int c1=0, c2=0;
                for (std::string::size_type c = 0; c < line.size(); c++) {
                    if (line[c] == '{')
                        ++c1;
                    else if (line[c] == '}')
                        ++c2;
                }
                if (c2>0 && (c1!=1 || c2!=1))
                    break;

                const char endChar = line[line.size() - 1U];

                if (decl && (endChar == ';' || (c1==1 && c2==1 && endChar=='}'))) {
                    if (test(settings, filedata, pos)) {
                        std::cout << "removed struct/class declaration at line " << pos << std::endl;
                        filedata[pos].clear();
                        changed = true;
                    } else {
                        std::cout << "kept struct/class declaration at line " << pos << std::endl;
                    }
                }

                if (line[0] != '#') {
                    if (decl && std::isalpha(line[0]) && endChar == ':') {
                        decl = true;
                        for (std::string::size_type linepos = 0U; linepos+1U < line.size(); ++linepos)
                            decl &= (std::isspace(line[linepos]) || std::isalpha(line[linepos]));
                    } else
                        decl = bool(endChar == ';' || endChar == '}');
                }
            }
        }
    }

    return changed;
}

static bool removeIfEndIf(const ReduceSettings &settings, std::vector<std::string> &filedata)
{
    bool changed = false;

    // #if - #endif
    for (std::size_t i = 0; i < filedata.size(); ++i) {
        while (filedata[i].compare(0,3,"#if") == 0) {
            std::size_t pos2 = i + 1;
            while (pos2 < filedata.size() && filedata[pos2].empty())
                ++pos2;
            if (pos2 < filedata.size() && filedata[pos2] == "#endif") {
                if (test(settings, filedata, i, pos2)) {
                    std::cout << "Removed #if - #endif block at lines " << i << "-" << pos2 << std::endl;
                    filedata[i].clear();
                    filedata[pos2].clear();
                    i = 0;
                    changed = true;
                } else {
                    std::cout << "Kept #if - #endif block at lines " << i << "-" << pos2 << std::endl;
                    break;
                }
            } else {
                break;
            }
        }
    }

    // #ifndef UNUSED_ID
    for (std::size_t i = 0; i < filedata.size(); ++i) {
        if (filedata[i].compare(0,8,"#ifndef ") == 0) {
            bool erase = true;
            bool def = false;
            const std::string id(filedata[i].substr(8));
            for (std::size_t i2 = 0; i2 < filedata.size(); i2++) {
                if (i2 == i)
                    continue;
                if (filedata[i2].find(id) != std::string::npos) {
                    if (!def && filedata[i2].compare(0,8,"#define ")==0)
                        def = true;
                    else
                        erase = false;
                }
            }
            if (erase) {
                unsigned int level = 0;
                for (std::size_t i2 = i + 1U; i2 < filedata.size(); i2++) {
                    if (filedata[i2].compare(0,3,"#if")==0)
                        ++level;
                    else if (filedata[i2] == "#else")
                        break;
                    else if (filedata[i2] == "#endif") {
                        if (level > 0)
                            --level;
                        else {
                            std::vector<std::string> filedata2(filedata);
                            filedata2[i].clear();
                            filedata2[i2].clear();

                            if (test(settings, filedata2, i)) {
                                std::cout << "Removed #ifndef at line " << i << std::endl;
                                filedata.swap(filedata2);
                                changed = true;
                            } else {
                                std::cout << "Kept #ifndef at line " << i << std::endl;
                                break;
                            }

                            break;
                        }
                    }
                }
            }
        }
    }

    return changed;
}

static bool removeUnusedDefines(const ReduceSettings &settings, std::vector<std::string> &filedata)
{
    bool changed = false;

    for (std::size_t i = 0; i < filedata.size(); ++i) {
        if (filedata[i].compare(0,8,"#define ")==0 && filedata[i].find("\\")==std::string::npos) {
            // Try to remove macro..

            if (test(settings, filedata, i)) {
                std::cout << "Removed #define at line " << i << std::endl;
                filedata[i].clear();
                changed = true;
            } else {
                std::cout << "Kept #define at line " << i << std::endl;
            }
        }
    }

    return changed;
}

static bool removeSingleLines(const ReduceSettings &settings, std::vector<std::string> &filedata)
{
    bool changed = false;

    bool decl = true;
    for (std::size_t i = 0; i < filedata.size(); ++i) {
        const std::string line = filedata[i];
        if (line.empty())
            continue;

        const char endChar = line[line.size() - 1U];
        if (decl && endChar == ';') {
            if (test(settings, filedata, i)) {
                std::cout << "Removed statement at line " << i << std::endl;
                filedata[i].clear();
            } else {
                std::cout << "Kept statement at line " << i << std::endl;
            }
        } else {
            decl = bool (endChar == ';' || endChar == '{' || endChar == '}');
        }
    }

    return changed;
}

// Try to remove stuff from statements
static bool cleanupStatements(const ReduceSettings &settings, std::vector<std::string> &filedata)
{
    bool changed = false;

    for (std::size_t i = 0; i < filedata.size(); ++i) {
        std::string line = filedata[i];
        if (line.empty())
            continue;

        for (std::string::size_type pos = 0U; pos < line.size(); ++pos) {

            // function parameter..
            if (std::strchr("(,", line[pos])) {
                const std::string::size_type pos1 = (line[pos] == ',') ? pos : (pos + 1U);
                std::string::size_type pos2 = pos + 1;
                while (pos2 < line.size() && std::isspace(line[pos2]))
                    ++pos2;
                while (pos2 < line.size() && (std::isalnum(line[pos2]) || line[pos2]=='_'))
                    ++pos2;
                while (pos2 < line.size() && std::isspace(line[pos2]))
                    ++pos2;
                if (pos2 >= pos+2U && pos2<line.size() && std::strchr(",)", line[pos2])) {
                    const std::string backup(filedata[i]);
                    filedata[i] = line.substr(0,pos) + line.substr(pos2);
                    if (test(settings, filedata, ~0U)) {
                        std::cout << "Removed function parameter at line " << i << ", column " << pos1 << std::endl;
                        line = filedata[i];
                        changed = true;
                        continue;
                    } else {
                        std::cout << "Kept function parameter at line " << i << ", column " << pos1 << std::endl;
                        filedata[i] = backup;
                    }
                }
            }

            // cast
            if (line[pos] == '=' || line[pos] == '(') {
                const std::string::size_type pos1 = pos + 1;
                std::string::size_type pos2 = pos + 1;
                while (pos2 < line.size() && std::isspace(line[pos2]))
                    ++pos2;
                if (pos2>=line.size() || line[pos2]!='(')
                    continue;
                pos2++;
                while (pos2 < line.size() && std::isalpha(line[pos2]))
                    ++pos2;
                while (pos2 < line.size() && std::isspace(line[pos2]))
                    ++pos2;
                if (pos2>=line.size() || line[pos2]!='*')
                    continue;
                while (pos2 < line.size() && line[pos2]=='*')
                    ++pos2;
                if (pos2<line.size() && line[pos2]==')') {
                    pos2++;
                    const std::string backup(filedata[i]);
                    filedata[i] = line.substr(0,pos1) + line.substr(pos2);
                    if (test(settings, filedata, ~0U)) {
                        std::cout << "Removed cast at line " << i << ", column " << pos1 << std::endl;
                        line = filedata[i];
                        changed = true;
                    } else {
                        std::cout << "Kept cast at line " << i << ", column " << pos1 << std::endl;
                        filedata[i] = backup;
                    }
                }
            }
        }
    }

    return changed;
}


int main(int argc, char *argv[])
{
    std::cout << "cppcheck tool that reduce code for a hang / false positive" << std::endl;

    bool print = false;
    struct ReduceSettings settings = {0};
    settings.maxtime = 300;  // default timeout = 5 minutes

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--stdout") == 0)
            print = true;
        else if (strcmp(argv[i], "--hang") == 0) {
            settings.hang = true;
        } else if (strncmp(argv[i], "--maxtime=", 10) == 0)
            settings.maxtime = std::atoi(argv[i] + 10);
        else if (strncmp(argv[i],"--cfg=",6)==0)
            settings.defines = argv[i] + 6;
        else if (settings.filename==NULL && strchr(argv[i],'.'))
            settings.filename = argv[i];
        else if (settings.linenr == 0U && MathLib::isInt(argv[i]))
            settings.linenr = std::atoi(argv[i]);
        else {
            std::cerr << "invalid option " << argv[i] << std::endl;
            return EXIT_FAILURE;
        }
    }

    if ((!settings.hang && settings.linenr == 0U) || settings.filename == NULL) {
        std::cerr << "Syntax:" << std::endl
                  << argv[0] << " [--stdout] [--cfg=X] [--hang] [--maxtime=60] filename [linenr]" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "make sure " << (settings.hang ? "hang" : "false positive") << " can be reproduced" << std::endl;

    // Execute Cppcheck on the file..
    {
        CppcheckExecutor cppcheck(settings.defines, settings.linenr, settings.hang);
        if (!cppcheck.run(settings.filename, settings.maxtime)) {
            std::cerr << "Can't reproduce false positive at line " << settings.linenr << std::endl;
            return EXIT_FAILURE;
        }
    }

    // Read file..
    std::vector<std::string> filedata(readfile(settings.filename));

    // Write resulting code..
    if (!test(settings, filedata, ~0)) {
        std::cerr << "Cleanup failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Remove includes..
    std::set<std::string> headers;
    for (std::size_t i = 0; i < filedata.size(); i++) {
        if (filedata[i].compare(0,8,"#include")==0) {
            if (test(settings, filedata, i)) {
                std::cout << "removed #include : " << filedata[i] << std::endl;
                filedata[i].clear();
            } else {
                std::string header = filedata[i];
                header = header.substr(1U + header.find_first_of("\"<"));
                header = header.erase(header.find_last_of("\">"));
                if (headers.find(header) != headers.end()) {
                    std::cerr << "Failed to reduce headers" << std::endl;
                    return EXIT_FAILURE;
                }
                headers.insert(header);
                std::string path(settings.filename);
                if (path.find_first_of("\\/") != std::string::npos)
                    path = path.erase(1 + path.find_last_of("\\/"));
                else
                    path.clear();
                std::cout << "expand #include : " << (path+header) << std::endl;
                std::vector<std::string> data(readfile(path+header));
                if (!data.empty()) {
                    filedata[i].clear();
                    filedata.insert(filedata.begin()+i, data.begin(), data.end());
                    settings.linenr += data.size();
                }
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        changed |= removeMacrosInGlobalScope(settings,filedata);
        changed |= removeBlocksOfCode(settings,filedata);
        changed |= removeClassAndStructMembers(settings,filedata);
        changed |= removeIfEndIf(settings,filedata);
        changed |= removeUnusedDefines(settings,filedata);

        if (settings.hang) {
            changed |= removeSingleLines(settings,filedata);
            changed |= cleanupStatements(settings,filedata);
        }
    }

    // Write resulting code..
    {
        const std::string outfilename(std::string("__out__") + std::strrchr(settings.filename,'.'));
        std::ofstream fout;
        if (!print)
            fout.open(outfilename.c_str());
        std::ostream &os = print ? std::cout : fout;
        for (std::size_t i = 0; i < filedata.size(); i++) {
            if (!filedata[i].empty())
                os << filedata[i] << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
