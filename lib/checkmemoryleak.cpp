/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include "mathlib.h"
#include "tokenize.h"
#include "executionpath.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <iostream>
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

/** List of functions that can be ignored when searching for memory leaks.
 * These functions don't take the address of the given pointer
 * This list needs to be alphabetically sorted so we can run bsearch on it.
 * This list contains function names with const parameters e.g.: atof(const char *)
 * Reference: http://www.aquaphoenix.com/ref/gnu_c_library/libc_492.html#SEC492
 */
static const char * const call_func_white_list[] = {
    "_open", "_wopen", "access", "adjtime", "asctime", "asctime_r", "asprintf", "assert"
    , "atof", "atoi", "atol", "chdir", "chmod", "chown"
    , "clearerr", "creat", "ctime", "ctime_r", "delete", "execl", "execle"
    , "execlp", "execv", "execve", "fchmod", "fclose", "fcntl"
    , "fdatasync", "feof", "ferror", "fflush", "fgetc", "fgetpos", "fgets"
    , "flock", "fmemopen", "fnmatch", "fopen", "fopencookie", "for", "fprintf", "fputc", "fputs", "fread", "free"
    , "freopen", "fscanf", "fseek", "fseeko", "fsetpos", "fstat", "fsync", "ftell", "ftello"
    , "ftruncate", "fwrite", "getc", "getenv","getgrnam", "gethostbyaddr", "gethostbyname", "getnetbyname"
    , "getopt", "getopt_long", "getprotobyname", "getpwnam", "gets", "getservbyname", "getservbyport"
    , "glob", "gmtime", "gmtime_r", "if", "index", "inet_addr", "inet_aton", "inet_network", "initgroups", "ioctl"
    , "link", "localtime", "localtime_r"
    , "lockf", "lseek", "lstat", "mblen", "mbstowcs", "mbtowc", "memchr", "memcmp", "memcpy", "memmove", "memset"
    , "mkdir", "mkfifo", "mknod"
    , "obstack_printf", "obstack_vprintf", "open", "opendir", "parse_printf_format", "pathconf"
    , "perror", "popen" ,"posix_fadvise", "posix_fallocate", "pread"
    , "printf", "psignal", "putenv", "puts", "pwrite", "qsort", "read", "readahead", "readdir", "readdir_r"
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

static int call_func_white_list_compare(const void *a, const void *b)
{
    return strcmp((const char *)a, *(const char **)b);
}

//---------------------------------------------------------------------------

bool CheckMemoryLeak::isclass(const Tokenizer *_tokenizer, const Token *tok, unsigned int varid) const
{
    if (tok->isStandardType())
        return false;

    const Variable * var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(varid);

    // return false if the type is a simple record type without side effects
    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check base class for side effects */
    /** @todo false negative: check constructors for side effects */
    if (var && var->type() && var->type()->numConstructors == 0 &&
        (var->type()->varlist.empty() || var->type()->needInitialization == Scope::True) &&
        var->type()->derivedFrom.empty())
        return false;

    return true;
}
//---------------------------------------------------------------------------

CheckMemoryLeak::AllocType CheckMemoryLeak::getAllocationType(const Token *tok2, unsigned int varid, std::list<const Token *> *callstack) const
{
    // What we may have...
    //     * var = (char *)malloc(10);
    //     * var = new char[10];
    //     * var = strdup("hello");
    //     * var = strndup("hello", 3);
    if (tok2 && tok2->str() == "(") {
        tok2 = tok2->link();
        tok2 = tok2 ? tok2->next() : NULL;
    }
    if (! tok2)
        return No;
    if (! tok2->isName())
        return No;

    // Does tok2 point on "malloc", "strdup" or "kmalloc"..
    static const char * const mallocfunc[] = {"malloc",
            "calloc",
            "strdup",
            "strndup",
            "kmalloc",
            "kzalloc",
            "kcalloc",
            0
                                             };
    for (unsigned int i = 0; mallocfunc[i]; i++) {
        if (tok2->str() == mallocfunc[i])
            return Malloc;
    }

    // Using realloc..
    if (varid && Token::Match(tok2, "realloc ( %any% ,") && tok2->tokAt(2)->varId() != varid)
        return Malloc;

    // Does tok2 point on "g_malloc", "g_strdup", ..
    static const char * const gmallocfunc[] = {"g_new",
            "g_new0",
            "g_try_new",
            "g_try_new0",
            "g_malloc",
            "g_malloc0",
            "g_try_malloc",
            "g_try_malloc0",
            "g_strdup",
            "g_strndup",
            "g_strdup_printf",
            0
                                              };
    for (unsigned int i = 0; gmallocfunc[i]; i++) {
        if (tok2->str() == gmallocfunc[i])
            return gMalloc;
    }

    if (Token::Match(tok2, "new struct| %type% [;()]") ||
        Token::Match(tok2, "new ( std :: nothrow ) struct| %type% [;()]") ||
        Token::Match(tok2, "new ( nothrow ) struct| %type% [;()]"))
        return New;

    if (Token::Match(tok2, "new struct| %type% [") ||
        Token::Match(tok2, "new ( std :: nothrow ) struct| %type% [") ||
        Token::Match(tok2, "new ( nothrow ) struct| %type% ["))
        return NewArray;

    if (Token::Match(tok2, "fopen|tmpfile|g_fopen ("))
        return File;

    if (Token::Match(tok2, "open|openat|creat|mkstemp|mkostemp (")) {
        // is there a user function with this name?
        if (tokenizer && Token::findmatch(tokenizer->tokens(), ("%type% *|&| " + tok2->str()).c_str()))
            return No;
        return Fd;
    }

    if (Token::simpleMatch(tok2, "popen ("))
        return Pipe;

    if (Token::Match(tok2, "opendir|fdopendir ("))
        return Dir;

    // User function
    const Token *ftok = tokenizer->getFunctionTokenByName(tok2->str().c_str());
    if (ftok == NULL)
        return No;

    // Prevent recursion
    if (callstack && std::find(callstack->begin(), callstack->end(), ftok) != callstack->end())
        return No;

    std::list<const Token *> cs;
    if (!callstack)
        callstack = &cs;

    callstack->push_back(ftok);
    return functionReturnType(ftok, callstack);
}




CheckMemoryLeak::AllocType CheckMemoryLeak::getReallocationType(const Token *tok2, unsigned int varid) const
{
    // What we may have...
    //     * var = (char *)realloc(..;
    if (tok2 && tok2->str() == "(") {
        tok2 = tok2->link();
        tok2 = tok2 ? tok2->next() : NULL;
    }
    if (! tok2)
        return No;

    if (varid > 0 && ! Token::Match(tok2, "%var% ( %varid% [,)]", varid))
        return No;

    if (tok2->str() == "realloc")
        return Malloc;

    // GTK memory reallocation..
    if (Token::Match(tok2, "g_realloc|g_try_realloc|g_renew|g_try_renew"))
        return gMalloc;

    return No;
}


CheckMemoryLeak::AllocType CheckMemoryLeak::getDeallocationType(const Token *tok, unsigned int varid) const
{
    if (Token::Match(tok, "delete %varid% ;", varid))
        return New;

    if (Token::Match(tok, "delete [ ] %varid% ;", varid))
        return NewArray;

    if (Token::Match(tok, "delete ( %varid% ) ;", varid))
        return New;

    if (Token::Match(tok, "delete [ ] ( %varid% ) ;", varid))
        return NewArray;

    if (Token::Match(tok, "free|kfree ( %varid% ) ;", varid) ||
        Token::Match(tok, "free|kfree ( %varid% -", varid) ||
        Token::Match(tok, "realloc ( %varid% , 0 ) ;", varid))
        return Malloc;

    if (Token::Match(tok, "g_free ( %varid% ) ;", varid) ||
        Token::Match(tok, "g_free ( %varid% -", varid))
        return gMalloc;

    if (Token::Match(tok, "fclose ( %varid% )", varid) ||
        Token::simpleMatch(tok, "fcloseall ( )"))
        return File;

    if (Token::Match(tok, "close ( %varid% )", varid))
        return Fd;

    if (Token::Match(tok, "pclose ( %varid% )", varid))
        return Pipe;

    if (Token::Match(tok, "closedir ( %varid% )", varid))
        return Dir;

    return No;
}

CheckMemoryLeak::AllocType CheckMemoryLeak::getDeallocationType(const Token *tok, const std::string &varname) const
{
    if (Token::Match(tok, std::string("delete " + varname + " [,;]").c_str()))
        return New;

    if (Token::Match(tok, std::string("delete [ ] " + varname + " [,;]").c_str()))
        return NewArray;

    if (Token::Match(tok, std::string("delete ( " + varname + " ) [,;]").c_str()))
        return New;

    if (Token::Match(tok, std::string("delete [ ] ( " + varname + " ) [,;]").c_str()))
        return NewArray;

    if (Token::simpleMatch(tok, std::string("free ( " + varname + " ) ;").c_str()) ||
        Token::simpleMatch(tok, std::string("kfree ( " + varname + " ) ;").c_str()) ||
        Token::simpleMatch(tok, std::string("realloc ( " + varname + " , 0 ) ;").c_str()))
        return Malloc;

    if (Token::simpleMatch(tok, std::string("g_free ( " + varname + " ) ;").c_str()))
        return gMalloc;

    if (Token::simpleMatch(tok, std::string("fclose ( " + varname + " )").c_str()) ||
        Token::simpleMatch(tok, "fcloseall ( )"))
        return File;

    if (Token::simpleMatch(tok, std::string("close ( " + varname + " )").c_str()))
        return Fd;

    if (Token::simpleMatch(tok, std::string("pclose ( " + varname + " )").c_str()))
        return Pipe;

    if (Token::simpleMatch(tok, std::string("closedir ( " + varname + " )").c_str()))
        return Dir;

    return No;
}

//--------------------------------------------------------------------------


//--------------------------------------------------------------------------

void CheckMemoryLeak::memoryLeak(const Token *tok, const std::string &varname, AllocType alloctype)
{
    if (alloctype == CheckMemoryLeak::File ||
        alloctype == CheckMemoryLeak::Pipe ||
        alloctype == CheckMemoryLeak::Fd   ||
        alloctype == CheckMemoryLeak::Dir)
        resourceLeakError(tok, varname.c_str());
    else
        memleakError(tok, varname.c_str());
}
//---------------------------------------------------------------------------


void CheckMemoryLeak::reportErr(const Token *tok, Severity::SeverityType severity, const std::string &id, const std::string &msg) const
{
    std::list<const Token *> callstack;

    if (tok)
        callstack.push_back(tok);

    reportErr(callstack, severity, id, msg);
}

void CheckMemoryLeak::reportErr(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg) const
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locations;

    for (std::list<const Token *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it) {
        const Token * const tok = *it;

        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(tokenizer->file(tok));

        locations.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locations, severity, msg, id, false);

    if (errorLogger)
        errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void CheckMemoryLeak::memleakError(const Token *tok, const std::string &varname)
{
    reportErr(tok, Severity::error, "memleak", "Memory leak: " + varname);
}

void CheckMemoryLeak::memleakUponReallocFailureError(const Token *tok, const std::string &varname)
{
    reportErr(tok, Severity::error, "memleakOnRealloc", "Common realloc mistake: \'" + varname + "\' nulled but not freed upon failure");
}

void CheckMemoryLeak::resourceLeakError(const Token *tok, const std::string &varname)
{
    std::string errmsg("Resource leak");
    if (!varname.empty())
        errmsg += ": " + varname;
    reportErr(tok, Severity::error, "resourceLeak", errmsg);
}

void CheckMemoryLeak::deallocDeallocError(const Token *tok, const std::string &varname)
{
    reportErr(tok, Severity::error, "deallocDealloc", "Deallocating a deallocated pointer: " + varname);
}

void CheckMemoryLeak::deallocuseError(const Token *tok, const std::string &varname)
{
    reportErr(tok, Severity::error, "deallocuse", "Dereferencing '" + varname + "' after it is deallocated / released");
}

void CheckMemoryLeak::mismatchSizeError(const Token *tok, const std::string &sz)
{
    reportErr(tok, Severity::error, "mismatchSize", "The given size " + sz + " is mismatching");
}

void CheckMemoryLeak::mismatchAllocDealloc(const std::list<const Token *> &callstack, const std::string &varname)
{
    reportErr(callstack, Severity::error, "mismatchAllocDealloc", "Mismatching allocation and deallocation: " + varname);
}

CheckMemoryLeak::AllocType CheckMemoryLeak::functionReturnType(const Token *tok, std::list<const Token *> *callstack) const
{
    if (!tok)
        return No;

    // Locate start of function
    while (tok) {
        if (tok->str() == "{" || tok->str() == "}")
            return No;

        if (tok->str() == "(") {
            tok = tok->link();
            break;
        }

        tok = tok->next();
    }

    // Is this the start of a function?
    if (!Token::Match(tok, ") const| {"))
        return No;

    while (tok->str() != "{")
        tok = tok->next();

    // Get return pointer..
    unsigned int varid = 0;
    unsigned int indentlevel = 0;
    for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
        if (tok2->str() == "{")
            ++indentlevel;
        else if (tok2->str() == "}") {
            if (indentlevel <= 1)
                return No;
            --indentlevel;
        }
        if (Token::Match(tok2, "return %var% ;")) {
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

    // Check if return pointer is allocated..
    AllocType allocType = No;
    while (0 != (tok = tok->next())) {
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
        if (tok->str() == "return")
            return allocType;
    }

    return allocType;
}


const char *CheckMemoryLeak::functionArgAlloc(const Token *tok, unsigned int targetpar, AllocType &allocType) const
{
    // Find the varid of targetpar, then locate the start of the function..
    unsigned int parlevel = 0;
    unsigned int par = 0;
    unsigned int varid = 0;

    allocType = No;

    while (tok) {
        if (tok->str() == "{" || tok->str() == "}")
            return "";

        if (tok->str() == "(") {
            if (parlevel != 0)
                return "";
            ++parlevel;
            ++par;
        }

        else if (tok->str() == ")") {
            if (parlevel != 1)
                return "";
            break;
        }

        else if (parlevel == 1 && tok->str() == ",") {
            ++par;
        }

        tok = tok->next();

        if (parlevel == 1 && par == targetpar && Token::Match(tok, "%type% * * %var%")) {
            varid = tok->tokAt(3)->varId();
        }
    }

    if (varid == 0)
        return "";

    // Is this the start of a function?
    if (!Token::Match(tok, ") const| {"))
        return "";

    while (tok->str() != "{")
        tok = tok->next();

    // Check if pointer is allocated.
    unsigned int indentlevel = 0;
    int realloc = 0;
    while (0 != (tok = tok->next())) {
        if (tok->str() == "{")
            ++indentlevel;
        else if (tok->str() == "}") {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        } else if (tok->varId() == varid) {
            if (Token::Match(tok->tokAt(-3), "free ( * %varid% )", varid)) {
                realloc = 1;
                allocType = No;
            } else if (Token::Match(tok->previous(), "* %varid% =", varid)) {
                allocType = getAllocationType(tok->tokAt(2), varid);
                if (allocType == No) {
                    allocType = getReallocationType(tok->tokAt(2), varid);
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


void CheckMemoryLeakInFunction::parse_noreturn()
{
    noreturn.insert("exit");
    noreturn.insert("_exit");
    noreturn.insert("_Exit");
    noreturn.insert("abort");
    noreturn.insert("err");
    noreturn.insert("verr");
    noreturn.insert("errx");
    noreturn.insert("verrx");
    noreturn.insert("ExitProcess");
    noreturn.insert("ExitThread");
    noreturn.insert("pthread_exit");

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // parse this function to check if it contains an "exit" call..
        unsigned int indentlevel = 1;
        for (const Token *tok2 = scope->classStart->next(); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{")
                ++indentlevel;
            else if (tok2->str() == "}") {
                --indentlevel;
                if (indentlevel == 0)
                    break;
            }
            if (Token::Match(tok2->previous(), "[;{}] exit (")) {
                noreturn.insert(scope->className);
                break;
            }
        }

        // This function is not a noreturn function
        if (indentlevel == 0) {
            notnoreturn.insert(scope->className);
        }
    }
}


bool CheckMemoryLeakInFunction::notvar(const Token *tok, unsigned int varid, bool endpar) const
{
    const std::string end(endpar ? " &&|)" : " [;)&|]");
    return bool(Token::Match(tok, ("! %varid%" + end).c_str(), varid) ||
                Token::Match(tok, ("! ( %varid% )" + end).c_str(), varid));
}


static int countParameters(const Token *tok)
{
    if (!Token::Match(tok, "%var% ("))
        return -1;
    if (Token::Match(tok->tokAt(2), "void| )"))
        return 0;

    int numpar = 1;
    int parlevel = 0;
    for (; tok; tok = tok->next()) {
        if (tok->str() == "(")
            ++parlevel;

        else if (tok->str() == ")") {
            if (parlevel <= 1)
                return numpar;
            --parlevel;
        }

        else if (parlevel == 1 && tok->str() == ",") {
            ++numpar;
        }
    }

    return -1;
}

bool CheckMemoryLeakInFunction::test_white_list(const std::string &funcname)
{
    return (std::bsearch(funcname.c_str(), call_func_white_list,
                         sizeof(call_func_white_list) / sizeof(call_func_white_list[0]),
                         sizeof(call_func_white_list[0]), call_func_white_list_compare) != NULL);
}

const char * CheckMemoryLeakInFunction::call_func(const Token *tok, std::list<const Token *> callstack, const unsigned int varid, AllocType &alloctype, AllocType &dealloctype, bool &allocpar, unsigned int sz)
{
    if (test_white_list(tok->str())) {
        if (tok->str() == "asprintf" ||
            tok->str() == "delete" ||
            tok->str() == "fclose" ||
            tok->str() == "for" ||
            tok->str() == "free" ||
            tok->str() == "if" ||
            tok->str() == "realloc" ||
            tok->str() == "return" ||
            tok->str() == "switch" ||
            tok->str() == "while") {
            return 0;
        }

        // is the varid a parameter?
        for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(" || tok2->str() == ")")
                break;
            if (tok2->varId() == varid) {
                if (tok->strAt(-1) == ".")
                    return "use";
                else if (tok2->strAt(1) == "=")
                    return "assign";
                else
                    return"use_";
            }
        }

        return 0;
    }

    if (noreturn.find(tok->str()) != noreturn.end() && tok->strAt(-1) != "=")
        return "exit";

    if (varid > 0 && (getAllocationType(tok, varid) != No || getReallocationType(tok, varid) != No || getDeallocationType(tok, varid) != No))
        return 0;

    if (callstack.size() > 2)
        return "dealloc_";

    const std::string funcname(tok->str());
    for (std::list<const Token *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it) {
        if ((*it) && (*it)->str() == funcname)
            return "recursive";
    }
    callstack.push_back(tok);

    // lock/unlock..
    if (varid == 0) {
        const Token *ftok = _tokenizer->getFunctionTokenByName(funcname.c_str());
        while (ftok && (ftok->str() != "{"))
            ftok = ftok->next();
        if (!ftok)
            return 0;

        Token *func = getcode(ftok->tokAt(1), callstack, 0, alloctype, dealloctype, false, 1);
        simplifycode(func);
        const char *ret = 0;
        if (Token::simpleMatch(func, "; alloc ; }"))
            ret = "alloc";
        else if (Token::simpleMatch(func, "; dealloc ; }"))
            ret = "dealloc";
        Tokenizer::deleteTokens(func);
        return ret;
    }

    // how many parameters is there in the function call?
    int numpar = countParameters(tok);
    if (numpar <= 0) {
        // Taking return value => it is not a noreturn function
        if (tok->strAt(-1) == "=")
            return NULL;

        // Function is not noreturn
        if (notnoreturn.find(funcname) != notnoreturn.end())
            return NULL;

        return (tok->previous()->str() != "=") ? "callfunc" : NULL;
    }

    unsigned int par = 1;
    unsigned int parlevel = 0;

    const bool dot(tok->previous()->str() == ".");
    const bool eq(tok->previous()->str() == "=");

    for (; tok; tok = tok->next()) {
        if (tok->str() == "(")
            ++parlevel;
        else if (tok->str() == ")") {
            --parlevel;
            if (parlevel < 1) {
                return (eq || _settings->experimental) ? 0 : "callfunc";
            }
        }

        if (parlevel == 1) {
            if (tok->str() == ",")
                ++par;
            if (varid > 0 && Token::Match(tok, "[,()] %varid% [,()]", varid)) {
                if (dot)
                    return "use";

                const Token *ftok = _tokenizer->getFunctionTokenByName(funcname.c_str());
                if (!ftok)
                    return "use";

                // how many parameters does the function want?
                if (numpar != countParameters(ftok))
                    return "recursive";

                const char *parname = Tokenizer::getParameterName(ftok, par);
                if (! parname)
                    return "recursive";
                unsigned int parameterVarid = 0;
                {
                    const Token *partok = Token::findmatch(ftok, parname);
                    if (partok)
                        parameterVarid = partok->varId();
                }
                if (parameterVarid == 0)
                    return "recursive";
                // Check if the function deallocates the variable..
                while (ftok && (ftok->str() != "{"))
                    ftok = ftok->next();
                if (!ftok)
                    return 0;
                Token *func = getcode(ftok->tokAt(1), callstack, parameterVarid, alloctype, dealloctype, false, sz);
                //simplifycode(func, all);
                const Token *func_ = func;
                while (func_ && func_->str() == ";")
                    func_ = func_->next();

                const char *ret = 0;
                /** @todo handle "goto" */
                if (Token::findmatch(func_, "dealloc"))
                    ret = "dealloc";
                else if (Token::findmatch(func_, "use"))
                    ret = "use";
                else if (Token::findmatch(func_, "&use"))
                    ret = "&use";

                Tokenizer::deleteTokens(func);
                return ret;
            }
            if (varid > 0 && Token::Match(tok, "[,()] & %varid% [,()]", varid)) {
                const Token *ftok = _tokenizer->getFunctionTokenByName(funcname.c_str());
                AllocType a;
                const char *ret = functionArgAlloc(ftok, par, a);

                if (a != No) {
                    if (alloctype == No)
                        alloctype = a;
                    else if (alloctype != a)
                        alloctype = Many;
                    allocpar = true;
                    return ret;
                }
            }
            if (varid > 0 && Token::Match(tok, "[(,] %varid% . %var% [,)]", varid))
                return "use";
        }
    }
    return NULL;
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
    Token *rethead = 0, *rettail = 0;

    // variables whose value depends on if(!var). If one of these variables
    // is used in a if-condition then generate "ifv" instead of "if".
    std::set<unsigned int> extravar;

    // The first token should be ";"
    rethead = new Token(0);
    rethead->str(";");
    rethead->linenr(tok->linenr());
    rethead->fileIndex(tok->fileIndex());
    rettail = rethead;

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
        if (Token::Match(tok, "[;{}]") && varid > 0 && parlevel == 0) {
            if (Token::Match(tok->next(), "[{};]"))
                continue;

            // function calls are interesting..
            const Token *tok2 = tok;
            while (Token::Match(tok2->next(), "%var% ."))
                tok2 = tok2->tokAt(2);
            if (Token::Match(tok2->next(), "%var% ("))
                ;

            else if (Token::Match(tok->next(), "continue|break|return|throw|goto|do|else"))
                ;

            else {
                const Token *skipToToken = 0;

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
                tok = tok->tokAt(4)->link();
                continue;
            }

            if (Token::Match(tok->previous(), "[(;{}] %varid% =", varid) ||
                Token::Match(tok, "asprintf ( & %varid% ,", varid)) {
                CheckMemoryLeak::AllocType alloc;

                if (Token::simpleMatch(tok, "asprintf (")) {
                    // todo: check how the return value is used.
                    if (!Token::Match(tok->previous(), "[;{}]")) {
                        Tokenizer::deleteTokens(rethead);
                        return 0;
                    }
                    alloc = Malloc;
                    tok = tok->next()->link();
                } else {
                    alloc = getAllocationType(tok->tokAt(2), varid);
                }
                bool realloc = false;

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
                        realloc = true;
                        tok = tok->tokAt(2);
                        if (Token::Match(tok, "%var% ("))
                            tok = tok->next()->link();
                        continue;
                    }
                }

                // don't check classes..
                if (alloc == CheckMemoryLeak::New) {
                    if (Token::Match(tok->tokAt(2), "new struct| %type% [(;]")) {
                        const int offset = tok->strAt(3) == "struct" ? 1 : 0;
                        if (isclass(_tokenizer, tok->tokAt(3 + offset), varid)) {
                            alloc = No;
                        }
                    } else if (Token::Match(tok->tokAt(2), "new ( nothrow ) struct| %type%")) {
                        const int offset = tok->strAt(6) == "struct" ? 1 : 0;
                        if (isclass(_tokenizer, tok->tokAt(6 + offset), varid)) {
                            alloc = No;
                        }
                    } else if (Token::Match(tok->tokAt(2), "new ( std :: nothrow ) struct| %type%")) {
                        const int offset = tok->strAt(8) == "struct" ? 1 : 0;
                        if (isclass(_tokenizer, tok->tokAt(8 + offset), varid)) {
                            alloc = No;
                        }
                    }

                    if (alloc == No && alloctype == No)
                        alloctype = CheckMemoryLeak::New;
                }

                if (alloc != No) {
                    if (! realloc)
                        addtoken(&rettail, tok, "alloc");

                    if (alloctype != No && alloctype != alloc)
                        alloc = Many;

                    if (alloc != Many && dealloctype != No && dealloctype != Many && dealloctype != alloc) {
                        callstack.push_back(tok);
                        mismatchAllocDealloc(callstack, Token::findmatch(_tokenizer->tokens(), "%varid%", varid)->str());
                        callstack.pop_back();
                    }

                    alloctype = alloc;

                    if (Token::Match(tok, "%var% = %type% (")) {
                        tok = tok->tokAt(3)->link();
                        continue;
                    }
                }

                // assignment..
                else {
                    // is the pointer in rhs?
                    bool rhs = false;
                    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == ";") {
                            if (rhs)
                                tok = tok2;
                            break;
                        }

                        if (Token::Match(tok2, "[=+(,] %varid%", varid)) {
                            if (!rhs && Token::Match(tok2, "[(,]")) {
                                addtoken(&rettail, tok, "use");
                                addtoken(&rettail, tok, ";");
                            }
                            rhs = true;
                        }
                    }

                    if (!rhs)
                        addtoken(&rettail, tok, "assign");
                    continue;
                }
            }

            if (Token::Match(tok->previous(), "[;{})=|] %var%")) {
                AllocType dealloc = getDeallocationType(tok, varid);

                if (dealloc != No && tok->str() == "fcloseall" && alloctype != dealloc)
                    dealloc = No;

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
                        tok = tok->tokAt(2)->link();
                    continue;
                }
            }

            // if else switch
            if (Token::simpleMatch(tok, "if (")) {
                if (alloctype == Fd) {
                    if (Token::Match(tok, "if ( 0 <=|< %varid% )", varid) ||
                        Token::Match(tok, "if ( %varid% != -1 )", varid)) {
                        addtoken(&rettail, tok, "if(var)");
                        tok = tok->next()->link();
                        continue;
                    } else if (Token::Match(tok, "if ( %varid% == -1 )", varid) ||
                               Token::Match(tok, "if ( %varid% < 0 )", varid)) {
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
                    int innerParlevel = 0;
                    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == "(")
                            ++innerParlevel;
                        if (tok2->str() == ")") {
                            --innerParlevel;
                            if (innerParlevel <= 0)
                                break;
                        }
                        if (Token::Match(tok2, "close|pclose|fclose|closedir ( %varid% )", varid)) {
                            addtoken(&rettail, tok, "dealloc");
                            addtoken(&rettail, tok, ";");
                            dep = true;
                            break;
                        }
                        if (alloctype == Fd && Token::Match(tok2, "%varid% !=|>=", varid)) {
                            dep = true;
                        }
                        if (innerParlevel > 0 && Token::Match(tok2, "! %varid%", varid)) {
                            dep = true;
                        }
                        if (innerParlevel > 0 && Token::Match(tok2, "%var% (") && !test_white_list(tok2->str())) {
                            bool use = false;
                            for (const Token *tok3 = tok2->tokAt(2); tok3; tok3 = tok3->next()) {
                                if (tok3->str() == "(")
                                    tok3 = tok3->link();
                                else if (tok3->str() == ")")
                                    break;
                                else if (Token::Match(tok3->previous(), "(|, &| %varid% ,|)", varid)) {
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
                        }
                        if (tok2->varId() && extravar.find(tok2->varId()) != extravar.end()) {
                            dep = true;
                        }
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
            if (Token::simpleMatch(tok, "while ( true )") ||
                Token::simpleMatch(tok, "for ( ; ; )")) {
                addtoken(&rettail, tok, "while1");
                tok = tok->next()->link();
                continue;
            }

            else if (varid && getDeallocationType(tok->tokAt(2), varid) != No) {
                addtoken(&rettail, tok, "dealloc");
                addtoken(&rettail, tok, ";");
            }

            else if (alloctype == Fd && varid) {
                if (Token::Match(tok, "while ( 0 <= %varid% )", varid) ||
                    Token::Match(tok, "while ( %varid% != -1 )", varid)) {
                    addtoken(&rettail, tok, "while(var)");
                    tok = tok->next()->link();
                    continue;
                } else if (Token::Match(tok, "while ( %varid% == -1 )", varid) ||
                           Token::Match(tok, "while ( %varid% < 0 )", varid)) {
                    addtoken(&rettail, tok, "while(!var)");
                    tok = tok->next()->link();
                    continue;
                }
            }

            else if (varid && Token::Match(tok, "while ( %varid% )", varid)) {
                addtoken(&rettail, tok, "while(var)");
                tok = tok->next()->link();
                continue;
            } else if (varid && Token::simpleMatch(tok, "while (") && notvar(tok->tokAt(2), varid, true)) {
                addtoken(&rettail, tok, "while(!var)");
                tok = tok->next()->link();
                continue;
            }

            addtoken(&rettail, tok, "loop");

            if (varid > 0) {
                unsigned int parlevel2 = 0;
                for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(")
                        ++parlevel2;
                    else if (tok2->str() == ")") {
                        if (parlevel2 > 0)
                            --parlevel2;
                        else
                            break;
                    }
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
        if (tok->str() == "continue") {
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
                const Token *tok2 = tok->tokAt(5);
                while (tok2 && tok2->str() != ">")
                    tok2 = tok2->next();
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
                            Token::simpleMatch(tok2->next(), "[")) {
                        } else if (f.empty() ||
                                   !test_white_list(f.top()->str()) ||
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
        else if (Token::Match(tok, "try|throw|catch"))
            addtoken(&rettail, tok, tok->str());

        // Assignment..
        if (varid) {
            if (Token::simpleMatch(tok, "= {")) {
                unsigned int indentlevel2 = 0;
                bool use = false;
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "{")
                        ++indentlevel2;
                    else if (tok2->str() == "}") {
                        if (indentlevel2 <= 1)
                            break;
                        --indentlevel2;
                    } else if (tok2->varId() == varid) {
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

            if (Token::Match(tok, "[)=] %varid% [+;)]", varid) ||
                Token::Match(tok, "%var% + %varid%", varid) ||
                Token::Match(tok, "%varid% +=|-=", varid) ||
                Token::Match(tok, "+=|<< %varid% ;", varid) ||
                Token::Match(tok, "= strcpy|strcat|memmove|memcpy ( %varid% ,", varid) ||
                Token::Match(tok, "[;{}] %var% [ %varid% ]", varid)) {
                addtoken(&rettail, tok, "use");
            } else if (Token::Match(tok->previous(), ";|{|}|=|(|,|%op% %varid% [", varid)) {
                // warning is written for "dealloc ; use_ ;".
                // but this use doesn't affect the leak-checking
                addtoken(&rettail, tok, "use_");
            }
        }

        // Investigate function calls..
        if (Token::Match(tok, "%var% (")) {
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
                if (noreturn.find(tok->str()) != noreturn.end())
                    addtoken(&rettail, tok, "exit");

                else if (!test_white_list(tok->str())) {
                    int innerParlevel = 1;
                    for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == "(")
                            ++innerParlevel;
                        else if (tok2->str() == ")") {
                            --innerParlevel;
                            if (innerParlevel <= 0)
                                break;
                        }
                        if (tok2->varId() == varid) {
                            addtoken(&rettail, tok, "::use");
                            break;
                        }
                    }
                }
            }

            else {
                if (varid > 0 && Token::Match(tok, "%var% ( close|fclose|pclose ( %varid% ) ) ;", varid)) {
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
        if (Token::Match(tok, "( *| %var%") && Token::simpleMatch(tok->link(),") (")) {
            const Token *tok2 = tok->next();
            if (tok2->str() == "*")
                tok2 = tok2->next();
            tok2 = tok2->next();

            while (Token::Match(tok2, ". %var%"))
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
        if (varid > 0 && Token::Match(tok, "[=(,] & %varid% [.[,)]", varid)) {
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






void CheckMemoryLeakInFunction::simplifycode(Token *tok)
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

    // Insert extra ";"
    for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
        if (!tok2->previous() || Token::Match(tok2->previous(), "[;{}]")) {
            if (Token::Match(tok2, "assign|callfunc|use assign|callfunc|use")) {
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
            tok->deleteThis();
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
                        Token::eraseTokens(tok3, tok3->tokAt(6));
                        if (Token::simpleMatch(tok3->next(), "else"))
                            tok3->deleteNext();
                    }
                }

                if (Token::simpleMatch(tok2, "while1 { if { dealloc ; return ; } }")) {
                    tok2->str(";");
                    Token::eraseTokens(tok2, tok2->tokAt(4));
                    Token::eraseTokens(tok2->tokAt(4), tok2->tokAt(7));
                }
            }
        }

        // Main inner simplification loop
        for (Token *tok2 = tok; tok2; tok2 = tok2 ? tok2->next() : NULL) {
            // Delete extra ";"
            while (Token::Match(tok2, "[;{}] ;")) {
                tok2->deleteNext();
                done = false;
            }

            // Replace "{ }" with ";"
            if (Token::simpleMatch(tok2->next(), "{ }")) {
                tok2->eraseTokens(tok2, tok2->tokAt(3));
                tok2->insertToken(";");
                done = false;
            }

            // Delete braces around a single instruction..
            if (Token::Match(tok2->next(), "{ %var% ; }")) {
                tok2->deleteNext();
                Token::eraseTokens(tok2->tokAt(2), tok2->tokAt(4));
                done = false;
            }
            if (Token::Match(tok2->next(), "{ %var% %var% ; }")) {
                tok2->deleteNext();
                Token::eraseTokens(tok2->tokAt(3), tok2->tokAt(5));
                done = false;
            }

            // Reduce "if if|callfunc" => "if"
            else if (Token::Match(tok2, "if if|callfunc")) {
                tok2->deleteNext();
                done = false;
            }

            // outer/inner if blocks. Remove outer condition..
            else if (Token::Match(tok2->next(), "if|if(var) { if return use ; }")) {
                tok2->tokAt(6)->deleteNext();
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            else if (tok2->next() && tok2->next()->str() == "if") {
                // Delete empty if that is not followed by an else
                if (Token::Match(tok2->next(), "if ; !!else")) {
                    tok2->deleteNext();
                    done = false;
                }

                // Reduce "if X ; else X ;" => "X ;"
                else if (Token::Match(tok2->next(), "if %var% ; else %var% ;") &&
                         std::string(tok2->strAt(2)) == std::string(tok2->strAt(5))) {
                    Token::eraseTokens(tok2, tok2->tokAt(5));
                    done = false;
                }

                // Reduce "if continue ; if continue ;" => "if continue ;"
                else if (Token::simpleMatch(tok2->next(), "if continue ; if continue ;")) {
                    Token::eraseTokens(tok2, tok2->tokAt(4));
                    done = false;
                }

                // Reduce "if return ; alloc ;" => "alloc ;"
                else if (Token::Match(tok2, "[;{}] if return ; alloc|return ;")) {
                    Token::eraseTokens(tok2, tok2->tokAt(4));
                    done = false;
                }

                // "[;{}] if alloc ; else return ;" => "[;{}] alloc ;"
                else if (Token::Match(tok2, "[;{}] if alloc ; else return ;")) {
                    tok2->deleteNext();                                // Remove "if"
                    Token::eraseTokens(tok2->next(), tok2->tokAt(5));  // Remove "; else return"
                    done = false;
                }

                // Reduce "if ; else %var% ;" => "if %var% ;"
                else if (Token::Match(tok2->next(), "if ; else %var% ;")) {
                    Token::eraseTokens(tok2->next(), tok2->tokAt(4));
                    done = false;
                }

                // Reduce "if ; else" => "if"
                else if (Token::simpleMatch(tok2->next(), "if ; else")) {
                    Token::eraseTokens(tok2->next(), tok2->tokAt(4));
                    done = false;
                }

                // Reduce "if return ; else|if return|continue ;" => "if return ;"
                else if (Token::Match(tok2->next(), "if return ; else|if return|continue|break ;")) {
                    Token::eraseTokens(tok2->tokAt(3), tok2->tokAt(6));
                    done = false;
                }

                // Reduce "if continue|break ; else|if return ;" => "if return ;"
                else if (Token::Match(tok2->next(), "if continue|break ; if|else return ;")) {
                    Token::eraseTokens(tok2->next(), tok2->tokAt(5));
                    done = false;
                }

                // Remove "else" after "if continue|break|return"
                else if (Token::Match(tok2->next(), "if continue|break|return ; else")) {
                    tok2->tokAt(4)->deleteThis();
                    done = false;
                }

                // Delete "if { dealloc|assign|use ; return ; }"
                else if (Token::Match(tok2, "[;{}] if { dealloc|assign|use ; return ; }")) {
                    Token::eraseTokens(tok2, tok2->tokAt(8));
                    if (Token::simpleMatch(tok2->next(), "else"))
                        tok2->deleteNext();
                    done = false;
                }

                // Remove "if { dealloc ; callfunc ; } !!else|return"
                else if (Token::Match(tok2->next(), "if { dealloc|assign ; callfunc ; }") &&
                         !Token::Match(tok2->tokAt(8), "else|return")) {
                    Token::eraseTokens(tok2, tok2->tokAt(8));
                    done = false;
                }

                continue;
            }

            // Reduce "alloc while(!var) alloc ;" => "alloc ;"
            if (Token::Match(tok2, "[;{}] alloc ; while(!var) alloc ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(4));
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
                Token::eraseTokens(tok2, tok2->tokAt(2));
                done = false;
            }

            // Reduce "; if(!var) alloc ; !!else" => "; dealloc ; alloc ;"
            if (Token::Match(tok2, "; if(!var) alloc ; !!else")) {
                // Remove the "if(!var)"
                Token::eraseTokens(tok2, tok2->tokAt(2));

                // Insert "dealloc ;" before the "alloc ;"
                tok2->insertToken(";");
                tok2->insertToken("dealloc");

                done = false;
            }

            // Reduce "; if(!var) exit ;" => ";"
            if (Token::Match(tok2, "; if(!var) exit ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            // Reduce "if* ;"..
            if (Token::Match(tok2->next(), "if(var)|if(!var)|ifv ;")) {
                // Followed by else..
                if (Token::simpleMatch(tok2->tokAt(3), "else")) {
                    tok2 = tok2->next();
                    if (tok2->str() == "if(var)")
                        tok2->str("if(!var)");
                    else if (tok2->str() == "if(!var)")
                        tok2->str("if(var)");

                    // remove the "; else"
                    Token::eraseTokens(tok2, tok2->tokAt(3));
                } else {
                    // remove the "if*"
                    Token::eraseTokens(tok2, tok2->tokAt(2));
                }
                done = false;
            }

            // Reduce "else ;" => ";"
            if (Token::simpleMatch(tok2->next(), "else ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(2));
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
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            // Delete if block: "alloc; if return use ;"
            if (Token::Match(tok2, "alloc ; if return use ; !!else")) {
                Token::eraseTokens(tok2, tok2->tokAt(5));
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
                tok2->deleteThis();
                tok2->deleteThis();
                done = false;
            }

            // Remove the "if break|continue ;" that follows "dealloc ; alloc ;"
            if (! _settings->experimental && Token::Match(tok2, "dealloc ; alloc ; if break|continue ;")) {
                tok2 = tok2->tokAt(3);
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            // if break ; break ; => break ;
            if (Token::Match(tok2->previous(), "[;{}] if break ; break ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(4));
                done = false;
            }

            // Reduce "do { dealloc ; alloc ; } while(var) ;" => ";"
            if (Token::simpleMatch(tok2->next(), "do { dealloc ; alloc ; } while(var) ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(9));
                done = false;
            }

            // Reduce "do { alloc ; } " => "alloc ;"
            /** @todo If the loop "do { alloc ; }" can be executed twice, reduce it to "loop alloc ;" */
            if (Token::simpleMatch(tok2->next(), "do { alloc ; }")) {
                Token::eraseTokens(tok2, tok2->tokAt(3));
                Token::eraseTokens(tok2->tokAt(2), tok2->tokAt(4));
                done = false;
            }

            // Reduce "loop break ; => ";"
            if (Token::Match(tok2->next(), "loop break|continue ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            // Reduce "loop|do ;" => ";"
            if (Token::Match(tok2, "loop|do ;")) {
                tok2->deleteThis();
                done = false;
            }

            // Reduce "loop if break|continue ; !!else" => ";"
            if (Token::Match(tok2->next(), "loop if break|continue ; !!else")) {
                Token::eraseTokens(tok2, tok2->tokAt(4));
                done = false;
            }

            // Reduce "loop { if break|continue ; !!else" => "loop {"
            if (Token::Match(tok2, "loop { if break|continue ; !!else")) {
                Token::eraseTokens(tok2->next(), tok2->tokAt(5));
                done = false;
            }

            // Replace "do ; loop ;" with ";"
            if (Token::simpleMatch(tok2, "; loop ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            // Replace "loop loop .." with "loop .."
            if (Token::simpleMatch(tok2, "loop loop")) {
                tok2->deleteThis();
                done = false;
            }

            // Replace "loop if return ;" with "if return ;"
            if (Token::simpleMatch(tok2->next(), "loop if return")) {
                Token::eraseTokens(tok2, tok2->tokAt(2));
                done = false;
            }

            // Reduce "loop|while1 { dealloc ; alloc ; }"
            if (Token::Match(tok2, "loop|while1 { dealloc ; alloc ; }")) {
                // delete "loop|while1"
                tok2->deleteThis();
                // delete "{"
                tok2->deleteThis();

                // delete "}"
                Token::eraseTokens(tok2->tokAt(3), tok2->tokAt(5));

                done = false;
            }

            // loop { use ; callfunc ; }  =>  use ;
            // assume that the "callfunc" is not noreturn
            if (Token::simpleMatch(tok2, "loop { use ; callfunc ; }")) {
                Token::eraseTokens(tok2, tok2->tokAt(7));
                tok2->str("use");
                tok2->insertToken(";");
                done = false;
            }

            // Delete if block in "alloc ; if(!var) return ;"
            if (Token::Match(tok2, "alloc ; if(!var) return ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(4));
                done = false;
            }

            // Reduce "[;{}] return use ; %var%" => "[;{}] return use ;"
            if (Token::Match(tok2, "[;{}] return use ; %var%")) {
                Token::eraseTokens(tok2->tokAt(3), tok2->tokAt(5));
                done = false;
            }

            // Reduce "if(var) return use ;" => "return use ;"
            if (Token::Match(tok2->next(), "if(var) return use ; !!else")) {
                Token::eraseTokens(tok2, tok2->tokAt(2));
                done = false;
            }

            // malloc - realloc => alloc ; dealloc ; alloc ;
            // Reduce "[;{}] alloc ; dealloc ; alloc ;" => "[;{}] alloc ;"
            if (Token::Match(tok2, "[;{}] alloc ; dealloc ; alloc ;")) {
                Token::eraseTokens(tok2->next(), tok2->tokAt(6));
                done = false;
            }

            // use; dealloc; => dealloc;
            if (Token::Match(tok2, "[;{}] use ; dealloc ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            // use use => use
            if (Token::simpleMatch(tok2, "use use")) {
                tok2->deleteThis();
                done = false;
            }

            // use; if| use; => use;
            while (Token::Match(tok2, "[;{}] use ; if| use ;")) {
                Token *t = tok2->tokAt(2);
                t->deleteNext();
                t->deleteNext();
                if (t->strAt(1) == ";")
                    t->deleteNext();
                done = false;
            }

            // Delete first part in "use ; return use ;"
            if (Token::Match(tok2, "[;{}] use ; return use ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(3));
                done = false;
            }

            // try/catch
            if (Token::simpleMatch(tok2, "try ; catch exit ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(4));
                tok2->deleteThis();
                done = false;
            }

            // Delete second case in "case ; case ;"
            while (Token::simpleMatch(tok2, "case ; case ;")) {
                Token::eraseTokens(tok2, tok2->tokAt(3));
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

                    incase |= (_tok->str() == "case");
                    incase &= (_tok->str() != "break" && _tok->str() != "return");
                }

                if (!incase && valid) {
                    done = false;
                    tok2->str(";");
                    Token::eraseTokens(tok2, tok2->tokAt(2));
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
        if (done && _settings->experimental) {
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

    if ((result = Token::findmatch(tokens, "loop alloc ;")) != NULL) {
        return result;
    }

    if (Token::Match(tokens, "alloc ; if|if(var)|ifv break|continue|return ;")) {
        return tokens->tokAt(3);
    }

    if ((result = Token::findmatch(tokens, "alloc ; if|if(var)|ifv return ;")) != NULL) {
        return result->tokAt(3);
    }

    if ((result = Token::findmatch(tokens, "alloc ; alloc|assign|return callfunc| ;")) != NULL) {
        return result->tokAt(2);
    }

    if ((result = Token::findmatch(tokens, "; alloc ; if assign ;")) != NULL) {
        return result->tokAt(4);
    }

    if (((result = Token::findmatch(tokens, "; alloc ; if dealloc ; }")) != NULL) &&
        !result->tokAt(7)) {
        return result->tokAt(6);
    }

    if ((result = Token::findmatch(tokens, "alloc ; }")) != NULL) {
        if (result->tokAt(3) == NULL)
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

    return NULL;
}






// Check for memory leaks for a function variable.
void CheckMemoryLeakInFunction::checkScope(const Token *Tok1, const std::string &varname, unsigned int varid, bool classmember, unsigned int sz)
{
    std::list<const Token *> callstack;

    AllocType alloctype = No;
    AllocType dealloctype = No;

    const Token *result;

    Token *tok = getcode(Tok1, callstack, varid, alloctype, dealloctype, classmember, sz);
    //tok->printOut((std::string("Checkmemoryleak: getcode result for: ") + varname).c_str());

    // Simplify the code and check if freed memory is used..
    for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
        while (Token::Match(tok2, "[;{}] ;"))
            Token::eraseTokens(tok2, tok2->tokAt(2));
    }
    if ((result = Token::findmatch(tok, "[;{}] dealloc ; use_ ;")) != NULL) {
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
    if (Token::findmatch(tok, "alloc") == 0) {
        Tokenizer::deleteTokens(tok);
        return;
    }

    simplifycode(tok);

    if (_settings->debug && _settings->_verbose) {
        tok->printOut(("Checkmemoryleak: simplifycode result for: " + varname).c_str());
    }

    // If the variable is not allocated at all => no memory leak
    if (Token::findmatch(tok, "alloc") == 0) {
        Tokenizer::deleteTokens(tok);
        return;
    }

    /** @todo handle "goto" */
    if (Token::findmatch(tok, "goto")) {
        Tokenizer::deleteTokens(tok);
        return;
    }

    if ((result = findleak(tok)) != NULL) {
        memoryLeak(result, varname, alloctype);
    }

    else if ((result = Token::findmatch(tok, "dealloc ; dealloc ;")) != NULL) {
        deallocDeallocError(result->tokAt(2), varname);
    }

    // detect cases that "simplifycode" don't handle well..
    else if (_settings->debugwarnings) {
        Token *first = tok;
        while (first && first->str() == ";")
            first = first->next();

        bool noerr = false;
        noerr |= Token::simpleMatch(first, "alloc ; }");
        noerr |= Token::simpleMatch(first, "alloc ; dealloc ; }");
        noerr |= Token::simpleMatch(first, "alloc ; return use ; }");
        noerr |= Token::simpleMatch(first, "alloc ; use ; }");
        noerr |= Token::simpleMatch(first, "alloc ; use ; return ; }");
        noerr |= Token::simpleMatch(first, "alloc ; dealloc ; return ; }");
        noerr |= Token::simpleMatch(first, "if alloc ; dealloc ; }");
        noerr |= Token::simpleMatch(first, "if alloc ; return use ; }");
        noerr |= Token::simpleMatch(first, "if alloc ; use ; }");
        noerr |= Token::simpleMatch(first, "alloc ; ifv return ; dealloc ; }");
        noerr |= Token::simpleMatch(first, "alloc ; if return ; dealloc; }");

        // Unhandled case..
        if (! noerr) {
            std::ostringstream errmsg;
            errmsg << "inconclusive leak of " << varname << ": ";
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
                errmsg << " " << tok2->str();
            reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
        }
    }

    Tokenizer::deleteTokens(tok);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Check for memory leaks due to improper realloc() usage.
//   Below, "a" may be set to null without being freed if realloc() cannot
//   allocate the requested memory:
//     a = malloc(10); a = realloc(a, 100);
//---------------------------------------------------------------------------
void CheckMemoryLeakInFunction::checkReallocUsage()
{
    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // Record the varid's of the function parameters
        std::set<unsigned int> parameterVarIds;
        for (const Token *tok2 = scope->classDef->next(); tok2 && tok2->str() != ")"; tok2 = tok2->next()) {
            if (tok2->varId() != 0)
                parameterVarIds.insert(tok2->varId());
        }

        const Token *tok = scope->classStart;
        const Token *startOfFunction = tok;

        // Search for the "var = realloc(var, 100" pattern within this function
        unsigned int indentlevel = 1;
        for (tok = tok->next(); tok; tok = tok->next()) {
            if (tok->str() == "{")
                ++indentlevel;
            else if (tok->str() == "}") {
                --indentlevel;
                if (indentlevel == 0)
                    break;
            }

            if (tok->varId() > 0 &&
                Token::Match(tok, "%var% = realloc|g_try_realloc ( %var% , %any%") &&
                tok->varId() == tok->tokAt(4)->varId() &&
                parameterVarIds.find(tok->varId()) == parameterVarIds.end()) {
                // Check that another copy of the pointer wasn't saved earlier in the function
                if (Token::findmatch(startOfFunction, "%var% = %varid% ;", tok->varId()) ||
                    Token::findmatch(startOfFunction, "[{};] %varid% = %var% [;=]", tok->varId()))
                    continue;

                const Token* tokEndRealloc = tok->tokAt(3)->link();
                // Check that the allocation isn't followed immediately by an 'if (!var) { error(); }' that might handle failure
                if (Token::Match(tokEndRealloc->tokAt(1), "; if ( ! %varid% ) {", tok->varId())) {
                    const Token* tokEndBrace = tokEndRealloc->tokAt(7)->link();
                    if (tokEndBrace && Token::simpleMatch(tokEndBrace->tokAt(-2), ") ;") &&
                        Token::Match(tokEndBrace->tokAt(-2)->link()->tokAt(-2), "{|}|; %var% ("))
                        continue;
                }

                memleakUponReallocFailureError(tok, tok->str());
            } else if (tok->tokAt(1)->varId() > 0 &&
                       (Token::Match(tok, "* %var% = realloc|g_try_realloc ( * %var% , %any%") &&
                        tok->tokAt(1)->varId() == tok->tokAt(6)->varId())&&
                       parameterVarIds.find(tok->tokAt(1)->varId()) == parameterVarIds.end()) {
                // Check that another copy of the pointer wasn't saved earlier in the function
                if (Token::findmatch(startOfFunction, "%var% = * %varid% ;", tok->tokAt(1)->varId()) ||
                    Token::findmatch(startOfFunction, "[{};] * %varid% = %var% [;=]", tok->tokAt(1)->varId()))
                    continue;

                const Token* tokEndRealloc = tok->tokAt(4)->link();
                // Check that the allocation isn't followed immediately by an 'if (!var) { error(); }' that might handle failure
                if (Token::Match(tokEndRealloc->tokAt(1), "; if ( ! * %varid% ) {", tok->tokAt(1)->varId())) {
                    const Token* tokEndBrace = tokEndRealloc->tokAt(8)->link();
                    if (tokEndBrace && Token::simpleMatch(tokEndBrace->tokAt(-2), ") ;") &&
                        Token::Match(tokEndBrace->tokAt(-2)->link()->tokAt(-2), "{|}|; %var% ("))
                        continue;
                }
                memleakUponReallocFailureError(tok->tokAt(1), tok->tokAt(1)->str());
            }
        }
    }
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// Checks for memory leaks inside function..
//---------------------------------------------------------------------------

void CheckMemoryLeakInFunction::parseFunctionScope(const Token *tok, const Token *tok1, const bool classmember)
{
    // Check locking/unlocking of global resources..
    checkScope(tok->next(), "", 0, classmember, 1);

    // Locate parameters and check their usage..
    for (const Token *tok2 = tok1; tok2; tok2 = tok2->next()) {
        if (tok2 == tok)
            break;
        if (tok2->str() == ")")
            break;
        if (Token::Match(tok2, "[(,] %type% * %var% [,)]") && tok2->next()->isStandardType()) {
            const std::string varname(tok2->strAt(3));
            const unsigned int varid = tok2->tokAt(3)->varId();
            const unsigned int sz = _tokenizer->sizeOfType(tok2->next());
            checkScope(tok->next(), varname, varid, classmember, sz);
        }
    }

    // Locate variable declarations and check their usage..
    unsigned int indentlevel = 0;
    do {
        if (tok->str() == "{")
            ++indentlevel;
        else if (tok->str() == "}") {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        }

        // Skip these weird blocks... "( { ... } )"
        if (Token::simpleMatch(tok, "( {")) {
            tok = tok->link();
            if (!tok)
                break;
            continue;
        }

        if (!Token::Match(tok, "[{};] %type%"))
            continue;

        // Don't check static/extern variables
        if (Token::Match(tok->next(), "static|extern"))
            continue;

        // return/else is not part of a variable declaration..
        if (Token::Match(tok->next(), "return|else"))
            continue;

        unsigned int sz = _tokenizer->sizeOfType(tok->next());
        if (sz < 1)
            sz = 1;

        if (Token::Match(tok, "[{};] %type% * const| %var% [;=]")) {
            const Token *vartok = tok->tokAt(tok->tokAt(3)->str() != "const" ? 3 : 4);
            checkScope(tok->next(), vartok->str(), vartok->varId(), classmember, sz);
        }

        else if (Token::Match(tok, "[{};] %type% %type% * const| %var% [;=]")) {
            const Token *vartok = tok->tokAt(tok->tokAt(4)->str() != "const" ? 4 : 5);
            checkScope(tok->next(), vartok->str(), vartok->varId(), classmember, sz);
        }

        else if (Token::Match(tok, "[{};] int %var% [;=]")) {
            const Token *vartok = tok->tokAt(2);
            checkScope(tok->next(), vartok->str(), vartok->varId(), classmember, sz);
        }
    } while (0 != (tok = tok->next()));
}

void CheckMemoryLeakInFunction::check()
{
    // fill the "noreturn"
    parse_noreturn();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        const Token *tok = scope->classStart;
        const Token *tok1 = scope->classDef->next();
        bool classmember = scope->functionOf != NULL;

        parseFunctionScope(tok, tok1, classmember);
    }
}
//---------------------------------------------------------------------------





























//---------------------------------------------------------------------------
// Checks for memory leaks in classes..
//---------------------------------------------------------------------------



void CheckMemoryLeakInClass::check()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check classes and structures
        if (scope->isClassOrStruct()) {
            std::list<Variable>::const_iterator var;
            for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
                if (!var->isStatic() && var->nameToken()->previous()->str() == "*") {
                    // allocation but no deallocation of private variables in public function..
                    if (var->nameToken()->tokAt(-2)->isStandardType()) {
                        if (var->isPrivate())
                            checkPublicFunctions(&(*scope), var->nameToken());

                        variable(&(*scope), var->nameToken());
                    }

                    // known class?
                    else if (var->type()) {
                        // not derived?
                        if (var->type()->derivedFrom.empty()) {
                            if (var->isPrivate())
                                checkPublicFunctions(&(*scope), var->nameToken());

                            variable(&(*scope), var->nameToken());
                        }
                    }
                }
            }
        }
    }
}


void CheckMemoryLeakInClass::variable(const Scope *scope, const Token *tokVarname)
{
    const std::string varname = tokVarname->strAt(0);
    const std::string classname = scope->className;

    // Check if member variable has been allocated and deallocated..
    CheckMemoryLeak::AllocType Alloc = CheckMemoryLeak::No;
    CheckMemoryLeak::AllocType Dealloc = CheckMemoryLeak::No;

    bool allocInConstructor = false;
    bool deallocInDestructor = false;

    // Inspect member functions
    std::list<Function>::const_iterator func;
    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        const Token *functionToken = func->token;
        const bool constructor = func->type == Function::eConstructor;
        const bool destructor = func->type == Function::eDestructor;
        unsigned int indent = 0;
        bool initlist = false;
        for (const Token *tok = functionToken; tok; tok = tok->next()) {
            if (tok->str() == "{")
                ++indent;
            else if (tok->str() == "}") {
                if (indent <= 1)
                    break;
                --indent;
            } else if (indent == 0 && Token::simpleMatch(tok, ") :"))
                initlist = true;
            else if (initlist || indent > 0) {
                if (indent == 0) {
                    if (!Token::Match(tok, (":|, " + varname + " (").c_str()))
                        continue;
                }

                // Allocate..
                if (indent == 0 || Token::Match(tok, (varname + " =").c_str())) {
                    // var1 = var2 = ...
                    // bail out
                    if (Token::simpleMatch(tok->previous(), "="))
                        return;

                    // Foo::var1 = ..
                    // bail out when not same class
                    if (Token::simpleMatch(tok->previous(), "::") &&
                        tok->strAt(-2) != scope->className)
                        return;

                    AllocType alloc = getAllocationType(tok->tokAt((indent > 0) ? 2 : 3), 0);
                    if (alloc != CheckMemoryLeak::No) {
                        if (constructor)
                            allocInConstructor = true;

                        if (Alloc != No && Alloc != alloc)
                            alloc = CheckMemoryLeak::Many;

                        std::list<const Token *> callstack;
                        if (alloc != CheckMemoryLeak::Many && Dealloc != CheckMemoryLeak::No && Dealloc != CheckMemoryLeak::Many && Dealloc != alloc) {
                            callstack.push_back(tok);
                            mismatchAllocDealloc(callstack, classname + "::" + varname);
                            callstack.pop_back();
                        }

                        Alloc = alloc;
                    }
                }

                if (indent == 0)
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
                        callstack.pop_back();
                    }

                    Dealloc = dealloc;
                }

                // Function call .. possible deallocation
                else if (Token::Match(tok->previous(), "[{};] %var% (")) {
                    if (!CheckMemoryLeakInFunction::test_white_list(tok->str().c_str())) {
                        return;
                    }
                }
            }
        }
    }

    if (allocInConstructor && !deallocInDestructor) {
        memoryLeak(tokVarname, (classname + "::" + varname).c_str(), Alloc);
    } else if (Alloc != CheckMemoryLeak::No && Dealloc == CheckMemoryLeak::No) {
        memoryLeak(tokVarname, (classname + "::" + varname).c_str(), Alloc);
    }
}


void CheckMemoryLeakInClass::checkPublicFunctions(const Scope *scope, const Token *classtok)
{
    // Check that public functions deallocate the pointers that they allocate.
    // There is no checking how these functions are used and therefore it
    // isn't established if there is real leaks or not.
    if (!_settings->isEnabled("style"))
        return;

    const unsigned int varid = classtok->varId();

    // Parse public functions..
    // If they allocate member variables, they should also deallocate
    std::list<Function>::const_iterator func;

    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        if (func->type != Function::eConstructor &&
            func->access == Public && func->hasBody) {
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
    unsigned int indentlevel1 = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            ++indentlevel1;
        else if (tok->str() == "}")
            --indentlevel1;

        if (indentlevel1 == 0)
            continue;

        // check struct variables..
        if (Token::Match(tok, "struct|;|{|} %type% * %var% [=;]")) {
            checkStructVariable(tok->tokAt(3));
        } else if (Token::Match(tok, "struct|;|{|} %type% %var% ;")) {
            checkStructVariable(tok->tokAt(2));
        }
    }
}

bool CheckMemoryLeakStructMember::isMalloc(const Token *vartok)
{
    const unsigned int varid(vartok->varId());
    bool alloc = false;
    unsigned int indentlevel2 = 0;
    for (const Token *tok2 = vartok; tok2; tok2 = tok2->next()) {
        if (tok2->str() == "{")
            ++indentlevel2;
        else if (tok2->str() == "}") {
            if (indentlevel2 == 0)
                break;
            --indentlevel2;
        } else if (Token::Match(tok2, "= %varid% [;=]", varid)) {
            return false;
        } else if (Token::Match(tok2, "%varid% = malloc|kmalloc (", varid)) {
            alloc = true;
        }
    }
    return alloc;
}


void CheckMemoryLeakStructMember::checkStructVariable(const Token * const vartok)
{
    // This should be in the CheckMemoryLeak base class
    std::set<std::string> ignoredFunctions;
    ignoredFunctions.insert("if");
    ignoredFunctions.insert("for");
    ignoredFunctions.insert("while");
    ignoredFunctions.insert("malloc");

    if (vartok->varId() == 0)
        return;

    // Is struct variable a pointer?
    if (vartok->strAt(-1) == "*") {
        // Check that variable is allocated with malloc
        if (!isMalloc(vartok))
            return;
    } else {
        // If file extension is not .c then a destructor might cleanup
        // members
        const std::string &fname = _tokenizer->getFiles()->at(0);
        if (fname.find(".c") != fname.size() - 2U)
            return;
    }

    // Check struct..
    unsigned int indentlevel2 = 0;
    for (const Token *tok2 = vartok; tok2; tok2 = tok2->next()) {
        if (tok2->str() == "{")
            ++indentlevel2;

        else if (tok2->str() == "}") {
            if (indentlevel2 == 0)
                break;
            --indentlevel2;
        }

        // Unknown usage of struct
        /** @todo Check how the struct is used. Only bail out if necessary */
        else if (Token::Match(tok2, "[(,] %varid% [,)]", vartok->varId()))
            break;

        // Struct member is allocated => check if it is also properly deallocated..
        else if (Token::Match(tok2->previous(), "[;{}] %varid% . %var% = malloc|strdup|kmalloc (", vartok->varId())) {
            const unsigned int structid(vartok->varId());
            const unsigned int structmemberid(tok2->tokAt(2)->varId());

            // This struct member is allocated.. check that it is deallocated
            unsigned int indentlevel3 = indentlevel2;
            for (const Token *tok3 = tok2; tok3; tok3 = tok3->next()) {
                if (tok3->str() == "{")
                    ++indentlevel3;

                else if (tok3->str() == "}") {
                    if (indentlevel3 == 0) {
                        memoryLeak(tok3, (vartok->str() + "." + tok2->strAt(2)).c_str(), Malloc);
                        break;
                    }
                    --indentlevel3;
                }

                // Deallocating the struct member..
                else if (Token::Match(tok3, "free|kfree ( %var% . %varid% )", structmemberid)) {
                    // If the deallocation happens at the base level, don't check this member anymore
                    if (indentlevel3 == 0)
                        break;

                    // deallocating and then returning from function in a conditional block =>
                    // skip ahead out of the block
                    bool ret = false;
                    while (tok3) {
                        // debug info
                        const std::string tok3str_(tok3->str());
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
                    memoryLeak(tok3, (vartok->str() + "." + tok2->strAt(2)).c_str(), Malloc);
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
                        memoryLeak(tok3, (vartok->str() + "." + tok2->strAt(2)).c_str(), Malloc);
                    }
                    break;
                }

                // struct assignment..
                else if (Token::Match(tok3, "= %varid% ;", structid)) {
                    break;
                }

                // goto isn't handled well.. bail out even though there might be leaks
                else if (tok3->str() == "goto")
                    break;

                // using struct in a function call..
                else if (Token::Match(tok3, "%var% (")) {
                    // Calling non-function / function that doesn't deallocate?
                    if (ignoredFunctions.find(tok3->str()) != ignoredFunctions.end())
                        continue;

                    // Check if the struct is used..
                    bool deallocated = false;
                    unsigned int parlevel = 0;
                    for (const Token *tok4 = tok3; tok4; tok4 = tok4->next()) {
                        if (tok4->str() == "(")
                            ++parlevel;

                        else if (tok4->str() == ")") {
                            if (parlevel <= 1)
                                break;
                            --parlevel;
                        }

                        if (Token::Match(tok4, "[(,] &| %varid% [,)]", structid)) {
                            /** @todo check if the function deallocates the memory */
                            deallocated = true;
                            break;
                        }

                        if (Token::Match(tok4, "[(,] &| %varid% . %var% [,)]", structid)) {
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



#include "checkuninitvar.h"		// CheckUninitVar::analyse

void CheckMemoryLeakNoVar::check()
{
    std::set<std::string> uvarFunctions;
    {
        const CheckUninitVar c(_tokenizer, _settings, _errorLogger);
        c.analyse(_tokenizer->tokens(), uvarFunctions);
    }

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // goto the "}" that ends the executable scope..
        const Token *tok = scope->classEnd;

        // parse the executable scope until tok is reached...
        for (const Token *tok2 = tok->link(); tok2 && tok2 != tok; tok2 = tok2->next()) {
            // allocating memory in parameter for function call..
            if (Token::Match(tok2, "[(,] %var% (") && Token::Match(tok2->tokAt(2)->link(), ") [,)]")) {
                const AllocType allocType = getAllocationType(tok2->next(), 0);
                if (allocType != No) {
                    // locate outer function call..
                    for (const Token *tok3 = tok2; tok3; tok3 = tok3->previous()) {
                        if (tok3->str() == "(") {
                            // Is it a function call..
                            if (Token::Match(tok3->tokAt(-2), "[(,;{}] %var% (")) {
                                const std::string functionName = tok3->strAt(-1);
                                if (functionName == "delete" ||
                                    functionName == "free" ||
                                    functionName == "fclose" ||
                                    functionName == "realloc")
                                    break;
                                if (CheckMemoryLeakInFunction::test_white_list(functionName)) {
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

void CheckMemoryLeakNoVar::functionCallLeak(const Token *loc, const std::string &alloc, const std::string &functionCall)
{
    reportError(loc, Severity::error, "leakNoVarFunctionCall", "Allocation with " + alloc + ", " + functionCall + " doesn't release it.");
}


