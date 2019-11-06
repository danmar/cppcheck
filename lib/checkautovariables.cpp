/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
// Auto variables checks
//---------------------------------------------------------------------------

#include "checkautovariables.h"

#include "astutils.h"
#include "errorlogger.h"
#include "library.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"

#include <cstddef>
#include <list>
#include <functional>

//---------------------------------------------------------------------------


// Register this check class into cppcheck by creating a static instance of it..
namespace {
    CheckAutoVariables instance;
}

static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE562(562U);  // Return of Stack Variable Address
static const CWE CWE590(590U);  // Free of Memory not on the Heap

static bool isPtrArg(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && var->isPointer());
}

static bool isArrayArg(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && var->isArray());
}

static bool isArrayVar(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArray());
}

static bool isRefPtrArg(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && var->isReference() && var->isPointer());
}

static bool isNonReferenceArg(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && !var->isReference() && (var->isPointer() || var->valueType()->type >= ValueType::Type::CONTAINER || var->type()));
}

static bool isAutoVar(const Token *tok)
{
    const Variable *var = tok->variable();

    if (!var || !var->isLocal() || var->isStatic())
        return false;

    if (var->isReference()) {
        // address of reference variable can be taken if the address
        // of the variable it points at is not a auto-var
        // TODO: check what the reference variable references.
        return false;
    }

    if (Token::Match(tok, "%name% .|::")) {
        do {
            tok = tok->tokAt(2);
        } while (Token::Match(tok, "%name% .|::"));
        if (Token::Match(tok, "%name% ("))
            return false;
    }
    return true;
}

static bool isAutoVarArray(const Token *tok)
{
    if (!tok)
        return false;

    // &x[..]
    if (tok->isUnaryOp("&") && Token::simpleMatch(tok->astOperand1(), "["))
        return isAutoVarArray(tok->astOperand1()->astOperand1());

    // x+y
    if (tok->str() == "+")
        return isAutoVarArray(tok->astOperand1()) || isAutoVarArray(tok->astOperand2());

    // x-intexpr
    if (tok->str() == "-")
        return isAutoVarArray(tok->astOperand1()) &&
               tok->astOperand2() &&
               tok->astOperand2()->valueType() &&
               tok->astOperand2()->valueType()->isIntegral();

    const Variable *var = tok->variable();
    if (!var)
        return false;

    // Variable
    if (var->isLocal() && !var->isStatic() && var->isArray() && !var->isPointer())
        return true;

    // ValueFlow
    if (var->isPointer() && !var->isArgument()) {
        for (std::list<ValueFlow::Value>::const_iterator it = tok->values().begin(); it != tok->values().end(); ++it) {
            const ValueFlow::Value &val = *it;
            if (val.isTokValue() && isAutoVarArray(val.tokvalue))
                return true;
        }
    }

    return false;
}

// Verification that we really take the address of a local variable
static bool checkRvalueExpression(const Token * const vartok)
{
    const Variable * const var = vartok->variable();
    if (var == nullptr)
        return false;

    if (Token::Match(vartok->previous(), "& %name% [") && var->isPointer())
        return false;

    const Token * const next = vartok->next();
    // &a.b[0]
    if (Token::Match(vartok, "%name% . %var% [") && !var->isPointer()) {
        const Variable *var2 = next->next()->variable();
        return var2 && !var2->isPointer();
    }

    return ((next->str() != "." || (!var->isPointer() && (!var->isClass() || var->type()))) && next->strAt(2) != ".");
}

static bool isAddressOfLocalVariable(const Token *expr)
{
    if (!expr)
        return false;
    if (Token::Match(expr, "+|-"))
        return isAddressOfLocalVariable(expr->astOperand1()) || isAddressOfLocalVariable(expr->astOperand2());
    if (expr->isCast())
        return isAddressOfLocalVariable(expr->astOperand2() ? expr->astOperand2() : expr->astOperand1());
    if (expr->isUnaryOp("&")) {
        const Token *op = expr->astOperand1();
        bool deref = false;
        while (Token::Match(op, ".|[")) {
            if (op->originalName() == "->")
                return false;
            if (op->str() == "[")
                deref = true;
            op = op->astOperand1();
        }
        return op && isAutoVar(op) && (!deref || !op->variable()->isPointer());
    }
    return false;
}

