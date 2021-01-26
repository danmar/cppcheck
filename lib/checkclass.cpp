/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "astutils.h"
#include "library.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "errortypes.h"
#include "token.h"
#include "tokenize.h"
#include "utils.h"

#include <algorithm>
#include <cstdlib>
#include <utility>
//---------------------------------------------------------------------------

// Register CheckClass..
namespace {
    CheckClass instance;
}

static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE404(404U);  // Improper Resource Shutdown or Release
static const CWE CWE665(665U);  // Improper Initialization
static const CWE CWE758(758U);  // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE762(762U);  // Mismatched Memory Management Routines

static const char * getFunctionTypeName(Function::Type type)
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
    case Function::eLambda:
        return "lambda";
    }
    return "";
}

static bool isVariableCopyNeeded(const Variable &var)
{
    return var.isPointer() || (var.type() && var.type()->needInitialization == Type::NeedInitialization::True) || (var.valueType()->type >= ValueType::Type::CHAR);
}

//---------------------------------------------------------------------------

CheckClass::CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : Check(myName(), tokenizer, settings, errorLogger),
      mSymbolDatabase(tokenizer?tokenizer->getSymbolDatabase():nullptr)
{

}

//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    const bool printStyle = mSettings->isEnabled(Settings::STYLE);
    const bool printWarnings = mSettings->isEnabled(Settings::WARNING);
    if (!printStyle && !printWarnings)
        return;

    const bool printInconclusive = mSettings->inconclusive;
    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        const bool unusedTemplate = Token::simpleMatch(scope->classDef->previous(), ">");

        bool usedInUnion = false;
        for (const Scope &unionScope : mSymbolDatabase->scopeList) {
            if (unionScope.type != Scope::eUnion)
                continue;
            for (const Variable &var : unionScope.varlist) {
                if (var.type() && var.type()->classScope == scope) {
                    usedInUnion = true;
                    break;
                }
            }
        }

        // There are no constructors.
        if (scope->numConstructors == 0 && printStyle && !usedInUnion) {
            // If there is a private variable, there should be a constructor..
            for (const Variable &var : scope->varlist) {
                if (var.isPrivate() && !var.isStatic() && !var.isInit() &&
                    (!var.isClass() || (var.type() && var.type()->needInitialization == Type::NeedInitialization::True))) {
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
            for (const Scope * const nestedScope : scope->nestedList) {
                if (nestedScope->type == Scope::eUnion) {
                    bailout = true;
                    break;
                }
            }
            if (bailout)
                continue;
        }


        std::vector<Usage> usage(scope->varlist.size());

        for (const Function &func : scope->functionList) {
            if (!func.hasBody() || !(func.isConstructor() || func.type == Function::eOperatorEqual))
                continue;

            // Bail: If initializer list is not recognized as a variable or type then skip since parsing is incomplete
            if (unusedTemplate && func.type == Function::eConstructor) {
                const Token *initList = func.constructorMemberInitialization();
                if (Token::Match(initList, ": %name% (") && initList->next()->tokType() == Token::eName)
                    break;
            }

            // Mark all variables not used
            clearAllVar(usage);

            std::list<const Function *> callstack;
            initializeVarList(func, callstack, scope, usage);

            // Check if any variables are uninitialized
            int count = -1;
            for (const Variable &var : scope->varlist) {
                ++count;

                // check for C++11 initializer
                if (var.hasDefault()) {
                    usage[count].init = true;
                    continue;
                }

                if (usage[count].assign || usage[count].init || var.isStatic())
                    continue;

                if (var.valueType()->pointer == 0 && var.type() && var.type()->needInitialization == Type::NeedInitialization::False && var.type()->derivedFrom.empty())
                    continue;

                if (var.isConst() && func.isOperator()) // We can't set const members in assignment operator
                    continue;

                // Check if this is a class constructor
                if (!var.isPointer() && !var.isPointerArray() && var.isClass() && func.type == Function::eConstructor) {
                    // Unknown type so assume it is initialized
                    if (!var.type())
                        continue;

                    // Known type that doesn't need initialization or
                    // known type that has member variables of an unknown type
                    else if (var.type()->needInitialization != Type::NeedInitialization::True)
                        continue;
                }

                // Check if type can't be copied
                if (!var.isPointer() && !var.isPointerArray() && var.typeScope()) {
                    if (func.type == Function::eMoveConstructor) {
                        if (canNotMove(var.typeScope()))
                            continue;
                    } else {
                        if (canNotCopy(var.typeScope()))
                            continue;
                    }
                }

                bool inconclusive = false;
                // Don't warn about unknown types in copy constructors since we
                // don't know if they can be copied or not..
                if ((func.type == Function::eCopyConstructor || func.type == Function::eMoveConstructor || func.type == Function::eOperatorEqual) && !isVariableCopyNeeded(var))
                    inconclusive = true;

                if (!printInconclusive && inconclusive)
                    continue;

                // It's non-static and it's not initialized => error
                if (func.type == Function::eOperatorEqual) {
                    const Token *operStart = func.arg;

                    bool classNameUsed = false;
                    for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next()) {
                        if (operTok->str() == scope->className) {
                            classNameUsed = true;
                            break;
                        }
                    }

                    if (classNameUsed)
                        operatorEqVarError(func.token, scope->className, var.name(), inconclusive);
                } else if (func.access != AccessControl::Private || mSettings->standards.cpp >= Standards::CPP11) {
                    // If constructor is not in scope then we maybe using a constructor from a different template specialization
                    if (!precedes(scope->bodyStart, func.tokenDef))
                        continue;
                    const Scope *varType = var.typeScope();
                    if (!varType || varType->type != Scope::eUnion) {
                        if (func.type == Function::eConstructor &&
                            func.nestedIn && (func.nestedIn->numConstructors - func.nestedIn->numCopyOrMoveConstructors) > 1 &&
                            func.argCount() == 0 && func.functionScope &&
                            func.arg && func.arg->link()->next() == func.functionScope->bodyStart &&
                            func.functionScope->bodyStart->link() == func.functionScope->bodyStart->next()) {
                            // don't warn about user defined default constructor when there are other constructors
                            if (printInconclusive)
                                uninitVarError(func.token, func.access == AccessControl::Private, func.type, scope->className, var.name(), true);
                        } else
                            uninitVarError(func.token, func.access == AccessControl::Private, func.type, scope->className, var.name(), inconclusive);
                    }
                }
            }
        }
    }
}

void CheckClass::checkExplicitConstructors()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        // Do not perform check, if the class/struct has not any constructors
        if (scope->numConstructors == 0)
            continue;

        // Is class abstract? Maybe this test is over-simplification, but it will suffice for simple cases,
        // and it will avoid false positives.
        bool isAbstractClass = false;
        for (const Function &func : scope->functionList) {
            if (func.isPure()) {
                isAbstractClass = true;
                break;
            }
        }

        // Abstract classes can't be instantiated. But if there is C++11
        // "misuse" by derived classes then these constructors must be explicit.
        if (isAbstractClass && mSettings->standards.cpp != Standards::CPP11)
            continue;

        for (const Function &func : scope->functionList) {

            // We are looking for constructors, which are meeting following criteria:
            //  1) Constructor is declared with a single parameter
            //  2) Constructor is not declared as explicit
            //  3) It is not a copy/move constructor of non-abstract class
            //  4) Constructor is not marked as delete (programmer can mark the default constructor as deleted, which is ok)
            if (!func.isConstructor() || func.isDelete() || (!func.hasBody() && func.access == AccessControl::Private))
                continue;

            if (!func.isExplicit() &&
                func.minArgCount() == 1 &&
                func.type != Function::eCopyConstructor &&
                func.type != Function::eMoveConstructor) {
                noExplicitConstructorError(func.tokenDef, scope->className, scope->type == Scope::eStruct);
            }
        }
    }
}

static bool isNonCopyable(const Scope *scope, bool *unknown)
{
    bool u = false;
    // check if there is base class that is not copyable
    for (const Type::BaseInfo &baseInfo : scope->definedType->derivedFrom) {
        if (!baseInfo.type || !baseInfo.type->classScope) {
            u = true;
            continue;
        }

        if (isNonCopyable(baseInfo.type->classScope, &u))
            return true;

        for (const Function &func : baseInfo.type->classScope->functionList) {
            if (func.type != Function::eCopyConstructor)
                continue;
            if (func.access == AccessControl::Private || func.isDelete())
                return true;
        }
    }
    *unknown = u;
    return false;
}

