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

#include "importproject.h"

#include "path.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "token.h"
#include "tokenize.h"
#include "utils.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream> // IWYU pragma: keep
#include <iostream>
#include <iterator>
#include <sstream> // IWYU pragma: keep
#include <unordered_set>
#include <utility>

#include <tinyxml2.h>

#include <simplecpp.h>

#include "json.h"

ImportProject::ImportProject()
{
    projectType = Type::UNKNOWN;
}

void ImportProject::ignorePaths(const std::vector<std::string> &ipaths)
{
    for (std::list<FileSettings>::iterator it = fileSettings.begin(); it != fileSettings.end();) {
        bool ignore = false;
        for (std::string i : ipaths) {
            if (it->filename.size() > i.size() && it->filename.compare(0,i.size(),i)==0) {
                ignore = true;
                break;
            }
            if (isValidGlobPattern(i) && matchglob(i, it->filename)) {
                ignore = true;
                break;
            }
            if (!Path::isAbsolute(i)) {
                i = mPath + i;
                if (it->filename.size() > i.size() && it->filename.compare(0,i.size(),i)==0) {
                    ignore = true;
                    break;
                }
            }
        }
        if (ignore)
            it = fileSettings.erase(it);
        else
            ++it;
    }
}

void ImportProject::ignoreOtherConfigs(const std::string &cfg)
{
    for (std::list<FileSettings>::iterator it = fileSettings.begin(); it != fileSettings.end();) {
        if (it->cfg != cfg)
            it = fileSettings.erase(it);
        else
            ++it;
    }
}

void ImportProject::FileSettings::setDefines(std::string defs)
{
    while (defs.find(";%(") != std::string::npos) {
        const std::string::size_type pos1 = defs.find(";%(");
        const std::string::size_type pos2 = defs.find(';', pos1+1);
        defs.erase(pos1, pos2 == std::string::npos ? pos2 : (pos2-pos1));
    }
    while (defs.find(";;") != std::string::npos)
        defs.erase(defs.find(";;"),1);
    while (!defs.empty() && defs[0] == ';')
        defs.erase(0, 1);
    while (!defs.empty() && endsWith(defs,';'))
        defs.erase(defs.size() - 1U); // TODO: Use std::string::pop_back() as soon as travis supports it
    bool eq = false;
    for (std::size_t pos = 0; pos < defs.size(); ++pos) {
        if (defs[pos] == '(' || defs[pos] == '=')
            eq = true;
        else if (defs[pos] == ';') {
            if (!eq) {
                defs.insert(pos,"=1");
                pos += 3;
            }
            if (pos < defs.size())
                eq = false;
        }
    }
    if (!eq && !defs.empty())
        defs += "=1";
    defines.swap(defs);
}

static bool simplifyPathWithVariables(std::string &s, std::map<std::string, std::string, cppcheck::stricmp> &variables)
{
    std::set<std::string, cppcheck::stricmp> expanded;
    std::string::size_type start = 0;
    while ((start = s.find("$(")) != std::string::npos) {
        const std::string::size_type end = s.find(')',start);
        if (end == std::string::npos)
            break;
        const std::string var = s.substr(start+2,end-start-2);
        if (expanded.find(var) != expanded.end())
            break;
        expanded.insert(var);
        std::map<std::string, std::string, cppcheck::stricmp>::const_iterator it1 = variables.find(var);
        // variable was not found within defined variables
        if (it1 == variables.end()) {
            const char *envValue = std::getenv(var.c_str());
            if (!envValue) {
                //! \todo generate a debug/info message about undefined variable
                break;
            }
            variables[var] = std::string(envValue);
            it1 = variables.find(var);
        }
        s.replace(start, end - start + 1, it1->second);
    }
    if (s.find("$(") != std::string::npos)
        return false;
    s = Path::simplifyPath(Path::fromNativeSeparators(s));
    return true;
}

void ImportProject::FileSettings::setIncludePaths(const std::string &basepath, const std::list<std::string> &in, std::map<std::string, std::string, cppcheck::stricmp> &variables)
{
    std::set<std::string> found;
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    const std::list<std::string> copyIn(in);
    includePaths.clear();
    for (const std::string &ipath : copyIn) {
        if (ipath.empty())
            continue;
        if (ipath.compare(0,2,"%(")==0)
            continue;
        std::string s(Path::fromNativeSeparators(ipath));
        if (!found.insert(s).second)
            continue;
        if (s[0] == '/' || (s.size() > 1U && s.compare(1,2,":/") == 0)) {
            if (!endsWith(s,'/'))
                s += '/';
            includePaths.push_back(std::move(s));
            continue;
        }

        if (endsWith(s,'/')) // this is a temporary hack, simplifyPath can crash if path ends with '/'
            s.erase(s.size() - 1U); // TODO: Use std::string::pop_back() as soon as travis supports it

        if (s.find("$(") == std::string::npos) {
            s = Path::simplifyPath(basepath + s);
        } else {
            if (!simplifyPathWithVariables(s, variables))
                continue;
        }
        if (s.empty())
            continue;
        includePaths.push_back(s + '/');
    }
}

ImportProject::Type ImportProject::import(const std::string &filename, Settings *settings)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
        return ImportProject::Type::MISSING;

    mPath = Path::getPathFromFilename(Path::fromNativeSeparators(filename));
    if (!mPath.empty() && !endsWith(mPath,'/'))
        mPath += '/';

    const std::vector<std::string> fileFilters =
        settings ? settings->fileFilters : std::vector<std::string>();

    if (endsWith(filename, ".json")) {
        if (importCompileCommands(fin)) {
            setRelativePaths(filename);
            return ImportProject::Type::COMPILE_DB;
        }
    } else if (endsWith(filename, ".sln")) {
        if (importSln(fin, mPath, fileFilters)) {
            setRelativePaths(filename);
            return ImportProject::Type::VS_SLN;
        }
    } else if (endsWith(filename, ".vcxproj")) {
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        if (importVcxproj(filename, variables, emptyString, fileFilters)) {
            setRelativePaths(filename);
            return ImportProject::Type::VS_VCXPROJ;
        }
    } else if (endsWith(filename, ".bpr")) {
        if (importBcb6Prj(filename)) {
            setRelativePaths(filename);
            return ImportProject::Type::BORLAND;
        }
    } else if (settings && endsWith(filename, ".cppcheck")) {
        if (importCppcheckGuiProject(fin, settings)) {
            setRelativePaths(filename);
            return ImportProject::Type::CPPCHECK_GUI;
        }
    } else {
        return ImportProject::Type::UNKNOWN;
    }
    return ImportProject::Type::FAILURE;
}

