/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "mathlib.h"
#include "token.h"

#include <map>
#include <set>
#include <string>
#include <list>

class TokenList;
namespace tinyxml2 {
    class XMLDocument;
}

/// @addtogroup Core
/// @{

/**
 * @brief Library definitions handling
 */
class CPPCHECKLIB Library {
public:
    Library();

    enum ErrorCode { OK, FILE_NOT_FOUND, BAD_XML, UNKNOWN_ELEMENT, MISSING_ATTRIBUTE, BAD_ATTRIBUTE_VALUE, UNSUPPORTED_FORMAT, DUPLICATE_PLATFORM_TYPE, PLATFORM_TYPE_REDEFINED };

    class Error {
    public:
        Error() : errorcode(OK) , reason("") {}
        explicit Error(ErrorCode e) : errorcode(e) , reason("") {}
        Error(ErrorCode e, const std::string &r) : errorcode(e), reason(r) {}
        ErrorCode     errorcode;
        std::string   reason;
    };

    Error load(const char exename [], const char path []);
    Error load(const tinyxml2::XMLDocument &doc);

    /** this is primarily meant for unit tests. it only returns true/false */
    bool loadxmldata(const char xmldata[], std::size_t len);

    /** get allocation id for function by name */
    int alloc(const char name[]) const {
        return getid(_alloc, name);
    }

    /** get allocation id for function */
    int alloc(const Token *tok) const {
        return isNotLibraryFunction(tok) && argumentChecks.find(tok->str()) != argumentChecks.end() ? 0 : getid(_alloc, tok->str());
    }

    /** get deallocation id for function */
    int dealloc(const Token *tok) const {
        return isNotLibraryFunction(tok) && argumentChecks.find(tok->str()) != argumentChecks.end() ? 0 : getid(_dealloc, tok->str());
    }

