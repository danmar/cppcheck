/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
// Auto variables checks
//---------------------------------------------------------------------------

#include "checkautovariables.h"

#include <sstream>
#include <iostream>
#include <string>

//---------------------------------------------------------------------------


// Register this check class into cppcheck by creating a static instance of it..
namespace
{
static CheckAutoVariables instance;
}


// _callStack used when parsing into subfunctions.


bool CheckAutoVariables::errorAv(const Token* left, const Token* right)
{
    if (fp_list.find(left->str()) == fp_list.end())
    {
        return false;
    }

    return isAutoVar(right->varId());
}

bool CheckAutoVariables::isAutoVar(unsigned int varId)
{
    if (varId == 0)
    {
        return false;
    }

    return (vd_list.find(varId) != vd_list.end());
}

bool CheckAutoVariables::isAutoVarArray(unsigned int varId)
{
    if (varId == 0)
    {
        return false;
    }

    return (vda_list.find(varId) != vda_list.end());
}

void print(const Token *tok, int num)
{
    const Token *t = tok;
    std::cout << tok->linenr() << " PRINT ";
    for (int i = 0; i < num; i++)
    {
        std::cout << " [" << t->str() << "] ";
        t = t->next();
    }
    std::cout << std::endl;
}

bool isTypeName(const Token *tok)
{
    bool ret = false;
    const std::string _str(tok->str());
    static const char * const type[] = {"case", "return", "delete", 0};
    for (int i = 0; type[i]; i++)
    {
        ret |= (_str == type[i]);
    }
    return !ret;
}

bool isExternOrStatic(const Token *tok)
{
    bool res = false;

    if (Token::Match(tok->tokAt(-1), "extern|static"))
    {
        res = true;
    }
    else if (Token::Match(tok->tokAt(-2), "extern|static"))
    {
        res = true;
    }
    else if (Token::Match(tok->tokAt(-3), "extern|static"))
    {
        res = true;
    }

    //std::cout << __PRETTY_FUNCTION__ << " " << tok->str() << " " << res << std::endl;
    return res;

}

void CheckAutoVariables::addVD(unsigned int varId)
{
    if (varId > 0)
    {
        vd_list.insert(varId);
    }
}

void CheckAutoVariables::addVDA(unsigned int varId)
{
    if (varId > 0)
    {
        vda_list.insert(varId);
    }
}

void CheckAutoVariables::autoVariables()
{
    bool begin_function = false;
    bool begin_function_decl = false;
    int bindent = 0;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {

        if (Token::Match(tok, "%type% *|::| %var% ("))
        {
            begin_function = true;
            fp_list.clear();
            vd_list.clear();
            vda_list.clear();
        }
        else if (begin_function && begin_function_decl && Token::Match(tok, "%type% * * %var%"))
        {
            fp_list.insert(tok->tokAt(3)->str());
        }
        else if (begin_function && begin_function_decl && Token::Match(tok, "%type% * %var% ["))
        {
            fp_list.insert(tok->tokAt(2)->str());
        }
        else if (begin_function && tok->str() == "(")
        {
            begin_function_decl = true;
        }
        else if (begin_function && tok->str() == ")")
        {
            begin_function_decl = false;
        }
        else if (begin_function && tok->str() == "{")
        {
            bindent++;
        }
        else if (begin_function && tok->str() == "}")
        {
            bindent--;
        }
        else if (bindent <= 0)
        {
            continue;
        }

        // Inside a function body
        if (Token::Match(tok, "%type% :: %any%") && !isExternOrStatic(tok))
        {
            addVD(tok->tokAt(2)->varId());
        }
        else if (Token::Match(tok, "%type% %var% ["))
        {
            addVDA(tok->next()->varId());
        }
        else if (Token::Match(tok, "%var% %var% ;") && !isExternOrStatic(tok) && isTypeName(tok))
        {
            addVD(tok->next()->varId());
        }
        else if (Token::Match(tok, "const %var% %var% ;") && !isExternOrStatic(tok) && isTypeName(tok->next()))
        {
            addVD(tok->tokAt(2)->varId());
        }
        //Critical assignment
        else if (Token::Match(tok, "[;{}] %var% = & %var%") && errorAv(tok->tokAt(1), tok->tokAt(4)))
        {
            errorAutoVariableAssignment(tok);
        }
        else if (Token::Match(tok, "[;{}] * %var% = & %var%") && errorAv(tok->tokAt(2), tok->tokAt(5)))
        {
            errorAutoVariableAssignment(tok);
        }
        else if (Token::Match(tok, "[;{}] %var% [ %any% ] = & %var%") && errorAv(tok->tokAt(1), tok->tokAt(7)))
        {
            errorAutoVariableAssignment(tok);
        }
        // Critical return
        else if (Token::Match(tok, "return & %var% ;") && isAutoVar(tok->tokAt(2)->varId()))
        {
            reportError(tok, Severity::error, "autoVariables", "Return of the address of an auto-variable");
        }
        // Invalid pointer deallocation
        else if (Token::Match(tok, "free ( %var% ) ;") && isAutoVarArray(tok->tokAt(2)->varId()))
        {
            reportError(tok, Severity::error, "autoVariables", "Invalid deallocation");
        }
    }

    vd_list.clear();
    vda_list.clear();
    fp_list.clear();
}
//---------------------------------------------------------------------------



