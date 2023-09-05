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

//---------------------------------------------------------------------------
#ifndef libraryH
#define libraryH
//---------------------------------------------------------------------------

#include "config.h"
#include "mathlib.h"
#include "errortypes.h"
#include "standards.h"

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class Token;
class Settings;

namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
}

/// @addtogroup Core
/// @{

/**
 * @brief Library definitions handling
 */
class CPPCHECKLIB Library {
    // TODO: get rid of this
    friend class TestSymbolDatabase; // For testing only
    friend class TestSingleExecutorBase; // For testing only
    friend class TestThreadExecutor; // For testing only
    friend class TestProcessExecutor; // For testing only

public:
    Library() = default;

    enum class ErrorCode { OK, FILE_NOT_FOUND, BAD_XML, UNKNOWN_ELEMENT, MISSING_ATTRIBUTE, BAD_ATTRIBUTE_VALUE, UNSUPPORTED_FORMAT, DUPLICATE_PLATFORM_TYPE, PLATFORM_TYPE_REDEFINED };

    class Error {
    public:
        Error() : errorcode(ErrorCode::OK) {}
        explicit Error(ErrorCode e) : errorcode(e) {}
        template<typename T>
        Error(ErrorCode e, T&& r) : errorcode(e), reason(r) {}
        ErrorCode errorcode;
        std::string reason;
    };

    Error load(const char exename[], const char path[]);
    Error load(const tinyxml2::XMLDocument &doc);

    /** this is used for unit tests */
    bool loadxmldata(const char xmldata[], std::size_t len);

    struct AllocFunc {
        int groupId;
        int arg;
        enum class BufferSize {none,malloc,calloc,strdup};
        BufferSize bufferSize;
        int bufferSizeArg1;
        int bufferSizeArg2;
        int reallocArg;
        bool initData;
    };

    /** get allocation info for function */
    const AllocFunc* getAllocFuncInfo(const Token *tok) const;

    /** get deallocation info for function */
    const AllocFunc* getDeallocFuncInfo(const Token *tok) const;

    /** get reallocation info for function */
    const AllocFunc* getReallocFuncInfo(const Token *tok) const;

    /** get allocation id for function */
    int getAllocId(const Token *tok, int arg) const;

    /** get deallocation id for function */
    int getDeallocId(const Token *tok, int arg) const;

    /** get reallocation id for function */
    int getReallocId(const Token *tok, int arg) const;

    /** get allocation info for function by name (deprecated, use other alloc) */
    const AllocFunc* getAllocFuncInfo(const char name[]) const {
        return getAllocDealloc(mAlloc, name);
    }

    /** get deallocation info for function by name (deprecated, use other alloc) */
    const AllocFunc* getDeallocFuncInfo(const char name[]) const {
        return getAllocDealloc(mDealloc, name);
    }

    /** get allocation id for function by name (deprecated, use other alloc) */
    int allocId(const char name[]) const {
        const AllocFunc* af = getAllocDealloc(mAlloc, name);
        return af ? af->groupId : 0;
    }

    /** get deallocation id for function by name (deprecated, use other alloc) */
    int deallocId(const char name[]) const {
        const AllocFunc* af = getAllocDealloc(mDealloc, name);
        return af ? af->groupId : 0;
    }

    /** set allocation id for function */
    void setalloc(const std::string &functionname, int id, int arg) {
        mAlloc[functionname].groupId = id;
        mAlloc[functionname].arg = arg;
    }

    void setdealloc(const std::string &functionname, int id, int arg) {
        mDealloc[functionname].groupId = id;
        mDealloc[functionname].arg = arg;
    }

    void setrealloc(const std::string &functionname, int id, int arg, int reallocArg = 1) {
        mRealloc[functionname].groupId = id;
        mRealloc[functionname].arg = arg;
        mRealloc[functionname].reallocArg = reallocArg;
    }

    /** add noreturn function setting */
    void setnoreturn(const std::string& funcname, bool noreturn) {
        mNoReturn[funcname] = noreturn ? FalseTrueMaybe::True : FalseTrueMaybe::False;
    }

