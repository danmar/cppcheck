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
#include "checkother.h"
#include "astutils.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "utils.h"

#include <stack>
#include <algorithm> // find_if()
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckOther instance;
}

static const struct CWE CWE128(128U);   // Wrap-around Error
static const struct CWE CWE131(131U);   // Incorrect Calculation of Buffer Size
static const struct CWE CWE197(197U);   // Numeric Truncation Error
static const struct CWE CWE362(362U);   // Concurrent Execution using Shared Resource with Improper Synchronization ('Race Condition')
static const struct CWE CWE369(369U);   // Divide By Zero
static const struct CWE CWE398(398U);   // Indicator of Poor Code Quality
static const struct CWE CWE475(475U);   // Undefined Behavior for Input to API
static const struct CWE CWE482(482U);   // Comparing instead of Assigning
static const struct CWE CWE561(561U);   // Dead Code
static const struct CWE CWE563(563U);   // Assignment to Variable without Use ('Unused Variable')
static const struct CWE CWE570(570U);   // Expression is Always False
static const struct CWE CWE571(571U);   // Expression is Always True
static const struct CWE CWE672(672U);   // Operation on a Resource after Expiration or Release
static const struct CWE CWE628(628U);   // Function Call with Incorrectly Specified Arguments
static const struct CWE CWE683(683U);   // Function Call With Incorrect Order of Arguments
static const struct CWE CWE686(686U);   // Function Call With Incorrect Argument Type
static const struct CWE CWE687(687U);   // Function Call With Incorrectly Specified Argument Value
static const struct CWE CWE688(688U);   // Function Call With Incorrect Variable or Reference as Argument
static const struct CWE CWE704(704U);   // Incorrect Type Conversion or Cast
static const struct CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const struct CWE CWE768(768U);   // Incorrect Short Circuit Evaluation
static const struct CWE CWE783(783U);   // Operator Precedence Logic Error

//----------------------------------------------------------------------------------
// The return value of fgetc(), getc(), ungetc(), getchar() etc. is an integer value.
// If this return value is stored in a character variable and then compared
// to compared to EOF, which is an integer, the comparison maybe be false.
//
// Reference:
// - Ticket #160
// - http://www.cplusplus.com/reference/cstdio/fgetc/
// - http://www.cplusplus.com/reference/cstdio/getc/
// - http://www.cplusplus.com/reference/cstdio/getchar/
// - http://www.cplusplus.com/reference/cstdio/ungetc/ ...
//----------------------------------------------------------------------------------
void CheckOther::checkCastIntToCharAndBack()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        std::map<unsigned int, std::string> vars;
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // Quick check to see if any of the matches below have any chances
            if (!tok->isName() || !Token::Match(tok, "%var%|EOF %comp%|="))
                continue;
            if (Token::Match(tok, "%var% = fclose|fflush|fputc|fputs|fscanf|getchar|getc|fgetc|putchar|putc|puts|scanf|sscanf|ungetc (")) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    vars[tok->varId()] = tok->strAt(2);
                }
            } else if (Token::Match(tok, "EOF %comp% ( %var% = fclose|fflush|fputc|fputs|fscanf|getchar|getc|fgetc|putchar|putc|puts|scanf|sscanf|ungetc (")) {
                tok = tok->tokAt(3);
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    checkCastIntToCharAndBackError(tok, tok->strAt(2));
                }
            } else if (_tokenizer->isCPP() && (Token::Match(tok, "EOF %comp% ( %var% = std :: cin . get (") || Token::Match(tok, "EOF %comp% ( %var% = cin . get ("))) {
                tok = tok->tokAt(3);
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    checkCastIntToCharAndBackError(tok, "cin.get");
                }
            } else if (_tokenizer->isCPP() && (Token::Match(tok, "%var% = std :: cin . get (") || Token::Match(tok, "%var% = cin . get ("))) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    vars[tok->varId()] = "cin.get";
                }
            } else if (Token::Match(tok, "%var% %comp% EOF")) {
                if (vars.find(tok->varId()) != vars.end()) {
                    checkCastIntToCharAndBackError(tok, vars[tok->varId()]);
                }
            } else if (Token::Match(tok, "EOF %comp% %var%")) {
                tok = tok->tokAt(2);
                if (vars.find(tok->varId()) != vars.end()) {
                    checkCastIntToCharAndBackError(tok, vars[tok->varId()]);
                }
            }
        }
    }
}

void CheckOther::checkCastIntToCharAndBackError(const Token *tok, const std::string &strFunctionName)
{
    reportError(
        tok,
        Severity::warning,
        "checkCastIntToCharAndBack",
        "Storing "+ strFunctionName +"() return value in char variable and then comparing with EOF.\n"
        "When saving "+ strFunctionName +"() return value in char variable there is loss of precision. "
        " When "+ strFunctionName +"() returns EOF this value is truncated. Comparing the char "
        "variable with EOF can have unexpected results. For instance a loop \"while (EOF != (c = "+ strFunctionName +"());\" "
        "loops forever on some compilers/platforms and on other compilers/platforms it will stop "
        "when the file contains a matching character.", CWE197, false
    );
}


//---------------------------------------------------------------------------
// Clarify calculation precedence for ternary operators.
//---------------------------------------------------------------------------
void CheckOther::clarifyCalculation()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // ? operator where lhs is arithmetical expression
            if (tok->str() != "?" || !tok->astOperand1() || !tok->astOperand1()->isCalculation())
                continue;
            if (!tok->astOperand1()->isArithmeticalOp() && tok->astOperand1()->tokType() != Token::eBitOp)
                continue;

            // Is code clarified by parentheses already?
            const Token *tok2 = tok->astOperand1();
            for (; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->str() == ")")
                    break;
                else if (tok2->str() == "?") {
                    clarifyCalculationError(tok, tok->astOperand1()->str());
                    break;
                }
            }
        }
    }
}

void CheckOther::clarifyCalculationError(const Token *tok, const std::string &op)
{
    // suspicious calculation
    const std::string calc("'a" + op + "b?c:d'");

    // recommended calculation #1
    const std::string s1("'(a" + op + "b)?c:d'");

    // recommended calculation #2
    const std::string s2("'a" + op + "(b?c:d)'");

    reportError(tok,
                Severity::style,
                "clarifyCalculation",
                "Clarify calculation precedence for '" + op + "' and '?'.\n"
                "Suspicious calculation. Please use parentheses to clarify the code. "
                "The code '" + calc + "' should be written as either '" + s1 + "' or '" + s2 + "'.", CWE783, false);
}

//---------------------------------------------------------------------------
// Clarify (meaningless) statements like *foo++; with parentheses.
//---------------------------------------------------------------------------
void CheckOther::clarifyStatement()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "* %name%") && tok->astOperand1()) {
                const Token *tok2 = tok->previous();

                while (tok2 && tok2->str() == "*")
                    tok2 = tok2->previous();

                if (tok2 && !tok2->astParent() && Token::Match(tok2, "[{};]")) {
                    tok2 = tok->astOperand1();
                    if (Token::Match(tok2, "++|-- [;,]"))
                        clarifyStatementError(tok2);
                }
            }
        }
    }
}

void CheckOther::clarifyStatementError(const Token *tok)
{
    reportError(tok, Severity::warning, "clarifyStatement", "Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n"
                "A statement like '*A++;' might not do what you intended. Postfix 'operator++' is executed before 'operator*'. "
                "Thus, the dereference is meaningless. Did you intend to write '(*A)++;'?", CWE783, false);
}

//---------------------------------------------------------------------------
// Check for suspicious occurrences of 'if(); {}'.
//---------------------------------------------------------------------------
void CheckOther::checkSuspiciousSemicolon()
{
    if (!_settings->inconclusive || !_settings->isEnabled("warning"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Look for "if(); {}", "for(); {}" or "while(); {}"
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type == Scope::eIf || i->type == Scope::eElse || i->type == Scope::eWhile || i->type == Scope::eFor) {
            // Ensure the semicolon is at the same line number as the if/for/while statement
            // and the {..} block follows it without an extra empty line.
            if (Token::simpleMatch(i->classStart, "{ ; } {") &&
                i->classStart->previous()->linenr() == i->classStart->tokAt(2)->linenr()
                && i->classStart->linenr()+1 >= i->classStart->tokAt(3)->linenr()) {
                SuspiciousSemicolonError(i->classDef);
            }
        }
    }
}

void CheckOther::SuspiciousSemicolonError(const Token* tok)
{
    reportError(tok, Severity::warning, "suspiciousSemicolon",
                "Suspicious use of ; at the end of '" + (tok ? tok->str() : std::string()) + "' statement.", CWE398, true);
}


//---------------------------------------------------------------------------
// For C++ code, warn if C-style casts are used on pointer types
//---------------------------------------------------------------------------
void CheckOther::warningOldStylePointerCast()
{
    // Only valid on C++ code
    if (!_settings->isEnabled("style") || !_tokenizer->isCPP())
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        const Token* tok;
        if (scope->function && scope->function->isConstructor())
            tok = scope->classDef;
        else
            tok = scope->classStart;
        for (; tok && tok != scope->classEnd; tok = tok->next()) {
            // Old style pointer casting..
            if (!Token::Match(tok, "( const| %type% * const| ) (| %name%|%num%|%bool%|%char%|%str%"))
                continue;

            // skip first "const" in "const Type* const"
            if (tok->strAt(1) == "const")
                tok = tok->next();
            const Token* typeTok = tok->next();
            // skip second "const" in "const Type* const"
            if (tok->strAt(3) == "const")
                tok = tok->next();

            const Token *p = tok->tokAt(4);
            if (p->hasKnownIntValue() && p->values().front().intvalue==0) // Casting nullpointers is safe
                continue;

            // Is "type" a class?
            if (typeTok->type())
                cstyleCastError(tok);
        }
    }
}

void CheckOther::cstyleCastError(const Token *tok)
{
    reportError(tok, Severity::style, "cstyleCast",
                "C-style pointer casting\n"
                "C-style pointer casting detected. C++ offers four different kinds of casts as replacements: "
                "static_cast, const_cast, dynamic_cast and reinterpret_cast. A C-style cast could evaluate to "
                "any of those automatically, thus it is considered safer if the programmer explicitly states "
                "which kind of cast is expected. See also: https://www.securecoding.cert.org/confluence/display/cplusplus/EXP05-CPP.+Do+not+use+C-style+casts.", CWE398, false);
}

//---------------------------------------------------------------------------
// float* f; double* d = (double*)f; <-- Pointer cast to a type with an incompatible binary data representation
//---------------------------------------------------------------------------

void CheckOther::invalidPointerCast()
{
    if (!_settings->isEnabled("portability"))
        return;

    const bool printInconclusive = _settings->inconclusive;
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            const Token* toTok = nullptr;
            const Token* fromTok = nullptr;
            // Find cast
            if (Token::Match(tok, "( const| %type% %type%| const| * )")) {
                toTok = tok;
                fromTok = tok->astOperand1();
            } else if (Token::simpleMatch(tok, "reinterpret_cast <") && tok->linkAt(1)) {
                toTok = tok->linkAt(1)->next();
                fromTok = toTok->astOperand2();
            }
            if (!fromTok)
                continue;

            const ValueType* fromType = fromTok->valueType();
            const ValueType* toType = toTok->valueType();
            if (!fromType || !toType || !fromType->pointer || !toType->pointer)
                continue;

            if (fromType->type != toType->type && fromType->type >= ValueType::Type::BOOL && toType->type >= ValueType::Type::BOOL && (toType->type != ValueType::Type::CHAR || printInconclusive)) {
                if (toType->isIntegral() && fromType->isIntegral())
                    continue;
                std::string toStr = toType->isIntegral() ? "integer *" : toType->str();
                toStr = toStr.substr(0, toStr.size()-2);
                std::string fromStr = fromType->isIntegral() ? "integer *" : fromType->str();
                fromStr = fromStr.substr(0, fromStr.size() - 2);

                invalidPointerCastError(tok, fromStr, toStr, toType->type == ValueType::Type::CHAR);
            }
        }
    }
}

void CheckOther::invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to, bool inconclusive)
{
    if (to == "integer") { // If we cast something to int*, this can be useful to play with its binary data representation
        if (!inconclusive)
            reportError(tok, Severity::portability, "invalidPointerCast", "Casting from " + from + "* to integer* is not portable due to different binary data representations on different platforms.", CWE704, false);
        else
            reportError(tok, Severity::portability, "invalidPointerCast", "Casting from " + from + "* to char* is not portable due to different binary data representations on different platforms.", CWE704, true);
    } else
        reportError(tok, Severity::portability, "invalidPointerCast", "Casting between " + from + "* and " + to + "* which have an incompatible binary data representation.", CWE704, false);
}

//---------------------------------------------------------------------------
// This check detects errors on POSIX systems, when a pipe command called
// with a wrong dimensioned file descriptor array. The pipe command requires
// exactly an integer array of dimension two as parameter.
//
// References:
//  - http://linux.die.net/man/2/pipe
//  - ticket #3521
//---------------------------------------------------------------------------
void CheckOther::checkPipeParameterSize()
{
    if (!_settings->standards.posix)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "pipe ( %var% )") ||
                Token::Match(tok, "pipe2 ( %var% ,")) {
                const Token * const varTok = tok->tokAt(2);

                const Variable *var = varTok->variable();
                MathLib::bigint dim;
                if (var && var->isArray() && !var->isArgument() && ((dim=var->dimension(0U)) < 2)) {
                    const std::string strDim = MathLib::toString(dim);
                    checkPipeParameterSizeError(varTok,varTok->str(), strDim);
                }
            }
        }
    }
}

