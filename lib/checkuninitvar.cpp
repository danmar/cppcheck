/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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
#include "checkuninitvar.h"

#include "astutils.h"
#include "checknullpointer.h"   // CheckNullPointer::isPointerDeref
#include "errorlogger.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"

#include <cassert>
#include <list>
#include <map>
#include <utility>


namespace tinyxml2 {
    class XMLElement;
}

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckUninitVar instance;
}

//---------------------------------------------------------------------------

static bool isSizeOfEtc(const Token *tok)
{
    return Token::Match(tok, "sizeof|typeof|offsetof|decltype|__typeof__ (");
}

// get ast parent, skip possible address-of and casts
static const Token *getAstParentSkipPossibleCastAndAddressOf(const Token *vartok, bool *unknown)
{
    if (unknown)
        *unknown = false;
    if (!vartok)
        return nullptr;
    const Token *parent = vartok->astParent();
    while (Token::Match(parent, ".|::"))
        parent = parent->astParent();
    if (!parent)
        return nullptr;
    if (parent->isUnaryOp("&"))
        parent = parent->astParent();
    else if (parent->str() == "&" && vartok == parent->astOperand2() && Token::Match(parent->astOperand1()->previous(), "( %type% )")) {
        parent = parent->astParent();
        if (unknown)
            *unknown = true;
    }
    while (parent && parent->isCast())
        parent = parent->astParent();
    return parent;
}

void CheckUninitVar::check()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    std::set<std::string> arrayTypeDefs;
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name% [") && tok->variable() && Token::Match(tok->variable()->typeStartToken(), "%type% %var% ;"))
            arrayTypeDefs.insert(tok->variable()->typeStartToken()->str());
    }

    // check every executable scope
    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.isExecutable()) {
            checkScope(&scope, arrayTypeDefs);
        }
    }
}

void CheckUninitVar::checkScope(const Scope* scope, const std::set<std::string> &arrayTypeDefs)
{
    for (const Variable &var : scope->varlist) {
        if ((mTokenizer->isCPP() && var.type() && !var.isPointer() && var.type()->needInitialization != Type::NeedInitialization::True) ||
            var.isStatic() || var.isExtern() || var.isReference())
            continue;

        // don't warn for try/catch exception variable
        if (var.isThrow())
            continue;

        if (Token::Match(var.nameToken()->next(), "[({:]"))
            continue;

        if (Token::Match(var.nameToken(), "%name% =")) { // Variable is initialized, but Rhs might be not
            checkRhs(var.nameToken(), var, NO_ALLOC, 0U, emptyString);
            continue;
        }
        if (Token::Match(var.nameToken(), "%name% ) (") && Token::simpleMatch(var.nameToken()->linkAt(2), ") =")) { // Function pointer is initialized, but Rhs might be not
            checkRhs(var.nameToken()->linkAt(2)->next(), var, NO_ALLOC, 0U, emptyString);
            continue;
        }

        if (var.isArray() || var.isPointerToArray()) {
            const Token *tok = var.nameToken()->next();
            if (var.isPointerToArray())
                tok = tok->next();
            while (Token::simpleMatch(tok->link(), "] ["))
                tok = tok->link()->next();
            if (Token::Match(tok->link(), "] =|{"))
                continue;
        }

        bool stdtype = mTokenizer->isC() && arrayTypeDefs.find(var.typeStartToken()->str()) == arrayTypeDefs.end();
        const Token* tok = var.typeStartToken();
        for (; tok != var.nameToken() && tok->str() != "<"; tok = tok->next()) {
            if (tok->isStandardType() || tok->isEnumType())
                stdtype = true;
        }
        if (var.isArray() && !stdtype)
            continue;

        while (tok && tok->str() != ";")
            tok = tok->next();
        if (!tok)
            continue;

        if (tok->astParent() && Token::simpleMatch(tok->astParent()->previous(), "for (") &&
            checkLoopBody(tok->astParent()->link()->next(), var, var.isArray() ? ARRAY : NO_ALLOC, emptyString, true))
            continue;

        if (var.isArray()) {
            Alloc alloc = ARRAY;
            const std::map<nonneg int, VariableValue> variableValue;
            bool init = false;
            for (const Token *parent = var.nameToken(); parent; parent = parent->astParent()) {
                if (parent->str() == "=") {
                    init = true;
                    break;
                }
            }
            if (!init)
                checkScopeForVariable(tok, var, nullptr, nullptr, &alloc, emptyString, variableValue);
            continue;
        }
        if (stdtype || var.isPointer()) {
            Alloc alloc = NO_ALLOC;
            const std::map<nonneg int, VariableValue> variableValue;
            checkScopeForVariable(tok, var, nullptr, nullptr, &alloc, emptyString, variableValue);
        }
        if (var.type())
            checkStruct(tok, var);
    }

    if (scope->function) {
        for (const Variable &arg : scope->function->argumentList) {
            if (arg.declarationId() && Token::Match(arg.typeStartToken(), "%type% * %name% [,)]")) {
                // Treat the pointer as initialized until it is assigned by malloc
                for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
                    if (!Token::Match(tok, "[;{}] %varid% =", arg.declarationId()))
                        continue;
                    const Token *allocFuncCallToken = findAllocFuncCallToken(tok->tokAt(2)->astOperand2(), mSettings->library);
                    if (!allocFuncCallToken)
                        continue;
                    const Library::AllocFunc *allocFunc = mSettings->library.getAllocFuncInfo(allocFuncCallToken);
                    if (!allocFunc || allocFunc->initData)
                        continue;

                    if (arg.typeStartToken()->strAt(-1) == "struct" || (arg.type() && arg.type()->isStructType()))
                        checkStruct(tok, arg);
                    else if (arg.typeStartToken()->isStandardType() || arg.typeStartToken()->isEnumType()) {
                        Alloc alloc = NO_ALLOC;
                        const std::map<nonneg int, VariableValue> variableValue;
                        checkScopeForVariable(tok->next(), arg, nullptr, nullptr, &alloc, emptyString, variableValue);
                    }
                }
            }
        }
    }
}

void CheckUninitVar::checkStruct(const Token *tok, const Variable &structvar)
{
    const Token *typeToken = structvar.typeStartToken();
    const SymbolDatabase * symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope2 : symbolDatabase->classAndStructScopes) {
        if (scope2->className == typeToken->str() && scope2->numConstructors == 0U) {
            for (const Variable &var : scope2->varlist) {
                if (var.isStatic() || var.hasDefault() || var.isArray() ||
                    (!mTokenizer->isC() && var.isClass() && (!var.type() || var.type()->needInitialization != Type::NeedInitialization::True)))
                    continue;

                // is the variable declared in a inner union?
                bool innerunion = false;
                for (const Scope *innerScope : scope2->nestedList) {
                    if (innerScope->type == Scope::eUnion) {
                        if (var.typeStartToken()->linenr() >= innerScope->bodyStart->linenr() &&
                            var.typeStartToken()->linenr() <= innerScope->bodyEnd->linenr()) {
                            innerunion = true;
                            break;
                        }
                    }
                }

                if (!innerunion) {
                    Alloc alloc = NO_ALLOC;
                    const Token *tok2 = tok;
                    if (tok->str() == "}")
                        tok2 = tok2->next();
                    const std::map<nonneg int, VariableValue> variableValue;
                    checkScopeForVariable(tok2, structvar, nullptr, nullptr, &alloc, var.name(), variableValue);
                }
            }
        }
    }
}

static VariableValue operator!(VariableValue v)
{
    v.notEqual = !v.notEqual;
    return v;
}
static bool operator==(const VariableValue & v, MathLib::bigint i)
{
    return v.notEqual ? (i != v.value) : (i == v.value);
}
static bool operator!=(const VariableValue & v, MathLib::bigint i)
{
    return v.notEqual ? (i == v.value) : (i != v.value);
}

