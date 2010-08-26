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

CheckClass::CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : Check(tokenizer, settings, errorLogger),
      hasSymbolDatabase(false)
{

}

void CheckClass::createSymbolDatabase()
{
    // Multiple calls => bail out
    if (hasSymbolDatabase)
        return;

    hasSymbolDatabase = true;

    // find all namespaces (class,struct and namespace)
    SpaceInfo *info = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Locate next class
        if (Token::Match(tok, "class|struct|namespace %var% [{:]"))
        {
            SpaceInfo *new_info = new SpaceInfo;
            new_info->check = this;
            new_info->isNamespace = tok->str() == "namespace";
            new_info->className = tok->next()->str();
            new_info->classDef = tok;
            new_info->nest = info;
            new_info->access = tok->str() == "struct" ? Public : Private;
            new_info->numConstructors = 0;

            const Token *tok2 = tok->tokAt(2);

            // only create variable list and base list if not namespace
            if (!new_info->isNamespace)
            {
                new_info->getVarList();

                // goto initial '{'
                tok2 = initBaseInfo(new_info, tok);
            }

            new_info->classStart = tok2;
            new_info->classEnd = new_info->classStart->link();

            info = new_info;

            // add namespace
            spaceInfoMMap.insert(std::make_pair(info->className, info));

            tok = tok2;
        }

        // check if in space
        else if (info)
        {
            // check for end of space
            if (tok == info->classEnd)
            {
                info = info->nest;
            }

            // check if in class or structure
            else if (!info->isNamespace)
            {
                // What section are we in..
                if (tok->str() == "private:")
                    info->access = Private;
                else if (tok->str() == "protected:")
                    info->access = Protected;
                else if (tok->str() == "public:")
                    info->access = Public;
                else if (Token::Match(tok, "public|protected|private %var% :"))
                {
                    if (tok->str() == "private")
                        info->access = Private;
                    else if (tok->str() == "protected")
                        info->access = Protected;
                    else if (tok->str() == "public")
                        info->access = Public;

                    tok = tok->tokAt(2);
                }

                // function?
                else if (((Token::Match(tok, "%var% (") || Token::Match(tok, "operator %any% (")) && tok->previous()->str() != "::") &&
                         Token::Match(tok->str() == "operator" ? tok->tokAt(2)->link() : tok->next()->link(), ") const| ;|{|=|:"))
                {
                    Func function;

                    // save the access type
                    function.access = info->access;

                    // save the function name location
                    function.tokenDef = tok;

                    // operator function
                    if (function.tokenDef->str() ==  "operator")
                    {
                        function.isOperator = true;

                        // update the function name location
                        function.tokenDef = function.tokenDef->next();

                        // 'operator =' is special
                        if (function.tokenDef->str() == "=")
                            function.type = Func::OperatorEqual;
                    }

                    // class constructor/destructor
                    else if (function.tokenDef->str() == info->className)
                    {
                        if (function.tokenDef->previous()->str() == "~")
                            function.type = Func::Destructor;
                        else if ((Token::Match(function.tokenDef, "%var% ( const %var% & )") ||
                                  Token::Match(function.tokenDef, "%var% ( const %var% & %var% )")) &&
                                 function.tokenDef->strAt(3) == info->className)
                            function.type = Func::CopyConstructor;
                        else
                            function.type = Func::Constructor;

                        if (function.tokenDef->previous()->str() == "explicit")
                            function.isExplicit = true;
                    }

                    const Token *tok1 = tok;

                    // look for end of previous statement
                    while (tok1->previous() && !Token::Match(tok1->previous(), ";|}|{|public:|protected:|private:"))
                    {
                        // virtual function
                        if (tok1->previous()->str() == "virtual")
                        {
                            function.isVirtual = true;
                            break;
                        }

                        // static function
                        else if (tok1->previous()->str() == "static")
                        {
                            function.isStatic = true;
                            break;
                        }

                        // friend function
                        else if (tok1->previous()->str() == "friend")
                        {
                            function.isFriend = true;
                            break;
                        }

                        tok1 = tok1->previous();
                    }

                    // const function
                    if (function.tokenDef->next()->link()->next()->str() == "const")
                        function.isConst = true;

                    // pure virtual function
                    if (Token::Match(function.tokenDef->next()->link(), ") const| = 0 ;"))
                        function.isPure = true;

                    // count the number of constructors
                    if (function.type == Func::Constructor || function.type == Func::CopyConstructor)
                        info->numConstructors++;

                    // assume implementation is inline (definition and implementation same)
                    function.token = function.tokenDef;

                    // jump to end of args
                    const Token *next = function.tokenDef->next()->link();

                    // out of line function
                    if (Token::Match(next, ") const| ;") ||
                        Token::Match(next, ") const| = 0 ;"))
                    {
                        // find implementation using names on stack
                        SpaceInfo * nest = info;
                        unsigned int depth = 0;

                        std::string classPattern;
                        std::string classPath;
                        std::string searchPattern;
                        const Token *funcArgs = function.tokenDef->tokAt(2);

                        if (function.isOperator)
                            classPattern = "operator " + function.tokenDef->str() + " (";
                        else
                            classPattern = function.tokenDef->str() + " (";

                        while (!function.hasBody && nest)
                        {
                            classPath = nest->className + std::string(" :: ") + classPath;
                            searchPattern = classPath + classPattern;
                            depth++;
                            nest = nest->nest;

                            // start looking at end of class
                            SpaceInfo * top = info;
                            const Token *found = top->classEnd;
                            while ((found = Token::findmatch(found, searchPattern.c_str(), nest ? nest->classEnd : 0)) != NULL)
                            {
                                // skip other classes
                                if (found->previous()->str() == "::")
                                    break;

                                // goto function name
                                while (found->next()->str() != "(")
                                    found = found->next();

                                if (Token::Match(found->next()->link(), function.isConst ? ") const {" : ") {"))
                                {
                                    if (argsMatch(funcArgs, found->tokAt(2), classPath, depth))
                                    {
                                        function.token = found;
                                        function.hasBody = true;

                                        info->functionList.push_back(function);
                                        break;
                                    }

                                    // skip function body
                                    while (found->str() != "{")
                                        found = found->next();

                                    found = found->link();
                                }
                            }
                        }

                        if (!function.hasBody)
                            info->functionList.push_back(function);

                        tok = next->next();
                    }

                    // inline function
                    else
                    {
                        function.isInline = true;
                        function.hasBody = true;

                        info->functionList.push_back(function);

                        // skip over function body
                        tok = next->next();
                        while (tok->str() != "{")
                            tok = tok->next();
                        tok = tok->link();
                    }
                }

                // friend class declaration?
                else if (Token::Match(tok, "friend class| %any% ;"))
                {
                    FriendInfo friendInfo;

                    friendInfo.name = tok->strAt(1) == "class" ? tok->strAt(2) : tok->strAt(1);
                    /** @todo fill this in later after parsing is complete */
                    friendInfo.spaceInfo = 0;

                    info->friendList.push_back(friendInfo);
                }
            }
        }
    }

    std::multimap<std::string, SpaceInfo *>::iterator it;

    // finish filling in base class info
    for (it = spaceInfoMMap.begin(); it != spaceInfoMMap.end(); ++it)
    {
        info = it->second;

        // skip namespaces
        if (info->isNamespace)
            continue;

        for (unsigned int i = 0; i < info->derivedFrom.size(); ++i)
        {
            std::multimap<std::string, SpaceInfo *>::iterator it1;

            for (it1 = spaceInfoMMap.begin(); it1 != spaceInfoMMap.end(); ++it1)
            {
                SpaceInfo *spaceInfo = it1->second;

                /** @todo handle derived base classes and namespaces */
                if (!spaceInfo->isNamespace)
                {
                    if (spaceInfo->className == info->derivedFrom[i].name)
                    {
                        if (spaceInfo->nest == info->nest)
                        {
                            info->derivedFrom[i].spaceInfo = spaceInfo;
                            break;
                        }
                    }
                }
            }
        }
    }
}

