/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#include "checknullpointer.h"
#include "executionpath.h"
#include "mathlib.h"
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckNullPointer instance;
}

//---------------------------------------------------------------------------



/**
 * @brief parse a function call and extract information about variable usage
 * @param tok first token
 * @param var variables that the function read / write.
 * @param value 0 => invalid with null pointers as parameter.
 *              1-.. => invalid with uninitialized data.
 */
void CheckNullPointer::parseFunctionCall(const Token &tok, std::list<const Token *> &var, unsigned char value)
{
    // standard functions that dereference first parameter..
    // both uninitialized data and null pointers are invalid.
    static std::set<std::string> functionNames1;
    if (functionNames1.empty())
    {
        functionNames1.insert("memchr");
        functionNames1.insert("memcmp");
        functionNames1.insert("strcat");
        functionNames1.insert("strncat");
        functionNames1.insert("strchr");
        functionNames1.insert("strrchr");
        functionNames1.insert("strcmp");
        functionNames1.insert("strncmp");
        functionNames1.insert("strdup");
        functionNames1.insert("strndup");
        functionNames1.insert("strlen");
        functionNames1.insert("strstr");
    }

    // standard functions that dereference second parameter..
    // both uninitialized data and null pointers are invalid.
    static std::set<std::string> functionNames2;
    if (functionNames2.empty())
    {
        functionNames2.insert("memcmp");
        functionNames2.insert("memcpy");
        functionNames2.insert("memmove");
        functionNames2.insert("strcat");
        functionNames2.insert("strncat");
        functionNames2.insert("strcmp");
        functionNames2.insert("strncmp");
        functionNames2.insert("strcpy");
        functionNames2.insert("strncpy");
        functionNames2.insert("strstr");
    }

    // 1st parameter..
    if (Token::Match(&tok, "%var% ( %var% ,|)") && tok.tokAt(2)->varId() > 0)
    {
        if (functionNames1.find(tok.str()) != functionNames1.end())
            var.push_back(tok.tokAt(2));
        else if (value == 0 && Token::Match(&tok, "memchr|memcmp|memcpy|memmove|memset|strcpy|printf|sprintf|snprintf"))
            var.push_back(tok.tokAt(2));
        else if (Token::simpleMatch(&tok, "fflush"))
            var.push_back(tok.tokAt(2));
    }

    // 2nd parameter..
    if (Token::Match(&tok, "%var% ( %any% , %var% ,|)") && tok.tokAt(4)->varId() > 0)
    {
        if (functionNames2.find(tok.str()) != functionNames2.end())
            var.push_back(tok.tokAt(4));
    }
}


/**
 * Is there a pointer dereference? Everything that should result in
 * a nullpointer dereference error message will result in a true
 * return value. If it's unknown if the pointer is dereferenced false
 * is returned.
 * @param tok token for the pointer
 * @param unknown it is not known if there is a pointer dereference (could be reported as a debug message)
 * @return true => there is a dereference
 */
bool CheckNullPointer::isPointerDeRef(const Token *tok, bool &unknown)
{
    unknown = false;

    // Dereferencing pointer..
    if (Token::Match(tok->tokAt(-3), "!!sizeof [;{}=+-/(,] * %var%"))
        return true;

    if (!Token::simpleMatch(tok->tokAt(-2), "& (") && tok->strAt(-1) != "&"  && tok->strAt(-1) != "&&" && Token::Match(tok->next(), ". %var%"))
        return true;

    if (Token::Match(tok->previous(), "[;{}=+-/(,] %var% ["))
        return true;

    if (Token::Match(tok->previous(), "return %var% ["))
        return true;

    if (Token::Match(tok, "%var% ("))
        return true;

    if (Token::Match(tok, "%var% = %var% .") &&
        tok->varId() > 0 &&
        tok->varId() == tok->tokAt(2)->varId())
        return true;

    // Not a dereference..
    if (Token::Match(tok->previous(), "[;{}] %var% ="))
        return false;

    // unknown if it's a dereference
    unknown = true;

    // assume that it's not a dereference (no false positives)
    return false;
}