static void conditionAlwaysTrueOrFalse(const Token *tok, const std::map<nonneg int, VariableValue> &variableValue, bool *alwaysTrue, bool *alwaysFalse)
{
    if (!tok)
        return;

    if (tok->hasKnownIntValue()) {
        if (tok->getKnownIntValue() == 0)
            *alwaysFalse = true;
        else
            *alwaysTrue = true;
        return;
    }

    if (tok->isName() || tok->str() == ".") {
        while (tok && tok->str() == ".")
            tok = tok->astOperand2();
        const std::map<nonneg int, VariableValue>::const_iterator it = variableValue.find(tok ? tok->varId() : ~0U);
        if (it != variableValue.end()) {
            *alwaysTrue = (it->second != 0LL);
            *alwaysFalse = (it->second == 0LL);
        }
    }

    else if (tok->isComparisonOp()) {
        if (variableValue.empty()) {
            return;
        }

        const Token *vartok, *numtok;
        if (tok->astOperand2() && tok->astOperand2()->isNumber()) {
            vartok = tok->astOperand1();
            numtok = tok->astOperand2();
        } else if (tok->astOperand1() && tok->astOperand1()->isNumber()) {
            vartok = tok->astOperand2();
            numtok = tok->astOperand1();
        } else {
            return;
        }

        while (vartok && vartok->str() == ".")
            vartok = vartok->astOperand2();

        const std::map<nonneg int, VariableValue>::const_iterator it = variableValue.find(vartok ? vartok->varId() : ~0U);
        if (it == variableValue.end())
            return;

        if (tok->str() == "==")
            *alwaysTrue  = (it->second == MathLib::toLongNumber(numtok->str()));
        else if (tok->str() == "!=")
            *alwaysTrue  = (it->second != MathLib::toLongNumber(numtok->str()));
        else
            return;
        *alwaysFalse = !(*alwaysTrue);
    }

    else if (tok->str() == "!") {
        bool t=false,f=false;
        conditionAlwaysTrueOrFalse(tok->astOperand1(), variableValue, &t, &f);
        if (t || f) {
            *alwaysTrue = !t;
            *alwaysFalse = !f;
        }
    }

    else if (tok->str() == "||") {
        bool t1=false, f1=false;
        conditionAlwaysTrueOrFalse(tok->astOperand1(), variableValue, &t1, &f1);
        bool t2=false, f2=false;
        if (!t1)
            conditionAlwaysTrueOrFalse(tok->astOperand2(), variableValue, &t2, &f2);
        *alwaysTrue = (t1 || t2);
        *alwaysFalse = (f1 && f2);
    }

    else if (tok->str() == "&&") {
        bool t1=false, f1=false;
        conditionAlwaysTrueOrFalse(tok->astOperand1(), variableValue, &t1, &f1);
        bool t2=false, f2=false;
        if (!f1)
            conditionAlwaysTrueOrFalse(tok->astOperand2(), variableValue, &t2, &f2);
        *alwaysTrue = (t1 && t2);
        *alwaysFalse = (f1 || f2);
    }
}

static bool isVariableUsed(const Token *tok, const Variable& var)
{
    if (!tok)
        return false;
    if (tok->str() == "&" && !tok->astOperand2())
        return false;
    if (tok->isConstOp())
        return isVariableUsed(tok->astOperand1(),var) || isVariableUsed(tok->astOperand2(),var);
    if (tok->varId() != var.declarationId())
        return false;
    if (!var.isArray())
        return true;

    const Token *parent = tok->astParent();
    while (Token::Match(parent, "[?:]"))
        parent = parent->astParent();
    // no dereference, then array is not "used"
    if (!Token::Match(parent, "*|["))
        return false;
    const Token *parent2 = parent->astParent();
    // TODO: handle function calls. There is a TODO assertion in TestUninitVar::uninitvar_arrays
    return !parent2 || parent2->isConstOp() || (parent2->str() == "=" && parent2->astOperand2() == parent);
}