void CheckOther::checkPipeParameterSizeError(const Token *tok, const std::string &strVarName, const std::string &strDim)
{
    reportError(tok, Severity::error,
                "wrongPipeParameterSize", "Buffer '" + strVarName + "' must have size of 2 integers if used as parameter of pipe().\n"
                "The pipe()/pipe2() system command takes an argument, which is an array of exactly two integers.\n"
                "The variable '" + strVarName + "' is an array of size " + strDim + ", which does not match.", CWE686, false);
}

//---------------------------------------------------------------------------
// Detect redundant assignments: x = 0; x = 4;
//---------------------------------------------------------------------------

static bool nonLocal(const Variable* var)
{
    return !var || (!var->isLocal() && !var->isArgument()) || var->isStatic() || var->isReference();
}

static bool nonLocalVolatile(const Variable* var)
{
    if (var && var->isVolatile())
        return false;
    return nonLocal(var);
}

static void eraseNotLocalArg(std::map<unsigned int, const Token*>& container, const SymbolDatabase* symbolDatabase)
{
    for (std::map<unsigned int, const Token*>::iterator i = container.begin(); i != container.end();) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i->first);
        if (!var || nonLocal(var)) {
            container.erase(i++);
        } else
            ++i;
    }
}

static void eraseMemberAssignments(const unsigned int varId, const std::map<unsigned int, std::set<unsigned int> > &membervars, std::map<unsigned int, const Token*> &varAssignments)
{
    const std::map<unsigned int, std::set<unsigned int> >::const_iterator it = membervars.find(varId);
    if (it != membervars.end()) {
        const std::set<unsigned int>& v = it->second;
        for (std::set<unsigned int>::const_iterator vit = v.begin(); vit != v.end(); ++vit) {
            varAssignments.erase(*vit);
            if (*vit != varId)
                eraseMemberAssignments(*vit, membervars, varAssignments);
        }
    }
}

static bool checkExceptionHandling(const Token* tok)
{
    const Variable* var = tok->variable();
    const Scope* upperScope = tok->scope();
    if (var && upperScope == var->scope())
        return true;
    while (upperScope && upperScope->type != Scope::eTry && upperScope->type != Scope::eLambda && (!var || upperScope->nestedIn != var->scope()) && upperScope->isExecutable()) {
        upperScope = upperScope->nestedIn;
    }
    if (var && upperScope && upperScope->type == Scope::eTry) {
        // Check all exception han
        const Token* tok2 = upperScope->classEnd;
        while (Token::simpleMatch(tok2, "} catch (")) {
            tok2 = tok2->linkAt(2)->next();
            if (Token::findmatch(tok2, "%varid%", tok2->link(), var->declarationId()))
                return false;
            tok2 = tok2->link();
        }
    }
    return true;
}

void CheckOther::checkRedundantAssignment()
{
    const bool printPerformance = _settings->isEnabled("performance");
    const bool printStyle = _settings->isEnabled("style");
    const bool printWarning = _settings->isEnabled("warning");
    if (!printWarning && !printPerformance && !printStyle)
        return;

    const bool printInconclusive = _settings->inconclusive;
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (!scope->isExecutable())
            continue;

        std::map<unsigned int, const Token*> varAssignments;
        std::map<unsigned int, const Token*> memAssignments;
        std::map<unsigned int, std::set<unsigned int> > membervars;
        std::set<unsigned int> initialized;
        const Token* writtenArgumentsEnd = nullptr;

        for (const Token* tok = scope->classStart->next(); tok && tok != scope->classEnd; tok = tok->next()) {
            if (tok == writtenArgumentsEnd)
                writtenArgumentsEnd = nullptr;

            if (tok->str() == "?" && tok->astOperand2()) {
                tok = Token::findmatch(tok->astOperand2(), ";|}");
                if (!tok)
                    break;
                varAssignments.clear();
                memAssignments.clear();
            } else if (tok->str() == "{" && tok->strAt(-1) != "{" && tok->strAt(-1) != "=" && tok->strAt(-4) != "case" && tok->strAt(-3) != "default") { // conditional or non-executable inner scope: Skip it and reset status
                tok = tok->link();
                varAssignments.clear();
                memAssignments.clear();
            } else if (Token::Match(tok, "for|if|while (")) {
                tok = tok->linkAt(1);
            } else if (Token::Match(tok, "break|return|continue|throw|goto|asm")) {
                varAssignments.clear();
                memAssignments.clear();
            } else if (tok->tokType() == Token::eVariable && !Token::Match(tok, "%name% (")) {
                const Token *eq = nullptr;
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                    if (Token::Match(tok2, "[([]")) {
                        // bail out if there is a variable in rhs - we only track 1 variable
                        bool bailout = false;
                        for (const Token *tok3 = tok2->link(); tok3 != tok2; tok3 = tok3->previous()) {
                            if (tok3->varId()) {
                                if (!tok3->variable())
                                    bailout = true;
                                else {
                                    const Variable *var = tok3->variable();
                                    if (!var->isConst() || var->isReference() || var->isPointer())
                                        bailout = true;
                                }
                            }
                        }
                        if (bailout)
                            break;
                        tok2 = tok2->link();
                    } else if (Token::Match(tok2, "[)];,]"))
                        break;
                    else if (tok2->str() == "=") {
                        eq = tok2;
                        break;
                    }
                }

                // Set initialization flag
                if (!Token::Match(tok, "%var% ["))
                    initialized.insert(tok->varId());
                else {
                    const Token *tok2 = tok->next();
                    while (tok2 && tok2->str() == "[")
                        tok2 = tok2->link()->next();
                    if (tok2 && tok2->str() != ";")
                        initialized.insert(tok->varId());
                }

                const Token *startToken = tok;
                while (Token::Match(startToken, "%name%|::|.")) {
                    startToken = startToken->previous();
                    if (Token::Match(startToken, "%name% . %var%"))
                        membervars[startToken->varId()].insert(startToken->tokAt(2)->varId());
                }

                std::map<unsigned int, const Token*>::iterator it = varAssignments.find(tok->varId());
                if (eq && Token::Match(startToken, "[;{}]")) { // Assignment
                    if (it != varAssignments.end()) {
                        const Token *oldeq = nullptr;
                        for (const Token *tok2 = it->second; tok2; tok2 = tok2->next()) {
                            if (Token::Match(tok2, "[([]"))
                                tok2 = tok2->link();
                            else if (Token::Match(tok2, "[)];,]"))
                                break;
                            else if (Token::Match(tok2, "++|--|=")) {
                                oldeq = tok2;
                                break;
                            }
                        }
                        if (!oldeq) {
                            const Token *tok2 = it->second;
                            while (Token::Match(tok2, "%name%|.|[|*|("))
                                tok2 = tok2->astParent();
                            if (Token::Match(tok2, "++|--"))
                                oldeq = tok2;
                        }

                        // Ensure that LHS in assignments are the same
                        bool error = oldeq && eq->astOperand1() && isSameExpression(_tokenizer->isCPP(), true, eq->astOperand1(), oldeq->astOperand1(), _settings->library, true);

                        // Ensure that variable is not used on right side
                        std::stack<const Token *> tokens;
                        tokens.push(eq->astOperand2());
                        while (!tokens.empty()) {
                            const Token *rhs = tokens.top();
                            tokens.pop();
                            if (!rhs)
                                continue;
                            tokens.push(rhs->astOperand1());
                            tokens.push(rhs->astOperand2());
                            if (rhs->varId() == tok->varId()) {
                                error = false;
                                break;
                            }
                            if (Token::Match(rhs->previous(), "%name% (") && nonLocalVolatile(tok->variable())) { // Called function might use the variable
                                const Function* const func = rhs->function();
                                const Variable* const var = tok->variable();
                                if (!var || var->isGlobal() || var->isReference() || ((!func || func->nestedIn) && rhs->strAt(-1) != ".")) {// Global variable, or member function
                                    error = false;
                                    break;
                                }
                            }
                        }

                        if (error) {
                            if (printWarning && scope->type == Scope::eSwitch && Token::findmatch(it->second, "default|case", tok))
                                redundantAssignmentInSwitchError(it->second, tok, eq->astOperand1()->expressionString());
                            else if (printStyle) {
                                // c++, unknown type => assignment might have additional side effects
                                const bool possibleSideEffects(_tokenizer->isCPP() && !tok->valueType());

                                // TODO nonlocal variables are not tracked entirely.
                                const bool nonlocal = it->second->variable() && nonLocalVolatile(it->second->variable());

                                // Warnings are inconclusive if there are possible side effects or if variable is not
                                // tracked perfectly.
                                const bool inconclusive = possibleSideEffects | nonlocal;

                                if (printInconclusive || !inconclusive)
                                    if (_tokenizer->isC() || checkExceptionHandling(tok)) // see #6555 to see how exception handling might have an impact
                                        redundantAssignmentError(it->second, tok, eq->astOperand1()->expressionString(), inconclusive);
                            }
                        }
                        it->second = tok;
                    }
                    if (!Token::simpleMatch(tok->tokAt(2), "0 ;") || (tok->variable() && tok->variable()->nameToken() != tok->tokAt(-2)))
                        varAssignments[tok->varId()] = tok;
                    memAssignments.erase(tok->varId());
                    eraseMemberAssignments(tok->varId(), membervars, varAssignments);
                } else if ((tok->next() && tok->next()->tokType() == Token::eIncDecOp) || (tok->previous()->tokType() == Token::eIncDecOp && tok->strAt(1) == ";")) { // Variable incremented/decremented; Prefix-Increment is only suspicious, if its return value is unused
                    varAssignments[tok->varId()] = tok;
                    memAssignments.erase(tok->varId());
                    eraseMemberAssignments(tok->varId(), membervars, varAssignments);
                } else if (!Token::simpleMatch(tok->tokAt(-2), "sizeof (")) { // Other usage of variable
                    if (it != varAssignments.end())
                        varAssignments.erase(it);
                    if (!writtenArgumentsEnd) // Indicates that we are in the first argument of strcpy/memcpy/... function
                        memAssignments.erase(tok->varId());
                }
            } else if (Token::Match(tok, "%name% (") && !_settings->library.isFunctionConst(tok->str(), true)) { // Function call. Global variables might be used. Reset their status
                const bool memfunc = Token::Match(tok, "memcpy|memmove|memset|strcpy|strncpy|sprintf|snprintf|strcat|strncat|wcscpy|wcsncpy|swprintf|wcscat|wcsncat");
                if (tok->varId()) // operator() or function pointer
                    varAssignments.erase(tok->varId());

                if (memfunc && tok->strAt(-1) != "(" && tok->strAt(-1) != "=") {
                    const Token* param1 = tok->tokAt(2);
                    writtenArgumentsEnd = param1->next();
                    if (param1->varId() && param1->strAt(1) == "," && !Token::Match(tok, "strcat|strncat|wcscat|wcsncat") && param1->variable() && param1->variable()->isLocal() && param1->variable()->isArray()) {
                        if (tok->str() == "memset" && initialized.find(param1->varId()) == initialized.end())
                            initialized.insert(param1->varId());
                        else {
                            const std::map<unsigned int, const Token*>::const_iterator it = memAssignments.find(param1->varId());
                            if (it == memAssignments.end())
                                memAssignments[param1->varId()] = tok;
                            else {
                                bool read = false;
                                for (const Token *tok2 = tok->linkAt(1); tok2 != writtenArgumentsEnd; tok2 = tok2->previous()) {
                                    if (tok2->varId() == param1->varId()) {
                                        // TODO: is this a read? maybe it's a write
                                        read = true;
                                        break;
                                    }
                                }
                                if (read) {
                                    memAssignments[param1->varId()] = tok;
                                    continue;
                                }

                                if (printWarning && scope->type == Scope::eSwitch && Token::findmatch(it->second, "default|case", tok))
                                    redundantCopyInSwitchError(it->second, tok, param1->str());
                                else if (printPerformance)
                                    redundantCopyError(it->second, tok, param1->str());
                            }
                        }
                    }
                } else if (scope->type == Scope::eSwitch) { // Avoid false positives if noreturn function is called in switch
                    const Function* const func = tok->function();
                    if (!func || !func->hasBody()) {
                        varAssignments.clear();
                        memAssignments.clear();
                        continue;
                    }
                    const Token* funcEnd = func->functionScope->classEnd;
                    bool noreturn;
                    if (!_tokenizer->IsScopeNoReturn(funcEnd, &noreturn) && !noreturn) {
                        eraseNotLocalArg(varAssignments, symbolDatabase);
                        eraseNotLocalArg(memAssignments, symbolDatabase);
                    } else {
                        varAssignments.clear();
                        memAssignments.clear();
                    }
                } else { // Noreturn functions outside switch don't cause problems
                    eraseNotLocalArg(varAssignments, symbolDatabase);
                    eraseNotLocalArg(memAssignments, symbolDatabase);
                }
            }
        }
    }
}

void CheckOther::redundantCopyError(const Token *tok1, const Token* tok2, const std::string& var)
{
    const std::list<const Token *> callstack = make_container< std::list<const Token *> >() << tok1 << tok2;
    reportError(callstack, Severity::performance, "redundantCopy",
                "Buffer '" + var + "' is being written before its old content has been used.", CWE563, false);
}

void CheckOther::redundantCopyInSwitchError(const Token *tok1, const Token* tok2, const std::string &var)
{
    const std::list<const Token *> callstack = make_container< std::list<const Token *> >() << tok1 << tok2;
    reportError(callstack, Severity::warning, "redundantCopyInSwitch",
                "Buffer '" + var + "' is being written before its old content has been used. 'break;' missing?", CWE563, false);
}

