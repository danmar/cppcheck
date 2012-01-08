/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include "symboldatabase.h"

#include <list>
#include <string>

//---------------------------------------------------------------------------


// Register this check class into cppcheck by creating a static instance of it..
namespace {
    static CheckAutoVariables instance;
}


bool CheckAutoVariables::errorAv(const Token* left, const Token* right)
{
    const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(left->varId());

    if (!var || !var->isArgument() ||
        (!var->isArray() && !Token::Match(var->nameToken()->tokAt(-3), "%type% * *")) ||
        (var->isArray() && !Token::Match(var->nameToken()->tokAt(-2), "%type% *")))
        return false;

    return isAutoVar(right->varId());
}

bool CheckAutoVariables::isAutoVar(unsigned int varId)
{
    const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(varId);

    if (!var || !var->isLocal() || var->isStatic() || var->isArray() || var->isPointer())
        return false;

    if (Token::simpleMatch(var->nameToken()->previous(), "&")) {
        // address of reference variable can be taken if the address
        // of the variable it points at is not a auto-var
        // TODO: check what the reference variable references.
        return false;
    }

    return true;
}

bool CheckAutoVariables::isAutoVarArray(unsigned int varId)
{
    const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(varId);

    if (!var || !var->isLocal() || var->isStatic() || !var->isArray())
        return false;

    return true;
}

void CheckAutoVariables::autoVariables()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            //Critical assignment
            if (Token::Match(tok, "[;{}] * %var% = & %var%") && errorAv(tok->tokAt(2), tok->tokAt(5))) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->tokAt(5)->varId());
                if (var && (!var->isClass() || var->type()))
                    errorAutoVariableAssignment(tok->next(), false);
            } else if (Token::Match(tok, "[;{}] %var% . %var% = & %var%")) {
                // TODO: check if the parameter is only changed temporarily (#2969)
                if (_settings->inconclusive) {
                    const Variable * var1 = symbolDatabase->getVariableFromVarId(tok->next()->varId());
                    if (var1 && var1->isArgument() && Token::Match(var1->nameToken()->tokAt(-2), "%type% *")) {
                        const Variable * var2 = symbolDatabase->getVariableFromVarId(tok->tokAt(6)->varId());
                        if (var2 && var2->isLocal() && !var2->isStatic() && !Token::simpleMatch(var2->typeEndToken(), "*"))
                            errorAutoVariableAssignment(tok->next(), _settings->inconclusive);
                    }
                }
                tok = tok->tokAt(6);
            } else if (Token::Match(tok, "[;{}] %var% . %var% = %var% ;")) {
                // TODO: check if the parameter is only changed temporarily (#2969)
                if (_settings->inconclusive) {
                    const Variable * var1 = symbolDatabase->getVariableFromVarId(tok->next()->varId());
                    if (var1 && var1->isArgument() && Token::Match(var1->nameToken()->tokAt(-2), "%type% *")) {
                        const Variable * var2 = symbolDatabase->getVariableFromVarId(tok->tokAt(5)->varId());
                        if (var2 && var2->isLocal() && var2->isArray() && !var2->isStatic())
                            errorAutoVariableAssignment(tok->next(), _settings->inconclusive);
                    }
                }
                tok = tok->tokAt(5);
            } else if (Token::Match(tok, "[;{}] * %var% = %var% ;")) {
                const Variable * var1 = symbolDatabase->getVariableFromVarId(tok->tokAt(2)->varId());
                if (var1 && var1->isArgument() && Token::Match(var1->nameToken()->tokAt(-3), "%type% * *")) {
                    const Variable * var2 = symbolDatabase->getVariableFromVarId(tok->tokAt(4)->varId());
                    if (var2 && var2->isLocal() && var2->isArray() && !var2->isStatic())
                        errorAutoVariableAssignment(tok->next(), false);
                }
                tok = tok->tokAt(4);
            } else if (Token::Match(tok, "[;{}] %var% [ %any% ] = & %var%") && errorAv(tok->next(), tok->tokAt(7))) {
                errorAutoVariableAssignment(tok->next(), false);
            }
            // Critical return
            else if (Token::Match(tok, "return & %var% ;") && isAutoVar(tok->tokAt(2)->varId())) {
                errorReturnAddressToAutoVariable(tok);
            } else if (Token::Match(tok, "return & %var% [") &&
                       Token::simpleMatch(tok->linkAt(3), "] ;") &&
                       isAutoVarArray(tok->tokAt(2)->varId())) {
                errorReturnAddressToAutoVariable(tok);
            } else if (Token::Match(tok, "return & %var% ;") && tok->tokAt(2)->varId()) {
                const Variable * var1 = symbolDatabase->getVariableFromVarId(tok->tokAt(2)->varId());
                if (var1 && var1->isArgument() && var1->typeEndToken()->str() != "&")
                    errorReturnAddressOfFunctionParameter(tok, tok->strAt(2));
            }
            // Invalid pointer deallocation
            else if (Token::Match(tok, "free ( %var% ) ;") && isAutoVarArray(tok->tokAt(2)->varId())) {
                errorInvalidDeallocation(tok);
            }
        }
    }
}