bool CheckUninitVar::checkScopeForVariable(const Token *tok, const Variable& var, bool * const possibleInit, bool * const noreturn, Alloc* const alloc, const std::string &membervar, std::map<nonneg int, VariableValue> variableValue)
{
    const bool suppressErrors(possibleInit && *possibleInit);  // Assume that this is a variable delaratkon, rather than a fundef
    const bool printDebug = mSettings->debugwarnings;

    if (possibleInit)
        *possibleInit = false;

    int number_of_if = 0;

    if (var.declarationId() == 0U)
        return true;

    for (; tok; tok = tok->next()) {
        // End of scope..
        if (tok->str() == "}") {
            if (number_of_if && possibleInit)
                *possibleInit = true;

            // might be a noreturn function..
            if (mTokenizer->isScopeNoReturn(tok)) {
                if (noreturn)
                    *noreturn = true;
                return false;
            }

            break;
        }

        // Unconditional inner scope or try..
        if (tok->str() == "{" && Token::Match(tok->previous(), ",|;|{|}|try")) {
            bool possibleInitInner = false;
            if (checkScopeForVariable(tok->next(), var, &possibleInitInner, noreturn, alloc, membervar, variableValue))
                return true;
            tok = tok->link();
            if (possibleInitInner) {
                number_of_if = 1;
                if (possibleInit)
                    *possibleInit = true;
            }
            continue;
        }

        // assignment with nonzero constant..
        if (Token::Match(tok->previous(), "[;{}] %var% = - %name% ;"))
            variableValue[tok->varId()] = !VariableValue(0);

        // Inner scope..
        else if (Token::simpleMatch(tok, "if (")) {
            bool alwaysTrue = false;
            bool alwaysFalse = false;

            // Is variable assigned in condition?
            if (!membervar.empty()) {
                for (const Token *cond = tok->linkAt(1); cond != tok; cond = cond->previous()) {
                    if (cond->varId() == var.declarationId() && isMemberVariableAssignment(cond, membervar))
                        return true;
                }
            }

            conditionAlwaysTrueOrFalse(tok->next()->astOperand2(), variableValue, &alwaysTrue, &alwaysFalse);

            // initialization / usage in condition..
            if (!alwaysTrue && checkIfForWhileHead(tok->next(), var, suppressErrors, bool(number_of_if == 0), *alloc, membervar))
                return true;

            // checking if a not-zero variable is zero => bail out
            nonneg int condVarId = 0;
            VariableValue condVarValue(0);
            const Token *condVarTok = nullptr;
            if (alwaysFalse)
                ;
            else if (Token::simpleMatch(tok, "if (") &&
                     astIsVariableComparison(tok->next()->astOperand2(), "!=", "0", &condVarTok)) {
                const std::map<nonneg int,VariableValue>::const_iterator it = variableValue.find(condVarTok->varId());
                if (it != variableValue.end() && it->second != 0)
                    return true;   // this scope is not fully analysed => return true
                else {
                    condVarId = condVarTok->varId();
                    condVarValue = !VariableValue(0);
                }
            } else if (Token::simpleMatch(tok, "if (") && Token::Match(tok->next()->astOperand2(), "==|!=")) {
                const Token *condition = tok->next()->astOperand2();
                const Token *lhs = condition->astOperand1();
                const Token *rhs = condition->astOperand2();
                const Token *vartok = (lhs && lhs->hasKnownIntValue()) ? rhs : lhs;
                const Token *numtok = (lhs == vartok) ? rhs : lhs;
                while (Token::simpleMatch(vartok, "."))
                    vartok = vartok->astOperand2();
                if (vartok && vartok->varId() && numtok && numtok->hasKnownIntValue()) {
                    const std::map<nonneg int,VariableValue>::const_iterator it = variableValue.find(vartok->varId());
                    if (it != variableValue.end() && it->second != numtok->getKnownIntValue())
                        return true;   // this scope is not fully analysed => return true
                    condVarId = vartok->varId();
                    condVarValue = VariableValue(numtok->getKnownIntValue());
                    if (condition->str() == "!=")
                        condVarValue = !condVarValue;
                }
            }

            // goto the {
            tok = tok->next()->link()->next();

            if (!tok)
                break;
            if (tok->str() == "{") {
                bool possibleInitIf((!alwaysTrue && number_of_if > 0) || suppressErrors);
                bool noreturnIf = false;
                const bool initif = !alwaysFalse && checkScopeForVariable(tok->next(), var, &possibleInitIf, &noreturnIf, alloc, membervar, variableValue);

                // bail out for such code:
                //    if (a) x=0;    // conditional initialization
                //    if (b) return; // cppcheck doesn't know if b can be false when a is false.
                //    x++;           // it's possible x is always initialized
                if (!alwaysTrue && noreturnIf && number_of_if > 0) {
                    if (printDebug) {
                        std::string condition;
                        for (const Token *tok2 = tok->linkAt(-1); tok2 != tok; tok2 = tok2->next()) {
                            condition += tok2->str();
                            if (tok2->isName() && tok2->next()->isName())
                                condition += ' ';
                        }
                        reportError(tok, Severity::debug, "bailoutUninitVar", "bailout uninitialized variable checking for '" + var.name() + "'. can't determine if this condition can be false when previous condition is false: " + condition);
                    }
                    return true;
                }

                if (alwaysTrue && (initif || noreturnIf))
                    return true;

                std::map<nonneg int, VariableValue> varValueIf;
                if (!alwaysFalse && !initif && !noreturnIf) {
                    for (const Token *tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                        if (Token::Match(tok2, "[;{}.] %name% = - %name% ;"))
                            varValueIf[tok2->next()->varId()] = !VariableValue(0);
                        else if (Token::Match(tok2, "[;{}.] %name% = %num% ;"))
                            varValueIf[tok2->next()->varId()] = VariableValue(MathLib::toLongNumber(tok2->strAt(3)));
                    }
                }

                if (initif && condVarId > 0)
                    variableValue[condVarId] = !condVarValue;

                // goto the }
                tok = tok->link();

                if (!Token::simpleMatch(tok, "} else {")) {
                    if (initif || possibleInitIf) {
                        ++number_of_if;
                        if (number_of_if >= 2)
                            return true;
                    }
                } else {
                    // goto the {
                    tok = tok->tokAt(2);

                    bool possibleInitElse((!alwaysFalse && number_of_if > 0) || suppressErrors);
                    bool noreturnElse = false;
                    const bool initelse = !alwaysTrue && checkScopeForVariable(tok->next(), var, &possibleInitElse, &noreturnElse, alloc, membervar, variableValue);

                    std::map<nonneg int, VariableValue> varValueElse;
                    if (!alwaysTrue && !initelse && !noreturnElse) {
                        for (const Token *tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                            if (Token::Match(tok2, "[;{}.] %var% = - %name% ;"))
                                varValueElse[tok2->next()->varId()] = !VariableValue(0);
                            else if (Token::Match(tok2, "[;{}.] %var% = %num% ;"))
                                varValueElse[tok2->next()->varId()] = VariableValue(MathLib::toLongNumber(tok2->strAt(3)));
                        }
                    }

                    if (initelse && condVarId > 0 && !noreturnIf && !noreturnElse)
                        variableValue[condVarId] = condVarValue;

                    // goto the }
                    tok = tok->link();

                    if ((alwaysFalse || initif || noreturnIf) &&
                        (alwaysTrue || initelse || noreturnElse))
                        return true;

                    if (initif || initelse || possibleInitElse)
                        ++number_of_if;
                    if (!initif && !noreturnIf)
                        variableValue.insert(varValueIf.begin(), varValueIf.end());
                    if (!initelse && !noreturnElse)
                        variableValue.insert(varValueElse.begin(), varValueElse.end());
                }
            }
        }

        // = { .. }
        else if (Token::simpleMatch(tok, "= {")) {
            // end token
            const Token *end = tok->next()->link();

            // If address of variable is taken in the block then bail out
            if (var.isPointer() || var.isArray()) {
                if (Token::findmatch(tok->tokAt(2), "%varid%", end, var.declarationId()))
                    return true;
            } else if (Token::findmatch(tok->tokAt(2), "& %varid%", end, var.declarationId())) {
                return true;
            }

            const Token *errorToken = nullptr;
            visitAstNodes(tok->next(),
                          [&](const Token *child) {
                if (child->isUnaryOp("&"))
                    return ChildrenToVisit::none;
                if (child->str() == "," || child->str() == "{" || child->isConstOp())
                    return ChildrenToVisit::op1_and_op2;
                if (child->str() == "." && Token::Match(child->astOperand1(), "%varid%", var.declarationId()) && child->astOperand2() && child->astOperand2()->str() == membervar) {
                    errorToken = child;
                    return ChildrenToVisit::done;
                }
                return ChildrenToVisit::none;
            });

            if (errorToken) {
                uninitStructMemberError(errorToken->astOperand2(), errorToken->astOperand1()->str() + "." + membervar);
                return true;
            }

            // Skip block
            tok = end;
            continue;
        }

        // skip sizeof / offsetof
        if (isSizeOfEtc(tok))
            tok = tok->linkAt(1);

        // for/while..
        else if (Token::Match(tok, "for|while (") || Token::simpleMatch(tok, "do {")) {
            const bool forwhile = Token::Match(tok, "for|while (");

            // is variable initialized in for-head?
            if (forwhile && checkIfForWhileHead(tok->next(), var, tok->str() == "for", false, *alloc, membervar))
                return true;

            // goto the {
            const Token *tok2 = forwhile ? tok->next()->link()->next() : tok->next();

            if (tok2 && tok2->str() == "{") {
                const bool init = checkLoopBody(tok2, var, *alloc, membervar, (number_of_if > 0) || suppressErrors);

                // variable is initialized in the loop..
                if (init)
                    return true;

                // is variable used in for-head?
                bool initcond = false;
                if (!suppressErrors) {
                    const Token *startCond = forwhile ? tok->next() : tok->next()->link()->tokAt(2);
                    initcond = checkIfForWhileHead(startCond, var, false, bool(number_of_if == 0), *alloc, membervar);
                }

                // goto "}"
                tok = tok2->link();

                // do-while => goto ")"
                if (!forwhile) {
                    // Assert that the tokens are '} while ('
                    if (!Token::simpleMatch(tok, "} while (")) {
                        if (printDebug)
                            reportError(tok,Severity::debug,"","assertion failed '} while ('");
                        break;
                    }

                    // Goto ')'
                    tok = tok->linkAt(2);

                    if (!tok)
                        // bailout : invalid code / bad tokenizer
                        break;

                    if (initcond)
                        // variable is initialized in while-condition
                        return true;
                }
            }
        }

        // Unknown or unhandled inner scope
        else if (Token::simpleMatch(tok, ") {") || (Token::Match(tok, "%name% {") && tok->str() != "try")) {
            if (tok->str() == "struct" || tok->str() == "union") {
                tok = tok->linkAt(1);
                continue;
            }
            return true;
        }

        // bailout if there is ({
        if (Token::simpleMatch(tok, "( {")) {
            return true;
        }

        // bailout if there is assembler code or setjmp
        if (Token::Match(tok, "asm|setjmp (")) {
            return true;
        }

        // bailout if there is a goto label
        if (Token::Match(tok, "[;{}] %name% :")) {
            return true;
        }

        if (tok->str() == "?") {
            if (!tok->astOperand2())
                return true;
            const bool used1 = isVariableUsed(tok->astOperand2()->astOperand1(), var);
            const bool used0 = isVariableUsed(tok->astOperand2()->astOperand2(), var);
            const bool err = (number_of_if == 0) ? (used1 || used0) : (used1 && used0);
            if (err)
                uninitvarError(tok, var.nameToken()->str(), *alloc);

            // Todo: skip expression if there is no error
            return true;
        }

        if (Token::Match(tok, "return|break|continue|throw|goto")) {
            if (noreturn)
                *noreturn = true;

            tok = tok->next();
            while (tok && tok->str() != ";") {
                // variable is seen..
                if (tok->varId() == var.declarationId()) {
                    if (!membervar.empty()) {
                        if (!suppressErrors && Token::Match(tok, "%name% . %name%") && tok->strAt(2) == membervar && Token::Match(tok->next()->astParent(), "%cop%|return|throw|?"))
                            uninitStructMemberError(tok, tok->str() + "." + membervar);
                        else if (mTokenizer->isCPP() && !suppressErrors && Token::Match(tok, "%name%") && Token::Match(tok->astParent(), "return|throw|?"))
                            uninitStructMemberError(tok, tok->str() + "." + membervar);
                    }

                    // Use variable
                    else if (!suppressErrors && isVariableUsage(tok, var.isPointer(), *alloc))
                        uninitvarError(tok, tok->str(), *alloc);

                    return true;
                }

                else if (isSizeOfEtc(tok))
                    tok = tok->linkAt(1);

                else if (tok->str() == "?") {
                    if (!tok->astOperand2())
                        return true;
                    const bool used1 = isVariableUsed(tok->astOperand2()->astOperand1(), var);
                    const bool used0 = isVariableUsed(tok->astOperand2()->astOperand2(), var);
                    const bool err = (number_of_if == 0) ? (used1 || used0) : (used1 && used0);
                    if (err)
                        uninitvarError(tok, var.nameToken()->str(), *alloc);
                    return true;
                }

                tok = tok->next();
            }

            return (noreturn == nullptr);
        }

        // variable is seen..
        if (tok->varId() == var.declarationId()) {
            // calling function that returns uninit data through pointer..
            if (var.isPointer() && Token::simpleMatch(tok->next(), "=")) {
                const Token *rhs = tok->next()->astOperand2();
                while (rhs && rhs->isCast())
                    rhs = rhs->astOperand2() ? rhs->astOperand2() : rhs->astOperand1();
                if (rhs && Token::Match(rhs->previous(), "%name% (")) {
                    const Library::AllocFunc *allocFunc = mSettings->library.getAllocFuncInfo(rhs->astOperand1());
                    if (allocFunc && !allocFunc->initData) {
                        *alloc = NO_CTOR_CALL;
                        continue;
                    }
                }
            }
            if (mTokenizer->isCPP() && var.isPointer() && (var.typeStartToken()->isStandardType() || var.typeStartToken()->isEnumType() || (var.type() && var.type()->needInitialization == Type::NeedInitialization::True)) && Token::simpleMatch(tok->next(), "= new")) {
                *alloc = CTOR_CALL;

                // type has constructor(s)
                if (var.typeScope() && var.typeScope()->numConstructors > 0)
                    return true;

                // standard or enum type: check if new initializes the allocated memory
                if (var.typeStartToken()->isStandardType() || var.typeStartToken()->isEnumType()) {
                    // scalar new with initialization
                    if (Token::Match(tok->next(), "= new %type% ("))
                        return true;

                    // array new
                    if (Token::Match(tok->next(), "= new %type% [") && Token::simpleMatch(tok->linkAt(4), "] ("))
                        return true;
                }

                continue;
            }


            if (!membervar.empty()) {
                if (isMemberVariableAssignment(tok, membervar)) {
                    checkRhs(tok, var, *alloc, number_of_if, membervar);
                    return true;
                }

                if (isMemberVariableUsage(tok, var.isPointer(), *alloc, membervar)) {
                    uninitStructMemberError(tok, tok->str() + "." + membervar);
                    return true;
                }

                if (Token::Match(tok->previous(), "[(,] %name% [,)]"))
                    return true;

                if (Token::Match(tok->previous(), "= %var% . %var% ;") && membervar == tok->strAt(2))
                    return true;

            } else {
                // Use variable
                if (!suppressErrors && isVariableUsage(tok, var.isPointer(), *alloc)) {
                    uninitvarError(tok, tok->str(), *alloc);
                    return true;
                }

                else {
                    const Token *parent = tok;
                    while (parent->astParent() && ((astIsLHS(parent) && parent->astParent()->str() == "[") || parent->astParent()->isUnaryOp("*"))) {
                        parent = parent->astParent();
                        if (parent->str() == "[") {
                            if (const Token *errorToken = checkExpr(parent->astOperand2(), var, *alloc, number_of_if==0)) {
                                if (!suppressErrors)
                                    uninitvarError(errorToken, errorToken->expressionString(), *alloc);
                                return true;
                            }
                        }
                    }
                    if (Token::simpleMatch(parent->astParent(), "=") && astIsLHS(parent)) {
                        const Token *eq = parent->astParent();
                        if (const Token *errorToken = checkExpr(eq->astOperand2(), var, *alloc, number_of_if==0)) {
                            if (!suppressErrors)
                                uninitvarError(errorToken, errorToken->expressionString(), *alloc);
                            return true;
                        }
                    }

                    // assume that variable is assigned
                    return true;
                }
            }
        }
    }

    return false;
}

