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
#include "checkuninitvar.h"
#include "mathlib.h"
#include "executionpath.h"
#include "checknullpointer.h"   // CheckNullPointer::parseFunctionCall
#include "symboldatabase.h"
#include <algorithm>
#include <map>
#include <cassert>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckUninitVar instance;
}

//---------------------------------------------------------------------------


// Skip [ .. ]
static const Token * skipBrackets(const Token *tok)
{
    while (tok && tok->str() == "[")
        tok = tok->link()->next();
    return tok;
}


/// @addtogroup Checks
/// @{

/**
 * @brief %Check that uninitialized variables aren't used (using ExecutionPath)
 * */
class UninitVar : public ExecutionPath {
public:
    /** Startup constructor */
    explicit UninitVar(Check *c, const SymbolDatabase* db, const Library *lib, bool isc)
        : ExecutionPath(c, 0), symbolDatabase(db), library(lib), isC(isc), var(0), alloc(false), strncpy_(false), memset_nonzero(false) {
    }

private:
    /** Create a copy of this check */
    ExecutionPath *copy() {
        return new UninitVar(*this);
    }

    /** no implementation => compiler error if used */
    void operator=(const UninitVar &);

    /** internal constructor for creating extra checks */
    UninitVar(Check *c, const Variable* v, const SymbolDatabase* db, const Library *lib, bool isc)
        : ExecutionPath(c, v->declarationId()), symbolDatabase(db), library(lib), isC(isc), var(v), alloc(false), strncpy_(false), memset_nonzero(false) {
    }

    /** is other execution path equal? */
    bool is_equal(const ExecutionPath *e) const {
        const UninitVar *c = static_cast<const UninitVar *>(e);
        return (var == c->var && alloc == c->alloc && strncpy_ == c->strncpy_ && memset_nonzero == c->memset_nonzero);
    }

    /** pointer to symbol database */
    const SymbolDatabase* symbolDatabase;

    /** pointer to library */
    const Library *library;

    const bool isC;

    /** variable for this check */
    const Variable* var;

    /** is this variable allocated? */
    bool  alloc;

    /** is this variable initialized with strncpy (not always zero-terminated) */
    bool  strncpy_;

    /** is this variable initialized but not zero-terminated (memset) */
    bool  memset_nonzero;

