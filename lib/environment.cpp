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

#include "environment.h"
#include "tinyxml2.h"

Environment::Environment() : allocid(0)
{
    while (!ismemory(++allocid));
    _alloc["malloc"] = allocid;
    _alloc["calloc"] = allocid;
    _alloc["strdup"] = allocid;
    _alloc["strndup"] = allocid;
    _dealloc["free"] = allocid;

    while (!isresource(++allocid));
    _alloc["fopen"] = allocid;
    _dealloc["fclose"] = allocid;
}

Environment::Environment(const Environment &env) : use(env.use), ignore(env.ignore), noreturn(env.noreturn), allocid(env.allocid), _alloc(env._alloc), _dealloc(env._dealloc)
{

}

Environment::~Environment() { }

bool Environment::load(const char path[])
{
    tinyxml2::XMLDocument doc;

    const tinyxml2::XMLError error = doc.LoadFile(path);
    if (error != tinyxml2::XML_NO_ERROR)
        return false;

    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (strcmp(rootnode->Name(),"def") != 0)
        return false;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (strcmp(node->Name(),"memory")==0) {
            while (!ismemory(++allocid));
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                if (strcmp(memorynode->Name(),"alloc")==0)
                    _alloc[memorynode->GetText()] = allocid;
                else if (strcmp(memorynode->Name(),"dealloc")==0)
                    _dealloc[memorynode->GetText()] = allocid;
                else if (strcmp(memorynode->Name(),"use")==0)
                    use.insert(memorynode->GetText());
                else
                    return false;
            }
        }

        else if (strcmp(node->Name(),"ignore")==0)
            ignore.insert(node->GetText());
        else if (strcmp(node->Name(),"noreturn")==0)
            noreturn.insert(node->GetText());
        else
            return false;
    }
    return true;
}