CheckClass::~CheckClass()
{
    std::multimap<std::string, SpaceInfo *>::iterator it;

    for (it = spaceInfoMMap.begin(); it != spaceInfoMMap.end(); ++it)
        delete it->second;
}

//---------------------------------------------------------------------------

const Token *CheckClass::initBaseInfo(SpaceInfo *info, const Token *tok)
{
    // goto initial '{'
    const Token *tok2 = tok->tokAt(2);
    int level = 0;
    while (tok2 && tok2->str() != "{")
    {
        // skip unsupported templates
        if (tok2->str() == "<")
            level++;
        else if (tok2->str() == ">")
            level--;

        // check for base classes
        else if (level == 0 && Token::Match(tok2, ":|,"))
        {
            BaseInfo base;

            tok2 = tok2->next();

            if (tok2->str() == "public")
            {
                base.access = Public;
                tok2 = tok2->next();
            }
            else if (tok2->str() == "protected")
            {
                base.access = Protected;
                tok2 = tok2->next();
            }
            else if (tok2->str() == "private")
            {
                base.access = Private;
                tok2 = tok2->next();
            }
            else
            {
                if (tok->str() == "class")
                    base.access = Private;
                else if (tok->str() == "struct")
                    base.access = Public;
            }

            // handle derived base classes
            while (Token::Match(tok2, "%var% ::"))
            {
                base.name += tok2->str();
                base.name += " :: ";
                tok2 = tok2->tokAt(2);
            }

            base.name += tok2->str();

            base.spaceInfo = 0;

            // save pattern for base class name
            info->derivedFrom.push_back(base);
        }
        tok2 = tok2->next();
    }

    return tok2;
}

