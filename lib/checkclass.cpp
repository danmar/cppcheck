/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "symboldatabase.h"

#include <string>
#include <algorithm>
#include <cctype>

//---------------------------------------------------------------------------

// Register CheckClass..
namespace {
    CheckClass instance;

    const char * getFunctionTypeName(
        Function::Type type)
    {
        switch (type) {
        case Function::eConstructor:
            return "constructor";
        case Function::eCopyConstructor:
            return "copy constructor";
        case Function::eMoveConstructor:
            return "move constructor";
        case Function::eDestructor:
            return "destructor";
        case Function::eFunction:
            return "function";
        case Function::eOperatorEqual:
            return "operator=";
        }
        return "";
    }

    inline bool isPureWithoutBody(Function const & func)
    {
        return func.isPure() && !func.hasBody();
    }
}

//---------------------------------------------------------------------------

CheckClass::CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : Check(myName(), tokenizer, settings, errorLogger),
      symbolDatabase(tokenizer?tokenizer->getSymbolDatabase():nullptr)
{

}

//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    const bool printStyle = _settings->isEnabled("style");
    const bool printWarnings = _settings->isEnabled("warning");
    if (!printStyle && !printWarnings)
        return;

    const bool printInconclusive = _settings->inconclusive;
    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];

        // There are no constructors.
        if (scope->numConstructors == 0 && printStyle) {
            // If there is a private variable, there should be a constructor..
            std::list<Variable>::const_iterator var;
            for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
                if (var->isPrivate() && !var->isStatic() && !Token::Match(var->nameToken(), "%varid% ; %varid% =", var->declarationId()) &&
                    (!var->isClass() || (var->type() && var->type()->needInitialization == Type::True))) {
                    noConstructorError(scope->classDef, scope->className, scope->classDef->str() == "struct");
                    break;
                }
            }
        }

        if (!printWarnings)
            continue;

        // #3196 => bailout if there are nested unions
        // TODO: handle union variables better
        {
            bool bailout = false;
            for (std::list<Scope *>::const_iterator it = scope->nestedList.begin(); it != scope->nestedList.end(); ++it) {
                const Scope * const nestedScope = *it;
                if (nestedScope->type == Scope::eUnion) {
                    bailout = true;
                    break;
                }
            }
            if (bailout)
                continue;
        }


        std::list<Function>::const_iterator func;
        std::vector<Usage> usage(scope->varlist.size());

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (!func->hasBody() || !(func->isConstructor() ||
                                      func->type == Function::eOperatorEqual))
                continue;

            // Mark all variables not used
            clearAllVar(usage);

            std::list<const Function *> callstack;
            initializeVarList(*func, callstack, scope, usage);

            // Check if any variables are uninitialized
            std::list<Variable>::const_iterator var;
            unsigned int count = 0;
            for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count) {
                // check for C++11 initializer
                if (var->hasDefault()) {
                    usage[count].init = true;
                    continue;
                }

                if (usage[count].assign || usage[count].init || var->isStatic())
                    continue;

                if (var->isConst() && func->isOperator()) // We can't set const members in assignment operator
                    continue;

                // Check if this is a class constructor
                if (!var->isPointer() && var->isClass() && func->type == Function::eConstructor) {
                    // Unknown type so assume it is initialized
                    if (!var->type())
                        continue;

                    // Known type that doesn't need initialization or
                    // known type that has member variables of an unknown type
                    else if (var->type()->needInitialization != Type::True)
                        continue;
                }

                // Check if type can't be copied
                if (!var->isPointer() && var->typeScope()) {
                    if (func->type == Function::eMoveConstructor) {
                        if (canNotMove(var->typeScope()))
                            continue;
                    } else {
                        if (canNotCopy(var->typeScope()))
                            continue;
                    }
                }

                bool inconclusive = false;
                // Don't warn about unknown types in copy constructors since we
                // don't know if they can be copied or not..
                if (!var->isPointer() &&
                    !(var->type() && var->type()->needInitialization != Type::True) &&
                    (func->type == Function::eCopyConstructor || func->type == Function::eOperatorEqual)) {
                    if (!var->typeStartToken()->isStandardType()) {
                        if (printInconclusive)
                            inconclusive = true;
                        else
                            continue;
                    }
                }

                // It's non-static and it's not initialized => error
                if (func->type == Function::eOperatorEqual) {
                    const Token *operStart = func->arg;

                    bool classNameUsed = false;
                    for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next()) {
                        if (operTok->str() == scope->className) {
                            classNameUsed = true;
                            break;
                        }
                    }

                    if (classNameUsed)
                        operatorEqVarError(func->token, scope->className, var->name(), inconclusive);
                } else if (func->access != Private) {
                    const Scope *varType = var->typeScope();
                    if (!varType || varType->type != Scope::eUnion) {
                        if (func->type == Function::eConstructor &&
                            func->nestedIn && (func->nestedIn->numConstructors - func->nestedIn->numCopyOrMoveConstructors) > 1 &&
                            func->argCount() == 0 && func->functionScope &&
                            func->arg && func->arg->link()->next() == func->functionScope->classStart &&
                            func->functionScope->classStart->link() == func->functionScope->classStart->next()) {
                            // don't warn about user defined default constructor when there are other constructors
                            if (printInconclusive)
                                uninitVarError(func->token, scope->className, var->name(), true);
                        } else
                            uninitVarError(func->token, scope->className, var->name(), inconclusive);
                    }
                }
            }
        }
    }
}

void CheckClass::checkExplicitConstructors()
{
    if (!_settings->isEnabled("style"))
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];

        // Do not perform check, if the class/struct has not any constructors
        if (scope->numConstructors == 0)
            continue;

        // Is class abstract? Maybe this test is over-simplification, but it will suffice for simple cases,
        // and it will avoid false positives.
        bool isAbstractClass = false;
        for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->isPure()) {
                isAbstractClass = true;
                break;
            }
        }

        for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {

            // We are looking for constructors, which are meeting following criteria:
            //  1) Constructor is declared with a single parameter
            //  2) Constructor is not declared as explicit
            //  3) It is not a copy/move constructor of non-abstract class
            //  4) Constructor is not marked as delete (programmer can mark the default constructor as deleted, which is ok)
            if (!func->isConstructor() || func->isDelete())
                continue;

            if (!func->isExplicit() && func->argCount() == 1) {
                // We must decide, if it is not a copy/move constructor, or it is a copy/move constructor of abstract class.
                if (func->type != Function::eCopyConstructor && func->type != Function::eMoveConstructor) {
                    noExplicitConstructorError(func->tokenDef, scope->className, scope->type == Scope::eStruct);
                } else if (isAbstractClass) {
                    noExplicitCopyMoveConstructorError(func->tokenDef, scope->className, scope->type == Scope::eStruct);
                }
            }
        }
    }
}

void CheckClass::copyconstructors()
{
    if (!_settings->isEnabled("style"))
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];
        std::map<unsigned int, const Token*> allocatedVars;

        for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->type == Function::eConstructor && func->functionScope) {
                const Token* tok = func->functionScope->classDef->linkAt(1);
                for (const Token* const end = func->functionScope->classStart; tok != end; tok = tok->next()) {
                    if (Token::Match(tok, "%var% ( new|malloc|g_malloc|g_try_malloc|realloc|g_realloc|g_try_realloc")) {
                        const Variable* var = tok->variable();
                        if (var && var->isPointer() && var->scope() == scope)
                            allocatedVars[tok->varId()] = tok;
                    }
                }
                for (const Token* const end = func->functionScope->classEnd; tok != end; tok = tok->next()) {
                    if (Token::Match(tok, "%var% = new|malloc|g_malloc|g_try_malloc|realloc|g_realloc|g_try_realloc")) {
                        const Variable* var = tok->variable();
                        if (var && var->isPointer() && var->scope() == scope)
                            allocatedVars[tok->varId()] = tok;
                    }
                }
            }
        }

        std::set<const Token*> copiedVars;
        const Token* copyCtor = 0;
        for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->type == Function::eCopyConstructor) {
                copyCtor = func->tokenDef;
                if (func->functionScope) {
                    const Token* tok = func->tokenDef->linkAt(1)->next();
                    if (tok->str()==":") {
                        tok=tok->next();
                        while (Token::Match(tok, "%name% (")) {
                            if (allocatedVars.find(tok->varId()) != allocatedVars.end()) {
                                if (tok->varId() && Token::Match(tok->tokAt(2), "%name% . %name% )"))
                                    copiedVars.insert(tok);
                                else if (!Token::Match(tok->tokAt(2), "%any% )"))
                                    allocatedVars.erase(tok->varId()); // Assume memory is allocated
                            }
                            tok = tok->linkAt(1)->tokAt(2);
                        }
                    }
                    for (tok=func->functionScope->classStart; tok!=func->functionScope->classEnd; tok=tok->next()) {
                        if (Token::Match(tok, "%var% = new|malloc|g_malloc|g_try_malloc|realloc|g_realloc|g_try_realloc")) {
                            allocatedVars.erase(tok->varId());
                        } else if (Token::Match(tok, "%var% = %name% . %name% ;") && allocatedVars.find(tok->varId()) != allocatedVars.end()) {
                            copiedVars.insert(tok);
                        }
                    }
                } else // non-copyable or implementation not seen
                    allocatedVars.clear();
                break;
            }
        }
        if (!copyCtor) {
            if (!allocatedVars.empty() && scope->definedType->derivedFrom.empty()) // TODO: Check if base class is non-copyable
                noCopyConstructorError(scope->classDef, scope->className, scope->type == Scope::eStruct);
        } else {
            if (!copiedVars.empty()) {
                for (std::set<const Token*>::const_iterator it = copiedVars.begin(); it != copiedVars.end(); ++it) {
                    copyConstructorShallowCopyError(*it, (*it)->str());
                }
            }
            // throw error if count mismatch
            /* FIXME: This doesn't work. See #4154
            for (std::map<unsigned int, const Token*>::const_iterator i = allocatedVars.begin(); i != allocatedVars.end(); ++i) {
                copyConstructorMallocError(copyCtor, i->second, i->second->str());
            }
            */
        }
    }
}

