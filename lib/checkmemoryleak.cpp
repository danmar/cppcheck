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


#include "checkmemoryleak.h"
#include "symboldatabase.h"
#include "mathlib.h"
#include "tokenize.h"

#include <algorithm>
#include <sstream>
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

/**
 * Count function parameters
 * \param tok Function name token before the '('
 */
static unsigned int countParameters(const Token *tok)
{
    tok = tok->tokAt(2);
    if (tok->str() == ")")
        return 0;

    unsigned int numpar = 1;
    while (nullptr != (tok = tok->nextArgument()))
        numpar++;

    return numpar;
}


/** List of functions that can be ignored when searching for memory leaks.
 * These functions don't take the address of the given pointer
 * This list needs to be alphabetically sorted so we can run bsearch on it.
 * This list contains function names with const parameters e.g.: atof(const char *)
 * Reference: http://www.aquaphoenix.com/ref/gnu_c_library/libc_492.html#SEC492
 */
static const char * const call_func_white_list[] = {
    "_open", "_wopen", "access", "adjtime", "asctime", "asctime_r", "asprintf", "assert"
    , "atof", "atoi", "atol", "chdir", "chmod", "chown"
    , "clearerr", "creat", "ctime", "ctime_r", "execl", "execle"
    , "execlp", "execv", "execve", "fchmod", "fclose", "fcntl"
    , "fdatasync", "feof", "ferror", "fflush", "fgetc", "fgetpos", "fgets"
    , "flock", "fmemopen", "fnmatch", "fopen", "fopencookie", "for", "fprintf", "fputc", "fputs", "fread", "free"
    , "freopen", "fscanf", "fseek", "fseeko", "fsetpos", "fstat", "fsync", "ftell", "ftello"
    , "ftruncate", "fwrite", "getc", "getenv","getgrnam", "gethostbyaddr", "gethostbyname", "getnetbyname"
    , "getopt", "getopt_long", "getprotobyname", "getpwnam", "gets", "getservbyname", "getservbyport"
    , "glob", "gmtime", "gmtime_r", "if", "index", "inet_addr", "inet_aton", "inet_network", "initgroups", "ioctl"
    , "link", "localtime", "localtime_r"
    , "lockf", "lseek", "lstat", "mblen", "mbstowcs", "mbtowc", "memchr", "memcmp", "memcpy", "memmove", "memset"
    , "mkdir", "mkfifo", "mknod", "mkstemp"
    , "obstack_printf", "obstack_vprintf", "open", "opendir", "parse_printf_format", "pathconf"
    , "perror", "popen" ,"posix_fadvise", "posix_fallocate", "pread"
    , "printf", "psignal", "puts", "pwrite", "qsort", "read", "readahead", "readdir", "readdir_r"
    , "readlink", "readv"
    , "realloc", "regcomp", "remove", "rename", "return", "rewind", "rewinddir", "rindex"
    , "rmdir" ,"scandir", "scanf", "seekdir"
    , "setbuf", "setbuffer", "sethostname", "setlinebuf", "setlocale" ,"setvbuf", "sizeof" ,"snprintf", "sprintf", "sscanf"
    , "stat", "stpcpy", "strcasecmp", "strcat", "strchr", "strcmp", "strcoll"
    , "strcpy", "strcspn", "strdup", "stricmp", "strlen", "strncasecmp", "strncat", "strncmp"
    , "strncpy", "strpbrk","strrchr", "strspn", "strstr", "strtod", "strtok", "strtol", "strtoul", "strxfrm", "switch"
    , "symlink", "sync_file_range", "system", "telldir", "tempnam", "time", "typeid", "unlink"
    , "utime", "utimes", "vasprintf", "vfprintf", "vfscanf", "vprintf"
    , "vscanf", "vsnprintf", "vsprintf", "vsscanf", "while", "wordexp","write", "writev"
};

//---------------------------------------------------------------------------

bool CheckMemoryLeak::isclass(const Token *tok, unsigned int varid) const
{
    if (tok->isStandardType())
        return false;

    const Variable * var = tokenizer->getSymbolDatabase()->getVariableFromVarId(varid);

    // return false if the type is a simple record type without side effects
    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check base class for side effects */
    /** @todo false negative: check constructors for side effects */
    if (var && var->typeScope() && var->typeScope()->numConstructors == 0 &&
        (var->typeScope()->varlist.empty() || var->type()->needInitialization == Type::True) &&
        var->type()->derivedFrom.empty())
        return false;

    return true;
}
//---------------------------------------------------------------------------

