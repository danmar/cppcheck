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
#include "checkuninitvar.h"
#include "astutils.h"
#include "mathlib.h"
#include "checknullpointer.h"   // CheckNullPointer::isPointerDeref
#include "symboldatabase.h"
#include <map>
#include <cassert>
#include <stack>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckUninitVar instance;
}

//---------------------------------------------------------------------------

// CWE ids used:
static const struct CWE CWE676(676U);
static const struct CWE CWE908(908U);
static const struct CWE CWE825(825U);


void CheckUninitVar::check()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    std::list<Scope>::const_iterator scope;

    std::set<std::string> arrayTypeDefs;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name% [") && tok->variable() && Token::Match(tok->variable()->typeStartToken(), "%type% %var% ;"))
            arrayTypeDefs.insert(tok->variable()->typeStartToken()->str());
    }

    // check every executable scope
    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (scope->isExecutable()) {
            checkScope(&*scope, arrayTypeDefs);
        }
    }
}

void CheckUninitVar::checkScope(const Scope* scope, const std::set<std::string> &arrayTypeDefs)
{
    for (std::list<Variable>::const_iterator i = scope->varlist.begin(); i != scope->varlist.end(); ++i) {
        if ((_tokenizer->isCPP() && i->type() && !i->isPointer() && i->type()->needInitialization != Type::True) ||
            i->isStatic() || i->isExtern() || i->isReference())
            continue;

        // don't warn for try/catch exception variable
        if (i->isThrow())
            continue;

        if (i->nameToken()->strAt(1) == "(" || i->nameToken()->strAt(1) == "{"  || i->nameToken()->strAt(1) == ":")
            continue;

        if (Token::Match(i->nameToken(), "%name% =")) { // Variable is initialized, but Rhs might be not
            checkRhs(i->nameToken(), *i, NO_ALLOC, 0U, emptyString);
            continue;
        }
        if (Token::Match(i->nameToken(), "%name% ) (") && Token::simpleMatch(i->nameToken()->linkAt(2), ") =")) { // Function pointer is initialized, but Rhs might be not
            checkRhs(i->nameToken()->linkAt(2)->next(), *i, NO_ALLOC, 0U, emptyString);
            continue;
        }

        if (i->isArray() || i->isPointerToArray()) {
            const Token *tok = i->nameToken()->next();
            if (i->isPointerToArray())
                tok = tok->next();
            while (Token::simpleMatch(tok->link(), "] ["))
                tok = tok->link()->next();
            if (Token::Match(tok->link(), "] =|{"))
                continue;
        }

        bool stdtype = _tokenizer->isC() && arrayTypeDefs.find(i->typeStartToken()->str()) == arrayTypeDefs.end();
        const Token* tok = i->typeStartToken();
        for (; tok != i->nameToken() && tok->str() != "<"; tok = tok->next()) {
            if (tok->isStandardType() || tok->isEnumType())
                stdtype = true;
        }
        if (i->isArray() && !stdtype)
            continue;

        while (tok && tok->str() != ";")
            tok = tok->next();
        if (!tok)
            continue;

        if (tok->astParent() && Token::simpleMatch(tok->astParent()->previous(), "for (") &&
            checkLoopBody(tok->astParent()->link()->next(), *i, i->isArray() ? ARRAY : NO_ALLOC, emptyString, true))
            continue;

        if (i->isArray()) {
            Alloc alloc = ARRAY;
            checkScopeForVariable(tok, *i, nullptr, nullptr, &alloc, emptyString);
            continue;
        }
        if (stdtype || i->isPointer()) {
            Alloc alloc = NO_ALLOC;
            checkScopeForVariable(tok, *i, nullptr, nullptr, &alloc, emptyString);
        }
        if (i->type())
            checkStruct(tok, *i);
    }

    if (scope->function) {
        for (unsigned int i = 0; i < scope->function->argCount(); i++) {
            const Variable *arg = scope->function->getArgumentVar(i);
            if (arg && arg->declarationId() && Token::Match(arg->typeStartToken(), "%type% * %name% [,)]")) {
                // Treat the pointer as initialized until it is assigned by malloc
                for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
                    if (Token::Match(tok, "[;{}] %varid% = %name% (", arg->declarationId()) &&
                        _settings->library.returnuninitdata.count(tok->strAt(3)) == 1U) {
                        if (arg->typeStartToken()->strAt(-1) == "struct" || (arg->type() && arg->type()->isStructType()))
                            checkStruct(tok, *arg);
                        else if (arg->typeStartToken()->isStandardType() || arg->typeStartToken()->isEnumType()) {
                            Alloc alloc = NO_ALLOC;
                            checkScopeForVariable(tok->next(), *arg, nullptr, nullptr, &alloc, emptyString);
                        }
                    }
                }
            }
        }
    }
}

