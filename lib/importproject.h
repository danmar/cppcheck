/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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

class Settings;

/**
 * @brief Importing project settings.
 */
class CPPCHECKLIB ImportProject {
public:
    enum class Type {
        UNKNOWN,
        MISSING,
        COMPILE_DB,
        VS_SLN,
        VS_VCXPROJ,
        BORLAND,
        CPPCHECK_GUI
    };

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

        void parseCommand(const std::string &command);
        void setDefines(std::string defs);
        void setIncludePaths(const std::string &basepath, const std::list<std::string> &in, std::map<std::string, std::string, cppcheck::stricmp> &variables);
    };
    std::list<FileSettings> fileSettings;

    void selectOneVsConfig(cppcheck::Platform::PlatformType platform);

    // Cppcheck GUI output
    struct {
        std::string analyzeAllVsConfigs;
        std::vector<std::string> pathNames;
        std::list<std::string> libraries;
        std::list<std::string> excludedPaths;
        std::string projectFile;
        std::string platform;
    } guiProject;

    void ignorePaths(const std::vector<std::string> &ipaths);
    void ignoreOtherConfigs(const std::string &cfg);
    void ignoreOtherPlatforms(cppcheck::Platform::PlatformType platformType);

    Type import(const std::string &filename, Settings *settings=nullptr);
protected:
    void importCompileCommands(std::istream &istr);
    bool importCppcheckGuiProject(std::istream &istr, Settings *settings);
private:
    void importSln(std::istream &istr, const std::string &path);
    void importVcxproj(const std::string &filename, std::map<std::string, std::string, cppcheck::stricmp> &variables, const std::string &additionalIncludeDirectories);
    void importBcb6Prj(const std::string &projectFilename);

    std::string mPath;
};


namespace CppcheckXml {
constexpr char ProjectElementName[] = "project";
constexpr char ProjectVersionAttrib[] = "version";
constexpr char ProjectFileVersion[] = "1";
constexpr char BuildDirElementName[] = "builddir";
constexpr char ImportProjectElementName[] = "importproject";
constexpr char AnalyzeAllVsConfigsElementName[] = "analyze-all-vs-configs";
constexpr char IncludeDirElementName[] = "includedir";
constexpr char DirElementName[] = "dir";
constexpr char DirNameAttrib[] = "name";
constexpr char DefinesElementName[] = "defines";
constexpr char DefineName[] = "define";
constexpr char DefineNameAttrib[] = "name";
constexpr char UndefinesElementName[] = "undefines";
constexpr char UndefineName[] = "undefine";
constexpr char PathsElementName[] = "paths";
constexpr char PathName[] = "dir";
constexpr char PathNameAttrib[] = "name";
constexpr char RootPathName[] = "root";
constexpr char RootPathNameAttrib[] = "name";
constexpr char IgnoreElementName[] = "ignore";
constexpr char IgnorePathName[] = "path";
constexpr char IgnorePathNameAttrib[] = "name";
constexpr char ExcludeElementName[] = "exclude";
constexpr char ExcludePathName[] = "path";
constexpr char ExcludePathNameAttrib[] = "name";
constexpr char LibrariesElementName[] = "libraries";
constexpr char LibraryElementName[] = "library";
constexpr char PlatformElementName[] = "platform";
constexpr char SuppressionsElementName[] = "suppressions";
constexpr char SuppressionElementName[] = "suppression";
constexpr char AddonElementName[] = "addon";
constexpr char AddonsElementName[] = "addons";
constexpr char ToolElementName[] = "tool";
constexpr char ToolsElementName[] = "tools";
constexpr char TagsElementName[] = "tags";
constexpr char TagElementName[] = "tag";
constexpr char CheckHeadersElementName[] = "check-headers";
constexpr char CheckUnusedTemplatesElementName[] = "check-unused-templates";
constexpr char MaxCtuDepthElementName[] = "max-ctu-depth";
constexpr char CheckUnknownFunctionReturn[] = "check-unknown-function-return-values";
constexpr char Name[] = "name";
}

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