static std::string readUntil(const std::string &command, std::string::size_type *pos, const char until[])
{
    std::string ret;
    bool escapedString = false;
    bool str = false;
    bool escape = false;
    for (; *pos < command.size() && (str || !std::strchr(until, command[*pos])); (*pos)++) {
        if (escape)
            escape = false;
        else if (command[*pos] == '\\') {
            if (str)
                escape = true;
            else if (command[*pos + 1] == '"') {
                if (escapedString)
                    return ret + "\\\"";
                escapedString = true;
                ret += "\\\"";
                (*pos)++;
                continue;
            }
        } else if (command[*pos] == '\"')
            str = !str;
        ret += command[*pos];
    }
    return ret;
}

static std::string unescape(const std::string &in)
{
    std::string out;
    bool escape = false;
    for (const char c: in) {
        if (escape) {
            escape = false;
            if (!std::strchr("\\\"\'",c))
                out += "\\";
            out += c;
        } else if (c == '\\')
            escape = true;
        else
            out += c;
    }
    return out;
}

void ImportProject::FileSettings::parseCommand(const std::string& command)
{
    std::string defs;

    // Parse command..
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = command.find(' ',pos))) {
        while (pos < command.size() && command[pos] == ' ')
            pos++;
        if (pos >= command.size())
            break;
        if (command[pos] != '/' && command[pos] != '-')
            continue;
        pos++;
        if (pos >= command.size())
            break;
        const char F = command[pos++];
        if (std::strchr("DUI", F)) {
            while (pos < command.size() && command[pos] == ' ')
                ++pos;
        }
        const std::string fval = readUntil(command, &pos, " =");
        if (F=='D') {
            std::string defval = readUntil(command, &pos, " ");
            defs += fval;
            if (defval.size() >= 3 && defval.compare(0,2,"=\"")==0 && defval.back()=='\"')
                defval = "=" + unescape(defval.substr(2, defval.size() - 3));
            else if (defval.size() >= 5 && defval.compare(0, 3, "=\\\"") == 0 && endsWith(defval, "\\\""))
                defval = "=\"" + unescape(defval.substr(3, defval.size() - 5)) + "\"";
            if (!defval.empty())
                defs += defval;
            defs += ';';
        } else if (F=='U')
            undefs.insert(fval);
        else if (F=='I') {
            std::string i = fval;
            if (i.size() > 1 && i[0] == '\"' && i.back() == '\"')
                i = unescape(i.substr(1, i.size() - 2));
            if (std::find(includePaths.cbegin(), includePaths.cend(), i) == includePaths.cend())
                includePaths.push_back(std::move(i));
        } else if (F=='s' && fval.compare(0,2,"td") == 0) {
            ++pos;
            const std::string stdval = readUntil(command, &pos, " ");
            standard = stdval;
            // TODO: use simplecpp::DUI::std instead of specifying it manually
            if (standard.compare(0, 3, "c++") || standard.compare(0, 5, "gnu++")) {
                const std::string stddef = simplecpp::getCppStdString(standard);
                if (stddef.empty()) {
                    // TODO: log error
                    continue;
                }

                defs += "__cplusplus=";
                defs += stddef;
                defs += ";";
            } else if (standard.compare(0, 1, "c") || standard.compare(0, 3, "gnu")) {
                const std::string stddef = simplecpp::getCStdString(standard);
                if (stddef.empty()) {
                    // TODO: log error
                    continue;
                }

                defs += "__STDC_VERSION__=";
                defs += stddef;
                defs += ";";
            }
        } else if (F == 'i' && fval == "system") {
            ++pos;
            std::string isystem = readUntil(command, &pos, " ");
            systemIncludePaths.push_back(std::move(isystem));
        } else if (F=='m') {
            if (fval == "unicode") {
                defs += "UNICODE";
                defs += ";";
            }
        } else if (F=='f') {
            if (fval == "pic") {
                defs += "__pic__";
                defs += ";";
            } else if (fval == "PIC") {
                defs += "__PIC__";
                defs += ";";
            } else if (fval == "pie") {
                defs += "__pie__";
                defs += ";";
            } else if (fval == "PIE") {
                defs += "__PIE__";
                defs += ";";
            }
        }
    }
    setDefines(defs);
}

bool ImportProject::importCompileCommands(std::istream &istr)
{
    picojson::value compileCommands;
    istr >> compileCommands;
    if (!compileCommands.is<picojson::array>()) {
        printError("compilation database is not a JSON array");
        return false;
    }

    for (const picojson::value &fileInfo : compileCommands.get<picojson::array>()) {
        picojson::object obj = fileInfo.get<picojson::object>();
        std::string dirpath = Path::fromNativeSeparators(obj["directory"].get<std::string>());

        /* CMAKE produces the directory without trailing / so add it if not
         * there - it is needed by setIncludePaths() */
        if (!endsWith(dirpath, '/'))
            dirpath += '/';

        const std::string directory = dirpath;

        std::string command;
        if (obj.count("arguments")) {
            if (obj["arguments"].is<picojson::array>()) {
                for (const picojson::value& arg : obj["arguments"].get<picojson::array>()) {
                    if (arg.is<std::string>()) {
                        std::string str = arg.get<std::string>();
                        if (str.find(' ') != std::string::npos)
                            str = "\"" + str + "\"";
                        command += str + " ";
                    }
                }
            } else {
                printError("'arguments' field in compilation database entry is not a JSON array");
                return false;
            }
        } else if (obj.count("command")) {
            if (obj["command"].is<std::string>()) {
                command = obj["command"].get<std::string>();
            } else {
                printError("'command' field in compilation database entry is not a string");
                return false;
            }
        } else {
            printError("no 'arguments' or 'command' field found in compilation database entry");
            return false;
        }

        if (!obj.count("file") || !obj["file"].is<std::string>()) {
            printError("skip compilation database entry because it does not have a proper 'file' field");
            continue;
        }

        const std::string file = Path::fromNativeSeparators(obj["file"].get<std::string>());

        // Accept file?
        if (!Path::acceptFile(file))
            continue;

        struct FileSettings fs;
        if (Path::isAbsolute(file))
            fs.filename = Path::simplifyPath(file);
#ifdef _WIN32
        else if (file[0] == '/' && directory.size() > 2 && std::isalpha(directory[0]) && directory[1] == ':')
            // directory: C:\foo\bar
            // file: /xy/z.c
            // => c:/xy/z.c
            fs.filename = Path::simplifyPath(directory.substr(0,2) + file);
#endif
        else
            fs.filename = Path::simplifyPath(directory + file);
        if (!sourceFileExists(fs.filename)) {
            printError("'" + fs.filename + "' from compilation database does not exist");
            return false;
        }
        fs.parseCommand(command); // read settings; -D, -I, -U, -std, -m*, -f*
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        fs.setIncludePaths(directory, fs.includePaths, variables);
        fileSettings.push_back(std::move(fs));
    }

    return true;
}

