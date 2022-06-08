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

    struct ArgumentCheck {
        const std::vector<const Token *> *arguments;
        const Scope *assertionScope;
    };

}


/**
 * \brief checks if token is able to dereference a pointer/variable
 * \param token the token to analyze
 * \retval true token is dereferencing a variable
 * \retval false not a valid token
 */
static bool isDereferencingToken(const Token *token){
    if (!token)
        return false;

    const std::string dereferencingToken = token->str();
    return dereferencingToken == "*" || dereferencingToken == "[";
}

/**
 * \brief returns the lhs variable of the modification
 * \param modifyOperator the modify operator
 * \return variable if found else nullptr
 */
static const Variable *getLhsVariable(const Token *modifyOperator) {
    const std::pair<const Token *, const Token *> tokens = modifyOperator->findExpressionStartEndTokens();
    const Token* lhs = tokens.first;
    while (lhs != tokens.second && lhs->variable() == nullptr) {
        lhs = lhs->next();
    }
    return lhs->variable();
}


/**
 * \brief searches for the correct argument/variable passed on function call for the given parameter
 * \param function function which arguments should be searched for the variable
 * \param parameter variable which may is one of the function's parameter
 * \param argsChecking arguments passed on function call
 * \return found variable passed to the function
 */
static const Variable *findPassedVariable(const Function *function, const Variable *parameter, const ArgumentCheck &argsChecking) {
    const Variable *variable = nullptr;
    for (int paramCtr = 0; paramCtr < argsChecking.arguments->size(); paramCtr++) {
        const Variable *arg = function->getArgumentVar(paramCtr);
        if (arg == parameter) {
            variable = argsChecking.arguments->at(paramCtr)->variable();
            break;
        }
    }
    return variable;
}


/**
 * \brief checks if a variable's value may be changed with the given operator
 * \retval true variable's value may change
 * \retval false no possible value altering
 */
static bool isVariableValueChangingOperator(const Token *token) {
    if (token->isAssignmentOp())
        return true;
    if (token->isIncDecOp())
        return true;

    return false;
}


/**
 * \brief checks if assignment of variable has side effect
 * \param var variable a value is assigned to
 * \param assertionScope scope of the assertion
 * \retval true the assignment has a possible side effect
 * \retval false no side effect from assignment
 */
static bool isVariableAssignmentWithSideEffect(const Variable *var, const Scope *assertionScope) {
    if (!var)
        return false;

    // Variable declared in inner scope in assert => don't warn
    const Scope *variableScope = var->scope();
    if (assertionScope != variableScope) {
        const Scope *scope = variableScope;
        while (scope && scope != assertionScope)
            scope = scope->nestedIn;
        if (scope == assertionScope)
            return false;
    }

    // assignment
    if (var->isConst()) {
        return false;
    }

    // TODO: function calls on var
    return true;
}


/**
 * \brief check if the function is modifying a passed argument
 * \param modifyOperator operator which leads to variable modification
 * \param function function currently beeing processed
 * \param parameter the argument to check
 * \param argsChecking all arguments passed to the function
 * \retval true argument is being modified
 * \retval false argument not modified
 */
static bool isArgumentModified(const Token *modifyOperator, const Function *function, const Variable *parameter, const ArgumentCheck &argsChecking) {
    const Variable *variable = findPassedVariable(function, parameter, argsChecking);
    if (!variable)
        variable = parameter;
    // See ticket #4937. Assigning function arguments not passed by reference is ok.
    if (parameter->isReference())
        return isVariableAssignmentWithSideEffect(variable, argsChecking.assertionScope);
    // Pointers need to be de-referenced, otherwise there is no error
    // TODO: what if assigned first and then de-referenced?
    if (parameter->isPointer()) {
        if (isDereferencingToken(modifyOperator->astOperand1()))
            return isVariableAssignmentWithSideEffect(variable, argsChecking.assertionScope);
        if (isDereferencingToken(modifyOperator->astParent()))
            return isVariableAssignmentWithSideEffect(variable, argsChecking.assertionScope);
    }
    return false;
}


