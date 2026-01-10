/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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
#include "filesettings.h"
#include "path.h"
#include "utils.h"

#include <cstring>
#include <exception>
#include <map>
#include <sstream>
#include <utility>

#include "xml.h"

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

void AnalyzerInformation::writeFilesTxt(const std::string &buildDir, const std::list<std::string> &sourcefiles, const std::string &userDefines, const std::list<FileSettings> &fileSettings)
{
    const std::string filesTxt(buildDir + "/files.txt");
    std::ofstream fout(filesTxt);
    fout << getFilesTxt(sourcefiles, userDefines, fileSettings);
}

std::string AnalyzerInformation::getFilesTxt(const std::list<std::string> &sourcefiles, const std::string &userDefines, const std::list<FileSettings> &fileSettings) {
    std::ostringstream ret;

    std::map<std::string, unsigned int> fileCount;

    for (const std::string &f : sourcefiles) {
        const std::string afile = getFilename(f);
        ret << afile << ".a" << (++fileCount[afile]) << sep << sep << sep << Path::simplifyPath(f) << '\n';
        if (!userDefines.empty())
            ret << afile << ".a" << (++fileCount[afile]) << sep << userDefines << sep << sep << Path::simplifyPath(f) << '\n';
    }

    for (const FileSettings &fs : fileSettings) {
        const std::string afile = getFilename(fs.filename());
        const std::string id = fs.fsFileId > 0 ? std::to_string(fs.fsFileId) : "";
        ret << afile << ".a" << (++fileCount[afile]) << sep << fs.cfg << sep << id << sep << Path::simplifyPath(fs.filename()) << std::endl;
    }

    return ret.str();
}

void AnalyzerInformation::close()
{
    if (mOutputStream.is_open()) {
        mOutputStream << "</analyzerinfo>\n";
        mOutputStream.close();
    }
}

bool AnalyzerInformation::skipAnalysis(const tinyxml2::XMLDocument &analyzerInfoDoc, std::size_t hash, std::list<ErrorMessage> &errors)
{
    const tinyxml2::XMLElement * const rootNode = analyzerInfoDoc.FirstChildElement();
    if (rootNode == nullptr)
        return false;

    const char *attr = rootNode->Attribute("hash");
    if (!attr || attr != std::to_string(hash))
        return false;

    // Check for invalid license error or internal error, in which case we should retry analysis
    for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "error") == 0 &&
            (e->Attribute("id", "premium-invalidLicense") ||
             e->Attribute("id", "premium-internalError") ||
             e->Attribute("id", "internalError")
            ))
            return false;
    }

    for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "error") == 0)
            errors.emplace_back(e);
    }

    return true;
}

std::string AnalyzerInformation::getAnalyzerInfoFileFromFilesTxt(std::istream& filesTxt, const std::string &sourcefile, const std::string &cfg, int fileIndex)
{
    const std::string id = (fileIndex > 0) ? std::to_string(fileIndex) : "";
    std::string line;
    const std::string end(sep + cfg + sep + id + sep + Path::simplifyPath(sourcefile));
    while (std::getline(filesTxt,line)) {
        if (line.size() <= end.size() + 2U)
            continue;
        if (!endsWith(line, end.c_str(), end.size()))
            continue;
        return line.substr(0,line.find(sep));
    }
    return "";
}

std::string AnalyzerInformation::getAnalyzerInfoFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, int fsFileId)
{
    std::ifstream fin(Path::join(buildDir, "files.txt"));
    if (fin.is_open()) {
        const std::string& ret = getAnalyzerInfoFileFromFilesTxt(fin, sourcefile, cfg, fsFileId);
        if (!ret.empty())
            return Path::join(buildDir, ret);
    }

    const std::string::size_type pos = sourcefile.rfind('/');
    std::string filename;
    if (pos == std::string::npos)
        filename = sourcefile;
    else
        filename = sourcefile.substr(pos + 1);
    return Path::join(buildDir, std::move(filename)) + ".analyzerinfo";
}

bool AnalyzerInformation::analyzeFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, int fsFileId, std::size_t hash, std::list<ErrorMessage> &errors)
{
    if (buildDir.empty() || sourcefile.empty())
        return true;
    close();

    const std::string analyzerInfoFile = AnalyzerInformation::getAnalyzerInfoFile(buildDir,sourcefile,cfg,fsFileId);

    tinyxml2::XMLDocument analyzerInfoDoc;
    const tinyxml2::XMLError xmlError = analyzerInfoDoc.LoadFile(analyzerInfoFile.c_str());
    if (xmlError == tinyxml2::XML_SUCCESS && skipAnalysis(analyzerInfoDoc, hash, errors))
        return false;

    mOutputStream.open(analyzerInfoFile);
    if (mOutputStream.is_open()) {
        mOutputStream << "<?xml version=\"1.0\"?>\n";
        mOutputStream << "<analyzerinfo hash=\"" << hash << "\">\n";
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

bool AnalyzerInformation::Info::parse(const std::string& filesTxtLine) {
    const std::string::size_type sep1 = filesTxtLine.find(sep);
    if (sep1 == std::string::npos)
        return false;
    const std::string::size_type sep2 = filesTxtLine.find(sep, sep1+1);
    if (sep2 == std::string::npos)
        return false;
    const std::string::size_type sep3 = filesTxtLine.find(sep, sep2+1);
    if (sep3 == std::string::npos)
        return false;

    if (sep3 == sep2 + 1)
        fsFileId = 0;
    else {
        try {
            fsFileId = std::stoi(filesTxtLine.substr(sep2+1, sep3-sep2-1));
        } catch (const std::exception&) {
            return false;
        }
    }

    afile = filesTxtLine.substr(0, sep1);
    cfg = filesTxtLine.substr(sep1+1, sep2-sep1-1);
    sourceFile = filesTxtLine.substr(sep3+1);
    return true;
}

// TODO: bail out on unexpected data
void AnalyzerInformation::processFilesTxt(const std::string& buildDir, const std::function<void(const char* checkattr, const tinyxml2::XMLElement* e, const Info& filesTxtInfo)>& handler)
{
    const std::string filesTxt(buildDir + "/files.txt");
    std::ifstream fin(filesTxt.c_str());
    std::string filesTxtLine;
    while (std::getline(fin, filesTxtLine)) {
        AnalyzerInformation::Info filesTxtInfo;
        if (!filesTxtInfo.parse(filesTxtLine)) {
            return;
        }

        const std::string xmlfile = buildDir + '/' + filesTxtInfo.afile;

        tinyxml2::XMLDocument doc;
        const tinyxml2::XMLError error = doc.LoadFile(xmlfile.c_str());
        if (error != tinyxml2::XML_SUCCESS)
            return;

        const tinyxml2::XMLElement * const rootNode = doc.FirstChildElement();
        if (rootNode == nullptr)
            return;

        for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "FileInfo") != 0)
                continue;
            const char *checkattr = e->Attribute("check");
            if (checkattr == nullptr)
                continue;
            handler(checkattr, e, filesTxtInfo);
        }
    }
}

