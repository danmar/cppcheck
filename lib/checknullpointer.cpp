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
#include "checknullpointer.h"
#include "executionpath.h"
#include "mathlib.h"
#include "symboldatabase.h"
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckNullPointer instance;
}

//---------------------------------------------------------------------------


/** Is string uppercase? */
bool CheckNullPointer::isUpper(const std::string &str)
{
    for (unsigned int i = 0; i < str.length(); ++i)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            return false;
    }
    return true;
}

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
        functionNames1.insert("fclose");
        functionNames1.insert("feof");
        functionNames1.insert("fwrite");
        functionNames1.insert("fseek");
        functionNames1.insert("ftell");
        functionNames1.insert("fgetpos");
        functionNames1.insert("fsetpos");
        functionNames1.insert("rewind");
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
    if ((Token::Match(&tok, "%var% ( %var% ,|)") && tok.tokAt(2)->varId() > 0) ||
        (value == 0 && Token::Match(&tok, "%var% ( 0 ,|)")))
    {
        if (functionNames1.find(tok.str()) != functionNames1.end())
            var.push_back(tok.tokAt(2));
        else if (value == 0 && Token::Match(&tok, "memchr|memcmp|memcpy|memmove|memset|strcpy|printf|sprintf"))
            var.push_back(tok.tokAt(2));
        else if (value == 0 && Token::simpleMatch(&tok, "snprintf") && tok.strAt(4) != "0")
            var.push_back(tok.tokAt(2));
        else if (value != 0 && Token::simpleMatch(&tok, "fflush"))
            var.push_back(tok.tokAt(2));
    }

    // 2nd parameter..
    if ((Token::Match(&tok, "%var% ( %any% , %var% ,|)") && tok.tokAt(4)->varId() > 0) ||
        (value == 0 && Token::Match(&tok, "%var% ( %any% , 0 ,|)")))
    {
        if (functionNames2.find(tok.str()) != functionNames2.end())
            var.push_back(tok.tokAt(4));
    }

    // TODO: Handle sprintf/printf better.
    if (Token::Match(&tok, "printf ( %str% , %var% ,|)") && tok.tokAt(4)->varId() > 0)
    {
        const std::string &formatstr(tok.tokAt(2)->str());
        const std::string::size_type pos = formatstr.find("%");
        if (pos != std::string::npos && formatstr.compare(pos,2,"%s") == 0)
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
    if (Token::Match(tok->tokAt(-3), "!!sizeof [;{}=+-/(,] * %var%") && Token::Match(tok->tokAt(-3), "!!decltype [;{}=+-/(,] * %var%"))
        return true;

    if (!Token::simpleMatch(tok->tokAt(-2), "& (") && !Token::Match(tok->tokAt(-2), "sizeof|decltype (") && tok->strAt(-1) != "&"  && tok->strAt(-1) != "&&" && Token::Match(tok->next(), ". %var%"))
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

bool CheckNullPointer::isPointer(const unsigned int varid)
{
    // Check if given variable is a pointer
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const Variable *variableInfo = symbolDatabase->getVariableFromVarId(varid);
    const Token *tok = variableInfo ? variableInfo->typeStartToken() : NULL;

    if (Token::Match(tok, "%type% %type% * %varid% [;)=]", varid))
        return true;

    // maybe not a pointer
    if (!Token::Match(tok, "%type% * %varid% [;)=]", varid))
        return false;

    // it is a pointer
    if (!tok->previous() ||
        Token::Match(tok->previous(), "[({};]") ||
        tok->previous()->isName())
    {
        return true;
    }

    // it is not a pointer
    return false;
}

void CheckNullPointer::nullPointerAfterLoop()
{
    // Locate insufficient null-pointer handling after loop
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // only interested in while ( %var% )
        // TODO: Aren't there false negatives. Shouldn't other loops be handled such as:
        //       - while ( ! %var% )
        //       - while ( %var% && .. )
        if (! Token::Match(tok, "while ( %var% )|&&"))
            continue;

        // Get variable id for the loop variable
        const unsigned int varid(tok->tokAt(2)->varId());
        if (varid == 0)
            continue;

        // Is variable a pointer?
        if (!isPointer(varid))
            continue;

        // Get variable name for the loop variable
        const std::string varname(tok->strAt(2));

        // Locate the end of the while loop body..
        const Token *tok2 = tok->next()->link()->next()->link();

        // Check if the variable is dereferenced after the while loop
        while (0 != (tok2 = tok2 ? tok2->next() : 0))
        {
            // Don't check into inner scopes or outer scopes. Stop checking if "break" is found
            if (tok2->str() == "{" || tok2->str() == "}" || tok2->str() == "break")
                break;

            // loop variable is found..
            if (tok2->varId() == varid)
            {
                // dummy variable.. is it unknown if pointer is dereferenced or not?
                bool unknown = false;

                // Is the loop variable dereferenced?
                if (CheckNullPointer::isPointerDeRef(tok2, unknown))
                {
                    nullPointerError(tok2, varname, tok->linenr());
                }
                break;
            }
        }
    }
}

