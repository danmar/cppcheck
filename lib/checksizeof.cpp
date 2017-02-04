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
#include "checksizeof.h"
#include "symboldatabase.h"
#include <algorithm>
#include <cctype>


//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckSizeof instance;
}

// CWE IDs used:
static const struct CWE CWE398(398U);   // Indicator of Poor Code Quality
static const struct CWE CWE467(467U);   // Use of sizeof() on a Pointer Type
static const struct CWE CWE682(682U);   // Incorrect Calculation
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
                " and 'sizeof(char)' can return different results.", CWE682, false);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckSizeof::checkSizeofForArrayParameter()
{
    if (!_settings->isEnabled("warning"))
        return;
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

                const Variable *var = varTok->variable();
                if (var && var->isArray() && var->isArgument() && !var->isReference())
                    sizeofForArrayParameterError(tok);
            }
        }
    }
}

void CheckSizeof::sizeofForArrayParameterError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "sizeofwithsilentarraypointer", "Using 'sizeof' on array given as function argument "
                "returns size of a pointer.\n"
                "Using 'sizeof' for array given as function argument returns the size of a pointer. "
                "It does not return the size of the whole array in bytes as might be "
                "expected. For example, this code:\n"
                "     int f(char a[100]) {\n"
                "         return sizeof(a);\n"
                "     }\n"
                "returns 4 (in 32-bit systems) or 8 (in 64-bit systems) instead of 100 (the "
                "size of the array in bytes).", CWE467, false
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
            const Token* tokSize;
            const Token* tokFunc;
            const Token *variable = nullptr;
            const Token *variable2 = nullptr;

            // Find any function that may use sizeof on a pointer
            // Once leaving those tests, it is mandatory to have:
            // - variable matching the used pointer
            // - tokVar pointing on the argument where sizeof may be used
            if (Token::Match(tok->tokAt(2), "malloc|alloca|calloc (")) {
                if (Token::Match(tok, "%var% ="))
                    variable = tok;
                else if (tok->strAt(1) == ")" && Token::Match(tok->linkAt(1)->tokAt(-2), "%var% ="))
                    variable = tok->linkAt(1)->tokAt(-2);
                else if (tok->link() && Token::Match(tok, "> ( malloc|alloca|calloc (") && Token::Match(tok->link()->tokAt(-3), "%var% ="))
                    variable = tok->link()->tokAt(-3);
                tokSize = tok->tokAt(4);
                tokFunc = tok->tokAt(2);
            } else if (Token::simpleMatch(tok, "memset (") && tok->strAt(-1) != ".") {
                variable = tok->tokAt(2);
                tokSize = variable->nextArgument();
                if (tokSize)
                    tokSize = tokSize->nextArgument();
                tokFunc = tok;
            } else if (Token::Match(tok, "memcpy|memcmp|memmove|strncpy|strncmp|strncat (") && tok->strAt(-1) != ".") {
                variable = tok->tokAt(2);
                variable2 = variable->nextArgument();
                if (!variable2)
                    continue;
                tokSize = variable2->nextArgument();
                tokFunc = tok;
            } else {
                continue;
            }

            if (tokFunc && tokFunc->str() == "calloc")
                tokSize = tokSize->nextArgument();

            if (tokFunc && tokSize) {
                for (const Token* tok2 = tokSize; tok2 != tokFunc->linkAt(1); tok2 = tok2->next()) {
                    if (Token::simpleMatch(tok2, "/ sizeof")) {
                        // Allow division with sizeof(char)
                        if (Token::simpleMatch(tok2->next(), "sizeof (")) {
                            const Token *sztok = tok2->tokAt(2)->astOperand2();
                            const ValueType *vt = ((sztok != nullptr) ? sztok->valueType() : nullptr);
                            if (vt && vt->type == ValueType::CHAR && vt->pointer == 0)
                                continue;
                        }

                        divideBySizeofError(tok2, tokFunc->str());
                    }
                }
            }

            if (!variable || !tokSize)
                continue;

            while (Token::Match(variable, "%var% ::|."))
                variable = variable->tokAt(2);

            while (Token::Match(variable2, "%var% ::|."))
                variable2 = variable2->tokAt(2);

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
            for (; tokSize && tokSize->str() != ")" && tokSize->str() != "," && tokSize->str() != "sizeof"; tokSize = tokSize->next()) {}

            if (tokSize->str() != "sizeof")
                continue;

            // Now check for the sizeof usage: Does the level of pointer indirection match?
            if (tokSize->linkAt(1)->strAt(-1) == "*") {
                if (variable && variable->valueType() && variable->valueType()->pointer == 1 && variable->valueType()->type != ValueType::VOID)
                    sizeofForPointerError(variable, variable->str());
                else if (variable2 && variable2->valueType() && variable2->valueType()->pointer == 1 && variable2->valueType()->type != ValueType::VOID)
                    sizeofForPointerError(variable2, variable2->str());
            }

            if (Token::simpleMatch(tokSize, "sizeof ( &"))
                tokSize = tokSize->tokAt(3);
            else if (Token::Match(tokSize, "sizeof (|&"))
                tokSize = tokSize->tokAt(2);
            else
                tokSize = tokSize->next();

            while (Token::Match(tokSize, "%var% ::|."))
                tokSize = tokSize->tokAt(2);

            if (Token::Match(tokSize, "%var% [|("))
                continue;

            // Now check for the sizeof usage again. Once here, everything using sizeof(varid) or sizeof(&varid)
            // looks suspicious
            if (variable && tokSize->varId() == variable->varId())
                sizeofForPointerError(variable, variable->str());
            if (variable2 && tokSize->varId() == variable2->varId())
                sizeofForPointerError(variable2, variable2->str());
        }
    }
}