void CheckClass::copyconstructors()
{
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        std::map<int, const Token*> allocatedVars;

        for (const Function &func : scope->functionList) {
            if (func.type != Function::eConstructor || !func.functionScope)
                continue;
            const Token* tok = func.token->linkAt(1);
            for (const Token* const end = func.functionScope->bodyStart; tok != end; tok = tok->next()) {
                if (Token::Match(tok, "%var% ( new") ||
                    (Token::Match(tok, "%var% ( %name% (") && mSettings->library.getAllocFuncInfo(tok->tokAt(2)))) {
                    const Variable* var = tok->variable();
                    if (var && var->isPointer() && var->scope() == scope)
                        allocatedVars[tok->varId()] = tok;
                }
            }
            for (const Token* const end = func.functionScope->bodyEnd; tok != end; tok = tok->next()) {
                if (Token::Match(tok, "%var% = new") ||
                    (Token::Match(tok, "%var% = %name% (") && mSettings->library.getAllocFuncInfo(tok->tokAt(2)))) {
                    const Variable* var = tok->variable();
                    if (var && var->isPointer() && var->scope() == scope && !var->isStatic())
                        allocatedVars[tok->varId()] = tok;
                }
            }
        }

        if (!allocatedVars.empty()) {
            const Function *funcCopyCtor = nullptr;
            const Function *funcOperatorEq = nullptr;
            const Function *funcDestructor = nullptr;
            for (const Function &func : scope->functionList) {
                if (func.type == Function::eCopyConstructor)
                    funcCopyCtor = &func;
                else if (func.type == Function::eOperatorEqual)
                    funcOperatorEq = &func;
                else if (func.type == Function::eDestructor)
                    funcDestructor = &func;
            }
            if (!funcCopyCtor || funcCopyCtor->isDefault()) {
                bool unknown = false;
                if (!isNonCopyable(scope, &unknown) && !unknown)
                    noCopyConstructorError(scope, funcCopyCtor, allocatedVars.begin()->second, unknown);
            }
            if (!funcOperatorEq || funcOperatorEq->isDefault()) {
                bool unknown = false;
                if (!isNonCopyable(scope, &unknown) && !unknown)
                    noOperatorEqError(scope, funcOperatorEq, allocatedVars.begin()->second, unknown);
            }
            if (!funcDestructor || funcDestructor->isDefault()) {
                const Token * mustDealloc = nullptr;
                for (std::map<int, const Token*>::const_iterator it = allocatedVars.begin(); it != allocatedVars.end(); ++it) {
                    if (!Token::Match(it->second, "%var% [(=] new %type%")) {
                        mustDealloc = it->second;
                        break;
                    }
                    if (it->second->valueType() && it->second->valueType()->isIntegral()) {
                        mustDealloc = it->second;
                        break;
                    }
                    const Variable *var = it->second->variable();
                    if (var && var->typeScope() && var->typeScope()->functionList.empty() && var->type()->derivedFrom.empty()) {
                        mustDealloc = it->second;
                        break;
                    }
                }
                if (mustDealloc)
                    noDestructorError(scope, funcDestructor, mustDealloc);
            }
        }

        std::set<const Token*> copiedVars;
        const Token* copyCtor = nullptr;
        for (const Function &func : scope->functionList) {
            if (func.type != Function::eCopyConstructor)
                continue;
            copyCtor = func.tokenDef;
            if (!func.functionScope) {
                allocatedVars.clear();
                break;
            }
            const Token* tok = func.tokenDef->linkAt(1)->next();
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
            for (tok = func.functionScope->bodyStart; tok != func.functionScope->bodyEnd; tok = tok->next()) {
                if (Token::Match(tok, "%var% = new|malloc|g_malloc|g_try_malloc|realloc|g_realloc|g_try_realloc")) {
                    allocatedVars.erase(tok->varId());
                } else if (Token::Match(tok, "%var% = %name% . %name% ;") && allocatedVars.find(tok->varId()) != allocatedVars.end()) {
                    copiedVars.insert(tok);
                }
            }
            break;
        }
        if (copyCtor && !copiedVars.empty()) {
            for (const Token *cv : copiedVars)
                copyConstructorShallowCopyError(cv, cv->str());
            // throw error if count mismatch
            /* FIXME: This doesn't work. See #4154
            for (std::map<int, const Token*>::const_iterator i = allocatedVars.begin(); i != allocatedVars.end(); ++i) {
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
    reportError(tok, Severity::warning, "copyCtorPointerCopying",
                "$symbol:" + varname + "\nValue of pointer '$symbol', which points to allocated memory, is copied in copy constructor instead of allocating new memory.", CWE398, false);
}

static std::string noMemberErrorMessage(const Scope *scope, const char function[], bool isdefault)
{
    const std::string &classname = scope ? scope->className : "class";
    const std::string type = (scope && scope->type == Scope::eStruct) ? "Struct" : "Class";
    const bool isDestructor = (function[0] == 'd');
    std::string errmsg = "$symbol:" + classname + '\n';

    if (isdefault) {
        errmsg += type + " '$symbol' has dynamic memory/resource allocation(s). The " + function + " is explicitly defaulted but the default " + function + " does not work well.";
        if (isDestructor)
            errmsg += " It is recommended to define the " + std::string(function) + '.';
        else
            errmsg += " It is recommended to define or delete the " + std::string(function) + '.';
    } else {
        errmsg += type + " '$symbol' does not have a " + function + " which is recommended since it has dynamic memory/resource allocation(s).";
    }

    return errmsg;
}

void CheckClass::noCopyConstructorError(const Scope *scope, bool isdefault, const Token *alloc, bool inconclusive)
{
    reportError(alloc, Severity::warning, "noCopyConstructor", noMemberErrorMessage(scope, "copy constructor", isdefault), CWE398, inconclusive);
}

void CheckClass::noOperatorEqError(const Scope *scope, bool isdefault, const Token *alloc, bool inconclusive)
{
    reportError(alloc, Severity::warning, "noOperatorEq", noMemberErrorMessage(scope, "operator=", isdefault), CWE398, inconclusive);
}

void CheckClass::noDestructorError(const Scope *scope, bool isdefault, const Token *alloc)
{
    reportError(alloc, Severity::warning, "noDestructor", noMemberErrorMessage(scope, "destructor", isdefault), CWE398, false);
}

bool CheckClass::canNotCopy(const Scope *scope)
{
    bool constructor = false;
    bool publicAssign = false;
    bool publicCopy = false;

    for (const Function &func : scope->functionList) {
        if (func.isConstructor())
            constructor = true;
        if (func.access != AccessControl::Public)
            continue;
        if (func.type == Function::eCopyConstructor) {
            publicCopy = true;
            break;
        } else if (func.type == Function::eOperatorEqual) {
            publicAssign = true;
            break;
        }
    }

    return constructor && !(publicAssign || publicCopy);
}

bool CheckClass::canNotMove(const Scope *scope)
{
    bool constructor = false;
    bool publicAssign = false;
    bool publicCopy = false;
    bool publicMove = false;

    for (const Function &func : scope->functionList) {
        if (func.isConstructor())
            constructor = true;
        if (func.access != AccessControl::Public)
            continue;
        if (func.type == Function::eCopyConstructor) {
            publicCopy = true;
            break;
        } else if (func.type == Function::eMoveConstructor) {
            publicMove = true;
            break;
        } else if (func.type == Function::eOperatorEqual) {
            publicAssign = true;
            break;
        }
    }

    return constructor && !(publicAssign || publicCopy || publicMove);
}

void CheckClass::assignVar(nonneg int varid, const Scope *scope, std::vector<Usage> &usage)
{
    int count = 0;

    for (std::list<Variable>::const_iterator var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count) {
        if (var->declarationId() == varid) {
            usage[count].assign = true;
            return;
        }
    }
}

void CheckClass::initVar(nonneg int varid, const Scope *scope, std::vector<Usage> &usage)
{
    int count = 0;

    for (std::list<Variable>::const_iterator var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count) {
        if (var->declarationId() == varid) {
            usage[count].init = true;
            return;
        }
    }
}

void CheckClass::assignAllVar(std::vector<Usage> &usage)
{
    for (Usage & i : usage)
        i.assign = true;
}

void CheckClass::clearAllVar(std::vector<Usage> &usage)
{
    for (Usage & i : usage) {
        i.assign = false;
        i.init = false;
    }
}

bool CheckClass::isBaseClassFunc(const Token *tok, const Scope *scope)
{
    // Iterate through each base class...
    for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
        const Type *derivedFrom = i.type;

        // Check if base class exists in database
        if (derivedFrom && derivedFrom->classScope) {
            const std::list<Function>& functionList = derivedFrom->classScope->functionList;

            for (const Function &func : functionList) {
                if (func.tokenDef->str() == tok->str())
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
        throw InternalError(nullptr, "Internal Error: Invalid syntax"); // #5702
    bool initList = func.isConstructor();
    const Token *ftok = func.arg->link()->next();
    int level = 0;
    for (; ftok && ftok != func.functionScope->bodyEnd; ftok = ftok->next()) {
        // Class constructor.. initializing variables like this
        // clKalle::clKalle() : var(value) { }
        if (initList) {
            if (level == 0 && Token::Match(ftok, "%name% {|(") && Token::Match(ftok->linkAt(1), "}|) ,|{")) {
                if (ftok->str() != func.name()) {
                    initVar(ftok->varId(), scope, usage);
                } else { // c++11 delegate constructor
                    const Function *member = ftok->function();
                    // member function not found => assume it initializes all members
                    if (!member) {
                        assignAllVar(usage);
                        return;
                    }

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
            } else if (level != 0 && Token::Match(ftok, "%name% =")) // assignment in the initializer: var(value = x)
                assignVar(ftok->varId(), scope, usage);

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
        if (Token::Match(ftok, ">>|& %name%") && isLikelyStreamRead(true, ftok)) {
            assignVar(ftok->next()->varId(), scope, usage);
        }

        // If assignment comes after an && or || this is really inconclusive because of short circuiting
        if (Token::Match(ftok, "%oror%|&&"))
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
            for (const Variable &var : scope->varlist) {
                if (var.declarationId() == ftok->next()->varId()) {
                    /** @todo false negative: we assume function changes variable state */
                    assignVar(ftok->next()->varId(), scope, usage);
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

        // Ticket #7068
        else if (Token::Match(ftok, "::| memset ( &| this . %name%")) {
            if (ftok->str() == "::")
                ftok = ftok->next();
            int offsetToMember = 4;
            if (ftok->strAt(2) == "&")
                ++offsetToMember;
            assignVar(ftok->tokAt(offsetToMember)->varId(), scope, usage);
            ftok = ftok->linkAt(1);
            continue;
        }

        // Clearing array..
        else if (Token::Match(ftok, "::| memset ( %name% ,")) {
            if (ftok->str() == "::")
                ftok = ftok->next();
            assignVar(ftok->tokAt(2)->varId(), scope, usage);
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
        } else if (Token::Match(ftok, "::| %name% (") && !Token::Match(ftok, "if|while|for")) {
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
                            assignVar(tok2->varId(), scope, usage);
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
                            assignVar(tok->varId(), scope, usage);
                        }
                    }
                }
            }
        }

        // Assignment of member variable?
        else if (Token::Match(ftok, "%name% =")) {
            assignVar(ftok->varId(), scope, usage);
            bool bailout = ftok->variable() && ftok->variable()->isReference();
            const Token* tok2 = ftok->tokAt(2);
            if (tok2->str() == "&") {
                tok2 = tok2->next();
                bailout = true;
            }
            if (tok2->variable() && (bailout || tok2->variable()->isArray()) && tok2->strAt(1) != "[")
                assignVar(tok2->varId(), scope, usage);
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
                assignVar(ftok->varId(), scope, usage);
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "* %name% =")) {
            assignVar(ftok->next()->varId(), scope, usage);
        } else if (Token::Match(ftok, "* this . %name% =")) {
            assignVar(ftok->tokAt(3)->varId(), scope, usage);
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Token::Match(ftok, "%name% . clear|Clear (")) {
            assignVar(ftok->varId(), scope, usage);
        }
    }
}