void CheckClass::SpaceInfo::getVarList()
{
    // Get variable list..
    const Token *tok1 = classDef;
    unsigned int indentlevel = 0;
    AccessControl varaccess = tok1->str() == "struct" ? Public : Private;
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
        bool b = false;
        if (tok->str() == "public:")
        {
            varaccess = Public;
            b = true;
        }
        else if (tok->str() == "protected:")
        {
            varaccess = Protected;
            b = true;
        }
        else if (tok->str() == "private:")
        {
            varaccess = Private;
            b = true;
        }
        else if (Token::Match(tok, "public|protected|private %var% :"))
        {
            if (tok->str() == "public")
                varaccess = Public;
            else if (tok->str() == "protected")
                varaccess = Protected;
            else if (tok->str() == "private")
                varaccess = Private;
            tok = tok->tokAt(2);
            b = true;
        }

        // Search for start of statement..
        if (! Token::Match(tok, "[;{}]") && ! b)
            continue;

        // This is the start of a statement
        const Token *next = tok->next();
        const Token *vartok = 0;

        // If next token contains a ":".. it is not part of a variable declaration
        if (next->str().find(":") != std::string::npos)
            continue;

        if (Token::Match(next, "public|protected|private %var% :"))
        {
            if (next->str() == "public")
                varaccess = Public;
            else if (next->str() == "protected")
                varaccess = Protected;
            else if (next->str() == "private")
                varaccess = Private;
            tok = tok->tokAt(2);
            continue;
        }

        // Is it a forward declaration?
        if (Token::Match(next, "class|struct|union %var% ;"))
        {
            tok = tok->tokAt(2);
            continue;
        }

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

        // It it a nested derived class or structure?
        if (Token::Match(next, "class|struct %type% :"))
        {
            next = next->tokAt(2);
            while (next->str() != "{")
                next = next->next();
            continue;
        }

        // Is it a variable declaration?
        bool isClass = false;
        if (Token::Match(next, "%type% %var% ;|:"))
        {
            if (!next->isStandardType())
                isClass = true;

            vartok = next->tokAt(1);
        }

        // Structure?
        else if (Token::Match(next, "struct|union %type% %var% ;"))
        {
            vartok = next->tokAt(2);
        }

        // Pointer?
        else if (Token::Match(next, "%type% * %var% ;"))
            vartok = next->tokAt(2);
        else if (Token::Match(next, "%type% %type% * %var% ;"))
            vartok = next->tokAt(3);
        else if (Token::Match(next, "%type% :: %type% * %var% ;"))
            vartok = next->tokAt(4);

        // Array?
        else if (Token::Match(next, "%type% %var% [") && next->next()->str() != "operator")
        {
            if (!next->isStandardType())
                isClass = true;

            vartok = next->tokAt(1);
        }

        // Pointer array?
        else if (Token::Match(next, "%type% * %var% ["))
            vartok = next->tokAt(2);
        else if (Token::Match(next, "%type% :: %type% * %var% ["))
            vartok = next->tokAt(4);

        // std::string..
        else if (Token::Match(next, "%type% :: %type% %var% ;"))
        {
            isClass = true;
            vartok = next->tokAt(3);
        }

        // Container..
        else if (Token::Match(next, "%type% :: %type% <") ||
                 Token::Match(next, "%type% <"))
        {
            isClass = true;
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
                vartok = next->tokAt(1);
            else if (next && Token::Match(next, "> * %var% ;"))
                vartok = next->tokAt(2);
        }

        // If the vartok was set in the if-blocks above, create a entry for this variable..
        if (vartok && vartok->str() != "operator")
        {
            if (vartok->varId() == 0)
            {
                check->reportError(vartok, Severity::error, "cppcheckError", "Internal error. CheckClass::SpaceInfo::getVarList found variable \'" + vartok->str() + "\' with varid 0.");
            }

            varlist.push_back(Var(vartok, false, varaccess, isMutable, isStatic, isClass));
        }
    }
}

