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

#include "project.h"
#include "path.h"
#include "tokenlist.h"
#include "tinyxml2.h"
#include <fstream>

void Project::load(const std::string &filename) {
    std::ifstream fin(filename);
    if (!fin.is_open())
        return;
    if (filename == "compile_commands.json") {
        loadCompileCommands(fin);
    } else if (filename.find(".vcxproj") != std::string::npos) {
        loadVcxproj(filename);
    }
}

void Project::loadCompileCommands(std::istream &istr) {
    std::map<std::string, std::string> values;

    Settings settings;
    TokenList tokenList(&settings);
    tokenList.createTokens(istr);
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
      if (Token::Match(tok, "%str% : %str% [,}]")) {
        std::string key = tok->str();
        std::string value = tok->strAt(2);
        values[key.substr(1, key.size() - 2U)] = value.substr(1, value.size() - 2U);
      }

      else if (tok->str() == "}") {
            if (!values["file"].empty() && !values["command"].empty()) {
                struct FileSettings fs;
                fs.filename = Path::fromNativeSeparators(values["file"]);
                std::string command = values["command"];
                std::string::size_type pos = 0;
                while (std::string::npos != (pos = command.find(" ",pos))) {
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
                    while (pos < command.size() && command[pos] != ' ')
                        fval += command[pos++];
                    if (F=='D')
                        fs.defines += fval + ";";
                    else if (F=='U')
                        fs.undefs.insert(fval);
                    else if (F=='I')
                        fs.includePaths.push_back(fval);
                }
                fileSettings.push_back(fs);
            }
            values.clear();
        }
    }
}

namespace {
  struct ProjectConfiguration {
    ProjectConfiguration(const tinyxml2::XMLElement *cfg) {
        for (const tinyxml2::XMLElement *e = cfg->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(),"Configuration")==0)
                configuration = e->GetText();
            else if (std::strcmp(e->Name(),"Platform")==0)
                platform = e->GetText();
        }
    }
    std::string configuration;
    std::string platform;
  };

  struct ItemDefinitionGroup {
    ItemDefinitionGroup(const tinyxml2::XMLElement *idg) {
      const char *condAttr = idg->Attribute("Condition");
      if (condAttr)
          condition = condAttr;
      for (const tinyxml2::XMLElement *e1 = idg->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
            if (std::strcmp(e1->Name(), "ClCompile") != 0)
                continue;
            for (const tinyxml2::XMLElement *e = e1->FirstChildElement(); e; e = e->NextSiblingElement()) {
                if (std::strcmp(e->Name(), "PreprocessorDefinitions") == 0)
                    preprocessorDefinitions = e->GetText();
                else if (std::strcmp(e->Name(), "AdditionalIncludeDirectories") == 0)
                    additionalIncludePaths = e->GetText();
            }
        }
    }
    bool conditionIsTrue(const ProjectConfiguration &p) const {
        std::string c = condition;
        std::string::size_type pos = 0;
        while ((pos = c.find("$(Configuration)")) != std::string::npos) {
            c.erase(pos,16);
            c.insert(pos,p.configuration);
        }
        while ((pos = c.find("$(Platform)")) != std::string::npos) {
          c.erase(pos, 11);
          c.insert(pos, p.platform);
        }
        // TODO : Better evaluation
        Settings s;
        std::istringstream istr(c);
        TokenList tokens(&s);
        tokens.createTokens(istr);
        tokens.createAst();
        for (const Token *tok = tokens.front(); tok; tok = tok->next()) {
            if (tok->str() == "==" && tok->astOperand1() && tok->astOperand2() && tok->astOperand1()->str() == tok->astOperand2()->str())
                return true;
        }
        return false;
    }
    std::string condition;
    std::string preprocessorDefinitions;
    std::string additionalIncludePaths;
  };
};

static std::list<std::string> toStringList(const std::string &s) {
    std::list<std::string> ret;
    std::string::size_type pos1 = 0;
    std::string::size_type pos2;
    while ((pos2 = s.find(";",pos1)) != std::string::npos) {
        ret.push_back(s.substr(pos1, pos2-pos1));
        pos1 = pos2 + 1;
        if (pos1 >= s.size())
            break;
    }
    if (pos1 < s.size())
        ret.push_back(s.substr(pos1));
    return ret;
}

void Project::loadVcxproj(const std::string &filename)
{
    std::list<ProjectConfiguration> projectConfigurationList;
    std::list<std::string> compileList;
    std::list<ItemDefinitionGroup> itemDefinitionGroupList;

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
            itemDefinitionGroupList.push_back(ItemDefinitionGroup(node));
        }
    }

    for (std::list<std::string>::const_iterator c = compileList.begin(); c != compileList.end(); ++c) {
        for (std::list<ProjectConfiguration>::const_iterator p = projectConfigurationList.begin(); p != projectConfigurationList.end(); ++p) {
            for (std::list<ItemDefinitionGroup>::const_iterator i = itemDefinitionGroupList.begin(); i != itemDefinitionGroupList.end(); ++i) {
                if (!i->conditionIsTrue(*p))
                    continue;
                     FileSettings fs;
                     fs.filename = Path::simplifyPath(Path::getPathFromFilename(filename) + *c);
                     fs.defines  = i->preprocessorDefinitions;
                     fs.includePaths = toStringList(i->additionalIncludePaths);
                     if (p->platform == "Win32")
                         fs.platformType = Settings::Win32W;
                     else if (p->platform == "x64")
                         fs.platformType = Settings::Win64;
                     fileSettings.push_back(fs);
            }
        }
    }
}
