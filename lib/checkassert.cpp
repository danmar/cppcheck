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
// You should not write statements with side effects in assert()
//---------------------------------------------------------------------------

#include "checkassert.h"

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

    logChecker("CheckAssert::assertWithSideEffects"); // warning

    for (const Token* tok = mTokenizer->list.front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "assert ("))
            continue;

        const Token *endTok = tok->next()->link();
        for (const Token* tmp = tok->next(); tmp != endTok; tmp = tmp->next()) {
            if (Token::simpleMatch(tmp, "sizeof ("))
                tmp = tmp->linkAt(1);

            checkVariableAssignment(tmp, tok->scope());

            if (tmp->tokType() != Token::eFunction)
                continue;

            const Function* f = tmp->function();
            const Scope* scope = f->functionScope;
            if (!scope) {
                // guess that const method doesn't have side effects
                if (f->nestedIn->isClassOrStruct() && !f->isConst() && !f->isStatic())
                    sideEffectInAssertError(tmp, f->name()); // Non-const member function called, assume it has side effects
                continue;
            }

            for (const Token *tok2 = scope->bodyStart; tok2 != scope->bodyEnd; tok2 = tok2->next()) {
                if (!tok2->isAssignmentOp() && tok2->tokType() != Token::eIncDecOp)
                    continue;

                const Variable* var = tok2->previous()->variable();
                if (!var || var->isLocal() || (var->isArgument() && !var->isReference() && !var->isPointer()))
                    continue; // See ticket #4937. Assigning function arguments not passed by reference is ok.
                if (var->isArgument() && var->isPointer() && tok2->strAt(-2) != "*")
                    continue; // Pointers need to be dereferenced, otherwise there is no error

                bool noReturnInScope = true;
                for (const Token *rt = scope->bodyStart; rt != scope->bodyEnd; rt = rt->next()) {
                    if (rt->str() != "return") continue; // find all return statements
                    if (inSameScope(rt, tok2)) {
                        noReturnInScope = false;
                        break;
                    }
                }
                if (noReturnInScope) continue;

                sideEffectInAssertError(tmp, f->name());
                break;
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

// checks if side effects happen on the variable prior to tmp
void CheckAssert::checkVariableAssignment(const Token* assignTok, const Scope *assertionScope)
{
    if (!assignTok->isAssignmentOp() && assignTok->tokType() != Token::eIncDecOp)
        return;

    const Variable* var = assignTok->astOperand1()->variable();
    if (!var)
        return;

    // Variable declared in inner scope in assert => don't warn
    if (assertionScope != var->scope()) {
        const Scope *s = var->scope();
        while (s && s != assertionScope)
            s = s->nestedIn;
        if (s == assertionScope)
            return;
    }

    // assignment
    if (assignTok->isAssignmentOp() || assignTok->tokType() == Token::eIncDecOp) {
        if (var->isConst()) {
            return;
        }
        assignmentInAssertError(assignTok, var->name());
    }
    // TODO: function calls on var
}

bool CheckAssert::inSameScope(const Token* returnTok, const Token* assignTok)
{
    // TODO: even if a return is in the same scope, the assignment might not affect it.
    return returnTok->scope() == assignTok->scope();
}