//---------------------------------------------------------------------------

void CheckClass::SpaceInfo::initVar(const std::string &varname)
{
    std::list<Var>::iterator i;

    for (i = varlist.begin(); i != varlist.end(); ++i)
    {
        if (i->token->str() == varname)
        {
            i->init = true;
            return;
        }
    }
}

void CheckClass::SpaceInfo::markAllVar(bool value)
{
    std::list<Var>::iterator i;

    for (i = varlist.begin(); i != varlist.end(); ++i)
        i->init = value;
}

//---------------------------------------------------------------------------

void CheckClass::SpaceInfo::initializeVarList(const Func &func, std::list<std::string> &callstack)
{
    bool Assign = false;
    unsigned int indentlevel = 0;
    const Token *ftok = func.token;

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
                initVar(ftok->strAt(0));

                // assignment in the initializer..
                // : var(value = x)
                if (Token::Match(ftok->tokAt(2), "%var% ="))
                    initVar(ftok->strAt(2));
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
            initVar(ftok->strAt(1));
        }

        // Before a new statement there is "[{};)=]"
        if (! Token::Match(ftok, "[{};()=]"))
            continue;

        if (Token::simpleMatch(ftok, "( !"))
            ftok = ftok->next();

        // Using the operator= function to initialize all variables..
        if (Token::simpleMatch(ftok->next(), "* this = "))
        {
            markAllVar(true);
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
            markAllVar(true);
            return;
        }

        // Clearing array..
        else if (Token::Match(ftok, "memset ( %var% ,"))
        {
            initVar(ftok->strAt(2));
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
                    markAllVar(true);
                    return;
                }
            }

            // recursive call / calling overloaded function
            // assume that all variables are initialized
            if (std::find(callstack.begin(), callstack.end(), ftok->str()) != callstack.end())
            {
                markAllVar(true);
                return;
            }

            // check if member function
            std::list<Func>::const_iterator it;
            for (it = functionList.begin(); it != functionList.end(); ++it)
            {
                if (ftok->str() == it->tokenDef->str())
                    break;
            }

            // member function found
            if (it != functionList.end())
            {
                // member function has implementation
                if (it->hasBody)
                {
                    // initialize variable use list using member function
                    callstack.push_back(ftok->str());
                    initializeVarList(*it, callstack);
                    callstack.pop_back();
                }

                // there is a called member function, but it has no implementation, so we assume it initializes everything
                else
                {
                    markAllVar(true);
                }
            }

            // not member function
            else
            {
                // could be a base class virtual function, so we assume it initializes everything
                if (!derivedFrom.empty())
                    markAllVar(true);

                // has friends, so we assume it initializes everything
                if (!friendList.empty())
                    markAllVar(true);

                // the function is external and it's neither friend nor inherited virtual function.
                // assume all variables that are passed to it are initialized..
                else
                {
                    unsigned int indentlevel2 = 0;
                    for (const Token *tok = ftok->tokAt(2); tok; tok = tok->next())
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
                            initVar(tok->strAt(0));
                        }
                    }
                }
            }
        }

        // Assignment of member variable?
        else if (Token::Match(ftok, "%var% ="))
        {
            initVar(ftok->strAt(0));
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%var% [ %any% ] ="))
        {
            initVar(ftok->strAt(0));
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%var% [ %any% ] [ %any% ] ="))
        {
            initVar(ftok->strAt(0));
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "* %var% ="))
        {
            initVar(ftok->strAt(1));
        }

        // Assignment of struct member of member variable?
        else if (Token::Match(ftok, "%var% . %any% ="))
        {
            initVar(ftok->strAt(0));
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Token::Match(ftok, "%var% . clear|Clear ("))
        {
            initVar(ftok->strAt(0));
        }
    }
}