    /** allocating pointer. For example : p = malloc(10); */
    static void alloc_pointer(std::list<ExecutionPath *> &checks, unsigned int varid) {
        // loop through the checks and perform a allocation if the
        // variable id matches
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid) {
                if (c->var->isPointer() && !c->var->isArray())
                    c->alloc = true;
                else
                    bailOutVar(checks, varid);
                break;
            }
        }
    }

    /** Initializing a pointer value. For example: *p = 0; */
    static void init_pointer(std::list<ExecutionPath *> &checks, const Token *tok) {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        // loop through the checks and perform a initialization if the
        // variable id matches
        std::list<ExecutionPath *>::iterator it = checks.begin();
        while (it != checks.end()) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid) {
                if (c->alloc || c->var->isArray()) {
                    delete c;
                    checks.erase(it++);
                    continue;
                } else {
                    use_pointer(checks, tok);
                }
            }

            ++it;
        }
    }

    /** Deallocate a pointer. For example: free(p); */
    static void dealloc_pointer(std::list<ExecutionPath *> &checks, const Token *tok) {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        // loop through the checks and perform a deallocation if the
        // variable id matches
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid) {
                // unallocated pointer variable => error
                if (c->var->isPointer() && !c->var->isArray() && !c->alloc) {
                    CheckUninitVar *checkUninitVar = dynamic_cast<CheckUninitVar *>(c->owner);
                    if (checkUninitVar) {
                        checkUninitVar->uninitvarError(tok, c->var->name());
                        break;
                    }
                }
                c->alloc = false;
            }
        }
    }

    /**
     * Pointer assignment:  p = x;
     * if p is a pointer and x is an array/pointer then bail out
     * \param checks all available checks
     * \param tok1 the "p" token
     * \param tok2 the "x" token
     */
    static void pointer_assignment(std::list<ExecutionPath *> &checks, const Token *tok1, const Token *tok2) {
        // Variable id for "left hand side" variable
        const unsigned int varid1(tok1->varId());
        if (varid1 == 0)
            return;

        // Variable id for "right hand side" variable
        const unsigned int varid2(tok2->varId());
        if (varid2 == 0)
            return;

        std::list<ExecutionPath *>::const_iterator it;

        // bail out if first variable is a pointer
        for (it = checks.begin(); it != checks.end(); ++it) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid1 && c->var->isPointer() && !c->var->isArray()) {
                bailOutVar(checks, varid1);
                break;
            }
        }

        // bail out if second variable is a array/pointer
        for (it = checks.begin(); it != checks.end(); ++it) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid2 && (c->var->isPointer() || c->var->isArray())) {
                bailOutVar(checks, varid2);
                break;
            }
        }
    }


    /** Initialize an array with strncpy. */
    static void init_strncpy(std::list<ExecutionPath *> &checks, const Token *tok) {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid) {
                c->strncpy_ = true;
            }
        }
    }

    /** Initialize an array with memset (not zero). */
    static void init_memset_nonzero(std::list<ExecutionPath *> &checks, const Token *tok) {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid) {
                c->memset_nonzero = true;
            }
        }
    }



    /**
     * use - called from the use* functions below.
     * @param checks all available checks
     * @param tok variable token
     * @param mode specific behaviour
     * @return if error is found, true is returned
     */
    static bool use(std::list<ExecutionPath *> &checks, const Token *tok, const int mode) {
        const unsigned int varid(tok->varId());
        if (varid == 0)
            return false;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it) {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid) {
                // mode 0 : the variable is used "directly"
                // example: .. = var;
                // it is ok to read the address of an uninitialized array.
                // it is ok to read the address of an allocated pointer
                if (mode == 0 && (c->var->isArray() || (c->var->isPointer() && c->alloc)))
                    continue;

                // mode 2 : reading array data with mem.. function. It's ok if the
                //          array is not null-terminated
                if (mode == 2 && c->strncpy_)
                    continue;

                // mode 3 : bad usage of pointer. if it's not a pointer then the usage is ok.
                // example: ptr->foo();
                if (mode == 3 && (!c->var->isPointer() || c->var->isArray()))
                    continue;

                // mode 4 : using dead pointer is invalid.
                if (mode == 4 && (!c->var->isPointer() || c->var->isArray() || c->alloc))
                    continue;

                // mode 5 : reading uninitialized array or pointer is invalid.
                if (mode == 5 && (!c->var->isArray() && !c->var->isPointer()))
                    continue;

                CheckUninitVar *checkUninitVar = dynamic_cast<CheckUninitVar *>(c->owner);
                if (checkUninitVar) {
                    if (c->strncpy_ || c->memset_nonzero) {
                        if (!Token::Match(c->var->typeStartToken(), "char|wchar_t")) {
                            continue;
                        }
                        if (Token::Match(tok->next(), "[")) { // Check if it's not being accesed like: 'str[1]'
                            continue;
                        }
                        checkUninitVar->uninitstringError(tok, c->var->name(), c->strncpy_);
                    } else if (c->var->isPointer() && !c->var->isArray() && c->alloc)
                        checkUninitVar->uninitdataError(tok, c->var->name());
                    else
                        checkUninitVar->uninitvarError(tok, c->var->name());
                    return true;
                }
            }
        }

        // No error found
        return false;
    }

    /**
     * Reading variable. Use this function in situations when it is
     * invalid to read the data of the variable but not the address.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use(std::list<ExecutionPath *> &checks, const Token *tok) {
        return use(checks, tok, 0);
    }

    /**
     * Reading array elements. If the variable is not an array then the usage is ok.
     * @param checks all available checks
     * @param tok variable token
     */
    static void use_array(std::list<ExecutionPath *> &checks, const Token *tok) {
        use(checks, tok, 1);
    }

    /**
     * Reading array elements with a "mem.." function. It's ok if the array is not null-terminated.
     * @param checks all available checks
     * @param tok variable token
     */
    static void use_array_mem(std::list<ExecutionPath *> &checks, const Token *tok) {
        use(checks, tok, 2);
    }

    /**
     * Bad pointer usage. If the variable is not a pointer then the usage is ok.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_pointer(std::list<ExecutionPath *> &checks, const Token *tok) {
        return use(checks, tok, 3);
    }

    /**
     * Using variable.. if it's a dead pointer the usage is invalid.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_dead_pointer(std::list<ExecutionPath *> &checks, const Token *tok) {
        return use(checks, tok, 4);
    }

    /**
     * Using variable.. reading from uninitialized array or pointer data is invalid.
     * Example: = x[0];
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_array_or_pointer_data(std::list<ExecutionPath *> &checks, const Token *tok) {
        return use(checks, tok, 5);
    }

    /**
     * Parse right hand side expression in statement
     * @param tok2 start token of rhs
     * @param checks the execution paths
     */
    static void parserhs(const Token *tok2, std::list<ExecutionPath *> &checks) {
        // check variable usages in rhs/index
        while (NULL != (tok2 = tok2->next())) {
            if (Token::Match(tok2, "[;)=]"))
                break;
            if (Token::Match(tok2, "%var% ("))
                break;
            if (Token::Match(tok2, "%var% <") && Token::Match(tok2->linkAt(1), "> ("))
                break;
            if (tok2->varId() &&
                !Token::Match(tok2->previous(), "&|::") &&
                !Token::simpleMatch(tok2->tokAt(-2), "& (") &&
                tok2->strAt(1) != "=") {
                // Multiple assignments..
                if (Token::Match(tok2->next(), ".|[")) {
                    const Token * tok3 = tok2;
                    while (tok3) {
                        if (Token::Match(tok3->next(), ". %var%"))
                            tok3 = tok3->tokAt(2);
                        else if (tok3->strAt(1) == "[")
                            tok3 = tok3->next()->link();
                        else
                            break;
                    }
                    if (tok3 && tok3->strAt(1) == "=")
                        continue;
                }
                bool foundError;
                if (tok2->previous()->str() == "*" || tok2->next()->str() == "[")
                    foundError = use_array_or_pointer_data(checks, tok2);
                else
                    foundError = use(checks, tok2);

                // prevent duplicate error messages
                if (foundError) {
                    bailOutVar(checks, tok2->varId());
                }
            }
        }

    }

    /** parse tokens. @sa ExecutionPath::parse */
    const Token *parse(const Token &tok, std::list<ExecutionPath *> &checks) const {
        // Variable declaration..
        if (tok.varId() && Token::Match(&tok, "%var% [[;]")) {
            const Variable* var2 = tok.variable();
            if (var2 && var2->nameToken() == &tok && !var2->isStatic() && !var2->isExtern() && !var2->isConst() && !Token::simpleMatch(tok.linkAt(1), "] [")) {
                if (tok.linkAt(1)) { // array
                    const Token* endtok = tok.next();
                    while (endtok->link())
                        endtok = endtok->link()->next();
                    if (endtok->str() != ";")
                        return &tok;
                }
                const Scope* parent = var2->scope()->nestedIn;
                while (parent) {
                    for (std::list<Variable>::const_iterator j = parent->varlist.begin(); j != parent->varlist.end(); ++j) {
                        if (j->name() == var2->name()) {
                            ExecutionPath::bailOutVar(checks, j->declarationId()); // If there is a variable with the same name in other scopes, this might cause false positives, if there are unexpanded macros
                            break;
                        }
                    }
                    parent = parent->nestedIn;
                }

                if (var2->isPointer())
                    checks.push_back(new UninitVar(owner, var2, symbolDatabase, library, isC));
                else if (var2->typeEndToken()->str() != ">") {
                    bool stdtype = false;  // TODO: change to isC to handle unknown types better
                    for (const Token* tok2 = var2->typeStartToken(); tok2 != var2->nameToken(); tok2 = tok2->next()) {
                        if (tok2->isStandardType()) {
                            stdtype = true;
                            break;
                        }
                    }
                    if (stdtype && (!var2->isArray() || var2->nameToken()->linkAt(1)->strAt(1) == ";"))
                        checks.push_back(new UninitVar(owner, var2, symbolDatabase, library, isC));
                }
                return &tok;
            }
        }

        if (tok.str() == "return") {
            // is there assignment or ternary operator in the return statement?
            bool assignment = false;
            for (const Token *tok2 = tok.next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->str() == "=" || (!isC && tok2->str() == ">>") || Token::Match(tok2, "(|, &")) {
                    assignment = true;
                    break;
                }
                if (Token::Match(tok2, "[(,] &| %var% [,)]")) {
                    tok2 = tok2->next();
                    if (!tok2->isName())
                        tok2 = tok2->next();
                    ExecutionPath::bailOutVar(checks, tok2->varId());
                }
            }

            if (!assignment) {
                for (const Token *tok2 = tok.next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                    if (tok2->isName() && tok2->strAt(1) == "(")
                        tok2 = tok2->next()->link();

                    else if (tok2->varId())
                        use(checks, tok2);
                }
            }
        }

        if (tok.varId()) {
            // array variable passed as function parameter..
            if (Token::Match(tok.previous(), "[(,] %var% [+-,)]")) {
                // #4896 : This checking was removed because of FP,
                // the new uninitvar checking is used instead to catch
                // these errors.
                ExecutionPath::bailOutVar(checks, tok.varId());
                return &tok;
            }

            // Used..
            if (Token::Match(tok.previous(), "[[(,+-*/|=] %var% ]|)|,|;|%op%") && !tok.next()->isAssignmentOp()) {
                // Taking address of array..
                std::list<ExecutionPath *>::const_iterator it;
                for (it = checks.begin(); it != checks.end(); ++it) {
                    UninitVar *c = dynamic_cast<UninitVar *>(*it);
                    if (c && c->varId == tok.varId()) {
                        if (c->var->isArray() || c->alloc)
                            bailOutVar(checks, tok.varId());
                        break;
                    }
                }

                // initialize reference variable
                if (Token::Match(tok.tokAt(-3), "& %var% ="))
                    bailOutVar(checks, tok.varId());
                else
                    use(checks, &tok);
                return &tok;
            }

            if ((tok.previous() && tok.previous()->type() == Token::eIncDecOp) || (tok.next() && tok.next()->type() == Token::eIncDecOp)) {
                use(checks, &tok);
                return &tok;
            }

            if (Token::Match(tok.previous(), "[;{}] %var% [=[.]")) {
                if (tok.next()->str() == ".") {
                    if (Token::Match(&tok, "%var% . %var% (")) {
                        const Function *function = tok.tokAt(2)->function();
                        if (function && function->isStatic)
                            return &tok;
                    }
                    if (use_dead_pointer(checks, &tok)) {
                        return &tok;
                    }
                } else {
                    const Token *tok2 = tok.next();

                    if (tok2->str() == "[") {
                        const Token *tok3 = tok2->link();
                        while (Token::simpleMatch(tok3, "] ["))
                            tok3 = tok3->next()->link();

                        // Possible initialization
                        if (Token::simpleMatch(tok3, "] >>"))
                            return &tok;

                        if (Token::simpleMatch(tok3, "] =")) {
                            if (use_dead_pointer(checks, &tok)) {
                                return &tok;
                            }

                            parserhs(tok2, checks);
                            tok2 = tok3->next();
                        }
                    }

                    parserhs(tok2, checks);
                }

                // pointer aliasing?
                if (Token::Match(tok.tokAt(2), "%var% ;")) {
                    pointer_assignment(checks, &tok, tok.tokAt(2));
                }
            }

            if (tok.strAt(1) == "(") {
                use_pointer(checks, &tok);
            }

            if (Token::Match(tok.tokAt(-2), "[;{}] *")) {
                if (tok.strAt(1) == "=") {
                    // is the pointer used in the rhs?
                    bool used = false;
                    for (const Token *tok2 = tok.tokAt(2); tok2; tok2 = tok2->next()) {
                        if (Token::Match(tok2, "[,;=(]"))
                            break;
                        else if (Token::Match(tok2, "* %varid%", tok.varId())) {
                            used = true;
                            break;
                        }
                    }
                    if (used)
                        use_pointer(checks, &tok);
                    else
                        init_pointer(checks, &tok);
                } else {
                    use_pointer(checks, &tok);
                }
                return &tok;
            }

            if (Token::Match(tok.next(), "= malloc|kmalloc") || Token::simpleMatch(tok.next(), "= new char [") ||
                (Token::Match(tok.next(), "= %var% (") && library->returnuninitdata.find(tok.strAt(2)) != library->returnuninitdata.end())) {
                alloc_pointer(checks, tok.varId());
                if (tok.strAt(3) == "(")
                    return tok.tokAt(3);
            }

            else if ((!isC && (Token::Match(tok.previous(), "<<|>>") || Token::Match(tok.previous(), "[;{}] %var% <<"))) ||
                     tok.strAt(1) == "=") {
                // TODO: Don't bail out for "<<" and ">>" if these are
                // just computations
                ExecutionPath::bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (tok.strAt(1) == "[" && tok.next()->link()) {
                const Token *tok2 = tok.next()->link();
                if (tok2->strAt(1) == "=") {
                    ExecutionPath::bailOutVar(checks, tok.varId());
                    return &tok;
                }
            }

            if (tok.strAt(-1) == "delete" ||
                Token::simpleMatch(tok.tokAt(-3), "delete [ ]")) {
                dealloc_pointer(checks, &tok);
                return &tok;
            }
        }

        if (Token::Match(&tok, "%var% (") && uvarFunctions.find(tok.str()) == uvarFunctions.end()) {
            // sizeof/typeof doesn't dereference. A function name that is all uppercase
            // might be an unexpanded macro that uses sizeof/typeof
            if (Token::Match(&tok, "sizeof|typeof ("))
                return tok.next()->link();

            // deallocate pointer
            if (Token::Match(&tok, "free|kfree|fclose ( %var% )") ||
                Token::Match(&tok, "realloc ( %var%")) {
                dealloc_pointer(checks, tok.tokAt(2));
                if (tok.str() == "realloc")
                    ExecutionPath::bailOutVar(checks, tok.tokAt(2)->varId());
                return tok.tokAt(3);
            }

            // parse usage..
            {
                std::list<const Token *> var1;
                CheckNullPointer::parseFunctionCall(tok, var1, library, 1);
                for (std::list<const Token *>::const_iterator it = var1.begin(); it != var1.end(); ++it) {
                    // does iterator point at first function parameter?
                    const bool firstPar(*it == tok.tokAt(2));

                    // is function memset/memcpy/etc?
                    if (tok.str().compare(0,3,"mem") == 0)
                        use_array_mem(checks, *it);

                    // second parameter for strncpy/strncat/etc
                    else if (!firstPar && tok.str().compare(0,4,"strn") == 0)
                        use_array_mem(checks, *it);

                    else
                        use_array(checks, *it);

                    use_dead_pointer(checks, *it);
                }

                // Using uninitialized pointer is bad if using null pointer is bad
                std::list<const Token *> var2;
                CheckNullPointer::parseFunctionCall(tok, var2, library, 0);
                for (std::list<const Token *>::const_iterator it = var2.begin(); it != var2.end(); ++it) {
                    if (std::find(var1.begin(), var1.end(), *it) == var1.end())
                        use_dead_pointer(checks, *it);
                }
            }

            // strncpy doesn't null-terminate first parameter
            if (Token::Match(&tok, "strncpy ( %var% ,")) {
                if (Token::Match(tok.tokAt(4), "%str% ,")) {
                    if (Token::Match(tok.tokAt(6), "%num% )")) {
                        const std::size_t len = Token::getStrLength(tok.tokAt(4));
                        const MathLib::bigint sz = MathLib::toLongNumber(tok.strAt(6));
                        if (sz >= 0 && len >= static_cast<unsigned long>(sz)) {
                            init_strncpy(checks, tok.tokAt(2));
                            return tok.next()->link();
                        }
                    }
                } else {
                    init_strncpy(checks, tok.tokAt(2));
                    return tok.next()->link();
                }
            }

            // memset (not zero terminated)..
            if (Token::Match(&tok, "memset ( %var% , !!0 , %num% )")) {
                init_memset_nonzero(checks, tok.tokAt(2));
                return tok.next()->link();
            }

            if (Token::Match(&tok, "asm ( %str% )")) {
                ExecutionPath::bailOut(checks);
                return &tok;
            }

            // is the variable passed as a parameter to some function?
            unsigned int parlevel = 0;
            std::set<unsigned int> bailouts;
            for (const Token *tok2 = tok.next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    ++parlevel;

                else if (tok2->str() == ")") {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }

                else if (Token::Match(tok2, "sizeof|typeof (")) {
                    tok2 = tok2->next()->link();
                    if (!tok2)
                        break;
                }

                // ticket #2367 : unexpanded macro that uses sizeof|typeof?
                else if (Token::Match(tok2, "%type% (") && tok2->isUpperCaseName()) {
                    tok2 = tok2->next()->link();
                    if (!tok2)
                        break;
                }

                else if (tok2->varId()) {
                    if (Token::Match(tok2->tokAt(-2), "[(,] *") || Token::Match(tok2->next(), ". %var%")) {
                        // find function call..
                        const Token *functionCall = tok2;
                        while (NULL != (functionCall = functionCall ? functionCall->previous() : 0)) {
                            if (functionCall->str() == "(")
                                break;
                            if (functionCall->str() == ")")
                                functionCall = functionCall->link();
                        }

                        functionCall = functionCall ? functionCall->previous() : 0;
                        if (functionCall) {
                            if (functionCall->isName() && !functionCall->isUpperCaseName() && use_dead_pointer(checks, tok2))
                                ExecutionPath::bailOutVar(checks, tok2->varId());
                        }
                    }

                    // it is possible that the variable is initialized here
                    if (Token::Match(tok2->previous(), "[(,] %var% [,)]"))
                        bailouts.insert(tok2->varId());

                    // array initialization..
                    if (Token::Match(tok2->previous(), "[,(] %var% [+-]")) {
                        // if var is array, bailout
                        for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it) {
                            if ((*it)->varId == tok2->varId()) {
                                const UninitVar *c = dynamic_cast<const UninitVar *>(*it);
                                if (c && (c->var->isArray() || (c->var->isPointer() && c->alloc)))
                                    bailouts.insert(tok2->varId());
                                break;
                            }
                        }
                    }
                }
            }

            for (std::set<unsigned int>::const_iterator it = bailouts.begin(); it != bailouts.end(); ++it)
                ExecutionPath::bailOutVar(checks, *it);
        }

        // function call via function pointer
        if (Token::Match(&tok, "( * %var% ) (") ||
            (Token::Match(&tok, "( *| %var% .|::") && Token::Match(tok.link()->tokAt(-2), ".|:: %var% ) ("))) {
            // is the variable passed as a parameter to some function?
            const Token *tok2 = tok.link()->next();
            for (const Token* const end2 = tok2->link(); tok2 != end2; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    // it is possible that the variable is initialized here
                    ExecutionPath::bailOutVar(checks, tok2->varId());
                }
            }
        }

        if (tok.str() == "return") {
            // Todo: if (!array && ..
            if (Token::Match(tok.next(), "%var% ;")) {
                use(checks, tok.next());
            } else if (Token::Match(tok.next(), "%var% [")) {
                use_array_or_pointer_data(checks, tok.next());
            }
        }

        if (tok.varId()) {
            if (tok.strAt(-1) == "=") {
                if (Token::Match(tok.tokAt(-3), "& %var% =")) {
                    bailOutVar(checks, tok.varId());
                    return &tok;
                }

                if (!Token::Match(tok.tokAt(-3), ". %var% =")) {
                    if (!Token::Match(tok.tokAt(-3), "[;{}] %var% =")) {
                        use(checks, &tok);
                        return &tok;
                    }

                    const unsigned int varid2 = tok.tokAt(-2)->varId();
                    if (varid2) {
                        {
                            use(checks, &tok);
                            return &tok;
                        }
                    }
                }
            }

            if (tok.strAt(1) == ".") {
                bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (tok.strAt(1) == "[") {
                ExecutionPath::bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (Token::Match(tok.tokAt(-2), "[,(=] *")) {
                use_pointer(checks, &tok);
                return &tok;
            }

            if (tok.strAt(-1) == "&") {
                ExecutionPath::bailOutVar(checks, tok.varId());
            }
        }

        // Parse "for"
        if (Token::Match(&tok, "[;{}] for (")) {
            // initialized variables
            std::set<unsigned int> varid1;
            varid1.insert(0);

            // Parse token
            const Token *tok2;

            // parse setup
            for (tok2 = tok.tokAt(3); tok2 != tok.link(); tok2 = tok2->next()) {
                if (tok2->str() == ";")
                    break;
                if (tok2->varId())
                    varid1.insert(tok2->varId());
            }
            if (tok2 == tok.link())
                return &tok;

            // parse condition
            if (Token::Match(tok2, "; %var% <|<=|>=|> %num% ;")) {
                // If the variable hasn't been initialized then call "use"
                if (varid1.find(tok2->next()->varId()) == varid1.end())
                    use(checks, tok2->next());
            }

            // goto stepcode
            tok2 = tok2->next();
            while (tok2 && tok2->str() != ";")
                tok2 = tok2->next();

            // parse the stepcode
            if (Token::Match(tok2, "; ++|-- %var% ) {") ||
                Token::Match(tok2, "; %var% ++|-- ) {")) {
                // get id of variable..
                unsigned int varid = tok2->next()->varId();
                if (!varid)
                    varid = tok2->tokAt(2)->varId();

                // Check that the variable hasn't been initialized and
                // that it isn't initialized in the body..
                if (varid1.find(varid) == varid1.end()) {
                    for (const Token *tok3 = tok2->tokAt(5); tok3 && tok3 != tok2->linkAt(4); tok3 = tok3->next()) {
                        if (tok3->varId() == varid) {
                            varid = 0;  // variable is used.. maybe it's initialized. clear the variable id.
                            break;
                        }
                    }

                    // If the variable isn't initialized in the body call "use"
                    if (varid != 0) {
                        // goto variable
                        tok2 = tok2->next();
                        if (!tok2->varId())
                            tok2 = tok2->next();

                        // call "use"
                        use(checks, tok2);
                    }
                }
            }
        }

        return &tok;
    }

    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks) {
        if (tok.varId() && Token::Match(&tok, "%var% <|<=|==|!=|)"))
            use(checks, &tok);

        else if (Token::Match(&tok, "!| %var% [") && !Token::simpleMatch(skipBrackets(tok.next()), "="))
            use_array_or_pointer_data(checks, tok.str() == "!" ? tok.next() : &tok);

        else if (Token::Match(&tok, "!| %var% (")) {
            const Token * const ftok = (tok.str() == "!") ? tok.next() : &tok;
            std::list<const Token *> var1;
            CheckNullPointer::parseFunctionCall(*ftok, var1, library, 1);
            for (std::list<const Token *>::const_iterator it = var1.begin(); it != var1.end(); ++it) {
                // is function memset/memcpy/etc?
                if (ftok->str().compare(0,3,"mem") == 0)
                    use_array_mem(checks, *it);
                else
                    use_array(checks, *it);
            }
        }

        else if (Token::Match(&tok, "! %var% )")) {
            use(checks, &tok);
            return false;
        }

        return ExecutionPath::parseCondition(tok, checks);
    }

    void parseLoopBody(const Token *tok, std::list<ExecutionPath *> &checks) const {
        while (tok) {
            if (tok->str() == "{" || tok->str() == "}" || tok->str() == "for")
                return;
            if (Token::simpleMatch(tok, "if (")) {
                // bail out all variables that are used in the condition
                const Token* const end2 = tok->linkAt(1);
                for (const Token *tok2 = tok->tokAt(2); tok2 != end2; tok2 = tok2->next()) {
                    if (tok2->varId())
                        ExecutionPath::bailOutVar(checks, tok2->varId());
                }
            }
            const Token *next = parse(*tok, checks);
            tok = next->next();
        }
    }

