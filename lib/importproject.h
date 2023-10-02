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

//---------------------------------------------------------------------------
#ifndef importprojectH
#define importprojectH
//---------------------------------------------------------------------------

#include "config.h"
#include "platform.h"
#include "utils.h"

#include <iosfwd>
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
        FAILURE,
        COMPILE_DB,
        VS_SLN,
        VS_VCXPROJ,
        BORLAND,
        CPPCHECK_GUI
    };

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
        cppcheck::Platform::Type platformType = cppcheck::Platform::Type::Unspecified;
        bool msc{};
        bool useMfc{};

        void parseCommand(const std::string& command);
        void setDefines(std::string defs);
        void setIncludePaths(const std::string &basepath, const std::list<std::string> &in, std::map<std::string, std::string, cppcheck::stricmp> &variables);
    };
    std::list<FileSettings> fileSettings;
    Type projectType;

    ImportProject();
    virtual ~ImportProject() = default;
    ImportProject(const ImportProject&) = default;
    ImportProject& operator=(const ImportProject&) = default;

    void selectOneVsConfig(cppcheck::Platform::Type platform);
    void selectVsConfigurations(cppcheck::Platform::Type platform, const std::vector<std::string> &configurations);

    std::list<std::string> getVSConfigs();

    // Cppcheck GUI output
    struct {
        std::string analyzeAllVsConfigs;
        std::vector<std::string> pathNames;
        std::list<std::string> libraries;
        std::list<std::string> excludedPaths;
        std::list<std::string> checkVsConfigs;
        std::string projectFile;
        std::string platform;
    } guiProject;

    void ignorePaths(const std::vector<std::string> &ipaths);
    void ignoreOtherConfigs(const std::string &cfg);

    Type import(const std::string &filename, Settings *settings=nullptr);
protected:
    bool importCompileCommands(std::istream &istr);
    bool importCppcheckGuiProject(std::istream &istr, Settings *settings);
    virtual bool sourceFileExists(const std::string &file);
private:
    bool importSln(std::istream &istr, const std::string &path, const std::vector<std::string> &fileFilters);
    bool importVcxproj(const std::string &filename, std::map<std::string, std::string, cppcheck::stricmp> &variables, const std::string &additionalIncludeDirectories, const std::vector<std::string> &fileFilters);
    bool importBcb6Prj(const std::string &projectFilename);

    static void printError(const std::string &message);

    void setRelativePaths(const std::string &filename);

    std::string mPath;
    std::set<std::string> mAllVSConfigs;
};


namespace CppcheckXml {
    extern const char ProjectElementName[];
    extern const char ProjectVersionAttrib[];
    extern const char ProjectFileVersion[];
    extern const char BuildDirElementName[];
    extern const char ImportProjectElementName[];
    extern const char AnalyzeAllVsConfigsElementName[];
    extern const char Parser[];
    extern const char IncludeDirElementName[];
    extern const char DirElementName[];
    extern const char DirNameAttrib[];
    extern const char DefinesElementName[];
    extern const char DefineName[];
    extern const char DefineNameAttrib[];
    extern const char UndefinesElementName[];
    extern const char UndefineName[];
    extern const char PathsElementName[];
    extern const char PathName[];
    extern const char PathNameAttrib[];
    extern const char RootPathName[];
    extern const char RootPathNameAttrib[];
    extern const char IgnoreElementName[];
    extern const char IgnorePathName[];
    extern const char IgnorePathNameAttrib[];
    extern const char ExcludeElementName[];
    extern const char ExcludePathName[];
    extern const char ExcludePathNameAttrib[];
    extern const char FunctionContracts[];
    extern const char VariableContractsElementName[];
    extern const char LibrariesElementName[];
    extern const char LibraryElementName[];
    extern const char PlatformElementName[];
    extern const char SuppressionsElementName[];
    extern const char SuppressionElementName[];
    extern const char AddonElementName[];
    extern const char AddonsElementName[];
    extern const char ToolElementName[];
    extern const char ToolsElementName[];
    extern const char TagsElementName[];
    extern const char TagElementName[];
    extern const char TagWarningsElementName[];
    extern const char TagAttributeName[];
    extern const char WarningElementName[];
    extern const char HashAttributeName[];
    extern const char CheckLevelExhaustiveElementName[];
    extern const char CheckHeadersElementName[];
    extern const char CheckUnusedTemplatesElementName[];
    extern const char MaxCtuDepthElementName[];
    extern const char MaxTemplateRecursionElementName[];
    extern const char CheckUnknownFunctionReturn[];
    extern const char ClangTidy[];
    extern const char Name[];
    extern const char VSConfigurationElementName[];
    extern const char VSConfigurationName[];
    // Cppcheck Premium
    extern const char BughuntingElementName[];
    extern const char CodingStandardsElementName[];
    extern const char CodingStandardElementName[];
    extern const char CertIntPrecisionElementName[];
    extern const char ProjectNameElementName[];
}

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