CheckMemoryLeak::AllocType CheckMemoryLeak::getAllocationType(const Token *tok2, unsigned int varid, std::list<const Function*> *callstack) const
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

    if (!Token::Match(tok2, "%type%|%name% ::|. %type%")) {
        // Does tok2 point on "malloc", "strdup" or "kmalloc"..
        static const char * const mallocfunc[] = {
            "malloc",
            "calloc",
            "strdup",
            "strndup",
            "kmalloc",
            "kzalloc",
            "kcalloc"
        };
        for (unsigned int i = 0; i < sizeof(mallocfunc)/sizeof(*mallocfunc); i++) {
            if (tok2->str() == mallocfunc[i])
                return Malloc;
        }

        // Using realloc..
        if (varid && Token::Match(tok2, "realloc ( %any% ,") && tok2->tokAt(2)->varId() != varid)
            return Malloc;

        if (Token::Match(tok2, "new struct| %type% [;()]") ||
            Token::Match(tok2, "new ( std :: nothrow ) struct| %type% [;()]") ||
            Token::Match(tok2, "new ( nothrow ) struct| %type% [;()]"))
            return New;

        if (Token::Match(tok2, "new struct| %type% *| [") ||
            Token::Match(tok2, "new ( std :: nothrow ) struct| %type% *| [") ||
            Token::Match(tok2, "new ( nothrow ) struct| %type% *| ["))
            return NewArray;

        if (Token::Match(tok2, "fopen|tmpfile|g_fopen ("))
            return File;

        if (settings1->standards.posix) {
            if (Token::Match(tok2, "open|openat|creat|mkstemp|mkostemp|socket (")) {
                // simple sanity check of function parameters..
                // TODO: Make such check for all these functions
                unsigned int num = countParameters(tok2);
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

        // Does tok2 point on "g_malloc", "g_strdup", ..
        const int alloctype = settings1->library.alloc(tok2);
        if (alloctype > 0) {
            if (alloctype == settings1->library.dealloc("free"))
                return Malloc;
            if (alloctype == settings1->library.dealloc("fclose"))
                return File;
            return Library::ismemory(alloctype) ? OtherMem : OtherRes;
        }
    }

    while (Token::Match(tok2,"%type%|%name% ::|. %type%"))
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


CheckMemoryLeak::AllocType CheckMemoryLeak::getReallocationType(const Token *tok2, unsigned int varid)
{
    // What we may have...
    //     * var = (char *)realloc(..;
    if (tok2 && tok2->str() == "(") {
        tok2 = tok2->link();
        tok2 = tok2 ? tok2->next() : nullptr;
    }
    if (! tok2)
        return No;

    if (varid > 0 && ! Token::Match(tok2, "%name% ( %varid% [,)]", varid))
        return No;

    if (tok2->str() == "realloc")
        return Malloc;

    // GTK memory reallocation..
    //if (Token::Match(tok2, "g_realloc|g_try_realloc|g_renew|g_try_renew"))

    return No;
}


CheckMemoryLeak::AllocType CheckMemoryLeak::getDeallocationType(const Token *tok, unsigned int varid) const
{
    if (tokenizer->isCPP()) {
        if (Token::Match(tok, "delete %varid% ;", varid))
            return New;

        if (Token::Match(tok, "delete [ ] %varid% ;", varid))
            return NewArray;

        if (Token::Match(tok, "delete ( %varid% ) ;", varid))
            return New;

        if (Token::Match(tok, "delete [ ] ( %varid% ) ;", varid))
            return NewArray;
    }

    if (tok && tok->str() == "::")
        tok = tok->next();

    if (Token::Match(tok, "free|kfree ( %varid% ) [;:]", varid) ||
        Token::Match(tok, "free|kfree ( %varid% -|,", varid) ||
        Token::Match(tok, "realloc ( %varid% , 0 ) ;", varid))
        return Malloc;

    if (Token::Match(tok, "fclose ( %varid% )", varid) ||
        Token::simpleMatch(tok, "fcloseall ( )"))
        return File;

    if (settings1->standards.posix) {
        if (Token::Match(tok, "close ( %varid% )", varid))
            return Fd;

        if (Token::Match(tok, "pclose ( %varid% )", varid))
            return Pipe;
    }

    // Does tok2 point on "g_free", etc ..
    if (Token::Match(tok, "%type% ( %varid% )", varid)) {
        const int dealloctype = settings1->library.dealloc(tok);
        if (dealloctype > 0)
            return Library::ismemory(dealloctype) ? OtherMem : OtherRes;
    }

    return No;
}

CheckMemoryLeak::AllocType CheckMemoryLeak::getDeallocationType(const Token *tok, const std::string &varname) const
{
    if (tokenizer->isCPP()) {
        if (Token::Match(tok, std::string("delete " + varname + " [,;]").c_str()))
            return New;

        if (Token::Match(tok, std::string("delete [ ] " + varname + " [,;]").c_str()))
            return NewArray;

        if (Token::Match(tok, std::string("delete ( " + varname + " ) [,;]").c_str()))
            return New;

        if (Token::Match(tok, std::string("delete [ ] ( " + varname + " ) [,;]").c_str()))
            return NewArray;
    }

    if (Token::simpleMatch(tok, std::string("free ( " + varname + " ) ;").c_str()) ||
        Token::simpleMatch(tok, std::string("kfree ( " + varname + " ) ;").c_str()) ||
        Token::simpleMatch(tok, std::string("realloc ( " + varname + " , 0 ) ;").c_str()))
        return Malloc;

    if (Token::simpleMatch(tok, std::string("fclose ( " + varname + " )").c_str()) ||
        Token::simpleMatch(tok, "fcloseall ( )"))
        return File;

    if (Token::simpleMatch(tok, std::string("close ( " + varname + " )").c_str()))
        return Fd;

    if (Token::simpleMatch(tok, std::string("pclose ( " + varname + " )").c_str()))
        return Pipe;

    if (Token::Match(tok, ("%type% ( " + varname + " )").c_str())) {
        int type = settings1->library.dealloc(tok);
        if (type > 0)
            return Library::ismemory(type) ? OtherMem : OtherRes;
    }

    return No;
}

//--------------------------------------------------------------------------


//--------------------------------------------------------------------------

void CheckMemoryLeak::memoryLeak(const Token *tok, const std::string &varname, AllocType alloctype)
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

void CheckMemoryLeak::reportErr(const Token *tok, Severity::SeverityType severity, const std::string &id, const std::string &msg, unsigned int cwe) const
{
    std::list<const Token *> callstack;

    if (tok)
        callstack.push_back(tok);

    reportErr(callstack, severity, id, msg, cwe);
}

void CheckMemoryLeak::reportErr(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg, unsigned int cwe) const
{
    ErrorLogger::ErrorMessage errmsg(callstack, tokenizer?&tokenizer->list:0, severity, id, msg, false);
    errmsg._cwe = cwe;

    if (errorLogger)
        errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void CheckMemoryLeak::memleakError(const Token *tok, const std::string &varname) const
{
    reportErr(tok, Severity::error, "memleak", "Memory leak: " + varname, 0U);
}

void CheckMemoryLeak::memleakUponReallocFailureError(const Token *tok, const std::string &varname) const
{
    reportErr(tok, Severity::error, "memleakOnRealloc", "Common realloc mistake: \'" + varname + "\' nulled but not freed upon failure", 0U);
}

void CheckMemoryLeak::resourceLeakError(const Token *tok, const std::string &varname) const
{
    std::string errmsg("Resource leak");
    if (!varname.empty())
        errmsg += ": " + varname;
    reportErr(tok, Severity::error, "resourceLeak", errmsg, 775U);
}

void CheckMemoryLeak::deallocDeallocError(const Token *tok, const std::string &varname) const
{
    reportErr(tok, Severity::error, "deallocDealloc", "Deallocating a deallocated pointer: " + varname, 0U);
}

void CheckMemoryLeak::deallocuseError(const Token *tok, const std::string &varname) const
{
    reportErr(tok, Severity::error, "deallocuse", "Dereferencing '" + varname + "' after it is deallocated / released", 0U);
}

void CheckMemoryLeak::mismatchSizeError(const Token *tok, const std::string &sz) const
{
    reportErr(tok, Severity::error, "mismatchSize", "The allocated size " + sz + " is not a multiple of the underlying type's size.", 0U);
}

void CheckMemoryLeak::mismatchAllocDealloc(const std::list<const Token *> &callstack, const std::string &varname) const
{
    reportErr(callstack, Severity::error, "mismatchAllocDealloc", "Mismatching allocation and deallocation: " + varname, 0U);
}

CheckMemoryLeak::AllocType CheckMemoryLeak::functionReturnType(const Function* func, std::list<const Function*> *callstack) const
{
    if (!func || !func->hasBody())
        return No;

    // Get return pointer..
    unsigned int varid = 0;
    unsigned int indentlevel = 0;
    for (const Token *tok2 = func->functionScope->classStart; tok2 != func->functionScope->classEnd; tok2 = tok2->next()) {
        if (tok2->str() == "{")
            ++indentlevel;
        else if (tok2->str() == "}") {
            if (indentlevel <= 1)
                return No;
            --indentlevel;
        }
        if (Token::Match(tok2, "return %name% ;")) {
            if (indentlevel != 1)
                return No;
            varid = tok2->next()->varId();
            break;
        } else if (tok2->str() == "return") {
            AllocType allocType = getAllocationType(tok2->next(), 0, callstack);
            if (allocType != No)
                return allocType;
        }
    }

    // Not returning pointer value..
    if (varid == 0)
        return No;

    // If variable is not local then alloctype shall be "No"
    // Todo: there can be false negatives about mismatching allocation/deallocation.
    //       => Generate "alloc ; use ;" if variable is not local?
    const Variable *var = tokenizer->getSymbolDatabase()->getVariableFromVarId(varid);
    if (!var || !var->isLocal() || var->isStatic())
        return No;

    // Check if return pointer is allocated..
    AllocType allocType = No;
    for (const Token* tok = func->functionScope->classStart; tok != func->functionScope->classEnd; tok = tok->next()) {
        if (Token::Match(tok, "%varid% =", varid)) {
            allocType = getAllocationType(tok->tokAt(2), varid, callstack);
        }
        if (Token::Match(tok, "= %varid% ;", varid)) {
            return No;
        }
        if (Token::Match(tok, "static %type% * %varid% [;{}=]", varid)) {
            return No;
        }
        if (Token::Match(tok, "[(,] %varid% [,)]", varid)) {
            return No;
        }
        if (Token::Match(tok, "[(,] & %varid% [.,)]", varid)) {
            return No;
        }
        if (allocType == No && tok->str() == "return")
            return No;
    }

    return allocType;
}


const char *CheckMemoryLeak::functionArgAlloc(const Function *func, unsigned int targetpar, AllocType &allocType) const
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
    int realloc = 0;
    for (tok = func->functionScope->classStart; tok && tok != func->functionScope->classEnd; tok = tok->next()) {
        if (tok->varId() == arg->declarationId()) {
            if (Token::Match(tok->tokAt(-3), "free ( * %name% )")) {
                realloc = 1;
                allocType = No;
            } else if (Token::Match(tok->previous(), "* %name% =")) {
                allocType = getAllocationType(tok->tokAt(2), arg->declarationId());
                if (allocType == No) {
                    allocType = getReallocationType(tok->tokAt(2), arg->declarationId());
                }
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


bool CheckMemoryLeakInFunction::notvar(const Token *tok, unsigned int varid, bool endpar)
{
    const std::string end(endpar ? " &&|)" : " [;)&|]");
    return bool(Token::Match(tok, ("! %varid%" + end).c_str(), varid) ||
                Token::Match(tok, ("! ( %varid% )" + end).c_str(), varid));
}



bool CheckMemoryLeakInFunction::test_white_list(const std::string &funcname, const Settings *settings, bool cpp)
{
    return (std::binary_search(call_func_white_list,
                               call_func_white_list+sizeof(call_func_white_list) / sizeof(call_func_white_list[0]),
                               funcname) || (settings->library.leakignore.find(funcname) != settings->library.leakignore.end()) || (cpp && funcname == "delete"));
}


const char * CheckMemoryLeakInFunction::call_func(const Token *tok, std::list<const Token *> callstack, const unsigned int varid, AllocType &alloctype, AllocType &dealloctype, bool &allocpar, unsigned int sz)
{
    if (test_white_list(tok->str(), _settings, tokenizer->isCPP())) {
        if (tok->str() == "asprintf" ||
            tok->str() == "delete" ||
            tok->str() == "fclose" ||
            tok->str() == "for" ||
            tok->str() == "free" ||
            tok->str() == "if" ||
            tok->str() == "realloc" ||
            tok->str() == "return" ||
            tok->str() == "switch" ||
            tok->str() == "while" ||
            tok->str() == "sizeof") {
            return 0;
        }

        // is the varid a parameter?
        for (const Token *tok2 = tok->tokAt(2); tok2 != tok->linkAt(1); tok2 = tok2->next()) {
            if (tok2->str() == "(") {
                tok2 = tok2->nextArgument();
                if (!tok2)
                    break;
            }
            if (tok2->varId() == varid) {
                if (tok->strAt(-1) == ".")
                    return "use";
                else if (tok2->strAt(1) == "=")
                    return "assign";
                else if (tok->str()=="printf")
                    return "use"; // <- it is not certain printf dereference the pointer TODO: check the format string
                else
                    return "use_";
            }
        }

        return 0;
    }

    if (_settings->library.isnoreturn(tok) && tok->strAt(-1) != "=")
        return "exit";

    if (varid > 0 && (getReallocationType(tok, varid) != No || getDeallocationType(tok, varid) != No))
        return 0;

    if (callstack.size() > 2)
        return "dealloc_";

    const std::string& funcname(tok->str());
    for (std::list<const Token *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it) {
        if ((*it) && (*it)->str() == funcname)
            return "recursive";
    }
    callstack.push_back(tok);

    // lock/unlock..
    if (varid == 0) {
        const Function* func = tok->function();
        if (!func || !func->hasBody())
            return 0;

        Token *ftok = getcode(func->functionScope->classStart->next(), callstack, 0, alloctype, dealloctype, false, 1);
        simplifycode(ftok);
        const char *ret = nullptr;
        if (Token::simpleMatch(ftok, "; alloc ; }"))
            ret = "alloc";
        else if (Token::simpleMatch(ftok, "; dealloc ; }"))
            ret = "dealloc";
        TokenList::deleteTokens(ftok);
        return ret;
    }

    // how many parameters is there in the function call?
    const unsigned int numpar = countParameters(tok);
    if (numpar == 0) {
        // Taking return value => it is not a noreturn function
        if (tok->strAt(-1) == "=")
            return nullptr;

        // Function is not noreturn
        if (tok->function() && tok->function()->functionScope) {
            std::string temp;
            if (!_settings->library.isScopeNoReturn(tok->function()->functionScope->classEnd, &temp) && temp.empty())
                return nullptr;
        } else if (_settings->library.isnotnoreturn(tok))
            return nullptr;

        return "callfunc";
    }

    unsigned int par = 0;

    const bool dot(tok->previous()->str() == ".");
    const bool eq(tok->previous()->str() == "=");

    const Token *functok = tok;

    tok = Token::findsimplematch(tok, "(");
    if (tok)
        tok = tok->next();

    for (; tok; tok = tok->nextArgument()) {
        ++par;
        if (Token::Match(tok, "%varid% [,()]", varid)) {
            if (dot)
                return "use";

            const Function* function = functok->function();
            if (!function)
                return "use";

            // how many parameters does the function want?
            if (numpar != function->argCount()) // TODO: Handle default parameters
                return "recursive";

            if (!function->functionScope)
                return "use";
            const Variable* param = function->getArgumentVar(par-1);
            if (!param || !param->nameToken())
                return "use";
            Token *func = getcode(function->functionScope->classStart->next(), callstack, param->declarationId(), alloctype, dealloctype, false, sz);
            //simplifycode(func);
            const Token *func_ = func;
            while (func_ && func_->str() == ";")
                func_ = func_->next();

            const char *ret = nullptr;
            /** @todo handle "goto" */
            if (Token::findsimplematch(func_, "dealloc"))
                ret = "dealloc";
            else if (Token::findsimplematch(func_, "use"))
                ret = "use";
            else if (Token::findsimplematch(func_, "&use"))
                ret = "&use";

            TokenList::deleteTokens(func);
            return ret;
        }
        if (Token::Match(tok, "& %varid% [,()]", varid)) {
            const Function *func = functok->function();
            if (func == 0)
                continue;
            AllocType a;
            const char *ret = functionArgAlloc(func, par, a);

            if (a != No) {
                if (alloctype == No)
                    alloctype = a;
                else if (alloctype != a)
                    alloctype = Many;
                allocpar = true;
                return ret;
            }
        }
        if (Token::Match(tok, "%varid% . %name% [,)]", varid))
            return "use";
    }
    return (eq || _settings->experimental) ? 0 : "callfunc";
}


static void addtoken(Token **rettail, const Token *tok, const std::string &str)
{
    (*rettail)->insertToken(str);
    (*rettail) = (*rettail)->next();
    (*rettail)->linenr(tok->linenr());
    (*rettail)->fileIndex(tok->fileIndex());
}


Token *CheckMemoryLeakInFunction::getcode(const Token *tok, std::list<const Token *> callstack, const unsigned int varid, CheckMemoryLeak::AllocType &alloctype, CheckMemoryLeak::AllocType &dealloctype, bool classmember, unsigned int sz)
{
    // variables whose value depends on if(!var). If one of these variables
    // is used in a if-condition then generate "ifv" instead of "if".
    std::set<unsigned int> extravar;

    // The first token should be ";"
    Token* rethead = new Token(0);
    rethead->str(";");
    rethead->linenr(tok->linenr());
    rethead->fileIndex(tok->fileIndex());
    Token* rettail = rethead;

    int indentlevel = 0;
    int parlevel = 0;
    for (; tok; tok = tok->next()) {
        if (tok->str() == "{") {
            addtoken(&rettail, tok, "{");
            ++indentlevel;
        } else if (tok->str() == "}") {
            addtoken(&rettail, tok, "}");
            if (indentlevel <= 0)
                break;
            --indentlevel;
        }

        else if (tok->str() == "(")
            ++parlevel;
        else if (tok->str() == ")")
            --parlevel;

        if (parlevel == 0 && tok->str() == ";")
            addtoken(&rettail, tok, ";");

        // Start of new statement.. check if the statement has anything interesting
        if (varid > 0 && parlevel == 0 && Token::Match(tok, "[;{}]")) {
            if (Token::Match(tok->next(), "[{};]"))
                continue;

            // function calls are interesting..
            const Token *tok2 = tok;
            while (Token::Match(tok2->next(), "%name% ."))
                tok2 = tok2->tokAt(2);
            if (Token::Match(tok2->next(), "%name% ("))
                ;

            else if (Token::Match(tok->next(), "continue|break|return|throw|goto|do|else"))
                ;

            else {
                const Token *skipToToken = nullptr;

                // scan statement for interesting keywords / varid
                for (tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == ";") {
                        // nothing interesting found => skip this statement
                        skipToToken = tok2->previous();
                        break;
                    }

                    if (tok2->varId() == varid ||
                        tok2->str() == ":" || tok2->str() == "{" || tok2->str() == "}") {
                        break;
                    }
                }

                if (skipToToken) {
                    tok = skipToToken;
                    continue;
                }
            }
        }

        if (varid == 0) {
            if (!callstack.empty() && Token::Match(tok, "[;{}] __cppcheck_lock|__cppcheck_unlock ( ) ;")) {
                // Type of leak = Resource leak
                alloctype = dealloctype = CheckMemoryLeak::File;

                if (tok->next()->str() == "__cppcheck_lock") {
                    addtoken(&rettail, tok, "alloc");
                } else {
                    addtoken(&rettail, tok, "dealloc");
                }

                tok = tok->tokAt(3);
                continue;
            }

            if (Token::simpleMatch(tok, "if (")) {
                addtoken(&rettail, tok, "if");
                tok = tok->next()->link();
                continue;
            }
        } else {

            if (Token::Match(tok, "%varid% = close ( %varid% )", varid)) {
                addtoken(&rettail, tok, "dealloc");
                addtoken(&rettail, tok, ";");
                addtoken(&rettail, tok, "assign");
                addtoken(&rettail, tok, ";");
                tok = tok->tokAt(5);
                continue;
            }

            // var = strcpy|.. ( var ,
            if (Token::Match(tok, "[;{}] %varid% = memcpy|memmove|memset|strcpy|strncpy|strcat|strncat ( %varid% ,", varid)) {
                tok = tok->linkAt(4);
                continue;
            }

            if (Token::Match(tok->previous(), "[(;{}] %varid% =", varid) ||
                Token::Match(tok, "asprintf|vasprintf ( & %varid% ,", varid)) {
                CheckMemoryLeak::AllocType alloc;

                if (Token::Match(tok, "asprintf|vasprintf (")) {
                    // todo: check how the return value is used.
                    if (!Token::Match(tok->previous(), "[;{}]")) {
                        TokenList::deleteTokens(rethead);
                        return 0;
                    }
                    alloc = Malloc;
                    tok = tok->next()->link();
                } else {
                    alloc = getAllocationType(tok->tokAt(2), varid);
                }
                //bool realloc = false;

                if (sz > 1 &&
                    Token::Match(tok->tokAt(2), "malloc ( %num% )") &&
                    (MathLib::toLongNumber(tok->strAt(4)) % long(sz)) != 0) {
                    mismatchSizeError(tok->tokAt(4), tok->strAt(4));
                }

                if (alloc == CheckMemoryLeak::No) {
                    alloc = getReallocationType(tok->tokAt(2), varid);
                    if (alloc != CheckMemoryLeak::No) {
                        addtoken(&rettail, tok, "realloc");
                        addtoken(&rettail, tok, ";");
                        //TODO: this assignment is redundant, should be fixed
                        //realloc = true;
                        tok = tok->tokAt(2);
                        if (Token::Match(tok, "%name% ("))
                            tok = tok->next()->link();
                        continue;
                    }
                }

                // don't check classes..
                if (alloc == CheckMemoryLeak::New) {
                    if (Token::Match(tok->tokAt(2), "new struct| %type% [(;]")) {
                        const int offset = tok->strAt(3) == "struct" ? 1 : 0;
                        if (isclass(tok->tokAt(3 + offset), varid)) {
                            alloc = No;
                        }
                    } else if (Token::Match(tok->tokAt(2), "new ( nothrow ) struct| %type%")) {
                        const int offset = tok->strAt(6) == "struct" ? 1 : 0;
                        if (isclass(tok->tokAt(6 + offset), varid)) {
                            alloc = No;
                        }
                    } else if (Token::Match(tok->tokAt(2), "new ( std :: nothrow ) struct| %type%")) {
                        const int offset = tok->strAt(8) == "struct" ? 1 : 0;
                        if (isclass(tok->tokAt(8 + offset), varid)) {
                            alloc = No;
                        }
                    }

                    if (alloc == No && alloctype == No)
                        alloctype = CheckMemoryLeak::New;
                }

                if (alloc != No) {
                    //if (! realloc)
                    addtoken(&rettail, tok, "alloc");

                    if (alloctype != No && alloctype != alloc)
                        alloc = Many;

                    if (alloc != Many && dealloctype != No && dealloctype != Many && dealloctype != alloc) {
                        callstack.push_back(tok);
                        mismatchAllocDealloc(callstack, Token::findmatch(_tokenizer->tokens(), "%varid%", varid)->str());
                        callstack.pop_back();
                    }

                    alloctype = alloc;

                    if (Token::Match(tok, "%name% = %type% (")) {
                        tok = tok->linkAt(3);
                        continue;
                    }
                }

                // assignment..
                else {
                    // is the pointer in rhs?
                    bool rhs = false;
                    bool trailingSemicolon = false;
                    bool used = false;
                    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == ";") {
                            trailingSemicolon = true;
                            if (rhs)
                                tok = tok2;
                            break;
                        }

                        if (!used && !rhs) {
                            if (Token::Match(tok2, "[=+(,] %varid%", varid)) {
                                if (Token::Match(tok2, "[(,]")) {
                                    used = true;
                                    addtoken(&rettail, tok, "use");
                                    addtoken(&rettail, tok, ";");
                                }
                                rhs = true;
                            }
                        }
                    }

                    if (!used) {
                        if (!rhs)
                            addtoken(&rettail, tok, "assign");
                        else {
                            addtoken(&rettail, tok, "use_");
                            if (trailingSemicolon)
                                addtoken(&rettail, tok, ";");
                        }
                    }
                    continue;
                }
            }

            if (Token::Match(tok->previous(), "%op%|;|{|}|) ::| %name%") || (Token::Match(tok->previous(), "( ::| %name%") && (!rettail || rettail->str() != "loop"))) {
                if (Token::Match(tok, "%varid% ?", varid))
                    tok = tok->tokAt(2);

                AllocType dealloc = getDeallocationType(tok, varid);

                if (dealloc != No && tok->str() == "fcloseall" && alloctype != dealloc)
                    //TODO: this assignment is redundant, should be fixed
                    /*dealloc = No*/;

                else if (dealloc != No) {
                    addtoken(&rettail, tok, "dealloc");

                    if (dealloctype != No && dealloctype != dealloc)
                        dealloc = Many;

                    if (dealloc != Many && alloctype != No && alloctype != Many && alloctype != dealloc) {
                        callstack.push_back(tok);
                        mismatchAllocDealloc(callstack, Token::findmatch(_tokenizer->tokens(), "%varid%", varid)->str());
                        callstack.pop_back();
                    }
                    dealloctype = dealloc;

                    if (tok->strAt(2) == "(")
                        tok = tok->linkAt(2);
                    continue;
                }
            }

            // if else switch
            if (Token::simpleMatch(tok, "if (")) {
                if (alloctype == Fd) {
                    if (Token::Match(tok, "if ( 0|-1 <=|< %varid% )", varid) ||
                        Token::Match(tok, "if ( %varid% >|>= 0|-1 )", varid) ||
                        Token::Match(tok, "if ( %varid% != -1 )", varid) ||
                        Token::Match(tok, "if ( -1 != %varid% )", varid)) {
                        addtoken(&rettail, tok, "if(var)");
                        tok = tok->next()->link();
                        continue;
                    } else if (Token::Match(tok, "if ( %varid% == -1 )", varid) ||
                               Token::Match(tok, "if ( -1 == %varid% )", varid) ||
                               Token::Match(tok, "if ( %varid% < 0 )", varid) ||
                               Token::Match(tok, "if ( 0 > %varid% )", varid)) {
                        addtoken(&rettail, tok, "if(!var)");
                        tok = tok->next()->link();
                        continue;
                    }
                }

                if (Token::Match(tok, "if ( %varid% )", varid)) {
                    addtoken(&rettail, tok, "if(var)");

                    // Make sure the "use" will not be added
                    tok = tok->next()->link();
                    continue;
                } else if (Token::simpleMatch(tok, "if (") && notvar(tok->tokAt(2), varid, true)) {
                    addtoken(&rettail, tok, "if(!var)");

                    // parse the if-body.
                    // if a variable is assigned then add variable to "extravar".
                    for (const Token *tok2 = tok->next()->link()->tokAt(2); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == "{")
                            tok2 = tok2->link();
                        else if (tok2->str() == "}")
                            break;
                        else if (Token::Match(tok2, "%var% ="))
                            extravar.insert(tok2->varId());
                    }

                    tok = tok->next()->link();
                    continue;
                } else {
                    // Check if the condition depends on var or extravar somehow..
                    bool dep = false;
                    const Token* const end = tok->linkAt(1);
                    for (const Token *tok2 = tok->next(); tok2 != end; tok2 = tok2->next()) {
                        if (Token::Match(tok2, "close|pclose|fclose|closedir ( %varid% )", varid)) {
                            addtoken(&rettail, tok, "dealloc");
                            addtoken(&rettail, tok, ";");
                            dep = true;
                            break;
                        } else if (alloctype == Fd && Token::Match(tok2, "%varid% !=|>=", varid)) {
                            dep = true;
                        } else if (Token::Match(tok2, "! %varid%", varid)) {
                            dep = true;
                        } else if (Token::Match(tok2, "%name% (") && !test_white_list(tok2->str(), _settings, tokenizer->isCPP())) {
                            bool use = false;
                            for (const Token *tok3 = tok2->tokAt(2); tok3; tok3 = tok3->nextArgument()) {
                                if (Token::Match(tok3->previous(), "(|, &| %varid% ,|)", varid)) {
                                    use = true;
                                    break;
                                }
                            }
                            if (use) {
                                addtoken(&rettail, tok, "use");
                                addtoken(&rettail, tok, ";");
                                dep = false;
                                break;
                            }
                        } else if (tok2->varId() && extravar.find(tok2->varId()) != extravar.end()) {
                            dep = true;
                        } else if (tok2->varId() == varid &&
                                   (tok2->next()->isConstOp() || tok2->previous()->isConstOp()))
                            dep = true;
                    }

                    if (Token::Match(tok, "if ( ! %varid% &&", varid)) {
                        addtoken(&rettail, tok, "if(!var)");
                    } else if (tok->next() &&
                               tok->next()->link() &&
                               Token::Match(tok->next()->link()->tokAt(-3), "&& ! %varid%", varid)) {
                        addtoken(&rettail, tok, "if(!var)");
                    } else {
                        addtoken(&rettail, tok, (dep ? "ifv" : "if"));
                    }

                    tok = tok->next()->link();
                    continue;
                }
            }
        }

        if ((tok->str() == "else") || (tok->str() == "switch")) {
            addtoken(&rettail, tok, tok->str());
            if (Token::simpleMatch(tok, "switch ("))
                tok = tok->next()->link();
            continue;
        }

        if ((tok->str() == "case")) {
            addtoken(&rettail, tok, "case");
            addtoken(&rettail, tok, ";");
            if (Token::Match(tok, "case %any% :"))
                tok = tok->tokAt(2);
            continue;
        }

        if ((tok->str() == "default")) {
            addtoken(&rettail, tok, "default");
            addtoken(&rettail, tok, ";");
            continue;
        }

        // Loops..
        else if ((tok->str() == "for") || (tok->str() == "while")) {
            const Token* const end = tok->linkAt(1);

            if (Token::simpleMatch(tok, "while ( true )") ||
                Token::simpleMatch(tok, "for ( ; ; )")) {
                addtoken(&rettail, tok, "while1");
                tok = end;
                continue;
            }

            else if (varid && getDeallocationType(tok->tokAt(2), varid) != No) {
                addtoken(&rettail, tok, "dealloc");
                addtoken(&rettail, tok, ";");
            }

            else if (alloctype == Fd && varid) {
                if (Token::Match(tok, "while ( 0 <= %varid% )", varid) ||
                    Token::Match(tok, "while ( %varid% >= 0 )", varid) ||
                    Token::Match(tok, "while ( %varid% != -1 )", varid) ||
                    Token::Match(tok, "while ( -1 != %varid% )", varid)) {
                    addtoken(&rettail, tok, "while(var)");
                    tok = end;
                    continue;
                } else if (Token::Match(tok, "while ( %varid% == -1 )", varid) ||
                           Token::Match(tok, "while ( -1 == %varid% )", varid) ||
                           Token::Match(tok, "while ( %varid% < 0 )", varid) ||
                           Token::Match(tok, "while ( 0 > %varid% )", varid)) {
                    addtoken(&rettail, tok, "while(!var)");
                    tok = end;
                    continue;
                }
            }

            else if (varid && Token::Match(tok, "while ( %varid% )", varid)) {
                addtoken(&rettail, tok, "while(var)");
                tok = end;
                continue;
            } else if (varid && Token::simpleMatch(tok, "while (") && notvar(tok->tokAt(2), varid, true)) {
                addtoken(&rettail, tok, "while(!var)");
                tok = end;
                continue;
            }

            addtoken(&rettail, tok, "loop");

            if (varid > 0) {
                for (const Token *tok2 = tok->tokAt(2); tok2 && tok2 != end; tok2 = tok2->next()) {
                    if (notvar(tok2, varid)) {
                        addtoken(&rettail, tok2, "!var");
                        break;
                    }
                }
            }

            continue;
        }
        if ((tok->str() == "do")) {
            addtoken(&rettail, tok, "do");
            continue;
        }

        // continue / break..
        else if (tok->str() == "continue") {
            addtoken(&rettail, tok, "continue");
        } else if (tok->str() == "break") {
            addtoken(&rettail, tok, "break");
        } else if (tok->str() == "goto") {
            addtoken(&rettail, tok, "goto");
        }

        // Return..
        else if (tok->str() == "return") {
            addtoken(&rettail, tok, "return");
            if (varid == 0) {
                addtoken(&rettail, tok, ";");
                while (tok && tok->str() != ";")
                    tok = tok->next();
                if (!tok)
                    break;
                continue;
            }

            // Returning a auto_ptr of this allocated variable..
            if (Token::simpleMatch(tok->next(), "std :: auto_ptr <")) {
                const Token *tok2 = tok->linkAt(4);
                if (Token::Match(tok2, "> ( %varid% )", varid)) {
                    addtoken(&rettail, tok, "use");
                    tok = tok2->tokAt(3);
                }
            }

            else if (varid && Token::Match(tok, "return strcpy|strncpy|memcpy ( %varid%", varid)) {
                addtoken(&rettail, tok, "use");
                tok = tok->tokAt(2);
            }

            else {
                bool use = false;

                std::stack<const Token *> f;

                for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == ";") {
                        tok = tok2;
                        break;
                    }

                    if (tok2->str() == "(")
                        f.push(tok2->previous());
                    else if (!f.empty() && tok2->str() == ")")
                        f.pop();

                    if (tok2->varId() == varid) {
                        // Read data..
                        if (!Token::Match(tok2->previous(), "&|(") &&
                            tok2->strAt(1) == "[") {
                        } else if (f.empty() ||
                                   !test_white_list(f.top()->str(), _settings, tokenizer->isCPP()) ||
                                   getDeallocationType(f.top(),varid)) {
                            use = true;
                        }
                    }
                }
                if (use)
                    addtoken(&rettail, tok, "use");
                addtoken(&rettail, tok, ";");
            }
        }

        // throw..
        else if (Token::Match(tok, "try|throw|catch")) {
            addtoken(&rettail, tok, tok->str());
            if (tok->strAt(1) == "(")
                tok = tok->next()->link();
        }

        // Assignment..
        if (varid) {
            if (Token::simpleMatch(tok, "= {")) {
                const Token* const end2 = tok->linkAt(1);
                bool use = false;
                for (const Token *tok2 = tok; tok2 != end2; tok2 = tok2->next()) {
                    if (tok2->varId() == varid) {
                        use = true;
                        break;
                    }
                }

                if (use) {
                    addtoken(&rettail, tok, "use");
                    addtoken(&rettail, tok, ";");
                    tok = tok->next()->link();
                    continue;
                }
            }

            if (Token::Match(tok, "& %name% = %varid% ;", varid)) {
                while (rethead->next())
                    rethead->deleteNext();
                return rethead;
            }
            if (Token::Match(tok, "[)=] %varid% [+;)]", varid) ||
                (Token::Match(tok, "%name% + %varid%", varid) &&
                 tok->strAt(3) != "[" &&
                 tok->strAt(3) != ".") ||
                Token::Match(tok, "<< %varid% ;", varid) ||
                Token::Match(tok, "= strcpy|strcat|memmove|memcpy ( %varid% ,", varid) ||
                Token::Match(tok, "[;{}] %name% [ %varid% ]", varid)) {
                addtoken(&rettail, tok, "use");
            } else if (Token::Match(tok->previous(), ";|{|}|=|(|,|%cop% %varid% .|[", varid)) {
                // warning is written for "dealloc ; use_ ;".
                // but this use doesn't affect the leak-checking
                addtoken(&rettail, tok, "use_");
            }
        }

        // Investigate function calls..
        if (Token::Match(tok, "%name% (")) {
            // A function call should normally be followed by ";"
            if (Token::simpleMatch(tok->next()->link(), ") {")) {
                if (!Token::Match(tok, "if|for|while|switch")) {
                    addtoken(&rettail, tok, "exit");
                    addtoken(&rettail, tok, ";");
                    tok = tok->next()->link();
                    continue;
                }
            }

            // Calling setjmp / longjmp => bail out
            else if (Token::Match(tok, "setjmp|longjmp")) {
                while (rethead->next())
                    rethead->deleteNext();
                return rethead;
            }

            // Inside class function.. if the var is passed as a parameter then
            // just add a "::use"
            // The "::use" means that a member function was probably called but it wasn't analysed further
            else if (classmember) {
                if (_settings->library.isnoreturn(tok))
                    addtoken(&rettail, tok, "exit");

                else if (!test_white_list(tok->str(), _settings, tokenizer->isCPP())) {
                    const Token* const end2 = tok->linkAt(1);
                    for (const Token *tok2 = tok->tokAt(2); tok2 != end2; tok2 = tok2->next()) {
                        if (tok2->varId() == varid) {
                            addtoken(&rettail, tok, "::use");
                            break;
                        }
                    }
                }
            }

            else {
                if (varid > 0 && Token::Match(tok, "%name% ( close|fclose|pclose ( %varid% ) ) ;", varid)) {
                    addtoken(&rettail, tok, "dealloc");
                    tok = tok->next()->link();
                    continue;
                }

                bool allocpar = false;
                const char *str = call_func(tok, callstack, varid, alloctype, dealloctype, allocpar, sz);
                if (str) {
                    if (allocpar) {
                        addtoken(&rettail, tok, str);
                        tok = tok->next()->link();
                    } else if (varid == 0 || str != std::string("alloc")) {
                        addtoken(&rettail, tok, str);
                    } else if (Token::Match(tok->tokAt(-2), "%varid% =", varid)) {
                        addtoken(&rettail, tok, str);
                    }
                } else if (varid > 0 &&
                           getReallocationType(tok, varid) != No &&
                           tok->tokAt(2)->varId() == varid) {
                    addtoken(&rettail, tok, "if");
                    addtoken(&rettail, tok, "{");
                    addtoken(&rettail, tok, "dealloc");
                    addtoken(&rettail, tok, ";");
                    addtoken(&rettail, tok, "}");
                    tok = tok->next()->link();
                    continue;
                }
            }
        }

        // Callback..
        if (Token::Match(tok, "( *| %name%") && Token::simpleMatch(tok->link(),") (")) {
            const Token *tok2 = tok->next();
            if (tok2->str() == "*")
                tok2 = tok2->next();
            tok2 = tok2->next();

            while (Token::Match(tok2, ". %name%"))
                tok2 = tok2->tokAt(2);

            if (Token::simpleMatch(tok2, ") (")) {
                for (; tok2; tok2 = tok2->next()) {
                    if (Token::Match(tok2, "[;{]"))
                        break;
                    else if (tok2->varId() == varid) {
                        addtoken(&rettail, tok, "use");
                        break;
                    }
                }
            }
        }

        // Linux lists..
        if (varid > 0 && Token::Match(tok, "[=(,] & (| %varid% [.[,)]", varid)) {
            // Is variable passed to a "leak-ignore" function?
            bool leakignore = false;
            if (Token::Match(tok, "[(,]")) {
                const Token *parent = tok;
                while (parent && parent->str() != "(")
                    parent = parent->astParent();
                if (parent && parent->astOperand1() && parent->astOperand1()->isName()) {
                    const std::string &functionName = parent->astOperand1()->str();
                    if (_settings->library.leakignore.find(functionName) != _settings->library.leakignore.end())
                        leakignore = true;
                }
            }
            // Not passed to "leak-ignore" function, add "&use".
            if (!leakignore)
                addtoken(&rettail, tok, "&use");
        }
    }

    for (Token *tok1 = rethead; tok1; tok1 = tok1->next()) {
        if (Token::simpleMatch(tok1, "callfunc alloc ;")) {
            tok1->deleteThis();
            tok1->insertToken("use");
            tok1->insertToken(";");
        }
    }

    return rethead;
}




