/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckAssert instance;
}

void CheckAssert::assertWithSideEffects()
{
    if (!_settings->isEnabled("warning"))
        return;

    const Token *tok = findAssertPattern(_tokenizer->tokens());
    const Token *endTok = tok ? tok->next()->link() : NULL;

    while (tok && endTok) {
        for (const Token* tmp = tok->tokAt(1); tmp != endTok; tmp = tmp->next()) {
            checkVariableAssignment(tmp, true);

            if (tmp->isName() && tmp->type() == Token::eFunction) {
                const Function* f = tmp->function();
                if (f->argCount() == 0 && f->isConst) continue;

                // functions with non-const references
                else if (f->argCount() != 0) {
                    for (std::list<Variable>::const_iterator it = f->argumentList.begin(); it != f->argumentList.end(); ++it) {
                        if (it->isConst() || it->isLocal()) continue;
                        else if (it->isReference()) {
                            const Token* next = it->nameToken()->next();
                            bool isAssigned = checkVariableAssignment(next, false);
                            if (isAssigned)
                                sideEffectInAssertError(tmp, f->name());
                            continue;
                        }
                    }
                }

                // variables in function scope
                const Scope* scope = f->functionScope;
                if (!scope) continue;

                for (const Token *tok2 = scope->classStart; tok2 != scope->classEnd; tok2 = tok2->next()) {
                    if (tok2->type() != Token::eAssignmentOp && tok2->type() != Token::eIncDecOp) continue;
                    const Variable* var = tok2->previous()->variable();
                    if (!var || var->isLocal()) continue;
                    if (var->isArgument() &&
                        (var->isConst() || (!var->isReference() && !var->isPointer())))
                        // see ticket #4937. Assigning function arguments not passed by reference is ok.
                        continue;
                    std::vector<const Token*> returnTokens; // find all return statements
                    for (const Token *rt = scope->classStart; rt != scope->classEnd; rt = rt->next()) {
                        if (!Token::Match(rt, "return %any%")) continue;
                        returnTokens.push_back(rt);
                    }

                    bool noReturnInScope = true;
                    for (std::vector<const Token*>::iterator rt = returnTokens.begin(); rt != returnTokens.end(); ++rt) {
                        noReturnInScope &= !inSameScope(*rt, tok2);
                    }
                    if (noReturnInScope) continue;
                    bool isAssigned = checkVariableAssignment(tok2, false);
                    if (isAssigned)
                        sideEffectInAssertError(tmp, f->name());
                }
            }
        }

        tok = findAssertPattern(endTok->next());
        endTok = tok ? tok->next()->link() : NULL;
    }
}
//---------------------------------------------------------------------------


void CheckAssert::sideEffectInAssertError(const Token *tok, const std::string& functionName)
{
    reportError(tok, Severity::warning,
                "assertWithSideEffect", "Assert statement calls a function which may have desired side effects: '" + functionName + "'.\n"
                "Non-pure function: '" + functionName + "' is called inside assert statement. "
                "Assert statements are removed from release builds so the code inside "
                "assert statement is not executed. If the code is needed also in release "
                "builds, this is a bug.");
}

void CheckAssert::assignmentInAssertError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::warning,
                "assignmentInAssert", "Assert statement modifies '" + varname + "'.\n"
                "Variable '" + varname + "' is modified insert assert statement. "
                "Assert statements are removed from release builds so the code inside "
                "assert statement is not executed. If the code is needed also in release "
                "builds, this is a bug.");
}

// checks if side effects happen on the variable prior to tmp
bool CheckAssert::checkVariableAssignment(const Token* assignTok, bool reportErr /*= true*/)
{
    const Variable* v = assignTok->previous()->variable();
    if (!v) return false;

    // assignment
    if (assignTok->isAssignmentOp() || assignTok->type() == Token::eIncDecOp) {

        if (v->isConst()) return false;

        if (reportErr) // report as variable assignment error
            assignmentInAssertError(assignTok, v->name());

        return true;
    }
    return false;
    // TODO: function calls on v
}

const Token* CheckAssert::findAssertPattern(const Token* start)
{
    return Token::findmatch(start, "assert ( %any%");
}

bool CheckAssert::inSameScope(const Token* returnTok, const Token* assignTok)
{
    // TODO: even if a return is in the same scope, the assignment might not affect it.
    return returnTok->scope() == assignTok->scope();
}