    static bool isCompliantValidationExpression(const char* p);

    /** is allocation type memory? */
    static bool ismemory(const int id) {
        return ((id > 0) && ((id & 1) == 0));
    }
    static bool ismemory(const AllocFunc* const func) {
        return func && (func->groupId > 0) && ((func->groupId & 1) == 0);
    }

    /** is allocation type resource? */
    static bool isresource(const int id) {
        return ((id > 0) && ((id & 1) == 1));
    }
    static bool isresource(const AllocFunc* const func) {
        return func && (func->groupId > 0) && ((func->groupId & 1) == 1);
    }

    bool formatstr_function(const Token* ftok) const;
    int formatstr_argno(const Token* ftok) const;
    bool formatstr_scan(const Token* ftok) const;
    bool formatstr_secure(const Token* ftok) const;

    struct NonOverlappingData {
        int ptr1Arg;
        int ptr2Arg;
        int sizeArg;
        int strlenArg;
    };
    const NonOverlappingData* getNonOverlappingData(const Token *ftok) const;

    struct WarnInfo {
        std::string message;
        Standards standards;
        Severity::SeverityType severity;
    };
    std::map<std::string, WarnInfo> functionwarn;

    const WarnInfo* getWarnInfo(const Token* ftok) const;

    // returns true if ftok is not a library function
    bool isNotLibraryFunction(const Token *ftok) const;
    bool matchArguments(const Token *ftok, const std::string &functionName) const;

    enum class UseRetValType { NONE, DEFAULT, ERROR_CODE };
    UseRetValType getUseRetValType(const Token* ftok) const;

    const std::string& returnValue(const Token *ftok) const;
    const std::string& returnValueType(const Token *ftok) const;
    int returnValueContainer(const Token *ftok) const;
    std::vector<MathLib::bigint> unknownReturnValues(const Token *ftok) const;

    bool isnoreturn(const Token *ftok) const;
    bool isnotnoreturn(const Token *ftok) const;

    bool isScopeNoReturn(const Token *end, std::string *unknownFunc) const;

    class Container {
    public:
        Container() = default;

        enum class Action {
            RESIZE,
            CLEAR,
            PUSH,
            POP,
            FIND,
            INSERT,
            ERASE,
            CHANGE_CONTENT,
            CHANGE,
            CHANGE_INTERNAL,
            NO_ACTION
        };
        enum class Yield {
            AT_INDEX,
            ITEM,
            BUFFER,
            BUFFER_NT,
            START_ITERATOR,
            END_ITERATOR,
            ITERATOR,
            SIZE,
            EMPTY,
            NO_YIELD
        };
        struct Function {
            Action action;
            Yield yield;
            std::string returnType;
        };
        struct RangeItemRecordTypeItem {
            std::string name;
            int templateParameter; // TODO: use this
        };
        std::string startPattern, startPattern2, endPattern, itEndPattern;
        std::map<std::string, Function> functions;
        int type_templateArgNo = -1;
        std::vector<RangeItemRecordTypeItem> rangeItemRecordType;
        int size_templateArgNo = -1;
        bool arrayLike_indexOp{};
        bool stdStringLike{};
        bool stdAssociativeLike{};
        bool opLessAllowed = true;
        bool hasInitializerListConstructor{};
        bool unstableErase{};
        bool unstableInsert{};
        bool view{};

        Action getAction(const std::string& function) const {
            const std::map<std::string, Function>::const_iterator i = functions.find(function);
            if (i != functions.end())
                return i->second.action;
            return Action::NO_ACTION;
        }

        Yield getYield(const std::string& function) const {
            const std::map<std::string, Function>::const_iterator i = functions.find(function);
            if (i != functions.end())
                return i->second.yield;
            return Yield::NO_YIELD;
        }

        const std::string& getReturnType(const std::string& function) const {
            auto i = functions.find(function);
            return (i != functions.end()) ? i->second.returnType : emptyString;
        }

        static Yield yieldFrom(const std::string& yieldName);
        static Action actionFrom(const std::string& actionName);
    };
    std::unordered_map<std::string, Container> containers;
    const Container* detectContainer(const Token* typeStart) const;
    const Container* detectIterator(const Token* typeStart) const;
    const Container* detectContainerOrIterator(const Token* typeStart, bool* isIterator = nullptr, bool withoutStd = false) const;