void CheckNullPointer::nullPointerAfterLoop()
{
    // Locate insufficient null-pointer handling after loop
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (! Token::Match(tok, "while ( %var% )"))
            continue;

        const unsigned int varid(tok->tokAt(2)->varId());
        if (varid == 0)
            continue;

        const std::string varname(tok->strAt(2));

        // Locate the end of the while loop..
        const Token *tok2 = tok->tokAt(4);
        if (tok2->str() == "{")
            tok2 = tok2->link();
        else
        {
            while (tok2 && tok2->str() != ";")
                tok2 = tok2->next();
        }

        // Goto next token
        if (tok2)
            tok2 = tok2->next();

        // Check if the variable is dereferenced..
        while (tok2)
        {
            if (tok2->str() == "{" || tok2->str() == "}" || tok2->str() == "break")
                break;

            if (tok2->varId() == varid)
            {
                bool unknown = false;
                if (CheckNullPointer::isPointerDeRef(tok2, unknown))
                {
                    // Is this variable a pointer?
                    const Token *tok3 = Token::findmatch(_tokenizer->tokens(), "%type% * %varid% [;)=]", varid);
                    if (!tok3)
                        break;

                    if (!tok3->previous() ||
                        Token::Match(tok3->previous(), "[({};]") ||
                        tok3->previous()->isName())
                    {
                        nullPointerError(tok2, varname);
                    }
                }
                break;
            }

            tok2 = tok2->next();
        }
    }
}

void CheckNullPointer::nullPointerLinkedList()
{
    // looping through items in a linked list in a inner loop..
    for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
    {
        // search for a "for" token..
        if (!Token::simpleMatch(tok1, "for ("))
            continue;

        if (!Token::simpleMatch(tok1->next()->link(), ") {"))
            continue;

        // is there any dereferencing occuring in the for statement..
        unsigned int parlevel2 = 1;
        for (const Token *tok2 = tok1->tokAt(2); tok2; tok2 = tok2->next())
        {
            // Parantheses..
            if (tok2->str() == "(")
                ++parlevel2;
            else if (tok2->str() == ")")
            {
                if (parlevel2 <= 1)
                    break;
                --parlevel2;
            }

            // Dereferencing a variable inside the "for" parantheses..
            else if (Token::Match(tok2, "%var% . %var%"))
            {
                const unsigned int varid(tok2->varId());
                if (varid == 0)
                    continue;

                if (Token::Match(tok2->tokAt(-2), "%varid% ?", varid))
                    continue;

                const std::string varname(tok2->str());

                // Check usage of dereferenced variable in the loop..
                unsigned int indentlevel3 = 0;
                for (const Token *tok3 = tok1->next()->link(); tok3; tok3 = tok3->next())
                {
                    if (tok3->str() == "{")
                        ++indentlevel3;
                    else if (tok3->str() == "}")
                    {
                        if (indentlevel3 <= 1)
                            break;
                        --indentlevel3;
                    }
                    else if (Token::Match(tok3, "while ( %varid% &&|)", varid))
                    {
                        // Make sure there is a "break" to prevent segmentation faults..
                        unsigned int indentlevel4 = indentlevel3;
                        for (const Token *tok4 = tok3->next()->link(); tok4; tok4 = tok4->next())
                        {
                            if (tok4->str() == "{")
                                ++indentlevel4;
                            else if (tok4->str() == "}")
                            {
                                if (indentlevel4 <= 1)
                                {
                                    // Is this variable a pointer?
                                    const Token *tempTok = Token::findmatch(_tokenizer->tokens(), "%type% * %varid% [;)=]", varid);
                                    if (tempTok)
                                        nullPointerError(tok1, varname, tok3->linenr());

                                    break;
                                }
                                --indentlevel4;
                            }
                            else if (tok4->str() == "break" || tok4->str() == "return")
                                break;
                        }
                    }
                }
            }
        }
    }
}

