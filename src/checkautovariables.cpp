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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

//---------------------------------------------------------------------------
// Auto variables checks
//---------------------------------------------------------------------------

#include "checkautovariables.h"

#include <algorithm>
#include <sstream>
#include <list>
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
    const std::string left_var(left->str());
    const std::string right_var(right->str());
    std::list<std::string>::iterator it_fp;

    for (it_fp = fp_list.begin(); it_fp != fp_list.end(); ++it_fp)
    {
        std::string vname(*it_fp);

        //The left argument is a formal parameter
        if (vname == left_var)
        {
            //cout << "Beccato" << endl;
            break;
        }

    }
    //The left argument is NOT a formal parameter
    if (it_fp == fp_list.end())
        return false;

    std::list<std::string>::iterator id_vd;
    for (id_vd = vd_list.begin(); id_vd != vd_list.end(); ++id_vd)
    {
        std::string vname(*id_vd);
        //The left argument is a variable declaration
        if (vname == right_var)
            break;
    }
    //The left argument is NOT a variable declaration
    if (id_vd == vd_list.end())
        return false;
    //If I reach this point there is a wrong assignement of an auto-variable to an effective parameter of a function
    return true;

}
bool CheckAutoVariables::isAutoVar(const Token* t)
{
    std::list<std::string>::iterator id_vd;
    std::string v(t->str());
    for (id_vd = vd_list.begin(); id_vd != vd_list.end(); ++id_vd)
    {
        std::string vname(*id_vd);
        if (vname == v)
            return true;
    }
    return false;
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
    std::string _str(tok->str());
    static const char * const type[] = {"case", "return", "delete", 0};
    for (int i = 0; type[i]; i++)
        ret |= (_str == type[i]);
    return !ret;
}
bool isStatic(const Token *tok)
{
    bool res = false;

    if (tok->tokAt(-1)->str() == "static")
        res = true;
    else if (tok->tokAt(-2)->str() == "static")
        res = true;
    else if (tok->tokAt(-3)->str() == "static")
        res = true;

    //std::cout << __PRETTY_FUNCTION__ << " " << tok->str() << " " << res << std::endl;
    return res;

}
void CheckAutoVariables::addVD(const Token* tok)
{
    std::string var_name(tok->str());

    //std::cout << "VD " << tok->linenr() << " " << var_name << std::endl;
    vd_list.push_back(var_name);
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
        }
        else if (begin_function && begin_function_decl && Token::Match(tok, "%type% * * %var%"))
        {
            std::string var_name(tok->tokAt(3)->str());
            fp_list.push_back(var_name);
        }
        else if (begin_function && begin_function_decl && Token::Match(tok, "%type% * %var% ["))
        {
            std::string var_name(tok->tokAt(2)->str());
            fp_list.push_back(var_name);
        }
        else if (begin_function && tok->str() == "(")
            begin_function_decl = true;
        else if (begin_function && tok->str() == ")")
        {
            begin_function_decl = false;
        }
        else if (begin_function && tok->str() == "{")
            bindent++;
        else if (begin_function && tok->str() == "}")
        {
            bindent--;
        }
        else if (bindent > 0 && Token::Match(tok, "%type% :: %any%") && !isStatic(tok)) //Inside a function
        {
            addVD(tok->tokAt(2));
        }
        else if (bindent > 0 && Token::Match(tok, "%var% %var% ;") && !isStatic(tok)) //Inside a function
        {
            if (!isTypeName(tok))
                continue;
            addVD(tok->tokAt(1));
        }
        else if (bindent > 0 && Token::Match(tok, "const %var% %var% ;") && !isStatic(tok)) //Inside a function
        {
            if (!isTypeName(tok->tokAt(1)))
                continue;
            addVD(tok->tokAt(2));
        }
        else if (bindent > 0 && Token::Match(tok, "[;{}] %var% = & %var%")) //Critical assignement
        {
            if (errorAv(tok->tokAt(1), tok->tokAt(4)))
                reportError(tok,
                            Severity::error,
                            "autoVariables",
                            "Wrong assignement of an auto-variable to an effective parameter of a function");
        }
        else if (bindent > 0 && Token::Match(tok, "[;{}] %var% [ %any% ] = & %var%")) //Critical assignement
        {
            if (errorAv(tok->tokAt(1), tok->tokAt(7)))
                reportError(tok,
                            Severity::error,
                            "autoVariables",
                            "Wrong assignement of an auto-variable to an effective parameter of a function");
        }
        else if (bindent > 0 && Token::Match(tok, "return & %var% ;")) //Critical return
        {
            if (isAutoVar(tok->tokAt(2)))
                reportError(tok,
                            Severity::error,
                            "autoVariables",
                            "Return of the address of an auto-variable");
        }
    }
    vd_list.clear();
    fp_list.clear();
}
//---------------------------------------------------------------------------



void CheckAutoVariables::returnPointerToLocalArray()
{
    bool infunc = false;
    int indentlevel = 0;
    std::list<unsigned int> arrayVar;
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
                ++indentlevel;
            else if (tok->str() == "}")
            {
                --indentlevel;
                if (indentlevel <= 0)
                    infunc = false;
                continue;
            }

            // Declaring a local array..
            if (Token::Match(tok, "[;{}] %type% %var% ["))
            {
                arrayVar.push_back(tok->tokAt(2)->varId());
            }

            // Return pointer to local array variable..
            if (Token::Match(tok, "return %var% ;"))
            {
                unsigned int varid = tok->next()->varId();
                if (varid > 0 && std::find(arrayVar.begin(), arrayVar.end(), varid) != arrayVar.end())
                    errorReturnPointerToLocalArray(tok);
            }
        }

        // Declaring array variable..


    }
}

void CheckAutoVariables::errorReturnPointerToLocalArray(const Token *tok)
{
    reportError(tok, Severity::error, "returnLocalVariable", "Returning pointer to local array variable");
}