    /** get deallocation id for function by name */
    int dealloc(const char name[]) const {
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

    bool formatstr_function(const std::string& funcname) const {
        return _formatstr.find(funcname) != _formatstr.cend();
    }

    bool formatstr_scan(const std::string& funcname) const {
        return _formatstr.at(funcname).first;
    }

    bool formatstr_secure(const std::string& funcname) const {
        return _formatstr.at(funcname).second;
    }

    std::set<std::string> use;
    std::set<std::string> leakignore;
    std::set<std::string> functionconst;
    std::set<std::string> functionpure;
    std::set<std::string> useretval;

    // returns true if ftok is not a library function
    bool isNotLibraryFunction(const Token *ftok) const;

    bool isnoreturn(const Token *ftok) const;
    bool isnotnoreturn(const Token *ftok) const;

    bool isScopeNoReturn(const Token *end, std::string *unknownFunc) const;

    class Container {
    public:
        Container() :
            type_templateArgNo(-1),
            size_templateArgNo(-1),
            arrayLike_indexOp(false),
            stdStringLike(false) {
        }

        enum Action {
            RESIZE, CLEAR, PUSH, POP, FIND,
            NO_ACTION
        };
        enum Yield {
            AT_INDEX, ITEM, BUFFER, BUFFER_NT, START_ITERATOR, END_ITERATOR, SIZE, EMPTY,
            NO_YIELD
        };
        struct Function {
            Action action;
            Yield yield;
        };
        std::string startPattern, endPattern;
        std::map<std::string, Function> functions;
        int type_templateArgNo;
        int size_templateArgNo;
        bool arrayLike_indexOp;
        bool stdStringLike;

        Action getAction(const std::string& function) const {
            std::map<std::string, Function>::const_iterator i = functions.find(function);
            if (i != functions.end())
                return i->second.action;
            return NO_ACTION;
        }

        Yield getYield(const std::string& function) const {
            std::map<std::string, Function>::const_iterator i = functions.find(function);
            if (i != functions.end())
                return i->second.yield;
            return NO_YIELD;
        }
    };
    std::map<std::string, Container> containers;
    const Container* detectContainer(const Token* typeStart) const;

    class ArgumentChecks {
    public:
        ArgumentChecks() :
            notbool(false),
            notnull(false),
            notuninit(false),
            formatstr(false),
            strz(false) {
        }

        bool         notbool;
        bool         notnull;
        bool         notuninit;
        bool         formatstr;
        bool         strz;
        std::string  valid;

        class MinSize {
        public:
            enum Type {NONE,STRLEN,ARGVALUE,SIZEOF,MUL};
            MinSize(Type t, int a) : type(t), arg(a), arg2(0) {}
            Type type;
            int arg;
            int arg2;
        };
        std::list<MinSize> minsizes;
    };

    // function name, argument nr => argument data
    std::map<std::string, std::map<int, ArgumentChecks> > argumentChecks;

    bool isboolargbad(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->notbool;
    }

    bool isnullargbad(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->notnull;
    }

    bool isuninitargbad(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->notuninit;
    }

    bool isargformatstr(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->formatstr;
    }

    bool isargstrz(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->strz;
    }

    bool isargvalid(const Token *ftok, int argnr, const MathLib::bigint argvalue) const;

    const std::string& validarg(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg ? arg->valid : emptyString;
    }

    bool hasminsize(const std::string &functionName) const {
        std::map<std::string, std::map<int, ArgumentChecks> >::const_iterator it1;
        it1 = argumentChecks.find(functionName);
        if (it1 == argumentChecks.end())
            return false;
        std::map<int,ArgumentChecks>::const_iterator it2;
        for (it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
            if (!it2->second.minsizes.empty())
                return true;
        }
        return false;
    }

    const std::list<ArgumentChecks::MinSize> *argminsizes(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg ? &arg->minsizes : nullptr;
    }

    bool markupFile(const std::string &path) const;

    bool processMarkupAfterCode(const std::string &path) const;

    const std::set<std::string> &markupExtensions() const {
        return _markupExtensions;
    }

    bool reportErrors(const std::string &path) const;

    bool ignorefunction(const std::string &function) const {
        return (_ignorefunction.find(function) != _ignorefunction.end());
    }

    bool isexecutableblock(const std::string &file, const std::string &token) const;

    int blockstartoffset(const std::string &file) const;

    const std::string& blockstart(const std::string &file) const;
    const std::string& blockend(const std::string &file) const;

    bool iskeyword(const std::string &file, const std::string &keyword) const;

    bool isexporter(const std::string &prefix) const {
        return _exporters.find(prefix) != _exporters.end();
    }

    bool isexportedprefix(const std::string &prefix, const std::string &token) const {
        const std::map<std::string, ExportedFunctions>::const_iterator it = _exporters.find(prefix);
        return (it != _exporters.end() && it->second.isPrefix(token));
    }

    bool isexportedsuffix(const std::string &prefix, const std::string &token) const {
        const std::map<std::string, ExportedFunctions>::const_iterator it = _exporters.find(prefix);
        return (it != _exporters.end() && it->second.isSuffix(token));
    }

    bool isimporter(const std::string& file, const std::string &importer) const;

    bool isreflection(const std::string &token) const {
        const std::map<std::string,int>::const_iterator it
            = _reflection.find(token);
        return it != _reflection.end();
    }

    int reflectionArgument(const std::string &token) const {
        int argIndex = -1;
        const std::map<std::string,int>::const_iterator it
            = _reflection.find(token);
        if (it != _reflection.end()) {
            argIndex = it->second;
        }
        return argIndex;
    }

    std::set<std::string> returnuninitdata;
    std::vector<std::string> defines; // to provide some library defines

    struct PodType {
        unsigned int   size;
        char           sign;
    };
    const struct PodType *podtype(const std::string &name) const {
        const std::map<std::string, struct PodType>::const_iterator it = podtypes.find(name);
        return (it != podtypes.end()) ? &(it->second) : nullptr;
    }

    struct PlatformType {
        PlatformType()
            : _signed(false)
            , _unsigned(false)
            , _long(false)
            , _pointer(false)
            , _ptr_ptr(false)
            , _const_ptr(false) {
        }
        bool operator == (const PlatformType & type) const {
            return (_type == type._type &&
                    _signed == type._signed &&
                    _unsigned == type._unsigned &&
                    _long == type._long &&
                    _pointer == type._pointer &&
                    _ptr_ptr == type._ptr_ptr &&
                    _const_ptr == type._const_ptr);
        }
        bool operator != (const PlatformType & type) const {
            return !(*this == type);
        }
        std::string _type;
        bool _signed;
        bool _unsigned;
        bool _long;
        bool _pointer;
        bool _ptr_ptr;
        bool _const_ptr;
    };

    struct Platform {
        const PlatformType *platform_type(const std::string &name) const {
            const std::map<std::string, struct PlatformType>::const_iterator it = _platform_types.find(name);
            return (it != _platform_types.end()) ? &(it->second) : nullptr;
        }
        std::map<std::string, PlatformType> _platform_types;
    };

    const PlatformType *platform_type(const std::string &name, const std::string & platform) const {
        const std::map<std::string, Platform>::const_iterator it = platforms.find(platform);

        if (it != platforms.end()) {
            const PlatformType * const type = it->second.platform_type(name);

            if (type)
                return type;
        }

        const std::map<std::string, PlatformType>::const_iterator it2 = platform_types.find(name);

        return (it2 != platform_types.end()) ? &(it2->second) : nullptr;
    }

private:
    class ExportedFunctions {
    public:
        void addPrefix(const std::string& prefix) {
            _prefixes.insert(prefix);
        }
        void addSuffix(const std::string& suffix) {
            _suffixes.insert(suffix);
        }
        bool isPrefix(const std::string& prefix) const {
            return (_prefixes.find(prefix) != _prefixes.end());
        }
        bool isSuffix(const std::string& suffix) const {
            return (_suffixes.find(suffix) != _suffixes.end());
        }

    private:
        std::set<std::string> _prefixes;
        std::set<std::string> _suffixes;
    };
    class CodeBlock {
    public:
        CodeBlock() : _offset(0) {}

        void setStart(const std::string& s) {
            _start = s;
        }
        void setEnd(const std::string& e) {
            _end = e;
        }
        void setOffset(const int o) {
            _offset = o;
        }
        void addBlock(const std::string& blockName) {
            _blocks.insert(blockName);
        }
        const std::string& start() const {
            return _start;
        }
        const std::string& end() const {
            return _end;
        }
        int offset() const {
            return _offset;
        }
        bool isBlock(const std::string& blockName) const {
            return _blocks.find(blockName) != _blocks.end();
        }

    private:
        std::string _start;
        std::string _end;
        int _offset;
        std::set<std::string> _blocks;
    };
    int allocid;
    std::set<std::string> _files;
    std::map<std::string, int> _alloc; // allocation functions
    std::map<std::string, int> _dealloc; // deallocation functions
    std::map<std::string, bool> _noreturn; // is function noreturn?
    std::set<std::string> _ignorefunction; // ignore functions/macros from a library (gtk, qt etc)
    std::map<std::string, bool> _reporterrors;
    std::map<std::string, bool> _processAfterCode;
    std::set<std::string> _markupExtensions; // file extensions of markup files
    std::map<std::string, std::set<std::string> > _keywords; // keywords for code in the library
    std::map<std::string, CodeBlock> _executableblocks; // keywords for blocks of executable code
    std::map<std::string, ExportedFunctions> _exporters; // keywords that export variables/functions to libraries (meta-code/macros)
    std::map<std::string, std::set<std::string> > _importers; // keywords that import variables/functions
    std::map<std::string,int> _reflection; // invocation of reflection
    std::map<std::string, std::pair<bool, bool> > _formatstr; // Parameters for format string checking
    std::map<std::string, struct PodType> podtypes; // pod types
    std::map<std::string, PlatformType> platform_types; // platform independent typedefs
    std::map<std::string, Platform> platforms; // platform dependent typedefs

    const ArgumentChecks * getarg(const Token *ftok, int argnr) const;

    static int getid(const std::map<std::string,int> &data, const std::string &name) {
        const std::map<std::string,int>::const_iterator it = data.find(name);
        return (it == data.end()) ? 0 : it->second;
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // libraryH
