/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

static bool isGlobalPtr(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isGlobal() && var->isPointer());
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

static bool variableIsUsedInScope(const Token* start, unsigned int varId, const Scope *scope)
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

static bool reassignedGlobalPointer(const Token *vartok, unsigned int pointerVarId, const Settings *settings, bool cpp)
{
    return isVariableChanged(vartok,
                             vartok->variable()->scope()->bodyEnd,
                             pointerVarId,
                             true,
                             settings,
                             cpp);
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
            } else if (Token::Match(tok, "[;{}] * %var% = & %var%") && isPtrArg(tok->tokAt(2)) && isAutoVar(tok->tokAt(5))) {
                if (checkRvalueExpression(tok->tokAt(5)))
                    errorAutoVariableAssignment(tok->next(), false);
            } else if (mSettings->isEnabled(Settings::WARNING) && Token::Match(tok, "[;{}] %var% = &| %var% ;") && isGlobalPtr(tok->next())) {
                const Token * const pointer = tok->next();
                if (isAutoVarArray(tok->tokAt(3))) {
                    const Token * const array = tok->tokAt(3);
                    if (!reassignedGlobalPointer(array, pointer->varId(), mSettings, mTokenizer->isCPP()))
                        errorAssignAddressOfLocalArrayToGlobalPointer(pointer, array);
                } else if (isAutoVar(tok->tokAt(4))) {
                    const Token * const variable = tok->tokAt(4);
                    if (!reassignedGlobalPointer(variable, pointer->varId(), mSettings, mTokenizer->isCPP()))
                        errorAssignAddressOfLocalVariableToGlobalPointer(pointer, variable);
                }
            } else if (Token::Match(tok, "[;{}] %var% . %var% = & %var%")) {
                // TODO: check if the parameter is only changed temporarily (#2969)
                if (printInconclusive && isPtrArg(tok->next())) {
                    const Token * const var2tok = tok->tokAt(6);
                    if (isAutoVar(var2tok) && checkRvalueExpression(var2tok))
                        errorAutoVariableAssignment(tok->next(), true);
                }
                tok = tok->tokAt(6);
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
            } else if (Token::Match(tok, "[;{}] %var% [") && Token::Match(tok->linkAt(2), "] = & %var%") &&
                       (isPtrArg(tok->next()) || isArrayArg(tok->next())) && isAutoVar(tok->linkAt(2)->tokAt(3))) {
                const Token* const varTok = tok->linkAt(2)->tokAt(3);
                if (checkRvalueExpression(varTok))
                    errorAutoVariableAssignment(tok->next(), false);
            }
            // Invalid pointer deallocation
            else if ((Token::Match(tok, "%name% ( %var% ) ;") && mSettings->library.dealloc(tok)) ||
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
            } else if ((Token::Match(tok, "%name% ( & %var% ) ;") && mSettings->library.dealloc(tok)) ||
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

void CheckAutoVariables::errorAssignAddressOfLocalArrayToGlobalPointer(const Token *pointer, const Token *array)
{
    const std::string pointerName = pointer ? pointer->str() : std::string("pointer");
    const std::string arrayName   = array ? array->str() : std::string("array");
    reportError(pointer, Severity::warning, "autoVariablesAssignGlobalPointer",
                "$symbol:" + arrayName + "\nAddress of local array $symbol is assigned to global pointer " + pointerName +" and not reassigned before $symbol goes out of scope.", CWE562, false);
}

void CheckAutoVariables::errorAssignAddressOfLocalVariableToGlobalPointer(const Token *pointer, const Token *variable)
{
    const std::string pointerName = pointer ? pointer->str() : std::string("pointer");
    const std::string variableName = variable ? variable->str() : std::string("variable");
    reportError(pointer, Severity::warning, "autoVariablesAssignGlobalPointer",
                "$symbol:" + variableName + "\nAddress of local variable $symbol is assigned to global pointer " + pointerName +" and not reassigned before $symbol goes out of scope.", CWE562, false);
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

// return temporary?
bool CheckAutoVariables::returnTemporary(const Token *tok)
{
    bool func = false;     // Might it be a function call?
    bool retref = false;   // is there such a function that returns a reference?
    bool retvalue = false; // is there such a function that returns a value?

    const Function *function = tok->function();
    if (function) {
        // Ticket #5478: Only functions or operator equal might return a temporary
        if (function->type != Function::eOperatorEqual && function->type != Function::eFunction)
            return false;
        retref = function->tokenDef->strAt(-1) == "&";
        if (!retref) {
            const Token *start = function->retDef;
            if (start->str() == "const")
                start = start->next();
            if (start->str() == "::")
                start = start->next();

            if (Token::simpleMatch(start, "std ::")) {
                if (start->strAt(3) != "<" || !Token::simpleMatch(start->linkAt(3), "> ::"))
                    retvalue = true;
                else
                    retref = true; // Assume that a reference is returned
            } else {
                if (start->type())
                    retvalue = true;
                else
                    retref = true;
            }
        }
        func = true;
    }
    if (!func && tok->type())
        return true;

    return (!retref && retvalue);
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

void CheckAutoVariables::returnReference()
{
    if (mTokenizer->isC())
        return;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (!scope->function)
            continue;

        const Token *tok = scope->function->tokenDef;

        // have we reached a function that returns a reference?
        if (tok->previous() && tok->previous()->str() == "&") {
            for (const Token *tok2 = scope->bodyStart->next(); tok2 && tok2 != scope->bodyEnd; tok2 = tok2->next()) {
                if (!tok2->scope()->isExecutable()) {
                    tok2 = tok2->scope()->bodyEnd;
                    if (!tok2)
                        break;
                    continue;
                }

                // Skip over lambdas
                const Token *lambdaEndToken = findLambdaEndToken(tok2);
                if (lambdaEndToken)
                    tok2 = lambdaEndToken->next();

                if (tok2->str() == "(")
                    tok2 = tok2->link();

                if (tok2->str() != "return")
                    continue;

                // return..
                if (Token::Match(tok2, "return %var% ;")) {
                    // is the returned variable a local variable?
                    if (isAutoVar(tok2->next())) {
                        const Variable *var1 = tok2->next()->variable();
                        // If reference variable is used, check what it references
                        if (Token::Match(var1->nameToken(), "%var% [=(]")) {
                            const Token *tok3 = var1->nameToken()->tokAt(2);
                            if (!Token::Match(tok3, "%var% [);.]"))
                                continue;

                            // Only report error if variable that is referenced is
                            // a auto variable
                            if (!isAutoVar(tok3))
                                continue;
                        }

                        // report error..
                        errorReturnReference(tok2);
                    }
                }

                // return reference to temporary..
                else if (Token::Match(tok2, "return %name% (") &&
                         Token::simpleMatch(tok2->linkAt(2), ") ;")) {
                    if (returnTemporary(tok2->next())) {
                        // report error..
                        errorReturnTempReference(tok2);
                    }
                }

                // Return reference to a literal or the result of a calculation
                else if (tok2->astOperand1() && (tok2->astOperand1()->isCalculation() || tok2->next()->isLiteral()) && astHasAutoResult(tok2->astOperand1())) {
                    errorReturnTempReference(tok2);
                }
            }
        }
    }
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

static int getPointerDepth(const Token *tok)
{
    if (!tok)
        return 0;
    return tok->valueType() ? tok->valueType()->pointer : 0;
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
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        for (const ValueFlow::Value& val:tok->values()) {
            if (!val.isLifetimeValue())
                continue;
            // Skip temporaries for now
            if (val.tokvalue == tok)
                continue;
            if (Token::Match(tok->astParent(), "return|throw")) {
                if (getPointerDepth(tok) < getPointerDepth(val.tokvalue))
                    continue;
                if (tok->astParent()->str() == "return" && !astIsContainer(tok) && scope->function &&
                    mSettings->library.detectContainer(scope->function->retDef))
                    continue;
                if (isInScope(val.tokvalue, scope)) {
                    errorReturnDanglingLifetime(tok, &val);
                    break;
                }
            } else if (isDeadScope(val.tokvalue, tok->scope())) {
                errorInvalidLifetime(tok, &val);
                break;
            } else if (tok->variable() && !tok->variable()->isLocal() && !tok->variable()->isArgument() &&
                       isInScope(val.tokvalue, tok->scope())) {
                errorDanglngLifetime(tok, &val);
                break;
            }
        }
        const Token *lambdaEndToken = findLambdaEndToken(tok);
        if (lambdaEndToken) {
            checkVarLifetimeScope(lambdaEndToken->link(), lambdaEndToken);
            tok = lambdaEndToken;
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

static std::string lifetimeType(const Token *tok, const ValueFlow::Value* val)
{
    std::string result;
    if (!val)
        return "object";
    switch (val->lifetimeKind) {
    case ValueFlow::Value::Lambda:
        result = "lambda";
        break;
    case ValueFlow::Value::Iterator:
        result = "iterator";
        break;
    case ValueFlow::Value::Object:
        if (astIsPointer(tok))
            result = "pointer";
        else
            result = "object";
        break;
    }
    return result;
}

static std::string lifetimeMessage(const Token *tok, const ValueFlow::Value *val, ErrorPath &errorPath)
{
    const Token *vartok = val ? val->tokvalue : nullptr;
    std::string type = lifetimeType(tok, val);
    std::string msg = type;
    if (vartok) {
        errorPath.emplace_back(vartok, "Variable created here.");
        const Variable * var = vartok->variable();
        if (var) {
            switch (val->lifetimeKind) {
            case ValueFlow::Value::Object:
                if (type == "pointer")
                    msg += " to local variable";
                else
                    msg += " that points to local variable";
                break;
            case ValueFlow::Value::Lambda:
                msg += " that captures local variable";
                break;
            case ValueFlow::Value::Iterator:
                msg += " to local container";
                break;
            }
            msg += " '" + var->name() + "'";
        }
    }
    return msg;
}

void CheckAutoVariables::errorReturnDanglingLifetime(const Token *tok, const ValueFlow::Value *val)
{
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Returning " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "returnDanglingLifetime", msg + " that will be invalid when returning.", CWE562, false);
}

void CheckAutoVariables::errorInvalidLifetime(const Token *tok, const ValueFlow::Value* val)
{
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Using " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "invalidLifetime", msg + " that is out of scope.", CWE562, false);
}

void CheckAutoVariables::errorDanglngLifetime(const Token *tok, const ValueFlow::Value *val)
{
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string tokName = tok ? tok->str() : "x";
    std::string msg = "Non-local variable '" + tokName + "' will use " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "danglingLifetime", msg + ".", CWE562, false);
}

void CheckAutoVariables::errorReturnReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnReference", "Reference to auto variable returned.", CWE562, false);
}

void CheckAutoVariables::errorReturnTempReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnTempReference", "Reference to temporary returned.", CWE562, false);
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