void CheckMemoryLeakInFunction::simplifycode(Token *tok) const
{
    {
        // Replace "throw" that is not in a try block with "return"
        int indentlevel = 0;
        int trylevel = -1;
        for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{")
                ++indentlevel;
            else if (tok2->str() == "}") {
                --indentlevel;
                if (indentlevel <= trylevel)
                    trylevel = -1;
            } else if (trylevel == -1 && tok2->str() == "try")
                trylevel = indentlevel;
            else if (trylevel == -1 && tok2->str() == "throw")
                tok2->str("return");
        }
    }

    const bool printExperimental = _settings->experimental;

    // Insert extra ";"
    for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
        if (!tok2->previous() || Token::Match(tok2->previous(), "[;{}]")) {
            if (Token::Match(tok2, "assign|callfunc|use assign|callfunc|use|}")) {
                tok2->insertToken(";");
            }
        }
    }

    // remove redundant braces..
    for (Token *start = tok; start; start = start->next()) {
        if (Token::simpleMatch(start, "; {")) {
            // the "link" doesn't work here. Find the end brace..
            unsigned int indent = 0;
            for (Token *end = start; end; end = end->next()) {
                if (end->str() == "{")
                    ++indent;
                else if (end->str() == "}") {
                    if (indent <= 1) {
                        // If the start/end braces are redundant, delete them
                        if (indent == 1 && Token::Match(end->previous(), "[;{}] } %any%")) {
                            start->deleteNext();
                            end->deleteThis();
                        }
                        break;
                    }
                    --indent;
                }
            }
        }
    }

    // reduce the code..
    // it will be reduced in N passes. When a pass completes without any
    // simplifications the loop is done.
    bool done = false;
    while (! done) {
        //tok->printOut("simplifycode loop..");
        done = true;

        // reduce callfunc
        for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "callfunc") {
                if (!Token::Match(tok2->previous(), "[;{}] callfunc ; }"))
                    tok2->deleteThis();
            }
        }

        // If the code starts with "if return ;" then remove it
        if (Token::Match(tok, ";| if return ;")) {
            tok->deleteNext();
            tok->deleteThis();
            if (tok->str() == "return")
                tok->deleteThis();
            if (tok->strAt(1) == "else")
                tok->deleteNext();
        }

        // simplify "while1" contents..
        for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (Token::simpleMatch(tok2, "while1 {")) {
                unsigned int innerIndentlevel = 0;
                for (Token *tok3 = tok2->tokAt(2); tok3; tok3 = tok3->next()) {
                    if (tok3->str() == "{")
                        ++innerIndentlevel;
                    else if (tok3->str() == "}") {
                        if (innerIndentlevel == 0)
                            break;
                        --innerIndentlevel;
                    }
                    while (innerIndentlevel == 0 && Token::Match(tok3, "[{};] if|ifv|else { continue ; }")) {
                        tok3->deleteNext(5);
                        if (tok3->strAt(1) == "else")
                            tok3->deleteNext();
                    }
                }

                if (Token::simpleMatch(tok2, "while1 { if { dealloc ; return ; } }")) {
                    tok2->str(";");
                    tok2->deleteNext(3);
                    tok2->tokAt(4)->deleteNext(2);
                }
            }
        }

        // Main inner simplification loop
        for (Token *tok2 = tok; tok2; tok2 = tok2 ? tok2->next() : nullptr) {
            // Delete extra ";"
            while (Token::Match(tok2, "[;{}] ;")) {
                tok2->deleteNext();
                done = false;
            }

            // Replace "{ }" with ";"
            if (Token::simpleMatch(tok2->next(), "{ }")) {
                tok2->deleteNext(2);
                tok2->insertToken(";");
                done = false;
            }

            // Delete braces around a single instruction..
            if (Token::Match(tok2->next(), "{ %name% ; }")) {
                tok2->deleteNext();
                tok2->tokAt(2)->deleteNext();
                done = false;
            }
            if (Token::Match(tok2->next(), "{ %name% %name% ; }")) {
                tok2->deleteNext();
                tok2->tokAt(3)->deleteNext();
                done = false;
            }

            // Reduce "if if|callfunc" => "if"
            else if (Token::Match(tok2, "if if|callfunc")) {
                tok2->deleteNext();
                done = false;
            }

            // outer/inner if blocks. Remove outer condition..
            else if (Token::Match(tok2->next(), "if|if(var) { if return use ; }")) {
                tok2->deleteNext(2);
                tok2->tokAt(4)->deleteNext();
                done = false;
            }

            else if (tok2->next() && tok2->next()->str() == "if") {
                // Delete empty if that is not followed by an else
                if (Token::Match(tok2->next(), "if ; !!else")) {
                    tok2->deleteNext();
                    done = false;
                }

                // Reduce "if X ; else X ;" => "X ;"
                else if (Token::Match(tok2->next(), "if %name% ; else %name% ;") &&
                         tok2->strAt(2) == tok2->strAt(5)) {
                    tok2->deleteNext(4);
                    done = false;
                }

                // Reduce "if continue ; if continue ;" => "if continue ;"
                else if (Token::simpleMatch(tok2->next(), "if continue ; if continue ;")) {
                    tok2->deleteNext(3);
                    done = false;
                }

                // Reduce "if return ; alloc ;" => "alloc ;"
                else if (Token::Match(tok2, "[;{}] if return ; alloc|return ;")) {
                    tok2->deleteNext(3);
                    done = false;
                }

                // "[;{}] if alloc ; else return ;" => "[;{}] alloc ;"
                else if (Token::Match(tok2, "[;{}] if alloc ; else return ;")) {
                    // Remove "if"
                    tok2->deleteNext();
                    // Remove "; else return"
                    tok2->next()->deleteNext(3);
                    done = false;
                }

                // Reduce "if ; else %name% ;" => "if %name% ;"
                else if (Token::Match(tok2->next(), "if ; else %name% ;")) {
                    tok2->next()->deleteNext(2);
                    done = false;
                }

                // Reduce "if ; else" => "if"
                else if (Token::simpleMatch(tok2->next(), "if ; else")) {
                    tok2->next()->deleteNext(2);
                    done = false;
                }

                // Reduce "if return ; else|if return|continue ;" => "if return ;"
                else if (Token::Match(tok2->next(), "if return ; else|if return|continue|break ;")) {
                    tok2->tokAt(3)->deleteNext(3);
                    done = false;
                }

                // Reduce "if continue|break ; else|if return ;" => "if return ;"
                else if (Token::Match(tok2->next(), "if continue|break ; if|else return ;")) {
                    tok2->next()->deleteNext(3);
                    done = false;
                }

                // Remove "else" after "if continue|break|return"
                else if (Token::Match(tok2->next(), "if continue|break|return ; else")) {
                    tok2->tokAt(3)->deleteNext();
                    done = false;
                }

                // Delete "if { dealloc|assign|use ; return ; }"
                else if (Token::Match(tok2, "[;{}] if { dealloc|assign|use ; return ; }") &&
                         !Token::findmatch(tok, "if {| alloc ;")) {
                    tok2->deleteNext(7);
                    if (tok2->strAt(1) == "else")
                        tok2->deleteNext();
                    done = false;
                }

                // Remove "if { dealloc ; callfunc ; } !!else|return"
                else if (Token::Match(tok2->next(), "if { dealloc|assign ; callfunc ; }") &&
                         !Token::Match(tok2->tokAt(8), "else|return")) {
                    tok2->deleteNext(7);
                    done = false;
                }

                continue;
            }

            // Reduce "alloc while(!var) alloc ;" => "alloc ;"
            if (Token::Match(tok2, "[;{}] alloc ; while(!var) alloc ;")) {
                tok2->deleteNext(3);
                done = false;
            }

            // Reduce "ifv return;" => "if return use;"
            if (Token::simpleMatch(tok2, "ifv return ;")) {
                tok2->str("if");
                tok2->next()->insertToken("use");
                done = false;
            }

            // Reduce "if(var) dealloc ;" and "if(var) use ;" that is not followed by an else..
            if (Token::Match(tok2, "[;{}] if(var) assign|dealloc|use ; !!else")) {
                tok2->deleteNext();
                done = false;
            }

            // Reduce "; if(!var) alloc ; !!else" => "; dealloc ; alloc ;"
            if (Token::Match(tok2, "; if(!var) alloc ; !!else")) {
                // Remove the "if(!var)"
                tok2->deleteNext();

                // Insert "dealloc ;" before the "alloc ;"
                tok2->insertToken(";");
                tok2->insertToken("dealloc");

                done = false;
            }

            // Reduce "if(!var) exit ;" => ";"
            if (Token::simpleMatch(tok2, "; if(!var) exit ;")) {
                tok2->deleteNext(2);
                done = false;
            }

            // Reduce "if* ;"..
            if (Token::Match(tok2->next(), "if(var)|if(!var)|ifv ;")) {
                // Followed by else..
                if (tok2->strAt(3) == "else") {
                    tok2 = tok2->next();
                    if (tok2->str() == "if(var)")
                        tok2->str("if(!var)");
                    else if (tok2->str() == "if(!var)")
                        tok2->str("if(var)");

                    // remove the "; else"
                    tok2->deleteNext(2);
                } else {
                    // remove the "if*"
                    tok2->deleteNext();
                }
                done = false;
            }

            // Reduce "else ;" => ";"
            if (Token::simpleMatch(tok2->next(), "else ;")) {
                tok2->deleteNext();
                done = false;
            }

            // Reduce "while1 continue| ;" => "use ;"
            if (Token::Match(tok2, "while1 if| continue| ;")) {
                tok2->str("use");
                while (tok2->strAt(1) != ";")
                    tok2->deleteNext();
                done = false;
            }

            // Reduce "while1 if break ;" => ";"
            if (Token::simpleMatch(tok2, "while1 if break ;")) {
                tok2->str(";");
                tok2->deleteNext(2);
                done = false;
            }

            // Delete if block: "alloc; if return use ;"
            if (Token::Match(tok2, "alloc ; if return use ; !!else")) {
                tok2->deleteNext(4);
                done = false;
            }

            // Reduce "alloc|dealloc|use|callfunc ; exit ;" => "; exit ;"
            if (Token::Match(tok2, "[;{}] alloc|dealloc|use|callfunc ; exit ;")) {
                tok2->deleteNext();
                done = false;
            }

            // Reduce "alloc|dealloc|use ; if(var) exit ;"
            if (Token::Match(tok2, "alloc|dealloc|use ; if(var) exit ;")) {
                tok2->deleteThis();
                done = false;
            }

            // Remove "if exit ;"
            if (Token::simpleMatch(tok2, "if exit ;")) {
                tok2->deleteNext();
                tok2->deleteThis();
                done = false;
            }

            // Remove the "if break|continue ;" that follows "dealloc ; alloc ;"
            if (!printExperimental && Token::Match(tok2, "dealloc ; alloc ; if break|continue ;")) {
                tok2->tokAt(3)->deleteNext(2);
                done = false;
            }

            // if break ; break ; => break ;
            if (Token::Match(tok2->previous(), "[;{}] if break ; break ;")) {
                tok2->deleteNext(3);
                done = false;
            }

            // Reduce "do { dealloc ; alloc ; } while(var) ;" => ";"
            if (Token::simpleMatch(tok2->next(), "do { dealloc ; alloc ; } while(var) ;")) {
                tok2->deleteNext(8);
                done = false;
            }

            // Reduce "do { alloc ; } " => "alloc ;"
            /** @todo If the loop "do { alloc ; }" can be executed twice, reduce it to "loop alloc ;" */
            if (Token::simpleMatch(tok2->next(), "do { alloc ; }")) {
                tok2->deleteNext(2);
                tok2->tokAt(2)->deleteNext();
                done = false;
            }

            // Reduce "loop break ; => ";"
            if (Token::Match(tok2->next(), "loop break|continue ;")) {
                tok2->deleteNext(2);
                done = false;
            }

            // Reduce "loop|do ;" => ";"
            if (Token::Match(tok2, "loop|do ;")) {
                tok2->deleteThis();
                done = false;
            }

            // Reduce "loop if break|continue ; !!else" => ";"
            if (Token::Match(tok2->next(), "loop if break|continue ; !!else")) {
                tok2->deleteNext(3);
                done = false;
            }

            // Reduce "loop { if break|continue ; !!else" => "loop {"
            if (Token::Match(tok2, "loop { if break|continue ; !!else")) {
                tok2->next()->deleteNext(3);
                done = false;
            }

            // Replace "do ; loop ;" with ";"
            if (Token::simpleMatch(tok2, "; loop ;")) {
                tok2->deleteNext(2);
                done = false;
            }

            // Replace "loop loop .." with "loop .."
            if (Token::simpleMatch(tok2, "loop loop")) {
                tok2->deleteThis();
                done = false;
            }

            // Replace "loop if return ;" with "if return ;"
            if (Token::simpleMatch(tok2->next(), "loop if return")) {
                tok2->deleteNext();
                done = false;
            }

            // Reduce "loop|while1 { dealloc ; alloc ; }"
            if (Token::Match(tok2, "loop|while1 { dealloc ; alloc ; }")) {
                // delete "{"
                tok2->deleteNext();
                // delete "loop|while1"
                tok2->deleteThis();

                // delete "}"
                tok2->tokAt(3)->deleteNext();

                done = false;
            }

            // loop { use ; callfunc ; }  =>  use ;
            // assume that the "callfunc" is not noreturn
            if (Token::simpleMatch(tok2, "loop { use ; callfunc ; }")) {
                tok2->deleteNext(6);
                tok2->str("use");
                tok2->insertToken(";");
                done = false;
            }

            // Delete if block in "alloc ; if(!var) return ;"
            if (Token::simpleMatch(tok2, "alloc ; if(!var) return ;")) {
                tok2->deleteNext(3);
                done = false;
            }

            // Reduce "[;{}] return use ; %name%" => "[;{}] return use ;"
            if (Token::Match(tok2, "[;{}] return use ; %name%")) {
                tok2->tokAt(3)->deleteNext();
                done = false;
            }

            // Reduce "if(var) return use ;" => "return use ;"
            if (Token::Match(tok2->next(), "if(var) return use ; !!else")) {
                tok2->deleteNext();
                done = false;
            }

            // malloc - realloc => alloc ; dealloc ; alloc ;
            // Reduce "[;{}] alloc ; dealloc ; alloc ;" => "[;{}] alloc ;"
            if (Token::Match(tok2, "[;{}] alloc ; dealloc ; alloc ;")) {
                tok2->deleteNext(4);
                done = false;
            }

            // use; dealloc; => dealloc;
            if (Token::Match(tok2, "[;{}] use ; dealloc ;")) {
                tok2->deleteNext(2);
                done = false;
            }

            // use use => use
            while (Token::simpleMatch(tok2, "use use")) {
                tok2->deleteNext();
                done = false;
            }

            // use use_ => use
            if (Token::simpleMatch(tok2, "use use_")) {
                tok2->deleteNext();
                done = false;
            }

            // use_ use => use
            if (Token::simpleMatch(tok2, "use_ use")) {
                tok2->deleteThis();
                done = false;
            }

            // use & use => use
            while (Token::simpleMatch(tok2, "use & use")) {
                tok2->deleteNext(2);
                done = false;
            }

            // & use use => use
            while (Token::simpleMatch(tok2, "& use use")) {
                tok2->deleteThis();
                tok2->deleteThis();
                done = false;
            }

            // use; if| use; => use;
            while (Token::Match(tok2, "[;{}] use ; if| use ;")) {
                Token *t = tok2->tokAt(2);
                t->deleteNext(2+(t->str()=="if" ? 1 : 0));
                done = false;
            }

            // Delete first part in "use ; return use ;"
            if (Token::Match(tok2, "[;{}] use ; return use ;")) {
                tok2->deleteNext(2);
                done = false;
            }

            // try/catch
            if (Token::simpleMatch(tok2, "try ; catch exit ;")) {
                tok2->deleteNext(3);
                tok2->deleteThis();
                done = false;
            }

            // Delete second case in "case ; case ;"
            while (Token::simpleMatch(tok2, "case ; case ;")) {
                tok2->deleteNext(2);
                done = false;
            }

            // Replace switch with if (if not complicated)
            if (Token::simpleMatch(tok2, "switch {")) {
                // Right now, I just handle if there are a few case and perhaps a default.
                bool valid = false;
                bool incase = false;
                for (const Token * _tok = tok2->tokAt(2); _tok; _tok = _tok->next()) {
                    if (_tok->str() == "{")
                        break;

                    else if (_tok->str() == "}") {
                        valid = true;
                        break;
                    }

                    else if (_tok->str() == "switch")
                        break;

                    else if (_tok->str() == "loop")
                        break;

                    else if (incase && _tok->str() == "case")
                        break;

                    else if (Token::Match(_tok, "return !!;"))
                        break;

                    if (Token::Match(_tok, "if return|break use| ;"))
                        _tok = _tok->tokAt(2);

                    incase = incase || (_tok->str() == "case");
                    incase = incase && (_tok->str() != "break" && _tok->str() != "return");
                }

                if (!incase && valid) {
                    done = false;
                    tok2->str(";");
                    tok2->deleteNext();
                    tok2 = tok2->next();
                    bool first = true;
                    while (Token::Match(tok2, "case|default")) {
                        const bool def(tok2->str() == "default");
                        tok2->str(first ? "if" : "}");
                        if (first) {
                            first = false;
                            tok2->insertToken("{");
                        } else {
                            // Insert "else [if] {
                            tok2->insertToken("{");
                            if (! def)
                                tok2->insertToken("if");
                            tok2->insertToken("else");
                            tok2 = tok2->next();
                        }
                        while (tok2) {
                            if (tok2->str() == "}")
                                break;
                            if (Token::Match(tok2, "break|return ;"))
                                break;
                            if (Token::Match(tok2, "if return|break use| ;"))
                                tok2 = tok2->tokAt(2);
                            else
                                tok2 = tok2->next();
                        }
                        if (Token::simpleMatch(tok2, "break ;")) {
                            tok2->str(";");
                            tok2 = tok2->tokAt(2);
                        } else if (tok2 && tok2->str() == "return") {
                            tok2 = tok2->tokAt(2);
                        }
                    }
                }
            }
        }

        // If "--all" is given, remove all "callfunc"..
        if (done &&  printExperimental) {
            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "callfunc") {
                    tok2->deleteThis();
                    done = false;
                }
            }
        }
    }
}