bool CheckClass::argsMatch(const Token *first, const Token *second, const std::string &path, unsigned int depth) const
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

        // function missing variable name
        else if (second->next()->str() == "," && first->next()->str() != ",")
            first = first->next();
        else if (second->next()->str() == ")" && first->next()->str() != ")")
            first = first->next();

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

        // variable with class path
        else if (depth && Token::Match(first->next(), "%var%"))
        {
            std::string param = path + first->next()->str();

            if (Token::Match(second->next(), param.c_str()))
            {
                second = second->tokAt(int(depth) * 2);
            }
            else if (depth > 1)
            {
                std::string    short_path = path;

                // remove last " :: "
                short_path.resize(short_path.size() - 4);

                // remove last name
                while (!short_path.empty() && short_path[short_path.size() - 1] != ' ')
                    short_path.resize(short_path.size() - 1);

                param = short_path + first->next()->str();

                if (Token::Match(second->next(), param.c_str()))
                {
                    second = second->tokAt((int(depth) - 1) * 2);
                }
            }
        }

        first = first->next();
        second = second->next();
    }

    return match;
}

//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    if (!_settings->_checkCodingStyle)
        return;

    createSymbolDatabase();

    std::multimap<std::string, SpaceInfo *>::iterator i;

    for (i = spaceInfoMMap.begin(); i != spaceInfoMMap.end(); ++i)
    {
        SpaceInfo *info = i->second;

        // don't check namespaces
        if (info->isNamespace)
            continue;

        // There are no constructors.
        if (info->numConstructors == 0)
        {
            // If there is a private variable, there should be a constructor..
            std::list<Var>::const_iterator var;
            for (var = info->varlist.begin(); var != info->varlist.end(); ++var)
            {
                if (var->access == Private && !var->isClass && !var->isStatic)
                {
                    noConstructorError(info->classDef, info->className, info->classDef->str() == "struct");
                    break;
                }
            }
        }

        std::list<Func>::const_iterator it;

        for (it = info->functionList.begin(); it != info->functionList.end(); ++it)
        {
            if (!it->hasBody || !(it->type == Func::Constructor || it->type == Func::CopyConstructor || it->type == Func::OperatorEqual))
                continue;

            // Mark all variables not used
            info->markAllVar(false);

            std::list<std::string> callstack;
            info->initializeVarList(*it, callstack);

            // Check if any variables are uninitialized
            std::list<Var>::const_iterator var;
            for (var = info->varlist.begin(); var != info->varlist.end(); ++var)
            {
                // skip classes for regular constructor
                if (var->isClass && it->type == Func::Constructor)
                    continue;

                if (var->init || var->isStatic)
                    continue;

                // It's non-static and it's not initialized => error
                if (it->type == Func::OperatorEqual)
                {
                    const Token *operStart = 0;
                    if (it->token->str() == "=")
                        operStart = it->token->tokAt(1);
                    else
                        operStart = it->token->tokAt(3);

                    bool classNameUsed = false;
                    for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next())
                    {
                        if (operTok->str() == info->className)
                        {
                            classNameUsed = true;
                            break;
                        }
                    }

                    if (classNameUsed)
                        operatorEqVarError(it->token, info->className, var->token->str());
                }
                else if (it->access != Private && !var->isStatic)
                    uninitVarError(it->token, info->className, var->token->str());
            }
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

    const char pattern_class[] = "class|struct %var% {";

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

            // or if the function address is used somewhere...
            // eg. sigc::mem_fun(this, &className::classFunction)
            const std::string _pattern2("& " + classname + " :: " + FuncList.front()->str());
            if (!Token::findmatch(_tokenizer->tokens(), _pattern.c_str()) &&
                !Token::findmatch(_tokenizer->tokens(), _pattern2.c_str()))
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

    createSymbolDatabase();

    std::multimap<std::string, SpaceInfo *>::const_iterator i;

    for (i = spaceInfoMMap.begin(); i != spaceInfoMMap.end(); ++i)
    {
        std::list<Func>::const_iterator it;

        for (it = i->second->functionList.begin(); it != i->second->functionList.end(); ++it)
        {
            if (it->type == Func::OperatorEqual && it->access != Private)
            {
                if (it->token->strAt(-2) == "void")
                    operatorEqReturnError(it->token->tokAt(-2));
            }
        }
    }
}