static bool variableIsUsedInScope(const Token* start, nonneg int varId, const Scope *scope)
{
    if (!start) // Ticket #5024
        return false;

    for (const Token *tok = start; tok && tok != scope->bodyEnd; tok = tok->next()) {
        if (tok->varId() == varId)
            return true;
        const Scope::ScopeType scopeType = tok->scope()->type;
        if (scopeType == Scope::eFor || scopeType == Scope::eDo || scopeType == Scope::eWhile) // In case of loops, better checking would be necessary
            return true;
        if (Token::simpleMatch(tok, "asm ("))
            return true;
    }
    return false;
}

void CheckAutoVariables::assignFunctionArg()
{
    const bool printStyle = mSettings->isEnabled(Settings::STYLE);
    const bool printWarning = mSettings->isEnabled(Settings::WARNING);
    if (!printStyle && !printWarning)
        return;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            // TODO: What happens if this is removed?
            if (tok->astParent())
                continue;
            if (!(tok->isAssignmentOp() || Token::Match(tok, "++|--")) || !Token::Match(tok->astOperand1(), "%var%"))
                continue;
            const Token* const vartok = tok->astOperand1();
            if (isNonReferenceArg(vartok) &&
                !Token::Match(vartok->next(), "= %varid% ;", vartok->varId()) &&
                !variableIsUsedInScope(Token::findsimplematch(vartok->next(), ";"), vartok->varId(), scope) &&
                !Token::findsimplematch(vartok, "goto", scope->bodyEnd)) {
                if (vartok->variable()->isPointer() && printWarning)
                    errorUselessAssignmentPtrArg(vartok);
                else if (printStyle)
                    errorUselessAssignmentArg(vartok);
            }
        }
    }
}

void CheckAutoVariables::autoVariables()
{
    const bool printInconclusive = mSettings->inconclusive;
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            // Skip lambda..
            if (const Token *lambdaEndToken = findLambdaEndToken(tok)) {
                tok = lambdaEndToken;
                continue;
            }
            // Critical assignment
            if (Token::Match(tok, "[;{}] %var% = & %var%") && isRefPtrArg(tok->next()) && isAutoVar(tok->tokAt(4))) {
                if (checkRvalueExpression(tok->tokAt(4)))
                    errorAutoVariableAssignment(tok->next(), false);
            } else if (Token::Match(tok, "[;{}] * %var% =") && isPtrArg(tok->tokAt(2)) && isAddressOfLocalVariable(tok->tokAt(3)->astOperand2())) {
                errorAutoVariableAssignment(tok->next(), false);
            } else if (Token::Match(tok, "[;{}] %var% . %var% =") && isPtrArg(tok->next()) && isAddressOfLocalVariable(tok->tokAt(4)->astOperand2())) {
                errorAutoVariableAssignment(tok->next(), false);
            } else if (Token::Match(tok, "[;{}] %var% . %var% = %var% ;")) {
                // TODO: check if the parameter is only changed temporarily (#2969)
                if (printInconclusive && isPtrArg(tok->next())) {
                    if (isAutoVarArray(tok->tokAt(5)))
                        errorAutoVariableAssignment(tok->next(), true);
                }
                tok = tok->tokAt(5);
            } else if (Token::Match(tok, "[;{}] * %var% = %var% ;")) {
                const Variable * var1 = tok->tokAt(2)->variable();
                if (var1 && var1->isArgument() && Token::Match(var1->nameToken()->tokAt(-3), "%type% * *")) {
                    if (isAutoVarArray(tok->tokAt(4)))
                        errorAutoVariableAssignment(tok->next(), false);
                }
                tok = tok->tokAt(4);
            } else if (Token::Match(tok, "[;{}] %var% [") && Token::simpleMatch(tok->linkAt(2), "] =") &&
                       (isPtrArg(tok->next()) || isArrayArg(tok->next())) && isAddressOfLocalVariable(tok->linkAt(2)->next()->astOperand2())) {
                errorAutoVariableAssignment(tok->next(), false);
            }
            // Invalid pointer deallocation
            else if ((Token::Match(tok, "%name% ( %var% ) ;") && mSettings->library.getDeallocFuncInfo(tok)) ||
                     (mTokenizer->isCPP() && Token::Match(tok, "delete [| ]| (| %var% !!["))) {
                tok = Token::findmatch(tok->next(), "%var%");
                if (isArrayVar(tok))
                    errorInvalidDeallocation(tok, nullptr);
                else if (tok && tok->variable() && tok->variable()->isPointer()) {
                    for (const ValueFlow::Value &v : tok->values()) {
                        if (v.isTokValue() && isArrayVar(v.tokvalue)) {
                            errorInvalidDeallocation(tok, &v);
                            break;
                        }
                    }
                }
            } else if ((Token::Match(tok, "%name% ( & %var% ) ;") && mSettings->library.getDeallocFuncInfo(tok)) ||
                       (mTokenizer->isCPP() && Token::Match(tok, "delete [| ]| (| & %var% !!["))) {
                tok = Token::findmatch(tok->next(), "%var%");
                if (isAutoVar(tok))
                    errorInvalidDeallocation(tok, nullptr);
            }
        }
    }
}