const Token *CheckMemoryLeakInFunction::findleak(const Token *tokens)
{
    const Token *result;

    if ((result = Token::findsimplematch(tokens, "loop alloc ;")) != nullptr) {
        return result;
    }

    if (Token::Match(tokens, "alloc ; if|if(var)|ifv break|continue|return ;")) {
        return tokens->tokAt(3);
    }

    if ((result = Token::findmatch(tokens, "alloc ; if|if(var)|ifv return ;")) != nullptr) {
        return result->tokAt(3);
    }

    if ((result = Token::findmatch(tokens, "alloc ; alloc|assign|return callfunc| ;")) != nullptr) {
        return result->tokAt(2);
    }

    if ((result = Token::findsimplematch(tokens, "; alloc ; if assign ;")) != nullptr) {
        return result->tokAt(4);
    }

    if (((result = Token::findsimplematch(tokens, "; alloc ; if dealloc ; }")) != nullptr) &&
        !result->tokAt(7)) {
        return result->tokAt(6);
    }

    if ((result = Token::findsimplematch(tokens, "alloc ; }")) != nullptr) {
        if (result->tokAt(3) == nullptr)
            return result->tokAt(2);
    }

    // No deallocation / usage => report leak at the last token
    if (!Token::findmatch(tokens, "dealloc|use")) {
        const Token *last = tokens;
        while (last->next())
            last = last->next();

        // not a leak if exit is called before the end of the function
        if (!Token::Match(last->tokAt(-2), "exit|callfunc ; }"))
            return last;
    }

    return nullptr;
}


