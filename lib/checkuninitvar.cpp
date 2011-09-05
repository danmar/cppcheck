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


//---------------------------------------------------------------------------
#include "checkuninitvar.h"
#include "mathlib.h"
#include "executionpath.h"
#include "checknullpointer.h"   // CheckNullPointer::parseFunctionCall
#include <algorithm>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckUninitVar instance;
}

//---------------------------------------------------------------------------

/// @addtogroup Checks
/// @{

/**
 * @brief %Check that uninitialized variables aren't used (using ExecutionPath)
 * */
class UninitVar : public ExecutionPath
{
public:
    /** Startup constructor */
    UninitVar(Check *c)
        : ExecutionPath(c, 0), pointer(false), array(false), alloc(false), strncpy_(false), memset_nonzero(false)
    {
    }

private:
    /** Create a copy of this check */
    ExecutionPath *copy()
    {
        return new UninitVar(*this);
    }

    /** no implementation => compiler error if used */
    void operator=(const UninitVar &);

    /** internal constructor for creating extra checks */
    UninitVar(Check *c, unsigned int v, const std::string &name, bool p, bool a)
        : ExecutionPath(c, v), varname(name), pointer(p), array(a), alloc(false), strncpy_(false), memset_nonzero(false)
    {
    }

    /** is other execution path equal? */
    bool is_equal(const ExecutionPath *e) const
    {
        const UninitVar *c = static_cast<const UninitVar *>(e);
        return (varname == c->varname && pointer == c->pointer && array == c->array && alloc == c->alloc && strncpy_ == c->strncpy_ && memset_nonzero == c->memset_nonzero);
    }

    /** variable name for this check */
    const std::string varname;

    /** is this variable a pointer? */
    const bool pointer;

    /** is this variable an array? */
    const bool array;

    /** is this variable allocated? */
    bool  alloc;

    /** is this variable initialized with strncpy (not always zero-terminated) */
    bool  strncpy_;

    /** is this variable initialized but not zero-terminated (memset) */
    bool  memset_nonzero;