public:

    /** Functions that don't handle uninitialized variables well */
    static std::set<std::string> uvarFunctions;

    static void analyseFunctions(const Token * const tokens, std::set<std::string> &func) {
        for (const Token *tok = tokens; tok; tok = tok->next()) {
            if (tok->str() == "{") {
                tok = tok->link();
                continue;
            }
            if (tok->str() != "::" && Token::Match(tok->next(), "%var% ( %type%")) {
                if (!Token::Match(tok->linkAt(2), ") [{;]"))
                    continue;
                const Token *tok2 = tok->tokAt(3);
                while (tok2 && tok2->str() != ")") {
                    if (tok2->str() == ",")
                        tok2 = tok2->next();

                    if (Token::Match(tok2, "%type% %var% ,|)") && tok2->isStandardType()) {
                        tok2 = tok2->tokAt(2);
                        continue;
                    }

                    if (tok2->isStandardType() && Token::Match(tok2, "%type% & %var% ,|)")) {
                        const unsigned int varid(tok2->tokAt(2)->varId());

                        // flags for read/write
                        bool r = false, w = false;

                        // check how the variable is used in the function
                        unsigned int indentlevel = 0;
                        for (const Token *tok3 = tok2; tok3; tok3 = tok3->next()) {
                            if (tok3->str() == "{")
                                ++indentlevel;
                            else if (tok3->str() == "}") {
                                if (indentlevel <= 1)
                                    break;
                                --indentlevel;
                            } else if (indentlevel == 0 && tok3->str() == ";")
                                break;
                            else if (indentlevel >= 1 && tok3->varId() == varid) {
                                if (tok3->previous()->type() == Token::eIncDecOp ||
                                    tok3->next()->type() == Token::eIncDecOp) {
                                    r = true;
                                }

                                else {
                                    w = true;
                                    break;
                                }
                            }
                        }

                        if (!r || w)
                            break;

                        tok2 = tok2->tokAt(3);
                        continue;
                    }

                    if (Token::Match(tok2, "const %type% &|*| const| %var% ,|)") && tok2->next()->isStandardType()) {
                        tok2 = tok2->tokAt(3);
                        while (tok2->isName())
                            tok2 = tok2->next();
                        continue;
                    }

                    if (Token::Match(tok2, "const %type% %var% [ ] ,|)") && tok2->next()->isStandardType()) {
                        tok2 = tok2->tokAt(5);
                        continue;
                    }

                    /// @todo enable this code. if pointer is written in function then dead pointer is invalid but valid pointer is ok.
                    /*
                    if (Token::Match(tok2, "const| struct| %type% * %var% ,|)"))
                    {
                        while (tok2->isName())
                            tok2 = tok2->next();
                        tok2 = tok2->tokAt(2);
                        continue;
                    }
                    */

                    break;
                }

                // found simple function..
                if (tok2 && tok2->link() == tok->tokAt(2))
                    func.insert(tok->next()->str());
            }
        }
    }
};