//---------------------------------------------------------------------------

void CheckAutoVariables::errorReturnAddressToAutoVariable(const Token *tok)
{
    reportError(tok, Severity::error, "returnAddressOfAutoVariable", "Address of an auto-variable returned.", CWE562, false);
}

void CheckAutoVariables::errorReturnAddressToAutoVariable(const Token *tok, const ValueFlow::Value *value)
{
    reportError(tok, Severity::error, "returnAddressOfAutoVariable", "Address of auto-variable '" + value->tokvalue->astOperand1()->expressionString() + "' returned", CWE562, false);
}

void CheckAutoVariables::errorReturnPointerToLocalArray(const Token *tok)
{
    reportError(tok, Severity::error, "returnLocalVariable", "Pointer to local array variable returned.", CWE562, false);
}

void CheckAutoVariables::errorAutoVariableAssignment(const Token *tok, bool inconclusive)
{
    if (!inconclusive) {
        reportError(tok, Severity::error, "autoVariables",
                    "Address of local auto-variable assigned to a function parameter.\n"
                    "Dangerous assignment - the function parameter is assigned the address of a local "
                    "auto-variable. Local auto-variables are reserved from the stack which "
                    "is freed when the function ends. So the pointer to a local variable "
                    "is invalid after the function ends.", CWE562, false);
    } else {
        reportError(tok, Severity::error, "autoVariables",
                    "Address of local auto-variable assigned to a function parameter.\n"
                    "Function parameter is assigned the address of a local auto-variable. "
                    "Local auto-variables are reserved from the stack which is freed when "
                    "the function ends. The address is invalid after the function ends and it "
                    "might 'leak' from the function through the parameter.",
                    CWE562,
                    true);
    }
}

void CheckAutoVariables::errorReturnAddressOfFunctionParameter(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "returnAddressOfFunctionParameter",
                "$symbol:" + varname + "\n"
                "Address of function parameter '$symbol' returned.\n"
                "Address of the function parameter '$symbol' becomes invalid after the function exits because "
                "function parameters are stored on the stack which is freed when the function exits. Thus the returned "
                "value is invalid.", CWE562, false);
}

void CheckAutoVariables::errorUselessAssignmentArg(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "uselessAssignmentArg",
                "Assignment of function parameter has no effect outside the function.", CWE398, false);
}

void CheckAutoVariables::errorUselessAssignmentPtrArg(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "uselessAssignmentPtrArg",
                "Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?", CWE398, false);
}

//---------------------------------------------------------------------------

static bool astHasAutoResult(const Token *tok)
{
    if (tok->astOperand1() && !astHasAutoResult(tok->astOperand1()))
        return false;
    if (tok->astOperand2() && !astHasAutoResult(tok->astOperand2()))
        return false;

    if (tok->isOp()) {
        if (tok->tokType() == Token::eIncDecOp)
            return false;
        if ((tok->str() == "<<" || tok->str() == ">>") && tok->astOperand1()) {
            const Token* tok2 = tok->astOperand1();
            while (tok2 && tok2->isUnaryOp("*"))
                tok2 = tok2->astOperand1();
            return tok2 && tok2->variable() && !tok2->variable()->isClass() && !tok2->variable()->isStlType(); // Class or unknown type on LHS: Assume it is a stream
        }
        return true;
    }

    if (tok->isLiteral())
        return true;

    if (tok->isName()) {
        // TODO: check function calls, struct members, arrays, etc also
        if (!tok->variable())
            return false;
        if (tok->variable()->isStlType())
            return true;
        if (tok->variable()->isClass() || tok->variable()->isPointer() || tok->variable()->isReference()) // TODO: Properly handle pointers/references to classes in symbol database
            return false;

        return true;
    }

    return false;
}

static bool isInScope(const Token * tok, const Scope * scope)
{
    if (!tok)
        return false;
    if (!scope)
        return false;
    const Variable * var = tok->variable();
    if (var && (var->isGlobal() || var->isStatic() || var->isExtern()))
        return false;
    if (tok->scope() && tok->scope()->isNestedIn(scope))
        return true;
    if (!var)
        return false;
    if (var->isArgument() && !var->isReference()) {
        const Scope * tokScope = tok->scope();
        if (!tokScope)
            return false;
        for (const Scope * argScope:tokScope->nestedList) {
            if (argScope && argScope->isNestedIn(scope))
                return true;
        }
    }
    return false;
}

