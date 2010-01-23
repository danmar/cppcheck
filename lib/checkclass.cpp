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
#include "checkclass.h"

#include "tokenize.h"
#include "token.h"
#include "errorlogger.h"

#include <locale>

#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>

//---------------------------------------------------------------------------

// Register CheckClass..
namespace
{
CheckClass instance;
}


//---------------------------------------------------------------------------

CheckClass::Var *CheckClass::getVarList(const Token *tok1, bool withClasses, bool isStruct)
{
    // Get variable list..
    Var *varlist = NULL;
    unsigned int indentlevel = 0;
    bool priv = !isStruct;
    for (const Token *tok = tok1; tok; tok = tok->next())
    {
        if (!tok->next())
            break;

        if (tok->str() == "{")
            ++indentlevel;
        else if (tok->str() == "}")
        {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        }

        if (indentlevel != 1)
            continue;

        if (tok->str() == "__published:")
        {
            priv = false;
            for (; tok; tok = tok->next())
            {
                if (tok->str() == "{")
                    ++indentlevel;
                else if (tok->str() == "}")
                {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }
                if (indentlevel == 1 && Token::Match(tok->next(), "private:|protected:|public:"))
                    break;
            }
            if (tok)
                continue;
            else
                break;
        }

        // "private:" "public:" "protected:" etc
        const bool b((*tok->strAt(0) != ':') && strchr(tok->strAt(0), ':') != 0);

        if (b)
            priv = bool(tok->str() == "private:");

        // Search for start of statement..
        if (! Token::Match(tok, "[;{}]") && ! b)
            continue;

        // This is the start of a statement
        const Token *next = tok->next();
        const char *varname = 0;

        // If next token contains a ":".. it is not part of a variable declaration
        if (next->str().find(":") != std::string::npos)
            continue;

        // Variable declarations that start with "static" shall be ignored..
        if (next->str() == "static")
            continue;

        // Type definitions shall be ignored..
        if (next->str() == "typedef")
            continue;

        // Is it a variable declaration?
        if (Token::Match(next, "%type% %var% ;"))
        {
            if (withClasses)
                varname = next->strAt(1);
            else if (next->isStandardType())
                varname = next->strAt(1);
            else if (Token::findmatch(_tokenizer->tokens(), ("enum " + next->str()).c_str()))
                varname = next->strAt(1);
        }

        // Pointer?
        else if (Token::Match(next, "%type% * %var% ;"))
        {
            varname = next->strAt(2);
        }

        // Pointer?
        else if (Token::Match(next, "%type% %type% * %var% ;"))
        {
            varname = next->strAt(3);
        }

        // Array?
        else if (Token::Match(next, "%type% %var% [") && next->next()->str() != "operator")
        {
            if (Token::findmatch(_tokenizer->tokens(), ("class " + next->str()).c_str()))
                continue;
            varname = next->strAt(1);
        }

        // Pointer array?
        else if (Token::Match(next, "%type% * %var% ["))
        {
            varname = next->strAt(2);
        }

        // std::string..
        else if (withClasses && Token::Match(next, "std :: string %var% ;"))
        {
            varname = next->strAt(3);
        }

        // Container..
        else if (withClasses && Token::Match(next, "std :: %type% <"))
        {
            while (next && next->str() != ">")
                next = next->next();
            if (Token::Match(next, "> %var% ;"))
                varname = next->strAt(1);
        }

        // If the varname was set in one of the two if-block above, create a entry for this variable..
        if (varname && strcmp(varname, "operator"))
        {
            Var *var = new Var(varname, false, priv, varlist);
            varlist  = var;
        }
    }

    return varlist;
}
//---------------------------------------------------------------------------

void CheckClass::initVar(Var *varlist, const char varname[])
{
    for (Var *var = varlist; var; var = var->next)
    {
        if (strcmp(var->name, varname) == 0)
        {
            var->init = true;
            return;
        }
    }
}
//---------------------------------------------------------------------------

