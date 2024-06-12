/* -*- C++ -*-
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

#ifndef fileSettingsH
#define fileSettingsH

#include "config.h"
#include "path.h"
#include "platform.h"
#include "standards.h"

#include <list>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

class FileWithDetails
{
public:
    explicit FileWithDetails(std::string path)
        : FileWithDetails(std::move(path), Standards::Language::None, 0)
    {}

    FileWithDetails(std::string path, Standards::Language lang, std::size_t size)
        : mPath(std::move(path))
        , mPathSimplified(Path::simplifyPath(mPath))
        , mLang(lang)
        , mSize(size)
    {
        if (mPath.empty())
            throw std::runtime_error("empty path specified");
    }

    const std::string& path() const
    {
        return mPath;
    }

    const std::string& spath() const
    {
        return mPathSimplified;
    }

    std::size_t size() const
    {
        return mSize;
    }

    void setLang(Standards::Language lang)
    {
        mLang = lang;
    }

    Standards::Language lang() const
    {
        return mLang;
    }
private:
    std::string mPath;
    std::string mPathSimplified;
    Standards::Language mLang = Standards::Language::None;
    std::size_t mSize;
};

/** File settings. Multiple configurations for a file is allowed. */
struct CPPCHECKLIB FileSettings {
    explicit FileSettings(std::string path)
        : file(std::move(path))
    {}

    FileSettings(std::string path, Standards::Language lang, std::size_t size)
        : file(std::move(path), lang, size)
    {}

    std::string cfg;
    FileWithDetails file;
    const std::string& filename() const
    {
        return file.path();
    }
    // cppcheck-suppress unusedFunction
    const std::string& sfilename() const
    {
        return file.spath();
    }
    std::string defines;
    // TODO: handle differently
    std::string cppcheckDefines() const {
        return defines + (msc ? ";_MSC_VER=1900" : "") + (useMfc ? ";__AFXWIN_H__=1" : "");
    }
    std::set<std::string> undefs;
    std::list<std::string> includePaths;
    // only used by clang mode
    std::list<std::string> systemIncludePaths;
    std::string standard;
    Platform::Type platformType = Platform::Type::Unspecified;
    // TODO: get rid of these
    bool msc{};
    bool useMfc{};
};

#endif // fileSettingsH
