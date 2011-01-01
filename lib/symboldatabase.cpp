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
#include "symboldatabase.h"

#include "tokenize.h"
#include "token.h"
#include "settings.h"
#include "errorlogger.h"
#include "check.h"

#include <locale>

#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>


//---------------------------------------------------------------------------

SymbolDatabase::SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : _tokenizer(tokenizer), _settings(settings), _errorLogger(errorLogger)
{
    // fill the classAndStructTypes set..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "class|struct %var% [:{;]"))
            classAndStructTypes.insert(tok->next()->str());
    }

    // find all namespaces (class,struct and namespace)
    SpaceInfo *info = new SpaceInfo(this, NULL, NULL);
    spaceInfoList.push_back(info);
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Locate next class
        if (Token::Match(tok, "class|struct|namespace %var% [{:]"))
        {
            SpaceInfo *new_info = new SpaceInfo(this, tok, info);
            const Token *tok2 = tok->tokAt(2);

            // only create base list for classes and structures
            if (new_info->isClassOrStruct())
            {
                // goto initial '{'
                tok2 = initBaseInfo(new_info, tok);
            }

            new_info->classStart = tok2;
            new_info->classEnd = tok2->link();

            // make sure we have valid code
            if (!new_info->classEnd)
            {
                delete new_info;
                break;
            }

            info = new_info;

            // add namespace
            spaceInfoList.push_back(info);

            tok = tok2;
        }

        else
        {
            // check for end of space
            if (tok == info->classEnd)
            {
                info = info->nestedIn;
                continue;
            }

            // check if in class or structure
            else if (info->type == SpaceInfo::Class || info->type == SpaceInfo::Struct)
            {
                const Token *funcStart = 0;
                const Token *argStart = 0;

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

                // class function?
                else if (tok->previous()->str() != "::" && isFunction(tok, &funcStart, &argStart))
                {
                    Func function;

                    // save the function definition argument start '('
                    function.argDef = argStart;

                    // save the access type
                    function.access = info->access;

                    // save the function name location
                    function.tokenDef = funcStart;

                    // operator function
                    if (function.tokenDef->previous()->str() ==  "operator")
                    {
                        function.isOperator = true;

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

                    // function returning function pointer
                    else if (tok->str() == "(")
                    {
                        function.retFuncPtr = true;
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

                    const Token *end;

                    if (!function.retFuncPtr)
                        end = function.argDef->link();
                    else
                        end = tok->link()->next()->link();

                    // const function
                    if (end->next()->str() == "const")
                        function.isConst = true;

                    // pure virtual function
                    if (Token::Match(end, ") const| = 0 ;"))
                        function.isPure = true;

                    // count the number of constructors
                    if (function.type == Func::Constructor || function.type == Func::CopyConstructor)
                        info->numConstructors++;

                    // assume implementation is inline (definition and implementation same)
                    function.token = function.tokenDef;

                    // out of line function
                    if (Token::Match(end, ") const| ;") ||
                        Token::Match(end, ") const| = 0 ;"))
                    {
                        // find the function implementation later
                        tok = end->next();

                        info->functionList.push_back(function);
                    }

                    // inline function
                    else
                    {
                        function.isInline = true;
                        function.hasBody = true;
                        function.arg = function.argDef;

                        info->functionList.push_back(function);

                        const Token *tok2 = funcStart;
                        SpaceInfo *functionOf = info;

                        addNewFunction(&info, &tok2);
                        if (info)
                            info->functionOf = functionOf;

                        tok = tok2;
                    }
                }

                // nested class function?
                else if (tok->previous()->str() == "::" && isFunction(tok, &funcStart, &argStart))
                    addFunction(&info, &tok, argStart);

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
            else if (info->type == SpaceInfo::Namespace || info->type == SpaceInfo::Global)
            {
                const Token *funcStart = 0;
                const Token *argStart = 0;

                // function?
                if (isFunction(tok, &funcStart, &argStart))
                {
                    // has body?
                    if (Token::Match(argStart->link(), ") const| {|:"))
                    {
                        // class function
                        if (tok->previous() && tok->previous()->str() == "::")
                            addFunction(&info, &tok, argStart);

                        // class destructor
                        else if (tok->previous() && tok->previous()->str() == "~" &&
                                 tok->previous()->previous() && tok->previous()->previous()->str() == "::")
                            addFunction(&info, &tok, argStart);

                        // regular function
                        else
                        {
                            Func function;

                            // save the function definition argument start '('
                            function.argDef = argStart;

                            // save the access type
                            function.access = Public;

                            // save the function name location
                            function.tokenDef = funcStart;
                            function.token = funcStart;

                            function.isInline = false;
                            function.hasBody = true;
                            function.arg = function.argDef;
                            function.type = Func::Function;

                            SpaceInfo *old_info = info;

                            addNewFunction(&info, &tok);

                            if (info)
                                old_info->functionList.push_back(function);

                            // syntax error
                            else
                            {
                                info = old_info;
                                break;
                            }
                        }
                    }

                    // function returning function pointer with body
                    else if (Token::simpleMatch(argStart->link(), ") ) (") &&
                             Token::Match(argStart->link()->tokAt(2)->link(), ") const| {"))
                    {
                        const Token *tok1 = funcStart;
                        SpaceInfo *old_info = info;

                        // class function
                        if (tok1->previous()->str() == "::")
                            addFunction(&info, &tok1, argStart);

                        // regular function
                        else
                            addNewFunction(&info, &tok1);

                        // syntax error?
                        if (!info)
                            info = old_info;

                        tok = tok1;
                    }
                }
            }
        }
    }

    std::list<SpaceInfo *>::iterator it;

    // fill in base class info
    for (it = spaceInfoList.begin(); it != spaceInfoList.end(); ++it)
    {
        info = *it;

        // skip namespaces and functions
        if (!info->isClassOrStruct())
            continue;

        // finish filling in base class info
        for (unsigned int i = 0; i < info->derivedFrom.size(); ++i)
        {
            std::list<SpaceInfo *>::iterator it1;

            for (it1 = spaceInfoList.begin(); it1 != spaceInfoList.end(); ++it1)
            {
                SpaceInfo *spaceInfo = *it1;

                /** @todo handle derived base classes and namespaces */
                if (spaceInfo->type == SpaceInfo::Class || spaceInfo->type == SpaceInfo::Struct)
                {
                    // do class names match?
                    if (spaceInfo->className == info->derivedFrom[i].name)
                    {
                        // are they in the same namespace or different namespaces with same name?
                        if ((spaceInfo->nestedIn == info->nestedIn) ||
                            ((spaceInfo->nestedIn && spaceInfo->nestedIn->type == SpaceInfo::Namespace) &&
                             (info->nestedIn && info->nestedIn->type == SpaceInfo::Namespace) &&
                             (spaceInfo->nestedIn->className == info->nestedIn->className)))
                        {
                            info->derivedFrom[i].spaceInfo = spaceInfo;
                            break;
                        }
                    }
                }
            }
        }
    }

    // fill in variable info
    for (it = spaceInfoList.begin(); it != spaceInfoList.end(); ++it)
    {
        info = *it;

        // skip functions
        if (info->type != SpaceInfo::Function)
        {
            // find variables
            info->getVarList();
        }
    }

    // determine if user defined type needs initialization
    unsigned int unknowns = 0; // stop checking when there are no unknowns
    unsigned int retry = 0;    // bail if we don't resolve all the variable types for some reason

    do
    {
        unknowns = 0;

        for (it = spaceInfoList.begin(); it != spaceInfoList.end(); ++it)
        {
            info = *it;

            if (info->isClassOrStruct() && info->needInitialization == SpaceInfo::Unknown)
            {
                // check for default constructor
                bool hasDefaultConstructor = false;

                std::list<SymbolDatabase::Func>::const_iterator func;

                for (func = info->functionList.begin(); func != info->functionList.end(); ++func)
                {
                    if (func->type == SymbolDatabase::Func::Constructor)
                    {
                        // check for no arguments: func ( )
                        if (func->argDef->next() == func->argDef->link())
                        {
                            hasDefaultConstructor = true;
                            break;
                        }

                        /** check for arguments with default values */
                        else if (func->argCount() == func->initializedArgCount())
                        {
                            hasDefaultConstructor = true;
                            break;
                        }
                    }
                }

                // User defined types with user defined default constructor doesn't need initialization.
                // We assume the default constructor initializes everything.
                // Another check will figure out if the constructor actually initializes everything.
                if (hasDefaultConstructor)
                    info->needInitialization = SpaceInfo::False;

                // check each member variable to see if it needs initialization
                else
                {
                    bool needInitialization = false;
                    bool unknown = false;

                    std::list<Var>::const_iterator var;
                    for (var = info->varlist.begin(); var != info->varlist.end(); ++var)
                    {
                        if (var->isClass)
                        {
                            if (var->type)
                            {
                                // does this type need initialization?
                                if (var->type->needInitialization == SpaceInfo::True)
                                    needInitialization = true;
                                else if (var->type->needInitialization == SpaceInfo::Unknown)
                                    unknown = true;
                            }
                        }
                        else
                            needInitialization = true;
                    }

                    if (!unknown)
                    {
                        if (needInitialization)
                            info->needInitialization = SpaceInfo::True;
                        else
                            info->needInitialization = SpaceInfo::False;
                    }

                    if (info->needInitialization == SpaceInfo::Unknown)
                        unknowns++;
                }
            }
        }

        retry++;
    }
    while (unknowns && retry < 100);

    // this shouldn't happen so output a debug warning
    if (retry == 100 && _settings->debugwarnings)
    {
        for (it = spaceInfoList.begin(); it != spaceInfoList.end(); ++it)
        {
            info = *it;

            if (info->isClassOrStruct() && info->needInitialization == SpaceInfo::Unknown)
            {
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.line = info->classDef->linenr();
                loc.setfile(_tokenizer->file(info->classDef));
                locationList.push_back(loc);

                const ErrorLogger::ErrorMessage errmsg(locationList,
                                                       Severity::debug,
                                                       "SymbolDatabase::SymbolDatabase couldn't resolve all user defined types.",
                                                       "debug");
                if (_errorLogger)
                    _errorLogger->reportErr(errmsg);
                else
                    Check::reportError(errmsg);
            }
        }
    }
}

SymbolDatabase::~SymbolDatabase()
{
    std::list<SpaceInfo *>::iterator it;

    for (it = spaceInfoList.begin(); it != spaceInfoList.end(); ++it)
        delete *it;
}

bool SymbolDatabase::isFunction(const Token *tok, const Token **funcStart, const Token **argStart) const
{
    // function returning function pointer? '... ( ... %var% ( ... ))( ... ) {'
    if (tok->str() == "(" &&
        tok->link()->previous()->str() == ")" &&
        tok->link()->next() &&
        tok->link()->next()->str() == "(" &&
        tok->link()->next()->link()->next() &&
        Token::Match(tok->link()->next()->link()->next(), "{|;|const|="))
    {
        *funcStart = tok->link()->previous()->link()->previous();
        *argStart = tok->link()->previous()->link();
        return true;
    }

    // regular function?
    else if (Token::Match(tok, "%var% (") && Token::Match(tok->next()->link(), ") const| ;|{|=|:"))
    {
        *funcStart = tok;
        *argStart = tok->next();
        return true;
    }

    // simple operator?
    else if (Token::Match(tok, "operator %any% (") && Token::Match(tok->tokAt(2)->link(), ") const| ;|{|="))
    {
        *funcStart = tok->next();
        *argStart = tok->tokAt(2);
        return true;
    }

    // operator[] or operator()?
    else if (Token::Match(tok, "operator %any% %any% (") && Token::Match(tok->tokAt(3)->link(), ") const| ;|{|="))
    {
        *funcStart = tok->next();
        *argStart = tok->tokAt(3);
        return true;
    }

    // operator new/delete []?
    else if (Token::Match(tok, "operator %any% %any% %any% (") && Token::Match(tok->tokAt(4)->link(), ") const| ;|{|="))
    {
        *funcStart = tok->next();
        *argStart = tok->tokAt(4);
        return true;
    }

    // complex user defined operator?
    else if (Token::Match(tok, "operator %any% %any% %any% %any% (") && Token::Match(tok->tokAt(5)->link(), ") const| ;|{|="))
    {
        *funcStart = tok->next();
        *argStart = tok->tokAt(5);
        return true;
    }

    return false;
}

bool SymbolDatabase::argsMatch(const SpaceInfo *info, const Token *first, const Token *second, const std::string &path, unsigned int depth) const
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
            first = first->tokAt(2);

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

        // nested class variable
        else if (depth == 0 && Token::Match(first->next(), "%var%") &&
                 second->next()->str() == info->className && second->strAt(2) == "::" &&
                 first->next()->str() == second->strAt(3))
        {
            second = second->tokAt(2);
        }

        first = first->next();
        second = second->next();
    }

    return match;
}