/** Functions that don't handle uninitialized variables well */
std::set<std::string> UninitVar::uvarFunctions;


/// @}


void CheckUninitVar::analyse(const Token * tokens, std::set<std::string> &func) const
{
    UninitVar::analyseFunctions(tokens, func);
}

void CheckUninitVar::saveAnalysisData(const std::set<std::string> &data) const
{
    UninitVar::uvarFunctions.insert(data.begin(), data.end());
}

void CheckUninitVar::executionPaths()
{
    // check if variable is accessed uninitialized..
    {
        // no writing if multiple threads are used (TODO: thread safe analysis?)
        if (_settings->_jobs == 1)
            UninitVar::analyseFunctions(_tokenizer->tokens(), UninitVar::uvarFunctions);

        UninitVar c(this, _tokenizer->getSymbolDatabase(), &_settings->library, _tokenizer->isC());
        checkExecutionPaths(_tokenizer->getSymbolDatabase(), &c);
    }
}


void CheckUninitVar::check()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    std::list<Scope>::const_iterator scope;

    // check every executable scope
    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (scope->isExecutable()) {
            checkScope(&*scope);
        }
    }
}

void CheckUninitVar::checkScope(const Scope* scope)
{
    for (std::list<Variable>::const_iterator i = scope->varlist.begin(); i != scope->varlist.end(); ++i) {
        if ((_tokenizer->isCPP() && i->type() && !i->isPointer() && i->type()->needInitialization != Type::True) ||
            i->isStatic() || i->isExtern() || i->isConst() || i->isArray() || i->isReference())
            continue;
        // don't warn for try/catch exception variable
        {
            const Token *start = i->typeStartToken();
            while (start && start->isName())
                start = start->previous();
            if (start && Token::simpleMatch(start->previous(), "catch ("))
                continue;
        }
        if (i->nameToken()->strAt(1) == "(")
            continue;
        bool stdtype = _tokenizer->isC();
        const Token* tok = i->typeStartToken();
        for (; tok && tok->str() != ";" && tok->str() != "<"; tok = tok->next()) {
            if (tok->isStandardType())
                stdtype = true;
        }
        while (tok && tok->str() != ";")
            tok = tok->next();
        if (!tok)
            continue;
        if (Token::Match(i->nameToken(), "%var% =")) {
            checkRhs(i->nameToken(), *i, false, "");
            continue;
        }
        if (stdtype || i->isPointer()) {
            bool alloc = false;
            checkScopeForVariable(scope, tok, *i, NULL, NULL, &alloc, "");
        }
        if (Token::Match(i->typeStartToken(), "struct %type% *| %var% ;"))
            checkStruct(scope, tok, *i);
    }

    if (scope->function) {
        for (unsigned int i = 0; i < scope->function->argCount(); i++) {
            const Variable *arg = scope->function->getArgumentVar(i);
            if (arg && arg->declarationId() && Token::Match(arg->typeStartToken(), "struct| %type% * %var% [,)]")) {
                // Treat the pointer as initialized until it is assigned by malloc
                for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
                    if (Token::Match(tok, "[;{}] %varid% = %var% (", arg->declarationId()) &&
                        _settings->library.returnuninitdata.count(tok->strAt(3)) == 1U) {
                        if (arg->typeStartToken()->str() == "struct")
                            checkStruct(scope, tok, *arg);
                        else if (arg->typeStartToken()->isStandardType()) {
                            bool alloc = false;
                            checkScopeForVariable(scope, tok->next(), *arg, NULL, NULL, &alloc, "");
                        }
                    }
                }
            }
        }
    }
}

