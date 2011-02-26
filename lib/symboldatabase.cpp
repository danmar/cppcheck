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
    // find all namespaces (class,struct and namespace)
    Scope *scope = new Scope(this, NULL, NULL);
    scopeList.push_back(scope);
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Locate next class
        if (Token::Match(tok, "class|struct|namespace %var% [{:]"))
        {
            Scope *new_scope = new Scope(this, tok, scope);
            const Token *tok2 = tok->tokAt(2);

            // only create base list for classes and structures
            if (new_scope->isClassOrStruct())
            {
                // fill the classAndStructTypes set..
                classAndStructTypes.insert(new_scope->className);

                // goto initial '{'
                tok2 = initBaseInfo(new_scope, tok);

                // make sure we have valid code
                if (!tok2)
                {
                    delete new_scope;
                    break;
                }
            }

            new_scope->classStart = tok2;
            new_scope->classEnd = tok2->link();

            // make sure we have valid code
            if (!new_scope->classEnd)
            {
                delete new_scope;
                break;
            }

            scope = new_scope;

            // add namespace
            scopeList.push_back(scope);

            tok = tok2;
        }

        // forward declaration
        else if (Token::Match(tok, "class|struct %var% ;"))
        {
            // fill the classAndStructTypes set..
            classAndStructTypes.insert(tok->next()->str());

            /** @todo save forward declarations in database someday */
            tok = tok->tokAt(2);
            continue;
        }

        else
        {
            // check for end of space
            if (tok == scope->classEnd)
            {
                scope = scope->nestedIn;
                continue;
            }

            // check if in class or structure
            else if (scope->type == Scope::eClass || scope->type == Scope::eStruct)
            {
                const Token *funcStart = 0;
                const Token *argStart = 0;

                // What section are we in..
                if (tok->str() == "private:")
                    scope->access = Private;
                else if (tok->str() == "protected:")
                    scope->access = Protected;
                else if (tok->str() == "public:")
                    scope->access = Public;
                else if (Token::Match(tok, "public|protected|private %var% :"))
                {
                    if (tok->str() == "private")
                        scope->access = Private;
                    else if (tok->str() == "protected")
                        scope->access = Protected;
                    else if (tok->str() == "public")
                        scope->access = Public;

                    tok = tok->tokAt(2);
                }

                // class function?
                else if (tok->previous()->str() != "::" && isFunction(tok, &funcStart, &argStart))
                {
                    Function function;

                    // save the function definition argument start '('
                    function.argDef = argStart;

                    // save the access type
                    function.access = scope->access;

                    // save the function name location
                    function.tokenDef = funcStart;

                    // operator function
                    if (function.tokenDef->str().find("operator") == 0)
                    {
                        function.isOperator = true;

                        // 'operator =' is special
                        if (function.tokenDef->str() == "operator=")
                            function.type = Function::eOperatorEqual;
                    }

                    // class constructor/destructor
                    else if (function.tokenDef->str() == scope->className)
                    {
                        if (function.tokenDef->previous()->str() == "~")
                            function.type = Function::eDestructor;
                        else if ((Token::Match(function.tokenDef, "%var% ( const %var% & )") ||
                                  Token::Match(function.tokenDef, "%var% ( const %var% & %var% )")) &&
                                 function.tokenDef->strAt(3) == scope->className)
                            function.type = Function::eCopyConstructor;
                        else if ((Token::Match(function.tokenDef, "%var% ( %var% & )") ||
                                  Token::Match(function.tokenDef, "%var% ( %var% & %var% )")) &&
                                 function.tokenDef->strAt(2) == scope->className)
                            function.type = Function::eCopyConstructor;
                        else
                            function.type = Function::eConstructor;

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
                    if (Token::Match(end, ") const| = %any%"))
                        function.isPure = true;

                    // count the number of constructors
                    if (function.type == Function::eConstructor ||
                        function.type == Function::eCopyConstructor)
                        scope->numConstructors++;

                    // assume implementation is inline (definition and implementation same)
                    function.token = function.tokenDef;
                    function.arg = function.argDef;

                    // out of line function
                    if (Token::Match(end, ") const| ;") ||
                        Token::Match(end, ") const| = %any%"))
                    {
                        // find the function implementation later
                        tok = end->next();

                        scope->functionList.push_back(function);
                    }

                    // inline function
                    else
                    {
                        function.isInline = true;
                        function.hasBody = true;

                        // find start of function '{'
                        while (end && end->str() != "{")
                            end = end->next();

                        // save start of function
                        function.start = end;

                        scope->functionList.push_back(function);

                        const Token *tok2 = funcStart;
                        Scope *functionOf = scope;

                        addNewFunction(&scope, &tok2);
                        if (scope)
                            scope->functionOf = functionOf;

                        tok = tok2;
                    }
                }

                // nested class or friend function?
                else if (tok->previous()->str() == "::" && isFunction(tok, &funcStart, &argStart))
                {
                    /** @todo check entire qualification for match */
                    Scope * nested = scope->findInNestedListRecursive(tok->strAt(-2));

                    if (nested)
                        addFunction(&scope, &tok, argStart);
                    else
                    {
                        /** @todo handle friend functions */
                    }
                }

                // friend class declaration?
                else if (Token::Match(tok, "friend class| %any% ;"))
                {
                    Scope::FriendInfo friendInfo;

                    friendInfo.name = tok->strAt(1) == "class" ? tok->strAt(2) : tok->strAt(1);
                    /** @todo fill this in later after parsing is complete */
                    friendInfo.scope = 0;

                    scope->friendList.push_back(friendInfo);
                }
            }
            else if (scope->type == Scope::eNamespace || scope->type == Scope::eGlobal)
            {
                const Token *funcStart = 0;
                const Token *argStart = 0;

                // function?
                if (isFunction(tok, &funcStart, &argStart))
                {
                    // has body?
                    if (Token::Match(argStart->link(), ") const| {|:"))
                    {
                        Scope *old_scope = scope;

                        // class function
                        if (tok->previous() && tok->previous()->str() == "::")
                            addFunction(&scope, &tok, argStart);

                        // class destructor
                        else if (tok->previous() && tok->previous()->str() == "~" &&
                                 tok->previous()->previous() && tok->previous()->previous()->str() == "::")
                            addFunction(&scope, &tok, argStart);

                        // regular function
                        else
                        {
                            Function function;

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
                            function.type = Function::eFunction;

                            addNewFunction(&scope, &tok);

                            if (scope)
                                old_scope->functionList.push_back(function);
                        }

                        // syntax error
                        if (!scope)
                        {
                            scope = old_scope;
                            break;
                        }
                    }

                    // function returning function pointer with body
                    else if (Token::simpleMatch(argStart->link(), ") ) (") &&
                             Token::Match(argStart->link()->tokAt(2)->link(), ") const| {"))
                    {
                        const Token *tok1 = funcStart;
                        Scope *old_scope = scope;

                        // class function
                        if (tok1->previous()->str() == "::")
                            addFunction(&scope, &tok1, argStart);

                        // regular function
                        else
                        {
                            Function function;

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
                            function.type = Function::eFunction;
                            function.retFuncPtr = true;

                            addNewFunction(&scope, &tok1);

                            if (scope)
                                old_scope->functionList.push_back(function);
                        }

                        // syntax error?
                        if (!scope)
                        {
                            scope = old_scope;
                            break;
                        }

                        tok = tok1;
                    }
                }
            }
        }
    }

    std::list<Scope *>::iterator it;

    // fill in base class info
    for (it = scopeList.begin(); it != scopeList.end(); ++it)
    {
        scope = *it;

        // skip namespaces and functions
        if (!scope->isClassOrStruct())
            continue;

        // finish filling in base class info
        for (unsigned int i = 0; i < scope->derivedFrom.size(); ++i)
        {
            std::list<Scope *>::iterator it1;

            for (it1 = scopeList.begin(); it1 != scopeList.end(); ++it1)
            {
                Scope *scope1 = *it1;

                /** @todo handle derived base classes and namespaces */
                if (scope1->type == Scope::eClass || scope1->type == Scope::eStruct)
                {
                    // do class names match?
                    if (scope1->className == scope->derivedFrom[i].name)
                    {
                        // are they in the same namespace or different namespaces with same name?
                        if ((scope1->nestedIn == scope->nestedIn) ||
                            ((scope1->nestedIn && scope1->nestedIn->type == Scope::eNamespace) &&
                             (scope->nestedIn && scope->nestedIn->type == Scope::eNamespace) &&
                             (scope1->nestedIn->className == scope->nestedIn->className)))
                        {
                            scope->derivedFrom[i].scope = scope1;
                            break;
                        }
                    }
                }
            }
        }
    }

    // fill in variable info
    for (it = scopeList.begin(); it != scopeList.end(); ++it)
    {
        scope = *it;

        // skip functions
        if (scope->type != Scope::eFunction)
        {
            // find variables
            scope->getVariableList();
        }
    }

    // fill in function arguments
    for (it = scopeList.begin(); it != scopeList.end(); ++it)
    {
        scope = *it;

        std::list<Function>::iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            // add arguments
            func->addArguments(this, scope);
        }
    }

    // determine if user defined type needs initialization
    unsigned int unknowns = 0; // stop checking when there are no unknowns
    unsigned int retry = 0;    // bail if we don't resolve all the variable types for some reason

    do
    {
        unknowns = 0;

        for (it = scopeList.begin(); it != scopeList.end(); ++it)
        {
            scope = *it;

            if (scope->isClassOrStruct() && scope->needInitialization == Scope::Unknown)
            {
                // check for default constructor
                bool hasDefaultConstructor = false;

                std::list<Function>::const_iterator func;

                for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
                {
                    if (func->type == Function::eConstructor)
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
                    scope->needInitialization = Scope::False;

                // check each member variable to see if it needs initialization
                else
                {
                    bool needInitialization = false;
                    bool unknown = false;

                    std::list<Variable>::const_iterator var;
                    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var)
                    {
                        if (var->isClass())
                        {
                            if (var->type())
                            {
                                // does this type need initialization?
                                if (var->type()->needInitialization == Scope::True)
                                    needInitialization = true;
                                else if (var->type()->needInitialization == Scope::Unknown)
                                    unknown = true;
                            }
                        }
                        else
                            needInitialization = true;
                    }

                    if (!unknown)
                    {
                        if (needInitialization)
                            scope->needInitialization = Scope::True;
                        else
                            scope->needInitialization = Scope::False;
                    }

                    if (scope->needInitialization == Scope::Unknown)
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
        for (it = scopeList.begin(); it != scopeList.end(); ++it)
        {
            scope = *it;

            if (scope->isClassOrStruct() && scope->needInitialization == Scope::Unknown)
            {
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.line = scope->classDef->linenr();
                loc.setfile(_tokenizer->file(scope->classDef));
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
    std::list<Scope *>::iterator it;

    for (it = scopeList.begin(); it != scopeList.end(); ++it)
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
    else if (Token::Match(tok, "%var% (") &&
             (Token::Match(tok->next()->link(), ") const| ;|{|=") ||
              Token::Match(tok->next()->link(), ") : %var% (|::")))
    {
        *funcStart = tok;
        *argStart = tok->next();
        return true;
    }

    return false;
}

bool SymbolDatabase::argsMatch(const Scope *scope, const Token *first, const Token *second, const std::string &path, unsigned int depth) const
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
                 second->next()->str() == scope->className && second->strAt(2) == "::" &&
                 first->next()->str() == second->strAt(3))
        {
            second = second->tokAt(2);
        }

        first = first->next();
        second = second->next();
    }

    return match;
}

void SymbolDatabase::addFunction(Scope **scope, const Token **tok, const Token *argStart)
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

    std::list<Scope *>::iterator it1;

    // search for match
    for (it1 = scopeList.begin(); it1 != scopeList.end(); ++it1)
    {
        Scope *scope1 = *it1;

        bool match = false;
        if (scope1->className == tok1->str() && (scope1->type != Scope::eFunction))
        {
            // do the spaces match (same space) or do their names match (multiple namespaces)
            if ((*scope == scope1->nestedIn) || (*scope && scope1 &&
                                                 (*scope)->className == scope1->nestedIn->className &&
                                                 !(*scope)->className.empty() &&
                                                 (*scope)->type == scope1->nestedIn->type))
            {
                Scope *scope2 = scope1;

                while (scope2 && count > 0)
                {
                    count--;
                    tok1 = tok1->tokAt(2);
                    scope2 = scope2->findInNestedList(tok1->str());
                }

                if (count == 0 && scope2)
                {
                    match = true;
                    scope1 = scope2;
                }
            }
        }

        if (match)
        {
            std::list<Function>::iterator func;

            for (func = scope1->functionList.begin(); func != scope1->functionList.end(); ++func)
            {
                if (!func->hasBody)
                {
                    if (func->type == Function::eDestructor &&
                        (*tok)->previous()->str() == "~" &&
                        func->tokenDef->str() == (*tok)->str())
                    {
                        if (argsMatch(scope1, func->tokenDef->next(), (*tok)->next(), path, path_length))
                        {
                            func->hasBody = true;
                            func->token = *tok;
                            func->arg = argStart;
                            const Token *start = argStart->link()->next();
                            while (start && start->str() != "{")
                                start = start->next();
                            func->start = start;
                        }
                    }
                    else if (func->tokenDef->str() == (*tok)->str() && (*tok)->previous()->str() != "~")
                    {
                        if (argsMatch(scope1, func->tokenDef->next(), (*tok)->next(), path, path_length))
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
                                    const Token *start = argStart->link()->next();
                                    while (start && start->str() != "{")
                                        start = start->next();
                                    func->start = start;
                                }
                            }

                            // function returning function pointer?
                            else if (func->retFuncPtr)
                            {
                                // todo check for const
                                func->hasBody = true;
                                func->token = *tok;
                                func->arg = argStart;
                                const Token *start = argStart->link()->next()->next()->link()->next();
                                while (start && start->str() != "{")
                                    start = start->next();
                                func->start = start;
                            }
                        }
                    }

                    if (func->hasBody)
                    {
                        addNewFunction(scope, tok);
                        if (scope)
                        {
                            (*scope)->functionOf = scope1;
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
        addNewFunction(scope, tok);
}

void SymbolDatabase::addNewFunction(Scope **scope, const Token **tok)
{
    const Token *tok1 = *tok;
    Scope *new_scope = new Scope(this, tok1, *scope);

    // skip to start of function
    while (tok1 && tok1->str() != "{")
        tok1 = tok1->next();

    if (tok1)
    {
        new_scope->classStart = tok1;
        new_scope->classEnd = tok1->link();

        // syntax error?
        if (!new_scope->classEnd)
        {
            (*scope)->nestedList.pop_back();
            delete new_scope;
            while (tok1->next())
                tok1 = tok1->next();
            *scope = NULL;
            *tok = tok1;
            return;
        }

        *scope = new_scope;

        // add space
        scopeList.push_back(new_scope);

        *tok = tok1;
    }
    else
    {
        (*scope)->nestedList.pop_back();
        delete new_scope;
        *scope = NULL;
        *tok = NULL;
    }
}

const Token *SymbolDatabase::initBaseInfo(Scope *scope, const Token *tok)
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
            Scope::BaseInfo base;

            tok2 = tok2->next();

            // check for invalid code
            if (!tok2 || !tok2->next())
                return NULL;

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
            base.scope = 0;

            // add unhandled templates
            if (tok2->next()->str() == "<")
            {
                int level1 = 1;
                while (tok2->next())
                {
                    base.name += tok2->next()->str();

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
            scope->derivedFrom.push_back(base);
        }
        tok2 = tok2->next();
    }

    return tok2;
}

//---------------------------------------------------------------------------

unsigned int Function::argCount() const
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

unsigned int Function::initializedArgCount() const
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

void Function::addArguments(const SymbolDatabase *symbolDatabase, const Scope *scope)
{
    // check for non-empty argument list "( ... )"
    if (arg->link() != arg->next())
    {
        unsigned int count = 0;
        const Token *startTok;
        const Token *endTok;
        const Token *nameTok;
        bool isConstVar;
        const Token *tok = arg->next();
        for (;;)
        {
            startTok = tok;
            endTok = NULL;
            nameTok = NULL;
            isConstVar = bool(tok->str() == "const");

            while (tok->str() != "," && tok->str() != ")")
            {
                if (tok->varId() != 0)
                {
                    nameTok = tok;
                    endTok = tok->previous();
                }
                tok = tok->next();
            }

            // check for argument with no name
            if (!endTok)
                endTok = tok->previous();

            const Token *typeTok = startTok;
            if (isConstVar)
                typeTok = typeTok->next();

            const Scope *argType = NULL;
            if (!typeTok->isStandardType())
                argType = symbolDatabase->findVariableType(scope, typeTok);

            bool isClassVar = startTok == endTok && !startTok->isStandardType();

            argumentList.push_back(Variable(nameTok, startTok, endTok, count++, Argument, false, false, isConstVar, isClassVar, argType, scope));

            if (tok->str() == ")")
                break;

            tok = tok->next();
        }
    }
}

//---------------------------------------------------------------------------

Scope::Scope(SymbolDatabase *check_, const Token *classDef_, Scope *nestedIn_) :
    check(check_),
    classDef(classDef_),
    classStart(NULL),
    classEnd(NULL),
    nestedIn(nestedIn_),
    numConstructors(0),
    needInitialization(Scope::Unknown),
    functionOf(NULL)
{
    if (!classDef)
    {
        type = Scope::eGlobal;
        access = Public;
    }
    else if (classDef->str() == "class")
    {
        type = Scope::eClass;
        className = classDef->next()->str();
        access = Private;
    }
    else if (classDef->str() == "struct")
    {
        type = Scope::eStruct;
        className = classDef->next()->str();
        access = Public;
    }
    else if (classDef->str() == "union")
    {
        type = Scope::eUnion;
        className = classDef->next()->str();
        access = Public;
    }
    else if (classDef->str() == "namespace")
    {
        type = Scope::eNamespace;
        className = classDef->next()->str();
        access = Public;
    }
    else
    {
        type = Scope::eFunction;
        className = classDef->str();
        access = Public;
    }

    if (nestedIn)
        nestedIn->nestedList.push_back(this);
}

bool
Scope::hasDefaultConstructor() const
{
    if (numConstructors)
    {
        std::list<Function>::const_iterator func;

        for (func = functionList.begin(); func != functionList.end(); ++func)
        {
            if (func->type == Function::eConstructor &&
                func->argDef->link() == func->argDef->next())
                return true;
        }
    }
    return false;
}

AccessControl Scope::defaultAccess() const
{
    switch (type)
    {
    case eGlobal:
        return Global;
    case eClass:
        return Private;
    case eStruct:
        return Public;
    case eUnion:
        return Public;
    case eNamespace:
        return Namespace;
    case eFunction:
        return Local;
    }

    return Public;
}

// Get variable list..
void Scope::getVariableList()
{
    AccessControl varaccess = defaultAccess();
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
        else if (tok->previous() && !Token::Match(tok->previous(), ";|{|}|public:|protected:|private:"))
            continue;
        else if (Token::Match(tok, ";|{|}"))
            continue;

        // This is the start of a statement
        const Token *vartok = NULL;
        const Token *typetok = NULL;
        const Token *typestart = tok;

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
                                                       "Scope::getVariableList found variable \'" + vartok->str() + "\' with varid 0.",
                                                       "debug");
                if (check->_errorLogger)
                    check->_errorLogger->reportErr(errmsg);
                else
                    Check::reportError(errmsg);
            }

            const Scope *scope = NULL;

            if (typetok)
                scope = check->findVariableType(this, typetok);

            addVariable(vartok, typestart, vartok->previous(), varaccess, isMutable, isStatic, isConst, isClass, scope, this);
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
    while (Token::Match(ret, "%type% ::"))
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

bool Scope::isVariableDeclaration(const Token* tok, const Token*& vartok, const Token*& typetok) const
{
    const Token* localTypeTok = skipScopeIdentifiers(tok);
    const Token* localVarTok = NULL;

    if (Token::Match(localTypeTok, "%type% <"))
    {
        const Token* closeTok = NULL;
        bool found = findClosingBracket(localTypeTok->next(), closeTok);
        if (found)
        {
            localVarTok = skipPointers(closeTok->next());

            if (Token::Match(localVarTok, ":: %type% %var% ;|="))
            {
                localTypeTok = localVarTok->next();
                localVarTok = localVarTok->tokAt(2);
            }
        }
    }
    else if (Token::Match(localTypeTok, "%type%"))
    {
        localVarTok = skipPointers(localTypeTok->next());
    }

    if (isSimpleVariable(localVarTok) || isArrayVariable(localVarTok))
    {
        vartok = localVarTok;
        typetok = localTypeTok;
    }

    return NULL != vartok;
}

bool Scope::isSimpleVariable(const Token* tok) const
{
    return Token::Match(tok, "%var% ;|=");
}

bool Scope::isArrayVariable(const Token* tok) const
{
    return Token::Match(tok, "%var% [") && tok->next()->str() != "operator";
}

bool Scope::findClosingBracket(const Token* tok, const Token*& close) const
{
    bool found = false;
    if (NULL != tok && tok->str() == "<")
    {
        unsigned int depth = 0;
        for (close = tok; (close != NULL) && (close->str() != ";") && (close->str() != "="); close = close->next())
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

const Scope *SymbolDatabase::findVariableType(const Scope *start, const Token *type) const
{
    std::list<Scope *>::const_iterator it;

    for (it = scopeList.begin(); it != scopeList.end(); ++it)
    {
        const Scope *scope = *it;

        // skip namespaces and functions
        if (scope->type == Scope::eNamespace || scope->type == Scope::eFunction || scope->type == Scope::eGlobal)
            continue;

        // do the names match?
        if (scope->className == type->str())
        {
            // check if type does not have a namespace
            if (type->previous()->str() != "::")
            {
                const Scope *parent = start;

                // check if in same namespace
                while (parent && parent != scope->nestedIn)
                    parent = parent->nestedIn;

                if (scope->nestedIn == parent)
                    return scope;
            }

            // type has a namespace
            else
            {
                // FIXME check if namespace path matches supplied path
                return scope;
            }
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------

const Scope *SymbolDatabase::findFunctionScopeByToken(const Token *tok) const
{
    std::list<Scope *>::const_iterator scope;

    for (scope = scopeList.begin(); scope != scopeList.end(); ++scope)
    {
        if ((*scope)->type == Scope::eFunction)
        {
            if ((*scope)->classDef == tok)
                return (*scope);
        }
    }
    return 0;
}

//---------------------------------------------------------------------------

const Function *SymbolDatabase::findFunctionByToken(const Token *tok) const
{
    std::list<Scope *>::const_iterator scope;

    for (scope = scopeList.begin(); scope != scopeList.end(); ++scope)
    {
        std::list<Function>::const_iterator func;

        for (func = (*scope)->functionList.begin(); func != (*scope)->functionList.end(); ++func)
        {
            if (func->token == tok)
                return &*func;
        }
    }
    return 0;
}

//---------------------------------------------------------------------------

Scope * Scope::findInNestedList(const std::string & name)
{
    std::list<Scope *>::iterator it;

    for (it = nestedList.begin(); it != nestedList.end(); ++it)
    {
        if ((*it)->className == name)
            return (*it);
    }
    return 0;
}

//---------------------------------------------------------------------------

Scope * Scope::findInNestedListRecursive(const std::string & name)
{
    std::list<Scope *>::iterator it;

    for (it = nestedList.begin(); it != nestedList.end(); ++it)
    {
        if ((*it)->className == name)
            return (*it);
    }

    for (it = nestedList.begin(); it != nestedList.end(); ++it)
    {
        Scope *child = (*it)->findInNestedListRecursive(name);
        if (child)
            return child;
    }
    return 0;
}

//---------------------------------------------------------------------------

const Function *Scope::getDestructor() const
{
    std::list<Function>::const_iterator it;
    for (it = functionList.begin(); it != functionList.end(); ++it)
    {
        if (it->type == Function::eDestructor)
            return &*it;
    }
    return 0;
}

//---------------------------------------------------------------------------

unsigned int Scope::getNestedNonFunctions() const
{
    unsigned int nested = 0;
    std::list<Scope *>::const_iterator ni;
    for (ni = nestedList.begin(); ni != nestedList.end(); ++ni)
    {
        if ((*ni)->type != Scope::eFunction)
            nested++;
    }
    return nested;
}