void SymbolDatabase::addFunction(SpaceInfo **info, const Token **tok, const Token *argStart)
{
    int count = 0;
    bool added = false;
    std::string path;
    unsigned int path_length = 0;
    const Token *tok1;

    // skip class/struct name
    if ((*tok)->previous()->str() == "~")
        tok1 = (*tok)->tokAt(-3);
    else
        tok1 = (*tok)->tokAt(-2);

    // back up to head of path
    while (tok1 && tok1->previous() && tok1->previous()->str() == "::")
    {
        path = tok1->str() + " :: " + path;
        tok1 = tok1->tokAt(-2);
        count++;
        path_length++;
    }

    if (count)
    {
        path = tok1->str() + " :: " + path;
        path_length++;
    }

    std::list<SpaceInfo *>::iterator it1;

    // search for match
    for (it1 = spaceInfoList.begin(); it1 != spaceInfoList.end(); ++it1)
    {
        SpaceInfo *info1 = *it1;

        bool match = false;
        if (info1->className == tok1->str() && (info1->type != SpaceInfo::Function))
        {
            // do the spaces match (same space) or do their names match (multiple namespaces)
            if ((*info == info1->nestedIn) || (*info && info1 &&
                                               (*info)->className == info1->nestedIn->className &&
                                               !(*info)->className.empty() &&
                                               (*info)->type == info1->nestedIn->type))
            {
                SpaceInfo *info2 = info1;

                while (info2 && count > 0)
                {
                    count--;
                    tok1 = tok1->tokAt(2);
                    info2 = info2->findInNestedList(tok1->str());
                }

                if (count == 0 && info2)
                {
                    match = true;
                    info1 = info2;
                }
            }
        }

        if (match)
        {
            std::list<Func>::iterator func;

            for (func = info1->functionList.begin(); func != info1->functionList.end(); ++func)
            {
                if (!func->hasBody)
                {
                    if (func->isOperator &&
                        (*tok)->str() == "operator" &&
                        func->tokenDef->str() == (*tok)->strAt(1))
                    {
                        if (argsMatch(info1, func->tokenDef->tokAt(2), (*tok)->tokAt(3), path, path_length))
                        {
                            func->hasBody = true;
                            func->token = (*tok)->next();
                            func->arg = argStart;
                        }
                    }
                    else if (func->type == SymbolDatabase::Func::Destructor &&
                             (*tok)->previous()->str() == "~" &&
                             func->tokenDef->str() == (*tok)->str())
                    {
                        if (argsMatch(info1, func->tokenDef->next(), (*tok)->next(), path, path_length))
                        {
                            func->hasBody = true;
                            func->token = *tok;
                            func->arg = argStart;
                        }
                    }
                    else if (func->tokenDef->str() == (*tok)->str() && (*tok)->previous()->str() != "~")
                    {
                        if (argsMatch(info1, func->tokenDef->next(), (*tok)->next(), path, path_length))
                        {
                            // normal function?
                            if (!func->retFuncPtr && (*tok)->next()->link())
                            {
                                if ((func->isConst && (*tok)->next()->link()->next()->str() == "const") ||
                                    (!func->isConst && (*tok)->next()->link()->next()->str() != "const"))
                                {
                                    func->hasBody = true;
                                    func->token = *tok;
                                    func->arg = argStart;
                                }
                            }

                            // function returning function pointer?
                            else if (func->retFuncPtr)
                            {
                                // todo check for const
                                func->hasBody = true;
                                func->token = *tok;
                                func->arg = argStart;
                            }
                        }
                    }

                    if (func->hasBody)
                    {
                        addNewFunction(info, tok);
                        if (info)
                        {
                            (*info)->functionOf = info1;
                            added = true;
                        }
                        break;
                    }
                }
            }
        }
    }

    // check for class function for unknown class
    if (!added)
        addNewFunction(info, tok);
}