void CheckNullPointer::nullPointerStructByDeRefAndChec()
{
    // don't check vars that has been tested against null already
    std::set<unsigned int> skipvar;
    skipvar.insert(0);

    // Dereferencing a struct pointer and then checking if it's NULL..
    for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
    {
        if (Token::Match(tok1, "if|while ( !| %var% )"))
        {
            tok1 = tok1->tokAt(2);
            if (tok1->str() == "!")
                tok1 = tok1->next();
            skipvar.insert(tok1->varId());
            continue;
        }

        // dereference in assignment
        if (Token::Match(tok1, "[{};] %var% = %var% . %var%"))
        {
            if (std::string(tok1->strAt(1)) == tok1->strAt(3))
                continue;
            tok1 = tok1->tokAt(3);
        }

        // dereference in function call
        else if (Token::Match(tok1->tokAt(-2), "%var% ( %var% . %var%") ||
                 Token::Match(tok1->previous(), ", %var% . %var%"))
        {

        }

        // Goto next token
        else
        {
            continue;
        }

        // struct dereference was found - investigate if it is later
        // checked that it is not NULL
        const unsigned int varid1(tok1->varId());
        if (skipvar.find(varid1) != skipvar.end())
            continue;

        const std::string varname(tok1->str());

        unsigned int indentlevel2 = 0;
        for (const Token *tok2 = tok1->tokAt(3); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
                ++indentlevel2;

            else if (tok2->str() == "}")
            {
                if (indentlevel2 <= 1)
                    break;
                --indentlevel2;
            }

            // goto destination..
            else if (tok2->isName() && Token::simpleMatch(tok2->next(), ":"))
                break;

            // Reassignment of the struct
            else if (tok2->varId() == varid1)
            {
                if (tok2->next()->str() == "=")
                    break;
                if (Token::Match(tok2->tokAt(-2), "[,(] &"))
                    break;
            }

            // Loop..
            /** @todo don't bail out if the variable is not used in the loop */
            else if (tok2->str() == "do")
                break;

            // return at base level => stop checking
            else if (indentlevel2 == 0 && tok2->str() == "return")
                break;

            else if (Token::Match(tok2, "if ( !| %varid% )", varid1))
            {
                // Is this variable a pointer?
                const Token *tempTok = Token::findmatch(_tokenizer->tokens(), "%type% * %varid% [;)=]", varid1);
                if (tempTok)
                    nullPointerError(tok1, varname, tok2->linenr());
                break;
            }
        }
    }
}

void CheckNullPointer::nullPointerByDeRefAndChec()
{
    // Dereferencing a pointer and then checking if it's NULL..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "if" && Token::Match(tok->previous(), "; if ( ! %var% )"))
        {
            const unsigned int varid(tok->tokAt(3)->varId());
            if (varid == 0)
                continue;

            const std::string varname(tok->strAt(3));

            // Check that variable is a pointer..
            const Token *decltok = Token::findmatch(_tokenizer->tokens(), "%varid%", varid);
            if (!Token::Match(decltok->tokAt(-3), "[{};,(] %type% *"))
                continue;

            for (const Token *tok1 = tok->previous(); tok1 && tok1 != decltok; tok1 = tok1->previous())
            {
                if (tok1->varId() == varid)
                {
                    bool unknown = false;
                    if (Token::Match(tok1->tokAt(-2), "%varid% = %varid% .", varid))
                    {
                        break;
                    }
                    else if (CheckNullPointer::isPointerDeRef(tok1, unknown))
                    {
                        nullPointerError(tok1, varname, tok->linenr());
                        break;
                    }
                    else if (Token::simpleMatch(tok1->previous(), "&"))
                    {
                        break;
                    }
                    else if (Token::simpleMatch(tok1->next(), "="))
                    {
                        break;
                    }
                }

                else if (tok1->str() == "{" ||
                         tok1->str() == "}")
                    break;

                // label..
                else if (Token::Match(tok1, "%type% :"))
                    break;
            }
        }
    }
}

void CheckNullPointer::nullPointerByCheckAndDeRef()
{
    // Check if pointer is NULL and then dereference it..
    std::set<unsigned int> pointerVariables;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "* %var% [;,)=]"))
            pointerVariables.insert(tok->next()->varId());

        else if (Token::Match(tok, "if ("))
        {
            const Token *vartok = 0;
            if (Token::Match(tok, "if ( ! %var% ) {"))
                vartok = tok->tokAt(3);
            else if (Token::Match(tok, "if ( NULL|0 == %var% ) {"))
                vartok = tok->tokAt(4);
            else if (Token::Match(tok, "if ( %var% == NULL|0 ) {"))
                vartok = tok->tokAt(2);
            else
                continue;

            bool null = true;
            const unsigned int varid(vartok->varId());
            if (varid == 0)
                continue;
            if (pointerVariables.find(varid) == pointerVariables.end())
                continue;

            // Name of the pointer
            const std::string &pointerName = vartok->str();

            unsigned int indentlevel = 1;
            for (const Token *tok2 = tok->next()->link()->tokAt(2); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == "}")
                {
                    if (indentlevel == 0)
                        break;
                    --indentlevel;
                    if (null && indentlevel == 0)
                    {
                        // skip all "else" blocks because they are not executed in this execution path
                        while (Token::Match(tok2, "} else {"))
                            tok2 = tok2->tokAt(2)->link();
                        null = false;
                    }
                }

                if (Token::Match(tok2, "goto|return|continue|break|throw|if"))
                {
                    if (Token::Match(tok2, "return * %varid%", varid))
                        nullPointerError(tok2, tok->strAt(3));
                    break;
                }

                // parameters to sizeof are not dereferenced
                if (Token::Match(tok2, "decltype|sizeof ("))
                {
                    tok2 = tok2->next()->link();
                    continue;
                }

                // abort function..
                if (Token::simpleMatch(tok2, ") ; }") &&
                    Token::Match(tok2->link()->tokAt(-2), "[;{}] %var% ("))
                {
                    break;
                }

                if (tok2->varId() == varid)
                {
                    bool unknown = false;

                    if (Token::Match(tok2->previous(), "[;{}=] %var% = 0 ;"))
                        ;

                    else if (CheckNullPointer::isPointerDeRef(tok2, unknown))
                        nullPointerError(tok2, pointerName);

                    else
                        break;
                }
            }
        }
    }
}


