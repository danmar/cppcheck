/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "astutils.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "keywords.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "templatesimplifier.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream> // IWYU pragma: keep
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
//---------------------------------------------------------------------------

SymbolDatabase::SymbolDatabase(Tokenizer& tokenizer, const Settings& settings, ErrorLogger* errorLogger)
    : mTokenizer(tokenizer), mSettings(settings), mErrorLogger(errorLogger)
{
    if (!mTokenizer.tokens())
        return;

    mIsCpp = isCPP();

    if (mSettings.platform.defaultSign == 's' || mSettings.platform.defaultSign == 'S')
        mDefaultSignedness = ValueType::SIGNED;
    else if (mSettings.platform.defaultSign == 'u' || mSettings.platform.defaultSign == 'U')
        mDefaultSignedness = ValueType::UNSIGNED;
    else
        mDefaultSignedness = ValueType::UNKNOWN_SIGN;

    createSymbolDatabaseFindAllScopes();
    createSymbolDatabaseClassInfo();
    createSymbolDatabaseVariableInfo();
    createSymbolDatabaseCopyAndMoveConstructors();
    createSymbolDatabaseFunctionScopes();
    createSymbolDatabaseClassAndStructScopes();
    createSymbolDatabaseFunctionReturnTypes();
    createSymbolDatabaseNeedInitialization();
    createSymbolDatabaseVariableSymbolTable();
    createSymbolDatabaseSetScopePointers();
    createSymbolDatabaseSetVariablePointers();
    setValueTypeInTokenList(false);
    createSymbolDatabaseSetTypePointers();
    createSymbolDatabaseSetFunctionPointers(true);
    createSymbolDatabaseSetSmartPointerType();
    setValueTypeInTokenList(false);
    createSymbolDatabaseEnums();
    createSymbolDatabaseEscapeFunctions();
    createSymbolDatabaseIncompleteVars();
    createSymbolDatabaseExprIds();
    debugSymbolDatabase();
}

static const Token* skipScopeIdentifiers(const Token* tok)
{
    if (Token::Match(tok, ":: %name%"))
        tok = tok->next();
    while (Token::Match(tok, "%name% ::") ||
           (Token::Match(tok, "%name% <") && Token::Match(tok->linkAt(1), ">|>> ::"))) {
        if (tok->strAt(1) == "::")
            tok = tok->tokAt(2);
        else
            tok = tok->linkAt(1)->tokAt(2);
    }

    return tok;
}

static bool isExecutableScope(const Token* tok)
{
    if (!Token::simpleMatch(tok, "{"))
        return false;
    const Token * tok2 = tok->link()->previous();
    if (Token::simpleMatch(tok2, "; }"))
        return true;
    if (tok2 == tok)
        return false;
    if (Token::simpleMatch(tok2, "} }")) { // inner scope
        const Token* startTok = tok2->link();
        if (Token::Match(startTok->previous(), "do|try|else {"))
            return true;
        if (Token::Match(startTok->previous(), ")|] {"))
            return !findLambdaStartToken(tok2);
        return isExecutableScope(startTok);
    }
    return false;
}