void SymbolDatabase::addNewFunction(SymbolDatabase::SpaceInfo **info, const Token **tok)
{
    const Token *tok1 = *tok;
    SpaceInfo *new_info = new SpaceInfo(this, tok1, *info);

    // skip to start of function
    while (tok1 && tok1->str() != "{")
        tok1 = tok1->next();

    if (tok1)
    {
        new_info->classStart = tok1;
        new_info->classEnd = tok1->link();

        // syntax error?
        if (!new_info->classEnd)
        {
            delete new_info;
            while (tok1->next())
                tok1 = tok1->next();
            *info = NULL;
            *tok = tok1;
            return;
        }

        *info = new_info;

        // add space
        spaceInfoList.push_back(new_info);

        *tok = tok1;
    }
}

const Token *SymbolDatabase::initBaseInfo(SpaceInfo *info, const Token *tok)
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

            // don't add unhandled templates
            if (tok2->next()->str() == "<")
            {
                int level1 = 1;
                while (tok2->next())
                {
                    if (tok2->next()->str() == ">")
                    {
                        level1--;
                        if (level == 0)
                            break;
                    }
                    else if (tok2->next()->str() == "<")
                        level1++;

                    tok2 = tok2->next();
                }
            }

            // save pattern for base class name
            else
            {
                info->derivedFrom.push_back(base);
            }
        }
        tok2 = tok2->next();
    }

    return tok2;
}

