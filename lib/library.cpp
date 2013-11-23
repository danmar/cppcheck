/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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

#include "library.h"
#include "path.h"
#include "tinyxml2.h"

#include <string>
#include <algorithm>

Library::Library() : allocid(0)
{
}

bool Library::load(const char exename[], const char path[])
{
    if (std::strchr(path,',') != NULL) {
        bool ret = true;
        std::string p(path);
        while (p.find(",") != std::string::npos) {
            const std::string::size_type pos = p.find(",");
            ret &= load(exename, p.substr(0,pos).c_str());
            p = p.substr(pos+1);
        }
        if (!p.empty())
            ret &= load(exename, p.c_str());
        return ret;
    }

    // open file..
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(path);
    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
        // failed to open file.. is there no extension?
        std::string fullfilename(path);
        if (Path::getFilenameExtension(fullfilename) == "") {
            fullfilename += ".cfg";
            error = doc.LoadFile(fullfilename.c_str());
        }

        if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
            // Try to locate the library configuration in the installation folder..
            const std::string installfolder = Path::fromNativeSeparators(Path::getPathFromFilename(exename));
            const std::string filename = installfolder + "cfg/" + fullfilename;
            error = doc.LoadFile(filename.c_str());
        }
    }

    return (error == tinyxml2::XML_NO_ERROR) && load(doc);
}

bool Library::load(const tinyxml2::XMLDocument &doc)
{
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();

    if (rootnode == NULL)
        return false;

    if (strcmp(rootnode->Name(),"def") != 0)
        return false;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (strcmp(node->Name(),"memory")==0 || strcmp(node->Name(),"resource")==0) {
            if (strcmp(node->Name(), "memory")==0)
                while (!ismemory(++allocid));
            else
                while (!isresource(++allocid));
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                if (strcmp(memorynode->Name(),"alloc")==0) {
                    _alloc[memorynode->GetText()] = allocid;
                    const char *init = memorynode->Attribute("init");
                    if (init && strcmp(init,"false")==0) {
                        returnuninitdata.insert(memorynode->GetText());
                    }
                } else if (strcmp(memorynode->Name(),"dealloc")==0)
                    _dealloc[memorynode->GetText()] = allocid;
                else if (strcmp(memorynode->Name(),"use")==0)
                    use.insert(memorynode->GetText());
                else
                    return false;
            }
        }

        else if (strcmp(node->Name(),"function")==0) {
            const char *name = node->Attribute("name");
            if (name == NULL)
                return false;

            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                if (strcmp(functionnode->Name(),"noreturn")==0)
                    _noreturn[name] = (strcmp(functionnode->GetText(), "true") == 0);
                else if (strcmp(functionnode->Name(),"leak-ignore")==0)
                    leakignore.insert(name);
                else if (strcmp(functionnode->Name(), "arg") == 0 && functionnode->Attribute("nr") != NULL) {
                    const int nr = atoi(functionnode->Attribute("nr"));
                    bool notnull = false;
                    bool notuninit = false;
                    bool formatstr = false;
                    bool strz = false;
                    for (const tinyxml2::XMLElement *argnode = functionnode->FirstChildElement(); argnode; argnode = argnode->NextSiblingElement()) {
                        if (strcmp(argnode->Name(), "not-null") == 0)
                            notnull = true;
                        else if (strcmp(argnode->Name(), "not-uninit") == 0)
                            notuninit = true;
                        else if (strcmp(argnode->Name(), "formatstr") == 0)
                            formatstr = true;
                        else if (strcmp(argnode->Name(), "strz") == 0)
                            strz = true;
                        else
                            return false;
                    }
                    argumentChecks[name][nr].notnull = notnull;
                    argumentChecks[name][nr].notuninit = notuninit;
                    argumentChecks[name][nr].formatstr = formatstr;
                    argumentChecks[name][nr].strz = strz;
                } else if (strcmp(functionnode->Name(), "ignorefunction") == 0) {
                    _ignorefunction[name] = (strcmp(functionnode->GetText(), "true") == 0);
                } else
                    return false;
            }
        }

        else if (strcmp(node->Name(),"files")==0) {
            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                if (strcmp(functionnode->Name(), "file") == 0) {
                    _markupExtensions.insert(functionnode->Attribute("ext"));
                    const char * report = functionnode->Attribute("reporterrors");
                    if (report)
                        _reporterrors[functionnode->Attribute("ext")] = strcmp(report, "true")==0;
                } else
                    return false;
            }
        }

        else if (strcmp(node->Name(), "keywords") == 0) {
            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                if (strcmp(functionnode->Name(), "library") == 0) {
                    const char * const extension = functionnode->Attribute("extension");
                    for (const tinyxml2::XMLElement *librarynode = functionnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "keyword") == 0) {
                            _keywords[extension].push_back(librarynode->Attribute("name"));
                        } else
                            return false;
                    }
                } else
                    return false;
            }
        }

        else if (strcmp(node->Name(), "exported") == 0) {
            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                if (strcmp(functionnode->Name(), "exporter") == 0) {
                    const char * prefix = (functionnode->Attribute("prefix"));
                    if (!prefix)
                        return false;

                    for (const tinyxml2::XMLElement *enode = functionnode->FirstChildElement(); enode; enode = enode->NextSiblingElement()) {
                        if (strcmp(enode->Name(), "prefix") == 0) {
                            _exporters[prefix].addPrefix(enode->Attribute("name"));
                        } else if (strcmp(enode->Name(), "suffix") == 0) {
                            _exporters[prefix].addSuffix(enode->Attribute("name"));
                        } else
                            return false;
                    }
                } else
                    return false;
            }
        }

        else if (strcmp(node->Name(), "imported") == 0) {
            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                if (strcmp(functionnode->Name(), "library") == 0) {
                    const char * const extension = functionnode->Attribute("extension");
                    for (const tinyxml2::XMLElement *librarynode = functionnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "importer") == 0) {
                            _importers[extension].push_back(librarynode->Attribute("name"));
                        } else
                            return false;
                    }
                }
            }
        }

        else if (strcmp(node->Name(), "reflection") == 0) {
            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                if (strcmp(functionnode->Name(), "library") == 0) {
                    const char * const extension = functionnode->Attribute("extension");
                    for (const tinyxml2::XMLElement *librarynode = functionnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "call") == 0) {
                            const char * const argString = librarynode->Attribute("arg");
                            if (argString) {
                                _reflection[extension][librarynode->Attribute("name")]
                                    = atoi(argString);
                            }
                        } else
                            return false;
                    }
                } else
                    return false;
            }
        }

        else if (strcmp(node->Name(), "codeblocks") == 0) {
            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                if (strcmp(functionnode->Name(), "library") == 0) {
                    const char * const extension = functionnode->Attribute("extension");
                    for (const tinyxml2::XMLElement *librarynode = functionnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "block") == 0) {
                            _executableblocks[extension].addBlock(librarynode->Attribute("name"));
                        } else if (strcmp(librarynode->Name(), "structure") == 0) {
                            const char * start = librarynode->Attribute("start");
                            if (start)
                                _executableblocks[extension].setStart(start);
                            const char * end = librarynode->Attribute("end");
                            if (end)
                                _executableblocks[extension].setEnd(end);
                            const char * offset = librarynode->Attribute("offset");
                            if (offset)
                                _executableblocks[extension].setOffset(atoi(offset));
                        } else
                            return false;
                    }
                }
            }

        } else
            return false;
    }
    return true;
}
