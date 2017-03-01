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

#include "importproject.h"
#include "path.h"
#include "settings.h"
#include "tokenize.h"
#include "token.h"
#include "tinyxml2.h"
#include <fstream>

void ImportProject::ignorePaths(const std::vector<std::string> &ipaths)
{
    for (std::list<FileSettings>::iterator it = fileSettings.begin(); it != fileSettings.end();) {
        bool ignore = false;
        for (std::size_t i = 0; i < ipaths.size(); ++i) {
            if (it->filename.size() > ipaths[i].size() && it->filename.compare(0,ipaths[i].size(),ipaths[i])==0) {
                ignore = true;
                break;
            }
        }
        if (ignore)
            fileSettings.erase(it++);
        else
            ++it;
    }
}

void ImportProject::ignoreOtherConfigs(const std::string &cfg)
{
    for (std::list<FileSettings>::iterator it = fileSettings.begin(); it != fileSettings.end();) {
        if (it->cfg != cfg)
            fileSettings.erase(it++);
        else
            ++it;
    }
}

void ImportProject::ignoreOtherPlatforms(cppcheck::Platform::PlatformType platformType)
{
    for (std::list<FileSettings>::iterator it = fileSettings.begin(); it != fileSettings.end();) {
        if (it->platformType != cppcheck::Platform::Unspecified && it->platformType != platformType)
            fileSettings.erase(it++);
        else
            ++it;
    }
}

void ImportProject::FileSettings::setDefines(std::string defs)
{
    while (defs.find(";%(") != std::string::npos) {
        std::string::size_type pos1 = defs.find(";%(");
        std::string::size_type pos2 = defs.find(';', pos1+1);
        defs.erase(pos1, pos2 == std::string::npos ? pos2 : (pos2-pos1));
    }
    while (defs.find(";;") != std::string::npos)
        defs.erase(defs.find(";;"),1);
    if (!defs.empty() && defs.back() == ';')
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

static bool simplifyPathWithVariables(std::string &s, const std::map<std::string, std::string> &variables)
{
    std::set<std::string> expanded;
    std::string::size_type start = 0;
    while ((start = s.find("$(")) != std::string::npos) {
        std::string::size_type end = s.find(')',start);
        if (end == std::string::npos)
            break;
        const std::string var = s.substr(start+2,end-start-2);
        if (expanded.find(var) != expanded.end())
            break;
        expanded.insert(var);
        std::map<std::string, std::string>::const_iterator it1 = variables.find(var);
        if (it1 == variables.end())
            break;
        s = s.substr(0,start) + it1->second + s.substr(end+1);
    }
    if (s.find("$(") != std::string::npos)
        return false;
    s = Path::simplifyPath(Path::fromNativeSeparators(s));
    return true;
}

void ImportProject::FileSettings::setIncludePaths(const std::string &basepath, const std::list<std::string> &in, const std::map<std::string, std::string> &variables)
{
    std::list<std::string> I;
    for (std::list<std::string>::const_iterator it = in.begin(); it != in.end(); ++it) {
        if (it->empty())
            continue;
        if (it->compare(0,2,"%(")==0)
            continue;
        std::string s(Path::fromNativeSeparators(*it));
        if (s[0] == '/' || (s.size() > 1U && s.compare(1,2,":/") == 0)) {
            if (s.back() != '/')
                s += '/';
            I.push_back(s);
            continue;
        }

        if (s.back() == '/') // this is a temporary hack, simplifyPath can crash if path ends with '/'
            s.erase(s.size() - 1U); // TODO: Use std::string::pop_back() as soon as travis supports it

        if (s.find("$(")==std::string::npos) {
            s = Path::simplifyPath(basepath + s);
        } else {
            if (!simplifyPathWithVariables(s,variables))
                continue;
        }
        if (s.empty())
            continue;
        I.push_back(s + '/');
    }
    includePaths.swap(I);
}

void ImportProject::import(const std::string &filename)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
        return;
    if (filename.find("compile_commands.json") != std::string::npos) {
        importCompileCommands(fin);
    } else if (filename.find(".sln") != std::string::npos) {
        std::string path(Path::getPathFromFilename(Path::fromNativeSeparators(filename)));
        if (!path.empty() && path.back() != '/')
            path += '/';
        importSln(fin,path);
    } else if (filename.find(".vcxproj") != std::string::npos) {
        std::map<std::string, std::string> variables;
        importVcxproj(filename, variables, emptyString);
    }
}