void CheckOther::redundantAssignmentError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive)
{
    const std::list<const Token *> callstack = make_container< std::list<const Token *> >() << tok1 << tok2;
    if (inconclusive)
        reportError(callstack, Severity::style, "redundantAssignment",
                    "Variable '" + var + "' is reassigned a value before the old one has been used if variable is no semaphore variable.\n"
                    "Variable '" + var + "' is reassigned a value before the old one has been used. Make sure that this variable is not used like a semaphore in a threading environment before simplifying this code.", CWE563, true);
    else
        reportError(callstack, Severity::style, "redundantAssignment",
                    "Variable '" + var + "' is reassigned a value before the old one has been used.", CWE563, false);
}

void CheckOther::redundantAssignmentInSwitchError(const Token *tok1, const Token* tok2, const std::string &var)
{
    const std::list<const Token *> callstack = make_container< std::list<const Token *> >() << tok1 << tok2;
    reportError(callstack, Severity::warning, "redundantAssignInSwitch",
                "Variable '" + var + "' is reassigned a value before the old one has been used. 'break;' missing?", CWE563, false);
}


//---------------------------------------------------------------------------
//    switch (x)
//    {
//        case 2:
//            y = a;        // <- this assignment is redundant
//        case 3:
//            y = b;        // <- case 2 falls through and sets y twice
//    }
//---------------------------------------------------------------------------
static inline bool isFunctionOrBreakPattern(const Token *tok)
{
    if (Token::Match(tok, "%name% (") || Token::Match(tok, "break|continue|return|exit|goto|throw"))
        return true;

    return false;
}

void CheckOther::checkRedundantAssignmentInSwitch()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Find the beginning of a switch. E.g.:
    //   switch (var) { ...
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eSwitch || !i->classStart)
            continue;

        // Check the contents of the switch statement
        std::map<unsigned int, const Token*> varsWithBitsSet;
        std::map<unsigned int, std::string> bitOperations;

        for (const Token *tok2 = i->classStart->next(); tok2 != i->classEnd; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                // Inside a conditional or loop. Don't mark variable accesses as being redundant. E.g.:
                //   case 3: b = 1;
                //   case 4: if (a) { b = 2; }    // Doesn't make the b=1 redundant because it's conditional
                if (Token::Match(tok2->previous(), ")|else {") && tok2->link()) {
                    const Token* endOfConditional = tok2->link();
                    for (const Token* tok3 = tok2; tok3 != endOfConditional; tok3 = tok3->next()) {
                        if (tok3->varId() != 0) {
                            varsWithBitsSet.erase(tok3->varId());
                            bitOperations.erase(tok3->varId());
                        } else if (isFunctionOrBreakPattern(tok3)) {
                            varsWithBitsSet.clear();
                            bitOperations.clear();
                        }
                    }
                    tok2 = endOfConditional;
                }
            }

            // Variable assignment. Report an error if it's assigned to twice before a break. E.g.:
            //    case 3: b = 1;    // <== redundant
            //    case 4: b = 2;

            if (Token::Match(tok2->previous(), ";|{|}|: %var% = %any% ;")) {
                varsWithBitsSet.erase(tok2->varId());
                bitOperations.erase(tok2->varId());
            }

            // Bitwise operation. Report an error if it's performed twice before a break. E.g.:
            //    case 3: b |= 1;    // <== redundant
            //    case 4: b |= 1;
            else if (Token::Match(tok2->previous(), ";|{|}|: %var% %assign% %num% ;") &&
                     (tok2->strAt(1) == "|=" || tok2->strAt(1) == "&=") &&
                     Token::Match(tok2->next()->astOperand2(), "%num%")) {
                std::string bitOp = tok2->strAt(1)[0] + tok2->strAt(2);
                std::map<unsigned int, const Token*>::const_iterator i2 = varsWithBitsSet.find(tok2->varId());

                // This variable has not had a bit operation performed on it yet, so just make a note of it
                if (i2 == varsWithBitsSet.end()) {
                    varsWithBitsSet[tok2->varId()] = tok2;
                    bitOperations[tok2->varId()] = bitOp;
                }

                // The same bit operation has been performed on the same variable twice, so report an error
                else if (bitOperations[tok2->varId()] == bitOp)
                    redundantBitwiseOperationInSwitchError(i2->second, i2->second->str());

                // A different bit operation was performed on the variable, so clear it
                else {
                    varsWithBitsSet.erase(tok2->varId());
                    bitOperations.erase(tok2->varId());
                }
            }

            // Bitwise operation. Report an error if it's performed twice before a break. E.g.:
            //    case 3: b = b | 1;    // <== redundant
            //    case 4: b = b | 1;
            else if (Token::Match(tok2->previous(), ";|{|}|: %var% = %name% %or%|& %num% ;") &&
                     tok2->varId() == tok2->tokAt(2)->varId()) {
                std::string bitOp = tok2->strAt(3) + tok2->strAt(4);
                std::map<unsigned int, const Token*>::const_iterator i2 = varsWithBitsSet.find(tok2->varId());

                // This variable has not had a bit operation performed on it yet, so just make a note of it
                if (i2 == varsWithBitsSet.end()) {
                    varsWithBitsSet[tok2->varId()] = tok2;
                    bitOperations[tok2->varId()] = bitOp;
                }

                // The same bit operation has been performed on the same variable twice, so report an error
                else if (bitOperations[tok2->varId()] == bitOp)
                    redundantBitwiseOperationInSwitchError(i2->second, i2->second->str());

                // A different bit operation was performed on the variable, so clear it
                else {
                    varsWithBitsSet.erase(tok2->varId());
                    bitOperations.erase(tok2->varId());
                }
            }

            // Not a simple assignment so there may be good reason if this variable is assigned to twice. E.g.:
            //    case 3: b = 1;
            //    case 4: b++;
            else if (tok2->varId() != 0 && tok2->strAt(1) != "|" && tok2->strAt(1) != "&") {
                varsWithBitsSet.erase(tok2->varId());
                bitOperations.erase(tok2->varId());
            }

            // Reset our record of assignments if there is a break or function call. E.g.:
            //    case 3: b = 1; break;
            if (isFunctionOrBreakPattern(tok2)) {
                varsWithBitsSet.clear();
                bitOperations.clear();
            }
        }
    }
}

void CheckOther::redundantBitwiseOperationInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "redundantBitwiseOperationInSwitch", "Redundant bitwise operation on '" + varname + "' in 'switch' statement. 'break;' missing?");
}


//---------------------------------------------------------------------------
// Check for statements like case A||B: in switch()
//---------------------------------------------------------------------------
void CheckOther::checkSuspiciousCaseInSwitch()
{
    if (!_settings->inconclusive || !_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eSwitch)
            continue;

        for (const Token* tok = i->classStart->next(); tok != i->classEnd; tok = tok->next()) {
            if (tok->str() == "case") {
                const Token* finding = nullptr;
                for (const Token* tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == ":")
                        break;
                    if (Token::Match(tok2, "[;}{]"))
                        break;

                    if (tok2->str() == "?")
                        finding = nullptr;
                    else if (Token::Match(tok2, "&&|%oror%"))
                        finding = tok2;
                }
                if (finding)
                    suspiciousCaseInSwitchError(finding, finding->str());
            }
        }
    }
}

void CheckOther::suspiciousCaseInSwitchError(const Token* tok, const std::string& operatorString)
{
    reportError(tok, Severity::warning, "suspiciousCase",
                "Found suspicious case label in switch(). Operator '" + operatorString + "' probably doesn't work as intended.\n"
                "Using an operator like '" + operatorString + "' in a case label is suspicious. Did you intend to use a bitwise operator, multiple case labels or if/else instead?", CWE398, true);
}

//---------------------------------------------------------------------------
//    if (x == 1)
//        x == 0;       // <- suspicious equality comparison.
//---------------------------------------------------------------------------
void CheckOther::checkSuspiciousEqualityComparison()
{
    if (!_settings->isEnabled("warning") || !_settings->inconclusive)
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (Token::simpleMatch(tok, "for (")) {
                const Token* const openParen = tok->next();
                const Token* const closeParen = tok->linkAt(1);

                // Search for any suspicious equality comparison in the initialization
                // or increment-decrement parts of the for() loop.
                // For example:
                //    for (i == 2; i < 10; i++)
                // or
                //    for (i = 0; i < 10; i == a)
                if (Token::Match(openParen->next(), "%name% =="))
                    suspiciousEqualityComparisonError(openParen->tokAt(2));
                if (closeParen->strAt(-2) == "==")
                    suspiciousEqualityComparisonError(closeParen->tokAt(-2));

                // Skip over for() loop conditions because "for (;running==1;)"
                // is a bit strange, but not necessarily incorrect.
                tok = closeParen;
            } else if (Token::Match(tok, "[;{}] *| %name% == %any% ;")) {

                // Exclude compound statements surrounded by parentheses, such as
                //    printf("%i\n", ({x==0;}));
                // because they may appear as an expression in GNU C/C++.
                // See http://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html
                const Token* afterStatement = tok->strAt(1) == "*" ? tok->tokAt(6) : tok->tokAt(5);
                if (!Token::simpleMatch(afterStatement, "} )"))
                    suspiciousEqualityComparisonError(tok->next());
            }
        }
    }
}

void CheckOther::suspiciousEqualityComparisonError(const Token* tok)
{
    reportError(tok, Severity::warning, "suspiciousEqualityComparison",
                "Found suspicious equality comparison. Did you intend to assign a value instead?", CWE482, true);
}


//---------------------------------------------------------------------------
//    Find consecutive return, break, continue, goto or throw statements. e.g.:
//        break; break;
//    Detect dead code, that follows such a statement. e.g.:
//        return(0); foo();
//---------------------------------------------------------------------------
void CheckOther::checkUnreachableCode()
{
    if (!_settings->isEnabled("style"))
        return;
    const bool printInconclusive = _settings->inconclusive;
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        for (const Token* tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            const Token* secondBreak = 0;
            const Token* labelName = 0;
            if (tok->link() && Token::Match(tok, "(|[|<"))
                tok = tok->link();
            else if (Token::Match(tok, "break|continue ;"))
                secondBreak = tok->tokAt(2);
            else if (Token::Match(tok, "[;{}:] return|throw")) {
                tok = tok->next(); // tok should point to return or throw
                for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(" || tok2->str() == "{")
                        tok2 = tok2->link();
                    if (tok2->str() == ";") {
                        secondBreak = tok2->next();
                        break;
                    }
                }
            } else if (Token::Match(tok, "goto %any% ;")) {
                secondBreak = tok->tokAt(3);
                labelName = tok->next();
            } else if (Token::Match(tok, "%name% (") && _settings->library.isnoreturn(tok) && !Token::Match(tok->next()->astParent(), "?|:")) {
                if ((!tok->function() || (tok->function()->token != tok && tok->function()->tokenDef != tok)) && tok->linkAt(1)->strAt(1) != "{")
                    secondBreak = tok->linkAt(1)->tokAt(2);
            }

            // Statements follow directly, no line between them. (#3383)
            // TODO: Try to find a better way to avoid false positives due to preprocessor configurations.
            const bool inconclusive = secondBreak && (secondBreak->linenr() - 1 > secondBreak->previous()->linenr());

            if (secondBreak && (printInconclusive || !inconclusive)) {
                if (Token::Match(secondBreak, "continue|goto|throw") ||
                    (secondBreak->str() == "return" && (tok->str() == "return" || secondBreak->strAt(1) == ";"))) { // return with value after statements like throw can be necessary to make a function compile
                    duplicateBreakError(secondBreak, inconclusive);
                    tok = Token::findmatch(secondBreak, "[}:]");
                } else if (secondBreak->str() == "break") { // break inside switch as second break statement should not issue a warning
                    if (tok->str() == "break") // If the previous was a break, too: Issue warning
                        duplicateBreakError(secondBreak, inconclusive);
                    else {
                        if (tok->scope()->type != Scope::eSwitch) // Check, if the enclosing scope is a switch
                            duplicateBreakError(secondBreak, inconclusive);
                    }
                    tok = Token::findmatch(secondBreak, "[}:]");
                } else if (!Token::Match(secondBreak, "return|}|case|default") && secondBreak->strAt(1) != ":") { // TODO: No bailout for unconditional scopes
                    // If the goto label is followed by a loop construct in which the label is defined it's quite likely
                    // that the goto jump was intended to skip some code on the first loop iteration.
                    bool labelInFollowingLoop = false;
                    if (labelName && Token::Match(secondBreak, "while|do|for")) {
                        const Token *scope2 = Token::findsimplematch(secondBreak, "{");
                        if (scope2) {
                            for (const Token *tokIter = scope2; tokIter != scope2->link() && tokIter; tokIter = tokIter->next()) {
                                if (Token::Match(tokIter, "[;{}] %any% :") && labelName->str() == tokIter->strAt(1)) {
                                    labelInFollowingLoop = true;
                                    break;
                                }
                            }
                        }
                    }

                    // hide FP for statements that just hide compiler warnings about unused function arguments
                    bool silencedCompilerWarningOnly = false;
                    const Token *silencedWarning = secondBreak;
                    for (;;) {
                        if (Token::Match(silencedWarning, "( void ) %name% ;")) {
                            silencedWarning = silencedWarning->tokAt(5);
                            continue;
                        } else if (silencedWarning && silencedWarning == scope->classEnd)
                            silencedCompilerWarningOnly = true;

                        break;
                    }
                    if (silencedWarning)
                        secondBreak = silencedWarning;

                    if (!labelInFollowingLoop && !silencedCompilerWarningOnly)
                        unreachableCodeError(secondBreak, inconclusive);
                    tok = Token::findmatch(secondBreak, "[}:]");
                } else
                    tok = secondBreak;

                if (!tok)
                    break;
                tok = tok->previous(); // Will be advanced again by for loop
            }
        }
    }
}

