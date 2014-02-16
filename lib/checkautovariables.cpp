/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
#include "checkuninitvar.h"

#include <list>
#include <string>

//---------------------------------------------------------------------------


// Register this check class into cppcheck by creating a static instance of it..
namespace {
    static CheckAutoVariables instance;
}


bool CheckAutoVariables::isPtrArg(const Token *tok)
{
    const Variable *var = tok->variable();

    return (var && var->isArgument() && var->isPointer());
}

bool CheckAutoVariables::isRefPtrArg(const Token *tok)
{
    const Variable *var = tok->variable();

    return (var && var->isArgument() && var->isReference() && var->isPointer());
}

bool CheckAutoVariables::isNonReferenceArg(const Token *tok)
{
    const Variable *var = tok->variable();

    return (var && var->isArgument() && !var->isReference() && (var->isPointer() || var->typeStartToken()->isStandardType() || var->type()));
}

bool CheckAutoVariables::isAutoVar(const Token *tok)
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

    return true;
}

bool CheckAutoVariables::isAutoVarArray(const Token *tok)
{
    const Variable *var = tok->variable();

    return (var && var->isLocal() && !var->isStatic() && var->isArray());
}

// Verification that we really take the address of a local variable
static bool checkRvalueExpression(const Token * const vartok)
{
    const Variable * const var = vartok->variable();
    if (var == nullptr)
        return false;

    const Token * const next = vartok->next();

    if (Token::Match(vartok->previous(), "& %var% [") && var->isPointer())
        return false;

    // &a.b[0]
    if (Token::Match(vartok, "%var% . %var% [") && !var->isPointer()) {
        const Variable *var2 = next->next()->variable();
        return var2 && !var2->isPointer();
    }

    return ((next->str() != "." || (!var->isPointer() && (!var->isClass() || var->type()))) && next->strAt(2) != ".");
}

static bool variableIsUsedInScope(const Token* start, unsigned int varId, const Scope *scope)
{
    if (!start) // Ticket #5024
        return false;

    for (const Token *tok = start; tok != scope->classEnd; tok = tok->next()) {
        if (tok->varId() == varId)
            return true;
        if (tok->scope()->type == Scope::eFor || tok->scope()->type == Scope::eDo || tok->scope()->type == Scope::eWhile) // In case of loops, better checking would be necessary
            return true;
        if (Token::simpleMatch(tok, "asm ("))
            return true;
    }
    return false;
}

void CheckAutoVariables::assignFunctionArg()
{
    if (!_settings->isEnabled("warning"))
        return;
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "[;{}] %var% =") &&
                isNonReferenceArg(tok->next()) &&
                !variableIsUsedInScope(Token::findsimplematch(tok->tokAt(2), ";"), tok->next()->varId(), scope)) {
                errorUselessAssignmentPtrArg(tok->next());
            }
        }
    }
}

void CheckAutoVariables::autoVariables()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            // Critical assignment
            if (Token::Match(tok, "[;{}] %var% = & %var%") && isRefPtrArg(tok->next()) && isAutoVar(tok->tokAt(4))) {
                if (checkRvalueExpression(tok->tokAt(4)))
                    errorAutoVariableAssignment(tok->next(), false);
            } else if (Token::Match(tok, "[;{}] * %var% = & %var%") && isPtrArg(tok->tokAt(2)) && isAutoVar(tok->tokAt(5))) {
                if (checkRvalueExpression(tok->tokAt(5)))
                    errorAutoVariableAssignment(tok->next(), false);
            } else if (Token::Match(tok, "[;{}] %var% . %var% = & %var%")) {
                // TODO: check if the parameter is only changed temporarily (#2969)
                if (_settings->inconclusive) {
                    const Variable * var1 = tok->next()->variable();
                    if (var1 && var1->isArgument() && var1->isPointer()) {
                        const Token * const var2tok = tok->tokAt(6);
                        if (isAutoVar(var2tok) && checkRvalueExpression(var2tok))
                            errorAutoVariableAssignment(tok->next(), true);
                    }
                }
                tok = tok->tokAt(6);
            } else if (Token::Match(tok, "[;{}] %var% . %var% = %var% ;")) {
                // TODO: check if the parameter is only changed temporarily (#2969)
                if (_settings->inconclusive) {
                    const Variable * var1 = tok->next()->variable();
                    if (var1 && var1->isArgument() && var1->isPointer()) {
                        if (isAutoVarArray(tok->tokAt(5)))
                            errorAutoVariableAssignment(tok->next(), true);
                    }
                }
                tok = tok->tokAt(5);
            } else if (Token::Match(tok, "[;{}] * %var% = %var% ;")) {
                const Variable * var1 = tok->tokAt(2)->variable();
                if (var1 && var1->isArgument() && Token::Match(var1->nameToken()->tokAt(-3), "%type% * *")) {
                    if (isAutoVarArray(tok->tokAt(4)))
                        errorAutoVariableAssignment(tok->next(), false);
                }
                tok = tok->tokAt(4);
            } else if (Token::Match(tok, "[;{}] %var% [") && Token::Match(tok->linkAt(2), "] = & %var%") && isPtrArg(tok->next()) && isAutoVar(tok->linkAt(2)->tokAt(3))) {
                const Token* const varTok = tok->linkAt(2)->tokAt(3);
                if (checkRvalueExpression(varTok))
                    errorAutoVariableAssignment(tok->next(), false);
            }
            // Critical return
            else if (Token::Match(tok, "return & %var% ;") && isAutoVar(tok->tokAt(2))) {
                errorReturnAddressToAutoVariable(tok);
            } else if (Token::Match(tok, "return & %var% [") &&
                       Token::simpleMatch(tok->linkAt(3), "] ;") &&
                       isAutoVarArray(tok->tokAt(2))) {
                errorReturnAddressToAutoVariable(tok);
            } else if (Token::Match(tok, "return & %var% ;") && tok->tokAt(2)->varId()) {
                const Variable * var1 = tok->tokAt(2)->variable();
                if (var1 && var1->isArgument() && var1->typeEndToken()->str() != "&")
                    errorReturnAddressOfFunctionParameter(tok, tok->strAt(2));
            }
            // Invalid pointer deallocation
            else if (Token::Match(tok, "free ( %var% ) ;") || Token::Match(tok, "delete [| ]| (| %var% !![")) {
                tok = Token::findmatch(tok->next(), "%var%");
                if (isAutoVarArray(tok))
                    errorInvalidDeallocation(tok);
            }
        }
    }
}