void CheckClass::initializeVarList(const Token *tok1, const Token *ftok, Var *varlist, const char classname[], std::list<std::string> &callstack, bool isStruct)
{
    bool Assign = false;
    unsigned int indentlevel = 0;

    for (; ftok; ftok = ftok->next())
    {
        if (!ftok->next())
            break;

        // Class constructor.. initializing variables like this
        // clKalle::clKalle() : var(value) { }
        if (indentlevel == 0)
        {
            if (Assign && Token::Match(ftok, "%var% ("))
            {
                initVar(varlist, ftok->strAt(0));
            }

            Assign |= (ftok->str() == ":");
        }


        if (ftok->str() == "{")
        {
            ++indentlevel;
            Assign = false;
        }

        else if (ftok->str() == "}")
        {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        }

        if (indentlevel < 1)
            continue;

        // Variable getting value from stream?
        if (Token::Match(ftok, ">> %var%"))
        {
            initVar(varlist, ftok->strAt(1));
        }

        // Before a new statement there is "[{};)=]" or "else"
        if (! Token::Match(ftok, "[{};)=]") && ftok->str() != "else")
            continue;

        // Using the operator= function to initialize all variables..
        if (Token::simpleMatch(ftok->next(), "* this = "))
        {
            for (Var *var = varlist; var; var = var->next)
                var->init = true;
            break;
        }

        if (!Token::Match(ftok->next(), "%var%") &&
            !Token::Match(ftok->next(), "this . %var%") &&
            !Token::Match(ftok->next(), "* %var% =") &&
            !Token::Match(ftok->next(), "( * this ) . %var%"))
            continue;

        // Goto the first token in this statement..
        ftok = ftok->next();

        // Skip "( * this )"
        if (Token::simpleMatch(ftok, "( * this ) ."))
        {
            ftok = ftok->tokAt(5);
        }

        // Skip "this->"
        if (Token::simpleMatch(ftok, "this ."))
            ftok = ftok->tokAt(2);

        // Skip "classname :: "
        if (Token::Match(ftok, "%var% ::"))
            ftok = ftok->tokAt(2);

        // Clearing all variables..
        if (Token::simpleMatch(ftok, "memset ( this ,"))
        {
            for (Var *var = varlist; var; var = var->next)
                var->init = true;
            break;
        }

        // Calling member function?
        else if (Token::Match(ftok, "%var% ("))
        {
            // No recursive calls!
            if (std::find(callstack.begin(), callstack.end(), ftok->str()) == callstack.end())
            {
                callstack.push_back(ftok->str());
                int i = 0;
                const Token *ftok2 = Tokenizer::findClassFunction(tok1, classname, ftok->strAt(0), i, isStruct);
                if (ftok2)
                {
                    initializeVarList(tok1, ftok2, varlist, classname, callstack, isStruct);
                }
                else  // there is a called member function, but it is not defined where we can find it, so we assume it initializes everything
                {
                    // check if the function is part of this class..
                    const Token *tok = Token::findmatch(_tokenizer->tokens(), (std::string("class ") + classname + " {").c_str());
                    for (tok = tok ? tok->tokAt(3) : 0; tok; tok = tok->next())
                    {
                        if (tok->str() == "{")
                        {
                            tok = tok->link();
                            if (!tok)
                                break;
                        }
                        else if (tok->str() == "}")
                        {
                            break;
                        }
                        else if (tok->str() == ftok->str() || tok->str() == "friend")
                        {
                            tok = 0;
                            break;
                        }
                    }

                    // bail out..
                    if (!tok)
                    {
                        for (Var *var = varlist; var; var = var->next)
                            var->init = true;
                        break;
                    }

                    // the function is external and it's neither friend nor inherited virtual function.
                    // assume all variables that are passed to it are initialized..
                    unsigned int indentlevel = 0;
                    for (tok = ftok->tokAt(2); tok; tok = tok->next())
                    {
                        if (tok->str() == "(")
                            ++indentlevel;
                        else if (tok->str() == ")")
                        {
                            if (indentlevel == 0)
                                break;
                            --indentlevel;
                        }
                        if (tok->isName())
                        {
                            initVar(varlist, tok->strAt(0));
                        }
                    }
                    continue;
                }
            }
        }

        // Assignment of member variable?
        else if (Token::Match(ftok, "%var% ="))
        {
            initVar(varlist, ftok->strAt(0));
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%var% [ %any% ] ="))
        {
            initVar(varlist, ftok->strAt(0));
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%var% [ %any% ] [ %any% ] ="))
        {
            initVar(varlist, ftok->strAt(0));
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "* %var% ="))
        {
            initVar(varlist, ftok->strAt(1));
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Token::Match(ftok, "%var% . clear|Clear ("))
        {
            initVar(varlist, ftok->strAt(0));
        }
    }
}






//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    const char pattern_class[] = "class|struct %var% [{:]";

    // Locate class
    const Token *tok1 = Token::findmatch(_tokenizer->tokens(), pattern_class);
    while (tok1)
    {
        const char *className;
        className = tok1->strAt(1);
        const Token *classNameToken = tok1->tokAt(1);
        bool isStruct = tok1->str() == "struct";

        /** @todo handling of private constructors should be improved */
        bool hasPrivateConstructor = false;
        {
            int indentlevel = 0;
            bool isPrivate = !isStruct;
            for (const Token *tok = tok1; tok; tok = tok->next())
            {
                // Indentation
                if (tok->str() == "{")
                    ++indentlevel;

                else if (tok->str() == "}")
                {
                    --indentlevel;
                    if (indentlevel <= 0)
                        break;
                }

                // Parse class contents (indentlevel == 1)..
                if (indentlevel == 1)
                {
                    // What section are we in.. private/non-private
                    if (tok->str() == "private:")
                        isPrivate = true;
                    else if (tok->str() == "protected:" || tok->str() == "public:")
                        isPrivate = false;

                    // Is there a private constructor?
                    else if (isPrivate && Token::simpleMatch(tok, (classNameToken->str() + " (").c_str()))
                    {
                        hasPrivateConstructor = true;
                        break;
                    }
                }
            }
        }

        if (hasPrivateConstructor && !_settings->_showAll)
        {
            /** @todo Handle private constructors. Right now to avoid
             * false positives we just bail out */
            tok1 = Token::findmatch(tok1->next(), pattern_class);
            continue;
        }

        // Are there a class constructor?
        std::string tempPattern = "%any% " + classNameToken->str() + " (";
        const Token *constructor_token = Token::findmatch(tok1, tempPattern.c_str());
        while (constructor_token && constructor_token->str() == "~")
            constructor_token = Token::findmatch(constructor_token->next(), tempPattern.c_str());

        // There are no constructor.
        if (! constructor_token)
        {
            // If "--style" has been given, give a warning
            if (_settings->_checkCodingStyle)
            {
                // Get class variables...
                Var *varlist = getVarList(tok1, false, isStruct);

                // If there is a private variable, there should be a constructor..
                for (const Var *var = varlist; var; var = var->next)
                {
                    if (var->priv)
                    {
                        noConstructorError(tok1, classNameToken->str(), isStruct);
                        break;
                    }
                }

                // Delete the varlist..
                while (varlist)
                {
                    Var *nextvar = varlist->next;
                    delete varlist;
                    varlist = nextvar;
                }
            }

            tok1 = Token::findmatch(tok1->next(), pattern_class);
            continue;
        }

        // Check constructors
        checkConstructors(tok1, className, hasPrivateConstructor, isStruct);

        // Check assignment operators
        checkConstructors(tok1, "operator =", hasPrivateConstructor, isStruct);

        tok1 = Token::findmatch(tok1->next(), pattern_class);
    }
}