void CheckOther::duplicateBreakError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "duplicateBreak",
                "Consecutive return, break, continue, goto or throw statements are unnecessary.\n"
                "Consecutive return, break, continue, goto or throw statements are unnecessary. "
                "The second statement can never be executed, and so should be removed.", CWE561, inconclusive);
}

void CheckOther::unreachableCodeError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "unreachableCode",
                "Statements following return, break, continue, goto or throw will never be executed.", CWE561, inconclusive);
}

//---------------------------------------------------------------------------
// memset(p, y, 0 /* bytes to fill */) <- 2nd and 3rd arguments inverted
//---------------------------------------------------------------------------
void CheckOther::checkMemsetZeroBytes()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|wmemset (") && (numberOfArguments(tok)==3)) {
                const Token* lastParamTok = tok->next()->link()->previous();
                if (lastParamTok->str() == "0")
                    memsetZeroBytesError(tok);
            }
        }
    }
}

void CheckOther::memsetZeroBytesError(const Token *tok)
{
    const std::string summary("memset() called to fill 0 bytes.");
    const std::string verbose(summary + " The second and third arguments might be inverted."
                              " The function memset ( void * ptr, int value, size_t num ) sets the"
                              " first num bytes of the block of memory pointed by ptr to the specified value.");
    reportError(tok, Severity::warning, "memsetZeroBytes", summary + "\n" + verbose, CWE687, false);
}

void CheckOther::checkMemsetInvalid2ndParam()
{
    const bool printPortability = _settings->isEnabled("portability");
    const bool printWarning = _settings->isEnabled("warning");
    if (!printWarning && !printPortability)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok && (tok != scope->classEnd); tok = tok->next()) {
            if (Token::simpleMatch(tok, "memset (")) {
                const Token* firstParamTok = tok->tokAt(2);
                if (!firstParamTok)
                    continue;
                const Token* secondParamTok = firstParamTok->nextArgument();
                if (!secondParamTok)
                    continue;

                // Second parameter is zero literal, i.e. 0.0f
                if (Token::Match(secondParamTok, "%num% ,") && MathLib::isNullValue(secondParamTok->str()))
                    continue;

                const Token *top = secondParamTok;
                while (top->astParent() && top->astParent()->str() != ",")
                    top = top->astParent();

                // Check if second parameter is a float variable or a float literal != 0.0f
                if (printPortability && astIsFloat(top,false)) {
                    memsetFloatError(secondParamTok, top->expressionString());
                } else if (printWarning && secondParamTok->isNumber()) { // Check if the second parameter is a literal and is out of range
                    const long long int value = MathLib::toLongNumber(secondParamTok->str());
                    if (value < -128 || value > 255)
                        memsetValueOutOfRangeError(secondParamTok, secondParamTok->str());
                }
            }
        }
    }
}

void CheckOther::memsetFloatError(const Token *tok, const std::string &var_value)
{
    const std::string message("The 2nd memset() argument '" + var_value +
                              "' is a float, its representation is implementation defined.");
    const std::string verbose(message + " memset() is used to set each byte of a block of memory to a specific value and"
                              " the actual representation of a floating-point value is implementation defined.");
    reportError(tok, Severity::portability, "memsetFloat", message + "\n" + verbose, CWE688, false);
}

void CheckOther::memsetValueOutOfRangeError(const Token *tok, const std::string &value)
{
    const std::string message("The 2nd memset() argument '" + value + "' doesn't fit into an 'unsigned char'.");
    const std::string verbose(message + " The 2nd parameter is passed as an 'int', but the function fills the block of memory using the 'unsigned char' conversion of this value.");
    reportError(tok, Severity::warning, "memsetValueOutOfRange", message + "\n" + verbose, CWE686, false);
}

//---------------------------------------------------------------------------
// Check scope of variables..
//---------------------------------------------------------------------------
void CheckOther::checkVariableScope()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (unsigned int i = 1; i < symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || !var->isLocal() || (!var->isPointer() && !var->typeStartToken()->isStandardType()))
            continue;

        if (var->isConst())
            continue;

        bool forHead = false; // Don't check variables declared in header of a for loop
        for (const Token* tok = var->typeStartToken(); tok; tok = tok->previous()) {
            if (tok->str() == "(") {
                forHead = true;
                break;
            } else if (Token::Match(tok, "[;{}]"))
                break;
        }
        if (forHead)
            continue;

        const Token* tok = var->nameToken()->next();
        if (Token::Match(tok, "; %varid% = %any% ;", var->declarationId())) {
            tok = tok->tokAt(3);
            if (!tok->isNumber() && tok->tokType() != Token::eString && tok->tokType() != Token::eChar && !tok->isBoolean())
                continue;
        }
        // bailout if initialized with function call that has possible side effects
        if (Token::Match(tok, "[(=]") && Token::simpleMatch(tok->astOperand2(), "("))
            continue;
        bool reduce = true;
        bool used = false; // Don't warn about unused variables
        for (; tok && tok != var->scope()->classEnd; tok = tok->next()) {
            if (tok->str() == "{" && tok->scope() != tok->previous()->scope() && !tok->isExpandedMacro() && tok->scope()->type != Scope::eLambda) {
                if (used) {
                    bool used2 = false;
                    if (!checkInnerScope(tok, var, used2) || used2) {
                        reduce = false;
                        break;
                    }
                } else if (!checkInnerScope(tok, var, used)) {
                    reduce = false;
                    break;
                }

                tok = tok->link();

                // parse else if blocks..
            } else if (Token::simpleMatch(tok, "else { if (") && Token::simpleMatch(tok->linkAt(3), ") {")) {
                const Token *endif = tok->linkAt(3)->linkAt(1);
                bool elseif = false;
                if (Token::simpleMatch(endif, "} }"))
                    elseif = true;
                else if (Token::simpleMatch(endif, "} else {") && Token::simpleMatch(endif->linkAt(2),"} }"))
                    elseif = true;
                if (elseif && Token::findmatch(tok->next(), "%varid%", tok->linkAt(1), var->declarationId())) {
                    reduce = false;
                    break;
                }
            } else if (tok->varId() == var->declarationId() || tok->str() == "goto") {
                reduce = false;
                break;
            }
        }

        if (reduce && used)
            variableScopeError(var->nameToken(), var->name());
    }
}

bool CheckOther::checkInnerScope(const Token *tok, const Variable* var, bool& used)
{
    const Scope* scope = tok->next()->scope();
    bool loopVariable = scope->type == Scope::eFor || scope->type == Scope::eWhile || scope->type == Scope::eDo;
    bool noContinue = true;
    const Token* forHeadEnd = 0;
    const Token* end = tok->link();
    if (scope->type == Scope::eUnconditional && (tok->strAt(-1) == ")" || tok->previous()->isName())) // Might be an unknown macro like BOOST_FOREACH
        loopVariable = true;

    if (scope->type == Scope::eDo) {
        end = end->linkAt(2);
    } else if (loopVariable && tok->strAt(-1) == ")") {
        tok = tok->linkAt(-1); // Jump to opening ( of for/while statement
    } else if (scope->type == Scope::eSwitch) {
        for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
            if (used) {
                bool used2 = false;
                if (!checkInnerScope((*i)->classStart, var, used2) || used2) {
                    return false;
                }
            } else if (!checkInnerScope((*i)->classStart, var, used)) {
                return false;
            }
        }
    }

    bool bFirstAssignment=false;
    for (; tok && tok != end; tok = tok->next()) {
        if (tok->str() == "goto")
            return false;
        if (tok->str() == "continue")
            noContinue = false;

        if (Token::simpleMatch(tok, "for ("))
            forHeadEnd = tok->linkAt(1);
        if (tok == forHeadEnd)
            forHeadEnd = 0;

        if (loopVariable && noContinue && tok->scope() == scope && !forHeadEnd && scope->type != Scope::eSwitch && Token::Match(tok, "%varid% =", var->declarationId())) { // Assigned in outer scope.
            loopVariable = false;
            unsigned int indent = 0;
            for (const Token* tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) { // Ensure that variable isn't used on right side of =, too
                if (tok2->str() == "(")
                    indent++;
                else if (tok2->str() == ")") {
                    if (indent == 0)
                        break;
                    indent--;
                } else if (tok2->str() == ";")
                    break;
                else if (tok2->varId() == var->declarationId()) {
                    loopVariable = true;
                    break;
                }
            }
        }

        if (loopVariable && Token::Match(tok, "%varid% !!=", var->declarationId())) // Variable used in loop
            return false;

        if (Token::Match(tok, "& %varid%", var->declarationId())) // Taking address of variable
            return false;

        if (Token::Match(tok, "%varid% =", var->declarationId()))
            bFirstAssignment = true;

        if (!bFirstAssignment && Token::Match(tok, "* %varid%", var->declarationId())) // dereferencing means access to previous content
            return false;

        if (Token::Match(tok, "= %varid%", var->declarationId()) && (var->isArray() || var->isPointer())) // Create a copy of array/pointer. Bailout, because the memory it points to might be necessary in outer scope
            return false;

        if (tok->varId() == var->declarationId()) {
            used = true;
            if (scope->type == Scope::eSwitch && scope == tok->scope())
                return false; // Used in outer switch scope - unsafe or impossible to reduce scope
        }
    }

    return true;
}

void CheckOther::variableScopeError(const Token *tok, const std::string &varname)
{
    reportError(tok,
                Severity::style,
                "variableScope",
                "The scope of the variable '" + varname + "' can be reduced.\n"
                "The scope of the variable '" + varname + "' can be reduced. Warning: Be careful "
                "when fixing this message, especially when there are inner loops. Here is an "
                "example where cppcheck will write that the scope for 'i' can be reduced:\n"
                "void f(int x)\n"
                "{\n"
                "    int i = 0;\n"
                "    if (x) {\n"
                "        // it's safe to move 'int i = 0;' here\n"
                "        for (int n = 0; n < 10; ++n) {\n"
                "            // it is possible but not safe to move 'int i = 0;' here\n"
                "            do_something(&i);\n"
                "        }\n"
                "    }\n"
                "}\n"
                "When you see this message it is always safe to reduce the variable scope 1 level.", CWE398, false);
}

//---------------------------------------------------------------------------
// Comma in return statement: return a+1, b++;. (experimental)
//---------------------------------------------------------------------------
void CheckOther::checkCommaSeparatedReturn()
{
    // This is experimental for now. See #5076
    if (!_settings->experimental)
        return;

    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "return") {
            tok = tok->next();
            while (tok && tok->str() != ";") {
                if (tok->link() && Token::Match(tok, "[([{<]"))
                    tok = tok->link();

                if (!tok->isExpandedMacro() && tok->str() == "," && tok->linenr() != tok->next()->linenr())
                    commaSeparatedReturnError(tok);

                tok = tok->next();
            }
            // bailout: missing semicolon (invalid code / bad tokenizer)
            if (!tok)
                break;
        }
    }
}

void CheckOther::commaSeparatedReturnError(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "commaSeparatedReturn",
                "Comma is used in return statement. The comma can easily be misread as a ';'.\n"
                "Comma is used in return statement. When comma is used in a return statement it can "
                "easily be misread as a semicolon. For example in the code below the value "
                "of 'b' is returned if the condition is true, but it is easy to think that 'a+1' is "
                "returned:\n"
                "    if (x)\n"
                "        return a + 1,\n"
                "    b++;\n"
                "However it can be useful to use comma in macros. Cppcheck does not warn when such a "
                "macro is then used in a return statement, it is less likely such code is misunderstood.", CWE398, false);
}

//---------------------------------------------------------------------------
// Check for function parameters that should be passed by reference
//---------------------------------------------------------------------------
static std::size_t estimateSize(const Type* type, const Settings* settings, const SymbolDatabase* symbolDatabase, std::size_t recursionDepth = 0)
{
    if (recursionDepth > 20)
        return 0;

    std::size_t cumulatedSize = 0;
    for (std::list<Variable>::const_iterator i = type->classScope->varlist.cbegin(); i != type->classScope->varlist.cend(); ++i) {
        std::size_t size = 0;
        if (i->isStatic())
            continue;
        if (i->isPointer() || i->isReference())
            size = settings->sizeof_pointer;
        else if (i->type() && i->type()->classScope)
            size = estimateSize(i->type(), settings, symbolDatabase, recursionDepth+1);
        else if (i->isStlStringType() || (i->isStlType() && Token::Match(i->typeStartToken(), "std :: %type% <") && !Token::simpleMatch(i->typeStartToken()->linkAt(3), "> ::")))
            size = 16; // Just guess
        else
            size = symbolDatabase->sizeOfType(i->typeStartToken());

        if (i->isArray())
            cumulatedSize += size*i->dimension(0);
        else
            cumulatedSize += size;
    }
    for (std::vector<Type::BaseInfo>::const_iterator i = type->derivedFrom.cbegin(); i != type->derivedFrom.cend(); ++i)
        if (i->type && i->type->classScope)
            cumulatedSize += estimateSize(i->type, settings, symbolDatabase, recursionDepth+1);
    return cumulatedSize;
}

