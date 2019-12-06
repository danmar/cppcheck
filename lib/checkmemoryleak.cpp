/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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


#include "checkmemoryleak.h"

#include "astutils.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"

#include <algorithm>
#include <cstddef>
#include <set>
#include <stack>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckMemoryLeakInFunction instance1;
    CheckMemoryLeakInClass instance2;
    CheckMemoryLeakStructMember instance3;
    CheckMemoryLeakNoVar instance4;
}

// CWE ID used:
static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE401(401U);  // Improper Release of Memory Before Removing Last Reference ('Memory Leak')
static const CWE CWE771(771U);  // Missing Reference to Active Allocated Resource
static const CWE CWE772(772U);  // Missing Release of Resource after Effective Lifetime


/** List of functions that can be ignored when searching for memory leaks.
 * These functions don't take the address of the given pointer
 * This list contains function names with const parameters e.g.: atof(const char *)
 * TODO: This list should be replaced by <leak-ignore/> in .cfg files.
 */
static const std::set<std::string> call_func_white_list = {
    "_open", "_wopen", "access", "adjtime", "asctime_r", "asprintf", "chdir", "chmod", "chown"
    , "creat", "ctime_r", "execl", "execle", "execlp", "execv", "execve", "fchmod", "fcntl"
    , "fdatasync", "fclose", "flock", "fmemopen", "fnmatch", "fopen", "fopencookie", "for", "free"
    , "freopen", "fseeko", "fstat", "fsync", "ftello", "ftruncate", "getgrnam", "gethostbyaddr", "gethostbyname"
    , "getnetbyname", "getopt", "getopt_long", "getprotobyname", "getpwnam", "getservbyname", "getservbyport"
    , "glob", "gmtime", "gmtime_r", "if", "index", "inet_addr", "inet_aton", "inet_network", "initgroups"
    , "ioctl", "link", "localtime_r", "lockf", "lseek", "lstat", "mkdir", "mkfifo", "mknod", "mkstemp"
    , "obstack_printf", "obstack_vprintf", "open", "opendir", "parse_printf_format", "pathconf"
    , "perror", "popen", "posix_fadvise", "posix_fallocate", "pread", "psignal", "pwrite", "read", "readahead"
    , "readdir", "readdir_r", "readlink", "readv", "realloc", "regcomp", "return", "rewinddir", "rindex"
    , "rmdir", "scandir", "seekdir", "setbuffer", "sethostname", "setlinebuf", "sizeof", "strdup"
    , "stat", "stpcpy", "strcasecmp", "stricmp", "strncasecmp", "switch"
    , "symlink", "sync_file_range", "telldir", "tempnam", "time", "typeid", "unlink"
    , "utime", "utimes", "vasprintf", "while", "wordexp", "write", "writev"
};

//---------------------------------------------------------------------------

CheckMemoryLeak::AllocType CheckMemoryLeak::getAllocationType(const Token *tok2, nonneg int varid, std::list<const Function*> *callstack) const
{
    // What we may have...
    //     * var = (char *)malloc(10);
    //     * var = new char[10];
    //     * var = strdup("hello");
    //     * var = strndup("hello", 3);
    if (tok2 && tok2->str() == "(") {
        tok2 = tok2->link();
        tok2 = tok2 ? tok2->next() : nullptr;
    }
    if (! tok2)
        return No;
    if (tok2->str() == "::")
        tok2 = tok2->next();
    if (! tok2->isName())
        return No;

    if (!Token::Match(tok2, "%name% ::|. %type%")) {
        // Using realloc..
        AllocType reallocType = getReallocationType(tok2, varid);
        if (reallocType != No)
            return reallocType;

        if (mTokenizer_->isCPP() && tok2->str() == "new") {
            if (tok2->strAt(1) == "(" && !Token::Match(tok2->next(),"( std| ::| nothrow )"))
                return No;
            if (tok2->astOperand1() && (tok2->astOperand1()->str() == "[" || (tok2->astOperand1()->astOperand1() && tok2->astOperand1()->astOperand1()->str() == "[")))
                return NewArray;
            const Token *typeTok = tok2->next();
            while (Token::Match(typeTok, "%name% :: %name%"))
                typeTok = typeTok->tokAt(2);
            if (typeTok->type() && typeTok->type()->isClassType()) {
                const Scope *classScope = typeTok->type()->classScope;
                if (classScope && classScope->numConstructors > 0)
                    return No;
            }
            return New;
        }

        if (mSettings_->posix()) {
            if (Token::Match(tok2, "open|openat|creat|mkstemp|mkostemp|socket (")) {
                // simple sanity check of function parameters..
                // TODO: Make such check for all these functions
                const int num = numberOfArguments(tok2);
                if (tok2->str() == "open" && num != 2 && num != 3)
                    return No;

                // is there a user function with this name?
                if (tok2->function())
                    return No;
                return Fd;
            }

            if (Token::simpleMatch(tok2, "popen ("))
                return Pipe;
        }

        // Does tok2 point on a Library allocation function?
        const int alloctype = mSettings_->library.getAllocId(tok2, -1);
        if (alloctype > 0) {
            if (alloctype == mSettings_->library.deallocId("free"))
                return Malloc;
            if (alloctype == mSettings_->library.deallocId("fclose"))
                return File;
            return Library::ismemory(alloctype) ? OtherMem : OtherRes;
        }
    }

    while (Token::Match(tok2,"%name% ::|. %type%"))
        tok2 = tok2->tokAt(2);

    // User function
    const Function* func = tok2->function();
    if (func == nullptr)
        return No;

    // Prevent recursion
    if (callstack && std::find(callstack->begin(), callstack->end(), func) != callstack->end())
        return No;

    std::list<const Function*> cs;
    if (!callstack)
        callstack = &cs;

    callstack->push_back(func);
    return functionReturnType(func, callstack);
}