const Token *CheckUninitVar::checkExpr(const Token *tok, const Variable& var, const Alloc alloc, bool known, bool *bailout)
{
    if (!tok)
        return nullptr;
    if (isSizeOfEtc(tok->previous()))
        return nullptr;

    if (tok->astOperand1()) {
        bool bailout1 = false;
        const Token *errorToken = checkExpr(tok->astOperand1(), var, alloc, known, &bailout1);
        if (bailout && bailout1)
            *bailout = true;
        if (errorToken)
            return errorToken;
        if ((bailout1 || !known) && Token::Match(tok, "%oror%|&&|?"))
            return nullptr;
    }
    if (tok->astOperand2())
        return checkExpr(tok->astOperand2(), var, alloc, known, bailout);
    if (tok->varId() == var.declarationId()) {
        const Token *errorToken = isVariableUsage(tok, var.isPointer(), alloc);
        if (errorToken)
            return errorToken;
        else if (bailout)
            *bailout = true;
    }
    return nullptr;
}

bool CheckUninitVar::checkIfForWhileHead(const Token *startparentheses, const Variable& var, bool suppressErrors, bool isuninit, Alloc alloc, const std::string &membervar)
{
    const Token * const endpar = startparentheses->link();
    if (Token::Match(startparentheses, "( ! %name% %oror%") && startparentheses->tokAt(2)->getValue(0))
        suppressErrors = true;
    for (const Token *tok = startparentheses->next(); tok && tok != endpar; tok = tok->next()) {
        if (tok->varId() == var.declarationId()) {
            if (Token::Match(tok, "%name% . %name%")) {
                if (membervar.empty())
                    return true;
                if (tok->strAt(2) == membervar) {
                    if (isMemberVariableAssignment(tok, membervar))
                        return true;

                    if (!suppressErrors && isMemberVariableUsage(tok, var.isPointer(), alloc, membervar))
                        uninitStructMemberError(tok, tok->str() + "." + membervar);
                }
                continue;
            }

            if (const Token *errorToken = isVariableUsage(tok, var.isPointer(), alloc)) {
                if (suppressErrors)
                    continue;
                uninitvarError(errorToken, errorToken->expressionString(), alloc);
            }
            return true;
        }
        // skip sizeof / offsetof
        if (isSizeOfEtc(tok))
            tok = tok->linkAt(1);
        if ((!isuninit || !membervar.empty()) && tok->str() == "&&")
            suppressErrors = true;
    }
    return false;
}