void CheckOther::checkPassByReference()
{
    if (!_settings->isEnabled("performance") || _tokenizer->isC())
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (unsigned int i = 1; i < symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || !var->isArgument() || !var->isClass() || var->isPointer() || var->isArray() || var->isReference() || var->isEnumType())
            continue;

        if (var->scope() && var->scope()->function->arg->link()->strAt(-1) == ".")
            continue; // references could not be used as va_start parameters (#5824)

        bool inconclusive = false;

        const Token* const tok = var->typeStartToken();
        if (var->isStlStringType()) {
            ;
        } else if (var->isStlType() && Token::Match(tok, "std :: %type% <") && !Token::simpleMatch(tok->linkAt(3), "> ::") && !Token::Match(tok->tokAt(2), "initializer_list|weak_ptr|auto_ptr")) {
            ;
        } else if (var->type() && !var->type()->isEnumType()) { // Check if type is a struct or class.
            // Ensure that it is a large object.
            if (!var->type()->classScope)
                inconclusive = true;
            else if (estimateSize(var->type(), _settings, symbolDatabase) <= 8)
                continue;
        } else
            continue;

        if (inconclusive && !_settings->inconclusive)
            continue;

        bool isConst = var->isConst();
        if (!isConst) {
            // Check if variable could be const
            if (!var->scope() || var->scope()->function->isVirtual())
                continue;

            isConst = true;
            for (const Token* tok2 = var->scope()->classStart; tok2 != var->scope()->classEnd; tok2 = tok2->next()) {
                if (tok2->varId() == var->declarationId()) {
                    const Token* parent = tok2->astParent();
                    if (!parent)
                        ;
                    else if (parent->str() == "<<" || parent->str() == ">>") {
                        if (parent->str() == "<<" && parent->astOperand1() == tok2)
                            isConst = false;
                        else if (parent->str() == ">>" && parent->astOperand2() == tok2)
                            isConst = false;
                    } else if (parent->str() == "," || parent->str() == "(") { // function argument
                        const Token* tok3 = tok2->previous();
                        unsigned int argNr = 0;
                        while (tok3 && tok3->str() != "(") {
                            if (tok3->link() && Token::Match(tok3, ")|]|}|>"))
                                tok3 = tok3->link();
                            else if (tok3->link())
                                break;
                            else if (tok3->str() == ";")
                                break;
                            else if (tok3->str() == ",")
                                argNr++;
                            tok3 = tok3->previous();
                        }
                        if (!tok3 || tok3->str() != "(" || !tok3->astOperand1() || !tok3->astOperand1()->function())
                            isConst = false;
                        else {
                            const Variable* argVar = tok3->astOperand1()->function()->getArgumentVar(argNr);
                            if (!argVar|| (!argVar->isConst() && argVar->isReference()))
                                isConst = false;
                        }
                    } else if (parent->isConstOp())
                        ;
                    else if (parent->isAssignmentOp()) {
                        if (parent->astOperand1() == tok2)
                            isConst = false;
                        else if (parent->astOperand1()->str() == "&") {
                            const Variable* assignedVar = parent->previous()->variable();
                            if (!assignedVar || !assignedVar->isConst())
                                isConst = false;
                        }
                    } else if (Token::Match(tok2, "%var% . %name% (")) {
                        const Function* func = tok2->tokAt(2)->function();
                        if (func && (func->isConst() || func->isStatic()))
                            ;
                        else
                            isConst = false;
                    } else
                        isConst = false;

                    if (!isConst)
                        break;
                }
            }
        }

        if (isConst)
            passedByValueError(tok, var->name(), inconclusive);
    }
}

void CheckOther::passedByValueError(const Token *tok, const std::string &parname, bool inconclusive)
{
    reportError(tok, Severity::performance, "passedByValue",
                "Function parameter '" + parname + "' should be passed by reference.\n"
                "Parameter '" +  parname + "' is passed by value. It could be passed "
                "as a (const) reference which is usually faster and recommended in C++.", CWE398, inconclusive);
}

//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------

void CheckOther::checkCharVariable()
{
    const bool warning = _settings->isEnabled("warning");
    const bool portability = _settings->isEnabled("portability");
    if (!warning && !portability)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% [")) {
                if (!tok->variable())
                    continue;
                if (!tok->variable()->isArray() && !tok->variable()->isPointer())
                    continue;
                const Token *index = tok->next()->astOperand2();
                if (warning && tok->variable()->isArray() && astIsSignedChar(index) && index->getValueGE(0x80, _settings))
                    signedCharArrayIndexError(tok);
                if (portability && astIsUnknownSignChar(index) && index->getValueGE(0x80, _settings))
                    unknownSignCharArrayIndexError(tok);
            } else if (warning && Token::Match(tok, "[&|^]") && tok->astOperand2() && tok->astOperand1()) {
                bool warn = false;
                if (astIsSignedChar(tok->astOperand1())) {
                    const ValueFlow::Value *v1 = tok->astOperand1()->getValueLE(-1, _settings);
                    const ValueFlow::Value *v2 = tok->astOperand2()->getMaxValue(false);
                    if (!v1)
                        v1 = tok->astOperand1()->getValueGE(0x80, _settings);
                    if (v1 && !(tok->str() == "&" && v2 && v2->isKnown() && v2->intvalue >= 0 && v2->intvalue < 0x100))
                        warn = true;
                } else if (!warn && astIsSignedChar(tok->astOperand2())) {
                    const ValueFlow::Value *v1 = tok->astOperand2()->getValueLE(-1, _settings);
                    const ValueFlow::Value *v2 = tok->astOperand1()->getMaxValue(false);
                    if (!v1)
                        v1 = tok->astOperand2()->getValueGE(0x80, _settings);
                    if (v1 && !(tok->str() == "&" && v2 && v2->isKnown() && v2->intvalue >= 0 && v2->intvalue < 0x100))
                        warn = true;
                }

                // is the result stored in a short|int|long?
                if (warn && Token::simpleMatch(tok->astParent(), "=")) {
                    const Token *lhs = tok->astParent()->astOperand1();
                    if (lhs && lhs->valueType() && lhs->valueType()->type >= ValueType::Type::SHORT)
                        charBitOpError(tok); // This is an error..
                }
            }
        }
    }
}

void CheckOther::signedCharArrayIndexError(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "signedCharArrayIndex",
                "Signed 'char' type used as array index.\n"
                "Signed 'char' type used as array index. If the value "
                "can be greater than 127 there will be a buffer underflow "
                "because of sign extension.", CWE128, false);
}

void CheckOther::unknownSignCharArrayIndexError(const Token *tok)
{
    reportError(tok,
                Severity::portability,
                "unknownSignCharArrayIndex",
                "'char' type used as array index.\n"
                "'char' type used as array index. Values greater that 127 will be "
                "treated depending on whether 'char' is signed or unsigned on target platform.", CWE758, false);
}

void CheckOther::charBitOpError(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "charBitOp",
                "When using 'char' variables in bit operations, sign extension can generate unexpected results.\n"
                "When using 'char' variables in bit operations, sign extension can generate unexpected results. For example:\n"
                "    char c = 0x80;\n"
                "    int i = 0 | c;\n"
                "    if (i & 0x8000)\n"
                "        printf(\"not expected\");\n"
                "The \"not expected\" will be printed on the screen.", CWE398, false);
}

//---------------------------------------------------------------------------
// Incomplete statement..
//---------------------------------------------------------------------------
void CheckOther::checkIncompleteStatement()
{
    if (!_settings->isEnabled("warning"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
            if (Token::simpleMatch(tok, ") {") && Token::simpleMatch(tok->next()->link(), "} ;"))
                tok = tok->next()->link();
        }

        else if (Token::simpleMatch(tok, "= {"))
            tok = tok->next()->link();

        // C++11 struct/array/etc initialization in initializer list
        else if (Token::Match(tok->previous(), "%name%|] {") && !Token::findsimplematch(tok,";",tok->link()))
            tok = tok->link();

        // C++11 vector initialization / return { .. }
        else if (Token::Match(tok,"> %name% {") || Token::Match(tok, "[;{}] return {"))
            tok = tok->linkAt(2);

        // C++11 initialize set in initializer list : [,:] std::set<int>{1} [{,]
        else if (Token::simpleMatch(tok,"> {") && tok->link())
            tok = tok->next()->link();

        else if (Token::Match(tok, "[;{}] %str%|%num%")) {
            // No warning if numeric constant is followed by a "." or ","
            if (Token::Match(tok->next(), "%num% [,.]"))
                continue;

            // No warning for [;{}] (void *) 0 ;
            if (Token::Match(tok, "[;{}] 0 ;") && (tok->next()->isCast() || tok->next()->isExpandedMacro()))
                continue;

            // bailout if there is a "? :" in this statement
            bool bailout = false;
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "?") {
                    bailout = true;
                    break;
                } else if (tok2->str() == ";")
                    break;
            }
            if (bailout)
                continue;

            // no warning if this is the last statement in a ({})
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "[;{}]")) {
                    bailout = Token::simpleMatch(tok2, "; } )");
                    break;
                }
            }
            if (bailout)
                continue;

            constStatementError(tok->next(), tok->next()->isNumber() ? "numeric" : "string");
        }
    }
}

void CheckOther::constStatementError(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::warning, "constStatement", "Redundant code: Found a statement that begins with " + type + " constant.", CWE398, false);
}

//---------------------------------------------------------------------------
// Detect division by zero.
//---------------------------------------------------------------------------
void CheckOther::checkZeroDivision()
{
    const bool printWarnings = _settings->isEnabled("warning");
    const bool printInconclusive = _settings->inconclusive;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->astOperand2() || !tok->astOperand1())
            continue;
        if (tok->str() != "%" && tok->str() != "/" && tok->str() != "%=" && tok->str() != "/=")
            continue;
        if (!tok->valueType() || !tok->valueType()->isIntegral())
            continue;
        if (tok->astOperand1()->isNumber()) {
            if (MathLib::isFloat(tok->astOperand1()->str()))
                continue;
        } else if (tok->astOperand1()->isName()) {
            if (!tok->astOperand1()->valueType()->isIntegral())
                continue;
        } else if (!tok->astOperand1()->isArithmeticalOp())
            continue;

        // Value flow..
        const ValueFlow::Value *value = tok->astOperand2()->getValue(0LL);
        if (!value)
            continue;
        if (!printInconclusive && value->inconclusive)
            continue;
        if (value->condition == nullptr)
            zerodivError(tok, value->inconclusive);
        else if (printWarnings)
            zerodivcondError(value->condition,tok,value->inconclusive);
    }
}

void CheckOther::zerodivError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::error, "zerodiv", "Division by zero.", CWE369, inconclusive);
}

void CheckOther::zerodivcondError(const Token *tokcond, const Token *tokdiv, bool inconclusive)
{
    std::list<const Token *> callstack;
    if (tokcond && tokdiv) {
        callstack.push_back(tokcond);
        callstack.push_back(tokdiv);
    }
    const std::string linenr(MathLib::toString(tokdiv ? tokdiv->linenr() : 0));
    reportError(callstack, Severity::warning, "zerodivcond", ValueFlow::eitherTheConditionIsRedundant(tokcond) + " or there is division by zero at line " + linenr + ".", CWE369, inconclusive);
}

//---------------------------------------------------------------------------
// Check for NaN (not-a-number) in an arithmetic expression, e.g.
// double d = 1.0 / 0.0 + 100.0;
//---------------------------------------------------------------------------

void CheckOther::checkNanInArithmeticExpression()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "inf.0 +|-") ||
            Token::Match(tok, "+|- inf.0") ||
            Token::Match(tok, "+|- %num% / 0.0")) {
            nanInArithmeticExpressionError(tok);
        }
    }
}

void CheckOther::nanInArithmeticExpressionError(const Token *tok)
{
    reportError(tok, Severity::style, "nanInArithmeticExpression",
                "Using NaN/Inf in a computation.\n"
                "Using NaN/Inf in a computation. "
                "Although nothing bad really happens, it is suspicious.", CWE369, false);
}

//---------------------------------------------------------------------------
// Creating instance of classes which are destroyed immediately
//---------------------------------------------------------------------------
void CheckOther::checkMisusedScopedObject()
{
    // Skip this check for .c files
    if (_tokenizer->isC())
        return;

    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if ((tok->next()->type() || (tok->next()->function() && tok->next()->function()->isConstructor())) // TODO: The rhs of || should be removed; It is a workaround for a symboldatabase bug
                && Token::Match(tok, "[;{}] %name% (")
                && Token::Match(tok->linkAt(2), ") ; !!}")
                && (!tok->next()->function() || // is not a function on this scope
                    tok->next()->function()->isConstructor())) { // or is function in this scope and it's a ctor
                tok = tok->next();
                misusedScopeObjectError(tok, tok->str());
                tok = tok->next();
            }
        }
    }
}

void CheckOther::misusedScopeObjectError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::style,
                "unusedScopedObject", "Instance of '" + varname + "' object is destroyed immediately.", CWE563, false);
}

