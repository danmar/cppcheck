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

            if (isVariableAssignment(tmp)) {
                const auto *var = tmp->astOperand1()->variable();
                if (checkVariableAssignmentSideEffect(var, tok->scope())) {
                    assignmentInAssertError(tmp, var->name());
                }
            }

            if (tmp->tokType() != Token::eFunction)   // TODO: constructor calls with initializer list are not detected!
                continue;
            const Function *func = tmp->function();
            const auto args = getArguments(tmp);
            if (isFunctionWithSideEffect(func, {&args, tok->scope()})) {
                sideEffectInAssertError(tmp, func->name());
            }
        }
        tok = endTok;
    }
}

//---------------------------------------------------------------------------


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

//---------------------------------------------------------------------------

/**
 * \brief checks if a variable is assigned
 * \retval true   - is variable assignment
 * \retval false  - no variable assignment
 */
bool CheckAssert::isVariableAssignment(const Token *token) {
    if (token->isAssignmentOp())
        return true;
    if (token->tokType() == Token::eIncDecOp)
        return true;

    return false;
}

/**
 * \brief checks if assignment of variable has side effect
 * \param var             - variable a value is assigned to
 * \param assertionScope  - scope of the assertion
 * \retval true           - the assignment has a possible side effect
 * \retval false          - no side effect from assignment
 */
bool CheckAssert::checkVariableAssignmentSideEffect(const Variable *var, const Scope *assertionScope) {
    if (!var)
        return false;

    // Variable declared in inner scope in assert => don't warn
    const auto *variableScope = var->scope();
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
 * \brief checks if a function call may has side effects
 * \param function      - function to check
 * \param argsChecking  - function arguments
 * \retval true         - function has side effects
 * \retval false        - not sure if function has side effects
 */
bool CheckAssert::isFunctionWithSideEffect(const Function *function, ArgumentCheck argsChecking) {
    // constexpr never has any side effect
    if (function->isConstexpr())
        return false;

    // is normal class function
    if (function->nestedIn->isClassOrStruct() && !function->isConstructor() && !function->isConst() && !function->isStatic())
        return true;

    const Scope *scope = function->functionScope;
    if (!scope) return false;

    if (function->isConstructor()) {
        const auto *initializationList =
                function->constructorMemberInitialization();
        if (initializationList) {
            for (const Token *tok = initializationList; tok != scope->bodyStart; tok = tok->next()) {
                if (tok->tokType() == Token::eIncDecOp) {
                    const Variable* var = getLhsVariable(tok);
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

        const Variable* var = getLhsVariable(tok);
        if (!var)
            continue;

        if (var->isStatic() || var->isGlobal())
            return true;

        if (var->isLocal())
            continue;

        if (var->isArgument() && argsChecking.arguments) {
            return checkArgument(tok, function, var, argsChecking);
        }
    }
    return false;
}

/**
 * \brief returns the lhs variable of the modification
 * \param modifyOperator  - the modify operator
 * \retval Variable - variable if found
 * \retval nullptr  - if no variable found
 */
const Variable *CheckAssert::getLhsVariable(const Token *modifyOperator) {
    const Token *lhs = modifyOperator->astOperand1();
    while (lhs != modifyOperator && lhs->variable() == nullptr) {
        // de-referenced or address of -> this seems to be something with a side effect
        // let checkArgument decide
        if (lhs->str() == "*" || lhs->str() == "&")
            lhs = lhs->next();
    }
    return lhs->variable();
}


/**
 * \brief check if the function is modifying a passed argument
 * \param modifyOperator  - operator which leads to variable modification
 * \param function        - function currently beeing processed
 * \param parameter       - the argument to check
 * \param argsChecking    - all arguments passed to the function
 * \retval true           - argument is being modified
 * \retval false          - argument not modified
 */
bool CheckAssert::checkArgument(const Token *modifyOperator, const Function *function, const Variable *parameter, const ArgumentCheck &argsChecking) {
    const Variable *variable = findPassedVariable(function, parameter, argsChecking);
    if (!variable)
        variable = parameter;
    // See ticket #4937. Assigning function arguments not passed by reference is ok.
    if (parameter->isReference())
        return checkVariableAssignmentSideEffect(variable, argsChecking.assertionScope);
    // Pointers need to be de-referenced, otherwise there is no error
    // TODO: what if assigned first and then de-referenced?
    if (parameter->isPointer() &&
        ((modifyOperator->astOperand1() && modifyOperator->astOperand1()->str() == "*") || // needed for modifyOperator type eAssignmentOp
         (modifyOperator->astParent() && modifyOperator->astParent()->str() == "*"))) // needed for modifyOperator type eIncDecOp
        return checkVariableAssignmentSideEffect(variable, argsChecking.assertionScope);
    return false;
}

/**
 * \brief searches for the correct argument/variable passed on function call for the given parameter
 * \param function      - function which arguments should be searched for the variable
 * \param parameter     - variable which may is one of the function's parameter
 * \param argsChecking  - arguments passed on function call
 * \return found variable passed to the function
 */
const Variable *CheckAssert::findPassedVariable(const Function *function, const Variable *parameter, const ArgumentCheck &argsChecking) {
    const Variable *variable = nullptr;
    for( int paramCtr = 0; paramCtr < argsChecking.arguments->size(); paramCtr++) {
        const Variable *arg = function->getArgumentVar(paramCtr);
        if (arg == parameter) {
            variable = argsChecking.arguments->at(paramCtr)->variable();
            break;
        }
    }
    return variable;
}