/* This doesn't work. See #4154
void CheckClass::copyConstructorMallocError(const Token *cctor, const Token *alloc, const std::string& varname)
{
    std::list<const Token*> callstack;
    callstack.push_back(cctor);
    callstack.push_back(alloc);
    reportError(callstack, Severity::warning, "copyCtorNoAllocation", "Copy constructor does not allocate memory for member '" + varname + "' although memory has been allocated in other constructors.");
}
*/

void CheckClass::copyConstructorShallowCopyError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::style, "copyCtorPointerCopying", "Value of pointer '" + varname + "', which points to allocated memory, is copied in copy constructor instead of allocating new memory.");
}

void CheckClass::noCopyConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    // The constructor might be intentionally missing. Therefore this is not a "warning"
    reportError(tok, Severity::style, "noCopyConstructor",
                "'" + std::string(isStruct ? "struct" : "class") + " " + classname +
                "' does not have a copy constructor which is recommended since the class contains a pointer to allocated memory.");
}

bool CheckClass::canNotCopy(const Scope *scope)
{
    std::list<Function>::const_iterator func;
    bool constructor = false;
    bool publicAssign = false;
    bool publicCopy = false;

    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        if (func->isConstructor())
            constructor = true;
        if ((func->type == Function::eCopyConstructor) &&
            func->access == Public)
            publicCopy = true;
        else if (func->type == Function::eOperatorEqual && func->access == Public)
            publicAssign = true;
    }

    return constructor && !(publicAssign || publicCopy);
}

bool CheckClass::canNotMove(const Scope *scope)
{
    std::list<Function>::const_iterator func;
    bool constructor = false;
    bool publicAssign = false;
    bool publicCopy = false;
    bool publicMove = false;

    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        if (func->isConstructor())
            constructor = true;
        if ((func->type == Function::eCopyConstructor) &&
            func->access == Public)
            publicCopy = true;
        else if ((func->type == Function::eMoveConstructor) &&
                 func->access == Public)
            publicMove = true;
        else if (func->type == Function::eOperatorEqual && func->access == Public)
            publicAssign = true;
    }

    return constructor && !(publicAssign || publicCopy || publicMove);
}

void CheckClass::assignVar(const std::string &varname, const Scope *scope, std::vector<Usage> &usage)
{
    std::list<Variable>::const_iterator var;
    unsigned int count = 0;

    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count) {
        if (var->name() == varname) {
            usage[count].assign = true;
            return;
        }
    }
}

void CheckClass::initVar(const std::string &varname, const Scope *scope, std::vector<Usage> &usage)
{
    std::list<Variable>::const_iterator var;
    unsigned int count = 0;

    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count) {
        if (var->name() == varname) {
            usage[count].init = true;
            return;
        }
    }
}

void CheckClass::assignAllVar(std::vector<Usage> &usage)
{
    for (std::size_t i = 0; i < usage.size(); ++i)
        usage[i].assign = true;
}

void CheckClass::clearAllVar(std::vector<Usage> &usage)
{
    for (std::size_t i = 0; i < usage.size(); ++i) {
        usage[i].assign = false;
        usage[i].init = false;
    }
}

bool CheckClass::isBaseClassFunc(const Token *tok, const Scope *scope)
{
    // Iterate through each base class...
    for (std::size_t i = 0; i < scope->definedType->derivedFrom.size(); ++i) {
        const Type *derivedFrom = scope->definedType->derivedFrom[i].type;

        // Check if base class exists in database
        if (derivedFrom && derivedFrom->classScope) {
            const std::list<Function>& functionList = derivedFrom->classScope->functionList;
            std::list<Function>::const_iterator func;

            for (func = functionList.begin(); func != functionList.end(); ++func) {
                if (func->tokenDef->str() == tok->str())
                    return true;
            }
        }

        // Base class not found so assume it is in it.
        else
            return true;
    }

    return false;
}

void CheckClass::initializeVarList(const Function &func, std::list<const Function *> &callstack, const Scope *scope, std::vector<Usage> &usage)
{
    if (!func.functionScope)
        throw InternalError(0, "Internal Error: Invalid syntax"); // #5702
    bool initList = func.isConstructor();
    const Token *ftok = func.arg->link()->next();
    int level = 0;
    for (; ftok && ftok != func.functionScope->classEnd; ftok = ftok->next()) {
        // Class constructor.. initializing variables like this
        // clKalle::clKalle() : var(value) { }
        if (initList) {
            if (level == 0 && Token::Match(ftok, "%name% {|(") && Token::Match(ftok->linkAt(1), "}|) ,|{")) {
                if (ftok->str() != func.name()) {
                    initVar(ftok->str(), scope, usage);
                } else { // c++11 delegate constructor
                    const Function *member = ftok->function();
                    // member function found
                    if (member) {
                        // recursive call
                        // assume that all variables are initialized
                        if (std::find(callstack.begin(), callstack.end(), member) != callstack.end()) {
                            /** @todo false negative: just bail */
                            assignAllVar(usage);
                            return;
                        }

                        // member function has implementation
                        if (member->hasBody()) {
                            // initialize variable use list using member function
                            callstack.push_back(member);
                            initializeVarList(*member, callstack, scope, usage);
                            callstack.pop_back();
                        }

                        // there is a called member function, but it has no implementation, so we assume it initializes everything
                        else {
                            assignAllVar(usage);
                        }
                    }
                }
            } else if (level != 0 && Token::Match(ftok, "%name% =")) // assignment in the initializer: var(value = x)
                assignVar(ftok->str(), scope, usage);

            // Level handling
            if (ftok->link() && Token::Match(ftok, "(|<"))
                level++;
            else if (ftok->str() == "{") {
                if (level != 0 ||
                    (Token::Match(ftok->previous(), "%name%|>") && Token::Match(ftok->link(), "} ,|{")))
                    level++;
                else
                    initList = false;
            } else if (ftok->link() && Token::Match(ftok, ")|>|}"))
                level--;
        }

        if (initList)
            continue;

        // Variable getting value from stream?
        if (Token::Match(ftok, ">> %name%")) {
            assignVar(ftok->strAt(1), scope, usage);
        }

        // Before a new statement there is "[{};()=[]"
        if (! Token::Match(ftok, "[{};()=[]"))
            continue;

        if (Token::simpleMatch(ftok, "( !"))
            ftok = ftok->next();

        // Using the operator= function to initialize all variables..
        if (Token::Match(ftok->next(), "return| (| * this )| =")) {
            assignAllVar(usage);
            break;
        }

        // Using swap to assign all variables..
        if (func.type == Function::eOperatorEqual && Token::Match(ftok, "[;{}] %name% (") && Token::Match(ftok->linkAt(2), ") . %name% ( *| this ) ;")) {
            assignAllVar(usage);
            break;
        }

        // Calling member variable function?
        if (Token::Match(ftok->next(), "%var% . %name% (")) {
            std::list<Variable>::const_iterator var;
            for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
                if (var->declarationId() == ftok->next()->varId()) {
                    /** @todo false negative: we assume function changes variable state */
                    assignVar(ftok->next()->str(), scope, usage);
                    break;
                }
            }

            ftok = ftok->tokAt(2);
        }

        if (!Token::Match(ftok->next(), "::| %name%") &&
            !Token::Match(ftok->next(), "*| this . %name%") &&
            !Token::Match(ftok->next(), "* %name% =") &&
            !Token::Match(ftok->next(), "( * this ) . %name%"))
            continue;

        // Goto the first token in this statement..
        ftok = ftok->next();

        // skip "return"
        if (ftok->str() == "return")
            ftok = ftok->next();

        // Skip "( * this )"
        if (Token::simpleMatch(ftok, "( * this ) .")) {
            ftok = ftok->tokAt(5);
        }

        // Skip "this->"
        if (Token::simpleMatch(ftok, "this ."))
            ftok = ftok->tokAt(2);

        // Skip "classname :: "
        if (Token::Match(ftok, ":: %name%"))
            ftok = ftok->next();
        while (Token::Match(ftok, "%name% ::"))
            ftok = ftok->tokAt(2);

        // Clearing all variables..
        if (Token::Match(ftok, "::| memset ( this ,")) {
            assignAllVar(usage);
            return;
        }

        // Clearing array..
        else if (Token::Match(ftok, "::| memset ( %name% ,")) {
            if (ftok->str() == "::")
                ftok = ftok->next();
            assignVar(ftok->strAt(2), scope, usage);
            ftok = ftok->linkAt(1);
            continue;
        }

        // Calling member function?
        else if (Token::simpleMatch(ftok, "operator= (") &&
                 ftok->previous()->str() != "::") {
            if (ftok->function() && ftok->function()->nestedIn == scope) {
                const Function *member = ftok->function();
                // recursive call
                // assume that all variables are initialized
                if (std::find(callstack.begin(), callstack.end(), member) != callstack.end()) {
                    /** @todo false negative: just bail */
                    assignAllVar(usage);
                    return;
                }

                // member function has implementation
                if (member->hasBody()) {
                    // initialize variable use list using member function
                    callstack.push_back(member);
                    initializeVarList(*member, callstack, scope, usage);
                    callstack.pop_back();
                }

                // there is a called member function, but it has no implementation, so we assume it initializes everything
                else {
                    assignAllVar(usage);
                }
            }

            // using default operator =, assume everything initialized
            else {
                assignAllVar(usage);
            }
        } else if (Token::Match(ftok, "::| %name% (") && ftok->str() != "if") {
            if (ftok->str() == "::")
                ftok = ftok->next();

            // Passing "this" => assume that everything is initialized
            for (const Token *tok2 = ftok->next()->link(); tok2 && tok2 != ftok; tok2 = tok2->previous()) {
                if (tok2->str() == "this") {
                    assignAllVar(usage);
                    return;
                }
            }

            // check if member function
            if (ftok->function() && ftok->function()->nestedIn == scope &&
                !ftok->function()->isConstructor()) {
                const Function *member = ftok->function();

                // recursive call
                // assume that all variables are initialized
                if (std::find(callstack.begin(), callstack.end(), member) != callstack.end()) {
                    assignAllVar(usage);
                    return;
                }

                // member function has implementation
                if (member->hasBody()) {
                    // initialize variable use list using member function
                    callstack.push_back(member);
                    initializeVarList(*member, callstack, scope, usage);
                    callstack.pop_back();

                    // Assume that variables that are passed to it are initialized..
                    for (const Token *tok2 = ftok; tok2; tok2 = tok2->next()) {
                        if (Token::Match(tok2, "[;{}]"))
                            break;
                        if (Token::Match(tok2, "[(,] &| %name% [,)]")) {
                            tok2 = tok2->next();
                            if (tok2->str() == "&")
                                tok2 = tok2->next();
                            assignVar(tok2->str(), scope, usage);
                        }
                    }
                }

                // there is a called member function, but it has no implementation, so we assume it initializes everything
                else {
                    assignAllVar(usage);
                }
            }

            // not member function
            else {
                // could be a base class virtual function, so we assume it initializes everything
                if (!func.isConstructor() && isBaseClassFunc(ftok, scope)) {
                    /** @todo False Negative: we should look at the base class functions to see if they
                     *  call any derived class virtual functions that change the derived class state
                     */
                    assignAllVar(usage);
                }

                // has friends, so we assume it initializes everything
                if (!scope->definedType->friendList.empty())
                    assignAllVar(usage);

                // the function is external and it's neither friend nor inherited virtual function.
                // assume all variables that are passed to it are initialized..
                else {
                    for (const Token *tok = ftok->tokAt(2); tok && tok != ftok->next()->link(); tok = tok->next()) {
                        if (tok->isName()) {
                            assignVar(tok->str(), scope, usage);
                        }
                    }
                }
            }
        }

        // Assignment of member variable?
        else if (Token::Match(ftok, "%name% =")) {
            assignVar(ftok->str(), scope, usage);
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%name% [|.")) {
            const Token *tok2 = ftok;
            while (tok2) {
                if (tok2->strAt(1) == "[")
                    tok2 = tok2->next()->link();
                else if (Token::Match(tok2->next(), ". %name%"))
                    tok2 = tok2->tokAt(2);
                else
                    break;
            }
            if (tok2 && tok2->strAt(1) == "=")
                assignVar(ftok->str(), scope, usage);
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "* %name% =")) {
            assignVar(ftok->next()->str(), scope, usage);
        } else if (Token::Match(ftok, "* this . %name% =")) {
            assignVar(ftok->strAt(3), scope, usage);
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Token::Match(ftok, "%name% . clear|Clear (")) {
            assignVar(ftok->str(), scope, usage);
        }
    }
}