void CheckUninitVar::checkStruct(const Scope* scope, const Token *tok, const Variable &structvar)
{
    const Token *typeToken = structvar.typeStartToken();
    if (typeToken->str() == "struct")
        typeToken = typeToken->next();
    const std::string structname(typeToken->str());
    const SymbolDatabase * symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t j = 0U; j < symbolDatabase->classAndStructScopes.size(); ++j) {
        const Scope *scope2 = symbolDatabase->classAndStructScopes[j];
        if (scope2->className == structname && scope2->numConstructors == 0U) {
            for (std::list<Variable>::const_iterator it = scope2->varlist.begin(); it != scope2->varlist.end(); ++it) {
                const Variable &var = *it;
                if (!var.isArray()) {
                    // is the variable declared in a inner union?
                    bool innerunion = false;
                    for (std::list<Scope>::const_iterator it2 = symbolDatabase->scopeList.begin(); it2 != symbolDatabase->scopeList.end(); ++it2) {
                        const Scope &innerScope = *it2;
                        if (innerScope.type == Scope::eUnion && innerScope.nestedIn == scope2) {
                            if (var.typeStartToken()->linenr() >= innerScope.classStart->linenr() &&
                                var.typeStartToken()->linenr() <= innerScope.classEnd->linenr()) {
                                innerunion = true;
                                break;
                            }

                        }
                    }

                    if (!innerunion) {
                        bool alloc = false;
                        const Token *tok2 = tok;
                        if (tok->str() == "}")
                            tok2 = tok2->next();
                        checkScopeForVariable(scope, tok2, structvar, NULL, NULL, &alloc, var.name());
                    }
                }
            }
        }
    }
}

static void conditionAlwaysTrueOrFalse(const Token *tok, const std::map<unsigned int, int> &variableValue, bool *alwaysTrue, bool *alwaysFalse)
{
    assert(Token::simpleMatch(tok, "if ("));

    const Token *vartok = tok->tokAt(2);
    const bool NOT(vartok->str() == "!");
    if (NOT)
        vartok = vartok->next();

    while (Token::Match(vartok, "%var% . %var%"))
        vartok = vartok->tokAt(2);

    std::map<unsigned int, int>::const_iterator it = variableValue.find(vartok->varId());
    if (it == variableValue.end())
        return;

    // always true
    if (Token::Match(vartok, "%var% %oror%|)")) {
        if (NOT)
            *alwaysTrue = bool(it->second == 0);
        else
            *alwaysTrue = bool(it->second != 0);
    } else if (Token::Match(vartok, "%var% == %num% %or%|)")) {
        *alwaysTrue = bool(it->second == MathLib::toLongNumber(vartok->strAt(2)));
    } else if (Token::Match(vartok, "%var% != %num% %or%|)")) {
        *alwaysTrue = bool(it->second != MathLib::toLongNumber(vartok->strAt(2)));
    }

    // always false
    if (Token::Match(vartok, "%var% &&|)")) {
        if (NOT)
            *alwaysFalse = bool(it->second != 0);
        else
            *alwaysFalse = bool(it->second == 0);
    } else if (Token::Match(vartok, "%var% == %num% &&|)")) {
        *alwaysFalse = bool(it->second != MathLib::toLongNumber(vartok->strAt(2)));
    } else if (Token::Match(vartok, "%var% != %num% &&|)")) {
        *alwaysFalse = bool(it->second == MathLib::toLongNumber(vartok->strAt(2)));
    }
}