bool ImportProject::importSln(std::istream &istr, const std::string &path, const std::vector<std::string> &fileFilters)
{
    std::string line;

    if (!std::getline(istr,line)) {
        printError("Visual Studio solution file is empty");
        return false;
    }

    if (line.find("Microsoft Visual Studio Solution File") != 0) {
        // Skip BOM
        if (!std::getline(istr, line) || line.find("Microsoft Visual Studio Solution File") != 0) {
            printError("Visual Studio solution file header not found");
            return false;
        }
    }

    std::map<std::string,std::string,cppcheck::stricmp> variables;
    variables["SolutionDir"] = path;

    bool found = false;

    while (std::getline(istr,line)) {
        if (line.compare(0,8,"Project(")!=0)
            continue;
        const std::string::size_type pos = line.find(".vcxproj");
        if (pos == std::string::npos)
            continue;
        const std::string::size_type pos1 = line.rfind('\"',pos);
        if (pos1 == std::string::npos)
            continue;
        std::string vcxproj(line.substr(pos1+1, pos-pos1+7));
        if (!Path::isAbsolute(vcxproj))
            vcxproj = path + vcxproj;
        if (!importVcxproj(Path::fromNativeSeparators(vcxproj), variables, emptyString, fileFilters)) {
            printError("failed to load '" + vcxproj + "' from Visual Studio solution");
            return false;
        }
        found = true;
    }

    if (!found) {
        printError("no projects found in Visual Studio solution file");
        return false;
    }

    return true;
}

namespace {
    struct ProjectConfiguration {
        explicit ProjectConfiguration(const tinyxml2::XMLElement *cfg) {
            const char *a = cfg->Attribute("Include");
            if (a)
                name = a;
            for (const tinyxml2::XMLElement *e = cfg->FirstChildElement(); e; e = e->NextSiblingElement()) {
                if (!e->GetText())
                    continue;
                if (std::strcmp(e->Name(),"Configuration")==0)
                    configuration = e->GetText();
                else if (std::strcmp(e->Name(),"Platform")==0) {
                    platformStr = e->GetText();
                    if (platformStr == "Win32")
                        platform = Win32;
                    else if (platformStr == "x64")
                        platform = x64;
                    else
                        platform = Unknown;
                }
            }
        }
        std::string name;
        std::string configuration;
        enum { Win32, x64, Unknown } platform = Unknown;
        std::string platformStr;
    };

    struct ItemDefinitionGroup {
        explicit ItemDefinitionGroup(const tinyxml2::XMLElement *idg, std::string includePaths) : additionalIncludePaths(std::move(includePaths)) {
            const char *condAttr = idg->Attribute("Condition");
            if (condAttr)
                condition = condAttr;
            for (const tinyxml2::XMLElement *e1 = idg->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
                if (std::strcmp(e1->Name(), "ClCompile") == 0) {
                    enhancedInstructionSet = "StreamingSIMDExtensions2";
                    for (const tinyxml2::XMLElement *e = e1->FirstChildElement(); e; e = e->NextSiblingElement()) {
                        if (e->GetText()) {
                            if (std::strcmp(e->Name(), "PreprocessorDefinitions") == 0)
                                preprocessorDefinitions = e->GetText();
                            else if (std::strcmp(e->Name(), "AdditionalIncludeDirectories") == 0) {
                                if (!additionalIncludePaths.empty())
                                    additionalIncludePaths += ';';
                                additionalIncludePaths += e->GetText();
                            } else if (std::strcmp(e->Name(), "LanguageStandard") == 0) {
                                if (std::strcmp(e->GetText(), "stdcpp14") == 0)
                                    cppstd = Standards::CPP14;
                                else if (std::strcmp(e->GetText(), "stdcpp17") == 0)
                                    cppstd = Standards::CPP17;
                                else if (std::strcmp(e->GetText(), "stdcpp20") == 0)
                                    cppstd = Standards::CPP20;
                                else if (std::strcmp(e->GetText(), "stdcpplatest") == 0)
                                    cppstd = Standards::CPPLatest;
                            } else if (std::strcmp(e->Name(), "EnableEnhancedInstructionSet") == 0) {
                                enhancedInstructionSet = e->GetText();
                            }
                        }
                    }
                }
                else if (std::strcmp(e1->Name(), "Link") == 0) {
                    for (const tinyxml2::XMLElement *e = e1->FirstChildElement(); e; e = e->NextSiblingElement()) {
                        if (!e->GetText())
                            continue;
                        if (std::strcmp(e->Name(), "EntryPointSymbol") == 0) {
                            entryPointSymbol = e->GetText();
                        }
                    }
                }
            }
        }

        static void replaceAll(std::string &c, const std::string &from, const std::string &to) {
            std::string::size_type pos;
            while ((pos = c.find(from)) != std::string::npos) {
                c.erase(pos,from.size());
                c.insert(pos,to);
            }
        }

        bool conditionIsTrue(const ProjectConfiguration &p) const {
            if (condition.empty())
                return true;
            std::string c = '(' + condition + ");";
            replaceAll(c, "$(Configuration)", p.configuration);
            replaceAll(c, "$(Platform)", p.platformStr);

            // TODO : Better evaluation
            Settings s;
            std::istringstream istr(c);
            Tokenizer tokenizer(&s);
            tokenizer.tokenize(istr,"vcxproj");
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
                if (tok->str() == "(" && tok->astOperand1() && tok->astOperand2()) {
                    if (tok->astOperand1()->expressionString() == "Configuration.Contains")
                        return ('\'' + p.configuration + '\'') == tok->astOperand2()->str();
                }
                if (tok->str() == "==" && tok->astOperand1() && tok->astOperand2() && tok->astOperand1()->str() == tok->astOperand2()->str())
                    return true;
            }
            return false;
        }
        std::string condition;
        std::string enhancedInstructionSet;
        std::string preprocessorDefinitions;
        std::string additionalIncludePaths;
        std::string entryPointSymbol; // TODO: use this
        Standards::cppstd_t cppstd = Standards::CPPLatest;
    };
}

static std::list<std::string> toStringList(const std::string &s)
{
    std::list<std::string> ret;
    std::string::size_type pos1 = 0;
    std::string::size_type pos2;
    while ((pos2 = s.find(';',pos1)) != std::string::npos) {
        ret.push_back(s.substr(pos1, pos2-pos1));
        pos1 = pos2 + 1;
        if (pos1 >= s.size())
            break;
    }
    if (pos1 < s.size())
        ret.push_back(s.substr(pos1));
    return ret;
}