void CheckClass::noConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    // For performance reasons the constructor might be intentionally missing. Therefore this is not a "warning"
    reportError(tok, Severity::style, "noConstructor",
                "The " + std::string(isStruct ? "struct" : "class") + " '" + classname +
                "' does not have a constructor.\n"
                "The " + std::string(isStruct ? "struct" : "class") + " '" + classname +
                "' does not have a constructor although it has private member variables. "
                "Member variables of builtin types are left uninitialized when the class is "
                "instantiated. That may cause bugs or undefined behavior.");
}

void CheckClass::noExplicitConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    const std::string message(std::string(isStruct ? "Struct" : "Class") + " '" + classname + "' has a constructor with 1 argument that is not explicit.");
    const std::string verbose(message + " Such constructors should in general be explicit for type safety reasons. Using the explicit keyword in the constructor means some mistakes when using the class can be avoided.");
    reportError(tok, Severity::style, "noExplicitConstructor", message + "\n" + verbose);
}

void CheckClass::noExplicitCopyMoveConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    const std::string message(std::string(isStruct ? "Abstract struct" : "Abstract class") + " '" + classname + "' has a copy/move constructor that is not explicit.");
    const std::string verbose(message + " For abstract classes, even copy/move constructors may be declared explicit, as, by definition, abstract classes cannot be instantiated, and so objects of such type should never be passed by value.");
    reportError(tok, Severity::style, "noExplicitCopyMoveConstructor", message + "\n" + verbose);
}

void CheckClass::uninitVarError(const Token *tok, const std::string &classname, const std::string &varname, bool inconclusive)
{
    reportError(tok, Severity::warning, "uninitMemberVar", "Member variable '" + classname + "::" + varname + "' is not initialized in the constructor.", 0U, inconclusive);
}

void CheckClass::operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname, bool inconclusive)
{
    reportError(tok, Severity::warning, "operatorEqVarError", "Member variable '" + classname + "::" + varname + "' is not assigned a value in '" + classname + "::operator='.", 0U, inconclusive);
}

//---------------------------------------------------------------------------
// ClassCheck: Use initialization list instead of assignment
//---------------------------------------------------------------------------

void CheckClass::initializationListUsage()
{
    if (!_settings->isEnabled("performance"))
        return;

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        // Check every constructor
        if (!scope->function || (!scope->function->isConstructor()))
            continue;

        const Scope* owner = scope->functionOf;
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%name% (")) // Assignments might depend on this function call or if/for/while/switch statement from now on.
                break;
            if (Token::Match(tok, "try|do {"))
                break;
            if (Token::Match(tok, "%var% = %any%")) {
                const Variable* var = tok->variable();
                if (var && var->scope() == owner && !var->isStatic()) {
                    bool allowed = true;
                    for (const Token* tok2 = tok->tokAt(2); tok2->str() != ";"; tok2 = tok2->next()) {
                        if (tok2->varId()) {
                            const Variable* var2 = tok2->variable();
                            if (var2 && var2->scope() == owner &&
                                tok2->strAt(-1)!=".") { // Is there a dependency between two member variables?
                                allowed = false;
                                break;
                            } else if (var2 && (var2->isArray() && var2->isLocal())) { // Can't initialize with a local array
                                allowed = false;
                                break;
                            }
                        } else if (tok2->str() == "this") { // 'this' instance is not completely constructed in initialization list
                            allowed = false;
                            break;
                        } else if (Token::Match(tok2, "%name% (") && tok2->strAt(-1) != "." && isMemberFunc(owner, tok2)) { // Member function called?
                            allowed = false;
                            break;
                        }
                    }
                    if (!allowed)
                        continue;
                    if (!var->isPointer() && !var->isReference() && (var->type() || var->isStlStringType() || (Token::Match(var->typeStartToken(), "std :: %type% <") && !Token::simpleMatch(var->typeStartToken()->linkAt(3), "> ::"))))
                        suggestInitializationList(tok, tok->str());
                }
            }
        }
    }
}

void CheckClass::suggestInitializationList(const Token* tok, const std::string& varname)
{
    reportError(tok, Severity::performance, "useInitializationList", "Variable '" + varname + "' is assigned in constructor body. Consider performing initialization in initialization list.\n"
                "When an object of a class is created, the constructors of all member variables are called consecutively "
                "in the order the variables are declared, even if you don't explicitly write them to the initialization list. You "
                "could avoid assigning '" + varname + "' a value by passing the value to the constructor in the initialization list.");
}

//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

static bool checkFunctionUsage(const std::string& name, const Scope* scope)
{
    if (!scope)
        return true; // Assume it is used, if scope is not seen

    for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        if (func->functionScope) {
            if (Token::Match(func->tokenDef, "%name% (")) {
                for (const Token *ftok = func->tokenDef->tokAt(2); ftok && ftok->str() != ")"; ftok = ftok->next()) {
                    if (Token::Match(ftok, "= %name% [(,)]") && ftok->strAt(1) == name)
                        return true;
                    if (ftok->str() == "(")
                        ftok = ftok->link();
                }
            }
            for (const Token *ftok = func->functionScope->classDef->linkAt(1); ftok != func->functionScope->classEnd; ftok = ftok->next()) {
                if (ftok->str() == name) // Function used. TODO: Handle overloads
                    return true;
            }
        } else if ((func->type != Function::eCopyConstructor &&
                    func->type != Function::eOperatorEqual) ||
                   func->access != Private) // Assume it is used, if a function implementation isn't seen, but empty private copy constructors and assignment operators are OK
            return true;
    }

    for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
        if ((*i)->isClassOrStruct())
            if (checkFunctionUsage(name, *i)) // Check nested classes, which can access private functions of their base
                return true;
    }

    for (std::list<Variable>::const_iterator i = scope->varlist.begin(); i != scope->varlist.end(); ++i) {
        if (i->isStatic()) {
            const Token* tok = Token::findmatch(scope->classEnd, "%varid% =|(|{", i->declarationId());
            if (tok)
                tok = tok->tokAt(2);
            while (tok && tok->str() != ";") {
                if (tok->str() == name && (tok->strAt(-1) == "." || tok->strAt(-2) == scope->className))
                    return true;
                tok = tok->next();
            }
        }
    }

    return false; // Unused in this scope
}

