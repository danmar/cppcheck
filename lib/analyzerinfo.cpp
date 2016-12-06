/*
* Cppcheck - A tool for static C/C++ code analysis
* Copyright (C) 2007-2016 Cppcheck team.
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
#include <tinyxml2.h>
#include <sstream>

AnalyzerInformation::~AnalyzerInformation()
{
    close();
}

void AnalyzerInformation::close()
{
    analyzerInfoFile.clear();
    if (fout.is_open()) {
        fout << "</analyzerinfo>\n";
        fout.close();
    }
}

static bool skipAnalysis(const std::string &analyzerInfoFile, unsigned long long checksum, std::list<ErrorLogger::ErrorMessage> *errors)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(analyzerInfoFile.c_str());
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
            errors->push_back(ErrorLogger::ErrorMessage(e));
    }

    return true;
}

std::string AnalyzerInformation::getAnalyzerInfoFile(const std::string &buildDir, const std::string &sourcefile)
{
    const std::string files(buildDir + "/files.txt");
    std::ifstream fin(files.c_str());
    if (fin.is_open()) {
        int id = 1;
        std::string line;
        while (std::getline(fin,line)) {
            if (line == sourcefile) {
                std::ostringstream ostr;
                ostr << buildDir << '/' << id << ".analyzeinfo";
                return ostr.str();
            }
            id++;
        }
    }

    std::string filename = Path::fromNativeSeparators(buildDir);
    if (filename.back() != '/')
        filename += '/';
    const std::string::size_type pos = sourcefile.rfind('/');
    if (pos == std::string::npos)
        filename += sourcefile;
    else
        filename += sourcefile.substr(pos+1);
    filename += ".analyzerinfo";
    return filename;
}

bool AnalyzerInformation::analyzeFile(const std::string &buildDir, const std::string &sourcefile, unsigned long long checksum, std::list<ErrorLogger::ErrorMessage> *errors)
{
    if (buildDir.empty() || sourcefile.empty())
        return true;
    close();

    analyzerInfoFile = AnalyzerInformation::getAnalyzerInfoFile(buildDir,sourcefile);

    if (skipAnalysis(analyzerInfoFile, checksum, errors))
        return false;

    fout.open(analyzerInfoFile);
    if (fout.is_open()) {
        fout << "<?xml version=\"1.0\"?>\n";
        fout << "<analyzerinfo checksum=\"" << checksum << "\">\n";
    } else {
        analyzerInfoFile.clear();
    }

    return true;
}

void AnalyzerInformation::reportErr(const ErrorLogger::ErrorMessage &msg, bool verbose)
{
    if (fout.is_open())
        fout << msg.toXML(verbose,2) << '\n';
}

void AnalyzerInformation::setFileInfo(const std::string &check, const std::string &fileInfo)
{
    if (fout.is_open() && !fileInfo.empty())
        fout << "  <FileInfo check=\"" << check << "\">\n" << fileInfo << "  </FileInfo>\n";
}