//---------------------------------------------------------------------------

void CheckAutoVariables::returnPointerToLocalArray()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        const Token *tok = scope->classDef;

        // skip any qualification
        while (Token::Match(tok->tokAt(-2), "%type% ::"))
            tok = tok->tokAt(-2);

        // have we reached a function that returns a pointer
        if (tok->previous() && tok->previous()->str() == "*") {
            for (const Token *tok2 = scope->classStart; tok2 && tok2 != scope->classEnd; tok2 = tok2->next()) {
                // Return pointer to local array variable..
                if (Token::Match(tok2, "return %var% ;")) {
                    const unsigned int varid = tok2->next()->varId();
                    const Variable *var = symbolDatabase->getVariableFromVarId(varid);

                    if (var && var->isLocal() && !var->isStatic() && var->isArray()) {
                        errorReturnPointerToLocalArray(tok2);
                    }
                }
            }
        }
    }
}

void CheckAutoVariables::errorReturnAddressToAutoVariable(const Token *tok)
{
    reportError(tok, Severity::error, "returnAddressOfAutoVariable", "Return of the address of an auto-variable");
}

void CheckAutoVariables::errorReturnPointerToLocalArray(const Token *tok)
{
    reportError(tok, Severity::error, "returnLocalVariable", "Returning pointer to local array variable");
}

void CheckAutoVariables::errorAutoVariableAssignment(const Token *tok, bool inconclusive)
{
    if (!inconclusive) {
        reportError(tok, Severity::error, "autoVariables",
                    "Assigning address of local auto-variable to a function parameter.\n"
                    "Dangerous assignment - function parameter takes the address of a local "
                    "auto-variable. Local auto-variables are reserved from the stack. And the "
                    "stack is freed when the function ends. So the pointer to a local variable "
                    "is invalid after the function ends.");
    } else {
        reportInconclusiveError(tok, Severity::error, "autoVariables",
                                "Inconclusive: Assigning address of local auto-variable to a function parameter.\n"
                                "Inconclusive: function parameter takes the address of a local auto-variable. "
                                "Local auto-variables are reserved from the stack. And the stack is freed when "
                                "the function ends. The address is invalid after the function ends and it "
                                "might 'leak' from the function through the parameter.");
    }
}

void CheckAutoVariables::errorReturnAddressOfFunctionParameter(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "returnAddressOfFunctionParameter",
                "Return the address of function parameter '" + varname + "'\n"
                "Address of the function parameter '" + varname + "' is invalid after the function exits. "
                "Function parameters are created into the stack. When the function exits the stack is deleted.");
}

//---------------------------------------------------------------------------