void CheckClass::noConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    // For performance reasons the constructor might be intentionally missing. Therefore this is not a "warning"
    reportError(tok, Severity::style, "noConstructor",
                "$symbol:" + classname + "\n" +
                "The " + std::string(isStruct ? "struct" : "class") + " '$symbol' does not have a constructor although it has private member variables.\n"
                "The " + std::string(isStruct ? "struct" : "class") + " '$symbol' does not have a constructor "
                "although it has private member variables. Member variables of builtin types are left "
                "uninitialized when the class is instantiated. That may cause bugs or undefined behavior.", CWE398, false);
}

void CheckClass::noExplicitConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    const std::string message(std::string(isStruct ? "Struct" : "Class") + " '$symbol' has a constructor with 1 argument that is not explicit.");
    const std::string verbose(message + " Such constructors should in general be explicit for type safety reasons. Using the explicit keyword in the constructor means some mistakes when using the class can be avoided.");
    reportError(tok, Severity::style, "noExplicitConstructor", "$symbol:" + classname + '\n' + message + '\n' + verbose, CWE398, false);
}

void CheckClass::uninitVarError(const Token *tok, bool isprivate, Function::Type functionType, const std::string &classname, const std::string &varname, bool inconclusive)
{
    std::string message;
    if ((functionType == Function::eCopyConstructor || functionType == Function::eMoveConstructor) && inconclusive)
        message = "Member variable '$symbol' is not assigned in the copy constructor. Should it be copied?";
    else
        message = "Member variable '$symbol' is not initialized in the constructor.";
    reportError(tok, Severity::warning, isprivate ? "uninitMemberVarPrivate" : "uninitMemberVar", "$symbol:" + classname + "::" + varname + "\n" + message, CWE398, inconclusive);
}

void CheckClass::operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname, bool inconclusive)
{
    reportError(tok, Severity::warning, "operatorEqVarError", "$symbol:" + classname + "::" + varname + "\nMember variable '$symbol' is not assigned a value in '" + classname + "::operator='.", CWE398, inconclusive);
}

//---------------------------------------------------------------------------
// ClassCheck: Use initialization list instead of assignment
//---------------------------------------------------------------------------

void CheckClass::initializationListUsage()
{
    if (!mSettings->isEnabled(Settings::PERFORMANCE))
        return;

    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        // Check every constructor
        if (!scope->function || !scope->function->isConstructor())
            continue;

        // Do not warn when a delegate constructor is called
        if (const Token *initList = scope->function->constructorMemberInitialization()) {
            if (Token::Match(initList, ": %name% {|(") && initList->strAt(1) == scope->className)
                continue;
        }

        const Scope* owner = scope->functionOf;
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%name% (")) // Assignments might depend on this function call or if/for/while/switch statement from now on.
                break;
            if (Token::Match(tok, "try|do {"))
                break;
            if (!Token::Match(tok, "%var% =") || tok->strAt(-1) == "*" || tok->strAt(-1) == ".")
                continue;

            const Variable* var = tok->variable();
            if (!var || var->scope() != owner || var->isStatic())
                continue;
            if (var->isPointer() || var->isReference() || var->isEnumType())
                continue;
            if (!WRONG_DATA(!var->valueType(), tok) && var->valueType()->type > ValueType::Type::ITERATOR)
                continue;

            // bailout: multi line lambda in rhs => do not warn
            if (findLambdaEndToken(tok->tokAt(2)) && tok->tokAt(2)->findExpressionStartEndTokens().second->linenr() > tok->tokAt(2)->linenr())
                continue;

            // Access local var member in rhs => do not warn
            bool localmember = false;
            visitAstNodes(tok->next()->astOperand2(),
            [&](const Token *rhs) {
                if (rhs->str() == "." && rhs->astOperand1() && rhs->astOperand1()->variable() && rhs->astOperand1()->variable()->isLocal())
                    localmember = true;
                return ChildrenToVisit::op1_and_op2;
            });
            if (localmember)
                continue;

            bool allowed = true;
            visitAstNodes(tok->next()->astOperand2(),
            [&](const Token *tok2) {
                const Variable* var2 = tok2->variable();
                if (var2) {
                    if (var2->scope() == owner && tok2->strAt(-1)!=".") { // Is there a dependency between two member variables?
                        allowed = false;
                        return ChildrenToVisit::done;
                    } else if (var2->isArray() && var2->isLocal()) { // Can't initialize with a local array
                        allowed = false;
                        return ChildrenToVisit::done;
                    }
                } else if (tok2->str() == "this") { // 'this' instance is not completely constructed in initialization list
                    allowed = false;
                    return ChildrenToVisit::done;
                } else if (Token::Match(tok2, "%name% (") && tok2->strAt(-1) != "." && isMemberFunc(owner, tok2)) { // Member function called?
                    allowed = false;
                    return ChildrenToVisit::done;
                }
                return ChildrenToVisit::op1_and_op2;
            });
            if (!allowed)
                continue;

            suggestInitializationList(tok, tok->str());
        }
    }
}

void CheckClass::suggestInitializationList(const Token* tok, const std::string& varname)
{
    reportError(tok, Severity::performance, "useInitializationList", "$symbol:" + varname + "\nVariable '$symbol' is assigned in constructor body. Consider performing initialization in initialization list.\n"
                "When an object of a class is created, the constructors of all member variables are called consecutively "
                "in the order the variables are declared, even if you don't explicitly write them to the initialization list. You "
                "could avoid assigning '$symbol' a value by passing the value to the constructor in the initialization list.", CWE398, false);
}

//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

static bool checkFunctionUsage(const Function *privfunc, const Scope* scope)
{
    if (!scope)
        return true; // Assume it is used, if scope is not seen

    for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
        if (func->functionScope) {
            if (Token::Match(func->tokenDef, "%name% (")) {
                for (const Token *ftok = func->tokenDef->tokAt(2); ftok && ftok->str() != ")"; ftok = ftok->next()) {
                    if (Token::Match(ftok, "= %name% [(,)]") && ftok->strAt(1) == privfunc->name())
                        return true;
                    if (ftok->str() == "(")
                        ftok = ftok->link();
                }
            }
            for (const Token *ftok = func->functionScope->classDef->linkAt(1); ftok != func->functionScope->bodyEnd; ftok = ftok->next()) {
                if (ftok->function() == privfunc)
                    return true;
                if (ftok->varId() == 0U && ftok->str() == privfunc->name()) // TODO: This condition should be redundant
                    return true;
            }
        } else if ((func->type != Function::eCopyConstructor &&
                    func->type != Function::eOperatorEqual) ||
                   func->access != AccessControl::Private) // Assume it is used, if a function implementation isn't seen, but empty private copy constructors and assignment operators are OK
            return true;
    }

    const std::map<std::string, Type*>::const_iterator end = scope->definedTypesMap.end();
    for (std::map<std::string, Type*>::const_iterator iter = scope->definedTypesMap.begin(); iter != end; ++ iter) {
        const Type *type = (*iter).second;
        if (type->enclosingScope == scope && checkFunctionUsage(privfunc, type->classScope))
            return true;
    }

    for (const Variable &var : scope->varlist) {
        if (var.isStatic()) {
            const Token* tok = Token::findmatch(scope->bodyEnd, "%varid% =|(|{", var.declarationId());
            if (tok)
                tok = tok->tokAt(2);
            while (tok && tok->str() != ";") {
                if (tok->function() == privfunc)
                    return true;
                tok = tok->next();
            }
        }
    }

    return false; // Unused in this scope
}

void CheckClass::privateFunctions()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        // do not check borland classes with properties..
        if (Token::findsimplematch(scope->bodyStart, "; __property ;", scope->bodyEnd))
            continue;

        std::list<const Function*> privateFuncs;
        for (const Function &func : scope->functionList) {
            // Get private functions..
            if (func.type == Function::eFunction && func.access == AccessControl::Private && !func.isOperator()) // TODO: There are smarter ways to check private operator usage
                privateFuncs.push_back(&func);
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
            // Check that all private functions are used
            bool used = checkFunctionUsage(privateFuncs.front(), scope); // Usage in this class
            // Check in friend classes
            const std::vector<Type::FriendInfo>& friendList = scope->definedType->friendList;
            for (int i = 0; i < friendList.size() && !used; i++) {
                if (friendList[i].type)
                    used = checkFunctionUsage(privateFuncs.front(), friendList[i].type->classScope);
                else
                    used = true; // Assume, it is used if we do not see friend class
            }

            if (!used)
                unusedPrivateFunctionError(privateFuncs.front()->tokenDef, scope->className, privateFuncs.front()->name());

            privateFuncs.pop_front();
        }
    }
}

void CheckClass::unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "unusedPrivateFunction", "$symbol:" + classname + "::" + funcname + "\nUnused private function: '$symbol'", CWE398, false);
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
    return nullptr;
}