//-----------------------------------------------------------------------------
// check for duplicate code in if and else branches
// if (a) { b = true; } else { b = true; }
//-----------------------------------------------------------------------------
void CheckOther::checkDuplicateBranch()
{
    // This is inconclusive since in practice most warnings are noise:
    // * There can be unfixed low-priority todos. The code is fine as it
    //   is but it could be possible to enhance it. Writing a warning
    //   here is noise since the code is fine (see cppcheck, abiword, ..)
    // * There can be overspecified code so some conditions can't be true
    //   and their conditional code is a duplicate of the condition that
    //   is always true just in case it would be false. See for instance
    //   abiword.
    if (!_settings->isEnabled("style") || !_settings->inconclusive)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (scope->type != Scope::eIf)
            continue;

        // check all the code in the function for if (..) else
        if (Token::simpleMatch(scope->classEnd, "} else {")) {
            // Make sure there are no macros (different macros might be expanded
            // to the same code)
            bool macro = false;
            for (const Token *tok = scope->classStart; tok != scope->classEnd->linkAt(2); tok = tok->next()) {
                if (tok->isExpandedMacro()) {
                    macro = true;
                    break;
                }
            }
            if (macro)
                continue;

            // save if branch code
            std::string branch1 = scope->classStart->next()->stringifyList(scope->classEnd);

            if (branch1.empty())
                continue;

            // save else branch code
            std::string branch2 = scope->classEnd->tokAt(3)->stringifyList(scope->classEnd->linkAt(2));

            // check for duplicates
            if (branch1 == branch2)
                duplicateBranchError(scope->classDef, scope->classEnd->next());
        }
    }
}

void CheckOther::duplicateBranchError(const Token *tok1, const Token *tok2)
{
    const std::list<const Token *> toks = make_container< std::list<const Token *> >() << tok2 << tok1;

    reportError(toks, Severity::style, "duplicateBranch", "Found duplicate branches for 'if' and 'else'.\n"
                "Finding the same code in an 'if' and related 'else' branch is suspicious and "
                "might indicate a cut and paste or logic error. Please examine this code "
                "carefully to determine if it is correct.", CWE398, true);
}


//-----------------------------------------------------------------------------
// Check for a free() of an invalid address
// char* p = malloc(100);
// free(p + 10);
//-----------------------------------------------------------------------------
void CheckOther::checkInvalidFree()
{
    std::map<unsigned int, bool> allocatedVariables;

    const bool printInconclusive = _settings->inconclusive;
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {

            // Keep track of which variables were assigned addresses to newly-allocated memory
            if (Token::Match(tok, "%var% = malloc|g_malloc|new")) {
                allocatedVariables.insert(std::make_pair(tok->varId(), false));
            }

            // If a previously-allocated pointer is incremented or decremented, any subsequent
            // free involving pointer arithmetic may or may not be invalid, so we should only
            // report an inconclusive result.
            else if (Token::Match(tok, "%var% = %name% +|-") &&
                     tok->varId() == tok->tokAt(2)->varId() &&
                     allocatedVariables.find(tok->varId()) != allocatedVariables.end()) {
                if (printInconclusive)
                    allocatedVariables[tok->varId()] = true;
                else
                    allocatedVariables.erase(tok->varId());
            }

            // If a previously-allocated pointer is assigned a completely new value,
            // we can't know if any subsequent free() on that pointer is valid or not.
            else if (Token::Match(tok, "%var% =")) {
                allocatedVariables.erase(tok->varId());
            }

            // If a variable that was previously assigned a newly-allocated memory location is
            // added or subtracted from when used to free the memory, report an error.
            else if (Token::Match(tok, "free|g_free|delete ( %any% +|-") ||
                     Token::Match(tok, "delete [ ] ( %any% +|-") ||
                     Token::Match(tok, "delete %any% +|- %any%")) {

                const int varIndex = tok->strAt(1) == "(" ? 2 :
                                     tok->strAt(3) == "(" ? 4 : 1;
                const unsigned int var1 = tok->tokAt(varIndex)->varId();
                const unsigned int var2 = tok->tokAt(varIndex + 2)->varId();
                const std::map<unsigned int, bool>::const_iterator alloc1 = allocatedVariables.find(var1);
                const std::map<unsigned int, bool>::const_iterator alloc2 = allocatedVariables.find(var2);
                if (alloc1 != allocatedVariables.end()) {
                    invalidFreeError(tok, alloc1->second);
                } else if (alloc2 != allocatedVariables.end()) {
                    invalidFreeError(tok, alloc2->second);
                }
            }

            // If the previously-allocated variable is passed in to another function
            // as a parameter, it might be modified, so we shouldn't report an error
            // if it is later used to free memory
            else if (Token::Match(tok, "%name% (") && !_settings->library.isFunctionConst(tok->str(), true)) {
                const Token* tok2 = Token::findmatch(tok->next(), "%var%", tok->linkAt(1));
                while (tok2 != nullptr) {
                    allocatedVariables.erase(tok2->varId());
                    tok2 = Token::findmatch(tok2->next(), "%var%", tok->linkAt(1));
                }
            }
        }
    }
}

void CheckOther::invalidFreeError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::error, "invalidFree", "Invalid memory address freed.", CWE(0U), inconclusive);
}


//---------------------------------------------------------------------------
// check for the same expression on both sides of an operator
// (x == x), (x && x), (x || x)
// (x.y == x.y), (x.y && x.y), (x.y || x.y)
//---------------------------------------------------------------------------

namespace {
    bool notconst(const Function* func)
    {
        return !func->isConst();
    }

    void getConstFunctions(const SymbolDatabase *symbolDatabase, std::list<const Function*> &constFunctions)
    {
        std::list<Scope>::const_iterator scope;
        for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
            std::list<Function>::const_iterator func;
            // only add const functions that do not have a non-const overloaded version
            // since it is pretty much impossible to tell which is being called.
            typedef std::map<std::string, std::list<const Function*> > StringFunctionMap;
            StringFunctionMap functionsByName;
            for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                functionsByName[func->tokenDef->str()].push_back(&*func);
            }
            for (StringFunctionMap::iterator it = functionsByName.begin();
                 it != functionsByName.end(); ++it) {
                std::list<const Function*>::const_iterator nc = std::find_if(it->second.begin(), it->second.end(), notconst);
                if (nc == it->second.end()) {
                    // ok to add all of them
                    constFunctions.splice(constFunctions.end(), it->second);
                }
            }
        }
    }
}

void CheckOther::checkDuplicateExpression()
{
    const bool styleEnabled=_settings->isEnabled("style");
    const bool warningEnabled=_settings->isEnabled("warning");
    if (!styleEnabled && !warningEnabled)
        return;

    // Parse all executing scopes..
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;
    std::list<const Function*> constFunctions;
    getConstFunctions(symbolDatabase, constFunctions);

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (tok->isOp() && tok->astOperand1() && !Token::Match(tok, "+|*|<<|>>|+=|*=|<<=|>>=")) {
                if (Token::Match(tok, "==|!=|-") && astIsFloat(tok->astOperand1(), true))
                    continue;
                if (isSameExpression(_tokenizer->isCPP(), true, tok->astOperand1(), tok->astOperand2(), _settings->library, true)) {
                    if (isWithoutSideEffects(_tokenizer->isCPP(), tok->astOperand1())) {
                        const bool assignment = tok->str() == "=";
                        if (assignment && warningEnabled)
                            selfAssignmentError(tok, tok->astOperand1()->expressionString());
                        else {
                            if (_tokenizer->isCPP() && _settings->standards.cpp==Standards::CPP11 && tok->str() == "==") {
                                const Token* parent = tok->astParent();
                                while (parent && parent->astParent()) {
                                    parent = parent->astParent();
                                }
                                if (parent && parent->previous() && parent->previous()->str() == "static_assert") {
                                    continue;
                                }
                            }
                            if (styleEnabled)
                                duplicateExpressionError(tok, tok, tok->str());
                        }
                    }
                } else if (!Token::Match(tok, "[-/%]")) { // These operators are not associative
                    if (styleEnabled && tok->astOperand2() && tok->str() == tok->astOperand1()->str() && isSameExpression(_tokenizer->isCPP(), true, tok->astOperand2(), tok->astOperand1()->astOperand2(), _settings->library, true) && isWithoutSideEffects(_tokenizer->isCPP(), tok->astOperand2()))
                        duplicateExpressionError(tok->astOperand2(), tok->astOperand2(), tok->str());
                    else if (tok->astOperand2()) {
                        const Token *ast1 = tok->astOperand1();
                        while (ast1 && tok->str() == ast1->str()) {
                            if (isSameExpression(_tokenizer->isCPP(), true, ast1->astOperand1(), tok->astOperand2(), _settings->library, true) && isWithoutSideEffects(_tokenizer->isCPP(), ast1->astOperand1()))
                                // TODO: warn if variables are unchanged. See #5683
                                // Probably the message should be changed to 'duplicate expressions X in condition or something like that'.
                                ;//duplicateExpressionError(ast1->astOperand1(), tok->astOperand2(), tok->str());
                            else if (styleEnabled && isSameExpression(_tokenizer->isCPP(), true, ast1->astOperand2(), tok->astOperand2(), _settings->library, true) && isWithoutSideEffects(_tokenizer->isCPP(), ast1->astOperand2()))
                                duplicateExpressionError(ast1->astOperand2(), tok->astOperand2(), tok->str());
                            if (!isConstExpression(ast1->astOperand2(), _settings->library, true))
                                break;
                            ast1 = ast1->astOperand1();
                        }
                    }
                }
            } else if (styleEnabled && tok->astOperand1() && tok->astOperand2() && tok->str() == ":" && tok->astParent() && tok->astParent()->str() == "?") {
                if (isSameExpression(_tokenizer->isCPP(), true, tok->astOperand1(), tok->astOperand2(), _settings->library, false))
                    duplicateExpressionTernaryError(tok);
            }
        }
    }
}

void CheckOther::duplicateExpressionError(const Token *tok1, const Token *tok2, const std::string &op)
{
    const std::list<const Token *> toks = make_container< std::list<const Token *> >() << tok2 << tok1;

    reportError(toks, Severity::style, "duplicateExpression", "Same expression on both sides of \'" + op + "\'.\n"
                "Finding the same expression on both sides of an operator is suspicious and might "
                "indicate a cut and paste or logic error. Please examine this code carefully to "
                "determine if it is correct.", CWE398, false);
}

void CheckOther::duplicateExpressionTernaryError(const Token *tok)
{
    reportError(tok, Severity::style, "duplicateExpressionTernary", "Same expression in both branches of ternary operator.\n"
                "Finding the same expression in both branches of ternary operator is suspicious as "
                "the same code is executed regardless of the condition.", CWE398, false);
}

void CheckOther::selfAssignmentError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "selfAssignment", "Redundant assignment of '" + varname + "' to itself.", CWE398, false);
}

//-----------------------------------------------------------------------------
// Check is a comparison of two variables leads to condition, which is
// always true or false.
// For instance: int a = 1; if(isless(a,a)){...}
// In this case isless(a,a) always evaluates to false.
//
// Reference:
// - http://www.cplusplus.com/reference/cmath/
//-----------------------------------------------------------------------------
void CheckOther::checkComparisonFunctionIsAlwaysTrueOrFalse()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->isName() && Token::Match(tok, "isgreater|isless|islessgreater|isgreaterequal|islessequal ( %var% , %var% )")) {
                const unsigned int varidLeft = tok->tokAt(2)->varId();// get the left varid
                const unsigned int varidRight = tok->tokAt(4)->varId();// get the right varid
                // compare varids: if they are not zero but equal
                // --> the comparison function is called with the same variables
                if (varidLeft == varidRight) {
                    const std::string& functionName = tok->str(); // store function name
                    const std::string& varNameLeft = tok->strAt(2); // get the left variable name
                    if (functionName == "isgreater" || functionName == "isless" || functionName == "islessgreater") {
                        // e.g.: isgreater(x,x) --> (x)>(x) --> false
                        checkComparisonFunctionIsAlwaysTrueOrFalseError(tok, functionName, varNameLeft, false);
                    } else { // functionName == "isgreaterequal" || functionName == "islessequal"
                        // e.g.: isgreaterequal(x,x) --> (x)>=(x) --> true
                        checkComparisonFunctionIsAlwaysTrueOrFalseError(tok, functionName, varNameLeft, true);
                    }
                }
            }
        }
    }
}
void CheckOther::checkComparisonFunctionIsAlwaysTrueOrFalseError(const Token* tok, const std::string &functionName, const std::string &varName, const bool result)
{
    const std::string strResult = result ? "true" : "false";
    const struct CWE cweResult = result ? CWE571 : CWE570;

    reportError(tok, Severity::warning, "comparisonFunctionIsAlwaysTrueOrFalse",
                "Comparison of two identical variables with " + functionName + "(" + varName + "," + varName + ") always evaluates to " + strResult + ".\n"
                "The function " + functionName + " is designed to compare two variables. Calling this function with one variable (" + varName + ") "
                "for both parameters leads to a statement which is always " + strResult + ".", cweResult, false);
}

//---------------------------------------------------------------------------
// Check testing sign of unsigned variables and pointers.
//---------------------------------------------------------------------------
void CheckOther::checkSignOfUnsignedVariable()
{
    if (!_settings->isEnabled("style"))
        return;

    const bool inconclusive = _tokenizer->codeWithTemplates();
    if (inconclusive && !_settings->inconclusive)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        // check all the code in the function
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isComparisonOp() || !tok->astOperand1() || !tok->astOperand2())
                continue;

            if (Token::Match(tok, "<|<= 0") && tok->next() == tok->astOperand2()) {
                const ValueType* vt = tok->astOperand1()->valueType();
                if (vt && vt->pointer)
                    pointerLessThanZeroError(tok, inconclusive);
                if (vt && vt->sign == ValueType::UNSIGNED)
                    unsignedLessThanZeroError(tok, tok->astOperand1()->expressionString(), inconclusive);
            } else if (Token::Match(tok->previous(), "0 >|>=") && tok->previous() == tok->astOperand1()) {
                const ValueType* vt = tok->astOperand2()->valueType();
                if (vt && vt->pointer)
                    pointerLessThanZeroError(tok, inconclusive);
                if (vt && vt->sign == ValueType::UNSIGNED)
                    unsignedLessThanZeroError(tok, tok->astOperand2()->expressionString(), inconclusive);
            } else if (Token::simpleMatch(tok, ">= 0") && tok->next() == tok->astOperand2()) {
                const ValueType* vt = tok->astOperand1()->valueType();
                if (vt && vt->pointer)
                    pointerPositiveError(tok, inconclusive);
                if (vt && vt->sign == ValueType::UNSIGNED)
                    unsignedPositiveError(tok, tok->astOperand1()->str(), inconclusive);
            } else if (Token::simpleMatch(tok->previous(), "0 <=") && tok->previous() == tok->astOperand1()) {
                const ValueType* vt = tok->astOperand2()->valueType();
                if (vt && vt->pointer)
                    pointerPositiveError(tok, inconclusive);
                if (vt && vt->sign == ValueType::UNSIGNED)
                    unsignedPositiveError(tok, tok->astOperand2()->str(), inconclusive);
            }
        }
    }
}

