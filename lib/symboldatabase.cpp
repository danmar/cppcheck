/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "utils.h"

#include <string>
#include <ostream>
#include <climits>
#include <iostream>
#include <iomanip>
#include <cctype>

//---------------------------------------------------------------------------

SymbolDatabase::SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : _tokenizer(tokenizer), _settings(settings), _errorLogger(errorLogger)
{
    cpp = isCPP();

    if (_settings->defaultSign == 's' || _settings->defaultSign == 'S')
        defaultSignedness = ValueType::SIGNED;
    else if (_settings->defaultSign == 'u' || _settings->defaultSign == 'U')
        defaultSignedness = ValueType::UNSIGNED;
    else
        defaultSignedness = ValueType::UNKNOWN_SIGN;

    createSymbolDatabaseFindAllScopes();
    createSymbolDatabaseClassInfo();
    createSymbolDatabaseVariableInfo();
    createSymbolDatabaseFunctionScopes();
    createSymbolDatabaseClassAndStructScopes();
    createSymbolDatabaseFunctionReturnTypes();
    createSymbolDatabaseNeedInitialization();
    createSymbolDatabaseVariableSymbolTable();
    createSymbolDatabaseSetScopePointers();
    createSymbolDatabaseSetFunctionPointers(true);
    createSymbolDatabaseSetVariablePointers();
    createSymbolDatabaseSetTypePointers();
    createSymbolDatabaseEnums();
    createSymbolDatabaseUnknownArrayDimensions();
}