//---------------------------------------------------------------------------

void CheckAutoVariables::returnPointerToLocalArray()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        if (!scope->function)
            continue;

        const Token *tok = scope->function->tokenDef;

        // have we reached a function that returns a pointer
        if (tok->previous() && tok->previous()->str() == "*") {
            for (const Token *tok2 = scope->classStart->next(); tok2 && tok2 != scope->classEnd; tok2 = tok2->next()) {
                // Return pointer to local array variable..
                if (Token::Match(tok2, "return %var% ;")) {
                    if (isAutoVarArray(tok2->next())) {
                        errorReturnPointerToLocalArray(tok2);
                    }
                }
            }
        }
    }
}

void CheckAutoVariables::errorReturnAddressToAutoVariable(const Token *tok)
{
    reportError(tok, Severity::error, "returnAddressOfAutoVariable", "Address of an auto-variable returned.");
}

void CheckAutoVariables::errorReturnPointerToLocalArray(const Token *tok)
{
    reportError(tok, Severity::error, "returnLocalVariable", "Pointer to local array variable returned.");
}

void CheckAutoVariables::errorAutoVariableAssignment(const Token *tok, bool inconclusive)
{
    if (!inconclusive) {
        reportError(tok, Severity::error, "autoVariables",
                    "Address of local auto-variable assigned to a function parameter.\n"
                    "Dangerous assignment - the function parameter is assigned the address of a local "
                    "auto-variable. Local auto-variables are reserved from the stack which "
                    "is freed when the function ends. So the pointer to a local variable "
                    "is invalid after the function ends.");
    } else {
        reportError(tok, Severity::error, "autoVariables",
                    "Address of local auto-variable assigned to a function parameter.\n"
                    "Function parameter is assigned the address of a local auto-variable. "
                    "Local auto-variables are reserved from the stack which is freed when "
                    "the function ends. The address is invalid after the function ends and it "
                    "might 'leak' from the function through the parameter.", true);
    }
}

void CheckAutoVariables::errorReturnAddressOfFunctionParameter(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "returnAddressOfFunctionParameter",
                "Address of function parameter '" + varname + "' returned.\n"
                "Address of the function parameter '" + varname + "' becomes invalid after the function exits because "
                "function parameters are stored on the stack which is freed when the function exits. Thus the returned "
                "value is invalid.");
}

void CheckAutoVariables::errorUselessAssignmentPtrArg(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "uselessAssignmentPtrArg",
                "Assignment of function parameter has no effect outside the function.");
}

//---------------------------------------------------------------------------

// return temporary?
bool CheckAutoVariables::returnTemporary(const Token *tok) const
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    bool func = false;     // Might it be a function call?
    bool retref = false;   // is there such a function that returns a reference?
    bool retvalue = false; // is there such a function that returns a value?

    const Function *function = tok->function();
    if (function) {
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
                if (symbolDatabase->isClassOrStruct(start->str()))
                    retvalue = true;
                else
                    retref = true;
            }
        }
        func = true;
    }
    if (!func && symbolDatabase->isClassOrStruct(tok->str()))
        return true;

    return bool(!retref && retvalue);
}

//---------------------------------------------------------------------------

void CheckAutoVariables::returnReference()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        if (!scope->function)
            continue;

        const Token *tok = scope->function->tokenDef;

        // have we reached a function that returns a reference?
        if (tok->previous() && tok->previous()->str() == "&") {
            for (const Token *tok2 = scope->classStart->next(); tok2 && tok2 != scope->classEnd; tok2 = tok2->next()) {
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
                else if (Token::Match(tok2, "return %var% (") &&
                         Token::simpleMatch(tok2->linkAt(2), ") ;")) {
                    if (returnTemporary(tok2->next())) {
                        // report error..
                        errorReturnTempReference(tok2);
                    }
                }
            }
        }
    }
}

void CheckAutoVariables::errorReturnReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnReference", "Reference to auto variable returned.");
}

void CheckAutoVariables::errorReturnTempReference(const Token *tok)
{
    reportError(tok, Severity::error, "returnTempReference", "Reference to temporary returned.");
}

void CheckAutoVariables::errorInvalidDeallocation(const Token *tok)
{
    reportError(tok,
                Severity::error,
                "autovarInvalidDeallocation",
                "Deallocation of an auto-variable results in undefined behaviour.\n"
                "The deallocation of an auto-variable results in undefined behaviour. You should only free memory "
                "that has been allocated dynamically.");
}
