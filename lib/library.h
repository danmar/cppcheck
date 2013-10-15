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
        const std::list<std::string>::const_iterator it =
            std::find(_fileextensions.begin(), _fileextensions.end(), extension);
        return it != _fileextensions.end();
    }

    bool reportErrors(const std::string &path) const {
        const std::map<std::string, bool>::const_iterator it =
                _reporterrors.find(Path::getFilenameExtensionInLowerCase(path));
        if (it != _reporterrors.end()) {
            return it->second;
        }
        // assume true if we don't know as it'll be a core-type (c/cpp etc)
        return true;
    }

    bool ignorefunction(const std::string &function) const {
        const std::map<std::string, bool>::const_iterator it = _ignorefunction.find(function);
        return (it != _ignorefunction.end() && it->second);
    }

    bool isexecutableblock(const std::string &file, const std::string &token) const {
        bool isexecblock;
        const std::map<std::string, codeBlocks_t>::const_iterator map_it
                = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));

        if (map_it != _executableblocks.end()) {
            const std::list<std::string>::const_iterator it =
                std::find(map_it->second.blocks.begin(), map_it->second.blocks.end(), token);
            isexecblock = it != map_it->second.blocks.end();
        } else {
            isexecblock = false;
        }
        return isexecblock;
    }

    int blockstartoffset(const std::string &file) const {
        int offset = -1;
        const std::map<std::string, codeBlocks_t>::const_iterator map_it
                = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));

        if (map_it != _executableblocks.end()) {
            offset = map_it->second.offset;
        }
        return offset;
    }

    std::string blockstart(const std::string &file) const {
        std::string start;
        const std::map<std::string, codeBlocks_t>::const_iterator map_it
                = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));

        if (map_it != _executableblocks.end()) {
            start = map_it->second.start;
        }
        return start;
    }

    std::string blockend(const std::string &file) const {
        std::string end;
        const std::map<std::string, codeBlocks_t>::const_iterator map_it
                = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));

        if (map_it != _executableblocks.end()) {
            end = map_it->second.end;
        }
        return end;
    }

    bool iskeyword(const std::string &file, const std::string &keyword) const {
        bool iskw;
        const std::map<std::string, std::list<std::string> >::const_iterator it =
                _keywords.find(Path::getFilenameExtensionInLowerCase(file));

        if (it != _keywords.end()) {
            const std::list<std::string> list = it->second;
            const std::list<std::string>::const_iterator list_it =
                std::find(list.begin(), list.end(), keyword);
            iskw = list_it != list.end();
        } else {
            iskw = false;
        }
        return iskw;
    }

    bool isexporter(const std::string &prefix) const {
        const std::map<std::string, exported_t>::const_iterator it =
            _exporters.find(prefix);
        return it != _exporters.end();
    }

    bool isexportedprefix(const std::string &prefix, const std::string &token) const {
        const std::map<std::string, exported_t>::const_iterator it = _exporters.find(prefix);
        std::list<std::string>::const_iterator token_it;
        if (it != _exporters.end()) {
            token_it = std::find(it->second.prefixes.begin(), it->second.prefixes.end(), token);
            return token_it != it->second.prefixes.end();
        } else
            return false;
    }

    bool isexportedsuffix(const std::string &prefix, const std::string &token) const {
        const std::map<std::string, exported_t>::const_iterator it = _exporters.find(prefix);
        std::list<std::string>::const_iterator token_it;
        if (it != _exporters.end()) {
            token_it = std::find(it->second.suffixes.begin(), it->second.suffixes.end(), token);
            return token_it != it->second.suffixes.end();
        } else
            return false;
    }

    bool isimporter(const std::string& file, const std::string &importer) const {
        bool isImporter;
        const std::map<std::string, std::list<std::string> >::const_iterator it =
                _importers.find(Path::getFilenameExtensionInLowerCase(file));

        if (it != _importers.end()) {
            const std::list<std::string> list = it->second;
            const std::list<std::string>::const_iterator it2 =
                std::find(list.begin(), list.end(), importer);
            isImporter = (it2 != list.end());
        } else {
            isImporter = false;
        }
        return isImporter;
    }

    bool isreflection(const std::string& file, const std::string &token) const {
        bool isReflecMethod;
        const std::map<std::string,std::map<std::string,int> >::const_iterator it
                = _reflection.find(Path::getFilenameExtensionInLowerCase(file));
        if (it != _reflection.end())
        {
            const std::map<std::string,int>::const_iterator it2 =
                it->second.find(token);
            isReflecMethod = it2 != it->second.end();
        } else {
            isReflecMethod = false;
        }
        return isReflecMethod;
    }

    int reflectionArgument(const std::string& file, const std::string &token) const {
        int argIndex = -1;
        const std::map<std::string,std::map<std::string,int> >::const_iterator it
                = _reflection.find(Path::getFilenameExtensionInLowerCase(file));
        if (it != _reflection.end())
        {
            const std::map<std::string,int>::const_iterator it2 =
                it->second.find(token);
            if (it2 != it->second.end()) {
                argIndex = it2->second;
            }
        }
        return argIndex;
    }

    std::set<std::string> returnuninitdata;

private:
    typedef struct
    {
        std::list<std::string> prefixes;
        std::list<std::string> suffixes;
    } exported_t;
    int allocid;
    typedef struct
    {
        std::string start;
        std::string end;
        int offset;
        std::list<std::string> blocks;
    } codeBlocks_t;
    std::map<std::string, int> _alloc; // allocation functions
    std::map<std::string, int> _dealloc; // deallocation functions
    std::map<std::string, bool> _noreturn; // is function noreturn?
    std::map<std::string, bool> _ignorefunction; // ignore functions/macros from a library (gtk, qt etc)
    std::map<std::string, bool> _reporterrors;
    std::list<std::string> _fileextensions; // accepted file extensions
    std::map<std::string, std::list<std::string> > _keywords; // keywords for code in the library
    std::map<std::string, codeBlocks_t> _executableblocks; // keywords for blocks of executable code
    std::map<std::string, exported_t> _exporters; // keywords that export variables/functions to libraries (meta-code/macros)
    std::map<std::string, std::list<std::string> > _importers; // keywords that import variables/functions
    std::map<std::string,std::map<std::string,int> > _reflection; // invokation of reflection


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