static void importPropertyGroup(const tinyxml2::XMLElement *node, std::map<std::string,std::string,cppcheck::stricmp> &variables, std::string &includePath, bool *useOfMfc)
{
    if (useOfMfc) {
        for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "UseOfMfc") == 0) {
                *useOfMfc = true;
                break;
            }
        }
    }

    const char* labelAttribute = node->Attribute("Label");
    if (labelAttribute && std::strcmp(labelAttribute, "UserMacros") == 0) {
        for (const tinyxml2::XMLElement *propertyGroup = node->FirstChildElement(); propertyGroup; propertyGroup = propertyGroup->NextSiblingElement()) {
            const std::string name(propertyGroup->Name());
            const char *text = propertyGroup->GetText();
            variables[name] = std::string(text ? text : "");
        }

    } else if (!labelAttribute) {
        for (const tinyxml2::XMLElement *propertyGroup = node->FirstChildElement(); propertyGroup; propertyGroup = propertyGroup->NextSiblingElement()) {
            if (std::strcmp(propertyGroup->Name(), "IncludePath") != 0)
                continue;
            const char *text = propertyGroup->GetText();
            if (!text)
                continue;
            std::string path(text);
            const std::string::size_type pos = path.find("$(IncludePath)");
            if (pos != std::string::npos)
                path.replace(pos, 14U, includePath);
            includePath = path;
        }
    }
}

static void loadVisualStudioProperties(const std::string &props, std::map<std::string,std::string,cppcheck::stricmp> &variables, std::string &includePath, const std::string &additionalIncludeDirectories, std::list<ItemDefinitionGroup> &itemDefinitionGroupList)
{
    std::string filename(props);
    // variables can't be resolved
    if (!simplifyPathWithVariables(filename, variables))
        return;

    // prepend project dir (if it exists) to transform relative paths into absolute ones
    if (!Path::isAbsolute(filename) && variables.count("ProjectDir") > 0)
        filename = Path::getAbsoluteFilePath(variables.at("ProjectDir") + filename);

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS)
        return;
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr)
        return;
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (std::strcmp(node->Name(), "ImportGroup") == 0) {
            const char *labelAttribute = node->Attribute("Label");
            if (labelAttribute == nullptr || std::strcmp(labelAttribute, "PropertySheets") != 0)
                continue;
            for (const tinyxml2::XMLElement *importGroup = node->FirstChildElement(); importGroup; importGroup = importGroup->NextSiblingElement()) {
                if (std::strcmp(importGroup->Name(), "Import") == 0) {
                    const char *projectAttribute = importGroup->Attribute("Project");
                    if (projectAttribute == nullptr)
                        continue;
                    std::string loadprj(projectAttribute);
                    if (loadprj.find('$') == std::string::npos) {
                        loadprj = Path::getPathFromFilename(filename) + loadprj;
                    }
                    loadVisualStudioProperties(loadprj, variables, includePath, additionalIncludeDirectories, itemDefinitionGroupList);
                }
            }
        } else if (std::strcmp(node->Name(),"PropertyGroup")==0) {
            importPropertyGroup(node, variables, includePath, nullptr);
        } else if (std::strcmp(node->Name(),"ItemDefinitionGroup")==0) {
            itemDefinitionGroupList.emplace_back(node, additionalIncludeDirectories);
        }
    }
}

bool ImportProject::importVcxproj(const std::string &filename, std::map<std::string, std::string, cppcheck::stricmp> &variables, const std::string &additionalIncludeDirectories, const std::vector<std::string> &fileFilters)
{
    variables["ProjectDir"] = Path::simplifyPath(Path::getPathFromFilename(filename));

    std::list<ProjectConfiguration> projectConfigurationList;
    std::list<std::string> compileList;
    std::list<ItemDefinitionGroup> itemDefinitionGroupList;
    std::string includePath;

    bool useOfMfc = false;

    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
    if (error != tinyxml2::XML_SUCCESS) {
        printError(std::string("Visual Studio project file is not a valid XML - ") + tinyxml2::XMLDocument::ErrorIDToName(error));
        return false;
    }
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr) {
        printError("Visual Studio project file has no XML root node");
        return false;
    }
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (std::strcmp(node->Name(), "ItemGroup") == 0) {
            const char *labelAttribute = node->Attribute("Label");
            if (labelAttribute && std::strcmp(labelAttribute, "ProjectConfigurations") == 0) {
                for (const tinyxml2::XMLElement *cfg = node->FirstChildElement(); cfg; cfg = cfg->NextSiblingElement()) {
                    if (std::strcmp(cfg->Name(), "ProjectConfiguration") == 0) {
                        const ProjectConfiguration p(cfg);
                        if (p.platform != ProjectConfiguration::Unknown) {
                            projectConfigurationList.emplace_back(cfg);
                            mAllVSConfigs.insert(p.configuration);
                        }
                    }
                }
            } else {
                for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                    if (std::strcmp(e->Name(), "ClCompile") == 0) {
                        const char *include = e->Attribute("Include");
                        if (include && Path::acceptFile(include))
                            compileList.emplace_back(include);
                    }
                }
            }
        } else if (std::strcmp(node->Name(), "ItemDefinitionGroup") == 0) {
            itemDefinitionGroupList.emplace_back(node, additionalIncludeDirectories);
        } else if (std::strcmp(node->Name(), "PropertyGroup") == 0) {
            importPropertyGroup(node, variables, includePath, &useOfMfc);
        } else if (std::strcmp(node->Name(), "ImportGroup") == 0) {
            const char *labelAttribute = node->Attribute("Label");
            if (labelAttribute && std::strcmp(labelAttribute, "PropertySheets") == 0) {
                for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                    if (std::strcmp(e->Name(), "Import") == 0) {
                        const char *projectAttribute = e->Attribute("Project");
                        if (projectAttribute)
                            loadVisualStudioProperties(projectAttribute, variables, includePath, additionalIncludeDirectories, itemDefinitionGroupList);
                    }
                }
            }
        }
    }

    for (const std::string &c : compileList) {
        const std::string cfilename = Path::simplifyPath(Path::isAbsolute(c) ? c : Path::getPathFromFilename(filename) + c);
        if (!fileFilters.empty() && !matchglobs(fileFilters, cfilename))
            continue;

        for (const ProjectConfiguration &p : projectConfigurationList) {

            if (!guiProject.checkVsConfigs.empty()) {
                const bool doChecking = std::any_of(guiProject.checkVsConfigs.cbegin(), guiProject.checkVsConfigs.cend(), [&](const std::string& c) {
                    return c == p.configuration;
                });
                if (!doChecking)
                    continue;
            }

            FileSettings fs;
            fs.filename = cfilename;
            fs.cfg = p.name;
            fs.msc = true;
            fs.useMfc = useOfMfc;
            fs.defines = "_WIN32=1";
            if (p.platform == ProjectConfiguration::Win32)
                fs.platformType = cppcheck::Platform::Type::Win32W;
            else if (p.platform == ProjectConfiguration::x64) {
                fs.platformType = cppcheck::Platform::Type::Win64;
                fs.defines += ";_WIN64=1";
            }
            std::string additionalIncludePaths;
            for (const ItemDefinitionGroup &i : itemDefinitionGroupList) {
                if (!i.conditionIsTrue(p))
                    continue;
                fs.standard = Standards::getCPP(i.cppstd);
                fs.defines += ';' + i.preprocessorDefinitions;
                if (i.enhancedInstructionSet == "StreamingSIMDExtensions")
                    fs.defines += ";__SSE__";
                else if (i.enhancedInstructionSet == "StreamingSIMDExtensions2")
                    fs.defines += ";__SSE2__";
                else if (i.enhancedInstructionSet == "AdvancedVectorExtensions")
                    fs.defines += ";__AVX__";
                else if (i.enhancedInstructionSet == "AdvancedVectorExtensions2")
                    fs.defines += ";__AVX2__";
                else if (i.enhancedInstructionSet == "AdvancedVectorExtensions512")
                    fs.defines += ";__AVX512__";
                additionalIncludePaths += ';' + i.additionalIncludePaths;
            }
            fs.setDefines(fs.defines);
            fs.setIncludePaths(Path::getPathFromFilename(filename), toStringList(includePath + ';' + additionalIncludePaths), variables);
            fileSettings.push_back(std::move(fs));
        }
    }

    return true;
}