void SymbolDatabase::createSymbolDatabaseFindAllScopes()
{
    // create global scope
    scopeList.emplace_back(this, nullptr, nullptr);

    // pointer to current scope
    Scope *scope = &scopeList.back();

    // Store the ending of init lists
    std::stack<std::pair<const Token*, const Scope*>> endInitList;
    auto inInitList = [&] {
        if (endInitList.empty())
            return false;
        return endInitList.top().second == scope;
    };

    auto addLambda = [this, &scope](const Token* tok, const Token* lambdaEndToken) -> const Token* {
        const Token* lambdaStartToken = lambdaEndToken->link();
        const Token* argStart = lambdaStartToken->astParent();
        const Token* funcStart = Token::simpleMatch(argStart, "[") ? argStart : argStart->astParent();
        const Function* function = addGlobalFunction(scope, tok, argStart, funcStart);
        if (!function)
            mTokenizer.syntaxError(tok);
        return lambdaStartToken;
    };

    // Store current access in each scope (depends on evaluation progress)
    std::map<const Scope*, AccessControl> access;

    // find all scopes
    for (const Token *tok = mTokenizer.tokens(); tok; tok = tok ? tok->next() : nullptr) {
        // #5593 suggested to add here:
        if (mErrorLogger)
            mErrorLogger->reportProgress(mTokenizer.list.getSourceFilePath(),
                                         "SymbolDatabase",
                                         tok->progressValue());
        // Locate next class
        if ((mTokenizer.isCPP() && tok->isKeyword() &&
             ((Token::Match(tok, "class|struct|union|namespace ::| %name% final| {|:|::|<") &&
               !Token::Match(tok->previous(), "new|friend|const|enum|typedef|mutable|volatile|using|)|(|<")) ||
              (Token::Match(tok, "enum class| %name% {") ||
               Token::Match(tok, "enum class| %name% : %name% {"))))
            || (mTokenizer.isC() && tok->isKeyword() && Token::Match(tok, "struct|union|enum %name% {"))) {
            const Token *tok2 = tok->tokAt(2);

            if (tok->strAt(1) == "::")
                tok2 = tok2->next();
            else if (mTokenizer.isCPP() && tok->strAt(1) == "class")
                tok2 = tok2->next();

            while (Token::Match(tok2, ":: %name%"))
                tok2 = tok2->tokAt(2);
            while (Token::Match(tok2, "%name% :: %name%"))
                tok2 = tok2->tokAt(2);

            // skip over template args
            while (tok2 && tok2->str() == "<" && tok2->link()) {
                tok2 = tok2->link()->next();
                while (Token::Match(tok2, ":: %name%"))
                    tok2 = tok2->tokAt(2);
            }

            // skip over final
            if (mTokenizer.isCPP() && Token::simpleMatch(tok2, "final"))
                tok2 = tok2->next();

            // make sure we have valid code
            if (!Token::Match(tok2, "{|:")) {
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
                    else if (Token::Match(tok2, "%name% (") && mTokenizer.isFunctionHead(tok2->next(), "{;"))
                        continue;
                    else if (Token::Match(tok2, "%name% [|="))
                        continue;
                    // skip template
                    else if (Token::simpleMatch(tok2, ";") &&
                             Token::Match(tok->previous(), "template|> class|struct")) {
                        tok = tok2;
                        continue;
                    }
                    // forward declaration
                    else if (Token::simpleMatch(tok2, ";") &&
                             Token::Match(tok, "class|struct|union")) {
                        // TODO: see if it can be used
                        tok = tok2;
                        continue;
                    }
                    // skip constructor
                    else if (Token::simpleMatch(tok2, "(") &&
                             Token::simpleMatch(tok2->link(), ") ;")) {
                        tok = tok2->link()->next();
                        continue;
                    } else
                        throw InternalError(tok2, "SymbolDatabase bailout; unhandled code", InternalError::SYNTAX);
                    continue;
                }
                break; // bail
            }

            const Token * name = tok->next();

            if (name->str() == "class" && name->strAt(-1) == "enum")
                name = name->next();

            Scope *new_scope = findScope(name, scope);

            if (new_scope) {
                // only create base list for classes and structures
                if (new_scope->isClassOrStruct()) {
                    // goto initial '{'
                    if (!new_scope->definedType)
                        mTokenizer.syntaxError(nullptr); // #6808
                    tok2 = new_scope->definedType->initBaseInfo(tok, tok2);
                    // make sure we have valid code
                    if (!tok2) {
                        break;
                    }
                }

                // definition may be different than declaration
                if (mTokenizer.isCPP() && tok->str() == "class") {
                    access[new_scope] = AccessControl::Private;
                    new_scope->type = Scope::eClass;
                } else if (tok->str() == "struct") {
                    access[new_scope] = AccessControl::Public;
                    new_scope->type = Scope::eStruct;
                }

                new_scope->classDef = tok;
                new_scope->setBodyStartEnd(tok2);
                // make sure we have valid code
                if (!new_scope->bodyEnd) {
                    mTokenizer.syntaxError(tok);
                }
                scope = new_scope;
                tok = tok2;
            } else {
                scopeList.emplace_back(this, tok, scope);
                new_scope = &scopeList.back();

                if (tok->str() == "class")
                    access[new_scope] = AccessControl::Private;
                else if (tok->str() == "struct" || tok->str() == "union")
                    access[new_scope] = AccessControl::Public;

                // fill typeList...
                if (new_scope->isClassOrStructOrUnion() || new_scope->type == Scope::eEnum) {
                    Type* new_type = findType(name, scope);
                    if (!new_type) {
                        typeList.emplace_back(new_scope->classDef, new_scope, scope);
                        new_type = &typeList.back();
                        scope->definedTypesMap[new_type->name()] = new_type;
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
                        mTokenizer.syntaxError(tok);
                    }
                } else if (new_scope->type == Scope::eEnum) {
                    if (tok2->str() == ":")
                        tok2 = tok2->tokAt(2);
                }

                new_scope->setBodyStartEnd(tok2);

                // make sure we have valid code
                if (!new_scope->bodyEnd) {
                    mTokenizer.syntaxError(tok);
                }

                if (new_scope->type == Scope::eEnum) {
                    tok2 = new_scope->addEnum(tok, mTokenizer.isCPP());
                    scope->nestedList.push_back(new_scope);

                    if (!tok2)
                        mTokenizer.syntaxError(tok);
                } else {
                    // make the new scope the current scope
                    scope->nestedList.push_back(new_scope);
                    scope = new_scope;
                }

                tok = tok2;
            }
        }

        // Namespace and unknown macro (#3854)
        else if (mTokenizer.isCPP() && tok->isKeyword() &&
                 Token::Match(tok, "namespace %name% %type% (") &&
                 tok->tokAt(2)->isUpperCaseName() &&
                 Token::simpleMatch(tok->linkAt(3), ") {")) {
            scopeList.emplace_back(this, tok, scope);

            Scope *new_scope = &scopeList.back();
            access[new_scope] = AccessControl::Public;

            const Token *tok2 = tok->linkAt(3)->next();

            new_scope->setBodyStartEnd(tok2);

            // make sure we have valid code
            if (!new_scope->bodyEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope->nestedList.push_back(new_scope);
            scope = &scopeList.back();

            tok = tok2;
        }

        // forward declaration
        else if (tok->isKeyword() && Token::Match(tok, "class|struct|union %name% ;") &&
                 tok->strAt(-1) != "friend") {
            if (!findType(tok->next(), scope)) {
                // fill typeList..
                typeList.emplace_back(tok, nullptr, scope);
                Type* new_type = &typeList.back();
                scope->definedTypesMap[new_type->name()] = new_type;
            }
            tok = tok->tokAt(2);
        }

        // using namespace
        else if (mTokenizer.isCPP() && tok->isKeyword() && Token::Match(tok, "using namespace ::| %type% ;|::")) {
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

        // using type alias
        else if (mTokenizer.isCPP() && tok->isKeyword() && Token::Match(tok, "using %name% =")) {
            if (tok->strAt(-1) != ">" && !findType(tok->next(), scope)) {
                // fill typeList..
                typeList.emplace_back(tok, nullptr, scope);
                Type* new_type = &typeList.back();
                scope->definedTypesMap[new_type->name()] = new_type;
            }

            tok = tok->tokAt(3);

            while (tok && tok->str() != ";") {
                if (Token::simpleMatch(tok, "decltype ("))
                    tok = tok->linkAt(1);
                else
                    tok = tok->next();
            }
        }

        // unnamed struct and union
        else if (tok->isKeyword() && Token::Match(tok, "struct|union {") &&
                 Token::Match(tok->next()->link(), "} *|&| %name% ;|[|=")) {
            scopeList.emplace_back(this, tok, scope);

            Scope *new_scope = &scopeList.back();
            access[new_scope] = AccessControl::Public;

            const Token* varNameTok = tok->next()->link()->next();
            if (varNameTok->str() == "*") {
                varNameTok = varNameTok->next();
            } else if (varNameTok->str() == "&") {
                varNameTok = varNameTok->next();
            }

            typeList.emplace_back(tok, new_scope, scope);
            {
                Type* new_type = &typeList.back();
                new_scope->definedType = new_type;
                scope->definedTypesMap[new_type->name()] = new_type;
            }

            scope->addVariable(varNameTok, tok, tok, access[scope], new_scope->definedType, scope, &mSettings);

            const Token *tok2 = tok->next();

            new_scope->setBodyStartEnd(tok2);

            // make sure we have valid code
            if (!new_scope->bodyEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope->nestedList.push_back(new_scope);
            scope = new_scope;

            tok = tok2;
        }

        // anonymous struct, union and namespace
        else if (tok->isKeyword() && ((Token::Match(tok, "struct|union {") &&
                                       Token::simpleMatch(tok->next()->link(), "} ;")) ||
                                      Token::simpleMatch(tok, "namespace {"))) {
            scopeList.emplace_back(this, tok, scope);

            Scope *new_scope = &scopeList.back();
            access[new_scope] = AccessControl::Public;

            const Token *tok2 = tok->next();

            new_scope->setBodyStartEnd(tok2);

            typeList.emplace_back(tok, new_scope, scope);
            {
                Type* new_type = &typeList.back();
                new_scope->definedType = new_type;
                scope->definedTypesMap[new_type->name()] = new_type;
            }

            // make sure we have valid code
            if (!new_scope->bodyEnd) {
                scopeList.pop_back();
                break;
            }

            // make the new scope the current scope
            scope->nestedList.push_back(new_scope);
            scope = new_scope;

            tok = tok2;
        }

        // forward declared enum
        else if (tok->isKeyword() && (Token::Match(tok, "enum class| %name% ;") || Token::Match(tok, "enum class| %name% : %name% ;"))) {
            typeList.emplace_back(tok, nullptr, scope);
            Type* new_type = &typeList.back();
            scope->definedTypesMap[new_type->name()] = new_type;
            tok = tok->tokAt(2);
        }

        // check for end of scope
        else if (tok == scope->bodyEnd) {
            do {
                access.erase(scope);
                scope = const_cast<Scope*>(scope->nestedIn);
            } while (scope->type != Scope::eGlobal && succeeds(tok, scope->bodyEnd));
            continue;
        }
        // check for end of init list
        else if (inInitList() && tok == endInitList.top().first) {
            endInitList.pop();
            continue;
        }

        // check if in class or structure or union
        else if (scope->isClassOrStructOrUnion()) {
            const Token *funcStart = nullptr;
            const Token *argStart = nullptr;
            const Token *declEnd = nullptr;

            // What section are we in..
            if (tok->str() == "private:")
                access[scope] = AccessControl::Private;
            else if (tok->str() == "protected:")
                access[scope] = AccessControl::Protected;
            else if (tok->str() == "public:" || tok->str() == "__published:")
                access[scope] = AccessControl::Public;
            else if (Token::Match(tok, "public|protected|private %name% :")) {
                if (tok->str() == "private")
                    access[scope] = AccessControl::Private;
                else if (tok->str() == "protected")
                    access[scope] = AccessControl::Protected;
                else
                    access[scope] = AccessControl::Public;

                tok = tok->tokAt(2);
            }

            // class function?
            else if (isFunction(tok, scope, &funcStart, &argStart, &declEnd)) {
                if (tok->previous()->str() != "::" || tok->strAt(-2) == scope->className) {
                    Function function(&mTokenizer, tok, scope, funcStart, argStart);

                    // save the access type
                    function.access = access[scope];

                    const Token *end = function.argDef->link();

                    // count the number of constructors
                    if (function.isConstructor())
                        scope->numConstructors++;

                    // assume implementation is inline (definition and implementation same)
                    function.token = function.tokenDef;
                    function.arg = function.argDef;

                    // out of line function
                    if (const Token *endTok = mTokenizer.isFunctionHead(end, ";")) {
                        tok = endTok;
                        scope->addFunction(function);
                    }

                    // inline function
                    else {
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
                    const Scope * const nested = scope->findInNestedListRecursive(tok->strAt(-2));

                    if (nested)
                        addClassFunction(&scope, &tok, argStart);
                    else {
                        /** @todo handle friend functions */
                    }
                }
            }

            // friend class declaration?
            else if (mTokenizer.isCPP() && tok->isKeyword() && Token::Match(tok, "friend class|struct| ::| %any% ;|::")) {
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

                // fill this in after parsing is complete
                friendInfo.type = nullptr;

                if (!scope->definedType)
                    mTokenizer.syntaxError(tok);

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
                        const Function* const function = addGlobalFunction(scope, tok, argStart, funcStart);

                        if (!function)
                            mTokenizer.syntaxError(tok);
                    }

                    // syntax error?
                    if (!scope)
                        mTokenizer.syntaxError(tok);
                }
                // function prototype?
                else if (declEnd && declEnd->str() == ";") {
                    if (tok->astParent() && tok->astParent()->str() == "::" &&
                        Token::Match(declEnd->previous(), "default|delete")) {
                        addClassFunction(&scope, &tok, argStart);
                        continue;
                    }

                    bool newFunc = true; // Is this function already in the database?
                    auto range = scope->functionMap.equal_range(tok->str());
                    for (std::multimap<std::string, const Function*>::const_iterator it = range.first; it != range.second; ++it) {
                        if (it->second->argsMatch(scope, it->second->argDef, argStart, emptyString, 0)) {
                            newFunc = false;
                            break;
                        }
                    }

                    // save function prototype in database
                    if (newFunc) {
                        addGlobalFunctionDecl(scope, tok, argStart, funcStart);
                    }

                    tok = declEnd;
                    continue;
                }
            } else if (const Token *lambdaEndToken = findLambdaEndToken(tok)) {
                tok = addLambda(tok, lambdaEndToken);
            }
        } else if (scope->isExecutable()) {
            if (tok->isKeyword() && Token::Match(tok, "else|try|do {")) {
                const Token* tok1 = tok->next();
                if (tok->str() == "else")
                    scopeList.emplace_back(this, tok, scope, Scope::eElse, tok1);
                else if (tok->str() == "do")
                    scopeList.emplace_back(this, tok, scope, Scope::eDo, tok1);
                else //if (tok->str() == "try")
                    scopeList.emplace_back(this, tok, scope, Scope::eTry, tok1);

                tok = tok1;
                scope->nestedList.push_back(&scopeList.back());
                scope = &scopeList.back();
            } else if (tok->isKeyword() && Token::Match(tok, "if|for|while|catch|switch (") && Token::simpleMatch(tok->next()->link(), ") {")) {
                const Token *scopeStartTok = tok->next()->link()->next();
                if (tok->str() == "if")
                    scopeList.emplace_back(this, tok, scope, Scope::eIf, scopeStartTok);
                else if (tok->str() == "for") {
                    scopeList.emplace_back(this, tok, scope, Scope::eFor, scopeStartTok);
                } else if (tok->str() == "while")
                    scopeList.emplace_back(this, tok, scope, Scope::eWhile, scopeStartTok);
                else if (tok->str() == "catch") {
                    scopeList.emplace_back(this, tok, scope, Scope::eCatch, scopeStartTok);
                } else // if (tok->str() == "switch")
                    scopeList.emplace_back(this, tok, scope, Scope::eSwitch, scopeStartTok);

                scope->nestedList.push_back(&scopeList.back());
                scope = &scopeList.back();
                if (scope->type == Scope::eFor)
                    scope->checkVariable(tok->tokAt(2), AccessControl::Local, &mSettings); // check for variable declaration and add it to new scope if found
                else if (scope->type == Scope::eCatch)
                    scope->checkVariable(tok->tokAt(2), AccessControl::Throw, &mSettings); // check for variable declaration and add it to new scope if found
                tok = scopeStartTok;
            } else if (Token::Match(tok, "%var% {")) {
                endInitList.emplace(tok->next()->link(), scope);
                tok = tok->next();
            } else if (const Token *lambdaEndToken = findLambdaEndToken(tok)) {
                tok = addLambda(tok, lambdaEndToken);
            } else if (tok->str() == "{") {
                if (inInitList()) {
                    endInitList.emplace(tok->link(), scope);
                } else if (isExecutableScope(tok)) {
                    scopeList.emplace_back(this, tok, scope, Scope::eUnconditional, tok);
                    scope->nestedList.push_back(&scopeList.back());
                    scope = &scopeList.back();
                } else if (scope->isExecutable()) {
                    endInitList.emplace(tok->link(), scope);
                } else {
                    tok = tok->link();
                }
            }
            // syntax error?
            if (!scope)
                mTokenizer.syntaxError(tok);
            // End of scope or list should be handled above
            if (tok->str() == "}")
                mTokenizer.syntaxError(tok);
        }
    }
}

void SymbolDatabase::createSymbolDatabaseClassInfo()
{
    if (mTokenizer.isC())
        return;

    // fill in using info
    for (Scope& scope : scopeList) {
        for (Scope::UsingInfo& usingInfo : scope.usingList) {
            // only find if not already found
            if (usingInfo.scope == nullptr) {
                // check scope for match
                const Scope * const found = findScope(usingInfo.start->tokAt(2), &scope);
                if (found) {
                    // set found scope
                    usingInfo.scope = found;
                    break;
                }
            }
        }
    }

    // fill in base class info
    for (Type& type : typeList) {
        // finish filling in base class info
        for (Type::BaseInfo & i : type.derivedFrom) {
            const Type* found = findType(i.nameTok, type.enclosingScope, /*lookOutside*/ true);
            if (found && found->findDependency(&type)) {
                // circular dependency
                //mTokenizer.syntaxError(nullptr);
            } else {
                i.type = found;
            }
        }
    }

    // fill in friend info
    for (Type & type : typeList) {
        for (Type::FriendInfo &friendInfo : type.friendList) {
            friendInfo.type = findType(friendInfo.nameStart, type.enclosingScope);
        }
    }
}


void SymbolDatabase::createSymbolDatabaseVariableInfo()
{
    // fill in variable info
    for (Scope& scope : scopeList) {
        // find variables
        scope.getVariableList(&mSettings);
    }

    // fill in function arguments
    for (Scope& scope : scopeList) {
        std::list<Function>::iterator func;

        for (func = scope.functionList.begin(); func != scope.functionList.end(); ++func) {
            // add arguments
            func->addArguments(this, &scope);
        }
    }
}

void SymbolDatabase::createSymbolDatabaseCopyAndMoveConstructors()
{
    // fill in class and struct copy/move constructors
    for (Scope& scope : scopeList) {
        if (!scope.isClassOrStruct())
            continue;

        std::list<Function>::iterator func;
        for (func = scope.functionList.begin(); func != scope.functionList.end(); ++func) {
            if (!func->isConstructor() || func->minArgCount() != 1)
                continue;

            const Variable* firstArg = func->getArgumentVar(0);
            if (firstArg->type() == scope.definedType) {
                if (firstArg->isRValueReference())
                    func->type = Function::eMoveConstructor;
                else if (firstArg->isReference() && !firstArg->isPointer())
                    func->type = Function::eCopyConstructor;
            }

            if (func->type == Function::eCopyConstructor ||
                func->type == Function::eMoveConstructor)
                scope.numCopyOrMoveConstructors++;
        }
    }
}

void SymbolDatabase::createSymbolDatabaseFunctionScopes()
{
    // fill in function scopes
    for (const Scope & scope : scopeList) {
        if (scope.type == Scope::eFunction)
            functionScopes.push_back(&scope);
    }
}

void SymbolDatabase::createSymbolDatabaseClassAndStructScopes()
{
    // fill in class and struct scopes
    for (const Scope& scope : scopeList) {
        if (scope.isClassOrStruct())
            classAndStructScopes.push_back(&scope);
    }
}

void SymbolDatabase::createSymbolDatabaseFunctionReturnTypes()
{
    // fill in function return types
    for (Scope& scope : scopeList) {
        std::list<Function>::iterator func;

        for (func = scope.functionList.begin(); func != scope.functionList.end(); ++func) {
            // add return types
            if (func->retDef) {
                const Token *type = func->retDef;
                while (Token::Match(type, "static|const|struct|union|enum"))
                    type = type->next();
                if (type) {
                    func->retType = findVariableTypeInBase(&scope, type);
                    if (!func->retType)
                        func->retType = findTypeInNested(type, func->nestedIn);
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseNeedInitialization()
{
    if (mTokenizer.isC()) {
        // For C code it is easy, as there are no constructors and no default values
        for (const Scope& scope : scopeList) {
            if (scope.definedType)
                scope.definedType->needInitialization = Type::NeedInitialization::True;
        }
    } else {
        // For C++, it is more difficult: Determine if user defined type needs initialization...
        unsigned int unknowns = 0; // stop checking when there are no unknowns
        unsigned int retry = 0;    // bail if we don't resolve all the variable types for some reason

        do {
            unknowns = 0;

            for (Scope& scope : scopeList) {
                if (!scope.isClassOrStructOrUnion())
                    continue;
                if (scope.classDef && Token::simpleMatch(scope.classDef->previous(), ">")) // skip uninstantiated template
                    continue;

                if (!scope.definedType) {
                    mBlankTypes.emplace_back();
                    scope.definedType = &mBlankTypes.back();
                }

                if (scope.isClassOrStruct() && scope.definedType->needInitialization == Type::NeedInitialization::Unknown) {
                    // check for default constructor
                    bool hasDefaultConstructor = false;

                    for (const Function& func : scope.functionList) {
                        if (func.type == Function::eConstructor) {
                            // check for no arguments: func ( )
                            if (func.argCount() == 0) {
                                hasDefaultConstructor = true;
                                break;
                            }

                            /** check for arguments with default values */
                            if (func.argCount() == func.initializedArgCount()) {
                                hasDefaultConstructor = true;
                                break;
                            }
                        }
                    }

                    // User defined types with user defined default constructor doesn't need initialization.
                    // We assume the default constructor initializes everything.
                    // Another check will figure out if the constructor actually initializes everything.
                    if (hasDefaultConstructor)
                        scope.definedType->needInitialization = Type::NeedInitialization::False;

                    // check each member variable to see if it needs initialization
                    else {
                        bool needInitialization = false;
                        bool unknown = false;

                        for (const Variable& var: scope.varlist) {
                            if (var.isClass()) {
                                if (var.type()) {
                                    // does this type need initialization?
                                    if (var.type()->needInitialization == Type::NeedInitialization::True && !var.hasDefault() && !var.isStatic())
                                        needInitialization = true;
                                    else if (var.type()->needInitialization == Type::NeedInitialization::Unknown) {
                                        if (!(var.valueType() && var.valueType()->type == ValueType::CONTAINER))
                                            unknown = true;
                                    }
                                }
                            } else if (!var.hasDefault() && !var.isStatic()) {
                                needInitialization = true;
                                break;
                            }
                        }

                        if (needInitialization)
                            scope.definedType->needInitialization = Type::NeedInitialization::True;
                        else if (!unknown)
                            scope.definedType->needInitialization = Type::NeedInitialization::False;
                        else {
                            if (scope.definedType->needInitialization == Type::NeedInitialization::Unknown)
                                unknowns++;
                        }
                    }
                } else if (scope.type == Scope::eUnion && scope.definedType->needInitialization == Type::NeedInitialization::Unknown)
                    scope.definedType->needInitialization = Type::NeedInitialization::True;
            }

            retry++;
        } while (unknowns && retry < 100);

        // this shouldn't happen so output a debug warning
        if (retry == 100 && mSettings.debugwarnings) {
            for (const Scope& scope : scopeList) {
                if (scope.isClassOrStruct() && scope.definedType->needInitialization == Type::NeedInitialization::Unknown)
                    debugMessage(scope.classDef, "debug", "SymbolDatabase couldn't resolve all user defined types.");
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseVariableSymbolTable()
{
    // create variable symbol table
    mVariableList.resize(mTokenizer.varIdCount() + 1);
    std::fill_n(mVariableList.begin(), mVariableList.size(), nullptr);

    // check all scopes for variables
    for (Scope& scope : scopeList) {
        // add all variables
        for (Variable& var: scope.varlist) {
            const unsigned int varId = var.declarationId();
            if (varId)
                mVariableList[varId] = &var;
            // fix up variables without type
            if (!var.type() && !var.typeStartToken()->isStandardType()) {
                const Type *type = findType(var.typeStartToken(), &scope);
                if (type)
                    var.type(type);
            }
        }

        // add all function parameters
        for (Function& func : scope.functionList) {
            for (Variable& arg: func.argumentList) {
                // check for named parameters
                if (arg.nameToken() && arg.declarationId()) {
                    const unsigned int declarationId = arg.declarationId();
                    mVariableList[declarationId] = &arg;
                    // fix up parameters without type
                    if (!arg.type() && !arg.typeStartToken()->isStandardType()) {
                        const Type *type = findTypeInNested(arg.typeStartToken(), &scope);
                        if (type)
                            arg.type(type);
                    }
                }
            }
        }
    }

    // fill in missing variables if possible
    for (const Scope *func: functionScopes) {
        for (const Token *tok = func->bodyStart->next(); tok && tok != func->bodyEnd; tok = tok->next()) {
            // check for member variable
            if (!Token::Match(tok, "%var% .|["))
                continue;
            const Token* tokDot = tok->next();
            while (Token::simpleMatch(tokDot, "["))
                tokDot = tokDot->link()->next();
            if (!Token::Match(tokDot, ". %var%"))
                continue;
            const Token *member = tokDot->next();
            if (mVariableList[member->varId()] == nullptr) {
                const Variable *var1 = mVariableList[tok->varId()];
                if (var1 && var1->typeScope()) {
                    const Variable* memberVar = var1->typeScope()->getVariable(member->str());
                    if (memberVar) {
                        // add this variable to the look up table
                        mVariableList[member->varId()] = memberVar;
                    }
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseSetScopePointers()
{
    auto setScopePointers = [](const Scope &scope, const Token *bodyStart, const Token *bodyEnd) {
        assert(bodyStart);
        assert(bodyEnd);

        const_cast<Token *>(bodyEnd)->scope(&scope);

        for (Token* tok = const_cast<Token *>(bodyStart); tok != bodyEnd; tok = tok->next()) {
            if (bodyStart != bodyEnd && tok->str() == "{") {
                bool isEndOfScope = false;
                for (Scope* innerScope: scope.nestedList) {
                    const auto &list = innerScope->bodyStartList;
                    if (std::find(list.cbegin(), list.cend(), tok) != list.cend()) {     // Is begin of inner scope
                        tok = tok->link();
                        if (tok->next() == bodyEnd || !tok->next()) {
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
            tok->scope(&scope);
        }
    };

    // Set scope pointers
    for (const Scope& scope: scopeList) {
        if (scope.type == Scope::eGlobal)
            setScopePointers(scope, mTokenizer.list.front(), mTokenizer.list.back());
        else {
            for (const Token *bodyStart: scope.bodyStartList)
                setScopePointers(scope, bodyStart, bodyStart->link());
        }
    }
}

void SymbolDatabase::createSymbolDatabaseSetFunctionPointers(bool firstPass)
{
    if (firstPass) {
        // Set function definition and declaration pointers
        for (const Scope& scope: scopeList) {
            for (const Function& func: scope.functionList) {
                if (func.tokenDef)
                    const_cast<Token *>(func.tokenDef)->function(&func);

                if (func.token)
                    const_cast<Token *>(func.token)->function(&func);
            }
        }
    }

    // Set function call pointers
    for (Token* tok = mTokenizer.list.front(); tok != mTokenizer.list.back(); tok = tok->next()) {
        if (tok->isName() && !tok->function() && tok->varId() == 0 && ((tok->astParent() && tok->astParent()->isComparisonOp()) || Token::Match(tok, "%name% [{(,)>;]")) && !isReservedName(tok->str())) {
            if (tok->next()->str() == ">" && !tok->next()->link())
                continue;

            bool isTemplateArg = false;
            if (!Token::Match(tok->next(), "(|{")) {
                const Token *start = tok;
                while (Token::Match(start->tokAt(-2), "%name% ::"))
                    start = start->tokAt(-2);
                if (!Token::Match(start->previous(), "[(,<=]") && !Token::simpleMatch(start->previous(), "::") && !Token::Match(start->tokAt(-2), "[(,<=] &") && !Token::Match(start, "%name% ;"))
                    continue;
                isTemplateArg = Token::simpleMatch(start->previous(), "<") || Token::simpleMatch(start->tokAt(-2), "<");
            }

            const Function *function = findFunction(tok);
            if (!function || (isTemplateArg && function->isConstructor()))
                continue;

            tok->function(function);

            if (tok->next()->str() != "(")
                const_cast<Function *>(function)->functionPointerUsage = tok;
        }
    }

    // Set C++ 11 delegate constructor function call pointers
    for (const Scope& scope: scopeList) {
        for (const Function& func: scope.functionList) {
            // look for initializer list
            if (func.isConstructor() && func.functionScope && func.functionScope->functionOf && func.arg) {
                const Token * tok = func.arg->link()->next();
                if (tok->str() == "noexcept") {
                    const Token * closingParenTok = tok->linkAt(1);
                    if (!closingParenTok || !closingParenTok->next()) {
                        continue;
                    }
                    tok = closingParenTok->next();
                }
                if (tok->str() != ":") {
                    continue;
                }
                tok = tok->next();
                while (tok && tok != func.functionScope->bodyStart) {
                    if (Token::Match(tok, "%name% {|(")) {
                        if (tok->str() == func.tokenDef->str()) {
                            const Function *function = func.functionScope->functionOf->findFunction(tok);
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
    std::unordered_set<std::string> typenames;
    for (const Type &t : typeList) {
        typenames.insert(t.name());
    }

    // Set type pointers
    for (Token* tok = mTokenizer.list.front(); tok != mTokenizer.list.back(); tok = tok->next()) {
        if (!tok->isName() || tok->varId() || tok->function() || tok->type() || tok->enumerator())
            continue;

        if (typenames.find(tok->str()) == typenames.end())
            continue;

        const Type *type = findVariableType(tok->scope(), tok);
        if (type)
            tok->type(type);
    }
}

void SymbolDatabase::createSymbolDatabaseSetSmartPointerType()
{
    for (Scope &scope: scopeList) {
        for (Variable &var: scope.varlist) {
            if (var.valueType() && var.valueType()->smartPointerTypeToken && !var.valueType()->smartPointerType) {
                ValueType vt(*var.valueType());
                vt.smartPointerType = vt.smartPointerTypeToken->type();
                var.setValueType(vt);
            }
        }
    }
}

void SymbolDatabase::fixVarId(VarIdMap & varIds, const Token * vartok, Token * membertok, const Variable * membervar)
{
    VarIdMap::iterator varId = varIds.find(vartok->varId());
    if (varId == varIds.end()) {
        MemberIdMap memberId;
        if (membertok->varId() == 0) {
            memberId[membervar->nameToken()->varId()] = const_cast<Tokenizer &>(mTokenizer).newVarId();
            mVariableList.push_back(membervar);
        } else
            mVariableList[membertok->varId()] = membervar;
        varIds.insert(std::make_pair(vartok->varId(), memberId));
        varId = varIds.find(vartok->varId());
    }
    MemberIdMap::iterator memberId = varId->second.find(membervar->nameToken()->varId());
    if (memberId == varId->second.end()) {
        if (membertok->varId() == 0) {
            varId->second.insert(std::make_pair(membervar->nameToken()->varId(), const_cast<Tokenizer &>(mTokenizer).newVarId()));
            mVariableList.push_back(membervar);
            memberId = varId->second.find(membervar->nameToken()->varId());
        } else
            mVariableList[membertok->varId()] = membervar;
    }
    if (membertok->varId() == 0)
        membertok->varId(memberId->second);
}

static bool isContainerYieldElement(Library::Container::Yield yield);

void SymbolDatabase::createSymbolDatabaseSetVariablePointers()
{
    VarIdMap varIds;

    auto setMemberVar = [&](const Variable* membervar, Token* membertok, const Token* vartok) -> void {
        if (membervar) {
            membertok->variable(membervar);
            if (vartok && (membertok->varId() == 0 || mVariableList[membertok->varId()] == nullptr))
                fixVarId(varIds, vartok, membertok, membervar);
        }
    };

    // Set variable pointers
    for (Token* tok = mTokenizer.list.front(); tok != mTokenizer.list.back(); tok = tok->next()) {
        if (!tok->isName() || tok->isKeyword() || tok->isStandardType())
            continue;
        if (tok->varId())
            tok->variable(getVariableFromVarId(tok->varId()));

        // Set Token::variable pointer for array member variable
        // Since it doesn't point at a fixed location it doesn't have varid
        const bool isVar = tok->variable() && (tok->variable()->typeScope() || tok->variable()->isSmartPointer() ||
                                               (tok->valueType() && (tok->valueType()->type == ValueType::CONTAINER || tok->valueType()->type == ValueType::ITERATOR)));
        const bool isArrayAccess = isVar && Token::simpleMatch(tok->astParent(), "[");
        const bool isDirectAccess = isVar && !isArrayAccess && Token::simpleMatch(tok->astParent(), ".");
        const bool isDerefAccess = isVar && !isDirectAccess && Token::simpleMatch(tok->astParent(), "*") && Token::simpleMatch(tok->astParent()->astParent(), ".");
        if (isVar && (isArrayAccess || isDirectAccess || isDerefAccess)) {
            Token* membertok{};
            if (isArrayAccess) {
                membertok = tok->astParent();
                while (Token::simpleMatch(membertok, "["))
                    membertok = membertok->astParent();
                if (membertok)
                    membertok = membertok->astOperand2();
            }
            else if (isDirectAccess) {
                membertok = tok->astParent()->astOperand2();
                if (membertok == tok) {
                    Token* gptok = tok->astParent()->astParent();
                    if (Token::simpleMatch(gptok, ".")) // chained access
                        membertok = gptok->astOperand2();
                    else if (Token::simpleMatch(gptok, "[") && Token::simpleMatch(gptok->astParent(), "."))
                        membertok = gptok->astParent()->astOperand2();
                }
            }
            else { // isDerefAccess
                membertok = tok->astParent();
                while (Token::simpleMatch(membertok, "*"))
                    membertok = membertok->astParent();
                if (membertok)
                    membertok = membertok->astOperand2();
            }

            if (membertok && membertok != tok) {
                const Variable *var = tok->variable();
                if (var->typeScope()) {
                    const Variable *membervar = var->typeScope()->getVariable(membertok->str());
                    setMemberVar(membervar, membertok, tok);
                } else if (const ::Type *type = var->smartPointerType()) {
                    const Scope *classScope = type->classScope;
                    const Variable *membervar = classScope ? classScope->getVariable(membertok->str()) : nullptr;
                    setMemberVar(membervar, membertok, tok);
                } else if (tok->valueType() && tok->valueType()->type == ValueType::CONTAINER) {
                    if (const Token* ctt = tok->valueType()->containerTypeToken) {
                        while (ctt && ctt->isKeyword())
                            ctt = ctt->next();
                        const Type* ct = findTypeInNested(ctt, tok->scope());
                        if (ct && ct->classScope && ct->classScope->definedType) {
                            const Variable *membervar = ct->classScope->getVariable(membertok->str());
                            setMemberVar(membervar, membertok, tok);
                        }
                    }
                } else if (const Type* iterType = var->iteratorType()) {
                    if (iterType->classScope && iterType->classScope->definedType) {
                        const Variable *membervar = iterType->classScope->getVariable(membertok->str());
                        setMemberVar(membervar, membertok, tok);
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
            Token* membertok;
            if (tok->next()->link()->next()->str() == ".")
                membertok = tok->next()->link()->next()->next();
            else
                membertok = tok->next()->link()->next()->link()->next()->next();
            if (type) {
                const Variable *membervar = membertok->variable();
                if (!membervar) {
                    if (type->classScope) {
                        membervar = type->classScope->getVariable(membertok->str());
                        setMemberVar(membervar, membertok, tok->function()->retDef);
                    }
                }
            } else if (mSettings.library.detectSmartPointer(tok->function()->retDef)) {
                if (const Token* templateArg = Token::findsimplematch(tok->function()->retDef, "<")) {
                    if (const Type* spType = findTypeInNested(templateArg->next(), tok->scope())) {
                        if (spType->classScope) {
                            const Variable* membervar = spType->classScope->getVariable(membertok->str());
                            setMemberVar(membervar, membertok, tok->function()->retDef);
                        }
                    }
                }
            }
        }
        else if (Token::simpleMatch(tok->astParent(), ".") && tok->next()->str() == "(" &&
                 astIsContainer(tok->astParent()->astOperand1()) && Token::Match(tok->next()->link(), ") . %name% !!(")) {
            const ValueType* vt = tok->astParent()->astOperand1()->valueType();
            const Library::Container* cont = vt->container;
            auto it = cont->functions.find(tok->str());
            if (it != cont->functions.end() && isContainerYieldElement(it->second.yield) && vt->containerTypeToken) {
                Token* memberTok = tok->next()->link()->tokAt(2);
                const Scope* scope = vt->containerTypeToken->scope();
                const Type* contType{};
                const std::string& typeStr = vt->containerTypeToken->str(); // TODO: handle complex type expressions
                while (scope && !contType) {
                    contType = scope->findType(typeStr); // find the type stored in the container
                    scope = scope->nestedIn;
                }
                if (contType && contType->classScope) {
                    const Variable* membervar = contType->classScope->getVariable(memberTok->str());
                    setMemberVar(membervar, memberTok, vt->containerTypeToken);
                }
            }
        }
    }
}

void SymbolDatabase::createSymbolDatabaseEnums()
{
    // fill in enumerators in enum
    for (const Scope &scope : scopeList) {
        if (scope.type != Scope::eEnum)
            continue;

        // add enumerators to enumerator tokens
        for (const Enumerator & i : scope.enumeratorList)
            const_cast<Token *>(i.name)->enumerator(&i);
    }

    std::set<std::string> tokensThatAreNotEnumeratorValues;

    for (const Scope &scope : scopeList) {
        if (scope.type != Scope::eEnum)
            continue;

        for (const Enumerator & enumerator : scope.enumeratorList) {
            // look for initialization tokens that can be converted to enumerators and convert them
            if (enumerator.start) {
                if (!enumerator.end)
                    mTokenizer.syntaxError(enumerator.start);
                for (const Token * tok3 = enumerator.start; tok3 && tok3 != enumerator.end->next(); tok3 = tok3->next()) {
                    if (tok3->tokType() == Token::eName) {
                        const Enumerator * e = findEnumerator(tok3, tokensThatAreNotEnumeratorValues);
                        if (e)
                            const_cast<Token *>(tok3)->enumerator(e);
                    }
                }
            }
        }
    }

    // find enumerators
    for (Token* tok = mTokenizer.list.front(); tok != mTokenizer.list.back(); tok = tok->next()) {
        const bool isVariable = (tok->tokType() == Token::eVariable && !tok->variable());
        if (tok->tokType() != Token::eName && !isVariable)
            continue;
        const Enumerator * enumerator = findEnumerator(tok, tokensThatAreNotEnumeratorValues);
        if (enumerator) {
            if (isVariable)
                tok->varId(0);
            tok->enumerator(enumerator);
        }
    }
}

void SymbolDatabase::createSymbolDatabaseIncompleteVars()
{
    // TODO: replace with Keywords::getX()
    static const std::unordered_set<std::string> cpp20keywords = {
        "alignas",
        "alignof",
        "axiom",
        "co_await",
        "co_return",
        "co_yield",
        "concept",
        "synchronized",
        "consteval",
        "reflexpr",
        "requires",
    };
    static const std::unordered_set<std::string> cppkeywords = {
        "asm",
        "auto",
        "catch",
        "char",
        "class",
        "const",
        "constexpr",
        "decltype",
        "default",
        "do",
        "enum",
        "explicit",
        "export",
        "extern",
        "final",
        "friend",
        "inline",
        "mutable",
        "namespace",
        "new",
        "noexcept",
        "nullptr",
        "override",
        "private",
        "protected",
        "public",
        "register",
        "sizeof",
        "static",
        "static_assert",
        "struct",
        "template",
        "this",
        "thread_local",
        "throw",
        "try",
        "typedef",
        "typeid",
        "typename",
        "union",
        "using",
        "virtual",
        "void",
        "volatile",
        "NULL",
    };
    for (Token* tok = mTokenizer.list.front(); tok != mTokenizer.list.back(); tok = tok->next()) {
        const Scope * scope = tok->scope();
        if (!scope)
            continue;
        if (!scope->isExecutable())
            continue;
        if (tok->varId() != 0)
            continue;
        if (tok->isCast() && !isCPPCast(tok) && tok->link() && tok->str() == "(") {
            tok = tok->link();
            continue;
        }
        if (Token::Match(tok, "catch|typeid (")) {
            tok = tok->linkAt(1);
            continue;
        }
        if (!(tok->isNameOnly() || tok->isKeyword()))
            continue;
        if (tok->type())
            continue;
        if (Token::Match(tok->next(), "::|.|(|{|:|%var%"))
            continue;
        if (Token::Match(tok->next(), "&|&&|* *| *| )|,|%var%|const"))
            continue;
        // Very likely a typelist
        if (Token::Match(tok->tokAt(-2), "%type% ,") || Token::Match(tok->next(), ", %type%"))
            continue;
        // Inside template brackets
        if (Token::simpleMatch(tok->next(), "<") && tok->linkAt(1)) {
            tok = tok->linkAt(1);
            continue;
        }
        if (tok->isKeyword())
            continue;
        // Skip goto labels
        if (Token::simpleMatch(tok->previous(), "goto"))
            continue;
        // TODO: handle all C/C++ standards
        if (cppkeywords.count(tok->str()) > 0)
            continue;
        if (mSettings.standards.cpp >= Standards::CPP20 && cpp20keywords.count(tok->str()) > 0)
            continue;
        std::string fstr = tok->str();
        const Token* ftok = tok->previous();
        while (Token::simpleMatch(ftok, "::")) {
            if (!Token::Match(ftok->previous(), "%name%"))
                break;
            fstr.insert(0, ftok->previous()->str() + "::");
            ftok = ftok->tokAt(-2);
        }
        if (mSettings.library.functions.find(fstr) != mSettings.library.functions.end())
            continue;
        if (mTokenizer.isCPP()) {
            const Token* parent = tok->astParent();
            while (Token::Match(parent, "::|[|{"))
                parent = parent->astParent();
            if (Token::simpleMatch(parent, "new"))
                continue;
        }
        tok->isIncompleteVar(true);
    }
}

void SymbolDatabase::createSymbolDatabaseEscapeFunctions()
{
    for (const Scope& scope : scopeList) {
        if (scope.type != Scope::eFunction)
            continue;
        Function * function = scope.function;
        if (!function)
            continue;
        if (Token::findsimplematch(scope.bodyStart, "return", scope.bodyEnd))
            continue;
        function->isEscapeFunction(isReturnScope(scope.bodyEnd, &mSettings.library, nullptr, true));
    }
}

static bool isExpression(const Token* tok)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok, "{") && tok->scope() && tok->scope()->bodyStart != tok &&
        (tok->astOperand1() || tok->astOperand2()))
        return true;
    if (!Token::Match(tok, "(|.|[|::|?|:|++|--|%cop%|%assign%"))
        return false;
    if (Token::Match(tok, "*|&|&&")) {
        const Token* vartok = findAstNode(tok, [&](const Token* tok2) {
            const Variable* var = tok2->variable();
            if (!var)
                return false;
            return var->nameToken() == tok2;
        });
        if (vartok)
            return false;
    }
    return true;
}

static std::string getIncompleteNameID(const Token* tok)
{
    std::string result = tok->str() + "@";
    while (Token::Match(tok->astParent(), ".|::"))
        tok = tok->astParent();
    return result + tok->expressionString();
}

void SymbolDatabase::createSymbolDatabaseExprIds()
{
    nonneg int base = 0;
    // Find highest varId
    for (const Variable *var : mVariableList) {
        if (!var)
            continue;
        base = std::max<MathLib::bigint>(base, var->declarationId());
    }
    nonneg int id = base + 1;
    // Find incomplete vars that are used in constant context
    std::unordered_map<std::string, nonneg int> unknownConstantIds;
    const Token* inConstExpr = nullptr;
    for (const Token* tok = mTokenizer.list.front(); tok != mTokenizer.list.back(); tok = tok->next()) {
        if (Token::Match(tok, "decltype|sizeof|typeof (") && tok->next()->link()) {
            tok = tok->next()->link()->previous();
        } else if (tok == inConstExpr) {
            inConstExpr = nullptr;
        } else if (inConstExpr) {
            if (!tok->isIncompleteVar())
                continue;
            if (!isExpression(tok->astParent()))
                continue;
            const std::string& name = getIncompleteNameID(tok);
            if (unknownConstantIds.count(name) > 0)
                continue;
            unknownConstantIds[name] = id++;
        } else if (tok->link() && tok->str() == "<") {
            inConstExpr = tok->link();
        } else if (Token::Match(tok, "%var% [") && tok->variable() && tok->variable()->nameToken() == tok) {
            inConstExpr = tok->next()->link();
        }
    }

    auto exprScopes = functionScopes; // functions + global lambdas
    std::copy_if(scopeList.front().nestedList.begin(), scopeList.front().nestedList.end(), std::back_inserter(exprScopes), [](const Scope* scope) {
        return scope && scope->type == Scope::eLambda;
    });

    for (const Scope * scope : exprScopes) {
        nonneg int thisId = 0;
        std::unordered_map<std::string, std::vector<Token*>> exprs;

        std::unordered_map<std::string, nonneg int> unknownIds;
        // Assign IDs to incomplete vars which are part of an expression
        // Such variables should be assumed global
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->isIncompleteVar())
                continue;
            if (!isExpression(tok->astParent()))
                continue;
            const std::string& name = getIncompleteNameID(tok);
            nonneg int sid = 0;
            if (unknownConstantIds.count(name) > 0) {
                sid = unknownConstantIds.at(name);
                tok->isIncompleteConstant(true);
            } else if (unknownIds.count(name) == 0) {
                sid = id++;
                unknownIds[name] = sid;
            } else {
                sid = unknownIds.at(name);
            }
            assert(sid > 0);
            tok->exprId(sid);
        }

        // Assign IDs
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->varId() > 0) {
                tok->exprId(tok->varId());
            } else if (isExpression(tok)) {
                exprs[tok->str()].push_back(tok);
                tok->exprId(id++);

                if (id == std::numeric_limits<nonneg int>::max() / 4) {
                    throw InternalError(nullptr, "Ran out of expression ids.", InternalError::INTERNAL);
                }
            } else if (isCPP() && Token::simpleMatch(tok, "this")) {
                if (thisId == 0)
                    thisId = id++;
                tok->exprId(thisId);
            }
        }

        // Apply CSE
        for (const auto& p:exprs) {
            const std::vector<Token*>& tokens = p.second;
            const std::size_t N = tokens.size();
            for (std::size_t i = 0; i < N; ++i) {
                Token* const tok1 = tokens[i];
                for (std::size_t j = i + 1; j < N; ++j) {
                    Token* const tok2 = tokens[j];
                    if (tok1->exprId() == tok2->exprId())
                        continue;
                    if (!isSameExpression(isCPP(), true, tok1, tok2, mSettings.library, false, false))
                        continue;
                    nonneg int const cid = std::min(tok1->exprId(), tok2->exprId());
                    tok1->exprId(cid);
                    tok2->exprId(cid);
                }
            }
        }
        // Mark expressions that are unique
        std::unordered_map<nonneg int, Token*> exprMap;
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->exprId() == 0)
                continue;
            auto p = exprMap.emplace(tok->exprId(), tok);
            // Already exists so set it to null
            if (!p.second) {
                p.first->second = nullptr;
            }
        }
        for (const auto& p : exprMap) {
            if (!p.second)
                continue;
            if (p.second->variable()) {
                const Variable* var = p.second->variable();
                if (var->nameToken() != p.second)
                    continue;
            }
            p.second->setUniqueExprId();
        }
    }
}

void SymbolDatabase::setArrayDimensionsUsingValueFlow()
{
    // set all unknown array dimensions
    for (const Variable *var : mVariableList) {
        // check each array variable
        if (!var || !var->isArray())
            continue;
        // check each array dimension
        for (const Dimension &const_dimension : var->dimensions()) {
            Dimension &dimension = const_cast<Dimension &>(const_dimension);
            if (dimension.num != 0 || !dimension.tok)
                continue;

            if (Token::Match(dimension.tok->previous(), "[<,]")) {
                if (dimension.known)
                    continue;
                if (!Token::Match(dimension.tok->previous(), "[<,]"))
                    continue;

                // In template arguments, there might not be AST
                // Determine size by using the "raw tokens"
                TokenList tokenList(&mSettings);
                tokenList.addtoken(";", 0, 0, 0, false);
                bool fail = false;
                for (const Token *tok = dimension.tok; tok && !Token::Match(tok, "[,>]"); tok = tok->next()) {
                    if (!tok->isName())
                        tokenList.addtoken(tok->str(), 0, 0, 0, false);

                    else if (tok->hasKnownIntValue())
                        tokenList.addtoken(std::to_string(tok->getKnownIntValue()), 0, 0, 0, false);

                    else {
                        fail = true;
                        break;
                    }
                }

                if (fail)
                    continue;

                tokenList.addtoken(";", 0, 0, 0, false);

                for (Token *tok = tokenList.front(); tok;) {
                    if (TemplateSimplifier::simplifyNumericCalculations(tok, false))
                        tok = tokenList.front();
                    else
                        tok = tok->next();
                }

                if (Token::Match(tokenList.front(), "; %num% ;")) {
                    dimension.known = true;
                    dimension.num = MathLib::toBigNumber(tokenList.front()->next()->str());
                }

                continue;
            }

            // Normal array [..dimension..]
            dimension.known = false;

            // check for a single token dimension
            if (dimension.tok->hasKnownIntValue()) {
                dimension.known = true;
                dimension.num = dimension.tok->getKnownIntValue();
                continue;
            }

            if (dimension.tok->valueType() && dimension.tok->valueType()->pointer == 0) {
                int bits = 0;
                switch (dimension.tok->valueType()->type) {
                case ValueType::Type::CHAR:
                    bits = mSettings.platform.char_bit;
                    break;
                case ValueType::Type::SHORT:
                    bits = mSettings.platform.short_bit;
                    break;
                case ValueType::Type::INT:
                    bits = mSettings.platform.int_bit;
                    break;
                case ValueType::Type::LONG:
                    bits = mSettings.platform.long_bit;
                    break;
                case ValueType::Type::LONGLONG:
                    bits = mSettings.platform.long_long_bit;
                    break;
                default:
                    break;
                }

                if (bits > 0 && bits <= 62) {
                    if (dimension.tok->valueType()->sign == ValueType::Sign::UNSIGNED)
                        dimension.num = 1LL << bits;
                    else
                        dimension.num = 1LL << (bits - 1);
                }
            }
        }
    }
}

SymbolDatabase::~SymbolDatabase()
{
    // Clear scope, type, function and variable pointers
    for (Token* tok = mTokenizer.list.front(); tok; tok = tok->next()) {
        tok->scope(nullptr);
        tok->type(nullptr);
        tok->function(nullptr);
        tok->variable(nullptr);
        tok->enumerator(nullptr);
        tok->setValueType(nullptr);
    }
}

bool SymbolDatabase::isFunction(const Token *tok, const Scope* outerScope, const Token **funcStart, const Token **argStart, const Token** declEnd) const
{
    if (tok->varId())
        return false;

    // function returning function pointer? '... ( ... %name% ( ... ))( ... ) {'
    // function returning reference to array '... ( & %name% ( ... ))[ ... ] {'
    // TODO: Activate this again
    if ((false) && tok->str() == "(" && tok->strAt(1) != "*" && // NOLINT(readability-simplify-boolean-expr)
        (tok->link()->previous()->str() == ")" || Token::simpleMatch(tok->link()->tokAt(-2), ") const"))) {
        const Token* tok2 = tok->link()->next();
        if (tok2 && tok2->str() == "(" && Token::Match(tok2->link()->next(), "{|;|const|=")) {
            const Token* argStartTok;
            if (tok->link()->previous()->str() == "const")
                argStartTok = tok->link()->linkAt(-2);
            else
                argStartTok = tok->link()->linkAt(-1);
            *funcStart = argStartTok->previous();
            *argStart = argStartTok;
            *declEnd = Token::findmatch(tok2->link()->next(), "{|;");
            return true;
        }
        if (tok2 && tok2->str() == "[") {
            while (tok2 && tok2->str() == "[")
                tok2 = tok2->link()->next();
            if (Token::Match(tok2, "{|;|const|=")) {
                const Token* argStartTok;
                if (tok->link()->previous()->str() == "const")
                    argStartTok = tok->link()->linkAt(-2);
                else
                    argStartTok = tok->link()->linkAt(-1);
                *funcStart = argStartTok->previous();
                *argStart = argStartTok;
                *declEnd = Token::findmatch(tok2, "{|;");
                return true;
            }
        }
    }

    else if (!tok->isName() || !tok->next() || !tok->next()->link())
        return false;

    // regular function?
    else if (Token::Match(tok, "%name% (") && !isReservedName(tok->str()) && tok->previous() &&
             (Token::Match(tok->previous(), "%name%|>|&|&&|*|::|~") || // Either a return type or scope qualifier in front of tok
              outerScope->isClassOrStructOrUnion())) { // or a ctor/dtor
        const Token* tok1 = tok->previous();
        const Token* tok2 = tok->next()->link()->next();

        if (!mTokenizer.isFunctionHead(tok->next(), ";:{"))
            return false;

        // skip over destructor "~"
        if (tok1->str() == "~")
            tok1 = tok1->previous();

        // skip over qualification
        while (Token::simpleMatch(tok1, "::")) {
            tok1 = tok1->previous();
            if (tok1 && tok1->isName())
                tok1 = tok1->previous();
            else if (tok1 && tok1->str() == ">" && tok1->link() && Token::Match(tok1->link()->previous(), "%name%"))
                tok1 = tok1->link()->tokAt(-2);
        }

        // skip over const, noexcept, throw, override, final and volatile specifiers
        while (Token::Match(tok2, "const|noexcept|throw|override|final|volatile|&|&&")) {
            tok2 = tok2->next();
            if (tok2 && tok2->str() == "(")
                tok2 = tok2->link()->next();
        }

        // skip over trailing return type
        if (tok2 && tok2->str() == ".") {
            for (tok2 = tok2->next(); tok2; tok2 = tok2->next()) {
                if (Token::Match(tok2, ";|{|=|override|final"))
                    break;
                if (tok2->link() && Token::Match(tok2, "<|[|("))
                    tok2 = tok2->link();
            }
        }

        // done if constructor or destructor
        if (!Token::Match(tok1, "{|}|;|public:|protected:|private:") && tok1) {
            // skip over pointers and references
            while (Token::Match(tok1, "%type%|*|&|&&") && !endsWith(tok1->str(), ':') && (!isReservedName(tok1->str()) || tok1->str() == "const"))
                tok1 = tok1->previous();

            // skip over decltype
            if (Token::simpleMatch(tok1, ")") && tok1->link() &&
                Token::simpleMatch(tok1->link()->previous(), "decltype ("))
                tok1 = tok1->link()->tokAt(-2);

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
            if (tok1 && tok1->isName()) {
                if (tok1->str() == "return")
                    return false;
                if (tok1->str() != "friend")
                    tok1 = tok1->previous();
            }

            // skip over qualification
            while (Token::simpleMatch(tok1, "::")) {
                tok1 = tok1->previous();
                if (tok1 && tok1->isName())
                    tok1 = tok1->previous();
                else if (tok1 && tok1->str() == ">" && tok1->link() && Token::Match(tok1->link()->previous(), "%name%"))
                    tok1 = tok1->link()->tokAt(-2);
                else if (Token::simpleMatch(tok1, ")") && tok1->link() &&
                         Token::simpleMatch(tok1->link()->previous(), "decltype ("))
                    tok1 = tok1->link()->tokAt(-2);
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
        if (mTokenizer.isC()) {
            debugMessage(tok, "debug", "SymbolDatabase::isFunction found C function '" + tok->str() + "' without a return type.");
            *funcStart = tok;
            *argStart = tok->next();
            *declEnd = tok->linkAt(1)->next();
            return true;
        }
        mTokenizer.syntaxError(tok);
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
            const ErrorMessage errmsg(callstack, &mTokenizer.list, Severity::debug,
                                      "symbolDatabaseWarning",
                                      msg,
                                      Certainty::normal);
            mErrorLogger->reportErr(errmsg);
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
    for (std::vector<const Variable *>::const_iterator iter = mVariableList.cbegin(); iter!=mVariableList.cend(); ++iter) {
        const Variable * const var = *iter;
        if (var) {
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
    if (mSettings.debugwarnings) {
        validateExecutableScopes();
    }
    // TODO
    //validateVariables();
}

void SymbolDatabase::clangSetVariables(const std::vector<const Variable *> &variableList)
{
    mVariableList = variableList;
}

void SymbolDatabase::debugSymbolDatabase() const
{
    if (!mSettings.debugnormal && !mSettings.debugwarnings)
        return;
    for (const Token* tok = mTokenizer.list.front(); tok != mTokenizer.list.back(); tok = tok->next()) {
        if (tok->astParent() && tok->astParent()->getTokenDebug() == tok->getTokenDebug())
            continue;
        if (tok->getTokenDebug() == TokenDebug::ValueType) {

            std::string msg = "Value type is ";
            ErrorPath errorPath;
            if (tok->valueType()) {
                msg += tok->valueType()->str();
                errorPath.insert(errorPath.end(), tok->valueType()->debugPath.cbegin(), tok->valueType()->debugPath.cend());

            } else {
                msg += "missing";
            }
            errorPath.emplace_back(tok, "");
            mErrorLogger->reportErr(
                {errorPath, &mTokenizer.list, Severity::debug, "valueType", msg, CWE{0}, Certainty::normal});
        }
    }
}

Variable::Variable(const Token *name_, const std::string &clangType, const Token *typeStart,
                   const Token *typeEnd, nonneg int index_, AccessControl access_,
                   const Type *type_, const Scope *scope_)
    : mNameToken(name_),
    mTypeStartToken(typeStart),
    mTypeEndToken(typeEnd),
    mIndex(index_),
    mAccess(access_),
    mFlags(0),
    mType(type_),
    mScope(scope_)
{
    if (!mTypeStartToken && mTypeEndToken) {
        mTypeStartToken = mTypeEndToken;
        while (Token::Match(mTypeStartToken->previous(), "%type%|*|&"))
            mTypeStartToken = mTypeStartToken->previous();
    }

    while (Token::Match(mTypeStartToken, "const|struct|static")) {
        if (mTypeStartToken->str() == "static")
            setFlag(fIsStatic, true);
        mTypeStartToken = mTypeStartToken->next();
    }

    if (Token::simpleMatch(mTypeEndToken, "&"))
        setFlag(fIsReference, true);
    else if (Token::simpleMatch(mTypeEndToken, "&&")) {
        setFlag(fIsReference, true);
        setFlag(fIsRValueRef, true);
    }

    std::string::size_type pos = clangType.find('[');
    if (pos != std::string::npos) {
        setFlag(fIsArray, true);
        do {
            const std::string::size_type pos1 = pos+1;
            pos = clangType.find(']', pos1);
            Dimension dim;
            dim.tok = nullptr;
            dim.known = pos > pos1;
            if (pos > pos1)
                dim.num = MathLib::toBigNumber(clangType.substr(pos1, pos - pos1));
            else
                dim.num = 0;
            mDimensions.push_back(dim);
            ++pos;
        } while (pos < clangType.size() && clangType[pos] == '[');
    }

    // Is there initialization in variable declaration
    const Token *initTok = mNameToken ? mNameToken->next() : nullptr;
    while (initTok && initTok->str() == "[")
        initTok = initTok->link()->next();
    if (Token::Match(initTok, "=|{") || (initTok && initTok->isSplittedVarDeclEq()))
        setFlag(fIsInit, true);
}

Variable::Variable(const Variable &var, const Scope *scope)
{
    *this = var;
    mScope = scope;
}

Variable::Variable(const Variable &var)
{
    *this = var;
}

Variable::~Variable()
{
    delete mValueType;
}

Variable& Variable::operator=(const Variable &var)
{
    if (this == &var)
        return *this;

    mNameToken = var.mNameToken;
    mTypeStartToken = var.mTypeStartToken;
    mTypeEndToken = var.mTypeEndToken;
    mIndex = var.mIndex;
    mAccess = var.mAccess;
    mFlags = var.mFlags;
    mType = var.mType;
    mScope = var.mScope;
    mDimensions = var.mDimensions;
    delete mValueType;
    mValueType = nullptr;
    if (var.mValueType)
        mValueType = new ValueType(*var.mValueType);

    return *this;
}

bool Variable::isMember() const {
    return mScope && mScope->isClassOrStructOrUnion();
}

bool Variable::isPointerArray() const
{
    return isArray() && nameToken() && nameToken()->previous() && (nameToken()->previous()->str() == "*");
}

bool Variable::isUnsigned() const
{
    return mValueType ? (mValueType->sign == ValueType::Sign::UNSIGNED) : mTypeStartToken->isUnsigned();
}

const Token * Variable::declEndToken() const
{
    Token const * declEnd = typeStartToken();
    while (declEnd && !Token::Match(declEnd, "[;,)={]")) {
        if (declEnd->link() && Token::Match(declEnd,"(|[|<"))
            declEnd = declEnd->link();
        declEnd = declEnd->next();
    }
    return declEnd;
}

void Variable::evaluate(const Settings* settings)
{
    // Is there initialization in variable declaration
    const Token *initTok = mNameToken ? mNameToken->next() : nullptr;
    while (Token::Match(initTok, "[|(")) {
        initTok = initTok->link()->next();
        if (Token::simpleMatch(initTok, ")"))
            initTok = initTok->next();
    }
    if (Token::Match(initTok, "=|{") || (initTok && initTok->isSplittedVarDeclEq()))
        setFlag(fIsInit, true);

    if (!settings)
        return;

    const Library * const lib = &settings->library;

    // TODO: ValueType::parseDecl() is also performing a container lookup
    bool isContainer = false;
    if (mNameToken)
        setFlag(fIsArray, arrayDimensions(settings, isContainer));

    if (mTypeStartToken)
        setValueType(ValueType::parseDecl(mTypeStartToken,*settings));

    const Token* tok = mTypeStartToken;
    while (tok && tok->previous() && tok->previous()->isName())
        tok = tok->previous();
    const Token* end = mTypeEndToken;
    if (end)
        end = end->next();
    while (tok != end) {
        if (tok->str() == "static")
            setFlag(fIsStatic, true);
        else if (tok->str() == "extern")
            setFlag(fIsExtern, true);
        else if (tok->str() == "volatile" || Token::simpleMatch(tok, "std :: atomic <"))
            setFlag(fIsVolatile, true);
        else if (tok->str() == "mutable")
            setFlag(fIsMutable, true);
        else if (tok->str() == "const")
            setFlag(fIsConst, true);
        else if (tok->str() == "constexpr") {
            setFlag(fIsConst, true);
            setFlag(fIsStatic, true);
        } else if (tok->str() == "*") {
            setFlag(fIsPointer, !isArray() || (isContainer && !Token::Match(tok->next(), "%name% [")) || Token::Match(tok, "* const| %name% )"));
            setFlag(fIsConst, false); // Points to const, isn't necessarily const itself
        } else if (tok->str() == "&") {
            if (isReference())
                setFlag(fIsRValueRef, true);
            setFlag(fIsReference, true);
        } else if (tok->str() == "&&") { // Before simplification, && isn't split up
            setFlag(fIsRValueRef, true);
            setFlag(fIsReference, true); // Set also fIsReference
        }

        if (tok->isAttributeMaybeUnused()) {
            setFlag(fIsMaybeUnused, true);
        }

        if (tok->str() == "<" && tok->link())
            tok = tok->link();
        else
            tok = tok->next();
    }

    while (Token::Match(mTypeStartToken, "static|const|constexpr|volatile %any%"))
        mTypeStartToken = mTypeStartToken->next();
    while (mTypeEndToken && mTypeEndToken->previous() && Token::Match(mTypeEndToken, "const|volatile"))
        mTypeEndToken = mTypeEndToken->previous();

    if (mTypeStartToken) {
        std::string strtype = mTypeStartToken->str();
        for (const Token *typeToken = mTypeStartToken; Token::Match(typeToken, "%type% :: %type%"); typeToken = typeToken->tokAt(2))
            strtype += "::" + typeToken->strAt(2);
        setFlag(fIsClass, !lib->podtype(strtype) && !mTypeStartToken->isStandardType() && !isEnumType() && !isPointer() && !isReference() && strtype != "...");
        setFlag(fIsStlType, Token::simpleMatch(mTypeStartToken, "std ::"));
        setFlag(fIsStlString, ::isStlStringType(mTypeStartToken));
        setFlag(fIsSmartPointer, mTypeStartToken->isCpp() && lib->isSmartPointer(mTypeStartToken));
    }
    if (mAccess == AccessControl::Argument) {
        tok = mNameToken;
        if (!tok) {
            // Argument without name
            tok = mTypeEndToken;
            // back up to start of array dimensions
            while (tok && tok->str() == "]")
                tok = tok->link()->previous();
            // add array dimensions if present
            if (tok && tok->next()->str() == "[")
                setFlag(fIsArray, arrayDimensions(settings, isContainer));
        }
        if (!tok)
            return;
        tok = tok->next();
        while (tok->str() == "[")
            tok = tok->link();
        setFlag(fHasDefault, tok->str() == "=");
    }
    // check for C++11 member initialization
    if (mScope && mScope->isClassOrStruct()) {
        // type var = x or
        // type var = {x}
        // type var = x; gets simplified to: type var ; var = x ;
        Token const * declEnd = declEndToken();
        if ((Token::Match(declEnd, "; %name% =") && declEnd->strAt(1) == mNameToken->str()) ||
            Token::Match(declEnd, "=|{"))
            setFlag(fHasDefault, true);
    }

    if (mTypeStartToken) {
        if (Token::Match(mTypeStartToken, "float|double"))
            setFlag(fIsFloatType, true);
    }
}

void Variable::setValueType(const ValueType &valueType)
{
    if (valueType.type == ValueType::Type::UNKNOWN_TYPE) {
        const Token *declType = Token::findsimplematch(mTypeStartToken, "decltype (", mTypeEndToken);
        if (declType && !declType->next()->valueType())
            return;
    }
    delete mValueType;
    mValueType = nullptr;
    mValueType = new ValueType(valueType);
    if ((mValueType->pointer > 0) && (!isArray() || Token::Match(mNameToken->previous(), "( * %name% )")))
        setFlag(fIsPointer, true);
    setFlag(fIsConst, mValueType->constness & (1U << mValueType->pointer));
    if (mValueType->smartPointerType)
        setFlag(fIsSmartPointer, true);
}

const Type* Variable::smartPointerType() const
{
    if (!isSmartPointer())
        return nullptr;

    if (mValueType->smartPointerType)
        return mValueType->smartPointerType;

    // TODO: Cache result, handle more complex type expression
    const Token* typeTok = typeStartToken();
    while (Token::Match(typeTok, "%name%|::"))
        typeTok = typeTok->next();
    if (Token::Match(typeTok, "< %name% >")) {
        const Scope* scope = typeTok->scope();
        const Type* ptrType{};
        while (scope && !ptrType) {
            ptrType = scope->findType(typeTok->next()->str());
            scope = scope->nestedIn;
        }
        return ptrType;
    }
    return nullptr;
}

const Type* Variable::iteratorType() const
{
    if (!mValueType || mValueType->type != ValueType::ITERATOR)
        return nullptr;

    if (mValueType->containerTypeToken)
        return mValueType->containerTypeToken->type();

    return nullptr;
}

bool Variable::isStlStringViewType() const
{
    return getFlag(fIsStlType) && valueType() && valueType()->container && valueType()->container->stdStringLike && valueType()->container->view;
}

std::string Variable::getTypeName() const
{
    std::string ret;
    // TODO: For known types, generate the full type name
    for (const Token *typeTok = mTypeStartToken; Token::Match(typeTok, "%name%|::") && typeTok->varId() == 0; typeTok = typeTok->next()) {
        ret += typeTok->str();
        if (Token::simpleMatch(typeTok->next(), "<") && typeTok->next()->link()) // skip template arguments
            typeTok = typeTok->next()->link();
    }
    return ret;
}

static bool isOperator(const Token *tokenDef)
{
    if (!tokenDef)
        return false;
    if (tokenDef->isOperatorKeyword())
        return true;
    const std::string &name = tokenDef->str();
    return name.size() > 8 && startsWith(name,"operator") && std::strchr("+-*/%&|~^<>!=[(", name[8]);
}

Function::Function(const Tokenizer *mTokenizer,
                   const Token *tok,
                   const Scope *scope,
                   const Token *tokDef,
                   const Token *tokArgDef)
    : tokenDef(tokDef),
    argDef(tokArgDef),
    nestedIn(scope)
{
    // operator function
    if (::isOperator(tokenDef)) {
        isOperator(true);

        // 'operator =' is special
        if (tokenDef->str() == "operator=")
            type = Function::eOperatorEqual;
    }

    else if (tokenDef->str() == "[") {
        type = Function::eLambda;
    }

    // class constructor/destructor
    else if (((tokenDef->str() == scope->className) ||
              (tokenDef->str().substr(0, scope->className.size()) == scope->className &&
               tokenDef->str().size() > scope->className.size() + 1 &&
               tokenDef->str()[scope->className.size() + 1] == '<')) &&
             scope->type != Scope::ScopeType::eNamespace) {
        // destructor
        if (tokenDef->previous()->str() == "~")
            type = Function::eDestructor;
        // constructor of any kind
        else
            type = Function::eConstructor;

        isExplicit(tokenDef->strAt(-1) == "explicit" || tokenDef->strAt(-2) == "explicit");
    }

    const Token *tok1 = setFlags(tok, scope);

    // find the return type
    if (!isConstructor() && !isDestructor()) {
        // @todo auto type deduction should be checked
        // @todo attributes and exception specification can also precede trailing return type
        if (Token::Match(argDef->link()->next(), "const|volatile| &|&&| .")) { // Trailing return type
            hasTrailingReturnType(true);
            if (argDef->link()->strAt(1) == ".")
                retDef = argDef->link()->tokAt(2);
            else if (argDef->link()->strAt(2) == ".")
                retDef = argDef->link()->tokAt(3);
            else if (argDef->link()->strAt(3) == ".")
                retDef = argDef->link()->tokAt(4);
        } else if (!isLambda()) {
            if (tok1->str() == ">")
                tok1 = tok1->next();
            while (Token::Match(tok1, "extern|virtual|static|friend|struct|union|enum"))
                tok1 = tok1->next();
            retDef = tok1;
        }
    }

    const Token *end = argDef->link();

    // parse function attributes..
    tok = end->next();
    while (tok) {
        if (tok->str() == "const")
            isConst(true);
        else if (tok->str() == "&")
            hasLvalRefQualifier(true);
        else if (tok->str() == "&&")
            hasRvalRefQualifier(true);
        else if (tok->str() == "override")
            setFlag(fHasOverrideSpecifier, true);
        else if (tok->str() == "final")
            setFlag(fHasFinalSpecifier, true);
        else if (tok->str() == "volatile")
            isVolatile(true);
        else if (tok->str() == "noexcept") {
            isNoExcept(!Token::simpleMatch(tok->next(), "( false )"));
            if (tok->next()->str() == "(")
                tok = tok->linkAt(1);
        } else if (Token::simpleMatch(tok, "throw (")) {
            isThrow(true);
            if (tok->strAt(2) != ")")
                throwArg = tok->next();
            tok = tok->linkAt(1);
        } else if (Token::Match(tok, "= 0|default|delete ;")) {
            const std::string& modifier = tok->strAt(1);
            isPure(modifier == "0");
            isDefault(modifier == "default");
            isDelete(modifier == "delete");
        } else if (tok->str() == ".") { // trailing return type
            // skip over return type
            while (tok && !Token::Match(tok->next(), ";|{|override|final"))
                tok = tok->next();
        } else
            break;
        if (tok)
            tok = tok->next();
    }

    if (mTokenizer->isFunctionHead(end, ":{")) {
        // assume implementation is inline (definition and implementation same)
        token = tokenDef;
        arg = argDef;
        isInline(true);
        hasBody(true);
    }
}

Function::Function(const Token *tokenDef, const std::string &clangType)
    : tokenDef(tokenDef)
{
    // operator function
    if (::isOperator(tokenDef)) {
        isOperator(true);

        // 'operator =' is special
        if (tokenDef->str() == "operator=")
            type = Function::eOperatorEqual;
    }

    setFlags(tokenDef, tokenDef->scope());

    if (endsWith(clangType, " const"))
        isConst(true);
}

const Token *Function::setFlags(const Token *tok1, const Scope *scope)
{
    if (tok1->isInline())
        isInlineKeyword(true);

    // look for end of previous statement
    while (tok1->previous() && !Token::Match(tok1->previous(), ";|}|{|public:|protected:|private:")) {
        tok1 = tok1->previous();

        if (tok1->isInline())
            isInlineKeyword(true);

        // extern function
        if (tok1->isExternC() || tok1->str() == "extern") {
            isExtern(true);
        }

        // virtual function
        else if (tok1->str() == "virtual") {
            hasVirtualSpecifier(true);
        }

        // static function
        else if (tok1->str() == "static") {
            isStatic(true);
            if (scope->type == Scope::eNamespace || scope->type == Scope::eGlobal)
                isStaticLocal(true);
        }

        // friend function
        else if (tok1->str() == "friend") {
            isFriend(true);
        }

        // constexpr function
        else if (tok1->str() == "constexpr") {
            isConstexpr(true);
        }

        // decltype
        else if (tok1->str() == ")" && Token::simpleMatch(tok1->link()->previous(), "decltype (")) {
            tok1 = tok1->link()->previous();
        }

        else if (tok1->link() && tok1->str() == ">") {
            // Function template
            if (Token::simpleMatch(tok1->link()->previous(), "template <")) {
                templateDef = tok1->link()->previous();
                break;
            }
            tok1 = tok1->link();
        }
    }
    return tok1;
}

std::string Function::fullName() const
{
    std::string ret = name();
    for (const Scope *s = nestedIn; s; s = s->nestedIn) {
        if (!s->className.empty())
            ret = s->className + "::" + ret;
    }
    ret += "(";
    for (const Variable &a : argumentList)
        ret += (a.index() == 0 ? "" : ",") + a.name();
    return ret + ")";
}

static std::string qualifiedName(const Scope *scope)
{
    std::string name = scope->className;
    while (scope->nestedIn) {
        if (!scope->nestedIn->className.empty())
            name = (scope->nestedIn->className + " :: ") + name;
        scope = scope->nestedIn;
    }
    return name;
}

static bool usingNamespace(const Scope *scope, const Token *first, const Token *second, int &offset)
{
    // check if qualifications match first before checking if using is needed
    const Token *tok1 = first;
    const Token *tok2 = second;
    bool match = false;
    while (Token::Match(tok1, "%type% :: %type%") && Token::Match(tok2, "%type% :: %type%")) {
        if (tok1->str() == tok2->str()) {
            tok1 = tok1->tokAt(2);
            tok2 = tok2->tokAt(2);
            match = true;
        } else {
            match = false;
            break;
        }
    }

    if (match)
        return false;

    offset = 0;
    std::string name = first->str();

    while (Token::Match(first, "%type% :: %type%")) {
        if (offset)
            name += (" :: " + first->str());
        offset += 2;
        first = first->tokAt(2);
        if (first->str() == second->str()) {
            break;
        }
    }

    if (offset) {
        while (scope) {
            for (const auto & info : scope->usingList) {
                if (info.scope) {
                    if (name == qualifiedName(info.scope))
                        return true;
                }
                // no scope so get name from using
                else {
                    const Token *start = info.start->tokAt(2);
                    std::string nsName;
                    while (start && start->str() != ";") {
                        if (!nsName.empty())
                            nsName += " ";
                        nsName += start->str();
                        start = start->next();
                    }
                    if (nsName == name)
                        return true;
                }
            }
            scope = scope->nestedIn;
        }
    }

    return false;
}

static bool typesMatch(
    const Scope *first_scope,
    const Token *first_token,
    const Scope *second_scope,
    const Token *second_token,
    const Token **new_first,
    const Token **new_second)
{
    // get first type
    const Type* first_type = first_scope->check->findType(first_token, first_scope, /*lookOutside*/ true);
    if (first_type) {
        // get second type
        const Type* second_type = second_scope->check->findType(second_token, second_scope, /*lookOutside*/ true);
        // check if types match
        if (first_type == second_type) {
            const Token* tok1 = first_token;
            while (tok1 && tok1->str() != first_type->name())
                tok1 = tok1->next();
            const Token *tok2 = second_token;
            while (tok2 && tok2->str() != second_type->name())
                tok2 = tok2->next();
            // update parser token positions
            if (tok1 && tok2) {
                *new_first = tok1->previous();
                *new_second = tok2->previous();
                return true;
            }
        }
    }
    return false;
}

bool Function::argsMatch(const Scope *scope, const Token *first, const Token *second, const std::string &path, nonneg int path_length) const
{
    const bool isCPP = scope->check->isCPP();
    if (!isCPP) // C does not support overloads
        return true;

    int arg_path_length = path_length;
    int offset = 0;
    int openParen = 0;

    // check for () == (void) and (void) == ()
    if ((Token::simpleMatch(first, "( )") && Token::simpleMatch(second, "( void )")) ||
        (Token::simpleMatch(first, "( void )") && Token::simpleMatch(second, "( )")))
        return true;

    auto skipTopLevelConst = [](const Token* start) -> const Token* {
        const Token* tok = start->next();
        if (Token::simpleMatch(tok, "const")) {
            tok = tok->next();
            while (Token::Match(tok, "%name%|%type%|::"))
                tok = tok->next();
            if (Token::Match(tok, ",|)|="))
                return start->next();
        }
        return start;
    };

    while (first->str() == second->str() &&
           first->isLong() == second->isLong() &&
           first->isUnsigned() == second->isUnsigned()) {
        if (first->str() == "(")
            openParen++;

        // at end of argument list
        else if (first->str() == ")") {
            if (openParen == 1)
                return true;
            --openParen;
        }

        // skip optional type information
        if (Token::Match(first->next(), "struct|enum|union|class"))
            first = first->next();
        if (Token::Match(second->next(), "struct|enum|union|class"))
            second = second->next();

        // skip const on type passed by value
        const Token* const oldSecond = second;
        first = skipTopLevelConst(first);
        second = skipTopLevelConst(second);

        // skip default value assignment
        if (oldSecond == second && first->next()->str() == "=") {
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
        } else if (oldSecond == second && second->next()->str() == "=") {
            second = second->nextArgument();
            if (second)
                second = second->tokAt(-2);
            if (!second) { // End of argument list (second)
                return false;
            }
        }

        // definition missing variable name
        else if ((first->next()->str() == "," && second->next()->str() != ",") ||
                 (Token::Match(first, "!!( )") && second->next()->str() != ")")) {
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
                 (Token::Match(second, "!!( )") && first->next()->str() != ")")) {
            first = first->next();
            // skip default value assignment
            if (first->next()->str() == "=") {
                do {
                    first = first->next();
                } while (!Token::Match(first->next(), ",|)"));
            }
        } else if (second->next()->str() == "[" && first->next()->str() != "[")
            first = first->next();

        // unnamed parameters
        else if (Token::Match(first, "(|, %type% ,|)") && Token::Match(second, "(|, %type% ,|)")) {
            if (first->next()->expressionString() != second->next()->expressionString())
                break;
            first = first->next();
            second = second->next();
            continue;
        }

        // argument list has different number of arguments
        else if (openParen == 1 && second->str() == ")" && first->str() != ")")
            break;

        // check for type * x == type x[]
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
        else if (first->next()->str() == "*" && second->next()->str() == "*" &&
                 ((first->strAt(2) != "const" && second->strAt(2) == "const") ||
                  (first->strAt(2) == "const" && second->strAt(2) != "const"))) {
            if (first->strAt(2) != "const") {
                if (Token::Match(first->tokAt(2), "%name%| ,|)") && Token::Match(second->tokAt(3), "%name%| ,|)")) {
                    first = first->tokAt(Token::Match(first->tokAt(2), "%name%") ? 2 : 1);
                    second = second->tokAt(Token::Match(second->tokAt(3), "%name%") ? 3 : 2);
                } else {
                    first = first->next();
                    second = second->tokAt(2);
                }
            } else {
                if (Token::Match(second->tokAt(2), "%name%| ,|)") && Token::Match(first->tokAt(3), "%name%| ,|)")) {
                    first = first->tokAt(Token::Match(first->tokAt(3), "%name%") ? 3 : 2);
                    second = second->tokAt(Token::Match(second->tokAt(2), "%name%") ? 2 : 1);
                } else {
                    first = first->tokAt(2);
                    second = second->next();
                }
            }
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

        // using namespace
        else if (usingNamespace(scope, first->next(), second->next(), offset))
            first = first->tokAt(offset);

        // same type with different qualification
        else if (typesMatch(scope, first->next(), nestedIn, second->next(), &first, &second))
            ;

        // variable with class path
        else if (arg_path_length && Token::Match(first->next(), "%name%") && first->strAt(1) != "const") {
            std::string param = path;

            if (Token::simpleMatch(second->next(), param.c_str(), param.size())) {
                // check for redundant qualification before skipping it
                if (!Token::simpleMatch(first->next(), param.c_str(), param.size())) {
                    second = second->tokAt(int(arg_path_length));
                    arg_path_length = 0;
                }
            }

            // nested or base class variable
            else if (arg_path_length <= 2 && Token::Match(first->next(), "%name%") &&
                     (Token::Match(second->next(), "%name% :: %name%") ||
                      (Token::Match(second->next(), "%name% <") &&
                       Token::Match(second->linkAt(1), "> :: %name%"))) &&
                     ((second->next()->str() == scope->className) ||
                      (scope->nestedIn && second->next()->str() == scope->nestedIn->className) ||
                      (scope->definedType && scope->definedType->isDerivedFrom(second->next()->str()))) &&
                     (first->next()->str() == second->strAt(3))) {
                if (Token::Match(second->next(), "%name% <"))
                    second = second->linkAt(1)->next();
                else
                    second = second->tokAt(2);
            }

            // remove class name
            else if (arg_path_length > 2 && first->strAt(1) != second->strAt(1)) {
                std::string short_path = path;
                unsigned int short_path_length = arg_path_length;

                // remove last " :: "
                short_path.resize(short_path.size() - 4);
                short_path_length--;

                // remove last name
                std::string::size_type lastSpace = short_path.find_last_of(' ');
                if (lastSpace != std::string::npos) {
                    short_path.resize(lastSpace+1);
                    short_path_length--;
                    if (short_path[short_path.size() - 1] == '>') {
                        short_path.resize(short_path.size() - 3);
                        while (short_path[short_path.size() - 1] == '<') {
                            lastSpace = short_path.find_last_of(' ');
                            short_path.resize(lastSpace+1);
                            short_path_length--;
                        }
                    }
                }

                param = short_path;
                if (Token::simpleMatch(second->next(), param.c_str(), param.size())) {
                    second = second->tokAt(int(short_path_length));
                    arg_path_length = 0;
                }
            }
        }

        first = first->next();
        second = second->next();

        // reset path length
        if (first->str() == "," || second->str() == ",")
            arg_path_length = path_length;
    }

    return false;
}

static bool isUnknownType(const Token* start, const Token* end)
{
    while (Token::Match(start, "const|volatile"))
        start = start->next();
    start = skipScopeIdentifiers(start);
    if (start->tokAt(1) == end && !start->type() && !start->isStandardType())
        return true;
    // TODO: Try to deduce the type of the expression
    if (Token::Match(start, "decltype|typeof"))
        return true;
    return false;
}

static const Token* getEnableIfReturnType(const Token* start)
{
    if (!start)
        return nullptr;
    for (const Token* tok = start->next(); precedes(tok, start->link()); tok = tok->next()) {
        if (tok->link() && Token::Match(tok, "(|[|{|<")) {
            tok = tok->link();
            continue;
        }
        if (Token::simpleMatch(tok, ","))
            return tok->next();
    }
    return nullptr;
}

template<class Predicate>
static bool checkReturns(const Function* function, bool unknown, bool emptyEnableIf, Predicate pred)
{
    if (!function)
        return false;
    if (function->type != Function::eFunction && function->type != Function::eOperatorEqual)
        return false;
    const Token* defStart = function->retDef;
    if (!defStart)
        return unknown;
    const Token* defEnd = function->returnDefEnd();
    if (!defEnd)
        return unknown;
    if (defEnd == defStart)
        return unknown;
    if (pred(defStart, defEnd))
        return true;
    if (Token::Match(defEnd->tokAt(-1), "*|&|&&"))
        return false;
    // void STDCALL foo()
    while (defEnd->previous() != defStart && Token::Match(defEnd->tokAt(-2), "%name%|> %name%") &&
           !Token::Match(defEnd->tokAt(-2), "const|volatile"))
        defEnd = defEnd->previous();
    // enable_if
    const Token* enableIfEnd = nullptr;
    if (Token::simpleMatch(defEnd->previous(), ">"))
        enableIfEnd = defEnd->previous();
    else if (Token::simpleMatch(defEnd->tokAt(-3), "> :: type"))
        enableIfEnd = defEnd->tokAt(-3);
    if (enableIfEnd && enableIfEnd->link() &&
        Token::Match(enableIfEnd->link()->previous(), "enable_if|enable_if_t|EnableIf")) {
        if (const Token* start = getEnableIfReturnType(enableIfEnd->link())) {
            defStart = start;
            defEnd = enableIfEnd;
        } else {
            return emptyEnableIf;
        }
    }
    assert(defEnd != defStart);
    if (pred(defStart, defEnd))
        return true;
    if (isUnknownType(defStart, defEnd))
        return unknown;
    return false;
}

bool Function::returnsConst(const Function* function, bool unknown)
{
    return checkReturns(function, unknown, false, [](const Token* defStart, const Token* defEnd) {
        return Token::findsimplematch(defStart, "const", defEnd);
    });
}

bool Function::returnsReference(const Function* function, bool unknown, bool includeRValueRef)
{
    return checkReturns(function, unknown, false, [includeRValueRef](const Token* /*defStart*/, const Token* defEnd) {
        return includeRValueRef ? Token::Match(defEnd->previous(), "&|&&") : Token::simpleMatch(defEnd->previous(), "&");
    });
}

bool Function::returnsPointer(const Function* function, bool unknown)
{
    return checkReturns(function, unknown, false, [](const Token* /*defStart*/, const Token* defEnd) {
        return Token::simpleMatch(defEnd->previous(), "*");
    });
}

bool Function::returnsStandardType(const Function* function, bool unknown)
{
    return checkReturns(function, unknown, true, [](const Token* /*defStart*/, const Token* defEnd) {
        return defEnd->previous() && defEnd->previous()->isStandardType();
    });
}

bool Function::returnsVoid(const Function* function, bool unknown)
{
    return checkReturns(function, unknown, true, [](const Token* /*defStart*/, const Token* defEnd) {
        return Token::simpleMatch(defEnd->previous(), "void");
    });
}

std::vector<const Token*> Function::findReturns(const Function* f)
{
    std::vector<const Token*> result;
    if (!f)
        return result;
    const Scope* scope = f->functionScope;
    if (!scope)
        return result;
    if (!scope->bodyStart)
        return result;
    for (const Token* tok = scope->bodyStart->next(); tok && tok != scope->bodyEnd; tok = tok->next()) {
        if (tok->str() == "{" && tok->scope() &&
            (tok->scope()->type == Scope::eLambda || tok->scope()->type == Scope::eClass)) {
            tok = tok->link();
            continue;
        }
        if (Token::simpleMatch(tok->astParent(), "return")) {
            result.push_back(tok);
        }
        // Skip lambda functions since the scope may not be set correctly
        const Token* lambdaEndToken = findLambdaEndToken(tok);
        if (lambdaEndToken) {
            tok = lambdaEndToken;
        }
    }
    return result;
}

const Token * Function::constructorMemberInitialization() const
{
    if (!isConstructor() || !arg)
        return nullptr;
    if (Token::simpleMatch(arg->link(), ") :"))
        return arg->link()->next();
    if (Token::simpleMatch(arg->link(), ") noexcept (") && arg->link()->linkAt(2)->strAt(1) == ":")
        return arg->link()->linkAt(2)->next();
    return nullptr;
}

bool Function::isSafe(const Settings *settings) const
{
    if (settings->safeChecks.externalFunctions) {
        if (nestedIn->type == Scope::ScopeType::eNamespace && token->fileIndex() != 0)
            return true;
        if (nestedIn->type == Scope::ScopeType::eGlobal && (token->fileIndex() != 0 || !isStatic()))
            return true;
    }

    if (settings->safeChecks.internalFunctions) {
        if (nestedIn->type == Scope::ScopeType::eNamespace && token->fileIndex() == 0)
            return true;
        if (nestedIn->type == Scope::ScopeType::eGlobal && (token->fileIndex() == 0 || isStatic()))
            return true;
    }

    if (settings->safeChecks.classes && access == AccessControl::Public && (nestedIn->type == Scope::ScopeType::eClass || nestedIn->type == Scope::ScopeType::eStruct))
        return true;

    return false;
}

Function* SymbolDatabase::addGlobalFunction(Scope*& scope, const Token*& tok, const Token *argStart, const Token* funcStart)
{
    Function* function = nullptr;
    // Lambda functions are always unique
    if (tok->str() != "[") {
        auto range = scope->functionMap.equal_range(tok->str());
        for (std::multimap<std::string, const Function*>::const_iterator it = range.first; it != range.second; ++it) {
            const Function *f = it->second;
            if (f->hasBody())
                continue;
            if (f->argsMatch(scope, f->argDef, argStart, emptyString, 0)) {
                function = const_cast<Function *>(it->second);
                break;
            }
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
    Function function(&mTokenizer, tok, scope, funcStart, argStart);
    scope->addFunction(std::move(function));
    return &scope->functionList.back();
}

void SymbolDatabase::addClassFunction(Scope **scope, const Token **tok, const Token *argStart)
{
    const bool destructor((*tok)->previous()->str() == "~");
    const bool has_const(argStart->link()->strAt(1) == "const");
    const bool lval(argStart->link()->strAt(has_const ? 2 : 1) == "&");
    const bool rval(argStart->link()->strAt(has_const ? 2 : 1) == "&&");
    int count = 0;
    std::string path;
    unsigned int path_length = 0;
    const Token *tok1 = (*tok);

    if (destructor)
        tok1 = tok1->previous();

    // back up to head of path
    while (tok1 && tok1->previous() && tok1->previous()->str() == "::" && tok1->tokAt(-2) &&
           ((tok1->tokAt(-2)->isName() && !tok1->tokAt(-2)->isStandardType()) ||
            (tok1->strAt(-2) == ">" && tok1->linkAt(-2) && Token::Match(tok1->linkAt(-2)->previous(), "%name%")))) {
        count++;
        const Token * tok2 = tok1->tokAt(-2);
        if (tok2->str() == ">")
            tok2 = tok2->link()->previous();

        if (tok2) {
            do {
                path = tok1->previous()->str() + " " + path;
                tok1 = tok1->previous();
                path_length++;
            } while (tok1 != tok2);
        } else
            return; // syntax error ?
    }

    // syntax error?
    if (!tok1)
        return;

    // add global namespace if present
    if (tok1->strAt(-1) == "::") {
        path_length++;
        path.insert(0, ":: ");
    }

    std::list<Scope>::iterator it1;

    // search for match
    for (it1 = scopeList.begin(); it1 != scopeList.end(); ++it1) {
        Scope *scope1 = &(*it1);

        bool match = false;

        // check in namespace if using found
        if (*scope == scope1 && !scope1->usingList.empty()) {
            std::vector<Scope::UsingInfo>::const_iterator it2;
            for (it2 = scope1->usingList.cbegin(); it2 != scope1->usingList.cend(); ++it2) {
                if (it2->scope) {
                    Function * func = findFunctionInScope(tok1, it2->scope, path, path_length);
                    if (func) {
                        if (!func->hasBody()) {
                            const Token *closeParen = (*tok)->next()->link();
                            if (closeParen) {
                                const Token *eq = mTokenizer.isFunctionHead(closeParen, ";");
                                if (eq && Token::simpleMatch(eq->tokAt(-2), "= default ;")) {
                                    func->isDefault(true);
                                    return;
                                }
                            }
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

                while (scope2 && count > 1) {
                    count--;
                    if (tok1->strAt(1) == "<")
                        tok1 = tok1->linkAt(1)->tokAt(2);
                    else
                        tok1 = tok1->tokAt(2);
                    scope2 = scope2->findRecordInNestedList(tok1->str());
                }

                if (count == 1 && scope2) {
                    match = true;
                    scope1 = scope2;
                }
            }
        }

        if (match) {
            auto range = scope1->functionMap.equal_range((*tok)->str());
            for (std::multimap<std::string, const Function*>::const_iterator it = range.first; it != range.second; ++it) {
                Function * func = const_cast<Function *>(it->second);
                if (!func->hasBody()) {
                    if (func->argsMatch(scope1, func->argDef, (*tok)->next(), path, path_length)) {
                        const Token *closeParen = (*tok)->next()->link();
                        if (closeParen) {
                            const Token *eq = mTokenizer.isFunctionHead(closeParen, ";");
                            if (eq && Token::simpleMatch(eq->tokAt(-2), "= default ;")) {
                                func->isDefault(true);
                                return;
                            }
                            if (func->type == Function::eDestructor && destructor) {
                                func->hasBody(true);
                            } else if (func->type != Function::eDestructor && !destructor) {
                                // normal function?
                                const bool hasConstKeyword = closeParen->next()->str() == "const";
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
    scopeList.emplace_back(this, tok1, *scope);
    Scope *newScope = &scopeList.back();

    // find start of function '{'
    bool foundInitList = false;
    while (tok1 && tok1->str() != "{" && tok1->str() != ";") {
        if (tok1->link() && Token::Match(tok1, "(|[|<")) {
            tok1 = tok1->link();
        } else if (foundInitList && Token::Match(tok1, "%name%|> {") && Token::Match(tok1->linkAt(1), "} ,|{")) {
            tok1 = tok1->linkAt(1);
        } else {
            if (tok1->str() == ":")
                foundInitList = true;
            tok1 = tok1->next();
        }
    }

    if (tok1 && tok1->str() == "{") {
        newScope->setBodyStartEnd(tok1);

        // syntax error?
        if (!newScope->bodyEnd) {
            mTokenizer.unmatchedToken(tok1);
        } else {
            (*scope)->nestedList.push_back(newScope);
            *scope = newScope;
        }
    } else if (tok1 && Token::Match(tok1->tokAt(-2), "= default|delete ;")) {
        scopeList.pop_back();
    } else {
        throw InternalError(*tok, "Analysis failed (function not recognized). If the code is valid then please report this failure.");
    }
    *tok = tok1;
}

bool Type::isClassType() const
{
    return classScope && classScope->type == Scope::ScopeType::eClass;
}

bool Type::isEnumType() const
{
    //We explicitly check for "enum" because a forward declared enum doesn't get its own scope
    return (classDef && classDef->str() == "enum") ||
           (classScope && classScope->type == Scope::ScopeType::eEnum);
}

bool Type::isStructType() const
{
    return classScope && classScope->type == Scope::ScopeType::eStruct;
}

bool Type::isUnionType() const
{
    return classScope && classScope->type == Scope::ScopeType::eUnion;
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
                base.access = AccessControl::Public;
                tok2 = tok2->next();
            } else if (tok2->str() == "protected") {
                base.access = AccessControl::Protected;
                tok2 = tok2->next();
            } else if (tok2->str() == "private") {
                base.access = AccessControl::Private;
                tok2 = tok2->next();
            } else {
                if (tok->str() == "class")
                    base.access = AccessControl::Private;
                else if (tok->str() == "struct")
                    base.access = AccessControl::Public;
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

            const Type * baseType = classScope->check->findType(base.nameTok, enclosingScope);
            if (baseType && !baseType->findDependency(this))
                base.type = baseType;

            // save pattern for base class name
            derivedFrom.push_back(std::move(base));
        } else
            tok2 = tok2->next();
    }

    return tok2;
}

std::string Type::name() const
{
    const Token* start = classDef->next();
    if (classScope && classScope->enumClass && isEnumType())
        start = start->tokAt(1);
    else if (start->str() == "class")
        start = start->tokAt(1);
    else if (!start->isName())
        return emptyString;
    const Token* next = start;
    while (Token::Match(next, "::|<|>|(|)|[|]|*|&|&&|%name%")) {
        if (Token::Match(next, "<|(|[") && next->link())
            next = next->link();
        next = next->next();
    }
    std::string result;
    for (const Token* tok = start; tok != next; tok = tok->next()) {
        if (!result.empty())
            result += ' ';
        result += tok->str();
    }
    return result;
}

void SymbolDatabase::debugMessage(const Token *tok, const std::string &type, const std::string &msg) const
{
    if (tok && mSettings.debugwarnings && mErrorLogger) {
        const std::list<const Token*> locationList(1, tok);
        const ErrorMessage errmsg(locationList, &mTokenizer.list,
                                  Severity::debug,
                                  type,
                                  msg,
                                  Certainty::normal);
        mErrorLogger->reportErr(errmsg);
    }
}

const Function* Type::getFunction(const std::string& funcName) const
{
    if (classScope) {
        const std::multimap<std::string, const Function *>::const_iterator it = classScope->functionMap.find(funcName);

        if (it != classScope->functionMap.end())
            return it->second;
    }

    for (const Type::BaseInfo & i : derivedFrom) {
        if (i.type) {
            const Function* const func = i.type->getFunction(funcName);
            if (func)
                return func;
        }
    }
    return nullptr;
}

bool Type::hasCircularDependencies(std::set<BaseInfo>* ancestors) const
{
    std::set<BaseInfo> knownAncestors;
    if (!ancestors) {
        ancestors=&knownAncestors;
    }
    for (std::vector<BaseInfo>::const_iterator parent=derivedFrom.cbegin(); parent!=derivedFrom.cend(); ++parent) {
        if (!parent->type)
            continue;
        if (this==parent->type)
            return true;
        if (ancestors->find(*parent)!=ancestors->end())
            return true;

        ancestors->insert(*parent);
        if (parent->type->hasCircularDependencies(ancestors))
            return true;
    }
    return false;
}

bool Type::findDependency(const Type* ancestor) const
{
    return this == ancestor || std::any_of(derivedFrom.cbegin(), derivedFrom.cend(), [&](const BaseInfo& d) {
        return d.type && (d.type == this || d.type->findDependency(ancestor));
    });
}

bool Type::isDerivedFrom(const std::string & ancestor) const
{
    for (std::vector<BaseInfo>::const_iterator parent=derivedFrom.cbegin(); parent!=derivedFrom.cend(); ++parent) {
        if (parent->name == ancestor)
            return true;
        if (parent->type && parent->type->isDerivedFrom(ancestor))
            return true;
    }
    return false;
}

bool Variable::arrayDimensions(const Settings* settings, bool& isContainer)
{
    isContainer = false;
    const Library::Container* container = (mTypeStartToken && mTypeStartToken->isCpp()) ? settings->library.detectContainer(mTypeStartToken) : nullptr;
    if (container && container->arrayLike_indexOp && container->size_templateArgNo > 0) {
        const Token* tok = Token::findsimplematch(mTypeStartToken, "<");
        if (tok) {
            isContainer = true;
            Dimension dimension_;
            tok = tok->next();
            for (int i = 0; i < container->size_templateArgNo && tok; i++) {
                tok = tok->nextTemplateArgument();
            }
            if (Token::Match(tok, "%num% [,>]")) {
                dimension_.tok = tok;
                dimension_.known = true;
                dimension_.num = MathLib::toBigNumber(tok->str());
            } else if (tok) {
                dimension_.tok = tok;
                dimension_.known = false;
            }
            mDimensions.push_back(dimension_);
            return true;
        }
    }

    const Token *dim = mNameToken;
    if (!dim) {
        // Argument without name
        dim = mTypeEndToken;
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
        dimension_.known = false;
        // check for empty array dimension []
        if (dim->next()->str() != "]") {
            dimension_.tok = dim->astOperand2();
            ValueFlow::valueFlowConstantFoldAST(const_cast<Token *>(dimension_.tok), settings);
            if (dimension_.tok && dimension_.tok->hasKnownIntValue()) {
                dimension_.num = dimension_.tok->getKnownIntValue();
                dimension_.known = true;
            }
        }
        mDimensions.push_back(dimension_);
        dim = dim->link()->next();
        arr = true;
    }
    return arr;
}

static std::string scopeTypeToString(Scope::ScopeType type)
{
    switch (type) {
    case Scope::ScopeType::eGlobal:
        return "Global";
    case Scope::ScopeType::eClass:
        return "Class";
    case Scope::ScopeType::eStruct:
        return "Struct";
    case Scope::ScopeType::eUnion:
        return "Union";
    case Scope::ScopeType::eNamespace:
        return "Namespace";
    case Scope::ScopeType::eFunction:
        return "Function";
    case Scope::ScopeType::eIf:
        return "If";
    case Scope::ScopeType::eElse:
        return "Else";
    case Scope::ScopeType::eFor:
        return "For";
    case Scope::ScopeType::eWhile:
        return "While";
    case Scope::ScopeType::eDo:
        return "Do";
    case Scope::ScopeType::eSwitch:
        return "Switch";
    case Scope::ScopeType::eTry:
        return "Try";
    case Scope::ScopeType::eCatch:
        return "Catch";
    case Scope::ScopeType::eUnconditional:
        return "Unconditional";
    case Scope::ScopeType::eLambda:
        return "Lambda";
    case Scope::ScopeType::eEnum:
        return "Enum";
    }
    return "Unknown";
}

static std::ostream & operator << (std::ostream & s, Scope::ScopeType type)
{
    s << scopeTypeToString(type);
    return s;
}

static std::string accessControlToString(AccessControl access)
{
    switch (access) {
    case AccessControl::Public:
        return "Public";
    case AccessControl::Protected:
        return "Protected";
    case AccessControl::Private:
        return "Private";
    case AccessControl::Global:
        return "Global";
    case AccessControl::Namespace:
        return "Namespace";
    case AccessControl::Argument:
        return "Argument";
    case AccessControl::Local:
        return "Local";
    case AccessControl::Throw:
        return "Throw";
    }
    return "Unknown";
}

static std::string tokenToString(const Token* tok, const Tokenizer& tokenizer)
{
    std::ostringstream oss;
    if (tok) {
        oss << tok->str() << " ";
        oss << tokenizer.list.fileLine(tok) << " ";
    }
    oss << tok;
    return oss.str();
}

static std::string scopeToString(const Scope* scope, const Tokenizer& tokenizer)
{
    std::ostringstream oss;
    if (scope) {
        oss << scope->type << " ";
        if (!scope->className.empty())
            oss << scope->className << " ";
        if (scope->classDef)
            oss << tokenizer.list.fileLine(scope->classDef) << " ";
    }
    oss << scope;
    return oss.str();
}

static std::string tokenType(const Token * tok)
{
    std::ostringstream oss;
    if (tok) {
        if (tok->isUnsigned())
            oss << "unsigned ";
        else if (tok->isSigned())
            oss << "signed ";
        if (tok->isComplex())
            oss << "_Complex ";
        if (tok->isLong())
            oss << "long ";
        oss << tok->str();
    }
    return oss.str();
}

void SymbolDatabase::printVariable(const Variable *var, const char *indent) const
{
    std::cout << indent << "mNameToken: " << tokenToString(var->nameToken(), mTokenizer) << std::endl;
    if (var->nameToken()) {
        std::cout << indent << "    declarationId: " << var->declarationId() << std::endl;
    }
    std::cout << indent << "mTypeStartToken: " << tokenToString(var->typeStartToken(), mTokenizer) << std::endl;
    std::cout << indent << "mTypeEndToken: " << tokenToString(var->typeEndToken(), mTokenizer) << std::endl;

    if (var->typeStartToken()) {
        const Token * autoTok = nullptr;
        std::cout << indent << "   ";
        for (const Token * tok = var->typeStartToken(); tok != var->typeEndToken()->next(); tok = tok->next()) {
            std::cout << " " << tokenType(tok);
            if (tok->str() == "auto")
                autoTok = tok;
        }
        std::cout << std::endl;
        if (autoTok) {
            const ValueType * valueType = autoTok->valueType();
            std::cout << indent << "    auto valueType: " << valueType << std::endl;
            if (var->typeStartToken()->valueType()) {
                std::cout << indent << "        " << valueType->str() << std::endl;
            }
        }
    } else if (var->valueType()) {
        std::cout << indent << "   " << var->valueType()->str() << std::endl;
    }
    std::cout << indent << "mIndex: " << var->index() << std::endl;
    std::cout << indent << "mAccess: " << accessControlToString(var->accessControl()) << std::endl;
    std::cout << indent << "mFlags: " << std::endl;
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
    std::cout << indent << "mType: ";
    if (var->type()) {
        std::cout << var->type()->type() << " " << var->type()->name();
        std::cout << " " << mTokenizer.list.fileLine(var->type()->classDef);
        std::cout << " " << var->type() << std::endl;
    } else
        std::cout << "none" << std::endl;

    if (var->nameToken()) {
        const ValueType * valueType = var->nameToken()->valueType();
        std::cout << indent << "valueType: " << valueType << std::endl;
        if (valueType) {
            std::cout << indent << "    " << valueType->str() << std::endl;
        }
    }

    std::cout << indent << "mScope: " << scopeToString(var->scope(), mTokenizer) << std::endl;

    std::cout << indent << "mDimensions:";
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

    for (std::list<Scope>::const_iterator scope = scopeList.cbegin(); scope != scopeList.cend(); ++scope) {
        std::cout << "Scope: " << &*scope << " " << scope->type << std::endl;
        std::cout << "    className: " << scope->className << std::endl;
        std::cout << "    classDef: " << tokenToString(scope->classDef, mTokenizer) << std::endl;
        std::cout << "    bodyStart: " << tokenToString(scope->bodyStart, mTokenizer) << std::endl;
        std::cout << "    bodyEnd: " << tokenToString(scope->bodyEnd, mTokenizer) << std::endl;

        // find the function body if not implemented inline
        for (auto func = scope->functionList.cbegin(); func != scope->functionList.cend(); ++func) {
            std::cout << "    Function: " << &*func << std::endl;
            std::cout << "        name: " << tokenToString(func->tokenDef, mTokenizer) << std::endl;
            std::cout << "        type: " << (func->type == Function::eConstructor? "Constructor" :
                                              func->type == Function::eCopyConstructor ? "CopyConstructor" :
                                              func->type == Function::eMoveConstructor ? "MoveConstructor" :
                                              func->type == Function::eOperatorEqual ? "OperatorEqual" :
                                              func->type == Function::eDestructor ? "Destructor" :
                                              func->type == Function::eFunction ? "Function" :
                                              func->type == Function::eLambda ? "Lambda" :
                                              "Unknown") << std::endl;
            std::cout << "        access: " << accessControlToString(func->access) << std::endl;
            std::cout << "        hasBody: " << func->hasBody() << std::endl;
            std::cout << "        isInline: " << func->isInline() << std::endl;
            std::cout << "        isConst: " << func->isConst() << std::endl;
            std::cout << "        hasVirtualSpecifier: " << func->hasVirtualSpecifier() << std::endl;
            std::cout << "        isPure: " << func->isPure() << std::endl;
            std::cout << "        isStatic: " << func->isStatic() << std::endl;
            std::cout << "        isStaticLocal: " << func->isStaticLocal() << std::endl;
            std::cout << "        isExtern: " << func->isExtern() << std::endl;
            std::cout << "        isFriend: " << func->isFriend() << std::endl;
            std::cout << "        isExplicit: " << func->isExplicit() << std::endl;
            std::cout << "        isDefault: " << func->isDefault() << std::endl;
            std::cout << "        isDelete: " << func->isDelete() << std::endl;
            std::cout << "        hasOverrideSpecifier: " << func->hasOverrideSpecifier() << std::endl;
            std::cout << "        hasFinalSpecifier: " << func->hasFinalSpecifier() << std::endl;
            std::cout << "        isNoExcept: " << func->isNoExcept() << std::endl;
            std::cout << "        isThrow: " << func->isThrow() << std::endl;
            std::cout << "        isOperator: " << func->isOperator() << std::endl;
            std::cout << "        hasLvalRefQual: " << func->hasLvalRefQualifier() << std::endl;
            std::cout << "        hasRvalRefQual: " << func->hasRvalRefQualifier() << std::endl;
            std::cout << "        isVariadic: " << func->isVariadic() << std::endl;
            std::cout << "        isVolatile: " << func->isVolatile() << std::endl;
            std::cout << "        hasTrailingReturnType: " << func->hasTrailingReturnType() << std::endl;
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
            if (func->isAttributeNodiscard())
                std::cout << " nodiscard ";
            std::cout << std::endl;
            std::cout << "        noexceptArg: " << (func->noexceptArg ? func->noexceptArg->str() : "none") << std::endl;
            std::cout << "        throwArg: " << (func->throwArg ? func->throwArg->str() : "none") << std::endl;
            std::cout << "        tokenDef: " << tokenToString(func->tokenDef, mTokenizer) << std::endl;
            std::cout << "        argDef: " << tokenToString(func->argDef, mTokenizer) << std::endl;
            if (!func->isConstructor() && !func->isDestructor())
                std::cout << "        retDef: " << tokenToString(func->retDef, mTokenizer) << std::endl;
            if (func->retDef) {
                std::cout << "           ";
                for (const Token * tok = func->retDef; tok && tok != func->tokenDef && !Token::Match(tok, "{|;|override|final"); tok = tok->next())
                    std::cout << " " << tokenType(tok);
                std::cout << std::endl;
            }
            std::cout << "        retType: " << func->retType << std::endl;

            if (const ValueType* valueType = func->tokenDef->next()->valueType()) {
                std::cout << "        valueType: " << valueType << std::endl;
                std::cout << "            " << valueType->str() << std::endl;
            }

            if (func->hasBody()) {
                std::cout << "        token: " << tokenToString(func->token, mTokenizer) << std::endl;
                std::cout << "        arg: " << tokenToString(func->arg, mTokenizer) << std::endl;
            }
            std::cout << "        nestedIn: " << scopeToString(func->nestedIn, mTokenizer) << std::endl;
            std::cout << "        functionScope: " << scopeToString(func->functionScope, mTokenizer) << std::endl;

            for (auto var = func->argumentList.cbegin(); var != func->argumentList.cend(); ++var) {
                std::cout << "        Variable: " << &*var << std::endl;
                printVariable(&*var, "            ");
            }
        }

        for (auto var = scope->varlist.cbegin(); var != scope->varlist.cend(); ++var) {
            std::cout << "    Variable: " << &*var << std::endl;
            printVariable(&*var, "        ");
        }

        if (scope->type == Scope::eEnum) {
            std::cout << "    enumType: ";
            if (scope->enumType) {
                std::cout << scope->enumType->stringify(false, true, false);
            } else
                std::cout << "int";
            std::cout << std::endl;
            std::cout << "    enumClass: " << scope->enumClass << std::endl;
            for (const Enumerator &enumerator : scope->enumeratorList) {
                std::cout << "        Enumerator: " << enumerator.name->str() << " = ";
                if (enumerator.value_known)
                    std::cout << enumerator.value;

                if (enumerator.start) {
                    const Token * tok = enumerator.start;
                    std::cout << (enumerator.value_known ? " " : "") << "[" << tok->str();
                    while (tok && tok != enumerator.end) {
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

        std::size_t count = scope->nestedList.size();
        for (std::vector<Scope*>::const_iterator nsi = scope->nestedList.cbegin(); nsi != scope->nestedList.cend(); ++nsi) {
            std::cout << " " << (*nsi) << " " << (*nsi)->type << " " << (*nsi)->className;
            if (count-- > 1)
                std::cout << ",";
        }

        std::cout << " )" << std::endl;

        for (auto use = scope->usingList.cbegin(); use != scope->usingList.cend(); ++use) {
            std::cout << "    using: " << use->scope << " " << use->start->strAt(2);
            const Token *tok1 = use->start->tokAt(3);
            while (tok1 && tok1->str() == "::") {
                std::cout << "::" << tok1->strAt(1);
                tok1 = tok1->tokAt(2);
            }
            std::cout << " " << mTokenizer.list.fileLine(use->start) << std::endl;
        }

        std::cout << "    functionOf: " << scopeToString(scope->functionOf, mTokenizer) << std::endl;

        std::cout << "    function: " << scope->function;
        if (scope->function)
            std::cout << " " << scope->function->name();
        std::cout << std::endl;
    }

    for (std::list<Type>::const_iterator type = typeList.cbegin(); type != typeList.cend(); ++type) {
        std::cout << "Type: " << &(*type) << std::endl;
        std::cout << "    name: " << type->name() << std::endl;
        std::cout << "    classDef: " << tokenToString(type->classDef, mTokenizer) << std::endl;
        std::cout << "    classScope: " << type->classScope << std::endl;
        std::cout << "    enclosingScope: " << type->enclosingScope;
        if (type->enclosingScope) {
            std::cout << " " << type->enclosingScope->type << " "
                      << type->enclosingScope->className;
        }
        std::cout << std::endl;
        std::cout << "    needInitialization: " << (type->needInitialization == Type::NeedInitialization::Unknown ? "Unknown" :
                                                    type->needInitialization == Type::NeedInitialization::True ? "True" :
                                                    type->needInitialization == Type::NeedInitialization::False ? "False" :
                                                    "Invalid") << std::endl;

        std::cout << "    derivedFrom[" << type->derivedFrom.size() << "] = (";
        std::size_t count = type->derivedFrom.size();
        for (const Type::BaseInfo & i : type->derivedFrom) {
            if (i.isVirtual)
                std::cout << "Virtual ";

            std::cout << (i.access == AccessControl::Public    ? " Public" :
                          i.access == AccessControl::Protected ? " Protected" :
                          i.access == AccessControl::Private   ? " Private" :
                          " Unknown");

            if (i.type)
                std::cout << " " << i.type;
            else
                std::cout << " Unknown";

            std::cout << " " << i.name;
            if (count-- > 1)
                std::cout << ",";
        }

        std::cout << " )" << std::endl;

        std::cout << "    friendList[" << type->friendList.size() << "] = (";
        for (size_t i = 0; i < type->friendList.size(); i++) {
            if (type->friendList[i].type)
                std::cout << type->friendList[i].type;
            else
                std::cout << " Unknown";

            std::cout << ' ';
            if (type->friendList[i].nameEnd)
                std::cout << type->friendList[i].nameEnd->str();
            if (i+1 < type->friendList.size())
                std::cout << ',';
        }

        std::cout << " )" << std::endl;
    }

    for (std::size_t i = 1; i < mVariableList.size(); i++) {
        std::cout << "mVariableList[" << i << "]: " << mVariableList[i];
        if (mVariableList[i]) {
            std::cout << " " << mVariableList[i]->name() << " "
                      << mTokenizer.list.fileLine(mVariableList[i]->nameToken());
        }
        std::cout << std::endl;
    }
    std::cout << std::resetiosflags(std::ios::boolalpha);
}

void SymbolDatabase::printXml(std::ostream &out) const
{
    std::string outs;

    std::set<const Variable *> variables;

    // Scopes..
    outs += "  <scopes>\n";
    for (std::list<Scope>::const_iterator scope = scopeList.cbegin(); scope != scopeList.cend(); ++scope) {
        outs += "    <scope";
        outs += " id=\"";
        outs += id_string(&*scope);
        outs += "\"";
        outs += " type=\"";
        outs += scopeTypeToString(scope->type);
        outs += "\"";
        if (!scope->className.empty()) {
            outs += " className=\"";
            outs += ErrorLogger::toxml(scope->className);
            outs += "\"";
        }
        if (scope->bodyStart) {
            outs += " bodyStart=\"";
            outs += id_string(scope->bodyStart);
            outs += '\"';
        }
        if (scope->bodyEnd) {
            outs += " bodyEnd=\"";
            outs += id_string(scope->bodyEnd);
            outs += '\"';
        }
        if (scope->nestedIn) {
            outs += " nestedIn=\"";
            outs += id_string(scope->nestedIn);
            outs += "\"";
        }
        if (scope->function) {
            outs += " function=\"";
            outs += id_string(scope->function);
            outs += "\"";
        }
        if (scope->definedType) {
            outs += " definedType=\"";
            outs += id_string(scope->definedType);
            outs += "\"";
        }
        if (scope->functionList.empty() && scope->varlist.empty())
            outs += "/>\n";
        else {
            outs += ">\n";
            if (!scope->functionList.empty()) {
                outs += "      <functionList>\n";
                for (std::list<Function>::const_iterator function = scope->functionList.cbegin(); function != scope->functionList.cend(); ++function) {
                    outs += "        <function id=\"";
                    outs += id_string(&*function);
                    outs += "\" token=\"";
                    outs += id_string(function->token);
                    outs += "\" tokenDef=\"";
                    outs += id_string(function->tokenDef);
                    outs += "\" name=\"";
                    outs += ErrorLogger::toxml(function->name());
                    outs += '\"';
                    outs += " type=\"";
                    outs += (function->type == Function::eConstructor? "Constructor" :
                             function->type == Function::eCopyConstructor ? "CopyConstructor" :
                             function->type == Function::eMoveConstructor ? "MoveConstructor" :
                             function->type == Function::eOperatorEqual ? "OperatorEqual" :
                             function->type == Function::eDestructor ? "Destructor" :
                             function->type == Function::eFunction ? "Function" :
                             function->type == Function::eLambda ? "Lambda" :
                             "Unknown");
                    outs += '\"';
                    if (function->nestedIn->definedType) {
                        if (function->hasVirtualSpecifier())
                            outs += " hasVirtualSpecifier=\"true\"";
                        else if (function->isImplicitlyVirtual())
                            outs += " isImplicitlyVirtual=\"true\"";
                    }
                    if (function->access == AccessControl::Public || function->access == AccessControl::Protected || function->access == AccessControl::Private) {
                        outs += " access=\"";
                        outs += accessControlToString(function->access);
                        outs +="\"";
                    }
                    if (function->isInlineKeyword())
                        outs += " isInlineKeyword=\"true\"";
                    if (function->isStatic())
                        outs += " isStatic=\"true\"";
                    if (function->isAttributeNoreturn())
                        outs += " isAttributeNoreturn=\"true\"";
                    if (const Function* overriddenFunction = function->getOverriddenFunction()) {
                        outs += " overriddenFunction=\"";
                        outs += id_string(overriddenFunction);
                        outs += "\"";
                    }
                    if (function->argCount() == 0U)
                        outs += "/>\n";
                    else {
                        outs += ">\n";
                        for (unsigned int argnr = 0; argnr < function->argCount(); ++argnr) {
                            const Variable *arg = function->getArgumentVar(argnr);
                            outs += "          <arg nr=\"";
                            outs += std::to_string(argnr+1);
                            outs += "\" variable=\"";
                            outs += id_string(arg);
                            outs += "\"/>\n";
                            variables.insert(arg);
                        }
                        outs += "        </function>\n";
                    }
                }
                outs += "      </functionList>\n";
            }
            if (!scope->varlist.empty()) {
                outs += "      <varlist>\n";
                for (std::list<Variable>::const_iterator var = scope->varlist.cbegin(); var != scope->varlist.cend(); ++var) {
                    outs += "        <var id=\"";
                    outs += id_string(&*var);
                    outs += "\"/>\n";
                }
                outs += "      </varlist>\n";
            }
            outs += "    </scope>\n";
        }
    }
    outs += "  </scopes>\n";

    if (!typeList.empty()) {
        outs += "  <types>\n";
        for (const Type& type:typeList) {
            outs += "    <type id=\"";
            outs += id_string(&type);
            outs += "\" classScope=\"";
            outs += id_string(type.classScope);
            outs += "\"";
            if (type.derivedFrom.empty()) {
                outs += "/>\n";
                continue;
            }
            outs += ">\n";
            for (const Type::BaseInfo& baseInfo: type.derivedFrom) {
                outs += "      <derivedFrom";
                outs += " access=\"";
                outs += accessControlToString(baseInfo.access);
                outs += "\"";
                outs += " type=\"";
                outs += id_string(baseInfo.type);
                outs += "\"";
                outs += " isVirtual=\"";
                outs += bool_to_string(baseInfo.isVirtual);
                outs += "\"";
                outs += " nameTok=\"";
                outs += id_string(baseInfo.nameTok);
                outs += "\"";
                outs += "/>\n";
            }
            outs += "    </type>\n";
        }
        outs += "  </types>\n";
    }

    // Variables..
    for (const Variable *var : mVariableList)
        variables.insert(var);
    outs += "  <variables>\n";
    for (const Variable *var : variables) {
        if (!var)
            continue;
        outs += "    <var id=\"";
        outs += id_string(var);
        outs += '\"';
        outs += " nameToken=\"";
        outs += id_string(var->nameToken());
        outs += '\"';
        outs += " typeStartToken=\"";
        outs += id_string(var->typeStartToken());
        outs += '\"';
        outs += " typeEndToken=\"";
        outs += id_string(var->typeEndToken());
        outs += '\"';
        outs += " access=\"";
        outs += accessControlToString(var->mAccess);
        outs += '\"';
        outs += " scope=\"";
        outs += id_string(var->scope());
        outs += '\"';
        if (var->valueType()) {
            outs += " constness=\"";
            outs += std::to_string(var->valueType()->constness);
            outs += '\"';
        }
        outs += " isArray=\"";
        outs += bool_to_string(var->isArray());
        outs += '\"';
        outs += " isClass=\"";
        outs += bool_to_string(var->isClass());
        outs += '\"';
        outs += " isConst=\"";
        outs += bool_to_string(var->isConst());
        outs += '\"';
        outs += " isExtern=\"";
        outs += bool_to_string(var->isExtern());
        outs += '\"';
        outs += " isPointer=\"";
        outs += bool_to_string(var->isPointer());
        outs += '\"';
        outs += " isReference=\"";
        outs += bool_to_string(var->isReference());
        outs += '\"';
        outs += " isStatic=\"";
        outs += bool_to_string(var->isStatic());
        outs += '\"';
        outs += " isVolatile=\"";
        outs += bool_to_string(var->isVolatile());
        outs += '\"';
        outs += "/>\n";
    }
    outs += "  </variables>\n";

    out << outs;
}

//---------------------------------------------------------------------------

static const Type* findVariableTypeIncludingUsedNamespaces(const SymbolDatabase* symbolDatabase, const Scope* scope, const Token* typeTok)
{
    const Type* argType = symbolDatabase->findVariableType(scope, typeTok);
    if (argType)
        return argType;

    // look for variable type in any using namespace in this scope or above
    while (scope) {
        for (const Scope::UsingInfo &ui : scope->usingList) {
            if (ui.scope) {
                argType = symbolDatabase->findVariableType(ui.scope, typeTok);
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
    if (!Token::simpleMatch(start, "("))
        return;
    if (!(start && start->link() != start->next() && !Token::simpleMatch(start, "( void )")))
        return;

    unsigned int count = 0;

    for (const Token* tok = start->next(); tok; tok = tok->next()) {
        if (Token::Match(tok, ",|)"))
            return; // Syntax error

        const Token* startTok = tok;
        const Token* endTok   = nullptr;
        const Token* nameTok  = nullptr;

        do {
            if (Token::simpleMatch(tok, "decltype (")) {
                tok = tok->linkAt(1)->next();
                continue;
            }
            if (tok != startTok && !nameTok && Token::Match(tok, "( & %var% ) [")) {
                nameTok = tok->tokAt(2);
                endTok = nameTok->previous();
                tok = tok->link();
            } else if (tok != startTok && !nameTok && Token::Match(tok, "( * %var% ) (") && Token::Match(tok->link()->linkAt(1), ") [,)]")) {
                nameTok = tok->tokAt(2);
                endTok = nameTok->previous();
                tok = tok->link()->linkAt(1);
            } else if (tok != startTok && !nameTok && Token::Match(tok, "( * %var% ) [")) {
                nameTok = tok->tokAt(2);
                endTok = nameTok->previous();
                tok = tok->link();
            } else if (tok->varId() != 0) {
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
        while (Token::Match(typeTok, "const|volatile|enum|struct|::"))
            typeTok = typeTok->next();
        if (Token::Match(typeTok, ",|)")) { // #8333
            symbolDatabase->mTokenizer.syntaxError(typeTok);
        }
        if (Token::Match(typeTok, "%type% <") && Token::Match(typeTok->linkAt(1), "> :: %type%"))
            typeTok = typeTok->linkAt(1)->tokAt(2);
        // skip over qualification
        while (Token::Match(typeTok, "%type% ::")) {
            typeTok = typeTok->tokAt(2);
            if (Token::Match(typeTok, "%type% <") && Token::Match(typeTok->linkAt(1), "> :: %type%"))
                typeTok = typeTok->linkAt(1)->tokAt(2);
        }

        // check for argument with no name or missing varid
        if (!endTok) {
            if (tok->previous()->isName() && !Token::Match(tok->tokAt(-1), "const|volatile")) {
                if (tok->previous() != typeTok) {
                    nameTok = tok->previous();
                    endTok = nameTok->previous();

                    if (hasBody())
                        symbolDatabase->debugMessage(nameTok, "varid0", "Function::addArguments found argument \'" + nameTok->str() + "\' with varid 0.");
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
        while (Token::Match(startTok, "enum|struct|const|volatile"))
            startTok = startTok->next();

        if (startTok == nameTok)
            break;

        argumentList.emplace_back(nameTok, startTok, endTok, count++, AccessControl::Argument, argType, functionScope, &symbolDatabase->mSettings);

        if (tok->str() == ")") {
            // check for a variadic function or a variadic template function
            if (Token::simpleMatch(endTok, "..."))
                isVariadic(true);

            break;
        }
    }

    // count default arguments
    for (const Token* tok = argDef->next(); tok && tok != argDef->link(); tok = tok->next()) {
        if (tok->str() == "=") {
            initArgCount++;
            if (tok->strAt(1) == "[") {
                const Token* lambdaStart = tok->next();
                if (type == eLambda)
                    tok = findLambdaEndTokenWithoutAST(lambdaStart);
                else {
                    tok = findLambdaEndToken(lambdaStart);
                    if (!tok)
                        tok = findLambdaEndTokenWithoutAST(lambdaStart);
                }
                if (!tok)
                    throw InternalError(lambdaStart, "Analysis failed (lambda not recognized). If the code is valid then please report this failure.", InternalError::INTERNAL);
            }
        }
    }
}

bool Function::isImplicitlyVirtual(bool defaultVal) const
{
    if (hasVirtualSpecifier()) //If it has the virtual specifier it's definitely virtual
        return true;
    if (hasOverrideSpecifier()) //If it has the override specifier then it's either virtual or not going to compile
        return true;
    bool foundAllBaseClasses = true;
    if (getOverriddenFunction(&foundAllBaseClasses)) //If it overrides a base class's method then it's virtual
        return true;
    if (foundAllBaseClasses) //If we've seen all the base classes and none of the above were true then it must not be virtual
        return false;
    return defaultVal; //If we can't see all the bases classes then we can't say conclusively
}

std::vector<const Function*> Function::getOverloadedFunctions() const
{
    std::vector<const Function*> result;
    const Scope* scope = nestedIn;

    while (scope) {
        const bool isMemberFunction = scope->isClassOrStruct() && !isStatic();
        for (std::multimap<std::string, const Function*>::const_iterator it = scope->functionMap.find(tokenDef->str());
             it != scope->functionMap.end() && it->first == tokenDef->str();
             ++it) {
            const Function* func = it->second;
            if (isMemberFunction && isMemberFunction == func->isStatic())
                continue;
            result.push_back(func);
        }
        if (isMemberFunction)
            break;
        scope = scope->nestedIn;
    }

    return result;
}

const Function *Function::getOverriddenFunction(bool *foundAllBaseClasses) const
{
    if (foundAllBaseClasses)
        *foundAllBaseClasses = true;
    if (!nestedIn->isClassOrStruct())
        return nullptr;
    return getOverriddenFunctionRecursive(nestedIn->definedType, foundAllBaseClasses);
}

// prevent recursion if base is the same except for different template parameters
static bool isDerivedFromItself(const std::string& thisName, const std::string& baseName)
{
    if (thisName.back() != '>')
        return false;
    const auto pos = thisName.find('<');
    if (pos == std::string::npos)
        return false;
    return thisName.compare(0, pos + 1, baseName, 0, pos + 1) == 0;
}

const Function * Function::getOverriddenFunctionRecursive(const ::Type* baseType, bool *foundAllBaseClasses) const
{
    // check each base class
    for (const ::Type::BaseInfo & i : baseType->derivedFrom) {
        const ::Type* derivedFromType = i.type;
        // check if base class exists in database
        if (!derivedFromType || !derivedFromType->classScope) {
            if (foundAllBaseClasses)
                *foundAllBaseClasses = false;
            continue;
        }

        const Scope *parent = derivedFromType->classScope;

        // check if function defined in base class
        auto range = parent->functionMap.equal_range(tokenDef->str());
        for (std::multimap<std::string, const Function*>::const_iterator it = range.first; it != range.second; ++it) {
            const Function * func = it->second;
            if (func->isImplicitlyVirtual()) { // Base is virtual and of same name
                const Token *temp1 = func->tokenDef->previous();
                const Token *temp2 = tokenDef->previous();
                bool match = true;

                // check for matching return parameters
                while (!Token::Match(temp1, "virtual|public:|private:|protected:|{|}|;")) {
                    if (temp1->str() != temp2->str() &&
                        !(temp1->type() && temp2->type() && temp2->type()->isDerivedFrom(temp1->type()->name()))) {
                        match = false;
                        break;
                    }

                    temp1 = temp1->previous();
                    temp2 = temp2->previous();
                }

                // check for matching function parameters
                match = match && argsMatch(baseType->classScope, func->argDef, argDef, emptyString, 0);

                // check for matching cv-ref qualifiers
                match = match
                        && isConst() == func->isConst()
                        && isVolatile() == func->isVolatile()
                        && hasRvalRefQualifier() == func->hasRvalRefQualifier()
                        && hasLvalRefQualifier() == func->hasLvalRefQualifier();

                // it's a match
                if (match) {
                    return func;
                }
            }
        }

        if (!derivedFromType->derivedFrom.empty() && !derivedFromType->hasCircularDependencies() && !isDerivedFromItself(baseType->classScope->className, i.name)) {
            // avoid endless recursion, see #5289 Crash: Stack overflow in isImplicitlyVirtual_rec when checking SVN and
            // #5590 with a loop within the class hierarchy.
            const Function *func = getOverriddenFunctionRecursive(derivedFromType, foundAllBaseClasses);
            if (func) {
                return func;
            }
        }
    }
    return nullptr;
}

const Variable* Function::getArgumentVar(nonneg int num) const
{
    if (num < argumentList.size()) {
        auto it = argumentList.begin();
        std::advance(it, num);
        return &*it;
    }
    return nullptr;
}


//---------------------------------------------------------------------------

Scope::Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_, ScopeType type_, const Token *start_) :
    check(check_),
    classDef(classDef_),
    nestedIn(nestedIn_),
    type(type_)
{
    setBodyStartEnd(start_);
}

Scope::Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_) :
    check(check_),
    classDef(classDef_),
    nestedIn(nestedIn_)
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
    } else if (classDef->str() == "[") {
        type = Scope::eLambda;
    } else {
        type = Scope::eFunction;
    }
    // skip over qualification if present
    nameTok = skipScopeIdentifiers(nameTok);
    if (nameTok && ((type == Scope::eEnum && Token::Match(nameTok, ":|{")) || nameTok->str() != "{")) // anonymous and unnamed structs/unions don't have a name
        className = nameTok->str();
}

AccessControl Scope::defaultAccess() const
{
    switch (type) {
    case eGlobal:
        return AccessControl::Global;
    case eClass:
        return AccessControl::Private;
    case eStruct:
        return AccessControl::Public;
    case eUnion:
        return AccessControl::Public;
    case eNamespace:
        return AccessControl::Namespace;
    default:
        return AccessControl::Local;
    }
}

void Scope::addVariable(const Token *token_, const Token *start_, const Token *end_,
                        AccessControl access_, const Type *type_, const Scope *scope_, const Settings* settings)
{
    // keep possible size_t -> int truncation outside emplace_back() to have a single line
    // C4267 VC++ warning instead of several dozens lines
    const int varIndex = varlist.size();
    varlist.emplace_back(token_, start_, end_, varIndex, access_, type_, scope_, settings);
}

// Get variable list..
void Scope::getVariableList(const Settings* settings)
{
    if (!bodyStartList.empty()) {
        for (const Token *bs: bodyStartList)
            getVariableList(settings, bs->next(), bs->link());
    }

    // global scope
    else if (type == Scope::eGlobal)
        getVariableList(settings, check->mTokenizer.tokens(), nullptr);

    // forward declaration
    else
        return;
}

void Scope::getVariableList(const Settings* settings, const Token* start, const Token* end)
{
    // Variable declared in condition: if (auto x = bar())
    if (Token::Match(classDef, "if|while ( %type%") && Token::simpleMatch(classDef->next()->astOperand2(), "=")) {
        checkVariable(classDef->tokAt(2), defaultAccess(), settings);
    }

    AccessControl varaccess = defaultAccess();
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        // syntax error?
        if (tok->next() == nullptr)
            break;

        // Is it a function?
        if (tok->str() == "{") {
            tok = tok->link();
            continue;
        }

        // Is it a nested class or structure?
        if (tok->isKeyword() && Token::Match(tok, "class|struct|union|namespace %type% :|{")) {
            tok = tok->tokAt(2);
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (tok) {
                // skip implementation
                tok = tok->link();
                continue;
            }
            break;
        }
        if (tok->isKeyword() && Token::Match(tok, "struct|union {")) {
            if (Token::Match(tok->next()->link(), "} %name% ;|[")) {
                tok = tok->next()->link()->tokAt(2);
                continue;
            }
            if (Token::simpleMatch(tok->next()->link(), "} ;")) {
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
            break;
        }

        // "private:" "public:" "protected:" etc
        else if (tok->str() == "public:") {
            varaccess = AccessControl::Public;
            continue;
        } else if (tok->str() == "protected:") {
            varaccess = AccessControl::Protected;
            continue;
        } else if (tok->str() == "private:") {
            varaccess = AccessControl::Private;
            continue;
        }

        // Is it a forward declaration?
        else if (tok->isKeyword() && Token::Match(tok, "class|struct|union %name% ;")) {
            tok = tok->tokAt(2);
            continue;
        }

        // Borland C++: Ignore properties..
        else if (tok->str() == "__property")
            continue;

        // skip return, goto and delete
        else if (tok->isKeyword() && Token::Match(tok, "return|delete|goto")) {
            while (tok->next() &&
                   tok->next()->str() != ";" &&
                   tok->next()->str() != "}" /* ticket #4994 */) {
                tok = tok->next();
            }
            continue;
        }

        // skip case/default
        if (tok->isKeyword() && Token::Match(tok, "case|default")) {
            while (tok->next() && !Token::Match(tok->next(), "[:;{}]"))
                tok = tok->next();
            continue;
        }

        // Search for start of statement..
        if (tok->previous() && !Token::Match(tok->previous(), ";|{|}|public:|protected:|private:"))
            continue;
        if (tok->str() == ";")
            continue;

        tok = checkVariable(tok, varaccess, settings);

        if (!tok)
            break;
    }
}

const Token *Scope::checkVariable(const Token *tok, AccessControl varaccess, const Settings* settings)
{
    // Is it a throw..?
    if (tok->isKeyword() && Token::Match(tok, "throw %any% (") &&
        Token::simpleMatch(tok->linkAt(2), ") ;")) {
        return tok->linkAt(2);
    }

    if (tok->isKeyword() && Token::Match(tok, "throw %any% :: %any% (") &&
        Token::simpleMatch(tok->linkAt(4), ") ;")) {
        return tok->linkAt(4);
    }

    // friend?
    if (tok->isKeyword() && Token::Match(tok, "friend %type%") && tok->next()->varId() == 0) {
        const Token *next = Token::findmatch(tok->tokAt(2), ";|{");
        if (next && next->str() == "{")
            next = next->link();
        return next;
    }

    // skip const|volatile|static|mutable|extern
    while (tok && tok->isKeyword() && Token::Match(tok, "const|constexpr|volatile|static|mutable|extern")) {
        tok = tok->next();
    }

    // the start of the type tokens does not include the above modifiers
    const Token *typestart = tok;

    // C++17 structured bindings
    if (settings->standards.cpp >= Standards::CPP17 && Token::Match(tok, "auto &|&&| [")) {
        const Token *typeend = Token::findsimplematch(typestart, "[")->previous();
        for (tok = typeend->tokAt(2); Token::Match(tok, "%name%|,"); tok = tok->next()) {
            if (tok->varId())
                addVariable(tok, typestart, typeend, varaccess, nullptr, this, settings);
        }
        return typeend->linkAt(1);
    }

    while (tok && tok->isKeyword() && Token::Match(tok, "class|struct|union|enum")) {
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
                check->debugMessage(vartok, "varid0", "Scope::checkVariable found variable \'" + vartok->str() + "\' with varid 0.");
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

        addVariable(vartok, typestart, vartok->previous(), varaccess, vType, this, settings);
    }

    return tok;
}

const Variable *Scope::getVariable(const std::string &varname) const
{
    auto it = std::find_if(varlist.begin(), varlist.end(), [&varname](const Variable& var) {
        return var.name() == varname;
    });
    if (it != varlist.end())
        return &*it;

    if (definedType) {
        for (const Type::BaseInfo& baseInfo: definedType->derivedFrom) {
            if (baseInfo.type && baseInfo.type->classScope) {
                if (const Variable* var = baseInfo.type->classScope->getVariable(varname))
                    return var;
            }
        }
    }
    return nullptr;
}

static const Token* skipPointers(const Token* tok)
{
    while (Token::Match(tok, "*|&|&&") || (Token::Match(tok, "( [*&]") && Token::Match(tok->link()->next(), "(|["))) {
        tok = tok->next();
        if (tok->strAt(-1) == "(" && Token::Match(tok, "%type% ::"))
            tok = tok->tokAt(2);
    }

    if (Token::simpleMatch(tok, "( *") && Token::simpleMatch(tok->link()->previous(), "] ) ;")) {
        const Token *tok2 = skipPointers(tok->next());
        if (Token::Match(tok2, "%name% [") && Token::simpleMatch(tok2->linkAt(1), "] ) ;"))
            return tok2;
    }

    return tok;
}

static const Token* skipPointersAndQualifiers(const Token* tok)
{
    tok = skipPointers(tok);
    while (Token::Match(tok, "const|static|volatile")) {
        tok = tok->next();
        tok = skipPointers(tok);
    }

    return tok;
}

bool Scope::isVariableDeclaration(const Token* const tok, const Token*& vartok, const Token*& typetok) const
{
    if (!tok)
        return false;

    const bool isCPP = check && check->mTokenizer.isCPP();

    if (isCPP && Token::Match(tok, "throw|new"))
        return false;

    const bool isCPP11 = isCPP && check->mSettings.standards.cpp >= Standards::CPP11;

    if (isCPP11 && tok->str() == "using")
        return false;

    const Token* localTypeTok = skipScopeIdentifiers(tok);
    const Token* localVarTok = nullptr;

    while (Token::simpleMatch(localTypeTok, "alignas (") && Token::Match(localTypeTok->linkAt(1), ") %name%"))
        localTypeTok = localTypeTok->linkAt(1)->next();

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

        if (isCPP11 && Token::simpleMatch(localTypeTok, "decltype (") && Token::Match(localTypeTok->linkAt(1), ") %name%|*|&|&&"))
            localVarTok = skipPointersAndQualifiers(localTypeTok->linkAt(1)->next());
        else {
            localVarTok = skipPointersAndQualifiers(localTypeTok->next());
            if (isCPP11 && Token::simpleMatch(localVarTok, "decltype (") && Token::Match(localVarTok->linkAt(1), ") %name%|*|&|&&"))
                localVarTok = skipPointersAndQualifiers(localVarTok->linkAt(1)->next());
        }
    }

    if (!localVarTok)
        return false;

    while (Token::Match(localVarTok, "const|*|&"))
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

            if (tok2 && tok2->str() != ";" && (isCpp || tok2->str() != ")"))
                tok2 = nullptr;
        } else
            tok2 = nullptr;
    } else
        tok2 = nullptr;

    return tok2;
}

const Enumerator * SymbolDatabase::findEnumerator(const Token * tok, std::set<std::string>& tokensThatAreNotEnumeratorValues) const
{
    if (tok->isKeyword())
        return nullptr;

    const std::string& tokStr = tok->str();

    if (tokensThatAreNotEnumeratorValues.find(tokStr) != tokensThatAreNotEnumeratorValues.end())
        return nullptr;

    const Scope* scope = tok->scope();

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
                const Scope* temp = scope->nestedIn->findRecordInNestedList(tok1->str());
                if (!temp && scope->functionOf)
                    temp = scope->functionOf->findRecordInNestedList(tok1->str());
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
                const Enumerator * enumerator = scope->findEnumerator(tokStr);

                if (enumerator) // enum class
                    return enumerator;
                // enum
                for (std::vector<Scope *>::const_iterator it = scope->nestedList.cbegin(), end = scope->nestedList.cend(); it != end; ++it) {
                    enumerator = (*it)->findEnumerator(tokStr);

                    if (enumerator && !(enumerator->scope && enumerator->scope->enumClass))
                        return enumerator;
                }
            }
        }
    } else { // unqualified name
        const Enumerator * enumerator = scope->findEnumerator(tokStr);

        if (enumerator && !(enumerator->scope && enumerator->scope->enumClass))
            return enumerator;

        if (Token::simpleMatch(tok->astParent(), ".")) {
            const Token* varTok = tok->astParent()->astOperand1();
            if (varTok && varTok->variable() && varTok->variable()->type() && varTok->variable()->type()->classScope)
                scope = varTok->variable()->type()->classScope;
        }
        else if (Token::simpleMatch(tok->astParent(), "[")) {
            const Token* varTok = tok->astParent()->previous();
            if (varTok && varTok->variable() && varTok->variable()->scope() && Token::simpleMatch(tok->astParent()->astOperand1(), "::"))
                scope = varTok->variable()->scope();
        }

        for (std::vector<Scope *>::const_iterator s = scope->nestedList.cbegin(); s != scope->nestedList.cend(); ++s) {
            enumerator = (*s)->findEnumerator(tokStr);

            if (enumerator && !(enumerator->scope && enumerator->scope->enumClass))
                return enumerator;
        }

        if (scope->definedType) {
            const std::vector<Type::BaseInfo> & derivedFrom = scope->definedType->derivedFrom;
            for (const Type::BaseInfo & i : derivedFrom) {
                const Type *derivedFromType = i.type;
                if (derivedFromType && derivedFromType->classScope) {
                    enumerator = derivedFromType->classScope->findEnumerator(tokStr);

                    if (enumerator && !(enumerator->scope && enumerator->scope->enumClass))
                        return enumerator;
                }
            }
        }

        while (scope->nestedIn) {
            if (scope->type == Scope::eFunction && scope->functionOf)
                scope = scope->functionOf;
            else
                scope = scope->nestedIn;

            enumerator = scope->findEnumerator(tokStr);

            if (enumerator && !(enumerator->scope && enumerator->scope->enumClass))
                return enumerator;

            for (std::vector<Scope*>::const_iterator s = scope->nestedList.cbegin(); s != scope->nestedList.cend(); ++s) {
                enumerator = (*s)->findEnumerator(tokStr);

                if (enumerator && !(enumerator->scope && enumerator->scope->enumClass))
                    return enumerator;
            }
        }
    }

    tokensThatAreNotEnumeratorValues.insert(tokStr);

    return nullptr;
}

//---------------------------------------------------------------------------

const Type* SymbolDatabase::findVariableTypeInBase(const Scope* scope, const Token* typeTok)
{
    if (scope && scope->definedType && !scope->definedType->derivedFrom.empty()) {
        const std::vector<Type::BaseInfo> &derivedFrom = scope->definedType->derivedFrom;
        for (const Type::BaseInfo & i : derivedFrom) {
            const Type *base = i.type;
            if (base && base->classScope) {
                if (base->classScope == scope)
                    return nullptr;
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
        // check if type same as scope
        if (start->isClassOrStruct() && typeTok->str() == start->className)
            return start->definedType;

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

        while ((Token::Match(tok1->tokAt(-2), "%type% ::") && !tok1->tokAt(-2)->isKeyword()) ||
               (Token::simpleMatch(tok1->tokAt(-2), "> ::") && tok1->linkAt(-2) && Token::Match(tok1->linkAt(-2)->tokAt(-1), "%type%"))) {
            if (tok1->strAt(-1) == "::")
                tok1 = tok1->tokAt(-2);
            else
                tok1 = tok1->linkAt(-2)->tokAt(-1);
        }

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

                const Scope *scope1 = scope->findRecordInNestedList(tok1->str());

                if (scope1) {
                    scope = scope1;
                    break;
                }
                if (scope->type == Scope::eFunction && scope->functionOf)
                    scope = scope->functionOf;
                else
                    scope = scope->nestedIn;
            }
        }

        if (scope) {
            // follow qualification
            while (scope && (Token::Match(tok1, "%type% ::") ||
                             (Token::Match(tok1, "%type% <") && Token::simpleMatch(tok1->linkAt(1), "> ::")))) {
                if (tok1->strAt(1) == "::")
                    tok1 = tok1->tokAt(2);
                else
                    tok1 = tok1->linkAt(1)->tokAt(2);
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

static bool hasEmptyCaptureList(const Token* tok) {
    if (!Token::simpleMatch(tok, "{"))
        return false;
    const Token* listTok = tok->astParent();
    if (Token::simpleMatch(listTok, "("))
        listTok = listTok->astParent();
    return Token::simpleMatch(listTok, "[ ]");
}

bool Scope::hasInlineOrLambdaFunction() const
{
    return std::any_of(nestedList.begin(), nestedList.end(), [&](const Scope* s) {
        // Inline function
        if (s->type == Scope::eUnconditional && Token::simpleMatch(s->bodyStart->previous(), ") {"))
            return true;
        // Lambda function
        if (s->type == Scope::eLambda && !hasEmptyCaptureList(s->bodyStart))
            return true;
        if (s->hasInlineOrLambdaFunction())
            return true;
        return false;
    });
}

void Scope::findFunctionInBase(const std::string & name, nonneg int args, std::vector<const Function *> & matches) const
{
    if (isClassOrStruct() && definedType && !definedType->derivedFrom.empty()) {
        const std::vector<Type::BaseInfo> &derivedFrom = definedType->derivedFrom;
        for (const Type::BaseInfo & i : derivedFrom) {
            const Type *base = i.type;
            if (base && base->classScope) {
                if (base->classScope == this) // Ticket #5120, #5125: Recursive class; tok should have been found already
                    continue;

                auto range = base->classScope->functionMap.equal_range(name);
                for (std::multimap<std::string, const Function*>::const_iterator it = range.first; it != range.second; ++it) {
                    const Function *func = it->second;
                    if ((func->isVariadic() && args >= (func->argCount() - 1)) ||
                        (args == func->argCount() || (args < func->argCount() && args >= func->minArgCount()))) {
                        matches.push_back(func);
                    }
                }

                base->classScope->findFunctionInBase(name, args, matches);
            }
        }
    }
}

const Scope *Scope::findRecordInBase(const std::string & name) const
{
    if (isClassOrStruct() && definedType && !definedType->derivedFrom.empty()) {
        const std::vector<Type::BaseInfo> &derivedFrom = definedType->derivedFrom;
        for (const Type::BaseInfo & i : derivedFrom) {
            const Type *base = i.type;
            if (base && base->classScope) {
                if (base->classScope == this) // Recursive class; tok should have been found already
                    continue;

                if (base->name() == name) {
                    return base->classScope;
                }

                const ::Type * t = base->classScope->findType(name);
                if (t)
                    return t->classScope;
            }
        }
    }

    return nullptr;
}

std::vector<const Scope*> Scope::findAssociatedScopes() const
{
    std::vector<const Scope*> result = {this};
    if (isClassOrStruct() && definedType && !definedType->derivedFrom.empty()) {
        const std::vector<Type::BaseInfo>& derivedFrom = definedType->derivedFrom;
        for (const Type::BaseInfo& i : derivedFrom) {
            const Type* base = i.type;
            if (base && base->classScope) {
                if (contains(result, base->classScope))
                    continue;
                std::vector<const Scope*> baseScopes = base->classScope->findAssociatedScopes();
                result.insert(result.end(), baseScopes.cbegin(), baseScopes.cend());
            }
        }
    }
    return result;
}

//---------------------------------------------------------------------------

static void checkVariableCallMatch(const Variable* callarg, const Variable* funcarg, size_t& same, size_t& fallback1, size_t& fallback2)
{
    if (callarg) {
        const ValueType::MatchResult res = ValueType::matchParameter(callarg->valueType(), callarg, funcarg);
        if (res == ValueType::MatchResult::SAME) {
            same++;
            return;
        }
        if (res == ValueType::MatchResult::FALLBACK1) {
            fallback1++;
            return;
        }
        if (res == ValueType::MatchResult::FALLBACK2) {
            fallback2++;
            return;
        }
        if (res == ValueType::MatchResult::NOMATCH)
            return;

        const bool ptrequals = callarg->isArrayOrPointer() == funcarg->isArrayOrPointer();
        const bool constEquals = !callarg->isArrayOrPointer() || ((callarg->typeStartToken()->strAt(-1) == "const") == (funcarg->typeStartToken()->strAt(-1) == "const"));
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
            const bool takesInt = Token::Match(funcarg->typeStartToken(), "char|short|int|long");
            const bool takesFloat = Token::Match(funcarg->typeStartToken(), "float|double");
            const bool passesInt = Token::Match(callarg->typeStartToken(), "char|short|int|long");
            const bool passesFloat = Token::Match(callarg->typeStartToken(), "float|double");
            if ((takesInt && passesInt) || (takesFloat && passesFloat))
                fallback1++;
            else if ((takesInt && passesFloat) || (takesFloat && passesInt))
                fallback2++;
        }
    }
}

static std::string getTypeString(const Token *typeToken)
{
    if (!typeToken)
        return "";
    while (Token::Match(typeToken, "%name%|*|&|::")) {
        if (typeToken->str() == "::") {
            std::string ret;
            while (Token::Match(typeToken, ":: %name%")) {
                ret += "::" + typeToken->strAt(1);
                typeToken = typeToken->tokAt(2);
                if (typeToken->str() == "<") {
                    for (const Token *tok = typeToken; tok != typeToken->link(); tok = tok->next())
                        ret += tok->str();
                    ret += ">";
                    typeToken = typeToken->link()->next();
                }
            }
            return ret;
        }
        if (Token::Match(typeToken, "%name% const| %var%|*|&")) {
            return typeToken->str();
        }
        typeToken = typeToken->next();
    }
    return "";
}

static bool hasMatchingConstructor(const Scope* classScope, const ValueType* argType) {
    if (!classScope || !argType)
        return false;
    return std::any_of(classScope->functionList.cbegin(),
                       classScope->functionList.cend(),
                       [&](const Function& f) {
        if (!f.isConstructor() || f.argCount() != 1 || !f.getArgumentVar(0))
            return false;
        const ValueType* vt = f.getArgumentVar(0)->valueType();
        return vt &&
        vt->type == argType->type &&
        (argType->sign == ValueType::Sign::UNKNOWN_SIGN || vt->sign == argType->sign) &&
        vt->pointer == argType->pointer &&
        (vt->constness & 1) >= (argType->constness & 1);
    });
}

const Function* Scope::findFunction(const Token *tok, bool requireConst) const
{
    const bool isCall = Token::Match(tok->next(), "(|{");

    const std::vector<const Token *> arguments = getArguments(tok);

    std::vector<const Function *> matches;

    // find all the possible functions that could match
    const std::size_t args = arguments.size();

    auto addMatchingFunctions = [&](const Scope *scope) {
        auto range = scope->functionMap.equal_range(tok->str());
        for (std::multimap<std::string, const Function *>::const_iterator it = range.first; it != range.second; ++it) {
            const Function *func = it->second;
            if (!isCall || args == func->argCount() ||
                (func->isVariadic() && args >= (func->minArgCount() - 1)) ||
                (args < func->argCount() && args >= func->minArgCount())) {
                matches.push_back(func);
            }
        }
    };

    addMatchingFunctions(this);

    // check in anonymous namespaces
    for (const Scope *nestedScope : nestedList) {
        if (nestedScope->type == eNamespace && nestedScope->className.empty())
            addMatchingFunctions(nestedScope);
    }

    const std::size_t numberOfMatchesNonBase = matches.size();

    // check in base classes
    findFunctionInBase(tok->str(), args, matches);

    // Non-call => Do not match parameters
    if (!isCall) {
        return matches.empty() ? nullptr : matches[0];
    }

    std::vector<const Function*> fallback1Func, fallback2Func;

    // check each function against the arguments in the function call for a match
    for (std::size_t i = 0; i < matches.size();) {
        if (i > 0 && i == numberOfMatchesNonBase && fallback1Func.empty() && !fallback2Func.empty())
            break;

        bool constFallback = false;
        const Function * func = matches[i];
        size_t same = 0;

        if (requireConst && !func->isConst()) {
            i++;
            continue;
        }

        if (!requireConst || !func->isConst()) {
            // get the function this call is in
            const Scope * scope = tok->scope();

            // check if this function is a member function
            if (scope && scope->functionOf && scope->functionOf->isClassOrStruct() && scope->function &&
                func->nestedIn == scope->functionOf) {
                // check if isConst mismatches
                if (scope->function->isConst() != func->isConst()) {
                    if (scope->function->isConst()) {
                        ++i;
                        continue;
                    }
                    constFallback = true;
                }
            }
        }

        size_t fallback1 = 0;
        size_t fallback2 = 0;
        bool erased = false;
        for (std::size_t j = 0; j < args; ++j) {

            // don't check variadic arguments
            if (func->isVariadic() && j > (func->argCount() - 1)) {
                break;
            }
            const Variable *funcarg = func->getArgumentVar(j);

            if (!arguments[j]->valueType()) {
                const Token *vartok = arguments[j];
                int pointer = 0;
                while (vartok && (vartok->isUnaryOp("&") || vartok->isUnaryOp("*"))) {
                    pointer += vartok->isUnaryOp("&") ? 1 : -1;
                    vartok = vartok->astOperand1();
                }
                if (vartok && vartok->variable()) {
                    const Token *callArgTypeToken = vartok->variable()->typeStartToken();
                    const Token *funcArgTypeToken = funcarg->typeStartToken();

                    auto parseDecl = [](const Token *typeToken) -> ValueType {
                        ValueType ret;
                        while (Token::Match(typeToken->previous(), "%name%"))
                            typeToken = typeToken->previous();
                        while (Token::Match(typeToken, "%name%|*|&|::|<"))
                        {
                            if (typeToken->str() == "const")
                                ret.constness |= (1 << ret.pointer);
                            else if (typeToken->str() == "*")
                                ret.pointer++;
                            else if (typeToken->str() == "<") {
                                if (!typeToken->link())
                                    break;
                                typeToken = typeToken->link();
                            }
                            typeToken = typeToken->next();
                        }
                        return ret;
                    };

                    const std::string type1 = getTypeString(callArgTypeToken);
                    const std::string type2 = getTypeString(funcArgTypeToken);
                    if (!type1.empty() && type1 == type2) {
                        ValueType callArgType = parseDecl(callArgTypeToken);
                        callArgType.pointer += pointer;
                        ValueType funcArgType = parseDecl(funcArgTypeToken);

                        callArgType.sign = funcArgType.sign = ValueType::Sign::SIGNED;
                        callArgType.type = funcArgType.type = ValueType::Type::INT;

                        const ValueType::MatchResult res = ValueType::matchParameter(&callArgType, &funcArgType);
                        if (res == ValueType::MatchResult::SAME)
                            ++same;
                        else if (res == ValueType::MatchResult::FALLBACK1)
                            ++fallback1;
                        else if (res == ValueType::MatchResult::FALLBACK2)
                            ++fallback2;
                        continue;
                    }
                }
            }

            // check for a match with a variable
            if (Token::Match(arguments[j], "%var% ,|)")) {
                const Variable * callarg = arguments[j]->variable();
                checkVariableCallMatch(callarg, funcarg, same, fallback1, fallback2);
            }

            else if (funcarg->isStlStringType() && arguments[j]->valueType() && arguments[j]->valueType()->pointer == 1 && arguments[j]->valueType()->type == ValueType::Type::CHAR)
                fallback2++;

            // check for a match with nullptr
            else if (funcarg->isPointer() && Token::Match(arguments[j], "nullptr|NULL ,|)"))
                same++;

            else if (funcarg->isPointer() && MathLib::isNullValue(arguments[j]->str()))
                fallback1++;

            else if (!funcarg->isPointer() && funcarg->type() && hasMatchingConstructor(funcarg->type()->classScope, arguments[j]->valueType()))
                fallback2++;

            // Try to evaluate the apparently more complex expression
            else if (check->isCPP()) {
                const Token *vartok = arguments[j];
                if (vartok->str() == ".") {
                    const Token* rml = nextAfterAstRightmostLeaf(vartok);
                    if (rml)
                        vartok = rml->previous();
                }
                while (vartok->isUnaryOp("&") || vartok->isUnaryOp("*"))
                    vartok = vartok->astOperand1();
                const Variable* var = vartok->variable();
                // smart pointer deref?
                bool unknownDeref = false;
                if (var && vartok->astParent() && vartok->astParent()->str() == "*") {
                    if (var->isSmartPointer() && var->valueType() && var->valueType()->smartPointerTypeToken)
                        var = var->valueType()->smartPointerTypeToken->variable();
                    else
                        unknownDeref = true;
                }
                const Token* valuetok = arguments[j];
                if (valuetok->str() == "::") {
                    const Token* rml = nextAfterAstRightmostLeaf(valuetok);
                    if (rml)
                        valuetok = rml->previous();
                }
                if (vartok->isEnumerator())
                    valuetok = vartok;
                const ValueType::MatchResult res = ValueType::matchParameter(valuetok->valueType(), var, funcarg);
                if (res == ValueType::MatchResult::SAME)
                    ++same;
                else if (res == ValueType::MatchResult::FALLBACK1)
                    ++fallback1;
                else if (res == ValueType::MatchResult::FALLBACK2)
                    ++fallback2;
                else if (res == ValueType::MatchResult::NOMATCH) {
                    if (unknownDeref)
                        continue;
                    // can't match so remove this function from possible matches
                    matches.erase(matches.begin() + i);
                    erased = true;
                    break;
                }
            }

            else
                // C code: if number of arguments match then do not match types
                fallback1++;
        }

        const size_t hasToBe = func->isVariadic() ? (func->argCount() - 1) : args;

        // check if all arguments matched
        if (same == hasToBe) {
            if (constFallback || (!requireConst && func->isConst()))
                fallback1Func.emplace_back(func);
            else
                return func;
        }

        else {
            if (same + fallback1 == hasToBe)
                fallback1Func.emplace_back(func);
            else if (same + fallback2 + fallback1 == hasToBe)
                fallback2Func.emplace_back(func);
        }

        if (!erased)
            ++i;
    }

    // Fallback cases
    for (const auto& fb : { fallback1Func, fallback2Func }) {
        if (fb.size() == 1)
            return fb.front();
        if (fb.size() == 2) {
            if (fb[0]->isConst() && !fb[1]->isConst())
                return fb[1];
            if (fb[1]->isConst() && !fb[0]->isConst())
                return fb[0];
        }
    }

    // remove pure virtual function if there is an overrider
    auto itPure = std::find_if(matches.begin(), matches.end(), [](const Function* m) {
        return m->isPure();
    });
    if (itPure != matches.end() && std::any_of(matches.begin(), matches.end(), [&](const Function* m) {
        return m->isImplicitlyVirtual() && m != *itPure;
    }))
        matches.erase(itPure);

    // Only one candidate left
    if (matches.size() == 1)
        return matches[0];

    // Prioritize matches in derived scopes
    for (const auto& fb : { fallback1Func, fallback2Func }) {
        const Function* ret = nullptr;
        for (int i = 0; i < fb.size(); ++i) {
            if (std::find(matches.cbegin(), matches.cend(), fb[i]) == matches.cend())
                continue;
            if (this == fb[i]->nestedIn) {
                if (!ret)
                    ret = fb[i];
                else {
                    ret = nullptr;
                    break;
                }
            }
        }
        if (ret)
            return ret;
    }

    return nullptr;
}

//---------------------------------------------------------------------------

const Function* SymbolDatabase::findFunction(const Token* const tok) const
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

        while (Token::Match(tok1->tokAt(-2), ">|%type% ::")) {
            if (tok1->strAt(-2) == ">") {
                if (tok1->linkAt(-2))
                    tok1 = tok1->linkAt(-2)->tokAt(-1);
                else
                    break;
            } else
                tok1 = tok1->tokAt(-2);
        }

        // check for global scope
        if (tok1->strAt(-1) == "::") {
            currScope = &scopeList.front();

            if (const Function* f = currScope->findFunction(tok))
                return f;

            currScope = currScope->findRecordInNestedList(tok1->str());
        }

        // find start of qualification
        else {
            while (currScope) {
                if (currScope->className == tok1->str())
                    break;

                const Scope *scope = currScope->findRecordInNestedList(tok1->str());

                if (scope) {
                    currScope = scope;
                    break;
                }
                currScope = currScope->nestedIn;
            }
        }

        if (currScope) {
            while (currScope && tok1 && !(Token::Match(tok1, "%type% :: %name% [(),>]") ||
                                          (Token::Match(tok1, "%type% <") && Token::Match(tok1->linkAt(1), "> :: %name% (")))) {
                if (tok1->strAt(1) == "::")
                    tok1 = tok1->tokAt(2);
                else if (tok1->strAt(1) == "<")
                    tok1 = tok1->linkAt(1)->tokAt(2);
                else
                    tok1 = nullptr;

                if (tok1) {
                    const Function* func = currScope->findFunction(tok1);
                    if (func)
                        return func;

                    currScope = currScope->findRecordInNestedList(tok1->str());
                }
            }

            if (tok1)
                tok1 = tok1->tokAt(2);

            if (currScope && tok1)
                return currScope->findFunction(tok1);
        }
    }

    // check for member function
    else if (Token::Match(tok->tokAt(-2), "!!this .")) {
        const Token* tok1 = tok->previous()->astOperand1();
        if (tok1 && tok1->valueType() && tok1->valueType()->typeScope)
            return tok1->valueType()->typeScope->findFunction(tok, tok1->valueType()->constness == 1);
        if (tok1 && Token::Match(tok1->previous(), "%name% (") && tok1->previous()->function() &&
            tok1->previous()->function()->retDef) {
            ValueType vt = ValueType::parseDecl(tok1->previous()->function()->retDef, mSettings);
            if (vt.typeScope)
                return vt.typeScope->findFunction(tok, vt.constness == 1);
        } else if (Token::Match(tok1, "%var% .")) {
            const Variable *var = getVariableFromVarId(tok1->varId());
            if (var && var->typeScope())
                return var->typeScope()->findFunction(tok, var->valueType()->constness == 1);
            if (var && var->smartPointerType() && var->smartPointerType()->classScope && tok1->next()->originalName() == "->")
                return var->smartPointerType()->classScope->findFunction(tok, var->valueType()->constness == 1);
            if (var && var->iteratorType() && var->iteratorType()->classScope && tok1->next()->originalName() == "->")
                return var->iteratorType()->classScope->findFunction(tok, var->valueType()->constness == 1);
        } else if (Token::simpleMatch(tok->previous()->astOperand1(), "(")) {
            const Token *castTok = tok->previous()->astOperand1();
            if (castTok->isCast()) {
                ValueType vt = ValueType::parseDecl(castTok->next(),mSettings);
                if (vt.typeScope)
                    return vt.typeScope->findFunction(tok, vt.constness == 1);
            }
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
        // check using namespace
        currScope = tok->scope();
        while (currScope) {
            for (const auto& ul : currScope->usingList) {
                if (ul.scope) {
                    const Function* func = ul.scope->findFunction(tok);
                    if (func)
                        return func;
                }
            }
            currScope = currScope->nestedIn;
        }
    }
    // Check for constructor
    if (Token::Match(tok, "%name% (|{")) {
        ValueType vt = ValueType::parseDecl(tok, mSettings);
        if (vt.typeScope)
            return vt.typeScope->findFunction(tok, false);
    }
    return nullptr;
}

//---------------------------------------------------------------------------

const Scope *SymbolDatabase::findScopeByName(const std::string& name) const
{
    auto it = std::find_if(scopeList.cbegin(), scopeList.cend(), [&](const Scope& s) {
        return s.className == name;
    });
    return it == scopeList.end() ? nullptr : &*it;
}

//---------------------------------------------------------------------------

const Scope *Scope::findRecordInNestedList(const std::string & name, bool isC) const
{
    for (const Scope* scope: nestedList) {
        if (scope->className == name && scope->type != eFunction)
            return scope;
        if (isC) {
            const Scope* nestedScope = scope->findRecordInNestedList(name, isC);
            if (nestedScope)
                return nestedScope;
        }
    }

    const Type * nested_type = findType(name);

    if (nested_type) {
        if (nested_type->isTypeAlias()) {
            if (nested_type->typeStart == nested_type->typeEnd)
                return findRecordInNestedList(nested_type->typeStart->str());
        } else
            return nested_type->classScope;
    }

    return nullptr;
}

//---------------------------------------------------------------------------

const Type* Scope::findType(const std::string & name) const
{
    auto it = definedTypesMap.find(name);

    // Type was found
    if (definedTypesMap.end() != it)
        return it->second;

    // is type defined in anonymous namespace..
    it = definedTypesMap.find(emptyString);
    if (it != definedTypesMap.end()) {
        for (const Scope *scope : nestedList) {
            if (scope->className.empty() && (scope->type == eNamespace || scope->isClassOrStructOrUnion())) {
                const Type *t = scope->findType(name);
                if (t)
                    return t;
            }
        }
    }

    // Type was not found
    return nullptr;
}

//---------------------------------------------------------------------------

Scope *Scope::findInNestedListRecursive(const std::string & name)
{
    auto it = std::find_if(nestedList.cbegin(), nestedList.cend(), [&](const Scope* s) {
        return s->className == name;
    });
    if (it != nestedList.end())
        return *it;

    for (Scope* scope: nestedList) {
        Scope *child = scope->findInNestedListRecursive(name);
        if (child)
            return child;
    }
    return nullptr;
}

//---------------------------------------------------------------------------

const Function *Scope::getDestructor() const
{
    auto it = std::find_if(functionList.cbegin(), functionList.cend(), [](const Function& f) {
        return f.type == Function::eDestructor;
    });
    return it == functionList.end() ? nullptr : &*it;
}

//---------------------------------------------------------------------------

bool SymbolDatabase::isCPP() const
{
    return mTokenizer.isCPP();
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
        } else if (tok->strAt(1) == "<" && Token::simpleMatch(tok->linkAt(1), "> ::")) {
            scope = scope->findRecordInNestedList(tok->str());
            tok = tok->linkAt(1)->tokAt(2);
        } else
            return scope->findRecordInNestedList(tok->str());
    }

    // not a valid path
    return nullptr;
}

//---------------------------------------------------------------------------

const Type* SymbolDatabase::findType(const Token *startTok, const Scope *startScope, bool lookOutside) const
{
    // skip over struct or union
    if (Token::Match(startTok, "struct|union"))
        startTok = startTok->next();

    // type same as scope
    if (startTok->str() == startScope->className && startScope->isClassOrStruct() && startTok->strAt(1) != "::")
        return startScope->definedType;

    if (mTokenizer.isC()) {
        const Scope* scope = startScope;
        while (scope) {
            if (startTok->str() == scope->className && scope->isClassOrStruct())
                return scope->definedType;
            const Scope* typeScope = scope->findRecordInNestedList(startTok->str(), /*isC*/ true);
            if (typeScope) {
                if (startTok->str() == typeScope->className && typeScope->isClassOrStruct()) {
                    if (const Type* type = typeScope->definedType)
                        return type;
                }
            }
            scope = scope->nestedIn;
        }
        return nullptr;
    }

    const Scope* start_scope = startScope;

    // absolute path - directly start in global scope
    if (startTok->str() == "::") {
        startTok = startTok->next();
        start_scope = &scopeList.front();
    }

    const Token* tok = startTok;
    const Scope* scope = start_scope;

    while (scope && tok && tok->isName()) {
        if (tok->strAt(1) == "::" || (tok->strAt(1) == "<" && Token::simpleMatch(tok->linkAt(1), "> ::"))) {
            scope = scope->findRecordInNestedList(tok->str());
            if (scope) {
                if (tok->strAt(1) == "::")
                    tok = tok->tokAt(2);
                else
                    tok = tok->linkAt(1)->tokAt(2);
            } else {
                start_scope = start_scope->nestedIn;
                if (!start_scope)
                    break;
                scope = start_scope;
                tok = startTok;
            }
        } else {
            const Scope* scope1{};
            const Type* type = scope->findType(tok->str());
            if (type)
                return type;
            if (lookOutside && (scope1 = scope->findRecordInBase(tok->str()))) {
                type = scope1->definedType;
                if (type)
                    return type;
            } else if (lookOutside && scope->type == Scope::ScopeType::eNamespace) {
                scope = scope->nestedIn;
                continue;
            } else
                break;
        }
    }

    // check using namespaces
    while (startScope) {
        for (std::vector<Scope::UsingInfo>::const_iterator it = startScope->usingList.cbegin();
             it != startScope->usingList.cend(); ++it) {
            tok = startTok;
            scope = it->scope;
            start_scope = startScope;

            while (scope && tok && tok->isName()) {
                if (tok->strAt(1) == "::" || (tok->strAt(1) == "<" && Token::simpleMatch(tok->linkAt(1), "> ::"))) {
                    scope = scope->findRecordInNestedList(tok->str());
                    if (scope) {
                        if (tok->strAt(1) == "::")
                            tok = tok->tokAt(2);
                        else
                            tok = tok->linkAt(1)->tokAt(2);
                    } else {
                        start_scope = start_scope->nestedIn;
                        if (!start_scope)
                            break;
                        scope = start_scope;
                        tok = startTok;
                    }
                } else {
                    const Type * type = scope->findType(tok->str());
                    if (type)
                        return type;
                    if (const Scope *scope1 = scope->findRecordInBase(tok->str())) {
                        type = scope1->definedType;
                        if (type)
                            return type;
                    } else
                        break;
                }
            }
        }
        startScope = startScope->nestedIn;
    }

    // not a valid path
    return nullptr;
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
        if (tok->strAt(1) == "::" || (tok->strAt(1) == "<" && Token::simpleMatch(tok->linkAt(1), "> ::"))) {
            hasPath = true;
            scope = scope->findRecordInNestedList(tok->str());
            if (scope) {
                if (tok->strAt(1) == "::")
                    tok = tok->tokAt(2);
                else
                    tok = tok->linkAt(1)->tokAt(2);
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

            scope = scope->nestedIn;
            if (!scope)
                break;
        }
    }

    // not a valid path
    return nullptr;
}

//---------------------------------------------------------------------------

const Scope * SymbolDatabase::findNamespace(const Token * tok, const Scope * scope) const
{
    const Scope * s = findScope(tok, scope);

    if (s)
        return s;
    if (scope->nestedIn)
        return findNamespace(tok, scope->nestedIn);

    return nullptr;
}

//---------------------------------------------------------------------------

Function * SymbolDatabase::findFunctionInScope(const Token *func, const Scope *ns, const std::string & path, nonneg int path_length)
{
    const Function * function = nullptr;
    const bool destructor = func->strAt(-1) == "~";

    auto range = ns->functionMap.equal_range(func->str());
    for (std::multimap<std::string, const Function*>::const_iterator it = range.first; it != range.second; ++it) {
        if (it->second->argsMatch(ns, it->second->argDef, func->next(), path, path_length) &&
            it->second->isDestructor() == destructor) {
            function = it->second;
            break;
        }
    }

    if (!function) {
        const Scope * scope = ns->findRecordInNestedList(func->str());
        if (scope && Token::Match(func->tokAt(1), "::|<")) {
            if (func->strAt(1) == "::")
                func = func->tokAt(2);
            else if (func->linkAt(1))
                func = func->linkAt(1)->tokAt(2);
            else
                return nullptr;
            if (func->str() == "~")
                func = func->next();
            function = findFunctionInScope(func, scope, path, path_length);
        }
    }

    return const_cast<Function *>(function);
}

//---------------------------------------------------------------------------

bool SymbolDatabase::isReservedName(const std::string& iName) const
{
    if (isCPP()) {
        static const auto& cpp_keywords = Keywords::getAll(Standards::cppstd_t::CPPLatest);
        return cpp_keywords.find(iName) != cpp_keywords.cend();
    }
    static const auto& c_keywords = Keywords::getAll(Standards::cstd_t::CLatest);
    return c_keywords.find(iName) != c_keywords.cend();
}

nonneg int SymbolDatabase::sizeOfType(const Token *type) const
{
    int size = mTokenizer.sizeOfType(type);

    if (size == 0 && type->type() && type->type()->isEnumType() && type->type()->classScope) {
        size = mSettings.platform.sizeof_int;
        const Token * enum_type = type->type()->classScope->enumType;
        if (enum_type)
            size = mTokenizer.sizeOfType(enum_type);
    }

    return size;
}

static const Token* parsedecl(const Token* type,
                              ValueType* const valuetype,
                              ValueType::Sign defaultSignedness,
                              const Settings& settings,
                              bool isCpp,
                              SourceLocation loc = SourceLocation::current());

void SymbolDatabase::setValueType(Token* tok, const Variable& var, SourceLocation loc)
{
    ValueType valuetype;
    if (mSettings.debugnormal || mSettings.debugwarnings)
        valuetype.setDebugPath(tok, loc);
    if (var.nameToken())
        valuetype.bits = var.nameToken()->bits();

    valuetype.pointer = var.dimensions().size();
    // HACK: don't set pointer for plain std::array
    if (var.valueType() && var.valueType()->container && Token::simpleMatch(var.typeStartToken(), "std :: array") && !Token::simpleMatch(var.nameToken()->next(), "["))
        valuetype.pointer = 0;

    valuetype.typeScope = var.typeScope();
    if (var.valueType()) {
        valuetype.container = var.valueType()->container;
        valuetype.containerTypeToken = var.valueType()->containerTypeToken;
    }
    valuetype.smartPointerType = var.smartPointerType();
    if (parsedecl(var.typeStartToken(), &valuetype, mDefaultSignedness, mSettings, mIsCpp)) {
        if (tok->str() == "." && tok->astOperand1()) {
            const ValueType * const vt = tok->astOperand1()->valueType();
            if (vt && (vt->constness & 1) != 0)
                valuetype.constness |= 1;
        }
        setValueType(tok, valuetype);
    }
}

static ValueType::Type getEnumType(const Scope* scope, const Platform& platform);

void SymbolDatabase::setValueType(Token* tok, const Enumerator& enumerator, SourceLocation loc)
{
    ValueType valuetype;
    if (mSettings.debugnormal || mSettings.debugwarnings)
        valuetype.setDebugPath(tok, loc);
    valuetype.typeScope = enumerator.scope;
    const Token * type = enumerator.scope->enumType;
    if (type) {
        valuetype.type = ValueType::typeFromString(type->str(), type->isLong());
        if (valuetype.type == ValueType::Type::UNKNOWN_TYPE && type->isStandardType())
            valuetype.fromLibraryType(type->str(), mSettings);

        if (valuetype.isIntegral()) {
            if (type->isSigned())
                valuetype.sign = ValueType::Sign::SIGNED;
            else if (type->isUnsigned())
                valuetype.sign = ValueType::Sign::UNSIGNED;
            else if (valuetype.type == ValueType::Type::CHAR)
                valuetype.sign = mDefaultSignedness;
            else
                valuetype.sign = ValueType::Sign::SIGNED;
        }

        setValueType(tok, valuetype);
    } else {
        valuetype.sign = ValueType::SIGNED;
        valuetype.type = getEnumType(enumerator.scope, mSettings.platform);
        setValueType(tok, valuetype);
    }
}

static void setAutoTokenProperties(Token * const autoTok)
{
    const ValueType *valuetype = autoTok->valueType();
    if (valuetype->isIntegral() || valuetype->isFloat())
        autoTok->isStandardType(true);
}

bool isContainerYieldElement(Library::Container::Yield yield)
{
    return yield == Library::Container::Yield::ITEM || yield == Library::Container::Yield::AT_INDEX ||
           yield == Library::Container::Yield::BUFFER || yield == Library::Container::Yield::BUFFER_NT;
}

static bool isContainerYieldPointer(Library::Container::Yield yield)
{
    return yield == Library::Container::Yield::BUFFER || yield == Library::Container::Yield::BUFFER_NT;
}

void SymbolDatabase::setValueType(Token* tok, const ValueType& valuetype, SourceLocation loc)
{
    ValueType* valuetypePtr = new ValueType(valuetype);
    if (mSettings.debugnormal || mSettings.debugwarnings)
        valuetypePtr->setDebugPath(tok, loc);
    tok->setValueType(valuetypePtr);
    Token *parent = tok->astParent();
    if (!parent || parent->valueType())
        return;
    if (!parent->astOperand1())
        return;

    const ValueType *vt1 = parent->astOperand1()->valueType();
    const ValueType *vt2 = parent->astOperand2() ? parent->astOperand2()->valueType() : nullptr;

    if (vt1 && Token::Match(parent, "<<|>>")) {
        if (!mIsCpp || (vt2 && vt2->isIntegral())) {
            if (vt1->type < ValueType::Type::BOOL || vt1->type >= ValueType::Type::INT) {
                ValueType vt(*vt1);
                vt.reference = Reference::None;
                setValueType(parent, vt);
            } else {
                ValueType vt(*vt1);
                vt.type = ValueType::Type::INT; // Integer promotion
                vt.sign = ValueType::Sign::SIGNED;
                vt.reference = Reference::None;
                setValueType(parent, vt);
            }

        }
        return;
    }

    if (vt1 && vt1->container && vt1->containerTypeToken && Token::Match(parent, ". %name% (") &&
        isContainerYieldElement(vt1->container->getYield(parent->next()->str()))) {
        ValueType item;
        if (parsedecl(vt1->containerTypeToken, &item, mDefaultSignedness, mSettings, mIsCpp)) {
            if (item.constness == 0)
                item.constness = vt1->constness;
            if (isContainerYieldPointer(vt1->container->getYield(parent->next()->str())))
                item.pointer += 1;
            else
                item.reference = Reference::LValue;
            setValueType(parent->tokAt(2), item);
        }
    }

    if (vt1 && vt1->smartPointerType && Token::Match(parent, ". %name% (") && parent->originalName() == "->" && !parent->next()->function()) {
        const Scope *scope = vt1->smartPointerType->classScope;
        const Function *f = scope ? scope->findFunction(parent->next(), false) : nullptr;
        if (f)
            parent->next()->function(f);
    }

    if (parent->isAssignmentOp()) {
        if (vt1) {
            auto vt = *vt1;
            vt.reference = Reference::None;
            setValueType(parent, vt);
        } else if (mIsCpp && ((Token::Match(parent->tokAt(-3), "%var% ; %var% =") && parent->strAt(-3) == parent->strAt(-1)) ||
                              Token::Match(parent->tokAt(-1), "%var% ="))) {
            Token *var1Tok = parent->strAt(-2) == ";" ? parent->tokAt(-3) : parent->tokAt(-1);
            Token *autoTok = nullptr;
            if (Token::simpleMatch(var1Tok->tokAt(-1), "auto"))
                autoTok = var1Tok->previous();
            else if (Token::Match(var1Tok->tokAt(-2), "auto *|&|&&"))
                autoTok = var1Tok->tokAt(-2);
            else if (Token::simpleMatch(var1Tok->tokAt(-3), "auto * const"))
                autoTok = var1Tok->tokAt(-3);
            if (autoTok) {
                ValueType vt(*vt2);
                if (vt.constness & (1 << vt.pointer))
                    vt.constness &= ~(1 << vt.pointer);
                if (autoTok->strAt(1) == "*" && vt.pointer)
                    vt.pointer--;
                if (Token::Match(autoTok->tokAt(-1), "const|constexpr"))
                    vt.constness |= (1 << vt.pointer);
                setValueType(autoTok, vt);
                setAutoTokenProperties(autoTok);
                if (vt2->pointer > vt.pointer)
                    vt.pointer++;
                setValueType(var1Tok, vt);
                if (var1Tok != parent->previous())
                    setValueType(parent->previous(), vt);
                Variable *var = const_cast<Variable *>(parent->previous()->variable());
                if (var) {
                    ValueType vt2_(*vt2);
                    if (vt2_.pointer == 0 && autoTok->strAt(1) == "*")
                        vt2_.pointer = 1;
                    if ((vt.constness & (1 << vt2->pointer)) != 0)
                        vt2_.constness |= (1 << vt2->pointer);
                    if (!Token::Match(autoTok->tokAt(1), "*|&"))
                        vt2_.constness = vt.constness;
                    if (Token::simpleMatch(autoTok->tokAt(1), "* const"))
                        vt2_.constness |= (1 << vt2->pointer);
                    var->setValueType(vt2_);
                    if (vt2->typeScope && vt2->typeScope->definedType) {
                        var->type(vt2->typeScope->definedType);
                        if (autoTok->valueType()->pointer == 0)
                            autoTok->type(vt2->typeScope->definedType);
                    }
                }
            }
        }
        return;
    }

    if (parent->str() == "[" && (!mIsCpp || parent->astOperand1() == tok) && valuetype.pointer > 0U && !Token::Match(parent->previous(), "[{,]")) {
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
    // std::move
    if (vt2 && parent->str() == "(" && Token::simpleMatch(parent->tokAt(-3), "std :: move (")) {
        ValueType vt = valuetype;
        vt.reference = Reference::RValue;
        setValueType(parent, vt);
        return;
    }
    if (parent->str() == "*" && !parent->astOperand2() && valuetype.pointer > 0U) {
        ValueType vt(valuetype);
        vt.pointer -= 1U;
        setValueType(parent, vt);
        return;
    }
    // Dereference iterator
    if (parent->str() == "*" && !parent->astOperand2() && valuetype.type == ValueType::Type::ITERATOR &&
        valuetype.containerTypeToken) {
        ValueType vt;
        if (parsedecl(valuetype.containerTypeToken, &vt, mDefaultSignedness, mSettings, mIsCpp)) {
            if (vt.constness == 0)
                vt.constness = valuetype.constness;
            vt.reference = Reference::LValue;
            setValueType(parent, vt);
            return;
        }
    }
    // Dereference smart pointer
    if (parent->str() == "*" && !parent->astOperand2() && valuetype.type == ValueType::Type::SMART_POINTER &&
        valuetype.smartPointerTypeToken) {
        ValueType vt;
        if (parsedecl(valuetype.smartPointerTypeToken, &vt, mDefaultSignedness, mSettings, mIsCpp)) {
            if (vt.constness == 0)
                vt.constness = valuetype.constness;
            setValueType(parent, vt);
            return;
        }
    }
    if (parent->str() == "*" && Token::simpleMatch(parent->astOperand2(), "[") && valuetype.pointer > 0U) {
        const Token *op1 = parent->astOperand2()->astOperand1();
        while (op1 && op1->str() == "[")
            op1 = op1->astOperand1();
        const ValueType& vt(valuetype);
        if (op1 && op1->variable() && op1->variable()->nameToken() == op1) {
            setValueType(parent, vt);
            return;
        }
    }
    if (parent->str() == "&" && !parent->astOperand2()) {
        ValueType vt(valuetype);
        vt.reference = Reference::None; //Given int& x; the type of &x is int* not int&*
        bool isArrayToPointerDecay = false;
        for (const Token* child = parent->astOperand1(); child;) {
            if (Token::Match(child, ".|::"))
                child = child->astOperand2();
            else {
                isArrayToPointerDecay = child->variable() && child->variable()->isArray();
                break;
            }
        }
        if (!isArrayToPointerDecay)
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
            auto it = std::find_if(typeScope->varlist.begin(), typeScope->varlist.end(), [&name](const Variable& v) {
                return v.nameToken()->str() == name;
            });
            if (it != typeScope->varlist.end())
                var = &*it;
        }
        if (var) {
            setValueType(parent, *var);
            return;
        }
        if (const Enumerator* enu = parent->astOperand2()->enumerator())
            setValueType(parent, *enu);
        return;
    }

    // range for loop, auto
    if (vt2 &&
        parent->str() == ":" &&
        Token::Match(parent->astParent(), "( const| auto *|&|&&| %var% :") && // TODO: east-const, multiple const, ref to ptr
        !parent->previous()->valueType() &&
        Token::simpleMatch(parent->astParent()->astOperand1(), "for")) {
        const bool isconst = Token::simpleMatch(parent->astParent()->next(), "const");
        Token * const autoToken = parent->astParent()->tokAt(isconst ? 2 : 1);
        if (vt2->pointer) {
            ValueType autovt(*vt2);
            autovt.pointer--;
            autovt.constness = 0;
            setValueType(autoToken, autovt);
            setAutoTokenProperties(autoToken);
            ValueType varvt(*vt2);
            varvt.pointer--;
            if (Token::simpleMatch(autoToken->next(), "&"))
                varvt.reference = Reference::LValue;
            if (isconst) {
                if (varvt.pointer && varvt.reference != Reference::None)
                    varvt.constness |= (1 << varvt.pointer);
                else
                    varvt.constness |= 1;
            }
            setValueType(parent->previous(), varvt);
            Variable *var = const_cast<Variable *>(parent->previous()->variable());
            if (var) {
                var->setValueType(varvt);
                if (vt2->typeScope && vt2->typeScope->definedType) {
                    var->type(vt2->typeScope->definedType);
                    autoToken->type(vt2->typeScope->definedType);
                }
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

            // Try to determine type of "auto" token.
            // TODO: Get type better
            bool setType = false;
            ValueType autovt;
            const Type *templateArgType = nullptr; // container element type / smart pointer type
            if (!vt2->container->rangeItemRecordType.empty()) {
                setType = true;
                autovt.type = ValueType::Type::RECORD;
            } else if (vt2->containerTypeToken) {
                if (mSettings.library.isSmartPointer(vt2->containerTypeToken)) {
                    const Token *smartPointerTypeTok = vt2->containerTypeToken;
                    while (Token::Match(smartPointerTypeTok, "%name%|::"))
                        smartPointerTypeTok = smartPointerTypeTok->next();
                    if (Token::simpleMatch(smartPointerTypeTok, "<")) {
                        if ((templateArgType = findTypeInNested(smartPointerTypeTok->next(), tok->scope()))) {
                            setType = true;
                            autovt.smartPointerType = templateArgType;
                            autovt.type = ValueType::Type::NONSTD;
                        }
                    }
                } else if (parsedecl(vt2->containerTypeToken, &autovt, mDefaultSignedness, mSettings, mIsCpp)) {
                    setType = true;
                    templateArgType = vt2->containerTypeToken->type();
                    if (Token::simpleMatch(autoToken->next(), "&"))
                        autovt.reference = Reference::LValue;
                    else if (Token::simpleMatch(autoToken->next(), "&&"))
                        autovt.reference = Reference::RValue;
                    if (autoToken->previous()->str() == "const") {
                        if (autovt.pointer && autovt.reference != Reference::None)
                            autovt.constness |= 2;
                        else
                            autovt.constness |= 1;
                    }
                }
            }

            if (setType) {
                // Type of "auto" has been determined.. set type information for "auto" and variable tokens
                setValueType(autoToken, autovt);
                setAutoTokenProperties(autoToken);
                ValueType varvt(autovt);
                if (autoToken->strAt(1) == "*" && autovt.pointer)
                    autovt.pointer--;
                if (isconst)
                    varvt.constness |= (1 << autovt.pointer);
                setValueType(parent->previous(), varvt);
                Variable * var = const_cast<Variable *>(parent->previous()->variable());
                if (var) {
                    var->setValueType(varvt);
                    if (templateArgType && templateArgType->classScope && templateArgType->classScope->definedType) {
                        autoToken->type(templateArgType->classScope->definedType);
                        var->type(templateArgType->classScope->definedType);
                    }
                }
            }
        }
    }

    if (vt1 && vt1->containerTypeToken && parent->str() == "[") {
        ValueType vtParent;
        if (parsedecl(vt1->containerTypeToken, &vtParent, mDefaultSignedness, mSettings, mIsCpp)) {
            setValueType(parent, vtParent);
            return;
        }
    }

    if (mIsCpp && vt2 && Token::simpleMatch(parent->previous(), "decltype (")) {
        setValueType(parent, *vt2);
        return;
    }

    // c++17 auto type deduction of braced init list
    if (mIsCpp && mSettings.standards.cpp >= Standards::CPP17 && vt2 && Token::Match(parent->tokAt(-2), "auto %var% {")) {
        Token *autoTok = parent->tokAt(-2);
        setValueType(autoTok, *vt2);
        setAutoTokenProperties(autoTok);
        if (parent->previous()->variable())
            const_cast<Variable*>(parent->previous()->variable())->setValueType(*vt2);
        else
            debugMessage(parent->previous(), "debug", "Missing variable class for variable with varid");
        return;
    }

    if (!vt1)
        return;
    if (parent->astOperand2() && !vt2)
        return;

    const bool ternary = parent->str() == ":" && parent->astParent() && parent->astParent()->str() == "?";
    if (ternary) {
        if (vt2 && vt1->pointer == vt2->pointer && vt1->type == vt2->type && vt1->sign == vt2->sign)
            setValueType(parent, *vt2);
        parent = parent->astParent();
    }

    if (ternary || parent->isArithmeticalOp() || parent->tokType() == Token::eIncDecOp) {

        // CONTAINER + x => CONTAINER
        if (parent->str() == "+" && vt1->type == ValueType::Type::CONTAINER && vt2 && vt2->isIntegral()) {
            setValueType(parent, *vt1);
            return;
        }
        // x + CONTAINER => CONTAINER
        if (parent->str() == "+" && vt1->isIntegral() && vt2 && vt2->type == ValueType::Type::CONTAINER) {
            setValueType(parent, *vt2);
            return;
        }

        if (parent->isArithmeticalOp()) {
            if (vt1->pointer != 0U && vt2 && vt2->pointer == 0U) {
                setValueType(parent, *vt1);
                return;
            }

            if (vt1->pointer == 0U && vt2 && vt2->pointer != 0U) {
                setValueType(parent, *vt2);
                return;
            }
        } else if (ternary) {
            if (vt1->pointer != 0U && vt2 && vt2->pointer == 0U) {
                if (vt2->isPrimitive())
                    setValueType(parent, *vt1);
                else
                    setValueType(parent, *vt2);
                return;
            }

            if (vt1->pointer == 0U && vt2 && vt2->pointer != 0U) {
                if (vt1->isPrimitive())
                    setValueType(parent, *vt2);
                else
                    setValueType(parent, *vt1);
                return;
            }

            if (vt1->isTypeEqual(vt2)) {
                setValueType(parent, *vt1);
                return;
            }
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

        // iterator +/- integral = iterator
        if (vt1->type == ValueType::Type::ITERATOR && vt2 && vt2->isIntegral() &&
            (parent->str() == "+" || parent->str() == "-")) {
            setValueType(parent, *vt1);
            return;
        }

        if (parent->str() == "+" && vt1->type == ValueType::Type::CONTAINER && vt2 && vt2->type == ValueType::Type::CONTAINER && vt1->container == vt2->container) {
            setValueType(parent, *vt1);
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
        if (vt.type < ValueType::Type::INT && !(ternary && vt.type==ValueType::Type::BOOL)) {
            vt.type = ValueType::Type::INT;
            vt.sign = ValueType::Sign::SIGNED;
            vt.originalTypeName.clear();
        }

        setValueType(parent, vt);
        return;
    }
}

static ValueType::Type getEnumType(const Scope* scope, const Platform& platform) // TODO: also determine sign?
{
    ValueType::Type type = ValueType::Type::INT;
    for (const Token* tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
        if (!tok->isAssignmentOp())
            continue;
        const Token* vTok = tok->astOperand2();
        if (!vTok->hasKnownIntValue()) {
            if (!vTok->isLiteral())
                continue;
            if (const ValueType* vt = vTok->valueType()) {
                if ((vt->type > type && (vt->type == ValueType::Type::LONG || vt->type == ValueType::Type::LONGLONG)))
                    type = vt->type;
            }
            continue;
        }
        const MathLib::bigint value = vTok->getKnownIntValue();
        if (!platform.isIntValue(value)) {
            type = ValueType::Type::LONG;
            if (!platform.isLongValue(value))
                type = ValueType::Type::LONGLONG;
        }
    }
    return type;
}

static const Token* parsedecl(const Token* type,
                              ValueType* const valuetype,
                              ValueType::Sign defaultSignedness,
                              const Settings& settings,
                              bool isCpp,
                              SourceLocation loc)
{
    if (settings.debugnormal || settings.debugwarnings)
        valuetype->setDebugPath(type, loc);
    const Token * const previousType = type;
    const unsigned int pointer0 = valuetype->pointer;
    while (Token::Match(type->previous(), "%name%") && !endsWith(type->previous()->str(), ':'))
        type = type->previous();
    valuetype->sign = ValueType::Sign::UNKNOWN_SIGN;
    if (!valuetype->typeScope && !valuetype->smartPointerType)
        valuetype->type = ValueType::Type::UNKNOWN_TYPE;
    else if (valuetype->smartPointerType)
        valuetype->type = ValueType::Type::SMART_POINTER;
    else if (valuetype->typeScope->type == Scope::eEnum) {
        const Token * enum_type = valuetype->typeScope->enumType;
        if (enum_type) {
            if (enum_type->isSigned())
                valuetype->sign = ValueType::Sign::SIGNED;
            else if (enum_type->isUnsigned())
                valuetype->sign = ValueType::Sign::UNSIGNED;
            else
                valuetype->sign = defaultSignedness;
            const ValueType::Type t = ValueType::typeFromString(enum_type->str(), enum_type->isLong());
            if (t != ValueType::Type::UNKNOWN_TYPE)
                valuetype->type = t;
            else if (enum_type->isStandardType())
                valuetype->fromLibraryType(enum_type->str(), settings);
        } else
            valuetype->type = getEnumType(valuetype->typeScope, settings.platform);
    } else
        valuetype->type = ValueType::Type::RECORD;
    bool par = false;
    while (Token::Match(type, "%name%|*|&|&&|::|(") && !Token::Match(type, "typename|template") && type->varId() == 0 &&
           !type->variable() && !type->function()) {
        bool isIterator = false;
        if (type->str() == "(") {
            if (Token::Match(type->link(), ") const| {"))
                break;
            if (par)
                break;
            par = true;
        }
        if (Token::simpleMatch(type, "decltype (") && type->next()->valueType()) {
            const ValueType *vt2 = type->next()->valueType();
            if (valuetype->sign == ValueType::Sign::UNKNOWN_SIGN)
                valuetype->sign = vt2->sign;
            if (valuetype->type == ValueType::Type::UNKNOWN_TYPE)
                valuetype->type = vt2->type;
            valuetype->constness += vt2->constness;
            valuetype->pointer += vt2->pointer;
            valuetype->reference = vt2->reference;
            type = type->linkAt(1)->next();
            continue;
        }
        if (type->isSigned())
            valuetype->sign = ValueType::Sign::SIGNED;
        else if (type->isUnsigned())
            valuetype->sign = ValueType::Sign::UNSIGNED;
        if (valuetype->type == ValueType::Type::UNKNOWN_TYPE &&
            type->type() && type->type()->isTypeAlias() && type->type()->typeStart &&
            type->type()->typeStart->str() != type->str() && type->type()->typeStart != previousType)
            parsedecl(type->type()->typeStart, valuetype, defaultSignedness, settings, isCpp);
        else if (Token::Match(type, "const|constexpr"))
            valuetype->constness |= (1 << (valuetype->pointer - pointer0));
        else if (settings.clang && type->str().size() > 2 && type->str().find("::") < type->str().find('<')) {
            TokenList typeTokens(&settings);
            std::string::size_type pos1 = 0;
            do {
                const std::string::size_type pos2 = type->str().find("::", pos1);
                if (pos2 == std::string::npos) {
                    typeTokens.addtoken(type->str().substr(pos1), 0, 0, 0, false);
                    break;
                }
                typeTokens.addtoken(type->str().substr(pos1, pos2 - pos1), 0, 0, 0, false);
                typeTokens.addtoken("::", 0, 0, 0, false);
                pos1 = pos2 + 2;
            } while (pos1 < type->str().size());
            const Library::Container* container =
                settings.library.detectContainerOrIterator(typeTokens.front(), &isIterator);
            if (container) {
                if (isIterator)
                    valuetype->type = ValueType::Type::ITERATOR;
                else
                    valuetype->type = ValueType::Type::CONTAINER;
                valuetype->container = container;
            } else {
                const Scope *scope = type->scope();
                valuetype->typeScope = scope->check->findScope(typeTokens.front(), scope);
                if (valuetype->typeScope)
                    valuetype->type = (scope->type == Scope::ScopeType::eClass) ? ValueType::Type::RECORD : ValueType::Type::NONSTD;
            }
        } else if (const Library::Container* container = (isCpp ? settings.library.detectContainerOrIterator(type, &isIterator) : nullptr)) {
            if (isIterator)
                valuetype->type = ValueType::Type::ITERATOR;
            else
                valuetype->type = ValueType::Type::CONTAINER;
            valuetype->container = container;
            while (Token::Match(type, "%type%|::|<") && type->str() != "const") {
                if (type->str() == "<" && type->link()) {
                    if (container->type_templateArgNo >= 0) {
                        const Token *templateType = type->next();
                        for (int j = 0; templateType && j < container->type_templateArgNo; j++)
                            templateType = templateType->nextTemplateArgument();
                        valuetype->containerTypeToken = templateType;
                    }
                    type = type->link();
                }
                type = type->next();
            }
            if (type && type->str() == "(" && type->previous()->function())
                // we are past the end of the type
                type = type->previous();
            continue;
        } else if (const Library::SmartPointer* smartPointer = (isCpp ? settings.library.detectSmartPointer(type) : nullptr)) {
            const Token* argTok = Token::findsimplematch(type, "<");
            if (!argTok)
                break;
            valuetype->smartPointer = smartPointer;
            valuetype->smartPointerTypeToken = argTok->next();
            valuetype->smartPointerType = argTok->next()->type();
            valuetype->type = ValueType::Type::SMART_POINTER;
            type = argTok->link();
            if (type)
                type = type->next();
            continue;
        } else if (Token::Match(type, "%name% :: %name%")) {
            std::string typestr;
            const Token *end = type;
            while (Token::Match(end, "%name% :: %name%")) {
                typestr += end->str() + "::";
                end = end->tokAt(2);
            }
            typestr += end->str();
            if (valuetype->fromLibraryType(typestr, settings))
                type = end;
        } else if (ValueType::Type::UNKNOWN_TYPE != ValueType::typeFromString(type->str(), type->isLong())) {
            const ValueType::Type t0 = valuetype->type;
            valuetype->type = ValueType::typeFromString(type->str(), type->isLong());
            if (t0 == ValueType::Type::LONG) {
                if (valuetype->type == ValueType::Type::LONG)
                    valuetype->type = ValueType::Type::LONGLONG;
                else if (valuetype->type == ValueType::Type::DOUBLE)
                    valuetype->type = ValueType::Type::LONGDOUBLE;
            }
        } else if (type->str() == "auto") {
            const ValueType *vt = type->valueType();
            if (!vt)
                return nullptr;
            valuetype->type = vt->type;
            valuetype->pointer = vt->pointer;
            valuetype->reference = vt->reference;
            if (vt->sign != ValueType::Sign::UNKNOWN_SIGN)
                valuetype->sign = vt->sign;
            valuetype->constness = vt->constness;
            valuetype->originalTypeName = vt->originalTypeName;
            const bool hasConst = Token::simpleMatch(type->previous(), "const");
            while (Token::Match(type, "%name%|*|&|&&|::") && !type->variable()) {
                if (type->str() == "*") {
                    valuetype->pointer = 1;
                    if (hasConst)
                        valuetype->constness = 1;
                } else if (type->str() == "&") {
                    valuetype->reference = Reference::LValue;
                } else if (type->str() == "&&") {
                    valuetype->reference = Reference::RValue;
                }
                if (type->str() == "const")
                    valuetype->constness |= (1 << valuetype->pointer);
                type = type->next();
            }
            break;
        } else if (!valuetype->typeScope && (type->str() == "struct" || type->str() == "enum"))
            valuetype->type = type->str() == "struct" ? ValueType::Type::RECORD : ValueType::Type::NONSTD;
        else if (!valuetype->typeScope && type->type() && type->type()->classScope) {
            if (type->type()->classScope->type == Scope::ScopeType::eEnum) {
                valuetype->sign = ValueType::Sign::SIGNED;
                valuetype->type = getEnumType(type->type()->classScope, settings.platform);
            } else {
                valuetype->type = ValueType::Type::RECORD;
            }
            valuetype->typeScope = type->type()->classScope;
        } else if (type->isName() && valuetype->sign != ValueType::Sign::UNKNOWN_SIGN && valuetype->pointer == 0U)
            return nullptr;
        else if (type->str() == "*")
            valuetype->pointer++;
        else if (type->str() == "&")
            valuetype->reference = Reference::LValue;
        else if (type->str() == "&&")
            valuetype->reference = Reference::RValue;
        else if (type->isStandardType())
            valuetype->fromLibraryType(type->str(), settings);
        else if (Token::Match(type->previous(), "!!:: %name% !!::"))
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

    return (type && (valuetype->type != ValueType::Type::UNKNOWN_TYPE || valuetype->pointer > 0 || valuetype->reference != Reference::None)) ? type : nullptr;
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

    const Scope *classScope = getClassScope(tok->astOperand1());
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

static const Function* getFunction(const Token* tok) {
    if (!tok)
        return nullptr;
    if (tok->function() && tok->function()->retDef)
        return tok->function();
    if (const Variable* lvar = tok->variable()) { // lambda
        const Function* lambda{};
        if (Token::Match(lvar->nameToken()->next(), "; %varid% = [", lvar->declarationId()))
            lambda = lvar->nameToken()->tokAt(4)->function();
        else if (Token::simpleMatch(lvar->nameToken()->next(), "{ ["))
            lambda = lvar->nameToken()->tokAt(2)->function();
        if (lambda && lambda->retDef)
            return lambda;
    }
    return nullptr;
}

void SymbolDatabase::setValueTypeInTokenList(bool reportDebugWarnings, Token *tokens)
{
    if (!tokens)
        tokens = const_cast<Tokenizer &>(mTokenizer).list.front();

    for (Token *tok = tokens; tok; tok = tok->next())
        tok->setValueType(nullptr);

    for (Token *tok = tokens; tok; tok = tok->next()) {
        if (tok->isNumber()) {
            if (MathLib::isFloat(tok->str())) {
                ValueType::Type type = ValueType::Type::DOUBLE;
                const char suffix = tok->str()[tok->str().size() - 1];
                if (suffix == 'f' || suffix == 'F')
                    type = ValueType::Type::FLOAT;
                else if (suffix == 'L' || suffix == 'l')
                    type = ValueType::Type::LONGDOUBLE;
                setValueType(tok, ValueType(ValueType::Sign::UNKNOWN_SIGN, type, 0U));
            } else if (MathLib::isInt(tok->str())) {
                const std::string tokStr = MathLib::abs(tok->str());
                const bool unsignedSuffix = (tokStr.find_last_of("uU") != std::string::npos);
                ValueType::Sign sign = unsignedSuffix ? ValueType::Sign::UNSIGNED : ValueType::Sign::SIGNED;
                ValueType::Type type = ValueType::Type::INT;
                const MathLib::biguint value = MathLib::toBigUNumber(tokStr);
                for (std::size_t pos = tokStr.size() - 1U; pos > 0U; --pos) {
                    const char suffix = tokStr[pos];
                    if (suffix == 'u' || suffix == 'U')
                        sign = ValueType::Sign::UNSIGNED;
                    else if (suffix == 'l' || suffix == 'L')
                        type = (type == ValueType::Type::INT) ? ValueType::Type::LONG : ValueType::Type::LONGLONG;
                    else if (pos > 2U && suffix == '4' && tokStr[pos - 1] == '6' && tokStr[pos - 2] == 'i') {
                        type = ValueType::Type::LONGLONG;
                        pos -= 2;
                    } else break;
                }
                if (mSettings.platform.type != Platform::Type::Unspecified) {
                    if (type <= ValueType::Type::INT && mSettings.platform.isIntValue(unsignedSuffix ? (value >> 1) : value))
                        type = ValueType::Type::INT;
                    else if (type <= ValueType::Type::INT && !MathLib::isDec(tokStr) && mSettings.platform.isIntValue(value >> 2)) {
                        type = ValueType::Type::INT;
                        sign = ValueType::Sign::UNSIGNED;
                    } else if (type <= ValueType::Type::LONG && mSettings.platform.isLongValue(unsignedSuffix ? (value >> 1) : value))
                        type = ValueType::Type::LONG;
                    else if (type <= ValueType::Type::LONG && !MathLib::isDec(tokStr) && mSettings.platform.isLongValue(value >> 2)) {
                        type = ValueType::Type::LONG;
                        sign = ValueType::Sign::UNSIGNED;
                    } else if (mSettings.platform.isLongLongValue(unsignedSuffix ? (value >> 1) : value))
                        type = ValueType::Type::LONGLONG;
                    else {
                        type = ValueType::Type::LONGLONG;
                        sign = ValueType::Sign::UNSIGNED;
                    }
                }

                setValueType(tok, ValueType(sign, type, 0U));
            }
        } else if (tok->isComparisonOp() || tok->tokType() == Token::eLogicalOp) {
            if (mIsCpp && tok->isComparisonOp() && (getClassScope(tok->astOperand1()) || getClassScope(tok->astOperand2()))) {
                const Function *function = getOperatorFunction(tok);
                if (function) {
                    ValueType vt;
                    parsedecl(function->retDef, &vt, mDefaultSignedness, mSettings, mIsCpp);
                    setValueType(tok, vt);
                    continue;
                }
            }
            setValueType(tok, ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::BOOL, 0U));
        } else if (tok->isBoolean()) {
            setValueType(tok, ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::BOOL, 0U));
        } else if (tok->tokType() == Token::eChar || tok->tokType() == Token::eString) {
            nonneg int const pointer = tok->tokType() == Token::eChar ? 0U : 1U;
            nonneg int const constness = tok->tokType() == Token::eChar ? 0U : 1U;
            ValueType valuetype(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::CHAR, pointer, constness);

            if (mIsCpp && mSettings.standards.cpp >= Standards::CPP20 && tok->isUtf8()) {
                valuetype.originalTypeName = "char8_t";
                valuetype.fromLibraryType(valuetype.originalTypeName, mSettings);
            } else if (tok->isUtf16()) {
                valuetype.originalTypeName = "char16_t";
                valuetype.fromLibraryType(valuetype.originalTypeName, mSettings);
            } else if (tok->isUtf32()) {
                valuetype.originalTypeName = "char32_t";
                valuetype.fromLibraryType(valuetype.originalTypeName, mSettings);
            } else if (tok->isLong()) {
                valuetype.originalTypeName = "wchar_t";
                valuetype.type = ValueType::Type::WCHAR_T;
            } else if ((tok->tokType() == Token::eChar) && ((tok->isCChar() && !mIsCpp) || (tok->isCMultiChar()))) {
                valuetype.type = ValueType::Type::INT;
                valuetype.sign = ValueType::Sign::SIGNED;
            }
            setValueType(tok, valuetype);
        } else if (tok->link() && Token::Match(tok, "(|{")) {
            const Token* start = tok->astOperand1() ? tok->astOperand1()->findExpressionStartEndTokens().first : nullptr;
            // cast
            if (tok->isCast() && !tok->astOperand2() && Token::Match(tok, "( %name%")) {
                ValueType valuetype;
                if (Token::simpleMatch(parsedecl(tok->next(), &valuetype, mDefaultSignedness, mSettings, mIsCpp), ")"))
                    setValueType(tok, valuetype);
            }

            // C++ cast
            else if (tok->astOperand2() && Token::Match(tok->astOperand1(), "static_cast|const_cast|dynamic_cast|reinterpret_cast < %name%") && tok->astOperand1()->linkAt(1)) {
                ValueType valuetype;
                if (Token::simpleMatch(parsedecl(tok->astOperand1()->tokAt(2), &valuetype, mDefaultSignedness, mSettings, mIsCpp), ">"))
                    setValueType(tok, valuetype);
            }

            // Construct smart pointer
            else if (mIsCpp && mSettings.library.isSmartPointer(start)) {
                ValueType valuetype;
                if (parsedecl(start, &valuetype, mDefaultSignedness, mSettings, mIsCpp)) {
                    setValueType(tok, valuetype);
                    setValueType(tok->astOperand1(), valuetype);
                }

            }

            // function or lambda
            else if (const Function* f = getFunction(tok->previous())) {
                ValueType valuetype;
                if (parsedecl(f->retDef, &valuetype, mDefaultSignedness, mSettings, mIsCpp))
                    setValueType(tok, valuetype);
            }

            else if (Token::simpleMatch(tok->previous(), "sizeof (")) {
                ValueType valuetype(ValueType::Sign::UNSIGNED, ValueType::Type::LONG, 0U);
                if (mSettings.platform.type == Platform::Type::Win64)
                    valuetype.type = ValueType::Type::LONGLONG;

                valuetype.originalTypeName = "size_t";
                setValueType(tok, valuetype);

                if (Token::Match(tok, "( %type% %type%| *| *| )")) {
                    ValueType vt;
                    if (parsedecl(tok->next(), &vt, mDefaultSignedness, mSettings, mIsCpp)) {
                        setValueType(tok->next(), vt);
                    }
                }
            }

            // function style cast
            else if (tok->previous() && tok->previous()->isStandardType()) {
                ValueType valuetype;
                if (tok->astOperand1() && valuetype.fromLibraryType(tok->astOperand1()->expressionString(), mSettings)) {
                    setValueType(tok, valuetype);
                    continue;
                }

                valuetype.type = ValueType::typeFromString(tok->previous()->str(), tok->previous()->isLong());
                if (tok->previous()->isUnsigned())
                    valuetype.sign = ValueType::Sign::UNSIGNED;
                else if (tok->previous()->isSigned())
                    valuetype.sign = ValueType::Sign::SIGNED;
                else if (valuetype.isIntegral() && valuetype.type != ValueType::UNKNOWN_INT)
                    valuetype.sign = mDefaultSignedness;
                setValueType(tok, valuetype);
            }

            // constructor call
            else if (tok->previous() && tok->previous()->function() && tok->previous()->function()->isConstructor()) {
                ValueType valuetype;
                valuetype.type = ValueType::RECORD;
                valuetype.typeScope = tok->previous()->function()->token->scope();
                setValueType(tok, valuetype);
            }

            else if (Token::simpleMatch(tok->previous(), "= {") && tok->tokAt(-2) && tok->tokAt(-2)->valueType()) {
                ValueType vt = *tok->tokAt(-2)->valueType();
                setValueType(tok, vt);
            }

            // library type/function
            else if (tok->previous()) {
                // Aggregate constructor
                if (Token::Match(tok->previous(), "%name%")) {
                    ValueType valuetype;
                    if (parsedecl(tok->previous(), &valuetype, mDefaultSignedness, mSettings, mIsCpp)) {
                        if (valuetype.typeScope) {
                            setValueType(tok, valuetype);
                            continue;
                        }
                    }
                }
                if (mIsCpp && tok->astParent() && Token::Match(tok->astOperand1(), "%name%|::")) {
                    const Token *typeStartToken = tok->astOperand1();
                    while (typeStartToken && typeStartToken->str() == "::")
                        typeStartToken = typeStartToken->astOperand1();
                    if (mSettings.library.detectContainerOrIterator(typeStartToken) ||
                        mSettings.library.detectSmartPointer(typeStartToken)) {
                        ValueType vt;
                        if (parsedecl(typeStartToken, &vt, mDefaultSignedness, mSettings, mIsCpp)) {
                            setValueType(tok, vt);
                            continue;
                        }
                    }

                    const std::string e = tok->astOperand1()->expressionString();

                    if ((e == "std::make_shared" || e == "std::make_unique") && Token::Match(tok->astOperand1(), ":: %name% < %name%")) {
                        ValueType vt;
                        parsedecl(tok->astOperand1()->tokAt(3), &vt, mDefaultSignedness, mSettings, mIsCpp);
                        if (vt.typeScope) {
                            vt.smartPointerType = vt.typeScope->definedType;
                            vt.typeScope = nullptr;
                        }
                        if (e == "std::make_shared" && mSettings.library.smartPointers.count("std::shared_ptr") > 0)
                            vt.smartPointer = &mSettings.library.smartPointers.at("std::shared_ptr");
                        if (e == "std::make_unique" && mSettings.library.smartPointers.count("std::unique_ptr") > 0)
                            vt.smartPointer = &mSettings.library.smartPointers.at("std::unique_ptr");
                        vt.type = ValueType::Type::SMART_POINTER;
                        vt.smartPointerTypeToken = tok->astOperand1()->tokAt(3);
                        setValueType(tok, vt);
                        continue;
                    }

                    ValueType podtype;
                    if (podtype.fromLibraryType(e, mSettings)) {
                        setValueType(tok, podtype);
                        continue;
                    }
                }

                const std::string& typestr(mSettings.library.returnValueType(tok->previous()));
                if (!typestr.empty()) {
                    ValueType valuetype;
                    TokenList tokenList(&mSettings);
                    std::istringstream istr(typestr+";");
                    tokenList.createTokens(istr);
                    tokenList.simplifyStdType();
                    if (parsedecl(tokenList.front(), &valuetype, mDefaultSignedness, mSettings, mIsCpp)) {
                        valuetype.originalTypeName = typestr;
                        setValueType(tok, valuetype);
                    }
                }

                //Is iterator fetching function invoked on container?
                const bool isReturnIter = typestr == "iterator";
                if (typestr.empty() || isReturnIter) {
                    if (Token::simpleMatch(tok->astOperand1(), ".") &&
                        tok->astOperand1()->astOperand1() &&
                        tok->astOperand1()->astOperand2() &&
                        tok->astOperand1()->astOperand1()->valueType() &&
                        tok->astOperand1()->astOperand1()->valueType()->container) {
                        const Library::Container *cont = tok->astOperand1()->astOperand1()->valueType()->container;
                        const auto it = cont->functions.find(tok->astOperand1()->astOperand2()->str());
                        if (it != cont->functions.end()) {
                            if (it->second.yield == Library::Container::Yield::START_ITERATOR ||
                                it->second.yield == Library::Container::Yield::END_ITERATOR ||
                                it->second.yield == Library::Container::Yield::ITERATOR) {
                                ValueType vt;
                                vt.type = ValueType::Type::ITERATOR;
                                vt.container = cont;
                                vt.containerTypeToken =
                                    tok->astOperand1()->astOperand1()->valueType()->containerTypeToken;
                                setValueType(tok, vt);
                                continue;
                            }
                        }
                        //Is iterator fetching function called?
                    } else if (Token::simpleMatch(tok->astOperand1(), "::") &&
                               tok->astOperand2() &&
                               tok->astOperand2()->isVariable()) {
                        const auto* const paramVariable = tok->astOperand2()->variable();
                        if (!paramVariable ||
                            !paramVariable->valueType() ||
                            !paramVariable->valueType()->container) {
                            continue;
                        }

                        const auto yield = astFunctionYield(tok->previous(), &mSettings);
                        if (yield == Library::Container::Yield::START_ITERATOR ||
                            yield == Library::Container::Yield::END_ITERATOR ||
                            yield == Library::Container::Yield::ITERATOR) {
                            ValueType vt;
                            vt.type = ValueType::Type::ITERATOR;
                            vt.container = paramVariable->valueType()->container;
                            vt.containerTypeToken = paramVariable->valueType()->containerTypeToken;
                            setValueType(tok, vt);
                        }
                    }
                    if (isReturnIter) {
                        const std::vector<const Token*> args = getArguments(tok);
                        if (!args.empty()) {
                            const Library::ArgumentChecks::IteratorInfo* info = mSettings.library.getArgIteratorInfo(tok->previous(), 1);
                            if (info && info->it) {
                                const Token* contTok = args[0];
                                if (Token::simpleMatch(args[0]->astOperand1(), ".") && args[0]->astOperand1()->astOperand1()) // .begin()
                                    contTok = args[0]->astOperand1()->astOperand1();
                                else if (Token::simpleMatch(args[0], "(") && args[0]->astOperand2()) // std::begin()
                                    contTok = args[0]->astOperand2();
                                while (Token::simpleMatch(contTok, "[")) // move to container token
                                    contTok = contTok->astOperand1();
                                if (Token::simpleMatch(contTok, "."))
                                    contTok = contTok->astOperand2();
                                if (contTok && contTok->variable() && contTok->variable()->valueType() && contTok->variable()->valueType()->container) {
                                    ValueType vt;
                                    vt.type = ValueType::Type::ITERATOR;
                                    vt.container = contTok->variable()->valueType()->container;
                                    vt.containerTypeToken = contTok->variable()->valueType()->containerTypeToken;
                                    setValueType(tok, vt);
                                } else if (Token::simpleMatch(contTok, "(") && contTok->astOperand1() && contTok->astOperand1()->function()) {
                                    const Function* func = contTok->astOperand1()->function();
                                    if (const ValueType* funcVt = func->tokenDef->next()->valueType()) {
                                        ValueType vt;
                                        vt.type = ValueType::Type::ITERATOR;
                                        vt.container = funcVt->container;
                                        vt.containerTypeToken = funcVt->containerTypeToken;
                                        setValueType(tok, vt);
                                    }
                                }
                            }
                        }
                    }
                    continue;
                }
                TokenList tokenList(&mSettings);
                std::istringstream istr(typestr+";");
                if (tokenList.createTokens(istr)) {
                    ValueType vt;
                    tokenList.simplifyPlatformTypes();
                    tokenList.simplifyStdType();
                    if (parsedecl(tokenList.front(), &vt, mDefaultSignedness, mSettings, mIsCpp)) {
                        vt.originalTypeName = typestr;
                        setValueType(tok, vt);
                    }
                }
            }
        } else if (tok->str() == "return") {
            const Scope *functionScope = tok->scope();
            while (functionScope && functionScope->isExecutable() && functionScope->type != Scope::eLambda && functionScope->type != Scope::eFunction)
                functionScope = functionScope->nestedIn;
            if (functionScope && functionScope->type == Scope::eFunction && functionScope->function &&
                functionScope->function->retDef) {
                ValueType vt = ValueType::parseDecl(functionScope->function->retDef, mSettings);
                setValueType(tok, vt);
                if (Token::simpleMatch(tok, "return {"))
                    setValueType(tok->next(), vt);
            }
        } else if (tok->variable()) {
            setValueType(tok, *tok->variable());
            if (!tok->variable()->valueType() && tok->valueType())
                const_cast<Variable*>(tok->variable())->setValueType(*tok->valueType());
        } else if (tok->enumerator()) {
            setValueType(tok, *tok->enumerator());
        } else if (tok->isKeyword() && tok->str() == "new") {
            const Token *typeTok = tok->next();
            if (Token::Match(typeTok, "( std| ::| nothrow )"))
                typeTok = typeTok->link()->next();
            bool isIterator = false;
            if (const Library::Container* c = mSettings.library.detectContainerOrIterator(typeTok, &isIterator)) {
                ValueType vt;
                vt.pointer = 1;
                vt.container = c;
                vt.type = isIterator ? ValueType::Type::ITERATOR : ValueType::Type::CONTAINER;
                setValueType(tok, vt);
                continue;
            }
            std::string typestr;
            while (Token::Match(typeTok, "%name% :: %name%")) {
                typestr += typeTok->str() + "::";
                typeTok = typeTok->tokAt(2);
            }
            if (!Token::Match(typeTok, "%type% ;|[|("))
                continue;
            typestr += typeTok->str();
            ValueType vt;
            vt.pointer = 1;
            if (typeTok->type() && typeTok->type()->classScope) {
                vt.type = ValueType::Type::RECORD;
                vt.typeScope = typeTok->type()->classScope;
            } else {
                vt.type = ValueType::typeFromString(typestr, typeTok->isLong());
                if (vt.type == ValueType::Type::UNKNOWN_TYPE)
                    vt.fromLibraryType(typestr, mSettings);
                if (vt.type == ValueType::Type::UNKNOWN_TYPE)
                    continue;
                if (typeTok->isUnsigned())
                    vt.sign = ValueType::Sign::UNSIGNED;
                else if (typeTok->isSigned())
                    vt.sign = ValueType::Sign::SIGNED;
                if (vt.sign == ValueType::Sign::UNKNOWN_SIGN && vt.isIntegral())
                    vt.sign = (vt.type == ValueType::Type::CHAR) ? mDefaultSignedness : ValueType::Sign::SIGNED;
            }
            setValueType(tok, vt);
            if (Token::simpleMatch(tok->astOperand1(), "(")) {
                vt.pointer--;
                setValueType(tok->astOperand1(), vt);
            }
        } else if (tok->isKeyword() && tok->str() == "return" && tok->scope()) {
            const Scope* fscope = tok->scope();
            while (fscope && !fscope->function)
                fscope = fscope->nestedIn;
            if (fscope && fscope->function && fscope->function->retDef) {
                ValueType vt;
                parsedecl(fscope->function->retDef, &vt, mDefaultSignedness, mSettings, mIsCpp);
                setValueType(tok, vt);
            }
        } else if (tok->isKeyword() && tok->str() == "this" && tok->scope()->isExecutable()) {
            const Scope* fscope = tok->scope();
            while (fscope && !fscope->function)
                fscope = fscope->nestedIn;
            const Scope* defScope = fscope && fscope->function->tokenDef ? fscope->function->tokenDef->scope() : nullptr;
            if (defScope && defScope->isClassOrStruct()) {
                ValueType vt(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::RECORD, 1);
                vt.typeScope = defScope;
                if (fscope->function->isConst())
                    vt.constness = 1;
                setValueType(tok, vt);
            }
        }
    }

    if (reportDebugWarnings && mSettings.debugwarnings) {
        for (Token *tok = tokens; tok; tok = tok->next()) {
            if (tok->str() == "auto" && !tok->valueType()) {
                if (Token::Match(tok->next(), "%name% ; %name% = [") && isLambdaCaptureList(tok->tokAt(5)))
                    continue;
                if (Token::Match(tok->next(), "%name% {|= [") && isLambdaCaptureList(tok->tokAt(3)))
                    continue;
                debugMessage(tok, "autoNoType", "auto token with no type.");
            }
        }
    }

    // Update functions with new type information.
    createSymbolDatabaseSetFunctionPointers(false);

    // Update auto variables with new type information.
    createSymbolDatabaseSetVariablePointers();
}

ValueType ValueType::parseDecl(const Token *type, const Settings &settings)
{
    ValueType vt;
    parsedecl(type, &vt, settings.platform.defaultSign == 'u' ? Sign::UNSIGNED : Sign::SIGNED, settings, type->isCpp());
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
    if (typestr == "wchar_t")
        return ValueType::Type::WCHAR_T;
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

bool ValueType::fromLibraryType(const std::string &typestr, const Settings &settings)
{
    const Library::PodType* podtype = settings.library.podtype(typestr);
    if (podtype && (podtype->sign == 's' || podtype->sign == 'u')) {
        if (podtype->size == 1)
            type = ValueType::Type::CHAR;
        else if (podtype->size == settings.platform.sizeof_int)
            type = ValueType::Type::INT;
        else if (podtype->size == settings.platform.sizeof_short)
            type = ValueType::Type::SHORT;
        else if (podtype->size == settings.platform.sizeof_long)
            type = ValueType::Type::LONG;
        else if (podtype->size == settings.platform.sizeof_long_long)
            type = ValueType::Type::LONGLONG;
        else if (podtype->stdtype == Library::PodType::Type::BOOL)
            type = ValueType::Type::BOOL;
        else if (podtype->stdtype == Library::PodType::Type::CHAR)
            type = ValueType::Type::CHAR;
        else if (podtype->stdtype == Library::PodType::Type::SHORT)
            type = ValueType::Type::SHORT;
        else if (podtype->stdtype == Library::PodType::Type::INT)
            type = ValueType::Type::INT;
        else if (podtype->stdtype == Library::PodType::Type::LONG)
            type = ValueType::Type::LONG;
        else if (podtype->stdtype == Library::PodType::Type::LONGLONG)
            type = ValueType::Type::LONGLONG;
        else
            type = ValueType::Type::UNKNOWN_INT;
        sign = (podtype->sign == 'u') ? ValueType::UNSIGNED : ValueType::SIGNED;
        return true;
    }
    if (podtype && podtype->stdtype == Library::PodType::Type::NO) {
        type = ValueType::Type::POD;
        sign = ValueType::UNKNOWN_SIGN;
        return true;
    }

    const Library::PlatformType *platformType = settings.library.platform_type(typestr, settings.platform.toString());
    if (platformType) {
        if (platformType->mType == "char")
            type = ValueType::Type::CHAR;
        else if (platformType->mType == "short")
            type = ValueType::Type::SHORT;
        else if (platformType->mType == "wchar_t")
            type = ValueType::Type::WCHAR_T;
        else if (platformType->mType == "int")
            type = platformType->mLong ? ValueType::Type::LONG : ValueType::Type::INT;
        else if (platformType->mType == "long")
            type = platformType->mLong ? ValueType::Type::LONGLONG : ValueType::Type::LONG;
        if (platformType->mSigned)
            sign = ValueType::SIGNED;
        else if (platformType->mUnsigned)
            sign = ValueType::UNSIGNED;
        if (platformType->mPointer)
            pointer = 1;
        if (platformType->mPtrPtr)
            pointer = 2;
        if (platformType->mConstPtr)
            constness = 1;
        return true;
    }
    if (!podtype && (typestr == "size_t" || typestr == "std::size_t")) {
        originalTypeName = "size_t";
        sign = ValueType::UNSIGNED;
        if (settings.platform.sizeof_size_t == settings.platform.sizeof_long)
            type = ValueType::Type::LONG;
        else if (settings.platform.sizeof_size_t == settings.platform.sizeof_long_long)
            type = ValueType::Type::LONGLONG;
        else if (settings.platform.sizeof_size_t == settings.platform.sizeof_int)
            type = ValueType::Type::INT;
        else
            type = ValueType::Type::UNKNOWN_INT;
        return true;
    }

    return false;
}

std::string ValueType::dump() const
{
    std::string ret;
    switch (type) {
    case UNKNOWN_TYPE:
        return "";
    case NONSTD:
        ret += "valueType-type=\"nonstd\"";
        break;
    case POD:
        ret += "valueType-type=\"pod\"";
        break;
    case RECORD:
        ret += "valueType-type=\"record\"";
        break;
    case SMART_POINTER:
        ret += "valueType-type=\"smart-pointer\"";
        break;
    case CONTAINER: {
        ret += "valueType-type=\"container\"";
        ret += " valueType-containerId=\"";
        ret += id_string(container);
        ret += "\"";
        break;
    }
    case ITERATOR:
        ret += "valueType-type=\"iterator\"";
        break;
    case VOID:
        ret += "valueType-type=\"void\"";
        break;
    case BOOL:
        ret += "valueType-type=\"bool\"";
        break;
    case CHAR:
        ret += "valueType-type=\"char\"";
        break;
    case SHORT:
        ret += "valueType-type=\"short\"";
        break;
    case WCHAR_T:
        ret += "valueType-type=\"wchar_t\"";
        break;
    case INT:
        ret += "valueType-type=\"int\"";
        break;
    case LONG:
        ret += "valueType-type=\"long\"";
        break;
    case LONGLONG:
        ret += "valueType-type=\"long long\"";
        break;
    case UNKNOWN_INT:
        ret += "valueType-type=\"unknown int\"";
        break;
    case FLOAT:
        ret += "valueType-type=\"float\"";
        break;
    case DOUBLE:
        ret += "valueType-type=\"double\"";
        break;
    case LONGDOUBLE:
        ret += "valueType-type=\"long double\"";
        break;
    }

    switch (sign) {
    case Sign::UNKNOWN_SIGN:
        break;
    case Sign::SIGNED:
        ret += " valueType-sign=\"signed\"";
        break;
    case Sign::UNSIGNED:
        ret += " valueType-sign=\"unsigned\"";
        break;
    }

    if (bits > 0) {
        ret += " valueType-bits=\"";
        ret += std::to_string(bits);
        ret += '\"';
    }

    if (pointer > 0) {
        ret += " valueType-pointer=\"";
        ret += std::to_string(pointer);
        ret += '\"';
    }

    if (constness > 0) {
        ret += " valueType-constness=\"";
        ret += std::to_string(constness);
        ret += '\"';
    }

    if (reference == Reference::None)
        ret += " valueType-reference=\"None\"";
    else if (reference == Reference::LValue)
        ret += " valueType-reference=\"LValue\"";
    else if (reference == Reference::RValue)
        ret += " valueType-reference=\"RValue\"";

    if (typeScope) {
        ret += " valueType-typeScope=\"";
        ret += id_string(typeScope);
        ret += '\"';
    }

    if (!originalTypeName.empty()) {
        ret += " valueType-originalTypeName=\"";
        ret += ErrorLogger::toxml(originalTypeName);
        ret += '\"';
    }

    return ret;
}

bool ValueType::isConst(nonneg int indirect) const
{
    if (indirect > pointer)
        return false;
    return constness & (1 << (pointer - indirect));
}

MathLib::bigint ValueType::typeSize(const Platform &platform, bool p) const
{
    if (p && pointer)
        return platform.sizeof_pointer;

    if (typeScope && typeScope->definedType && typeScope->definedType->sizeOf)
        return typeScope->definedType->sizeOf;

    switch (type) {
    case ValueType::Type::BOOL:
        return platform.sizeof_bool;
    case ValueType::Type::CHAR:
        return 1;
    case ValueType::Type::SHORT:
        return platform.sizeof_short;
    case ValueType::Type::WCHAR_T:
        return platform.sizeof_wchar_t;
    case ValueType::Type::INT:
        return platform.sizeof_int;
    case ValueType::Type::LONG:
        return platform.sizeof_long;
    case ValueType::Type::LONGLONG:
        return platform.sizeof_long_long;
    case ValueType::Type::FLOAT:
        return platform.sizeof_float;
    case ValueType::Type::DOUBLE:
        return platform.sizeof_double;
    case ValueType::Type::LONGDOUBLE:
        return platform.sizeof_long_double;
    default:
        break;
    }

    // Unknown invalid size
    return 0;
}

bool ValueType::isTypeEqual(const ValueType* that) const
{
    if (!that)
        return false;
    auto tie = [](const ValueType* vt) {
        return std::tie(vt->type, vt->container, vt->pointer, vt->typeScope, vt->smartPointer);
    };
    return tie(this) == tie(that);
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
        else if (type == WCHAR_T)
            ret += " wchar_t";
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
        const Scope *scope = typeScope->definedType ? typeScope->definedType->enclosingScope : typeScope->nestedIn;
        while (scope && scope->type != Scope::eGlobal) {
            if (scope->type == Scope::eClass || scope->type == Scope::eStruct || scope->type == Scope::eNamespace)
                className = scope->className + "::" + className;
            scope = (scope->definedType && scope->definedType->enclosingScope) ? scope->definedType->enclosingScope : scope->nestedIn;
        }
        ret += ' ' + className;
    } else if (type == ValueType::Type::CONTAINER && container) {
        ret += " container(" + container->startPattern + ')';
    } else if (type == ValueType::Type::ITERATOR && container) {
        ret += " iterator(" + container->startPattern + ')';
    } else if (type == ValueType::Type::SMART_POINTER && smartPointer) {
        ret += " smart-pointer(" + smartPointer->name + ")";
    }
    for (unsigned int p = 0; p < pointer; p++) {
        ret += " *";
        if (constness & (2 << p))
            ret += " const";
    }
    if (reference == Reference::LValue)
        ret += " &";
    else if (reference == Reference::RValue)
        ret += " &&";
    return ret.empty() ? ret : ret.substr(1);
}

void ValueType::setDebugPath(const Token* tok, SourceLocation ctx, SourceLocation local)
{
    std::string file = ctx.file_name();
    if (file.empty())
        return;
    std::string s = Path::stripDirectoryPart(file) + ":" + std::to_string(ctx.line()) + ": " + ctx.function_name() +
                    " => " + local.function_name();
    debugPath.emplace_back(tok, std::move(s));
}

ValueType::MatchResult ValueType::matchParameter(const ValueType *call, const ValueType *func)
{
    if (!call || !func)
        return ValueType::MatchResult::UNKNOWN;
    if (call->pointer != func->pointer) {
        if (call->pointer > 1 && func->pointer == 1 && func->type == ValueType::Type::VOID)
            return ValueType::MatchResult::FALLBACK1;
        if (call->pointer == 1 && func->pointer == 0 && func->isIntegral() && func->sign != ValueType::Sign::SIGNED)
            return ValueType::MatchResult::FALLBACK1;
        if (call->pointer == 1 && call->type == ValueType::Type::CHAR && func->pointer == 0 && func->container && func->container->stdStringLike)
            return ValueType::MatchResult::FALLBACK2;
        return ValueType::MatchResult::NOMATCH; // TODO
    }
    if (call->pointer > 0) {
        if ((call->constness | func->constness) != func->constness)
            return ValueType::MatchResult::NOMATCH;
        if (call->constness == 0 && func->constness != 0 && func->reference != Reference::None)
            return ValueType::MatchResult::NOMATCH;
    }
    if (call->type != func->type || (call->isEnum() && !func->isEnum())) {
        if (call->type == ValueType::Type::VOID || func->type == ValueType::Type::VOID)
            return ValueType::MatchResult::FALLBACK1;
        if (call->pointer > 0)
            return func->type == ValueType::UNKNOWN_TYPE ? ValueType::MatchResult::UNKNOWN : ValueType::MatchResult::NOMATCH;
        if (call->isIntegral() && func->isIntegral())
            return call->type < func->type ?
                   ValueType::MatchResult::FALLBACK1 :
                   ValueType::MatchResult::FALLBACK2;
        if (call->isFloat() && func->isFloat())
            return ValueType::MatchResult::FALLBACK1;
        if (call->isIntegral() && func->isFloat())
            return ValueType::MatchResult::FALLBACK2;
        if (call->isFloat() && func->isIntegral())
            return ValueType::MatchResult::FALLBACK2;
        return ValueType::MatchResult::UNKNOWN; // TODO
    }

    if (call->typeScope != nullptr || func->typeScope != nullptr) {
        if (call->typeScope != func->typeScope &&
            !(call->typeScope && func->typeScope && call->typeScope->definedType && call->typeScope->definedType->isDerivedFrom(func->typeScope->className)))
            return ValueType::MatchResult::NOMATCH;
    }

    if (call->container != nullptr || func->container != nullptr) {
        if (call->container != func->container)
            return ValueType::MatchResult::NOMATCH;
    }

    if (func->typeScope != nullptr && func->container != nullptr) {
        if (func->type < ValueType::Type::VOID || func->type == ValueType::Type::UNKNOWN_INT)
            return ValueType::MatchResult::UNKNOWN;
    }

    if (call->isIntegral() && func->isIntegral() && call->sign != ValueType::Sign::UNKNOWN_SIGN && func->sign != ValueType::Sign::UNKNOWN_SIGN && call->sign != func->sign)
        return ValueType::MatchResult::FALLBACK1;

    if (func->reference != Reference::None && func->constness > call->constness)
        return ValueType::MatchResult::FALLBACK1;

    return ValueType::MatchResult::SAME;
}

ValueType::MatchResult ValueType::matchParameter(const ValueType *call, const Variable *callVar, const Variable *funcVar)
{
    ValueType vt;
    const ValueType* pvt = funcVar->valueType();
    if (pvt && funcVar->isArray() && !(funcVar->isStlType() && Token::simpleMatch(funcVar->typeStartToken(), "std :: array"))) { // std::array doesn't decay to a pointer
        vt = *pvt;
        if (vt.pointer == 0) // don't bump array of pointers
            ++vt.pointer;
        pvt = &vt;
    }
    const ValueType::MatchResult res = ValueType::matchParameter(call, pvt);
    if (callVar && ((res == ValueType::MatchResult::SAME && call->container) || res == ValueType::MatchResult::UNKNOWN)) {
        const std::string type1 = getTypeString(callVar->typeStartToken());
        const std::string type2 = getTypeString(funcVar->typeStartToken());
        const bool templateVar =
            funcVar->scope() && funcVar->scope()->function && funcVar->scope()->function->templateDef;
        if (type1 == type2)
            return ValueType::MatchResult::SAME;
        if (!templateVar && type1.find("auto") == std::string::npos && type2.find("auto") == std::string::npos)
            return ValueType::MatchResult::NOMATCH;
    }
    return res;
}