static bool isDeadScope(const Token * tok, const Scope * scope)
{
    if (!tok)
        return false;
    if (!scope)
        return false;
    const Variable * var = tok->variable();
    if (var && (!var->isLocal() || var->isStatic() || var->isExtern()))
        return false;
    if (tok->scope() && tok->scope()->bodyEnd != scope->bodyEnd && precedes(tok->scope()->bodyEnd, scope->bodyEnd))
        return true;
    return false;
}

static const Token * getParentLifetime(const Token *tok)
{
    if (!tok)
        return tok;
    const Variable * var = tok->variable();
    // TODO: Call getLifetimeVariable for deeper analysis
    if (!var)
        return tok;
    if (var->isLocal())
        return tok;
    const Token * parent = getParentMember(tok);
    if (parent != tok)
        return getParentLifetime(parent);
    return tok;
}

static int getPointerDepth(const Token *tok)
{
    if (!tok)
        return 0;
    return tok->valueType() ? tok->valueType()->pointer : 0;
}

static bool isDeadTemporary(bool cpp, const Token* tok, const Token* expr, const Library* library)
{
    if (!isTemporary(cpp, tok, library))
        return false;
    if (expr && !precedes(nextAfterAstRightmostLeaf(tok->astTop()), nextAfterAstRightmostLeaf(expr->astTop())))
        return false;
    return true;
}

void CheckAutoVariables::checkVarLifetimeScope(const Token * start, const Token * end)
{
    if (!start)
        return;
    const Scope * scope = start->scope();
    if (!scope)
        return;
    // If the scope is not set correctly then skip checking it
    if (scope->bodyStart != start)
        return;
    bool returnRef = Function::returnsReference(scope->function);
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        // Return reference from function
        if (returnRef && Token::simpleMatch(tok->astParent(), "return")) {
            for (const LifetimeToken& lt : getLifetimeTokens(tok)) {
                const Variable* var = lt.token->variable();
                if (var && !var->isGlobal() && !var->isStatic() && !var->isReference() && !var->isRValueReference() &&
                    isInScope(var->nameToken(), tok->scope())) {
                    errorReturnReference(tok, lt.errorPath, lt.inconclusive);
                    break;
                } else if (isDeadTemporary(mTokenizer->isCPP(), lt.token, nullptr, &mSettings->library)) {
                    errorReturnTempReference(tok, lt.errorPath, lt.inconclusive);
                    break;
                }
            }
            // Assign reference to non-local variable
        } else if (Token::Match(tok->previous(), "&|&& %var% =") && tok->astParent() == tok->next() &&
                   tok->variable() && tok->variable()->nameToken() == tok &&
                   tok->variable()->declarationId() == tok->varId() && tok->variable()->isStatic() &&
                   !tok->variable()->isArgument()) {
            ErrorPath errorPath;
            const Variable *var = getLifetimeVariable(tok, errorPath);
            if (var && isInScope(var->nameToken(), tok->scope())) {
                errorDanglingReference(tok, var, errorPath);
                continue;
            }
        }
        for (const ValueFlow::Value& val:tok->values()) {
            if (!val.isLocalLifetimeValue())
                continue;
            const Token * tokvalue = getParentLifetime(val.tokvalue);
            if (Token::Match(tok->astParent(), "return|throw")) {
                if (getPointerDepth(tok) < getPointerDepth(tokvalue))
                    continue;
                if (!isLifetimeBorrowed(tok, mSettings))
                    continue;
                if ((tokvalue->variable() && isInScope(tokvalue->variable()->nameToken(), scope)) ||
                    isDeadTemporary(mTokenizer->isCPP(), tokvalue, tok, &mSettings->library)) {
                    errorReturnDanglingLifetime(tok, &val);
                    break;
                }
            } else if (tokvalue->variable() && isDeadScope(tokvalue->variable()->nameToken(), tok->scope())) {
                errorInvalidLifetime(tok, &val);
                break;
            } else if (!tokvalue->variable() && isDeadTemporary(mTokenizer->isCPP(), tokvalue, tok, &mSettings->library)) {
                errorDanglingTemporaryLifetime(tok, &val);
                break;
            } else if (tokvalue->variable() && isInScope(tokvalue->variable()->nameToken(), tok->scope())) {
                const Variable * var = nullptr;
                const Token * tok2 = tok;
                if (Token::simpleMatch(tok->astParent(), "=")) {
                    if (tok->astParent()->astOperand2() == tok) {
                        var = getLHSVariable(tok->astParent());
                        tok2 = tok->astParent()->astOperand1();
                    }
                } else if (tok->variable() && tok->variable()->declarationId() == tok->varId()) {
                    var = tok->variable();
                }
                if (!isLifetimeBorrowed(tok, mSettings))
                    continue;
                if (var && !var->isLocal() && !var->isArgument() && !isVariableChanged(tok->next(), tok->scope()->bodyEnd, var->declarationId(), var->isGlobal(), mSettings, mTokenizer->isCPP())) {
                    errorDanglngLifetime(tok2, &val);
                    break;
                }
            }
        }
        const Token *lambdaEndToken = findLambdaEndToken(tok);
        if (lambdaEndToken) {
            checkVarLifetimeScope(lambdaEndToken->link(), lambdaEndToken);
            tok = lambdaEndToken;
        }
        if (tok->str() == "{" && tok->scope()) {
            // Check functions in local classes
            if (tok->scope()->type == Scope::eClass ||
                tok->scope()->type == Scope::eStruct ||
                tok->scope()->type == Scope::eUnion) {
                for (const Function& f:tok->scope()->functionList) {
                    if (f.functionScope)
                        checkVarLifetimeScope(f.functionScope->bodyStart, f.functionScope->bodyEnd);
                }
                tok = tok->link();
            }
        }
    }
}

