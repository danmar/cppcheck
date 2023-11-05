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

#ifndef fileSettingsH
#define fileSettingsH

#include "config.h"
#include "platform.h"

#include <list>
#include <set>
#include <string>

/** File settings. Multiple configurations for a file is allowed. */
struct CPPCHECKLIB FileSettings {
    std::string cfg;
    std::string filename;
    std::string defines;
    std::string cppcheckDefines() const {
        return defines + (msc ? ";_MSC_VER=1900" : "") + (useMfc ? ";__AFXWIN_H__=1" : "");
    }
    std::set<std::string> undefs;
    std::list<std::string> includePaths;
    std::list<std::string> systemIncludePaths;
    std::string standard;
    Platform::Type platformType = Platform::Type::Unspecified;
    bool msc{};
    bool useMfc{};
};

#endif // fileSettingsH