bool CheckUninitVar::checkScopeForVariable(const Scope* scope, const Token *tok, const Variable& var, bool * const possibleInit, bool * const noreturn, bool * const alloc, const std::string &membervar)
{
    const bool suppressErrors(possibleInit && *possibleInit);

    if (possibleInit)
        *possibleInit = false;

    unsigned int number_of_if = 0;

    if (var.declarationId() == 0U)
        return true;

    // variable values
    std::map<unsigned int, int> variableValue;
    static const int NOT_ZERO = (1<<30); // special variable value

    for (; tok; tok = tok->next()) {
        // End of scope..
        if (tok->str() == "}") {
            if (number_of_if && possibleInit)
                *possibleInit = true;

            // might be a noreturn function..
            if (_tokenizer->IsScopeNoReturn(tok)) {
                if (noreturn)
                    *noreturn = true;
                return true;
            }

            break;
        }

        // Unconditional inner scope..
        if (tok->str() == "{" && Token::Match(tok->previous(), "[;{}]")) {
            if (checkScopeForVariable(scope, tok->next(), var, possibleInit, NULL, alloc, membervar))
                return true;
            tok = tok->link();
            continue;
        }

        // assignment with nonzero constant..
        if (Token::Match(tok->previous(), "[;{}] %var% = - %var% ;") && tok->varId() > 0)
            variableValue[tok->varId()] = NOT_ZERO;

        // Inner scope..
        if (Token::simpleMatch(tok, "if (")) {
            bool alwaysTrue = false;
            bool alwaysFalse = false;

            conditionAlwaysTrueOrFalse(tok, variableValue, &alwaysTrue, &alwaysFalse);

            // initialization / usage in condition..
            if (!alwaysTrue && checkIfForWhileHead(tok->next(), var, suppressErrors, bool(number_of_if == 0), alloc && *alloc, membervar))
                return true;

            // checking if a not-zero variable is zero => bail out
            unsigned int condVarId = 0, condVarValue = 0;
            if (Token::Match(tok, "if ( %var% )")) {
                std::map<unsigned int,int>::const_iterator it = variableValue.find(tok->tokAt(2)->varId());
                if (it != variableValue.end() && it->second == NOT_ZERO)
                    return true;   // this scope is not fully analysed => return true
                else {
                    condVarId = tok->tokAt(2)->varId();
                    condVarValue = NOT_ZERO;
                }
            }

            // goto the {
            tok = tok->next()->link()->next();

            if (!tok)
                break;
            if (tok->str() == "{") {
                bool possibleInitIf(number_of_if > 0 || suppressErrors);
                bool noreturnIf = false;
                const bool initif = !alwaysFalse && checkScopeForVariable(scope, tok->next(), var, &possibleInitIf, &noreturnIf, alloc, membervar);

                // bail out for such code:
                //    if (a) x=0;    // conditional initialization
                //    if (b) return; // cppcheck doesn't know if b can be false when a is false.
                //    x++;           // it's possible x is always initialized
                if (!alwaysTrue && noreturnIf && number_of_if > 0) {
                    if (_settings->debugwarnings) {
                        std::string condition;
                        for (const Token *tok2 = tok->linkAt(-1); tok2 != tok; tok2 = tok2->next()) {
                            condition += tok2->str();
                            if (tok2->isName() && tok2->next()->isName())
                                condition += ' ';
                        }
                        reportError(tok, Severity::debug, "debug", "bailout uninitialized variable checking for '" + var.nameToken()->str() + "'. can't determine if this condition can be false when previous condition is false: " + condition);
                    }
                    return true;
                }

                if (alwaysTrue && noreturnIf)
                    return true;

                std::map<unsigned int, int> varValueIf;
                if (!alwaysFalse && !initif && !noreturnIf) {
                    for (const Token *tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                        if (Token::Match(tok2, "[;{}.] %var% = - %var% ;"))
                            varValueIf[tok2->next()->varId()] = NOT_ZERO;
                        if (Token::Match(tok2, "[;{}.] %var% = %num% ;"))
                            varValueIf[tok2->next()->varId()] = (int)MathLib::toLongNumber(tok2->strAt(3));
                    }
                }

                if (initif && condVarId > 0U)
                    variableValue[condVarId] = condVarValue ^ NOT_ZERO;

                // goto the }
                tok = tok->link();

                if (!Token::simpleMatch(tok, "} else {")) {
                    if (initif || possibleInitIf) {
                        ++number_of_if;
                        if (number_of_if >= 2)
                            return true;
                    }
                } else {
                    // goto the {
                    tok = tok->tokAt(2);

                    bool possibleInitElse(number_of_if > 0 || suppressErrors);
                    bool noreturnElse = false;
                    const bool initelse = !alwaysTrue && checkScopeForVariable(scope, tok->next(), var, &possibleInitElse, NULL, alloc, membervar);

                    std::map<unsigned int, int> varValueElse;
                    if (!alwaysTrue && !initelse && !noreturnElse) {
                        for (const Token *tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                            if (Token::Match(tok2, "[;{}.] %var% = - %var% ;"))
                                varValueElse[tok2->next()->varId()] = NOT_ZERO;
                            if (Token::Match(tok2, "[;{}.] %var% = %num% ;"))
                                varValueElse[tok2->next()->varId()] = (int)MathLib::toLongNumber(tok2->strAt(3));
                        }
                    }

                    if (initelse && condVarId > 0U && !noreturnIf && !noreturnElse)
                        variableValue[condVarId] = condVarValue;

                    // goto the }
                    tok = tok->link();

                    if ((alwaysFalse || initif || noreturnIf) &&
                        (alwaysTrue || initelse || noreturnElse))
                        return true;

                    if ((initif || initelse || possibleInitElse) && !noreturnIf && !noreturnElse) {
                        ++number_of_if;
                        variableValue.insert(varValueIf.begin(), varValueIf.end());
                        variableValue.insert(varValueElse.begin(), varValueElse.end());
                    }
                }
            }
        }

        // = { .. }
        if (Token::simpleMatch(tok, "= {")) {
            // end token
            const Token *end = tok->next()->link();

            // If address of variable is taken in the block then bail out
            if (Token::findmatch(tok->tokAt(2), "& %varid%", end, var.declarationId()))
                return true;

            // Skip block
            tok = end;
            continue;
        }

        // skip sizeof / offsetof
        if (Token::Match(tok, "sizeof|typeof|offsetof|decltype ("))
            tok = tok->next()->link();

        // for/while..
        if (Token::Match(tok, "for|while (") || Token::simpleMatch(tok, "do {")) {
            const bool forwhile = Token::Match(tok, "for|while (");

            // is variable initialized in for-head (don't report errors yet)?
            if (forwhile && checkIfForWhileHead(tok->next(), var, true, false, alloc && *alloc, membervar))
                return true;

            // goto the {
            const Token *tok2 = forwhile ? tok->next()->link()->next() : tok->next();

            if (tok2 && tok2->str() == "{") {
                bool init = checkLoopBody(tok2, var, alloc && *alloc, membervar, (number_of_if > 0) | suppressErrors);

                // variable is initialized in the loop..
                if (init)
                    return true;

                // is variable used in for-head?
                bool initcond = false;
                if (!suppressErrors) {
                    const Token *startCond = forwhile ? tok->next() : tok->next()->link()->tokAt(2);
                    initcond = checkIfForWhileHead(startCond, var, false, bool(number_of_if == 0), alloc && *alloc, membervar);
                }

                // goto "}"
                tok = tok2->link();

                // do-while => goto ")"
                if (!forwhile) {
                    // Assert that the tokens are '} while ('
                    if (!Token::simpleMatch(tok, "} while (")) {
                        if (_settings->debugwarnings)
                            reportError(tok,Severity::debug,"","assertion failed '} while ('");
                        break;
                    }

                    // Goto ')'
                    tok = tok->linkAt(2);

                    if (!tok)
                        // bailout : invalid code / bad tokenizer
                        break;

                    if (initcond)
                        // variable is initialized in while-condition
                        return true;
                }
            }
        }

        // TODO: handle loops, try, etc
        if (Token::simpleMatch(tok, ") {") || Token::Match(tok, "%var% {")) {
            return true;
        }

        // bailout if there is assembler code
        if (Token::simpleMatch(tok, "asm (")) {
            return true;
        }

        if (Token::Match(tok, "return|break|continue|throw|goto")) {
            if (noreturn)
                *noreturn = true;

            while (tok && tok->str() != ";") {
                // variable is seen..
                if (tok->varId() == var.declarationId()) {
                    if (!membervar.empty()) {
                        if (Token::Match(tok, "%var% . %var% ;|%cop%") && tok->strAt(2) == membervar)
                            uninitStructMemberError(tok, tok->str() + "." + membervar);
                        else
                            return true;
                    }

                    // Use variable
                    else if (!suppressErrors && isVariableUsage(tok, var.isPointer(), alloc && *alloc, _tokenizer->isCPP())) {
                        if (alloc && *alloc)
                            uninitdataError(tok, tok->str());
                        else
                            uninitvarError(tok, tok->str());
                    }

                    else
                        // assume that variable is assigned
                        return true;
                }

                else if (Token::Match(tok, "sizeof|typeof|offsetof|decltype ("))
                    tok = tok->linkAt(1);

                else if (tok->str() == "?")
                    // TODO: False negatives when "?:" is used.
                    // Fix the tokenizer and then remove this bailout.
                    // The tokenizer should replace "return x?y:z;" with "if(x)return y;return z;"
                    return true;

                tok = tok->next();
            }

            return bool(noreturn==NULL);
        }

        // variable is seen..
        if (tok->varId() == var.declarationId()) {
            // calling function that returns uninit data through pointer..
            if (var.isPointer() &&
                Token::Match(tok->next(), "= %var% (") &&
                Token::simpleMatch(tok->linkAt(3), ") ;") &&
                _settings->library.returnuninitdata.count(tok->strAt(2)) > 0U) {
                if (alloc)
                    *alloc = true;
                continue;
            }


            if (!membervar.empty()) {
                if (isMemberVariableAssignment(tok, membervar)) {
                    checkRhs(tok, var, alloc && *alloc, membervar);
                    return true;
                }

                if (isMemberVariableUsage(tok, var.isPointer(), alloc && *alloc, membervar))
                    uninitStructMemberError(tok, tok->str() + "." + membervar);

                else if (Token::Match(tok->previous(), "[(,] %var% [,)]"))
                    return true;

            } else {
                // Use variable
                if (!suppressErrors && isVariableUsage(tok, var.isPointer(), alloc && *alloc, _tokenizer->isCPP())) {
                    if (alloc && *alloc)
                        uninitdataError(tok, tok->str());
                    else
                        uninitvarError(tok, tok->str());
                }

                else {
                    if (tok->strAt(1) == "=")
                        checkRhs(tok, var, alloc && *alloc, "");

                    // assume that variable is assigned
                    return true;
                }
            }
        }
    }

    return false;
}