void CheckUninitVar::checkStruct(const Token *tok, const Variable &structvar)
{
    const Token *typeToken = structvar.typeStartToken();
    const SymbolDatabase * symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t j = 0U; j < symbolDatabase->classAndStructScopes.size(); ++j) {
        const Scope *scope2 = symbolDatabase->classAndStructScopes[j];
        if (scope2->className == typeToken->str() && scope2->numConstructors == 0U) {
            for (std::list<Variable>::const_iterator it = scope2->varlist.begin(); it != scope2->varlist.end(); ++it) {
                const Variable &var = *it;

                if (var.hasDefault() || var.isArray() || (!_tokenizer->isC() && var.isClass() && (!var.type() || var.type()->needInitialization != Type::True)))
                    continue;

                // is the variable declared in a inner union?
                bool innerunion = false;
                for (std::list<Scope>::const_iterator it2 = symbolDatabase->scopeList.begin(); it2 != symbolDatabase->scopeList.end(); ++it2) {
                    const Scope &innerScope = *it2;
                    if (innerScope.type == Scope::eUnion && innerScope.nestedIn == scope2) {
                        if (var.typeStartToken()->linenr() >= innerScope.classStart->linenr() &&
                            var.typeStartToken()->linenr() <= innerScope.classEnd->linenr()) {
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
                    checkScopeForVariable(tok2, structvar, nullptr, nullptr, &alloc, var.name());
                }
            }
        }
    }
}

struct VariableValue {
    explicit VariableValue(MathLib::bigint val = 0) : value(val), notEqual(false) {}
    MathLib::bigint value;
    bool notEqual;
};
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

static void conditionAlwaysTrueOrFalse(const Token *tok, const std::map<unsigned int, VariableValue> &variableValue, bool *alwaysTrue, bool *alwaysFalse)
{
    if (!tok)
        return;

    if (tok->isName() || tok->str() == ".") {
        while (tok && tok->str() == ".")
            tok = tok->astOperand2();
        const std::map<unsigned int, VariableValue>::const_iterator it = variableValue.find(tok ? tok->varId() : ~0U);
        if (it != variableValue.end()) {
            *alwaysTrue = (it->second != 0LL);
            *alwaysFalse = (it->second == 0LL);
        }
    }

    else if (tok->isComparisonOp()) {
        if (tok->values().size() == 1U && tok->values().front().isKnown()) {
            if (tok->values().front().intvalue)
                *alwaysTrue = true;
            else
                *alwaysFalse = true;
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

        const std::map<unsigned int, VariableValue>::const_iterator it = variableValue.find(vartok ? vartok->varId() : ~0U);
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
        if (t||f) {
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

bool CheckUninitVar::checkScopeForVariable(const Token *tok, const Variable& var, bool * const possibleInit, bool * const noreturn, Alloc* const alloc, const std::string &membervar)
{
    const bool suppressErrors(possibleInit && *possibleInit);
    const bool printDebug = _settings->debugwarnings;

    if (possibleInit)
        *possibleInit = false;

    unsigned int number_of_if = 0;

    if (var.declarationId() == 0U)
        return true;

    // variable values
    std::map<unsigned int, VariableValue> variableValue;

    for (; tok; tok = tok->next()) {
        // End of scope..
        if (tok->str() == "}") {
            if (number_of_if && possibleInit)
                *possibleInit = true;

            // might be a noreturn function..
            if (_tokenizer->IsScopeNoReturn(tok)) {
                if (noreturn)
                    *noreturn = true;
                return false;
            }

            break;
        }

        // Unconditional inner scope or try..
        if (tok->str() == "{" && Token::Match(tok->previous(), ";|{|}|try")) {
            if (checkScopeForVariable(tok->next(), var, possibleInit, noreturn, alloc, membervar))
                return true;
            tok = tok->link();
            continue;
        }

        // assignment with nonzero constant..
        if (Token::Match(tok->previous(), "[;{}] %var% = - %name% ;"))
            variableValue[tok->varId()] = !VariableValue(0);

        // Inner scope..
        else if (Token::simpleMatch(tok, "if (")) {
            bool alwaysTrue = false;
            bool alwaysFalse = false;

            conditionAlwaysTrueOrFalse(tok->next()->astOperand2(), variableValue, &alwaysTrue, &alwaysFalse);

            // initialization / usage in condition..
            if (!alwaysTrue && checkIfForWhileHead(tok->next(), var, suppressErrors, bool(number_of_if == 0), *alloc, membervar))
                return true;

            // checking if a not-zero variable is zero => bail out
            unsigned int condVarId = 0;
            VariableValue condVarValue(0);
            const Token *condVarTok = nullptr;
            if (alwaysFalse)
                ;
            else if (Token::simpleMatch(tok, "if (") &&
                     astIsVariableComparison(tok->next()->astOperand2(), "!=", "0", &condVarTok)) {
                std::map<unsigned int,VariableValue>::const_iterator it = variableValue.find(condVarTok->varId());
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
                const Token *vartok = rhs && rhs->isNumber() ? lhs : rhs;
                const Token *numtok = rhs && rhs->isNumber() ? rhs : lhs;
                while (Token::simpleMatch(vartok, "."))
                    vartok = vartok->astOperand2();
                if (vartok && vartok->varId() && numtok) {
                    std::map<unsigned int,VariableValue>::const_iterator it = variableValue.find(vartok->varId());
                    if (it != variableValue.end() && it->second != MathLib::toLongNumber(numtok->str()))
                        return true;   // this scope is not fully analysed => return true
                    else {
                        condVarId = vartok->varId();
                        condVarValue = VariableValue(MathLib::toLongNumber(numtok->str()));
                        if (condition->str() == "!=")
                            condVarValue = !condVarValue;
                    }
                }
            }

            // goto the {
            tok = tok->next()->link()->next();

            if (!tok)
                break;
            if (tok->str() == "{") {
                bool possibleInitIf((!alwaysTrue && number_of_if > 0) || suppressErrors);
                bool noreturnIf = false;
                const bool initif = !alwaysFalse && checkScopeForVariable(tok->next(), var, &possibleInitIf, &noreturnIf, alloc, membervar);

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
                        reportError(tok, Severity::debug, "debug", "bailout uninitialized variable checking for '" + var.name() + "'. can't determine if this condition can be false when previous condition is false: " + condition);
                    }
                    return true;
                }

                if (alwaysTrue && (initif || noreturnIf))
                    return true;

                std::map<unsigned int, VariableValue> varValueIf;
                if (!alwaysFalse && !initif && !noreturnIf) {
                    for (const Token *tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                        if (Token::Match(tok2, "[;{}.] %name% = - %name% ;"))
                            varValueIf[tok2->next()->varId()] = !VariableValue(0);
                        else if (Token::Match(tok2, "[;{}.] %name% = %num% ;"))
                            varValueIf[tok2->next()->varId()] = VariableValue(MathLib::toLongNumber(tok2->strAt(3)));
                    }
                }

                if (initif && condVarId > 0U)
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
                    const bool initelse = !alwaysTrue && checkScopeForVariable(tok->next(), var, &possibleInitElse, &noreturnElse, alloc, membervar);

                    std::map<unsigned int, VariableValue> varValueElse;
                    if (!alwaysTrue && !initelse && !noreturnElse) {
                        for (const Token *tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                            if (Token::Match(tok2, "[;{}.] %var% = - %name% ;"))
                                varValueElse[tok2->next()->varId()] = !VariableValue(0);
                            else if (Token::Match(tok2, "[;{}.] %var% = %num% ;"))
                                varValueElse[tok2->next()->varId()] = VariableValue(MathLib::toLongNumber(tok2->strAt(3)));
                        }
                    }

                    if (initelse && condVarId > 0U && !noreturnIf && !noreturnElse)
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

            // Skip block
            tok = end;
            continue;
        }

        // skip sizeof / offsetof
        if (Token::Match(tok, "sizeof|typeof|offsetof|decltype ("))
            tok = tok->next()->link();

        // for/while..
        else if (Token::Match(tok, "for|while (") || Token::simpleMatch(tok, "do {")) {
            const bool forwhile = Token::Match(tok, "for|while (");

            // is variable initialized in for-head (don't report errors yet)?
            if (forwhile && checkIfForWhileHead(tok->next(), var, true, false, *alloc, membervar))
                return true;

            // goto the {
            const Token *tok2 = forwhile ? tok->next()->link()->next() : tok->next();

            if (tok2 && tok2->str() == "{") {
                bool init = checkLoopBody(tok2, var, *alloc, membervar, (number_of_if > 0) || suppressErrors);

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
                        if (Token::Match(tok, "%name% . %name% ;|%cop%") && tok->strAt(2) == membervar)
                            uninitStructMemberError(tok, tok->str() + "." + membervar);
                        else
                            return true;
                    }

                    // Use variable
                    else if (!suppressErrors && isVariableUsage(tok, var.isPointer(), *alloc))
                        uninitvarError(tok, tok->str(), *alloc);

                    else
                        // assume that variable is assigned
                        return true;
                }

                else if (Token::Match(tok, "sizeof|typeof|offsetof|decltype ("))
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

            return bool(noreturn==nullptr);
        }

        // variable is seen..
        if (tok->varId() == var.declarationId()) {
            // calling function that returns uninit data through pointer..
            if (var.isPointer() &&
                Token::Match(tok->next(), "= %name% (") &&
                Token::simpleMatch(tok->linkAt(3), ") ;") &&
                _settings->library.returnuninitdata.count(tok->strAt(2)) > 0U) {
                *alloc = NO_CTOR_CALL;
                continue;
            }
            if (var.isPointer() && (var.typeStartToken()->isStandardType() || var.typeStartToken()->isEnumType() || (var.type() && var.type()->needInitialization == Type::True)) && Token::simpleMatch(tok->next(), "= new")) {
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
                    if (Token::Match(tok->next(), "= new %type% [")) {
                        const Token* tokClosingBracket=tok->linkAt(4);
                        // array new with initialization
                        if (tokClosingBracket && Token::simpleMatch(tokClosingBracket->next(), "( )"))
                            return true;
                    }
                }

                continue;
            }


            if (!membervar.empty()) {
                if (isMemberVariableAssignment(tok, membervar)) {
                    checkRhs(tok, var, *alloc, number_of_if, membervar);
                    return true;
                }

                if (isMemberVariableUsage(tok, var.isPointer(), *alloc, membervar))
                    uninitStructMemberError(tok, tok->str() + "." + membervar);

                else if (Token::Match(tok->previous(), "[(,] %name% [,)]"))
                    return true;

            } else {
                // Use variable
                if (!suppressErrors && isVariableUsage(tok, var.isPointer(), *alloc))
                    uninitvarError(tok, tok->str(), *alloc);

                else {
                    if (tok->strAt(1) == "=")
                        checkRhs(tok, var, *alloc, number_of_if, emptyString);

                    // assume that variable is assigned
                    return true;
                }
            }
        }
    }

    return false;
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

            if (isVariableUsage(tok, var.isPointer(), alloc)) {
                if (suppressErrors)
                    continue;
                uninitvarError(tok, tok->str(), alloc);
            }
            return true;
        }
        if (Token::Match(tok, "sizeof|decltype|offsetof ("))
            tok = tok->next()->link();
        if ((!isuninit || !membervar.empty()) && tok->str() == "&&")
            suppressErrors = true;
    }
    return false;
}