/** recursively check loop, return error token */
const Token* CheckUninitVar::checkLoopBodyRecursive(const Token *start, const Variable& var, const Alloc alloc, const std::string &membervar, bool &bailout) const
{
    assert(start->str() == "{");

    const Token *errorToken = nullptr;

    const Token *const end = start->link();
    for (const Token *tok = start->next(); tok != end; tok = tok->next()) {
        // skip sizeof / offsetof
        if (isSizeOfEtc(tok)) {
            tok = tok->linkAt(1);
            continue;
        }

        if (Token::Match(tok, "asm ( %str% ) ;")) {
            bailout = true;
            return nullptr;
        }

        // for loop; skip third expression until loop body has been analyzed..
        if (tok->str() == ";" && Token::simpleMatch(tok->astParent(), ";") && Token::simpleMatch(tok->astParent()->astParent(), "(")) {
            const Token *top = tok->astParent()->astParent();
            if (!Token::simpleMatch(top->previous(), "for (") || !Token::simpleMatch(top->link(), ") {"))
                continue;
            const Token *bodyStart = top->link()->next();
            const Token *errorToken1 = checkLoopBodyRecursive(bodyStart, var, alloc, membervar, bailout);
            if (!errorToken)
                errorToken = errorToken1;
            if (bailout)
                return nullptr;
        }
        // for loop; skip loop body if there is third expression
        if (Token::simpleMatch(tok, ") {") &&
            Token::simpleMatch(tok->link()->previous(), "for (") &&
            Token::simpleMatch(tok->link()->astOperand2(), ";") &&
            Token::simpleMatch(tok->link()->astOperand2()->astOperand2(), ";")) {
            tok = tok->linkAt(1);
        }

        if (tok->str() == "{") {
            // switch => bailout
            if (tok->scope() && tok->scope()->type == Scope::ScopeType::eSwitch) {
                bailout = true;
                return nullptr;
            }

            const Token *errorToken1 = checkLoopBodyRecursive(tok, var, alloc, membervar, bailout);
            tok = tok->link();
            if (Token::simpleMatch(tok, "} else {")) {
                const Token *elseBody = tok->tokAt(2);
                const Token *errorToken2 = checkLoopBodyRecursive(elseBody, var, alloc, membervar, bailout);
                tok = elseBody->link();
                if (errorToken1 && errorToken2)
                    return errorToken1;
                if (errorToken2)
                    errorToken = errorToken2;
            }
            if (bailout)
                return nullptr;
            if (!errorToken)
                errorToken = errorToken1;
        }

        if (tok->varId() != var.declarationId())
            continue;

        bool conditionalUsage = false;
        for (const Token* parent = tok; parent; parent = parent->astParent()) {
            if (Token::Match(parent->astParent(), "%oror%|&&|?") && astIsRHS(parent)) {
                conditionalUsage = true;
                break;
            }
        }

        if (!membervar.empty()) {
            if (isMemberVariableAssignment(tok, membervar)) {
                bool assign = true;
                bool rhs = false;
                // Used for tracking if an ")" is inner or outer
                const Token *rpar = nullptr;
                for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "=")
                        rhs = true;

                    // Look at inner expressions but not outer expressions
                    if (!rpar && tok2->str() == "(")
                        rpar = tok2->link();
                    else if (tok2->str() == ")") {
                        // No rpar => this is an outer right parenthesis
                        if (!rpar)
                            break;
                        if (rpar == tok2)
                            rpar = nullptr;
                    }

                    if (tok2->str() == ";" || (!rpar && tok2->str() == ","))
                        break;
                    if (rhs && tok2->varId() == var.declarationId() && isMemberVariableUsage(tok2, var.isPointer(), alloc, membervar)) {
                        assign = false;
                        break;
                    }
                }
                if (assign) {
                    bailout = true;
                    return nullptr;
                }
            }
            if (isMemberVariableUsage(tok, var.isPointer(), alloc, membervar)) {
                if (!conditionalUsage)
                    return tok;
                if (!errorToken)
                    errorToken = tok;
            } else if (Token::Match(tok->previous(), "[(,] %name% [,)]")) {
                bailout = true;
                return nullptr;
            }
        } else {
            if (const Token *errtok = isVariableUsage(tok, var.isPointer(), alloc)) {
                if (!conditionalUsage)
                    return errtok;
                if (!errorToken)
                    errorToken = errtok;
            } else if (tok->strAt(1) == "=") {
                bool varIsUsedInRhs = false;
                visitAstNodes(tok->next()->astOperand2(), [&](const Token * t) {
                    if (!t)
                        return ChildrenToVisit::none;
                    if (t->varId() == var.declarationId()) {
                        varIsUsedInRhs = true;
                        return ChildrenToVisit::done;
                    }
                    if (isSizeOfEtc(t->previous()))
                        return ChildrenToVisit::none;
                    return ChildrenToVisit::op1_and_op2;
                });
                if (!varIsUsedInRhs) {
                    bailout = true;
                    return nullptr;
                }
            } else {
                bailout = true;
                return nullptr;
            }
        }
    }

    return errorToken;
}

bool CheckUninitVar::checkLoopBody(const Token *tok, const Variable& var, const Alloc alloc, const std::string &membervar, const bool suppressErrors)
{
    bool bailout = false;
    const Token *errorToken = checkLoopBodyRecursive(tok, var, alloc, membervar, bailout);

    if (!suppressErrors && !bailout && errorToken) {
        if (membervar.empty())
            uninitvarError(errorToken, errorToken->expressionString(), alloc);
        else
            uninitStructMemberError(errorToken, errorToken->expressionString() + "." + membervar);
        return true;
    }

    return bailout;
}

void CheckUninitVar::checkRhs(const Token *tok, const Variable &var, Alloc alloc, nonneg int number_of_if, const std::string &membervar)
{
    bool rhs = false;
    int indent = 0;
    while (nullptr != (tok = tok->next())) {
        if (tok->str() == "=")
            rhs = true;
        else if (rhs && tok->varId() == var.declarationId()) {
            if (membervar.empty() && isVariableUsage(tok, var.isPointer(), alloc))
                uninitvarError(tok, tok->str(), alloc);
            else if (!membervar.empty() && isMemberVariableUsage(tok, var.isPointer(), alloc, membervar))
                uninitStructMemberError(tok, tok->str() + "." + membervar);
            else if (Token::Match(tok, "%var% ="))
                break;
            else if (Token::Match(tok->previous(), "[(,&]"))
                break;
        } else if (tok->str() == ";" || (indent==0 && tok->str() == ","))
            break;
        else if (tok->str() == "(")
            ++indent;
        else if (tok->str() == ")") {
            if (indent == 0)
                break;
            --indent;
        } else if (tok->str() == "?" && tok->astOperand2()) {
            const bool used1 = isVariableUsed(tok->astOperand2()->astOperand1(), var);
            const bool used0 = isVariableUsed(tok->astOperand2()->astOperand2(), var);
            const bool err = (number_of_if == 0) ? (used1 || used0) : (used1 && used0);
            if (err)
                uninitvarError(tok, var.nameToken()->str(), alloc);
            break;
        } else if (isSizeOfEtc(tok))
            tok = tok->linkAt(1);

    }
}

static bool astIsLhs(const Token *tok)
{
    return tok && tok->astParent() && tok == tok->astParent()->astOperand1();
}

static bool astIsRhs(const Token *tok)
{
    return tok && tok->astParent() && tok == tok->astParent()->astOperand2();
}