    struct ArgumentChecks {
        bool notbool{};
        bool notnull{};
        int notuninit = -1;
        bool formatstr{};
        bool strz{};
        bool optional{};
        bool variadic{};
        std::string valid;

        struct IteratorInfo {
            int container{};
            bool it{};
            bool first{};
            bool last{};
        };
        IteratorInfo iteratorInfo;

        struct MinSize {
            enum class Type { NONE, STRLEN, ARGVALUE, SIZEOF, MUL, VALUE };
            MinSize(Type t, int a) : type(t), arg(a) {}
            Type type;
            int arg;
            int arg2 = 0;
            long long value = 0;
            std::string baseType;
        };
        std::vector<MinSize> minsizes;

        enum class Direction {
            DIR_IN,     ///< Input to called function. Data is treated as read-only.
            DIR_OUT,    ///< Output to caller. Data is passed by reference or address and is potentially written.
            DIR_INOUT,  ///< Input to called function, and output to caller. Data is passed by reference or address and is potentially modified.
            DIR_UNKNOWN ///< direction not known / specified
        };
        Direction direction = Direction::DIR_UNKNOWN;
    };

    struct Function {
        std::map<int, ArgumentChecks> argumentChecks; // argument nr => argument data
        bool use{};
        bool leakignore{};
        bool isconst{};
        bool ispure{};
        UseRetValType useretval = UseRetValType::NONE;
        bool ignore{};  // ignore functions/macros from a library (gtk, qt etc)
        bool formatstr{};
        bool formatstr_scan{};
        bool formatstr_secure{};
        Container::Action containerAction = Container::Action::NO_ACTION;
        Container::Yield containerYield = Container::Yield::NO_YIELD;
        std::string returnType;
    };

    const Function *getFunction(const Token *ftok) const;
    std::unordered_map<std::string, Function> functions;
    bool isUse(const std::string& functionName) const;
    bool isLeakIgnore(const std::string& functionName) const;
    bool isFunctionConst(const std::string& functionName, bool pure) const;
    bool isFunctionConst(const Token *ftok) const;

    bool isboolargbad(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->notbool;
    }

    bool isnullargbad(const Token *ftok, int argnr) const;
    bool isuninitargbad(const Token *ftok, int argnr, int indirect = 0, bool *hasIndirect=nullptr) const;

    bool isargformatstr(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->formatstr;
    }

    bool isargstrz(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->strz;
    }

    bool isIntArgValid(const Token *ftok, int argnr, const MathLib::bigint argvalue) const;
    bool isFloatArgValid(const Token *ftok, int argnr, double argvalue) const;

    const std::string& validarg(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg ? arg->valid : emptyString;
    }

    const ArgumentChecks::IteratorInfo *getArgIteratorInfo(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->iteratorInfo.it ? &arg->iteratorInfo : nullptr;
    }

    bool hasminsize(const Token *ftok) const;

    const std::vector<ArgumentChecks::MinSize> *argminsizes(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg ? &arg->minsizes : nullptr;
    }

    ArgumentChecks::Direction getArgDirection(const Token* ftok, int argnr) const;

    bool markupFile(const std::string &path) const;

    bool processMarkupAfterCode(const std::string &path) const;

    const std::set<std::string> &markupExtensions() const {
        return mMarkupExtensions;
    }

    bool reportErrors(const std::string &path) const;

    bool ignorefunction(const std::string &functionName) const;

    bool isexecutableblock(const std::string &file, const std::string &token) const;

    int blockstartoffset(const std::string &file) const;

    const std::string& blockstart(const std::string &file) const;
    const std::string& blockend(const std::string &file) const;

    bool iskeyword(const std::string &file, const std::string &keyword) const;

    bool isexporter(const std::string &prefix) const {
        return mExporters.find(prefix) != mExporters.end();
    }