void CheckSizeof::sizeofForPointerError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "pointerSize",
                "Size of pointer '" + varname + "' used instead of size of its data.\n"
                "Size of pointer '" + varname + "' used instead of size of its data. "
                "This is likely to lead to a buffer overflow. You probably intend to "
                "write 'sizeof(*" + varname + ")'.", CWE467, false);
}

void CheckSizeof::divideBySizeofError(const Token *tok, const std::string &memfunc)
{
    reportError(tok, Severity::warning, "sizeofDivisionMemfunc",
                "Division by result of sizeof(). " + memfunc + "() expects a size in bytes, did you intend to multiply instead?", CWE682, false);
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
                "code is equivalent to 'sizeof(size_t)'", CWE682, false);
}

//-----------------------------------------------------------------------------

void CheckSizeof::sizeofCalculation()
{
    if (!_settings->isEnabled("warning"))
        return;

    const bool printInconclusive = _settings->inconclusive;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {

            // ignore if the `sizeof` result is cast to void inside a macro, i.e. the calculation is
            // expected to be parsed but skipped, such as in a disabled custom ASSERT() macro
            if (tok->isExpandedMacro() && tok->previous()) {
                const Token *cast_end = (tok->previous()->str() == "(") ? tok->previous() : tok;
                if (Token::simpleMatch(cast_end->tokAt(-3), "( void )") ||
                    Token::simpleMatch(cast_end->previous(), "static_cast<void>")) {
                    continue;
                }
            }

            const Token *argument = tok->next()->astOperand2();
            if (argument && argument->isCalculation() && (!argument->isExpandedMacro() || printInconclusive))
                sizeofCalculationError(argument, argument->isExpandedMacro());
        }
    }
}

void CheckSizeof::sizeofCalculationError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::warning,
                "sizeofCalculation", "Found calculation inside sizeof().", CWE682, inconclusive);
}

//-----------------------------------------------------------------------------
// Check for code like sizeof()*sizeof() or sizeof(ptr)/value
//-----------------------------------------------------------------------------
void CheckSizeof::suspiciousSizeofCalculation()
{
    if (!_settings->isEnabled("warning") || !_settings->inconclusive)
        return;

    // TODO: Use AST here. This should be possible as soon as sizeof without brackets is correctly parsed
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            const Token* const end = tok->linkAt(1);
            const Variable* var = end->previous()->variable();
            if (end->strAt(-1) == "*" || (var && var->isPointer() && !var->isArray())) {
                if (end->strAt(1) == "/")
                    divideSizeofError(tok);
            } else if (Token::simpleMatch(end, ") * sizeof") && end->next()->astOperand1() == tok->next())
                multiplySizeofError(tok);
        }
    }
}