void CheckNullPointer::nullPointerLinkedList()
{
    // looping through items in a linked list in a inner loop.
    // Here is an example:
    //    for (const Token *tok = tokens; tok; tok = tok->next) {
    //        if (tok->str() == "hello")
    //            tok = tok->next;   // <- tok might become a null pointer!
    //    }
    for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
    {
        // search for a "for" token..
        if (!Token::simpleMatch(tok1, "for ("))
            continue;

        // is there any dereferencing occurring in the for statement
        // parlevel2 counts the parentheses when using tok2.
        unsigned int parlevel2 = 1;
        for (const Token *tok2 = tok1->tokAt(2); tok2; tok2 = tok2->next())
        {
            // Parentheses..
            if (tok2->str() == "(")
                ++parlevel2;
            else if (tok2->str() == ")")
            {
                if (parlevel2 <= 1)
                    break;
                --parlevel2;
            }

            // Dereferencing a variable inside the "for" parentheses..
            else if (Token::Match(tok2, "%var% . %var%"))
            {
                // Variable id for dereferenced variable
                const unsigned int varid(tok2->varId());
                if (varid == 0)
                    continue;

                if (Token::Match(tok2->tokAt(-2), "%varid% ?", varid))
                    continue;

                // Variable name of dereferenced variable
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

                    // TODO: are there false negatives for "while ( %varid% ||"
                    else if (Token::Match(tok3, "while ( %varid% &&|)", varid))
                    {
                        // Make sure there is a "break" or "return" inside the loop.
                        // Without the "break" a null pointer could be dereferenced in the
                        // for statement.
                        // indentlevel4 is a counter for { and }. When scanning the code with tok4
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
                                    if (isPointer(varid))
                                        nullPointerError(tok1, varname, tok3->linenr());

                                    break;
                                }
                                --indentlevel4;
                            }

                            // There is a "break" or "return" inside the loop.
                            // TODO: there can be false negatives. There could still be
                            //       execution paths that are not properly terminated
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
    // Dereferencing a struct pointer and then checking if it's NULL..

    // skipvar: don't check vars that has been tested against null already
    std::set<unsigned int> skipvar;
    skipvar.insert(0);

    // Scan through all tokens
    for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
    {
        // Checking if some pointer is null.
        // then add the pointer to skipvar => is it known that it isn't NULL
        if (Token::Match(tok1, "if|while ( !| %var% )"))
        {
            tok1 = tok1->tokAt(2);
            if (tok1->str() == "!")
                tok1 = tok1->next();
            skipvar.insert(tok1->varId());
            continue;
        }

        /**
         * @todo There are lots of false negatives here. A dereference
         *  is only investigated if a few specific conditions are met.
         */

        // dereference in assignment
        if (Token::Match(tok1, "[;{}] %var% . %var%"))
        {
            tok1 = tok1->next();
        }

        // dereference in assignment
        else if (Token::Match(tok1, "[{};] %var% = %var% . %var%"))
        {
            if (std::string(tok1->strAt(1)) == tok1->strAt(3))
                continue;
            tok1 = tok1->tokAt(3);
        }

        // dereference in condition
        else if (Token::Match(tok1, "if ( !| %var% ."))
        {
            tok1 = tok1->tokAt(2);
            if (tok1->str() == "!")
                tok1 = tok1->next();
        }

        // dereference in function call (but not sizeof|decltype)
        else if ((Token::Match(tok1->tokAt(-2), "%var% ( %var% . %var%") && !Token::Match(tok1->tokAt(-2), "sizeof|decltype ( %var% . %var%")) ||
                 Token::Match(tok1->previous(), ", %var% . %var%"))
        {
            // Is the function return value taken by the pointer?
            bool assignment = false;
            const unsigned int varid1(tok1->varId());
            if (varid1 == 0)
                continue;
            const Token *tok2 = tok1->previous();
            while (tok2 && !Token::Match(tok2, "[;{}]"))
            {
                if (Token::Match(tok2, "%varid% =", varid1))
                {
                    assignment = true;
                    break;
                }
                tok2 = tok2->previous();
            }
            if (assignment)
                continue;
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

        // name of struct pointer
        const std::string varname(tok1->str());

        // is pointer local?
        bool isLocal = false;
        const Variable * var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok1->varId());
        if (var && (var->isLocal() || var->isArgument()))
            isLocal = true;

        // member function may or may not nullify the pointer if it's global (#2647)
        if (!isLocal)
        {
            const Token *tok2 = tok1;
            while (Token::Match(tok2, "%var% ."))
                tok2 = tok2->tokAt(2);
            if (Token::Match(tok2,"%var% ("))
                continue;
        }

        // count { and } using tok2
        unsigned int indentlevel2 = 0;
        for (const Token *tok2 = tok1->tokAt(3); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
                ++indentlevel2;

            else if (tok2->str() == "}")
            {
                if (indentlevel2 == 0)
                    break;
                --indentlevel2;
            }

            // label / ?:
            else if (tok2->str() == ":")
                break;

            // Reassignment of the struct
            else if (tok2->varId() == varid1)
            {
                if (tok2->next()->str() == "=")
                {
                    // Avoid false positives when there is 'else if'
                    // TODO: can this be handled better?
                    if (tok1->strAt(-2) == "if")
                        skipvar.insert(varid1);
                    break;
                }
                if (Token::Match(tok2->tokAt(-2), "[,(] &"))
                    break;
            }

            // Loop..
            /** @todo don't bail out if the variable is not used in the loop */
            else if (tok2->str() == "do")
                break;

            // return/break at base level => stop checking
            else if (indentlevel2 == 0 && (tok2->str() == "return" || tok2->str() == "break"))
                break;

            // Function call: If the pointer is not a local variable it
            // might be changed by the call.
            else if (Token::Match(tok2, "[;{}] %var% (") &&
                     Token::simpleMatch(tok2->tokAt(2)->link(), ") ;") && !isLocal)
            {
                break;
            }

            // Check if pointer is null.
            // TODO: false negatives for "if (!p || .."
            else if (Token::Match(tok2, "if ( !| %varid% )|&&", varid1))
            {
                // Is this variable a pointer?
                if (isPointer(varid1))
                    nullPointerError(tok1, varname, tok2->linenr());
                break;
            }
        }
    }
}

void CheckNullPointer::nullPointerByDeRefAndChec()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Dereferencing a pointer and then checking if it's NULL..
    // This check will first scan for the check. And then scan backwards
    // from the check, searching for dereferencing.
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // TODO: false negatives.
        // - logical operators
        // - while
        if (tok->str() == "if" && Token::Match(tok->previous(), "; if ( !| %var% )|%oror%|&&"))
        {
            const Token * vartok = tok->tokAt(2);
            if (vartok->str() == "!")
                vartok = vartok->next();

            // Variable id for pointer
            const unsigned int varid(vartok->varId());
            if (varid == 0)
                continue;

            // Name of pointer
            const std::string varname(vartok->str());

            // Check that variable is a pointer..
            if (!isPointer(varid))
                continue;

            // Token where pointer is declared
            const Variable *var = symbolDatabase->getVariableFromVarId(varid);
            if (!var)
                continue;

            const Token * const decltok = var->nameToken();

            for (const Token *tok1 = tok->previous(); tok1 && tok1 != decltok; tok1 = tok1->previous())
            {
                if (tok1->str() == ")" && Token::Match(tok1->link()->previous(), "%var% ("))
                {
                    const Token *tok2 = tok1->link();
                    while (tok2 && !Token::Match(tok2, "[;{}?:]"))
                        tok2 = tok2->previous();
                    if (Token::Match(tok2, "[?:]"))
                        break;
                    if (Token::Match(tok2, "[;{}] %varid% = %var%", varid))
                        break;

                    if (Token::Match(tok1->link()->previous(), "while ( %varid%", varid))
                        break;

                    if (Token::simpleMatch(tok1->link()->previous(), "sizeof ("))
                    {
                        tok1 = tok1->link()->previous();
                        continue;
                    }

                    if (Token::Match(tok2, "[;{}] %var% ( %varid% ,", varid))
                    {
                        std::list<const Token *> varlist;
                        parseFunctionCall(*(tok2->next()), varlist, 0);
                        if (!varlist.empty() && varlist.front() == tok2->tokAt(3))
                        {
                            nullPointerError(tok2->tokAt(3), varname, tok->linenr());
                            break;
                        }
                    }

                    // calling unknown function => it might initialize the pointer
                    if (!(var->isLocal() || var->isArgument()))
                        break;
                }

                if (tok1->str() == "break")
                    break;

                if (tok1->varId() == varid)
                {
                    // Don't write warning if the dereferencing is
                    // guarded by ?:
                    const Token *tok2 = tok1->previous();
                    if (tok2 && (tok2->isArithmeticalOp() || tok2->str() == "("))
                    {
                        while (tok2 && !Token::Match(tok2, "[;{}?:]"))
                        {
                            if (tok2->str() == ")")
                                tok2 = tok2->link();
                            tok2 = tok2->previous();
                        }
                    }
                    if (Token::Match(tok2, "[?:]"))
                        continue;

                    // unknown : this is set by isPointerDeRef if it is
                    //           uncertain
                    bool unknown = false;

                    if (Token::Match(tok1->tokAt(-2), "%varid% = %varid% .", varid))
                    {
                        break;
                    }
                    else if (Token::Match(tok1->previous(), "&&|%oror%"))
                    {
                        break;
                    }
                    else if (Token::Match(tok1->tokAt(-2), "&&|%oror% !"))
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

    // used to check if a variable is a pointer.
    // TODO: Use isPointer?
    std::set<unsigned int> pointerVariables;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "* %var% [;,)=]"))
            pointerVariables.insert(tok->next()->varId());

        else if (Token::simpleMatch(tok, "if ("))
        {
            // TODO: investigate false negatives:
            // - handle "while"?
            // - if there are logical operators
            // - if (x) { } else { ... }

            // If the if-body ends with a unknown macro then bailout
            {
                // goto the end parenthesis
                const Token *endpar = tok->next()->link();
                const Token *endbody = Token::simpleMatch(endpar, ") {") ? endpar->next()->link() : 0;
                if (endbody &&
                    Token::Match(endbody->tokAt(-3), "[;{}] %var% ;") &&
                    isUpper(endbody->tokAt(-2)->str()))
                    continue;
            }

            // vartok : token for the variable
            const Token *vartok = 0;
            if (Token::Match(tok, "if ( ! %var% )|&&"))
                vartok = tok->tokAt(3);
            else if (Token::Match(tok, "if|while ( %var% )|&&"))
                vartok = tok->tokAt(2);
            else if (Token::Match(tok, "if ( ! ( %var% ="))
                vartok = tok->tokAt(4);
            else
                continue;

            // variable id for pointer
            const unsigned int varid(vartok->varId());
            if (varid == 0)
                continue;

            const unsigned int linenr = vartok->linenr();

            // Check if variable is a pointer. TODO: Use isPointer?
            if (pointerVariables.find(varid) == pointerVariables.end())
                continue;

            if (Token::Match(vartok->next(), "&& ( %varid% =", varid))
                continue;

            // if this is true then it is known that the pointer is null
            bool null = true;

            // start token = inside the if-body
            const Token *tok1 = tok->next()->link()->tokAt(2);

            // indentlevel inside the if-body is 1
            unsigned int indentlevel = 1;

            if (Token::Match(tok, "if|while ( %var% )|&&"))
            {
                // pointer might be null
                null = false;

                // start token = first token after the if/while body
                tok1 = tok1->previous()->link();
                tok1 = tok1 ? tok1->next() : NULL;
                if (!tok1)
                    continue;

                // indentlevel at the base level is 0
                indentlevel = 0;
            }

            // Name of the pointer
            const std::string &pointerName = vartok->str();

            // Count { and } for tok2
            for (const Token *tok2 = tok1; tok2; tok2 = tok2->next())
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
                        while (Token::simpleMatch(tok2, "} else {"))
                            tok2 = tok2->tokAt(2)->link();
                        null = false;
                    }
                }

                if (Token::Match(tok2, "goto|return|continue|break|throw|if|switch"))
                {
                    bool dummy = false;
                    if (Token::Match(tok2, "return * %varid%", varid))
                        nullPointerError(tok2, pointerName, linenr);
                    else if (Token::Match(tok2, "return %varid%", varid) &&
                             CheckNullPointer::isPointerDeRef(tok2->next(), dummy))
                        nullPointerError(tok2, pointerName, linenr);
                    break;
                }

                // parameters to sizeof are not dereferenced
                if (Token::Match(tok2, "decltype|sizeof ("))
                {
                    tok2 = tok2->next()->link();
                    continue;
                }

                // function call, check if pointer is dereferenced
                if (Token::Match(tok2, "%var% ("))
                {
                    std::list<const Token *> var;
                    parseFunctionCall(*tok2, var, 0);
                    for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                    {
                        if ((*it)->varId() == varid)
                        {
                            nullPointerError(*it, pointerName, linenr);
                            break;
                        }
                    }
                }

                // calling unknown function (abort/init)..
                if (Token::simpleMatch(tok2, ") ;") &&
                    (Token::Match(tok2->link()->tokAt(-2), "[;{}.] %var% (") ||
                     Token::Match(tok2->link()->tokAt(-5), "[;{}] ( * %var% ) (")))
                {
                    // noreturn function?
                    if (tok2->strAt(2) == "}")
                        break;

                    // init function (global variables)
                    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
                    const Variable *var = symbolDatabase->getVariableFromVarId(varid);
                    if (!var || !(var->isLocal() || var->isArgument()))
                        break;
                }

                if (tok2->varId() == varid)
                {
                    // unknown: this is set to true by isPointerDeRef if
                    //          the function fails to determine if there
                    //          is a dereference or not
                    bool unknown = false;

                    if (Token::Match(tok2->previous(), "[;{}=] %var% = 0 ;"))
                        ;

                    else if (CheckNullPointer::isPointerDeRef(tok2, unknown))
                        nullPointerError(tok2, pointerName, linenr);

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

/** Dereferencing null constant (simplified token list) */
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
                if (Token::Match(tok->previous(), "return|;|{|}|=|(|,|%op%"))
                {
                    nullPointerError(tok);
                }
            }

            else if (indentlevel > 0 && Token::Match(tok->previous(), "[={};] %var% ("))
            {
                std::list<const Token *> var;
                parseFunctionCall(*tok, var, 0);

                // is one of the var items a NULL pointer?
                for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                {
                    if ((*it)->str() == "0")
                    {
                        nullPointerError(*it);
                    }
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
        if (Token::Match(tok.previous(), "[;{}] const| struct| %type% * %var% ;"))
        {
            const Token * vartok = tok.tokAt(2);

            if (tok.str() == "const")
                vartok = vartok->next();

            if (tok.str() == "struct")
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

        else if (Token::simpleMatch(&tok, "( 0 &&"))
            return tok.link();

        if (tok.varId() != 0)
        {
            // unknown : not really used. it is passed to isPointerDeRef.
            //           if isPointerDeRef fails to determine if there
            //           is a dereference the this will be set to true.
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

        else if (tok.str() == "return")
        {
            bool unknown = false;
            const Token *vartok = tok.next();
            if (vartok->str() == "*")
                vartok = vartok->next();
            if (vartok->varId() && CheckNullPointer::isPointerDeRef(vartok, unknown))
            {
                dereference(checks, vartok);
            }
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