void CheckClass::checkMemset()
{
    const bool printWarnings = mSettings->isEnabled(Settings::WARNING);
    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|memcpy|memmove (")) {
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
                    int numIndirToVariableType = 0; // Offset to the actual type in terms of dereference/addressof
                    for (;; arg1 = arg1->next()) {
                        if (arg1->str() == "&")
                            ++numIndirToVariableType;
                        else if (arg1->str() == "*")
                            --numIndirToVariableType;
                        else
                            break;
                    }

                    const Variable * const var = arg1->variable();
                    if (var && arg1->strAt(1) == ",") {
                        if (var->isArrayOrPointer()) {
                            const Token *endTok = var->typeEndToken();
                            while (Token::simpleMatch(endTok, "*")) {
                                ++numIndirToVariableType;
                                endTok = endTok->previous();
                            }
                        }

                        if (var->isArray())
                            numIndirToVariableType += int(var->dimensions().size());

                        if (numIndirToVariableType == 1)
                            type = var->typeScope();
                    }
                }

                // No type defined => The tokens didn't match
                if (!typeTok && !type)
                    continue;

                if (typeTok && typeTok->str() == "(")
                    typeTok = typeTok->next();

                if (!type && typeTok->type())
                    type = typeTok->type()->classScope;

                if (type) {
                    const std::set<const Scope *> parsedTypes;
                    checkMemsetType(scope, tok, type, false, parsedTypes);
                }
            } else if (tok->variable() && tok->variable()->typeScope() && Token::Match(tok, "%var% = calloc|malloc|realloc|g_malloc|g_try_malloc|g_realloc|g_try_realloc (")) {
                const std::set<const Scope *> parsedTypes;
                checkMemsetType(scope, tok->tokAt(2), tok->variable()->typeScope(), true, parsedTypes);

                if (printWarnings && tok->variable()->typeScope()->numConstructors > 0)
                    mallocOnClassWarning(tok, tok->strAt(2), tok->variable()->typeScope()->classDef);
            }
        }
    }
}

void CheckClass::checkMemsetType(const Scope *start, const Token *tok, const Scope *type, bool allocation, std::set<const Scope *> parsedTypes)
{
    // If type has been checked there is no need to check it again
    if (parsedTypes.find(type) != parsedTypes.end())
        return;
    parsedTypes.insert(type);

    const bool printPortability = mSettings->isEnabled(Settings::PORTABILITY);

    // recursively check all parent classes
    for (const Type::BaseInfo & i : type->definedType->derivedFrom) {
        const Type* derivedFrom = i.type;
        if (derivedFrom && derivedFrom->classScope)
            checkMemsetType(start, tok, derivedFrom->classScope, allocation, parsedTypes);
    }

    // Warn if type is a class that contains any virtual functions
    for (const Function &func : type->functionList) {
        if (func.hasVirtualSpecifier()) {
            if (allocation)
                mallocOnClassError(tok, tok->str(), type->classDef, "virtual function");
            else
                memsetError(tok, tok->str(), "virtual function", type->classDef->str());
        }
    }

    // Warn if type is a class or struct that contains any std::* variables
    for (const Variable &var : type->varlist) {
        if (var.isReference() && !var.isStatic()) {
            memsetErrorReference(tok, tok->str(), type->classDef->str());
            continue;
        }
        // don't warn if variable static or const, pointer or array of pointers
        if (!var.isStatic() && !var.isConst() && !var.isPointer() && (!var.isArray() || var.typeEndToken()->str() != "*")) {
            const Token *tok1 = var.typeStartToken();
            const Scope *typeScope = var.typeScope();

            std::string typeName;
            if (Token::Match(tok1, "%type% ::")) {
                const Token *typeTok = tok1;
                while (Token::Match(typeTok, "%type% ::")) {
                    typeName += typeTok->str() + "::";
                    typeTok = typeTok->tokAt(2);
                }
                typeName += typeTok->str();
            }

            // check for std:: type
            if (var.isStlType() && typeName != "std::array" && !mSettings->library.podtype(typeName)) {
                if (allocation)
                    mallocOnClassError(tok, tok->str(), type->classDef, "'" + typeName + "'");
                else
                    memsetError(tok, tok->str(), "'" + typeName + "'", type->classDef->str());
            }

            // check for known type
            else if (typeScope && typeScope != type)
                checkMemsetType(start, tok, typeScope, allocation, parsedTypes);

            // check for float
            else if (printPortability && var.isFloatingType() && tok->str() == "memset")
                memsetErrorFloat(tok, type->classDef->str());
        }
    }
}

void CheckClass::mallocOnClassWarning(const Token* tok, const std::string &memfunc, const Token* classTok)
{
    std::list<const Token *> toks = { tok, classTok };
    reportError(toks, Severity::warning, "mallocOnClassWarning",
                "$symbol:" + memfunc +"\n"
                "Memory for class instance allocated with $symbol(), but class provides constructors.\n"
                "Memory for class instance allocated with $symbol(), but class provides constructors. This is unsafe, "
                "since no constructor is called and class members remain uninitialized. Consider using 'new' instead.", CWE762, false);
}

void CheckClass::mallocOnClassError(const Token* tok, const std::string &memfunc, const Token* classTok, const std::string &classname)
{
    std::list<const Token *> toks = { tok, classTok };
    reportError(toks, Severity::error, "mallocOnClassError",
                "$symbol:" + memfunc +"\n"
                "$symbol:" + classname +"\n"
                "Memory for class instance allocated with " + memfunc + "(), but class contains a " + classname + ".\n"
                "Memory for class instance allocated with " + memfunc + "(), but class a " + classname + ". This is unsafe, "
                "since no constructor is called and class members remain uninitialized. Consider using 'new' instead.", CWE665, false);
}

void CheckClass::memsetError(const Token *tok, const std::string &memfunc, const std::string &classname, const std::string &type)
{
    reportError(tok, Severity::error, "memsetClass",
                "$symbol:" + memfunc +"\n"
                "$symbol:" + classname +"\n"
                "Using '" + memfunc + "' on " + type + " that contains a " + classname + ".\n"
                "Using '" + memfunc + "' on " + type + " that contains a " + classname + " is unsafe, because constructor, destructor "
                "and copy operator calls are omitted. These are necessary for this non-POD type to ensure that a valid object "
                "is created.", CWE762, false);
}

void CheckClass::memsetErrorReference(const Token *tok, const std::string &memfunc, const std::string &type)
{
    reportError(tok, Severity::error, "memsetClassReference",
                "$symbol:" + memfunc +"\n"
                "Using '" + memfunc + "' on " + type + " that contains a reference.", CWE665, false);
}

void CheckClass::memsetErrorFloat(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::portability, "memsetClassFloat", "Using memset() on " + type + " which contains a floating point number.\n"
                "Using memset() on " + type + " which contains a floating point number."
                " This is not portable because memset() sets each byte of a block of memory to a specific value and"
                " the actual representation of a floating-point value is implementation defined."
                " Note: In case of an IEEE754-1985 compatible implementation setting all bits to zero results in the value 0.0.", CWE758, false);
}


//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C&) { ... return *this; }"
// operator= should return a reference to *this
//---------------------------------------------------------------------------

void CheckClass::operatorEqRetRefThis()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->type == Function::eOperatorEqual && func->hasBody()) {
                // make sure return signature is correct
                if (func->retType == func->nestedIn->definedType && func->tokenDef->strAt(-1) == "&") {
                    checkReturnPtrThis(scope, &(*func), func->functionScope->bodyStart, func->functionScope->bodyEnd);
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
        if (tok->str() != "return")
            continue;

        foundReturn = true;

        const Token *retExpr = tok->astOperand1();
        if (retExpr && retExpr->str() == "=")
            retExpr = retExpr->astOperand1();
        if (retExpr && retExpr->isUnaryOp("*") && Token::simpleMatch(retExpr->astOperand1(), "this"))
            continue;

        std::string cast("( " + scope->className + " & )");
        if (Token::simpleMatch(tok->next(), cast.c_str(), cast.size()))
            tok = tok->tokAt(4);

        // check if a function is called
        if (tok->strAt(2) == "(" &&
            tok->linkAt(2)->next()->str() == ";") {
            // check if it is a member function
            for (std::list<Function>::const_iterator it = scope->functionList.begin(); it != scope->functionList.end(); ++it) {
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
        else if (!(Token::simpleMatch(tok->next(), "operator= (") ||
                   Token::simpleMatch(tok->next(), "this . operator= (") ||
                   (Token::Match(tok->next(), "%type% :: operator= (") &&
                    tok->next()->str() == scope->className)))
            operatorEqRetRefThisError(func->token);
    }
    if (foundReturn) {
        return;
    }
    if (startTok->next() == last) {
        const std::string tmp("( const " + scope->className + " &");
        if (Token::simpleMatch(func->argDef, tmp.c_str(), tmp.size())) {
            // Typical wrong way to suppress default assignment operator by declaring it and leaving empty
            operatorEqMissingReturnStatementError(func->token, func->access == AccessControl::Public);
        } else {
            operatorEqMissingReturnStatementError(func->token, true);
        }
        return;
    }
    if (mSettings->library.isScopeNoReturn(last, nullptr)) {
        // Typical wrong way to prohibit default assignment operator
        // by always throwing an exception or calling a noreturn function
        operatorEqShouldBeLeftUnimplementedError(func->token);
        return;
    }

    operatorEqMissingReturnStatementError(func->token, func->access == AccessControl::Public);
}

void CheckClass::operatorEqRetRefThisError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqRetRefThis", "'operator=' should return reference to 'this' instance.", CWE398, false);
}

void CheckClass::operatorEqShouldBeLeftUnimplementedError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqShouldBeLeftUnimplemented", "'operator=' should either return reference to 'this' instance or be declared private and left unimplemented.", CWE398, false);
}