void CheckAutoVariables::returnPointerToLocalArray()
{
    bool infunc = false;
    int indentlevel = 0;
    std::set<unsigned int> arrayVar;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Is there a function declaration for a function that returns a pointer?
        if (!infunc && (Token::Match(tok, "%type% * %var% (") || Token::Match(tok, "%type% * %var% :: %var% (")))
        {
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == ")")
                {
                    tok = tok2;
                    break;
                }
            }
            if (Token::simpleMatch(tok, ") {"))
            {
                infunc = true;
                indentlevel = 0;
                arrayVar.clear();
            }
        }

        // Parsing a function that returns a pointer..
        if (infunc)
        {
            if (tok->str() == "{")
            {
                ++indentlevel;
            }
            else if (tok->str() == "}")
            {
                --indentlevel;
                if (indentlevel <= 0)
                {
                    infunc = false;
                }
                continue;
            }

            // Declaring a local array..
            if (Token::Match(tok, "[;{}] %type% %var% ["))
            {
                const unsigned int varid = tok->tokAt(2)->varId();
                if (varid > 0)
                {
                    arrayVar.insert(varid);
                }
            }

            // Return pointer to local array variable..
            if (Token::Match(tok, "return %var% ;"))
            {
                const unsigned int varid = tok->next()->varId();
                if (varid > 0 && arrayVar.find(varid) != arrayVar.end())
                {
                    errorReturnPointerToLocalArray(tok);
                }
            }
        }

        // Declaring array variable..


    }
}

void CheckAutoVariables::errorReturnPointerToLocalArray(const Token *tok)
{
    reportError(tok, Severity::error, "returnLocalVariable", "Returning pointer to local array variable");
}

void CheckAutoVariables::errorAutoVariableAssignment(const Token *tok)
{
    reportError(tok, Severity::error, "autoVariables", "Wrong assignment of an auto-variable to an effective parameter of a function");
}



// return temporary?
bool CheckAutoVariables::returnTemporary(const Token *tok) const
{
    if (!Token::Match(tok, "return %var% ("))
        return false;
    return bool(0 != Token::findmatch(_tokenizer->tokens(), ("std :: string " + tok->next()->str() + " (").c_str()));
}


