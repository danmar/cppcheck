/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "helpers.h"

#include "filelister.h"
#include "filesettings.h"
#include "library.h"
#include "path.h"
#include "pathmatch.h"
#include "preprocessor.h"

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <simplecpp.h>

#include "xml.h"

class SuppressionList;

const Settings SimpleTokenizer::s_settings;

// TODO: better path-only usage
ScopedFile::ScopedFile(std::string name, const std::string &content, std::string path)
    : mName(std::move(name))
    , mPath(Path::toNativeSeparators(std::move(path)))
    , mFullPath(Path::join(mPath, mName))
{
    if (!mPath.empty() && mPath != Path::getCurrentPath()) {
        if (Path::isDirectory(mPath))
            throw std::runtime_error("ScopedFile(" + mFullPath + ") - directory already exists");
#ifdef _WIN32
        if (!CreateDirectoryA(mPath.c_str(), nullptr))
            throw std::runtime_error("ScopedFile(" + mFullPath + ") - could not create directory");
#else
        if (mkdir(mPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
            throw std::runtime_error("ScopedFile(" + mFullPath + ") - could not create directory");
#endif
    }

    if (Path::isFile(mFullPath))
        throw std::runtime_error("ScopedFile(" + mFullPath + ") - file already exists");

    std::ofstream of(mFullPath);
    if (!of.is_open())
        throw std::runtime_error("ScopedFile(" + mFullPath + ") - could not open file");
    of << content;
}

ScopedFile::~ScopedFile() {
    const int remove_res = std::remove(mFullPath.c_str());
    if (remove_res != 0) {
        std::cout << "ScopedFile(" << mFullPath + ") - could not delete file (" << remove_res << ")" << std::endl;
    }
    if (!mPath.empty() && mPath != Path::getCurrentPath()) {
        // TODO: remove all files
        // TODO: simplify the function call
        // hack to be able to delete *.plist output files
        std::list<FileWithDetails> files;
        const std::string res = FileLister::addFiles(files, mPath, {".plist"}, false, PathMatch());
        if (!res.empty()) {
            std::cout << "ScopedFile(" << mPath + ") - generating file list failed (" << res << ")" << std::endl;
        }
        for (const auto &f : files)
        {
            const std::string &file = f.path();
            const int rm_f_res = std::remove(file.c_str());
            if (rm_f_res != 0) {
                std::cout << "ScopedFile(" << mPath + ") - could not delete '" << file << "' (" << rm_f_res << ")" << std::endl;
            }
        }

#ifdef _WIN32
        if (!RemoveDirectoryA(mPath.c_str())) {
            std::cout << "ScopedFile(" << mFullPath + ") - could not delete folder (" << GetLastError() << ")" << std::endl;
        }
#else
        const int rmdir_res = rmdir(mPath.c_str());
        if (rmdir_res == -1) {
            const int err = errno;
            std::cout << "ScopedFile(" << mFullPath + ") - could not delete folder (" << err << ")" << std::endl;
        }
#endif
    }
}

// TODO: we should be using the actual Preprocessor implementation
std::string PreprocessorHelper::getcode(const Settings& settings, ErrorLogger& errorlogger, const std::string &filedata, const std::string &cfg, const std::string &filename, SuppressionList *inlineSuppression)
{
    std::map<std::string, std::string> cfgcode = getcode(settings, errorlogger, filedata.c_str(), std::set<std::string>{cfg}, filename, inlineSuppression);
    const auto it = cfgcode.find(cfg);
    if (it == cfgcode.end())
        return "";
    return it->second;
}

std::map<std::string, std::string> PreprocessorHelper::getcode(const Settings& settings, ErrorLogger& errorlogger, const char code[], const std::string &filename, SuppressionList *inlineSuppression)
{
    return getcode(settings, errorlogger, code, {}, filename, inlineSuppression);
}

std::map<std::string, std::string> PreprocessorHelper::getcode(const Settings& settings, ErrorLogger& errorlogger, const char code[], std::set<std::string> cfgs, const std::string &filename, SuppressionList *inlineSuppression)
{
    simplecpp::OutputList outputList;
    std::vector<std::string> files;

    std::istringstream istr(code);
    simplecpp::TokenList tokens(istr, files, Path::simplifyPath(filename), &outputList);
    Preprocessor preprocessor(settings, errorlogger, Path::identify(tokens.getFiles()[0], false));
    if (inlineSuppression)
        preprocessor.inlineSuppressions(tokens, *inlineSuppression);
    preprocessor.removeComments(tokens);
    preprocessor.simplifyPragmaAsm(tokens);

    preprocessor.reportOutput(outputList, true);

    if (Preprocessor::hasErrors(outputList))
        return {};

    std::map<std::string, std::string> cfgcode;
    if (cfgs.empty())
        cfgs = preprocessor.getConfigs(tokens);
    for (const std::string & config : cfgs) {
        try {
            // TODO: also preserve location information when #include exists - enabling that will fail since #line is treated like a regular token
            cfgcode[config] = preprocessor.getcode(tokens, config, files, std::string(code).find("#file") != std::string::npos);
        } catch (const simplecpp::Output &) {
            cfgcode[config] = "";
        }
    }

    return cfgcode;
}

void SimpleTokenizer2::preprocess(const char code[], std::vector<std::string> &files, const std::string& file0, Tokenizer& tokenizer, ErrorLogger& errorlogger)
{
    // TODO: get rid of stream
    std::istringstream istr(code);
    const simplecpp::TokenList tokens1(istr, files, file0);

    Preprocessor preprocessor(tokenizer.getSettings(), errorlogger, Path::identify(tokens1.getFiles()[0], false));
    simplecpp::TokenList tokens2 = preprocessor.preprocess(tokens1, "", files, true);

    // Tokenizer..
    tokenizer.list.createTokens(std::move(tokens2));

    std::list<Directive> directives = preprocessor.createDirectives(tokens1);
    tokenizer.setDirectives(std::move(directives));
}

bool LibraryHelper::loadxmldata(Library &lib, const char xmldata[], std::size_t len)
{
    tinyxml2::XMLDocument doc;
    return (tinyxml2::XML_SUCCESS == doc.Parse(xmldata, len)) && (lib.load(doc).errorcode == Library::ErrorCode::OK);
}

bool LibraryHelper::loadxmldata(Library &lib, Library::Error& liberr, const char xmldata[], std::size_t len)
{
    tinyxml2::XMLDocument doc;
    if (tinyxml2::XML_SUCCESS != doc.Parse(xmldata, len))
        return false;
    liberr = lib.load(doc);
    return true;
}

Library::Error LibraryHelper::loadxmldoc(Library &lib, const tinyxml2::XMLDocument& doc)
{
    return lib.load(doc);
}