// Check for memory leaks for a function variable.
void CheckMemoryLeakInFunction::checkScope(const Token *startTok, const std::string &varname, unsigned int varid, bool classmember, unsigned int sz)
{
    std::list<const Token *> callstack;

    AllocType alloctype = No;
    AllocType dealloctype = No;

    const Token *result;

    Token *tok = getcode(startTok, callstack, varid, alloctype, dealloctype, classmember, sz);
    //tok->printOut((std::string("Checkmemoryleak: getcode result for: ") + varname).c_str());

    const bool use_addr = bool(Token::findsimplematch(tok, "&use") != nullptr);

    // Simplify the code and check if freed memory is used..
    for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
        while (Token::Match(tok2, "[;{}] ;"))
            tok2->deleteNext();
    }
    if ((result = Token::findmatch(tok, "[;{}] dealloc ; use_ ;")) != nullptr) {
        deallocuseError(result->tokAt(3), varname);
    }

    // Replace "&use" with "use". Replace "use_" with ";"
    for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
        if (tok2->str() == "&use")
            tok2->str("use");
        else if (tok2->str() == "use_")
            tok2->str(";");
        else if (Token::simpleMatch(tok2, "loop use_ {"))
            tok2->deleteNext();
        else if (tok2->str() == "::use")    // Some kind of member function usage. Not analyzed very well.
            tok2->str("use");
        else if (tok2->str() == "recursive")
            tok2->str("use");
        else if (tok2->str() == "dealloc_")
            tok2->str("dealloc");
        else if (tok2->str() == "realloc") {
            tok2->str("dealloc");
            tok2->insertToken("alloc");
            tok2->insertToken(";");
        }
    }

    // If the variable is not allocated at all => no memory leak
    if (Token::findsimplematch(tok, "alloc") == 0) {
        TokenList::deleteTokens(tok);
        return;
    }

    simplifycode(tok);

    if (_settings->debug && _settings->_verbose) {
        tok->printOut(("Checkmemoryleak: simplifycode result for: " + varname).c_str());
    }

    // If the variable is not allocated at all => no memory leak
    if (Token::findsimplematch(tok, "alloc") == 0) {
        TokenList::deleteTokens(tok);
        return;
    }

    /** @todo handle "goto" */
    if (Token::findsimplematch(tok, "goto")) {
        TokenList::deleteTokens(tok);
        return;
    }

    if ((result = findleak(tok)) != nullptr) {
        memoryLeak(result, varname, alloctype);
    }

    else if (!use_addr && (result = Token::findsimplematch(tok, "dealloc ; dealloc ;")) != nullptr) {
        deallocDeallocError(result->tokAt(2), varname);
    }

    // detect cases that "simplifycode" don't handle well..
    else if (tok && _settings->debugwarnings) {
        Token *first = tok;
        while (first && first->str() == ";")
            first = first->next();

        bool noerr = false;
        noerr = noerr || Token::simpleMatch(first, "alloc ; }");
        noerr = noerr || Token::simpleMatch(first, "alloc ; dealloc ; }");
        noerr = noerr || Token::simpleMatch(first, "alloc ; return use ; }");
        noerr = noerr || Token::simpleMatch(first, "alloc ; use ; }");
        noerr = noerr || Token::simpleMatch(first, "alloc ; use ; return ; }");
        noerr = noerr || Token::simpleMatch(first, "alloc ; dealloc ; return ; }");
        noerr = noerr || Token::simpleMatch(first, "if alloc ; dealloc ; }");
        noerr = noerr || Token::simpleMatch(first, "if alloc ; return use ; }");
        noerr = noerr || Token::simpleMatch(first, "if alloc ; use ; }");
        noerr = noerr || Token::simpleMatch(first, "alloc ; ifv return ; dealloc ; }");
        noerr = noerr || Token::simpleMatch(first, "alloc ; if return ; dealloc; }");

        // Unhandled case..
        if (!noerr) {
            std::ostringstream errmsg;
            errmsg << "inconclusive leak of " << varname << ": ";
            errmsg << tok->stringifyList(false, false, false, false, false, 0, 0);
            reportError(first, Severity::debug, "debug", errmsg.str());
        }
    }

    TokenList::deleteTokens(tok);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for memory leaks due to improper realloc() usage.