    /** allocating pointer. For example : p = malloc(10); */
    static void alloc_pointer(std::list<ExecutionPath *> &checks, unsigned int varid)
    {
        // loop through the checks and perform a allocation if the
        // variable id matches
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid)
                c->alloc = true;
        }
    }

    /** Initializing a pointer value. For example: *p = 0; */
    static void init_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        // loop through the checks and perform a initialization if the
        // variable id matches
        std::list<ExecutionPath *>::iterator it = checks.begin();
        while (it != checks.end())
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid)
            {
                if (c->alloc || c->array)
                {
                    delete c;
                    checks.erase(it++);
                    continue;
                }
                else
                {
                    use_pointer(checks, tok);
                }
            }

            ++it;
        }
    }

    /** Deallocate a pointer. For example: free(p); */
    static void dealloc_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        // loop through the checks and perform a deallocation if the
        // variable id matches
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid)
            {
                // unallocated pointer variable => error
                if (c->pointer && !c->alloc)
                {
                    CheckUninitVar *checkUninitVar = dynamic_cast<CheckUninitVar *>(c->owner);
                    if (checkUninitVar)
                    {
                        checkUninitVar->uninitvarError(tok, c->varname);
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
    static void pointer_assignment(std::list<ExecutionPath *> &checks, const Token *tok1, const Token *tok2)
    {
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
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid1 && c->pointer)
            {
                bailOutVar(checks, varid1);
                break;
            }
        }

        // bail out if second variable is a array/pointer
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid2 && (c->pointer || c->array))
            {
                bailOutVar(checks, varid2);
                break;
            }
        }
    }


    /** Initialize an array with strncpy. */
    static void init_strncpy(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid)
            {
                c->strncpy_ = true;
            }
        }
    }

    /** Initialize an array with memset (not zero). */
    static void init_memset_nonzero(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid)
            {
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
    static bool use(std::list<ExecutionPath *> &checks, const Token *tok, const int mode)
    {
        const unsigned int varid(tok->varId());
        if (varid == 0)
            return false;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            UninitVar *c = dynamic_cast<UninitVar *>(*it);
            if (c && c->varId == varid)
            {
                // mode 0 : the variable is used "directly"
                // example: .. = var;
                // it is ok to read the address of an uninitialized array.
                // it is ok to read the address of an allocated pointer
                if (mode == 0 && (c->array || (c->pointer && c->alloc)))
                    continue;

                // mode 2 : bad usage of pointer. if it's not a pointer then the usage is ok.
                // example: ptr->foo();
                if (mode == 2 && !c->pointer)
                    continue;

                // mode 3 : using dead pointer is invalid.
                if (mode == 3 && (!c->pointer || c->alloc))
                    continue;

                // mode 4 : reading uninitialized array or pointer is invalid.
                if (mode == 4 && (!c->array && !c->pointer))
                    continue;

                CheckUninitVar *checkUninitVar = dynamic_cast<CheckUninitVar *>(c->owner);
                if (checkUninitVar)
                {
                    if (c->strncpy_ || c->memset_nonzero)
                        checkUninitVar->uninitstringError(tok, c->varname, c->strncpy_);
                    else if (c->pointer && c->alloc)
                        checkUninitVar->uninitdataError(tok, c->varname);
                    else
                        checkUninitVar->uninitvarError(tok, c->varname);
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
    static bool use(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 0);
    }

    /**
     * Reading array elements. If the variable is not an array then the usage is ok.
     * @param checks all available checks
     * @param tok variable token
     */
    static void use_array(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        use(checks, tok, 1);
    }

    /**
     * Bad pointer usage. If the variable is not a pointer then the usage is ok.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 2);
    }

    /**
     * Using variable.. if it's a dead pointer the usage is invalid.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_dead_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 3);
    }

    /**
     * Using variable.. reading from uninitialized array or pointer data is invalid.
     * Example: = x[0];
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_array_or_pointer_data(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 4);
    }


    /** declaring a variable */
    void declare(std::list<ExecutionPath *> &checks, const Token *vartok, const Token &tok, const bool p, const bool a) const
    {
        if (vartok->varId() == 0)
            return;

        // Suppress warnings if variable in inner scope has same name as variable in outer scope
        if (!tok.isStandardType())
        {
            std::set<unsigned int> dup;
            for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it)
            {
                UninitVar *c = dynamic_cast<UninitVar *>(*it);
                if (c && c->varname == vartok->str() && c->varId != vartok->varId())
                    dup.insert(c->varId);
            }
            if (!dup.empty())
            {
                for (std::set<unsigned int>::const_iterator it = dup.begin(); it != dup.end(); ++it)
                    bailOutVar(checks, *it);
                return;
            }
        }

        if (a || p || tok.isStandardType())
            checks.push_back(new UninitVar(owner, vartok->varId(), vartok->str(), p, a));
    }

    /**
     * Parse right hand side expression in statement
     * @param tok2 start token of rhs
     * @param checks the execution paths
     */
    void parserhs(const Token *tok2, std::list<ExecutionPath *> &checks) const
    {
        // check variable usages in rhs/index
        while (NULL != (tok2 = tok2->next()))
        {
            if (Token::Match(tok2, "[;)=?]"))
                break;
            if (Token::Match(tok2, "%var% ("))
                break;
            if (tok2->varId() &&
                !Token::Match(tok2->previous(), "&|::") &&
                !Token::simpleMatch(tok2->tokAt(-2), "& (") &&
                !Token::simpleMatch(tok2->next(), "="))
            {
                // Multiple assignments..
                if (Token::Match(tok2->next(), ".|["))
                {
                    const Token * tok3 = tok2;
                    while (tok3)
                    {
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
                if (foundError)
                {
                    bailOutVar(checks, tok2->varId());
                }
            }
        }

    }

    /** parse tokens. @sa ExecutionPath::parse */
    const Token *parse(const Token &tok, std::list<ExecutionPath *> &checks) const
    {
        // Variable declaration..
        if (Token::Match(tok.previous(), "[;{}] %var%") && tok.str() != "return")
        {
            if (Token::Match(&tok, "enum %type% {"))
                return tok.tokAt(2)->link();

            const Token * vartok = &tok;
            while (Token::Match(vartok, "const|struct"))
                vartok = vartok->next();

            if (Token::Match(vartok, "%type% *| %var% ;"))
            {
                vartok = vartok->next();
                const bool p(vartok->str() == "*");
                if (p)
                    vartok = vartok->next();
                declare(checks, vartok, tok, p, false);
                return vartok;
            }

            // Variable declaration for array..
            if (Token::Match(vartok, "%type% %var% [") &&
                vartok->isStandardType() &&
                Token::simpleMatch(vartok->tokAt(2)->link(), "] ;"))
            {
                vartok = vartok->next();
                declare(checks, vartok, tok, false, true);
                return vartok->next()->link();
            }

            // Template pointer variable..
            if (Token::Match(vartok, "%type% ::|<"))
            {
                while (Token::Match(vartok, "%type% ::"))
                    vartok = vartok->tokAt(2);
                if (Token::Match(vartok, "%type% < %type%"))
                {
                    vartok = vartok->tokAt(3);
                    while (vartok && (vartok->str() == "*" || vartok->isName()))
                        vartok = vartok->next();
                    if (Token::Match(vartok, "> * %var% ;"))
                    {
                        declare(checks, vartok->tokAt(2), tok, true, false);
                        return vartok->tokAt(2);
                    }
                }
            }
        }

        if (tok.str() == "return")
        {
            // is there assignment in the return statement?
            bool assignment = false;
            for (const Token *tok2 = tok.next(); tok2 && tok2->str() != ";"; tok2 = tok2->next())
            {
                if (tok2->str() == "=" || tok2->str() == ">>")
                {
                    assignment = true;
                    break;
                }
            }

            if (!assignment)
            {
                for (const Token *tok2 = tok.next(); tok2 && tok2->str() != ";"; tok2 = tok2->next())
                {
                    if (tok2->isName() && tok2->strAt(1) == "(")
                        tok2 = tok2->next()->link();

                    else if (tok2->varId())
                        use(checks, tok2);
                }
            }
        }

        if (tok.varId())
        {
            // array variable passed as function parameter..
            if (Token::Match(tok.previous(), "[(,] %var% [+-,)]"))
            {
                use(checks, &tok);
                //use_array(checks, &tok);
                return &tok;
            }

            // Used..
            if (Token::Match(tok.previous(), "[[(,+-*/|=] %var% ]|)|,|;|%op%"))
            {
                // initialize reference variable
                if (Token::Match(tok.tokAt(-3), "& %var% ="))
                    bailOutVar(checks, tok.varId());
                else
                    use(checks, &tok);
                return &tok;
            }

            if (Token::Match(tok.previous(), "++|--") || Token::Match(tok.next(), "++|--"))
            {
                use(checks, &tok);
                return &tok;
            }

            if (Token::Match(tok.previous(), "[;{}] %var% [=[.]"))
            {
                if (tok.next()->str() == ".")
                {
                    if (use_dead_pointer(checks, &tok))
                    {
                        return &tok;
                    }
                }
                else
                {
                    const Token *tok2 = tok.next();
                    if (tok2->str() == "[" && Token::simpleMatch(tok2->link(), "] ="))
                    {
                        if (use_dead_pointer(checks, &tok))
                        {
                            return &tok;
                        }

                        parserhs(tok2, checks);
                        tok2 = tok2->link()->next();
                    }
                    parserhs(tok2, checks);
                }

                // pointer aliasing?
                if (Token::Match(tok.tokAt(2), "%var% ;"))
                {
                    pointer_assignment(checks, &tok, tok.tokAt(2));
                }
            }

            if (Token::simpleMatch(tok.next(), "("))
            {
                use_pointer(checks, &tok);
            }

            if (Token::Match(tok.tokAt(-2), "[;{}] *"))
            {
                if (Token::simpleMatch(tok.next(), "="))
                {
                    // is the pointer used in the rhs?
                    bool used = false;
                    for (const Token *tok2 = tok.tokAt(2); tok2; tok2 = tok2->next())
                    {
                        if (Token::Match(tok2, "[,;=(]"))
                            break;
                        else if (Token::Match(tok2, "* %varid%", tok.varId()))
                        {
                            used = true;
                            break;
                        }
                    }
                    if (used)
                        use_pointer(checks, &tok);
                    else
                        init_pointer(checks, &tok);
                }
                else
                {
                    use_pointer(checks, &tok);
                }
                return &tok;
            }

            if (Token::Match(tok.next(), "= malloc|kmalloc") || Token::simpleMatch(tok.next(), "= new char ["))
            {
                alloc_pointer(checks, tok.varId());
                if (tok.tokAt(3)->str() == "(")
                    return tok.tokAt(3);
            }

            else if (Token::Match(tok.previous(), "<<|>>") || Token::simpleMatch(tok.next(), "="))
            {
                // TODO: Don't bail out for "<<" and ">>" if these are
                // just computations
                ExecutionPath::bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (Token::simpleMatch(tok.next(), "["))
            {
                const Token *tok2 = tok.next()->link();
                if (Token::simpleMatch(tok2 ? tok2->next() : 0, "="))
                {
                    ExecutionPath::bailOutVar(checks, tok.varId());
                    return &tok;
                }
            }

            if (Token::simpleMatch(tok.previous(), "delete") ||
                Token::simpleMatch(tok.tokAt(-3), "delete [ ]"))
            {
                dealloc_pointer(checks, &tok);
                return &tok;
            }
        }

        if (Token::Match(&tok, "%var% (") && uvarFunctions.find(tok.str()) == uvarFunctions.end())
        {
            // sizeof/typeof doesn't dereference. A function name that is all uppercase
            // might be an unexpanded macro that uses sizeof/typeof
            if (Token::Match(&tok, "sizeof|typeof ("))
                return tok.next()->link();

            // deallocate pointer
            if (Token::Match(&tok, "free|kfree|fclose ( %var% )"))
            {
                dealloc_pointer(checks, tok.tokAt(2));
                return tok.tokAt(3);
            }

            // parse usage..
            {
                std::list<const Token *> var;
                CheckNullPointer::parseFunctionCall(tok, var, 1);
                for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                {
                    use_array(checks, *it);
                    use_dead_pointer(checks, *it);
                }

                // Using uninitialized pointer is bad if using null pointer is bad
                std::list<const Token *> var2;
                CheckNullPointer::parseFunctionCall(tok, var2, 0);
                for (std::list<const Token *>::const_iterator it = var2.begin(); it != var2.end(); ++it)
                {
                    if (std::find(var.begin(), var.end(), *it) == var.end())
                        use_dead_pointer(checks, *it);
                }
            }

            // strncpy doesn't 0-terminate first parameter
            if (Token::Match(&tok, "strncpy ( %var% ,"))
            {
                if (Token::Match(tok.tokAt(4), "%str% ,"))
                {
                    if (Token::Match(tok.tokAt(6), "%num% )"))
                    {
                        const std::size_t len = Token::getStrLength(tok.tokAt(4));
                        const MathLib::bigint sz = MathLib::toLongNumber(tok.strAt(6));
                        if (sz >= 0 && len >= static_cast<unsigned long>(sz))
                        {
                            init_strncpy(checks, tok.tokAt(2));
                            return tok.next()->link();
                        }
                    }
                }
                else
                {
                    init_strncpy(checks, tok.tokAt(2));
                    return tok.next()->link();
                }
            }

            // memset (not zero terminated)..
            if (Token::Match(&tok, "memset ( %var% , !!0 , %num% )"))
            {
                init_memset_nonzero(checks, tok.tokAt(2));
                return tok.next()->link();
            }

            if (Token::simpleMatch(&tok, "asm ( )"))
            {
                ExecutionPath::bailOut(checks);
                return &tok;
            }

            // is the variable passed as a parameter to some function?
            unsigned int parlevel = 0;
            std::set<unsigned int> bailouts;
            for (const Token *tok2 = tok.next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    ++parlevel;

                else if (tok2->str() == ")")
                {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }

                else if (Token::Match(tok2, "sizeof|typeof ("))
                {
                    tok2 = tok2->next()->link();
                    if (!tok2)
                        break;
                }

                // ticket #2367 : unexpanded macro that uses sizeof|typeof?
                else if (Token::Match(tok2, "%type% (") && CheckNullPointer::isUpper(tok2->str()))
                {
                    tok2 = tok2->next()->link();
                    if (!tok2)
                        break;
                }

                else if (tok2->varId())
                {
                    if (Token::Match(tok2->tokAt(-2), "[(,] *") || Token::Match(tok2->next(), ". %var%"))
                    {
                        // find function call..
                        const Token *functionCall = tok2;
                        while (0 != (functionCall = functionCall ? functionCall->previous() : 0))
                        {
                            if (functionCall->str() == "(")
                                break;
                            if (functionCall->str() == ")")
                                functionCall = functionCall->link();
                        }

                        functionCall = functionCall ? functionCall->previous() : 0;
                        if (functionCall)
                        {
                            if (functionCall->isName() && !CheckNullPointer::isUpper(functionCall->str()) && use_dead_pointer(checks, tok2))
                                ExecutionPath::bailOutVar(checks, tok2->varId());
                        }
                    }

                    // it is possible that the variable is initialized here
                    if (Token::Match(tok2->previous(), "[(,] %var% [,)]"))
                        bailouts.insert(tok2->varId());

                    // array initialization..
                    if (Token::Match(tok2->previous(), "[,(] %var% [+-]"))
                    {
                        // if var is array, bailout
                        for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it)
                        {
                            if ((*it)->varId == tok2->varId())
                            {
                                const UninitVar *c = dynamic_cast<const UninitVar *>(*it);
                                if (c && (c->array || (c->pointer && c->alloc)))
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
        if (Token::Match(&tok, "( * %var% ) ("))
        {
            // is the variable passed as a parameter to some function?
            unsigned int parlevel = 0;
            for (const Token *tok2 = tok.link()->next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    ++parlevel;

                else if (tok2->str() == ")")
                {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }

                else if (tok2->varId())
                {
                    // it is possible that the variable is initialized here
                    ExecutionPath::bailOutVar(checks, tok2->varId());
                }
            }
        }

        if (tok.str() == "return")
        {
            // Todo: if (!array && ..
            if (Token::Match(tok.next(), "%var% ;"))
            {
                use(checks, tok.next());
            }
            else if (Token::Match(tok.next(), "%var% ["))
            {
                use_array_or_pointer_data(checks, tok.next());
            }
        }

        if (tok.varId())
        {
            if (Token::simpleMatch(tok.previous(), "="))
            {
                if (Token::Match(tok.tokAt(-3), "& %var% ="))
                {
                    bailOutVar(checks, tok.varId());
                    return &tok;
                }

                if (!Token::Match(tok.tokAt(-3), ". %var% ="))
                {
                    if (!Token::Match(tok.tokAt(-3), "[;{}] %var% ="))
                    {
                        use(checks, &tok);
                        return &tok;
                    }

                    const unsigned int varid2 = tok.tokAt(-2)->varId();
                    if (varid2)
                    {
                        {
                            use(checks, &tok);
                            return &tok;
                        }
                    }
                }
            }

            if (Token::simpleMatch(tok.next(), "."))
            {
                bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (Token::simpleMatch(tok.next(), "["))
            {
                ExecutionPath::bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (Token::Match(tok.tokAt(-2), "[,(=] *"))
            {
                use_pointer(checks, &tok);
                return &tok;
            }

            if (Token::simpleMatch(tok.previous(), "&"))
            {
                ExecutionPath::bailOutVar(checks, tok.varId());
            }
        }

        // Parse "for"
        if (Token::Match(&tok, "[;{}] for ("))
        {
            // initialized variables
            std::set<unsigned int> varid1;
            varid1.insert(0);

            // Parse token
            const Token *tok2;

            // parse setup
            for (tok2 = tok.tokAt(3); tok2 != tok.link(); tok2 = tok2->next())
            {
                if (tok2->str() == ";")
                    break;
                if (tok2->varId())
                    varid1.insert(tok2->varId());
            }
            if (tok2 == tok.link())
                return &tok;

            // parse condition
            if (Token::Match(tok2, "; %var% <|<=|>=|> %num% ;"))
            {
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
                Token::Match(tok2, "; %var% ++|-- ) {"))
            {
                // get id of variable..
                unsigned int varid = tok2->next()->varId();
                if (!varid)
                    varid = tok2->tokAt(2)->varId();

                // Check that the variable hasn't been initialized and
                // that it isn't initialized in the body..
                if (varid1.find(varid) == varid1.end())
                {
                    unsigned int indentlevel = 0;
                    for (const Token *tok3 = tok2->tokAt(5); tok3; tok3 = tok3->next())
                    {
                        if (tok3->str() == "{")
                            ++indentlevel;
                        else if (tok3->str() == "}")
                        {
                            if (indentlevel == 0)
                                break;
                            --indentlevel;
                        }
                        if (tok3->varId() == varid)
                        {
                            varid = 0;  // variable is used.. maybe it's initialized. clear the variable id.
                            break;
                        }
                    }

                    // If the variable isn't initialized in the body call "use"
                    if (varid != 0)
                    {
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

    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks)
    {
        if (tok.varId() && Token::Match(&tok, "%var% <|<=|==|!=|)"))
            use(checks, &tok);

        else if (Token::Match(&tok, "!| %var% ["))
            use_array_or_pointer_data(checks, tok.str() == "!" ? tok.next() : &tok);

        else if (Token::Match(&tok, "!| %var% ("))
        {
            std::list<const Token *> var;
            CheckNullPointer::parseFunctionCall(tok.str() == "!" ? *tok.next() : tok, var, 1);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                use_array(checks, *it);
        }

        else if (Token::Match(&tok, "! %var% )"))
        {
            use(checks, &tok);
            return false;
        }

        return ExecutionPath::parseCondition(tok, checks);
    }

    void parseLoopBody(const Token *tok, std::list<ExecutionPath *> &checks) const
    {
        while (tok)
        {
            if (tok->str() == "{" || tok->str() == "}" || tok->str() == "for")
                return;
            if (Token::simpleMatch(tok, "if ("))
            {
                // bail out all variables that are used in the condition
                unsigned int parlevel = 0;
                for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "(")
                        ++parlevel;
                    else if (tok2->str() == ")")
                    {
                        if (parlevel == 0)
                            break;
                        --parlevel;
                    }
                    else if (tok2->varId())
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

    static void analyseFunctions(const Token * const tokens, std::set<std::string> &func)
    {
        for (const Token *tok = tokens; tok; tok = tok->next())
        {
            if (tok->str() == "{")
            {
                tok = tok->link();
                continue;
            }
            if (tok->str() != "::" && Token::Match(tok->next(), "%var% ( %type%"))
            {
                if (!Token::Match(tok->tokAt(2)->link(), ") [{;]"))
                    continue;
                const Token *tok2 = tok->tokAt(3);
                while (tok2 && tok2->str() != ")")
                {
                    if (tok2->str() == ",")
                        tok2 = tok2->next();

                    if (Token::Match(tok2, "%type% %var% ,|)") && tok2->isStandardType())
                    {
                        tok2 = tok2->tokAt(2);
                        continue;
                    }

                    if (tok2->isStandardType() && Token::Match(tok2, "%type% & %var% ,|)"))
                    {
                        const unsigned int varid(tok2->tokAt(2)->varId());

                        // flags for read/write
                        bool r = false, w = false;

                        // check how the variable is used in the function
                        unsigned int indentlevel = 0;
                        for (const Token *tok3 = tok2; tok3; tok3 = tok3->next())
                        {
                            if (tok3->str() == "{")
                                ++indentlevel;
                            else if (tok3->str() == "}")
                            {
                                if (indentlevel <= 1)
                                    break;
                                --indentlevel;
                            }
                            else if (indentlevel == 0 && tok3->str() == ";")
                                break;
                            else if (indentlevel >= 1 && tok3->varId() == varid)
                            {
                                if (Token::Match(tok3->previous(), "++|--") ||
                                    Token::Match(tok3->next(), "++|--"))
                                {
                                    r = true;
                                }

                                else
                                {
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

                    if (Token::Match(tok2, "const %type% &|*| const| %var% ,|)") && tok2->next()->isStandardType())
                    {
                        tok2 = tok2->tokAt(3);
                        while (tok2->isName())
                            tok2 = tok2->next();
                        continue;
                    }

                    if (Token::Match(tok2, "const %type% %var% [ ] ,|)") && tok2->next()->isStandardType())
                    {
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


void CheckUninitVar::analyse(const Token * const tokens, std::set<std::string> &func) const
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

        UninitVar c(this);
        checkExecutionPaths(_tokenizer->tokens(), &c);
    }
}

void CheckUninitVar::uninitstringError(const Token *tok, const std::string &varname, bool strncpy_)
{
    reportError(tok, Severity::error, "uninitstring", "Dangerous usage of '" + varname + "'" + (strncpy_ ? " (strncpy doesn't always 0-terminate it)" : " (not 0-terminated)"));
}

void CheckUninitVar::uninitdataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitdata", "Data is allocated but not initialized: " + varname);
}

void CheckUninitVar::uninitvarError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitvar", "Uninitialized variable: " + varname);
}