void CheckSizeof::multiplySizeofError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "multiplySizeof", "Multiplying sizeof() with sizeof() indicates a logic error.", CWE682, true);
}

void CheckSizeof::divideSizeofError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "divideSizeof", "Division of result of sizeof() on pointer type.\n"
                "Division of result of sizeof() on pointer type. sizeof() returns the size of the pointer, "
                "not the size of the memory area it points to.", CWE682, true);
}

void CheckSizeof::sizeofVoid()
{
    if (!_settings->isEnabled("portability"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof ( )")) { // "sizeof(void)" gets simplified to sizeof ( )
            sizeofVoidError(tok);
        } else if (Token::simpleMatch(tok, "sizeof (") && tok->next()->astOperand2()) {
            const ValueType *vt = tok->next()->astOperand2()->valueType();
            if (vt && vt->type == ValueType::Type::VOID && vt->pointer == 0U)
                sizeofDereferencedVoidPointerError(tok, tok->strAt(3));
        } else if (tok->str() == "-") {
            // only warn for: 'void *' - 'integral'
            const ValueType *vt1  = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
            const ValueType *vt2  = tok->astOperand2() ? tok->astOperand2()->valueType() : nullptr;
            bool op1IsvoidPointer = (vt1 && vt1->type == ValueType::Type::VOID && vt1->pointer == 1U);
            bool op2IsIntegral    = (vt2 && vt2->isIntegral() && vt2->pointer == 0U);
            if (op1IsvoidPointer && op2IsIntegral)
                arithOperationsOnVoidPointerError(tok, tok->astOperand1()->expressionString(), vt1->str());
        } else if (Token::Match(tok, "+|++|--|+=|-=")) { // Arithmetic operations on variable of type "void*"
            const ValueType *vt1 = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
            const ValueType *vt2 = tok->astOperand2() ? tok->astOperand2()->valueType() : nullptr;

            bool voidpointer1 = (vt1 && vt1->type == ValueType::Type::VOID && vt1->pointer == 1U);
            bool voidpointer2 = (vt2 && vt2->type == ValueType::Type::VOID && vt2->pointer == 1U);

            if (voidpointer1)
                arithOperationsOnVoidPointerError(tok, tok->astOperand1()->expressionString(), vt1->str());

            if (!tok->isAssignmentOp() && voidpointer2)
                arithOperationsOnVoidPointerError(tok, tok->astOperand2()->expressionString(), vt2->str());
        }
    }
}

void CheckSizeof::sizeofVoidError(const Token *tok)
{
    const std::string message = "Behaviour of 'sizeof(void)' is not covered by the ISO C standard.";
    const std::string verbose = message + " A value for 'sizeof(void)' is defined only as part of a GNU C extension, which defines 'sizeof(void)' to be 1.";
    reportError(tok, Severity::portability, "sizeofVoid", message + "\n" + verbose, CWE682, false);
}

void CheckSizeof::sizeofDereferencedVoidPointerError(const Token *tok, const std::string &varname)
{
    const std::string message = "'*" + varname + "' is of type 'void', the behaviour of 'sizeof(void)' is not covered by the ISO C standard.";
    const std::string verbose = message + " A value for 'sizeof(void)' is defined only as part of a GNU C extension, which defines 'sizeof(void)' to be 1.";
    reportError(tok, Severity::portability, "sizeofDereferencedVoidPointer", message + "\n" + verbose, CWE682, false);
}

void CheckSizeof::arithOperationsOnVoidPointerError(const Token* tok, const std::string &varname, const std::string &vartype)
{
    const std::string message = "'" + varname + "' is of type '" + vartype + "'. When using void pointers in calculations, the behaviour is undefined.";
    const std::string verbose = message + " Arithmetic operations on 'void *' is a GNU C extension, which defines the 'sizeof(void)' to be 1.";
    reportError(tok, Severity::portability, "arithOperationsOnVoidPointer", message + "\n" + verbose, CWE467, false);
}
