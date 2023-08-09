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
    const char ProjectElementName[] = "project";
    const char ProjectVersionAttrib[] = "version";
    const char ProjectFileVersion[] = "1";
    const char BuildDirElementName[] = "builddir";
    const char ImportProjectElementName[] = "importproject";
    const char AnalyzeAllVsConfigsElementName[] = "analyze-all-vs-configs";
    const char Parser[] = "parser";
    const char IncludeDirElementName[] = "includedir";
    const char DirElementName[] = "dir";
    const char DirNameAttrib[] = "name";
    const char DefinesElementName[] = "defines";
    const char DefineName[] = "define";
    const char DefineNameAttrib[] = "name";
    const char UndefinesElementName[] = "undefines";
    const char UndefineName[] = "undefine";
    const char PathsElementName[] = "paths";
    const char PathName[] = "dir";
    const char PathNameAttrib[] = "name";
    const char RootPathName[] = "root";
    const char RootPathNameAttrib[] = "name";
    const char IgnoreElementName[] = "ignore";
    const char IgnorePathName[] = "path";
    const char IgnorePathNameAttrib[] = "name";
    const char ExcludeElementName[] = "exclude";
    const char ExcludePathName[] = "path";
    const char ExcludePathNameAttrib[] = "name";
    const char FunctionContracts[] = "function-contracts";
    const char VariableContractsElementName[] = "variable-contracts";
    const char LibrariesElementName[] = "libraries";
    const char LibraryElementName[] = "library";
    const char PlatformElementName[] = "platform";
    const char SuppressionsElementName[] = "suppressions";
    const char SuppressionElementName[] = "suppression";
    const char AddonElementName[] = "addon";
    const char AddonsElementName[] = "addons";
    const char ToolElementName[] = "tool";
    const char ToolsElementName[] = "tools";
    const char TagsElementName[] = "tags";
    const char TagElementName[] = "tag";
    const char TagWarningsElementName[] = "tag-warnings";
    const char TagAttributeName[] = "tag";
    const char WarningElementName[] = "warning";
    const char HashAttributeName[] = "hash";
    const char CheckLevelExhaustiveElementName[] = "check-level-exhaustive";
    const char CheckHeadersElementName[] = "check-headers";
    const char CheckUnusedTemplatesElementName[] = "check-unused-templates";
    const char MaxCtuDepthElementName[] = "max-ctu-depth";
    const char MaxTemplateRecursionElementName[] = "max-template-recursion";
    const char CheckUnknownFunctionReturn[] = "check-unknown-function-return-values";
    const char ClangTidy[] = "clang-tidy";
    const char Name[] = "name";
    const char VSConfigurationElementName[] = "vs-configurations";
    const char VSConfigurationName[] = "config";
    // Cppcheck Premium
    const char BughuntingElementName[] = "bug-hunting";
    const char CodingStandardsElementName[] = "coding-standards";
    const char CodingStandardElementName[] = "coding-standard";
    const char CertIntPrecisionElementName[] = "cert-c-int-precision";
    const char ProjectNameElementName[] = "project-name";
}

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