//   Below, "a" may be set to null without being freed if realloc() cannot
//   allocate the requested memory:
//     a = malloc(10); a = realloc(a, 100);
//---------------------------------------------------------------------------

static bool isNoArgument(const SymbolDatabase* symbolDatabase, unsigned int varid)
{
    const Variable* var = symbolDatabase->getVariableFromVarId(varid);
    return var && !var->isArgument();
}

void CheckMemoryLeakInFunction::checkReallocUsage()
{
    // only check functions
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        // Search for the "var = realloc(var, 100" pattern within this function
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->varId() > 0 &&
                Token::Match(tok, "%name% = realloc|g_try_realloc ( %name% , %any%") &&
                tok->varId() == tok->tokAt(4)->varId() &&
                isNoArgument(symbolDatabase, tok->varId())) {
                // Check that another copy of the pointer wasn't saved earlier in the function
                if (Token::findmatch(scope->classStart, "%name% = %varid% ;", tok, tok->varId()) ||
                    Token::findmatch(scope->classStart, "[{};] %varid% = %name% [;=]", tok, tok->varId()))
                    continue;

                const Token* tokEndRealloc = tok->linkAt(3);
                // Check that the allocation isn't followed immediately by an 'if (!var) { error(); }' that might handle failure
                if (Token::Match(tokEndRealloc->next(), "; if ( ! %varid% ) {", tok->varId())) {
                    const Token* tokEndBrace = tokEndRealloc->linkAt(7);
                    if (tokEndBrace && Token::simpleMatch(tokEndBrace->tokAt(-2), ") ;") &&
                        Token::Match(tokEndBrace->linkAt(-2)->tokAt(-2), "{|}|; %name% ("))
                        continue;
                }

                memleakUponReallocFailureError(tok, tok->str());
            } else if (tok->next()->varId() > 0 &&
                       (Token::Match(tok, "* %name% = realloc|g_try_realloc ( * %name% , %any%") &&
                        tok->next()->varId() == tok->tokAt(6)->varId()) &&
                       isNoArgument(symbolDatabase, tok->next()->varId())) {
                // Check that another copy of the pointer wasn't saved earlier in the function
                if (Token::findmatch(scope->classStart, "%name% = * %varid% ;", tok, tok->next()->varId()) ||
                    Token::findmatch(scope->classStart, "[{};] * %varid% = %name% [;=]", tok, tok->next()->varId()))
                    continue;

                const Token* tokEndRealloc = tok->linkAt(4);
                // Check that the allocation isn't followed immediately by an 'if (!var) { error(); }' that might handle failure
                if (Token::Match(tokEndRealloc->next(), "; if ( ! * %varid% ) {", tok->next()->varId())) {
                    const Token* tokEndBrace = tokEndRealloc->linkAt(8);
                    if (tokEndBrace && Token::simpleMatch(tokEndBrace->tokAt(-2), ") ;") &&
                        Token::Match(tokEndBrace->linkAt(-2)->tokAt(-2), "{|}|; %name% ("))
                        continue;
                }
                memleakUponReallocFailureError(tok->next(), tok->strAt(1));
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checks for memory leaks inside function..
//---------------------------------------------------------------------------

static bool isInMemberFunc(const Scope* scope)
{
    while (scope->nestedIn && !scope->functionOf)
        scope = scope->nestedIn;

    return (scope->functionOf != 0);
}

void CheckMemoryLeakInFunction::check()
{
    // Check locking/unlocking of global resources..
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        checkScope(scope->classStart->next(), "", 0, scope->functionOf != nullptr, 1);
    }

    // Check variables..
    for (unsigned int i = 1; i < symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || (!var->isLocal() && !var->isArgument()) || var->isStatic() || !var->scope())
            continue;

        if (var->isReference())
            continue;

        if (!var->isPointer() && var->typeStartToken()->str() != "int")
            continue;

        // check for known class without implementation (forward declaration)
        if (var->isPointer() && var->type() && !var->typeScope())
            continue;

        unsigned int sz = _tokenizer->sizeOfType(var->typeStartToken());
        if (sz < 1)
            sz = 1;

        if (var->isArgument())
            checkScope(var->scope()->classStart->next(), var->name(), i, isInMemberFunc(var->scope()), sz);
        else
            checkScope(var->nameToken(), var->name(), i, isInMemberFunc(var->scope()), sz);
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checks for memory leaks in classes..
//---------------------------------------------------------------------------


void CheckMemoryLeakInClass::check()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // only check classes and structures
    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];
        std::list<Variable>::const_iterator var;
        for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
            if (!var->isStatic() && var->isPointer()) {
                // allocation but no deallocation of private variables in public function..
                const Token *tok = var->typeStartToken();
                if (tok->isStandardType()) {
                    if (var->isPrivate())
                        checkPublicFunctions(scope, var->nameToken());

                    variable(scope, var->nameToken());
                }

                // known class?
                else if (var->type()) {
                    // not derived?
                    if (var->type()->derivedFrom.empty()) {
                        if (var->isPrivate())
                            checkPublicFunctions(scope, var->nameToken());

                        variable(scope, var->nameToken());
                    }
                }
            }
        }
    }
}