bool CheckUninitVar::checkLoopBody(const Token *tok, const Variable& var, const Alloc alloc, const std::string &membervar, const bool suppressErrors)
{
    const Token *usetok = nullptr;

    assert(tok->str() == "{");

    for (const Token * const end = tok->link(); tok != end; tok = tok->next()) {
        if (tok->varId() == var.declarationId()) {
            if (!membervar.empty()) {
                if (isMemberVariableAssignment(tok, membervar)) {
                    bool assign = true;
                    bool rhs = false;
                    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == "=")
                            rhs = true;
                        if (tok2->str() == ";")
                            break;
                        if (rhs && tok2->varId() == var.declarationId() && isMemberVariableUsage(tok2, var.isPointer(), alloc, membervar)) {
                            assign = false;
                            break;
                        }
                    }
                    if (assign)
                        return true;
                }

                if (Token::Match(tok, "%name% ="))
                    return true;

                if (isMemberVariableUsage(tok, var.isPointer(), alloc, membervar))
                    usetok = tok;
                else if (Token::Match(tok->previous(), "[(,] %name% [,)]"))
                    return true;
            } else {
                if (isVariableUsage(tok, var.isPointer(), alloc))
                    usetok = tok;
                else if (tok->strAt(1) == "=") {
                    // Is var used in rhs?
                    bool rhs = false;
                    std::stack<const Token *> tokens;
                    tokens.push(tok->next()->astOperand2());
                    while (!tokens.empty()) {
                        const Token *t = tokens.top();
                        tokens.pop();
                        if (!t)
                            continue;
                        if (t->varId() == var.declarationId()) {
                            // var is used in rhs
                            rhs = true;
                            break;
                        }
                        if (Token::simpleMatch(t->previous(),"sizeof ("))
                            continue;
                        tokens.push(t->astOperand1());
                        tokens.push(t->astOperand2());
                    }
                    if (!rhs)
                        return true;
                } else {
                    return true;
                }
            }
        }

        if (Token::Match(tok, "sizeof|typeof ("))
            tok = tok->next()->link();

        if (Token::Match(tok, "asm ( %str% ) ;"))
            return true;
    }

    if (!suppressErrors && usetok) {
        if (membervar.empty())
            uninitvarError(usetok, usetok->str(), alloc);
        else
            uninitStructMemberError(usetok, usetok->str() + "." + membervar);
        return true;
    }

    return false;
}