void CheckAutoVariables::returnReference()
{
    // locate function that returns a reference..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Skip executable scopes..
        if (Token::Match(tok, ") const| {"))
        {
            tok = tok->next();
            if (tok->str() == "const")
                tok = tok->next();
            tok = tok->link();
            continue;
        }

        // have we reached a function that returns a reference?
        if (Token::Match(tok, "%type% & %var% (") ||
            Token::Match(tok, "> & %var% ("))
        {
            // go to the '('
            const Token *tok2 = tok->tokAt(3);

            // go to the ')'
            tok2 = tok2->link();

            // is this a function implementation?
            if (Token::Match(tok2, ") const| {"))
            {
                unsigned int indentlevel = 0;
                std::set<unsigned int> localvar;    // local variables in function
                while (0 != (tok2 = tok2->next()))
                {
                    // indentlevel..
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}")
                    {
                        if (indentlevel <= 1)
                            break;
                        --indentlevel;
                    }

                    // declare local variable..
                    if (Token::Match(tok2, "[{};] %type%") && tok2->next()->str() != "return")
                    {
                        // goto next token..
                        tok2 = tok2->next();

                        // skip "const"
                        if (Token::Match(tok2, "const %type%"))
                            tok2 = tok2->next();

                        // skip "std::" if it is seen
                        if (Token::simpleMatch(tok2, "std ::"))
                            tok2 = tok2->tokAt(2);

                        // is it a variable declaration?
                        if (Token::Match(tok2, "%type% %var% ;"))
                            localvar.insert(tok2->next()->varId());
                        else if (Token::Match(tok2, "%type% < %any% > %var% ;"))
                            localvar.insert(tok2->tokAt(4)->varId());
                    }

                    // return..
                    else if (Token::Match(tok2, "return %var% ;"))
                    {
                        // is the returned variable a local variable?
                        if ((tok2->next()->varId() > 0) &&
                            (localvar.find(tok2->next()->varId()) != localvar.end()))
                        {
                            // report error..
                            errorReturnReference(tok2);
                        }
                    }

                    // return reference to temporary..
                    else if (returnTemporary(tok2))
                    {
                        // report error..
                        errorReturnTempReference(tok2);
                    }
                }
            }
        }
    }
}

void CheckAutoVariables::errorReturnReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnReference", "Returning reference to auto variable");
}

void CheckAutoVariables::errorReturnTempReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnTempReference", "Returning reference to temporary");
}


// Return c_str
void CheckAutoVariables::returncstr()
{
    // locate function that returns a const char *..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Skip executable scopes..
        if (Token::Match(tok, ") const| {"))
        {
            tok = tok->next();
            if (tok->str() == "const")
                tok = tok->next();
            tok = tok->link();
            continue;
        }

        // have we reached a function that returns a reference?
        if (Token::Match(tok, "const char * %var% ("))
        {
            // go to the '('
            const Token *tok2 = tok->tokAt(4);

            // go to the ')'
            tok2 = tok2->link();

            // is this a function implementation?
            if (Token::Match(tok2, ") const| {"))
            {
                unsigned int indentlevel = 0;
                std::set<unsigned int> localvar;    // local variables in function
                while (0 != (tok2 = tok2->next()))
                {
                    // indentlevel..
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}")
                    {
                        if (indentlevel <= 1)
                            break;
                        --indentlevel;
                    }

                    // declare local variable..
                    if (Token::Match(tok2, "[{};] %type%") && tok2->next()->str() != "return")
                    {
                        // goto next token..
                        tok2 = tok2->next();

                        // skip "const"
                        if (Token::Match(tok2, "const %type%"))
                            tok2 = tok2->next();

                        // skip "std::" if it is seen
                        if (Token::simpleMatch(tok2, "std ::"))
                            tok2 = tok2->tokAt(2);

                        // is it a variable declaration?
                        if (Token::Match(tok2, "%type% %var% ;"))
                            localvar.insert(tok2->next()->varId());
                    }

                    // return..
                    else if (Token::Match(tok2, "return %var% . c_str ( ) ;"))
                    {
                        // is the returned variable a local variable?
                        if ((tok2->next()->varId() > 0) &&
                            (localvar.find(tok2->next()->varId()) != localvar.end()))
                        {
                            // report error..
                            errorReturnAutocstr(tok2);
                        }
                    }

                    // return pointer to temporary..
                    else if (returnTemporary(tok2))
                    {
                        // report error..
                        errorReturnTempPointer(tok2);
                    }
                }
            }
        }
    }
}

void CheckAutoVariables::errorReturnAutocstr(const Token *tok)
{
    reportError(tok, Severity::error, "returnAutocstr", "Returning pointer to auto variable");
}

void CheckAutoVariables::errorReturnTempPointer(const Token *tok)
{
    reportError(tok, Severity::error, "returnTempPointer", "Returning pointer to temporary");
}