void CheckClass::privateFunctions()
{
    if (!_settings->isEnabled("style"))
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];

        // do not check borland classes with properties..
        if (Token::findsimplematch(scope->classStart, "; __property ;", scope->classEnd))
            continue;

        std::list<const Function*> privateFuncs;
        for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            // Get private functions..
            if (func->type == Function::eFunction && func->access == Private && !func->isOperator()) // TODO: There are smarter ways to check private operator usage
                privateFuncs.push_back(&*func);
        }

        // Bailout for overridden virtual functions of base classes
        if (!scope->definedType->derivedFrom.empty()) {
            // Check virtual functions
            for (std::list<const Function*>::iterator it = privateFuncs.begin(); it != privateFuncs.end();) {
                if ((*it)->isImplicitlyVirtual(true)) // Give true as default value to be returned if we don't see all base classes
                    privateFuncs.erase(it++);
                else
                    ++it;
            }
        }

        while (!privateFuncs.empty()) {
            const std::string& funcName = privateFuncs.front()->tokenDef->str();
            // Check that all private functions are used
            bool used = checkFunctionUsage(funcName, scope); // Usage in this class
            // Check in friend classes
            const std::list<Type::FriendInfo>& friendList = scope->definedType->friendList;
            for (std::list<Type::FriendInfo>::const_iterator it = friendList.begin(); !used && it != friendList.end(); ++it) {
                if (it->type)
                    used = checkFunctionUsage(funcName, it->type->classScope);
                else
                    used = true; // Assume, it is used if we do not see friend class
            }

            if (!used)
                unusedPrivateFunctionError(privateFuncs.front()->tokenDef, scope->className, funcName);

            privateFuncs.pop_front();
        }
    }
}

void CheckClass::unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "unusedPrivateFunction", "Unused private function: '" + classname + "::" + funcname + "'");
}

//---------------------------------------------------------------------------
// ClassCheck: Check that memset is not used on classes
//---------------------------------------------------------------------------

static const Scope* findFunctionOf(const Scope* scope)
{
    while (scope) {
        if (scope->type == Scope::eFunction)
            return scope->functionOf;
        scope = scope->nestedIn;
    }
    return 0;
}

void CheckClass::checkMemset()
{
    const bool printWarnings = _settings->isEnabled("warning");

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|memcpy|memmove ( %any%")) {
                const Token* arg1 = tok->tokAt(2);
                const Token* arg3 = arg1->nextArgument();
                if (arg3)
                    arg3 = arg3->nextArgument();
                if (!arg3)
                    // weird, shouldn't happen: memset etc should have
                    // 3 arguments.
                    continue;


                const Token *typeTok = nullptr;
                const Scope *type = nullptr;
                if (Token::Match(arg3, "sizeof ( %type% ) )"))
                    typeTok = arg3->tokAt(2);
                else if (Token::Match(arg3, "sizeof ( %type% :: %type% ) )"))
                    typeTok = arg3->tokAt(4);
                else if (Token::Match(arg3, "sizeof ( struct %type% ) )"))
                    typeTok = arg3->tokAt(3);
                else if (Token::simpleMatch(arg3, "sizeof ( * this ) )") || Token::simpleMatch(arg1, "this ,")) {
                    type = findFunctionOf(arg3->scope());
                } else if (Token::Match(arg1, "&|*|%var%")) {
                    int derefs = 1;
                    for (;; arg1 = arg1->next()) {
                        if (arg1->str() == "&")
                            derefs--;
                        else if (arg1->str() == "*")
                            derefs++;
                        else
                            break;
                    }

                    const Variable *var = arg1->variable();
                    if (var && arg1->strAt(1) == ",") {
                        if (var->isPointer()) {
                            derefs--;
                            if (var->typeEndToken() && Token::simpleMatch(var->typeEndToken()->previous(), "* *")) // Check if it's a pointer to pointer
                                derefs--;
                        }

                        if (var->isArray())
                            derefs -= (int)var->dimensions().size();

                        if (derefs == 0)
                            type = var->typeScope();
                    }
                }

                // No type defined => The tokens didn't match
                if (!typeTok && !type)
                    continue;

                if (typeTok && typeTok->str() == "(")
                    typeTok = typeTok->next();

                if (!type) {
                    const Type* t = symbolDatabase->findVariableType(scope, typeTok);
                    if (t)
                        type = t->classScope;
                }

                if (type) {
                    std::list<const Scope *> parsedTypes;
                    checkMemsetType(scope, tok, type, false, parsedTypes);
                }
            } else if (tok->variable() && tok->variable()->typeScope() && Token::Match(tok, "%var% = calloc|malloc|realloc|g_malloc|g_try_malloc|g_realloc|g_try_realloc (")) {
                std::list<const Scope *> parsedTypes;
                checkMemsetType(scope, tok->tokAt(2), tok->variable()->typeScope(), true, parsedTypes);

                if (tok->variable()->typeScope()->numConstructors > 0 && printWarnings)
                    mallocOnClassWarning(tok, tok->strAt(2), tok->variable()->typeScope()->classDef);
            }
        }
    }
}

void CheckClass::checkMemsetType(const Scope *start, const Token *tok, const Scope *type, bool allocation, std::list<const Scope *> parsedTypes)
{
    const bool printPortability = _settings->isEnabled("portability");

    // If type has been checked there is no need to check it again
    if (std::find(parsedTypes.begin(), parsedTypes.end(), type) != parsedTypes.end())
        return;
    parsedTypes.push_back(type);

    // recursively check all parent classes
    for (std::size_t i = 0; i < type->definedType->derivedFrom.size(); i++) {
        const Type* derivedFrom = type->definedType->derivedFrom[i].type;
        if (derivedFrom && derivedFrom->classScope)
            checkMemsetType(start, tok, derivedFrom->classScope, allocation, parsedTypes);
    }

    // Warn if type is a class that contains any virtual functions
    std::list<Function>::const_iterator func;

    for (func = type->functionList.begin(); func != type->functionList.end(); ++func) {
        if (func->isVirtual()) {
            if (allocation)
                mallocOnClassError(tok, tok->str(), type->classDef, "virtual method");
            else
                memsetError(tok, tok->str(), "virtual method", type->classDef->str());
        }
    }

    // Warn if type is a class or struct that contains any std::* variables
    std::list<Variable>::const_iterator var;

    for (var = type->varlist.begin(); var != type->varlist.end(); ++var) {
        if (var->isReference() && !var->isStatic()) {
            memsetErrorReference(tok, tok->str(), type->classDef->str());
            continue;
        }
        // don't warn if variable static or const, pointer or reference
        if (!var->isStatic() && !var->isConst() && !var->isPointer()) {
            const Token *tok1 = var->typeStartToken();
            const Scope *typeScope = var->typeScope();

            // check for std:: type
            if (var->isStlType() && tok1->strAt(2) != "array" && !_settings->library.podtype(tok1->strAt(2)))
                if (allocation)
                    mallocOnClassError(tok, tok->str(), type->classDef, "'std::" + tok1->strAt(2) + "'");
                else
                    memsetError(tok, tok->str(), "'std::" + tok1->strAt(2) + "'", type->classDef->str());

            // check for known type
            else if (typeScope && typeScope != type)
                checkMemsetType(start, tok, typeScope, allocation, parsedTypes);

            // check for float
            else if (tok->str() == "memset" && var->isFloatingType() && printPortability)
                memsetErrorFloat(tok, type->classDef->str());
        }
    }
}

void CheckClass::mallocOnClassWarning(const Token* tok, const std::string &memfunc, const Token* classTok)
{
    std::list<const Token *> toks;
    toks.push_back(tok);
    toks.push_back(classTok);
    reportError(toks, Severity::warning, "mallocOnClassWarning",
                "Memory for class instance allocated with " + memfunc + "(), but class provides constructors.\n"
                "Memory for class instance allocated with " + memfunc + "(), but class provides constructors. This is unsafe, "
                "since no constructor is called and class members remain uninitialized. Consider using 'new' instead.", 0U, false);
}

void CheckClass::mallocOnClassError(const Token* tok, const std::string &memfunc, const Token* classTok, const std::string &classname)
{
    std::list<const Token *> toks;
    toks.push_back(tok);
    toks.push_back(classTok);
    reportError(toks, Severity::error, "mallocOnClassError",
                "Memory for class instance allocated with " + memfunc + "(), but class contains a " + classname + ".\n"
                "Memory for class instance allocated with " + memfunc + "(), but class a " + classname + ". This is unsafe, "
                "since no constructor is called and class members remain uninitialized. Consider using 'new' instead.", 0U, false);
}

void CheckClass::memsetError(const Token *tok, const std::string &memfunc, const std::string &classname, const std::string &type)
{
    reportError(tok, Severity::error, "memsetClass",
                "Using '" + memfunc + "' on " + type + " that contains a " + classname + ".\n"
                "Using '" + memfunc + "' on " + type + " that contains a " + classname + " is unsafe, because constructor, destructor "
                "and copy operator calls are omitted. These are necessary for this non-POD type to ensure that a valid object "
                "is created.");
}