//---------------------------------------------------------------------------

unsigned int SymbolDatabase::Func::argCount() const
{
    unsigned int count = 0;

    if (argDef->link() != argDef->next())
    {
        count++;

        for (const Token *tok = argDef->next(); tok && tok->next() && tok->next() != argDef->link(); tok = tok->next())
        {
            if (tok->str() == ",")
                count++;
        }
    }

    return count;
}

unsigned int SymbolDatabase::Func::initializedArgCount() const
{
    unsigned int count = 0;

    if (argDef->link() != argDef->next())
    {
        for (const Token *tok = argDef->next(); tok && tok->next() && tok->next() != argDef->link(); tok = tok->next())
        {
            if (tok->str() == "=")
                count++;
        }
    }

    return count;
}

//---------------------------------------------------------------------------

SymbolDatabase::SpaceInfo::SpaceInfo(SymbolDatabase *check_, const Token *classDef_, SymbolDatabase::SpaceInfo *nestedIn_) :
    check(check_),
    classDef(classDef_),
    classStart(NULL),
    classEnd(NULL),
    nestedIn(nestedIn_),
    numConstructors(0),
    needInitialization(SpaceInfo::Unknown),
    functionOf(NULL)
{
    if (!classDef)
    {
        type = SpaceInfo::Global;
        access = Public;
    }
    else if (classDef->str() == "class")
    {
        type = SpaceInfo::Class;
        className = classDef->next()->str();
        access = Private;
    }
    else if (classDef->str() == "struct")
    {
        type = SpaceInfo::Struct;
        className = classDef->next()->str();
        access = Public;
    }
    else if (classDef->str() == "union")
    {
        type = SpaceInfo::Union;
        className = classDef->next()->str();
        access = Public;
    }
    else if (classDef->str() == "namespace")
    {
        type = SpaceInfo::Namespace;
        className = classDef->next()->str();
        access = Public;
    }
    else
    {
        type = SpaceInfo::Function;
        className = classDef->str();
        access = Public;
    }

    if (nestedIn)
        nestedIn->nestedList.push_back(this);
}