void CheckUninitVar::checkRhs(const Token *tok, const Variable &var, Alloc alloc, unsigned int number_of_if, const std::string &membervar)
{
    bool rhs = false;
    unsigned int indent = 0;
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
        } else if (Token::simpleMatch(tok, "sizeof ("))
            tok = tok->next()->link();
    }
}

bool CheckUninitVar::isVariableUsage(const Token *vartok, bool pointer, Alloc alloc) const
{
    if (alloc == NO_ALLOC && ((Token::Match(vartok->previous(), "return|delete") && vartok->strAt(1) != "=") || (vartok->strAt(-1) == "]" && vartok->linkAt(-1)->strAt(-1) == "delete")))
        return true;

    // Passing variable to typeof/__alignof__
    if (Token::Match(vartok->tokAt(-3), "typeof|__alignof__ ( * %name%"))
        return false;

    // Accessing Rvalue member using "." or "->"
    if (vartok->strAt(1) == "." && vartok->strAt(-1) != "&") {
        // Is struct member passed to function?
        if (!pointer && Token::Match(vartok->previous(), "[,(] %name% . %name%")) {
            // TODO: there are FN currently:
            // - should only return false if struct member is (or might be) array.
            // - should only return false if function argument is (or might be) non-const pointer or reference
            const Token *tok2 = vartok->next();
            do {
                tok2 = tok2->tokAt(2);
            } while (Token::Match(tok2, ". %name%"));
            if (Token::Match(tok2, "[,)]"))
                return false;
        } else if (pointer && alloc != CTOR_CALL && Token::Match(vartok, "%name% . %name% (")) {
            return true;
        }

        bool assignment = false;
        const Token* parent = vartok->astParent();
        while (parent) {
            if (parent->str() == "=") {
                assignment = true;
                break;
            }
            if (alloc != NO_ALLOC && parent->str() == "(") {
                if (!_settings->library.isFunctionConst(parent->strAt(-1), true)) {
                    assignment = true;
                    break;
                }
            }
            parent = parent->astParent();
        }
        if (!assignment)
            return true;
    }

    // Passing variable to function..
    if (Token::Match(vartok->previous(), "[(,] %name% [,)]") || Token::Match(vartok->tokAt(-2), "[(,] & %name% [,)]")) {
        const int use = isFunctionParUsage(vartok, pointer, alloc);
        if (use >= 0)
            return (use>0);
    }

    if (Token::Match(vartok->previous(), "++|--|%cop%")) {
        if (_tokenizer->isCPP() && alloc == ARRAY && Token::Match(vartok->tokAt(-4), "& %var% =|( *"))
            return false;

        if (_tokenizer->isCPP() && Token::Match(vartok->previous(), ">>|<<")) {
            const Token* tok2 = vartok->previous();
            if (Token::simpleMatch(tok2->astOperand1(), ">>"))
                return false; // Looks like stream operator, initializes the variable
            if (Token::simpleMatch(tok2, "<<")) {
                // Looks like stream operator, but could also initialize the variable. Check lhs.
                do {
                    tok2 = tok2->astOperand1();
                } while (Token::simpleMatch(tok2, "<<"));
                if (tok2 && tok2->strAt(-1) == "::")
                    tok2 = tok2->previous();
                if (tok2 && (Token::simpleMatch(tok2->previous(), "std ::") || (tok2->variable() && tok2->variable()->isStlType()) || tok2->isStandardType() || tok2->isEnumType()))
                    return true;
            }
            const Variable *var = vartok->tokAt(-2)->variable();
            return (var && (var->typeStartToken()->isStandardType() || var->typeStartToken()->isEnumType()));
        }

        // is there something like: ; "*((&var ..expr.. ="  => the variable is assigned
        if (vartok->previous()->str() == "&" && !vartok->previous()->astOperand2())
            return false;

        // bailout to avoid fp for 'int x = 2 + x();' where 'x()' is a unseen preprocessor macro (seen in linux)
        if (!pointer && vartok->next() && vartok->next()->str() == "(")
            return false;

        if (vartok->previous()->str() != "&" || !Token::Match(vartok->tokAt(-2), "[(,=?:]")) {
            if (alloc != NO_ALLOC && vartok->previous()->str() == "*") {
                // TestUninitVar::isVariableUsageDeref()
                const Token *parent = vartok->previous()->astParent();
                if (parent && parent->str() == "=" && parent->astOperand1() == vartok->previous())
                    return false;
                if (vartok->variable() && vartok->variable()->dimensions().size() >= 2)
                    return false;
                return true;
            }
            return alloc == NO_ALLOC;
        }
    }

    if (alloc == NO_ALLOC && Token::Match(vartok->previous(), "= %name% ;|%cop%")) {
        // taking reference?
        const Token *prev = vartok->tokAt(-2);
        while (Token::Match(prev, "%name%|*"))
            prev = prev->previous();
        if (!Token::simpleMatch(prev, "&"))
            return true;
    }

    bool unknown = false;
    if (pointer && alloc == NO_ALLOC && CheckNullPointer::isPointerDeRef(vartok, unknown)) {
        // function parameter?
        bool functionParameter = false;
        if (Token::Match(vartok->tokAt(-2), "%name% (") || vartok->previous()->str() == ",")
            functionParameter = true;

        // if this is not a function parameter report this dereference as variable usage
        if (!functionParameter)
            return true;
    } else if (alloc != NO_ALLOC && Token::Match(vartok, "%var% [")) {
        const Token *parent = vartok->next()->astParent();
        while (Token::Match(parent, "[|."))
            parent = parent->astParent();
        if (Token::simpleMatch(parent, "&") && !parent->astOperand2())
            return false;
        if (parent && Token::Match(parent->previous(), "if|while|switch ("))
            return true;
        if (Token::Match(parent, "[=,(]"))
            return false;
        return true;
    }

    if (_tokenizer->isCPP() && Token::Match(vartok->next(), "<<|>>")) {
        // Is this calculation done in rhs?
        const Token *tok = vartok;
        while (Token::Match(tok, "%name%|.|::"))
            tok = tok->previous();
        if (Token::Match(tok, "[;{}]"))
            return false;

        // Is variable a known POD type then this is a variable usage,
        // otherwise we assume it's not.
        const Variable *var = vartok->variable();
        return (var && (var->typeStartToken()->isStandardType() || var->typeStartToken()->isEnumType()));
    }

    if (alloc == NO_ALLOC && vartok->next() && vartok->next()->isOp() && !vartok->next()->isAssignmentOp())
        return true;

    if (vartok->strAt(1) == "]")
        return true;

    return false;
}