bool ImportProject::importBcb6Prj(const std::string &projectFilename)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(projectFilename.c_str());
    if (error != tinyxml2::XML_SUCCESS) {
        printError(std::string("Borland project file is not a valid XML - ") + tinyxml2::XMLDocument::ErrorIDToName(error));
        return false;
    }
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr) {
        printError("Borland project file has no XML root node");
        return false;
    }

    const std::string& projectDir = Path::simplifyPath(Path::getPathFromFilename(projectFilename));

    std::list<std::string> compileList;
    std::string includePath;
    std::string userdefines;
    std::string sysdefines;
    std::string cflag1;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (std::strcmp(node->Name(), "FILELIST") == 0) {
            for (const tinyxml2::XMLElement *f = node->FirstChildElement(); f; f = f->NextSiblingElement()) {
                if (std::strcmp(f->Name(), "FILE") == 0) {
                    const char *filename = f->Attribute("FILENAME");
                    if (filename && Path::acceptFile(filename))
                        compileList.emplace_back(filename);
                }
            }
        } else if (std::strcmp(node->Name(), "MACROS") == 0) {
            for (const tinyxml2::XMLElement *m = node->FirstChildElement(); m; m = m->NextSiblingElement()) {
                if (std::strcmp(m->Name(), "INCLUDEPATH") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        includePath = v;
                } else if (std::strcmp(m->Name(), "USERDEFINES") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        userdefines = v;
                } else if (std::strcmp(m->Name(), "SYSDEFINES") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        sysdefines = v;
                }
            }
        } else if (std::strcmp(node->Name(), "OPTIONS") == 0) {
            for (const tinyxml2::XMLElement *m = node->FirstChildElement(); m; m = m->NextSiblingElement()) {
                if (std::strcmp(m->Name(), "CFLAG1") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        cflag1 = v;
                }
            }
        }
    }

    std::set<std::string> cflags;

    // parse cflag1 and fill the cflags set
    {
        std::string arg;

        for (const char i : cflag1) {
            if (i == ' ' && !arg.empty()) {
                cflags.insert(arg);
                arg.clear();
                continue;
            }
            arg += i;
        }

        if (!arg.empty()) {
            cflags.insert(arg);
        }

        // cleanup: -t is "An alternate name for the -Wxxx switches; there is no difference"
        // -> Remove every known -txxx argument and replace it with its -Wxxx counterpart.
        //    This way, we know what we have to check for later on.
        static const std::map<std::string, std::string> synonyms = {
            { "-tC","-WC" },
            { "-tCDR","-WCDR" },
            { "-tCDV","-WCDV" },
            { "-tW","-W" },
            { "-tWC","-WC" },
            { "-tWCDR","-WCDR" },
            { "-tWCDV","-WCDV" },
            { "-tWD","-WD" },
            { "-tWDR","-WDR" },
            { "-tWDV","-WDV" },
            { "-tWM","-WM" },
            { "-tWP","-WP" },
            { "-tWR","-WR" },
            { "-tWU","-WU" },
            { "-tWV","-WV" }
        };

        for (std::map<std::string, std::string>::const_iterator i = synonyms.cbegin(); i != synonyms.cend(); ++i) {
            if (cflags.erase(i->first) > 0) {
                cflags.insert(i->second);
            }
        }
    }

    std::string predefines;
    std::string cppPredefines;

    // Collecting predefines. See BCB6 help topic "Predefined macros"
    {
        cppPredefines +=
            // Defined if you've selected C++ compilation; will increase in later releases.
            // value 0x0560 (but 0x0564 for our BCB6 SP4)
            // @see http://docwiki.embarcadero.com/RADStudio/Tokyo/en/Predefined_Macros#C.2B.2B_Compiler_Versions_in_Predefined_Macros
            ";__BCPLUSPLUS__=0x0560"

            // Defined if in C++ mode; otherwise, undefined.
            ";__cplusplus=1"

            // Defined as 1 for C++ files(meaning that templates are supported); otherwise, it is undefined.
            ";__TEMPLATES__=1"

            // Defined only for C++ programs to indicate that wchar_t is an intrinsically defined data type.
            ";_WCHAR_T"

            // Defined only for C++ programs to indicate that wchar_t is an intrinsically defined data type.
            ";_WCHAR_T_DEFINED"

            // Defined in any compiler that has an optimizer.
            ";__BCOPT__=1"

            // Version number.
            // BCB6 is 0x056X (SP4 is 0x0564)
            // @see http://docwiki.embarcadero.com/RADStudio/Tokyo/en/Predefined_Macros#C.2B.2B_Compiler_Versions_in_Predefined_Macros
            ";__BORLANDC__=0x0560"
            ";__TCPLUSPLUS__=0x0560"
            ";__TURBOC__=0x0560";

        // Defined if Calling Convention is set to cdecl; otherwise undefined.
        const bool useCdecl = (cflags.find("-p") == cflags.end()
                               && cflags.find("-pm") == cflags.end()
                               && cflags.find("-pr") == cflags.end()
                               && cflags.find("-ps") == cflags.end());
        if (useCdecl)
            predefines += ";__CDECL=1";

        // Defined by default indicating that the default char is unsigned char. Use the -K compiler option to undefine this macro.
        const bool treatCharAsUnsignedChar = (cflags.find("-K") != cflags.end());
        if (treatCharAsUnsignedChar)
            predefines += ";_CHAR_UNSIGNED=1";

        // Defined whenever one of the CodeGuard compiler options is used; otherwise it is undefined.
        const bool codeguardUsed = (cflags.find("-vGd") != cflags.end()
                                    || cflags.find("-vGt") != cflags.end()
                                    || cflags.find("-vGc") != cflags.end());
        if (codeguardUsed)
            predefines += ";__CODEGUARD__";

        // When defined, the macro indicates that the program is a console application.
        const bool isConsoleApp = (cflags.find("-WC") != cflags.end());
        if (isConsoleApp)
            predefines += ";__CONSOLE__=1";

        // Enable stack unwinding. This is true by default; use -xd- to disable.
        const bool enableStackUnwinding = (cflags.find("-xd-") == cflags.end());
        if (enableStackUnwinding)
            predefines += ";_CPPUNWIND=1";

        // Defined whenever the -WD compiler option is used; otherwise it is undefined.
        const bool isDLL = (cflags.find("-WD") != cflags.end());
        if (isDLL)
            predefines += ";__DLL__=1";

        // Defined when compiling in 32-bit flat memory model.
        // TODO: not sure how to switch to another memory model or how to read configuration from project file
        predefines += ";__FLAT__=1";

        // Always defined. The default value is 300. You can change the value to 400 or 500 by using the /4 or /5 compiler options.
        if (cflags.find("-6") != cflags.end())
            predefines += ";_M_IX86=600";
        else if (cflags.find("-5") != cflags.end())
            predefines += ";_M_IX86=500";
        else if (cflags.find("-4") != cflags.end())
            predefines += ";_M_IX86=400";
        else
            predefines += ";_M_IX86=300";

        // Defined only if the -WM option is used. It specifies that the multithread library is to be linked.
        const bool linkMtLib = (cflags.find("-WM") != cflags.end());
        if (linkMtLib)
            predefines += ";__MT__=1";

        // Defined if Calling Convention is set to Pascal; otherwise undefined.
        const bool usePascalCallingConvention = (cflags.find("-p") != cflags.end());
        if (usePascalCallingConvention)
            predefines += ";__PASCAL__=1";

        // Defined if you compile with the -A compiler option; otherwise, it is undefined.
        const bool useAnsiKeywordExtensions = (cflags.find("-A") != cflags.end());
        if (useAnsiKeywordExtensions)
            predefines += ";__STDC__=1";

        // Thread Local Storage. Always true in C++Builder.
        predefines += ";__TLC__=1";

        // Defined for Windows-only code.
        const bool isWindowsTarget = (cflags.find("-WC") != cflags.end()
                                      || cflags.find("-WCDR") != cflags.end()
                                      || cflags.find("-WCDV") != cflags.end()
                                      || cflags.find("-WD") != cflags.end()
                                      || cflags.find("-WDR") != cflags.end()
                                      || cflags.find("-WDV") != cflags.end()
                                      || cflags.find("-WM") != cflags.end()
                                      || cflags.find("-WP") != cflags.end()
                                      || cflags.find("-WR") != cflags.end()
                                      || cflags.find("-WU") != cflags.end()
                                      || cflags.find("-WV") != cflags.end());
        if (isWindowsTarget)
            predefines += ";_Windows";

        // Defined for console and GUI applications.
        // TODO: I'm not sure about the difference to define "_Windows".
        //       From description, I would assume __WIN32__ is only defined for
        //       executables, while _Windows would also be defined for DLLs, etc.
        //       However, in a newly created DLL project, both __WIN32__ and
        //       _Windows are defined. -> treating them the same for now.
        //       Also boost uses __WIN32__ for OS identification.
        const bool isConsoleOrGuiApp = isWindowsTarget;
        if (isConsoleOrGuiApp)
            predefines += ";__WIN32__=1";
    }

    // Include paths may contain variables like "$(BCB)\include" or "$(BCB)\include\vcl".
    // Those get resolved by ImportProject::FileSettings::setIncludePaths by
    // 1. checking the provided variables map ("BCB" => "C:\\Program Files (x86)\\Borland\\CBuilder6")
    // 2. checking env variables as a fallback
    // Setting env is always possible. Configuring the variables via cli might be an addition.
    // Reading the BCB6 install location from registry in windows environments would also be possible,
    // but I didn't see any such functionality around the source. Not in favor of adding it only
    // for the BCB6 project loading.
    std::map<std::string, std::string, cppcheck::stricmp> variables;
    const std::string defines = predefines + ";" + sysdefines + ";" + userdefines;
    const std::string cppDefines  = cppPredefines + ";" + defines;
    const bool forceCppMode = (cflags.find("-P") != cflags.end());

    for (const std::string &c : compileList) {
        // C++ compilation is selected by file extension by default, so these
        // defines have to be configured on a per-file base.
        //
        // > Files with the .CPP extension compile as C++ files. Files with a .C
        // > extension, with no extension, or with extensions other than .CPP,
        // > .OBJ, .LIB, or .ASM compile as C files.
        // (http://docwiki.embarcadero.com/RADStudio/Tokyo/en/BCC32.EXE,_the_C%2B%2B_32-bit_Command-Line_Compiler)
        //
        // We can also force C++ compilation for all files using the -P command line switch.
        const bool cppMode = forceCppMode || Path::getFilenameExtensionInLowerCase(c) == ".cpp";
        FileSettings fs;
        fs.setIncludePaths(projectDir, toStringList(includePath), variables);
        fs.setDefines(cppMode ? cppDefines : defines);
        fs.filename = Path::simplifyPath(Path::isAbsolute(c) ? c : projectDir + c);
        fileSettings.push_back(std::move(fs));
    }

    return true;
}