bool
SymbolDatabase::SpaceInfo::hasDefaultConstructor() const
{
    if (numConstructors)
    {
        std::list<Func>::const_iterator func;

        for (func = functionList.begin(); func != functionList.end(); ++func)
        {
            if (func->type == Func::Constructor &&
                func->argDef->link() == func->argDef->next())
                return true;
        }
    }
    return false;
}

// Get variable list..
void SymbolDatabase::SpaceInfo::getVarList()
{
    AccessControl varaccess = type == Class ? Private : Public;
    const Token *start;

    if (classStart)
        start = classStart->next();
    else
        start = check->_tokenizer->tokens();

    for (const Token *tok = start; tok; tok = tok->next())
    {
        // end of space?
        if (tok->str() == "}")
            break;

        // syntax error?
        else if (tok->next() == NULL)
            break;

        // Is it a function?
        else if (tok->str() == "{")
        {
            tok = tok->link();
            // syntax error?
            if (!tok)
                return;
            continue;
        }

        // Is it a nested class or structure?
        else if (Token::Match(tok, "class|struct|union|namespace %type% :|{"))
        {
            tok = tok->tokAt(2);
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (tok)
            {
                // skip implementation
                tok = tok->link();
                continue;
            }
            else
                break;
        }

        // Borland C++: Skip all variables in the __published section.
        // These are automatically initialized.
        else if (tok->str() == "__published:")
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
        else if (tok->str() == "public:")
        {
            varaccess = Public;
            continue;
        }
        else if (tok->str() == "protected:")
        {
            varaccess = Protected;
            continue;
        }
        else if (tok->str() == "private:")
        {
            varaccess = Private;
            continue;
        }

        // Is it a forward declaration?
        else if (Token::Match(tok, "class|struct|union %var% ;"))
        {
            tok = tok->tokAt(2);
            continue;
        }

        // Borland C++: Ignore properties..
        else if (tok->str() == "__property")
            continue;

        // Search for start of statement..
        else if (!tok->previous() || !Token::Match(tok->previous(), ";|{|}|public:|protected:|private:"))
            continue;
        else if (Token::Match(tok, ";|{|}"))
            continue;

        // This is the start of a statement
        const Token *vartok = NULL;
        const Token *typetok = NULL;

        // Is it const..?
        bool isConst = false;
        if (tok->str() == "const")
        {
            tok = tok->next();
            isConst = true;
        }

        // Is it a static variable?
        const bool isStatic(Token::simpleMatch(tok, "static"));
        if (isStatic)
        {
            tok = tok->next();
        }

        // Is it a mutable variable?
        const bool isMutable(Token::simpleMatch(tok, "mutable"));
        if (isMutable)
        {
            tok = tok->next();
        }

        // Is it const..?
        if (tok->str() == "const")
        {
            tok = tok->next();
            isConst = true;
        }

        bool isClass = false;

        if (Token::Match(tok, "struct|union"))
        {
            tok = tok->next();
        }

        if (isVariableDeclaration(tok, vartok, typetok))
        {
            isClass = (!typetok->isStandardType() && vartok->previous()->str() != "*");
            tok = vartok->next();
        }

        // If the vartok was set in the if-blocks above, create a entry for this variable..
        if (vartok && vartok->str() != "operator")
        {
            if (vartok->varId() == 0 && check->_settings->debugwarnings)
            {
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.line = vartok->linenr();
                loc.setfile(check->_tokenizer->file(vartok));
                locationList.push_back(loc);

                const ErrorLogger::ErrorMessage errmsg(locationList,
                                                       Severity::debug,
                                                       "SymbolDatabase::SpaceInfo::getVarList found variable \'" + vartok->str() + "\' with varid 0.",
                                                       "debug");
                if (check->_errorLogger)
                    check->_errorLogger->reportErr(errmsg);
                else
                    Check::reportError(errmsg);
            }

            const SpaceInfo *spaceInfo = NULL;

            if (typetok)
                spaceInfo = check->findVarType(this, typetok);

            addVar(vartok, varaccess, isMutable, isStatic, isConst, isClass, spaceInfo);
        }
    }
}

