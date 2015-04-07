/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include "checkvaarg.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckVaarg instance;
}


//---------------------------------------------------------------------------
// Ensure that correct parameter is passed to va_start()
//---------------------------------------------------------------------------

void CheckVaarg::va_start_argument()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    const bool printWarnings = _settings->isEnabled("warning");

    for (std::size_t i = 0; i < functions; ++i) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        const Function* function = scope->function;
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->scope()->isExecutable())
                tok = tok->scope()->classEnd;
            else if (Token::simpleMatch(tok, "va_start (")) {
                const Token* param2 = tok->tokAt(2)->nextArgument();
                if (!param2)
                    continue;
                const Variable* var = param2->variable();
                if (var && var->isReference())
                    referenceAs_va_start_error(param2, var->name());
                if (var && var->index() + 2 < function->argCount() && printWarnings) {
                    std::list<Variable>::const_reverse_iterator it = function->argumentList.rbegin();
                    ++it;
                    wrongParameterTo_va_start_error(tok, var->name(), it->name());
                }
                tok = tok->linkAt(1);
            }
        }
    }
}

void CheckVaarg::wrongParameterTo_va_start_error(const Token *tok, const std::string& paramIsName, const std::string& paramShouldName)
{
    reportError(tok, Severity::warning,
                "va_start_wrongParameter", "'" + paramIsName + "' given to va_start() is not last named argument of the function. Did you intend to pass '" + paramShouldName + "'?");
}

void CheckVaarg::referenceAs_va_start_error(const Token *tok, const std::string& paramName)
{
    reportError(tok, Severity::error,
                "va_start_referencePassed", "Using reference '" + paramName + "' as parameter for va_start() results in undefined behaviour.");
}

//---------------------------------------------------------------------------
// Detect missing va_end() if va_start() was used
// Detect va_list usage after va_end()
//---------------------------------------------------------------------------

void CheckVaarg::va_list_usage()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    for (unsigned int varid = 1; varid < symbolDatabase->getVariableListSize(); varid++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(varid);
        if (!var || var->isPointer() || var->isReference() || var->isArray() || !var->scope() || var->typeStartToken()->str() != "va_list")
            continue;
        if (!var->isLocal() && !var->isArgument()) // Check only local variables and arguments
            continue;

        bool open = var->isArgument(); // va_list passed as argument are opened
        bool exitOnEndOfStatement = false;

        const Token* tok = var->nameToken()->next();
        for (; tok != var->scope()->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "va_start ( %varid%", var->declarationId())) {
                if (open)
                    va_start_subsequentCallsError(tok, var->name());
                open = true;
                tok = tok->linkAt(1);
            } else if (Token::Match(tok, "va_end ( %varid%", var->declarationId())) {
                if (!open)
                    va_list_usedBeforeStartedError(tok, var->name());
                open = false;
                tok = tok->linkAt(1);
            } else if (Token::simpleMatch(tok, "va_copy (")) {
                bool nopen = open;
                if (tok->linkAt(1)->previous()->varId() == var->declarationId()) { // Source
                    if (!open)
                        va_list_usedBeforeStartedError(tok, var->name());
                }
                if (tok->tokAt(2)->varId() == var->declarationId()) { // Destination
                    if (open)
                        va_start_subsequentCallsError(tok, var->name());
                    nopen = true;
                }
                open = nopen;
                tok = tok->linkAt(1);
            } else if (Token::Match(tok, "throw|return"))
                exitOnEndOfStatement = true;
            else if (_tokenizer->isCPP() && tok->str() == "try") {
                open = false;
                break;
            } else if (!open && tok->varId() == var->declarationId())
                va_list_usedBeforeStartedError(tok, var->name());
            else if (exitOnEndOfStatement && tok->str() == ";")
                break;
        }
        if (open && !var->isArgument())
            va_end_missingError(tok, var->name());
    }
}

void CheckVaarg::va_end_missingError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "va_end_missing", "va_list '" + varname + "' was opened but not closed by va_end().");
}

void CheckVaarg::va_list_usedBeforeStartedError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "va_list_usedBeforeStarted", "va_list '" + varname + "' used before va_start() was called.");
}

void CheckVaarg::va_start_subsequentCallsError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "va_start_subsequentCalls", "va_start() or va_copy() called subsequently on '" + varname + "' without va_end() inbetween.");
}