void CheckClass::operatorEqMissingReturnStatementError(const Token *tok, bool error)
{
    if (error) {
        reportError(tok, Severity::error, "operatorEqMissingReturnStatement", "No 'return' statement in non-void function causes undefined behavior.", CWE398, false);
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
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        // skip classes with multiple inheritance
        if (scope->definedType->derivedFrom.size() > 1)
            continue;

        for (const Function &func : scope->functionList) {
            if (func.type == Function::eOperatorEqual && func.hasBody()) {
                // make sure that the operator takes an object of the same type as *this, otherwise we can't detect self-assignment checks
                if (func.argumentList.empty())
                    continue;
                const Token* typeTok = func.argumentList.front().typeEndToken();
                while (typeTok->str() == "const" || typeTok->str() == "&" || typeTok->str() == "*")
                    typeTok = typeTok->previous();
                if (typeTok->str() != scope->className)
                    continue;

                // make sure return signature is correct
                if (Token::Match(func.retDef, "%type% &") && func.retDef->str() == scope->className) {
                    // find the parameter name
                    const Token *rhs = func.argumentList.begin()->nameToken();

                    if (!hasAssignSelf(&func, rhs)) {
                        if (hasAllocation(&func, scope))
                            operatorEqToSelfError(func.token);
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
    const Token *last = func->functionScope->bodyEnd;
    for (const Token *tok = func->functionScope->bodyStart; tok && (tok != last); tok = tok->next()) {
        if (Token::Match(tok, "%var% = malloc|realloc|calloc|new") && isMemberVar(scope, tok))
            return true;

        // check for deallocating memory
        const Token *var;
        if (Token::Match(tok, "free ( %var%"))
            var = tok->tokAt(2);
        else if (Token::Match(tok, "delete [ ] %var%"))
            var = tok->tokAt(3);
        else if (Token::Match(tok, "delete %var%"))
            var = tok->next();
        else
            continue;
        // Check for assignment to the deleted pointer (only if its a member of the class)
        if (isMemberVar(scope, var)) {
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
    if (!rhs)
        return false;
    const Token *last = func->functionScope->bodyEnd;
    for (const Token *tok = func->functionScope->bodyStart; tok && tok != last; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "if ("))
            continue;

        bool ret = false;
        visitAstNodes(tok->next()->astOperand2(),
        [&](const Token *tok2) {
            if (!Token::Match(tok2, "==|!="))
                return ChildrenToVisit::op1_and_op2;
            if (Token::simpleMatch(tok2->astOperand1(), "this"))
                tok2 = tok2->astOperand2();
            else if (Token::simpleMatch(tok2->astOperand2(), "this"))
                tok2 = tok2->astOperand1();
            else
                return ChildrenToVisit::op1_and_op2;
            if (tok2 && tok2->isUnaryOp("&") && tok2->astOperand1()->str() == rhs->str())
                ret = true;
            return ret ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
        });
        if (ret)
            return ret;
    }

    return false;
}

void CheckClass::operatorEqToSelfError(const Token *tok)
{
    reportError(tok, Severity::warning, "operatorEqToSelf",
                "'operator=' should check for assignment to self to avoid problems with dynamic memory.\n"
                "'operator=' should check for assignment to self to ensure that each block of dynamically "
                "allocated memory is owned and managed by only one instance of the class.", CWE398, false);
}

//---------------------------------------------------------------------------
// A destructor in a base class should be virtual
//---------------------------------------------------------------------------

void CheckClass::virtualDestructor()
{
    // This error should only be given if:
    // * base class doesn't have virtual destructor
    // * derived class has non-empty destructor (only c++03, in c++11 it's UB see paragraph 3 in [expr.delete])
    // * base class is deleted
    // unless inconclusive in which case:
    // * A class with any virtual functions should have a destructor that is either public and virtual or protected
    const bool printInconclusive = mSettings->inconclusive;

    std::list<const Function *> inconclusiveErrors;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        // Skip base classes (unless inconclusive)
        if (scope->definedType->derivedFrom.empty()) {
            if (printInconclusive) {
                const Function *destructor = scope->getDestructor();
                if (destructor && !destructor->hasVirtualSpecifier() && destructor->access == AccessControl::Public) {
                    for (const Function &func : scope->functionList) {
                        if (func.hasVirtualSpecifier()) {
                            inconclusiveErrors.push_back(destructor);
                            break;
                        }
                    }
                }
            }
            continue;
        }

        // Check if destructor is empty and non-empty ..
        if (mSettings->standards.cpp <= Standards::CPP03) {
            // Find the destructor
            const Function *destructor = scope->getDestructor();

            // Check for destructor with implementation
            if (!destructor || !destructor->hasBody())
                continue;

            // Empty destructor
            if (destructor->token->linkAt(3) == destructor->token->tokAt(4))
                continue;
        }

        const Token *derived = scope->classDef;
        const Token *derivedClass = derived->next();

        // Iterate through each base class...
        for (const Type::BaseInfo & j : scope->definedType->derivedFrom) {
            // Check if base class is public and exists in database
            if (j.access != AccessControl::Private && j.type) {
                const Type *derivedFrom = j.type;
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
                    std::set<int> baseClassPointers;

                    for (const Variable* var : mSymbolDatabase->variableList()) {
                        if (var && var->isPointer() && var->type() == derivedFrom)
                            baseClassPointers.insert(var->declarationId());
                    }

                    // pointer variables of type 'Base *' that should not be deleted
                    std::set<int> dontDelete;

                    // No deletion of derived class instance through base class pointer found => the code is ok
                    bool ok = true;

                    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
                        if (Token::Match(tok, "[;{}] %var% =") &&
                            baseClassPointers.find(tok->next()->varId()) != baseClassPointers.end()) {
                            // new derived class..
                            const std::string tmp("new " + derivedClass->str());
                            if (Token::simpleMatch(tok->tokAt(3), tmp.c_str(), tmp.size())) {
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
                const Function *baseDestructor = derivedFromScope->getDestructor();

                // Check that there is a destructor..
                if (!baseDestructor) {
                    if (derivedFrom->derivedFrom.empty()) {
                        virtualDestructorError(derivedFrom->classDef, derivedFrom->name(), derivedClass->str(), false);
                    }
                } else if (!baseDestructor->hasVirtualSpecifier()) {
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
                        if (baseDestructor->access == AccessControl::Public) {
                            virtualDestructorError(baseDestructor->token, derivedFrom->name(), derivedClass->str(), false);
                            // check for duplicate error and remove it if found
                            const std::list<const Function *>::iterator found = find(inconclusiveErrors.begin(), inconclusiveErrors.end(), baseDestructor);
                            if (found != inconclusiveErrors.end())
                                inconclusiveErrors.erase(found);
                        }
                    }
                }
            }
        }
    }

    for (const Function *func : inconclusiveErrors)
        virtualDestructorError(func->tokenDef, func->name(), emptyString, true);
}

void CheckClass::virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived, bool inconclusive)
{
    if (inconclusive) {
        if (mSettings->isEnabled(Settings::WARNING))
            reportError(tok, Severity::warning, "virtualDestructor", "$symbol:" + Base + "\nClass '$symbol' which has virtual members does not have a virtual destructor.", CWE404, true);
    } else {
        reportError(tok, Severity::error, "virtualDestructor",
                    "$symbol:" + Base +"\n"
                    "$symbol:" + Derived +"\n"
                    "Class '" + Base + "' which is inherited by class '" + Derived + "' does not have a virtual destructor.\n"
                    "Class '" + Base + "' which is inherited by class '" + Derived + "' does not have a virtual destructor. "
                    "If you destroy instances of the derived class by deleting a pointer that points to the base class, only "
                    "the destructor of the base class is executed. Thus, dynamic memory that is managed by the derived class "
                    "could leak. This can be avoided by adding a virtual destructor to the base class.", CWE404, false);
    }
}

//---------------------------------------------------------------------------
// warn for "this-x". The indented code may be "this->x"
//---------------------------------------------------------------------------

void CheckClass::thisSubtraction()
{
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    const Token *tok = mTokenizer->tokens();
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
    reportError(tok, Severity::warning, "thisSubtraction", "Suspicious pointer subtraction. Did you intend to write '->'?", CWE398, false);
}

//---------------------------------------------------------------------------
// can member function be const?
//---------------------------------------------------------------------------

void CheckClass::checkConst()
{
    // This is an inconclusive check. False positives: #3322.
    if (!mSettings->inconclusive)
        return;

    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        for (const Function &func : scope->functionList) {
            // does the function have a body?
            if (func.type != Function::eFunction || !func.hasBody())
                continue;
            // don't warn for friend/static/virtual functions
            if (func.isFriend() || func.isStatic() || func.hasVirtualSpecifier())
                continue;

            // don't warn when returning non-const pointer/reference
            {
                bool isPointerOrReference = false;
                for (const Token *typeToken = func.retDef; typeToken; typeToken = typeToken->next()) {
                    if (Token::Match(typeToken, "(|{|;"))
                        break;
                    if (!isPointerOrReference && typeToken->str() == "const")
                        break;
                    if (Token::Match(typeToken, "*|&")) {
                        isPointerOrReference = true;
                        break;
                    }
                }
                if (isPointerOrReference)
                    continue;
            }


            if (func.isOperator()) { // Operator without return type: conversion operator
                const std::string& opName = func.tokenDef->str();
                if (opName.compare(8, 5, "const") != 0 && (endsWith(opName,'&') || endsWith(opName,'*')))
                    continue;
            } else if (mSettings->library.isSmartPointer(func.retDef)) {
                // Don't warn if a std::shared_ptr etc is returned
                continue;
            } else {
                // don't warn for unknown types..
                // LPVOID, HDC, etc
                if (func.retDef->str().size() > 2 && !func.retDef->type() && func.retDef->isUpperCaseName())
                    continue;
            }

            // check if base class function is virtual
            if (!scope->definedType->derivedFrom.empty() && func.isImplicitlyVirtual(true))
                continue;

            bool memberAccessed = false;
            // if nothing non-const was found. write error..
            if (!checkConstFunc(scope, &func, memberAccessed))
                continue;

            if (func.isConst() && (memberAccessed || func.isOperator()))
                continue;

            std::string classname = scope->className;
            const Scope *nest = scope->nestedIn;
            while (nest && nest->type != Scope::eGlobal) {
                classname = std::string(nest->className + "::" + classname);
                nest = nest->nestedIn;
            }

            // get function name
            std::string functionName = (func.tokenDef->isName() ? "" : "operator") + func.tokenDef->str();

            if (func.tokenDef->str() == "(")
                functionName += ")";
            else if (func.tokenDef->str() == "[")
                functionName += "]";

            if (func.isInline())
                checkConstError(func.token, classname, functionName, !memberAccessed && !func.isOperator());
            else // not inline
                checkConstError2(func.token, func.tokenDef, classname, functionName, !memberAccessed && !func.isOperator());
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

    for (const Variable &var : scope->varlist) {
        if (var.name() == tok->str()) {
            if (tok->varId() == 0)
                mSymbolDatabase->debugMessage(tok, "varid0", "CheckClass::isMemberVar found used member variable \'" + tok->str() + "\' with varid 0");

            return !var.isStatic();
        }
    }

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
            // find the base class
            const Type *derivedFrom = i.type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope && derivedFrom->classScope != scope) {
                if (isMemberVar(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::isMemberFunc(const Scope *scope, const Token *tok) const
{
    if (!tok->function()) {
        for (const Function &func : scope->functionList) {
            if (func.name() == tok->str()) {
                const Token* tok2 = tok->tokAt(2);
                int argsPassed = tok2->str() == ")" ? 0 : 1;
                for (;;) {
                    tok2 = tok2->nextArgument();
                    if (tok2)
                        argsPassed++;
                    else
                        break;
                }
                if (argsPassed == func.argCount() ||
                    (func.isVariadic() && argsPassed >= (func.argCount() - 1)) ||
                    (argsPassed < func.argCount() && argsPassed >= func.minArgCount()))
                    return true;
            }
        }
    } else if (tok->function()->nestedIn == scope)
        return !tok->function()->isStatic();

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
            // find the base class
            const Type *derivedFrom = i.type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope && derivedFrom->classScope != scope) {
                if (isMemberFunc(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::isConstMemberFunc(const Scope *scope, const Token *tok) const
{
    if (!tok->function())
        return false;
    else if (tok->function()->nestedIn == scope)
        return tok->function()->isConst();

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
            // find the base class
            const Type *derivedFrom = i.type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope) {
                if (isConstMemberFunc(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}


// The container contains the STL types whose operator[] is not a const.
static const std::set<std::string> stl_containers_not_const = { "map", "unordered_map" };

bool CheckClass::checkConstFunc(const Scope *scope, const Function *func, bool& memberAccessed) const
{
    if (mTokenizer->hasIfdef(func->functionScope->bodyStart, func->functionScope->bodyEnd))
        return false;

    // if the function doesn't have any assignment nor function call,
    // it can be a const function..
    for (const Token *tok1 = func->functionScope->bodyStart; tok1 && tok1 != func->functionScope->bodyEnd; tok1 = tok1->next()) {
        if (tok1->isName() && isMemberVar(scope, tok1)) {
            memberAccessed = true;
            const Variable* v = tok1->variable();
            if (v && v->isMutable())
                continue;

            if (tok1->str() == "this" && tok1->previous()->isAssignmentOp())
                return false;

            // non const pointer cast
            if (tok1->valueType() && tok1->valueType()->pointer > 0 && tok1->astParent() && tok1->astParent()->isCast() && !Token::simpleMatch(tok1->astParent(), "( const"))
                return false;

            const Token* lhs = tok1->previous();
            if (lhs->str() == "&") {
                lhs = lhs->previous();
                if (lhs->isAssignmentOp() && lhs->previous()->variable()) {
                    if (lhs->previous()->variable()->typeStartToken()->strAt(-1) != "const" && lhs->previous()->variable()->isPointer())
                        return false;
                }
            } else if (lhs->str() == ":" && lhs->astParent() && lhs->astParent()->str() == "(") { // range-based for-loop (C++11)
                // TODO: We could additionally check what is done with the elements to avoid false negatives. Here we just rely on "const" keyword being used.
                if (lhs->astParent()->strAt(1) != "const")
                    return false;
            } else {
                if (lhs->isAssignmentOp()) {
                    const Variable* lhsVar = lhs->previous()->variable();
                    if (lhsVar && !lhsVar->isConst() && lhsVar->isReference() && lhs == lhsVar->nameToken()->next())
                        return false;
                }
            }

            const Token* jumpBackToken = nullptr;
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
                        if (var && var->isStlType(stl_containers_not_const))
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
                    return false;
            }

            // Assignment
            else if (end->next()->isAssignmentOp())
                return false;

            // Streaming
            else if (end->strAt(1) == "<<" && tok1->strAt(-1) != "<<")
                return false;
            else if (isLikelyStreamRead(true, tok1->previous()))
                return false;

            // ++/--
            else if (end->next()->tokType() == Token::eIncDecOp || tok1->previous()->tokType() == Token::eIncDecOp)
                return false;


            const Token* start = tok1;
            while (tok1->strAt(-1) == ")")
                tok1 = tok1->linkAt(-1);

            if (start->strAt(-1) == "delete")
                return false;

            tok1 = jumpBackToken?jumpBackToken:end; // Jump back to first [ to check inside, or jump to end of expression
        }

        // streaming: <<
        else if (Token::simpleMatch(tok1->previous(), ") <<") &&
                 isMemberVar(scope, tok1->tokAt(-2))) {
            const Variable* var = tok1->tokAt(-2)->variable();
            if (!var || !var->isMutable())
                return false;
        }

        // streaming: >> *this
        else if (Token::simpleMatch(tok1, ">> * this") && isLikelyStreamRead(true, tok1)) {
            return false;
        }

        // function call..
        else if (Token::Match(tok1, "%name% (") && !tok1->isStandardType() &&
                 !Token::Match(tok1, "return|if|string|switch|while|catch|for")) {
            if (isMemberFunc(scope, tok1) && tok1->strAt(-1) != ".") {
                if (!isConstMemberFunc(scope, tok1))
                    return false;
                memberAccessed = true;
            }
            // Member variable given as parameter
            const Token *lpar = tok1->next();
            if (Token::simpleMatch(lpar, "( ) ("))
                lpar = lpar->tokAt(2);
            for (const Token* tok2 = lpar->next(); tok2 && tok2 != tok1->next()->link(); tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->isName() && isMemberVar(scope, tok2)) {
                    const Variable* var = tok2->variable();
                    if (!var || !var->isMutable())
                        return false; // TODO: Only bailout if function takes argument as non-const reference
                }
            }
        } else if (Token::simpleMatch(tok1, "> (") && (!tok1->link() || !Token::Match(tok1->link()->previous(), "static_cast|const_cast|dynamic_cast|reinterpret_cast"))) {
            return false;
        }
    }

    return true;
}

void CheckClass::checkConstError(const Token *tok, const std::string &classname, const std::string &funcname, bool suggestStatic)
{
    checkConstError2(tok, nullptr, classname, funcname, suggestStatic);
}

void CheckClass::checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname, bool suggestStatic)
{
    std::list<const Token *> toks;
    toks.push_back(tok1);
    if (tok2)
        toks.push_back(tok2);
    if (!suggestStatic)
        reportError(toks, Severity::style, "functionConst",
                    "$symbol:" + classname + "::" + funcname +"\n"
                    "Technically the member function '$symbol' can be const.\n"
                    "The member function '$symbol' can be made a const "
                    "function. Making this function 'const' should not cause compiler errors. "
                    "Even though the function can be made const function technically it may not make "
                    "sense conceptually. Think about your design and the task of the function first - is "
                    "it a function that must not change object internal state?", CWE398, true);
    else
        reportError(toks, Severity::performance, "functionStatic",
                    "$symbol:" + classname + "::" + funcname +"\n"
                    "Technically the member function '$symbol' can be static (but you may consider moving to unnamed namespace).\n"
                    "The member function '$symbol' can be made a static "
                    "function. Making a function static can bring a performance benefit since no 'this' instance is "
                    "passed to the function. This change should not cause compiler errors but it does not "
                    "necessarily make sense conceptually. Think about your design and the task of the function first - "
                    "is it a function that must not access members of class instances? And maybe it is more appropriate "
                    "to move this function to a unnamed namespace.", CWE398, true);
}

//---------------------------------------------------------------------------
// ClassCheck: Check that initializer list is in declared order.
//---------------------------------------------------------------------------

namespace { // avoid one-definition-rule violation
    struct VarInfo {
        VarInfo(const Variable *_var, const Token *_tok)
            : var(_var), tok(_tok) { }

        const Variable *var;
        const Token *tok;
    };
}

void CheckClass::initializerListOrder()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    // This check is not inconclusive.  However it only determines if the initialization
    // order is incorrect.  It does not determine if being out of order causes
    // a real error.  Out of order is not necessarily an error but you can never
    // have an error if the list is in order so this enforces defensive programming.
    if (!mSettings->inconclusive)
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        // iterate through all member functions looking for constructors
        for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
            if (func->isConstructor() && func->hasBody()) {
                // check for initializer list
                const Token *tok = func->arg->link()->next();

                if (tok->str() == ":") {
                    std::vector<VarInfo> vars;
                    tok = tok->next();

                    // find all variable initializations in list
                    while (tok && tok != func->functionScope->bodyStart) {
                        if (Token::Match(tok, "%name% (|{")) {
                            const Variable *var = scope->getVariable(tok->str());
                            if (var)
                                vars.emplace_back(var, tok);

                            if (Token::Match(tok->tokAt(2), "%name% =")) {
                                var = scope->getVariable(tok->strAt(2));

                                if (var)
                                    vars.emplace_back(var, tok->tokAt(2));
                            }
                            tok = tok->next()->link()->next();
                        } else
                            tok = tok->next();
                    }

                    // need at least 2 members to have out of order initialization
                    for (int j = 1; j < vars.size(); j++) {
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
    std::list<const Token *> toks = { tok1, tok2 };
    reportError(toks, Severity::style, "initializerList",
                "$symbol:" + classname + "::" + varname +"\n"
                "Member variable '$symbol' is in the wrong place in the initializer list.\n"
                "Member variable '$symbol' is in the wrong place in the initializer list. "
                "Members are initialized in the order they are declared, not in the "
                "order they are in the initializer list.  Keeping the initializer list "
                "in the same order that the members were declared prevents order dependent "
                "initialization errors.", CWE398, true);
}


//---------------------------------------------------------------------------
// Check for self initialization in initialization list
//---------------------------------------------------------------------------

void CheckClass::checkSelfInitialization()
{
    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        const Function* function = scope->function;
        if (!function || !function->isConstructor())
            continue;

        const Token* tok = function->arg->link()->next();
        if (tok->str() != ":")
            continue;

        for (; tok != scope->bodyStart; tok = tok->next()) {
            if (Token::Match(tok, "[:,] %var% (|{ %var% )|}") && tok->next()->varId() == tok->tokAt(3)->varId()) {
                selfInitializationError(tok, tok->strAt(1));
            }
        }
    }
}

void CheckClass::selfInitializationError(const Token* tok, const std::string& varname)
{
    reportError(tok, Severity::error, "selfInitialization", "$symbol:" + varname + "\nMember variable '$symbol' is initialized by itself.", CWE665, false);
}


//---------------------------------------------------------------------------
// Check for virtual function calls in constructor/destructor
//---------------------------------------------------------------------------

void CheckClass::checkVirtualFunctionCallInConstructor()
{
    if (! mSettings->isEnabled(Settings::WARNING))
        return;
    std::map<const Function *, std::list<const Token *> > virtualFunctionCallsMap;
    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        if (scope->function == nullptr || !scope->function->hasBody() ||
            !(scope->function->isConstructor() ||
              scope->function->isDestructor()))
            continue;

        const std::list<const Token *> & virtualFunctionCalls = getVirtualFunctionCalls(*scope->function, virtualFunctionCallsMap);
        for (const Token *callToken : virtualFunctionCalls) {
            std::list<const Token *> callstack(1, callToken);
            getFirstVirtualFunctionCallStack(virtualFunctionCallsMap, callToken, callstack);
            if (callstack.empty())
                continue;
            if (callstack.back()->function()->isPure())
                pureVirtualFunctionCallInConstructorError(scope->function, callstack, callstack.back()->str());
            else
                virtualFunctionCallInConstructorError(scope->function, callstack, callstack.back()->str());
        }
    }
}

const std::list<const Token *> & CheckClass::getVirtualFunctionCalls(const Function & function,
        std::map<const Function *, std::list<const Token *> > & virtualFunctionCallsMap)
{
    const std::map<const Function *, std::list<const Token *> >::const_iterator found = virtualFunctionCallsMap.find(&function);
    if (found != virtualFunctionCallsMap.end())
        return found->second;

    virtualFunctionCallsMap[&function] = std::list<const Token *>();
    std::list<const Token *> & virtualFunctionCalls = virtualFunctionCallsMap.find(&function)->second;

    if (!function.hasBody())
        return virtualFunctionCalls;

    for (const Token *tok = function.arg->link(); tok != function.functionScope->bodyEnd; tok = tok->next()) {
        if (function.type != Function::eConstructor &&
            function.type != Function::eCopyConstructor &&
            function.type != Function::eMoveConstructor &&
            function.type != Function::eDestructor) {
            if ((Token::simpleMatch(tok, ") {") && tok->link() && Token::Match(tok->link()->previous(), "if|switch")) ||
                Token::simpleMatch(tok, "else {")) {
                // Assume pure virtual function call is prevented by "if|else|switch" condition
                tok = tok->linkAt(1);
                continue;
            }
        }
        if (tok->scope()->type == Scope::eLambda)
            tok = tok->scope()->bodyEnd->next();

        const Function * callFunction = tok->function();
        if (!callFunction ||
            function.nestedIn != callFunction->nestedIn ||
            (tok->previous() && tok->previous()->str() == "."))
            continue;

        if (tok->previous() &&
            tok->previous()->str() == "(") {
            const Token * prev = tok->previous();
            if (prev->previous() &&
                (mSettings->library.ignorefunction(tok->str())
                 || mSettings->library.ignorefunction(prev->previous()->str())))
                continue;
        }

        if (callFunction->isImplicitlyVirtual()) {
            if (!callFunction->isPure() && Token::simpleMatch(tok->previous(), "::"))
                continue;
            virtualFunctionCalls.push_back(tok);
            continue;
        }

        const std::list<const Token *> & virtualFunctionCallsOfTok = getVirtualFunctionCalls(*callFunction, virtualFunctionCallsMap);
        if (!virtualFunctionCallsOfTok.empty())
            virtualFunctionCalls.push_back(tok);
    }
    return virtualFunctionCalls;
}

void CheckClass::getFirstVirtualFunctionCallStack(
    std::map<const Function *, std::list<const Token *> > & virtualFunctionCallsMap,
    const Token * callToken,
    std::list<const Token *> & pureFuncStack)
{
    const Function *callFunction = callToken->function();
    if (callFunction->isImplicitlyVirtual() && (!callFunction->isPure() || !callFunction->hasBody())) {
        pureFuncStack.push_back(callFunction->tokenDef);
        return;
    }
    std::map<const Function *, std::list<const Token *> >::const_iterator found = virtualFunctionCallsMap.find(callFunction);
    if (found == virtualFunctionCallsMap.end() || found->second.empty()) {
        pureFuncStack.clear();
        return;
    }
    const Token * firstCall = *found->second.begin();
    pureFuncStack.push_back(firstCall);
    getFirstVirtualFunctionCallStack(virtualFunctionCallsMap, firstCall, pureFuncStack);
}

void CheckClass::virtualFunctionCallInConstructorError(
    const Function * scopeFunction,
    const std::list<const Token *> & tokStack,
    const std::string &funcname)
{
    const char * scopeFunctionTypeName = scopeFunction ? getFunctionTypeName(scopeFunction->type) : "constructor";

    ErrorPath errorPath;
    int lineNumber = 1;
    for (const Token *tok : tokStack)
        errorPath.emplace_back(tok, "Calling " + tok->str());
    if (!errorPath.empty()) {
        lineNumber = errorPath.front().first->linenr();
        errorPath.back().second = funcname + " is a virtual function";
    }

    std::string constructorName;
    if (scopeFunction) {
        const Token *endToken = scopeFunction->argDef->link()->next();
        if (scopeFunction->type == Function::Type::eDestructor)
            constructorName = "~";
        for (const Token *tok = scopeFunction->tokenDef; tok != endToken; tok = tok->next()) {
            if (!constructorName.empty() && Token::Match(tok->previous(), "%name%|%num% %name%|%num%"))
                constructorName += ' ';
            constructorName += tok->str();
            if (tok->str() == ")")
                break;
        }
    }

    reportError(errorPath, Severity::style, "virtualCallInConstructor",
                "Virtual function '" + funcname + "' is called from " + scopeFunctionTypeName + " '" + constructorName + "' at line " + MathLib::toString(lineNumber) + ". Dynamic binding is not used.", CWE(0U), false);
}

void CheckClass::pureVirtualFunctionCallInConstructorError(
    const Function * scopeFunction,
    const std::list<const Token *> & tokStack,
    const std::string &purefuncname)
{
    const char * scopeFunctionTypeName = scopeFunction ? getFunctionTypeName(scopeFunction->type) : "constructor";

    ErrorPath errorPath;
    for (const Token *tok : tokStack)
        errorPath.emplace_back(tok, "Calling " + tok->str());
    if (!errorPath.empty())
        errorPath.back().second = purefuncname + " is a pure virtual function without body";

    reportError(errorPath, Severity::warning, "pureVirtualCall",
                "$symbol:" + purefuncname +"\n"
                "Call of pure virtual function '$symbol' in " + scopeFunctionTypeName + ".\n"
                "Call of pure virtual function '$symbol' in " + scopeFunctionTypeName + ". The call will fail during runtime.", CWE(0U), false);
}


//---------------------------------------------------------------------------
// Check for members hiding inherited members with the same name
//---------------------------------------------------------------------------

void CheckClass::checkDuplInheritedMembers()
{
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    // Iterate over all classes
    for (const Type &classIt : mSymbolDatabase->typeList) {
        // Iterate over the parent classes
        checkDuplInheritedMembersRecursive(&classIt, &classIt);
    }
}

void CheckClass::checkDuplInheritedMembersRecursive(const Type* typeCurrent, const Type* typeBase)
{
    for (const Type::BaseInfo &parentClassIt : typeBase->derivedFrom) {
        // Check if there is info about the 'Base' class
        if (!parentClassIt.type || !parentClassIt.type->classScope)
            continue;
        // Check if they have a member variable in common
        for (const Variable &classVarIt : typeCurrent->classScope->varlist) {
            for (const Variable &parentClassVarIt : parentClassIt.type->classScope->varlist) {
                if (classVarIt.name() == parentClassVarIt.name() && !parentClassVarIt.isPrivate()) { // Check if the class and its parent have a common variable
                    duplInheritedMembersError(classVarIt.nameToken(), parentClassVarIt.nameToken(),
                                              typeCurrent->name(), parentClassIt.type->name(), classVarIt.name(),
                                              typeCurrent->classScope->type == Scope::eStruct,
                                              parentClassIt.type->classScope->type == Scope::eStruct);
                }
            }
        }
        if (typeCurrent != parentClassIt.type)
            checkDuplInheritedMembersRecursive(typeCurrent, parentClassIt.type);
    }
}

void CheckClass::duplInheritedMembersError(const Token *tok1, const Token* tok2,
        const std::string &derivedName, const std::string &baseName,
        const std::string &variableName, bool derivedIsStruct, bool baseIsStruct)
{
    ErrorPath errorPath;
    errorPath.emplace_back(tok2, "Parent variable '" + baseName + "::" + variableName + "'");
    errorPath.emplace_back(tok1, "Derived variable '" + derivedName + "::" + variableName + "'");

    const std::string symbols = "$symbol:" + derivedName + "\n$symbol:" + variableName + "\n$symbol:" + baseName;

    const std::string message = "The " + std::string(derivedIsStruct ? "struct" : "class") + " '" + derivedName +
                                "' defines member variable with name '" + variableName + "' also defined in its parent " +
                                std::string(baseIsStruct ? "struct" : "class") + " '" + baseName + "'.";
    reportError(errorPath, Severity::warning, "duplInheritedMember", symbols + '\n' + message, CWE398, false);
}


//---------------------------------------------------------------------------
// Check that copy constructor and operator defined together
//---------------------------------------------------------------------------

enum class CtorType {
    NO,
    WITHOUT_BODY,
    WITH_BODY
};

void CheckClass::checkCopyCtorAndEqOperator()
{
    // This is disabled because of #8388
    // The message must be clarified. How is the behaviour different?
    return;

    // cppcheck-suppress unreachableCode - remove when code is enabled again
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        bool hasNonStaticVars = false;
        for (std::list<Variable>::const_iterator var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
            if (!var->isStatic()) {
                hasNonStaticVars = true;
                break;
            }
        }
        if (!hasNonStaticVars)
            continue;

        CtorType copyCtors = CtorType::NO;
        bool moveCtor = false;
        CtorType assignmentOperators = CtorType::NO;

        for (const Function &func : scope->functionList) {
            if (copyCtors == CtorType::NO && func.type == Function::eCopyConstructor) {
                copyCtors = func.hasBody() ? CtorType::WITH_BODY : CtorType::WITHOUT_BODY;
            }
            if (assignmentOperators == CtorType::NO && func.type == Function::eOperatorEqual) {
                const Variable * variable = func.getArgumentVar(0);
                if (variable && variable->type() && variable->type()->classScope == scope) {
                    assignmentOperators = func.hasBody() ? CtorType::WITH_BODY : CtorType::WITHOUT_BODY;
                }
            }
            if (func.type == Function::eMoveConstructor) {
                moveCtor = true;
                break;
            }
        }

        if (moveCtor)
            continue;

        // No method defined
        if (copyCtors != CtorType::WITH_BODY && assignmentOperators != CtorType::WITH_BODY)
            continue;

        // both methods are defined
        if (copyCtors != CtorType::NO && assignmentOperators != CtorType::NO)
            continue;

        copyCtorAndEqOperatorError(scope->classDef, scope->className, scope->type == Scope::eStruct, copyCtors == CtorType::WITH_BODY);
    }
}

void CheckClass::copyCtorAndEqOperatorError(const Token *tok, const std::string &classname, bool isStruct, bool hasCopyCtor)
{
    const std::string message = "$symbol:" + classname + "\n"
                                "The " + std::string(isStruct ? "struct" : "class") + " '$symbol' has '" +
                                getFunctionTypeName(hasCopyCtor ? Function::eCopyConstructor : Function::eOperatorEqual) +
                                "' but lack of '" + getFunctionTypeName(hasCopyCtor ? Function::eOperatorEqual : Function::eCopyConstructor) +
                                "'.";
    reportError(tok, Severity::warning, "copyCtorAndEqOperator", message);
}

void CheckClass::checkOverride()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;
    if (mSettings->standards.cpp < Standards::CPP11)
        return;
    for (const Scope * classScope : mSymbolDatabase->classAndStructScopes) {
        if (!classScope->definedType || classScope->definedType->derivedFrom.empty())
            continue;
        for (const Function &func : classScope->functionList) {
            if (func.hasOverrideSpecifier() || func.hasFinalSpecifier())
                continue;
            const Function *baseFunc = func.getOverriddenFunction();
            if (baseFunc)
                overrideError(baseFunc, &func);
        }
    }
}

void CheckClass::overrideError(const Function *funcInBase, const Function *funcInDerived)
{
    const std::string functionName = funcInDerived ? ((funcInDerived->isDestructor() ? "~" : "") + funcInDerived->name()) : "";
    const std::string funcType = (funcInDerived && funcInDerived->isDestructor()) ? "destructor" : "function";

    ErrorPath errorPath;
    if (funcInBase && funcInDerived) {
        errorPath.push_back(ErrorPathItem(funcInBase->tokenDef, "Virtual " + funcType + " in base class"));
        errorPath.push_back(ErrorPathItem(funcInDerived->tokenDef, char(std::toupper(funcType[0])) + funcType.substr(1) + " in derived class"));
    }

    reportError(errorPath, Severity::style, "missingOverride",
                "$symbol:" + functionName + "\n"
                "The " + funcType + " '$symbol' overrides a " + funcType + " in a base class but is not marked with a 'override' specifier.",
                CWE(0U) /* Unknown CWE! */,
                false);
}

void CheckClass::checkThisUseAfterFree()
{
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    for (const Scope * classScope : mSymbolDatabase->classAndStructScopes) {

        for (const Variable &var : classScope->varlist) {
            // Find possible "self pointer".. pointer/smartpointer member variable of "self" type.
            if (var.valueType() && var.valueType()->smartPointerType != classScope->definedType && var.valueType()->typeScope != classScope) {
                const ValueType valueType = ValueType::parseDecl(var.typeStartToken(), mSettings);
                if (valueType.smartPointerType != classScope->definedType)
                    continue;
            }

            // If variable is not static, check that "this" is assigned
            if (!var.isStatic()) {
                bool hasAssign = false;
                for (const Function &func : classScope->functionList) {
                    if (func.type != Function::Type::eFunction || !func.hasBody())
                        continue;
                    for (const Token *tok = func.functionScope->bodyStart; tok != func.functionScope->bodyEnd; tok = tok->next()) {
                        if (Token::Match(tok, "%varid% = this|shared_from_this", var.declarationId())) {
                            hasAssign = true;
                            break;
                        }
                    }
                    if (hasAssign)
                        break;
                }
                if (!hasAssign)
                    continue;
            }

            // Check usage of self pointer..
            for (const Function &func : classScope->functionList) {
                if (func.type != Function::Type::eFunction || !func.hasBody())
                    continue;

                const Token * freeToken = nullptr;
                std::set<const Function *> callstack;
                checkThisUseAfterFreeRecursive(classScope, &func, &var, callstack, &freeToken);
            }
        }
    }
}

bool CheckClass::checkThisUseAfterFreeRecursive(const Scope *classScope, const Function *func, const Variable *selfPointer, std::set<const Function *> callstack, const Token **freeToken)
{
    if (!func || !func->functionScope)
        return false;

    // avoid recursion
    if (callstack.count(func))
        return false;
    callstack.insert(func);

    const Token * const bodyStart = func->functionScope->bodyStart;
    const Token * const bodyEnd = func->functionScope->bodyEnd;
    for (const Token *tok = bodyStart; tok != bodyEnd; tok = tok->next()) {
        const bool isDestroyed = *freeToken != nullptr && !func->isStatic();
        if (Token::Match(tok, "delete %var% ;") && selfPointer == tok->next()->variable()) {
            *freeToken = tok;
            tok = tok->tokAt(2);
        } else if (Token::Match(tok, "%var% . reset ( )") && selfPointer == tok->variable())
            *freeToken = tok;
        else if (Token::Match(tok->previous(), "!!. %name% (") && tok->function() && tok->function()->nestedIn == classScope) {
            if (isDestroyed) {
                thisUseAfterFree(selfPointer->nameToken(), *freeToken, tok);
                return true;
            }
            if (checkThisUseAfterFreeRecursive(classScope, tok->function(), selfPointer, callstack, freeToken))
                return true;
        } else if (isDestroyed && Token::Match(tok->previous(), "!!. %name%") && tok->variable() && tok->variable()->scope() == classScope && !tok->variable()->isStatic() && !tok->variable()->isArgument()) {
            thisUseAfterFree(selfPointer->nameToken(), *freeToken, tok);
            return true;
        } else if (*freeToken && Token::Match(tok, "return|throw")) {
            // TODO
            return tok->str() == "throw";
        } else if (tok->str() == "{" && tok->scope()->type == Scope::ScopeType::eLambda) {
            tok = tok->link();
        }
    }
    return false;
}

void CheckClass::thisUseAfterFree(const Token *self, const Token *free, const Token *use)
{
    std::string selfPointer = self ? self->str() : "ptr";
    const ErrorPath errorPath = { ErrorPathItem(self, "Assuming '" + selfPointer + "' is used as 'this'"), ErrorPathItem(free, "Delete '" + selfPointer + "', invalidating 'this'"), ErrorPathItem(use, "Call method when 'this' is invalid") };
    const std::string usestr = use ? use->str() : "x";
    const std::string usemsg = use && use->function() ? ("Calling method '" + usestr + "()'") : ("Using member '" + usestr + "'");
    reportError(errorPath, Severity::warning, "thisUseAfterFree",
                "$symbol:" + selfPointer + "\n" +
                usemsg + " when 'this' might be invalid",
                CWE(0), false);
}

void CheckClass::checkUnsafeClassRefMember()
{
    if (!mSettings->safeChecks.classes || !mSettings->isEnabled(Settings::WARNING))
        return;
    for (const Scope * classScope : mSymbolDatabase->classAndStructScopes) {
        for (const Function &func : classScope->functionList) {
            if (!func.hasBody() || !func.isConstructor())
                continue;

            const Token *initList = func.constructorMemberInitialization();
            while (Token::Match(initList, "[:,] %name% (")) {
                if (Token::Match(initList->tokAt(2), "( %var% )")) {
                    const Variable * const memberVar = initList->next()->variable();
                    const Variable * const argVar = initList->tokAt(3)->variable();
                    if (memberVar && argVar && memberVar->isConst() && memberVar->isReference() && argVar->isArgument() && argVar->isConst() && argVar->isReference())
                        unsafeClassRefMemberError(initList->next(), classScope->className + "::" + memberVar->name());
                }
                initList = initList->linkAt(2)->next();
            }
        }
    }
}

void CheckClass::unsafeClassRefMemberError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "unsafeClassRefMember",
                "$symbol:" + varname + "\n"
                "Unsafe class: The const reference member '$symbol' is initialized by a const reference constructor argument. You need to be careful about lifetime issues.\n"
                "Unsafe class checking: The const reference member '$symbol' is initialized by a const reference constructor argument. You need to be careful about lifetime issues. If you pass a local variable or temporary value in this constructor argument, be extra careful. If the argument is always some global object that is never destroyed then this is safe usage. However it would be defensive to make the member '$symbol' a non-reference variable or a smart pointer.",
                CWE(0), false);
}
