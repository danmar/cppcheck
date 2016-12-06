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

//---------------------------------------------------------------------------
#ifndef importprojectH
#define importprojectH
//---------------------------------------------------------------------------

#include "config.h"
#include "platform.h"
#include <list>
#include <map>
#include <string>
#include <set>
#include <vector>

/// @addtogroup Core
/// @{

/**
 * @brief Importing project settings.
 */
class CPPCHECKLIB ImportProject {
public:
    /** File settings. Multiple configurations for a file is allowed. */
    struct FileSettings {
        FileSettings() : platformType(cppcheck::Platform::Unspecified) {}
        std::string cfg;
        std::string filename;
        std::string defines;
        std::set<std::string> undefs;
        std::list<std::string> includePaths;
        cppcheck::Platform::PlatformType platformType;

        void setDefines(std::string defs);
        void setIncludePaths(const std::string &basepath, const std::list<std::string> &in, const std::map<std::string, std::string> &variables);
    };
    std::list<FileSettings> fileSettings;

    void ignorePaths(const std::vector<std::string> &ipaths);
    void ignoreOtherConfigs(const std::string &cfg);
    void ignoreOtherPlatforms(cppcheck::Platform::PlatformType platformType);

    void import(const std::string &filename);
private:
    void importCompileCommands(std::istream &istr);
    void importSln(std::istream &istr, const std::string &path);
    void importVcxproj(const std::string &filename, std::map<std::string, std::string> variables, const std::string &additionalIncludeDirectories);
};

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