void CheckClass::memsetErrorReference(const Token *tok, const std::string &memfunc, const std::string &type)
{
    reportError(tok, Severity::error, "memsetClassReference", "Using '" + memfunc + "' on " + type + " that contains a reference.");
}

void CheckClass::memsetErrorFloat(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::portability, "memsetClassFloat", "Using memset() on " + type + " which contains a floating point number.\n"
                "Using memset() on " + type + " which contains a floating point number."
                " This is not portable because memset() sets each byte of a block of memory to a specific value and"
                " the actual representation of a floating-point value is implementation defined."
                " Note: In case of an IEEE754-1985 compatible implementation setting all bits to zero results in the value 0.0.");
}


//---------------------------------------------------------------------------
// ClassCheck: "void operator=(" and "const type & operator=("
//---------------------------------------------------------------------------

void CheckClass::operatorEq()
{
    if (!_settings->isEnabled("style"))
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];
        std::list<Function>::const_iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->type == Function::eOperatorEqual && func->access == Public) {
                // skip "deleted" functions - cannot be called anyway
                if (func->isDelete())
                    continue;
                // use definition for check so we don't have to deal with qualification
                if (!(Token::Match(func->retDef, "%type% &") && func->retDef->str() == scope->className)) {
                    // make sure we really have a copy assignment operator
                    if (Token::Match(func->tokenDef->tokAt(2), "const| %name% &")) {
                        if (func->tokenDef->strAt(2) == "const" &&
                            func->tokenDef->strAt(3) == scope->className)
                            operatorEqReturnError(func->retDef, scope->className);
                        else if (func->tokenDef->strAt(2) == scope->className)
                            operatorEqReturnError(func->retDef, scope->className);
                    }
                }
            }
        }
    }
}

void CheckClass::operatorEqReturnError(const Token *tok, const std::string &className)
{
    reportError(tok, Severity::style, "operatorEq", "'" + className + "::operator=' should return '" + className + " &'.\n"
                "The "+className+"::operator= does not conform to standard C/C++ behaviour. To conform to standard C/C++ behaviour, return a reference to self (such as: '"+className+" &"+className+"::operator=(..) { .. return *this; }'. For safety reasons it might be better to not fix this message. If you think that safety is always more important than conformance then please ignore/suppress this message. For more details about this topic, see the book \"Effective C++\" by Scott Meyers."
               );
}

//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C&) { ... return *this; }"
// operator= should return a reference to *this
//---------------------------------------------------------------------------

void CheckClass::operatorEqRetRefThis()
{
    if (!_settings->isEnabled("style"))
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];
        std::list<Function>::const_iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->type == Function::eOperatorEqual && func->hasBody()) {
                // make sure return signature is correct
                if (Token::Match(func->retDef, "%type% &") && func->retDef->str() == scope->className) {
                    checkReturnPtrThis(scope, &(*func), func->functionScope->classStart, func->functionScope->classEnd);
                }
            }
        }
    }
}

void CheckClass::checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last)
{
    std::set<const Function*> analyzedFunctions;
    checkReturnPtrThis(scope, func, tok, last, analyzedFunctions);
}

void CheckClass::checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last, std::set<const Function*>& analyzedFunctions)
{
    bool foundReturn = false;

    const Token* const startTok = tok;

    for (; tok && tok != last; tok = tok->next()) {
        // check for return of reference to this
        if (tok->str() == "return") {
            foundReturn = true;
            std::string cast("( " + scope->className + " & )");
            if (Token::simpleMatch(tok->next(), cast.c_str()))
                tok = tok->tokAt(4);

            // check if a function is called
            if (tok->strAt(2) == "(" &&
                tok->linkAt(2)->next()->str() == ";") {
                std::list<Function>::const_iterator it;

                // check if it is a member function
                for (it = scope->functionList.begin(); it != scope->functionList.end(); ++it) {
                    // check for a regular function with the same name and a body
                    if (it->type == Function::eFunction && it->hasBody() &&
                        it->token->str() == tok->next()->str()) {
                        // check for the proper return type
                        if (it->tokenDef->previous()->str() == "&" &&
                            it->tokenDef->strAt(-2) == scope->className) {
                            // make sure it's not a const function
                            if (!it->isConst()) {
                                /** @todo make sure argument types match */
                                // avoid endless recursions
                                if (analyzedFunctions.find(&*it) == analyzedFunctions.end()) {
                                    analyzedFunctions.insert(&*it);
                                    checkReturnPtrThis(scope, &*it, it->arg->link()->next(), it->arg->link()->next()->link(),
                                                       analyzedFunctions);
                                }
                                // just bail for now
                                else
                                    return;
                            }
                        }
                    }
                }
            }

            // check if *this is returned
            else if (!(Token::Match(tok->next(), "(| * this ;|=") ||
                       Token::simpleMatch(tok->next(), "operator= (") ||
                       Token::simpleMatch(tok->next(), "this . operator= (") ||
                       (Token::Match(tok->next(), "%type% :: operator= (") &&
                        tok->next()->str() == scope->className)))
                operatorEqRetRefThisError(func->token);
        }
    }
    if (foundReturn) {
        return;
    }
    if (startTok->next() == last) {
        if (Token::Match(func->argDef, std::string("( const " + scope->className + " &").c_str())) {
            // Typical wrong way to suppress default assignment operator by declaring it and leaving empty
            operatorEqMissingReturnStatementError(func->token, func->access == Public);
        } else {
            operatorEqMissingReturnStatementError(func->token, true);
        }
        return;
    }
    if (_settings->library.isScopeNoReturn(last, 0)) {
        // Typical wrong way to prohibit default assignment operator
        // by always throwing an exception or calling a noreturn function
        operatorEqShouldBeLeftUnimplementedError(func->token);
        return;
    }

    operatorEqMissingReturnStatementError(func->token, func->access == Public);
}

void CheckClass::operatorEqRetRefThisError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqRetRefThis", "'operator=' should return reference to 'this' instance.");
}

void CheckClass::operatorEqShouldBeLeftUnimplementedError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqShouldBeLeftUnimplemented", "'operator=' should either return reference to 'this' instance or be declared private and left unimplemented.");
}

void CheckClass::operatorEqMissingReturnStatementError(const Token *tok, bool error)
{
    if (error) {
        reportError(tok, Severity::error, "operatorEqMissingReturnStatement", "No 'return' statement in non-void function causes undefined behavior.");
    } else {
        operatorEqRetRefThisError(tok);
    }
}

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

void CheckClass::operatorEqToSelf()
{
    if (!_settings->isEnabled("warning"))
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];
        // skip classes with multiple inheritance
        if (scope->definedType->derivedFrom.size() > 1)
            continue;

        std::list<Function>::const_iterator func;
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->type == Function::eOperatorEqual && func->hasBody()) {
                // make sure that the operator takes an object of the same type as *this, otherwise we can't detect self-assignment checks
                if (func->argumentList.empty())
                    continue;
                const Token* typeTok = func->argumentList.front().typeEndToken();
                while (typeTok->str() == "const" || typeTok->str() == "&" || typeTok->str() == "*")
                    typeTok = typeTok->previous();
                if (typeTok->str() != scope->className)
                    continue;

                // make sure return signature is correct
                if (Token::Match(func->retDef, "%type% &") && func->retDef->str() == scope->className) {
                    // find the parameter name
                    const Token *rhs = func->argumentList.begin()->nameToken();

                    if (!hasAssignSelf(&(*func), rhs)) {
                        if (hasAllocation(&(*func), scope))
                            operatorEqToSelfError(func->token);
                    }
                }
            }
        }
    }
}