bool CheckUninitVar::checkIfForWhileHead(const Token *startparentheses, const Variable& var, bool suppressErrors, bool isuninit, bool alloc, const std::string &membervar)
{
    const Token * const endpar = startparentheses->link();
    for (const Token *tok = startparentheses->next(); tok && tok != endpar; tok = tok->next()) {
        if (tok->varId() == var.declarationId()) {
            if (Token::Match(tok, "%var% . %var%")) {
                if (tok->strAt(2) == membervar) {
                    if (isMemberVariableAssignment(tok, membervar))
                        return true;

                    if (!suppressErrors && isMemberVariableUsage(tok, var.isPointer(), alloc, membervar))
                        uninitStructMemberError(tok, tok->str() + "." + membervar);
                }
                continue;
            }

            if (isVariableUsage(tok, var.isPointer(), alloc, _tokenizer->isCPP())) {
                if (!suppressErrors)
                    uninitvarError(tok, tok->str());
                else
                    continue;
            }
            return true;
        }
        if (Token::Match(tok, "sizeof|decltype|offsetof ("))
            tok = tok->next()->link();
        if ((!isuninit || !membervar.empty()) && tok->str() == "&&")
            suppressErrors = true;
    }
    return false;
}

bool CheckUninitVar::checkLoopBody(const Token *tok, const Variable& var, const bool alloc, const std::string &membervar, const bool suppressErrors)
{
    const Token *usetok = NULL;

    assert(tok->str() == "{");

    for (const Token * const end = tok->link(); tok != end; tok = tok->next()) {
        if (tok->varId() == var.declarationId()) {
            if (!membervar.empty()) {
                if (isMemberVariableAssignment(tok, membervar)) {
                    bool assign = true;
                    bool rhs = false;
                    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == "=")
                            rhs = true;
                        if (tok2->str() == ";")
                            break;
                        if (rhs && tok2->varId() == var.declarationId() && isMemberVariableUsage(tok2, var.isPointer(), alloc, membervar)) {
                            assign = false;
                            break;
                        }
                    }
                    if (assign)
                        return true;
                }

                if (Token::Match(tok, "%var% ="))
                    return true;

                if (isMemberVariableUsage(tok, var.isPointer(), alloc, membervar))
                    usetok = tok;
                else if (Token::Match(tok->previous(), "[(,] %var% [,)]"))
                    return true;
            } else {
                if (isVariableUsage(tok, var.isPointer(), alloc, _tokenizer->isCPP()))
                    usetok = tok;
                else {
                    bool assign = true;
                    if (tok->strAt(1) == "=") {
                        unsigned int indentlevel = 0; // Handle '(a=1)..'
                        for (const Token *tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                            if (tok2->varId() == var.declarationId()) {
                                assign = false;
                                break;
                            } else if (tok2->str() == "(") {
                                ++indentlevel;
                            } else if (tok2->str() == ")") {
                                if (indentlevel <= 1U)
                                    break;
                                --indentlevel;
                            }
                        }
                    }
                    if (assign)
                        return true;
                }
            }
        }

        if (Token::Match(tok, "sizeof|typeof ("))
            tok = tok->next()->link();

        if (Token::Match(tok, "asm ( %str% ) ;"))
            return true;
    }

    if (!suppressErrors && usetok) {
        if (membervar.empty())
            uninitvarError(usetok, usetok->str());
        else
            uninitStructMemberError(usetok, usetok->str() + "." + membervar);
        return true;
    }

    return false;
}

void CheckUninitVar::checkRhs(const Token *tok, const Variable &var, bool alloc, const std::string &membervar)
{
    bool rhs = false;
    unsigned int indent = 0;
    while (NULL != (tok = tok->next())) {
        if (tok->str() == "=")
            rhs = true;
        else if (rhs && tok->varId() == var.declarationId()) {
            if (membervar.empty() && isVariableUsage(tok, var.isPointer(), alloc, _tokenizer->isCPP()))
                uninitvarError(tok, tok->str());
            else if (!membervar.empty() && isMemberVariableUsage(tok, var.isPointer(), alloc, membervar))
                uninitStructMemberError(tok, tok->str() + "." + membervar);

        } else if (tok->str() == ";" || (indent==0 && tok->str() == ","))
            break;
        else if (tok->str() == "(")
            ++indent;
        else if (tok->str() == ")") {
            if (indent == 0)
                break;
            --indent;
        } else if (Token::simpleMatch(tok, "sizeof ("))
            tok = tok->next()->link();
    }
}