void CheckClass::checkConstructors(const Token *tok1, const char funcname[], bool hasPrivateConstructor, bool isStruct)
{
    const char * const className = tok1->strAt(1);

    // Check that all member variables are initialized..
    bool withClasses = bool(_settings->_showAll && std::string(funcname) == "operator =");
    Var *varlist = getVarList(tok1, withClasses, isStruct);

    int indentlevel = 0;
    const Token *constructor_token = Tokenizer::findClassFunction(tok1, className, funcname, indentlevel, isStruct);
    std::list<std::string> callstack;
    initializeVarList(tok1, constructor_token, varlist, className, callstack, isStruct);
    while (constructor_token)
    {
        // Check if any variables are uninitialized
        for (Var *var = varlist; var; var = var->next)
        {
            if (var->init)
                continue;

            // Is it a static member variable?
            std::ostringstream pattern;
            pattern << className << " :: " << var->name << " =";
            if (Token::findmatch(_tokenizer->tokens(), pattern.str().c_str()))
                continue;

            // It's non-static and it's not initialized => error
            if (Token::simpleMatch(constructor_token, "operator = (") ||
                Token::simpleMatch(constructor_token->tokAt(2), "operator = ("))
            {
                const Token *operStart = 0;
                if (Token::simpleMatch(constructor_token, "operator = ("))
                    operStart = constructor_token->tokAt(2);
                else
                    operStart = constructor_token->tokAt(4);

                bool classNameUsed = false;
                for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next())
                {
                    if (operTok->str() == className)
                    {
                        classNameUsed = true;
                        break;
                    }
                }

                if (classNameUsed)
                    operatorEqVarError(constructor_token, className, var->name);
            }
            else
                uninitVarError(constructor_token, className, var->name, hasPrivateConstructor);
        }

        for (Var *var = varlist; var; var = var->next)
            var->init = false;

        constructor_token = Tokenizer::findClassFunction(constructor_token->next(), className, funcname, indentlevel, isStruct);
        callstack.clear();
        initializeVarList(tok1, constructor_token, varlist, className, callstack, isStruct);
    }

    // Delete the varlist..
    while (varlist)
    {
        Var *nextvar = varlist->next;
        delete varlist;
        varlist = nextvar;
    }
}


//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

