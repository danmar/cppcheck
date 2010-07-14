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

        // Borland C++: Skip all variables in the __published section.
        // These are automaticly initialized.
        if (tok->str() == "__published:")
        {
            priv = false;
            for (; tok; tok = tok->next())
            {
                if (tok->str() == "{")
                    tok = tok->link();
                if (Token::Match(tok->next(), "private:|protected:|public:"))
                    break;
            }
            if (tok)
                continue;
            else
                break;
        }

        // "private:" "public:" "protected:" etc
        const bool b((tok->str()[0] != ':') && tok->str().find(":") != std::string::npos);

        if (b)
            priv = bool(tok->str() == "private:");

        // Search for start of statement..
        if (! Token::Match(tok, "[;{}]") && ! b)
            continue;

        // This is the start of a statement
        const Token *next = tok->next();
        std::string varname;

        // If next token contains a ":".. it is not part of a variable declaration
        if (next->str().find(":") != std::string::npos)
            continue;

        // Borland C++: Ignore properties..
        if (next->str() == "__property")
            continue;

        // Is it const..?
        if (next->str() == "const")
        {
            next = next->next();
        }

        // Is it a static variable?
        const bool isStatic(Token::simpleMatch(next, "static"));
        if (isStatic)
        {
            next = next->next();
        }

        // Is it a mutable variable?
        const bool isMutable(Token::simpleMatch(next, "mutable"));
        if (isMutable)
        {
            next = next->next();
        }

        // Is it const..?
        if (next->str() == "const")
        {
            next = next->next();
        }

        // Is it a variable declaration?
        if (Token::Match(next, "%type% %var% ;|:"))
        {
            if (withClasses)
                varname = next->strAt(1);
            else if (next->isStandardType())
                varname = next->strAt(1);
            else if (Token::findmatch(_tokenizer->tokens(), ("enum " + next->str()).c_str()))
                varname = next->strAt(1);
        }

        // Structure?
        else if (Token::Match(next, "struct|union %type% %var% ;"))
        {
            varname = next->strAt(2);
        }

        // Pointer?
        else if (Token::Match(next, "%type% * %var% ;"))
            varname = next->strAt(2);
        else if (Token::Match(next, "%type% %type% * %var% ;"))
            varname = next->strAt(3);
        else if (Token::Match(next, "%type% :: %type% * %var% ;"))
            varname = next->strAt(4);

        // Array?
        else if (Token::Match(next, "%type% %var% [") && next->next()->str() != "operator")
        {
            if (!withClasses && !next->isStandardType())
            {
                continue;
            }
            varname = next->strAt(1);
        }

        // Pointer array?
        else if (Token::Match(next, "%type% * %var% ["))
            varname = next->strAt(2);
        else if (Token::Match(next, "%type% :: %type% * %var% ["))
            varname = next->strAt(4);

        // std::string..
        else if (withClasses && Token::Match(next, "%type% :: %type% %var% ;"))
        {
            varname = next->strAt(3);
        }

        // Container..
        else if (withClasses && (Token::Match(next, "%type% :: %type% <") ||
                                 Token::Match(next, "%type% <")))
        {
            // find matching ">"
            int level = 0;
            for (; next; next = next->next())
            {
                if (next->str() == "<")
                    level++;
                else if (next->str() == ">")
                {
                    level--;
                    if (level == 0)
                        break;
                }
            }
            if (next && Token::Match(next, "> %var% ;"))
                varname = next->strAt(1);
            else if (next && Token::Match(next, "> * %var% ;"))
                varname = next->strAt(2);
        }

        // If the varname was set in the if-blocks above, create a entry for this variable..
        if (!varname.empty() && varname != "operator")
        {
            Var *var = new Var(varname, false, priv, isMutable, isStatic, varlist);
            varlist  = var;
        }
    }

    return varlist;
}
//---------------------------------------------------------------------------

void CheckClass::initVar(Var *varlist, const std::string &varname)
{
    for (Var *var = varlist; var; var = var->next)
    {
        if (var->name == varname)
        {
            var->init = true;
            return;
        }
    }
}
//---------------------------------------------------------------------------