static bool isVoidCast(const Token *tok)
{
    return Token::simpleMatch(tok, "(") && tok->isCast() && tok->valueType() && tok->valueType()->type == ValueType::Type::VOID && tok->valueType()->pointer == 0;
}

const Token* CheckUninitVar::isVariableUsage(bool cpp, const Token *vartok, const Library& library, bool pointer, Alloc alloc, int indirect)
{
    const Token *valueExpr = vartok;   // non-dereferenced , no address of value as variable
    while (Token::Match(valueExpr->astParent(), ".|::") && astIsRhs(valueExpr))
        valueExpr = valueExpr->astParent();
    // stuff we ignore..
    while (valueExpr->astParent()) {
        // *&x
        if (valueExpr->astParent()->isUnaryOp("&") && valueExpr->astParent()->astParent() && valueExpr->astParent()->astParent()->isUnaryOp("*"))
            valueExpr = valueExpr->astParent()->astParent();
        // (type &)x
        else if (valueExpr->astParent()->isCast() && valueExpr->astParent()->isUnaryOp("(") && Token::simpleMatch(valueExpr->astParent()->link()->previous(), "& )"))
            valueExpr = valueExpr->astParent();
        else
            break;
    }
    if (!pointer) {
        if (Token::Match(vartok, "%name% [.(]") && vartok->variable() && !vartok->variable()->isPointer())
            return nullptr;
        while (Token::simpleMatch(valueExpr->astParent(), ".") && astIsLhs(valueExpr) && valueExpr->astParent()->valueType() && valueExpr->astParent()->valueType()->pointer == 0)
            valueExpr = valueExpr->astParent();
    }
    const Token *derefValue = nullptr; // dereferenced value expression
    if (alloc != NO_ALLOC) {
        const int arrayDim = (vartok->variable() && vartok->variable()->isArray()) ? vartok->variable()->dimensions().size() : 1;
        int deref = 0;
        derefValue = valueExpr;
        while (Token::Match(derefValue->astParent(), "+|-|*|[|.") ||
               (derefValue->astParent() && derefValue->astParent()->isCast()) ||
               (deref < arrayDim && Token::simpleMatch(derefValue->astParent(), "&") && derefValue->astParent()->isBinaryOp())) {
            const Token * const derefValueParent = derefValue->astParent();
            if (derefValueParent->str() == "*") {
                if (derefValueParent->isUnaryOp("*"))
                    ++deref;
                else
                    break;
            } else if (derefValueParent->str() == "[") {
                if (astIsLhs(derefValue))
                    ++deref;
                else
                    break;
            } else if (Token::Match(derefValueParent, "[+-]")) {
                if (deref >= arrayDim)
                    break;
            } else if (derefValueParent->str() == ".")
                ++deref;
            derefValue = derefValueParent;
            if (deref < arrayDim)
                valueExpr = derefValue;
        }
        if (deref < arrayDim) {
            // todo compare deref with array dimensions
            derefValue = nullptr;
        }
    } else if (vartok->astParent() && vartok->astParent()->isUnaryOp("&")) {
        const Token *child = vartok->astParent();
        const Token *parent = child->astParent();
        while (parent && (parent->isCast() || parent->str() == "+")) {
            child = parent;
            parent = child->astParent();
        }
        if (parent && (parent->isUnaryOp("*") || (parent->str() == "[" && astIsLhs(child))))
            derefValue = parent;
    }

    if (!valueExpr->astParent())
        return nullptr;

    // FIXME handle address of!!
    if (derefValue && derefValue->astParent() && derefValue->astParent()->isUnaryOp("&"))
        return nullptr;

    // BAILOUT for binary & without parent
    if (Token::simpleMatch(valueExpr->astParent(), "&") && astIsRhs(valueExpr) && Token::Match(valueExpr->astParent()->tokAt(-3), "( %name% ) &"))
        return nullptr;

    // safe operations
    if (isVoidCast(valueExpr->astParent()))
        return nullptr;
    if (Token::simpleMatch(valueExpr->astParent(), ".")) {
        const Token *parent = valueExpr->astParent();
        while (Token::simpleMatch(parent, "."))
            parent = parent->astParent();
        if (isVoidCast(parent))
            return nullptr;
    }
    if (alloc != NO_ALLOC) {
        if (Token::Match(valueExpr->astParent(), "%comp%|%oror%|&&|?|!"))
            return nullptr;
        if (Token::Match(valueExpr->astParent(), "%or%|&") && valueExpr->astParent()->isBinaryOp())
            return nullptr;
        if (alloc == CTOR_CALL && derefValue && Token::simpleMatch(derefValue->astParent(), "(") && astIsLhs(derefValue))
            return nullptr;
        if (Token::simpleMatch(valueExpr->astParent(), "return"))
            return nullptr;
    }

    // Passing variable to function..
    if (Token::Match(valueExpr->astParent(), "[(,]") && (valueExpr->astParent()->str() == "," || astIsRhs(valueExpr))) {
        const Token *parent = valueExpr->astParent();
        while (Token::simpleMatch(parent, ","))
            parent = parent->astParent();
        if (Token::simpleMatch(parent, "{"))
            return valueExpr;
        const int use = isFunctionParUsage(valueExpr, library, pointer, alloc, indirect);
        return (use>0) ? valueExpr : nullptr;
    }
    if (derefValue && Token::Match(derefValue->astParent(), "[(,]") && (derefValue->astParent()->str() == "," || astIsRhs(derefValue))) {
        const int use = isFunctionParUsage(derefValue, library, false, NO_ALLOC, indirect);
        return (use>0) ? derefValue : nullptr;
    }
    if (valueExpr->astParent()->isUnaryOp("&")) {
        const Token *parent = valueExpr->astParent();
        if (Token::Match(parent->astParent(), "[(,]") && (parent->astParent()->str() == "," || astIsRhs(parent))) {
            const int use = isFunctionParUsage(valueExpr, library, pointer, alloc, indirect);
            return (use>0) ? valueExpr : nullptr;
        }
        return nullptr;
    }

    // Assignments;
    // * Is this LHS in assignment
    // * Passing address in RHS to pointer variable
    {
        const Token *tok = derefValue ? derefValue : valueExpr;
        if (alloc == NO_ALLOC) {
            while (tok->valueType() && tok->valueType()->pointer == 0 && Token::simpleMatch(tok->astParent(), "."))
                tok = tok->astParent();
        }
        if (Token::simpleMatch(tok->astParent(), "=")) {
            if (astIsLhs(tok))
                return nullptr;
            if (alloc != NO_ALLOC && astIsRhs(valueExpr))
                return nullptr;
        }
    }

    // Initialize reference variable
    if (Token::Match((derefValue ? derefValue : vartok)->astParent(), "(|=") && astIsRhs(derefValue ? derefValue : vartok)) {
        const Token *rhstok = derefValue ? derefValue : vartok;
        const Token *lhstok = rhstok->astParent()->astOperand1();
        const Variable *lhsvar = lhstok->variable();
        if (lhsvar && lhsvar->isReference() && lhsvar->nameToken() == lhstok)
            return nullptr;
    }

    // range for loop
    if (Token::simpleMatch(valueExpr->astParent(), ":") &&
        valueExpr->astParent()->astParent() &&
        Token::simpleMatch(valueExpr->astParent()->astParent()->previous(), "for (")) {
        if (astIsLhs(valueExpr))
            return nullptr;
        // Taking value by reference?
        const Token *lhs = valueExpr->astParent()->astOperand1();
        if (lhs && lhs->variable() && lhs->variable()->nameToken() == lhs && lhs->variable()->isReference())
            return nullptr;
    }

    // Stream read/write
    // FIXME this code is a hack!!
    if (cpp && Token::Match(valueExpr->astParent(), "<<|>>")) {
        if (isLikelyStreamRead(cpp, vartok->previous()))
            return nullptr;

        if (valueExpr->valueType() && valueExpr->valueType()->type == ValueType::Type::VOID)
            return nullptr;
    }
    if (astIsRhs(derefValue) && isLikelyStreamRead(cpp, derefValue->astParent()))
        return nullptr;

    // Assignment with overloaded &
    if (cpp && Token::simpleMatch(valueExpr->astParent(), "&") && astIsRhs(valueExpr)) {
        const Token *parent = valueExpr->astParent();
        while (Token::simpleMatch(parent, "&") && parent->isBinaryOp())
            parent = parent->astParent();
        if (!parent) {
            const Token *lhs = valueExpr->astParent();
            while (Token::simpleMatch(lhs, "&") && lhs->isBinaryOp())
                lhs = lhs->astOperand1();
            if (lhs && lhs->isName() && (!lhs->valueType() || lhs->valueType()->type <= ValueType::Type::CONTAINER))
                return nullptr; // <- possible assignment
        }
    }

    return derefValue ? derefValue : valueExpr;
}

