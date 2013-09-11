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
#include "checksizeof.h"
#include "symboldatabase.h"
#include <algorithm>
#include <cctype>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckSizeof instance;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckSizeof::checkSizeofForNumericParameter()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "sizeof ( %num% )") ||
                Token::Match(tok, "sizeof %num%")) {
                sizeofForNumericParameterError(tok);
            }
        }
    }
}

void CheckSizeof::sizeofForNumericParameterError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "sizeofwithnumericparameter", "Suspicious usage of 'sizeof' with a numeric constant as parameter.\n"
                "It is unusual to use a constant value with sizeof. For example, 'sizeof(10)'"
                " returns 4 (in 32-bit systems) or 8 (in 64-bit systems) instead of 10. 'sizeof('A')'"
                " and 'sizeof(char)' can return different results.");
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckSizeof::checkSizeofForArrayParameter()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "sizeof ( %var% )") ||
                Token::Match(tok, "sizeof %var% !![")) {
                const Token* varTok = tok->next();
                if (varTok->str() == "(") {
                    varTok = varTok->next();
                }
                if (varTok->varId() > 0) {
                    const Variable *var = varTok->variable();
                    if (var && var->isArray() && var->isArgument()) {
                        sizeofForArrayParameterError(tok);
                    }
                }
            }
        }
    }
}

void CheckSizeof::sizeofForArrayParameterError(const Token *tok)
{
    reportError(tok, Severity::error,
                "sizeofwithsilentarraypointer", "Using 'sizeof' on array given as function argument "
                "returns size of a pointer.\n"
                "Using 'sizeof' for array given as function argument returns the size of a pointer. "
                "It does not return the size of the whole array in bytes as might be "
                "expected. For example, this code:\n"
                "     int f(char a[100]) {\n"
                "         return sizeof(a);\n"
                "     }\n"
                "returns 4 (in 32-bit systems) or 8 (in 64-bit systems) instead of 100 (the "
                "size of the array in bytes)."
               );
}

void CheckSizeof::checkSizeofForPointerSize()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            const Token *tokVar;
            const Token *variable;
            const Token *variable2 = 0;

            // Find any function that may use sizeof on a pointer
            // Once leaving those tests, it is mandatory to have:
            // - variable matching the used pointer
            // - tokVar pointing on the argument where sizeof may be used
            if (Token::Match(tok, "[*;{}] %var% = malloc|alloca (")) {
                variable = tok->next();
                tokVar = tok->tokAt(5);

            } else if (Token::Match(tok, "[*;{}] %var% = calloc (")) {
                variable = tok->next();
                tokVar = tok->tokAt(5)->nextArgument();

            } else if (Token::simpleMatch(tok, "memset (")) {
                variable = tok->tokAt(2);
                tokVar = variable->tokAt(2)->nextArgument();

                // The following tests can be inconclusive in case the variable in sizeof
                // is constant string by intention
            } else if (!_settings->inconclusive) {
                continue;

            } else if (Token::Match(tok, "memcpy|memcmp|memmove|strncpy|strncmp|strncat (")) {
                variable = tok->tokAt(2);
                variable2 = variable->nextArgument();
                tokVar = variable2->nextArgument();

            } else {
                continue;
            }

            // Ensure the variables are in the symbol database
            // Also ensure the variables are pointers
            // Only keep variables which are pointers
            const Variable *var = variable->variable();
            if (!var || !var->isPointer() || var->isArray()) {
                variable = 0;
            }

            if (variable2) {
                var = variable2->variable();
                if (!var || !var->isPointer() || var->isArray()) {
                    variable2 = 0;
                }
            }

            // If there are no pointer variable at this point, there is
            // no need to continue
            if (variable == 0 && variable2 == 0) {
                continue;
            }

            // Jump to the next sizeof token in the function and in the parameter
            // This is to allow generic operations with sizeof
            for (; tokVar && tokVar->str() != ")" && tokVar->str() != "," && tokVar->str() != "sizeof"; tokVar = tokVar->next()) {}

            // Now check for the sizeof usage. Once here, everything using sizeof(varid) or sizeof(&varid)
            // looks suspicious
            // Do it for first variable
            if (variable && (Token::Match(tokVar, "sizeof ( &| %varid% )", variable->varId()) ||
                             Token::Match(tokVar, "sizeof &| %varid%", variable->varId()))) {
                sizeofForPointerError(variable, variable->str());
            } else if (variable2 && (Token::Match(tokVar, "sizeof ( &| %varid% )", variable2->varId()) ||
                                     Token::Match(tokVar, "sizeof &| %varid%", variable2->varId()))) {
                sizeofForPointerError(variable2, variable2->str());
            }
        }
    }
}

void CheckSizeof::sizeofForPointerError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "pointerSize",
                "Size of pointer '" + varname + "' used instead of size of its data.\n"
                "Size of pointer '" + varname + "' used instead of size of its data. "
                "This is likely to lead to a buffer overflow. You probably intend to "
                "write 'sizeof(*" + varname + ")'.", true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckSizeof::sizeofsizeof()
{
    if (!_settings->isEnabled("warning"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof (| sizeof")) {
            sizeofsizeofError(tok);
            tok = tok->next();
        }
    }
}

void CheckSizeof::sizeofsizeofError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "sizeofsizeof", "Calling 'sizeof' on 'sizeof'.\n"
                "Calling sizeof for 'sizeof looks like a suspicious code and "
                "most likely there should be just one 'sizeof'. The current "
                "code is equivalent to 'sizeof(size_t)'");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckSizeof::sizeofCalculation()
{
    if (!_settings->isEnabled("warning"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            const Token* const end = tok->linkAt(1);
            for (const Token *tok2 = tok->tokAt(2); tok2 != end; tok2 = tok2->next()) {
                if (tok2->isConstOp() && (!tok2->isExpandedMacro() || _settings->inconclusive) && !Token::Match(tok2, ">|<|&") && (Token::Match(tok2->previous(), "%var%") || tok2->str() != "*")) {
                    if (!(Token::Match(tok2->previous(), "%type%") || Token::Match(tok2->next(), "%type%"))) {
                        sizeofCalculationError(tok2, tok2->isExpandedMacro());
                        break;
                    }
                } else if (tok2->type() == Token::eIncDecOp)
                    sizeofCalculationError(tok2, tok2->isExpandedMacro());
            }
        }
    }
}