// return temporary?
bool CheckAutoVariables::returnTemporary(const Token *tok) const
{
    if (!Token::Match(tok, "return %var% ("))
        return false;
    return bool(NULL != Token::findmatch(_tokenizer->tokens(), ("std :: string " + tok->next()->str() + " (").c_str()));
}

//---------------------------------------------------------------------------

void CheckAutoVariables::returnReference()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        const Token *tok = scope->classDef;

        // skip any qualification
        while (Token::Match(tok->tokAt(-2), "%type% ::"))
            tok = tok->tokAt(-2);

        // have we reached a function that returns a reference?
        if (tok->previous() && tok->previous()->str() == "&") {
            for (const Token *tok2 = scope->classStart; tok2 && tok2 != scope->classEnd; tok2 = tok2->next()) {
                // return..
                if (Token::Match(tok2, "return %var% ;")) {
                    // is the returned variable a local variable?
                    const unsigned int varid1 = tok2->next()->varId();
                    const Variable *var1 = symbolDatabase->getVariableFromVarId(varid1);

                    if (var1 && var1->isLocal() && !var1->isStatic()) {
                        // If reference variable is used, check what it references
                        if (Token::Match(var1->nameToken(), "%var% =")) {
                            const Token *tok3 = var1->nameToken()->tokAt(2);
                            if (!Token::Match(tok3, "%var% [;.]"))
                                continue;

                            // Only report error if variable that is referenced is
                            // a auto variable
                            const Variable *var2 = symbolDatabase->getVariableFromVarId(tok3->varId());
                            if (!var2 || !var2->isLocal() || var2->isStatic() || (var2->isPointer() && tok3->strAt(1) == "."))
                                continue;
                        }

                        // report error..
                        errorReturnReference(tok2);
                    }
                }

                // return reference to temporary..
                else if (returnTemporary(tok2)) {
                    // report error..
                    errorReturnTempReference(tok2);
                }
            }
        }
    }
}

void CheckAutoVariables::errorReturnReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnReference", "Returning reference to auto variable");
}

void CheckAutoVariables::errorReturnTempReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnTempReference", "Returning reference to temporary");
}

void CheckAutoVariables::errorInvalidDeallocation(const Token *tok)
{
    reportError(tok,
                Severity::error,
                "autovarInvalidDeallocation",
                "Deallocating auto-variable is invalid\n"
                "Deallocating an auto-variable is invalid. You should only free memory "
                "that has been allocated dynamically.");
}


//---------------------------------------------------------------------------

// Return c_str
void CheckAutoVariables::returncstr()
{
    // locate function that returns a const char *..
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        const Token *tok = scope->classDef;

        // skip any qualification
        while (Token::Match(tok->tokAt(-2), "%type% ::"))
            tok = tok->tokAt(-2);

        // have we reached a function that returns a const char *
        if (Token::simpleMatch(tok->tokAt(-3), "const char *")) {
            for (const Token *tok2 = scope->classStart; tok2 && tok2 != scope->classEnd; tok2 = tok2->next()) {
                // return..
                if (Token::Match(tok2, "return %var% . c_str ( ) ;")) {
                    // is the returned variable a local variable?
                    const unsigned int varid = tok2->next()->varId();
                    const Variable *var = symbolDatabase->getVariableFromVarId(varid);

                    if (var && var->isLocal() && !var->isStatic()) {
                        // report error..
                        errorReturnAutocstr(tok2);
                    }
                }

                // return pointer to temporary..
                else if (returnTemporary(tok2)) {
                    // report error..
                    errorReturnTempPointer(tok2);
                }
            }
        }
    }
}

void CheckAutoVariables::errorReturnAutocstr(const Token *tok)
{
    reportError(tok, Severity::error, "returnAutocstr", "Returning pointer to auto variable");
}

void CheckAutoVariables::errorReturnTempPointer(const Token *tok)
{
    reportError(tok, Severity::error, "returnTempPointer", "Returning pointer to temporary");
}