CheckMemoryLeak::AllocType CheckMemoryLeak::getReallocationType(const Token *tok2, nonneg int varid) const
{
    // What we may have...
    //     * var = (char *)realloc(..;
    if (tok2 && tok2->str() == "(") {
        tok2 = tok2->link();
        tok2 = tok2 ? tok2->next() : nullptr;
    }
    if (! tok2)
        return No;

    if (!Token::Match(tok2, "%name% ("))
        return No;

    const Library::AllocFunc *f = mSettings_->library.getReallocFuncInfo(tok2);
    if (!(f && f->reallocArg > 0 && f->reallocArg <= numberOfArguments(tok2)))
        return No;
    const Token* arg = getArguments(tok2).at(f->reallocArg - 1);
    while (arg && arg->isCast())
        arg = arg->astOperand1();
    while (arg && arg->isUnaryOp("*"))
        arg = arg->astOperand1();
    if (varid > 0 && !Token::Match(arg, "%varid% [,)]", varid))
        return No;

    const int realloctype = mSettings_->library.getReallocId(tok2, -1);
    if (realloctype > 0) {
        if (realloctype == mSettings_->library.deallocId("free"))
            return Malloc;
        if (realloctype == mSettings_->library.deallocId("fclose"))
            return File;
        return Library::ismemory(realloctype) ? OtherMem : OtherRes;
    }
    return No;
}


CheckMemoryLeak::AllocType CheckMemoryLeak::getDeallocationType(const Token *tok, nonneg int varid) const
{
    if (mTokenizer_->isCPP() && tok->str() == "delete" && tok->astOperand1()) {
        const Token* vartok = tok->astOperand1();
        if (Token::Match(vartok, ".|::"))
            vartok = vartok->astOperand2();

        if (vartok && vartok->varId() == varid) {
            if (tok->strAt(1) == "[")
                return NewArray;
            return New;
        }
    }

    if (tok->str() == "::")
        tok = tok->next();

    if (Token::Match(tok, "%name% (")) {
        if (Token::simpleMatch(tok, "fcloseall ( )"))
            return File;

        int argNr = 1;
        for (const Token* tok2 = tok->tokAt(2); tok2; tok2 = tok2->nextArgument()) {
            const Token* vartok = tok2;
            while (Token::Match(vartok, "%name% .|::"))
                vartok = vartok->tokAt(2);

            if (Token::Match(vartok, "%varid% )|,|-", varid)) {
                if (tok->str() == "realloc" && Token::simpleMatch(vartok->next(), ", 0 )"))
                    return Malloc;

                if (mSettings_->posix()) {
                    if (tok->str() == "close")
                        return Fd;
                    if (tok->str() == "pclose")
                        return Pipe;
                }

                // Does tok point on a Library deallocation function?
                const int dealloctype = mSettings_->library.getDeallocId(tok, argNr);
                if (dealloctype > 0) {
                    if (dealloctype == mSettings_->library.deallocId("free"))
                        return Malloc;
                    if (dealloctype == mSettings_->library.deallocId("fclose"))
                        return File;
                    return Library::ismemory(dealloctype) ? OtherMem : OtherRes;
                }
            }
            argNr++;
        }
    }

    return No;
}

