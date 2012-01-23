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

    // find all scopes
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Locate next class
        if (Token::Match(tok, "class|struct|union|namespace %var% [{:]")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();
            const Token *tok2 = tok->tokAt(2);

            // only create base list for classes and structures
            if (new_scope->isClassOrStruct()) {
                // goto initial '{'
                tok2 = initBaseInfo(new_scope, tok);

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
                 Token::Match(tok->next()->link(), "} |* %var% ;|[")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();

            std::vector<Dimension> dimensions;

            bool isPointer = false;
            bool isArray = false;

            const Token* varNameTok = tok->next()->link()->next();
            if (varNameTok->str() == "*") {
                isPointer = true;
                varNameTok = varNameTok->next();
            }

            if (varNameTok->next()->str() == "[")
                isArray = arrayDimensions(dimensions, varNameTok->next());

            scope->addVariable(varNameTok, tok, tok, scope->access, false, false, false, true, new_scope, scope, isArray, isPointer, dimensions);

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
                    scope->access = Private;
                else if (tok->str() == "protected:")
                    scope->access = Protected;
                else if (tok->str() == "public:")
                    scope->access = Public;
                else if (Token::Match(tok, "public|protected|private %var% :")) {
                    if (tok->str() == "private")
                        scope->access = Private;
                    else if (tok->str() == "protected")
                        scope->access = Protected;
                    else if (tok->str() == "public")
                        scope->access = Public;

                    tok = tok->tokAt(2);
                }

                // class function?
                else if (tok->previous()->str() != "::" && isFunction(tok, &funcStart, &argStart)) {
                    Function function;

                    // save the function definition argument start '('
                    function.argDef = argStart;

                    // save the access type
                    function.access = scope->access;

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

                        // save start of function
                        function.start = end;

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
                else if (tok->previous()->str() == "::" && isFunction(tok, &funcStart, &argStart)) {
                    /** @todo check entire qualification for match */
                    Scope * nested = scope->findInNestedListRecursive(tok->strAt(-2));

                    if (nested)
                        addFunction(&scope, &tok, argStart);
                    else {
                        /** @todo handle friend functions */
                    }
                }

                // friend class declaration?
                else if (Token::Match(tok, "friend class| %any% ;")) {
                    Scope::FriendInfo friendInfo;

                    friendInfo.name = tok->strAt(1) == "class" ? tok->strAt(2) : tok->strAt(1);
                    /** @todo fill this in later after parsing is complete */
                    friendInfo.scope = 0;

                    scope->friendList.push_back(friendInfo);
                }
            } else if (scope->type == Scope::eNamespace || scope->type == Scope::eGlobal) {
                const Token *funcStart = 0;
                const Token *argStart = 0;

                // function?
                if (isFunction(tok, &funcStart, &argStart)) {
                    // has body?
                    if (Token::Match(argStart->link(), ") const| {|:")) {
                        Scope *old_scope = scope;

                        // class function
                        if (tok->previous() && tok->previous()->str() == "::")
                            addFunction(&scope, &tok, argStart);

                        // class destructor
                        else if (tok->previous() && tok->previous()->str() == "~" &&
                                 tok->tokAt(-2) && tok->strAt(-2) == "::")
                            addFunction(&scope, &tok, argStart);

                        // regular function
                        else {
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

                            // find start of function '{'
                            const Token *start = tok;
                            while (start && start->str() != "{")
                                start = start->next();

                            // save start of function
                            function.start = start;

                            addNewFunction(&scope, &tok);

                            if (scope) {
                                old_scope->functionList.push_back(function);
                                scope->function = &old_scope->functionList.back();
                            }
                        }

                        // syntax error
                        if (!scope) {
                            scope = old_scope;
                            break;
                        }
                    }

                    // function returning function pointer with body
                    else if (Token::simpleMatch(argStart->link(), ") ) (") &&
                             Token::Match(argStart->link()->linkAt(2), ") const| {")) {
                        const Token *tok1 = funcStart;
                        Scope *old_scope = scope;

                        // class function
                        if (tok1->previous()->str() == "::")
                            addFunction(&scope, &tok1, argStart);

                        // regular function
                        else {
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

                            // find start of function '{'
                            const Token *start = tok;
                            while (start && start->str() != "{")
                                start = start->next();

                            // save start of function
                            function.start = start;

                            addNewFunction(&scope, &tok1);

                            if (scope) {
                                old_scope->functionList.push_back(function);
                                scope->function = &old_scope->functionList.back();
                            }
                        }

                        // syntax error?
                        if (!scope) {
                            scope = old_scope;
                            break;
                        }

                        tok = tok1;
                    }

                    // function prototype
                    else if (Token::simpleMatch(argStart->link(), ") ;")) {
                        /** @todo save function prototypes in database someday */
                        tok = argStart->link()->next();
                        continue;
                    }

                    // function returning function pointer prototype
                    else if (Token::simpleMatch(argStart->link(), ") ) (") &&
                             Token::simpleMatch(argStart->link()->linkAt(2), ") ;")) {
                        /** @todo save function prototypes in database someday */
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

    // fill in variable info
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        scope = &(*it);

        // find variables
        scope->getVariableList();
    }

    // fill in function arguments
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        scope = &(*it);

        std::list<Function>::iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            // add arguments
            func->addArguments(this, &*func, scope);
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

bool SymbolDatabase::isFunction(const Token *tok, const Token **funcStart, const Token **argStart) const
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
    else if (Token::Match(tok, "%var% (") &&
             (Token::Match(tok->next()->link(), ") const| ;|{|=") ||
              Token::Match(tok->next()->link(), ") : %var% (|::"))) {
        *funcStart = tok;
        *argStart = tok->next();
        return true;
    }

    return false;
}

bool SymbolDatabase::argsMatch(const Scope *scope, const Token *first, const Token *second, const std::string &path, unsigned int depth) const
{
    bool match = false;
    while (first->str() == second->str()) {
        // at end of argument list
        if (first->str() == ")") {
            match = true;
            break;
        }

        // skip default value assignment
        else if (first->next()->str() == "=") {
            first = first->tokAt(2);

            if (second->next()->str() == "=")
                second = second->tokAt(2);
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

            // skip default value assignment
            if (first->next()->str() == "=")
                first = first->tokAt(2);
        }

        // variable with class path
        else if (depth && Token::Match(first->next(), "%var%")) {
            std::string param = path + first->next()->str();

            if (Token::Match(second->next(), param.c_str())) {
                second = second->tokAt(int(depth) * 2);
            } else if (depth > 1) {
                std::string    short_path = path;

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

    return match;
}

void SymbolDatabase::addFunction(Scope **scope, const Token **tok, const Token *argStart)
{
    int count = 0;
    bool added = false;
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
                    if (func->type == Function::eDestructor && destructor) {
                        if (argsMatch(scope1, func->tokenDef->next(), (*tok)->next(), path, path_length)) {
                            func->hasBody = true;
                            func->token = *tok;
                            func->arg = argStart;
                            const Token *start = argStart->link()->next();
                            while (start && start->str() != "{")
                                start = start->next();
                            func->start = start;
                        }
                    } else if (func->type != Function::eDestructor && !destructor) {
                        if (argsMatch(scope1, func->tokenDef->next(), (*tok)->next(), path, path_length)) {
                            // normal function?
                            if (!func->retFuncPtr && (*tok)->next()->link()) {
                                if ((func->isConst && (*tok)->next()->link()->next()->str() == "const") ||
                                    (!func->isConst && (*tok)->next()->link()->next()->str() != "const")) {
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
                            else if (func->retFuncPtr) {
                                // todo check for const
                                func->hasBody = true;
                                func->token = *tok;
                                func->arg = argStart;
                                const Token *start = argStart->link()->linkAt(2)->next();
                                while (start && start->str() != "{")
                                    start = start->next();
                                func->start = start;
                            }
                        }
                    }

                    if (func->hasBody) {
                        addNewFunction(scope, tok);
                        if (*scope) {
                            (*scope)->functionOf = scope1;
                            (*scope)->function = &*func;
                            (*scope)->function->functionScope = *scope;

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

const Token *SymbolDatabase::initBaseInfo(Scope *scope, const Token *tok)
{
    // goto initial '{'
    const Token *tok2 = tok->tokAt(2);
    int level = 0;
    while (tok2 && tok2->str() != "{") {
        // skip unsupported templates
        if (tok2->str() == "<")
            level++;
        else if (tok2->str() == ">")
            level--;

        // check for base classes
        else if (level == 0 && Token::Match(tok2, ":|,")) {
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
            scope->derivedFrom.push_back(base);
        }
        tok2 = tok2->next();
    }

    return tok2;
}

void SymbolDatabase::debugMessage(const Token *tok, const std::string &msg) const
{
    if (tok && _settings->debugwarnings) {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(_tokenizer->file(tok));
        locationList.push_back(loc);

        const ErrorLogger::ErrorMessage errmsg(locationList,
                                               Severity::debug,
                                               msg,
                                               "debug",
                                               false);
        if (_errorLogger)
            _errorLogger->reportErr(errmsg);
        else
            Check::reportError(errmsg);
    }
}

bool SymbolDatabase::arrayDimensions(std::vector<Dimension> &dimensions, const Token *tok) const
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
          type == Scope::eUnconditional ? "Unconditional" :
          "Unknown");
    return s;
}

void SymbolDatabase::printVariable(const Variable *var, const char *indent) const
{
    std::cout << indent << "_name: " << var->nameToken();
    if (var->nameToken()) {
        std::cout << " " << var->name() << " " << _tokenizer->fileLine(var->nameToken()) << std::endl;
        std::cout << indent << "    varId: " << var->varId() << std::endl;
    } else
        std::cout << std::endl;
    std::cout << indent << "_start: " << var->typeStartToken() << " " << var->typeStartToken()->str()
              << " " << _tokenizer->fileLine(var->typeStartToken()) << std::endl;;
    std::cout << indent << "_end: " << var->typeEndToken() << " " << var->typeEndToken()->str()
              << " " << _tokenizer->fileLine(var->typeEndToken()) << std::endl;;
    std::cout << indent << "_index: " << var->index() << std::endl;
    std::cout << indent << "_access: " <<
              (var->isPublic() ? "Public" :
               var->isProtected() ? "Protected" :
               var->isPrivate() ? "Private" :
               var->isGlobal() ? "Global" :
               var->isNamespace() ? "Namespace" :
               var->isArgument() ? "Argument" :
               var->isLocal() ? "Local" :
               "???")  << std::endl;
    std::cout << indent << "_flags: " << std::endl;
    std::cout << indent << "    isMutable: " << (var->isMutable() ? "true" : "false") << std::endl;
    std::cout << indent << "    isStatic: " << (var->isStatic() ? "true" : "false") << std::endl;
    std::cout << indent << "    isConst: " << (var->isConst() ? "true" : "false") << std::endl;
    std::cout << indent << "    isClass: " << (var->isClass() ? "true" : "false") << std::endl;
    std::cout << indent << "    isArray: " << (var->isArray() ? "true" : "false") << std::endl;
    std::cout << indent << "    hasDefault: " << (var->hasDefault() ? "true" : "false") << std::endl;
    std::cout << indent << "_type: ";
    if (var->type()) {
        std::cout << var->type()->className << " " << var->type()->type << " "
                  << _tokenizer->fileLine(var->type()->classDef) << std::endl;
    } else
        std::cout << "none" << std::endl;

    std::cout << indent << "_scope: ";
    if (var->scope()) {
        std::cout << var->scope()->className << " " << var->scope()->type;
        if (var->scope()->classDef)
            std::cout << " " << _tokenizer->fileLine(var->scope()->classDef) << std::endl;
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
            std::cout << " " << scope->classDef->str() << " " << _tokenizer->fileLine(scope->classDef) << std::endl;
        else
            std::cout << std::endl;

        std::cout << "    classStart: " << scope->classStart;
        if (scope->classStart)
            std::cout << " " << scope->classStart->str() << " " << _tokenizer->fileLine(scope->classStart) << std::endl;
        else
            std::cout << std::endl;

        std::cout << "    classEnd: " << scope->classEnd;
        if (scope->classEnd)
            std::cout << " " << scope->classEnd->str() << " " <<  _tokenizer->fileLine(scope->classEnd) << std::endl;
        else
            std::cout << std::endl;

        std::list<Function>::const_iterator func;

        // find the function body if not implemented inline
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            std::cout << "    Function: " << &*func << std::endl;
            std::cout << "        name: " << func->tokenDef->str() << " "
                      << _tokenizer->fileLine(func->tokenDef) << std::endl;
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
            std::cout << "        tokenDef: " << _tokenizer->fileLine(func->tokenDef) << std::endl;
            std::cout << "        argDef: " << _tokenizer->fileLine(func->argDef) << std::endl;
            std::cout << "        token: " << _tokenizer->fileLine(func->token) << std::endl;
            std::cout << "        arg: " << _tokenizer->fileLine(func->arg) << std::endl;
            std::cout << "        functionScope: ";
            if (func->functionScope) {
                std::cout << func->functionScope->className << " "
                          <<  _tokenizer->fileLine(func->functionScope->classDef) << std::endl;
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
            std::cout << " " << _tokenizer->fileLine(*use) << std::endl;
        }

        std::cout << "    functionOf: " << scope->functionOf;
        if (scope->functionOf) {
            std::cout << " " << scope->functionOf->type << " " << scope->functionOf->className;
            if (scope->functionOf->classDef)
                std::cout << " " << _tokenizer->fileLine(scope->functionOf->classDef);
        }
        std::cout << std::endl;

        std::cout << "    function: " << scope->function;
        if (scope->function) {
            std::cout << " " << scope->function->tokenDef->str() << " "
                      << _tokenizer->fileLine(scope->function->tokenDef);
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

void Function::addArguments(const SymbolDatabase *symbolDatabase, const Function *func, const Scope *scope)
{
    // check for non-empty argument list "( ... )"
    if (arg->link() != arg->next() && !Token::simpleMatch(arg, "( void )")) {
        unsigned int count = 0;

        for (const Token* tok = arg->next(); tok; tok = tok->next()) {
            const Token* startTok = tok;
            const Token* endTok   = NULL;
            const Token* nameTok  = NULL;
            bool isConstVar   = bool(tok->str() == "const");
            bool isArrayVar   = false;
            bool hasDefault   = false;
            std::vector<Dimension> dimensions;

            while (tok->str() != "," && tok->str() != ")" && tok->str() != "=") {
                if (tok->varId() != 0) {
                    nameTok = tok;
                    endTok = tok->previous();
                } else if (tok->str() == "[") {
                    isArrayVar = symbolDatabase->arrayDimensions(dimensions, tok);

                    // skip array dimension(s)
                    tok = tok->link();
                    while (tok->next()->str() == "[")
                        tok = tok->next()->link();
                } else if (tok->str() == "<") {
                    int level = 1;
                    while (tok && tok->next()) {
                        tok = tok->next();
                        if (tok->str() == ">") {
                            --level;
                            if (level == 0)
                                break;
                        } else if (tok->str() == "<")
                            level++;
                    }
                    if (!tok) // something is wrong so just bail
                        return;
                }

                tok = tok->next();

                if (!tok) // something is wrong so just bail
                    return;
            }

            // check for argument with no name or missing varid
            if (!endTok) {
                if (tok->previous()->isName()) {
                    if (tok->previous() != startTok->tokAt(isConstVar ? 1 : 0)) {
                        nameTok = tok->previous();
                        endTok = nameTok->previous();

                        if (func->hasBody)
                            symbolDatabase->debugMessage(nameTok, "Function::addArguments found argument \'" + nameTok->str() + "\' with varid 0.");
                    } else
                        endTok = startTok;
                } else
                    endTok = tok->previous();
            }

            const Token *typeTok = startTok;
            if (isConstVar)
                typeTok = typeTok->next();

            const Scope *argType = NULL;
            if (!typeTok->isStandardType())
                argType = symbolDatabase->findVariableType(scope, typeTok);

            bool isClassVar = startTok == endTok && !startTok->isStandardType();
            bool isPointerVar = nameTok->strAt(-1) == "*" || nameTok->strAt(-2) == "*";

            // skip default values
            if (tok->str() == "=") {
                hasDefault = true;

                while (tok->str() != "," && tok->str() != ")")
                    tok = tok->next();
            }

            argumentList.push_back(Variable(nameTok, startTok, endTok, count++, Argument, false, false, isConstVar, isClassVar, argType, functionScope, isArrayVar, isPointerVar, hasDefault, dimensions));

            if (tok->str() == ")")
                break;
        }
    }
}

//---------------------------------------------------------------------------

Scope::Scope(SymbolDatabase *check_, const Token *classDef_, Scope *nestedIn_, ScopeType type_, const Token *start_) :
    check(check_),
    type(type_),
    classDef(classDef_),
    classStart(start_),
    classEnd(start_->link()),
    nestedIn(nestedIn_),
    access(Public),
    numConstructors(0),
    needInitialization(Scope::Unknown),
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
        access = Public;
    } else if (classDef->str() == "class") {
        type = Scope::eClass;
        className = classDef->next()->str();
        access = Private;
    } else if (classDef->str() == "struct") {
        type = Scope::eStruct;
        // anonymous and unnamed structs don't have a name
        if (classDef->next()->str() != "{")
            className = classDef->next()->str();
        access = Public;
    } else if (classDef->str() == "union") {
        type = Scope::eUnion;
        // anonymous and unnamed unions don't have a name
        if (classDef->next()->str() != "{")
            className = classDef->next()->str();
        access = Public;
    } else if (classDef->str() == "namespace") {
        type = Scope::eNamespace;
        className = classDef->next()->str();
        access = Public;
    } else {
        type = Scope::eFunction;
        className = classDef->str();
        access = Public;
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

    return Public;
}

// Get variable list..
void Scope::getVariableList()
{
    AccessControl varaccess = defaultAccess();
    const Token *start;
    int level = 1;

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

    // Is it const..?
    bool isConst = false;
    if (tok->str() == "const") {
        tok = tok->next();
        isConst = true;
    }

    // Is it a static variable?
    const bool isStatic(Token::simpleMatch(tok, "static"));
    if (isStatic) {
        tok = tok->next();
    }

    // Is it a mutable variable?
    const bool isMutable(Token::simpleMatch(tok, "mutable"));
    if (isMutable) {
        tok = tok->next();
    }

    // Is it const..?
    if (tok->str() == "const") {
        tok = tok->next();
        isConst = true;
    }

    // the start of the type tokens does not include the above modifiers
    const Token *typestart = tok;

    bool isClass = false;

    if (Token::Match(tok, "struct|union")) {
        tok = tok->next();
    }

    bool isArray = false;
    bool isPointer = false;
    std::vector<Dimension> dimensions;

    if (tok && isVariableDeclaration(tok, vartok, typetok, isArray, isPointer)) {
        isPointer = vartok->previous()->str() == "*" || Token::simpleMatch(vartok->tokAt(-2), "* const");
        isClass = (!typetok->isStandardType() && !isPointer && vartok->previous()->str() != "&");
        if (isArray) {
            isArray = check->arrayDimensions(dimensions, vartok->next());
            tok = vartok->next();
            while (tok && tok->str() == "[")
                tok = tok->link()->next();
        } else
            tok = vartok->next();
    }

    // If the vartok was set in the if-blocks above, create a entry for this variable..
    if (vartok && vartok->str() != "operator") {
        if (vartok->varId() == 0 && !vartok->isBoolean())
            check->debugMessage(vartok, "Scope::checkVariable found variable \'" + vartok->str() + "\' with varid 0.");

        const Scope *scope = NULL;

        if (typetok)
            scope = check->findVariableType(this, typetok);

        addVariable(vartok, typestart, vartok->previous(), varaccess, isMutable, isStatic, isConst, isClass, scope, this, isArray, isPointer, dimensions);
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

inline const Token* skipScopeIdentifiers(const Token* tok)
{
    const Token* ret = tok;

    if (Token::simpleMatch(ret, "::")) {
        ret = ret->next();
    }
    while (Token::Match(ret, "%type% ::")) {
        ret = ret->tokAt(2);
    }

    return ret;
}

inline const Token* skipPointers(const Token* tok)
{
    const Token* ret = tok;

    while (Token::Match(ret, "*|&")) {
        ret = ret->next();
    }

    return ret;
}

bool Scope::isVariableDeclaration(const Token* tok, const Token*& vartok, const Token*& typetok, bool &isArray, bool &isPointer) const
{
    const Token* localTypeTok = skipScopeIdentifiers(tok);
    const Token* localVarTok = NULL;

    if (Token::Match(localTypeTok, "%type% <")) {
        const Token* closeTok = NULL;
        bool found = findClosingBracket(localTypeTok->next(), closeTok);
        if (found) {
            localVarTok = skipPointers(closeTok->next());

            if (Token::Match(localVarTok, ":: %type% %var% ;|=")) {
                localTypeTok = localVarTok->next();
                localVarTok = localVarTok->tokAt(2);
            }
        }
    } else if (Token::Match(localTypeTok, "%type%")) {
        localVarTok = skipPointers(localTypeTok->next());
    }

    if (isSimpleVariable(localVarTok)) {
        vartok = localVarTok;
        typetok = localTypeTok;
        isArray = false;
    } else if (isArrayVariable(localVarTok)) {
        vartok = localVarTok;
        typetok = localTypeTok;
        isArray = true;
    } else if ((isLocal() || type == Scope::eFunction) &&
               Token::Match(localVarTok, "%var% (") &&
               Token::simpleMatch(localVarTok->next()->link(), ") ;")) {
        vartok = localVarTok;
        typetok = localTypeTok;
        isArray = false;
    }
    isPointer = vartok && (vartok->strAt(-1) == "*" || Token::simpleMatch(vartok->tokAt(-2), "* const"));

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
    if (NULL != tok && tok->str() == "<") {
        unsigned int depth = 0;
        for (close = tok; (close != NULL) && (close->str() != ";") && (close->str() != "="); close = close->next()) {
            if (close->str() == "<") {
                ++depth;
            } else if (close->str() == ">") {
                if (--depth == 0) {
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
    std::list<Scope>::const_iterator scope;

    for (scope = scopeList.begin(); scope != scopeList.end(); ++scope) {
        // skip namespaces and functions
        if (scope->type == Scope::eNamespace || scope->type == Scope::eFunction || scope->type == Scope::eGlobal)
            continue;

        // do the names match?
        if (scope->className == type->str()) {
            // check if type does not have a namespace
            if (type->previous() != NULL && type->previous()->str() != "::") {
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