void CheckMemoryLeakInClass::variable(const Scope *scope, const Token *tokVarname)
{
    const std::string& varname = tokVarname->str();
    const unsigned int varid = tokVarname->varId();
    const std::string& classname = scope->className;

    // Check if member variable has been allocated and deallocated..
    CheckMemoryLeak::AllocType Alloc = CheckMemoryLeak::No;
    CheckMemoryLeak::AllocType Dealloc = CheckMemoryLeak::No;

    bool allocInConstructor = false;
    bool deallocInDestructor = false;

    // Inspect member functions
    std::list<Function>::const_iterator func;
    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        const bool constructor = func->isConstructor();
        const bool destructor = func->isDestructor();
        if (!func->hasBody()) {
            if (destructor) { // implementation for destructor is not seen => assume it deallocates all variables properly
                deallocInDestructor = true;
                Dealloc = CheckMemoryLeak::Many;
            }
            continue;
        }
        bool body = false;
        const Token *end = func->functionScope->classEnd;
        for (const Token *tok = func->arg->link(); tok != end; tok = tok->next()) {
            if (tok == func->functionScope->classStart)
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

                        if (Alloc != No && Alloc != alloc)
                            alloc = CheckMemoryLeak::Many;

                        std::list<const Token *> callstack;
                        if (alloc != CheckMemoryLeak::Many && Dealloc != CheckMemoryLeak::No && Dealloc != CheckMemoryLeak::Many && Dealloc != alloc) {
                            callstack.push_back(tok);
                            mismatchAllocDealloc(callstack, classname + "::" + varname);
                        }

                        Alloc = alloc;
                    }
                }

                if (!body)
                    continue;

                // Deallocate..
                AllocType dealloc = getDeallocationType(tok, varname);
                if (dealloc == No) {
                    std::string temp = scope->className + " :: " + varname;
                    dealloc = getDeallocationType(tok, temp);
                }
                if (dealloc == No) {
                    std::string temp = "this . " + varname;
                    dealloc = getDeallocationType(tok, temp);
                }
                // some usage in the destructor => assume it's related
                // to deallocation
                if (destructor && tok->str() == varname)
                    dealloc = CheckMemoryLeak::Many;
                if (dealloc != CheckMemoryLeak::No) {
                    if (destructor)
                        deallocInDestructor = true;

                    // several types of allocation/deallocation?
                    if (Dealloc != CheckMemoryLeak::No && Dealloc != dealloc)
                        dealloc = CheckMemoryLeak::Many;

                    std::list<const Token *> callstack;
                    if (dealloc != CheckMemoryLeak::Many && Alloc != CheckMemoryLeak::No &&  Alloc != Many && Alloc != dealloc) {
                        callstack.push_back(tok);
                        mismatchAllocDealloc(callstack, classname + "::" + varname);
                    }

                    Dealloc = dealloc;
                }

                // Function call .. possible deallocation
                else if (Token::Match(tok->previous(), "[{};] %name% (")) {
                    if (!CheckMemoryLeakInFunction::test_white_list(tok->str(), _settings, tokenizer->isCPP())) {
                        return;
                    }
                }
            }
        }
    }

    if (allocInConstructor && !deallocInDestructor) {
        unsafeClassError(tokVarname, classname, classname + "::" + varname /*, Alloc*/);
    } else if (Alloc != CheckMemoryLeak::No && Dealloc == CheckMemoryLeak::No) {
        unsafeClassError(tokVarname, classname, classname + "::" + varname /*, Alloc*/);
    }
}