void CheckSizeof::sizeofCalculationError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::warning,
                "sizeofCalculation", "Found calculation inside sizeof().", inconclusive);
}

//-----------------------------------------------------------------------------
// Check for code like sizeof()*sizeof() or sizeof(ptr)/value
//-----------------------------------------------------------------------------
void CheckSizeof::suspiciousSizeofCalculation()
{
    if (!_settings->isEnabled("warning") || !_settings->inconclusive)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            const Token* const end = tok->linkAt(1);
            const Variable* var = end->previous()->variable();
            if (end->strAt(-1) == "*" || (var && var->isPointer() && !var->isArray())) {
                if (end->strAt(1) == "/")
                    divideSizeofError(tok);
            } else if (Token::simpleMatch(end, ") * sizeof"))
                multiplySizeofError(tok);
        }
    }
}

void CheckSizeof::multiplySizeofError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "multiplySizeof", "Multiplying sizeof() with sizeof() indicates a logic error.", true);
}

void CheckSizeof::divideSizeofError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "divideSizeof", "Division of result of sizeof() on pointer type.\n"
                "Division of result of sizeof() on pointer type. sizeof() returns the size of the pointer, "
                "not the size of the memory area it points to.", true);
}

void CheckSizeof::sizeofVoid()
{
    if (!_settings->isEnabled("portability"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof ( )")) { // "sizeof(void)" gets simplified to sizeof ( )
            sizeofVoidError(tok);
        } else if (Token::Match(tok, "sizeof ( * %var% )") && tok->tokAt(3)->variable() &&
                   (Token::Match(tok->tokAt(3)->variable()->typeStartToken(), "void * !!*")) &&
                   (!tok->tokAt(3)->variable()->isArray())) { // sizeof(*p) where p is of type "void*"
            sizeofDereferencedVoidPointerError(tok, tok->strAt(3));
        } else if (Token::Match(tok, "%var% +|-|++|--") ||
                   Token::Match(tok, "+|-|++|-- %var%")) { // Arithmetic operations on variable of type "void*"
            int index = (tok->isName()) ? 0 : 1;
            const Variable* var = tok->tokAt(index)->variable();
            if (var && Token::Match(var->typeStartToken(), "void *")) {
                std::string varname = tok->strAt(index);
                // In case this 'void *' var is a member then go back to the main object
                const Token* tok2 = tok->tokAt(index);
                if (index == 0) {
                    bool isMember = false;
                    while (Token::Match(tok2->previous(), ".")) {
                        isMember = true;
                        if (Token::Match(tok2->tokAt(-2), ")"))
                            tok2 = tok2->tokAt(-2)->link();
                        else if (Token::Match(tok2->tokAt(-2), "]"))
                            tok2 = tok2->tokAt(-2)->link()->previous();
                        else
                            tok2 = tok2->tokAt(-2);
                    }
                    if (isMember) {
                        // Get 'struct.member' complete name (without spaces)
                        varname = tok2->stringifyList(tok->tokAt(index)->next());
                        varname.erase(remove_if(varname.begin(), varname.end(),
                                                static_cast<int (*)(int)>(std::isspace)), varname.end());
                    }
                }
                // Check for cast on operations with '+|-'
                if (Token::Match(tok, "%var% +|-")) {
                    // Check for cast expression
                    if (Token::Match(tok2->previous(), ")") && !Token::Match(tok2->previous()->link(), "( const| void *"))
                        continue;
                }
                arithOperationsOnVoidPointerError(tok, varname,
                                                  var->typeStartToken()->stringifyList(var->typeEndToken()->next()));
            }
        }
    }
}

void CheckSizeof::sizeofVoidError(const Token *tok)
{
    const std::string message = "Behaviour of 'sizeof(void)' is not covered by the ISO C standard.";
    const std::string verbose = message + " A value for 'sizeof(void)' is defined only as part of a GNU C extension, which defines 'sizeof(void)' to be 1.";
    reportError(tok, Severity::portability, "sizeofVoid", message + "\n" + verbose);
}

void CheckSizeof::sizeofDereferencedVoidPointerError(const Token *tok, const std::string &varname)
{
    const std::string message = "'*" + varname + "' is of type 'void', the behaviour of 'sizeof(void)' is not covered by the ISO C standard.";
    const std::string verbose = message + " A value for 'sizeof(void)' is defined only as part of a GNU C extension, which defines 'sizeof(void)' to be 1.";
    reportError(tok, Severity::portability, "sizeofDereferencedVoidPointer", message + "\n" + verbose);
}

void CheckSizeof::arithOperationsOnVoidPointerError(const Token* tok, const std::string &varname, const std::string &vartype)
{
    const std::string message = "'" + varname + "' is of type '" + vartype + "'. When using void pointers in calculations, the behaviour is undefined.";
    const std::string verbose = message + " Arithmetic operations on 'void *' is a GNU C extension, which defines the 'sizeof(void)' to be 1.";
    reportError(tok, Severity::portability, "arithOperationsOnVoidPointer", message + "\n" + verbose);
}