bool CheckMemoryLeak::isReopenStandardStream(const Token *tok) const
{
    if (getReallocationType(tok, 0) == File) {
        const Library::AllocFunc *f = mSettings_->library.getReallocFuncInfo(tok);
        if (f && f->reallocArg > 0 && f->reallocArg <= numberOfArguments(tok)) {
            const Token* arg = getArguments(tok).at(f->reallocArg - 1);
            if (Token::Match(arg, "stdin|stdout|stderr"))
                return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------


//--------------------------------------------------------------------------

void CheckMemoryLeak::memoryLeak(const Token *tok, const std::string &varname, AllocType alloctype) const
{
    if (alloctype == CheckMemoryLeak::File ||
        alloctype == CheckMemoryLeak::Pipe ||
        alloctype == CheckMemoryLeak::Fd   ||
        alloctype == CheckMemoryLeak::OtherRes)
        resourceLeakError(tok, varname);
    else
        memleakError(tok, varname);
}
//---------------------------------------------------------------------------

void CheckMemoryLeak::reportErr(const Token *tok, Severity::SeverityType severity, const std::string &id, const std::string &msg, const CWE &cwe) const
{
    std::list<const Token *> callstack;

    if (tok)
        callstack.push_back(tok);

    reportErr(callstack, severity, id, msg, cwe);
}

void CheckMemoryLeak::reportErr(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg, const CWE &cwe) const
{
    const ErrorLogger::ErrorMessage errmsg(callstack, mTokenizer_ ? &mTokenizer_->list : nullptr, severity, id, msg, cwe, false);
    if (mErrorLogger_)
        mErrorLogger_->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void CheckMemoryLeak::memleakError(const Token *tok, const std::string &varname) const
{
    reportErr(tok, Severity::error, "memleak", "$symbol:" + varname + "\nMemory leak: $symbol", CWE(401U));
}

void CheckMemoryLeak::memleakUponReallocFailureError(const Token *tok, const std::string &reallocfunction, const std::string &varname) const
{
    reportErr(tok, Severity::error, "memleakOnRealloc", "$symbol:" + varname + "\nCommon " + reallocfunction + " mistake: \'$symbol\' nulled but not freed upon failure", CWE(401U));
}

void CheckMemoryLeak::resourceLeakError(const Token *tok, const std::string &varname) const
{
    std::string errmsg("Resource leak");
    if (!varname.empty())
        errmsg = "$symbol:" + varname + '\n' + errmsg + ": $symbol";
    reportErr(tok, Severity::error, "resourceLeak", errmsg, CWE(775U));
}

void CheckMemoryLeak::deallocDeallocError(const Token *tok, const std::string &varname) const
{
    reportErr(tok, Severity::error, "deallocDealloc", "$symbol:" + varname + "\nDeallocating a deallocated pointer: $symbol", CWE(415U));
}

void CheckMemoryLeak::deallocuseError(const Token *tok, const std::string &varname) const
{
    reportErr(tok, Severity::error, "deallocuse", "$symbol:" + varname + "\nDereferencing '$symbol' after it is deallocated / released", CWE(416U));
}

void CheckMemoryLeak::mismatchSizeError(const Token *tok, const std::string &sz) const
{
    reportErr(tok, Severity::error, "mismatchSize", "The allocated size " + sz + " is not a multiple of the underlying type's size.", CWE(131U));
}

void CheckMemoryLeak::mismatchAllocDealloc(const std::list<const Token *> &callstack, const std::string &varname) const
{
    reportErr(callstack, Severity::error, "mismatchAllocDealloc", "$symbol:" + varname + "\nMismatching allocation and deallocation: $symbol", CWE(762U));
}

CheckMemoryLeak::AllocType CheckMemoryLeak::functionReturnType(const Function* func, std::list<const Function*> *callstack) const
{
    if (!func || !func->hasBody() || !func->functionScope)
        return No;

    // Get return pointer..
    int varid = 0;
    for (const Token *tok2 = func->functionScope->bodyStart; tok2 != func->functionScope->bodyEnd; tok2 = tok2->next()) {
        if (const Token *endOfLambda = findLambdaEndToken(tok2))
            tok2 = endOfLambda;
        if (tok2->str() == "{" && !tok2->scope()->isExecutable())
            tok2 = tok2->link();
        if (tok2->str() == "return") {
            const AllocType allocType = getAllocationType(tok2->next(), 0, callstack);
            if (allocType != No)
                return allocType;

            if (tok2->scope() != func->functionScope || !tok2->astOperand1())
                return No;
            const Token* tok = tok2->astOperand1();
            if (Token::Match(tok, ".|::"))
                tok = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
            if (tok)
                varid = tok->varId();
            break;
        }
    }

    // Not returning pointer value..
    if (varid == 0)
        return No;

    // If variable is not local then alloctype shall be "No"
    // Todo: there can be false negatives about mismatching allocation/deallocation.
    //       => Generate "alloc ; use ;" if variable is not local?
    const Variable *var = mTokenizer_->getSymbolDatabase()->getVariableFromVarId(varid);
    if (!var || !var->isLocal() || var->isStatic())
        return No;

    // Check if return pointer is allocated..
    AllocType allocType = No;
    for (const Token* tok = func->functionScope->bodyStart; tok != func->functionScope->bodyEnd; tok = tok->next()) {
        if (Token::Match(tok, "%varid% =", varid)) {
            allocType = getAllocationType(tok->tokAt(2), varid, callstack);
        }
        if (Token::Match(tok, "= %varid% ;", varid)) {
            return No;
        }
        if (!mTokenizer_->isC() && Token::Match(tok, "[(,] %varid% [,)]", varid)) {
            return No;
        }
        if (Token::Match(tok, "[(,] & %varid% [.,)]", varid)) {
            return No;
        }
        if (Token::Match(tok, "[;{}] %varid% .", varid)) {
            return No;
        }
        if (allocType == No && tok->str() == "return")
            return No;
    }

    return allocType;
}


const char *CheckMemoryLeak::functionArgAlloc(const Function *func, nonneg int targetpar, AllocType &allocType) const
{
    allocType = No;

    if (!func || !func->functionScope)
        return "";

    if (!Token::simpleMatch(func->retDef, "void"))
        return "";

    std::list<Variable>::const_iterator arg = func->argumentList.begin();
    for (; arg != func->argumentList.end(); ++arg) {
        if (arg->index() == targetpar-1)
            break;
    }
    if (arg == func->argumentList.end())
        return "";

    // Is **
    if (!arg->isPointer())
        return "";
    const Token* tok = arg->typeEndToken();
    tok = tok->previous();
    if (tok->str() != "*")
        return "";

    // Check if pointer is allocated.
    bool realloc = false;
    for (tok = func->functionScope->bodyStart; tok && tok != func->functionScope->bodyEnd; tok = tok->next()) {
        if (tok->varId() == arg->declarationId()) {
            if (Token::Match(tok->tokAt(-3), "free ( * %name% )")) {
                realloc = true;
                allocType = No;
            } else if (Token::Match(tok->previous(), "* %name% =")) {
                allocType = getAllocationType(tok->tokAt(2), arg->declarationId());
                if (allocType != No) {
                    if (realloc)
                        return "realloc";
                    return "alloc";
                }
            } else {
                // unhandled variable usage: bailout
                return "";
            }
        }
    }

    return "";
}


static bool notvar(const Token *tok, nonneg int varid)
{
    if (!tok)
        return false;
    if (Token::Match(tok, "&&|;"))
        return notvar(tok->astOperand1(),varid) || notvar(tok->astOperand2(),varid);
    if (tok->str() == "(" && Token::Match(tok->astOperand1(), "UNLIKELY|LIKELY"))
        return notvar(tok->astOperand2(), varid);
    const Token *vartok = astIsVariableComparison(tok, "==", "0");
    return vartok && (vartok->varId() == varid);
}

static bool ifvar(const Token *tok, nonneg int varid, const std::string &comp, const std::string &rhs)
{
    if (!Token::simpleMatch(tok, "if ("))
        return false;
    const Token *condition = tok->next()->astOperand2();
    if (condition && condition->str() == "(" && Token::Match(condition->astOperand1(), "UNLIKELY|LIKELY"))
        condition = condition->astOperand2();
    if (!condition || condition->str() == "&&")
        return false;

    const Token *vartok = astIsVariableComparison(condition, comp, rhs);
    return (vartok && vartok->varId() == varid);
}

bool CheckMemoryLeakInFunction::test_white_list(const std::string &funcname, const Settings *settings, bool cpp)
{
    return ((call_func_white_list.find(funcname)!=call_func_white_list.end()) || settings->library.isLeakIgnore(funcname) || (cpp && funcname == "delete"));
}


//---------------------------------------------------------------------------
// Check for memory leaks due to improper realloc() usage.
//   Below, "a" may be set to null without being freed if realloc() cannot
//   allocate the requested memory:
//     a = malloc(10); a = realloc(a, 100);
//---------------------------------------------------------------------------

static bool isNoArgument(const SymbolDatabase* symbolDatabase, nonneg int varid)
{
    const Variable* var = symbolDatabase->getVariableFromVarId(varid);
    return var && !var->isArgument();
}

void CheckMemoryLeakInFunction::checkReallocUsage()
{
    // only check functions
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {

        // Search for the "var = realloc(var, 100" pattern within this function
        for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->varId() > 0 && Token::Match(tok, "%name% =")) {
                // Get the parenthesis in "realloc("
                const Token* parTok = tok->next()->astOperand2();
                // Skip casts
                while (parTok && parTok->isCast())
                    parTok = parTok->astOperand1();
                if (!parTok)
                    continue;

                const Token *const reallocTok = parTok->astOperand1();
                if (!reallocTok)
                    continue;
                const Library::AllocFunc* f = mSettings->library.getReallocFuncInfo(reallocTok);
                if (!(f && f->arg == -1 && mSettings->library.isnotnoreturn(reallocTok)))
                    continue;

                const AllocType allocType = getReallocationType(reallocTok, tok->varId());
                if (!((allocType == Malloc || allocType == OtherMem)))
                    continue;
                const Token* arg = getArguments(reallocTok).at(f->reallocArg - 1);
                while (arg && arg->isCast())
                    arg = arg->astOperand1();
                const Token* tok2 = tok;
                while (arg && arg->isUnaryOp("*") && tok2 && tok2->astParent() && tok2->astParent()->isUnaryOp("*")) {
                    arg = arg->astOperand1();
                    tok2 = tok2->astParent();
                }

                if (!arg || !tok2)
                    continue;

                if (!((tok->varId() == arg->varId()) && isNoArgument(symbolDatabase, tok->varId())))
                    continue;

                // Check that another copy of the pointer wasn't saved earlier in the function
                if (Token::findmatch(scope->bodyStart, "%name% = %varid% ;", tok, tok->varId()) ||
                    Token::findmatch(scope->bodyStart, "[{};] %varid% = *| %name% .| %name%| [;=]", tok, tok->varId()))
                    continue;

                const Token* tokEndRealloc = reallocTok->linkAt(1);
                // Check that the allocation isn't followed immediately by an 'if (!var) { error(); }' that might handle failure
                if (Token::simpleMatch(tokEndRealloc->next(), "; if (") &&
                    notvar(tokEndRealloc->tokAt(3)->astOperand2(), tok->varId())) {
                    const Token* tokEndBrace = tokEndRealloc->linkAt(3)->linkAt(1);
                    if (tokEndBrace && mTokenizer->isScopeNoReturn(tokEndBrace))
                        continue;
                }

                memleakUponReallocFailureError(tok, reallocTok->str(), tok->str());
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checks for memory leaks in classes..
//---------------------------------------------------------------------------


void CheckMemoryLeakInClass::check()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // only check classes and structures
    for (const Scope * scope : symbolDatabase->classAndStructScopes) {
        for (const Variable &var : scope->varlist) {
            if (!var.isStatic() && var.isPointer()) {
                // allocation but no deallocation of private variables in public function..
                const Token *tok = var.typeStartToken();
                // Either it is of standard type or a non-derived type
                if (tok->isStandardType() || (var.type() && var.type()->derivedFrom.empty())) {
                    if (var.isPrivate())
                        checkPublicFunctions(scope, var.nameToken());

                    variable(scope, var.nameToken());
                }
            }
        }
    }
}


void CheckMemoryLeakInClass::variable(const Scope *scope, const Token *tokVarname)
{
    const std::string& varname = tokVarname->str();
    const int varid = tokVarname->varId();
    const std::string& classname = scope->className;

    // Check if member variable has been allocated and deallocated..
    CheckMemoryLeak::AllocType memberAlloc = CheckMemoryLeak::No;
    CheckMemoryLeak::AllocType memberDealloc = CheckMemoryLeak::No;

    bool allocInConstructor = false;
    bool deallocInDestructor = false;

    // Inspect member functions
    for (const Function &func : scope->functionList) {
        const bool constructor = func.isConstructor();
        const bool destructor = func.isDestructor();
        if (!func.hasBody()) {
            if (destructor) { // implementation for destructor is not seen => assume it deallocates all variables properly
                deallocInDestructor = true;
                memberDealloc = CheckMemoryLeak::Many;
            }
            continue;
        }
        bool body = false;
        const Token *end = func.functionScope->bodyEnd;
        for (const Token *tok = func.arg->link(); tok != end; tok = tok->next()) {
            if (tok == func.functionScope->bodyStart)
                body = true;
            else {
                if (!body) {
                    if (!Token::Match(tok, ":|, %varid% (", varid))
                        continue;
                }

                // Allocate..
                if (!body || Token::Match(tok, "%varid% =", varid)) {
                    // var1 = var2 = ...
                    // bail out
                    if (tok->strAt(-1) == "=")
                        return;

                    // Foo::var1 = ..
                    // bail out when not same class
                    if (tok->strAt(-1) == "::" &&
                        tok->strAt(-2) != scope->className)
                        return;

                    AllocType alloc = getAllocationType(tok->tokAt(body ? 2 : 3), 0);
                    if (alloc != CheckMemoryLeak::No) {
                        if (constructor)
                            allocInConstructor = true;

                        if (memberAlloc != No && memberAlloc != alloc)
                            alloc = CheckMemoryLeak::Many;

                        if (alloc != CheckMemoryLeak::Many && memberDealloc != CheckMemoryLeak::No && memberDealloc != CheckMemoryLeak::Many && memberDealloc != alloc) {
                            std::list<const Token *> callstack;
                            callstack.push_back(tok);
                            mismatchAllocDealloc(callstack, classname + "::" + varname);
                        }

                        memberAlloc = alloc;
                    }
                }

                if (!body)
                    continue;

                // Deallocate..
                AllocType dealloc = getDeallocationType(tok, varid);
                // some usage in the destructor => assume it's related
                // to deallocation
                if (destructor && tok->str() == varname)
                    dealloc = CheckMemoryLeak::Many;
                if (dealloc != CheckMemoryLeak::No) {
                    if (destructor)
                        deallocInDestructor = true;

                    // several types of allocation/deallocation?
                    if (memberDealloc != CheckMemoryLeak::No && memberDealloc != dealloc)
                        dealloc = CheckMemoryLeak::Many;

                    if (dealloc != CheckMemoryLeak::Many && memberAlloc != CheckMemoryLeak::No &&  memberAlloc != Many && memberAlloc != dealloc) {
                        std::list<const Token *> callstack;
                        callstack.push_back(tok);
                        mismatchAllocDealloc(callstack, classname + "::" + varname);
                    }

                    memberDealloc = dealloc;
                }

                // Function call .. possible deallocation
                else if (Token::Match(tok->previous(), "[{};] %name% (")) {
                    if (!CheckMemoryLeakInFunction::test_white_list(tok->str(), mSettings, mTokenizer->isCPP())) {
                        return;
                    }
                }
            }
        }
    }

    if (allocInConstructor && !deallocInDestructor) {
        unsafeClassError(tokVarname, classname, classname + "::" + varname /*, memberAlloc*/);
    } else if (memberAlloc != CheckMemoryLeak::No && memberDealloc == CheckMemoryLeak::No) {
        unsafeClassError(tokVarname, classname, classname + "::" + varname /*, memberAlloc*/);
    }
}

void CheckMemoryLeakInClass::unsafeClassError(const Token *tok, const std::string &classname, const std::string &varname)
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    reportError(tok, Severity::style, "unsafeClassCanLeak",
                "$symbol:" + classname + "\n"
                "$symbol:" + varname + "\n"
                "Class '" + classname + "' is unsafe, '" + varname + "' can leak by wrong usage.\n"
                "The class '" + classname + "' is unsafe, wrong usage can cause memory/resource leaks for '" + varname + "'. This can for instance be fixed by adding proper cleanup in the destructor.", CWE398, false);
}


void CheckMemoryLeakInClass::checkPublicFunctions(const Scope *scope, const Token *classtok)
{
    // Check that public functions deallocate the pointers that they allocate.
    // There is no checking how these functions are used and therefore it
    // isn't established if there is real leaks or not.
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    const int varid = classtok->varId();

    // Parse public functions..
    // If they allocate member variables, they should also deallocate
    for (const Function &func : scope->functionList) {
        if ((func.type == Function::eFunction || func.type == Function::eOperatorEqual) &&
            func.access == AccessControl::Public && func.hasBody()) {
            const Token *tok2 = func.functionScope->bodyStart->next();
            if (Token::Match(tok2, "%varid% =", varid)) {
                const CheckMemoryLeak::AllocType alloc = getAllocationType(tok2->tokAt(2), varid);
                if (alloc != CheckMemoryLeak::No)
                    publicAllocationError(tok2, tok2->str());
            } else if (Token::Match(tok2, "%type% :: %varid% =", varid) &&
                       tok2->str() == scope->className) {
                const CheckMemoryLeak::AllocType alloc = getAllocationType(tok2->tokAt(4), varid);
                if (alloc != CheckMemoryLeak::No)
                    publicAllocationError(tok2, tok2->strAt(2));
            }
        }
    }
}

void CheckMemoryLeakInClass::publicAllocationError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "publicAllocationError", "$symbol:" + varname + "\nPossible leak in public function. The pointer '$symbol' is not deallocated before it is allocated.", CWE398, false);
}


void CheckMemoryLeakStructMember::check()
{
    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || !var->isLocal() || var->isStatic() || var->isReference())
            continue;
        if (var->typeEndToken()->isStandardType())
            continue;
        checkStructVariable(var);
    }
}