const Token* skipScopeIdentifiers(const Token* tok)
{
    const Token* ret = tok;

    if (Token::simpleMatch(ret, "::"))
    {
        ret = ret->next();
    }
    while (Token::Match(ret, "%type% :: "))
    {
        ret = ret->tokAt(2);
    }

    return ret;
}

const Token* skipPointers(const Token* tok)
{
    const Token* ret = tok;

    while (Token::simpleMatch(ret, "*"))
    {
        ret = ret->next();
    }

    return ret;
}

bool SymbolDatabase::SpaceInfo::isVariableDeclaration(const Token* tok, const Token*& vartok, const Token*& typetok) const
{
    const Token* localTypeTok = skipScopeIdentifiers(tok);

    if (Token::Match(localTypeTok, "%type% < "))
    {
        const Token* closetok = NULL;
        bool found = findClosingBracket(localTypeTok->next(), closetok);
        if (found)
        {
            if (Token::Match(closetok, "> %var% ;"))
            {
                vartok = closetok->next();
                typetok = localTypeTok;
            }
            else if (Token::Match(closetok, "> * %var% ;"))
            {
                vartok = closetok->tokAt(2);
                typetok = localTypeTok;
            }
            else if (Token::Match(closetok, "> :: %type% %var% ;"))
            {
                vartok = closetok->tokAt(3);
                typetok = closetok->tokAt(2);
            }
        }
    }
    else if (Token::Match(localTypeTok, "%type%"))
    {
        const Token* localVarTok = skipPointers(localTypeTok->next());

        if (isSimpleVariable(localVarTok) || isArrayVariable(localVarTok))
        {
            vartok = localVarTok;
            typetok = localTypeTok;
        }
    }

    return NULL != vartok;
}