void ImportProject::importCompileCommands(std::istream &istr)
{
    std::map<std::string, std::string> values;

    // TODO: Use a JSON parser

    Settings settings;
    TokenList tokenList(&settings);
    tokenList.createTokens(istr);
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%str% : %str% [,}]")) {
            const std::string& key = tok->str();
            const std::string& value = tok->strAt(2);
            values[key.substr(1, key.size() - 2U)] = value.substr(1, value.size() - 2U);
        }

        else if (tok->str() == "}") {
            if (!values["file"].empty() && !values["command"].empty()) {
                struct FileSettings fs;
                fs.filename = Path::fromNativeSeparators(values["file"]);
                const std::string& command = values["command"];
                const std::string directory = Path::fromNativeSeparators(values["directory"]);
                std::string::size_type pos = 0;
                while (std::string::npos != (pos = command.find(' ',pos))) {
                    pos++;
                    if (pos >= command.size())
                        break;
                    if (command[pos] != '/' && command[pos] != '-')
                        continue;
                    pos++;
                    if (pos >= command.size())
                        break;
                    char F = command[pos++];
                    std::string fval;
                    while (pos < command.size() && command[pos] != ' ') {
                        if (command[pos] != '\\')
                            fval += command[pos];
                        pos++;
                    }
                    if (F=='D')
                        fs.defines += fval + ";";
                    else if (F=='U')
                        fs.undefs.insert(fval);
                    else if (F=='I')
                        fs.includePaths.push_back(fval);
                }
                std::map<std::string, std::string> variables;
                fs.setIncludePaths(directory, fs.includePaths, variables);
                fs.setDefines(fs.defines);
                fileSettings.push_back(fs);
            }
            values.clear();
        }
    }
}

void ImportProject::importSln(std::istream &istr, const std::string &path)
{
    std::map<std::string,std::string> variables;
    variables["SolutionDir"] = path;

    std::string line;
    while (std::getline(istr,line)) {
        if (line.compare(0,8,"Project(")!=0)
            continue;
        const std::string::size_type pos = line.find(".vcxproj");
        if (pos == std::string::npos)
            continue;
        const std::string::size_type pos1 = line.rfind('\"',pos);
        if (pos == std::string::npos)
            continue;
        const std::string vcxproj(line.substr(pos1+1, pos-pos1+7));
        importVcxproj(path + Path::fromNativeSeparators(vcxproj), variables, emptyString);
    }
}

namespace {
    struct ProjectConfiguration {
        explicit ProjectConfiguration(const tinyxml2::XMLElement *cfg) {
            const char *a = cfg->Attribute("Include");
            if (a)
                name = a;
            for (const tinyxml2::XMLElement *e = cfg->FirstChildElement(); e; e = e->NextSiblingElement()) {
                if (e->GetText()) {
                    if (std::strcmp(e->Name(),"Configuration")==0)
                        configuration = e->GetText();
                    else if (std::strcmp(e->Name(),"Platform")==0)
                        platform = e->GetText();
                }
            }
        }
        std::string name;
        std::string configuration;
        std::string platform;
    };

