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
    extern CPPCHECKLIB const char ProjectElementName[];
    extern CPPCHECKLIB const char ProjectVersionAttrib[];
    extern CPPCHECKLIB const char ProjectFileVersion[];
    extern CPPCHECKLIB const char BuildDirElementName[];
    extern CPPCHECKLIB const char ImportProjectElementName[];
    extern CPPCHECKLIB const char AnalyzeAllVsConfigsElementName[];
    extern CPPCHECKLIB const char Parser[];
    extern CPPCHECKLIB const char IncludeDirElementName[];
    extern CPPCHECKLIB const char DirElementName[];
    extern CPPCHECKLIB const char DirNameAttrib[];
    extern CPPCHECKLIB const char DefinesElementName[];
    extern CPPCHECKLIB const char DefineName[];
    extern CPPCHECKLIB const char DefineNameAttrib[];
    extern CPPCHECKLIB const char UndefinesElementName[];
    extern CPPCHECKLIB const char UndefineName[];
    extern CPPCHECKLIB const char PathsElementName[];
    extern CPPCHECKLIB const char PathName[];
    extern CPPCHECKLIB const char PathNameAttrib[];
    extern CPPCHECKLIB const char RootPathName[];
    extern CPPCHECKLIB const char RootPathNameAttrib[];
    extern CPPCHECKLIB const char IgnoreElementName[];
    extern CPPCHECKLIB const char IgnorePathName[];
    extern CPPCHECKLIB const char IgnorePathNameAttrib[];
    extern CPPCHECKLIB const char ExcludeElementName[];
    extern CPPCHECKLIB const char ExcludePathName[];
    extern CPPCHECKLIB const char ExcludePathNameAttrib[];
    extern CPPCHECKLIB const char FunctionContracts[];
    extern CPPCHECKLIB const char VariableContractsElementName[];
    extern CPPCHECKLIB const char LibrariesElementName[];
    extern CPPCHECKLIB const char LibraryElementName[];
    extern CPPCHECKLIB const char PlatformElementName[];
    extern CPPCHECKLIB const char SuppressionsElementName[];
    extern CPPCHECKLIB const char SuppressionElementName[];
    extern CPPCHECKLIB const char AddonElementName[];
    extern CPPCHECKLIB const char AddonsElementName[];
    extern CPPCHECKLIB const char ToolElementName[];
    extern CPPCHECKLIB const char ToolsElementName[];
    extern CPPCHECKLIB const char TagsElementName[];
    extern CPPCHECKLIB const char TagElementName[];
    extern CPPCHECKLIB const char TagWarningsElementName[];
    extern CPPCHECKLIB const char TagAttributeName[];
    extern CPPCHECKLIB const char WarningElementName[];
    extern CPPCHECKLIB const char HashAttributeName[];
    extern CPPCHECKLIB const char CheckLevelExhaustiveElementName[];
    extern CPPCHECKLIB const char CheckHeadersElementName[];
    extern CPPCHECKLIB const char CheckUnusedTemplatesElementName[];
    extern CPPCHECKLIB const char MaxCtuDepthElementName[];
    extern CPPCHECKLIB const char MaxTemplateRecursionElementName[];
    extern CPPCHECKLIB const char CheckUnknownFunctionReturn[];
    extern CPPCHECKLIB const char ClangTidy[];
    extern CPPCHECKLIB const char Name[];
    extern CPPCHECKLIB const char VSConfigurationElementName[];
    extern CPPCHECKLIB const char VSConfigurationName[];
    // Cppcheck Premium
    extern CPPCHECKLIB const char BughuntingElementName[];
    extern CPPCHECKLIB const char CodingStandardsElementName[];
    extern CPPCHECKLIB const char CodingStandardElementName[];
    extern CPPCHECKLIB const char CertIntPrecisionElementName[];
    extern CPPCHECKLIB const char ProjectNameElementName[];
}

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