    bool isexportedprefix(const std::string &prefix, const std::string &token) const {
        const std::map<std::string, ExportedFunctions>::const_iterator it = mExporters.find(prefix);
        return (it != mExporters.end() && it->second.isPrefix(token));
    }

    bool isexportedsuffix(const std::string &prefix, const std::string &token) const {
        const std::map<std::string, ExportedFunctions>::const_iterator it = mExporters.find(prefix);
        return (it != mExporters.end() && it->second.isSuffix(token));
    }

    bool isimporter(const std::string& file, const std::string &importer) const;

    const Token* getContainerFromYield(const Token* tok, Container::Yield yield) const;
    const Token* getContainerFromAction(const Token* tok, Container::Action action) const;

    bool isreflection(const std::string &token) const {
        return mReflection.find(token) != mReflection.end();
    }

    int reflectionArgument(const std::string &token) const {
        const std::map<std::string, int>::const_iterator it = mReflection.find(token);
        if (it != mReflection.end())
            return it->second;
        return -1;
    }

    bool isentrypoint(const std::string &func) const {
        return func == "main" || mEntrypoints.find(func) != mEntrypoints.end();
    }

    std::vector<std::string> defines; // to provide some library defines

    struct SmartPointer {
        std::string name;
        bool unique = false;
    };

    std::unordered_map<std::string, SmartPointer> smartPointers;
    bool isSmartPointer(const Token *tok) const;
    const SmartPointer* detectSmartPointer(const Token* tok, bool withoutStd = false) const;

    struct PodType {
        unsigned int size;
        char sign;
        enum class Type { NO, BOOL, CHAR, SHORT, INT, LONG, LONGLONG } stdtype;
    };
    const struct PodType *podtype(const std::string &name) const {
        const std::unordered_map<std::string, struct PodType>::const_iterator it = mPodTypes.find(name);
        return (it != mPodTypes.end()) ? &(it->second) : nullptr;
    }

    struct PlatformType {
        bool operator == (const PlatformType & type) const {
            return (mSigned == type.mSigned &&
                    mUnsigned == type.mUnsigned &&
                    mLong == type.mLong &&
                    mPointer == type.mPointer &&
                    mPtrPtr == type.mPtrPtr &&
                    mConstPtr == type.mConstPtr &&
                    mType == type.mType);
        }
        bool operator != (const PlatformType & type) const {
            return !(*this == type);
        }
        std::string mType;
        bool mSigned{};
        bool mUnsigned{};
        bool mLong{};
        bool mPointer{};
        bool mPtrPtr{};
        bool mConstPtr{};
    };

    struct Platform {
        const PlatformType *platform_type(const std::string &name) const {
            const std::map<std::string, struct PlatformType>::const_iterator it = mPlatformTypes.find(name);
            return (it != mPlatformTypes.end()) ? &(it->second) : nullptr;
        }
        std::map<std::string, PlatformType> mPlatformTypes;
    };

    const PlatformType *platform_type(const std::string &name, const std::string & platform) const {
        const std::map<std::string, Platform>::const_iterator it = mPlatforms.find(platform);
        if (it != mPlatforms.end()) {
            const PlatformType * const type = it->second.platform_type(name);
            if (type)
                return type;
        }

        const std::map<std::string, PlatformType>::const_iterator it2 = mPlatformTypes.find(name);
        return (it2 != mPlatformTypes.end()) ? &(it2->second) : nullptr;
    }

    /**
     * Get function name for function call
     */
    std::string getFunctionName(const Token *ftok) const;

    static bool isContainerYield(const Token * const cond, Library::Container::Yield y, const std::string& fallback=emptyString);

    /** Suppress/check a type */
    enum class TypeCheck { def,
                           check,
                           suppress,
                           checkFiniteLifetime, // (unusedvar) object has side effects, but immediate destruction is wrong
    };
    TypeCheck getTypeCheck(std::string check, std::string typeName) const;
    bool hasAnyTypeCheck(const std::string& typeName) const;

private:
    // load a <function> xml node
    Error loadFunction(const tinyxml2::XMLElement * const node, const std::string &name, std::set<std::string> &unknown_elements);