/***
 * Is function parameter "used" so a "usage of uninitialized variable" can
 * be written? If parameter is passed "by value" then it is "used". If it
 * is passed "by reference" then it is not necessarily "used".
 * @return  -1 => unknown   0 => not used   1 => used
 */
int CheckUninitVar::isFunctionParUsage(const Token *vartok, bool pointer, Alloc alloc) const
{
    if (!Token::Match(vartok->previous(), "[(,]") && !Token::Match(vartok->tokAt(-2), "[(,] &"))
        return -1;

    // locate start parentheses in function call..
    unsigned int argumentNumber = 0;
    const Token *start = vartok;
    while (start && !Token::Match(start, "[;{}(]")) {
        if (start->str() == ")")
            start = start->link();
        else if (start->str() == ",")
            ++argumentNumber;
        start = start->previous();
    }

    // is this a function call?
    if (start && Token::Match(start->previous(), "%name% (")) {
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
                if (Token::Match(argStart, "const %type% & %name% [,)]"))
                    return 1;
                if ((pointer || address) && alloc == NO_ALLOC && Token::Match(argStart, "const struct| %type% * %name% [,)]"))
                    return 1;
                if ((pointer || address) && Token::Match(argStart, "const %type% %name% [") && Token::Match(argStart->linkAt(3), "] [,)]"))
                    return 1;
            }

        } else if (Token::Match(start->previous(), "if|while|for")) {
            // control-flow statement reading the variable "by value"
            return alloc == NO_ALLOC;
        } else {
            const bool isnullbad = _settings->library.isnullargbad(start->previous(), argumentNumber + 1);
            if (pointer && !address && isnullbad && alloc == NO_ALLOC)
                return 1;
            const bool isuninitbad = _settings->library.isuninitargbad(start->previous(), argumentNumber + 1);
            if (alloc != NO_ALLOC)
                return isnullbad && isuninitbad;
            return isuninitbad && (!address || isnullbad);
        }
    }

    // unknown
    return -1;
}