static std::string joinRelativePath(const std::string &path1, const std::string &path2)
{
    if (!path1.empty() && !Path::isAbsolute(path2))
        return path1 + path2;
    return path2;
}

static std::list<std::string> readXmlStringList(const tinyxml2::XMLElement *node, const std::string &path, const char name[], const char attribute[])
{
    std::list<std::string> ret;
    for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
        if (strcmp(child->Name(), name) != 0)
            continue;
        const char *attr = attribute ? child->Attribute(attribute) : child->GetText();
        if (attr)
            ret.push_back(joinRelativePath(path, attr));
    }
    return ret;
}

static std::string join(const std::list<std::string> &strlist, const char *sep)
{
    std::string ret;
    for (const std::string &s : strlist) {
        ret += (ret.empty() ? "" : sep) + s;
    }
    return ret;
}

static std::string istream_to_string(std::istream &istr)
{
    std::istreambuf_iterator<char> eos;
    return std::string(std::istreambuf_iterator<char>(istr), eos);
}

static const char * readSafe(const char *s, const char *def) {
    return s ? s : def;
}

bool ImportProject::importCppcheckGuiProject(std::istream &istr, Settings *settings)
{
    tinyxml2::XMLDocument doc;
    const std::string xmldata = istream_to_string(istr);
    const tinyxml2::XMLError error = doc.Parse(xmldata.data(), xmldata.size());
    if (error != tinyxml2::XML_SUCCESS) {
        printError(std::string("Cppcheck GUI project file is not a valid XML - ") + tinyxml2::XMLDocument::ErrorIDToName(error));
        return false;
    }
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr || strcmp(rootnode->Name(), CppcheckXml::ProjectElementName) != 0) {
        printError("Cppcheck GUI project file has no XML root node");
        return false;
    }

    const std::string &path = mPath;

    std::list<std::string> paths;
    std::list<Suppressions::Suppression> suppressions;
    Settings temp;

    guiProject.analyzeAllVsConfigs.clear();

    bool checkLevelExhaustive = false;

    // TODO: this should support all available command-line options
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (strcmp(node->Name(), CppcheckXml::RootPathName) == 0) {
            if (node->Attribute(CppcheckXml::RootPathNameAttrib)) {
                temp.basePaths.push_back(joinRelativePath(path, node->Attribute(CppcheckXml::RootPathNameAttrib)));
                temp.relativePaths = true;
            }
        } else if (strcmp(node->Name(), CppcheckXml::BuildDirElementName) == 0)
            temp.buildDir = joinRelativePath(path, readSafe(node->GetText(), ""));
        else if (strcmp(node->Name(), CppcheckXml::IncludeDirElementName) == 0)
            temp.includePaths = readXmlStringList(node, path, CppcheckXml::DirElementName, CppcheckXml::DirNameAttrib);
        else if (strcmp(node->Name(), CppcheckXml::DefinesElementName) == 0)
            temp.userDefines = join(readXmlStringList(node, "", CppcheckXml::DefineName, CppcheckXml::DefineNameAttrib), ";");
        else if (strcmp(node->Name(), CppcheckXml::UndefinesElementName) == 0) {
            for (const std::string &u : readXmlStringList(node, "", CppcheckXml::UndefineName, nullptr))
                temp.userUndefs.insert(u);
        } else if (strcmp(node->Name(), CppcheckXml::ImportProjectElementName) == 0) {
            const std::string t_str = readSafe(node->GetText(), "");
            if (!t_str.empty())
                guiProject.projectFile = path + t_str;
        }
        else if (strcmp(node->Name(), CppcheckXml::PathsElementName) == 0)
            paths = readXmlStringList(node, path, CppcheckXml::PathName, CppcheckXml::PathNameAttrib);
        else if (strcmp(node->Name(), CppcheckXml::ExcludeElementName) == 0)
            guiProject.excludedPaths = readXmlStringList(node, "", CppcheckXml::ExcludePathName, CppcheckXml::ExcludePathNameAttrib);
        else if (strcmp(node->Name(), CppcheckXml::FunctionContracts) == 0)
            ;
        else if (strcmp(node->Name(), CppcheckXml::VariableContractsElementName) == 0)
            ;
        else if (strcmp(node->Name(), CppcheckXml::IgnoreElementName) == 0)
            guiProject.excludedPaths = readXmlStringList(node, "", CppcheckXml::IgnorePathName, CppcheckXml::IgnorePathNameAttrib);
        else if (strcmp(node->Name(), CppcheckXml::LibrariesElementName) == 0)
            guiProject.libraries = readXmlStringList(node, "", CppcheckXml::LibraryElementName, nullptr);
        else if (strcmp(node->Name(), CppcheckXml::SuppressionsElementName) == 0) {
            for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
                if (strcmp(child->Name(), CppcheckXml::SuppressionElementName) != 0)
                    continue;
                Suppressions::Suppression s;
                s.errorId = readSafe(child->GetText(), "");
                s.fileName = readSafe(child->Attribute("fileName"), "");
                if (!s.fileName.empty())
                    s.fileName = joinRelativePath(path, s.fileName);
                s.lineNumber = child->IntAttribute("lineNumber", Suppressions::Suppression::NO_LINE);
                s.symbolName = readSafe(child->Attribute("symbolName"), "");
                s.hash = strToInt<std::size_t>(readSafe(child->Attribute("hash"), "0"));
                suppressions.push_back(std::move(s));
            }
        } else if (strcmp(node->Name(), CppcheckXml::VSConfigurationElementName) == 0)
            guiProject.checkVsConfigs = readXmlStringList(node, emptyString, CppcheckXml::VSConfigurationName, nullptr);
        else if (strcmp(node->Name(), CppcheckXml::PlatformElementName) == 0)
            guiProject.platform = readSafe(node->GetText(), "");
        else if (strcmp(node->Name(), CppcheckXml::AnalyzeAllVsConfigsElementName) == 0)
            guiProject.analyzeAllVsConfigs = readSafe(node->GetText(), "");
        else if (strcmp(node->Name(), CppcheckXml::Parser) == 0)
            temp.clang = true;
        else if (strcmp(node->Name(), CppcheckXml::AddonsElementName) == 0) {
            const auto& addons = readXmlStringList(node, emptyString, CppcheckXml::AddonElementName, nullptr);
            temp.addons.insert(addons.cbegin(), addons.cend());
        }
        else if (strcmp(node->Name(), CppcheckXml::TagsElementName) == 0)
            node->Attribute(CppcheckXml::TagElementName); // FIXME: Write some warning
        else if (strcmp(node->Name(), CppcheckXml::ToolsElementName) == 0) {
            const std::list<std::string> toolList = readXmlStringList(node, emptyString, CppcheckXml::ToolElementName, nullptr);
            for (const std::string &toolName : toolList) {
                if (toolName == CppcheckXml::ClangTidy)
                    temp.clangTidy = true;
            }
        } else if (strcmp(node->Name(), CppcheckXml::CheckHeadersElementName) == 0)
            temp.checkHeaders = (strcmp(readSafe(node->GetText(), ""), "true") == 0);
        else if (strcmp(node->Name(), CppcheckXml::CheckLevelExhaustiveElementName) == 0)
            checkLevelExhaustive = true;
        else if (strcmp(node->Name(), CppcheckXml::CheckUnusedTemplatesElementName) == 0)
            temp.checkUnusedTemplates = (strcmp(readSafe(node->GetText(), ""), "true") == 0);
        else if (strcmp(node->Name(), CppcheckXml::MaxCtuDepthElementName) == 0)
            temp.maxCtuDepth = strToInt<int>(readSafe(node->GetText(), "2")); // TODO: bail out when missing?
        else if (strcmp(node->Name(), CppcheckXml::MaxTemplateRecursionElementName) == 0)
            temp.maxTemplateRecursion = strToInt<int>(readSafe(node->GetText(), "100")); // TODO: bail out when missing?
        else if (strcmp(node->Name(), CppcheckXml::CheckUnknownFunctionReturn) == 0)
            ; // TODO
        else if (strcmp(node->Name(), Settings::SafeChecks::XmlRootName) == 0) {
            for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
                if (strcmp(child->Name(), Settings::SafeChecks::XmlClasses) == 0)
                    temp.safeChecks.classes = true;
                else if (strcmp(child->Name(), Settings::SafeChecks::XmlExternalFunctions) == 0)
                    temp.safeChecks.externalFunctions = true;
                else if (strcmp(child->Name(), Settings::SafeChecks::XmlInternalFunctions) == 0)
                    temp.safeChecks.internalFunctions = true;
                else if (strcmp(child->Name(), Settings::SafeChecks::XmlExternalVariables) == 0)
                    temp.safeChecks.externalVariables = true;
                else {
                    printError("Unknown '" + std::string(Settings::SafeChecks::XmlRootName) + "' element '" + std::string(child->Name()) + "' in Cppcheck project file");
                    return false;
                }
            }
        } else if (strcmp(node->Name(), CppcheckXml::TagWarningsElementName) == 0)
            ; // TODO
        // Cppcheck Premium features
        else if (strcmp(node->Name(), CppcheckXml::BughuntingElementName) == 0)
            temp.premiumArgs += " --bughunting";
        else if (strcmp(node->Name(), CppcheckXml::CertIntPrecisionElementName) == 0)
            temp.premiumArgs += std::string(" --cert-c-int-precision=") + readSafe(node->GetText(), "0");
        else if (strcmp(node->Name(), CppcheckXml::CodingStandardsElementName) == 0) {
            for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
                if (strcmp(child->Name(), CppcheckXml::CodingStandardElementName) == 0 && child->GetText())
                    temp.premiumArgs += std::string(" --") + child->GetText();
            }
        }
        else {
            printError("Unknown element '" + std::string(node->Name()) + "' in Cppcheck project file");
            return false;
        }
    }
    settings->basePaths = temp.basePaths;
    settings->relativePaths |= temp.relativePaths;
    settings->buildDir = temp.buildDir;
    settings->includePaths = temp.includePaths;
    settings->userDefines = temp.userDefines;
    settings->userUndefs = temp.userUndefs;
    settings->addons = temp.addons;
    settings->clang = temp.clang;
    settings->clangTidy = temp.clangTidy;

    if (!settings->premiumArgs.empty())
        settings->premiumArgs += temp.premiumArgs;
    else if (!temp.premiumArgs.empty())
        settings->premiumArgs = temp.premiumArgs.substr(1);

    for (const std::string &p : paths)
        guiProject.pathNames.push_back(p);
    settings->nomsg.addSuppressions(std::move(suppressions));
    settings->checkHeaders = temp.checkHeaders;
    settings->checkUnusedTemplates = temp.checkUnusedTemplates;
    settings->maxCtuDepth = temp.maxCtuDepth;
    settings->maxTemplateRecursion = temp.maxTemplateRecursion;
    settings->safeChecks = temp.safeChecks;

    if (checkLevelExhaustive)
        settings->setCheckLevelExhaustive();
    else
        settings->setCheckLevelNormal();

    return true;
}

