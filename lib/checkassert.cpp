/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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
// You should not write statements with side effects in assert()
//---------------------------------------------------------------------------

#include "checkassert.h"

#include "astutils.h"
#include "errortypes.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

//---------------------------------------------------------------------------

// CWE ids used
static const struct CWE CWE398(398U);   // Indicator of Poor Code Quality

// Register this check class (by creating a static instance of it)
namespace {
    CheckAssert instance;
}

void CheckAssert::assertWithSideEffects() {
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    for (const Token *tok = mTokenizer->list.front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "assert ("))
            continue;

        const Token *endTok = tok->next()->link();
        for (const Token *tmp = tok->next(); tmp != endTok; tmp = tmp->next()) {
            if (Token::simpleMatch(tmp, "sizeof ("))
                tmp = tmp->linkAt(1);

            if (isVariableAssignment(tmp)) {
                const auto *var = tmp->astOperand1()->variable();
                if (checkVariableAssignment(var, tok->scope())) {
                    assignmentInAssertError(tmp, var->name());
                }
            }

            if (tmp->tokType() != Token::eFunction)   // TODO: constructor calls with initializer list are not detected!
                continue;
            const Function *func = tmp->function();
            auto args = getArguments(tmp);
            if (isFunctionWithSideEffect(func, {&args, tok->scope()})) {
                sideEffectInAssertError(tmp, func->name());
            }
        }
        tok = endTok;
    }
}

bool CheckAssert::isFunctionWithSideEffect(const Function *function, argumentCheck argsChecking) const {
    if (function->isConstexpr()) return false;
    if (function->nestedIn->isClassOrStruct() && !function->isConstructor()) {
        if (!(function->isConst() || function->isStatic())) {
            return true;
        }
    }
    const Scope *scope = function->functionScope;
    if (!scope) return false;

    if (function->isConstructor()) {
        const auto *initializationList =
                function->constructorMemberInitialization();
        if (initializationList) {
            for (const Token *tok = initializationList; tok != scope->bodyStart; tok = tok->next()) {
                if (tok->tokType() == Token::eIncDecOp) {
                    const Variable *var = tok->previous()->variable();
                    if (!var)
                        continue;
                    if (var->isArgument() && argsChecking.arguments) {
                        return checkArgument(tok, function, var, argsChecking);
                    }
                }
            }
        }
    }

    for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
        if (tok->tokType() == Token::eFunction) {
            // previously function calls on a second level from assert aren't checked at all
            // now check at least if static or global variables are changed
            if (isFunctionWithSideEffect(tok->function()))
                return true;
            continue;
        }
        if (!tok->isAssignmentOp() && tok->tokType() != Token::eIncDecOp)
            continue;

        const Variable *var = tok->previous()->variable();
        if (!var)
            continue;

        if (var->isLocal())
            continue;
        if (var->isStatic() || var->isGlobal())
            return true;

        if (var->isArgument() && argsChecking.arguments) {
            return checkArgument(tok, function, var, argsChecking);
        }
    }
    return false;
}

const Variable *CheckAssert::findPassedVariable(const Function *function, const Variable *var, const argumentCheck &argsChecking) {
    int paramCtr = 0;
    const Variable *arg;
    const Variable *variable = nullptr;
    do {
        arg = function->getArgumentVar(paramCtr);
        if (arg == var) {
            variable = argsChecking.arguments->at(paramCtr)->variable();
            break;
        }
    } while (arg);
    return variable;
}

bool CheckAssert::checkArgument(const Token *assignIncToken, const Function *function, const Variable *var, const argumentCheck &argsChecking) {
    const Variable *variable = findPassedVariable(function, var, argsChecking);
    if (!variable)
        variable = var;
    // See ticket #4937. Assigning function arguments not passed by reference is ok.
    if (var->isReference())
        return checkVariableAssignment(variable, argsChecking.assertionScope);
    if (var->isPointer() && assignIncToken->strAt(-2) == "*")                    // TODO: what if assigned first and then dereferenced?
        return checkVariableAssignment(variable, argsChecking.assertionScope);   // Pointers need to be dereferenced, otherwise there is no error
}
//---------------------------------------------------------------------------


void CheckAssert::sideEffectInAssertError(const Token *tok, const std::string &functionName) {
    reportError(tok, Severity::warning,
                "assertWithSideEffect",
                "$symbol:" + functionName + "\n"
                                            "Assert statement calls a function which may have desired side effects: '$symbol'.\n"
                                            "Non-pure function: '$symbol' is called inside assert statement. "
                                            "Assert statements are removed from release builds so the code inside "
                                            "assert statement is not executed. If the code is needed also in release "
                                            "builds, this is a bug.",
                CWE398, Certainty::normal);
}

void CheckAssert::assignmentInAssertError(const Token *tok, const std::string &varname) {
    reportError(tok, Severity::warning,
                "assignmentInAssert",
                "$symbol:" + varname + "\n"
                                       "Assert statement modifies '$symbol'.\n"
                                       "Variable '$symbol' is modified inside assert statement. "
                                       "Assert statements are removed from release builds so the code inside "
                                       "assert statement is not executed. If the code is needed also in release "
                                       "builds, this is a bug.",
                CWE398, Certainty::normal);
}

bool CheckAssert::isVariableAssignment(const Token *token) {
    if (token->isAssignmentOp())
        return true;
    if (token->tokType() == Token::eIncDecOp)
        return true;

    return false;
}

// checks if side effects happen on the variable prior to tmp
bool CheckAssert::checkVariableAssignment(const Variable *var, const Scope *assertionScope) {
    if (!var)
        return false;

    // Variable declared in inner scope in assert => don't warn
    const auto *variableScope = var->scope();
    if (assertionScope != variableScope) {
        const Scope *s = variableScope;
        while (s && s != assertionScope)
            s = s->nestedIn;
        if (s == assertionScope)
            return false;
    }

    // assignment
    if (var->isConst()) {
        return false;
    }

    // TODO: function calls on var
    return true;
}
