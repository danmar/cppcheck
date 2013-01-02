
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>

#include "cppcheck.h"
#include "mathlib.h"

class CppcheckExecutor : public ErrorLogger {
private:
    CppCheck cppcheck;
    std::string pattern;
    bool foundLine;

public:
    CppcheckExecutor(std::size_t linenr)
        : ErrorLogger()
        , cppcheck(*this,false)
        , pattern(":" + MathLib::longToString(linenr) + "]")
        , foundLine(false) {
        cppcheck.settings().addEnabled("all");
        cppcheck.settings().inconclusive = true;
        cppcheck.settings()._force = true;
    }

    bool run(const char filename[]) {
        foundLine = false;
        cppcheck.check(filename);
        return foundLine;
    }

    void reportOut(const std::string &/*outmsg*/) { }
    void reportErr(const ErrorLogger::ErrorMessage &msg) {
        if (msg.toString(false).find(pattern) != std::string::npos) {
            foundLine = true;
            cppcheck.terminate();
        }
    }
    void reportProgress(const std::string & /*filename*/, const char /*stage*/[], const unsigned int /*value*/) { }
};


static bool test(const char *filename, std::size_t linenr, const std::vector<std::string> &filedata, const std::size_t line1, const std::size_t line2)
{
    std::string path(filename);
    if (path.find_first_of("\\/") != std::string::npos)
        path = path.erase(1 + path.find_last_of("\\/"));
    else
        path.clear();

    const std::string tempfilename(path + "__temp__" + std::strrchr(filename,'.'));
    std::ofstream fout(tempfilename.c_str());
    for (std::size_t i = 0; i < filedata.size(); i++)
        fout << ((i>=line1 && i<=line2) ? "" : filedata[i]) << std::endl;
    fout.close();

    CppcheckExecutor cppcheck(linenr);
    return cppcheck.run(tempfilename.c_str());
}

static bool test(const char *filename, std::size_t linenr, const std::vector<std::string> &filedata, const std::size_t line)
{
    return test(filename, linenr, filedata, line, line);
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

static bool removeMacrosInGlobalScope(std::vector<std::string> &filedata, const char filename[], const std::size_t linenr)
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
            if (test(filename, linenr, filedata, pos)) {
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

static bool removeBlocksOfCode(std::vector<std::string> &filedata, const char filename[], const std::size_t linenr)
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
            if (test(filename, linenr, filedata, i)) {
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
                if (filedata[pos].find("(") != std::string::npos && filedata[pos].find(")")==std::string::npos) {
                    ++pos2;
                    while (pos2+2U < filedata.size() && !filedata[pos2].empty() && filedata[pos2].find_first_of("(){}") == std::string::npos)
                        ++pos2;
                    if (filedata[pos2].find_first_of("({}")!=std::string::npos || filedata[pos2].find(")") == std::string::npos)
                        break;
                }
                if (pos2+2U >= filedata.size() || filedata[pos2+1U] != "{")
                    break;
                pos2 += 2;

                // find end of block..
                int level = 0;
                while ((pos2 < filedata.size()) && (filedata[pos2].empty() || std::isspace(filedata[pos2].at(0)) || (std::isalpha(filedata[pos2].at(0)) && filedata[pos2].at(filedata[pos2].size()-1U) == ':') || filedata[pos2].compare(0,3,"#if")==0 || filedata[pos2]=="#else" || filedata[pos2]=="#endif")) {
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
                    if (test(filename, linenr, filedata, pos, pos2)) {
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

static bool removeClassAndStructMembers(std::vector<std::string> &filedata, const char filename[], const std::size_t linenr)
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
                    if (test(filename, linenr, filedata, pos)) {
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

static bool removeIfEndIf(std::vector<std::string> &filedata, const char filename[], const std::size_t linenr)
{
    bool changed = false;

    // #if - #endif
    for (std::size_t i = 0; i < filedata.size(); ++i) {
        while (filedata[i].compare(0,3,"#if") == 0) {
            std::size_t pos2 = i + 1;
            while (pos2 < filedata.size() && filedata[pos2].empty())
                ++pos2;
            if (pos2 < filedata.size() && filedata[pos2] == "#endif") {
                if (test(filename, linenr, filedata, i, pos2)) {
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
    return changed;
}

int main(int argc, char *argv[])
{
    std::cout << "cppcheck tool that reduce code for a false positive" << std::endl;

    bool stdout = false;
    const char *filename = NULL;
    std::size_t linenr = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--stdout") == 0)
            stdout = true;
        else if (filename==NULL && strchr(argv[i],'.'))
            filename = argv[i];
        else if (linenr == 0U && MathLib::isInt(argv[i]))
            linenr = std::atoi(argv[i]);
        else {
            std::cerr << "invalid option " << argv[i] << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (linenr == 0U || filename == NULL) {
        std::cerr << "Syntax:" << std::endl
                  << argv[0] << " [--stdout] filename linenr" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "make sure false positive can be reproduced" << std::endl;

    // Execute Cppcheck on the file..
    {
        CppcheckExecutor cppcheck(linenr);
        if (!cppcheck.run(filename)) {
            std::cerr << "Can't reproduce false positive at line " << linenr << std::endl;
            return EXIT_FAILURE;
        }
    }

    // Read file..
    std::vector<std::string> filedata(readfile(filename));

    // Write resulting code..
    if (!test(filename, linenr, filedata, ~0)) {
        std::cerr << "Cleanup failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Remove includes..
    std::set<std::string> headers;
    for (std::size_t i = 0; i < filedata.size(); i++) {
        if (filedata[i].compare(0,8,"#include")==0) {
            if (test(filename, linenr, filedata, i)) {
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
                std::string path(filename);
                if (path.find_first_of("\\/") != std::string::npos)
                    path = path.erase(1 + path.find_last_of("\\/"));
                else
                    path.clear();
                std::cout << "expand #include : " << (path+header) << std::endl;
                std::vector<std::string> data(readfile(path+header));
                if (!data.empty()) {
                    filedata[i].clear();
                    filedata.insert(filedata.begin()+i, data.begin(), data.end());
                    linenr += data.size();
                }
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        changed |= removeMacrosInGlobalScope(filedata, filename, linenr);
        changed |= removeBlocksOfCode(filedata, filename, linenr);
        changed |= removeClassAndStructMembers(filedata, filename, linenr);
        changed |= removeIfEndIf(filedata, filename, linenr);
    }

    // Write resulting code..
    {
        const std::string outfilename(std::string("__out__") + std::strrchr(filename,'.'));
        std::ofstream fout;
        if (!stdout)
            fout.open(outfilename.c_str());
        std::ostream &os = stdout ? std::cout : fout;
        for (std::size_t i = 0; i < filedata.size(); i++) {
            if (!filedata[i].empty())
                os << filedata[i] << std::endl;
        }
    }

    return EXIT_SUCCESS;
}