void CheckClass::initializeVarList(const Token *tok1, const Token *ftok, Var *varlist, const std::string &classname, std::list<std::string> &callstack, bool isStruct)
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

                // assignment in the initializer..
                // : var(value = x)
                if (Token::Match(ftok->tokAt(2), "%var% ="))
                    initVar(varlist, ftok->strAt(2));
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

        // Before a new statement there is "[{};)=]"
        if (! Token::Match(ftok, "[{};()=]"))
            continue;

        if (Token::simpleMatch(ftok, "( !"))
            ftok = ftok->next();

        // Using the operator= function to initialize all variables..
        if (Token::simpleMatch(ftok->next(), "* this = "))
        {
            for (Var *var = varlist; var; var = var->next)
                var->init = true;
            break;
        }

        if (Token::Match(ftok->next(), "%var% . %var% ("))
            ftok = ftok->tokAt(2);

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
            return;
        }

        // Clearing array..
        else if (Token::Match(ftok, "memset ( %var% ,"))
        {
            initVar(varlist, ftok->strAt(2));
            ftok = ftok->next()->link();
            continue;
        }

        // Calling member function?
        else if (Token::Match(ftok, "%var% (") && ftok->str() != "if")
        {
            // Passing "this" => assume that everything is initialized
            for (const Token * tok2 = ftok->next()->link(); tok2 && tok2 != ftok; tok2 = tok2->previous())
            {
                if (tok2->str() == "this")
                {
                    for (Var *var = varlist; var; var = var->next)
                        var->init = true;
                    return;
                }
            }

            // recursive call / calling overloaded function
            // assume that all variables are initialized
            if (std::find(callstack.begin(), callstack.end(), ftok->str()) != callstack.end())
            {
                for (Var *var = varlist; var; var = var->next)
                    var->init = true;
                return;
            }

            int i = 0;
            const Token *ftok2 = _tokenizer->findClassFunction(tok1, classname, ftok->strAt(0), i, isStruct);
            if (ftok2)
            {
                callstack.push_back(ftok->str());
                initializeVarList(tok1, ftok2, varlist, classname, callstack, isStruct);
                callstack.pop_back();
            }
            else  // there is a called member function, but it is not defined where we can find it, so we assume it initializes everything
            {
                // check if the function is part of this class..
                const Token *tok = Token::findmatch(_tokenizer->tokens(), ((isStruct ? std::string("struct ") : std::string("class ")) + classname + " {").c_str());
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
                unsigned int indentlevel2 = 0;
                for (tok = ftok->tokAt(2); tok; tok = tok->next())
                {
                    if (tok->str() == "(")
                        ++indentlevel2;
                    else if (tok->str() == ")")
                    {
                        if (indentlevel2 == 0)
                            break;
                        --indentlevel2;
                    }
                    if (tok->isName())
                    {
                        initVar(varlist, tok->strAt(0));
                    }
                }
                continue;
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

        // Assignment of struct member of member variable?
        else if (Token::Match(ftok, "%var% . %any% ="))
        {
            initVar(varlist, ftok->strAt(0));
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

struct Constructor
{
    enum AccessControl { Public, Protected, Private };

    Constructor(const Token * tok, AccessControl acc, bool body, bool equal, bool copy) :
        token(tok), access(acc), hasBody(body), isOperatorEqual(equal), isCopyConstructor(copy)
    {
    }

    const Token *token;
    AccessControl access;
    bool hasBody;
    bool isOperatorEqual;
    bool isCopyConstructor;
};

static bool argsMatch(const Token *first, const Token *second)
{
    bool match = false;
    while (first->str() == second->str())
    {
        // at end of argument list
        if (first->str() == ")")
        {
            match = true;
            break;
        }

        // skip default value assignment
        else if (first->next()->str() == "=")
        {
            first = first->tokAt(2);
            continue;
        }

        // definition missing variable name
        else if (first->next()->str() == "," && second->next()->str() != ",")
            second = second->next();
        else if (first->next()->str() == ")" && second->next()->str() != ")")
            second = second->next();

        // argument list has different number of arguments
        else if (second->str() == ")")
            break;

        // variable names are different
        else if ((Token::Match(first->next(), "%var% ,|)|=") &&
                  Token::Match(second->next(), "%var% ,|)")) &&
                 (first->next()->str() != second->next()->str()))
        {
            // skip variable names
            first = first->next();
            second = second->next();

            // skip default value assignment
            if (first->next()->str() == "=")
                first = first->tokAt(2);
        }

        first = first->next();
        second = second->next();
    }

    return match;
}

void CheckClass::constructors()
{
    if (!_settings->_checkCodingStyle)
        return;

    const char pattern_class[] = "class|struct %var% [{:]";

    // Locate class
    const Token *tok1 = Token::findmatch(_tokenizer->tokens(), pattern_class);
    for (; tok1; tok1 = Token::findmatch(tok1->next(), pattern_class))
    {
        const std::string className = tok1->strAt(1);
        bool isStruct = tok1->str() == "struct";

        const Token *end = tok1;
        while (end->str() != "{")
            end = end->next();
        end = end->link();

        int indentlevel = 0;
        Constructor::AccessControl access = isStruct ? Constructor::Public : Constructor::Private;
        std::list<Constructor> constructorList;
        unsigned int numConstructors = 0;

        // check to end of class definition
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
                // What section are we in..
                if (tok->str() == "private:")
                    access = Constructor::Private;
                else if (tok->str() == "protected:")
                    access = Constructor::Protected;
                else if (tok->str() == "public:")
                    access = Constructor::Public;

                // Is there a constructor?
                else if (Token::simpleMatch(tok, (className + " (").c_str()) ||
                         Token::simpleMatch(tok, "operator = ("))
                {
                    bool operatorEqual = tok->str() == "operator";
                    bool copyConstructor = Token::Match(tok, "%var% ( const %var% & %var%| )");
                    const Token *next = operatorEqual ? tok->tokAt(2)->link() : tok->next()->link();

                    if (!operatorEqual)
                        numConstructors++;

                    // out of line function
                    if (Token::Match(next, ") ;"))
                    {
                        std::string classPattern;
                        int offset;
                        if (operatorEqual)
                        {
                            classPattern = className + " :: operator = (";
                            offset = 5;
                        }
                        else
                        {
                            classPattern = className + " :: " + className + " (";
                            offset = 4;
                        }

                        bool hasBody = false;

                        // start looking at end of class
                        const Token *constructor_token = end;
                        while ((constructor_token = Token::findmatch(constructor_token, classPattern.c_str())) != NULL)
                        {
                            // skip destructor and other classes
                            if (!Token::Match(constructor_token->previous(), "~|::"))
                            {
                                if (argsMatch(tok->tokAt(offset - 2), constructor_token->tokAt(offset)))
                                {
                                    if (operatorEqual || copyConstructor)
                                        constructorList.push_back(Constructor(constructor_token, access, true, operatorEqual, copyConstructor));
                                    else
                                        constructorList.push_front(Constructor(constructor_token, access, true, false, false));

                                    hasBody = true;
                                    break;
                                }
                            }

                            // skip function body
                            while (constructor_token->str() != "{")
                                constructor_token = constructor_token->next();
                            constructor_token = constructor_token->link();
                        }

                        // function body found?
                        if (!hasBody)
                        {
                            if (operatorEqual || copyConstructor)
                                constructorList.push_back(Constructor(tok, access, false, operatorEqual, copyConstructor));
                            else
                                constructorList.push_front(Constructor(tok, access, false, false, false));
                        }

                        tok = next->next();
                    }

                    // inline function
                    else
                    {
                        // skip destructor and other classes
                        if (!Token::Match(tok->previous(), "~|::"))
                        {
                            if (operatorEqual || copyConstructor)
                                constructorList.push_back(Constructor(tok, access, true, operatorEqual, copyConstructor));
                            else
                                constructorList.push_front(Constructor(tok, access, true, false, false));
                        }

                        // skip over function body
                        tok = next->next();
                        while (tok->str() != "{")
                            tok = tok->next();
                        tok = tok->link();
                    }
                }
            }
        }

        // Get variables that are not classes...
        Var *varlist = getVarList(tok1, false, isStruct);

        // There are no constructors.
        if (numConstructors == 0)
        {
            // If "--style" has been given, give a warning
            if (_settings->_checkCodingStyle)
            {
                // If there is a private variable, there should be a constructor..
                for (const Var *var = varlist; var; var = var->next)
                {
                    if (var->priv && !var->isStatic)
                    {
                        noConstructorError(tok1, className, isStruct);
                        break;
                    }
                }
            }
        }

        std::list<Constructor>::const_iterator it;
        bool hasClasses = false;

        for (it = constructorList.begin(); it != constructorList.end(); ++it)
        {
            // check for end of regular constructors and start of copy constructors
            bool needClasses = it->isCopyConstructor || it->isOperatorEqual;
            if (needClasses != hasClasses)
            {
                hasClasses = needClasses;

                // Delete the varlist..
                while (varlist)
                {
                    Var *nextvar = varlist->next;
                    delete varlist;
                    varlist = nextvar;
                }

                // Get variables including classes
                varlist = getVarList(tok1, true, isStruct);
            }

            if (!it->hasBody)
                continue;

            std::list<std::string> callstack;
            initializeVarList(tok1, it->token, varlist, className, callstack, isStruct);

            // Check if any variables are uninitialized
            for (Var *var = varlist; var; var = var->next)
            {
                if (var->init || var->isStatic)
                    continue;

                // It's non-static and it's not initialized => error
                if (it->isOperatorEqual)
                {
                    const Token *operStart = 0;
                    if (Token::simpleMatch(it->token, "operator = ("))
                        operStart = it->token->tokAt(2);
                    else
                        operStart = it->token->tokAt(4);

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
                        operatorEqVarError(it->token, className, var->name);
                }
                else if (it->access != Constructor::Private && !var->isStatic)
                    uninitVarError(it->token, className, var->name);
            }

            // Mark all variables not used
            for (Var *var = varlist; var; var = var->next)
                var->init = false;
        }

        // Delete the varlist..
        while (varlist)
        {
            Var *nextvar = varlist->next;
            delete varlist;
            varlist = nextvar;
        }
    }
}

//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

void CheckClass::privateFunctions()
{
    if (!_settings->_checkCodingStyle)
        return;

    const char pattern_class[] = "class|struct %var% {|:";

    // Locate some class
    for (const Token *tok1 = Token::findmatch(_tokenizer->tokens(), pattern_class);
         tok1; tok1 = Token::findmatch(tok1->next(), pattern_class))
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
            if ((inclass && indent_level == 1 && Token::Match(ftok, "%var% (")) ||
                (Token::Match(ftok, (classname + " :: ~| %var% (").c_str())))
            {
                while (ftok && ftok->str() != ")")
                    ftok = ftok->next();
                if (!ftok)
                    break;
                if (Token::Match(ftok, ") : %var% ("))
                {
                    while (!Token::Match(ftok->next(), "[{};]"))
                    {
                        if (Token::Match(ftok, "::|,|( %var% ,|)"))
                        {
                            // Remove function from FuncList
                            std::list<const Token *>::iterator it = FuncList.begin();
                            while (it != FuncList.end())
                            {
                                if (ftok->next()->str() == (*it)->str())
                                    FuncList.erase(it++);
                                else
                                    it++;
                            }
                        }
                        ftok = ftok->next();
                    }
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

        std::string type;
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
        if (type.empty())
            continue;

        // Warn if type is a class or struct that contains any std::* variables
        const std::string pattern2(std::string("struct|class ") + type + " {");
        for (const Token *tstruct = Token::findmatch(_tokenizer->tokens(), pattern2.c_str()); tstruct; tstruct = tstruct->next())
        {
            if (tstruct->str() == "}")
                break;

            // struct with function? skip function body..
            if (Token::simpleMatch(tstruct, ") {"))
            {
                tstruct = tstruct->next()->link();
                if (!tstruct)
                    break;
            }

            // before a statement there must be either:
            // * private:|protected:|public:
            // * { } ;
            if (Token::Match(tstruct, "[;{}]") ||
                tstruct->str().find(":") != std::string::npos)
            {
                if (Token::Match(tstruct->next(), "std :: %type% %var% ;"))
                    memsetStructError(tok, tok->str(), tstruct->strAt(3));

                else if (Token::Match(tstruct->next(), "std :: %type% < "))
                {
                    // backup the type
                    const std::string typestr(tstruct->strAt(3));

                    // check if it's a pointer variable..
                    unsigned int level = 0;
                    while (0 != (tstruct = tstruct->next()))
                    {
                        if (tstruct->str() == "<")
                            ++level;
                        else if (tstruct->str() == ">")
                        {
                            if (level <= 1)
                                break;
                            --level;
                        }
                        else if (tstruct->str() == "(")
                            tstruct = tstruct->link();
                    }

                    if (!tstruct)
                        break;

                    // found error => report
                    if (Token::Match(tstruct, "> %var% ;"))
                        memsetStructError(tok, tok->str(), typestr);
                }
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
    if (!_settings->_checkCodingStyle)
        return;

    const Token *tok2 = _tokenizer->tokens();
    const Token *tok;

    while ((tok = Token::findmatch(tok2, "void operator = (")) != NULL)
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

void CheckClass::operatorEqRetRefThis_finderr(const Token *tok, const std::string &classname)
{
    // find the ')'
    const Token *tok1 = tok->tokAt(2)->link();

    // is there an implementation?
    if (Token::simpleMatch(tok1, ") {"))
    {
        bool foundReturn = false;
        const Token *last = tok1->next()->link();
        for (tok1 = tok1->tokAt(2); tok1 && tok1 != last; tok1 = tok1->next())
        {
            // check for return of reference to this
            if (tok1->str() == "return")
            {
                foundReturn = true;
                std::string cast("( " + classname + " & )");
                if (Token::Match(tok1->next(), cast.c_str()))
                    tok1 = tok1->tokAt(4);

                if (!(Token::Match(tok1->tokAt(1), "(| * this ;|=") ||
                      Token::Match(tok1->tokAt(1), "(| * this +=") ||
                      Token::Match(tok1->tokAt(1), "operator = (")))
                    operatorEqRetRefThisError(tok);
            }
        }
        if (!foundReturn)
            operatorEqRetRefThisError(tok);
    }
}

void CheckClass::operatorEqRetRefThis()
{
    if (!_settings->_checkCodingStyle)
        return;

    const Token *tok2 = _tokenizer->tokens();
    const Token *tok;

    while ((tok = Token::findmatch(tok2, "operator = (")) != NULL)
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

            const Token *className = tok1;
            std::string nameString;

            nameStr(className, nameLength, nameString);

            if (tok1->tokAt(-1) && tok1->tokAt(-1)->str() == "&")
            {
                // check class name
                if (tok1->tokAt(-(1 + nameLength)) && nameMatch(className, tok1->tokAt(-(1 + nameLength)), nameLength))
                {
                    operatorEqRetRefThis_finderr(tok, className->str());
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
                const Token *className = 0;
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

                if (Token::Match(tok1, "struct|class %var%"))
                {
                    // data members in structs are public by default
                    isPublic = bool(tok1->str() == "struct");

                    className = tok1->tokAt(1);
                }

                if (tok->tokAt(-2) && tok->tokAt(-2)->str() == className->str())
                {
                    operatorEqRetRefThis_finderr(tok, className->str());
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
        if (Token::Match(tok, "{|;|, free ( %var%"))
        {
            const Token * var = tok->tokAt(3);

            // we should probably check that var is a pointer in this class

            const Token * tok1 = tok->tokAt(4);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%var% ="))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
        else if (Token::Match(tok, "{|;|, delete [ ] %var%"))
        {
            const Token * var = tok->tokAt(4);

            // we should probably check that var is a pointer in this class

            const Token * tok1 = tok->tokAt(5);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%var% = new %type% ["))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
        else if (Token::Match(tok, "{|;|, delete %var%"))
        {
            const Token * var = tok->tokAt(2);

            // we should probably check that var is a pointer in this class

            const Token * tok1 = tok->tokAt(3);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%var% = new"))
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
    if (!_settings->_checkCodingStyle)
        return;

    const Token *tok2 = _tokenizer->tokens();
    const Token *tok;

    while ((tok = Token::findmatch(tok2, "operator = (")) != NULL)
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

            const Token *className = tok1;
            std::string nameString;

            nameStr(className, nameLength, nameString);

            if (!hasMultipleInheritanceGlobal(_tokenizer->tokens(), nameString))
            {
                if (tok1->tokAt(-1) && tok1->tokAt(-1)->str() == "&")
                {
                    // check returned class name
                    if (tok1->tokAt(-(1 + nameLength)) && nameMatch(className, tok1->tokAt(-(1 + nameLength)), nameLength))
                    {
                        // check forward for proper function signature
                        std::string pattern = "const " + nameString + " & %var% )";
                        if (Token::Match(tok->tokAt(3), pattern.c_str()))
                        {
                            const Token * rhs = tok->tokAt(5 + nameLength);

                            if (nameMatch(className, tok->tokAt(4), nameLength))
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
                const Token *className = 0;
                while (tok1 && !Token::Match(tok1, "class|struct %var%"))
                    tok1 = tok1->previous();

                if (Token::Match(tok1, "struct|class %var%"))
                    className = tok1->tokAt(1);

                if (!hasMultipleInheritanceInline(tok1))
                {
                    if (Token::simpleMatch(tok->tokAt(-2), className->str().c_str()))
                    {
                        // check forward for proper function signature
                        if (Token::Match(tok->tokAt(3), "const %type% & %var% )"))
                        {
                            const Token * rhs = tok->tokAt(6);

                            if (tok->tokAt(4)->str() == className->str())
                            {
                                tok1 = tok->tokAt(2)->link();

                                if (tok1 && Token::simpleMatch(tok1->next(), "{"))
                                {
                                    const Token *first = tok1->next();
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
    // This error should only be given if:
    // * base class doesn't have virtual destructor
    // * derived class has non-empty destructor
    // * base class is deleted
    if (!_settings->inconclusive)
        return;

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
            const std::string baseName = derived->strAt(0);

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
            const Token *base = Token::findmatch(_tokenizer->tokens(), (std::string("%any% ~ ") + baseName + " (").c_str());
            while (base && base->str() == "::")
                base = Token::findmatch(base->next(), (std::string("%any% ~ ") + baseName + " (").c_str());

            const Token *reverseTok = base;
            while (Token::Match(base, "%var%") && base->str() != "virtual")
                base = base->previous();

            // Check that there is a destructor..
            if (! base)
            {
                // Is the class declaration available?
                base = Token::findmatch(_tokenizer->tokens(), (std::string("class ") + baseName + " {").c_str());
                if (base)
                {
                    virtualDestructorError(base, baseName, derivedClass->str());
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
            if (!Token::findmatch(_tokenizer->tokens(), (std::string("class ") + baseName + " {").c_str()))
                continue;

            // Make sure that the destructor is public (protected or private
            // would not compile if inheritance is used in a way that would
            // cause the bug we are trying to find here.)
            int indent = 0;
            while (reverseTok)
            {
                if (reverseTok->str() == "public:")
                {
                    virtualDestructorError(base, baseName, derivedClass->str());
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
    reportError(tok, Severity::style, "thisSubtraction", "Suspicious pointer subtraction");
}

void CheckClass::thisSubtraction()
{
    if (!_settings->_checkCodingStyle)
        return;

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
//---------------------------------------------------------------------------

struct NestInfo
{
    std::string className;
    const Token * classEnd;
    int levelEnd;
};

// Can a function be const?
void CheckClass::checkConst()
{
    if (!_settings->_checkCodingStyle)
        return;

    std::vector<NestInfo> nestInfo;
    int level = 0;
    Var *varlist = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{" && !nestInfo.empty())
            level++;
        else if (tok->str() == "}" && !nestInfo.empty())
        {
            level--;
            if (level == nestInfo.back().levelEnd)
                nestInfo.pop_back();
        }
        else if (Token::Match(tok, "class|struct %var% {"))
        {
            const Token *classTok = tok;

            // get class name..
            std::string classname(tok->strAt(1));

            // goto initial {'
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (!tok)
                break;

            const Token *classEnd = tok->link();

            NestInfo info;
            info.className = classname;
            info.classEnd = classEnd;
            info.levelEnd = level++;
            nestInfo.push_back(info);

            // Delete the varlist..
            while (varlist)
            {
                Var *nextvar = varlist->next;
                delete varlist;
                varlist = nextvar;
            }

            // Get class variables...
            varlist = getVarList(classTok, true, classTok->str() == "struct");

            // parse in this class definition to see if there are any simple getter functions
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    tok2 = tok2->link();
                else if (tok2->str() == "}")
                    break;

                // start of statement?
                if (!Token::Match(tok2->previous(), "[;{}]"))
                    continue;

                // skip private: public: etc
                if (tok2->isName() && tok2->str().find(":") != std::string::npos)
                {
                    if (tok2->next()->str() == "}")
                        continue;
                    tok2 = tok2->next();
                }

                // static functions can't be const
                // virtual functions may be non-const for a reason
                if (Token::Match(tok2, "static|virtual"))
                    continue;

                // don't warn for unknown types..
                // LPVOID, HDC, etc
                if (tok2->isName())
                {
                    bool allupper = true;
                    const std::string s(tok2->str());
                    for (std::string::size_type pos = 0; pos < s.size(); ++pos)
                    {
                        unsigned char ch = s[pos];
                        if (!(ch == '_' || (ch >= 'A' && ch <= 'Z')))
                        {
                            allupper = false;
                            break;
                        }
                    }

                    if (allupper)
                        continue;
                }

                // member function?
                if (isMemberFunc(tok2))
                {
                    // goto function name..
                    while (tok2->next()->str() != "(")
                        tok2 = tok2->next();

                    // get function name
                    const std::string functionName((tok2->isName() ? "" : "operator") + tok2->str());

                    // skip constructor
                    if (functionName == classname)
                        continue;

                    // goto the ')'
                    tok2 = tok2->next()->link();
                    if (!tok2)
                        break;

                    // is this a non-const function that is implemented inline?
                    if (Token::simpleMatch(tok2, ") {"))
                    {
                        const Token *paramEnd = tok2;

                        // if nothing non-const was found. write error..
                        if (checkConstFunc(classname, varlist, paramEnd))
                        {
                            for (int i = nestInfo.size() - 2; i >= 0; i--)
                                classname = std::string(nestInfo[i].className + "::" + classname);

                            checkConstError(tok2, classname, functionName);
                        }
                    }
                    else if (Token::simpleMatch(tok2, ") ;")) // not inline
                    {
                        for (int i = nestInfo.size() - 1; i >= 0; i--)
                        {
                            const Token *found = nestInfo[i].classEnd;
                            std::string pattern(functionName + " (");
                            int namespaceLevel = 0;
                            for (int j = nestInfo.size() - 1; j >= i; j--, namespaceLevel++)
                                pattern = std::string(nestInfo[j].className + " :: " + pattern);
                            while ((found = Token::findmatch(found->next(), pattern.c_str())) != NULL)
                            {
                                const Token *paramEnd = found->tokAt(1 + (2 * namespaceLevel))->link();
                                if (!paramEnd)
                                    break;
                                if (paramEnd->next()->str() != "{")
                                    break;

                                if (sameFunc(namespaceLevel, tok2, paramEnd))
                                {
                                    // if nothing non-const was found. write error..
                                    if (checkConstFunc(classname, varlist, paramEnd))
                                    {
                                        for (int k = nestInfo.size() - 2; k >= 0; k--)
                                            classname = std::string(nestInfo[k].className + "::" + classname);

                                        checkConstError2(found, tok2, classname, functionName);
                                    }
                                }
                            }
                        }
                    }
                }
            }
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

bool CheckClass::sameFunc(int nest, const Token *firstEnd, const Token *secondEnd)
{
    // check return type (search backwards until previous statement)
    const Token * firstStart = firstEnd->link()->tokAt(-2);
    const Token * secondStart = secondEnd->link()->tokAt((-2 * nest) - 2);

    bool firstDone = false;
    bool secondDone = false;

    for ( ; ; )
    {
        firstDone = false;
        secondDone = false;

        if (!firstStart || Token::Match(firstStart, ";|}|{|public:|protected:|private:"))
            firstDone = true;

        if (!secondStart || Token::Match(secondStart, ";|}|{|public:|protected:|private:"))
            secondDone = true;

        if (firstDone != secondDone)
            return false;

        // both done and match
        if (firstDone)
            break;

        if (secondStart->str() != firstStart->str())
            return false;

        firstStart = firstStart->previous();
        secondStart = secondStart->previous();
    }

    // check parameter types (names can be different or missing)
    firstStart = firstEnd->link()->next();
    secondStart = secondEnd->link()->next();

    for ( ; ; )
    {
        firstDone = false;
        secondDone = false;
        bool again = true;

        while (again)
        {
            again = false;

            if (firstStart == firstEnd)
                firstDone = true;

            if (secondStart == secondEnd)
                secondDone = true;

            // possible difference in number of parameters
            if (firstDone != secondDone)
            {
                // check for missing names
                if (firstDone)
                {
                    if (secondStart->varId() != 0)
                        again = true;
                }
                else
                {
                    if (firstStart->varId() != 0)
                        again = true;
                }

                if (!again)
                    return false;
            }

            // both done and match
            if (firstDone && !again)
                return true;

            if (firstStart->varId() != 0)
            {
                // skip variable name
                firstStart = firstStart->next();
                again = true;
            }

            if (secondStart->varId() != 0)
            {
                // skip variable name
                secondStart = secondStart->next();
                again = true;
            }
        }

        if (firstStart->str() != secondStart->str())
            return false;

        // retry after skipped variable names
        if (!again)
        {
            firstStart = firstStart->next();
            secondStart = secondStart->next();
        }
    }

    return true;
}

// A const member function can return either a copy of or
// a const reference or pointer to something.
// Is this a member function with these signatures:
// type var (                    returns a copy of something
// const type var (              returns a const copy of something
// const type & var (            returns a const reference to something
// const type * var (            returns a const pointer to something
// type operator any (           returns a copy of something
// const type operator any (     returns a const copy of something
// const type & operator any (   returns a const reference to something
// const type * operator any (   returns a const pointer to something
// Type can be anything from a standard type to a complex template.
bool CheckClass::isMemberFunc(const Token *tok)
{
    bool isConst = false;

    if (tok->str() == "const")
    {
        isConst = true;
        tok = tok->next();
    }

    if (Token::Match(tok, "%type%"))
    {
        tok = tok->next();

        if (Token::Match(tok, "%var% ("))
            return true;
        else if (Token::Match(tok, "operator %any% ("))
            return true;

        while (Token::Match(tok, ":: %type%"))
            tok = tok->tokAt(2);

        // template with parameter(s)?
        if (Token::Match(tok, "< %type%"))
        {
            unsigned int level = 1;
            tok = tok->tokAt(2);
            while (tok)
            {
                if (tok->str() == "<")
                    level++;
                else if (tok->str() == ">")
                {
                    level--;
                    if (level == 0)
                    {
                        tok = tok->next();
                        break;
                    }
                }

                // check for templates returning pointers or references
                else if (Token::Match(tok, "*|&"))
                {
                    int back = -2;
                    if (tok->strAt(back) == "::")
                        back -= 2;

                    if (tok->strAt(back) != "const")
                    {
                        if (!isConst)
                            return false;
                    }
                }
                tok = tok->next();
            }
        }

        // template with default type
        else if (Token::Match(tok, "< >"))
            tok = tok->tokAt(2);

        // operator something
        else if (Token::Match(tok, "operator %any% ("))
            return true;

        if (isConst)
        {
            while (Token::Match(tok, "*|&"))
            {
                tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();
            }
        }

        if (Token::Match(tok, "%var% ("))
            return true;
    }

    return false;
}

bool CheckClass::isMemberVar(const std::string &classname, const Var *varlist, const Token *tok)
{
    while (tok->previous() && !Token::Match(tok->previous(), "}|{|;|public:|protected:|private:|return|:|?"))
    {
        if (Token::Match(tok->previous(),  "* this"))
            return true;

        tok = tok->previous();
    }

    if (tok->str() == "this")
        return true;

    if (Token::Match(tok, "( * %var% ) ["))
        tok = tok->tokAt(2);

    // ignore class namespace
    if (tok->str() == classname && tok->next()->str() == "::")
        tok = tok->tokAt(2);

    for (const Var *var = varlist; var; var = var->next)
    {
        if (var->name == tok->str())
        {
            return !var->isMutable;
        }
    }

    return false;
}

bool CheckClass::checkConstFunc(const std::string &classname, const Var *varlist, const Token *tok)
{
    // if the function doesn't have any assignment nor function call,
    // it can be a const function..
    unsigned int indentlevel = 0;
    bool isconst = true;
    for (const Token *tok1 = tok; tok1; tok1 = tok1->next())
    {
        if (tok1->str() == "{")
            ++indentlevel;
        else if (tok1->str() == "}")
        {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        }

        // assignment.. = += |= ..
        else if (tok1->str() == "=" ||
                 (tok1->str().find("=") == 1 &&
                  tok1->str().find_first_of("<!>") == std::string::npos))
        {
            if (isMemberVar(classname, varlist, tok1->previous()))
            {
                isconst = false;
                break;
            }
        }

        // streaming: <<
        else if (tok1->str() == "<<" && isMemberVar(classname, varlist, tok1->previous()))
        {
            isconst = false;
            break;
        }

        // increment/decrement (member variable?)..
        else if (Token::Match(tok1, "++|--"))
        {
            isconst = false;
            break;
        }

        // function call..
        else if ((tok1->str() != "return" && Token::Match(tok1, "%var% (") && tok1->str() != "c_str") ||
                 Token::Match(tok1, "%var% < %any% > ("))
        {
            isconst = false;
            break;
        }

        // delete..
        else if (tok1->str() == "delete")
        {
            isconst = false;
            break;
        }
    }

    return isconst;
}

void CheckClass::checkConstError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "functionConst", "The function '" + classname + "::" + funcname + "' can be const");
}

void CheckClass::checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname)
{
    std::list<const Token *> toks;
    toks.push_back(tok1);
    toks.push_back(tok2);
    reportError(toks, Severity::style, "functionConst", "The function '" + classname + "::" + funcname + "' can be const");
}

void CheckClass::noConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    reportError(tok, Severity::style, "noConstructor", "The " + std::string(isStruct ? "struct" : "class") + " '" + classname + "' has no constructor. Member variables not initialized.");
}

void CheckClass::uninitVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::style, "uninitVar", "Member variable not initialized in the constructor '" + classname + "::" + varname + "'");
}

void CheckClass::operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::style, "operatorEqVarError", "Member variable '" + classname + "::" + varname + "' is not assigned a value in '" + classname + "::operator=" + "'");
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
    reportError(tok, Severity::style, "operatorEqToSelf", "'operator=' should check for assignment to self");
}