void CheckMemoryLeakInClass::unsafeClassError(const Token *tok, const std::string &classname, const std::string &varname)
{
    if (!_settings->isEnabled("style"))
        return;

    reportError(tok, Severity::style, "unsafeClassCanLeak",
                "Class '" + classname + "' is unsafe, '" + varname + "' can leak by wrong usage.\n"
                "The class '" + classname + "' is unsafe, wrong usage can cause memory/resource leaks for '" + varname + "'. This can for instance be fixed by adding proper cleanup in the destructor.");
}


void CheckMemoryLeakInClass::checkPublicFunctions(const Scope *scope, const Token *classtok)
{
    // Check that public functions deallocate the pointers that they allocate.
    // There is no checking how these functions are used and therefore it
    // isn't established if there is real leaks or not.
    if (!_settings->isEnabled("warning"))
        return;

    const unsigned int varid = classtok->varId();
    if (varid == 0) {
        _tokenizer->getSymbolDatabase()->debugMessage(classtok, "CheckMemoryInClass::checkPublicFunctions found variable \'" + classtok->str() + "\' with varid 0");
        return;
    }

    // Parse public functions..
    // If they allocate member variables, they should also deallocate
    std::list<Function>::const_iterator func;

    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        if ((func->type == Function::eFunction || func->type == Function::eOperatorEqual) &&
            func->access == Public && func->hasBody()) {
            const Token *tok2 = func->token;
            while (tok2->str() != "{")
                tok2 = tok2->next();
            if (Token::Match(tok2, "{|}|; %varid% =", varid)) {
                const CheckMemoryLeak::AllocType alloc = getAllocationType(tok2->tokAt(3), varid);
                if (alloc != CheckMemoryLeak::No)
                    publicAllocationError(tok2, tok2->strAt(1));
            } else if (Token::Match(tok2, "{|}|; %type% :: %varid% =", varid) &&
                       tok2->next()->str() == scope->className) {
                const CheckMemoryLeak::AllocType alloc = getAllocationType(tok2->tokAt(5), varid);
                if (alloc != CheckMemoryLeak::No)
                    publicAllocationError(tok2, tok2->strAt(3));
            }
        }
    }
}

void CheckMemoryLeakInClass::publicAllocationError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "publicAllocationError", "Possible leak in public function. The pointer '" + varname + "' is not deallocated before it is allocated.");
}


void CheckMemoryLeakStructMember::check()
{
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    for (unsigned int i = 1; i < symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || !var->isLocal() || var->isStatic())
            continue;
        if (var->typeEndToken()->isStandardType())
            continue;
        checkStructVariable(var);
    }
}

bool CheckMemoryLeakStructMember::isMalloc(const Variable *variable)
{
    const unsigned int declarationId(variable->declarationId());
    bool alloc = false;
    for (const Token *tok2 = variable->nameToken(); tok2 && tok2 != variable->scope()->classEnd; tok2 = tok2->next()) {
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
    // This should be in the CheckMemoryLeak base class
    static std::set<std::string> ignoredFunctions;
    if (ignoredFunctions.empty()) {
        ignoredFunctions.insert("if");
        ignoredFunctions.insert("for");
        ignoredFunctions.insert("while");
        ignoredFunctions.insert("malloc");
    }

    // Is struct variable a pointer?
    if (variable->isPointer()) {
        // Check that variable is allocated with malloc
        if (!isMalloc(variable))
            return;
    } else if (!_tokenizer->isC()) {
        // For non-C code a destructor might cleanup members
        return;
    }

    // Check struct..
    unsigned int indentlevel2 = 0;
    for (const Token *tok2 = variable->nameToken(); tok2 != variable->scope()->classEnd; tok2 = tok2->next()) {
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
        else if (Token::Match(tok2->previous(), "[;{}] %varid% . %var% = malloc|strdup|kmalloc|fopen (", variable->declarationId())
                 || Token::Match(tok2->previous(), "[;{}] %varid% . %var% = new", variable->declarationId())) {
            const unsigned int structid(variable->declarationId());
            const unsigned int structmemberid(tok2->tokAt(2)->varId());

            // This struct member is allocated.. check that it is deallocated
            unsigned int indentlevel3 = indentlevel2;
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
                else if (Token::Match(tok3, "free|kfree ( %var% . %varid% )", structmemberid)
                         || Token::Match(tok3, "delete %var% . %varid%", structmemberid)
                         || Token::Match(tok3, "delete [ ] %var% . %varid%", structmemberid)
                         || Token::Match(tok3, "fclose ( %var% . %varid%", structmemberid)) {
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
                else if (indentlevel2 == 0 && Token::Match(tok3, "free|kfree ( %varid% )", structid)) {
                    memoryLeak(tok3, variable->name() + "." + tok2->strAt(2), Malloc);
                    break;
                }

                // failed allocation => skip code..
                else if (Token::Match(tok3, "if ( ! %var% . %varid% )", structmemberid)) {
                    // Goto the ")"
                    tok3 = tok3->next()->link();

                    // make sure we have ") {".. it should be
                    if (!Token::simpleMatch(tok3, ") {"))
                        break;

                    // Goto the "}"
                    tok3 = tok3->next()->link();
                }

                // succeeded allocation
                else if (Token::Match(tok3, "if ( %var% . %varid% ) {", structmemberid)) {
                    // goto the ")"
                    tok3 = tok3->next()->link();

                    // check if the variable is deallocated or returned..
                    unsigned int indentlevel4 = 0;
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
                        !Token::Match(tok3, "return & %varid% .", structid)) {
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
                    if (ignoredFunctions.find(tok3->str()) != ignoredFunctions.end())
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



#include "checkuninitvar.h" // CheckUninitVar::analyse

void CheckMemoryLeakNoVar::check()
{
    std::set<std::string> uvarFunctions;
    {
        const CheckUninitVar c(_tokenizer, _settings, _errorLogger);
        c.analyseFunctions(_tokenizer, uvarFunctions);
    }

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // only check functions
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        // Checks if a call to an allocation function like malloc() is made and its return value is not assigned.
        checkForUnusedReturnValue(scope);

        // Checks to see if a function is called with memory allocated for an argument that
        // could be leaked if a function called for another argument throws.
        checkForUnsafeArgAlloc(scope);

        // goto the "}" that ends the executable scope..
        const Token *tok = scope->classEnd;

        // parse the executable scope until tok is reached...
        for (const Token *tok2 = tok->link(); tok2 && tok2 != tok; tok2 = tok2->next()) {
            // allocating memory in parameter for function call..
            if (Token::Match(tok2, "[(,] %name% (") && Token::Match(tok2->linkAt(2), ") [,)]")) {
                const AllocType allocType = getAllocationType(tok2->next(), 0);
                if (allocType != No) {
                    // locate outer function call..
                    for (const Token *tok3 = tok2; tok3; tok3 = tok3->previous()) {
                        if (tok3->str() == "(") {
                            // Is it a function call..
                            if (!Token::Match(tok3->tokAt(-2), "= %name% (")) {
                                const std::string& functionName = tok3->strAt(-1);
                                if ((tokenizer->isCPP() && functionName == "delete") ||
                                    functionName == "free" ||
                                    functionName == "fclose" ||
                                    functionName == "realloc")
                                    break;
                                if (CheckMemoryLeakInFunction::test_white_list(functionName, _settings, tokenizer->isCPP())) {
                                    functionCallLeak(tok2, tok2->strAt(1), functionName);
                                    break;
                                }
                                if (uvarFunctions.find(functionName) != uvarFunctions.end()) {
                                    functionCallLeak(tok2, tok2->strAt(1), functionName);
                                    break;
                                }
                            }
                            break;
                        } else if (tok3->str() == ")")
                            tok3 = tok3->link();
                        else if (Token::Match(tok3, "[;{}]"))
                            break;
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
// Checks if a call to an allocation function like malloc() is made and its return value is not assigned.
//---------------------------------------------------------------------------
void CheckMemoryLeakNoVar::checkForUnusedReturnValue(const Scope *scope)
{
    for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
        if (Token::Match(tok, "%name% (") && (!tok->next()->astParent() || tok->next()->astParent()->str() == "!" || tok->next()->astParent()->isComparisonOp()) && tok->next()->astOperand1() == tok) {
            const AllocType allocType = getAllocationType(tok, 0);
            if (allocType != No)
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
    if (!_tokenizer->isCPP() || !_settings->inconclusive || !_settings->isEnabled("warning"))
        return;

    for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
        if (Token::Match(tok, "%name% (")) {
            const Token *endParamToken = tok->next()->link();
            std::string pointerType;
            std::string objectType;
            std::string functionCalled;

            // Scan through the arguments to the function call
            for (const Token *tok2 = tok->tokAt(2); tok2 && tok2 != endParamToken; tok2 = tok2->nextArgument()) {
                const Function *func = tok2->function();
                const bool isNothrow = func && (func->isAttributeNothrow() || func->isThrow());

                if (Token::Match(tok2, "shared_ptr|unique_ptr < %name% > ( new %name%")) {
                    pointerType = tok2->str();
                    objectType = tok2->strAt(6);
                } else if (!isNothrow) {
                    if (Token::Match(tok2, "%name% ("))
                        functionCalled = tok2->str();
                    else if (Token::Match(tok2, "%name% < %name% > ("))
                        functionCalled = tok2->str() + "<" + tok2->strAt(2) + ">";
                }
            }

            if (!pointerType.empty() && !functionCalled.empty())
                unsafeArgAllocError(tok, functionCalled, pointerType, objectType);
        }
    }
}

void CheckMemoryLeakNoVar::functionCallLeak(const Token *loc, const std::string &alloc, const std::string &functionCall)
{
    reportError(loc, Severity::error, "leakNoVarFunctionCall", "Allocation with " + alloc + ", " + functionCall + " doesn't release it.");
}

void CheckMemoryLeakNoVar::returnValueNotUsedError(const Token *tok, const std::string &alloc)
{
    reportError(tok, Severity::error, "leakReturnValNotUsed", "Return value of allocation function " + alloc + " is not stored.");
}

void CheckMemoryLeakNoVar::unsafeArgAllocError(const Token *tok, const std::string &funcName, const std::string &ptrType, const std::string& objType)
{
    const std::string factoryFunc = ptrType == "shared_ptr" ? "make_shared" : "make_unique";
    reportError(tok, Severity::warning, "leakUnsafeArgAlloc",
                "Unsafe allocation. If " + funcName + "() throws, memory could be leaked. Use " + factoryFunc + "<" + objType + ">() instead.",
                0U,
                true); // Inconclusive because funcName may never throw
}