bool CheckUninitVar::isMemberVariableAssignment(const Token *tok, const std::string &membervar) const
{
    if (Token::Match(tok, "%name% . %name%") && tok->strAt(2) == membervar) {
        if (Token::Match(tok->tokAt(3), "[=.[]"))
            return true;
        else if (Token::Match(tok->tokAt(-2), "[(,=] &"))
            return true;
        else if (Token::Match(tok->tokAt(-2), "%name% >>") && Token::Match(tok->tokAt(3), ";|>>")) // #6680
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
            unsigned int argumentNumber = 0;
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
        int use = isFunctionParUsage(tok, isPointer, alloc);
        if (use == 1)
            return true;
    }

    if (isMemberVariableAssignment(tok, membervar))
        return false;

    if (Token::Match(tok, "%name% . %name%") && tok->strAt(2) == membervar && !(tok->tokAt(-2)->variable() && tok->tokAt(-2)->variable()->isReference()))
        return true;
    else if (!isPointer && Token::Match(tok->previous(), "[(,] %name% [,)]") && isVariableUsage(tok, isPointer, alloc))
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

    else if (_settings->experimental &&
             !isPointer &&
             Token::Match(tok->tokAt(-2), "[(,] & %name% [,)]") &&
             isVariableUsage(tok, isPointer, alloc))
        return true;

    return false;
}