void ImportProject::selectOneVsConfig(cppcheck::Platform::Type platform)
{
    std::set<std::string> filenames;
    for (std::list<ImportProject::FileSettings>::iterator it = fileSettings.begin(); it != fileSettings.end();) {
        if (it->cfg.empty()) {
            ++it;
            continue;
        }
        const ImportProject::FileSettings &fs = *it;
        bool remove = false;
        if (fs.cfg.compare(0,5,"Debug") != 0)
            remove = true;
        if (platform == cppcheck::Platform::Type::Win64 && fs.platformType != platform)
            remove = true;
        else if ((platform == cppcheck::Platform::Type::Win32A || platform == cppcheck::Platform::Type::Win32W) && fs.platformType == cppcheck::Platform::Type::Win64)
            remove = true;
        else if (filenames.find(fs.filename) != filenames.end())
            remove = true;
        if (remove) {
            it = fileSettings.erase(it);
        } else {
            filenames.insert(fs.filename);
            ++it;
        }
    }
}

void ImportProject::selectVsConfigurations(cppcheck::Platform::Type platform, const std::vector<std::string> &configurations)
{
    for (std::list<ImportProject::FileSettings>::iterator it = fileSettings.begin(); it != fileSettings.end();) {
        if (it->cfg.empty()) {
            ++it;
            continue;
        }
        const ImportProject::FileSettings &fs = *it;
        const auto config = fs.cfg.substr(0, fs.cfg.find('|'));
        bool remove = false;
        if (std::find(configurations.begin(), configurations.end(), config) == configurations.end())
            remove = true;
        if (platform == cppcheck::Platform::Type::Win64 && fs.platformType != platform)
            remove = true;
        else if ((platform == cppcheck::Platform::Type::Win32A || platform == cppcheck::Platform::Type::Win32W) && fs.platformType == cppcheck::Platform::Type::Win64)
            remove = true;
        if (remove) {
            it = fileSettings.erase(it);
        } else {
            ++it;
        }
    }
}

std::list<std::string> ImportProject::getVSConfigs()
{
    return std::list<std::string>(mAllVSConfigs.cbegin(), mAllVSConfigs.cend());
}

void ImportProject::setRelativePaths(const std::string &filename)
{
    if (Path::isAbsolute(filename))
        return;
    const std::vector<std::string> basePaths{Path::fromNativeSeparators(Path::getCurrentPath())};
    for (auto &fs: fileSettings) {
        fs.filename = Path::getRelativePath(fs.filename, basePaths);
        for (auto &includePath: fs.includePaths)
            includePath = Path::getRelativePath(includePath, basePaths);
    }
}

void ImportProject::printError(const std::string &message)
{
    std::cout << "cppcheck: error: " << message << std::endl;
}

bool ImportProject::sourceFileExists(const std::string &file)
{
    return Path::isFile(file);
}