void CheckAutoVariables::checkVarLifetime()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (!scope->function)
            continue;
        checkVarLifetimeScope(scope->bodyStart, scope->bodyEnd);
    }
}

void CheckAutoVariables::errorReturnDanglingLifetime(const Token *tok, const ValueFlow::Value *val)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Returning " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "returnDanglingLifetime", msg + " that will be invalid when returning.", CWE562, inconclusive);
}

void CheckAutoVariables::errorInvalidLifetime(const Token *tok, const ValueFlow::Value* val)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Using " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "invalidLifetime", msg + " that is out of scope.", CWE562, inconclusive);
}

void CheckAutoVariables::errorDanglingTemporaryLifetime(const Token* tok, const ValueFlow::Value* val)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Using " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "danglingTemporaryLifetime", msg + " to temporary.", CWE562, inconclusive);
}

void CheckAutoVariables::errorDanglngLifetime(const Token *tok, const ValueFlow::Value *val)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string tokName = tok ? tok->expressionString() : "x";
    std::string msg = "Non-local variable '" + tokName + "' will use " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "danglingLifetime", msg + ".", CWE562, inconclusive);
}

void CheckAutoVariables::errorReturnReference(const Token* tok, ErrorPath errorPath, bool inconclusive)
{
    errorPath.emplace_back(tok, "");
    reportError(
        errorPath, Severity::error, "returnReference", "Reference to local variable returned.", CWE562, inconclusive);
}

void CheckAutoVariables::errorDanglingReference(const Token *tok, const Variable *var, ErrorPath errorPath)
{
    std::string tokName = tok ? tok->str() : "x";
    std::string varName = var ? var->name() : "y";
    std::string msg = "Non-local reference variable '" + tokName + "' to local variable '" + varName + "'";
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "danglingReference", msg, CWE562, false);
}

void CheckAutoVariables::errorReturnTempReference(const Token* tok, ErrorPath errorPath, bool inconclusive)
{
    errorPath.emplace_back(tok, "");
    reportError(
        errorPath, Severity::error, "returnTempReference", "Reference to temporary returned.", CWE562, inconclusive);
}

void CheckAutoVariables::errorInvalidDeallocation(const Token *tok, const ValueFlow::Value *val)
{
    const Variable *var = val ? val->tokvalue->variable() : (tok ? tok->variable() : nullptr);

    std::string type = "auto-variable";
    if (var) {
        if (var->isGlobal())
            type = "global variable";
        else if (var->isStatic())
            type = "static variable";
    }

    if (val)
        type += " (" + val->tokvalue->str() + ")";

    reportError(getErrorPath(tok, val, "Deallocating memory that was not dynamically allocated"),
                Severity::error,
                "autovarInvalidDeallocation",
                "Deallocation of an " + type + " results in undefined behaviour.\n"
                "The deallocation of an " + type + " results in undefined behaviour. You should only free memory "
                "that has been allocated dynamically.", CWE590, false);
}
