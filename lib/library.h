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

//---------------------------------------------------------------------------
#ifndef libraryH
#define libraryH
//---------------------------------------------------------------------------

#include "config.h"
#include <map>
#include <set>
#include <string>

/// @addtogroup Core
/// @{

/**
 * @brief Library definitions handling
 */
class CPPCHECKLIB Library {
public:
    Library();
    Library(const Library &);
    ~Library();

    bool load(const char exename[], const char path[]);

    /** get allocation id for function (by name) */
    int alloc(const std::string &name) const {
        return getid(_alloc, name);
    }

    /** get deallocation id for function (by name) */
    int dealloc(const std::string &name) const {
        return getid(_dealloc, name);
    }

    /** set allocation id for function */
    void setalloc(const std::string &functionname, int id) {
        _alloc[functionname] = id;
    }

    void setdealloc(const std::string &functionname, int id) {
        _dealloc[functionname] = id;
    }

    /** add noreturn function setting */
    void setnoreturn(const std::string& funcname, bool noreturn) {
        _noreturn[funcname] = noreturn;
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
    std::set<std::string> leakignore;

    bool isnoreturn(const std::string &name) const {
        std::map<std::string,bool>::const_iterator it = _noreturn.find(name);
        return (it != _noreturn.end() && it->second);
    }

    bool isnotnoreturn(const std::string &name) const {
        std::map<std::string,bool>::const_iterator it = _noreturn.find(name);
        return (it != _noreturn.end() && !it->second);
    }

    struct ArgumentChecks {
        ArgumentChecks() {
            notnull = notuninit = formatstr = strz = false;
        }

        bool notnull;
        bool notuninit;
        bool formatstr;
        bool strz;
    };

    // function name, argument nr => argument data
    std::map<std::string, std::map<int, ArgumentChecks> > argumentChecks;

    bool isnullargbad(const std::string &functionName, int argnr) const {
        const ArgumentChecks *arg = getarg(functionName,argnr);
        return arg && arg->notnull;
    }

    bool isuninitargbad(const std::string &functionName, int argnr) const {
        const ArgumentChecks *arg = getarg(functionName,argnr);
        return arg && arg->notuninit;
    }

    bool isargformatstr(const std::string &functionName, int argnr) const {
        const ArgumentChecks *arg = getarg(functionName,argnr);
        return arg && arg->formatstr;
    }

    bool isargstrz(const std::string &functionName, int argnr) const {
        const ArgumentChecks *arg = getarg(functionName,argnr);
        return arg && arg->strz;
    }

    std::set<std::string> returnuninitdata;

private:
    int allocid;
    std::map<std::string, int> _alloc; // allocation functions
    std::map<std::string, int> _dealloc; // deallocation functions
    std::map<std::string, bool> _noreturn; // is function noreturn?

    const ArgumentChecks * getarg(const std::string &functionName, int argnr) const {
        std::map<std::string, std::map<int, ArgumentChecks> >::const_iterator it1;
        it1 = argumentChecks.find(functionName);
        if (it1 != argumentChecks.end()) {
            const std::map<int,ArgumentChecks>::const_iterator it2 = it1->second.find(argnr);
            if (it2 != it1->second.end())
                return &it2->second;
        }
        return NULL;
    }

    static int getid(const std::map<std::string,int> &data, const std::string &name) {
        const std::map<std::string,int>::const_iterator it = data.find(name);
        return (it == data.end()) ? 0 : it->second;
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // libraryH