bool CheckUninitVar::isVariableUsage(const Token *vartok, bool pointer, bool alloc, bool cpp)
{
    if (vartok->previous()->str() == "return" && !alloc)
        return true;

    // Passing variable to typeof/__alignof__
    if (Token::Match(vartok->tokAt(-3), "typeof|__alignof__ ( * %var%"))
        return false;

    // Passing variable to function..
    if (Token::Match(vartok->previous(), "[(,] %var% [,)]") || Token::Match(vartok->tokAt(-2), "[(,] & %var% [,)]")) {
        const bool address(vartok->previous()->str() == "&");

        // locate start parentheses in function call..
        unsigned int argumentNumber = 0;
        const Token *start = vartok;
        while (start && !Token::Match(start, "[;{}(]")) {
            if (start->str() == ")")
                start = start->link();
            else if (start->str() == ",")
                ++argumentNumber;
            start = start->previous();
        }

        // is this a function call?
        if (start && Token::Match(start->previous(), "%var% (")) {
            // check how function handle uninitialized data arguments..
            const Function *func = start->previous()->function();
            if (func) {
                const Variable *arg = func->getArgumentVar(argumentNumber);
                if (arg) {
                    const Token *argStart = arg->typeStartToken();
                    while (argStart->previous() && argStart->previous()->isName())
                        argStart = argStart->previous();
                    if (!address && Token::Match(argStart, "const| struct| %type% [,)]"))
                        return true;
                    if (!address && Token::Match(argStart, "const| struct| %type% %var% [,)]"))
                        return true;
                    if (Token::Match(argStart, "const %type% & %var% [,)]"))
                        return true;
                    if (pointer && !address && !alloc && Token::Match(argStart, "struct| %type% * %var% [,)]"))
                        return true;
                    if ((pointer || address) && !alloc && Token::Match(argStart, "const struct| %type% * %var% [,)]"))
                        return true;
                    if ((pointer || address) && Token::Match(argStart, "const %type% %var% [") && Token::Match(argStart->linkAt(3), "] [,)]"))
                        return true;
                }

            } else if (Token::Match(start->previous(), "if|while|for")) {
                // control-flow statement reading the variable "by value"
                return !alloc;
            }
        }
    }

    if (Token::Match(vartok->previous(), "++|--|%cop%")) {
        if (cpp && vartok->previous()->str() == ">>") {
            // assume that variable is initialized
            return false;
        }

        // is there something like: ; "*((&var ..expr.. ="  => the variable is assigned
        if (vartok->previous()->str() == "&") {
            const Token *tok2 = vartok->tokAt(-2);
            if (tok2 && (tok2->isConstOp() || Token::Match(tok2, "[;{}(=]")))
                return false; // address of
            if (tok2 && tok2->str() == ")")
                tok2 = tok2->link()->previous();
            if (Token::Match(tok2,"[()] ( %type% *| ) &") && tok2->tokAt(2)->varId() == 0)
                return false; // cast
            while (tok2 && tok2->str() == "(")
                tok2 = tok2->previous();
            while (tok2 && tok2->str() == "*")
                tok2 = tok2->previous();
            if (Token::Match(tok2, "[;{}] *")) {
                // there is some such code before vartok: "[*]+ [(]* &"
                // determine if there is a = after vartok
                for (tok2 = vartok; tok2; tok2 = tok2->next()) {
                    if (Token::Match(tok2, "[;{}]"))
                        break;
                    if (tok2->str() == "=")
                        return false;
                }
            }
        }

        if (vartok->previous()->str() != "&" || !Token::Match(vartok->tokAt(-2), "[(,=?:]")) {
            if (alloc && vartok->previous()->str() == "*") {
                const Token *parent = vartok->previous()->astParent();
                if (parent && parent->str() == "=" && parent->astOperand1() == vartok->previous())
                    return false;
                return true;
            }
            return !alloc;
        }
    }

    if (!alloc && Token::Match(vartok->previous(), "= %var% ;|%cop%"))
        return true;

    if (Token::Match(vartok->previous(), "? %var%")) {
        // this is only variable usage if variable is either:
        // * unconditionally uninitialized
        // * used in both rhs and lhs of ':' operator
        bool rhs = false;
        for (const Token *tok2 = vartok; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(")
                tok2 = tok2->link();
            else if (tok2->str() == ":")
                rhs = true;
            else if (Token::Match(tok2, "[)];,{}=]"))
                break;
            else if (rhs && tok2->varId() == vartok->varId())
                return true;
        }
    }

    bool unknown = false;
    if (pointer && CheckNullPointer::isPointerDeRef(vartok, unknown)) {
        // pointer is allocated - dereferencing it is ok.
        if (alloc)
            return false;

        // function parameter?
        bool functionParameter = false;
        if (Token::Match(vartok->tokAt(-2), "%var% (") || vartok->previous()->str() == ",")
            functionParameter = true;

        // if this is not a function parameter report this dereference as variable usage
        if (!functionParameter)
            return true;
    }

    if (pointer && Token::Match(vartok, "%var% . %var% (")) {
        const Function *function = vartok->tokAt(2)->function();
        return (!function || !function->isStatic);
    }

    if (cpp && Token::Match(vartok->next(), "<<|>>")) {
        // Is this calculation done in rhs?
        const Token *tok = vartok;
        while (tok && Token::Match(tok, "%var%|.|::"))
            tok = tok->previous();
        if (Token::Match(tok, "[;{}]"))
            return false;

        // Is variable a known POD type then this is a variable usage,
        // otherwise we assume it's not.
        const Variable *var = vartok->variable();
        return (var && var->typeStartToken()->isStandardType());
    }

    if (!alloc && vartok->next() && vartok->next()->isOp() && !vartok->next()->isAssignmentOp())
        return true;

    if (vartok->strAt(1) == "]")
        return true;

    return false;
}

bool CheckUninitVar::isMemberVariableAssignment(const Token *tok, const std::string &membervar)
{
    if (Token::Match(tok, "%var% . %var%") && tok->strAt(2) == membervar) {
        if (Token::Match(tok->tokAt(3), "[=.[]"))
            return true;
        else if (Token::Match(tok->tokAt(-2), "[(,=] &"))
            return true;
        else if ((tok->previous() && tok->previous()->isConstOp()) || Token::Match(tok->previous(), "[|="))
            ; // member variable usage
        else if (tok->tokAt(3)->isConstOp())
            ; // member variable usage
        else
            return true;
    } else if (tok->strAt(1) == "=")
        return true;
    else if (tok->strAt(-1) == "&")
        return true;
    return false;
}

bool CheckUninitVar::isMemberVariableUsage(const Token *tok, bool isPointer, bool alloc, const std::string &membervar) const
{
    if (isMemberVariableAssignment(tok, membervar))
        return false;

    if (Token::Match(tok, "%var% . %var%") && tok->strAt(2) == membervar)
        return true;
    else if (!isPointer && Token::Match(tok->previous(), "[(,] %var% [,)]") && isVariableUsage(tok, isPointer, alloc, _tokenizer->isCPP()))
        return true;

    return false;
}

void CheckUninitVar::uninitstringError(const Token *tok, const std::string &varname, bool strncpy_)
{
    reportError(tok, Severity::error, "uninitstring", "Dangerous usage of '" + varname + "'" + (strncpy_ ? " (strncpy doesn't always null-terminate it)." : " (not null-terminated)."));
}

void CheckUninitVar::uninitdataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitdata", "Memory is allocated but not initialized: " + varname);
}

void CheckUninitVar::uninitvarError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitvar", "Uninitialized variable: " + varname);
}

void CheckUninitVar::uninitStructMemberError(const Token *tok, const std::string &membername)
{
    reportError(tok,
                Severity::error,
                "uninitStructMember",
                "Uninitialized struct member: " + membername);
}