bool CheckClass::hasAllocation(const Function *func, const Scope* scope) const
{
    // This function is called when no simple check was found for assignment
    // to self.  We are currently looking for:
    //    - deallocate member ; ... member =
    //    - alloc member
    // That is not ideal because it can cause false negatives but its currently
    // necessary to prevent false positives.
    const Token *last = func->functionScope->classEnd;
    for (const Token *tok = func->functionScope->classStart; tok && (tok != last); tok = tok->next()) {
        if (Token::Match(tok, "%var% = malloc|realloc|calloc|new") && isMemberVar(scope, tok))
            return true;

        // check for deallocating memory
        const Token *var = nullptr;
        if (Token::Match(tok, "free ( %var%"))
            var = tok->tokAt(2);
        else if (Token::Match(tok, "delete [ ] %var%"))
            var = tok->tokAt(3);
        else if (Token::Match(tok, "delete %var%"))
            var = tok->next();
        // Check for assignment to the deleted pointer (only if its a member of the class)
        if (var && isMemberVar(scope, var)) {
            for (const Token *tok1 = var->next(); tok1 && (tok1 != last); tok1 = tok1->next()) {
                if (Token::Match(tok1, "%varid% =", var->varId()))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::hasAssignSelf(const Function *func, const Token *rhs)
{
    const Token *last = func->functionScope->classEnd;
    for (const Token *tok = func->functionScope->classStart; tok && tok != last; tok = tok->next()) {
        if (Token::simpleMatch(tok, "if (")) {
            const Token *tok1 = tok->tokAt(2);
            const Token *tok2 = tok->next()->link();

            if (tok1 && tok2) {
                for (; tok1 && tok1 != tok2; tok1 = tok1->next()) {
                    if (Token::Match(tok1, "this ==|!= & %name%")) {
                        if (tok1->strAt(3) == rhs->str())
                            return true;
                    } else if (Token::Match(tok1, "& %name% ==|!= this")) {
                        if (tok1->strAt(1) == rhs->str())
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

void CheckClass::operatorEqToSelfError(const Token *tok)
{
    reportError(tok, Severity::warning, "operatorEqToSelf",
                "'operator=' should check for assignment to self to avoid problems with dynamic memory.\n"
                "'operator=' should check for assignment to self to ensure that each block of dynamically "
                "allocated memory is owned and managed by only one instance of the class.");
}

//---------------------------------------------------------------------------
// A destructor in a base class should be virtual
//---------------------------------------------------------------------------

void CheckClass::virtualDestructor()
{
    // This error should only be given if:
    // * base class doesn't have virtual destructor
    // * derived class has non-empty destructor
    // * base class is deleted
    // unless inconclusive in which case:
    // * base class has virtual members but doesn't have virtual destructor
    const bool printInconclusive = _settings->inconclusive;

    std::list<const Function *> inconclusive_errors;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];

        // Skip base classes (unless inconclusive)
        if (scope->definedType->derivedFrom.empty()) {
            if (printInconclusive) {
                const Function *destructor = scope->getDestructor();
                if (destructor && !destructor->isVirtual()) {
                    std::list<Function>::const_iterator func;
                    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                        if (func->isVirtual()) {
                            inconclusive_errors.push_back(destructor);
                            break;
                        }
                    }
                }
            }
            continue;
        }

        // Find the destructor
        const Function *destructor = scope->getDestructor();

        // Check for destructor with implementation
        if (!destructor || !destructor->hasBody())
            continue;

        // Empty destructor
        if (destructor->token->linkAt(3) == destructor->token->tokAt(4))
            continue;

        const Token *derived = scope->classDef;
        const Token *derivedClass = derived->next();

        // Iterate through each base class...
        for (std::size_t j = 0; j < scope->definedType->derivedFrom.size(); ++j) {
            // Check if base class is public and exists in database
            if (scope->definedType->derivedFrom[j].access != Private && scope->definedType->derivedFrom[j].type) {
                const Type *derivedFrom = scope->definedType->derivedFrom[j].type;
                const Scope *derivedFromScope = derivedFrom->classScope;
                if (!derivedFromScope)
                    continue;

                // Check for this pattern:
                // 1. Base class pointer is given the address of derived class instance
                // 2. Base class pointer is deleted
                //
                // If this pattern is not seen then bailout the checking of these base/derived classes
                {
                    // pointer variables of type 'Base *'
                    std::set<unsigned int> basepointer;

                    for (std::size_t k = 1; k < symbolDatabase->getVariableListSize(); k++) {
                        const Variable* var = symbolDatabase->getVariableFromVarId(k);
                        if (var && var->isPointer() && var->type() == derivedFrom)
                            basepointer.insert(var->declarationId());
                    }

                    // pointer variables of type 'Base *' that should not be deleted
                    std::set<unsigned int> dontDelete;

                    // No deletion of derived class instance through base class pointer found => the code is ok
                    bool ok = true;

                    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
                        if (Token::Match(tok, "[;{}] %var% =") &&
                            basepointer.find(tok->next()->varId()) != basepointer.end()) {
                            // new derived class..
                            if (Token::simpleMatch(tok->tokAt(3), ("new " + derivedClass->str()).c_str())) {
                                dontDelete.insert(tok->next()->varId());
                            }
                        }

                        // Delete base class pointer that might point at derived class
                        else if (Token::Match(tok, "delete %var% ;") &&
                                 dontDelete.find(tok->next()->varId()) != dontDelete.end()) {
                            ok = false;
                            break;
                        }
                    }

                    // No base class pointer that points at a derived class is deleted
                    if (ok)
                        continue;
                }

                // Find the destructor declaration for the base class.
                const Function *base_destructor = derivedFromScope->getDestructor();
                const Token *base = nullptr;
                if (base_destructor)
                    base = base_destructor->token;

                // Check that there is a destructor..
                if (!base_destructor) {
                    if (derivedFrom->derivedFrom.empty()) {
                        virtualDestructorError(derivedFrom->classDef, derivedFrom->name(), derivedClass->str(), false);
                        // check for duplicate error and remove if if found
                        std::list<const Function *>::iterator found = find(inconclusive_errors.begin(), inconclusive_errors.end(), base_destructor);
                        if (found != inconclusive_errors.end())
                            inconclusive_errors.erase(found);
                    }
                } else if (!base_destructor->isVirtual()) {
                    // TODO: This is just a temporary fix, better solution is needed.
                    // Skip situations where base class has base classes of its own, because
                    // some of the base classes might have virtual destructor.
                    // Proper solution is to check all of the base classes. If base class is not
                    // found or if one of the base classes has virtual destructor, error should not
                    // be printed. See TODO test case "virtualDestructorInherited"
                    if (derivedFrom->derivedFrom.empty()) {
                        // Make sure that the destructor is public (protected or private
                        // would not compile if inheritance is used in a way that would
                        // cause the bug we are trying to find here.)
                        if (base_destructor->access == Public) {
                            virtualDestructorError(base, derivedFrom->name(), derivedClass->str(), false);
                            // check for duplicate error and remove if if found
                            std::list<const Function *>::iterator found = find(inconclusive_errors.begin(), inconclusive_errors.end(), base_destructor);
                            if (found != inconclusive_errors.end())
                                inconclusive_errors.erase(found);
                        }
                    }
                }
            }
        }
    }

    for (std::list<const Function *>::const_iterator i = inconclusive_errors.begin(); i != inconclusive_errors.end(); ++i)
        virtualDestructorError((*i)->tokenDef, (*i)->name(), "", true);
}

void CheckClass::virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived, bool inconclusive)
{
    if (inconclusive)
        reportError(tok, Severity::warning, "virtualDestructor", "Class '" + Base + "' which has virtual members does not have a virtual destructor.", 0U, true);
    else
        reportError(tok, Severity::error, "virtualDestructor", "Class '" + Base + "' which is inherited by class '" + Derived + "' does not have a virtual destructor.\n"
                    "Class '" + Base + "' which is inherited by class '" + Derived + "' does not have a virtual destructor. "
                    "If you destroy instances of the derived class by deleting a pointer that points to the base class, only "
                    "the destructor of the base class is executed. Thus, dynamic memory that is managed by the derived class "
                    "could leak. This can be avoided by adding a virtual destructor to the base class.");
}

//---------------------------------------------------------------------------
// warn for "this-x". The indented code may be "this->x"
//---------------------------------------------------------------------------

void CheckClass::thisSubtraction()
{
    if (!_settings->isEnabled("warning"))
        return;

    const Token *tok = _tokenizer->tokens();
    for (;;) {
        tok = Token::findmatch(tok, "this - %name%");
        if (!tok)
            break;

        if (tok->strAt(-1) != "*")
            thisSubtractionError(tok);

        tok = tok->next();
    }
}

void CheckClass::thisSubtractionError(const Token *tok)
{
    reportError(tok, Severity::warning, "thisSubtraction", "Suspicious pointer subtraction. Did you intend to write '->'?");
}

//---------------------------------------------------------------------------
// can member function be const?
//---------------------------------------------------------------------------

void CheckClass::checkConst()
{
    // This is an inconclusive check. False positives: #3322.
    if (!_settings->inconclusive)
        return;

    if (!_settings->isEnabled("style"))
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];
        std::list<Function>::const_iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            // does the function have a body?
            if (func->type == Function::eFunction && func->hasBody() && !func->isFriend() && !func->isStatic() && !func->isVirtual()) {
                // get last token of return type
                const Token *previous = func->tokenDef->previous();

                // does the function return a pointer or reference?
                if (Token::Match(previous, "*|&")) {
                    if (func->retDef->str() != "const")
                        continue;
                } else if (Token::Match(previous->previous(), "*|& >")) {
                    const Token *temp = previous;

                    bool foundConst = false;
                    while (!Token::Match(temp->previous(), ";|}|{|public:|protected:|private:")) {
                        temp = temp->previous();
                        if (temp->str() == "const") {
                            foundConst = true;
                            break;
                        }
                    }

                    if (!foundConst)
                        continue;
                } else if (func->isOperator() && Token::Match(previous, ";|{|}|public:|private:|protected:")) { // Operator without return type: conversion operator
                    const std::string& opName = func->tokenDef->str();
                    if (opName.compare(8, 5, "const") != 0 && opName[opName.size()-1] == '&')
                        continue;
                } else {
                    // don't warn for unknown types..
                    // LPVOID, HDC, etc
                    if (previous->isUpperCaseName() && previous->str().size() > 2 && !symbolDatabase->isClassOrStruct(previous->str()))
                        continue;
                }

                // check if base class function is virtual
                if (!scope->definedType->derivedFrom.empty()) {
                    if (func->isImplicitlyVirtual(true))
                        continue;
                }

                bool memberAccessed = false;
                // if nothing non-const was found. write error..
                if (checkConstFunc(scope, &*func, memberAccessed)) {
                    std::string classname = scope->className;
                    const Scope *nest = scope->nestedIn;
                    while (nest && nest->type != Scope::eGlobal) {
                        classname = std::string(nest->className + "::" + classname);
                        nest = nest->nestedIn;
                    }

                    // get function name
                    std::string functionName = (func->tokenDef->isName() ? "" : "operator") + func->tokenDef->str();

                    if (func->tokenDef->str() == "(")
                        functionName += ")";
                    else if (func->tokenDef->str() == "[")
                        functionName += "]";

                    if (!func->isConst() || (!memberAccessed && !func->isOperator())) {
                        if (func->isInline())
                            checkConstError(func->token, classname, functionName, !memberAccessed && !func->isOperator());
                        else // not inline
                            checkConstError2(func->token, func->tokenDef, classname, functionName, !memberAccessed && !func->isOperator());
                    }
                }
            }
        }
    }
}

bool CheckClass::isMemberVar(const Scope *scope, const Token *tok) const
{
    bool again = false;

    // try to find the member variable
    do {
        again = false;

        if (tok->str() == "this") {
            return true;
        } else if (Token::simpleMatch(tok->tokAt(-3), "( * this )")) {
            return true;
        } else if (Token::Match(tok->tokAt(-2), "%name% . %name%")) {
            tok = tok->tokAt(-2);
            again = true;
        } else if (Token::Match(tok->tokAt(-2), "] . %name%")) {
            tok = tok->linkAt(-2)->previous();
            again = true;
        } else if (tok->str() == "]") {
            tok = tok->link()->previous();
            again = true;
        }
    } while (again);

    std::list<Variable>::const_iterator var;
    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
        if (var->name() == tok->str()) {
            if (tok->varId() == 0)
                symbolDatabase->debugMessage(tok, "CheckClass::isMemberVar found used member variable \'" + tok->str() + "\' with varid 0");

            return !var->isStatic();
        }
    }

    // not found in this class
    if (!scope->definedType->derivedFrom.empty() && !scope->definedType->hasCircularDependencies()) {
        // check each base class
        for (std::size_t i = 0; i < scope->definedType->derivedFrom.size(); ++i) {
            // find the base class
            const Type *derivedFrom = scope->definedType->derivedFrom[i].type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope) {
                if (isMemberVar(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::isMemberFunc(const Scope *scope, const Token *tok) const
{
    if (tok->function() && tok->function()->nestedIn == scope)
        return !tok->function()->isStatic();

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        for (std::size_t i = 0; i < scope->definedType->derivedFrom.size(); ++i) {
            // find the base class
            const Type *derivedFrom = scope->definedType->derivedFrom[i].type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope && !derivedFrom->hasCircularDependencies()) {
                if (isMemberFunc(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::isConstMemberFunc(const Scope *scope, const Token *tok) const
{
    if (tok->function() && tok->function()->nestedIn == scope)
        return tok->function()->isConst();

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        for (std::size_t i = 0; i < scope->definedType->derivedFrom.size(); ++i) {
            // find the base class
            const Type *derivedFrom = scope->definedType->derivedFrom[i].type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope && !derivedFrom->hasCircularDependencies()) {
                if (isConstMemberFunc(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::checkConstFunc(const Scope *scope, const Function *func, bool& memberAccessed) const
{
    // if the function doesn't have any assignment nor function call,
    // it can be a const function..
    for (const Token *tok1 = func->functionScope->classStart; tok1 && tok1 != func->functionScope->classEnd; tok1 = tok1->next()) {
        if (tok1->isName() && isMemberVar(scope, tok1)) {
            memberAccessed = true;
            const Variable* v = tok1->variable();
            if (v && v->isMutable())
                continue;

            if (tok1->str() == "this" && tok1->previous()->isAssignmentOp())
                return (false);


            const Token* lhs = tok1->tokAt(-1);
            if (lhs->str() == "&") {
                lhs = lhs->previous();
                if (lhs->type() == Token::eAssignmentOp && lhs->previous()->variable()) {
                    if (lhs->previous()->variable()->typeStartToken()->strAt(-1) != "const" && lhs->previous()->variable()->isPointer())
                        return false;
                }
            } else {
                const Variable* v2 = lhs->previous()->variable();
                if (lhs->type() == Token::eAssignmentOp && v2)
                    if (!v2->isConst() && v2->isReference() && lhs == v2->nameToken()->next())
                        return false;
            }

            const Token* jumpBackToken = 0;
            const Token *lastVarTok = tok1;
            const Token *end = tok1;
            for (;;) {
                if (Token::Match(end->next(), ". %name%")) {
                    end = end->tokAt(2);
                    if (end->varId())
                        lastVarTok = end;
                } else if (end->strAt(1) == "[") {
                    if (end->varId()) {
                        const Variable *var = end->variable();
                        // The container contains the STL types whose operator[] is not a const.
                        // THIS ARRAY MUST BE ORDERED ALPHABETICALLY
                        static const char* const stl_containers [] = {
                            "map", "unordered_map"
                        };
                        if (var && var->isStlType(stl_containers))
                            return false;
                    }
                    if (!jumpBackToken)
                        jumpBackToken = end->next(); // Check inside the [] brackets
                    end = end->linkAt(1);
                } else if (end->strAt(1) == ")")
                    end = end->next();
                else
                    break;
            }

            if (end->strAt(1) == "(") {
                const Variable *var = lastVarTok->variable();
                if (!var)
                    return false;
                if (var->isStlType() // assume all std::*::size() and std::*::empty() are const
                    && (Token::Match(end, "size|empty|cend|crend|cbegin|crbegin|max_size|length|count|capacity|get_allocator|c_str|str ( )") || Token::Match(end, "rfind|copy")))
                    ;
                else if (!var->typeScope() || !isConstMemberFunc(var->typeScope(), end))
                    return (false);
            }

            // Assignment
            else if (end->next()->type() == Token::eAssignmentOp)
                return (false);

            // Streaming
            else if (end->strAt(1) == "<<" && tok1->strAt(-1) != "<<")
                return (false);
            else if (tok1->strAt(-1) == ">>")
                return (false);

            // ++/--
            else if (end->next()->type() == Token::eIncDecOp || tok1->previous()->type() == Token::eIncDecOp)
                return (false);


            const Token* start = tok1;
            while (tok1->strAt(-1) == ")")
                tok1 = tok1->linkAt(-1);

            if (start->strAt(-1) == "delete")
                return (false);

            tok1 = jumpBackToken?jumpBackToken:end; // Jump back to first [ to check inside, or jump to end of expression
        }

        // streaming: <<
        else if (Token::simpleMatch(tok1->previous(), ") <<") &&
                 isMemberVar(scope, tok1->tokAt(-2))) {
            const Variable* var = tok1->tokAt(-2)->variable();
            if (!var || !var->isMutable())
                return (false);
        }


        // function call..
        else if (Token::Match(tok1, "%name% (") && !tok1->isStandardType() &&
                 !Token::Match(tok1, "return|if|string|switch|while|catch|for")) {
            if (isMemberFunc(scope, tok1) && tok1->strAt(-1) != ".") {
                if (!isConstMemberFunc(scope, tok1))
                    return (false);
                memberAccessed = true;
            }
            // Member variable given as parameter
            for (const Token* tok2 = tok1->tokAt(2); tok2 && tok2 != tok1->next()->link(); tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->isName() && isMemberVar(scope, tok2)) {
                    const Variable* var = tok2->variable();
                    if (!var || !var->isMutable())
                        return (false); // TODO: Only bailout if function takes argument as non-const reference
                }
            }
        } else if (Token::simpleMatch(tok1, "> (") && (!tok1->link() || !Token::Match(tok1->link()->previous(), "static_cast|const_cast|dynamic_cast|reinterpret_cast"))) {
            return (false);
        }
    }

    return (true);
}

void CheckClass::checkConstError(const Token *tok, const std::string &classname, const std::string &funcname, bool suggestStatic)
{
    checkConstError2(tok, 0, classname, funcname, suggestStatic);
}

void CheckClass::checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname, bool suggestStatic)
{
    std::list<const Token *> toks;
    toks.push_back(tok1);
    if (tok2)
        toks.push_back(tok2);
    if (!suggestStatic)
        reportError(toks, Severity::style, "functionConst",
                    "Technically the member function '" + classname + "::" + funcname + "' can be const.\n"
                    "The member function '" + classname + "::" + funcname + "' can be made a const "
                    "function. Making this function 'const' should not cause compiler errors. "
                    "Even though the function can be made const function technically it may not make "
                    "sense conceptually. Think about your design and the task of the function first - is "
                    "it a function that must not change object internal state?", 0U, true);
    else
        reportError(toks, Severity::performance, "functionStatic",
                    "Technically the member function '" + classname + "::" + funcname + "' can be static.\n"
                    "The member function '" + classname + "::" + funcname + "' can be made a static "
                    "function. Making a function static can bring a performance benefit since no 'this' instance is "
                    "passed to the function. This change should not cause compiler errors but it does not "
                    "necessarily make sense conceptually. Think about your design and the task of the function first - "
                    "is it a function that must not access members of class instances?", 0U, true);
}

//---------------------------------------------------------------------------
// ClassCheck: Check that initializer list is in declared order.
//---------------------------------------------------------------------------

struct VarInfo {
    VarInfo(const Variable *_var, const Token *_tok)
        : var(_var), tok(_tok) { }

    const Variable *var;
    const Token *tok;
};

void CheckClass::initializerListOrder()
{
    if (!_settings->isEnabled("style"))
        return;

    // This check is not inconclusive.  However it only determines if the initialization
    // order is incorrect.  It does not determine if being out of order causes
    // a real error.  Out of order is not necessarily an error but you can never
    // have an error if the list is in order so this enforces defensive programming.
    if (!_settings->inconclusive)
        return;

    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];
        std::list<Function>::const_iterator func;

        // iterate through all member functions looking for constructors
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if ((func->isConstructor()) && func->hasBody()) {
                // check for initializer list
                const Token *tok = func->arg->link()->next();

                if (tok->str() == ":") {
                    std::vector<VarInfo> vars;
                    tok = tok->next();

                    // find all variable initializations in list
                    while (tok && tok != func->functionScope->classStart) {
                        if (Token::Match(tok, "%name% (|{")) {
                            const Variable *var = scope->getVariable(tok->str());
                            if (var)
                                vars.push_back(VarInfo(var, tok));

                            if (Token::Match(tok->tokAt(2), "%name% =")) {
                                var = scope->getVariable(tok->strAt(2));

                                if (var)
                                    vars.push_back(VarInfo(var, tok->tokAt(2)));
                            }
                            tok = tok->next()->link()->next();
                        } else
                            tok = tok->next();
                    }

                    // need at least 2 members to have out of order initialization
                    for (std::size_t j = 1; j < vars.size(); j++) {
                        // check for out of order initialization
                        if (vars[j].var->index() < vars[j - 1].var->index())
                            initializerListError(vars[j].tok,vars[j].var->nameToken(), scope->className, vars[j].var->name());
                    }
                }
            }
        }
    }
}

void CheckClass::initializerListError(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &varname)
{
    std::list<const Token *> toks;
    toks.push_back(tok1);
    toks.push_back(tok2);
    reportError(toks, Severity::style, "initializerList",
                "Member variable '" + classname + "::" +
                varname + "' is in the wrong place in the initializer list.\n"
                "Member variable '" + classname + "::" +
                varname + "' is in the wrong place in the initializer list. "
                "Members are initialized in the order they are declared, not in the "
                "order they are in the initializer list.  Keeping the initializer list "
                "in the same order that the members were declared prevents order dependent "
                "initialization errors.", 0U, true);
}


//---------------------------------------------------------------------------
// Check for self initialization in initialization list
//---------------------------------------------------------------------------

void CheckClass::checkSelfInitialization()
{
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        const Function* function = scope->function;
        if (!function || !function->isConstructor())
            continue;

        const Token* tok = function->arg->link()->next();
        if (tok->str() != ":")
            continue;

        for (; tok != scope->classStart; tok = tok->next()) {
            if (Token::Match(tok, "[:,] %var% (|{ %var% )|}") && tok->next()->varId() == tok->tokAt(3)->varId()) {
                selfInitializationError(tok, tok->strAt(1));
            }
        }
    }
}

void CheckClass::selfInitializationError(const Token* tok, const std::string& varname)
{
    reportError(tok, Severity::error, "selfInitialization", "Member variable '" + varname + "' is initialized by itself.");
}


//---------------------------------------------------------------------------
// Check for pure virtual function calls
//---------------------------------------------------------------------------

void CheckClass::checkPureVirtualFunctionCall()
{
    if (! _settings->isEnabled("warning"))
        return;
    const std::size_t functions = symbolDatabase->functionScopes.size();
    std::map<const Function *, std::list<const Token *> > callsPureVirtualFunctionMap;
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        if (scope->function == 0 || !scope->function->hasBody() ||
            !(scope->function->isConstructor() ||
              scope->function->isDestructor()))
            continue;

        const std::list<const Token *> & pureVirtualFunctionCalls=callsPureVirtualFunction(*scope->function,callsPureVirtualFunctionMap);
        for (std::list<const Token *>::const_iterator pureCallIter=pureVirtualFunctionCalls.begin();
             pureCallIter!=pureVirtualFunctionCalls.end();
             ++pureCallIter) {
            const Token & pureCall=**pureCallIter;
            std::list<const Token *> pureFuncStack;
            pureFuncStack.push_back(&pureCall);
            getFirstPureVirtualFunctionCallStack(callsPureVirtualFunctionMap, pureCall, pureFuncStack);
            if (!pureFuncStack.empty())
                callsPureVirtualFunctionError(*scope->function, pureFuncStack, pureFuncStack.back()->str());
        }
    }
}

const std::list<const Token *> & CheckClass::callsPureVirtualFunction(const Function & function,
        std::map<const Function *, std::list<const Token *> > & callsPureVirtualFunctionMap)
{
    std::pair<std::map<const Function *, std::list<const Token *> >::iterator, bool > found =
        callsPureVirtualFunctionMap.insert(std::pair<const Function *, std::list< const Token *> >(&function, std::list<const Token *>()));
    std::list<const Token *> & pureFunctionCalls = found.first->second;
    if (found.second) {
        if (function.hasBody()) {
            for (const Token *tok = function.arg->link();
                 tok && tok != function.functionScope->classEnd;
                 tok = tok->next()) {
                if (function.type != Function::eConstructor &&
                    function.type != Function::eCopyConstructor &&
                    function.type != Function::eMoveConstructor &&
                    function.type != Function::eDestructor) {
                    if ((Token::simpleMatch(tok, ") {") &&
                         tok->link() &&
                         Token::Match(tok->link()->previous(), "if|switch")) ||
                        Token::simpleMatch(tok, "else {")
                       ) {
                        // Assume pure virtual function call is prevented by "if|else|switch" condition
                        tok = tok->linkAt(1);
                        continue;
                    }
                }
                const Function * callFunction = tok->function();
                if (!callFunction ||
                    function.nestedIn != callFunction->nestedIn ||
                    (tok->previous() && tok->previous()->str() == "."))
                    continue;

                if (tok->previous() &&
                    tok->previous()->str() == "(") {
                    const Token * prev = tok->previous();
                    if (prev->previous() &&
                        (_settings->library.ignorefunction(tok->str())
                         || _settings->library.ignorefunction(prev->previous()->str())))
                        continue;
                }

                if (isPureWithoutBody(*callFunction)) {
                    pureFunctionCalls.push_back(tok);
                    continue;
                }

                const std::list<const Token *> & pureFunctionCallsOfTok = callsPureVirtualFunction(*callFunction,
                        callsPureVirtualFunctionMap);
                if (!pureFunctionCallsOfTok.empty()) {
                    pureFunctionCalls.push_back(tok);
                    continue;
                }
            }
        }
    }
    return pureFunctionCalls;
}

void CheckClass::getFirstPureVirtualFunctionCallStack(
    std::map<const Function *, std::list<const Token *> > & callsPureVirtualFunctionMap,
    const Token & pureCall,
    std::list<const Token *> & pureFuncStack)
{
    if (isPureWithoutBody(*pureCall.function())) {
        pureFuncStack.push_back(pureCall.function()->token);
        return;
    }
    std::map<const Function *, std::list<const Token *> >::const_iterator found = callsPureVirtualFunctionMap.find(pureCall.function());
    if (found == callsPureVirtualFunctionMap.end() ||
        found->second.empty()) {
        pureFuncStack.clear();
        return;
    }
    const Token & firstPureCall = **found->second.begin();
    pureFuncStack.push_back(&firstPureCall);
    getFirstPureVirtualFunctionCallStack(callsPureVirtualFunctionMap, firstPureCall, pureFuncStack);
}

void CheckClass::callsPureVirtualFunctionError(
    const Function & scopeFunction,
    const std::list<const Token *> & tokStack,
    const std::string &purefuncname)
{
    const char * scopeFunctionTypeName = getFunctionTypeName(scopeFunction.type);
    reportError(tokStack, Severity::warning, "pureVirtualCall", "Call of pure virtual function '" + purefuncname + "' in " + scopeFunctionTypeName + ".\n"
                "Call of pure virtual function '" + purefuncname + "' in " + scopeFunctionTypeName + ". The call will fail during runtime.", 0U, false);
}


//---------------------------------------------------------------------------
// Check for members hiding inherited members with the same name
//---------------------------------------------------------------------------

void CheckClass::checkDuplInheritedMembers()
{
    if (!_settings->isEnabled("warning"))
        return;

    // Iterate over all classes
    for (std::list<Type>::const_iterator classIt = symbolDatabase->typeList.begin();
         classIt != symbolDatabase->typeList.end();
         ++classIt) {
        // Iterate over the parent classes
        for (std::vector<Type::BaseInfo>::const_iterator parentClassIt = classIt->derivedFrom.begin();
             parentClassIt != classIt->derivedFrom.end();
             ++parentClassIt) {
            // Check if there is info about the 'Base' class
            if (!parentClassIt->type || !parentClassIt->type->classScope)
                continue;
            // Check if they have a member variable in common
            for (std::list<Variable>::const_iterator classVarIt = classIt->classScope->varlist.begin();
                 classVarIt != classIt->classScope->varlist.end();
                 ++classVarIt) {
                for (std::list<Variable>::const_iterator parentClassVarIt = parentClassIt->type->classScope->varlist.begin();
                     parentClassVarIt != parentClassIt->type->classScope->varlist.end();
                     ++parentClassVarIt) {
                    if (classVarIt->name() == parentClassVarIt->name() && !parentClassVarIt->isPrivate()) { // Check if the class and its parent have a common variable
                        duplInheritedMembersError(classVarIt->nameToken(), parentClassVarIt->nameToken(),
                                                  classIt->name(), parentClassIt->type->name(), classVarIt->name(),
                                                  classIt->classScope->type == Scope::eStruct,
                                                  parentClassIt->type->classScope->type == Scope::eStruct);
                    }
                }
            }
        }
    }
}

void CheckClass::duplInheritedMembersError(const Token *tok1, const Token* tok2,
        const std::string &derivedname, const std::string &basename,
        const std::string &variablename, bool derivedIsStruct, bool baseIsStruct)
{
    std::list<const Token *> toks;
    toks.push_back(tok1);
    toks.push_back(tok2);

    const std::string message = "The " + std::string(derivedIsStruct ? "struct" : "class") + " '" + derivedname +
                                "' defines member variable with name '" + variablename + "' also defined in its parent " +
                                std::string(baseIsStruct ? "struct" : "class") + " '" + basename + "'.";
    reportError(toks, Severity::warning, "duplInheritedMember", message, 0U, false);
}