void CheckUninitVar::uninitstringError(const Token *tok, const std::string &varname, bool strncpy_)
{
    reportError(tok, Severity::error, "uninitstring", "Dangerous usage of '" + varname + "'" + (strncpy_ ? " (strncpy doesn't always null-terminate it)." : " (not null-terminated)."), CWE676, false);
}

void CheckUninitVar::uninitdataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitdata", "Memory is allocated but not initialized: " + varname, CWE908, false);
}

void CheckUninitVar::uninitvarError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitvar", "Uninitialized variable: " + varname, CWE908, false);
}

void CheckUninitVar::uninitStructMemberError(const Token *tok, const std::string &membername)
{
    reportError(tok,
                Severity::error,
                "uninitStructMember",
                "Uninitialized struct member: " + membername, CWE908, false);
}

void CheckUninitVar::deadPointer()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    std::list<Scope>::const_iterator scope;

    // check every executable scope
    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (!scope->isExecutable())
            continue;
        // Dead pointers..
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->variable() &&
                tok->variable()->isPointer() &&
                isVariableUsage(tok, true, NO_ALLOC)) {
                const Token *alias = tok->getValueTokenDeadPointer();
                if (alias) {
                    deadPointerError(tok,alias);
                }
            }
        }
    }
}

void CheckUninitVar::deadPointerError(const Token *pointer, const Token *alias)
{
    const std::string strpointer(pointer ? pointer->str() : std::string("pointer"));
    const std::string stralias(alias ? alias->expressionString() : std::string("&x"));

    reportError(pointer,
                Severity::error,
                "deadpointer",
                "Dead pointer usage. Pointer '" + strpointer + "' is dead if it has been assigned '" + stralias + "' at line " + MathLib::toString(alias ? alias->linenr() : 0U) + ".", CWE825, false);
}
