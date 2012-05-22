/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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

#include <string>
#include <sstream>
#include <climits>

// Define ULLONG_MAX and LLONG_MAX for Borland
#ifdef __BORLANDC__
#define ULLONG_MAX ULONG_MAX
#define LLONG_MAX LONG_MAX
#endif

//---------------------------------------------------------------------------

SymbolDatabase::SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : _tokenizer(tokenizer), _settings(settings), _errorLogger(errorLogger)
{
    // create global scope
    scopeList.push_back(Scope(this, NULL, NULL));

    // pointer to current scope
    Scope *scope = &scopeList.back();

    // Store current access in each scope (depends on evaluation progress)
    std::map<const Scope*, AccessControl> access;

    // find all scopes
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Locate next class
        if (Token::Match(tok, "class|struct|union|namespace %var% [{:]")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();
            if (tok->str() == "class")
                access[new_scope] = Private;
            else if (tok->str() == "struct")
                access[new_scope] = Public;

            const Token *tok2 = tok->tokAt(2);

            // only create base list for classes and structures
            if (new_scope->isClassOrStruct()) {
                // goto initial '{'
                tok2 = new_scope->initBaseInfo(tok);

                // make sure we have valid code
                if (!tok2) {
                    scopeList.pop_back();
                    break;
                }
            }

            new_scope->classStart = tok2;
            new_scope->classEnd = tok2->link();

            // make sure we have valid code
            if (!new_scope->classEnd) {
                scopeList.pop_back();
                break;
            }

            // fill the classAndStructTypes set..
            if (new_scope->isClassOrStruct())
                classAndStructTypes.insert(new_scope->className);

            // make the new scope the current scope
            scope = &scopeList.back();
            scope->nestedIn->nestedList.push_back(scope);

            tok = tok2;
        }

        // forward declaration
        else if (Token::Match(tok, "class|struct %var% ;")) {
            // fill the classAndStructTypes set..
            classAndStructTypes.insert(tok->next()->str());

            /** @todo save forward declarations in database someday */
            tok = tok->tokAt(2);
            continue;
        }

        // using namespace
        else if (Token::Match(tok, "using namespace %type% ;|::")) {
            // save location
            scope->usingList.push_back(tok);

            tok = tok->tokAt(3);
        }

        // unnamed struct and union
        else if (Token::Match(tok, "struct|union {") &&
                 Token::Match(tok->next()->link(), "} *|&| %var% ;|[")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();
            access[new_scope] = Public;

            const Token* varNameTok = tok->next()->link()->next();
            if (varNameTok->str() == "*") {
                varNameTok = varNameTok->next();
            } else if (varNameTok->str() == "&") {
                varNameTok = varNameTok->next();
            }

            scope->addVariable(varNameTok, tok, tok, access[scope], new_scope, scope);

            const Token *tok2 = tok->next();

            new_scope->classStart = tok2;
            new_scope->classEnd = tok2->link();

            // make sure we have valid code
            if (!new_scope->classEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope = &scopeList.back();
            scope->nestedIn->nestedList.push_back(scope);

            tok = tok2;
        }

        // anonymous struct and union
        else if (Token::Match(tok, "struct|union {") &&
                 Token::simpleMatch(tok->next()->link(), "} ;")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();
            access[new_scope] = Public;

            const Token *tok2 = tok->next();

            new_scope->classStart = tok2;
            new_scope->classEnd = tok2->link();

            // make sure we have valid code
            if (!new_scope->classEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope = &scopeList.back();
            scope->nestedIn->nestedList.push_back(scope);

            tok = tok2;
        }

        else {
            // check for end of scope
            if (tok == scope->classEnd) {
                scope = scope->nestedIn;
                continue;
            }

            // check if in class or structure
            else if (scope->type == Scope::eClass || scope->type == Scope::eStruct) {
                const Token *funcStart = 0;
                const Token *argStart = 0;

                // What section are we in..
                if (tok->str() == "private:")
                    access[scope] = Private;
                else if (tok->str() == "protected:")
                    access[scope] = Protected;
                else if (tok->str() == "public:" || tok->str() == "__published:")
                    access[scope] = Public;
                else if (Token::Match(tok, "public|protected|private %var% :")) {
                    if (tok->str() == "private")
                        access[scope] = Private;
                    else if (tok->str() == "protected")
                        access[scope] = Protected;
                    else
                        access[scope] = Public;

                    tok = tok->tokAt(2);
                }

                // class function?
                else if (tok->previous()->str() != "::" && isFunction(tok, scope, &funcStart, &argStart)) {
                    Function function;

                    // save the function definition argument start '('
                    function.argDef = argStart;

                    // save the access type
                    function.access = access[scope];

                    // save the function name location
                    function.tokenDef = funcStart;

                    // operator function
                    if (function.tokenDef->str().find("operator") == 0) {
                        function.isOperator = true;

                        // 'operator =' is special
                        if (function.tokenDef->str() == "operator=")
                            function.type = Function::eOperatorEqual;
                    }

                    // class constructor/destructor
                    else if (function.tokenDef->str() == scope->className) {
                        // destructor
                        if (function.tokenDef->previous()->str() == "~")
                            function.type = Function::eDestructor;

                        // copy constructor
                        else if ((Token::Match(function.tokenDef, "%var% ( const %var% & )") ||
                                  Token::Match(function.tokenDef, "%var% ( const %var% & %var% )")) &&
                                 function.tokenDef->strAt(3) == scope->className)
                            function.type = Function::eCopyConstructor;

                        // copy constructor with non-const argument
                        else if ((Token::Match(function.tokenDef, "%var% ( %var% & )") ||
                                  Token::Match(function.tokenDef, "%var% ( %var% & %var% )")) &&
                                 function.tokenDef->strAt(2) == scope->className)
                            function.type = Function::eCopyConstructor;

                        // regular constructor
                        else
                            function.type = Function::eConstructor;

                        if (function.tokenDef->previous()->str() == "explicit")
                            function.isExplicit = true;
                    }

                    // function returning function pointer
                    else if (tok->str() == "(") {
                        function.retFuncPtr = true;
                    }

                    const Token *tok1 = tok;

                    // look for end of previous statement
                    while (tok1->previous() && !Token::Match(tok1->previous(), ";|}|{|public:|protected:|private:")) {
                        // virtual function
                        if (tok1->previous()->str() == "virtual") {
                            function.isVirtual = true;
                            break;
                        }

                        // static function
                        else if (tok1->previous()->str() == "static") {
                            function.isStatic = true;
                            break;
                        }

                        // friend function
                        else if (tok1->previous()->str() == "friend") {
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
                        Token::Match(end, ") const| = %any%")) {
                        // find the function implementation later
                        tok = end->next();

                        scope->functionList.push_back(function);
                    }

                    // inline function
                    else {
                        function.isInline = true;
                        function.hasBody = true;

                        // find start of function '{'
                        while (end && end->str() != "{")
                            end = end->next();
                        if (!end)
                            continue;

                        scope->functionList.push_back(function);

                        const Token *tok2 = funcStart;
                        Scope *functionOf = scope;

                        addNewFunction(&scope, &tok2);
                        if (scope) {
                            scope->functionOf = functionOf;
                            scope->function = &functionOf->functionList.back();
                            scope->function->functionScope = scope;
                        }

                        tok = tok2;
                    }
                }

                // nested class or friend function?
                else if (tok->previous()->str() == "::" && isFunction(tok, scope, &funcStart, &argStart)) {
                    /** @todo check entire qualification for match */
                    Scope * nested = scope->findInNestedListRecursive(tok->strAt(-2));

                    if (nested)
                        addClassFunction(&scope, &tok, argStart);
                    else {
                        /** @todo handle friend functions */
                    }
                }

                // friend class declaration?
                else if (Token::Match(tok, "friend class| %any% ;")) {
                    Scope::FriendInfo friendInfo;

                    friendInfo.name = tok->strAt(1) == "class" ? tok->strAt(2) : tok->strAt(1);
                    // fill this in after parsing is complete
                    friendInfo.scope = 0;

                    scope->friendList.push_back(friendInfo);
                }
            } else if (scope->type == Scope::eNamespace || scope->type == Scope::eGlobal) {
                const Token *funcStart = 0;
                const Token *argStart = 0;

                // function?
                if (isFunction(tok, scope, &funcStart, &argStart)) {
                    // has body?
                    if (Token::Match(argStart->link(), ") const| {|:")) {
                        Scope *old_scope = scope;

                        // class function
                        if (tok->previous() && tok->previous()->str() == "::")
                            addClassFunction(&scope, &tok, argStart);

                        // class destructor
                        else if (tok->previous() && tok->previous()->str() == "~" &&
                                 tok->tokAt(-2) && tok->strAt(-2) == "::")
                            addClassFunction(&scope, &tok, argStart);

                        // regular function
                        else
                            addGlobalFunction(scope, tok, argStart, funcStart);

                        // syntax error
                        if (!scope) {
                            scope = old_scope;
                            break;
                        }
                    }

                    // function returning function pointer with body
                    else if (Token::simpleMatch(argStart->link(), ") ) (") &&
                             Token::Match(argStart->link()->linkAt(2), ") const| {")) {
                        tok = funcStart;
                        Scope *old_scope = scope;

                        // class function
                        if (tok->previous()->str() == "::")
                            addClassFunction(&scope, &tok, argStart);

                        // regular function
                        else {
                            Function* function = addGlobalFunction(scope, tok, argStart, funcStart);
                            function->retFuncPtr = true;
                        }

                        // syntax error?
                        if (!scope) {
                            scope = old_scope;
                            break;
                        }
                    }

                    // function prototype
                    else if (Token::simpleMatch(argStart->link(), ") ;")) {
                        bool newFunc = true; // Is this function already in the database?
                        for (std::list<Function>::const_iterator i = scope->functionList.begin(); i != scope->functionList.end(); ++i) {
                            if (i->tokenDef->str() == tok->str() && Function::argsMatch(scope, i->argDef, argStart, "", 0))
                                newFunc = false;
                        }
                        // save function prototype in database
                        if (newFunc)
                            addGlobalFunctionDecl(scope, argStart, funcStart);

                        tok = argStart->link()->next();
                        continue;
                    }

                    // function returning function pointer prototype
                    else if (Token::simpleMatch(argStart->link(), ") ) (") &&
                             Token::simpleMatch(argStart->link()->linkAt(2), ") ;")) {
                        bool newFunc = true; // Is this function already in the database?
                        for (std::list<Function>::const_iterator i = scope->functionList.begin(); i != scope->functionList.end(); ++i) {
                            if (i->tokenDef->str() == tok->str() && Function::argsMatch(scope, i->argDef, argStart, "", 0))
                                newFunc = false;
                        }
                        // save function prototype in database
                        if (newFunc) {
                            Function* func = addGlobalFunctionDecl(scope, argStart, funcStart);
                            func->retFuncPtr = true;
                        }

                        tok = argStart->link()->linkAt(2)->next();
                        continue;
                    }
                }
            } else if (scope->type == Scope::eFunction || scope->isLocal()) {
                if (Token::simpleMatch(tok, "if (") &&
                    Token::simpleMatch(tok->next()->link(), ") {")) {
                    const Token *tok1 = tok->next()->link()->next();
                    scopeList.push_back(Scope(this, tok, scope, Scope::eIf, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                } else if (Token::simpleMatch(tok, "else {")) {
                    const Token *tok1 = tok->next();
                    scopeList.push_back(Scope(this, tok, scope, Scope::eElse, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                } else if (Token::simpleMatch(tok, "else if (") &&
                           Token::simpleMatch(tok->linkAt(2), ") {")) {
                    const Token *tok1 = tok->linkAt(2)->next();
                    scopeList.push_back(Scope(this, tok, scope, Scope::eElseIf, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                } else if (Token::simpleMatch(tok, "for (") &&
                           Token::simpleMatch(tok->next()->link(), ") {")) {
                    // save location of initialization
                    const Token *tok1 = tok->next()->link()->next();
                    const Token *tok2 = tok->tokAt(2);
                    scopeList.push_back(Scope(this, tok, scope, Scope::eFor, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                    // check for variable declaration and add it to new scope if found
                    scope->checkVariable(tok2, Local);
                } else if (Token::simpleMatch(tok, "while (") &&
                           Token::simpleMatch(tok->next()->link(), ") {")) {
                    const Token *tok1 = tok->next()->link()->next();
                    scopeList.push_back(Scope(this, tok, scope, Scope::eWhile, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                } else if (Token::simpleMatch(tok, "do {")) {
                    const Token *tok1 = tok->next();
                    scopeList.push_back(Scope(this, tok, scope, Scope::eDo, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                } else if (Token::simpleMatch(tok, "switch (") &&
                           Token::simpleMatch(tok->next()->link(), ") {")) {
                    const Token *tok1 = tok->next()->link()->next();
                    scopeList.push_back(Scope(this, tok, scope, Scope::eSwitch, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                } else if (Token::simpleMatch(tok, "try {")) {
                    const Token *tok1 = tok->next();
                    scopeList.push_back(Scope(this, tok, scope, Scope::eTry, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                } else if (Token::simpleMatch(tok, "catch (") &&
                           Token::simpleMatch(tok->next()->link(), ") {")) {
                    const Token *tok1 = tok->next()->link()->next();
                    const Token *tok2 = tok->tokAt(2);
                    scopeList.push_back(Scope(this, tok, scope, Scope::eCatch, tok1));
                    tok = tok1;
                    scope = &scopeList.back();
                    scope->nestedIn->nestedList.push_back(scope);
                    // check for variable declaration and add it to new scope if found
                    scope->checkVariable(tok2, Throw);
                } else if (tok->str() == "{") {
                    if (!Token::Match(tok->previous(), "=|,")) {
                        scopeList.push_back(Scope(this, tok, scope, Scope::eUnconditional, tok));
                        scope = &scopeList.back();
                        scope->nestedIn->nestedList.push_back(scope);
                    } else {
                        tok = tok->link();
                    }
                }
            }
        }
    }

    std::list<Scope>::iterator it;

    // fill in base class info
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        scope = &(*it);

        // skip namespaces and functions
        if (!scope->isClassOrStruct())
            continue;

        // finish filling in base class info
        for (unsigned int i = 0; i < scope->derivedFrom.size(); ++i) {
            std::list<Scope>::const_iterator it1;

            // check all scopes for match
            for (it1 = scopeList.begin(); it1 != scopeList.end(); ++it1) {
                // check scope for match
                const Scope *scope1 = it1->findQualifiedScope(scope->derivedFrom[i].name);

                // found match?
                if (scope1) {
                    // set found scope
                    scope->derivedFrom[i].scope = const_cast<Scope *>(scope1);
                    break;
                }
            }
        }
    }

    // fill in friend info
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        for (std::list<Scope::FriendInfo>::iterator i = it->friendList.begin(); i != it->friendList.end(); ++i) {
            for (std::list<Scope>::iterator j = scopeList.begin(); j != scopeList.end(); ++j) {
                // check scope for match
                scope = const_cast<Scope*>(j->findQualifiedScope(i->name));

                // found match?
                if (scope && scope->isClassOrStruct()) {
                    // set found scope
                    i->scope = scope;
                    break;
                }
            }
        }
    }

    // fill in variable info
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        // find variables
        it->getVariableList();
    }

    // fill in function arguments
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        std::list<Function>::iterator func;

        for (func = it->functionList.begin(); func != it->functionList.end(); ++func) {
            // add arguments
            func->addArguments(this, scope);
        }
    }

    // determine if user defined type needs initialization
    unsigned int unknowns = 0; // stop checking when there are no unknowns
    unsigned int retry = 0;    // bail if we don't resolve all the variable types for some reason

    do {
        unknowns = 0;

        for (it = scopeList.begin(); it != scopeList.end(); ++it) {
            scope = &(*it);

            if (scope->isClassOrStruct() && scope->needInitialization == Scope::Unknown) {
                // check for default constructor
                bool hasDefaultConstructor = false;

                std::list<Function>::const_iterator func;

                for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                    if (func->type == Function::eConstructor) {
                        // check for no arguments: func ( )
                        if (func->argCount() == 0) {
                            hasDefaultConstructor = true;
                            break;
                        }

                        /** check for arguments with default values */
                        else if (func->argCount() == func->initializedArgCount()) {
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
                else {
                    bool needInitialization = false;
                    bool unknown = false;

                    std::list<Variable>::const_iterator var;
                    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
                        if (var->isClass()) {
                            if (var->type()) {
                                // does this type need initialization?
                                if (var->type()->needInitialization == Scope::True)
                                    needInitialization = true;
                                else if (var->type()->needInitialization == Scope::Unknown)
                                    unknown = true;
                            }
                        } else
                            needInitialization = true;
                    }

                    if (!unknown) {
                        if (needInitialization)
                            scope->needInitialization = Scope::True;
                        else
                            scope->needInitialization = Scope::False;
                    }

                    if (scope->needInitialization == Scope::Unknown)
                        unknowns++;
                }
            } else if (scope->type == Scope::eUnion && scope->needInitialization == Scope::Unknown)
                scope->needInitialization = Scope::True;
        }

        retry++;
    } while (unknowns && retry < 100);

    // this shouldn't happen so output a debug warning
    if (retry == 100 && _settings->debugwarnings) {
        for (it = scopeList.begin(); it != scopeList.end(); ++it) {
            scope = &(*it);

            if (scope->isClassOrStruct() && scope->needInitialization == Scope::Unknown)
                debugMessage(scope->classDef, "SymbolDatabase::SymbolDatabase couldn't resolve all user defined types.");
        }
    }

    // create variable symbol table
    _variableList.resize(_tokenizer->varIdCount() + 1);
    std::fill_n(_variableList.begin(), _variableList.size(), (const Variable*)NULL);

    // check all scopes for variables
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        scope = &(*it);

        // add all variables
        std::list<Variable>::const_iterator var;
        for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
            unsigned int varId = var->varId();
            if (varId)
                _variableList[varId] = &(*var);
        }

        // add all function paramaters
        std::list<Function>::const_iterator func;
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            // ignore function without implementations
            if (!func->hasBody)
                continue;

            std::list<Variable>::const_iterator arg;
            for (arg = func->argumentList.begin(); arg != func->argumentList.end(); ++arg) {
                // check for named parameters
                if (arg->nameToken() && arg->varId()) {
                    unsigned int varId = arg->varId();
                    if (varId)
                        _variableList[varId] = &(*arg);
                }
            }
        }
    }

    /* set all unknown array dimensions that are set by a variable to the maximum size of that variable type */
    for (size_t i = 1; i <= _tokenizer->varIdCount(); i++) {
        // check each array variable
        if (_variableList[i] && _variableList[i]->isArray()) {
            // check each array dimension
            for (size_t j = 0; j < _variableList[i]->dimensions().size(); j++) {
                // check for a single token dimension that is a variable
                if (_variableList[i]->dimensions()[j].start &&
                    (_variableList[i]->dimensions()[j].start == _variableList[i]->dimensions()[j].end) &&
                    _variableList[i]->dimensions()[j].start->varId()) {
                    Dimension &dimension = const_cast<Dimension &>(_variableList[i]->dimensions()[j]);

                    // get maximum size from type
                    // find where this type is defined
                    const Variable *var = getVariableFromVarId(dimension.start->varId());

                    // make sure it is in the database
                    if (!var)
                        break;

                    // get type token
                    const Token *index_type = var->typeEndToken();

                    if (index_type->str() == "char") {
                        if (index_type->isUnsigned())
                            dimension.num = UCHAR_MAX + 1;
                        else if (index_type->isSigned())
                            dimension.num = SCHAR_MAX + 1;
                        else
                            dimension.num = CHAR_MAX + 1;
                    } else if (index_type->str() == "short") {
                        if (index_type->isUnsigned())
                            dimension.num = USHRT_MAX + 1;
                        else
                            dimension.num = SHRT_MAX + 1;
                    }

                    // checkScope assumes size is signed int so we limit the following sizes to INT_MAX
                    else if (index_type->str() == "int") {
                        if (index_type->isUnsigned())
                            dimension.num = UINT_MAX + 1ULL;
                        else
                            dimension.num = INT_MAX + 1ULL;
                    } else if (index_type->str() == "long") {
                        if (index_type->isUnsigned()) {
                            if (index_type->isLong())
                                dimension.num = ULLONG_MAX; // should be ULLONG_MAX + 1ULL
                            else
                                dimension.num = ULONG_MAX; // should be ULONG_MAX + 1ULL
                        } else {
                            if (index_type->isLong())
                                dimension.num = LLONG_MAX; // should be LLONG_MAX + 1LL
                            else
                                dimension.num = LONG_MAX;  // should be LONG_MAX + 1LL
                        }
                    }
                }
            }
        }
    }
}

bool SymbolDatabase::isFunction(const Token *tok, const Scope* outerScope, const Token **funcStart, const Token **argStart) const
{
    // function returning function pointer? '... ( ... %var% ( ... ))( ... ) {'
    if (tok->str() == "(" &&
        tok->link()->previous()->str() == ")" &&
        tok->link()->next() &&
        tok->link()->next()->str() == "(" &&
        tok->link()->next()->link()->next() &&
        Token::Match(tok->link()->next()->link()->next(), "{|;|const|=")) {
        *funcStart = tok->link()->previous()->link()->previous();
        *argStart = tok->link()->previous()->link();
        return true;
    }

    // regular function?
    else if (Token::Match(tok, "%var% (") && tok->previous() &&
             (tok->previous()->isName() || tok->previous()->str() == "&" || tok->previous()->str() == "*" || // Either a return type in front of tok
              tok->previous()->str() == "::" || tok->previous()->str() == "~" || // or a scope qualifier in front of tok
              outerScope->isClassOrStruct()) && // or a ctor/dtor
             (Token::Match(tok->next()->link(), ") const| ;|{|=") ||
              Token::Match(tok->next()->link(), ") : %var% (|::"))) {
        *funcStart = tok;
        *argStart = tok->next();
        return true;
    }

    return false;
}

void Variable::evaluate()
{
    const Token* tok = _start;
    while (tok && tok->previous() && tok->previous()->isName())
        tok = tok->previous();
    for (const Token* const end = _name?_name:_end; tok != end;) {
        if (tok->str() == "static")
            setFlag(fIsStatic, true);
        else if (tok->str() == "mutable")
            setFlag(fIsMutable, true);
        else if (tok->str() == "const")
            setFlag(fIsConst, true);
        else if (tok->str() == "*") {
            setFlag(fIsPointer, true);
            setFlag(fIsConst, false); // Points to const, isn't necessarily const itself
        } else if (tok->str() == "&")
            setFlag(fIsReference, true);

        if (tok->str() == "<")
            tok->findClosingBracket(tok);
        else
            tok = tok->next();
    }

    if (_name)
        setFlag(fIsArray, arrayDimensions(_dimensions, _name->next()));
    if (_start)
        setFlag(fIsClass, !_start->isStandardType() && !isPointer() && !isReference());
    if (_access == Argument && _name) {
        tok = _name->next();
        while (tok->str() == "[")
            tok = tok->link();
        setFlag(fHasDefault, tok->str() == "=");
    }
}

bool Function::argsMatch(const Scope *scope, const Token *first, const Token *second, const std::string &path, unsigned int depth)
{
    while (first->str() == second->str()) {
        // at end of argument list
        if (first->str() == ")") {
            return true;
        }

        // skip default value assignment
        else if (first->next()->str() == "=") {
            first = first->nextArgument();

            if (second->next()->str() == "=") {
                second = second->nextArgument();
                if (!first || !second) { // End of argument list (first or second)
                    return !first && !second;
                }
            } else if (!first) { // End of argument list (first)
                return second->next() && second->next()->str() == ")";
            }
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
                 (first->next()->str() != second->next()->str())) {
            // skip variable names
            first = first->next();
            second = second->next();
        }

        // variable with class path
        else if (depth && Token::Match(first->next(), "%var%")) {
            std::string param = path + first->next()->str();

            if (Token::Match(second->next(), param.c_str())) {
                second = second->tokAt(int(depth) * 2);
            } else if (depth > 1) {
                std::string short_path = path;

                // remove last " :: "
                short_path.resize(short_path.size() - 4);

                // remove last name
                while (!short_path.empty() && short_path[short_path.size() - 1] != ' ')
                    short_path.resize(short_path.size() - 1);

                param = short_path + first->next()->str();
                if (Token::Match(second->next(), param.c_str())) {
                    second = second->tokAt((int(depth) - 1) * 2);
                }
            }
        }

        // nested class variable
        else if (depth == 0 && Token::Match(first->next(), "%var%") &&
                 second->next()->str() == scope->className && second->strAt(2) == "::" &&
                 first->next()->str() == second->strAt(3)) {
            second = second->tokAt(2);
        }

        first = first->next();
        second = second->next();
    }

    return false;
}

Function* SymbolDatabase::addGlobalFunction(Scope*& scope, const Token*& tok, const Token *argStart, const Token* funcStart)
{
    Function* function = 0;
    for (std::list<Function>::iterator i = scope->functionList.begin(); i != scope->functionList.end(); ++i) {
        if (i->tokenDef->str() == tok->str() && Function::argsMatch(scope, i->argDef, argStart, "", 0))
            function = &*i;
    }
    if (!function)
        function = addGlobalFunctionDecl(scope, argStart, funcStart);

    function->arg = argStart;
    function->token = funcStart;
    function->hasBody = true;

    addNewFunction(&scope, &tok);

    if (scope) {
        scope->function = function;
        function->functionScope = scope;
        return function;
    }
    return 0;
}

Function* SymbolDatabase::addGlobalFunctionDecl(Scope*& scope, const Token *argStart, const Token* funcStart)
{
    Function function;

    // save the function definition argument start '('
    function.argDef = argStart;

    // save the access type
    function.access = Public;

    // save the function name location
    function.tokenDef = funcStart;

    function.isInline = false;
    function.hasBody = false;
    function.type = Function::eFunction;

    scope->functionList.push_back(function);
    return &scope->functionList.back();
}

void SymbolDatabase::addClassFunction(Scope **scope, const Token **tok, const Token *argStart)
{
    int count = 0;
    std::string path;
    unsigned int path_length = 0;
    const Token *tok1;

    const bool destructor((*tok)->previous()->str() == "~");

    // skip class/struct name
    if (destructor)
        tok1 = (*tok)->tokAt(-3);
    else
        tok1 = (*tok)->tokAt(-2);

    // syntax error?
    if (!tok1)
        return;

    // back up to head of path
    while (tok1 && tok1->previous() && tok1->previous()->str() == "::") {
        path = tok1->str() + " :: " + path;
        tok1 = tok1->tokAt(-2);
        count++;
        path_length++;
    }

    if (tok1 && count) {
        path = tok1->str() + " :: " + path;
        path_length++;
    }

    std::list<Scope>::iterator it1;

    // search for match
    for (it1 = scopeList.begin(); it1 != scopeList.end(); ++it1) {
        Scope *scope1 = &(*it1);

        bool match = false;
        if (scope1->className == tok1->str() && (scope1->type != Scope::eFunction)) {
            // do the scopes match (same scope) or do their names match (multiple namespaces)
            if ((*scope == scope1->nestedIn) || (*scope && scope1 &&
                                                 (*scope)->className == scope1->nestedIn->className &&
                                                 !(*scope)->className.empty() &&
                                                 (*scope)->type == scope1->nestedIn->type)) {

                // nested scopes => check that they match
                {
                    const Scope *s1 = *scope;
                    const Scope *s2 = scope1->nestedIn;
                    while (s1 && s2) {
                        if (s1->className != s2->className)
                            break;
                        s1 = s1->nestedIn;
                        s2 = s2->nestedIn;
                    }
                    // Not matching scopes
                    if (s1 || s2)
                        continue;
                }

                Scope *scope2 = scope1;

                while (scope2 && count > 0) {
                    count--;
                    tok1 = tok1->tokAt(2);
                    scope2 = scope2->findInNestedList(tok1->str());
                }

                if (count == 0 && scope2) {
                    match = true;
                    scope1 = scope2;
                }
            }
        }

        if (match) {
            std::list<Function>::iterator func;

            for (func = scope1->functionList.begin(); func != scope1->functionList.end(); ++func) {
                if (!func->hasBody && func->tokenDef->str() == (*tok)->str()) {
                    if (Function::argsMatch(scope1, func->argDef, (*tok)->next(), path, path_length)) {
                        if (func->type == Function::eDestructor && destructor) {
                            func->hasBody = true;
                        } else if (func->type != Function::eDestructor && !destructor) {
                            // normal function?
                            if (!func->retFuncPtr && (*tok)->next()->link()) {
                                if ((func->isConst && (*tok)->next()->link()->next()->str() == "const") ||
                                    (!func->isConst && (*tok)->next()->link()->next()->str() != "const")) {
                                    func->hasBody = true;
                                }
                            }

                            // function returning function pointer?
                            else if (func->retFuncPtr) {
                                // todo check for const
                                func->hasBody = true;
                            }
                        }

                        if (func->hasBody) {
                            func->token = *tok;
                            func->arg = argStart;
                            addNewFunction(scope, tok);
                            if (*scope) {
                                (*scope)->functionOf = scope1;
                                (*scope)->function = &*func;
                                (*scope)->function->functionScope = *scope;
                            }
                            return;
                        }
                    }
                }
            }
        }
    }

    // class function of unknown class
    addNewFunction(scope, tok);
}

void SymbolDatabase::addNewFunction(Scope **scope, const Token **tok)
{
    const Token *tok1 = *tok;
    scopeList.push_back(Scope(this, tok1, *scope));
    Scope *new_scope = &scopeList.back();

    // skip to start of function
    while (tok1 && tok1->str() != "{")
        tok1 = tok1->next();

    if (tok1) {
        new_scope->classStart = tok1;
        new_scope->classEnd = tok1->link();

        // syntax error?
        if (!new_scope->classEnd) {
            scopeList.pop_back();
            while (tok1->next())
                tok1 = tok1->next();
            *scope = NULL;
            *tok = tok1;
            return;
        }

        *scope = new_scope;
        *tok = tok1;
        (*scope)->nestedIn->nestedList.push_back(*scope);
    } else {
        scopeList.pop_back();
        *scope = NULL;
        *tok = NULL;
    }
}

const Token *Scope::initBaseInfo(const Token *tok)
{
    // goto initial '{'
    const Token *tok2 = tok->tokAt(2);
    while (tok2 && tok2->str() != "{") {
        // skip unsupported templates
        if (tok2->str() == "<")
            tok2->findClosingBracket(tok2);

        // check for base classes
        else if (Token::Match(tok2, ":|,")) {
            Scope::BaseInfo base;

            base.isVirtual = false;

            tok2 = tok2->next();

            // check for invalid code
            if (!tok2 || !tok2->next())
                return NULL;

            if (tok2->str() == "virtual") {
                base.isVirtual = true;
                tok2 = tok2->next();
            }

            if (tok2->str() == "public") {
                base.access = Public;
                tok2 = tok2->next();
            } else if (tok2->str() == "protected") {
                base.access = Protected;
                tok2 = tok2->next();
            } else if (tok2->str() == "private") {
                base.access = Private;
                tok2 = tok2->next();
            } else {
                if (tok->str() == "class")
                    base.access = Private;
                else if (tok->str() == "struct")
                    base.access = Public;
            }

            if (tok2->str() == "virtual") {
                base.isVirtual = true;
                tok2 = tok2->next();
            }

            // handle global namespace
            if (tok2->str() == "::") {
                base.name = ":: ";
                tok2 = tok2->next();
            }

            // handle derived base classes
            while (Token::Match(tok2, "%var% ::")) {
                base.name += tok2->str();
                base.name += " :: ";
                tok2 = tok2->tokAt(2);
            }

            base.name += tok2->str();
            base.scope = NULL;

            // add unhandled templates
            if (tok2->next() && tok2->next()->str() == "<") {
                tok2 = tok2->next();
                base.name += tok2->str();

                int level1 = 1;
                while (tok2->next()) {
                    base.name += tok2->next()->str();

                    if (tok2->next()->str() == ">") {
                        level1--;
                        if (level1 == 0)
                            break;
                    } else if (tok2->next()->str() == "<")
                        level1++;

                    tok2 = tok2->next();
                }
            }

            // save pattern for base class name
            derivedFrom.push_back(base);
        }
        tok2 = tok2->next();
    }

    return tok2;
}

void SymbolDatabase::debugMessage(const Token *tok, const std::string &msg) const
{
    if (tok && _settings->debugwarnings) {
        const std::list<const Token*> locationList(1, tok);
        const ErrorLogger::ErrorMessage errmsg(locationList, &_tokenizer->list,
                                               Severity::debug,
                                               msg,
                                               "debug",
                                               false);
        if (_errorLogger)
            _errorLogger->reportErr(errmsg);
    }
}

bool Variable::arrayDimensions(std::vector<Dimension> &dimensions, const Token *tok)
{
    bool isArray = false;

    const Token *dim = tok;

    while (dim && dim->next() && dim->str() == "[") {
        Dimension dimension;
        // check for empty array dimension []
        if (dim->next()->str() != "]") {
            dimension.start = dim->next();
            dimension.end = dim->link()->previous();
            if (dimension.start == dimension.end && dimension.start->isNumber())
                dimension.num = MathLib::toLongNumber(dimension.start->str());
        }
        dimensions.push_back(dimension);
        dim = dim->link()->next();
        isArray = true;
    }
    return isArray;
}

static std::ostream & operator << (std::ostream & s, Scope::ScopeType type)
{
    s << (type == Scope::eGlobal ? "Global" :
          type == Scope::eClass ? "Class" :
          type == Scope::eStruct ? "Struct" :
          type == Scope::eUnion ? "Union" :
          type == Scope::eNamespace ? "Namespace" :
          type == Scope::eFunction ? "Function" :
          type == Scope::eIf ? "If" :
          type == Scope::eElse ? "Else" :
          type == Scope::eElseIf ? "ElseIf" :
          type == Scope::eFor ? "For" :
          type == Scope::eWhile ? "While" :
          type == Scope::eDo ? "Do" :
          type == Scope::eSwitch ? "Switch" :
          type == Scope::eTry ? "Try" :
          type == Scope::eCatch ? "Catch" :
          type == Scope::eUnconditional ? "Unconditional" :
          "Unknown");
    return s;
}

void SymbolDatabase::printVariable(const Variable *var, const char *indent) const
{
    std::cout << indent << "_name: " << var->nameToken();
    if (var->nameToken()) {
        std::cout << " " << var->name() << " " << _tokenizer->list.fileLine(var->nameToken()) << std::endl;
        std::cout << indent << "    varId: " << var->varId() << std::endl;
    } else
        std::cout << std::endl;
    std::cout << indent << "_start: " << var->typeStartToken() << " " << var->typeStartToken()->str()
              << " " << _tokenizer->list.fileLine(var->typeStartToken()) << std::endl;;
    std::cout << indent << "_end: " << var->typeEndToken() << " " << var->typeEndToken()->str()
              << " " << _tokenizer->list.fileLine(var->typeEndToken()) << std::endl;;
    std::cout << indent << "_index: " << var->index() << std::endl;
    std::cout << indent << "_access: " <<
              (var->isPublic() ? "Public" :
               var->isProtected() ? "Protected" :
               var->isPrivate() ? "Private" :
               var->isGlobal() ? "Global" :
               var->isNamespace() ? "Namespace" :
               var->isArgument() ? "Argument" :
               var->isLocal() ? "Local" :
               var->isThrow() ? "Throw" :
               "???")  << std::endl;
    std::cout << indent << "_flags: " << std::endl;
    std::cout << indent << "    isMutable: " << (var->isMutable() ? "true" : "false") << std::endl;
    std::cout << indent << "    isStatic: " << (var->isStatic() ? "true" : "false") << std::endl;
    std::cout << indent << "    isConst: " << (var->isConst() ? "true" : "false") << std::endl;
    std::cout << indent << "    isClass: " << (var->isClass() ? "true" : "false") << std::endl;
    std::cout << indent << "    isArray: " << (var->isArray() ? "true" : "false") << std::endl;
    std::cout << indent << "    isPointer: " << (var->isPointer() ? "true" : "false") << std::endl;
    std::cout << indent << "    isReference: " << (var->isReference() ? "true" : "false") << std::endl;
    std::cout << indent << "    hasDefault: " << (var->hasDefault() ? "true" : "false") << std::endl;
    std::cout << indent << "_type: ";
    if (var->type()) {
        std::cout << var->type()->className << " " << var->type()->type << " "
                  << _tokenizer->list.fileLine(var->type()->classDef) << std::endl;
    } else
        std::cout << "none" << std::endl;

    std::cout << indent << "_scope: ";
    if (var->scope()) {
        std::cout << var->scope()->className << " " << var->scope()->type;
        if (var->scope()->classDef)
            std::cout << " " << _tokenizer->list.fileLine(var->scope()->classDef) << std::endl;
        else
            std::cout << std::endl;
    } else
        std::cout << "none" << std::endl;

    std::cout << indent << "_dimensions:";
    for (size_t i = 0; i < var->dimensions().size(); i++) {
        std::cout << " " << var->dimension(i);
    }
    std::cout << std::endl;
}

void SymbolDatabase::printOut(const char *title) const
{
    if (title)
        std::cout << "\n### " << title << " ###\n";

    std::list<Scope>::const_iterator scope;

    for (scope = scopeList.begin(); scope != scopeList.end(); ++scope) {
        std::cout << "Scope: " << &*scope << std::endl;
        std::cout << "    type: " << scope->type << std::endl;
        std::cout << "    className: " << scope->className << std::endl;
        std::cout << "    classDef: " << scope->classDef;
        if (scope->classDef)
            std::cout << " " << scope->classDef->str() << " " << _tokenizer->list.fileLine(scope->classDef) << std::endl;
        else
            std::cout << std::endl;

        std::cout << "    classStart: " << scope->classStart;
        if (scope->classStart)
            std::cout << " " << scope->classStart->str() << " " << _tokenizer->list.fileLine(scope->classStart) << std::endl;
        else
            std::cout << std::endl;

        std::cout << "    classEnd: " << scope->classEnd;
        if (scope->classEnd)
            std::cout << " " << scope->classEnd->str() << " " <<  _tokenizer->list.fileLine(scope->classEnd) << std::endl;
        else
            std::cout << std::endl;

        std::list<Function>::const_iterator func;

        // find the function body if not implemented inline
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            std::cout << "    Function: " << &*func << std::endl;
            std::cout << "        name: " << func->tokenDef->str() << " "
                      << _tokenizer->list.fileLine(func->tokenDef) << std::endl;
            std::cout << "        type: " << (func->type == Function::eConstructor? "Constructor" :
                                              func->type == Function::eCopyConstructor ? "CopyConstructor" :
                                              func->type == Function::eOperatorEqual ? "OperatorEqual" :
                                              func->type == Function::eDestructor ? "Destructor" :
                                              func->type == Function::eFunction ? "Function" :
                                              "???") << std::endl;
            std::cout << "        access: " << (func->access == Public ? "Public" :
                                                func->access == Protected ? "Protected" :
                                                func->access == Private ? "Private" :
                                                "???")  << std::endl;
            std::cout << "        hasBody: " << (func->hasBody ? "true" : "false") << std::endl;
            std::cout << "        isInline: " << (func->isInline ? "true" : "false") << std::endl;
            std::cout << "        isConst: " << (func->isConst ? "true" : "false") << std::endl;
            std::cout << "        isVirtual: " << (func->isVirtual ? "true" : "false") << std::endl;
            std::cout << "        isPure: " << (func->isPure ? "true" : "false") << std::endl;
            std::cout << "        isStatic: " << (func->isStatic ? "true" : "false") << std::endl;
            std::cout << "        isFriend: " << (func->isFriend ? "true" : "false") << std::endl;
            std::cout << "        isExplicit: " << (func->isExplicit ? "true" : "false") << std::endl;
            std::cout << "        isOperator: " << (func->isOperator ? "true" : "false") << std::endl;
            std::cout << "        retFuncPtr: " << (func->retFuncPtr ? "true" : "false") << std::endl;
            std::cout << "        tokenDef: " << _tokenizer->list.fileLine(func->tokenDef) << std::endl;
            std::cout << "        argDef: " << _tokenizer->list.fileLine(func->argDef) << std::endl;
            if (func->hasBody) {
                std::cout << "        token: " << _tokenizer->list.fileLine(func->token) << std::endl;
                std::cout << "        arg: " << _tokenizer->list.fileLine(func->arg) << std::endl;
            }
            std::cout << "        functionScope: ";
            if (func->functionScope) {
                std::cout << func->functionScope->className << " "
                          <<  _tokenizer->list.fileLine(func->functionScope->classDef) << std::endl;
            } else
                std::cout << "Unknown" << std::endl;

            std::list<Variable>::const_iterator var;

            for (var = func->argumentList.begin(); var != func->argumentList.end(); ++var) {
                std::cout << "        Variable: " << &*var << std::endl;
                printVariable(&*var, "            ");
            }
        }

        std::list<Variable>::const_iterator var;

        for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
            std::cout << "    Variable: " << &*var << std::endl;
            printVariable(&*var, "        ");
        }

        std::cout << "    derivedFrom[" << scope->derivedFrom.size() << "] = (";

        size_t count = scope->derivedFrom.size();
        for (size_t i = 0; i < scope->derivedFrom.size(); ++i) {
            if (scope->derivedFrom[i].isVirtual)
                std::cout << "Virtual ";

            std::cout << (scope->derivedFrom[i].access == Public    ? " Public " :
                          scope->derivedFrom[i].access == Protected ? " Protected " :
                          scope->derivedFrom[i].access == Private   ? " Private " :
                          " Unknown");

            if (scope->derivedFrom[i].scope)
                std::cout << scope->derivedFrom[i].scope->type;
            else
                std::cout << " Unknown";

            std::cout << " " << scope->derivedFrom[i].name;
            if (count-- > 1)
                std::cout << ",";
        }

        std::cout << " )" << std::endl;

        std::cout << "    nestedIn: " << scope->nestedIn;
        if (scope->nestedIn) {
            std::cout << " " << scope->nestedIn->type << " "
                      << scope->nestedIn->className;
        }
        std::cout << std::endl;

        std::cout << "    nestedList[" << scope->nestedList.size() << "] = (";

        std::list<Scope *>::const_iterator nsi;

        count = scope->nestedList.size();
        for (nsi = scope->nestedList.begin(); nsi != scope->nestedList.end(); ++nsi) {
            std::cout << " " << &(*nsi) << " " << (*nsi)->type << " " << (*nsi)->className;
            if (count-- > 1)
                std::cout << ",";
        }

        std::cout << " )" << std::endl;

        std::cout << "    needInitialization: " << (scope->needInitialization == Scope::Unknown ? "Unknown" :
                  scope->needInitialization == Scope::True ? "True" :
                  scope->needInitialization == Scope::False ? "False" :
                  "Invalid") << std::endl;

        std::list<const Token *>::const_iterator use;

        for (use = scope->usingList.begin(); use != scope->usingList.end(); ++use) {
            std::cout << "    using: " << (*use)->strAt(2);
            const Token *tok1 = (*use)->tokAt(3);
            while (tok1 && tok1->str() == "::") {
                std::cout << "::" << tok1->strAt(1);
                tok1 = tok1->tokAt(2);
            }
            std::cout << " " << _tokenizer->list.fileLine(*use) << std::endl;
        }

        std::cout << "    functionOf: " << scope->functionOf;
        if (scope->functionOf) {
            std::cout << " " << scope->functionOf->type << " " << scope->functionOf->className;
            if (scope->functionOf->classDef)
                std::cout << " " << _tokenizer->list.fileLine(scope->functionOf->classDef);
        }
        std::cout << std::endl;

        std::cout << "    function: " << scope->function;
        if (scope->function) {
            std::cout << " " << scope->function->tokenDef->str() << " "
                      << _tokenizer->list.fileLine(scope->function->tokenDef);
        }
        std::cout << std::endl;
    }

    for (size_t i = 0; i < _variableList.size(); i++) {
        std::cout << "_variableList[" << i << "] = " << _variableList[i] << std::endl;
    }
}

//---------------------------------------------------------------------------

unsigned int Function::initializedArgCount() const
{
    unsigned int count = 0;
    std::list<Variable>::const_iterator var;

    for (var = argumentList.begin(); var != argumentList.end(); ++var) {
        if (var->hasDefault())
            ++count;
    }

    return count;
}

void Function::addArguments(const SymbolDatabase *symbolDatabase, const Scope *scope)
{
    // check for non-empty argument list "( ... )"
    if (arg && arg->link() != arg->next() && !Token::simpleMatch(arg, "( void )")) {
        unsigned int count = 0;

        for (const Token* tok = arg->next(); tok; tok = tok->next()) {
            const Token* startTok = tok;
            const Token* endTok   = NULL;
            const Token* nameTok  = NULL;

            if (tok->str() == "," || tok->str() == ")")
                return; // Syntax error

            do {
                if (tok->varId() != 0) {
                    nameTok = tok;
                    endTok = tok->previous();
                } else if (tok->str() == "[") {
                    // skip array dimension(s)
                    tok = tok->link();
                    while (tok->next()->str() == "[")
                        tok = tok->next()->link();
                } else if (tok->str() == "<") {
                    bool success = tok->findClosingBracket(tok);
                    if (!tok || !success) // something is wrong so just bail out
                        return;
                }

                tok = tok->next();

                if (!tok) // something is wrong so just bail
                    return;
            } while (tok->str() != "," && tok->str() != ")" && tok->str() != "=");

            const Token *typeTok = startTok->tokAt(startTok->str() == "const" ? 1 : 0);
            if (typeTok->str() == "struct")
                typeTok = typeTok->next();

            // check for argument with no name or missing varid
            if (!endTok) {
                if (tok->previous()->isName()) {
                    if (tok->previous() != typeTok) {
                        nameTok = tok->previous();
                        endTok = nameTok->previous();

                        if (hasBody)
                            symbolDatabase->debugMessage(nameTok, "Function::addArguments found argument \'" + nameTok->str() + "\' with varid 0.");
                    } else
                        endTok = startTok;
                } else
                    endTok = tok->previous();
            }

            const Scope *argType = NULL;
            if (!typeTok->isStandardType())
                argType = symbolDatabase->findVariableType(scope, typeTok);

            // skip default values
            if (tok->str() == "=") {
                while (tok->str() != "," && tok->str() != ")")
                    tok = tok->next();
            }

            argumentList.push_back(Variable(nameTok, startTok, endTok, count++, Argument, argType, functionScope));

            if (tok->str() == ")")
                break;
        }
    }
}

bool Function::isImplicitlyVirtual(bool defaultVal) const
{
    if (isVirtual)
        return true;
    else if (access == Private || access == Public || access == Protected) {
        bool safe = true;
        bool hasVirt = isImplicitlyVirtual_rec(functionScope->functionOf, safe);
        if (hasVirt)
            return true;
        else if (safe)
            return false;
        else
            return defaultVal;
    } else
        return false;
}

bool Function::isImplicitlyVirtual_rec(const Scope* scope, bool& safe) const
{
    // check each base class
    for (unsigned int i = 0; i < scope->derivedFrom.size(); ++i) {
        // check if base class exists in database
        if (scope->derivedFrom[i].scope) {
            const Scope *parent = scope->derivedFrom[i].scope;

            std::list<Function>::const_iterator func;

            // check if function defined in base class
            for (func = parent->functionList.begin(); func != parent->functionList.end(); ++func) {
                if (func->isVirtual && func->tokenDef->str() == tokenDef->str()) { // Base is virtual and of same name
                    const Token *temp1 = func->tokenDef->previous();
                    const Token *temp2 = tokenDef->previous();
                    bool returnMatch = true;

                    // check for matching return parameters
                    while (temp1->str() != "virtual") {
                        if (temp1->str() != temp2->str()) {
                            returnMatch = false;
                            break;
                        }

                        temp1 = temp1->previous();
                        temp2 = temp2->previous();
                    }

                    // check for matching function parameters
                    if (returnMatch && argsMatch(scope, func->argDef, argDef, "", 0)) {
                        return true;
                    }
                }
            }

            if (!parent->derivedFrom.empty())
                if (isImplicitlyVirtual_rec(parent, safe))
                    return true;
        } else {
            // unable to find base class so assume it has no virtual function
            safe = false;
            return false;
        }
    }
    return false;
}

const Variable* Function::getArgumentVar(unsigned int num) const
{
    for (std::list<Variable>::const_iterator i = argumentList.begin(); i != argumentList.end(); ++i) {
        if (i->index() == num)
            return(&*i);
        else if (i->index() > num)
            return 0;
    }
    return 0;
}


//---------------------------------------------------------------------------

Scope::Scope(SymbolDatabase *check_, const Token *classDef_, Scope *nestedIn_, ScopeType type_, const Token *start_) :
    check(check_),
    classDef(classDef_),
    classStart(start_),
    classEnd(start_->link()),
    nestedIn(nestedIn_),
    numConstructors(0),
    needInitialization(Scope::Unknown),
    type(type_),
    functionOf(NULL),
    function(NULL)
{
}

Scope::Scope(SymbolDatabase *check_, const Token *classDef_, Scope *nestedIn_) :
    check(check_),
    classDef(classDef_),
    classStart(NULL),
    classEnd(NULL),
    nestedIn(nestedIn_),
    numConstructors(0),
    needInitialization(Scope::Unknown),
    functionOf(NULL),
    function(NULL)
{
    if (!classDef) {
        type = Scope::eGlobal;
    } else if (classDef->str() == "class") {
        type = Scope::eClass;
        className = classDef->next()->str();
    } else if (classDef->str() == "struct") {
        type = Scope::eStruct;
        // anonymous and unnamed structs don't have a name
        if (classDef->next()->str() != "{")
            className = classDef->next()->str();
    } else if (classDef->str() == "union") {
        type = Scope::eUnion;
        // anonymous and unnamed unions don't have a name
        if (classDef->next()->str() != "{")
            className = classDef->next()->str();
    } else if (classDef->str() == "namespace") {
        type = Scope::eNamespace;
        className = classDef->next()->str();
    } else {
        type = Scope::eFunction;
        className = classDef->str();
    }
}

bool Scope::hasDefaultConstructor() const
{
    if (numConstructors) {
        std::list<Function>::const_iterator func;

        for (func = functionList.begin(); func != functionList.end(); ++func) {
            if (func->type == Function::eConstructor && func->argCount() == 0)
                return true;
        }
    }
    return false;
}

AccessControl Scope::defaultAccess() const
{
    switch (type) {
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
    default:
        return Local;
    }
}

// Get variable list..
void Scope::getVariableList()
{
    AccessControl varaccess = defaultAccess();
    const Token *start;
    unsigned int level = 1;

    if (classStart)
        start = classStart->next();
    else
        start = check->_tokenizer->tokens();

    for (const Token *tok = start; tok; tok = tok->next()) {
        // end of scope?
        if (tok->str() == "}") {
            level--;
            if (level == 0)
                break;
        }

        // syntax error?
        else if (tok->next() == NULL)
            break;

        // Is it a function?
        else if (tok->str() == "{") {
            tok = tok->link();
            // syntax error?
            if (!tok)
                return;
            continue;
        }

        // Is it a nested class or structure?
        else if (Token::Match(tok, "class|struct|union|namespace %type% :|{")) {
            tok = tok->tokAt(2);
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (tok) {
                // skip implementation
                tok = tok->link();
                continue;
            } else
                break;
        } else if (Token::Match(tok, "struct|union {") && Token::Match(tok->next()->link(), "} %var% ;|[")) {
            tok = tok->next()->link()->tokAt(2);
            continue;
        } else if (Token::Match(tok, "struct|union {") && Token::simpleMatch(tok->next()->link(), "} ;")) {
            level++;
            tok = tok->next();
            continue;
        }

        // Borland C++: Skip all variables in the __published section.
        // These are automatically initialized.
        else if (tok->str() == "__published:") {
            for (; tok; tok = tok->next()) {
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
        else if (tok->str() == "public:") {
            varaccess = Public;
            continue;
        } else if (tok->str() == "protected:") {
            varaccess = Protected;
            continue;
        } else if (tok->str() == "private:") {
            varaccess = Private;
            continue;
        }

        // Is it a forward declaration?
        else if (Token::Match(tok, "class|struct|union %var% ;")) {
            tok = tok->tokAt(2);
            continue;
        }

        // Borland C++: Ignore properties..
        else if (tok->str() == "__property")
            continue;

        // skip return and delete
        else if (Token::Match(tok, "return|delete")) {
            while (tok->next() && tok->next()->str() != ";")
                tok = tok->next();
            continue;
        }

        // Search for start of statement..
        else if (tok->previous() && !Token::Match(tok->previous(), ";|{|}|public:|protected:|private:"))
            continue;
        else if (Token::Match(tok, ";|{|}"))
            continue;
        else if (Token::Match(tok, "goto %var% ;")) {
            tok = tok->tokAt(2);
            continue;
        }

        tok = checkVariable(tok, varaccess);

        if (!tok)
            break;
    }
}

const Token *Scope::checkVariable(const Token *tok, AccessControl varaccess)
{
    // This is the start of a statement
    const Token *vartok = NULL;
    const Token *typetok = NULL;

    // Is it a throw..?
    if (Token::Match(tok, "throw %any% (") &&
        Token::simpleMatch(tok->linkAt(2), ") ;")) {
        return tok->linkAt(2);
    } else if ((Token::Match(tok, "throw %any% :: %any% (") &&
                Token::simpleMatch(tok->linkAt(4), ") ;"))) {
        return tok->linkAt(4);
    }

    // Is it const..?
    if (tok->str() == "const") {
        tok = tok->next();
    }

    // Is it a static variable?
    if (tok->str() == "static") {
        tok = tok->next();
    }

    // Is it a mutable variable?
    if (tok->str() == "mutable") {
        tok = tok->next();
    }

    // Is it const..?
    if (tok->str() == "const") {
        tok = tok->next();
    }

    // the start of the type tokens does not include the above modifiers
    const Token *typestart = tok;

    if (Token::Match(tok, "struct|union")) {
        tok = tok->next();
    }

    if (tok && isVariableDeclaration(tok, vartok, typetok)) {
        // If the vartok was set in the if-blocks above, create a entry for this variable..
        tok = vartok->next();
        while (tok && tok->str() == "[")
            tok = tok->link()->next();

        if (vartok->varId() == 0 && !vartok->isBoolean())
            check->debugMessage(vartok, "Scope::checkVariable found variable \'" + vartok->str() + "\' with varid 0.");

        const Scope *scope = NULL;

        if (typetok)
            scope = check->findVariableType(this, typetok);

        addVariable(vartok, typestart, vartok->previous(), varaccess, scope, this);
    }

    return tok;
}

const Variable *Scope::getVariable(const std::string &varname) const
{
    std::list<Variable>::const_iterator iter;

    for (iter = varlist.begin(); iter != varlist.end(); ++iter) {
        if (iter->name() == varname)
            return &*iter;
    }

    return NULL;
}

static const Token* skipScopeIdentifiers(const Token* tok)
{
    if (Token::simpleMatch(tok, "::")) {
        tok = tok->next();
    }
    while (Token::Match(tok, "%type% ::")) {
        tok = tok->tokAt(2);
    }

    return tok;
}

static const Token* skipPointers(const Token* tok)
{
    while (Token::Match(tok, "*|&")) {
        tok = tok->next();
    }

    return tok;
}

bool Scope::isVariableDeclaration(const Token* tok, const Token*& vartok, const Token*& typetok) const
{
    if (tok && tok->str() == "throw" && check->_tokenizer->isCPP())
        return false;

    const Token* localTypeTok = skipScopeIdentifiers(tok);
    const Token* localVarTok = NULL;

    if (Token::Match(localTypeTok, "%type% <")) {
        const Token* closeTok = NULL;
        bool found = localTypeTok->next()->findClosingBracket(closeTok);
        if (found) {
            localVarTok = skipPointers(closeTok->next());

            if (Token::Match(localVarTok, ":: %type% %var% ;|=")) {
                localTypeTok = localVarTok->next();
                localVarTok = localVarTok->tokAt(2);
            }
        }
    } else if (Token::Match(localTypeTok, "%type%")) {
        localVarTok = skipPointers(localTypeTok->strAt(1)=="const"?localTypeTok->tokAt(2):localTypeTok->next());
    }

    if (localVarTok && localVarTok->str() == "const")
        localVarTok = localVarTok->next();

    if (Token::Match(localVarTok, "%var% ;|=")) {
        vartok = localVarTok;
        typetok = localTypeTok;
    } else if (Token::Match(localVarTok, "%var% [") && localVarTok->str() != "operator") {
        vartok = localVarTok;
        typetok = localTypeTok;
    } else if ((isLocal() || type == Scope::eFunction) &&
               Token::Match(localVarTok, "%var% (") &&
               Token::simpleMatch(localVarTok->next()->link(), ") ;")) {
        vartok = localVarTok;
        typetok = localTypeTok;
    } else if (type == eCatch &&
               (Token::Match(localTypeTok, "%var% )") ||
                Token::Match(localTypeTok, "%var% &| %var% )"))) {
        vartok = localVarTok;
        typetok = localTypeTok;
    }

    return NULL != vartok;
}



//---------------------------------------------------------------------------

const Scope *SymbolDatabase::findVariableType(const Scope *start, const Token *type) const
{
    std::list<Scope>::const_iterator scope;

    for (scope = scopeList.begin(); scope != scopeList.end(); ++scope) {
        // skip namespaces, functions, ...
        if (scope->type != Scope::eClass && scope->type != Scope::eStruct && scope->type != Scope::eUnion)
            continue;

        // do the names match?
        if (scope->className == type->str()) {
            // check if type does not have a namespace
            if (type->previous() == NULL || type->previous()->str() != "::") {
                const Scope *parent = start;

                // check if in same namespace
                while (parent) {
                    // out of line class function belongs to class
                    if (parent->type == Scope::eFunction && parent->functionOf)
                        parent = parent->functionOf;
                    else if (parent != scope->nestedIn)
                        parent = parent->nestedIn;
                    else
                        break;
                }

                if (scope->nestedIn == parent)
                    return &(*scope);
            }

            // type has a namespace
            else {
                // FIXME check if namespace path matches supplied path
                return &(*scope);
            }
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------

const Scope *SymbolDatabase::findFunctionScopeByToken(const Token *tok) const
{
    std::list<Scope>::const_iterator scope;

    for (scope = scopeList.begin(); scope != scopeList.end(); ++scope) {
        if (scope->type == Scope::eFunction) {
            if (scope->classDef == tok)
                return &(*scope);
        }
    }
    return 0;
}

//---------------------------------------------------------------------------

const Function *SymbolDatabase::findFunctionByToken(const Token *tok) const
{
    std::list<Scope>::const_iterator scope;

    for (scope = scopeList.begin(); scope != scopeList.end(); ++scope) {
        std::list<Function>::const_iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->token == tok)
                return &(*func);
        }
    }
    return 0;
}

//---------------------------------------------------------------------------

const Scope* SymbolDatabase::findScopeByName(const std::string& name) const
{
    for (std::list<Scope>::const_iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        if (it->className == name)
            return &*it;
    }
    return 0;
}

//---------------------------------------------------------------------------

Scope * Scope::findInNestedList(const std::string & name)
{
    std::list<Scope *>::iterator it;

    for (it = nestedList.begin(); it != nestedList.end(); ++it) {
        if ((*it)->className == name)
            return (*it);
    }
    return 0;
}

//---------------------------------------------------------------------------

Scope * Scope::findInNestedListRecursive(const std::string & name)
{
    std::list<Scope *>::iterator it;

    for (it = nestedList.begin(); it != nestedList.end(); ++it) {
        if ((*it)->className == name)
            return (*it);
    }

    for (it = nestedList.begin(); it != nestedList.end(); ++it) {
        Scope *child = (*it)->findInNestedListRecursive(name);
        if (child)
            return child;
    }
    return 0;
}

//---------------------------------------------------------------------------

const Scope * Scope::findQualifiedScope(const std::string & name) const
{
    if (type == Scope::eClass || type == Scope::eStruct || type == Scope::eNamespace) {
        if (name.compare(0, className.size(), className) == 0) {
            std::string path = name;
            path.erase(0, className.size());
            if (path.compare(0, 4, " :: ") == 0)
                path.erase(0, 4);
            else if (path.empty())
                return this;

            std::list<Scope *>::const_iterator it;

            for (it = nestedList.begin() ; it != nestedList.end(); ++it) {
                const Scope *scope1 = (*it)->findQualifiedScope(path);
                if (scope1)
                    return scope1;
            }
        }
    }
    return 0;
}

//---------------------------------------------------------------------------

const Function *Scope::getDestructor() const
{
    std::list<Function>::const_iterator it;
    for (it = functionList.begin(); it != functionList.end(); ++it) {
        if (it->type == Function::eDestructor)
            return &(*it);
    }
    return 0;
}

//---------------------------------------------------------------------------

unsigned int Scope::getNestedNonFunctions() const
{
    unsigned int nested = 0;
    std::list<Scope *>::const_iterator ni;
    for (ni = nestedList.begin(); ni != nestedList.end(); ++ni) {
        if ((*ni)->type != Scope::eFunction)
            nested++;
    }
    return nested;
}