void CheckClass::privateFunctions()
{
    // Locate some class
    for (const Token *tok1 = Token::findmatch(_tokenizer->tokens(), "class|struct %var% {"); tok1; tok1 = Token::findmatch(tok1->next(), "class|struct %var% {"))
    {
        /** @todo check that the whole class implementation is seen */
        // until the todo above is fixed we only check classes that are
        // declared in the source file
        if (tok1->fileIndex() != 0)
            continue;

        const std::string &classname = tok1->next()->str();

        // Get private functions..
        std::list<const Token *> FuncList;
        FuncList.clear();
        bool isStruct = tok1->str() == "struct";
        bool priv = !isStruct;
        unsigned int indent_level = 0;
        for (const Token *tok = tok1; tok; tok = tok->next())
        {
            if (Token::Match(tok, "friend %var%"))
            {
                /** @todo Handle friend classes */
                FuncList.clear();
                break;
            }

            if (tok->str() == "{")
                ++indent_level;
            else if (tok->str() == "}")
            {
                if (indent_level <= 1)
                    break;
                --indent_level;
            }

            else if (indent_level != 1)
                continue;
            else if (tok->str() == "private:")
                priv = true;
            else if (tok->str() == "public:")
                priv = false;
            else if (tok->str() == "protected:")
                priv = false;
            else if (priv)
            {
                if (Token::Match(tok, "typedef %type% ("))
                    tok = tok->tokAt(2)->link();

                else if (Token::Match(tok, "[:,] %var% ("))
                    tok = tok->tokAt(2)->link();

                else if (Token::Match(tok, "%var% (") &&
                         !Token::simpleMatch(tok->next()->link(), ") (") &&
                         !Token::Match(tok, classname.c_str()))
                {
                    FuncList.push_back(tok);
                }
            }

            /** @todo embedded class have access to private functions */
            if (tok->str() == "class")
            {
                FuncList.clear();
                break;
            }
        }

        // Check that all private functions are used..
        bool HasFuncImpl = false;
        bool inclass = false;
        indent_level = 0;
        for (const Token *ftok = _tokenizer->tokens(); ftok; ftok = ftok->next())
        {
            if (ftok->str() == "{")
                ++indent_level;
            else if (ftok->str() == "}")
            {
                if (indent_level > 0)
                    --indent_level;
                if (indent_level == 0)
                    inclass = false;
            }

            if (Token::Match(ftok, ("class " + classname + " :|{").c_str()))
            {
                indent_level = 0;
                inclass = true;
            }

            // Check member class functions to see what functions are used..
            if ((inclass && indent_level == 1 && Token::Match(ftok, ") const| {")) ||
                (Token::Match(ftok, (classname + " :: ~| %var% (").c_str())))
            {
                while (ftok && ftok->str() != ")")
                    ftok = ftok->next();
                if (!ftok)
                    break;
                if (Token::Match(ftok, ") : %var% ("))
                {
                    while (!Token::Match(ftok->next(), "[{};]"))
                        ftok = ftok->next();
                }
                if (!Token::Match(ftok, ") const| {"))
                    continue;

                if (ftok->fileIndex() == 0)
                    HasFuncImpl = true;

                // Parse function..
                int indentlevel2 = 0;
                for (const Token *tok2 = ftok; tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "{")
                        ++indentlevel2;
                    else if (tok2->str() == "}")
                    {
                        --indentlevel2;
                        if (indentlevel2 < 1)
                            break;
                    }
                    else if (Token::Match(tok2, "%var% ("))
                    {
                        // Remove function from FuncList
                        std::list<const Token *>::iterator it = FuncList.begin();
                        while (it != FuncList.end())
                        {
                            if (tok2->str() == (*it)->str())
                                FuncList.erase(it++);
                            else
                                it++;
                        }
                    }
                }
            }
        }

        while (HasFuncImpl && !FuncList.empty())
        {
            // Final check; check if the function pointer is used somewhere..
            const std::string _pattern("return|(|)|,|= " + FuncList.front()->str());
            if (!Token::findmatch(_tokenizer->tokens(), _pattern.c_str()))
            {
                unusedPrivateFunctionError(FuncList.front(), classname, FuncList.front()->str());
            }
            FuncList.pop_front();
        }
    }
}

//---------------------------------------------------------------------------
// ClassCheck: Check that memset is not used on classes
//---------------------------------------------------------------------------