    class ExportedFunctions {
    public:
        void addPrefix(std::string prefix) {
            mPrefixes.insert(std::move(prefix));
        }
        void addSuffix(std::string suffix) {
            mSuffixes.insert(std::move(suffix));
        }
        bool isPrefix(const std::string& prefix) const {
            return (mPrefixes.find(prefix) != mPrefixes.end());
        }
        bool isSuffix(const std::string& suffix) const {
            return (mSuffixes.find(suffix) != mSuffixes.end());
        }

    private:
        std::set<std::string> mPrefixes;
        std::set<std::string> mSuffixes;
    };
    class CodeBlock {
    public:
        CodeBlock() = default;

        void setStart(const char* s) {
            mStart = s;
        }
        void setEnd(const char* e) {
            mEnd = e;
        }
        void setOffset(const int o) {
            mOffset = o;
        }
        void addBlock(const char* blockName) {
            mBlocks.insert(blockName);
        }
        const std::string& start() const {
            return mStart;
        }
        const std::string& end() const {
            return mEnd;
        }
        int offset() const {
            return mOffset;
        }
        bool isBlock(const std::string& blockName) const {
            return mBlocks.find(blockName) != mBlocks.end();
        }

    private:
        std::string mStart;
        std::string mEnd;
        int mOffset{};
        std::set<std::string> mBlocks;
    };
    enum class FalseTrueMaybe { False, True, Maybe };
    int mAllocId{};
    std::set<std::string> mFiles;
    std::map<std::string, AllocFunc> mAlloc; // allocation functions
    std::map<std::string, AllocFunc> mDealloc; // deallocation functions
    std::map<std::string, AllocFunc> mRealloc; // reallocation functions
    std::unordered_map<std::string, FalseTrueMaybe> mNoReturn; // is function noreturn?
    std::map<std::string, std::string> mReturnValue;
    std::map<std::string, std::string> mReturnValueType;
    std::map<std::string, int> mReturnValueContainer;
    std::map<std::string, std::vector<MathLib::bigint>> mUnknownReturnValues;
    std::map<std::string, bool> mReportErrors;
    std::map<std::string, bool> mProcessAfterCode;
    std::set<std::string> mMarkupExtensions; // file extensions of markup files
    std::map<std::string, std::set<std::string>> mKeywords;  // keywords for code in the library
    std::unordered_map<std::string, CodeBlock> mExecutableBlocks; // keywords for blocks of executable code
    std::map<std::string, ExportedFunctions> mExporters; // keywords that export variables/functions to libraries (meta-code/macros)
    std::map<std::string, std::set<std::string>> mImporters;  // keywords that import variables/functions
    std::map<std::string, int> mReflection; // invocation of reflection
    std::unordered_map<std::string, struct PodType> mPodTypes; // pod types
    std::map<std::string, PlatformType> mPlatformTypes; // platform independent typedefs
    std::map<std::string, Platform> mPlatforms; // platform dependent typedefs
    std::map<std::pair<std::string,std::string>, TypeCheck> mTypeChecks;
    std::unordered_map<std::string, NonOverlappingData> mNonOverlappingData;
    std::unordered_set<std::string> mEntrypoints;

    const ArgumentChecks * getarg(const Token *ftok, int argnr) const;

    std::string getFunctionName(const Token *ftok, bool &error) const;

    static const AllocFunc* getAllocDealloc(const std::map<std::string, AllocFunc> &data, const std::string &name) {
        const std::map<std::string, AllocFunc>::const_iterator it = data.find(name);
        return (it == data.end()) ? nullptr : &it->second;
    }

    enum DetectContainer { ContainerOnly, IteratorOnly, Both };
    const Library::Container* detectContainerInternal(const Token* typeStart, DetectContainer detect, bool* isIterator = nullptr, bool withoutStd = false) const;
};

CPPCHECKLIB const Library::Container * getLibraryContainer(const Token * tok);

std::shared_ptr<Token> createTokenFromExpression(const std::string& returnValue,
                                                 const Settings* settings,
                                                 std::unordered_map<nonneg int, const Token*>* lookupVarId = nullptr);

/// @}
//---------------------------------------------------------------------------
#endif // libraryH
