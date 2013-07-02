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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <map>
#include <set>
#include <string>

/// @addtogroup Core
/// @{

/**
 * @brief Environment definitions handling
 */
class Environment {
public:
    Environment();
    Environment(const Environment &);
    ~Environment();

    bool load(const char path[]);

    /** get allocation id for function (by name) */
    int alloc(const std::string &name) const {
        return getid(_alloc, name);
    }

    /** get deallocation id for function (by name) */
    int dealloc(const std::string &name) const {
        return getid(_dealloc, name);
    }

    /** is allocation type memory? */
    static bool ismemory(int id) {
        return ((id > 0) && ((id & 1) == 0));
    }

    /** is allocation type resource? */
    static bool isresource(int id) {
        return ((id > 0) && ((id & 1) == 1));
    }

    std::set<std::string> use;
    std::set<std::string> ignore;
    std::set<std::string> noreturn;

private:
    int allocid;
    std::map<std::string, int> _alloc; // allocation functions
    std::map<std::string, int> _dealloc; // deallocation functions

    int getid(const std::map<std::string,int> &data, const std::string &name) const {
        const std::map<std::string,int>::const_iterator it = data.find(name);
        return (it == data.end()) ? 0 : it->second;
    }
};

/// @}

#endif // PATH_H_INCLUDED
