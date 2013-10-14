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
#include <list>
#include <algorithm>

#include "path.h"

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

    bool load(const char exename [], const char path []);

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
        std::map<std::string, bool>::const_iterator it = _noreturn.find(name);
        return (it != _noreturn.end() && it->second);
    }

    bool isnotnoreturn(const std::string &name) const {
        std::map<std::string, bool>::const_iterator it = _noreturn.find(name);
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
        const ArgumentChecks *arg = getarg(functionName, argnr);
        return arg && arg->notnull;
    }

    bool isuninitargbad(const std::string &functionName, int argnr) const {
        const ArgumentChecks *arg = getarg(functionName, argnr);
        return arg && arg->notuninit;
    }

    bool isargformatstr(const std::string &functionName, int argnr) const {
        const ArgumentChecks *arg = getarg(functionName, argnr);
        return arg && arg->formatstr;
    }

    bool isargstrz(const std::string &functionName, int argnr) const {
        const ArgumentChecks *arg = getarg(functionName, argnr);
        return arg && arg->strz;
    }

    bool acceptFile(const std::string &path) const {
        const std::string extension = Path::getFilenameExtensionInLowerCase(path);
        std::list<std::string>::const_iterator it =
            std::find(_fileextensions.begin(), _fileextensions.end(), extension);
        return it != _fileextensions.end();
    }

    bool reportErrors(const std::string &path) const {
        std::string extension = Path::getFilenameExtensionInLowerCase(path);
        std::map<std::string, bool>::const_iterator it = _reporterrors.find(extension);
        if (it != _reporterrors.end()) {
            return it->second;
        }
        // assume true if we don't know as it'll be a core-type (c/cpp etc)
        return true;
    }

    bool ignorefunction(const std::string &function) const {
        std::map<std::string, bool>::const_iterator it = _ignorefunction.find(function);
        return (it != _ignorefunction.end() && it->second);
    }

    bool isexecutableblock(const std::string &path, const std::string &token) const {
        bool ret;
        if (acceptFile(path)) {
            std::list<std::string>::const_iterator it =
                std::find(_executableblocks.begin(), _executableblocks.end(), token);
            ret = it != _executableblocks.end();
        } else {
            ret = false;
        }
        return ret;
    }

    int blockstartoffset() const {
        return _codeblockoffset;
    }

    std::string blockstart() const {
        return _codeblockstart;
    }

    std::string blockend() const {
        return _codeblockend;
    }

    bool iskeyword(const std::string &keyword) const {
        std::list<std::string>::const_iterator it =
            std::find(_keywords.begin(), _keywords.end(), keyword);
        return it != _keywords.end();
    }

    bool isexporter(const std::string &prefix) const {
        std::map<std::string, exported_t>::const_iterator it =
            _exporters.find(prefix);
        return it != _exporters.end();
    }

    bool isexportedprefix(const std::string &prefix, const std::string &token) const {
        std::map<std::string, exported_t>::const_iterator it = _exporters.find(prefix);
        std::list<std::string>::const_iterator token_it;
        if (it != _exporters.end()) {
            token_it = std::find(it->second.prefixes.begin(), it->second.prefixes.end(), token);
            return token_it != it->second.prefixes.end();
        } else
            return false;
    }

    bool isexportedsuffix(const std::string &prefix, const std::string &token) const {
        std::map<std::string, exported_t>::const_iterator it = _exporters.find(prefix);
        std::list<std::string>::const_iterator token_it;
        if (it != _exporters.end()) {
            token_it = std::find(it->second.suffixes.begin(), it->second.suffixes.end(), token);
            return token_it != it->second.suffixes.end();
        } else
            return false;
    }

    bool isimporter(const std::string &importer) const {
        std::list<std::string>::const_iterator it =
            std::find(_importers.begin(), _importers.end(), importer);
        return it != _importers.end();
    }

    std::set<std::string> returnuninitdata;

private:
    typedef struct
    {
        std::list<std::string> prefixes;
        std::list<std::string> suffixes;
    } exported_t;
    int allocid;
    std::map<std::string, int> _alloc; // allocation functions
    std::map<std::string, int> _dealloc; // deallocation functions
    std::map<std::string, bool> _noreturn; // is function noreturn?
    std::map<std::string, bool> _ignorefunction; // ignore functions/macros from a library (gtk, qt etc)
    std::map<std::string, bool> _reporterrors;
    std::list<std::string> _fileextensions; // accepted file extensions
    std::list<std::string> _keywords; // keywords for code in the library
    std::list<std::string> _executableblocks; // keywords for blocks of executable code
    std::map<std::string, exported_t> _exporters; // keywords that export variables/functions to libraries (meta-code/macros)
    std::list<std::string> _importers; // keywords that import variables/functions
    std::string _codeblockstart;
    std::string _codeblockend;
    int _codeblockoffset;

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
