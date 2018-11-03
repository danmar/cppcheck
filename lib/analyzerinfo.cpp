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

#include "analyzerinfo.h"

#include "path.h"
#include "utils.h"

#include <tinyxml2.h>
#include <cstring>
#include <map>
#include <sstream>

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

void AnalyzerInformation::writeFilesTxt(const std::string &buildDir, const std::list<std::string> &sourcefiles, const std::list<ImportProject::FileSettings> &fileSettings)
{
    std::map<std::string, unsigned int> fileCount;

    const std::string filesTxt(buildDir + "/files.txt");
    std::ofstream fout(filesTxt);
    for (const std::string &f : sourcefiles) {
        const std::string afile = getFilename(f);
        if (fileCount.find(afile) == fileCount.end())
            fileCount[afile] = 0;
        fout << afile << ".a" << (++fileCount[afile]) << "::" << Path::fromNativeSeparators(f) << '\n';
    }

    for (const ImportProject::FileSettings &fs : fileSettings) {
        const std::string afile = getFilename(fs.filename);
        if (fileCount.find(afile) == fileCount.end())
            fileCount[afile] = 0;
        fout << afile << ".a" << (++fileCount[afile]) << ":" << fs.cfg << ":" << Path::fromNativeSeparators(fs.filename) << std::endl;
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

static bool skipAnalysis(const std::string &analyzerInfoFile, unsigned long long checksum, std::list<ErrorLogger::ErrorMessage> *errors)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(analyzerInfoFile.c_str());
    if (error != tinyxml2::XML_SUCCESS)
        return false;

    const tinyxml2::XMLElement * const rootNode = doc.FirstChildElement();
    if (rootNode == nullptr)
        return false;

    const char *attr = rootNode->Attribute("checksum");
    if (!attr || attr != std::to_string(checksum))
        return false;

    for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "error") == 0)
            errors->emplace_back(e);
    }

    return true;
}

std::string AnalyzerInformation::getAnalyzerInfoFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg)
{
    const std::string files(buildDir + "/files.txt");
    std::ifstream fin(files);
    if (fin.is_open()) {
        std::string line;
        const std::string end(':' + cfg + ':' + sourcefile);
        while (std::getline(fin,line)) {
            if (line.size() <= end.size() + 2U)
                continue;
            if (!endsWith(line, end.c_str(), end.size()))
                continue;
            std::ostringstream ostr;
            ostr << buildDir << '/' << line.substr(0,line.find(':'));
            return ostr.str();
        }
    }

    std::string filename = Path::fromNativeSeparators(buildDir);
    if (!endsWith(filename, '/'))
        filename += '/';
    const std::string::size_type pos = sourcefile.rfind('/');
    if (pos == std::string::npos)
        filename += sourcefile;
    else
        filename += sourcefile.substr(pos+1);
    filename += ".analyzerinfo";
    return filename;
}

bool AnalyzerInformation::analyzeFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, unsigned long long checksum, std::list<ErrorLogger::ErrorMessage> *errors)
{
    if (buildDir.empty() || sourcefile.empty())
        return true;
    close();

    mAnalyzerInfoFile = AnalyzerInformation::getAnalyzerInfoFile(buildDir,sourcefile,cfg);

    if (skipAnalysis(mAnalyzerInfoFile, checksum, errors))
        return false;

    mOutputStream.open(mAnalyzerInfoFile);
    if (mOutputStream.is_open()) {
        mOutputStream << "<?xml version=\"1.0\"?>\n";
        mOutputStream << "<analyzerinfo checksum=\"" << checksum << "\">\n";
    } else {
        mAnalyzerInfoFile.clear();
    }

    return true;
}

void AnalyzerInformation::reportErr(const ErrorLogger::ErrorMessage &msg, bool /*verbose*/)
{
    if (mOutputStream.is_open())
        mOutputStream << msg.toXML() << '\n';
}

void AnalyzerInformation::setFileInfo(const std::string &check, const std::string &fileInfo)
{
    if (mOutputStream.is_open() && !fileInfo.empty())
        mOutputStream << "  <FileInfo check=\"" << check << "\">\n" << fileInfo << "  </FileInfo>\n";
}