void CheckNullPointer::nullPointer()
{
    nullPointerAfterLoop();
    nullPointerLinkedList();
    nullPointerStructByDeRefAndChec();
    nullPointerByDeRefAndChec();
    nullPointerByCheckAndDeRef();
}

/** Derefencing null constant (simplified token list) */
void CheckNullPointer::nullConstantDereference()
{
    // this is kept at 0 for all scopes that are not executing
    unsigned int indentlevel = 0;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // start of executable scope..
        if (indentlevel == 0 && Token::Match(tok, ") const| {"))
            indentlevel = 1;

        else if (indentlevel >= 1)
        {
            if (tok->str() == "{")
                ++indentlevel;

            else if (tok->str() == "}")
            {
                if (indentlevel <= 2)
                    indentlevel = 0;
                else
                    --indentlevel;
            }

            if (tok->str() == "(" && Token::Match(tok->previous(), "sizeof|decltype"))
                tok = tok->link();

            else if (Token::simpleMatch(tok, "exit ( )"))
            {
                // Goto end of scope
                while (tok && tok->str() != "}")
                {
                    if (tok->str() == "{")
                        tok = tok->link();
                    tok = tok->next();
                }
                if (!tok)
                    break;
            }

            else if (Token::simpleMatch(tok, "* 0"))
            {
                if (Token::Match(tok->previous(), "[<>;{}=+-*/(,]") ||
                    Token::Match(tok->previous(), "return|<<"))
                {
                    nullPointerError(tok);
                }
            }
        }
    }
}


/// @addtogroup Checks
/// @{


/**
 * @brief %Check for null pointer usage (using ExecutionPath)
 */

class Nullpointer : public ExecutionPath
{
public:
    /** Startup constructor */
    Nullpointer(Check *c) : ExecutionPath(c, 0), null(false)
    {
    }

private:
    /** Create checking of specific variable: */
    Nullpointer(Check *c, const unsigned int id, const std::string &name)
        : ExecutionPath(c, id),
          varname(name),
          null(false)
    {
    }

    /** Copy this check */
    ExecutionPath *copy()
    {
        return new Nullpointer(*this);
    }

    /** no implementation => compiler error if used by accident */
    void operator=(const Nullpointer &);

    /** is other execution path equal? */
    bool is_equal(const ExecutionPath *e) const
    {
        const Nullpointer *c = static_cast<const Nullpointer *>(e);
        return (varname == c->varname && null == c->null);
    }

    /** variable name for this check (empty => dummy check) */
    const std::string varname;

    /** is this variable null? */
    bool null;