/**
 * \brief checks if a function call may has side effects
 * \param function function to check
 * \param argsChecking function arguments
 * \retval true function has side effects
 * \retval false not sure if function has side effects
 */
static bool isFunctionCallWithSideEffect(const Function *function, ArgumentCheck argsChecking = {}) {
    // constexpr never has any side effect
    if (function->isConstexpr())
        return false;

    // is normal class function
    if (function->nestedIn->isClassOrStruct() && !function->isConstructor() && !function->isConst() && !function->isStatic())
        return true;

    const Scope *scope = function->functionScope;
    if (!scope) return false;

    if (function->isConstructor()) {
        const Token *initializationList = function->constructorMemberInitialization();
        if (initializationList) {
            for (const Token *tok = initializationList; tok != scope->bodyStart; tok = tok->next()) {
                if (tok->tokType() == Token::eIncDecOp) {
                    const Variable* var = getLhsVariable(tok);
                    if (!var)
                        continue;
                    if (var->isArgument() && argsChecking.arguments) {
                        return isArgumentModified(tok, function, var, argsChecking);
                    }
                }
            }
        }
    }

    for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
        if (tok->tokType() == Token::eFunction) {
            // previously function calls on a second level from assert aren't checked at all
            // now check at least if static or global variables are changed
            if (isFunctionCallWithSideEffect(tok->function()))
                return true;
            continue;
        }
        if (!tok->isAssignmentOp() && tok->tokType() != Token::eIncDecOp)
            continue;

        const Variable* var = getLhsVariable(tok);
        if (!var)
            continue;

        if (var->isStatic() || var->isGlobal())
            return true;

        if (var->isLocal())
            continue;

        if (var->isArgument() && argsChecking.arguments) {
            return isArgumentModified(tok, function, var, argsChecking);
        }
    }
    return false;
}


void CheckAssert::assertWithSideEffects()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    for (const Token* tok = mTokenizer->list.front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "assert ("))
            continue;

        const Token *endTok = tok->next()->link();
        for (const Token* tmp = tok->next(); tmp != endTok; tmp = tmp->next()) {
            if (Token::simpleMatch(tmp, "sizeof ("))
                tmp = tmp->linkAt(1);

            if (isVariableValueChangingOperator(tmp)) {
                const Variable *var = getLhsVariable(tmp);
                if (isVariableAssignmentWithSideEffect(var, tok->scope())) {
                    assignmentInAssertError(tmp, var->name());
                }
            }

            if (tmp->tokType() != Token::eFunction)   // TODO: constructor calls with initializer list are not detected!
                continue;
            const Function *func = tmp->function();
            const std::vector<const Token *> args = getArguments(tmp);
            if (isFunctionCallWithSideEffect(func, {&args, tok->scope()})) {
                sideEffectInAssertError(tmp, func->name());
            }
        }
        tok = endTok;
    }
}


void CheckAssert::sideEffectInAssertError(const Token *tok, const std::string& functionName)
{
    reportError(tok, Severity::warning,
                "assertWithSideEffect",
                "$symbol:" + functionName + "\n"
                "Assert statement calls a function which may have desired side effects: '$symbol'.\n"
                "Non-pure function: '$symbol' is called inside assert statement. "
                "Assert statements are removed from release builds so the code inside "
                "assert statement is not executed. If the code is needed also in release "
                "builds, this is a bug.", CWE398, Certainty::normal);
}


void CheckAssert::assignmentInAssertError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::warning,
                "assignmentInAssert",
                "$symbol:" + varname + "\n"
                "Assert statement modifies '$symbol'.\n"
                "Variable '$symbol' is modified inside assert statement. "
                "Assert statements are removed from release builds so the code inside "
                "assert statement is not executed. If the code is needed also in release "
                "builds, this is a bug.", CWE398, Certainty::normal);
}