    struct ItemDefinitionGroup {
        explicit ItemDefinitionGroup(const tinyxml2::XMLElement *idg, const std::string &includePaths) : additionalIncludePaths(includePaths) {
            const char *condAttr = idg->Attribute("Condition");
            if (condAttr)
                condition = condAttr;
            for (const tinyxml2::XMLElement *e1 = idg->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
                if (std::strcmp(e1->Name(), "ClCompile") != 0)
                    continue;
                for (const tinyxml2::XMLElement *e = e1->FirstChildElement(); e; e = e->NextSiblingElement()) {
                    if (e->GetText()) {
                        if (std::strcmp(e->Name(), "PreprocessorDefinitions") == 0)
                            preprocessorDefinitions = e->GetText();
                        else if (std::strcmp(e->Name(), "AdditionalIncludeDirectories") == 0) {
                            if (!additionalIncludePaths.empty())
                                additionalIncludePaths += ';';
                            additionalIncludePaths += e->GetText();
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
            replaceAll(c, "$(Platform)", p.platform);

            // TODO : Better evaluation
            Settings s;
            std::istringstream istr(c);
            Tokenizer tokenizer(&s, nullptr);
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
        std::string preprocessorDefinitions;
        std::string additionalIncludePaths;
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

static void importPropertyGroup(const tinyxml2::XMLElement *node, std::map<std::string,std::string> *variables, std::string *includePath, bool *useOfMfc)
{
    if (useOfMfc) {
        for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "UseOfMfc") == 0)
                *useOfMfc = true;
        }
    }

    if (node->Attribute("Label") && std::strcmp(node->Attribute("Label"),"UserMacros")==0) {
        for (const tinyxml2::XMLElement *propertyGroup = node->FirstChildElement(); propertyGroup; propertyGroup = propertyGroup->NextSiblingElement()) {
            const std::string name(propertyGroup->Name());
            const char *text = propertyGroup->GetText();
            (*variables)[name] = std::string(text ? text : "");
        }

    } else if (!node->Attribute("Label")) {
        for (const tinyxml2::XMLElement *propertyGroup = node->FirstChildElement(); propertyGroup; propertyGroup = propertyGroup->NextSiblingElement()) {
            if (std::strcmp(propertyGroup->Name(), "IncludePath") != 0)
                continue;
            const char *text = propertyGroup->GetText();
            if (!text)
                continue;
            std::string s(text);
            std::string::size_type pos = s.find("$(IncludePath)");
            if (pos != std::string::npos)
                s = s.substr(0,pos) + *includePath + s.substr(pos+14U);
            *includePath = s;
        }
    }
}

static void loadVisualStudioProperties(const std::string &props, std::map<std::string,std::string> *variables, std::string *includePath, const std::string &additionalIncludeDirectories, std::list<ItemDefinitionGroup> &itemDefinitionGroupList)
{
    std::string filename(props);
    if (!simplifyPathWithVariables(filename,*variables))
        return;
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS)
        return;
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr)
        return;
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (std::strcmp(node->Name(), "ImportGroup") == 0 && node->Attribute("Label") && std::strcmp(node->Attribute("Label"),"PropertySheets")==0) {
            for (const tinyxml2::XMLElement *importGroup = node->FirstChildElement(); importGroup; importGroup = importGroup->NextSiblingElement()) {
                if (std::strcmp(importGroup->Name(), "Import") == 0 && importGroup->Attribute("Project")) {
                    std::string loadprj = importGroup->Attribute("Project");
                    if (loadprj.find('$') == std::string::npos) {
                        loadprj = Path::getPathFromFilename(filename) + loadprj;
                    }
                    loadVisualStudioProperties(loadprj, variables, includePath, additionalIncludeDirectories, itemDefinitionGroupList);
                }
            }
        } else if (std::strcmp(node->Name(),"PropertyGroup")==0) {
            importPropertyGroup(node, variables, includePath, nullptr);
        } else if (std::strcmp(node->Name(),"ItemDefinitionGroup")==0) {
            itemDefinitionGroupList.push_back(ItemDefinitionGroup(node, additionalIncludeDirectories));
        }
    }
}

void ImportProject::importVcxproj(const std::string &filename, std::map<std::string, std::string> variables, const std::string &additionalIncludeDirectories)
{
    variables["ProjectDir"] = Path::simplifyPath(Path::getPathFromFilename(filename));

    std::list<ProjectConfiguration> projectConfigurationList;
    std::list<std::string> compileList;
    std::list<ItemDefinitionGroup> itemDefinitionGroupList;
    std::string includePath;

    bool useOfMfc = false;

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
    if (error != tinyxml2::XML_SUCCESS)
        return;
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr)
        return;
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (std::strcmp(node->Name(), "ItemGroup") == 0) {
            if (node->Attribute("Label") && std::strcmp(node->Attribute("Label"), "ProjectConfigurations") == 0) {
                for (const tinyxml2::XMLElement *cfg = node->FirstChildElement(); cfg; cfg = cfg->NextSiblingElement()) {
                    if (std::strcmp(cfg->Name(), "ProjectConfiguration") == 0)
                        projectConfigurationList.push_back(ProjectConfiguration(cfg));
                }
            } else {
                for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                    if (std::strcmp(e->Name(), "ClCompile") == 0)
                        compileList.push_back(e->Attribute("Include"));
                }
            }
        } else if (std::strcmp(node->Name(), "ItemDefinitionGroup") == 0) {
            itemDefinitionGroupList.push_back(ItemDefinitionGroup(node, additionalIncludeDirectories));
        } else if (std::strcmp(node->Name(), "PropertyGroup") == 0) {
            importPropertyGroup(node, &variables, &includePath, &useOfMfc);
        } else if (std::strcmp(node->Name(), "ImportGroup") == 0) {
            if (node->Attribute("Label") && std::strcmp(node->Attribute("Label"), "PropertySheets") == 0) {
                for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                    if (std::strcmp(e->Name(), "Import") == 0) {
                        const char *Project = e->Attribute("Project");
                        if (Project)
                            loadVisualStudioProperties(Project, &variables, &includePath, additionalIncludeDirectories, itemDefinitionGroupList);
                    }
                }
            }
        }
    }

    for (std::list<std::string>::const_iterator c = compileList.begin(); c != compileList.end(); ++c) {
        for (std::list<ProjectConfiguration>::const_iterator p = projectConfigurationList.begin(); p != projectConfigurationList.end(); ++p) {
            FileSettings fs;
            fs.filename = Path::simplifyPath(Path::getPathFromFilename(filename) + *c);
            fs.cfg = p->name;
            fs.defines = "_MSC_VER=1900;_WIN32=1";
            if (p->platform == "Win32")
                fs.platformType = cppcheck::Platform::Win32W;
            else if (p->platform == "x64") {
                fs.platformType = cppcheck::Platform::Win64;
                fs.defines += ";_WIN64=1";
            }
            if (useOfMfc)
                fs.defines += ";__AFXWIN_H__";
            std::string additionalIncludePaths;
            for (std::list<ItemDefinitionGroup>::const_iterator i = itemDefinitionGroupList.begin(); i != itemDefinitionGroupList.end(); ++i) {
                if (!i->conditionIsTrue(*p))
                    continue;
                fs.defines += ';' + i->preprocessorDefinitions;
                additionalIncludePaths += ';' + i->additionalIncludePaths;
            }
            fs.setDefines(fs.defines);
            fs.setIncludePaths(Path::getPathFromFilename(filename), toStringList(includePath + ';' + additionalIncludePaths), variables);
            fileSettings.push_back(fs);
        }
    }
}
