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

#include "helpers.h"

#include "filelister.h"
#include "path.h"
#include "pathmatch.h"
#include "preprocessor.h"

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <fstream> // IWYU pragma: keep
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

class Suppressions;

// TODO: better path-only usage
ScopedFile::ScopedFile(std::string name, const std::string &content, std::string path)
    : mName(std::move(name))
    , mPath(Path::toNativeSeparators(std::move(path)))
    , mFullPath(Path::join(mPath, mName))
{
    if (!mPath.empty() && mPath != Path::getCurrentPath()) {
#ifdef _WIN32
        if (!CreateDirectoryA(mPath.c_str(), nullptr))
            throw std::runtime_error("ScopedFile(" + mFullPath + ") - could not create directory");
#else
        if (mkdir(mPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
            throw std::runtime_error("ScopedFile(" + mFullPath + ") - could not create directory");
#endif
    }

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
        std::map<std::string, std::size_t> files;
        const std::string res = FileLister::addFiles(files, mPath, {".plist"}, false, PathMatch({}));
        if (!res.empty()) {
            std::cout << "ScopedFile(" << mPath + ") - generating file list failed (" << res << ")" << std::endl;
        }
        for (const auto &f : files)
        {
            const std::string &file = f.first;
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
std::string PreprocessorHelper::getcode(Preprocessor &preprocessor, const std::string &filedata, const std::string &cfg, const std::string &filename, Suppressions *inlineSuppression)
{
    simplecpp::OutputList outputList;
    std::vector<std::string> files;

    std::istringstream istr(filedata);
    simplecpp::TokenList tokens1(istr, files, Path::simplifyPath(filename), &outputList);
    if (inlineSuppression)
        preprocessor.inlineSuppressions(tokens1, *inlineSuppression);
    tokens1.removeComments();
    preprocessor.simplifyPragmaAsm(&tokens1);
    preprocessor.removeComments();
    preprocessor.setDirectives(tokens1);

    preprocessor.reportOutput(outputList, true);

    if (Preprocessor::hasErrors(outputList))
        return "";

    std::string ret;
    try {
        ret = preprocessor.getcode(tokens1, cfg, files, filedata.find("#file") != std::string::npos);
    } catch (const simplecpp::Output &) {
        ret.clear();
    }

    // Since "files" is a local variable the tracking info must be cleared..
    preprocessor.mMacroUsage.clear();
    preprocessor.mIfCond.clear();

    return ret;
}