const Token* CheckUninitVar::isVariableUsage(const Token *vartok, bool pointer, Alloc alloc, int indirect) const
{
    return CheckUninitVar::isVariableUsage(mTokenizer->isCPP(), vartok, mSettings->library, pointer, alloc, indirect);
}

/***
 * Is function parameter "used" so a "usage of uninitialized variable" can
 * be written? If parameter is passed "by value" then it is "used". If it
 * is passed "by reference" then it is not necessarily "used".
 * @return  -1 => unknown   0 => not used   1 => used
 */
int CheckUninitVar::isFunctionParUsage(const Token *vartok, const Library& library, bool pointer, Alloc alloc, int indirect)
{
    bool unknown = false;
    const Token *parent = getAstParentSkipPossibleCastAndAddressOf(vartok, &unknown);
    if (unknown || !Token::Match(parent, "[(,]"))
        return -1;

    // locate start parentheses in function call..
    int argumentNumber = 0;
    const Token *start = vartok;
    while (start && !Token::Match(start, "[;{}(]")) {
        if (start->str() == ")")
            start = start->link();
        else if (start->str() == ",")
            ++argumentNumber;
        start = start->previous();
    }
    if (!start)
        return -1;

    if (Token::simpleMatch(start->link(), ") {") && Token::Match(start->previous(), "if|for|while|switch"))
        return (!pointer || alloc == NO_ALLOC);

    // is this a function call?
    if (Token::Match(start->previous(), "%name% (")) {
        const bool address(vartok->previous()->str() == "&");
        const bool array(vartok->variable() && vartok->variable()->isArray());
        // check how function handle uninitialized data arguments..
        const Function *func = start->previous()->function();
        if (func) {
            const Variable *arg = func->getArgumentVar(argumentNumber);
            if (arg) {
                const Token *argStart = arg->typeStartToken();
                if (!address && !array && Token::Match(argStart, "%type% %name%| [,)]"))
                    return 1;
                if (pointer && !address && alloc == NO_ALLOC && Token::Match(argStart,  "%type% * %name% [,)]"))
                    return 1;
                while (argStart->previous() && argStart->previous()->isName())
                    argStart = argStart->previous();
                if (Token::Match(argStart, "const %type% & %name% [,)]")) {
                    // If it's a record it's ok to pass a partially uninitialized struct.
                    if (vartok->variable() && vartok->variable()->valueType() && vartok->variable()->valueType()->type == ValueType::Type::RECORD)
                        return -1;
                    return 1;
                }
                if ((pointer || address) && Token::Match(argStart, "const %type% %name% [") && Token::Match(argStart->linkAt(3), "] [,)]"))
                    return 1;
            }

        } else if (Token::Match(start->previous(), "if|while|for")) {
            // control-flow statement reading the variable "by value"
            return alloc == NO_ALLOC;
        } else {
            const bool isnullbad = library.isnullargbad(start->previous(), argumentNumber + 1);
            if (indirect == 0 && pointer && !address && isnullbad && alloc == NO_ALLOC)
                return 1;
            bool hasIndirect = false;
            const bool isuninitbad = library.isuninitargbad(start->previous(), argumentNumber + 1, indirect, &hasIndirect);
            if (alloc != NO_ALLOC)
                return (isnullbad || hasIndirect) && isuninitbad;
            return isuninitbad && (!address || isnullbad);
        }
    }

    // unknown
    return -1;
}

int CheckUninitVar::isFunctionParUsage(const Token *vartok, bool pointer, Alloc alloc, int indirect) const
{
    return CheckUninitVar::isFunctionParUsage(vartok, mSettings->library, pointer, alloc, indirect);
}

bool CheckUninitVar::isMemberVariableAssignment(const Token *tok, const std::string &membervar) const
{
    if (Token::Match(tok, "%name% . %name%") && tok->strAt(2) == membervar) {
        if (Token::Match(tok->tokAt(3), "[=.[]"))
            return true;
        else if (Token::Match(tok->tokAt(-2), "[(,=] &"))
            return true;
        else if (isLikelyStreamRead(mTokenizer->isCPP(), tok->previous()))
            return true;
        else if ((tok->previous() && tok->previous()->isConstOp()) || Token::Match(tok->previous(), "[|="))
            ; // member variable usage
        else if (tok->tokAt(3)->isConstOp())
            ; // member variable usage
        else if (Token::Match(tok->previous(), "[(,] %name% . %name% [,)]") &&
                 1 == isFunctionParUsage(tok, false, NO_ALLOC)) {
            return false;
        } else
            return true;
    } else if (tok->strAt(1) == "=")
        return true;
    else if (Token::Match(tok, "%var% . %name% (")) {
        const Token *ftok = tok->tokAt(2);
        if (!ftok->function() || !ftok->function()->isConst())
            // TODO: Try to determine if membervar is assigned in method
            return true;
    } else if (tok->strAt(-1) == "&") {
        if (Token::Match(tok->tokAt(-2), "[(,] & %name%")) {
            // locate start parentheses in function call..
            int argumentNumber = 0;
            const Token *ftok = tok;
            while (ftok && !Token::Match(ftok, "[;{}(]")) {
                if (ftok->str() == ")")
                    ftok = ftok->link();
                else if (ftok->str() == ",")
                    ++argumentNumber;
                ftok = ftok->previous();
            }

            // is this a function call?
            ftok = ftok ? ftok->previous() : nullptr;
            if (Token::Match(ftok, "%name% (")) {
                // check how function handle uninitialized data arguments..
                const Function *function = ftok->function();

                if (!function && mSettings) {
                    // Function definition not seen, check if direction is specified in the library configuration
                    const Library::ArgumentChecks::Direction argDirection = mSettings->library.getArgDirection(ftok, 1 + argumentNumber);
                    if (argDirection == Library::ArgumentChecks::Direction::DIR_IN)
                        return false;
                    else if (argDirection == Library::ArgumentChecks::Direction::DIR_OUT)
                        return true;
                }

                const Variable *arg      = function ? function->getArgumentVar(argumentNumber) : nullptr;
                const Token *argStart    = arg ? arg->typeStartToken() : nullptr;
                while (argStart && argStart->previous() && argStart->previous()->isName())
                    argStart = argStart->previous();
                if (Token::Match(argStart, "const struct| %type% * const| %name% [,)]"))
                    return false;
            }

            else if (ftok && Token::simpleMatch(ftok->previous(), "= * ("))
                return false;
        }
        return true;
    }
    return false;
}