void CheckOther::unsignedLessThanZeroError(const Token *tok, const std::string &varname, bool inconclusive)
{
    if (inconclusive) {
        reportError(tok, Severity::style, "unsignedLessThanZero",
                    "Checking if unsigned variable '" + varname + "' is less than zero. This might be a false warning.\n"
                    "Checking if unsigned variable '" + varname + "' is less than zero. An unsigned "
                    "variable will never be negative so it is either pointless or an error to check if it is. "
                    "It's not known if the used constant is a template parameter or not and therefore "
                    "this message might be a false warning.", CWE570, true);
    } else {
        reportError(tok, Severity::style, "unsignedLessThanZero",
                    "Checking if unsigned variable '" + varname + "' is less than zero.\n"
                    "The unsigned variable '" + varname + "' will never be negative so it "
                    "is either pointless or an error to check if it is.", CWE570, false);
    }
}

void CheckOther::pointerLessThanZeroError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "pointerLessThanZero",
                "A pointer can not be negative so it is either pointless or an error to check if it is.", CWE570, inconclusive);
}

void CheckOther::unsignedPositiveError(const Token *tok, const std::string &varname, bool inconclusive)
{
    if (inconclusive) {
        reportError(tok, Severity::style, "unsignedPositive",
                    "Unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it.\n"
                    "The unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it. "
                    "It's not known if the used constant is a "
                    "template parameter or not and therefore this message might be a false warning", CWE570, true);
    } else {
        reportError(tok, Severity::style, "unsignedPositive",
                    "Unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it.", CWE570, false);
    }
}

void CheckOther::pointerPositiveError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "pointerPositive",
                "A pointer can not be negative so it is either pointless or an error to check if it is not.", CWE570, inconclusive);
}

/* check if a constructor in given class scope takes a reference */
static bool constructorTakesReference(const Scope * const classScope)
{
    for (std::list<Function>::const_iterator func = classScope->functionList.begin(); func != classScope->functionList.end(); ++func) {
        if (func->isConstructor()) {
            const Function &constructor = *func;
            for (std::size_t argnr = 0U; argnr < constructor.argCount(); argnr++) {
                const Variable * const argVar = constructor.getArgumentVar(argnr);
                if (argVar && argVar->isReference()) {
                    return true;
                }
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------
// This check rule works for checking the "const A a = getA()" usage when getA() returns "const A &" or "A &".
// In most scenarios, "const A & a = getA()" will be more efficient.
//---------------------------------------------------------------------------
void CheckOther::checkRedundantCopy()
{
    if (!_settings->isEnabled("performance") || _tokenizer->isC() || !_settings->inconclusive)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::size_t i = 1; i < symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);

        if (!var || var->isReference() || !var->isConst() || var->isPointer() || (!var->type() && !var->isStlType())) // bailout if var is of standard type, if it is a pointer or non-const
            continue;

        const Token* startTok = var->nameToken();
        if (startTok->strAt(1) == "=") // %type% %name% = ... ;
            ;
        else if (startTok->strAt(1) == "(" && var->isClass() && var->typeScope()) {
            // Object is instantiated. Warn if constructor takes arguments by value.
            if (constructorTakesReference(var->typeScope()))
                continue;
        } else
            continue;

        const Token* tok = startTok->next()->astOperand2();
        if (!tok)
            continue;
        if (!Token::Match(tok->previous(), "%name% ("))
            continue;
        if (!Token::Match(tok->link(), ") )| ;")) // bailout for usage like "const A a = getA()+3"
            continue;

        const Function* func = tok->previous()->function();
        if (func && func->tokenDef->strAt(-1) == "&") {
            redundantCopyError(startTok, startTok->str());
        }
    }
}
void CheckOther::redundantCopyError(const Token *tok,const std::string& varname)
{
    reportError(tok, Severity::performance, "redundantCopyLocalConst",
                "Use const reference for '" + varname + "' to avoid unnecessary data copying.\n"
                "The const variable '"+varname+"' is assigned a copy of the data. You can avoid "
                "the unnecessary data copying by converting '" + varname + "' to const reference.",
                CWE398,
                true); // since #5618 that check became inconclusive
}

//---------------------------------------------------------------------------
// Checking for shift by negative values
//---------------------------------------------------------------------------

static bool isNegative(const Token *tok, const Settings *settings)
{
    return tok->valueType() && tok->valueType()->sign == ValueType::SIGNED && tok->getValueLE(-1LL, settings);
}

void CheckOther::checkNegativeBitwiseShift()
{
    const bool portability = _settings->isEnabled("portability");

    for (const Token* tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        if (!Token::Match(tok, "<<|>>|<<=|>>="))
            continue;

        // don't warn if lhs is a class. this is an overloaded operator then
        if (_tokenizer->isCPP()) {
            const ValueType * lhsType = tok->astOperand1()->valueType();
            if (!lhsType || !lhsType->isIntegral())
                continue;
        }

        // bailout if operation is protected by ?:
        bool ternary = false;
        for (const Token *parent = tok; parent; parent = parent->astParent()) {
            if (Token::Match(parent, "?|:")) {
                ternary = true;
                break;
            }
        }
        if (ternary)
            continue;

        // Get negative rhs value. preferably a value which doesn't have 'condition'.
        if (portability && isNegative(tok->astOperand1(), _settings))
            negativeBitwiseShiftError(tok, 1);
        else if (isNegative(tok->astOperand2(), _settings))
            negativeBitwiseShiftError(tok, 2);
    }
}


void CheckOther::negativeBitwiseShiftError(const Token *tok, int op)
{
    if (op == 1)
        // LHS - this is used by intention in various software, if it
        // is used often in a project and works as expected then this is
        // a portability issue
        reportError(tok, Severity::portability, "shiftNegativeLHS", "Shifting a negative value is technically undefined behaviour", CWE758, false);
    else // RHS
        reportError(tok, Severity::error, "shiftNegative", "Shifting by a negative value is undefined behaviour", CWE758, false);
}

//---------------------------------------------------------------------------
// Check for incompletely filled buffers.
//---------------------------------------------------------------------------
void CheckOther::checkIncompleteArrayFill()
{
    if (!_settings->inconclusive)
        return;
    const bool printWarning = _settings->isEnabled("warning");
    const bool printPortability = _settings->isEnabled("portability");
    if (!printPortability && !printWarning)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|memcpy|memmove ( %var% ,") && Token::Match(tok->linkAt(1)->tokAt(-2), ", %num% )")) {
                const Variable *var = tok->tokAt(2)->variable();
                if (!var || !var->isArray() || var->dimensions().empty() || !var->dimension(0))
                    continue;

                if (MathLib::toLongNumber(tok->linkAt(1)->strAt(-1)) == var->dimension(0)) {
                    unsigned int size = _tokenizer->sizeOfType(var->typeStartToken());
                    if (size == 0 && var->typeStartToken()->next()->str() == "*")
                        size = _settings->sizeof_pointer;
                    if ((size != 1 && size != 100 && size != 0) || var->isPointer()) {
                        if (printWarning)
                            incompleteArrayFillError(tok, var->name(), tok->str(), false);
                    } else if (Token::Match(var->typeStartToken(), "bool|_Bool") && printPortability) // sizeof(bool) is not 1 on all platforms
                        incompleteArrayFillError(tok, var->name(), tok->str(), true);
                }
            }
        }
    }
}

void CheckOther::incompleteArrayFillError(const Token* tok, const std::string& buffer, const std::string& function, bool boolean)
{
    if (boolean)
        reportError(tok, Severity::portability, "incompleteArrayFill",
                    "Array '" + buffer + "' might be filled incompletely. Did you forget to multiply the size given to '" + function + "()' with 'sizeof(*" + buffer + ")'?\n"
                    "The array '" + buffer + "' is filled incompletely. The function '" + function + "()' needs the size given in bytes, but the type 'bool' is larger than 1 on some platforms. Did you forget to multiply the size with 'sizeof(*" + buffer + ")'?", CWE131, true);
    else
        reportError(tok, Severity::warning, "incompleteArrayFill",
                    "Array '" + buffer + "' is filled incompletely. Did you forget to multiply the size given to '" + function + "()' with 'sizeof(*" + buffer + ")'?\n"
                    "The array '" + buffer + "' is filled incompletely. The function '" + function + "()' needs the size given in bytes, but an element of the given array is larger than one byte. Did you forget to multiply the size with 'sizeof(*" + buffer + ")'?", CWE131, true);
}

//---------------------------------------------------------------------------
// Detect NULL being passed to variadic function.
//---------------------------------------------------------------------------

void CheckOther::checkVarFuncNullUB()
{
    if (!_settings->isEnabled("portability"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            // Is NULL passed to a function?
            if (Token::Match(tok,"[(,] NULL [,)]")) {
                // Locate function name in this function call.
                const Token *ftok = tok;
                std::size_t argnr = 1;
                while (ftok && ftok->str() != "(") {
                    if (ftok->str() == ")")
                        ftok = ftok->link();
                    else if (ftok->str() == ",")
                        ++argnr;
                    ftok = ftok->previous();
                }
                ftok = ftok ? ftok->previous() : nullptr;
                if (ftok && ftok->isName()) {
                    // If this is a variadic function then report error
                    const Function *f = ftok->function();
                    if (f && f->argCount() <= argnr) {
                        const Token *tok2 = f->argDef;
                        tok2 = tok2 ? tok2->link() : nullptr; // goto ')'
                        if (tok2 && Token::simpleMatch(tok2->tokAt(-3), ". . ."))
                            varFuncNullUBError(tok);
                    }
                }
            }
        }
    }
}

void CheckOther::varFuncNullUBError(const Token *tok)
{
    reportError(tok,
                Severity::portability,
                "varFuncNullUB",
                "Passing NULL after the last typed argument to a variadic function leads to undefined behaviour.\n"
                "Passing NULL after the last typed argument to a variadic function leads to undefined behaviour.\n"
                "The C99 standard, in section 7.15.1.1, states that if the type used by va_arg() is not compatible with the type of the actual next argument (as promoted according to the default argument promotions), the behavior is undefined.\n"
                "The value of the NULL macro is an implementation-defined null pointer constant (7.17), which can be any integer constant expression with the value 0, or such an expression casted to (void*) (6.3.2.3). This includes values like 0, 0L, or even 0LL.\n"
                "In practice on common architectures, this will cause real crashes if sizeof(int) != sizeof(void*), and NULL is defined to 0 or any other null pointer constant that promotes to int.\n"
                "To reproduce you might be able to use this little code example on 64bit platforms. If the output includes \"ERROR\", the sentinel had only 4 out of 8 bytes initialized to zero and was not detected as the final argument to stop argument processing via va_arg(). Changing the 0 to (void*)0 or 0L will make the \"ERROR\" output go away.\n"
                "#include <stdarg.h>\n"
                "#include <stdio.h>\n"
                "\n"
                "void f(char *s, ...) {\n"
                "    va_list ap;\n"
                "    va_start(ap,s);\n"
                "    for (;;) {\n"
                "        char *p = va_arg(ap,char*);\n"
                "        printf(\"%018p, %s\\n\", p, (long)p & 255 ? p : \"\");\n"
                "        if(!p) break;\n"
                "    }\n"
                "    va_end(ap);\n"
                "}\n"
                "\n"
                "void g() {\n"
                "    char *s2 = \"x\";\n"
                "    char *s3 = \"ERROR\";\n"
                "\n"
                "    // changing 0 to 0L for the 7th argument (which is intended to act as sentinel) makes the error go away on x86_64\n"
                "    f(\"first\", s2, s2, s2, s2, s2, 0, s3, (char*)0);\n"
                "}\n"
                "\n"
                "void h() {\n"
                "    int i;\n"
                "    volatile unsigned char a[1000];\n"
                "    for (i = 0; i<sizeof(a); i++)\n"
                "        a[i] = -1;\n"
                "}\n"
                "\n"
                "int main() {\n"
                "    h();\n"
                "    g();\n"
                "    return 0;\n"
                "}", CWE475, false);
}

void CheckOther::checkRedundantPointerOp()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "&") {
            // bail out for logical AND operator
            if (tok->astOperand2())
                continue;

            // pointer dereference
            const Token *astTok = tok->astOperand1();
            if (!astTok || astTok->str() != "*")
                continue;

            // variable
            const Token *varTok = astTok->astOperand1();
            if (!varTok || varTok->isExpandedMacro() || varTok->varId() == 0)
                continue;

            const Variable *var = symbolDatabase->getVariableFromVarId(varTok->varId());
            if (!var || !var->isPointer())
                continue;

            redundantPointerOpError(tok, var->name(), false);
        }
    }
}

