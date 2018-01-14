/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2017 Cppcheck team.
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

//---------------------------------------------------------------------------
#ifndef importprojectH
#define importprojectH
//---------------------------------------------------------------------------

#include "config.h"
#include "platform.h"
#include "utils.h"

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

/// @addtogroup Core
/// @{

namespace cppcheck {
    struct stricmp {
        bool operator()(const std::string &lhs, const std::string &rhs) const {
            return caseInsensitiveStringCompare(lhs,rhs) < 0;
        }
    };
}

/**
 * @brief Importing project settings.
 */
class CPPCHECKLIB ImportProject {
public:
    /** File settings. Multiple configurations for a file is allowed. */
    struct CPPCHECKLIB FileSettings {
        FileSettings() : platformType(cppcheck::Platform::Unspecified), msc(false), useMfc(false) {}
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
        cppcheck::Platform::PlatformType platformType;
        bool msc;
        bool useMfc;

        void setDefines(std::string defs);
        void setIncludePaths(const std::string &basepath, const std::list<std::string> &in, std::map<std::string, std::string, cppcheck::stricmp> &variables);
    };
    std::list<FileSettings> fileSettings;

    void ignorePaths(const std::vector<std::string> &ipaths);
    void ignoreOtherConfigs(const std::string &cfg);
    void ignoreOtherPlatforms(cppcheck::Platform::PlatformType platformType);

    void import(const std::string &filename);
protected:
    void importCompileCommands(std::istream &istr);
private:
    void importSln(std::istream &istr, const std::string &path);
    void importVcxproj(const std::string &filename, std::map<std::string, std::string, cppcheck::stricmp> &variables, const std::string &additionalIncludeDirectories);
};

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