bool CheckMemoryLeakStructMember::isMalloc(const Variable *variable)
{
    const int declarationId(variable->declarationId());
    bool alloc = false;
    for (const Token *tok2 = variable->nameToken(); tok2 && tok2 != variable->scope()->bodyEnd; tok2 = tok2->next()) {
        if (Token::Match(tok2, "= %varid% [;=]", declarationId)) {
            return false;
        } else if (Token::Match(tok2, "%varid% = malloc|kmalloc (", declarationId)) {
            alloc = true;
        }
    }
    return alloc;
}

void CheckMemoryLeakStructMember::checkStructVariable(const Variable * const variable)
{
    // Is struct variable a pointer?
    if (variable->isPointer()) {
        // Check that variable is allocated with malloc
        if (!isMalloc(variable))
            return;
    } else if (!mTokenizer->isC() && (!variable->typeScope() || variable->typeScope()->getDestructor())) {
        // For non-C code a destructor might cleanup members
        return;
    }

    // Check struct..
    int indentlevel2 = 0;
    for (const Token *tok2 = variable->nameToken(); tok2 && tok2 != variable->scope()->bodyEnd; tok2 = tok2->next()) {
        if (tok2->str() == "{")
            ++indentlevel2;

        else if (tok2->str() == "}") {
            if (indentlevel2 == 0)
                break;
            --indentlevel2;
        }

        // Unknown usage of struct
        /** @todo Check how the struct is used. Only bail out if necessary */
        else if (Token::Match(tok2, "[(,] %varid% [,)]", variable->declarationId()))
            break;

        // Struct member is allocated => check if it is also properly deallocated..
        else if (Token::Match(tok2->previous(), "[;{}] %varid% . %var% =", variable->declarationId())) {
            if (getAllocationType(tok2->tokAt(4), tok2->tokAt(2)->varId()) == AllocType::No)
                continue;

            const int structid(variable->declarationId());
            const int structmemberid(tok2->tokAt(2)->varId());

            // This struct member is allocated.. check that it is deallocated
            int indentlevel3 = indentlevel2;
            for (const Token *tok3 = tok2; tok3; tok3 = tok3->next()) {
                if (tok3->str() == "{")
                    ++indentlevel3;

                else if (tok3->str() == "}") {
                    if (indentlevel3 == 0) {
                        memoryLeak(tok3, variable->name() + "." + tok2->strAt(2), Malloc);
                        break;
                    }
                    --indentlevel3;
                }

                // Deallocating the struct member..
                else if (getDeallocationType(tok3, structmemberid) != AllocType::No) {
                    // If the deallocation happens at the base level, don't check this member anymore
                    if (indentlevel3 == 0)
                        break;

                    // deallocating and then returning from function in a conditional block =>
                    // skip ahead out of the block
                    bool ret = false;
                    while (tok3) {
                        if (tok3->str() == "return")
                            ret = true;
                        else if (tok3->str() == "{" || tok3->str() == "}")
                            break;
                        tok3 = tok3->next();
                    }
                    if (!ret || !tok3 || tok3->str() != "}")
                        break;
                    --indentlevel3;
                    continue;
                }

                // Deallocating the struct..
                else if (Token::Match(tok3, "free|kfree ( %varid% )", structid)) {
                    if (indentlevel2 == 0)
                        memoryLeak(tok3, variable->name() + "." + tok2->strAt(2), Malloc);
                    break;
                }

                // failed allocation => skip code..
                else if (Token::simpleMatch(tok3, "if (") &&
                         notvar(tok3->next()->astOperand2(), structmemberid)) {
                    // Goto the ")"
                    tok3 = tok3->next()->link();

                    // make sure we have ") {".. it should be
                    if (!Token::simpleMatch(tok3, ") {"))
                        break;

                    // Goto the "}"
                    tok3 = tok3->next()->link();
                }

                // succeeded allocation
                else if (ifvar(tok3, structmemberid, "!=", "0")) {
                    // goto the ")"
                    tok3 = tok3->next()->link();

                    // check if the variable is deallocated or returned..
                    int indentlevel4 = 0;
                    for (const Token *tok4 = tok3; tok4; tok4 = tok4->next()) {
                        if (tok4->str() == "{")
                            ++indentlevel4;
                        else if (tok4->str() == "}") {
                            --indentlevel4;
                            if (indentlevel4 == 0)
                                break;
                        } else if (Token::Match(tok4, "free|kfree ( %var% . %varid% )", structmemberid)) {
                            break;
                        }
                    }

                    // was there a proper deallocation?
                    if (indentlevel4 > 0)
                        break;
                }

                // Returning from function..
                else if (tok3->str() == "return") {
                    // Returning from function without deallocating struct member?
                    if (!Token::Match(tok3, "return %varid% ;", structid) &&
                        !Token::Match(tok3, "return & %varid%", structid) &&
                        !(Token::Match(tok3, "return %varid% . %var%", structid) && tok3->tokAt(3)->varId() == structmemberid)) {
                        memoryLeak(tok3, variable->name() + "." + tok2->strAt(2), Malloc);
                    }
                    break;
                }

                // struct assignment..
                else if (Token::Match(tok3, "= %varid% ;", structid)) {
                    break;
                } else if (Token::Match(tok3, "= %var% . %varid% ;", structmemberid)) {
                    break;
                }

                // goto isn't handled well.. bail out even though there might be leaks
                else if (tok3->str() == "goto")
                    break;

                // using struct in a function call..
                else if (Token::Match(tok3, "%name% (")) {
                    // Calling non-function / function that doesn't deallocate?
                    if (CheckMemoryLeakInFunction::test_white_list(tok3->str(), mSettings, mTokenizer->isCPP()))
                        continue;

                    // Check if the struct is used..
                    bool deallocated = false;
                    const Token* const end4 = tok3->linkAt(1);
                    for (const Token *tok4 = tok3; tok4 != end4; tok4 = tok4->next()) {
                        if (Token::Match(tok4, "[(,] &| %varid% [,)]", structid)) {
                            /** @todo check if the function deallocates the memory */
                            deallocated = true;
                            break;
                        }

                        if (Token::Match(tok4, "[(,] &| %varid% . %name% [,)]", structid)) {
                            /** @todo check if the function deallocates the memory */
                            deallocated = true;
                            break;
                        }
                    }

                    if (deallocated)
                        break;
                }
            }
        }
    }
}