void CheckOther::redundantPointerOpError(const Token* tok, const std::string &varname, bool inconclusive)
{
    reportError(tok, Severity::style, "redundantPointerOp",
                "Redundant pointer operation on '" + varname + "' - it's already a pointer.", CWE398, inconclusive);
}

void CheckOther::checkInterlockedDecrement()
{
    if (!_settings->isWindowsPlatform()) {
        return;
    }

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->isName() && Token::Match(tok, "InterlockedDecrement ( & %name% ) ; if ( %name%|!|0")) {
            const Token* interlockedVarTok = tok->tokAt(3);
            const Token* checkStartTok =  interlockedVarTok->tokAt(5);
            if ((Token::Match(checkStartTok, "0 %comp% %name% )") && checkStartTok->strAt(2) == interlockedVarTok->str()) ||
                (Token::Match(checkStartTok, "! %name% )") && checkStartTok->strAt(1) == interlockedVarTok->str()) ||
                (Token::Match(checkStartTok, "%name% )") && checkStartTok->str() == interlockedVarTok->str()) ||
                (Token::Match(checkStartTok, "%name% %comp% 0 )") && checkStartTok->str() == interlockedVarTok->str())) {
                raceAfterInterlockedDecrementError(checkStartTok);
            }
        } else if (Token::Match(tok, "if ( ::| InterlockedDecrement ( & %name%")) {
            const Token* condEnd = tok->next()->link();
            const Token* funcTok = tok->tokAt(2);
            const Token* firstAccessTok = funcTok->str() == "::" ? funcTok->tokAt(4) : funcTok->tokAt(3);
            if (condEnd && condEnd->next() && condEnd->next()->link()) {
                const Token* ifEndTok = condEnd->next()->link();
                if (Token::Match(ifEndTok, "} return %name%")) {
                    const Token* secondAccessTok = ifEndTok->tokAt(2);
                    if (secondAccessTok->str() == firstAccessTok->str()) {
                        raceAfterInterlockedDecrementError(secondAccessTok);
                    }
                } else if (Token::Match(ifEndTok, "} else { return %name%")) {
                    const Token* secondAccessTok = ifEndTok->tokAt(4);
                    if (secondAccessTok->str() == firstAccessTok->str()) {
                        raceAfterInterlockedDecrementError(secondAccessTok);
                    }
                }
            }
        }
    }
}

void CheckOther::raceAfterInterlockedDecrementError(const Token* tok)
{
    reportError(tok, Severity::error, "raceAfterInterlockedDecrement",
                "Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.", CWE362, false);
}

void CheckOther::checkUnusedLabel()
{
    if (!_settings->isEnabled("style") && !_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (!tok->scope()->isExecutable())
                tok = tok->scope()->classEnd;

            if (Token::Match(tok, "{|}|; %name% :") && tok->strAt(1) != "default") {
                if (!Token::findsimplematch(scope->classStart->next(), ("goto " + tok->strAt(1)).c_str(), scope->classEnd->previous()))
                    unusedLabelError(tok->next(), tok->next()->scope()->type == Scope::eSwitch);
            }
        }
    }
}

void CheckOther::unusedLabelError(const Token* tok, bool inSwitch)
{
    if (inSwitch) {
        if (!tok || _settings->isEnabled("warning"))
            reportError(tok, Severity::warning, "unusedLabelSwitch",
                        "Label '" + (tok ? tok->str() : emptyString) + "' is not used. Should this be a 'case' of the enclosing switch()?", CWE398, false);
    } else {
        if (!tok || _settings->isEnabled("style"))
            reportError(tok, Severity::style, "unusedLabel",
                        "Label '" + (tok ? tok->str() : emptyString) + "' is not used.", CWE398, false);
    }
}


void CheckOther::checkEvaluationOrder()
{
    // This checker is not written according to C++11 sequencing rules
    if (_tokenizer->isCPP() && _settings->standards.cpp >= Standards::CPP11)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * functionScope = symbolDatabase->functionScopes[i];
        for (const Token* tok = functionScope->classStart; tok != functionScope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "++|--") && !tok->isAssignmentOp())
                continue;
            if (!tok->astOperand1())
                continue;
            for (const Token *tok2 = tok;; tok2 = tok2->astParent()) {
                // If ast parent is a sequence point then break
                const Token * const parent = tok2->astParent();
                if (!parent)
                    break;
                if (Token::Match(parent, "%oror%|&&|?|:|;"))
                    break;
                if (parent->str() == ",") {
                    const Token *par = parent;
                    while (Token::simpleMatch(par,","))
                        par = par->astParent();
                    // not function => break
                    if (!(par && par->str() == "(" && par->astOperand2()))
                        break;
                    // control flow (if|while|etc) => break
                    if (Token::simpleMatch(par->link(),") {"))
                        break;
                    // sequence point in function argument: dostuff((1,2),3) => break
                    par = par->next();
                    while (par && (par->previous() != parent))
                        par = par->nextArgument();
                    if (!par)
                        break;
                }
                if (parent->str() == "(" && parent->astOperand2())
                    break;

                // self assignment..
                if (tok2 == tok &&
                    tok->str() == "=" &&
                    parent->str() == "=" &&
                    isSameExpression(_tokenizer->isCPP(), false, tok->astOperand1(), parent->astOperand1(), _settings->library, true)) {
                    if (_settings->isEnabled("warning") &&
                        isSameExpression(_tokenizer->isCPP(), true, tok->astOperand1(), parent->astOperand1(), _settings->library, true))
                        selfAssignmentError(parent, tok->astOperand1()->expressionString());
                    break;
                }

                // Is expression used?
                bool foundError = false;
                std::stack<const Token *> tokens;
                tokens.push((parent->astOperand1() != tok2) ? parent->astOperand1() : parent->astOperand2());
                while (!tokens.empty() && !foundError) {
                    const Token * const tok3 = tokens.top();
                    tokens.pop();
                    if (!tok3)
                        continue;
                    if (tok3->str() == "&" && !tok3->astOperand2())
                        continue; // don't handle address-of for now
                    if (tok3->str() == "(" && Token::simpleMatch(tok3->previous(), "sizeof"))
                        continue; // don't care about sizeof usage
                    tokens.push(tok3->astOperand1());
                    tokens.push(tok3->astOperand2());
                    if (isSameExpression(_tokenizer->isCPP(), false, tok->astOperand1(), tok3, _settings->library, true)) {
                        foundError = true;
                    }
                }

                if (foundError) {
                    unknownEvaluationOrder(parent);
                    break;
                }
            }
        }
    }
}

void CheckOther::unknownEvaluationOrder(const Token* tok)
{
    reportError(tok, Severity::error, "unknownEvaluationOrder",
                "Expression '" + (tok ? tok->expressionString() : std::string("x = x++;")) + "' depends on order of evaluation of side effects", CWE768, false);
}

void CheckOther::checkAccessOfMovedVariable()
{
    if (!_tokenizer->isCPP() || _settings->standards.cpp < Standards::CPP11 || !_settings->isEnabled("warning"))
        return;
    const bool reportInconclusive = _settings->inconclusive;
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        const Token * scopeStart = scope->classStart;
        if (scope->function) {
            const Token * memberInitializationStart = scope->function->constructorMemberInitialization();
            if (memberInitializationStart)
                scopeStart = memberInitializationStart;
        }
        for (const Token* tok = scopeStart->next(); tok != scope->classEnd; tok = tok->next()) {
            const ValueFlow::Value * movedValue = tok->getMovedValue();
            if (!movedValue || movedValue->moveKind == ValueFlow::Value::NonMovedVariable)
                continue;
            if (movedValue->inconclusive && !reportInconclusive)
                continue;

            bool inconclusive = false;
            bool accessOfMoved = false;
            if (tok->strAt(1) == ".") {
                if (tok->next()->originalName() == "->")
                    accessOfMoved = true;
                else
                    inconclusive = true;
            } else {
                bool isVariableChanged = isVariableChangedByFunctionCall(tok, _settings, &inconclusive);
                accessOfMoved = !isVariableChanged;
                if (inconclusive) {
                    accessOfMoved = !isMovedParameterAllowedForInconclusiveFunction(tok);
                    if (accessOfMoved)
                        inconclusive = false;
                }
            }
            if (accessOfMoved || (inconclusive && reportInconclusive))
                accessMovedError(tok, tok->str(), movedValue->moveKind, inconclusive || movedValue->inconclusive);
        }
    }
}

bool CheckOther::isMovedParameterAllowedForInconclusiveFunction(const Token * tok)
{
    if (Token::simpleMatch(tok->tokAt(-4), "std :: move ("))
        return false;
    const Token * tokAtM2 = tok->tokAt(-2);
    if (Token::simpleMatch(tokAtM2, "> (") && tokAtM2->link()) {
        const Token * leftAngle = tokAtM2->link();
        if (Token::simpleMatch(leftAngle->tokAt(-3), "std :: forward <"))
            return false;
    }
    return true;
}

void CheckOther::accessMovedError(const Token *tok, const std::string &varname, ValueFlow::Value::MoveKind moveKind, bool inconclusive)
{
    const char * errorId = nullptr;
    const char * kindString = nullptr;
    switch (moveKind) {
    case ValueFlow::Value::MovedVariable:
        errorId = "accessMoved";
        kindString = "moved";
        break;
    case ValueFlow::Value::ForwardedVariable:
        errorId = "accessForwarded";
        kindString = "forwarded";
        break;
    default:
        return;
    }
    const std::string errmsg(std::string("Access of ") + kindString + " variable " + varname + ".");
    reportError(tok, Severity::warning, errorId, errmsg, CWE672, inconclusive);
}



void CheckOther::checkFuncArgNamesDifferent()
{
    const bool style = _settings->isEnabled("style");
    const bool inconclusive = _settings->inconclusive;
    const bool warning = _settings->isEnabled("warning");

    if (!(warning || (style && inconclusive)))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    // check every function
    for (std::size_t i = 0, end = symbolDatabase->functionScopes.size(); i < end; ++i) {
        const Function * function = symbolDatabase->functionScopes[i]->function;
        // only check functions with arguments
        if (!function || function->argCount() == 0)
            continue;

        // only check functions with separate declarations and definitions
        if (function->argDef == function->arg)
            continue;

        // get the function argument name tokens
        std::vector<const Token *>  declarations(function->argCount());
        std::vector<const Token *>  definitions(function->argCount());
        const Token * decl = function->argDef->next();
        for (std::size_t j = 0; j < function->argCount(); ++j) {
            declarations[j] = nullptr;
            definitions[j] = nullptr;
            // get the definition
            const Variable * variable = function->getArgumentVar(j);
            if (variable) {
                definitions[j] = variable->nameToken();
            }
            // get the declaration (search for first token with varId)
            while (decl && !Token::Match(decl, ",|)|;")) {
                // skip everything after the assignment because
                // it could also have a varId or be the first
                // token with a varId if there is no name token
                if (decl->str() == "=") {
                    decl = decl->nextArgument();
                    break;
                }
                // skip over template
                if (decl->link())
                    decl = decl->link();
                else if (decl->varId())
                    declarations[j] = decl;
                decl = decl->next();
            }
            if (Token::simpleMatch(decl, ","))
                decl = decl->next();
        }
        // check for different argument order
        if (warning) {
            bool order_different = false;
            for (std::size_t j = 0; j < function->argCount(); ++j) {
                if (!declarations[j] || !definitions[j] || declarations[j]->str() == definitions[j]->str())
                    continue;

                for (std::size_t k = 0; k < function->argCount(); ++k) {
                    if (j != k && definitions[k] && declarations[j]->str() == definitions[k]->str()) {
                        order_different = true;
                        break;
                    }
                }
            }
            if (order_different) {
                funcArgOrderDifferent(function->name(), function->argDef->next(), function->arg->next(), declarations, definitions);
                continue;
            }
        }
        // check for different argument names
        if (style && inconclusive) {
            for (std::size_t j = 0; j < function->argCount(); ++j) {
                if (declarations[j] && definitions[j] && declarations[j]->str() != definitions[j]->str())
                    funcArgNamesDifferent(function->name(), j, declarations[j], definitions[j]);
            }
        }
    }
}

void CheckOther::funcArgNamesDifferent(const std::string & functionName, size_t index,
                                       const Token* declaration, const Token* definition)
{
    std::list<const Token *> tokens;
    tokens.push_back(declaration);
    tokens.push_back(definition);
    reportError(tokens, Severity::style, "funcArgNamesDifferent",
                "Function '" + functionName + "' argument " + MathLib::toString(index + 1) + " names different: declaration '" +
                (declaration ? declaration->str() : std::string("A")) + "' definition '" +
                (definition ? definition->str() : std::string("B")) + "'.", CWE628, true);
}

void CheckOther::funcArgOrderDifferent(const std::string & functionName,
                                       const Token* declaration, const Token* definition,
                                       const std::vector<const Token *> & declarations,
                                       const std::vector<const Token *> & definitions)
{
    std::list<const Token *> tokens;
    tokens.push_back(declarations.size() ? declarations[0] ? declarations[0] : declaration : nullptr);
    tokens.push_back(definitions.size() ? definitions[0] ? definitions[0] : definition : nullptr);
    std::string msg = "Function '" + functionName + "' argument order different: declaration '";
    for (std::size_t i = 0; i < declarations.size(); ++i) {
        if (i != 0)
            msg += ", ";
        if (declarations[i])
            msg += declarations[i]->str();
    }
    msg += "' definition '";
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        if (i != 0)
            msg += ", ";
        if (definitions[i])
            msg += definitions[i]->str();
    }
    msg += "'";
    reportError(tokens, Severity::warning, "funcArgOrderDifferent", msg, CWE683, false);
}

