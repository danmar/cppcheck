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

#include "analyzerinfo.h"

#include "errorlogger.h"
#include "path.h"
#include "utils.h"

#include <tinyxml2.h>
#include <cstring>
#include <map>
#include <sstream> // IWYU pragma: keep

AnalyzerInformation::~AnalyzerInformation()
{
    close();
}

static std::string getFilename(const std::string &fullpath)
{
    std::string::size_type pos1 = fullpath.find_last_of("/\\");
    pos1 = (pos1 == std::string::npos) ? 0U : (pos1 + 1U);
    std::string::size_type pos2 = fullpath.rfind('.');
    if (pos2 < pos1)
        pos2 = std::string::npos;
    if (pos2 != std::string::npos)
        pos2 = pos2 - pos1;
    return fullpath.substr(pos1,pos2);
}

void AnalyzerInformation::writeFilesTxt(const std::string &buildDir, const std::list<std::string> &sourcefiles, const std::string &userDefines, const std::list<ImportProject::FileSettings> &fileSettings)
{
    std::map<std::string, unsigned int> fileCount;

    const std::string filesTxt(buildDir + "/files.txt");
    std::ofstream fout(filesTxt);
    for (const std::string &f : sourcefiles) {
        const std::string afile = getFilename(f);
        fout << afile << ".a" << (++fileCount[afile]) << "::" << Path::simplifyPath(Path::fromNativeSeparators(f)) << '\n';
        if (!userDefines.empty())
            fout << afile << ".a" << (++fileCount[afile]) << ":" << userDefines << ":" << Path::simplifyPath(Path::fromNativeSeparators(f)) << '\n';
    }

    for (const ImportProject::FileSettings &fs : fileSettings) {
        const std::string afile = getFilename(fs.filename);
        fout << afile << ".a" << (++fileCount[afile]) << ":" << fs.cfg << ":" << Path::simplifyPath(Path::fromNativeSeparators(fs.filename)) << std::endl;
    }
}

void AnalyzerInformation::close()
{
    mAnalyzerInfoFile.clear();
    if (mOutputStream.is_open()) {
        mOutputStream << "</analyzerinfo>\n";
        mOutputStream.close();
    }
}

static bool skipAnalysis(const std::string &analyzerInfoFile, std::size_t hash, std::list<ErrorMessage> &errors)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(analyzerInfoFile.c_str());
    if (error != tinyxml2::XML_SUCCESS)
        return false;

    const tinyxml2::XMLElement * const rootNode = doc.FirstChildElement();
    if (rootNode == nullptr)
        return false;

    const char *attr = rootNode->Attribute("hash");
    if (!attr || attr != std::to_string(hash))
        return false;

    for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "error") == 0)
            errors.emplace_back(e);
    }

    return true;
}

std::string AnalyzerInformation::getAnalyzerInfoFileFromFilesTxt(std::istream& filesTxt, const std::string &sourcefile, const std::string &cfg)
{
    std::string line;
    const std::string end(':' + cfg + ':' + Path::simplifyPath(sourcefile));
    while (std::getline(filesTxt,line)) {
        if (line.size() <= end.size() + 2U)
            continue;
        if (!endsWith(line, end.c_str(), end.size()))
            continue;
        return line.substr(0,line.find(':'));
    }
    return "";
}

std::string AnalyzerInformation::getAnalyzerInfoFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg)
{
    std::ifstream fin(Path::join(buildDir, "files.txt"));
    if (fin.is_open()) {
        const std::string& ret = getAnalyzerInfoFileFromFilesTxt(fin, sourcefile, cfg);
        if (!ret.empty())
            return Path::join(buildDir, ret);
    }

    const std::string::size_type pos = sourcefile.rfind('/');
    std::string filename;
    if (pos == std::string::npos)
        filename = sourcefile;
    else
        filename = sourcefile.substr(pos + 1);
    return Path::join(buildDir, filename) + ".analyzerinfo";
}

bool AnalyzerInformation::analyzeFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, std::size_t hash, std::list<ErrorMessage> &errors)
{
    if (buildDir.empty() || sourcefile.empty())
        return true;
    close();

    mAnalyzerInfoFile = AnalyzerInformation::getAnalyzerInfoFile(buildDir,sourcefile,cfg);

    if (skipAnalysis(mAnalyzerInfoFile, hash, errors))
        return false;

    mOutputStream.open(mAnalyzerInfoFile);
    if (mOutputStream.is_open()) {
        mOutputStream << "<?xml version=\"1.0\"?>\n";
        mOutputStream << "<analyzerinfo hash=\"" << hash << "\">\n";
    } else {
        mAnalyzerInfoFile.clear();
    }

    return true;
}

void AnalyzerInformation::reportErr(const ErrorMessage &msg)
{
    if (mOutputStream.is_open())
        mOutputStream << msg.toXML() << '\n';
}

void AnalyzerInformation::setFileInfo(const std::string &check, const std::string &fileInfo)
{
    if (mOutputStream.is_open() && !fileInfo.empty())
        mOutputStream << "  <FileInfo check=\"" << check << "\">\n" << fileInfo << "  </FileInfo>\n";
}