void CheckMemoryLeakNoVar::check()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // only check functions
    for (const Scope * scope : symbolDatabase->functionScopes) {

        // Checks if a call to an allocation function like malloc() is made and its return value is not assigned.
        checkForUnusedReturnValue(scope);

        // Checks to see if a function is called with memory allocated for an argument that
        // could be leaked if a function called for another argument throws.
        checkForUnsafeArgAlloc(scope);

        // Check for leaks where a the return value of an allocation function like malloc() is an input argument,
        // for example f(malloc(1)), where f is known to not release the input argument.
        checkForUnreleasedInputArgument(scope);
    }
}

//---------------------------------------------------------------------------
// Checks if an input argument to a function is the return value of an allocation function
// like malloc(), and the function does not release it.
//---------------------------------------------------------------------------
void CheckMemoryLeakNoVar::checkForUnreleasedInputArgument(const Scope *scope)
{
    // parse the executable scope until tok is reached...
    for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
        // allocating memory in parameter for function call..
        if (!Token::Match(tok, "%name% ("))
            continue;

        // check if the output of the function is assigned
        const Token* tok2 = tok->next()->astParent();
        while (tok2 && tok2->isCast())
            tok2 = tok2->astParent();
        if (tok2 && tok2->isAssignmentOp())
            continue;

        const std::string& functionName = tok->str();
        if ((mTokenizer->isCPP() && functionName == "delete") ||
            functionName == "free" ||
            functionName == "fclose" ||
            functionName == "realloc" ||
            functionName == "return")
            continue;

        if (!CheckMemoryLeakInFunction::test_white_list(functionName, mSettings, mTokenizer->isCPP()))
            continue;

        const std::vector<const Token *> args = getArguments(tok);
        for (const Token* arg : args) {
            if (arg->isOp())
                continue;
            while (arg->astOperand1())
                arg = arg->astOperand1();
            if (getAllocationType(arg, 0) == No)
                continue;
            if (isReopenStandardStream(arg))
                continue;
            functionCallLeak(arg, arg->str(), functionName);
        }

    }
}