//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C&) { ... return *this; }"
// operator= should return a reference to *this
//---------------------------------------------------------------------------

void CheckClass::operatorEqRetRefThis()
{
    if (!_settings->_checkCodingStyle)
        return;

    createSymbolDatabase();

    std::multimap<std::string, SpaceInfo *>::const_iterator i;

    for (i = spaceInfoMMap.begin(); i != spaceInfoMMap.end(); ++i)
    {
        const SpaceInfo *info = i->second;
        std::list<Func>::const_iterator it;

        for (it = info->functionList.begin(); it != info->functionList.end(); ++it)
        {
            if (it->type == Func::OperatorEqual && it->hasBody)
            {
                // make sure return signature is correct
                if (Token::Match(it->tokenDef->tokAt(-4), ";|}|{|public:|protected:|private: %type% &") &&
                    it->tokenDef->strAt(-3) == info->className)
                {
                    // find the ')'
                    const Token *tok = it->token->next()->link();

                    bool foundReturn = false;
                    const Token *last = tok->next()->link();
                    for (tok = tok->tokAt(2); tok && tok != last; tok = tok->next())
                    {
                        // check for return of reference to this
                        if (tok->str() == "return")
                        {
                            foundReturn = true;
                            std::string cast("( " + info->className + " & )");
                            if (Token::Match(tok->next(), cast.c_str()))
                                tok = tok->tokAt(4);

                            if (!(Token::Match(tok->tokAt(1), "(| * this ;|=") ||
                                  Token::Match(tok->tokAt(1), "(| * this +=") ||
                                  Token::Match(tok->tokAt(1), "operator = (")))
                                operatorEqRetRefThisError(it->token);
                        }
                    }
                    if (!foundReturn)
                        operatorEqRetRefThisError(it->token);
                }
            }
        }
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

void CheckClass::operatorEqToSelf()
{
    if (!_settings->_checkCodingStyle)
        return;

    createSymbolDatabase();

    std::multimap<std::string, SpaceInfo *>::const_iterator i;

    for (i = spaceInfoMMap.begin(); i != spaceInfoMMap.end(); ++i)
    {
        const SpaceInfo *info = i->second;
        std::list<Func>::const_iterator it;

        // skip classes with multiple inheritance
        if (info->derivedFrom.size() > 1)
            continue;

        for (it = info->functionList.begin(); it != info->functionList.end(); ++it)
        {
            if (it->type == Func::OperatorEqual && it->hasBody)
            {
                // make sure return signature is correct
                if (Token::Match(it->tokenDef->tokAt(-4), ";|}|{|public:|protected:|private: %type% &") &&
                    it->tokenDef->strAt(-3) == info->className)
                {
                    // check for proper function parameter signature
                    if ((Token::Match(it->tokenDef->next(), "( const %var% & )") ||
                         Token::Match(it->tokenDef->next(), "( const %var% & %var% )")) &&
                        it->tokenDef->strAt(3) == info->className)
                    {
                        // find the parameter name
                        const Token *rhs = it->token;
                        while (rhs->str() != "&")
                            rhs = rhs->next();
                        rhs = rhs->next();

                        // find the ')'
                        const Token *tok = it->token->next()->link();
                        const Token *tok1 = tok;

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

    createSymbolDatabase();

    std::multimap<std::string, SpaceInfo *>::const_iterator i;

    for (i = spaceInfoMMap.begin(); i != spaceInfoMMap.end(); ++i)
    {
        const SpaceInfo *info = i->second;

        // Skip base classes and namespaces
        if (info->derivedFrom.empty())
            continue;

        // Find the destructor
        const Func *destructor = info->getDestructor();

        // Check for destructor with implementation
        if (!destructor || !destructor->hasBody)
            continue;

        // Empty destructor
        if (destructor->tokenDef->tokAt(3)->link() == destructor->tokenDef->tokAt(4))
            continue;

        const Token *derived = info->classDef;
        const Token *derivedClass = derived->tokAt(1);

        // Iterate through each base class...
        for (unsigned int j = 0; j < info->derivedFrom.size(); ++j)
        {
            // Check if base class is public and exists in database
            if (info->derivedFrom[j].access == Public && info->derivedFrom[j].spaceInfo)
            {
                const SpaceInfo *spaceInfo = info->derivedFrom[j].spaceInfo;

                // Name of base class..
                const std::string baseName = spaceInfo->className;

                // Find the destructor declaration for the base class.
                const Func *base_destructor = spaceInfo->getDestructor();
                const Token *base = 0;
                if (base_destructor)
                    base = base_destructor->token;

                // Check that there is a destructor..
                if (!base_destructor)
                {
                    if (spaceInfo->derivedFrom.empty())
                        virtualDestructorError(spaceInfo->classDef, baseName, derivedClass->str());
                }
                else if (!base_destructor->isVirtual)
                {
                    // TODO: This is just a temporary fix, better solution is needed.
                    // Skip situations where base class has base classes of its own, because
                    // some of the base classes might have virtual destructor.
                    // Proper solution is to check all of the base classes. If base class is not
                    // found or if one of the base classes has virtual destructor, error should not
                    // be printed. See TODO test case "virtualDestructorInherited"
                    if (spaceInfo->derivedFrom.empty())
                    {
                        // Make sure that the destructor is public (protected or private
                        // would not compile if inheritance is used in a way that would
                        // cause the bug we are trying to find here.)
                        if (base_destructor->access == Public)
                            virtualDestructorError(base, baseName, derivedClass->str());
                    }
                }
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

// check if this function is defined virtual in the base classes
bool CheckClass::isVirtual(const SpaceInfo *info, const Token *functionToken) const
{
    // check each base class
    for (unsigned int i = 0; i < info->derivedFrom.size(); ++i)
    {
        // check if base class exists in database
        if (info->derivedFrom[i].spaceInfo)
        {
            const SpaceInfo *spaceInfo = info->derivedFrom[i].spaceInfo;

            std::list<Func>::const_iterator    it;

            // check if function defined in base class
            for (it = spaceInfo->functionList.begin(); it != spaceInfo->functionList.end(); ++it)
            {
                if (it->isVirtual)
                {
                    const Token *tok = it->tokenDef;

                    if (tok->str() == functionToken->str())
                    {
                        const Token *temp1 = tok->previous();
                        const Token *temp2 = functionToken->previous();
                        bool returnMatch = true;

                        // check for matching return parameters
                        while (temp1->str() != "virtual")
                        {
                            if (temp1->str() != temp2->str())
                            {
                                returnMatch = false;
                                break;
                            }

                            temp1 = temp1->previous();
                            temp2 = temp2->previous();
                        }

                        // check for matching function parameters
                        if (returnMatch && argsMatch(tok->tokAt(2), functionToken->tokAt(2), std::string(""), 0))
                        {
                            return true;
                        }
                    }
                }
            }

            if (!spaceInfo->derivedFrom.empty())
            {
                if (isVirtual(spaceInfo, functionToken))
                    return true;
            }
        }
        else
        {
            // unable to find base class so assume it has a virtual function
            return true;
        }
    }

    return false;
}

// Can a function be const?
void CheckClass::checkConst()
{
    if (!_settings->_checkCodingStyle || _settings->ifcfg)
        return;

    createSymbolDatabase();

    std::multimap<std::string, SpaceInfo *>::iterator it;

    for (it = spaceInfoMMap.begin(); it != spaceInfoMMap.end(); ++it)
    {
        SpaceInfo *info = it->second;

        std::list<Func>::iterator    it1;

        for (it1 = info->functionList.begin(); it1 != info->functionList.end(); ++it1)
        {
            const Func & func = *it1;

            // does the function have a body?
            if (func.type == Func::Function && func.hasBody && !func.isFriend && !func.isStatic && !func.isConst && !func.isVirtual)
            {
                // get function name
                const std::string functionName((func.tokenDef->isName() ? "" : "operator") + func.tokenDef->str());

                // get last token of return type
                const Token *previous = func.tokenDef->isName() ? func.token->previous() : func.token->tokAt(-2);
                while (previous->str() == "::")
                    previous = previous->tokAt(-2);

                // does the function return a pointer or reference?
                if (Token::Match(previous, "*|&"))
                {
                    const Token *temp = func.token->previous();

                    while (!Token::Match(temp->previous(), ";|}|{|public:|protected:|private:"))
                        temp = temp->previous();

                    if (temp->str() != "const")
                        continue;
                }
                else if (Token::Match(previous->previous(), "*|& >"))
                {
                    const Token *temp = func.token->previous();

                    while (!Token::Match(temp->previous(), ";|}|{|public:|protected:|private:"))
                    {
                        temp = temp->previous();
                        if (temp->str() == "const")
                            break;
                    }

                    if (temp->str() != "const")
                        continue;
                }
                else
                {
                    // don't warn for unknown types..
                    // LPVOID, HDC, etc
                    if (previous->isName())
                    {
                        bool allupper = true;
                        const std::string s(previous->str());
                        for (std::string::size_type pos = 0; pos < s.size(); ++pos)
                        {
                            const char ch = s[pos];
                            if (!(ch == '_' || (ch >= 'A' && ch <= 'Z')))
                            {
                                allupper = false;
                                break;
                            }
                        }

                        if (allupper)
                            continue;
                    }
                }

                const Token *paramEnd = func.token->next()->link();

                // check if base class function is virtual
                if (!info->derivedFrom.empty())
                {
                    if (isVirtual(info, func.tokenDef))
                        continue;
                }

                // if nothing non-const was found. write error..
                if (checkConstFunc(info, paramEnd))
                {
                    std::string classname = info->className;
                    SpaceInfo *nest = info->nest;
                    while (nest)
                    {
                        classname = std::string(nest->className + "::" + classname);
                        nest = nest->nest;
                    }

                    if (func.isInline)
                        checkConstError(func.token, classname, functionName);
                    else // not inline
                        checkConstError2(func.token, func.tokenDef, classname, functionName);
                }
            }
        }
    }
}

bool CheckClass::isMemberVar(const SpaceInfo *info, const Token *tok)
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
    if (tok->str() == info->className && tok->next()->str() == "::")
        tok = tok->tokAt(2);

    std::list<Var>::const_iterator var;
    for (var = info->varlist.begin(); var != info->varlist.end(); ++var)
    {
        if (var->token->str() == tok->str())
        {
            return !var->isMutable;
        }
    }

    // not found in this class
    if (!info->derivedFrom.empty())
    {
        // check each base class
        for (unsigned int i = 0; i < info->derivedFrom.size(); ++i)
        {
            // find the base class
            const SpaceInfo * spaceInfo = info->derivedFrom[i].spaceInfo;

            // find the function in the base class
            if (spaceInfo)
            {
                if (isMemberVar(spaceInfo, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::checkConstFunc(const SpaceInfo *info, const Token *tok)
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
            if (tok1->previous()->varId() == 0 && !info->derivedFrom.empty())
            {
                isconst = false;
                break;
            }
            else if (isMemberVar(info, tok1->previous()))
            {
                isconst = false;
                break;
            }
            else if (tok1->previous()->str() == "]")
            {
                // TODO: I assume that the assigned variable is a member variable
                //       don't assume it
                isconst = false;
                break;
            }
            else if (tok1->next()->str() == "this")
            {
                isconst = false;
                break;
            }
        }

        // streaming: <<
        else if (tok1->str() == "<<" && isMemberVar(info, tok1->previous()))
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
        else if ((Token::Match(tok1, "%var% (") && !Token::Match(tok1, "return|c_str|if")) ||
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