bool CheckUninitVar::isMemberVariableUsage(const Token *tok, bool isPointer, Alloc alloc, const std::string &membervar) const
{
    if (Token::Match(tok->previous(), "[(,] %name% . %name% [,)]") &&
        tok->strAt(2) == membervar) {
        const int use = isFunctionParUsage(tok, isPointer, alloc);
        if (use == 1)
            return true;
    }

    if (isMemberVariableAssignment(tok, membervar))
        return false;

    if (Token::Match(tok, "%name% . %name%") && tok->strAt(2) == membervar && !(tok->tokAt(-2)->variable() && tok->tokAt(-2)->variable()->isReference())) {
        const Token *parent = tok->next()->astParent();
        if (parent && parent->isUnaryOp("&"))
            return false;
        return true;
    } else if (!isPointer && Token::Match(tok->previous(), "[(,] %name% [,)]") && isVariableUsage(tok, isPointer, alloc))
        return true;

    else if (!isPointer && Token::Match(tok->previous(), "= %name% ;"))
        return true;

    // = *(&var);
    else if (!isPointer &&
             Token::simpleMatch(tok->astParent(),"&") &&
             Token::simpleMatch(tok->astParent()->astParent(),"*") &&
             Token::Match(tok->astParent()->astParent()->astParent(), "= * (| &") &&
             tok->astParent()->astParent()->astParent()->astOperand2() == tok->astParent()->astParent())
        return true;

    else if (mSettings->certainty.isEnabled(Certainty::experimental) &&
             !isPointer &&
             Token::Match(tok->tokAt(-2), "[(,] & %name% [,)]") &&
             isVariableUsage(tok, isPointer, alloc))
        return true;

    return false;
}

void CheckUninitVar::uninitstringError(const Token *tok, const std::string &varname, bool strncpy_)
{
    reportError(tok, Severity::error, "uninitstring", "$symbol:" + varname + "\nDangerous usage of '$symbol'" + (strncpy_ ? " (strncpy doesn't always null-terminate it)." : " (not null-terminated)."), CWE_USE_OF_POTENTIALLY_DANGEROUS_FUNCTION, Certainty::normal);
}

void CheckUninitVar::uninitdataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitdata", "$symbol:" + varname + "\nMemory is allocated but not initialized: $symbol", CWE_USE_OF_UNINITIALIZED_VARIABLE, Certainty::normal);
}

void CheckUninitVar::uninitvarError(const Token *tok, const std::string &varname, ErrorPath errorPath)
{
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "uninitvar", "$symbol:" + varname + "\nUninitialized variable: $symbol", CWE_USE_OF_UNINITIALIZED_VARIABLE, Certainty::normal);
}

void CheckUninitVar::uninitStructMemberError(const Token *tok, const std::string &membername)
{
    reportError(tok,
                Severity::error,
                "uninitStructMember",
                "$symbol:" + membername + "\nUninitialized struct member: $symbol", CWE_USE_OF_UNINITIALIZED_VARIABLE, Certainty::normal);
}

static bool isLeafDot(const Token* tok)
{
    if (!tok)
        return false;
    const Token * parent = tok->astParent();
    if (!Token::simpleMatch(parent, "."))
        return false;
    if (parent->astOperand2() == tok)
        return true;
    return isLeafDot(parent);
}

void CheckUninitVar::valueFlowUninit()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // check every executable scope
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (isSizeOfEtc(tok)) {
                tok = tok->linkAt(1);
                continue;
            }
            if (!tok->variable() && !tok->isUnaryOp("*"))
                continue;
            if (Token::Match(tok, "%name% ("))
                continue;
            const Token* parent = tok->astParent();
            while (Token::simpleMatch(parent, "."))
                parent = parent->astParent();
            if (parent && parent->isUnaryOp("&"))
                continue;
            if (isVoidCast(parent))
                continue;
            auto v = std::find_if(tok->values().begin(), tok->values().end(), std::mem_fn(&ValueFlow::Value::isUninitValue));
            if (v == tok->values().end())
                continue;
            if (v->isInconclusive())
                continue;
            if (v->indirect > 1 || v->indirect < 0)
                continue;
            bool uninitderef = false;
            if (tok->variable()) {
                bool unknown;
                const bool isarray = !tok->variable() || tok->variable()->isArray();
                const bool ispointer = astIsPointer(tok) && !isarray;
                const bool deref = CheckNullPointer::isPointerDeRef(tok, unknown, mSettings);
                if (ispointer && v->indirect == 1 && !deref)
                    continue;
                if (isarray && !deref)
                    continue;
                uninitderef = deref && v->indirect == 0;
                const bool isleaf = isLeafDot(tok) || uninitderef;
                if (Token::Match(tok->astParent(), ". %var%") && !isleaf)
                    continue;
            }
            if (!(Token::Match(tok->astParent(), ". %name% (") && uninitderef) &&
                isVariableChanged(tok, v->indirect, mSettings, mTokenizer->isCPP()))
                continue;
            bool inconclusive = false;
            if (isVariableChangedByFunctionCall(tok, v->indirect, mSettings, &inconclusive) || inconclusive)
                continue;
            uninitvarError(tok, tok->expressionString(), v->errorPath);
            const Token* nextTok = nextAfterAstRightmostLeaf(parent);
            if (nextTok == scope->bodyEnd)
                break;
            tok = nextTok ? nextTok : tok;
        }
    }
}

std::string CheckUninitVar::MyFileInfo::toString() const
{
    return CTU::toString(unsafeUsage);
}

Check::FileInfo *CheckUninitVar::getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const
{
    const CheckUninitVar checker(tokenizer, settings, nullptr);
    return checker.getFileInfo();
}

static bool isVariableUsage(const Check *check, const Token *vartok, MathLib::bigint *value)
{
    (void)value;
    const CheckUninitVar *c = dynamic_cast<const CheckUninitVar *>(check);
    return c && c->isVariableUsage(vartok, true, CheckUninitVar::Alloc::ARRAY);
}

Check::FileInfo *CheckUninitVar::getFileInfo() const
{
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage = CTU::getUnsafeUsage(mTokenizer, mSettings, this, ::isVariableUsage);
    if (unsafeUsage.empty())
        return nullptr;

    MyFileInfo *fileInfo = new MyFileInfo;
    fileInfo->unsafeUsage = unsafeUsage;
    return fileInfo;
}

Check::FileInfo * CheckUninitVar::loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const
{
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage = CTU::loadUnsafeUsageListFromXml(xmlElement);
    if (unsafeUsage.empty())
        return nullptr;

    MyFileInfo *fileInfo = new MyFileInfo;
    fileInfo->unsafeUsage = unsafeUsage;
    return fileInfo;
}

bool CheckUninitVar::analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger)
{
    if (!ctu)
        return false;
    bool foundErrors = false;
    (void)settings; // This argument is unused

    const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> callsMap = ctu->getCallsMap();

    for (Check::FileInfo *fi1 : fileInfo) {
        const MyFileInfo *fi = dynamic_cast<MyFileInfo*>(fi1);
        if (!fi)
            continue;
        for (const CTU::FileInfo::UnsafeUsage &unsafeUsage : fi->unsafeUsage) {
            const CTU::FileInfo::FunctionCall *functionCall = nullptr;

            const std::list<ErrorMessage::FileLocation> &locationList =
                CTU::FileInfo::getErrorPath(CTU::FileInfo::InvalidValueType::uninit,
                                            unsafeUsage,
                                            callsMap,
                                            "Using argument ARG",
                                            &functionCall,
                                            false);
            if (locationList.empty())
                continue;

            const ErrorMessage errmsg(locationList,
                                      emptyString,
                                      Severity::error,
                                      "Using argument " + unsafeUsage.myArgumentName + " that points at uninitialized variable " + functionCall->callArgumentExpression,
                                      "ctuuninitvar",
                                      CWE_USE_OF_UNINITIALIZED_VARIABLE,
                                      Certainty::normal);
            errorLogger.reportErr(errmsg);

            foundErrors = true;
        }
    }
    return foundErrors;
}