void CheckClass::noMemset()
{
    // Locate all 'memset' tokens..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (!Token::Match(tok, "memset|memcpy|memmove"))
            continue;

        const char *type = NULL;
        if (Token::Match(tok, "memset ( %var% , %num% , sizeof ( %type% ) )"))
            type = tok->strAt(8);
        else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( %type% ) )"))
            type = tok->strAt(9);
        else if (Token::Match(tok, "memset ( %var% , %num% , sizeof ( struct %type% ) )"))
            type = tok->strAt(9);
        else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( struct %type% ) )"))
            type = tok->strAt(10);
        else if (Token::Match(tok, "%type% ( %var% , %var% , sizeof ( %type% ) )"))
            type = tok->strAt(8);

        // No type defined => The tokens didn't match
        if (!(type && type[0]))
            continue;

        // Warn if type is a class..
        const std::string pattern1(std::string("class ") + type);
        if (Token::findmatch(_tokenizer->tokens(), pattern1.c_str()))
        {
            memsetClassError(tok, tok->str());
            continue;
        }

        // Warn if type is a struct that contains any std::*
        const std::string pattern2(std::string("struct ") + type + " {");
        for (const Token *tstruct = Token::findmatch(_tokenizer->tokens(), pattern2.c_str()); tstruct; tstruct = tstruct->next())
        {
            if (tstruct->str() == "}")
                break;

            if (Token::Match(tstruct, "std :: %type% %var% ;"))
            {
                memsetStructError(tok, tok->str(), tstruct->strAt(2));
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// ClassCheck: "void operator=("
//---------------------------------------------------------------------------

void CheckClass::operatorEq()
{
    const Token *tok2 = _tokenizer->tokens();
    const Token *tok;

    while ((tok = Token::findmatch(tok2, "void operator = (")))
    {
        const Token *tok1 = tok;
        while (tok1 && !Token::Match(tok1, "class|struct %var%"))
        {
            if (tok1->str() == "public:")
            {
                operatorEqReturnError(tok);
                break;
            }
            if (tok1->str() == "private:" || tok1->str() == "protected:")
                break;
            tok1 = tok1->previous();
        }

        if (tok1 && Token::Match(tok1, "struct %var%"))
            operatorEqReturnError(tok);

        tok2 = tok->next();
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C&) { ... return *this; }"
// operator= should return a reference to *this
//---------------------------------------------------------------------------

// match two lists of tokens
static bool nameMatch(const Token * tok1, const Token * tok2, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (tok1->tokAt(i) == 0 || tok2->tokAt(i) == 0)
            return false;

        if (tok1->tokAt(i)->str() != tok2->tokAt(i)->str())
            return false;
    }

    return true;
}

// create a class name from a list of tokens
static void nameStr(const Token * name, int length, std::string & str)
{
    for (int i = 0; i < length; i++)
    {
        if (i != 0)
            str += " ";

        str += name->tokAt(i)->str();
    }
}

void CheckClass::operatorEqRetRefThis()
{
    const Token *tok2 = _tokenizer->tokens();
    const Token *tok;

    while ((tok = Token::findmatch(tok2, "operator = (")))
    {
        const Token *tok1 = tok;

        if (tok1->tokAt(-2) && Token::Match(tok1->tokAt(-2), " %type% ::"))
        {
            // make sure this is an assignment operator
            int nameLength = 1;

            tok1 = tok1->tokAt(-2);

            // check backwards for proper function signature
            while (tok1->tokAt(-2) && Token::Match(tok1->tokAt(-2), " %type% ::"))
            {
                tok1 = tok1->tokAt(-2);
                nameLength += 2;
            }

            const Token *name = tok1;
            std::string nameString;

            nameStr(name, nameLength, nameString);

            if (tok1->tokAt(-1) && tok1->tokAt(-1)->str() == "&")
            {
                // check class name
                if (tok1->tokAt(-(1 + nameLength)) && nameMatch(name, tok1->tokAt(-(1 + nameLength)), nameLength))
                {
                    // may take overloaded argument types so no need to check them
                    tok1 = tok->tokAt(2)->link();

                    if (tok1 && tok1->next() && tok1->next()->str() == "{")
                    {
                        const Token *last = tok1->next()->link()->tokAt(-2);
                        for (tok1 = tok1->tokAt(2); tok1 != last; tok1 = tok1->next())
                        {
                            // check for return of reference to this
                            if (tok1->str() == "return")
                            {
                                if (!(Token::Match(tok1->tokAt(1), "* this ;") ||
                                      Token::Match(tok1->tokAt(1), "operator = (")))
                                    operatorEqRetRefThisError(tok);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // make sure this is an assignment operator
            tok1 = tok;

            // check backwards for proper function signature
            if (tok1->tokAt(-1) && tok1->tokAt(-1)->str() == "&")
            {
                const Token *name = 0;
                bool isPublic = false;
                while (tok1 && !Token::Match(tok1, "class|struct %var%"))
                {
                    if (!isPublic)
                    {
                        if (tok1->str() == "public:")
                            isPublic = true;
                    }
                    tok1 = tok1->previous();
                }

                if (tok1 && Token::Match(tok1, "struct %var%"))
                {
                    isPublic = true;
                    name = tok1->tokAt(1);
                }
                else if (tok1 && Token::Match(tok1, "class %var%"))
                {
                    name = tok1->tokAt(1);
                }

                if (tok->tokAt(-2) && tok->tokAt(-2)->str() == name->str())
                {
                    // may take overloaded argument types so no need to check them
                    tok1 = tok->tokAt(2)->link();

                    if (tok1 && tok1->next() && tok1->next()->str() == "{")
                    {
                        const Token *last = tok1->next()->link()->tokAt(-2);
                        for (tok1 = tok1->tokAt(2); tok1 != last; tok1 = tok1->next())
                        {
                            // check for return of reference to this
                            if (tok1->str() == "return")
                            {
                                if (!(Token::Match(tok1->tokAt(1), "* this ;") ||
                                      Token::Match(tok1->tokAt(1), "operator = (")))
                                    operatorEqRetRefThisError(tok);
                            }
                        }
                    }
                }
            }
        }

        tok2 = tok->next();
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C& rhs) { if (this == &rhs) ... }"
// operator= should check for assignment to self
//
// For simple classes, an assignment to self check is only a potential optimization.
//
// For classes that allocate dynamic memory, assignment to self can be a real error
// if it is deallocated and allocated again without being checked for.
//
// This check is not valid for classes with multiple inheritance because a
// class can have multiple addresses so there is no trivial way to check for
// assignment to self.
//---------------------------------------------------------------------------

static bool hasDeallocation(const Token * first, const Token * last)
{
    // This function is called when no simple check was found for assignment
    // to self.  We are currently looking for a specific sequence of:
    // deallocate member ; ... member = allocate
    // This check is far from ideal because it can cause false negatives.
    // Unfortunately, this is necessary to prevent false positives.
    // This check needs to do careful analysis someday to get this
    // correct with a high degree of certainty.
    for (const Token * tok = first; tok && (tok != last); tok = tok->next())
    {
        // check for deallocating memory
        if (Token::Match(tok, "{|;|, free ( %type%"))
        {
            const Token * var = tok->tokAt(3);

            // we should probably check that var is a pointer in this class

            const Token * tok1 = tok->tokAt(4);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%type% ="))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
        else if (Token::Match(tok, "{|;|, delete [ ] %type%"))
        {
            const Token * var = tok->tokAt(4);

            // we should probably check that var is a pointer in this class

            const Token * tok1 = tok->tokAt(5);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%type% = new ["))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
        else if (Token::Match(tok, "{|;|, delete %type%"))
        {
            const Token * var = tok->tokAt(2);

            // we should probably check that var is a pointer in this class

            const Token * tok1 = tok->tokAt(3);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%type% = new"))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
    }

    return false;
}

static bool hasAssignSelf(const Token * first, const Token * last, const Token * rhs)
{
    for (const Token * tok = first; tok && tok != last; tok = tok->next())
    {
        if (Token::Match(tok, "if ("))
        {
            const Token * tok1 = tok->tokAt(2);
            const Token * tok2 = tok->tokAt(1)->link();

            if (tok1 && tok2)
            {
                for (; tok1 && tok1 != tok2; tok1 = tok1->next())
                {
                    if (Token::Match(tok1, "this ==|!= & %var%"))
                    {
                        if (tok1->tokAt(3)->str() == rhs->str())
                            return true;
                    }
                    else if (Token::Match(tok1, "& %var% ==|!= this"))
                    {
                        if (tok1->tokAt(1)->str() == rhs->str())
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

static bool hasMultipleInheritanceInline(const Token * tok)
{
    while (tok && tok->str() != "{")
    {
        if (tok->str() == ",")
            return true;

        tok = tok->next();
    }

    return false;
}

static bool hasMultipleInheritanceGlobal(const Token * start, const std::string & name)
{
    const Token *tok = start;
    std::string pattern;
    std::string className = name;

    // check for nested classes
    while (className.find("::") != std::string::npos)
    {
        std::string tempName;

        // there is probably a better way to do this
        while (className[0] != ' ')
        {
            tempName += className[0];
            className.erase(0, 1);
        }

        className.erase(0, 4);

        pattern = "class|struct " + tempName;

        tok = Token::findmatch(tok, pattern.c_str());
    }

    pattern = "class|struct " + className;

    tok = Token::findmatch(tok, pattern.c_str());

    return hasMultipleInheritanceInline(tok);
}

void CheckClass::operatorEqToSelf()
{
    const Token *tok2 = _tokenizer->tokens();
    const Token *tok;

    while ((tok = Token::findmatch(tok2, "operator = (")))
    {
        const Token *tok1 = tok;

        // make sure this is an assignment operator
        if (tok1->tokAt(-2) && Token::Match(tok1->tokAt(-2), " %type% ::"))
        {
            int nameLength = 1;

            tok1 = tok1->tokAt(-2);

            // check backwards for proper function signature
            while (tok1->tokAt(-2) && Token::Match(tok1->tokAt(-2), " %type% ::"))
            {
                tok1 = tok1->tokAt(-2);
                nameLength += 2;
            }

            const Token *name = tok1;
            std::string nameString;

            nameStr(name, nameLength, nameString);

            if (!hasMultipleInheritanceGlobal(_tokenizer->tokens(), nameString))
            {
                if (tok1->tokAt(-1) && tok1->tokAt(-1)->str() == "&")
                {
                    // check returned class name
                    if (tok1->tokAt(-(1 + nameLength)) && nameMatch(name, tok1->tokAt(-(1 + nameLength)), nameLength))
                    {
                        // check forward for proper function signature
                        std::string pattern = "const " + nameString + " & %type% )";
                        if (Token::Match(tok->tokAt(3), pattern.c_str()))
                        {
                            const Token * rhs = tok->tokAt(5 + nameLength);

                            if (nameMatch(name, tok->tokAt(4), nameLength))
                            {
                                tok1 = tok->tokAt(2)->link();

                                if (tok1 && tok1->tokAt(1) && tok1->tokAt(1)->str() == "{" && tok1->tokAt(1)->link())
                                {
                                    const Token *first = tok1->tokAt(1);
                                    const Token *last = first->link();

                                    if (!hasAssignSelf(first, last, rhs))
                                    {
                                        if (hasDeallocation(first, last))
                                            operatorEqToSelfError(tok);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            tok1 = tok;

            // check backwards for proper function signature
            if (tok1->tokAt(-1) && tok1->tokAt(-1)->str() == "&")
            {
                const Token *name = 0;
                while (tok1 && !Token::Match(tok1, "class|struct %var%"))
                    tok1 = tok1->previous();

                if (tok1 && Token::Match(tok1, "struct %var%"))
                    name = tok1->tokAt(1);
                else if (tok1 && Token::Match(tok1, "class %var%"))
                    name = tok1->tokAt(1);

                if (!hasMultipleInheritanceInline(tok1))
                {
                    if (tok->tokAt(-2) && tok->tokAt(-2)->str() == name->str())
                    {
                        // check forward for proper function signature
                        if (Token::Match(tok->tokAt(3), "const %type% & %type% )"))
                        {
                            const Token * rhs = tok->tokAt(6);

                            if (tok->tokAt(4)->str() == name->str())
                            {
                                tok1 = tok->tokAt(2)->link();

                                if (tok1 && tok1->tokAt(1) && tok1->tokAt(1)->str() == "{" && tok1->tokAt(1)->link())
                                {
                                    const Token *first = tok1->tokAt(1);
                                    const Token *last = first->link();

                                    if (!hasAssignSelf(first, last, rhs))
                                    {
                                        if (hasDeallocation(first, last))
                                            operatorEqToSelfError(tok);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        tok2 = tok->next();
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// A destructor in a base class should be virtual
//---------------------------------------------------------------------------

void CheckClass::virtualDestructor()
{
    const char pattern_classdecl[] = "class %var% : %var%";

    const Token *derived = _tokenizer->tokens();
    while ((derived = Token::findmatch(derived, pattern_classdecl)) != NULL)
    {
        // Check that the derived class has a non empty destructor..
        {
            std::ostringstream destructorPattern;
            destructorPattern << "~ " << derived->strAt(1) << " ( ) {";
            const Token *derived_destructor = Token::findmatch(_tokenizer->tokens(), destructorPattern.str().c_str());

            // No destructor..
            if (! derived_destructor)
            {
                derived = derived->next();
                continue;
            }

            // Empty destructor..
            if (Token::Match(derived_destructor, "~ %var% ( ) { }"))
            {
                derived = derived->next();
                continue;
            }
        }

        const Token *derivedClass = derived->tokAt(1);

        // Iterate through each base class...
        derived = derived->tokAt(3);
        while (Token::Match(derived, "%var%"))
        {
            bool isPublic(derived->str() == "public");

            // What kind of inheritance is it.. public|protected|private
            if (Token::Match(derived, "public|protected|private"))
                derived = derived->next();

            // Name of base class..
            const char *baseName[2];
            baseName[0] = derived->strAt(0);
            baseName[1] = 0;

            // Update derived so it's ready for the next loop.
            do
            {
                if (derived->str() == "{")
                    break;

                if (derived->str() == ",")
                {
                    derived = derived->next();
                    break;
                }

                derived = derived->next();
            }
            while (derived);

            // If not public inheritance, skip checking of this base class..
            if (! isPublic)
                continue;

            // Find the destructor declaration for the base class.
            const Token *base = Token::findmatch(_tokenizer->tokens(), (std::string("%any% ~ ") + baseName[0] + " (").c_str());
            while (base && base->str() == "::")
                base = Token::findmatch(base->next(), (std::string("%any% ~ ") + baseName[0] + " (").c_str());

            const Token *reverseTok = base;
            while (Token::Match(base, "%var%") && base->str() != "virtual")
                base = base->previous();

            // Check that there is a destructor..
            if (! base)
            {
                // Is the class declaration available?
                base = Token::findmatch(_tokenizer->tokens(), (std::string("class ") + baseName[0] + " {").c_str());
                if (base)
                {
                    virtualDestructorError(base, baseName[0], derivedClass->str());
                }

                continue;
            }

            // There is a destructor. Check that it's virtual..
            else if (base->str() == "virtual")
                continue;

            // TODO: This is just a temporary fix, better solution is needed.
            // Skip situations where base class has base classes of its own, because
            // some of the base classes might have virtual destructor.
            // Proper solution is to check all of the base classes. If base class is not
            // found or if one of the base classes has virtual destructor, error should not
            // be printed. See TODO test case "virtualDestructorInherited"
            if (!Token::findmatch(_tokenizer->tokens(), (std::string("class ") + baseName[0] + " {").c_str()))
                continue;

            // Make sure that the destructor is public (protected or private
            // would not compile if inheritance is used in a way that would
            // cause the bug we are trying to find here.)
            int indent = 0;
            while (reverseTok)
            {
                if (reverseTok->str() == "public:")
                {
                    virtualDestructorError(base, baseName[0], derivedClass->str());
                    break;
                }
                else if (reverseTok->str() == "protected:" ||
                         reverseTok->str() == "private:")
                {
                    // No bug, protected/private destructor is allowed
                    break;
                }
                else if (reverseTok->str() == "{")
                {
                    indent++;
                    if (indent >= 1)
                    {
                        // We have found the start of the class without any sign
                        // of "public :" so we can assume that the destructor is not
                        // public and there is no bug in the code we are checking.
                        break;
                    }
                }
                else if (reverseTok->str() == "}")
                    indent--;

                reverseTok = reverseTok->previous();
            }
        }
    }
}
//---------------------------------------------------------------------------

void CheckClass::thisSubtractionError(const Token *tok)
{
    reportError(tok, Severity::possibleStyle, "thisSubtraction", "Suspicious pointer subtraction");
}

void CheckClass::thisSubtraction()
{
    const Token *tok = _tokenizer->tokens();
    for (;;)
    {
        tok = Token::findmatch(tok, "this - %var%");
        if (!tok)
            break;

        if (!Token::simpleMatch(tok->previous(), "*"))
            thisSubtractionError(tok);

        tok = tok->next();
    }
}


void CheckClass::checkConst()
{
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "class %var% :|{"))
        {
            // get class name..
            const std::string classname(tok->strAt(1));

            // goto initial {'
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (!tok)
                break;

            // parse in this class definition to see if there are any simple getter functions
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    tok2 = tok2->link();
                else if (tok2->str() == "}")
                    break;

                // member function?
                if (Token::Match(tok2, "[;}] %type% %var% ("))
                {
                    // get function name
                    const std::string functionName(tok2->strAt(2));

                    // goto the ')'
                    tok2 = tok2->tokAt(3)->link();
                    if (!tok2)
                        break;

                    // is this function implemented inline?
                    if (Token::simpleMatch(tok2, ") {"))
                    {
                        // if the function doesn't have any assignment nor function call,
                        // it can be a const function..
                        unsigned int indentlevel = 0;
                        bool isconst = true;
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
                            else if (tok3->str() == "=" ||
                                     Token::Match(tok, "%var% ("))
                            {
                                isconst = false;
                                break;
                            }
                        }

                        // nothing non-const was found. write error..
                        if (isconst)
                            checkConstError(tok2, classname, functionName);
                    }
                }
            }

        }
    }

}

void CheckClass::checkConstError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "functionConst", "The function '" + classname + "::" + funcname + "' can be const");
}



void CheckClass::noConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    reportError(tok, Severity::style, "noConstructor", "The " + std::string(isStruct ? "struct" : "class") + " '" + classname + "' has no constructor. Member variables not initialized.");
}

void CheckClass::uninitVarError(const Token *tok, const std::string &classname, const std::string &varname, bool hasPrivateConstructor)
{
    reportError(tok, hasPrivateConstructor ? Severity::possibleStyle : Severity::style, "uninitVar", "Member variable not initialized in the constructor '" + classname + "::" + varname + "'");
}

void CheckClass::operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::possibleStyle, "operatorEqVarError", "Member variable '" + classname + "::" + varname + "' is not assigned a value in '" + classname + "::operator=" + "'");
}

void CheckClass::unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "unusedPrivateFunction", "Unused private function '" + classname + "::" + funcname + "'");
}

void CheckClass::memsetClassError(const Token *tok, const std::string &memfunc)
{
    reportError(tok, Severity::error, "memsetClass", "Using '" + memfunc + "' on class");
}

void CheckClass::memsetStructError(const Token *tok, const std::string &memfunc, const std::string &classname)
{
    reportError(tok, Severity::error, "memsetStruct", "Using '" + memfunc + "' on struct that contains a 'std::" + classname + "'");
}

void CheckClass::operatorEqReturnError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEq", "'operator=' should return something");
}

void CheckClass::virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived)
{
    reportError(tok, Severity::error, "virtualDestructor", "Class " + Base + " which is inherited by class " + Derived + " does not have a virtual destructor");
}

void CheckClass::operatorEqRetRefThisError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqRetRefThis", "'operator=' should return reference to self");
}

void CheckClass::operatorEqToSelfError(const Token *tok)
{
    reportError(tok, Severity::possibleStyle, "operatorEqToSelf", "'operator=' should check for assignment to self");
}