//---------------------------------------------------------------------------
// Checks if a call to an allocation function like malloc() is made and its return value is not assigned.
//---------------------------------------------------------------------------
void CheckMemoryLeakNoVar::checkForUnusedReturnValue(const Scope *scope)
{
    for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;

        if (tok->varId())
            continue;

        const AllocType allocType = getAllocationType(tok, 0);
        if (allocType == No)
            continue;

        if (tok != tok->next()->astOperand1())
            continue;

        if (isReopenStandardStream(tok))
            continue;

        // get ast parent, skip casts
        const Token *parent = tok->next()->astParent();
        while (parent && parent->str() == "(" && !parent->astOperand2())
            parent = parent->astParent();

        if (!parent) {
            // Check if we are in a C++11 constructor
            const Token * closingBrace = Token::findmatch(tok, "}|;");
            if (closingBrace->str() == "}" && Token::Match(closingBrace->link()->tokAt(-1), "%name%"))
                continue;
            returnValueNotUsedError(tok, tok->str());
        } else if (Token::Match(parent, "%comp%|!")) {
            returnValueNotUsedError(tok, tok->str());
        }
    }
}

//---------------------------------------------------------------------------
// Check if an exception could cause a leak in an argument constructed with
// shared_ptr/unique_ptr. For example, in the following code, it is possible
// that if g() throws an exception, the memory allocated by "new int(42)"
// could be leaked. See stackoverflow.com/questions/19034538/
// why-is-there-memory-leak-while-using-shared-ptr-as-a-function-parameter
//
// void x() {
//    f(shared_ptr<int>(new int(42)), g());
// }
//---------------------------------------------------------------------------
void CheckMemoryLeakNoVar::checkForUnsafeArgAlloc(const Scope *scope)
{
    // This test only applies to C++ source
    if (!mTokenizer->isCPP() || !mSettings->inconclusive || !mSettings->isEnabled(Settings::WARNING))
        return;

    for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
        if (Token::Match(tok, "%name% (")) {
            const Token *endParamToken = tok->next()->link();
            const Token* pointerType = nullptr;
            const Token* functionCalled = nullptr;

            // Scan through the arguments to the function call
            for (const Token *tok2 = tok->tokAt(2); tok2 && tok2 != endParamToken; tok2 = tok2->nextArgument()) {
                const Function *func = tok2->function();
                const bool isNothrow = func && (func->isAttributeNothrow() || func->isThrow());

                if (Token::Match(tok2, "shared_ptr|unique_ptr <") && Token::Match(tok2->next()->link(), "> ( new %name%")) {
                    pointerType = tok2;
                } else if (!isNothrow) {
                    if (Token::Match(tok2, "%name% ("))
                        functionCalled = tok2;
                    else if (tok2->isName() && Token::simpleMatch(tok2->next()->link(), "> ("))
                        functionCalled = tok2;
                }
            }

            if (pointerType && functionCalled) {
                std::string functionName = functionCalled->str();
                if (functionCalled->strAt(1) == "<") {
                    functionName += '<';
                    for (const Token* tok2 = functionCalled->tokAt(2); tok2 != functionCalled->next()->link(); tok2 = tok2->next())
                        functionName += tok2->str();
                    functionName += '>';
                }
                std::string objectTypeName;
                for (const Token* tok2 = pointerType->tokAt(2); tok2 != pointerType->next()->link(); tok2 = tok2->next())
                    objectTypeName += tok2->str();

                unsafeArgAllocError(tok, functionName, pointerType->str(), objectTypeName);
            }
        }
    }
}

void CheckMemoryLeakNoVar::functionCallLeak(const Token *loc, const std::string &alloc, const std::string &functionCall)
{
    reportError(loc, Severity::error, "leakNoVarFunctionCall", "Allocation with " + alloc + ", " + functionCall + " doesn't release it.", CWE772, false);
}

void CheckMemoryLeakNoVar::returnValueNotUsedError(const Token *tok, const std::string &alloc)
{
    reportError(tok, Severity::error, "leakReturnValNotUsed", "$symbol:" + alloc + "\nReturn value of allocation function '$symbol' is not stored.", CWE771, false);
}

void CheckMemoryLeakNoVar::unsafeArgAllocError(const Token *tok, const std::string &funcName, const std::string &ptrType, const std::string& objType)
{
    const std::string factoryFunc = ptrType == "shared_ptr" ? "make_shared" : "make_unique";
    reportError(tok, Severity::warning, "leakUnsafeArgAlloc",
                "$symbol:" + funcName + "\n"
                "Unsafe allocation. If $symbol() throws, memory could be leaked. Use " + factoryFunc + "<" + objType + ">() instead.",
                CWE401,
                true); // Inconclusive because funcName may never throw
}