void SymbolDatabase::createSymbolDatabaseFindAllScopes()
{
    // create global scope
    scopeList.push_back(Scope(this, nullptr, nullptr));

    // pointer to current scope
    Scope *scope = &scopeList.back();

    // Store current access in each scope (depends on evaluation progress)
    std::map<const Scope*, AccessControl> access;

    // find all scopes
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok ? tok->next() : nullptr) {
        // #5593 suggested to add here:
        if (_errorLogger)
            _errorLogger->reportProgress(_tokenizer->list.getSourceFilePath(),
                                         "SymbolDatabase",
                                         tok->progressValue());
        // Locate next class
        if ((_tokenizer->isCPP() && ((Token::Match(tok, "class|struct|union|namespace ::| %name% {|:|::|<") && tok->strAt(-1) != "friend") ||
                                     (Token::Match(tok, "enum class| %name% {") || Token::Match(tok, "enum class| %name% : %name% {"))))
            || (_tokenizer->isC() && Token::Match(tok, "struct|union|enum %name% {"))) {
            const Token *tok2 = tok->tokAt(2);

            if (tok->strAt(1) == "::")
                tok2 = tok2->next();
            else if (_tokenizer->isCPP() && tok->strAt(1) == "class")
                tok2 = tok2->next();

            while (tok2 && tok2->str() == "::")
                tok2 = tok2->tokAt(2);

            // skip over template args
            if (tok2 && tok2->str() == "<" && tok2->link())
                tok2 = tok2->link()->next();

            // make sure we have valid code
            if (!tok2 || !Token::Match(tok2, "{|:")) {
                // check for qualified variable
                if (tok2 && tok2->next()) {
                    if (tok2->next()->str() == ";")
                        tok = tok2->next();
                    else if (Token::simpleMatch(tok2->next(), "= {") &&
                             Token::simpleMatch(tok2->linkAt(2), "} ;"))
                        tok = tok2->linkAt(2)->next();
                    else if (Token::Match(tok2->next(), "(|{") &&
                             tok2->next()->link()->strAt(1) == ";")
                        tok = tok2->next()->link()->next();
                    // skip variable declaration
                    else if (Token::Match(tok2, "*|&|>"))
                        continue;
                    else
                        break; // bail
                    continue;
                }
                break; // bail
            }

            Scope *new_scope = findScope(tok->next(), scope);

            if (new_scope) {
                // only create base list for classes and structures
                if (new_scope->isClassOrStruct()) {
                    // goto initial '{'
                    if (!new_scope->definedType) {
                        _tokenizer->syntaxError(nullptr); // #6808
                    }
                    tok2 = new_scope->definedType->initBaseInfo(tok, tok2);

                    // make sure we have valid code
                    if (!tok2) {
                        break;
                    }
                }

                // definition may be different than declaration
                if (_tokenizer->isCPP() && tok->str() == "class") {
                    access[new_scope] = Private;
                    new_scope->type = Scope::eClass;
                } else if (tok->str() == "struct") {
                    access[new_scope] = Public;
                    new_scope->type = Scope::eStruct;
                }

                new_scope->classDef = tok;
                new_scope->classStart = tok2;
                new_scope->classEnd = tok2->link();
                scope = new_scope;
                tok = tok2;
            } else {
                scopeList.push_back(Scope(this, tok, scope));
                new_scope = &scopeList.back();

                if (tok->str() == "class")
                    access[new_scope] = Private;
                else if (tok->str() == "struct")
                    access[new_scope] = Public;

                // fill typeList...
                if (new_scope->isClassOrStruct() || new_scope->type == Scope::eUnion || new_scope->type == Scope::eEnum) {
                    Type* new_type = findType(tok->next(), scope);
                    if (!new_type) {
                        typeList.push_back(Type(new_scope->classDef, new_scope, scope));
                        new_type = &typeList.back();
                        scope->definedTypes.push_back(new_type);
                    } else
                        new_type->classScope = new_scope;
                    new_scope->definedType = new_type;
                }

                // only create base list for classes and structures
                if (new_scope->isClassOrStruct()) {
                    // goto initial '{'
                    tok2 = new_scope->definedType->initBaseInfo(tok, tok2);

                    // make sure we have valid code
                    if (!tok2) {
                        _tokenizer->syntaxError(tok);
                    }
                } else if (new_scope->type == Scope::eEnum) {
                    if (tok2->str() == ":")
                        tok2 = tok2->tokAt(2);
                }

                new_scope->classStart = tok2;
                new_scope->classEnd = tok2->link();

                // make sure we have valid code
                if (!new_scope->classEnd) {
                    _tokenizer->syntaxError(tok);
                }

                if (new_scope->type == Scope::eEnum) {
                    tok2 = new_scope->addEnum(tok, _tokenizer->isCPP());
                    scope->nestedList.push_back(new_scope);

                    if (!tok2)
                        _tokenizer->syntaxError(tok);
                } else {
                    // make the new scope the current scope
                    scope->nestedList.push_back(new_scope);
                    scope = new_scope;
                }

                tok = tok2;
            }
        }

        // Namespace and unknown macro (#3854)
        else if (_tokenizer->isCPP() &&
                 Token::Match(tok, "namespace %name% %type% (") &&
                 tok->tokAt(2)->isUpperCaseName() &&
                 Token::simpleMatch(tok->linkAt(3), ") {")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();
            access[new_scope] = Public;

            const Token *tok2 = tok->linkAt(3)->next();

            new_scope->classStart = tok2;
            new_scope->classEnd = tok2->link();

            // make sure we have valid code
            if (!new_scope->classEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope->nestedList.push_back(new_scope);
            scope = &scopeList.back();

            tok = tok2;
        }

        // forward declaration
        else if (Token::Match(tok, "class|struct|union %name% ;") &&
                 tok->strAt(-1) != "friend") {
            if (!findType(tok->next(), scope)) {
                // fill typeList..
                typeList.push_back(Type(tok, 0, scope));
                scope->definedTypes.push_back(&typeList.back());
            }
            tok = tok->tokAt(2);
        }

        // using namespace
        else if (_tokenizer->isCPP() && Token::Match(tok, "using namespace ::| %type% ;|::")) {
            Scope::UsingInfo using_info;

            using_info.start = tok; // save location
            using_info.scope = findNamespace(tok->tokAt(2), scope);

            scope->usingList.push_back(using_info);

            // check for global namespace
            if (tok->strAt(2) == "::")
                tok = tok->tokAt(4);
            else
                tok = tok->tokAt(3);

            // skip over qualification
            while (Token::Match(tok, "%type% ::"))
                tok = tok->tokAt(2);
        }

        // unnamed struct and union
        else if (Token::Match(tok, "struct|union {") &&
                 Token::Match(tok->next()->link(), "} *|&| %name% ;|[")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();
            access[new_scope] = Public;

            const Token* varNameTok = tok->next()->link()->next();
            if (varNameTok->str() == "*") {
                varNameTok = varNameTok->next();
            } else if (varNameTok->str() == "&") {
                varNameTok = varNameTok->next();
            }

            typeList.push_back(Type(tok, new_scope, scope));
            new_scope->definedType = &typeList.back();
            scope->definedTypes.push_back(&typeList.back());

            scope->addVariable(varNameTok, tok, tok, access[scope], new_scope->definedType, scope, &_settings->library);

            const Token *tok2 = tok->next();

            new_scope->classStart = tok2;
            new_scope->classEnd = tok2->link();

            // make sure we have valid code
            if (!new_scope->classEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope->nestedList.push_back(new_scope);
            scope = new_scope;

            tok = tok2;
        }

        // anonymous struct, union and namespace
        else if ((Token::Match(tok, "struct|union {") &&
                  Token::simpleMatch(tok->next()->link(), "} ;")) ||
                 Token::simpleMatch(tok, "namespace {")) {
            scopeList.push_back(Scope(this, tok, scope));

            Scope *new_scope = &scopeList.back();
            access[new_scope] = Public;

            const Token *tok2 = tok->next();

            new_scope->classStart = tok2;
            new_scope->classEnd = tok2->link();

            typeList.push_back(Type(tok, new_scope, scope));
            new_scope->definedType = &typeList.back();
            scope->definedTypes.push_back(&typeList.back());

            // make sure we have valid code
            if (!new_scope->classEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope->nestedList.push_back(new_scope);
            scope = new_scope;

            tok = tok2;
        }

        // forward declared enum
        else if (Token::Match(tok, "enum class| %name% ;") || Token::Match(tok, "enum class| %name% : %name% ;")) {
            typeList.push_back(Type(tok, 0, scope));
            scope->definedTypes.push_back(&typeList.back());
            tok = tok->tokAt(2);
        }

        // check for end of scope
        else if (tok == scope->classEnd) {
            access.erase(scope);
            scope = const_cast<Scope*>(scope->nestedIn);
            continue;
        }

        // check if in class or structure
        else if (scope->type == Scope::eClass || scope->type == Scope::eStruct) {
            const Token *funcStart = nullptr;
            const Token *argStart = nullptr;
            const Token *declEnd = nullptr;

            // What section are we in..
            if (tok->str() == "private:")
                access[scope] = Private;
            else if (tok->str() == "protected:")
                access[scope] = Protected;
            else if (tok->str() == "public:" || tok->str() == "__published:")
                access[scope] = Public;
            else if (Token::Match(tok, "public|protected|private %name% :")) {
                if (tok->str() == "private")
                    access[scope] = Private;
                else if (tok->str() == "protected")
                    access[scope] = Protected;
                else
                    access[scope] = Public;

                tok = tok->tokAt(2);
            }

            // class function?
            else if (isFunction(tok, scope, &funcStart, &argStart, &declEnd)) {
                if (tok->previous()->str() != "::" || tok->strAt(-2) == scope->className) {
                    Function function;

                    // save the function definition argument start '('
                    function.argDef = argStart;

                    // save the access type
                    function.access = access[scope];

                    // save the function name location
                    function.tokenDef = funcStart;

                    // save the function parent scope
                    function.nestedIn = scope;

                    // operator function
                    if (function.tokenDef->isOperatorKeyword()) {
                        function.isOperator(true);

                        // 'operator =' is special
                        if (function.tokenDef->str() == "operator=")
                            function.type = Function::eOperatorEqual;
                    }

                    // class constructor/destructor
                    else if (function.tokenDef->str() == scope->className) {
                        // destructor
                        if (function.tokenDef->previous()->str() == "~")
                            function.type = Function::eDestructor;

                        // copy/move constructor?
                        else if (Token::Match(function.tokenDef, "%name% ( const| %name% &|&& &| %name%| )") ||
                                 Token::Match(function.tokenDef, "%name% ( const| %name% <")) {
                            const Token* typeTok = function.tokenDef->tokAt(2);
                            if (typeTok->str() == "const")
                                typeTok = typeTok->next();
                            if (typeTok->strAt(1) == "<") { // TODO: Remove this branch (#4710)
                                if (Token::Match(typeTok->linkAt(1), "> & %name%| )"))
                                    function.type = Function::eCopyConstructor;
                                else if (Token::Match(typeTok->linkAt(1), "> &&|& & %name%| )"))
                                    function.type = Function::eMoveConstructor;
                                else
                                    function.type = Function::eConstructor;
                            } else if (typeTok->strAt(1) == "&&" || typeTok->strAt(2) == "&")
                                function.type = Function::eMoveConstructor;
                            else
                                function.type = Function::eCopyConstructor;

                            if (typeTok->str() != function.tokenDef->str())
                                function.type = Function::eConstructor; // Overwrite, if types are not identical
                        }
                        // regular constructor
                        else
                            function.type = Function::eConstructor;

                        if (function.tokenDef->previous()->str() == "explicit")
                            function.isExplicit(true);
                    }

                    const Token *tok1 = tok;

                    // look for end of previous statement
                    while (tok1->previous() && !Token::Match(tok1->previous(), ";|}|{|public:|protected:|private:")) {
                        // virtual function
                        const Token* tok2 = tok1->previous();

                        if (tok2->str() == "virtual") {
                            function.isVirtual(true);
                            break;
                        }

                        // static function
                        else if (tok2->str() == "static") {
                            function.isStatic(true);
                            break;
                        }

                        // friend function
                        else if (tok2->str() == "friend") {
                            function.isFriend(true);
                            break;
                        }

                        // Function template
                        else if (tok2->link() && tok2->str() == ">" && Token::simpleMatch(tok2->link()->previous(), "template <"))
                            break;

                        tok1 = tok2;
                    }

                    // find the return type
                    if (!function.isConstructor() && !function.isDestructor()) {
                        if (argStart->link()->strAt(1) == ".") // Trailing return type
                            function.retDef = argStart->link()->tokAt(2);
                        else {
                            while (tok1 && Token::Match(tok1->next(), "virtual|static|friend|const|struct|union|enum"))
                                tok1 = tok1->next();

                            if (tok1)
                                function.retDef = tok1;
                        }
                    }

                    const Token *end = function.argDef->link();

                    // const function
                    if (end->next()->str() == "const")
                        function.isConst(true);

                    // count the number of constructors
                    if (function.isConstructor())
                        scope->numConstructors++;
                    if (function.type == Function::eCopyConstructor ||
                        function.type == Function::eMoveConstructor)
                        scope->numCopyOrMoveConstructors++;

                    // assume implementation is inline (definition and implementation same)
                    function.token = function.tokenDef;
                    function.arg = function.argDef;

                    // out of line function
                    if (_tokenizer->isFunctionHead(end, ";")) {
                        // find the function implementation later
                        tok = end->next();

                        if (tok->str() == "const")
                            tok = tok->next();

                        if (tok->str() == "&") {
                            function.hasLvalRefQualifier(true);
                            tok = tok->next();
                        } else if (tok->str() == "&&") {
                            function.hasRvalRefQualifier(true);
                            tok = tok->next();
                        } else if (tok->str() == "noexcept") {
                            function.isNoExcept(!Token::simpleMatch(tok->next(), "( false )"));
                            tok = tok->next();
                            if (tok->str() == "(")
                                tok = tok->link()->next();
                        } else if (Token::simpleMatch(tok, "throw (")) {
                            function.isThrow(true);
                            if (tok->strAt(2) != ")")
                                function.throwArg = end->tokAt(2);
                            tok = tok->linkAt(1)->next();
                        }

                        if (Token::Match(tok, "= %any% ;")) {
                            function.isPure(tok->strAt(1) == "0");
                            function.isDefault(tok->strAt(1) == "default");
                            function.isDelete(tok->strAt(1) == "delete");
                            tok = tok->tokAt(2);
                        }

                        // skip over unknown tokens
                        while (tok && tok->str() != ";")
                            tok = tok->next();

                        scope->addFunction(function);
                    }

                    // inline function
                    else {
                        function.isInline(true);
                        function.hasBody(true);

                        if (Token::Match(end, ") const| noexcept")) {
                            int arg = 2;

                            if (end->strAt(1) == "const")
                                arg++;

                            if (end->strAt(arg) == "(")
                                function.noexceptArg = end->tokAt(arg + 1);

                            function.isNoExcept(true);
                        } else if (Token::Match(end, ") const| throw (")) {
                            int arg = 3;

                            if (end->strAt(1) == "const")
                                arg++;

                            if (end->strAt(arg) != ")")
                                function.throwArg = end->tokAt(arg);

                            function.isThrow(true);
                        } else if (Token::Match(end, ") const| &|&&| [;{]")) {
                            int arg = 1;

                            if (end->strAt(arg) == "const")
                                arg++;

                            if (end->strAt(arg) == "&")
                                function.hasLvalRefQualifier(true);
                            else if (end->strAt(arg) == "&&")
                                function.hasRvalRefQualifier(true);
                        }

                        // find start of function '{'
                        bool foundInitList = false;
                        while (end && end->str() != "{" && end->str() != ";") {
                            if (end->link() && Token::Match(end, "(|<")) {
                                end = end->link();
                            } else if (foundInitList &&
                                       Token::Match(end, "%name%|> {") &&
                                       Token::Match(end->linkAt(1), "} ,|{")) {
                                end = end->linkAt(1);
                            } else {
                                if (end->str() == ":")
                                    foundInitList = true;
                                end = end->next();
                            }
                        }

                        if (!end || end->str() == ";")
                            continue;

                        scope->addFunction(function);

                        Function* funcptr = &scope->functionList.back();
                        const Token *tok2 = funcStart;

                        addNewFunction(&scope, &tok2);
                        if (scope) {
                            scope->functionOf = function.nestedIn;
                            scope->function = funcptr;
                            scope->function->functionScope = scope;
                        }

                        tok = tok2;
                    }
                }

                // nested class or friend function?
                else {
                    /** @todo check entire qualification for match */
                    Scope * nested = scope->findInNestedListRecursive(tok->strAt(-2));

                    if (nested)
                        addClassFunction(&scope, &tok, argStart);
                    else {
                        /** @todo handle friend functions */
                    }
                }
            }

            // friend class declaration?
            else if (_tokenizer->isCPP() && Token::Match(tok, "friend class| ::| %any% ;|::")) {
                Type::FriendInfo friendInfo;

                // save the name start
                friendInfo.nameStart = tok->strAt(1) == "class" ? tok->tokAt(2) : tok->next();
                friendInfo.nameEnd = friendInfo.nameStart;

                // skip leading "::"
                if (friendInfo.nameEnd->str() == "::")
                    friendInfo.nameEnd = friendInfo.nameEnd->next();

                // skip qualification "name ::"
                while (friendInfo.nameEnd && friendInfo.nameEnd->strAt(1) == "::")
                    friendInfo.nameEnd = friendInfo.nameEnd->tokAt(2);

                // save the name
                if (friendInfo.nameEnd)
                    friendInfo.name = friendInfo.nameEnd->str();

                // fill this in after parsing is complete
                friendInfo.type = 0;

                if (!scope->definedType)
                    _tokenizer->syntaxError(tok);

                scope->definedType->friendList.push_back(friendInfo);
            }
        } else if (scope->type == Scope::eNamespace || scope->type == Scope::eGlobal) {
            const Token *funcStart = nullptr;
            const Token *argStart = nullptr;
            const Token *declEnd = nullptr;

            // function?
            if (isFunction(tok, scope, &funcStart, &argStart, &declEnd)) {
                // has body?
                if (declEnd && declEnd->str() == "{") {
                    tok = funcStart;

                    // class function
                    if (tok->previous() && tok->previous()->str() == "::")
                        addClassFunction(&scope, &tok, argStart);

                    // class destructor
                    else if (tok->previous() &&
                             tok->previous()->str() == "~" &&
                             tok->strAt(-2) == "::")
                        addClassFunction(&scope, &tok, argStart);

                    // regular function
                    else {
                        Function* function = addGlobalFunction(scope, tok, argStart, funcStart);

                        if (!function)
                            _tokenizer->syntaxError(tok);

                        // global functions can't be const but we have tests that are
                        if (Token::Match(argStart->link(), ") const| noexcept")) {
                            int arg = 2;

                            if (argStart->link()->strAt(1) == "const")
                                arg++;

                            if (argStart->link()->strAt(arg) == "(")
                                function->noexceptArg = argStart->link()->tokAt(arg + 1);

                            function->isNoExcept(true);
                        } else if (Token::Match(argStart->link(), ") const| throw (")) {
                            int arg = 3;

                            if (argStart->link()->strAt(1) == "const")
                                arg++;

                            if (argStart->link()->strAt(arg) != ")")
                                function->throwArg = argStart->link()->tokAt(arg);

                            function->isThrow(true);
                        }

                        const Token *tok1 = tok->previous();

                        // look for end of previous statement
                        while (tok1 && !Token::Match(tok1, ";|}|{")) {
                            // static function
                            if (tok1->str() == "static") {
                                function->isStaticLocal(true);
                                break;
                            }

                            // extern function
                            else if (tok1->str() == "extern") {
                                function->isExtern(true);
                                break;
                            }

                            tok1 = tok1->previous();
                        }
                    }

                    // syntax error?
                    if (!scope)
                        _tokenizer->syntaxError(tok);
                }
                // function prototype?
                else if (declEnd && declEnd->str() == ";") {
                    bool newFunc = true; // Is this function already in the database?
                    for (std::multimap<std::string, const Function *>::const_iterator i = scope->functionMap.find(tok->str()); i != scope->functionMap.end() && i->first == tok->str(); ++i) {
                        if (Function::argsMatch(scope, i->second->argDef->next(), argStart->next(), emptyString, 0)) {
                            newFunc = false;
                            break;
                        }
                    }

                    // save function prototype in database
                    if (newFunc) {
                        Function* func = addGlobalFunctionDecl(scope, tok, argStart, funcStart);

                        if (Token::Match(argStart->link(), ") const| noexcept")) {
                            int arg = 2;

                            if (argStart->link()->strAt(1) == "const")
                                arg++;

                            if (argStart->link()->strAt(arg) == "(")
                                func->noexceptArg = argStart->link()->tokAt(arg + 1);

                            func->isNoExcept(true);
                        } else if (Token::Match(argStart->link(), ") const| throw (")) {
                            int arg = 3;

                            if (argStart->link()->strAt(1) == "const")
                                arg++;

                            if (argStart->link()->strAt(arg) != ")")
                                func->throwArg = argStart->link()->tokAt(arg);

                            func->isThrow(true);
                        }

                        const Token *tok1 = tok->previous();

                        // look for end of previous statement
                        while (tok1 && !Token::Match(tok1, ";|}|{")) {
                            // extern function
                            if (tok1->str() == "extern") {
                                func->isExtern(true);
                                break;
                            }

                            tok1 = tok1->previous();
                        }
                    }

                    tok = declEnd;
                    continue;
                }
            }
        } else if (scope->isExecutable()) {
            if (Token::Match(tok, "else|try|do {")) {
                const Token* tok1 = tok->next();
                if (tok->str() == "else")
                    scopeList.push_back(Scope(this, tok, scope, Scope::eElse, tok1));
                if (tok->str() == "do")
                    scopeList.push_back(Scope(this, tok, scope, Scope::eDo, tok1));
                else //if (tok->str() == "try")
                    scopeList.push_back(Scope(this, tok, scope, Scope::eTry, tok1));

                tok = tok1;
                scope->nestedList.push_back(&scopeList.back());
                scope = &scopeList.back();
            } else if (Token::Match(tok, "if|for|while|catch|switch (") && Token::simpleMatch(tok->next()->link(), ") {")) {
                const Token *scopeStartTok = tok->next()->link()->next();
                if (tok->str() == "if")
                    scopeList.push_back(Scope(this, tok, scope, Scope::eIf, scopeStartTok));
                else if (tok->str() == "for") {
                    scopeList.push_back(Scope(this, tok, scope, Scope::eFor, scopeStartTok));
                } else if (tok->str() == "while")
                    scopeList.push_back(Scope(this, tok, scope, Scope::eWhile, scopeStartTok));
                else if (tok->str() == "catch") {
                    scopeList.push_back(Scope(this, tok, scope, Scope::eCatch, scopeStartTok));
                } else // if (tok->str() == "switch")
                    scopeList.push_back(Scope(this, tok, scope, Scope::eSwitch, scopeStartTok));

                scope->nestedList.push_back(&scopeList.back());
                scope = &scopeList.back();
                if (scope->type == Scope::eFor)
                    scope->checkVariable(tok->tokAt(2), Local, &_settings->library); // check for variable declaration and add it to new scope if found
                else if (scope->type == Scope::eCatch)
                    scope->checkVariable(tok->tokAt(2), Throw, &_settings->library); // check for variable declaration and add it to new scope if found
                tok = scopeStartTok;
            } else if (tok->str() == "{") {
                if (tok->previous()->varId())
                    tok = tok->link();
                else {
                    const Token* tok2 = tok->previous();
                    while (!Token::Match(tok2, ";|}|{|)"))
                        tok2 = tok2->previous();
                    if (tok2->next() != tok && tok2->strAt(1) != ".")
                        tok2 = nullptr; // No lambda

                    if (tok2 && tok2->str() == ")" && tok2->link()->strAt(-1) == "]") {
                        scopeList.push_back(Scope(this, tok2->link()->linkAt(-1), scope, Scope::eLambda, tok));
                        scope->nestedList.push_back(&scopeList.back());
                        scope = &scopeList.back();
                    } else if (!Token::Match(tok->previous(), "=|,|(|return") && !(tok->strAt(-1) == ")" && Token::Match(tok->linkAt(-1)->previous(), "=|,|(|return"))) {
                        scopeList.push_back(Scope(this, tok, scope, Scope::eUnconditional, tok));
                        scope->nestedList.push_back(&scopeList.back());
                        scope = &scopeList.back();
                    } else {
                        tok = tok->link();
                    }
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseClassInfo()
{
    if (!_tokenizer->isC()) {
        // fill in base class info
        for (std::list<Type>::iterator it = typeList.begin(); it != typeList.end(); ++it) {
            // finish filling in base class info
            for (unsigned int i = 0; i < it->derivedFrom.size(); ++i) {
                const Type* found = findType(it->derivedFrom[i].nameTok, it->enclosingScope);
                if (found && found->findDependency(&(*it))) {
                    // circular dependency
                    //_tokenizer->syntaxError(nullptr);
                } else {
                    it->derivedFrom[i].type = found;
                }
            }
        }

        // fill in friend info
        for (std::list<Type>::iterator it = typeList.begin(); it != typeList.end(); ++it) {
            for (std::list<Type::FriendInfo>::iterator i = it->friendList.begin(); i != it->friendList.end(); ++i) {
                i->type = findType(i->nameStart, it->enclosingScope);
            }
        }

        // fill in using info
        for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
            for (std::list<Scope::UsingInfo>::iterator i = it->usingList.begin(); i != it->usingList.end(); ++i) {
                // only find if not already found
                if (i->scope == nullptr) {
                    // check scope for match
                    Scope *scope = findScope(i->start->tokAt(2), &(*it));
                    if (scope) {
                        // set found scope
                        i->scope = scope;
                        break;
                    }
                }
            }
        }
    }
}


void SymbolDatabase::createSymbolDatabaseVariableInfo()
{
    // fill in variable info
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        // find variables
        it->getVariableList(&_settings->library);
    }

    // fill in function arguments
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        std::list<Function>::iterator func;

        for (func = it->functionList.begin(); func != it->functionList.end(); ++func) {
            // add arguments
            func->addArguments(this, &*it);
        }
    }
}

void SymbolDatabase::createSymbolDatabaseFunctionScopes()
{
    // fill in function scopes
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        if (it->type == Scope::eFunction)
            functionScopes.push_back(&*it);
    }
}

void SymbolDatabase::createSymbolDatabaseClassAndStructScopes()
{
    // fill in class and struct scopes
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        if (it->isClassOrStruct())
            classAndStructScopes.push_back(&*it);
    }
}

void SymbolDatabase::createSymbolDatabaseFunctionReturnTypes()
{
    // fill in function return types
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        std::list<Function>::iterator func;

        for (func = it->functionList.begin(); func != it->functionList.end(); ++func) {
            // add return types
            if (func->retDef) {
                const Token *type = func->retDef;
                while (Token::Match(type, "static|const|struct|union|enum"))
                    type = type->next();
                if (type) {
                    func->retType = findVariableTypeInBase(&*it, type);
                    if (!func->retType)
                        func->retType = findTypeInNested(type, func->nestedIn);
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseNeedInitialization()
{
    if (_tokenizer->isC()) {
        // For C code it is easy, as there are no constructors and no default values
        for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
            Scope *scope = &(*it);
            if (scope->definedType)
                scope->definedType->needInitialization = Type::True;
        }
    } else {
        // For C++, it is more difficult: Determine if user defined type needs initialization...
        unsigned int unknowns = 0; // stop checking when there are no unknowns
        unsigned int retry = 0;    // bail if we don't resolve all the variable types for some reason

        do {
            unknowns = 0;

            for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
                Scope *scope = &(*it);

                if (!scope->definedType) {
                    _blankTypes.push_back(Type());
                    scope->definedType = &_blankTypes.back();
                }

                if (scope->isClassOrStruct() && scope->definedType->needInitialization == Type::Unknown) {
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
                        scope->definedType->needInitialization = Type::False;

                    // check each member variable to see if it needs initialization
                    else {
                        bool needInitialization = false;
                        bool unknown = false;

                        std::list<Variable>::const_iterator var;
                        for (var = scope->varlist.begin(); var != scope->varlist.end() && !needInitialization; ++var) {
                            if (var->isClass()) {
                                if (var->type()) {
                                    // does this type need initialization?
                                    if (var->type()->needInitialization == Type::True)
                                        needInitialization = true;
                                    else if (var->type()->needInitialization == Type::Unknown)
                                        unknown = true;
                                }
                            } else if (!var->hasDefault())
                                needInitialization = true;
                        }

                        if (needInitialization)
                            scope->definedType->needInitialization = Type::True;
                        else if (!unknown)
                            scope->definedType->needInitialization = Type::False;

                        if (scope->definedType->needInitialization == Type::Unknown)
                            unknowns++;
                    }
                } else if (scope->type == Scope::eUnion && scope->definedType->needInitialization == Type::Unknown)
                    scope->definedType->needInitialization = Type::True;
            }

            retry++;
        } while (unknowns && retry < 100);

        // this shouldn't happen so output a debug warning
        if (retry == 100 && _settings->debugwarnings) {
            for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
                const Scope *scope = &(*it);

                if (scope->isClassOrStruct() && scope->definedType->needInitialization == Type::Unknown)
                    debugMessage(scope->classDef, "SymbolDatabase::SymbolDatabase couldn't resolve all user defined types.");
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseVariableSymbolTable()
{
    // create variable symbol table
    _variableList.resize(_tokenizer->varIdCount() + 1);
    std::fill_n(_variableList.begin(), _variableList.size(), (const Variable*)nullptr);

    // check all scopes for variables
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        Scope *scope = &(*it);

        // add all variables
        for (std::list<Variable>::iterator var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
            unsigned int varId = var->declarationId();
            if (varId)
                _variableList[varId] = &(*var);
            // fix up variables without type
            if (!var->type() && !var->typeStartToken()->isStandardType()) {
                const Type *type = findType(var->typeStartToken(), scope);
                if (type)
                    var->type(type);
            }
        }

        // add all function parameters
        for (std::list<Function>::iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            for (std::list<Variable>::iterator arg = func->argumentList.begin(); arg != func->argumentList.end(); ++arg) {
                // check for named parameters
                if (arg->nameToken() && arg->declarationId()) {
                    const unsigned int declarationId = arg->declarationId();
                    if (declarationId > 0U)
                        _variableList[declarationId] = &(*arg);
                    // fix up parameters without type
                    if (!arg->type() && !arg->typeStartToken()->isStandardType()) {
                        const Type *type = findTypeInNested(arg->typeStartToken(), scope);
                        if (type)
                            arg->type(type);
                    }
                }
            }
        }
    }

    // fill in missing variables if possible
    const std::size_t functions = functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope *func = functionScopes[i];
        for (const Token *tok = func->classStart->next(); tok && tok != func->classEnd; tok = tok->next()) {
            // check for member variable
            if (tok->varId() && tok->next() &&
                (tok->next()->str() == "." ||
                 (tok->next()->str() == "[" && tok->linkAt(1)->strAt(1) == "."))) {
                const Token *tok1 = tok->next()->str() == "." ? tok->tokAt(2) : tok->linkAt(1)->tokAt(2);
                if (tok1 && tok1->varId() && _variableList[tok1->varId()] == 0) {
                    const Variable *var = _variableList[tok->varId()];
                    if (var && var->typeScope()) {
                        // find the member variable of this variable
                        const Variable *var1 = var->typeScope()->getVariable(tok1->str());
                        if (var1) {
                            // add this variable to the look up table
                            _variableList[tok1->varId()] = var1;
                        }
                    }
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseSetScopePointers()
{
    // Set scope pointers
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        Token* start = const_cast<Token*>(it->classStart);
        Token* end = const_cast<Token*>(it->classEnd);
        if (it->type == Scope::eGlobal) {
            start = const_cast<Token*>(_tokenizer->list.front());
            end = const_cast<Token*>(_tokenizer->list.back());
        }
        if (start && end) {
            start->scope(&*it);
            end->scope(&*it);
        }
        if (start != end && start->next() != end) {
            for (Token* tok = start->next(); tok != end; tok = tok->next()) {
                if (tok->str() == "{") {
                    bool isEndOfScope = false;
                    for (std::list<Scope*>::const_iterator innerScope = it->nestedList.begin(); innerScope != it->nestedList.end(); ++innerScope) {
                        if (tok == (*innerScope)->classStart) { // Is begin of inner scope
                            tok = tok->link();
                            if (!tok || tok->next() == end || !tok->next()) {
                                isEndOfScope = true;
                                break;
                            }
                            tok = tok->next();
                            break;
                        }
                    }
                    if (isEndOfScope)
                        break;
                }
                tok->scope(&*it);
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseSetFunctionPointers(bool firstPass)
{
    if (firstPass) {
        // Set function definition and declaration pointers
        for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
            for (std::list<Function>::const_iterator func = it->functionList.begin(); func != it->functionList.end(); ++func) {
                if (func->tokenDef)
                    const_cast<Token *>(func->tokenDef)->function(&*func);

                if (func->token)
                    const_cast<Token *>(func->token)->function(&*func);
            }
        }
    }

    // Set function call pointers
    for (const Token* tok = _tokenizer->list.front(); tok != _tokenizer->list.back(); tok = tok->next()) {
        if (!tok->function() && tok->varId() == 0 && Token::Match(tok, "%name% (") && !isReservedName(tok->str())) {
            const Function *function = findFunction(tok);
            if (function)
                const_cast<Token *>(tok)->function(function);
        }
    }

    // Set C++ 11 delegate constructor function call pointers
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        for (std::list<Function>::const_iterator func = it->functionList.begin(); func != it->functionList.end(); ++func) {
            // look for initializer list
            if (func->type == Function::eConstructor && func->functionScope &&
                func->functionScope->functionOf && func->arg && func->arg->link()->strAt(1) == ":") {
                const Token * tok = func->arg->link()->tokAt(2);
                while (tok && tok != func->functionScope->classStart) {
                    if (Token::Match(tok, "%name% {|(")) {
                        if (tok->str() == func->tokenDef->str()) {
                            const Function *function = func->functionScope->functionOf->findFunction(tok);
                            if (function)
                                const_cast<Token *>(tok)->function(function);
                            break;
                        }
                        tok = tok->linkAt(1);
                    }
                    tok = tok->next();
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseSetTypePointers()
{
    // Set type pointers
    for (const Token* tok = _tokenizer->list.front(); tok != _tokenizer->list.back(); tok = tok->next()) {
        if (!tok->isName() || tok->varId() || tok->function() || tok->type() || tok->enumerator())
            continue;

        const Type *type = findVariableType(tok->scope(), tok);
        if (type)
            const_cast<Token *>(tok)->type(type);
    }
}

void SymbolDatabase::fixVarId(VarIdMap & varIds, const Token * vartok, Token * membertok, const Variable * membervar)
{
    VarIdMap::iterator varId = varIds.find(vartok->varId());
    if (varId == varIds.end()) {
        MemberIdMap memberId;
        if (membertok->varId() == 0) {
            memberId[membervar->nameToken()->varId()] = const_cast<Tokenizer *>(_tokenizer)->newVarId();
            _variableList.push_back(membervar);
        } else
            _variableList[membertok->varId()] = membervar;
        varIds.insert(std::make_pair(vartok->varId(), memberId));
        varId = varIds.find(vartok->varId());
    }
    MemberIdMap::iterator memberId = varId->second.find(membervar->nameToken()->varId());
    if (memberId == varId->second.end()) {
        if (membertok->varId() == 0) {
            varId->second.insert(std::make_pair(membervar->nameToken()->varId(), const_cast<Tokenizer *>(_tokenizer)->newVarId()));
            _variableList.push_back(membervar);
        } else
            _variableList[membertok->varId()] = membervar;
        memberId = varId->second.find(membervar->nameToken()->varId());
    }
    if (membertok->varId() == 0)
        membertok->varId(memberId->second);
}

void SymbolDatabase::createSymbolDatabaseSetVariablePointers()
{
    VarIdMap varIds;

    // Set variable pointers
    for (const Token* tok = _tokenizer->list.front(); tok != _tokenizer->list.back(); tok = tok->next()) {
        if (tok->varId())
            const_cast<Token *>(tok)->variable(getVariableFromVarId(tok->varId()));

        // Set Token::variable pointer for array member variable
        // Since it doesn't point at a fixed location it doesn't have varid
        if (tok->variable() != nullptr &&
            (tok->variable()->typeScope() || (tok->valueType() && tok->valueType()->type == ValueType::CONTAINER)) &&
            Token::Match(tok, "%name% [|.")) {

            Token *tok2 = tok->next();
            // Locate "]"
            while (tok2 && tok2->str() == "[")
                tok2 = tok2->link()->next();

            Token *membertok = nullptr;
            if (Token::Match(tok2, ". %name%"))
                membertok = tok2->next();
            else if (Token::Match(tok2, ") . %name%") && tok->strAt(-1) == "(")
                membertok = tok2->tokAt(2);

            if (membertok) {
                const Variable *var = tok->variable();
                if (var && var->typeScope()) {
                    const Variable *membervar = var->typeScope()->getVariable(membertok->str());
                    if (membervar) {
                        membertok->variable(membervar);
                        if (membertok->varId() == 0 || _variableList[membertok->varId()] == nullptr)
                            fixVarId(varIds, tok, const_cast<Token *>(membertok), membervar);
                    }
                } else if (var && tok->valueType() && tok->valueType()->type == ValueType::CONTAINER) {
                    if (Token::Match(var->typeStartToken(), "std :: %type% < %type% *| *| >")) {
                        const Type * type = var->typeStartToken()->tokAt(4)->type();
                        if (type && type->classScope && type->classScope->definedType) {
                            const Variable *membervar = type->classScope->getVariable(membertok->str());
                            if (membervar) {
                                membertok->variable(membervar);
                                if (membertok->varId() == 0 || _variableList[membertok->varId()] == nullptr)
                                    fixVarId(varIds, tok, const_cast<Token *>(membertok), membervar);
                            }
                        }
                    }
                }
            }
        }

        // check for function returning record type
        // func(...).var
        // func(...)[...].var
        else if (tok->function() && tok->next()->str() == "(" &&
                 (Token::Match(tok->next()->link(), ") . %name% !!(") ||
                  (Token::Match(tok->next()->link(), ") [") && Token::Match(tok->next()->link()->next()->link(), "] . %name% !!(")))) {
            const Type *type = tok->function()->retType;
            if (type) {
                Token *membertok;
                if (tok->next()->link()->next()->str() == ".")
                    membertok = tok->next()->link()->next()->next();
                else
                    membertok = tok->next()->link()->next()->link()->next()->next();
                const Variable *membervar = membertok->variable();
                if (!membervar) {
                    if (type->classScope) {
                        membervar = type->classScope->getVariable(membertok->str());
                        if (membervar)
                            membertok->variable(membervar);
                    }
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseEnums()
{
    // fill in enumerators in enum
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        if (it->type != Scope::eEnum)
            continue;

        // add enumerators to enumerator tokens
        for (std::size_t i = 0, end = it->enumeratorList.size(); i < end; ++i)
            const_cast<Token *>(it->enumeratorList[i].name)->enumerator(&it->enumeratorList[i]);
    }

    // fill in enumerator values
    for (std::list<Scope>::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        if (it->type != Scope::eEnum)
            continue;

        MathLib::bigint value = 0;

        for (std::size_t i = 0, end = it->enumeratorList.size(); i < end; ++i) {
            Enumerator & enumerator = it->enumeratorList[i];

            // look for initialization tokens that can be converted to enumerators and convert them
            if (enumerator.start) {
                for (const Token * tok3 = enumerator.start; tok3 && tok3 != enumerator.end->next(); tok3 = tok3->next()) {
                    if (tok3->tokType() == Token::eName) {
                        const Enumerator * e = findEnumerator(tok3);
                        if (e)
                            const_cast<Token *>(tok3)->enumerator(e);
                    }
                }

                // look for possible constant folding expressions
                if (enumerator.start) {
                    // rhs of operator:
                    const Token *rhs = enumerator.start->previous()->astOperand2();

                    // constant folding of expression:
                    ValueFlow::valueFlowConstantFoldAST(rhs, _settings);

                    // get constant folded value:
                    if (rhs && rhs->values().size() == 1U && rhs->values().front().isKnown()) {
                        enumerator.value = rhs->values().front().intvalue;
                        enumerator.value_known = true;
                        value = enumerator.value + 1;
                    }
                }
            }

            // not initialized so use default value
            else {
                enumerator.value = value++;
                enumerator.value_known = true;
            }
        }
    }

    // find enumerators
    for (const Token* tok = _tokenizer->list.front(); tok != _tokenizer->list.back(); tok = tok->next()) {
        if (tok->tokType() != Token::eName)
            continue;
        const Enumerator * enumerator = findEnumerator(tok);
        if (enumerator)
            const_cast<Token *>(tok)->enumerator(enumerator);
    }
}

void SymbolDatabase::createSymbolDatabaseUnknownArrayDimensions()
{
    // set all unknown array dimensions
    for (std::size_t i = 1; i <= _tokenizer->varIdCount(); i++) {
        // check each array variable
        if (_variableList[i] && _variableList[i]->isArray()) {
            // check each array dimension
            const std::vector<Dimension>& dimensions = _variableList[i]->dimensions();
            for (std::size_t j = 0; j < dimensions.size(); j++) {
                Dimension &dimension = const_cast<Dimension &>(dimensions[j]);
                if (dimension.num == 0) {
                    dimension.known = false;
                    // check for a single token dimension
                    if (dimension.start && (dimension.start == dimension.end)) {
                        // check for an enumerator
                        if (dimension.start->enumerator()) {
                            if (dimension.start->enumerator()->value_known) {
                                dimension.num = dimension.start->enumerator()->value;
                                dimension.known = true;
                            }
                        }

                        // check for a variable
                        else if (dimension.start->varId()) {
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
                    // check for qualified enumerator
                    else if (dimension.end) {
                        // rhs of [
                        const Token *rhs = dimension.start->previous()->astOperand2();

                        // constant folding of expression:
                        ValueFlow::valueFlowConstantFoldAST(rhs, _settings);

                        // get constant folded value:
                        if (rhs && rhs->values().size() == 1U && rhs->values().front().isKnown()) {
                            dimension.num = rhs->values().front().intvalue;
                            dimension.known = true;
                        }
                    }
                }
            }
        }
    }
}

SymbolDatabase::~SymbolDatabase()
{
    // Clear scope, type, function and variable pointers
    for (const Token* tok = _tokenizer->list.front(); tok; tok = tok->next()) {
        const_cast<Token *>(tok)->scope(0);
        const_cast<Token *>(tok)->type(0);
        const_cast<Token *>(tok)->function(0);
        const_cast<Token *>(tok)->variable(0);
        const_cast<Token *>(tok)->enumerator(0);
        const_cast<Token *>(tok)->setValueType(0);
    }
}

bool SymbolDatabase::isFunction(const Token *tok, const Scope* outerScope, const Token **funcStart, const Token **argStart, const Token** declEnd) const
{
    if (tok->varId())
        return false;

    // function returning function pointer? '... ( ... %name% ( ... ))( ... ) {'
    if (tok->str() == "(" && tok->strAt(1) != "*" &&
        tok->link()->previous()->str() == ")") {
        const Token* tok2 = tok->link()->next();
        if (tok2 && tok2->str() == "(" && Token::Match(tok2->link()->next(), "{|;|const|=")) {
            const Token* argStartTok = tok->link()->previous()->link();
            *funcStart = argStartTok->previous();
            *argStart = argStartTok;
            *declEnd = Token::findmatch(tok2->link()->next(), "{|;");
            return true;
        }
    }

    // regular function?
    else if (Token::Match(tok, "%name% (") && !isReservedName(tok->str()) && tok->previous() &&
             (Token::Match(tok->previous(), "%name%|>|&|*|::|~") || // Either a return type or scope qualifier in front of tok
              outerScope->isClassOrStruct())) { // or a ctor/dtor
        const Token* tok1 = tok->previous();
        const Token* tok2 = tok->next()->link()->next();

        // skip over destructor "~"
        if (tok1->str() == "~")
            tok1 = tok1->previous();

        // skip over qualification
        while (Token::simpleMatch(tok1, "::")) {
            tok1 = tok1->previous();
            if (Token::Match(tok1, "%name%"))
                tok1 = tok1->previous();
            else if (tok1 && tok1->str() == ">" && tok1->link() && Token::Match(tok1->link()->previous(), "%name%"))
                tok1 = tok1->link()->tokAt(-2);
        }

        // skip over const, noexcept, throw and override specifiers
        while (Token::Match(tok2, "const|noexcept|throw|override")) {
            tok2 = tok2->next();
            if (tok2 && tok2->str() == "(")
                tok2 = tok2->link()->next();
        }

        if (tok2 && tok2->str() == ".") {
            for (tok2 = tok2->next(); tok2; tok2 = tok2->next()) {
                if (Token::Match(tok2, ";|{|="))
                    break;
                if (tok2->link() && Token::Match(tok2, "<|[|("))
                    tok2 = tok2->link();
            }
        }

        // done if constructor or destructor
        if (!Token::Match(tok1, "{|}|;|public:|protected:|private:") && tok1) {
            // skip over pointers and references
            while (Token::Match(tok1, "[*&]"))
                tok1 = tok1->previous();

            // skip over template
            if (tok1 && tok1->str() == ">") {
                if (tok1->link())
                    tok1 = tok1->link()->previous();
                else
                    return false;
            }

            // function can't have number or variable as return type
            if (tok1 && (tok1->isNumber() || tok1->varId()))
                return false;

            // skip over return type
            if (Token::Match(tok1, "%name%")) {
                if (tok1->str() == "return")
                    return false;
                tok1 = tok1->previous();
            }

            // skip over qualification
            while (Token::simpleMatch(tok1, "::")) {
                tok1 = tok1->previous();
                if (Token::Match(tok1, "%name%"))
                    tok1 = tok1->previous();
            }

            // skip over modifiers and other stuff
            while (Token::Match(tok1, "const|static|extern|template|virtual|struct|class|enum|%name%")) {
                // friend type func(); is not a function
                if (isCPP() && tok1->str() == "friend" && tok2->str() == ";")
                    return false;
                tok1 = tok1->previous();
            }

            // should be at a sequence point if this is a function
            if (!Token::Match(tok1, ">|{|}|;|public:|protected:|private:") && tok1)
                return false;
        }

        if (tok2 &&
            (Token::Match(tok2, ";|{|=") ||
             (tok2->isUpperCaseName() && Token::Match(tok2, "%name% ;|{")) ||
             (tok2->isUpperCaseName() && Token::Match(tok2, "%name% (") && tok2->next()->link()->strAt(1) == "{") ||
             Token::Match(tok2, ": ::| %name% (|::|<|{") ||
             Token::Match(tok2, "&|&&| ;|{") ||
             Token::Match(tok2, "= delete|default ;"))) {
            *funcStart = tok;
            *argStart = tok->next();
            *declEnd = Token::findmatch(tok2, "{|;");
            return true;
        }
    }

    // UNKNOWN_MACRO(a,b) { ... }
    else if (outerScope->type == Scope::eGlobal &&
             Token::Match(tok, "%name% (") &&
             tok->isUpperCaseName() &&
             Token::simpleMatch(tok->linkAt(1), ") {") &&
             (!tok->previous() || Token::Match(tok->previous(), "[;{}]"))) {
        *funcStart = tok;
        *argStart = tok->next();
        *declEnd = tok->linkAt(1)->next();
        return true;
    }

    // template constructor?
    else if (Token::Match(tok, "%name% <") && Token::simpleMatch(tok->next()->link(), "> (")) {
        const Token* tok2 = tok->next()->link()->next()->link();
        if (Token::Match(tok2, ") const| ;|{|=") ||
            Token::Match(tok2, ") : ::| %name% (|::|<|{") ||
            Token::Match(tok2, ") const| noexcept {|;|(")) {
            *funcStart = tok;
            *argStart = tok2->link();
            *declEnd = Token::findmatch(tok2->next(), "{|;");
            return true;
        }
    }

    // regular C function with missing return or invalid C++ ?
    else if (Token::Match(tok, "%name% (") && !isReservedName(tok->str()) &&
             Token::simpleMatch(tok->linkAt(1), ") {") &&
             (!tok->previous() || Token::Match(tok->previous(), ";|}"))) {
        if (_tokenizer->isC()) {
            debugMessage(tok, "SymbolDatabase::isFunction found C function '" + tok->str() + "' without a return type.");
            *funcStart = tok;
            *argStart = tok->next();
            *declEnd = tok->linkAt(1)->next();
            return true;
        }
        _tokenizer->syntaxError(tok);
    }

    return false;
}

void SymbolDatabase::validateExecutableScopes() const
{
    const std::size_t functions = functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope* const scope = functionScopes[i];
        const Function* const function = scope->function;
        if (scope->isExecutable() && !function) {
            const std::list<const Token*> callstack(1, scope->classDef);
            const std::string msg = std::string("Executable scope '") + scope->classDef->str() + "' with unknown function.";
            const ErrorLogger::ErrorMessage errmsg(callstack, &_tokenizer->list, Severity::debug,
                                                   "symbolDatabaseWarning",
                                                   msg,
                                                   false);
            _errorLogger->reportErr(errmsg);
        }
    }
}

namespace {
    const Function* getFunctionForArgumentvariable(const Variable * const var, const std::vector<const Scope *>& functionScopes)
    {
        const std::size_t functions = functionScopes.size();
        for (std::size_t i = 0; i < functions; ++i) {
            const Scope* const scope = functionScopes[i];
            const Function* const function = scope->function;
            if (function) {
                for (std::size_t arg=0; arg < function->argCount(); ++arg) {
                    if (var==function->getArgumentVar(arg))
                        return function;
                }
            }
        }
        return nullptr;
    }
}

void SymbolDatabase::validateVariables() const
{
    for (std::vector<const Variable *>::const_iterator iter = _variableList.begin(); iter!=_variableList.end(); ++iter) {
        if (*iter) {
            const Variable * const var = *iter;
            if (!var->scope()) {
                const Function* function = getFunctionForArgumentvariable(var, functionScopes);
                if (!var->isArgument() || (function && function->hasBody())) {
                    throw InternalError(var->nameToken(), "Analysis failed (variable without scope). If the code is valid then please report this failure.", InternalError::INTERNAL);
                    //std::cout << "!!!Variable found without scope: " << var->nameToken()->str() << std::endl;
                }
            }
        }
    }
}

void SymbolDatabase::validate() const
{
    if (_settings->debugwarnings) {
        validateExecutableScopes();
    }
    //validateVariables();
}

bool Variable::isPointerArray() const
{
    return isArray() && nameToken() && nameToken()->previous() && (nameToken()->previous()->str() == "*");
}

const Token * Variable::declEndToken() const
{
    Token const * declEnd = typeStartToken();
    while (declEnd && !Token::Match(declEnd, "[;,)={]")) {
        if (declEnd->link() && Token::Match(declEnd,"(|["))
            declEnd = declEnd->link();
        declEnd = declEnd->next();
    }
    return declEnd;
}

void Variable::evaluate(const Library* lib)
{
    if (_name)
        setFlag(fIsArray, arrayDimensions(lib));

    const Token* tok = _start;
    while (tok && tok->previous() && tok->previous()->isName())
        tok = tok->previous();
    for (const Token* const end = _name?_name:_end; tok != end;) {
        if (tok->str() == "static")
            setFlag(fIsStatic, true);
        else if (tok->str() == "extern")
            setFlag(fIsExtern, true);
        else if (tok->str() == "volatile")
            setFlag(fIsVolatile, true);
        else if (tok->str() == "mutable")
            setFlag(fIsMutable, true);
        else if (tok->str() == "const")
            setFlag(fIsConst, true);
        else if (tok->str() == "*") {
            setFlag(fIsPointer, !isArray() || Token::Match(tok->previous(), "( * %name% )"));
            setFlag(fIsConst, false); // Points to const, isn't necessarily const itself
        } else if (tok->str() == "&") {
            if (isReference())
                setFlag(fIsRValueRef, true);
            setFlag(fIsReference, true);
        } else if (tok->str() == "&&") { // Before simplification, && isn't split up
            setFlag(fIsRValueRef, true);
            setFlag(fIsReference, true); // Set also fIsReference
        }

        if (tok->str() == "<" && tok->link())
            tok = tok->link();
        else
            tok = tok->next();
    }

    while (Token::Match(_start, "static|const %any%"))
        _start = _start->next();
    while (_end && _end->previous() && _end->str() == "const")
        _end = _end->previous();

    if (_start) {
        std::string strtype = _start->str();
        for (const Token *typeToken = _start; Token::Match(typeToken, "%type% :: %type%"); typeToken = typeToken->tokAt(2))
            strtype += "::" + typeToken->strAt(2);
        setFlag(fIsClass, !lib->podtype(strtype) && !_start->isStandardType() && !isPointer() && !isReference());
        setFlag(fIsStlType, Token::simpleMatch(_start, "std ::"));
        setFlag(fIsStlString, isStlType() && (Token::Match(_start->tokAt(2), "string|wstring|u16string|u32string !!::") || (Token::simpleMatch(_start->tokAt(2), "basic_string <") && !Token::simpleMatch(_start->linkAt(3), "> ::"))));
    }
    if (_access == Argument) {
        tok = _name;
        if (!tok) {
            // Argument without name
            tok = _end;
            // back up to start of array dimensions
            while (tok && tok->str() == "]")
                tok = tok->link()->previous();
            // add array dimensions if present
            if (tok && tok->next()->str() == "[")
                setFlag(fIsArray, arrayDimensions(lib));
        }
        if (!tok)
            return;
        tok = tok->next();
        while (tok->str() == "[")
            tok = tok->link();
        setFlag(fHasDefault, tok->str() == "=");
    }
    // check for C++11 member initialization
    if (_scope && _scope->isClassOrStruct()) {
        // type var = x or
        // type var = {x}
        // type var = x; gets simplified to: type var ; var = x ;
        Token const * declEnd = declEndToken();
        if ((Token::Match(declEnd, "; %name% =") && declEnd->strAt(1) == _name->str()) ||
            Token::Match(declEnd, "=|{"))
            setFlag(fHasDefault, true);
    }

    if (_start) {
        if (Token::Match(_start, "float|double"))
            setFlag(fIsFloatType, true);
    }
}

bool Function::argsMatch(const Scope *scope, const Token *first, const Token *second, const std::string &path, unsigned int depth)
{
    const bool isCPP = scope->check->isCPP();
    if (!isCPP) // C does not support overloads
        return true;

    // skip "struct"
    if (first->str() == "struct" || first->str() == "enum")
        first = first->next();
    if (second->str() == "struct" || second->str() == "enum")
        second = second->next();

    // skip const on type passed by value
    if (Token::Match(first, "const %type% %name%|,|)"))
        first = first->next();
    if (Token::Match(second, "const %type% %name%|,|)"))
        second = second->next();

    while (first->str() == second->str() &&
           first->isLong() == second->isLong() &&
           first->isUnsigned() == second->isUnsigned()) {
        // at end of argument list
        if (first->str() == ")") {
            return true;
        }

        // skip default value assignment
        else if (first->next()->str() == "=") {
            first = first->nextArgument();
            if (first)
                first = first->tokAt(-2);
            if (second->next()->str() == "=") {
                second = second->nextArgument();
                if (second)
                    second = second->tokAt(-2);
                if (!first || !second) { // End of argument list (first or second)
                    return !first && !second;
                }
            } else if (!first) { // End of argument list (first)
                return !second->nextArgument(); // End of argument list (second)
            }
        } else if (second->next()->str() == "=") {
            second = second->nextArgument();
            if (second)
                second = second->tokAt(-2);
            if (!second) { // End of argument list (second)
                return false;
            }
        }

        // definition missing variable name
        else if ((first->next()->str() == "," && second->next()->str() != ",") ||
                 (first->next()->str() == ")" && second->next()->str() != ")")) {
            second = second->next();
            // skip default value assignment
            if (second->next()->str() == "=") {
                do {
                    second = second->next();
                } while (!Token::Match(second->next(), ",|)"));
            }
        } else if (first->next()->str() == "[" && second->next()->str() != "[")
            second = second->next();

        // function missing variable name
        else if ((second->next()->str() == "," && first->next()->str() != ",") ||
                 (second->next()->str() == ")" && first->next()->str() != ")")) {
            first = first->next();
            // skip default value assignment
            if (first->next()->str() == "=") {
                do {
                    first = first->next();
                } while (!Token::Match(first->next(), ",|)"));
            }
        } else if (second->next()->str() == "[" && first->next()->str() != "[")
            first = first->next();

        // argument list has different number of arguments
        else if (second->str() == ")")
            break;

        // ckeck for type * x == type x[]
        else if (Token::Match(first->next(), "* %name%| ,|)|=") &&
                 Token::Match(second->next(), "%name%| [ ] ,|)")) {
            do {
                first = first->next();
            } while (!Token::Match(first->next(), ",|)"));
            do {
                second = second->next();
            } while (!Token::Match(second->next(), ",|)"));
        }

        // const after *
        else if (first->next()->str() == "*" && first->strAt(2) != "const" &&
                 second->next()->str() == "*" && second->strAt(2) == "const") {
            first = first->next();
            second = second->tokAt(2);
        }

        // variable names are different
        else if ((Token::Match(first->next(), "%name% ,|)|=|[") &&
                  Token::Match(second->next(), "%name% ,|)|[")) &&
                 (first->next()->str() != second->next()->str())) {
            // skip variable names
            first = first->next();
            second = second->next();

            // skip default value assignment
            if (first->next()->str() == "=") {
                do {
                    first = first->next();
                } while (!Token::Match(first->next(), ",|)"));
            }
        }

        // variable with class path
        else if (depth && Token::Match(first->next(), "%name%")) {
            std::string param = path + first->next()->str();

            if (Token::simpleMatch(second->next(), param.c_str())) {
                second = second->tokAt(int(depth) * 2);
            } else if (depth > 1) {
                std::string short_path = path;

                // remove last " :: "
                short_path.resize(short_path.size() - 4);

                // remove last name
                std::string::size_type lastSpace = short_path.find_last_of(' ');
                if (lastSpace != std::string::npos)
                    short_path.resize(lastSpace+1);

                param = short_path + first->next()->str();
                if (Token::simpleMatch(second->next(), param.c_str())) {
                    second = second->tokAt((int(depth) - 1) * 2);
                }
            }
        }

        // nested or base class variable
        else if (depth == 0 && Token::Match(first->next(), "%name%") &&
                 Token::Match(second->next(), "%name% :: %name%") &&
                 ((second->next()->str() == scope->className) ||
                  (scope->definedType && scope->definedType->isDerivedFrom(second->next()->str()))) &&
                 (first->next()->str() == second->strAt(3))) {
            second = second->tokAt(2);
        }

        first = first->next();
        second = second->next();

        // skip "struct"
        if (first->str() == "struct" || first->str() == "enum")
            first = first->next();
        if (second->str() == "struct" || second->str() == "enum")
            second = second->next();

        // skip const on type passed by value
        if (Token::Match(first, "const %type% %name%|,|)") &&
            !Token::Match(first, "const %type% %name%| ["))
            first = first->next();
        if (Token::Match(second, "const %type% %name%|,|)") &&
            !Token::Match(second, "const %type% %name%| ["))
            second = second->next();
    }

    return false;
}

const Token * Function::constructorMemberInitialization() const
{
    if (!isConstructor() || !functionScope || !functionScope->classStart)
        return nullptr;
    if (Token::Match(token, "%name% (") && Token::simpleMatch(token->linkAt(1), ") :"))
        return token->linkAt(1)->next();
    return nullptr;
}

Function* SymbolDatabase::addGlobalFunction(Scope*& scope, const Token*& tok, const Token *argStart, const Token* funcStart)
{
    Function* function = nullptr;
    for (std::multimap<std::string, const Function *>::iterator i = scope->functionMap.find(tok->str()); i != scope->functionMap.end() && i->first == tok->str(); ++i) {
        if (Function::argsMatch(scope, i->second->argDef->next(), argStart->next(), emptyString, 0)) {
            function = const_cast<Function *>(i->second);
            break;
        }
    }

    if (!function)
        function = addGlobalFunctionDecl(scope, tok, argStart, funcStart);

    function->arg = argStart;
    function->token = funcStart;
    function->hasBody(true);

    addNewFunction(&scope, &tok);

    if (scope) {
        scope->function = function;
        function->functionScope = scope;
        return function;
    }
    return nullptr;
}

Function* SymbolDatabase::addGlobalFunctionDecl(Scope*& scope, const Token *tok, const Token *argStart, const Token* funcStart)
{
    Function function;

    // save the function definition argument start '('
    function.argDef = argStart;

    // save the access type
    function.access = Public;

    // save the function name location
    function.tokenDef = funcStart;

    function.isInline(false);
    function.hasBody(false);
    function.type = Function::eFunction;
    function.nestedIn = scope;

    const Token *tok1 = tok;

    // look for end of previous statement
    while (tok1->previous() && !Token::Match(tok1->previous(), ";|}|{"))
        tok1 = tok1->previous();

    // find the return type
    while (Token::Match(tok1, "static|extern|const")) {
        if (tok1->str() == "static")
            function.isStaticLocal(true);
        else if (tok1->str() == "extern")
            function.isExtern(true);

        tok1 = tok1->next();
    }

    if (function.argDef->link()->strAt(1) == ".")
        function.retDef = function.argDef->link()->tokAt(2);
    else if (tok1)
        function.retDef = tok1;

    scope->addFunction(function);
    return &scope->functionList.back();
}

void SymbolDatabase::addClassFunction(Scope **scope, const Token **tok, const Token *argStart)
{
    const bool destructor((*tok)->previous()->str() == "~");
    const bool has_const(argStart->link()->strAt(1) == "const");
    const bool lval(argStart->link()->strAt(has_const ? 2 : 1) == "&");
    const bool rval(argStart->link()->strAt(has_const ? 2 : 1) == "&&");
    const Token *tok1;
    // skip class/struct name
    if (destructor)
        tok1 = (*tok)->tokAt(-3);
    else if ((*tok)->strAt(-2) == ">" && (*tok)->linkAt(-2))
        tok1 = (*tok)->linkAt(-2)->previous();
    else
        tok1 = (*tok)->tokAt(-2);

    // syntax error?
    if (!tok1)
        return;

    int count = 0;
    std::string path;
    unsigned int path_length = 0;

    // back up to head of path
    while (tok1 && tok1->previous() && tok1->previous()->str() == "::" &&
           tok1->tokAt(-2) && (tok1->tokAt(-2)->isName() || (tok1->strAt(-2) == ">" && tok1->linkAt(-2)))) {
        if (tok1->strAt(-2) == ">") {
            tok1 = tok1->tokAt(-2);
            const Token * tok2 = tok1->previous();
            path = ":: " + path;
            if (tok2) {
                do {
                    path = tok1->str() + " " + path;
                    tok1 = tok1->previous();
                    count++;
                    path_length++;
                } while (tok1 != tok2);
            }
        } else {
            path = tok1->str() + " :: " + path;
            tok1 = tok1->tokAt(-2);
            count++;
            path_length++;
        }
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

        // check in namespace if using found
        if (*scope == scope1 && !scope1->usingList.empty()) {
            std::list<Scope::UsingInfo>::const_iterator it2;
            for (it2 = scope1->usingList.begin(); it2 != scope1->usingList.end(); ++it2) {
                if (it2->scope) {
                    Function * func = findFunctionInScope(tok1, it2->scope);
                    if (func) {
                        if (!func->hasBody()) {
                            func->hasBody(true);
                            func->token = *tok;
                            func->arg = argStart;
                            addNewFunction(scope, tok);
                            if (*scope) {
                                (*scope)->functionOf = func->nestedIn;
                                (*scope)->function = func;
                                (*scope)->function->functionScope = *scope;
                            }
                            return;
                        }
                    }
                }
            }
        }

        if (scope1->className == tok1->str() && (scope1->type != Scope::eFunction)) {
            // do the scopes match (same scope) or do their names match (multiple namespaces)
            if ((*scope == scope1->nestedIn) || (*scope &&
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
            for (std::multimap<std::string, const Function *>::iterator it = scope1->functionMap.find((*tok)->str()); it != scope1->functionMap.end() && it->first == (*tok)->str(); ++it) {
                Function * func = const_cast<Function *>(it->second);
                if (!func->hasBody()) {
                    if (Function::argsMatch(scope1, func->argDef, (*tok)->next(), path, path_length)) {
                        if (func->type == Function::eDestructor && destructor) {
                            func->hasBody(true);
                        } else if (func->type != Function::eDestructor && !destructor) {
                            // normal function?
                            if ((*tok)->next()->link()) {
                                const bool hasConstKeyword = (*tok)->next()->link()->next()->str() == "const";
                                if ((func->isConst() == hasConstKeyword) &&
                                    (func->hasLvalRefQualifier() == lval) &&
                                    (func->hasRvalRefQualifier() == rval)) {
                                    func->hasBody(true);
                                }
                            }
                        }

                        if (func->hasBody()) {
                            func->token = *tok;
                            func->arg = argStart;
                            addNewFunction(scope, tok);
                            if (*scope) {
                                (*scope)->functionOf = scope1;
                                (*scope)->function = func;
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
    Scope *newScope = &scopeList.back();

    // find start of function '{'
    bool foundInitList = false;
    while (tok1 && tok1->str() != "{" && tok1->str() != ";") {
        if (tok1->link() && Token::Match(tok1, "(|<")) {
            tok1 = tok1->link();
        } else if (foundInitList &&
                   Token::Match(tok1, "%name%|> {") &&
                   Token::Match(tok1->linkAt(1), "} ,|{")) {
            tok1 = tok1->linkAt(1);
        } else {
            if (tok1->str() == ":")
                foundInitList = true;
            tok1 = tok1->next();
        }
    }

    if (tok1 && tok1->str() == "{") {
        newScope->classStart = tok1;
        newScope->classEnd = tok1->link();

        // syntax error?
        if (!newScope->classEnd) {
            scopeList.pop_back();
            while (tok1->next())
                tok1 = tok1->next();
            *scope = nullptr;
            *tok = tok1;
            return;
        }

        (*scope)->nestedList.push_back(newScope);
        *scope = newScope;
        *tok = tok1;
    } else {
        scopeList.pop_back();
        *scope = nullptr;
        *tok = nullptr;
    }
}

const Token *Type::initBaseInfo(const Token *tok, const Token *tok1)
{
    // goto initial '{'
    const Token *tok2 = tok1;
    while (tok2 && tok2->str() != "{") {
        // skip unsupported templates
        if (tok2->str() == "<")
            tok2 = tok2->link();

        // check for base classes
        else if (Token::Match(tok2, ":|,")) {
            tok2 = tok2->next();

            // check for invalid code
            if (!tok2 || !tok2->next())
                return nullptr;

            Type::BaseInfo base;

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
            if (!tok2)
                return nullptr;
            if (tok2->str() == "virtual") {
                base.isVirtual = true;
                tok2 = tok2->next();
            }
            if (!tok2)
                return nullptr;

            base.nameTok = tok2;
            // handle global namespace
            if (tok2->str() == "::") {
                tok2 = tok2->next();
            }

            // handle derived base classes
            while (Token::Match(tok2, "%name% ::")) {
                tok2 = tok2->tokAt(2);
            }
            if (!tok2)
                return nullptr;

            base.name = tok2->str();

            tok2 = tok2->next();
            // add unhandled templates
            if (tok2 && tok2->link() && tok2->str() == "<") {
                for (const Token* const end = tok2->link()->next(); tok2 != end; tok2 = tok2->next()) {
                    base.name += tok2->str();
                }
            }

            // save pattern for base class name
            derivedFrom.push_back(base);
        } else
            tok2 = tok2->next();
    }

    return tok2;
}

const std::string& Type::name() const
{
    const Token* next = classDef->next();
    if (classScope && classScope->enumClass && isEnumType())
        return next->strAt(1);
    else if (next->isName())
        return next->str();
    return emptyString;
}

void SymbolDatabase::debugMessage(const Token *tok, const std::string &msg) const
{
    if (tok && _settings->debugwarnings) {
        const std::list<const Token*> locationList(1, tok);
        const ErrorLogger::ErrorMessage errmsg(locationList, &_tokenizer->list,
                                               Severity::debug,
                                               "debug",
                                               msg,
                                               false);
        if (_errorLogger)
            _errorLogger->reportErr(errmsg);
    }
}

const Function* Type::getFunction(const std::string& funcName) const
{
    if (classScope) {
        std::multimap<std::string, const Function *>::const_iterator it = classScope->functionMap.find(funcName);

        if (it != classScope->functionMap.end())
            return it->second;
    }

    for (std::size_t i = 0; i < derivedFrom.size(); i++) {
        if (derivedFrom[i].type) {
            const Function* const func = derivedFrom[i].type->getFunction(funcName);
            if (func)
                return func;
        }
    }
    return 0;
}

bool Type::hasCircularDependencies(std::set<BaseInfo>* ancestors) const
{
    std::set<BaseInfo> knownAncestors;
    if (!ancestors) {
        ancestors=&knownAncestors;
    }
    for (std::vector<BaseInfo>::const_iterator parent=derivedFrom.begin(); parent!=derivedFrom.end(); ++parent) {
        if (!parent->type)
            continue;
        else if (this==parent->type)
            return true;
        else if (ancestors->find(*parent)!=ancestors->end())
            return true;
        else {
            ancestors->insert(*parent);
            if (parent->type->hasCircularDependencies(ancestors))
                return true;
        }
    }
    return false;
}

bool Type::findDependency(const Type* ancestor) const
{
    if (this==ancestor)
        return true;
    for (std::vector<BaseInfo>::const_iterator parent=derivedFrom.begin(); parent!=derivedFrom.end(); ++parent) {
        if (parent->type && parent->type->findDependency(ancestor))
            return true;
    }
    return false;
}

bool Type::isDerivedFrom(const std::string & ancestor) const
{
    for (std::vector<BaseInfo>::const_iterator parent=derivedFrom.begin(); parent!=derivedFrom.end(); ++parent) {
        if (parent->name == ancestor)
            return true;
        if (parent->type && parent->type->isDerivedFrom(ancestor))
            return true;
    }
    return false;
}

bool Variable::arrayDimensions(const Library* lib)
{
    const Library::Container* container = lib->detectContainer(_start);
    if (container && container->arrayLike_indexOp && container->size_templateArgNo > 0) {
        const Token* tok = Token::findsimplematch(_start, "<");
        if (tok) {
            Dimension dimension_;
            tok = tok->next();
            for (int i = 0; i < container->size_templateArgNo && tok; i++) {
                tok = tok->nextTemplateArgument();
            }
            if (tok) {
                dimension_.start = tok;
                dimension_.end = Token::findmatch(tok, ",|>");
                if (dimension_.end)
                    dimension_.end = dimension_.end->previous();
                if (dimension_.start == dimension_.end) {
                    dimension_.num = MathLib::toLongNumber(dimension_.start->str());
                    dimension_.known = true;
                }
            }
            _dimensions.push_back(dimension_);
            return true;
        }
    }

    const Token *dim = _name;
    if (!dim) {
        // Argument without name
        dim = _end;
        // back up to start of array dimensions
        while (dim && dim->str() == "]")
            dim = dim->link()->previous();
    }
    if (dim)
        dim = dim->next();
    if (dim && dim->str() == ")")
        dim = dim->next();

    bool arr = false;
    while (dim && dim->next() && dim->str() == "[") {
        Dimension dimension_;
        // check for empty array dimension []
        if (dim->next()->str() != "]") {
            dimension_.start = dim->next();
            dimension_.end = dim->link()->previous();
            if (dimension_.start == dimension_.end && dimension_.start->isNumber()) {
                dimension_.num = MathLib::toLongNumber(dimension_.start->str());
                dimension_.known = true;
            }
        }
        _dimensions.push_back(dimension_);
        dim = dim->link()->next();
        arr = true;
    }
    return arr;
}

void Variable::setFlags(const ValueType &valuetype)
{
    if (valuetype.constness)
        setFlag(fIsConst,true);
    if (valuetype.pointer)
        setFlag(fIsPointer,true);
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
          type == Scope::eFor ? "For" :
          type == Scope::eWhile ? "While" :
          type == Scope::eDo ? "Do" :
          type == Scope::eSwitch ? "Switch" :
          type == Scope::eTry ? "Try" :
          type == Scope::eCatch ? "Catch" :
          type == Scope::eUnconditional ? "Unconditional" :
          type == Scope::eLambda ? "Lambda" :
          type == Scope::eEnum ? "Enum" :
          "Unknown");
    return s;
}

static std::string tokenToString(const Token* tok, const Tokenizer* tokenizer)
{
    std::ostringstream oss;
    if (tok) {
        oss << tok->str() << " ";
        oss << tokenizer->list.fileLine(tok) << " ";
    }
    oss << tok;
    return oss.str();
}

static std::string scopeToString(const Scope* scope, const Tokenizer* tokenizer)
{
    std::ostringstream oss;
    if (scope) {
        oss << scope->type << " ";
        if (scope->classDef)
            oss << tokenizer->list.fileLine(scope->classDef) << " ";
    }
    oss << scope;
    return oss.str();
}

void SymbolDatabase::printVariable(const Variable *var, const char *indent) const
{
    std::cout << indent << "_name: " << tokenToString(var->nameToken(), _tokenizer) << std::endl;
    if (var->nameToken()) {
        std::cout << indent << "    declarationId: " << var->declarationId() << std::endl;
    }
    std::cout << indent << "_start: " << tokenToString(var->typeStartToken(), _tokenizer) << std::endl;
    std::cout << indent << "_end: " << tokenToString(var->typeEndToken(), _tokenizer) << std::endl;
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
               "Unknown")  << std::endl;
    std::cout << indent << "_flags: " << std::endl;
    std::cout << indent << "    isMutable: " << var->isMutable() << std::endl;
    std::cout << indent << "    isStatic: " << var->isStatic() << std::endl;
    std::cout << indent << "    isExtern: " << var->isExtern() << std::endl;
    std::cout << indent << "    isLocal: " << var->isLocal() << std::endl;
    std::cout << indent << "    isConst: " << var->isConst() << std::endl;
    std::cout << indent << "    isClass: " << var->isClass() << std::endl;
    std::cout << indent << "    isArray: " << var->isArray() << std::endl;
    std::cout << indent << "    isPointer: " << var->isPointer() << std::endl;
    std::cout << indent << "    isReference: " << var->isReference() << std::endl;
    std::cout << indent << "    isRValueRef: " << var->isRValueReference() << std::endl;
    std::cout << indent << "    hasDefault: " << var->hasDefault() << std::endl;
    std::cout << indent << "    isStlType: " << var->isStlType() << std::endl;
    std::cout << indent << "_type: ";
    if (var->type()) {
        std::cout << var->type()->type() << " " << var->type()->name();
        std::cout << " " << _tokenizer->list.fileLine(var->type()->classDef);
        std::cout << " " << var->type() << std::endl;
    } else
        std::cout << "none" << std::endl;

    std::cout << indent << "_scope: " << scopeToString(var->scope(), _tokenizer) << std::endl;

    std::cout << indent << "_dimensions:";
    for (std::size_t i = 0; i < var->dimensions().size(); i++) {
        std::cout << " " << var->dimension(i);
        if (!var->dimensions()[i].known)
            std::cout << "?";
    }
    std::cout << std::endl;
}

void SymbolDatabase::printOut(const char *title) const
{
    std::cout << std::setiosflags(std::ios::boolalpha);
    if (title)
        std::cout << "\n### " << title << " ###\n";

    for (std::list<Scope>::const_iterator scope = scopeList.begin(); scope != scopeList.end(); ++scope) {
        std::cout << "Scope: " << &*scope << " " << scope->type << std::endl;
        std::cout << "    className: " << scope->className << std::endl;
        std::cout << "    classDef: " << tokenToString(scope->classDef, _tokenizer) << std::endl;
        std::cout << "    classStart: " << tokenToString(scope->classStart, _tokenizer) << std::endl;
        std::cout << "    classEnd: " << tokenToString(scope->classEnd, _tokenizer) << std::endl;

        std::list<Function>::const_iterator func;

        // find the function body if not implemented inline
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            std::cout << "    Function: " << &*func << std::endl;
            std::cout << "        name: " << tokenToString(func->tokenDef, _tokenizer) << std::endl;
            std::cout << "        type: " << (func->type == Function::eConstructor? "Constructor" :
                                              func->type == Function::eCopyConstructor ? "CopyConstructor" :
                                              func->type == Function::eMoveConstructor ? "MoveConstructor" :
                                              func->type == Function::eOperatorEqual ? "OperatorEqual" :
                                              func->type == Function::eDestructor ? "Destructor" :
                                              func->type == Function::eFunction ? "Function" :
                                              "Unknown") << std::endl;
            std::cout << "        access: " << (func->access == Public ? "Public" :
                                                func->access == Protected ? "Protected" :
                                                func->access == Private ? "Private" :
                                                "Unknown")  << std::endl;
            std::cout << "        hasBody: " << func->hasBody() << std::endl;
            std::cout << "        isInline: " << func->isInline() << std::endl;
            std::cout << "        isConst: " << func->isConst() << std::endl;
            std::cout << "        isVirtual: " << func->isVirtual() << std::endl;
            std::cout << "        isPure: " << func->isPure() << std::endl;
            std::cout << "        isStatic: " << func->isStatic() << std::endl;
            std::cout << "        isStaticLocal: " << func->isStaticLocal() << std::endl;
            std::cout << "        isExtern: " << func->isExtern() << std::endl;
            std::cout << "        isFriend: " << func->isFriend() << std::endl;
            std::cout << "        isExplicit: " << func->isExplicit() << std::endl;
            std::cout << "        isDefault: " << func->isDefault() << std::endl;
            std::cout << "        isDelete: " << func->isDelete() << std::endl;
            std::cout << "        isNoExcept: " << func->isNoExcept() << std::endl;
            std::cout << "        isThrow: " << func->isThrow() << std::endl;
            std::cout << "        isOperator: " << func->isOperator() << std::endl;
            std::cout << "        hasLvalRefQual: " << func->hasLvalRefQualifier() << std::endl;
            std::cout << "        hasRvalRefQual: " << func->hasRvalRefQualifier() << std::endl;
            std::cout << "        isVariadic: " << func->isVariadic() << std::endl;
            std::cout << "        attributes:";
            if (func->isAttributeConst())
                std::cout << " const ";
            if (func->isAttributePure())
                std::cout << " pure ";
            if (func->isAttributeNoreturn())
                std::cout << " noreturn ";
            if (func->isAttributeNothrow())
                std::cout << " nothrow ";
            if (func->isAttributeConstructor())
                std::cout << " constructor ";
            if (func->isAttributeDestructor())
                std::cout << " destructor ";
            std::cout << std::endl;
            std::cout << "        noexceptArg: " << (func->noexceptArg ? func->noexceptArg->str() : "none") << std::endl;
            std::cout << "        throwArg: " << (func->throwArg ? func->throwArg->str() : "none") << std::endl;
            std::cout << "        tokenDef: " << tokenToString(func->tokenDef, _tokenizer) << std::endl;
            std::cout << "        argDef: " << tokenToString(func->argDef, _tokenizer) << std::endl;
            if (!func->isConstructor() && !func->isDestructor())
                std::cout << "        retDef: " << tokenToString(func->retDef, _tokenizer) << std::endl;
            std::cout << "        retType: " << func->retType << std::endl;
            if (func->hasBody()) {
                std::cout << "        token: " << tokenToString(func->token, _tokenizer) << std::endl;
                std::cout << "        arg: " << tokenToString(func->arg, _tokenizer) << std::endl;
            }
            std::cout << "        nestedIn: " << scopeToString(func->nestedIn, _tokenizer) << std::endl;
            std::cout << "        functionScope: " << scopeToString(func->functionScope, _tokenizer) << std::endl;

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

        if (scope->type == Scope::eEnum) {
            std::cout << "    enumType: ";
            if (scope->enumType)
                scope->enumType->stringify(std::cout, false, true, false);
            else
                std::cout << "int";
            std::cout << std::endl;
            std::cout << "    enumClass: " << scope->enumClass << std::endl;
            for (std::vector<Enumerator>::const_iterator enumerator = scope->enumeratorList.begin(); enumerator != scope->enumeratorList.end(); ++enumerator) {
                std::cout << "        Enumerator: " << enumerator->name->str() << " = ";
                if (enumerator->value_known) {
                    std::cout << enumerator->value;
                }

                if (enumerator->start) {
                    const Token * tok = enumerator->start;
                    std::cout << (enumerator->value_known ? " " : "") << "[" << tok->str();
                    while (tok && tok != enumerator->end) {
                        if (tok->next())
                            std::cout << " " << tok->next()->str();
                        tok = tok->next();
                    }

                    std::cout << "]";
                }

                std::cout << std::endl;
            }
        }

        std::cout << "    nestedIn: " << scope->nestedIn;
        if (scope->nestedIn) {
            std::cout << " " << scope->nestedIn->type << " "
                      << scope->nestedIn->className;
        }
        std::cout << std::endl;

        std::cout << "    definedType: " << scope->definedType << std::endl;

        std::cout << "    nestedList[" << scope->nestedList.size() << "] = (";

        std::list<Scope *>::const_iterator nsi;

        std::size_t count = scope->nestedList.size();
        for (nsi = scope->nestedList.begin(); nsi != scope->nestedList.end(); ++nsi) {
            std::cout << " " << (*nsi) << " " << (*nsi)->type << " " << (*nsi)->className;
            if (count-- > 1)
                std::cout << ",";
        }

        std::cout << " )" << std::endl;

        std::list<Scope::UsingInfo>::const_iterator use;

        for (use = scope->usingList.begin(); use != scope->usingList.end(); ++use) {
            std::cout << "    using: " << use->scope << " " << use->start->strAt(2);
            const Token *tok1 = use->start->tokAt(3);
            while (tok1 && tok1->str() == "::") {
                std::cout << "::" << tok1->strAt(1);
                tok1 = tok1->tokAt(2);
            }
            std::cout << " " << _tokenizer->list.fileLine(use->start) << std::endl;
        }

        std::cout << "    functionOf: " << scopeToString(scope->functionOf, _tokenizer) << std::endl;

        std::cout << "    function: " << scope->function;
        if (scope->function)
            std::cout << " " << scope->function->name();
        std::cout << std::endl;
    }

    for (std::list<Type>::const_iterator type = typeList.begin(); type != typeList.end(); ++type) {
        std::cout << "Type: " << &(*type) << std::endl;
        std::cout << "    name: " << type->name() << std::endl;
        std::cout << "    classDef: " << tokenToString(type->classDef, _tokenizer) << std::endl;
        std::cout << "    classScope: " << type->classScope << std::endl;
        std::cout << "    enclosingScope: " << type->enclosingScope << std::endl;
        std::cout << "    needInitialization: " << (type->needInitialization == Type::Unknown ? "Unknown" :
                  type->needInitialization == Type::True ? "True" :
                  type->needInitialization == Type::False ? "False" :
                  "Invalid") << std::endl;

        std::cout << "    derivedFrom[" << type->derivedFrom.size() << "] = (";
        std::size_t count = type->derivedFrom.size();
        for (std::size_t i = 0; i < type->derivedFrom.size(); ++i) {
            if (type->derivedFrom[i].isVirtual)
                std::cout << "Virtual ";

            std::cout << (type->derivedFrom[i].access == Public    ? " Public" :
                          type->derivedFrom[i].access == Protected ? " Protected" :
                          type->derivedFrom[i].access == Private   ? " Private" :
                          " Unknown");

            if (type->derivedFrom[i].type)
                std::cout << " " << type->derivedFrom[i].type;
            else
                std::cout << " Unknown";

            std::cout << " " << type->derivedFrom[i].name;
            if (count-- > 1)
                std::cout << ",";
        }

        std::cout << " )" << std::endl;

        std::cout << "    friendList[" << type->friendList.size() << "] = (";

        std::list<Type::FriendInfo>::const_iterator fii;

        count = type->friendList.size();
        for (fii = type->friendList.begin(); fii != type->friendList.end(); ++fii) {
            if (fii->type)
                std::cout << fii->type;
            else
                std::cout << " Unknown";

            std::cout << " " << fii->name;
            if (count-- > 1)
                std::cout << ",";
        }

        std::cout << " )" << std::endl;
    }

    for (std::size_t i = 1; i < _variableList.size(); i++) {
        std::cout << "_variableList[" << i << "]: " << _variableList[i];
        if (_variableList[i]) {
            std::cout << " " << _variableList[i]->name() << " "
                      << _tokenizer->list.fileLine(_variableList[i]->nameToken());
        }
        std::cout << std::endl;
    }
    std::cout << std::resetiosflags(std::ios::boolalpha);
}

void SymbolDatabase::printXml(std::ostream &out) const
{
    out << std::setiosflags(std::ios::boolalpha);
    // Scopes..
    out << "  <scopes>" << std::endl;
    for (std::list<Scope>::const_iterator scope = scopeList.begin(); scope != scopeList.end(); ++scope) {
        out << "    <scope";
        out << " id=\"" << &*scope << "\"";
        out << " type=\"" << scope->type << "\"";
        if (!scope->className.empty())
            out << " className=\"" << ErrorLogger::toxml(scope->className) << "\"";
        if (scope->classStart)
            out << " classStart=\"" << scope->classStart << '\"';
        if (scope->classEnd)
            out << " classEnd=\"" << scope->classEnd << '\"';
        if (scope->nestedIn)
            out << " nestedIn=\"" << scope->nestedIn << "\"";
        if (scope->function)
            out << " function=\"" << scope->function << "\"";
        if (scope->functionList.empty() && scope->varlist.empty())
            out << "/>" << std::endl;
        else {
            out << '>' << std::endl;
            if (!scope->functionList.empty()) {
                out << "      <functionList>" << std::endl;
                for (std::list<Function>::const_iterator function = scope->functionList.begin(); function != scope->functionList.end(); ++function) {
                    out << "        <function id=\"" << &*function << "\" tokenDef=\"" << function->tokenDef << "\" name=\"" << ErrorLogger::toxml(function->name()) << '\"';
                    if (function->argCount() == 0U)
                        out << "/>" << std::endl;
                    else {
                        out << ">" << std::endl;
                        for (unsigned int argnr = 0; argnr < function->argCount(); ++argnr) {
                            const Variable *arg = function->getArgumentVar(argnr);
                            out << "          <arg nr=\"" << argnr+1 << "\" variable=\"" << arg << "\"/>" << std::endl;
                        }
                        out << "        </function>" << std::endl;
                    }
                }
                out << "      </functionList>" << std::endl;
            }
            if (!scope->varlist.empty()) {
                out << "      <varlist>" << std::endl;
                for (std::list<Variable>::const_iterator var = scope->varlist.begin(); var != scope->varlist.end(); ++var)
                    out << "        <var id=\""   << &*var << "\"/>" << std::endl;
                out << "      </varlist>" << std::endl;
            }
            out << "    </scope>" << std::endl;
        }
    }
    out << "  </scopes>" << std::endl;

    // Variables..
    out << "  <variables>" << std::endl;
    for (unsigned int i = 1U; i < _variableList.size(); i++) {
        const Variable *var = _variableList[i];
        if (!var)
            continue;
        out << "    <var id=\""   << var << '\"';
        out << " nameToken=\""      << var->nameToken() << '\"';
        out << " typeStartToken=\"" << var->typeStartToken() << '\"';
        out << " typeEndToken=\""   << var->typeEndToken() << '\"';
        out << " isArgument=\""     << var->isArgument() << '\"';
        out << " isArray=\""        << var->isArray() << '\"';
        out << " isClass=\""        << var->isClass() << '\"';
        out << " isLocal=\""        << var->isLocal() << '\"';
        out << " isPointer=\""      << var->isPointer() << '\"';
        out << " isReference=\""    << var->isReference() << '\"';
        out << " isStatic=\""       << var->isStatic() << '\"';
        out << "/>" << std::endl;
    }
    out << "  </variables>" << std::endl;
    out << std::resetiosflags(std::ios::boolalpha);
}

//---------------------------------------------------------------------------

static const Type* findVariableTypeIncludingUsedNamespaces(const SymbolDatabase* symbolDatabase, const Scope* scope, const Token* typeTok)
{
    const Type* argType = symbolDatabase->findVariableType(scope, typeTok);
    if (argType)
        return argType;

    // look for variable type in any using namespace in this scope or above
    while (scope) {
        for (std::list<Scope::UsingInfo>::const_iterator ui = scope->usingList.begin();
             ui != scope->usingList.end(); ++ui) {
            if (ui->scope) {
                argType = symbolDatabase->findVariableType(ui->scope, typeTok);
                if (argType)
                    return argType;
            }
        }
        scope = scope->nestedIn;
    }
    return nullptr;
}

//---------------------------------------------------------------------------

void Function::addArguments(const SymbolDatabase *symbolDatabase, const Scope *scope)
{
    // check for non-empty argument list "( ... )"
    const Token * start = arg ? arg : argDef;
    if (start && start->link() != start->next() && !Token::simpleMatch(start, "( void )")) {
        unsigned int count = 0;

        for (const Token* tok = start->next(); tok; tok = tok->next()) {
            if (Token::Match(tok, ",|)"))
                return; // Syntax error

            const Token* startTok = tok;
            const Token* endTok   = nullptr;
            const Token* nameTok  = nullptr;

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
                    tok = tok->link();
                    if (!tok) // something is wrong so just bail out
                        return;
                }

                tok = tok->next();

                if (!tok) // something is wrong so just bail
                    return;
            } while (tok->str() != "," && tok->str() != ")" && tok->str() != "=");

            const Token *typeTok = startTok;
            // skip over stuff to get to type
            while (Token::Match(typeTok, "const|enum|struct|::"))
                typeTok = typeTok->next();
            // skip over qualification
            while (Token::Match(typeTok, "%type% ::"))
                typeTok = typeTok->tokAt(2);

            // check for argument with no name or missing varid
            if (!endTok) {
                if (tok->previous()->isName() && tok->strAt(-1) != "const") {
                    if (tok->previous() != typeTok) {
                        nameTok = tok->previous();
                        endTok = nameTok->previous();

                        if (hasBody())
                            symbolDatabase->debugMessage(nameTok, "Function::addArguments found argument \'" + nameTok->str() + "\' with varid 0.");
                    } else
                        endTok = typeTok;
                } else
                    endTok = tok->previous();
            }

            const ::Type *argType = nullptr;
            if (!typeTok->isStandardType()) {
                argType = findVariableTypeIncludingUsedNamespaces(symbolDatabase, scope, typeTok);

                // save type
                const_cast<Token *>(typeTok)->type(argType);
            }

            // skip default values
            if (tok->str() == "=") {
                do {
                    if (tok->link() && Token::Match(tok, "[{[(<]"))
                        tok = tok->link();
                    tok = tok->next();
                } while (tok->str() != "," && tok->str() != ")");
            }

            // skip over stuff before type
            while (Token::Match(startTok, "enum|struct|const"))
                startTok = startTok->next();

            argumentList.push_back(Variable(nameTok, startTok, endTok, count++, Argument, argType, functionScope, &symbolDatabase->_settings->library));

            if (tok->str() == ")") {
                // check for a variadic function
                if (Token::simpleMatch(startTok, ". . ."))
                    isVariadic(true);

                break;
            }
        }

        // count default arguments
        for (const Token* tok = argDef->next(); tok && tok != argDef->link(); tok = tok->next()) {
            if (tok->str() == "=")
                initArgCount++;
        }
    }
}

bool Function::isImplicitlyVirtual(bool defaultVal) const
{
    if (isVirtual())
        return true;
    else if (access == Private || access == Public || access == Protected) {
        bool safe = true;
        bool hasVirt = isImplicitlyVirtual_rec(nestedIn->definedType, safe);
        if (hasVirt)
            return true;
        else if (safe)
            return false;
        else
            return defaultVal;
    } else
        return false;
}

bool Function::isImplicitlyVirtual_rec(const ::Type* baseType, bool& safe) const
{
    // check each base class
    for (std::size_t i = 0; i < baseType->derivedFrom.size(); ++i) {
        const ::Type* derivedFromType = baseType->derivedFrom[i].type;
        // check if base class exists in database
        if (derivedFromType && derivedFromType->classScope) {
            const Scope *parent = derivedFromType->classScope;

            // check if function defined in base class
            for (std::multimap<std::string, const Function *>::const_iterator it = parent->functionMap.find(tokenDef->str()); it != parent->functionMap.end() && it->first == tokenDef->str(); ++it) {
                const Function * func = it->second;
                if (func->isVirtual()) { // Base is virtual and of same name
                    const Token *temp1 = func->tokenDef->previous();
                    const Token *temp2 = tokenDef->previous();
                    bool returnMatch = true;

                    // check for matching return parameters
                    while (temp1->str() != "virtual") {
                        if (temp1->str() != temp2->str() &&
                            !(temp1->str() == derivedFromType->name() &&
                              temp2->str() == baseType->name())) {
                            returnMatch = false;
                            break;
                        }

                        temp1 = temp1->previous();
                        temp2 = temp2->previous();
                    }

                    // check for matching function parameters
                    if (returnMatch && argsMatch(baseType->classScope, func->argDef, argDef, emptyString, 0)) {
                        return true;
                    }
                }
            }

            if (!derivedFromType->derivedFrom.empty() && !derivedFromType->hasCircularDependencies()) {
                // avoid endless recursion, see #5289 Crash: Stack overflow in isImplicitlyVirtual_rec when checking SVN and
                // #5590 with a loop within the class hierarchie.
                if (isImplicitlyVirtual_rec(derivedFromType, safe))  {
                    return true;
                }
            }
        } else {
            // unable to find base class so assume it has no virtual function
            safe = false;
            return false;
        }
    }
    return false;
}

const Variable* Function::getArgumentVar(std::size_t num) const
{
    for (std::list<Variable>::const_iterator i = argumentList.begin(); i != argumentList.end(); ++i) {
        if (i->index() == num)
            return (&*i);
        else if (i->index() > num)
            return 0;
    }
    return 0;
}


//---------------------------------------------------------------------------

Scope::Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_, ScopeType type_, const Token *start_) :
    check(check_),
    classDef(classDef_),
    classStart(start_),
    classEnd(start_->link()),
    nestedIn(nestedIn_),
    numConstructors(0),
    numCopyOrMoveConstructors(0),
    type(type_),
    definedType(nullptr),
    functionOf(nullptr),
    function(nullptr),
    enumType(nullptr),
    enumClass(false)
{
}

Scope::Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_) :
    check(check_),
    classDef(classDef_),
    classStart(nullptr),
    classEnd(nullptr),
    nestedIn(nestedIn_),
    numConstructors(0),
    numCopyOrMoveConstructors(0),
    definedType(nullptr),
    functionOf(nullptr),
    function(nullptr),
    enumType(nullptr),
    enumClass(false)
{
    const Token *nameTok = classDef;
    if (!classDef) {
        type = Scope::eGlobal;
    } else if (classDef->str() == "class" && check && check->isCPP()) {
        type = Scope::eClass;
        nameTok = nameTok->next();
    } else if (classDef->str() == "struct") {
        type = Scope::eStruct;
        nameTok = nameTok->next();
    } else if (classDef->str() == "union") {
        type = Scope::eUnion;
        nameTok = nameTok->next();
    } else if (classDef->str() == "namespace") {
        type = Scope::eNamespace;
        nameTok = nameTok->next();
    } else if (classDef->str() == "enum") {
        type = Scope::eEnum;
        nameTok = nameTok->next();
        if (nameTok->str() == "class") {
            enumClass = true;
            nameTok = nameTok->next();
        }
    } else {
        type = Scope::eFunction;
    }
    // skip over qualification if present
    if (nameTok && nameTok->str() == "::")
        nameTok = nameTok->next();
    while (Token::Match(nameTok, "%type% ::"))
        nameTok = nameTok->tokAt(2);
    if (nameTok && ((type == Scope::eEnum && Token::Match(nameTok, ":|{")) || nameTok->str() != "{")) // anonymous and unnamed structs/unions don't have a name

        className = nameTok->str();
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
void Scope::getVariableList(const Library* lib)
{
    const Token *start;

    if (classStart)
        start = classStart->next();

    // global scope
    else if (className.empty())
        start = check->_tokenizer->tokens();

    // forward declaration
    else
        return;

    AccessControl varaccess = defaultAccess();
    for (const Token *tok = start; tok && tok != classEnd; tok = tok->next()) {
        // syntax error?
        if (tok->next() == nullptr)
            break;

        // Is it a function?
        else if (tok->str() == "{") {
            tok = tok->link();
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
        } else if (Token::Match(tok, "struct|union {")) {
            if (Token::Match(tok->next()->link(), "} %name% ;|[")) {
                tok = tok->next()->link()->tokAt(2);
                continue;
            } else if (Token::simpleMatch(tok->next()->link(), "} ;")) {
                tok = tok->next();
                continue;
            }
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
        else if (Token::Match(tok, "class|struct|union %name% ;")) {
            tok = tok->tokAt(2);
            continue;
        }

        // Borland C++: Ignore properties..
        else if (tok->str() == "__property")
            continue;

        // skip return, goto and delete
        else if (Token::Match(tok, "return|delete|goto")) {
            while (tok->next() &&
                   tok->next()->str() != ";" &&
                   tok->next()->str() != "}" /* ticket #4994 */) {
                tok = tok->next();
            }
            continue;
        }

        // Search for start of statement..
        else if (tok->previous() && !Token::Match(tok->previous(), ";|{|}|public:|protected:|private:"))
            continue;
        else if (tok->str() == ";")
            continue;

        tok = checkVariable(tok, varaccess, lib);

        if (!tok)
            break;
    }
}

const Token *Scope::checkVariable(const Token *tok, AccessControl varaccess, const Library* lib)
{
    // Is it a throw..?
    if (Token::Match(tok, "throw %any% (") &&
        Token::simpleMatch(tok->linkAt(2), ") ;")) {
        return tok->linkAt(2);
    } else if ((Token::Match(tok, "throw %any% :: %any% (") &&
                Token::simpleMatch(tok->linkAt(4), ") ;"))) {
        return tok->linkAt(4);
    }

    // friend?
    if (Token::Match(tok, "friend %type%") && tok->next()->varId() == 0) {
        const Token *next = Token::findmatch(tok->tokAt(2), ";|{");
        if (next && next->str() == "{")
            next = next->link();
        return next;
    }

    // skip const|static|mutable|extern
    while (Token::Match(tok, "const|static|mutable|extern")) {
        tok = tok->next();
    }

    // the start of the type tokens does not include the above modifiers
    const Token *typestart = tok;

    if (Token::Match(tok, "class|struct|union|enum")) {
        tok = tok->next();
    }

    // This is the start of a statement
    const Token *vartok = nullptr;
    const Token *typetok = nullptr;

    if (tok && isVariableDeclaration(tok, vartok, typetok)) {
        // If the vartok was set in the if-blocks above, create a entry for this variable..
        tok = vartok->next();
        while (Token::Match(tok, "[|{"))
            tok = tok->link()->next();

        if (vartok->varId() == 0) {
            if (!vartok->isBoolean())
                check->debugMessage(vartok, "Scope::checkVariable found variable \'" + vartok->str() + "\' with varid 0.");
            return tok;
        }

        const Type *vType = nullptr;

        if (typetok) {
            vType = findVariableTypeIncludingUsedNamespaces(check, this, typetok);

            const_cast<Token *>(typetok)->type(vType);
        }

        // skip "enum" or "struct"
        if (Token::Match(typestart, "enum|struct"))
            typestart = typestart->next();

        addVariable(vartok, typestart, vartok->previous(), varaccess, vType, this, lib);
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

    return nullptr;
}

static const Token* skipScopeIdentifiers(const Token* tok)
{
    if (tok && tok->str() == "::") {
        tok = tok->next();
    }
    while (Token::Match(tok, "%type% ::")) {
        tok = tok->tokAt(2);
    }

    return tok;
}

static const Token* skipPointers(const Token* tok)
{
    while (Token::Match(tok, "*|&|&&")  || (Token::Match(tok, "( [*&]") && Token::Match(tok->link()->next(), "(|["))) {
        tok = tok->next();
        if (tok->strAt(-1) == "(" && Token::Match(tok, "%type% ::"))
            tok = tok->tokAt(2);
    }

    return tok;
}

bool Scope::isVariableDeclaration(const Token* const tok, const Token*& vartok, const Token*& typetok) const
{
    if (check && check->_tokenizer->isCPP() && Token::Match(tok, "throw|new"))
        return false;

    const Token* localTypeTok = skipScopeIdentifiers(tok);
    const Token* localVarTok = nullptr;

    if (Token::Match(localTypeTok, "%type% <")) {
        if (Token::Match(tok, "const_cast|dynamic_cast|reinterpret_cast|static_cast <"))
            return false;

        const Token* closeTok = localTypeTok->next()->link();
        if (closeTok) {
            localVarTok = skipPointers(closeTok->next());

            if (Token::Match(localVarTok, ":: %type% %name% [;=({]")) {
                if (localVarTok->strAt(3) != "(" ||
                    Token::Match(localVarTok->linkAt(3), "[)}] ;")) {
                    localTypeTok = localVarTok->next();
                    localVarTok = localVarTok->tokAt(2);
                }
            }
        }
    } else if (Token::Match(localTypeTok, "%type%")) {
        localVarTok = skipPointers(localTypeTok->strAt(1)=="const"?localTypeTok->tokAt(2):localTypeTok->next());
    }

    if (!localVarTok)
        return false;

    if (localVarTok->str() == "const")
        localVarTok = localVarTok->next();

    if (Token::Match(localVarTok, "%name% ;|=") || (localVarTok && localVarTok->varId() && localVarTok->strAt(1) == ":")) {
        vartok = localVarTok;
        typetok = localTypeTok;
    } else if (Token::Match(localVarTok, "%name% )|[") && localVarTok->str() != "operator") {
        vartok = localVarTok;
        typetok = localTypeTok;
    } else if (localVarTok && localVarTok->varId() && Token::Match(localVarTok, "%name% (|{") &&
               Token::Match(localVarTok->next()->link(), ")|} ;")) {
        vartok = localVarTok;
        typetok = localTypeTok;
    } else if (type == eCatch &&
               Token::Match(localVarTok, "%name% )")) {
        vartok = localVarTok;
        typetok = localTypeTok;
    }

    return nullptr != vartok;
}

const Token * Scope::addEnum(const Token * tok, bool isCpp)
{
    const Token * tok2 = tok->next();

    // skip over class if present
    if (isCpp && tok2->str() == "class")
        tok2 = tok2->next();

    // skip over name
    tok2 = tok2->next();

    // save type if present
    if (tok2->str() == ":") {
        tok2 = tok2->next();

        enumType = tok2;
        tok2 = tok2->next();
    }

    // add enumerators
    if (tok2->str() == "{") {
        const Token * end = tok2->link();
        tok2 = tok2->next();

        while (Token::Match(tok2, "%name% =|,|}") ||
               (Token::Match(tok2, "%name% (") && Token::Match(tok2->linkAt(1), ") ,|}"))) {
            Enumerator enumerator(this);

            // save enumerator name
            enumerator.name = tok2;

            // skip over name
            tok2 = tok2->next();

            if (tok2->str() == "=") {
                // skip over "="
                tok2 = tok2->next();

                if (tok2->str() == "}")
                    return nullptr;

                enumerator.start = tok2;

                while (!Token::Match(tok2, ",|}")) {
                    if (tok2->link())
                        tok2 = tok2->link();
                    enumerator.end = tok2;
                    tok2 = tok2->next();
                }
            } else if (tok2->str() == "(") {
                // skip over unknown macro
                tok2 = tok2->link()->next();
            }

            if (tok2->str() == ",") {
                enumeratorList.push_back(enumerator);
                tok2 = tok2->next();
            } else if (tok2->str() == "}") {
                enumeratorList.push_back(enumerator);
                break;
            }
        }

        if (tok2 == end) {
            tok2 = tok2->next();

            if (tok2 && tok2->str() != ";")
                tok2 = nullptr;
        } else
            tok2 = nullptr;
    } else
        tok2 = nullptr;

    return tok2;
}

const Enumerator * SymbolDatabase::findEnumerator(const Token * tok) const
{
    const Scope * scope = tok->scope();

    // check for qualified name
    if (tok->strAt(-1) == "::") {
        // find first scope
        const Token *tok1 = tok;
        while (Token::Match(tok1->tokAt(-2), "%name% ::"))
            tok1 = tok1->tokAt(-2);

        if (tok1->strAt(-1) == "::")
            scope = &scopeList.front();
        else {
            // FIXME search base class here

            // find first scope
            while (scope && scope->nestedIn) {
                const Scope * temp = scope->nestedIn->findRecordInNestedList(tok1->str());
                if (temp) {
                    scope = temp;
                    break;
                }
                scope = scope->nestedIn;
            }
        }

        if (scope) {
            tok1 = tok1->tokAt(2);
            while (scope && Token::Match(tok1, "%name% ::")) {
                scope = scope->findRecordInNestedList(tok1->str());
                tok1 = tok1->tokAt(2);
            }

            if (scope) {
                const Enumerator * enumerator = scope->findEnumerator(tok->str());

                if (enumerator) // enum class
                    return enumerator;
                // enum
                else {
                    for (std::list<Scope *>::const_iterator it = scope->nestedList.begin(), end = scope->nestedList.end(); it != end; ++it) {
                        enumerator = (*it)->findEnumerator(tok->str());

                        if (enumerator)
                            return enumerator;
                    }
                }
            }
        }
    } else {
        const Enumerator * enumerator = scope->findEnumerator(tok->str());

        if (enumerator)
            return enumerator;

        for (std::list<Scope *>::const_iterator s = scope->nestedList.begin(); s != scope->nestedList.end(); ++s) {
            enumerator = (*s)->findEnumerator(tok->str());

            if (enumerator)
                return enumerator;
        }

        if (scope->definedType) {
            for (size_t i = 0, end = scope->definedType->derivedFrom.size(); i < end; ++i) {
                if (scope->definedType->derivedFrom[i].type && scope->definedType->derivedFrom[i].type->classScope) {
                    enumerator = scope->definedType->derivedFrom[i].type->classScope->findEnumerator(tok->str());

                    if (enumerator)
                        return enumerator;
                }
            }
        }

        while (scope && scope->nestedIn) {
            if (scope->type == Scope::eFunction && scope->functionOf)
                scope = scope->functionOf;
            else
                scope = scope->nestedIn;

            enumerator = scope->findEnumerator(tok->str());

            if (enumerator)
                return enumerator;

            for (std::list<Scope*>::const_iterator s = scope->nestedList.begin(); s != scope->nestedList.end(); ++s) {
                enumerator = (*s)->findEnumerator(tok->str());

                if (enumerator)
                    return enumerator;
            }
        }
    }

    return nullptr;
}

//---------------------------------------------------------------------------

const Type* SymbolDatabase::findVariableTypeInBase(const Scope* scope, const Token* typeTok) const
{
    if (scope && scope->definedType && !scope->definedType->derivedFrom.empty()) {
        for (std::size_t i = 0; i < scope->definedType->derivedFrom.size(); ++i) {
            const Type *base = scope->definedType->derivedFrom[i].type;
            if (base && base->classScope) {
                const Type * type = base->classScope->findType(typeTok->str());
                if (type)
                    return type;
                type = findVariableTypeInBase(base->classScope, typeTok);
                if (type)
                    return type;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------

const Type* SymbolDatabase::findVariableType(const Scope *start, const Token *typeTok) const
{
    const Scope *scope = start;

    // check if type does not have a namespace
    if (typeTok->strAt(-1) != "::" && typeTok->strAt(1) != "::") {
        while (scope) {
            // look for type in this scope
            const Type * type = scope->findType(typeTok->str());

            if (type)
                return type;

            // look for type in base classes if possible
            if (scope->isClassOrStruct()) {
                type = findVariableTypeInBase(scope, typeTok);

                if (type)
                    return type;
            }

            // check if in member function class to see if it's present in class
            if (scope->type == Scope::eFunction && scope->functionOf) {
                const Scope *scope1 = scope->functionOf;

                type = scope1->findType(typeTok->str());

                if (type)
                    return type;

                type = findVariableTypeInBase(scope1, typeTok);

                if (type)
                    return type;
            }

            scope = scope->nestedIn;
        }
    }

    // check for a qualified name and use it when given
    else if (typeTok->strAt(-1) == "::") {
        // check if type is not part of qualification
        if (typeTok->strAt(1) == "::")
            return nullptr;

        // find start of qualified function name
        const Token *tok1 = typeTok;

        while (Token::Match(tok1->tokAt(-2), "%type% ::"))
            tok1 = tok1->tokAt(-2);

        // check for global scope
        if (tok1->strAt(-1) == "::") {
            scope = &scopeList.front();

            scope = scope->findRecordInNestedList(tok1->str());
        }

        // find start of qualification
        else {
            while (scope) {
                if (scope->className == tok1->str())
                    break;
                else {
                    const Scope *scope1 = scope->findRecordInNestedList(tok1->str());

                    if (scope1) {
                        scope = scope1;
                        break;
                    } else
                        scope = scope->nestedIn;
                }
            }
        }

        if (scope) {
            // follow qualification
            while (scope && Token::Match(tok1, "%type% ::")) {
                tok1 = tok1->tokAt(2);
                const Scope * temp = scope->findRecordInNestedList(tok1->str());
                if (!temp) {
                    // look in base classes
                    const Type * type = findVariableTypeInBase(scope, tok1);

                    if (type)
                        return type;
                }
                scope = temp;
            }

            if (scope && scope->definedType)
                return scope->definedType;
        }
    }

    return nullptr;
}

bool Scope::hasInlineOrLambdaFunction() const
{
    for (std::list<Scope*>::const_iterator it = nestedList.begin(); it != nestedList.end(); ++it) {
        const Scope *s = *it;
        // Inline function
        if (s->type == Scope::eUnconditional && Token::simpleMatch(s->classStart->previous(), ") {"))
            return true;
        // Lambda function
        if (s->type == Scope::eLambda)
            return true;
    }
    return false;
}

void Scope::findFunctionInBase(const std::string & name, size_t args, std::vector<const Function *> & matches) const
{
    if (isClassOrStruct() && definedType && !definedType->derivedFrom.empty()) {
        for (std::size_t i = 0; i < definedType->derivedFrom.size(); ++i) {
            const Type *base = definedType->derivedFrom[i].type;
            if (base && base->classScope) {
                if (base->classScope == this) // Ticket #5120, #5125: Recursive class; tok should have been found already
                    continue;

                for (std::multimap<std::string, const Function *>::const_iterator it = base->classScope->functionMap.find(name); it != base->classScope->functionMap.end() && it->first == name; ++it) {
                    const Function *func = it->second;
                    if (args == func->argCount() || (args < func->argCount() && args >= func->minArgCount())) {
                        matches.push_back(func);
                    }
                }

                base->classScope->findFunctionInBase(name, args, matches);
            }
        }
    }
}

//---------------------------------------------------------------------------

static void checkVariableCallMatch(const Variable* callarg, const Variable* funcarg, size_t& same, size_t& fallback1, size_t& fallback2)
{
    if (callarg) {
        bool ptrequals = callarg->isArrayOrPointer() == funcarg->isArrayOrPointer();
        bool constEquals = !callarg->isArrayOrPointer() || ((callarg->typeStartToken()->strAt(-1) == "const") == (funcarg->typeStartToken()->strAt(-1) == "const"));
        if (ptrequals && constEquals &&
            callarg->typeStartToken()->str() == funcarg->typeStartToken()->str() &&
            callarg->typeStartToken()->isUnsigned() == funcarg->typeStartToken()->isUnsigned() &&
            callarg->typeStartToken()->isLong() == funcarg->typeStartToken()->isLong()) {
            same++;
        } else if (callarg->isArrayOrPointer()) {
            if (ptrequals && constEquals && funcarg->typeStartToken()->str() == "void")
                fallback1++;
            else if (constEquals && funcarg->isStlStringType() && Token::Match(callarg->typeStartToken(), "char|wchar_t"))
                fallback2++;
        } else if (ptrequals) {
            bool takesInt = Token::Match(funcarg->typeStartToken(), "char|short|int|long");
            bool takesFloat = Token::Match(funcarg->typeStartToken(), "float|double");
            bool passesInt = Token::Match(callarg->typeStartToken(), "char|short|int|long");
            bool passesFloat = Token::Match(callarg->typeStartToken(), "float|double");
            if ((takesInt && passesInt) || (takesFloat && passesFloat))
                fallback1++;
            else if ((takesInt && passesFloat) || (takesFloat && passesInt))
                fallback2++;
        }
    }
}

const Function* Scope::findFunction(const Token *tok, bool requireConst) const
{
    // make sure this is a function call
    const Token *end = tok->linkAt(1);
    if (!end)
        return nullptr;

    std::vector<const Token *> arguments;

    // find all the arguments for this function call
    for (const Token *arg = tok->tokAt(2); arg && arg != end; arg = arg->nextArgument()) {
        arguments.push_back(arg);
    }

    std::vector<const Function *> matches;

    // find all the possible functions that could match
    const std::size_t args = arguments.size();
    for (std::multimap<std::string, const Function *>::const_iterator it = functionMap.find(tok->str()); it != functionMap.cend() && it->first == tok->str(); ++it) {
        const Function *func = it->second;
        if (args == func->argCount() ||
            (func->isVariadic() && args >= (func->argCount() - 1)) ||
            (args < func->argCount() && args >= func->minArgCount())) {
            matches.push_back(func);
        }
    }

    // check in base classes
    findFunctionInBase(tok->str(), args, matches);

    const Function* fallback1Func = nullptr;
    const Function* fallback2Func = nullptr;

    // check each function against the arguments in the function call for a match
    for (std::size_t i = 0; i < matches.size();) {
        bool erased = false;
        bool constFallback = false;
        const Function * func = matches[i];
        size_t same = 0;

        if (!requireConst || !func->isConst()) {
            // get the function this call is in
            const Scope * scope = tok->scope();

            // check if this function is a member function
            if (scope && scope->functionOf && scope->functionOf->isClassOrStruct()) {
                // check if isConst mismatches
                if (!(scope->function && scope->function->isConst() == func->isConst())) {
                    if (scope->function->isConst()) {
                        if (!erased)
                            ++i;
                        continue;
                    }
                    constFallback = true;
                }
            }
        }

        size_t fallback1 = 0;
        size_t fallback2 = 0;
        for (std::size_t j = 0; j < args; ++j) {

            // don't check variadic arguments
            if (func->isVariadic() && j > (func->argCount() - 1)) {
                break;
            }
            const Variable *funcarg = func->getArgumentVar(j);
            // check for a match with a variable
            if (Token::Match(arguments[j], "%var% ,|)")) {
                const Variable * callarg = check->getVariableFromVarId(arguments[j]->varId());
                checkVariableCallMatch(callarg, funcarg, same, fallback1, fallback2);
            }

            // check for a match with address of a variable
            else if (Token::Match(arguments[j], "& %var% ,|)")) {
                const Variable * callarg = check->getVariableFromVarId(arguments[j]->next()->varId());
                if (callarg) {
                    bool funcargptr = (funcarg->typeEndToken()->str() == "*");
                    if (funcargptr &&
                        (callarg->typeStartToken()->str() == funcarg->typeStartToken()->str() &&
                         callarg->typeStartToken()->isUnsigned() == funcarg->typeStartToken()->isUnsigned() &&
                         callarg->typeStartToken()->isLong() == funcarg->typeStartToken()->isLong())) {
                        same++;
                    } else if (funcargptr && funcarg->typeStartToken()->str() == "void") {
                        fallback1++;
                    } else {
                        // can't match so remove this function from possible matches
                        matches.erase(matches.begin() + i);
                        erased = true;
                        break;
                    }
                }
            }

            // check for a match with a numeric literal
            else if (Token::Match(arguments[j], "%num% ,|)")) {
                if (MathLib::isInt(arguments[j]->str()) && (!funcarg->isPointer() || MathLib::isNullValue(arguments[j]->str()))) {
                    bool exactMatch = false;
                    if (arguments[j]->str().find("ll") != std::string::npos ||
                        arguments[j]->str().find("LL") != std::string::npos) {
                        if (arguments[j]->str().find('u') != std::string::npos ||
                            arguments[j]->str().find('U') != std::string::npos) {
                            if (funcarg->typeStartToken()->isLong() &&
                                funcarg->typeStartToken()->isUnsigned() &&
                                funcarg->typeStartToken()->str() == "long") {
                                exactMatch = true;
                            }
                        } else {
                            if (funcarg->typeStartToken()->isLong() &&
                                !funcarg->typeStartToken()->isUnsigned() &&
                                funcarg->typeStartToken()->str() == "long") {
                                exactMatch = true;
                            }
                        }
                    } else if (arguments[j]->str().find('l') != std::string::npos ||
                               arguments[j]->str().find('L') != std::string::npos) {
                        if (arguments[j]->str().find('u') != std::string::npos ||
                            arguments[j]->str().find('U') != std::string::npos) {
                            if (!funcarg->typeStartToken()->isLong() &&
                                funcarg->typeStartToken()->isUnsigned() &&
                                funcarg->typeStartToken()->str() == "long") {
                                exactMatch = true;
                            }
                        } else {
                            if (!funcarg->typeStartToken()->isLong() &&
                                !funcarg->typeStartToken()->isUnsigned() &&
                                funcarg->typeStartToken()->str() == "long") {
                                exactMatch = true;
                            }
                        }
                    } else if (arguments[j]->str().find('u') != std::string::npos ||
                               arguments[j]->str().find('U') != std::string::npos) {
                        if (funcarg->typeStartToken()->isUnsigned() &&
                            funcarg->typeStartToken()->str() == "int") {
                            exactMatch = true;
                        } else if (Token::Match(funcarg->typeStartToken(), "char|short")) {
                            exactMatch = true;
                        }
                    } else {
                        if (Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long")) {
                            exactMatch = true;
                        }
                    }

                    if (exactMatch)
                        if (funcarg->isPointer())
                            fallback2++;
                        else
                            same++;
                    else {
                        if (funcarg->isPointer() || Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long"))
                            fallback1++;
                        else if (Token::Match(funcarg->typeStartToken(), "float|double"))
                            fallback2++;
                    }
                } else if (!funcarg->isPointer()) {
                    bool exactMatch = false;
                    if (arguments[j]->str().find('f') != std::string::npos ||
                        arguments[j]->str().find('F') != std::string::npos) {
                        if (funcarg->typeStartToken()->str() == "float") {
                            exactMatch = true;
                        }
                    } else if (arguments[j]->str().find('l') != std::string::npos ||
                               arguments[j]->str().find('L') != std::string::npos) {
                        if (funcarg->typeStartToken()->isLong() &&
                            funcarg->typeStartToken()->str() == "double")  {
                            exactMatch = true;
                        }
                    } else {
                        if (!funcarg->typeStartToken()->isLong() &&
                            funcarg->typeStartToken()->str() == "double") {
                            exactMatch = true;
                        }
                    }
                    if (exactMatch)
                        same++;
                    else {
                        if (Token::Match(funcarg->typeStartToken(), "float|double"))
                            fallback1++;
                        else if (Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long"))
                            fallback2++;
                    }
                }
            }

            // check for a match with a string literal
            else if (Token::Match(arguments[j], "%str% ,|)")) {
                if (funcarg->typeStartToken() != funcarg->typeEndToken() &&
                    ((!arguments[j]->isLong() && Token::simpleMatch(funcarg->typeStartToken(), "char *")) ||
                     (arguments[j]->isLong() && Token::simpleMatch(funcarg->typeStartToken(), "wchar_t *"))))
                    same++;
                else if (Token::simpleMatch(funcarg->typeStartToken(), "void *"))
                    fallback1++;
                else if (funcarg->isStlStringType())
                    fallback2++;
            }

            // check for a match with a char literal
            else if (!funcarg->isArrayOrPointer() && Token::Match(arguments[j], "%char% ,|)")) {
                if (arguments[j]->isLong() && funcarg->typeStartToken()->str() == "wchar_t")
                    same++;
                else if (!arguments[j]->isLong() && funcarg->typeStartToken()->str() == "char")
                    same++;
                else if (Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long"))
                    fallback1++;
            }

            // check for a match with a boolean literal
            else if (!funcarg->isArrayOrPointer() && Token::Match(arguments[j], "%bool% ,|)")) {
                if (Token::Match(funcarg->typeStartToken(), "bool|_Bool"))
                    same++;
                else if (Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long"))
                    fallback1++;
            }

            // check for a match with nullptr
            else if (funcarg->isPointer() && Token::Match(arguments[j], "nullptr|NULL ,|)")) {
                same++;
            }

            // check that function argument type is not mismatching
            else if (funcarg->isReference() && arguments[j]->str() == "&") {
                // can't match so remove this function from possible matches
                matches.erase(matches.begin() + i);
                erased = true;
                break;
            }

            // Try to evaluate the apparently more complex expression
            else {
                const Token* argtok = arguments[j];
                while (argtok->astParent() && argtok->astParent() != tok->next() && argtok->astParent()->str() != ",") {
                    argtok = argtok->astParent();
                }
                if (argtok && argtok->valueType() && !funcarg->isArrayOrPointer()) { // TODO: Pointers
                    if (argtok->valueType()->type == ValueType::BOOL) {
                        if (funcarg->typeStartToken()->str() == "bool")
                            same++;
                        else if (Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long"))
                            fallback1++;
                    } else if (argtok->valueType()->isIntegral()) {
                        if (Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long"))
                            same++;
                        else if (Token::Match(funcarg->typeStartToken(), "float|double"))
                            fallback1++;
                    } else if (argtok->valueType()->isFloat()) {
                        if (Token::Match(funcarg->typeStartToken(), "float|double"))
                            same++;
                        else if (Token::Match(funcarg->typeStartToken(), "wchar_t|char|short|int|long"))
                            fallback1++;
                    }
                } else {
                    while (Token::Match(argtok, ".|::"))
                        argtok = argtok->astOperand2();

                    if (argtok) {
                        const Variable * callarg = check->getVariableFromVarId(argtok->varId());
                        checkVariableCallMatch(callarg, funcarg, same, fallback1, fallback2);
                    }
                }
            }
        }

        size_t hasToBe = func->isVariadic() ? (func->argCount() - 1) : args;

        // check if all arguments matched
        if (same == hasToBe) {
            if (constFallback)
                fallback1Func = func;
            else
                return func;
        }

        else if (!fallback1Func) {
            if (same + fallback1 == hasToBe)
                fallback1Func = func;
            else if (!fallback2Func && same + fallback2 + fallback1 == hasToBe)
                fallback2Func = func;
        }

        if (!erased)
            ++i;
    }

    // Fallback cases
    if (fallback1Func)
        return fallback1Func;

    if (fallback2Func)
        return fallback2Func;

    // Only one candidate left
    if (matches.size() == 1)
        return matches[0];

    return nullptr;
}

//---------------------------------------------------------------------------

const Function* SymbolDatabase::findFunction(const Token *tok) const
{
    // find the scope this function is in
    const Scope *currScope = tok->scope();
    while (currScope && currScope->isExecutable()) {
        if (currScope->functionOf)
            currScope = currScope->functionOf;
        else
            currScope = currScope->nestedIn;
    }

    // check for a qualified name and use it when given
    if (tok->strAt(-1) == "::") {
        // find start of qualified function name
        const Token *tok1 = tok;

        while (Token::Match(tok1->tokAt(-2), "%type% ::"))
            tok1 = tok1->tokAt(-2);

        // check for global scope
        if (tok1->strAt(-1) == "::") {
            currScope = &scopeList.front();

            currScope = currScope->findRecordInNestedList(tok1->str());
        }

        // find start of qualification
        else {
            while (currScope) {
                if (currScope->className == tok1->str())
                    break;
                else {
                    const Scope *scope = currScope->findRecordInNestedList(tok1->str());

                    if (scope) {
                        currScope = scope;
                        break;
                    } else
                        currScope = currScope->nestedIn;
                }
            }
        }

        if (currScope) {
            while (currScope && !Token::Match(tok1, "%type% :: %any% (")) {
                tok1 = tok1->tokAt(2);
                currScope = currScope->findRecordInNestedList(tok1->str());
            }

            tok1 = tok1->tokAt(2);

            if (currScope && tok1)
                return currScope->findFunction(tok1);
        }
    }

    // check for member function
    else if (Token::Match(tok->tokAt(-2), "!!this .")) {
        const Token *tok1 = tok->tokAt(-2);
        if (Token::Match(tok1, "%var% .")) {
            const Variable *var = getVariableFromVarId(tok1->varId());
            if (var && var->typeScope())
                return var->typeScope()->findFunction(tok, var->isConst());
        }
    }

    // check in enclosing scopes
    else {
        while (currScope) {
            const Function *func = currScope->findFunction(tok);
            if (func)
                return func;
            currScope = currScope->nestedIn;
        }
    }
    return 0;
}

//---------------------------------------------------------------------------

const Scope *SymbolDatabase::findScopeByName(const std::string& name) const
{
    for (std::list<Scope>::const_iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
        if (it->className == name)
            return &*it;
    }
    return 0;
}

//---------------------------------------------------------------------------

Scope *Scope::findInNestedList(const std::string & name)
{
    std::list<Scope *>::iterator it;

    for (it = nestedList.begin(); it != nestedList.end(); ++it) {
        if ((*it)->className == name)
            return (*it);
    }
    return 0;
}

//---------------------------------------------------------------------------

const Scope *Scope::findRecordInNestedList(const std::string & name) const
{
    std::list<Scope *>::const_iterator it;

    for (it = nestedList.begin(); it != nestedList.end(); ++it) {
        if ((*it)->className == name && (*it)->type != eFunction)
            return (*it);
    }
    return 0;
}

//---------------------------------------------------------------------------

const Type* Scope::findType(const std::string & name) const
{
    std::list<Type*>::const_iterator it;

    for (it = definedTypes.begin(); it != definedTypes.end(); ++it) {
        if ((*it)->name() == name)
            return (*it);
    }
    return 0;
}

//---------------------------------------------------------------------------

Scope *Scope::findInNestedListRecursive(const std::string & name)
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

bool SymbolDatabase::isCPP() const
{
    return _tokenizer->isCPP();
}

//---------------------------------------------------------------------------

const Scope *SymbolDatabase::findScope(const Token *tok, const Scope *startScope) const
{
    const Scope *scope = nullptr;
    // absolute path
    if (tok->str() == "::") {
        tok = tok->next();
        scope = &scopeList.front();
    }
    // relative path
    else if (tok->isName()) {
        scope = startScope;
    }

    while (scope && tok && tok->isName()) {
        if (tok->strAt(1) == "::") {
            scope = scope->findRecordInNestedList(tok->str());
            tok = tok->tokAt(2);
        } else
            return scope->findRecordInNestedList(tok->str());
    }

    // not a valid path
    return 0;
}

//---------------------------------------------------------------------------

const Type* SymbolDatabase::findType(const Token *startTok, const Scope *startScope) const
{
    // skip over struct or union
    if (Token::Match(startTok, "struct|union"))
        startTok = startTok->next();

    // type same as scope
    if (startTok->str() == startScope->className && startScope->isClassOrStruct() && startTok->strAt(1) != "::")
        return startScope->definedType;

    // absolute path - directly start in global scope
    if (startTok->str() == "::") {
        startTok = startTok->next();
        startScope = &scopeList.front();
    }

    const Token* tok = startTok;
    const Scope* scope = startScope;

    while (scope && tok && tok->isName()) {
        if (tok->strAt(1) == "::") {
            scope = scope->findRecordInNestedList(tok->str());
            if (scope) {
                tok = tok->tokAt(2);
            } else {
                startScope = startScope->nestedIn;
                if (!startScope)
                    break;
                scope = startScope;
                tok = startTok;
            }
        } else
            return scope->findType(tok->str());
    }

    // not a valid path
    return 0;
}

//---------------------------------------------------------------------------

const Type* SymbolDatabase::findTypeInNested(const Token *startTok, const Scope *startScope) const
{
    // skip over struct or union
    if (Token::Match(startTok, "struct|union|enum"))
        startTok = startTok->next();

    // type same as scope
    if (startTok->str() == startScope->className && startScope->isClassOrStruct())
        return startScope->definedType;

    bool hasPath = false;

    // absolute path - directly start in global scope
    if (startTok->str() == "::") {
        hasPath = true;
        startTok = startTok->next();
        startScope = &scopeList.front();
    }

    const Token* tok = startTok;
    const Scope* scope = startScope;

    while (scope && tok && tok->isName()) {
        if (tok->strAt(1) == "::") {
            hasPath = true;
            scope = scope->findRecordInNestedList(tok->str());
            if (scope) {
                tok = tok->tokAt(2);
            } else {
                startScope = startScope->nestedIn;
                if (!startScope)
                    break;
                scope = startScope;
                tok = startTok;
            }
        } else {
            const Type * type = scope->findType(tok->str());
            if (hasPath || type)
                return type;
            else {
                scope = scope->nestedIn;
                if (!scope)
                    break;
            }
        }
    }

    // not a valid path
    return 0;
}

//---------------------------------------------------------------------------

const Scope * SymbolDatabase::findNamespace(const Token * tok, const Scope * scope) const
{
    const Scope * s = findScope(tok, scope);

    if (s)
        return s;
    else if (scope->nestedIn)
        return findNamespace(tok, scope->nestedIn);

    return 0;
}

//---------------------------------------------------------------------------

Function * SymbolDatabase::findFunctionInScope(const Token *func, const Scope *ns)
{
    const Function * function = nullptr;
    const bool destructor = func->strAt(-1) == "~";

    for (std::multimap<std::string, const Function *>::const_iterator it = ns->functionMap.find(func->str());
         it != ns->functionMap.end() && it->first == func->str(); ++it) {

        if (Function::argsMatch(ns, func->tokAt(2), it->second->argDef->next(), emptyString, 0) &&
            it->second->isDestructor() == destructor) {
            function = it->second;
            break;
        }
    }

    if (!function) {
        const Scope * scope = ns->findRecordInNestedList(func->str());
        if (scope && func->strAt(1) == "::") {
            func = func->tokAt(2);
            if (func->str() == "~")
                func = func->next();
            function = findFunctionInScope(func, scope);
        }
    }

    return const_cast<Function *>(function);
}

//---------------------------------------------------------------------------

namespace {
    const std::set<std::string> c_keywords = make_container< std::set<std::string> >() <<
            "_Bool" << "auto" << "break" << "case" << "char" << "const" << "continue" << "default" << "do" <<
            "double" << "else" << "enum" << "extern" << "float" << "for" << "goto" << "if" << "inline" <<
            "int" << "long" << "register" << "restrict" << "return" << "short" << "signed" << "sizeof" <<
            "static" << "struct" << "switch" << "typedef" << "union" << "unsigned" << "void" << "volatile" <<
            "while";
    const std::set<std::string> cpp_keywords = make_container< std::set<std::string> >() <<
            c_keywords <<
            "alignas" << "alignof" << "and" << "and_eq" << "asm" << "auto" << "bitand" << "bitor" << "bool" <<
            "break" << "case" << "catch" << "char" << "class" << "compl" <<
            "concept" << "const" << "constexpr" << "const_cast" << "continue" << "decltype" << "default" <<
            "delete" << "do" << "double" << "dynamic_cast" << "else" << "enum" << "explicit" << "export" <<
            "extern" << "false" << "float" << "for" << "friend" << "goto" << "if" << "inline" << "int" << "long" <<
            "mutable" << "namespace" << "new" << "noexcept" << "not" << "not_eq" << "nullptr" << "operator" <<
            "or" << "or_eq" << "private" << "protected" << "public" << "register" << "reinterpret_cast" <<
            "requires" << "return" << "short" << "signed" << "sizeof" << "static" << "static_assert" <<
            "static_cast" << "struct" << "switch" << "template" << "this" << "thread_local" << "throw" <<
            "true" << "try" << "typedef" << "typeid" << "typename" << "union" << "unsigned" << "using" <<
            "virtual" << "void" << "volatile" << "wchar_t" << "while" << "xor" << "xor_eq";
}

bool SymbolDatabase::isReservedName(const std::string& iName) const
{
    if (isCPP())
        return cpp_keywords.find(iName) != cpp_keywords.cend();
    else
        return c_keywords.find(iName) != c_keywords.cend();
}

unsigned int SymbolDatabase::sizeOfType(const Token *type) const
{
    unsigned int size = _tokenizer->sizeOfType(type);

    if (size == 0 && type->type() && type->type()->isEnumType() && type->type()->classScope) {
        size = _settings->sizeof_int;
        const Token * enum_type = type->type()->classScope->enumType;
        if (enum_type)
            size = _tokenizer->sizeOfType(enum_type);
    }

    return size;
}

static const Token * parsedecl(const Token *type, ValueType * const valuetype, ValueType::Sign defaultSignedness, const Settings* settings);

void SymbolDatabase::setValueType(Token *tok, const Variable &var)
{
    ValueType valuetype;
    valuetype.pointer = var.dimensions().size();
    valuetype.typeScope = var.typeScope();
    if (parsedecl(var.typeStartToken(), &valuetype, defaultSignedness, _settings))
        setValueType(tok, valuetype);
}

void SymbolDatabase::setValueType(Token *tok, const Enumerator &enumerator)
{
    ValueType valuetype;
    valuetype.typeScope = enumerator.scope;
    const Token * type = enumerator.scope->enumType;
    if (type) {
        valuetype.type = ValueType::typeFromString(type->str(), type->isLong());
        if (valuetype.type == ValueType::Type::UNKNOWN_TYPE && type->isStandardType())
            valuetype.fromLibraryType(type->str(), _settings);

        if (valuetype.isIntegral()) {
            if (type->isSigned())
                valuetype.sign = ValueType::Sign::SIGNED;
            else if (type->isUnsigned())
                valuetype.sign = ValueType::Sign::UNSIGNED;
            else if (valuetype.type == ValueType::Type::CHAR)
                valuetype.sign = defaultSignedness;
            else
                valuetype.sign = ValueType::Sign::SIGNED;
        }

        setValueType(tok, valuetype);
    } else {
        valuetype.sign = ValueType::SIGNED;
        valuetype.type = ValueType::INT;
        setValueType(tok, valuetype);
    }
}

static void setAutoTokenProperties(Token * const autoTok)
{
    const ValueType *valuetype = autoTok->valueType();
    if (valuetype->isIntegral() || valuetype->isFloat())
        autoTok->isStandardType(true);
}

void SymbolDatabase::setValueType(Token *tok, const ValueType &valuetype)
{
    tok->setValueType(new ValueType(valuetype));
    Token *parent = const_cast<Token *>(tok->astParent());
    if (!parent || parent->valueType())
        return;
    if (!parent->astOperand1())
        return;

    const ValueType *vt1 = parent->astOperand1() ? parent->astOperand1()->valueType() : nullptr;
    const ValueType *vt2 = parent->astOperand2() ? parent->astOperand2()->valueType() : nullptr;

    if (vt1 && Token::Match(parent, "<<|>>")) {
        if (!cpp || (vt2 && vt2->isIntegral()))
            setValueType(parent, *vt1);
        return;
    }

    if (parent->isAssignmentOp()) {
        if (vt1)
            setValueType(parent, *vt1);
        else if (cpp && Token::Match(parent->tokAt(-3), "%var% ; %var% =") && parent->strAt(-3) == parent->strAt(-1)) {
            Token *var1Tok = parent->tokAt(-3);
            Token *autoTok = nullptr;
            if (Token::Match(var1Tok->tokAt(-2), "[;{}] auto"))
                autoTok = var1Tok->previous();
            else if (Token::Match(var1Tok->tokAt(-3), "[;{}] auto *"))
                autoTok = var1Tok->tokAt(-2);
            if (autoTok) {
                ValueType vt(*vt2);
                if (autoTok->strAt(1) == "*" && vt.pointer)
                    vt.pointer--;
                if (autoTok->strAt(-1) == "const")
                    vt.constness |= 1;
                setValueType(autoTok, vt);
                setAutoTokenProperties(autoTok);
                setValueType(var1Tok, *vt2);
                setValueType(parent->previous(), *vt2);
                const Variable *var = parent->previous()->variable();
                if (var) {
                    const_cast<Variable *>(var)->setFlags(*vt2);
                    if (vt2->typeScope && vt2->typeScope->definedType) {
                        const_cast<Variable *>(var)->type(vt2->typeScope->definedType);
                        if (autoTok->valueType()->pointer == 0)
                            autoTok->type(vt2->typeScope->definedType);
                    }
                }
            }
        }
        return;
    }

    if (parent->str() == "[" && (!cpp || parent->astOperand1() == tok) && valuetype.pointer > 0U && !Token::Match(parent->previous(), "[{,]")) {
        const Token *op1 = parent->astOperand1();
        while (op1 && op1->str() == "[")
            op1 = op1->astOperand1();

        ValueType vt(valuetype);
        // the "[" is a dereference unless this is a variable declaration
        if (!(op1 && op1->variable() && op1->variable()->nameToken() == op1))
            vt.pointer -= 1U;
        setValueType(parent, vt);
        return;
    }
    if (Token::Match(parent->previous(), "%name% (") && parent->astOperand1() == tok && valuetype.pointer > 0U) {
        ValueType vt(valuetype);
        vt.pointer -= 1U;
        setValueType(parent, vt);
        return;
    }
    if (parent->str() == "*" && !parent->astOperand2() && valuetype.pointer > 0U) {
        ValueType vt(valuetype);
        vt.pointer -= 1U;
        setValueType(parent, vt);
        return;
    }
    if (parent->str() == "&" && !parent->astOperand2()) {
        ValueType vt(valuetype);
        vt.pointer += 1U;
        setValueType(parent, vt);
        return;
    }

    if ((parent->str() == "." || parent->str() == "::") &&
        parent->astOperand2() && parent->astOperand2()->isName()) {
        const Variable* var = parent->astOperand2()->variable();
        if (!var && valuetype.typeScope && vt1) {
            const std::string &name = parent->astOperand2()->str();
            const Scope *typeScope = vt1->typeScope;
            if (!typeScope)
                return;
            for (std::list<Variable>::const_iterator it = typeScope->varlist.begin(); it != typeScope->varlist.end(); ++it) {
                if (it->nameToken()->str() == name) {
                    var = &*it;
                    break;
                }
            }
        }
        if (var)
            setValueType(parent, *var);
        return;
    }

    // range for loop, auto
    if (vt2 &&
        parent->str() == ":" &&
        Token::Match(parent->astParent(), "( const| auto *|&| %var% :") &&
        !parent->previous()->valueType() &&
        Token::simpleMatch(parent->astParent()->astOperand1(), "for")) {
        const bool isconst = Token::simpleMatch(parent->astParent()->next(), "const");
        Token * const autoToken = const_cast<Token *>(parent->astParent()->tokAt(isconst ? 2 : 1));
        if (vt2->pointer) {
            ValueType autovt(*vt2);
            autovt.pointer--;
            autovt.constness = 0;
            setValueType(autoToken, autovt);
            setAutoTokenProperties(autoToken);
            ValueType varvt(*vt2);
            varvt.pointer--;
            if (isconst)
                varvt.constness |= 1;
            setValueType(parent->previous(), varvt);
            const_cast<Variable *>(parent->previous()->variable())->setFlags(varvt);
            if (vt2->typeScope && vt2->typeScope->definedType) {
                const_cast<Variable *>(parent->previous()->variable())->type(vt2->typeScope->definedType);
                autoToken->type(vt2->typeScope->definedType);
            }
        } else if (vt2->container) {
            // TODO: Determine exact type of RHS
            const Token *typeStart = parent->astOperand2();
            while (typeStart) {
                if (typeStart->variable())
                    typeStart = typeStart->variable()->typeStartToken();
                else if (typeStart->str() == "(" && typeStart->previous() && typeStart->previous()->function())
                    typeStart = typeStart->previous()->function()->retDef;
                else
                    break;
            }
            // TODO: Get type better
            if (Token::Match(typeStart, "std :: %type% < %type% *| *| >")) {
                ValueType autovt;
                if (parsedecl(typeStart->tokAt(4), &autovt, defaultSignedness, _settings)) {
                    setValueType(autoToken, autovt);
                    setAutoTokenProperties(autoToken);
                    ValueType varvt(autovt);
                    if (isconst)
                        varvt.constness |= 1;
                    setValueType(parent->previous(), varvt);
                    const_cast<Variable *>(parent->previous()->variable())->setFlags(varvt);
                    const Type * type = typeStart->tokAt(4)->type();
                    if (type && type->classScope && type->classScope->definedType) {
                        autoToken->type(type->classScope->definedType);
                        const_cast<Variable *>(parent->previous()->variable())->type(type->classScope->definedType);
                    }
                }
            }
        }
    }

    if (!vt1)
        return;
    if (parent->astOperand2() && !vt2)
        return;

    bool ternary = parent->str() == ":" && parent->astParent() && parent->astParent()->str() == "?";
    if (ternary)
        parent = const_cast<Token*>(parent->astParent());

    if (ternary || parent->isArithmeticalOp() || parent->tokType() == Token::eIncDecOp) {
        if (vt1->pointer != 0U && vt2 && vt2->pointer == 0U) {
            setValueType(parent, *vt1);
            return;
        }

        if (vt1->pointer == 0U && vt2 && vt2->pointer != 0U) {
            setValueType(parent, *vt2);
            return;
        }

        if (vt1->pointer != 0U) {
            if (ternary || parent->tokType() == Token::eIncDecOp) // result is pointer
                setValueType(parent, *vt1);
            else // result is pointer diff
                setValueType(parent, ValueType(ValueType::Sign::SIGNED, ValueType::Type::INT, 0U, 0U, "ptrdiff_t"));
            return;
        }

        if (vt1->type == ValueType::Type::LONGDOUBLE || (vt2 && vt2->type == ValueType::Type::LONGDOUBLE)) {
            setValueType(parent, ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::LONGDOUBLE, 0U));
            return;
        }
        if (vt1->type == ValueType::Type::DOUBLE || (vt2 && vt2->type == ValueType::Type::DOUBLE)) {
            setValueType(parent, ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::DOUBLE, 0U));
            return;
        }
        if (vt1->type == ValueType::Type::FLOAT || (vt2 && vt2->type == ValueType::Type::FLOAT)) {
            setValueType(parent, ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::FLOAT, 0U));
            return;
        }
    }

    if (vt1->isIntegral() && vt1->pointer == 0U &&
        (!vt2 || (vt2->isIntegral() && vt2->pointer == 0U)) &&
        (ternary || parent->isArithmeticalOp() || parent->tokType() == Token::eBitOp || parent->tokType() == Token::eIncDecOp || parent->isAssignmentOp())) {

        ValueType vt;
        if (!vt2 || vt1->type > vt2->type) {
            vt.type = vt1->type;
            vt.sign = vt1->sign;
            vt.originalTypeName = vt1->originalTypeName;
        } else if (vt1->type == vt2->type) {
            vt.type = vt1->type;
            if (vt1->sign == ValueType::Sign::UNSIGNED || vt2->sign == ValueType::Sign::UNSIGNED)
                vt.sign = ValueType::Sign::UNSIGNED;
            else if (vt1->sign == ValueType::Sign::UNKNOWN_SIGN || vt2->sign == ValueType::Sign::UNKNOWN_SIGN)
                vt.sign = ValueType::Sign::UNKNOWN_SIGN;
            else
                vt.sign = ValueType::Sign::SIGNED;
            vt.originalTypeName = (vt1->originalTypeName.empty() ? vt2 : vt1)->originalTypeName;
        } else {
            vt.type = vt2->type;
            vt.sign = vt2->sign;
            vt.originalTypeName = vt2->originalTypeName;
        }
        if (vt.type < ValueType::Type::INT) {
            vt.type = ValueType::Type::INT;
            vt.sign = ValueType::Sign::SIGNED;
            vt.originalTypeName.clear();
        }

        setValueType(parent, vt);
        return;
    }
}

static const Token * parsedecl(const Token *type, ValueType * const valuetype, ValueType::Sign defaultSignedness, const Settings* settings)
{
    const unsigned int pointer0 = valuetype->pointer;
    while (Token::Match(type->previous(), "%name%"))
        type = type->previous();
    valuetype->sign = ValueType::Sign::UNKNOWN_SIGN;
    if (!valuetype->typeScope)
        valuetype->type = ValueType::Type::UNKNOWN_TYPE;
    else if (valuetype->typeScope->type == Scope::eEnum) {
        const Token * enum_type = valuetype->typeScope->enumType;
        if (enum_type) {
            if (enum_type->isSigned())
                valuetype->sign = ValueType::Sign::SIGNED;
            else if (enum_type->isUnsigned())
                valuetype->sign = ValueType::Sign::UNSIGNED;
            else
                valuetype->sign = defaultSignedness;
            ValueType::Type t = ValueType::typeFromString(enum_type->str(), enum_type->isLong());
            if (t != ValueType::Type::UNKNOWN_TYPE)
                valuetype->type = t;
            else if (enum_type->isStandardType())
                valuetype->fromLibraryType(enum_type->str(), settings);
        } else
            valuetype->type = ValueType::Type::INT;
    } else
        valuetype->type = ValueType::Type::RECORD;
    while (Token::Match(type, "%name%|*|&|::") && !type->variable()) {
        if (type->isSigned())
            valuetype->sign = ValueType::Sign::SIGNED;
        else if (type->isUnsigned())
            valuetype->sign = ValueType::Sign::UNSIGNED;
        if (type->str() == "const")
            valuetype->constness |= (1 << (valuetype->pointer - pointer0));
        else if (Token::Match(type, "%name% :: %name%")) {
            const Library::Container *container = settings->library.detectContainer(type);
            if (container) {
                valuetype->type = ValueType::Type::CONTAINER;
                valuetype->container = container;
            } else {
                std::string typestr;
                const Token *end = type;
                while (Token::Match(end, "%name% :: %name%")) {
                    typestr += end->str() + "::";
                    end = end->tokAt(2);
                }
                typestr += end->str();
                if (valuetype->fromLibraryType(typestr, settings))
                    type = end;
            }
        } else if (ValueType::Type::UNKNOWN_TYPE != ValueType::typeFromString(type->str(), type->isLong()))
            valuetype->type = ValueType::typeFromString(type->str(), type->isLong());
        else if (type->str() == "auto") {
            if (!type->valueType())
                return nullptr;
            const ValueType *vt = type->valueType();
            valuetype->type = vt->type;
            valuetype->pointer = vt->pointer;
            if (vt->sign != ValueType::Sign::UNKNOWN_SIGN)
                valuetype->sign = vt->sign;
            valuetype->constness = vt->constness;
            while (Token::Match(type, "%name%|*|&|::") && !type->variable())
                type = type->next();
            break;
        } else if (!valuetype->typeScope && (type->str() == "struct" || type->str() == "enum"))
            valuetype->type = type->str() == "struct" ? ValueType::Type::RECORD : ValueType::Type::NONSTD;
        else if (!valuetype->typeScope && type->type() && type->type()->classScope) {
            valuetype->type = ValueType::Type::RECORD;
            valuetype->typeScope = type->type()->classScope;
        } else if (type->isName() && valuetype->sign != ValueType::Sign::UNKNOWN_SIGN && valuetype->pointer == 0U)
            return nullptr;
        else if (type->str() == "*")
            valuetype->pointer++;
        else if (type->isStandardType())
            valuetype->fromLibraryType(type->str(), settings);
        if (!type->originalName().empty())
            valuetype->originalTypeName = type->originalName();
        type = type->next();
    }

    // Set signedness for integral types..
    if (valuetype->isIntegral() && valuetype->sign == ValueType::Sign::UNKNOWN_SIGN) {
        if (valuetype->type == ValueType::Type::CHAR)
            valuetype->sign = defaultSignedness;
        else if (valuetype->type >= ValueType::Type::SHORT)
            valuetype->sign = ValueType::Sign::SIGNED;
    }

    return (type && (valuetype->type != ValueType::Type::UNKNOWN_TYPE || valuetype->pointer > 0)) ? type : nullptr;
}

static const Scope *getClassScope(const Token *tok)
{
    return tok && tok->valueType() && tok->valueType()->typeScope && tok->valueType()->typeScope->isClassOrStruct() ?
           tok->valueType()->typeScope :
           nullptr;
}

static const Function *getOperatorFunction(const Token * const tok)
{
    const std::string functionName("operator" + tok->str());
    std::multimap<std::string, const Function *>::const_iterator it;
    const Scope *classScope;

    classScope = getClassScope(tok->astOperand1());
    if (classScope) {
        it = classScope->functionMap.find(functionName);
        if (it != classScope->functionMap.end())
            return it->second;
    }

    classScope = getClassScope(tok->astOperand2());
    if (classScope) {
        it = classScope->functionMap.find(functionName);
        if (it != classScope->functionMap.end())
            return it->second;
    }

    return nullptr;
}

void SymbolDatabase::setValueTypeInTokenList()
{
    Token * tokens = const_cast<Tokenizer *>(_tokenizer)->list.front();

    for (Token *tok = tokens; tok; tok = tok->next())
        tok->setValueType(nullptr);

    for (Token *tok = tokens; tok; tok = tok->next()) {
        if (tok->isNumber()) {
            if (MathLib::isFloat(tok->str())) {
                ValueType::Type type = ValueType::Type::DOUBLE;
                const char suffix = tok->str().back();
                if (suffix == 'f' || suffix == 'F')
                    type = ValueType::Type::FLOAT;
                else if (suffix == 'L' || suffix == 'l')
                    type = ValueType::Type::LONGDOUBLE;
                setValueType(tok, ValueType(ValueType::Sign::UNKNOWN_SIGN, type, 0U));
            } else if (MathLib::isInt(tok->str())) {
                bool unsignedSuffix = (tok->str().find_last_of("uU") != std::string::npos);
                ValueType::Sign sign = unsignedSuffix ? ValueType::Sign::UNSIGNED : ValueType::Sign::SIGNED;
                ValueType::Type type;
                const MathLib::bigint value = MathLib::toLongNumber(tok->str());
                if (_settings->platformType == cppcheck::Platform::Unspecified)
                    type = ValueType::Type::INT;
                else if (_settings->isIntValue(unsignedSuffix ? (value >> 1) : value))
                    type = ValueType::Type::INT;
                else if (_settings->isLongValue(unsignedSuffix ? (value >> 1) : value))
                    type = ValueType::Type::LONG;
                else
                    type = ValueType::Type::LONGLONG;
                if (MathLib::isIntHex(tok->str()))
                    sign = ValueType::Sign::UNSIGNED;
                for (std::size_t pos = tok->str().size() - 1U; pos > 0U; --pos) {
                    const char suffix = tok->str()[pos];
                    if (suffix == 'u' || suffix == 'U')
                        sign = ValueType::Sign::UNSIGNED;
                    else if (suffix == 'l' || suffix == 'L')
                        type = (type == ValueType::Type::INT) ? ValueType::Type::LONG : ValueType::Type::LONGLONG;
                    else if (pos > 2U && suffix == '4' && tok->str()[pos - 1] == '6' && tok->str()[pos - 2] == 'i') {
                        type = ValueType::Type::LONGLONG;
                        pos -= 2;
                    } else break;
                }
                setValueType(tok, ValueType(sign, type, 0U));
            }
        } else if (tok->isComparisonOp() || tok->tokType() == Token::eLogicalOp) {
            if (cpp && tok->isComparisonOp() && (getClassScope(tok->astOperand1()) || getClassScope(tok->astOperand2()))) {
                const Function *function = getOperatorFunction(tok);
                if (function) {
                    ValueType vt;
                    parsedecl(function->retDef, &vt, defaultSignedness, _settings);
                    setValueType(tok, vt);
                    continue;
                }
            }
            setValueType(tok, ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::BOOL, 0U));
        } else if (tok->tokType() == Token::eChar)
            setValueType(tok, ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::CHAR, 0U));
        else if (tok->tokType() == Token::eString) {
            ValueType valuetype(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::CHAR, 1U, 1U);
            if (tok->isLong()) {
                valuetype.originalTypeName = "wchar_t";
                valuetype.type = ValueType::Type::SHORT;
            }
            setValueType(tok, valuetype);
        } else if (tok->str() == "(") {
            // cast
            if (!tok->astOperand2() && Token::Match(tok, "( %name%")) {
                ValueType valuetype;
                if (Token::simpleMatch(parsedecl(tok->next(), &valuetype, defaultSignedness, _settings), ")"))
                    setValueType(tok, valuetype);
            }

            // C++ cast
            if (tok->astOperand2() && Token::Match(tok->astOperand1(), "static_cast|const_cast|dynamic_cast|reinterpret_cast < %name%") && tok->astOperand1()->linkAt(1)) {
                ValueType valuetype;
                if (Token::simpleMatch(parsedecl(tok->astOperand1()->tokAt(2), &valuetype, defaultSignedness, _settings), ">"))
                    setValueType(tok, valuetype);
            }

            // function
            else if (tok->previous() && tok->previous()->function() && tok->previous()->function()->retDef) {
                ValueType valuetype;
                if (parsedecl(tok->previous()->function()->retDef, &valuetype, defaultSignedness, _settings))
                    setValueType(tok, valuetype);
            }

            else if (Token::simpleMatch(tok->previous(), "sizeof (")) {
                // TODO: use specified size_t type
                ValueType valuetype(ValueType::Sign::UNSIGNED, ValueType::Type::LONG, 0U);
                valuetype.originalTypeName = "size_t";
                setValueType(tok, valuetype);

                if (Token::Match(tok, "( %type% %type%| *| *| )")) {
                    ValueType vt;
                    if (parsedecl(tok->next(), &vt, defaultSignedness, _settings)) {
                        setValueType(tok->next(), vt);
                    }
                }
            }

            // library function
            else if (tok->previous()) {
                const std::string& typestr(_settings->library.returnValueType(tok->previous()));
                if (typestr.empty() || typestr == "iterator") {
                    if (Token::simpleMatch(tok->astOperand1(), ".") &&
                        tok->astOperand1()->astOperand1() &&
                        tok->astOperand1()->astOperand2() &&
                        tok->astOperand1()->astOperand1()->valueType() &&
                        tok->astOperand1()->astOperand1()->valueType()->container) {
                        const Library::Container *cont = tok->astOperand1()->astOperand1()->valueType()->container;
                        std::map<std::string, Library::Container::Function>::const_iterator it = cont->functions.find(tok->astOperand1()->astOperand2()->str());
                        if (it != cont->functions.end()) {
                            if (it->second.yield == Library::Container::Yield::START_ITERATOR ||
                                it->second.yield == Library::Container::Yield::END_ITERATOR ||
                                it->second.yield == Library::Container::Yield::ITERATOR) {
                                ValueType vt;
                                vt.type = ValueType::Type::ITERATOR;
                                vt.container = cont;
                                setValueType(tok, vt);
                            }
                        }
                    }
                    continue;
                }
                TokenList tokenList(_settings);
                std::istringstream istr(typestr+";");
                if (tokenList.createTokens(istr)) {
                    ValueType vt;
                    if (parsedecl(tokenList.front(), &vt, defaultSignedness, _settings)) {
                        setValueType(tok, vt);
                    }
                }
            }
        } else if (tok->variable()) {
            setValueType(tok, *tok->variable());
        } else if (tok->enumerator()) {
            setValueType(tok, *tok->enumerator());
        } else if (cpp && tok->str() == "new") {
            const Token *typeTok = tok->next();
            if (Token::Match(typeTok, "( std| ::| nothrow )"))
                typeTok = typeTok->link()->next();
            std::string typestr;
            while (Token::Match(typeTok, "%name% :: %name%")) {
                typestr += typeTok->str() + "::";
                typeTok = typeTok->tokAt(2);
            }
            if (!Token::Match(typeTok, "%type% ;|[|("))
                return;
            typestr += typeTok->str();
            ValueType vt;
            vt.pointer = 1;
            if (typeTok->type() && typeTok->type()->classScope) {
                vt.type = ValueType::Type::RECORD;
                vt.typeScope = typeTok->type()->classScope;
            } else {
                vt.type = ValueType::typeFromString(typestr, typeTok->isLong());
                if (vt.type == ValueType::Type::UNKNOWN_TYPE)
                    vt.fromLibraryType(typestr, _settings);
                if (vt.type == ValueType::Type::UNKNOWN_TYPE)
                    return;
                if (typeTok->isUnsigned())
                    vt.sign = ValueType::Sign::UNSIGNED;
                else if (typeTok->isSigned())
                    vt.sign = ValueType::Sign::SIGNED;
                if (vt.sign == ValueType::Sign::UNKNOWN_SIGN && vt.isIntegral())
                    vt.sign = (vt.type == ValueType::Type::CHAR) ? defaultSignedness : ValueType::Sign::SIGNED;
            }
            setValueType(tok, vt);
        }
    }

    // Update functions with new type information.
    createSymbolDatabaseSetFunctionPointers(false);

    // Update auto variables with new type information.
    createSymbolDatabaseSetVariablePointers();
}

ValueType ValueType::parseDecl(const Token *type, const Settings *settings)
{
    ValueType vt;
    parsedecl(type, &vt, settings->defaultSign == 'u' ? Sign::UNSIGNED : Sign::SIGNED, settings);
    return vt;
}

ValueType::Type ValueType::typeFromString(const std::string &typestr, bool longType)
{
    if (typestr == "void")
        return ValueType::Type::VOID;
    if (typestr == "bool" || typestr == "_Bool")
        return ValueType::Type::BOOL;
    if (typestr== "char")
        return ValueType::Type::CHAR;
    if (typestr == "short")
        return ValueType::Type::SHORT;
    if (typestr == "int")
        return ValueType::Type::INT;
    if (typestr == "long")
        return longType ? ValueType::Type::LONGLONG : ValueType::Type::LONG;
    if (typestr == "float")
        return ValueType::Type::FLOAT;
    if (typestr == "double")
        return longType ? ValueType::Type::LONGDOUBLE : ValueType::Type::DOUBLE;
    return ValueType::Type::UNKNOWN_TYPE;
}

bool ValueType::fromLibraryType(const std::string &typestr, const Settings *settings)
{
    const Library::PodType* podtype = settings->library.podtype(typestr);
    if (podtype && (podtype->sign == 's' || podtype->sign == 'u')) {
        if (podtype->size == 1)
            type = ValueType::Type::CHAR;
        else if (podtype->size == settings->sizeof_int)
            type = ValueType::Type::INT;
        else if (podtype->size == settings->sizeof_short)
            type = ValueType::Type::SHORT;
        else if (podtype->size == settings->sizeof_long)
            type = ValueType::Type::LONG;
        else if (podtype->size == settings->sizeof_long_long)
            type = ValueType::Type::LONGLONG;
        else
            type = ValueType::Type::UNKNOWN_INT;
        sign = (podtype->sign == 'u') ? ValueType::UNSIGNED : ValueType::SIGNED;
        return true;
    }

    const Library::PlatformType *platformType = settings->library.platform_type(typestr, settings->platformString());
    if (platformType) {
        if (platformType->_type == "char")
            type = ValueType::Type::CHAR;
        else if (platformType->_type == "short")
            type = ValueType::Type::SHORT;
        else if (platformType->_type == "int")
            type = platformType->_long ? ValueType::Type::LONG : ValueType::Type::INT;
        else if (platformType->_type == "long")
            type = platformType->_long ? ValueType::Type::LONGLONG : ValueType::Type::LONG;
        if (platformType->_signed)
            sign = ValueType::SIGNED;
        else if (platformType->_unsigned)
            sign = ValueType::UNSIGNED;
        if (platformType->_pointer)
            pointer = 1;
        if (platformType->_ptr_ptr)
            pointer = 2;
        if (platformType->_const_ptr)
            constness = 1;
        return true;
    } else if (!podtype && (typestr == "size_t" || typestr == "std::size_t")) {
        originalTypeName = "size_t";
        sign = ValueType::UNSIGNED;
        if (settings->sizeof_size_t == settings->sizeof_long)
            type = ValueType::Type::LONG;
        else if (settings->sizeof_size_t == settings->sizeof_long_long)
            type = ValueType::Type::LONGLONG;
        else if (settings->sizeof_size_t == settings->sizeof_int)
            type = ValueType::Type::INT;
        else
            type = ValueType::Type::UNKNOWN_INT;
        return true;
    }

    return false;
}

std::string ValueType::str() const
{
    std::string ret;
    if (constness & 1)
        ret = " const";
    if (type == VOID)
        ret += " void";
    else if (isIntegral()) {
        if (sign == SIGNED)
            ret += " signed";
        else if (sign == UNSIGNED)
            ret += " unsigned";
        if (type == BOOL)
            ret += " bool";
        else if (type == CHAR)
            ret += " char";
        else if (type == SHORT)
            ret += " short";
        else if (type == INT)
            ret += " int";
        else if (type == LONG)
            ret += " long";
        else if (type == LONGLONG)
            ret += " long long";
        else if (type == UNKNOWN_INT)
            ret += " unknown_int";
    } else if (type == FLOAT)
        ret += " float";
    else if (type == DOUBLE)
        ret += " double";
    else if (type == LONGDOUBLE)
        ret += " long double";
    else if ((type == ValueType::Type::NONSTD || type == ValueType::Type::RECORD) && typeScope) {
        std::string className(typeScope->className);
        const Scope *scope = typeScope->nestedIn;
        while (scope && scope->type != Scope::eGlobal) {
            if (scope->type == Scope::eClass || scope->type == Scope::eStruct || scope->type == Scope::eNamespace)
                className = scope->className + "::" + className;
            scope = scope->nestedIn;
        }
        ret += ' ' + className;
    } else if (type == ValueType::Type::CONTAINER && container) {
        ret += " container(" + container->startPattern + ')';
    } else if (type == ValueType::Type::ITERATOR && container) {
        ret += " iterator(" + container->startPattern + ')';
    }
    for (unsigned int p = 0; p < pointer; p++) {
        ret += " *";
        if (constness & (2 << p))
            ret += " const";
    }
    return ret.empty() ? ret : ret.substr(1);
}