bool SymbolDatabase::SpaceInfo::isSimpleVariable(const Token* tok) const
{
    return Token::Match(tok, "%var% ;");
}

bool SymbolDatabase::SpaceInfo::isArrayVariable(const Token* tok) const
{
    return Token::Match(tok, "%var% [") && tok->next()->str() != "operator";
}

bool SymbolDatabase::SpaceInfo::findClosingBracket(const Token* tok, const Token*& close) const
{
    bool found = false;
    if (NULL != tok && tok->str() == "<")
    {
        unsigned int depth = 0;
        for (close = tok; (close != NULL) && (close->str() != ";"); close = close->next())
        {
            if (close->str() == "<")
            {
                ++depth;
            }
            else if (close->str() == ">")
            {
                if (--depth == 0)
                {
                    found = true;
                    break;
                }
            }
        }
    }

    return found;
}


//---------------------------------------------------------------------------

const SymbolDatabase::SpaceInfo *SymbolDatabase::findVarType(const SpaceInfo *start, const Token *type) const
{
    std::list<SpaceInfo *>::const_iterator it;

    for (it = spaceInfoList.begin(); it != spaceInfoList.end(); ++it)
    {
        const SpaceInfo *info = *it;

        // skip namespaces and functions
        if (info->type == SpaceInfo::Namespace || info->type == SpaceInfo::Function || info->type == SpaceInfo::Global)
            continue;

        // do the names match?
        if (info->className == type->str())
        {
            // check if type does not have a namespace
            if (type->previous()->str() != "::")
            {
                const SpaceInfo *parent = start;

                // check if in same namespace
                while (parent && parent != info->nestedIn)
                    parent = parent->nestedIn;

                if (info->nestedIn == parent)
                    return info;
            }

            // type has a namespace
            else
            {
                // FIXME check if namespace path matches supplied path
                return info;
            }
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------

SymbolDatabase::SpaceInfo * SymbolDatabase::SpaceInfo::findInNestedList(const std::string & name)
{
    std::list<SpaceInfo *>::iterator it;

    for (it = nestedList.begin(); it != nestedList.end(); ++it)
    {
        if ((*it)->className == name)
            return (*it);
    }
    return 0;
}

//---------------------------------------------------------------------------

const SymbolDatabase::Func *SymbolDatabase::SpaceInfo::getDestructor() const
{
    std::list<Func>::const_iterator it;
    for (it = functionList.begin(); it != functionList.end(); ++it)
    {
        if (it->type == Func::Destructor)
            return &*it;
    }
    return 0;
}

//---------------------------------------------------------------------------

unsigned int SymbolDatabase::SpaceInfo::getNestedNonFunctions() const
{
    unsigned int nested = 0;
    std::list<SpaceInfo *>::const_iterator ni;
    for (ni = nestedList.begin(); ni != nestedList.end(); ++ni)
    {
        if ((*ni)->type != SpaceInfo::Function)
            nested++;
    }
    return nested;
}