    /** variable is set to null */
    static void setnull(std::list<ExecutionPath *> &checks, const unsigned int varid)
    {
        std::list<ExecutionPath *>::iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            Nullpointer *c = dynamic_cast<Nullpointer *>(*it);
            if (c && c->varId == varid)
                c->null = true;
        }
    }

    /**
     * Dereferencing variable. Check if it is safe (if the variable is null there's an error)
     * @param checks Checks
     * @param tok token where dereferencing happens
     */
    static void dereference(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());

        std::list<ExecutionPath *>::iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            Nullpointer *c = dynamic_cast<Nullpointer *>(*it);
            if (c && c->varId == varid && c->null)
            {
                CheckNullPointer *checkNullPointer = dynamic_cast<CheckNullPointer *>(c->owner);
                if (checkNullPointer)
                {
                    checkNullPointer->nullPointerError(tok, c->varname);
                    return;
                }
            }
        }
    }

    /** parse tokens */
    const Token *parse(const Token &tok, std::list<ExecutionPath *> &checks) const
    {
        if (Token::Match(tok.previous(), "[;{}] const| %type% * %var% ;"))
        {
            const Token * vartok = tok.tokAt(2);

            if (tok.str() == "const")
                vartok = vartok->next();

            if (vartok->varId() != 0)
                checks.push_back(new Nullpointer(owner, vartok->varId(), vartok->str()));
            return vartok->next();
        }

        // Template pointer variable..
        if (Token::Match(tok.previous(), "[;{}] %type% ::|<"))
        {
            const Token * vartok = &tok;
            while (Token::Match(vartok, "%type% ::"))
                vartok = vartok->tokAt(2);
            if (Token::Match(vartok, "%type% < %type%"))
            {
                vartok = vartok->tokAt(3);
                while (vartok && (vartok->str() == "*" || vartok->isName()))
                    vartok = vartok->next();
            }
            if (vartok
                && (vartok->str() == ">" || vartok->isName())
                && Token::Match(vartok->next(), "* %var% ;|="))
            {
                vartok = vartok->tokAt(2);
                checks.push_back(new Nullpointer(owner, vartok->varId(), vartok->str()));
                if (Token::simpleMatch(vartok->next(), "= 0 ;"))
                    setnull(checks, vartok->varId());
                return vartok->next();
            }
        }

        if (Token::simpleMatch(&tok, "try {"))
        {
            // Bail out all used variables
            unsigned int indentlevel = 0;
            for (const Token *tok2 = &tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == "}")
                {
                    if (indentlevel == 0)
                        break;
                    if (indentlevel == 1 && !Token::simpleMatch(tok2,"} catch ("))
                        return tok2;
                    --indentlevel;
                }
                else if (tok2->varId())
                    bailOutVar(checks,tok2->varId());
            }
        }

        if (Token::Match(&tok, "%var% ("))
        {
            if (tok.str() == "sizeof")
                return tok.next()->link();

            // parse usage..
            std::list<const Token *> var;
            CheckNullPointer::parseFunctionCall(tok, var, 0);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                dereference(checks, *it);
        }

        if (tok.varId() != 0)
        {
            bool unknown = false;
            if (Token::Match(tok.previous(), "[;{}=] %var% = 0 ;"))
                setnull(checks, tok.varId());
            else if (CheckNullPointer::isPointerDeRef(&tok, unknown))
                dereference(checks, &tok);
            else
                // TODO: Report debug warning that it's unknown if a
                // pointer is dereferenced
                bailOutVar(checks, tok.varId());
        }

        else if (tok.str() == "delete")
        {
            const Token *ret = tok.next();
            if (Token::simpleMatch(ret, "[ ]"))
                ret = ret->tokAt(2);
            if (Token::Match(ret, "%var% ;"))
                return ret->next();
        }

        return &tok;
    }

    /** parse condition. @sa ExecutionPath::parseCondition */
    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks)
    {
        for (const Token *tok2 = &tok; tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "(" || tok2->str() == ")")
                break;
            if (Token::Match(tok2, "[<>=] * %var%"))
                dereference(checks, tok2->tokAt(2));
        }

        if (Token::Match(&tok, "!| %var% ("))
        {
            std::list<const Token *> var;
            CheckNullPointer::parseFunctionCall(tok.str() == "!" ? *tok.next() : tok, var, 0);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                dereference(checks, *it);
        }

        return ExecutionPath::parseCondition(tok, checks);
    }


    void parseLoopBody(const Token *tok, std::list<ExecutionPath *> &checks) const
    {
        while (tok)
        {
            if (Token::Match(tok, "{|}|return|goto|break|if"))
                return;
            const Token *next = parse(*tok, checks);
            if (next)
                tok = tok->next();
        }
    }

};
/// @}


void CheckNullPointer::executionPaths()
{
    // Check for null pointer errors..
    Nullpointer c(this);
    checkExecutionPaths(_tokenizer->tokens(), &c);
}

void CheckNullPointer::nullPointerError(const Token *tok)
{
    reportError(tok, Severity::error, "nullPointer", "Null pointer dereference");
}

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "nullPointer", "Possible null pointer dereference: " + varname);
}

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname, const unsigned int line)
{
    reportError(tok, Severity::error, "nullPointer", "Possible null pointer dereference: " + varname + " - otherwise it is redundant to check if " + varname + " is null at line " + MathLib::toString<unsigned int>(line));
}

