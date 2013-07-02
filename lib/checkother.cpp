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
#include "checkother.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "templatesimplifier.h"

#include <cmath> // fabs()
#include <stack>
#include <algorithm> // find_if()
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckOther instance;
}

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
            if (Token::Match(tok, "%var% = fclose|fflush|fputc|fputs|fscanf|getchar|getc|fgetc|putchar|putc|puts|scanf|sscanf|ungetc (")) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    vars[tok->varId()] = tok->strAt(2);
                }
            } else if (Token::Match(tok, "EOF %comp% ( %var% = fclose|fflush|fputc|fputs|fscanf|getchar|getc|fgetc|putchar|putc|puts|scanf|sscanf|ungetc (")) {
                tok = tok->tokAt(3);
                if (tok && tok->varId()) {
                    const Variable *var = tok->variable();
                    if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                        checkCastIntToCharAndBackError(tok, tok->strAt(2));
                    }
                }
            } else if (Token::Match(tok, "EOF %comp% ( %var% = std :: cin . get (") || Token::Match(tok, "EOF %comp% ( %var% = cin . get (")) {
                tok = tok->tokAt(3);
                if (tok && tok->varId()) {
                    const Variable *var = tok->variable();
                    if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                        checkCastIntToCharAndBackError(tok, "cin.get");
                    }
                }
            } else if (Token::Match(tok, "%var% = std :: cin . get (") || Token::Match(tok, "%var% = cin . get (")) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    vars[tok->varId()] = "cin.get";
                }
            }
            if (Token::Match(tok, "%var% %comp% EOF")) {
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
        "when the file contains a matching character."
    );
}


//---------------------------------------------------------------------------
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
            if (tok->str() == "?") {
                // condition
                const Token *cond = tok->previous();
                if (cond->isName() || cond->isNumber())
                    cond = cond->previous();
                else if (cond->str() == ")")
                    cond = cond->link()->previous();
                else
                    continue;

                if (cond && cond->str() == "!")
                    cond = cond->previous();

                if (!cond)
                    continue;

                // calculation
                if (!cond->isArithmeticalOp())
                    continue;

                const std::string &op = cond->str();
                cond = cond->previous();

                // skip previous multiplications..
                while (cond && cond->previous()) {
                    if ((cond->isName() || cond->isNumber()) && cond->previous()->str() == "*")
                        cond = cond->tokAt(-2);
                    else if (cond->str() == ")")
                        cond = cond->link()->previous();
                    else
                        break;
                }

                if (!cond)
                    continue;

                // first multiplication operand
                if (cond->str() == ")") {
                    clarifyCalculationError(cond, op);
                } else if (cond->isName() || cond->isNumber()) {
                    if (Token::Match(cond->previous(),"return|=|+|-|,|(") || cond->strAt(-1) == op)
                        clarifyCalculationError(cond, op);
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
                "The code '" + calc + "' should be written as either '" + s1 + "' or '" + s2 + "'.");
}

//---------------------------------------------------------------------------
// Clarify condition '(x = a < 0)' into '((x = a) < 0)' or '(x = (a < 0))'
// Clarify condition '(a & b == c)' into '((a & b) == c)' or '(a & (b == c))'
//---------------------------------------------------------------------------
void CheckOther::clarifyCondition()
{
    if (!_settings->isEnabled("style"))
        return;

    const bool isC = _tokenizer->isC();

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "( %var% [=&|^]")) {
                for (const Token *tok2 = tok->tokAt(3); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(" || tok2->str() == "[")
                        tok2 = tok2->link();
                    else if (tok2->type() == Token::eComparisonOp) {
                        // This might be a template
                        if (!isC && tok2->link())
                            break;

                        clarifyConditionError(tok, tok->strAt(2) == "=", false);
                        break;
                    } else if (!tok2->isName() && !tok2->isNumber() && tok2->str() != ".")
                        break;
                }
            }
        }
    }

    // using boolean result in bitwise operation ! x [&|^]
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%comp%|!")) {
                if (tok->link()) // don't write false positives when templates are used
                    continue;

                const Token *tok2 = tok->next();

                // Todo: There are false positives if '(' if encountered. It
                // is assumed there is something like '(char *)&..' and therefore
                // it bails out.
                if (Token::Match(tok2, "(|&"))
                    continue;

                while (tok2 && (tok2->isName() || tok2->isNumber() || Token::Match(tok2,".|(|["))) {
                    if (Token::Match(tok2, "(|["))
                        tok2 = tok2->link();
                    tok2 = tok2->next();
                }

                if (Token::Match(tok2, "[&|^]")) {
                    // don't write false positives when templates are used
                    if (Token::Match(tok2, "&|* ,|>") || Token::simpleMatch(tok2->previous(), "const &"))
                        continue;

                    // #3609 - CWinTraits<WS_CHILD|WS_VISIBLE>::..
                    if (!isC && Token::Match(tok->previous(), "%var% <")) {
                        const Token *tok3 = tok2;
                        while (Token::Match(tok3, "[&|^] %var%"))
                            tok3 = tok3->tokAt(2);
                        if (Token::Match(tok3, ",|>"))
                            continue;
                    }

                    clarifyConditionError(tok,false,true);
                }
            }
        }
    }
}

void CheckOther::clarifyConditionError(const Token *tok, bool assign, bool boolop)
{
    std::string errmsg;

    if (assign)
        errmsg = "Suspicious condition (assignment + comparison); Clarify expression with parentheses.";

    else if (boolop)
        errmsg = "Boolean result is used in bitwise operation. Clarify expression with parentheses.\n"
                 "Suspicious expression. Boolean result is used in bitwise operation. The operator '!' "
                 "and the comparison operators have higher precedence than bitwise operators. "
                 "It is recommended that the expression is clarified with parentheses.";
    else
        errmsg = "Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses.\n"
                 "Suspicious condition. Comparison operators have higher precedence than bitwise operators. "
                 "Please clarify the condition with parentheses.";

    reportError(tok,
                Severity::style,
                "clarifyCondition",
                errmsg);
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
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "* %var%")) {
                const Token *tok2=tok->previous();

                while (tok2 && tok2->str() == "*")
                    tok2=tok2->previous();

                if (Token::Match(tok2, "[{};]")) {
                    tok = tok->tokAt(2);
                    for (;;) {
                        if (tok->str() == "[")
                            tok = tok->link()->next();

                        if (Token::Match(tok, ".|:: %var%")) {
                            if (tok->strAt(2) == "(")
                                tok = tok->linkAt(2)->next();
                            else
                                tok = tok->tokAt(2);
                        } else
                            break;
                    }
                    if (Token::Match(tok, "++|-- [;,]"))
                        clarifyStatementError(tok);
                }
            }
        }
    }
}

void CheckOther::clarifyStatementError(const Token *tok)
{
    reportError(tok, Severity::warning, "clarifyStatement", "Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n"
                "A statement like '*A++;' might not do what you intended. Postfix 'operator++' is executed before 'operator*'. "
                "Thus, the dereference is meaningless. Did you intend to write '(*A)++;'?");
}


void CheckOther::checkSuspiciousSemicolon()
{
    if (!_settings->inconclusive || !_settings->isEnabled("warning"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Look for "if(); {}", "for(); {}" or "while(); {}"
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type == Scope::eIf || i->type == Scope::eElse || i->type == Scope::eElseIf || i->type == Scope::eWhile || i->type == Scope::eFor) {
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
                "Suspicious use of ; at the end of '" + (tok ? tok->str() : std::string()) + "' statement.", true);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::warningOldStylePointerCast()
{
    // Only valid on C++ code
    if (!_settings->isEnabled("style") || !_tokenizer->isCPP())
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Old style pointer casting..
        if (!Token::Match(tok, "( const| %type% * ) (| %var%") &&
            !Token::Match(tok, "( const| %type% * ) (| new"))
            continue;

        if (tok->strAt(1) == "const")
            tok = tok->next();

        if (tok->strAt(4) == "const")
            continue;

        // Is "type" a class?
        const std::string pattern("class|struct " + tok->strAt(1));
        if (Token::findmatch(_tokenizer->tokens(), pattern.c_str(), tok))
            cstyleCastError(tok);
    }
}

void CheckOther::cstyleCastError(const Token *tok)
{
    reportError(tok, Severity::style, "cstyleCast", "C-style pointer casting");
}

//---------------------------------------------------------------------------
// float* f; double* d = (double*)f; <-- Pointer cast to a type with an incompatible binary data representation
//---------------------------------------------------------------------------

static std::string analyzeType(const Token* tok)
{
    if (tok->str() == "double") {
        if (tok->isLong())
            return "long double";
        else
            return "double";
    }
    if (tok->str() == "float")
        return "float";
    if (Token::Match(tok, "int|long|short|char|size_t"))
        return "integer";
    return "";
}

void CheckOther::invalidPointerCast()
{
    if (!_settings->isEnabled("warning") && !_settings->isEnabled("portability"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            const Token* toTok = 0;
            const Token* nextTok = 0;
            // Find cast
            if (Token::Match(tok, "( const| %type% const| * )") ||
                Token::Match(tok, "( const| %type% %type% const| * )")) {
                toTok = tok->next();
                nextTok = tok->link()->next();
                if (nextTok && nextTok->str() == "(")
                    nextTok = nextTok->next();
            } else if (Token::Match(tok, "reinterpret_cast < const| %type% const| * > (") ||
                       Token::Match(tok, "reinterpret_cast < const| %type% %type% const| * > (")) {
                nextTok = tok->tokAt(5);
                while (nextTok->str() != "(")
                    nextTok = nextTok->next();
                nextTok = nextTok->next();
                toTok = tok->tokAt(2);
            }
            if (toTok && toTok->str() == "const")
                toTok = toTok->next();

            if (!nextTok || !toTok || !toTok->isStandardType())
                continue;

            // Find casted variable
            const Variable *var = 0;
            bool allocation = false;
            bool ref = false;
            if (Token::Match(nextTok, "new %type%"))
                allocation = true;
            else if (Token::Match(nextTok, "%var% !!["))
                var = nextTok->variable();
            else if (Token::Match(nextTok, "& %var%") && !Token::Match(nextTok->tokAt(2), "(|[")) {
                var = nextTok->next()->variable();
                ref = true;
            }

            const Token* fromTok = 0;

            if (allocation) {
                fromTok = nextTok->next();
            } else {
                if (!var || (!ref && !var->isPointer() && !var->isArray()) || (ref && (var->isPointer() || var->isArray())))
                    continue;
                fromTok = var->typeStartToken();
            }

            while (Token::Match(fromTok, "static|const"))
                fromTok = fromTok->next();
            if (!fromTok->isStandardType())
                continue;

            std::string fromType = analyzeType(fromTok);
            std::string toType = analyzeType(toTok);
            if (fromType != toType && !fromType.empty() && !toType.empty() && (toType != "integer" || _settings->isEnabled("portability")) && (toTok->str() != "char" || _settings->inconclusive))
                invalidPointerCastError(tok, fromType, toType, toTok->str() == "char");
        }
    }
}

void CheckOther::invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to, bool inconclusive)
{
    if (to == "integer") { // If we cast something to int*, this can be useful to play with its binary data representation
        if (!inconclusive)
            reportError(tok, Severity::portability, "invalidPointerCast", "Casting from " + from + "* to integer* is not portable due to different binary data representations on different platforms.");
        else
            reportError(tok, Severity::portability, "invalidPointerCast", "Casting from " + from + "* to char* is not portable due to different binary data representations on different platforms.", true);
    } else
        reportError(tok, Severity::warning, "invalidPointerCast", "Casting between " + from + "* and " + to + "* which have an incompatible binary data representation.");
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
                    const std::string strDim = MathLib::longToString(dim);
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
                "The variable '" + strVarName + "' is an array of size " + strDim + ", which does not match.");
}

//-----------------------------------------------------------------------------
// check usleep(), which is allowed to be called with in a range of [0,999999]
//
// Reference:
// - http://man7.org/linux/man-pages/man3/usleep.3.html
//-----------------------------------------------------------------------------
void CheckOther::checkSleepTimeInterval()
{
    if (!_settings->standards.posix)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "usleep ( %num% )")) {
                const Token * const numTok = tok->tokAt(2);
                MathLib::bigint value = MathLib::toLongNumber(numTok->str());
                if (value > 999999) { // less than 1 million
                    checkSleepTimeError(numTok, numTok->str());
                }
            }
        }
    }
}

void CheckOther::checkSleepTimeError(const Token *tok, const std::string &strDim)
{
    reportError(tok, Severity::error,
                "tooBigSleepTime", "The argument of usleep must be less than 1000000.\n"
                "The argument of usleep must be less than 1000000, but " + strDim + " is provided.");
}

//---------------------------------------------------------------------------
// Detect redundant assignments: x = 0; x = 4;
//---------------------------------------------------------------------------

static bool nonLocal(const Variable* var)
{
    return !var || (!var->isLocal() && !var->isArgument()) || var->isStatic() || var->isReference();
}

static void eraseNotLocalArg(std::map<unsigned int, const Token*>& container, const SymbolDatabase* symbolDatabase)
{
    for (std::map<unsigned int, const Token*>::iterator i = container.begin(); i != container.end();) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i->first);
        if (!var || nonLocal(var)) {
            container.erase(i++);
            if (i == container.end())
                break;
        } else
            ++i;
    }
}

void CheckOther::checkRedundantAssignment()
{
    bool performance = _settings->isEnabled("performance");
    bool warning = _settings->isEnabled("warning");
    if (!warning && !performance)
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (!scope->isExecutable())
            continue;

        std::map<unsigned int, const Token*> varAssignments;
        std::map<unsigned int, const Token*> memAssignments;
        std::set<unsigned int> initialized;
        const Token* writtenArgumentsEnd = 0;

        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok == writtenArgumentsEnd)
                writtenArgumentsEnd = 0;

            if (tok->str() == "{" && tok->strAt(-1) != "{" && tok->strAt(-1) != "=" && tok->strAt(-4) != "case" && tok->strAt(-3) != "default") { // conditional or non-executable inner scope: Skip it and reset status
                tok = tok->link();
                varAssignments.clear();
                memAssignments.clear();
            } else if (Token::Match(tok, "for|if|while (")) {
                tok = tok->linkAt(1);
            } else if (Token::Match(tok, "break|return|continue|throw|goto")) {
                varAssignments.clear();
                memAssignments.clear();
            } else if (tok->type() == Token::eVariable) {
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

                std::map<unsigned int, const Token*>::iterator it = varAssignments.find(tok->varId());
                if (tok->next()->isAssignmentOp() && Token::Match(tok->previous(), "[;{}]")) { // Assignment
                    if (it != varAssignments.end()) {
                        bool error = true; // Ensure that variable is not used on right side
                        for (const Token* tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                            if (tok2->str() == ";")
                                break;
                            else if (tok2->varId() == tok->varId())
                                error = false;
                            else if (Token::Match(tok2, "%var% (") && nonLocal(tok->variable())) { // Called function might use the variable
                                const Function* func = tok2->function();
                                const Variable* var = tok->variable();
                                if (!var || var->isGlobal() || var->isReference() || ((!func || func->nestedIn) && tok2->strAt(-1) != ".")) // Global variable, or member function
                                    error = false;
                            }
                        }
                        if (error) {
                            if (scope->type == Scope::eSwitch && Token::findmatch(it->second, "default|case", tok) && warning)
                                redundantAssignmentInSwitchError(it->second, tok, tok->str());
                            else if (performance)
                                redundantAssignmentError(it->second, tok, tok->str(), nonLocal(it->second->variable())); // Inconclusive for non-local variables
                        }
                        it->second = tok;
                    }
                    if (!Token::simpleMatch(tok->tokAt(2), "0 ;") || (tok->variable() && tok->variable()->nameToken() != tok->tokAt(-2)))
                        varAssignments[tok->varId()] = tok;
                    memAssignments.erase(tok->varId());
                } else if (tok->next()->type() == Token::eIncDecOp || (tok->previous()->type() == Token::eIncDecOp && tok->strAt(1) == ";")) { // Variable incremented/decremented; Prefix-Increment is only suspicious, if its return value is unused
                    varAssignments[tok->varId()] = tok;
                    memAssignments.erase(tok->varId());
                } else if (!Token::simpleMatch(tok->tokAt(-2), "sizeof (")) { // Other usage of variable
                    if (it != varAssignments.end())
                        varAssignments.erase(it);
                    if (!writtenArgumentsEnd) // Indicates that we are in the first argument of strcpy/memcpy/... function
                        memAssignments.erase(tok->varId());
                }
            } else if (Token::Match(tok, "%var% (")) { // Function call. Global variables might be used. Reset their status
                bool memfunc = Token::Match(tok, "memcpy|memmove|memset|strcpy|strncpy|sprintf|snprintf|strcat|strncat|wcscpy|wcsncpy|swprintf|wcscat|wcsncat");
                if (memfunc) {
                    const Token* param1 = tok->tokAt(2);
                    writtenArgumentsEnd = param1->next();
                    if (param1->varId() && param1->strAt(1) == "," && !Token::Match(tok, "strcat|strncat|wcscat|wcsncat")) {
                        if (tok->str() == "memset" && initialized.find(param1->varId()) == initialized.end() && param1->variable() && param1->variable()->isLocal() && param1->variable()->isArray())
                            initialized.insert(param1->varId());
                        else if (memAssignments.find(param1->varId()) == memAssignments.end())
                            memAssignments[param1->varId()] = tok;
                        else {
                            const std::map<unsigned int, const Token*>::iterator it = memAssignments.find(param1->varId());
                            if (scope->type == Scope::eSwitch && Token::findmatch(it->second, "default|case", tok) && warning)
                                redundantCopyInSwitchError(it->second, tok, param1->str());
                            else if (performance)
                                redundantCopyError(it->second, tok, param1->str());
                        }
                    }
                } else if (scope->type == Scope::eSwitch) { // Avoid false positives if noreturn function is called in switch
                    const Function* func = tok->function();
                    if (!func || !func->hasBody) {
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
    std::list<const Token*> callstack;
    callstack.push_back(tok1);
    callstack.push_back(tok2);
    reportError(callstack, Severity::performance, "redundantCopy",
                "Buffer '" + var + "' is being written before its old content has been used.");
}

void CheckOther::redundantCopyInSwitchError(const Token *tok1, const Token* tok2, const std::string &var)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok1);
    callstack.push_back(tok2);
    reportError(callstack, Severity::warning, "redundantCopyInSwitch",
                "Buffer '" + var + "' is being written before its old content has been used. 'break;' missing?");
}

void CheckOther::redundantAssignmentError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok1);
    callstack.push_back(tok2);
    if (inconclusive)
        reportError(callstack, Severity::performance, "redundantAssignment",
                    "Variable '" + var + "' is reassigned a value before the old one has been used if variable is no semaphore variable.\n"
                    "Variable '" + var + "' is reassigned a value before the old one has been used. Make sure that this variable is not used like a semaphore in a threading environment before simplifying this code.", true);
    else
        reportError(callstack, Severity::performance, "redundantAssignment",
                    "Variable '" + var + "' is reassigned a value before the old one has been used.");
}

void CheckOther::redundantAssignmentInSwitchError(const Token *tok1, const Token* tok2, const std::string &var)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok1);
    callstack.push_back(tok2);
    reportError(callstack, Severity::warning, "redundantAssignInSwitch",
                "Variable '" + var + "' is reassigned a value before the old one has been used. 'break;' missing?");
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
    if (Token::Match(tok, "%var% (") || Token::Match(tok, "break|continue|return|exit|goto|throw"))
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

            if (Token::Match(tok2->previous(), ";|{|}|: %var% = %any% ;") && tok2->varId() != 0) {
                varsWithBitsSet.erase(tok2->varId());
                bitOperations.erase(tok2->varId());
            }

            // Bitwise operation. Report an error if it's performed twice before a break. E.g.:
            //    case 3: b |= 1;    // <== redundant
            //    case 4: b |= 1;
            else if (Token::Match(tok2->previous(), ";|{|}|: %var% = %var% %or%|& %num% ;") &&
                     tok2->varId() != 0 && tok2->varId() == tok2->tokAt(2)->varId()) {
                std::string bitOp = tok2->strAt(3) + tok2->strAt(4);
                std::map<unsigned int, const Token*>::iterator i2 = varsWithBitsSet.find(tok2->varId());

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
//---------------------------------------------------------------------------
void CheckOther::checkSwitchCaseFallThrough()
{
    if (!(_settings->isEnabled("style") && _settings->experimental))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eSwitch || !i->classStart) // Find the beginning of a switch
            continue;

        // Check the contents of the switch statement
        std::stack<std::pair<Token *, bool> > ifnest;
        std::stack<Token *> loopnest;
        std::stack<Token *> scopenest;
        bool justbreak = true;
        bool firstcase = true;
        for (const Token *tok2 = i->classStart; tok2 != i->classEnd; tok2 = tok2->next()) {
            if (Token::simpleMatch(tok2, "if (")) {
                tok2 = tok2->next()->link()->next();
                if (tok2->link() == NULL) {
                    std::ostringstream errmsg;
                    errmsg << "unmatched if in switch: " << tok2->linenr();
                    reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                    break;
                }
                ifnest.push(std::make_pair(tok2->link(), false));
                justbreak = false;
            } else if (Token::simpleMatch(tok2, "while (")) {
                tok2 = tok2->next()->link()->next();
                // skip over "do { } while ( ) ;" case
                if (tok2->str() == "{") {
                    if (tok2->link() == NULL) {
                        std::ostringstream errmsg;
                        errmsg << "unmatched while in switch: " << tok2->linenr();
                        reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                        break;
                    }
                    loopnest.push(tok2->link());
                }
                justbreak = false;
            } else if (Token::simpleMatch(tok2, "do {")) {
                tok2 = tok2->next();
                if (tok2->link() == NULL) {
                    std::ostringstream errmsg;
                    errmsg << "unmatched do in switch: " << tok2->linenr();
                    reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                    break;
                }
                loopnest.push(tok2->link());
                justbreak = false;
            } else if (Token::simpleMatch(tok2, "for (")) {
                tok2 = tok2->next()->link()->next();
                if (tok2->link() == NULL) {
                    std::ostringstream errmsg;
                    errmsg << "unmatched for in switch: " << tok2->linenr();
                    reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                    break;
                }
                loopnest.push(tok2->link());
                justbreak = false;
            } else if (Token::simpleMatch(tok2, "switch (")) {
                // skip over nested switch, we'll come to that soon
                tok2 = tok2->next()->link()->next()->link();
            } else if (Token::Match(tok2, "break|continue|return|exit|goto|throw")) {
                if (loopnest.empty()) {
                    justbreak = true;
                }
                tok2 = Token::findsimplematch(tok2, ";");
            } else if (Token::Match(tok2, "case|default")) {
                if (!justbreak && !firstcase) {
                    switchCaseFallThrough(tok2);
                }
                tok2 = Token::findsimplematch(tok2, ":");
                justbreak = true;
                firstcase = false;
            } else if (tok2->str() == "{") {
                scopenest.push(tok2->link());
            } else if (tok2->str() == "}") {
                if (!ifnest.empty() && tok2 == ifnest.top().first) {
                    if (tok2->next()->str() == "else") {
                        tok2 = tok2->tokAt(2);
                        ifnest.pop();
                        if (tok2->link() == NULL) {
                            std::ostringstream errmsg;
                            errmsg << "unmatched if in switch: " << tok2->linenr();
                            reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                            break;
                        }
                        ifnest.push(std::make_pair(tok2->link(), justbreak));
                        justbreak = false;
                    } else {
                        justbreak &= ifnest.top().second;
                        ifnest.pop();
                    }
                } else if (!loopnest.empty() && tok2 == loopnest.top()) {
                    loopnest.pop();
                } else if (!scopenest.empty() && tok2 == scopenest.top()) {
                    scopenest.pop();
                } else {
                    if (!ifnest.empty() || !loopnest.empty() || !scopenest.empty()) {
                        std::ostringstream errmsg;
                        errmsg << "unexpected end of switch: ";
                        errmsg << "ifnest=" << ifnest.size();
                        if (!ifnest.empty())
                            errmsg << "," << ifnest.top().first->linenr();
                        errmsg << ", loopnest=" << loopnest.size();
                        if (!loopnest.empty())
                            errmsg << "," << loopnest.top()->linenr();
                        errmsg << ", scopenest=" << scopenest.size();
                        if (!scopenest.empty())
                            errmsg << "," << scopenest.top()->linenr();
                        reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                    }
                    // end of switch block
                    break;
                }
            } else if (tok2->str() != ";") {
                justbreak = false;
            }

        }
    }
}

void CheckOther::switchCaseFallThrough(const Token *tok)
{
    reportError(tok, Severity::style,
                "switchCaseFallThrough", "Switch falls through case without comment. 'break;' missing?");
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
                const Token* end = 0;
                for (const Token* tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == ":") {
                        end = tok2;
                        break;
                    }
                    if (Token::Match(tok2, "[?;}{]")) {
                        break;
                    }
                }

                if (end) {
                    const Token* finding = Token::findmatch(tok->next(), "&&|%oror%", end);
                    if (finding)
                        suspiciousCaseInSwitchError(tok, finding->str());
                }
            }
        }
    }
}

void CheckOther::suspiciousCaseInSwitchError(const Token* tok, const std::string& operatorString)
{
    reportError(tok, Severity::warning, "suspiciousCase",
                "Found suspicious case label in switch(). Operator '" + operatorString + "' probably doesn't work as intended.\n"
                "Using an operator like '" + operatorString + "' in a case label is suspicious. Did you intend to use a bitwise operator, multiple case labels or if/else instead?", true);
}

//---------------------------------------------------------------------------
//    if (x == 1)
//        x == 0;       // <- suspicious equality comparison.
//---------------------------------------------------------------------------
void CheckOther::checkSuspiciousEqualityComparison()
{
    if (!_settings->isEnabled("warning") || !_settings->inconclusive)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {

        if (Token::simpleMatch(tok, "for (")) {
            const Token* const openParen = tok->next();
            const Token* const closeParen = tok->linkAt(1);

            // Search for any suspicious equality comparison in the initialization
            // or increment-decrement parts of the for() loop.
            // For example:
            //    for (i == 2; i < 10; i++)
            // or
            //    for (i = 0; i < 10; i == a)
            const Token* tok2 = Token::findmatch(openParen, "[;(] %var% == %any% [;)]", closeParen);
            if (tok2 && (tok2 == openParen || tok2->tokAt(4) == closeParen)) {
                suspiciousEqualityComparisonError(tok2->tokAt(2));
            }

            // Equality comparisons with 0 are simplified to negation. For instance,
            // (x == 0) is simplified to (!x), so also check for suspicious negation
            // in the initialization or increment-decrement parts of the for() loop.
            // For example:
            //    for (!i; i < 10; i++)
            const Token* tok3 = Token::findmatch(openParen, "[;(] ! %var% [;)]", closeParen);
            if (tok3 && (tok3 == openParen || tok3->tokAt(3) == closeParen)) {
                suspiciousEqualityComparisonError(tok3->tokAt(2));
            }

            // Skip over for() loop conditions because "for (;running==1;)"
            // is a bit strange, but not necessarily incorrect.
            tok = closeParen;
        } else if (Token::Match(tok, "[;{}] *| %var% == %any% ;")) {

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

void CheckOther::suspiciousEqualityComparisonError(const Token* tok)
{
    reportError(tok, Severity::warning, "suspiciousEqualityComparison",
                "Found suspicious equality comparison. Did you intend to assign a value instead?", true);
}


//---------------------------------------------------------------------------
//    int x = 1;
//    x = x;            // <- redundant assignment to self
//
//    int y = y;        // <- redundant initialization to self
//---------------------------------------------------------------------------
static bool isTypeWithoutSideEffects(const Tokenizer *tokenizer, const Variable* var)
{
    return ((var && (!var->isClass() || var->isPointer() || Token::simpleMatch(var->typeStartToken(), "std ::"))) || !tokenizer->isCPP());
}

static inline const Token *findSelfAssignPattern(const Token *start)
{
    return Token::findmatch(start, "%var% = %var% ;|=|)");
}

void CheckOther::checkSelfAssignment()
{
    if (!_settings->isEnabled("warning"))
        return;

    const Token *tok = findSelfAssignPattern(_tokenizer->tokens());
    while (tok) {
        if (Token::Match(tok->previous(), "[;{}.]") &&
            tok->varId() && tok->varId() == tok->tokAt(2)->varId() &&
            isTypeWithoutSideEffects(_tokenizer, tok->variable())) {
            bool err = true;

            // no false positive for 'x = x ? x : 1;'
            // it is simplified to 'if (x) { x=x; } else { x=1; }'. The simplification
            // always write all tokens on 1 line (even if the statement is several lines), so
            // check if the linenr is the same for all the tokens.
            if (Token::Match(tok->tokAt(-2), ") { %var% = %var% ; } else { %varid% =", tok->varId())) {
                // Find the 'if' token
                const Token *tokif = tok->linkAt(-2)->previous();

                // find the '}' that terminates the 'else'-block
                const Token *else_end = tok->linkAt(6);

                if (tokif && else_end && tokif->linenr() == else_end->linenr())
                    err = false;
            }

            if (err)
                selfAssignmentError(tok, tok->str());
        }

        tok = findSelfAssignPattern(tok->next());
    }
}

void CheckOther::selfAssignmentError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "selfAssignment", "Redundant assignment of '" + varname + "' to itself.");
}

//---------------------------------------------------------------------------
//    if ((x != 1) || (x != 3))            // expression always true
//    if ((x == 1) && (x == 3))            // expression always false
//    if ((x < 1)  && (x > 3))             // expression always false
//    if ((x > 3)  || (x < 10))            // expression always true
//    if ((x > 5)  && (x != 1))            // second comparison always true
//
//    Check for suspect logic for an expression consisting of 2 comparison
//    expressions with a shared variable and constants and a logical operator
//    between them.
//
//    Suggest a different logical operator when the logical operator between
//    the comparisons is probably wrong.
//
//    Inform that second comparison is always true when first comparison is true.
//---------------------------------------------------------------------------

enum Position { First, Second, NA };
enum Relation { Equal, NotEqual, Less, LessEqual, More, MoreEqual };
struct Condition {
    Position   position;
    const char *opTokStr;
};

static std::string invertOperatorForOperandSwap(std::string s)
{
    for (std::string::size_type i = 0; i < s.length(); i++) {
        if (s[i] == '>')
            s[i] = '<';
        else if (s[i] == '<')
            s[i] = '>';
    }
    return s;
}

static bool analyzeLogicOperatorCondition(const Condition& c1, const Condition& c2,
        bool inv1, bool inv2,
        bool varFirst1, bool varFirst2,
        const std::string& firstConstant, const std::string& secondConstant,
        const Token* op1Tok, const Token* op3Tok,
        Relation relation)
{
    if (!(c1.position == NA || (c1.position == First && varFirst1) || (c1.position == Second && !varFirst1)))
        return false;

    if (!(c2.position == NA || (c2.position == First && varFirst2) || (c2.position == Second && !varFirst2)))
        return false;

    if (!Token::Match(op1Tok, inv1?invertOperatorForOperandSwap(c1.opTokStr).c_str():c1.opTokStr))
        return false;

    if (!Token::Match(op3Tok, inv2?invertOperatorForOperandSwap(c2.opTokStr).c_str():c2.opTokStr))
        return false;

    return (relation == Equal     && MathLib::isEqual(firstConstant, secondConstant)) ||
           (relation == NotEqual  && MathLib::isNotEqual(firstConstant, secondConstant)) ||
           (relation == Less      && MathLib::isLess(firstConstant, secondConstant)) ||
           (relation == LessEqual && MathLib::isLessEqual(firstConstant, secondConstant)) ||
           (relation == More      && MathLib::isGreater(firstConstant, secondConstant)) ||
           (relation == MoreEqual && MathLib::isGreaterEqual(firstConstant, secondConstant));
}

void CheckOther::checkIncorrectLogicOperator()
{
    bool style = _settings->isEnabled("style");
    bool warning = _settings->isEnabled("warning");
    if (!style && !warning)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t ii = 0; ii < functions; ++ii) {
        const Scope * scope = symbolDatabase->functionScopes[ii];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // Find a pair of comparison expressions with or without parentheses
            // with a shared variable and constants and with a logical operator between them.
            // e.g. if (x != 3 || x != 4)
            const Token *term1Tok = NULL, *term2Tok = NULL;
            const Token *op1Tok = NULL, *op2Tok = NULL, *op3Tok = NULL, *nextTok = NULL;

            if (Token::Match(tok, "( %any% %comp% %any% ) &&|%oror%")) {
                term1Tok = tok->next();
                op1Tok = tok->tokAt(2);
                op2Tok = tok->tokAt(5);
            } else if (Token::Match(tok, "%any% %comp% %any% &&|%oror%")) {
                term1Tok = tok;
                op1Tok = tok->next();
                op2Tok = tok->tokAt(3);
            }
            if (op2Tok) {
                if (Token::Match(op2Tok->next(), "( %any% %comp% %any% ) %any%")) {
                    term2Tok = op2Tok->tokAt(2);
                    op3Tok = op2Tok->tokAt(3);
                    nextTok = op2Tok->tokAt(6);
                } else if (Token::Match(op2Tok->next(), "%any% %comp% %any% %any%")) {
                    term2Tok = op2Tok->next();
                    op3Tok = op2Tok->tokAt(2);
                    nextTok = op2Tok->tokAt(4);
                }
            }

            if (nextTok) {
                // Find the common variable and the two different-valued constants
                std::string firstConstant, secondConstant;
                bool varFirst1, varFirst2;
                unsigned int varId;
                const Token *var1Tok = NULL, *var2Tok = NULL;
                if (Token::Match(term1Tok, "%var% %any% %num%")) {
                    var1Tok = term1Tok;
                    varId = var1Tok->varId();
                    if (!varId) {
                        continue;
                    }
                    varFirst1 = true;
                    firstConstant = term1Tok->strAt(2);
                } else if (Token::Match(term1Tok, "%num% %any% %var%")) {
                    var1Tok = term1Tok->tokAt(2);
                    varId = var1Tok->varId();
                    if (!varId) {
                        continue;
                    }
                    varFirst1 = false;
                    firstConstant = term1Tok->str();
                } else {
                    continue;
                }

                if (Token::Match(term2Tok, "%var% %any% %num%")) {
                    var2Tok = term2Tok;
                    varFirst2 = true;
                    secondConstant = term2Tok->strAt(2);
                } else if (Token::Match(term2Tok, "%num% %any% %var%")) {
                    var2Tok = term2Tok->tokAt(2);
                    varFirst2 = false;
                    secondConstant = term2Tok->str();
                } else {
                    continue;
                }

                if (varId != var2Tok->varId() || firstConstant.empty() || secondConstant.empty()) {
                    continue;
                }

                enum LogicError { AlwaysFalse, AlwaysTrue, FirstTrue, FirstFalse, SecondTrue, SecondFalse };

                static const struct LinkedConditions {
                    const char *before;
                    Condition  c1;
                    const char *op2TokStr;
                    Condition  c2;
                    const char *after;
                    Relation   relation;
                    LogicError error;
                } conditions[] = {
                    { "!!&&", { NA,    "!="   }, "%oror%", { NA,    "!="   }, "!!&&", NotEqual,  AlwaysTrue  }, // (x != 1) || (x != 3)  <- always true
                    { 0,      { NA,    "=="   }, "&&",     { NA,    "=="   }, 0,      NotEqual,  AlwaysFalse }, // (x == 1) && (x == 3)  <- always false
                    { "!!&&", { First, ">"    }, "%oror%", { First, "<"    }, "!!&&", Less,      AlwaysTrue  }, // (x > 3)  || (x < 10)  <- always true
                    { "!!&&", { First, ">="   }, "%oror%", { First, "<|<=" }, "!!&&", LessEqual, AlwaysTrue  }, // (x >= 3) || (x < 10)  <- always true
                    { "!!&&", { First, ">"    }, "%oror%", { First, "<="   }, "!!&&", LessEqual, AlwaysTrue  }, // (x > 3)  || (x <= 10) <- always true
                    { 0,      { First, "<"    }, "&&",     { First, ">"    }, 0,      LessEqual, AlwaysFalse }, // (x < 1)  && (x > 3)   <- always false
                    { 0,      { First, "<="   }, "&&",     { First, ">|>=" }, 0,      Less,      AlwaysFalse }, // (x <= 1) && (x > 3)   <- always false
                    { 0,      { First, "<"    }, "&&",     { First, ">="   }, 0,      Less,      AlwaysFalse }, // (x < 1)  && (x >= 3)  <- always false
                    { 0,      { First, ">"    }, "&&",     { NA,    "=="   }, 0,      MoreEqual, AlwaysFalse }, // (x > 5)  && (x == 1)  <- always false
                    { 0,      { First, "<"    }, "&&",     { NA,    "=="   }, 0,      LessEqual, AlwaysFalse }, // (x < 1)  && (x == 3)  <- always false
                    { 0,      { First, ">="   }, "&&",     { NA,    "=="   }, 0,      More,      AlwaysFalse }, // (x >= 5) && (x == 1)  <- always false
                    { 0,      { First, "<="   }, "&&",     { NA,    "=="   }, 0,      Less,      AlwaysFalse }, // (x <= 1) && (x == 3)  <- always false
                    { "!!&&", { NA,    "=="   }, "%oror%", { First, ">"    }, "!!&&", More,      SecondTrue  }, // (x == 4) || (x > 3)   <- second expression always true
                    { "!!&&", { NA,    "=="   }, "%oror%", { First, "<"    }, "!!&&", Less,      SecondTrue  }, // (x == 4) || (x < 5)   <- second expression always true
                    { "!!&&", { NA,    "=="   }, "%oror%", { First, ">="   }, "!!&&", MoreEqual, SecondTrue  }, // (x == 4) || (x >= 3)  <- second expression always true
                    { "!!&&", { NA,    "=="   }, "%oror%", { First, "<="   }, "!!&&", LessEqual, SecondTrue  }, // (x == 4) || (x <= 5)  <- second expression always true
                    { "!!&&", { First, ">"    }, "%oror%", { NA,    "!="   }, "!!&&", MoreEqual, SecondTrue  }, // (x > 5)  || (x != 1)  <- second expression always true
                    { "!!&&", { First, "<"    }, "%oror%", { NA,    "!="   }, "!!&&", LessEqual, SecondTrue  }, // (x < 1)  || (x != 3)  <- second expression always true
                    { "!!&&", { First, ">="   }, "%oror%", { NA,    "!="   }, "!!&&", More,      SecondTrue  }, // (x >= 5) || (x != 1)  <- second expression always true
                    { "!!&&", { First, "<="   }, "%oror%", { NA,    "!="   }, "!!&&", Less,      SecondTrue  }, // (x <= 1) || (x != 3)  <- second expression always true
                    { 0,      { First, ">"    }, "&&",     { NA,    "!="   }, 0,      MoreEqual, SecondTrue  }, // (x > 5)  && (x != 1)  <- second expression always true
                    { 0,      { First, "<"    }, "&&",     { NA,    "!="   }, 0,      LessEqual, SecondTrue  }, // (x < 1)  && (x != 3)  <- second expression always true
                    { 0,      { First, ">="   }, "&&",     { NA,    "!="   }, 0,      More,      SecondTrue  }, // (x >= 5) && (x != 1)  <- second expression always true
                    { 0,      { First, "<="   }, "&&",     { NA,    "!="   }, 0,      Less,      SecondTrue  }, // (x <= 1) && (x != 3)  <- second expression always true
                    { "!!&&", { First, ">|>=" }, "%oror%", { First, ">|>=" }, "!!&&", LessEqual, SecondTrue  }, // (x > 4)  || (x > 5)   <- second expression always true
                    { "!!&&", { First, "<|<=" }, "%oror%", { First, "<|<=" }, "!!&&", MoreEqual, SecondTrue  }, // (x < 5)  || (x < 4)   <- second expression always true
                    { 0,      { First, ">|>=" }, "&&",     { First, ">|>=" }, 0,      MoreEqual, SecondTrue  }, // (x > 4)  && (x > 5)   <- second expression always true
                    { 0,      { First, "<|<=" }, "&&",     { First, "<|<=" }, 0,      MoreEqual, SecondTrue  }, // (x < 5)  && (x < 4)   <- second expression always true
                    { 0,      { NA,    "=="   }, "&&",     { NA,    "!="   }, 0,      NotEqual,  SecondTrue  }, // (x == 3) && (x != 4)  <- second expression always true
                    { "!!&&", { NA,    "=="   }, "%oror%", { NA,    "!="   }, "!!&&", NotEqual,  SecondTrue  }, // (x == 3) || (x != 4)  <- second expression always true
                    { 0,      { NA,    "!="   }, "&&",     { NA,    "=="   }, 0,      Equal,     AlwaysFalse }, // (x != 3) && (x == 3)  <- expression always false
                    { "!!&&", { NA,    "!="   }, "%oror%", { NA,    "=="   }, "!!&&", Equal,     AlwaysTrue  }, // (x != 3) || (x == 3)  <- expression always true
                };

                for (unsigned int i = 0; i < (sizeof(conditions) / sizeof(conditions[0])); i++) {
                    if (!Token::Match(op2Tok, conditions[i].op2TokStr))
                        continue;

                    if (conditions[i].before != 0 && !Token::Match(tok->previous(), conditions[i].before))
                        continue;

                    if (conditions[i].after != 0 && !Token::Match(nextTok, conditions[i].after))
                        continue;

                    if (tok->previous()->isArithmeticalOp() || nextTok->isArithmeticalOp())
                        continue;

                    std::string cond1str = var1Tok->str() + " " + (varFirst1?op1Tok->str():invertOperatorForOperandSwap(op1Tok->str())) + " " + firstConstant;
                    std::string cond2str = var2Tok->str() + " " + (varFirst2?op3Tok->str():invertOperatorForOperandSwap(op3Tok->str())) + " " + secondConstant;
                    // cond1 op cond2
                    bool error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, false, false,
                                 varFirst1, varFirst2, firstConstant, secondConstant,
                                 op1Tok, op3Tok,
                                 conditions[i].relation);
                    // inv(cond1) op cond2 // invert first condition
                    if (!error && conditions[i].c1.position != NA)
                        error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, true, false,
                                                              !varFirst1, varFirst2, firstConstant, secondConstant,
                                                              op1Tok, op3Tok,
                                                              conditions[i].relation);
                    // cond1 op inv(cond2) // invert second condition
                    if (!error && conditions[i].c2.position != NA)
                        error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, false, true,
                                                              varFirst1, !varFirst2, firstConstant, secondConstant,
                                                              op1Tok, op3Tok,
                                                              conditions[i].relation);
                    // inv(cond1) op inv(cond2) // invert both conditions
                    if (!error && conditions[i].c1.position != NA && conditions[i].c2.position != NA)
                        error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, true, true,
                                                              !varFirst1, !varFirst2, firstConstant, secondConstant,
                                                              op1Tok, op3Tok,
                                                              conditions[i].relation);
                    if (!error)
                        std::swap(cond1str, cond2str);
                    // cond2 op cond1 // swap conditions
                    if (!error)
                        error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, false, false,
                                                              varFirst2, varFirst1, secondConstant, firstConstant,
                                                              op3Tok, op1Tok,
                                                              conditions[i].relation);
                    // cond2 op inv(cond1) // swap conditions; invert first condition
                    if (!error && conditions[i].c1.position != NA)
                        error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, true, false,
                                                              !varFirst2, varFirst1, secondConstant, firstConstant,
                                                              op3Tok, op1Tok,
                                                              conditions[i].relation);
                    // inv(cond2) op cond1 // swap conditions; invert second condition
                    if (!error && conditions[i].c2.position != NA)
                        error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, false, true,
                                                              varFirst2, !varFirst1, secondConstant, firstConstant,
                                                              op3Tok, op1Tok,
                                                              conditions[i].relation);
                    // inv(cond2) op inv(cond1) // swap conditions; invert both conditions
                    if (!error && conditions[i].c1.position != NA && conditions[i].c2.position != NA)
                        error = analyzeLogicOperatorCondition(conditions[i].c1, conditions[i].c2, true, true,
                                                              !varFirst2, !varFirst1, secondConstant, firstConstant,
                                                              op3Tok, op1Tok,
                                                              conditions[i].relation);

                    if (error) {
                        if (conditions[i].error == AlwaysFalse || conditions[i].error == AlwaysTrue) {
                            if (warning) {
                                const std::string text = cond1str + " " + op2Tok->str() + " " + cond2str;
                                incorrectLogicOperatorError(term1Tok, text, conditions[i].error == AlwaysTrue);
                            }
                        } else {
                            if (style) {
                                const std::string text = "If " + cond1str + ", the comparison " + cond2str +
                                                         " is always " + ((conditions[i].error == SecondTrue || conditions[i].error == AlwaysTrue) ? "true" : "false") + ".";
                                redundantConditionError(term1Tok, text);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}

void CheckOther::incorrectLogicOperatorError(const Token *tok, const std::string &condition, bool always)
{
    if (always)
        reportError(tok, Severity::warning, "incorrectLogicOperator",
                    "Logical disjunction always evaluates to true: " + condition + ".\n"
                    "Logical disjunction always evaluates to true: " + condition + ". "
                    "Are these conditions necessary? Did you intend to use && instead? Are the numbers correct? Are you comparing the correct variables?");
    else
        reportError(tok, Severity::warning, "incorrectLogicOperator",
                    "Logical conjunction always evaluates to false: " + condition + ".\n"
                    "Logical conjunction always evaluates to false: " + condition + ". "
                    "Are these conditions necessary? Did you intend to use || instead? Are the numbers correct? Are you comparing the correct variables?");
}

void CheckOther::redundantConditionError(const Token *tok, const std::string &text)
{
    reportError(tok, Severity::style, "redundantCondition", "Redundant condition: " + text);
}

//---------------------------------------------------------------------------
// strtol(str, 0, radix)  <- radix must be 0 or 2-36
//---------------------------------------------------------------------------
void CheckOther::invalidFunctionUsage()
{
    // strtol and strtoul..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "strtol|strtoul|strtoll|strtoull|wcstol|wcstoul|wcstoll|wcstoull ("))
            continue;

        const std::string& funcname = tok->str();
        tok = tok->tokAt(2);
        // Locate the third parameter of the function call..
        for (int i = 0; i < 2 && tok; i++)
            tok = tok->nextArgument();

        if (Token::Match(tok, "%num% )")) {
            const MathLib::bigint radix = MathLib::toLongNumber(tok->str());
            if (!(radix == 0 || (radix >= 2 && radix <= 36))) {
                dangerousUsageStrtolError(tok, funcname);
            }
        } else
            break;
    }

    // sprintf|snprintf overlapping data
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Get variable id of target buffer..
        unsigned int varid = 0;

        if (Token::Match(tok, "sprintf|snprintf|swprintf ( %var% ,"))
            varid = tok->tokAt(2)->varId();

        else if (Token::Match(tok, "sprintf|snprintf|swprintf ( %var% . %var% ,"))
            varid = tok->tokAt(4)->varId();

        if (varid == 0)
            continue;

        // goto ","
        const Token *tok2 = tok->tokAt(3);
        while (tok2->str() != ",")
            tok2 = tok2->next();

        tok2 = tok2->next(); // Jump behind ","

        if (tok->str() == "snprintf" || tok->str() == "swprintf") { // Jump over second parameter for snprintf and swprintf
            tok2 = tok2->nextArgument();
            if (!tok2)
                continue;
        }

        // is any source buffer overlapping the target buffer?
        do {
            if (Token::Match(tok2, "%varid% [,)]", varid)) {
                sprintfOverlappingDataError(tok2, tok2->str());
                break;
            }
        } while (NULL != (tok2 = tok2->nextArgument()));
    }
}

void CheckOther::dangerousUsageStrtolError(const Token *tok, const std::string& funcname)
{
    reportError(tok, Severity::error, "dangerousUsageStrtol", "Invalid radix in call to " + funcname + "(). It must be 0 or 2-36.");
}

void CheckOther::sprintfOverlappingDataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "sprintfOverlappingData",
                "Undefined behavior: Variable '" + varname + "' is used as parameter and destination in s[n]printf().\n"
                "The variable '" + varname + "' is used both as a parameter and as destination in "
                "s[n]printf(). The origin and destination buffers overlap. Quote from glibc (C-library) "
                "documentation (http://www.gnu.org/software/libc/manual/html_mono/libc.html#Formatted-Output-Functions): "
                "\"If copying takes place between objects that overlap as a result of a call "
                "to sprintf() or snprintf(), the results are undefined.\"");
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

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        const Token* secondBreak = 0;
        const Token* labelName = 0;
        if (tok->str() == "(")
            tok = tok->link();
        else if (Token::Match(tok, "break|continue ;"))
            secondBreak = tok->tokAt(2);
        else if (Token::Match(tok, "[;{}:] return|throw")) {
            tok = tok->next(); // tok should point to return or throw
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                if (tok2->str() == ";") {
                    secondBreak = tok2->next();
                    break;
                }
            }
        } else if (Token::Match(tok, "goto %any% ;")) {
            secondBreak = tok->tokAt(3);
            labelName = tok->next();
        }

        // Statements follow directly, no line between them. (#3383)
        // TODO: Try to find a better way to avoid false positives due to preprocessor configurations.
        bool inconclusive = secondBreak && (secondBreak->linenr()-1 > secondBreak->previous()->linenr());

        if (secondBreak && (_settings->inconclusive || !inconclusive)) {
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
                    const Token *scope = Token::findsimplematch(secondBreak, "{");
                    if (scope) {
                        for (const Token *tokIter = scope; tokIter != scope->link() && tokIter; tokIter = tokIter->next()) {
                            if (Token::Match(tokIter, "[;{}] %any% :") && labelName->str() == tokIter->strAt(1)) {
                                labelInFollowingLoop = true;
                                break;
                            }
                        }
                    }
                }
                if (!labelInFollowingLoop)
                    unreachableCodeError(secondBreak, inconclusive);
                tok = Token::findmatch(secondBreak, "[}:]");
            } else
                tok = secondBreak;

            if (!tok)
                break;
        }
    }
}

void CheckOther::duplicateBreakError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "duplicateBreak",
                "Consecutive return, break, continue, goto or throw statements are unnecessary.\n"
                "Consecutive return, break, continue, goto or throw statements are unnecessary. "
                "The second statement can never be executed, and so should be removed.", inconclusive);
}

void CheckOther::unreachableCodeError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "unreachableCode",
                "Statements following return, break, continue, goto or throw will never be executed.", inconclusive);
}

//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------
bool CheckOther::isUnsigned(const Variable* var) const
{
    return(var && var->typeStartToken()->isUnsigned() && !var->isPointer() && !var->isArray() && _tokenizer->sizeOfType(var->typeStartToken()) >= _settings->sizeof_int);
}
bool CheckOther::isSigned(const Variable* var)
{
    return(var && !var->typeStartToken()->isUnsigned() && Token::Match(var->typeEndToken(), "int|char|short|long") && !var->isPointer() && !var->isArray());
}

void CheckOther::checkUnsignedDivision()
{
    bool warning = _settings->isEnabled("warning");

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        const Token* ifTok = 0;
        // Check for "ivar / uvar" and "uvar / ivar"
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {

            if (Token::Match(tok, "[).]")) // Don't check members or casted variables
                continue;

            if (Token::Match(tok->next(), "%var% / %num%")) {
                if (tok->strAt(3)[0] == '-' && isUnsigned(tok->next()->variable())) {
                    udivError(tok->next(), false);
                }
            } else if (Token::Match(tok->next(), "%num% / %var%")) {
                if (tok->strAt(1)[0] == '-' && isUnsigned(tok->tokAt(3)->variable())) {
                    udivError(tok->next(), false);
                }
            } else if (Token::Match(tok->next(), "%var% / %var%") && _settings->inconclusive && warning && !ifTok) {
                const Variable* var1 = tok->next()->variable();
                const Variable* var2 = tok->tokAt(3)->variable();
                if ((isUnsigned(var1) && isSigned(var2)) || (isUnsigned(var2) && isSigned(var1))) {
                    udivError(tok->next(), true);
                }
            } else if (!ifTok && Token::simpleMatch(tok, "if ("))
                ifTok = tok->next()->link()->next()->link();
            else if (ifTok == tok)
                ifTok = 0;
        }
    }
}

void CheckOther::udivError(const Token *tok, bool inconclusive)
{
    if (inconclusive)
        reportError(tok, Severity::warning, "udivError", "Division with signed and unsigned operators. The result might be wrong.", true);
    else
        reportError(tok, Severity::error, "udivError", "Unsigned division. The result will be wrong.");
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
            if (Token::simpleMatch(tok, "memset (")) {
                const Token* lastParamTok = tok->next()->link()->previous();
                if (lastParamTok->str() == "0")
                    memsetZeroBytesError(tok, tok->strAt(2));
            }
        }
    }
}

void CheckOther::memsetZeroBytesError(const Token *tok, const std::string &varname)
{
    const std::string summary("memset() called to fill 0 bytes of '" + varname + "'.");
    const std::string verbose(summary + " Second and third arguments might be inverted.");
    reportError(tok, Severity::warning, "memsetZeroBytes", summary + "\n" + verbose);
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
        if (!var || !var->isLocal() || (!var->isPointer() && !var->typeStartToken()->isStandardType() && !var->typeStartToken()->next()->isStandardType()))
            continue;

        bool forHead = false; // Don't check variables declared in header of a for loop
        for (const Token* tok = var->typeStartToken(); tok; tok = tok->previous()) {
            if (tok->str() == "(") {
                forHead = true;
                break;
            } else if (tok->str() == "{" || tok->str() == ";" || tok->str() == "}")
                break;
        }
        if (forHead)
            continue;

        const Token* tok = var->nameToken()->next();
        if (Token::Match(tok, "; %varid% = %any% ;", var->varId())) {
            tok = tok->tokAt(3);
            if (!tok->isNumber() && tok->type() != Token::eString && tok->type() != Token::eChar && !tok->isBoolean())
                continue;
        } else if ((tok->str() == "=" || tok->str() == "(") &&
                   ((!tok->next()->isNumber() && tok->next()->type() != Token::eString && tok->next()->type() != Token::eChar && !tok->next()->isBoolean()) || tok->strAt(2) != ";"))
            continue;

        bool reduce = true;
        bool used = false; // Don't warn about unused variables
        for (; tok != var->scope()->classEnd; tok = tok->next()) {
            if (tok->str() == "{" && tok->strAt(-1) != "=") {
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
            } else if (tok->varId() == var->varId() || tok->str() == "goto") {
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

    for (; tok != end; tok = tok->next()) {
        if (tok->str() == "goto")
            return false;
        if (tok->str() == "continue")
            noContinue = false;

        if (Token::simpleMatch(tok, "for ("))
            forHeadEnd = tok->linkAt(1);
        if (tok == forHeadEnd)
            forHeadEnd = 0;

        if (loopVariable && noContinue && tok->scope() == scope && !forHeadEnd && scope->type != Scope::eSwitch && Token::Match(tok, "%varid% =", var->varId())) { // Assigned in outer scope.
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
                else if (tok2->varId() == var->varId()) {
                    loopVariable = true;
                    break;
                }
            }
        }

        if (loopVariable && Token::Match(tok, "%varid% !!=", var->varId())) // Variable used in loop
            return false;

        if (Token::Match(tok, "& %varid%", var->varId())) // Taking address of variable
            return false;

        if (Token::Match(tok, "= %varid%", var->varId()) && (var->isArray() || var->isPointer())) // Create a copy of array/pointer. Bailout, because the memory it points to might be necessary in outer scope
            return false;

        if (tok->varId() == var->varId()) {
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
                "When you see this message it is always safe to reduce the variable scope 1 level.");
}

void CheckOther::checkCommaSeparatedReturn()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok ; tok = tok->next())      {
        if (Token::Match(tok ,"return")) {

            while (tok->str() != ";")   {

                if (tok->str() == "(")
                    tok=tok->link();

                // Skip template parameters
                if (tok->str() == "<" && TemplateSimplifier::templateParameters(tok) > 0U) {
                    unsigned int level = 1U;
                    while (level > 0U && NULL != (tok = tok->next())) {
                        if (tok->str() == "<")
                            level++;
                        else if (tok->str() == ">")
                            level--;
                        else if (tok->str() == ">>")
                            level = level - ((level > 1U) ? 2 : 1);
                        else if (tok->str() == ";")
                            break;
                    }
                }

                if (!tok->isExpandedMacro() && tok->str() == "," && tok->linenr() != tok->next()->linenr())
                    commaSeparatedReturnError(tok);

                tok=tok->next();
            }
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
                "macro is then used in a return statement, it is less likely such code is misunderstood.");
}

//---------------------------------------------------------------------------
// Check for constant function parameters
//---------------------------------------------------------------------------
void CheckOther::checkConstantFunctionParameter()
{
    if (!_settings->isEnabled("performance") || _tokenizer->isC())
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (unsigned int i = 1; i < symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || !var->isArgument() || !var->isClass() || !var->isConst() || var->isPointer() || var->isArray() || var->isReference())
            continue;

        const Token* const tok = var->typeStartToken();
        // TODO: False negatives. This pattern only checks for string.
        //       Investigate if there are other classes in the std
        //       namespace and add them to the pattern. There are
        //       streams for example (however it seems strange with
        //       const stream parameter).
        if (Token::Match(tok, "std :: string|wstring")) {
            passedByValueError(tok, var->name());
        } else if (Token::Match(tok, "std :: %type% <") && !Token::simpleMatch(tok->linkAt(3), "> ::")) {
            passedByValueError(tok, var->name());
        } else if (var->type()) {  // Check if type is a struct or class.
            passedByValueError(tok, var->name());
        }
    }
}

void CheckOther::passedByValueError(const Token *tok, const std::string &parname)
{
    reportError(tok, Severity::performance, "passedByValue",
                "Function parameter '" + parname + "' should be passed by reference.\n"
                "Parameter '" +  parname + "' is passed by value. It could be passed "
                "as a (const) reference which is usually faster and recommended in C++.");
}

//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------
static bool isChar(const Variable* var)
{
    return(var && !var->isPointer() && !var->isArray() && var->typeStartToken()->str() == "char");
}

static bool isSignedChar(const Variable* var)
{
    return(isChar(var) && !var->typeStartToken()->isUnsigned());
}

void CheckOther::checkCharVariable()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if ((tok->str() != ".") && Token::Match(tok->next(), "%var% [ %var% ]")) {
                const Variable* arrayvar = tok->next()->variable();
                const Variable* indexvar = tok->tokAt(3)->variable();
                const MathLib::bigint arraysize = (arrayvar && arrayvar->isArray()) ? arrayvar->dimension(0U) : 0;
                if (isSignedChar(indexvar) && arraysize > 0x80)
                    charArrayIndexError(tok->next());
            }

            else if (Token::Match(tok, "[;{}] %var% = %any% [&^|] %any% ;")) {
                // is a char variable used in the calculation?
                if (!isSignedChar(tok->tokAt(3)->variable()) &&
                    !isSignedChar(tok->tokAt(5)->variable()))
                    continue;

                // it's ok with a bitwise and where the other operand is 0xff or less..
                if (tok->strAt(4) == "&") {
                    if (tok->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok->strAt(3)))
                        continue;
                    if (tok->tokAt(5)->isNumber() && MathLib::isGreater("0x100", tok->strAt(5)))
                        continue;
                }

                // is the result stored in a short|int|long?
                const Variable *var = tok->next()->variable();
                if (var && Token::Match(var->typeStartToken(), "short|int|long") && !var->isPointer() && !var->isArray())
                    charBitOpError(tok->tokAt(4)); // This is an error..
            }

            else if (Token::Match(tok, "[;{}] %var% = %any% [&^|] ( * %var% ) ;")) {
                const Variable* var = tok->tokAt(7)->variable();
                if (!var || !var->isPointer() || var->typeStartToken()->str() != "char" || var->typeStartToken()->isUnsigned())
                    continue;
                // it's ok with a bitwise and where the other operand is 0xff or less..
                if (tok->strAt(4) == "&" && tok->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok->strAt(3)))
                    continue;

                // is the result stored in a short|int|long?
                var = tok->next()->variable();
                if (var && Token::Match(var->typeStartToken(), "short|int|long") && !var->isPointer() && !var->isArray())
                    charBitOpError(tok->tokAt(4)); // This is an error..
            }
        }
    }
}

void CheckOther::charArrayIndexError(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "charArrayIndex",
                "Signed 'char' type used as array index.\n"
                "Signed 'char' type used as array index. If the value "
                "can be greater than 127 there will be a buffer underflow "
                "because of sign extension.");
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
                "The \"not expected\" will be printed on the screen.");
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

        // C++11 struct/array initialization in initializer list
        else if (tok->str() == "{" && Token::Match(tok->tokAt(-2), ",|: %var%") && Token::Match(tok->link(), "} [,{]"))
            tok = tok->link();

        // C++11 vector initialization / return { .. }
        else if (Token::Match(tok,"> %var% {") || Token::Match(tok, "[;{}] return {"))
            tok = tok->linkAt(2);

        else if (Token::Match(tok, "[;{}] %str%") || Token::Match(tok, "[;{}] %num%")) {
            // No warning if numeric constant is followed by a "." or ","
            if (Token::Match(tok->next(), "%num% [,.]"))
                continue;

            // bailout if there is a "? :" in this statement
            bool bailout = false;
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "?")
                    bailout = true;
                else if (tok2->str() == ";")
                    break;
            }
            if (bailout)
                continue;

            constStatementError(tok->next(), tok->next()->isNumber() ? "numeric" : "string");
        }
    }
}

void CheckOther::constStatementError(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::warning, "constStatement", "Redundant code: Found a statement that begins with " + type + " constant.");
}

//---------------------------------------------------------------------------
// str plus char
//---------------------------------------------------------------------------

void CheckOther::strPlusChar()
{
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "[=(] %str% + %any%")) {
                // char constant..
                if (tok->tokAt(3)->type() == Token::eChar)
                    strPlusCharError(tok->next());

                // char variable..
                if (isChar(tok->tokAt(3)->variable()))
                    strPlusCharError(tok->next());
            }
        }
    }
}

void CheckOther::strPlusCharError(const Token *tok)
{
    reportError(tok, Severity::error, "strPlusChar", "Unusual pointer arithmetic. A value of type 'char' is added to a string literal.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkZeroDivision()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[/%] %num%") &&
            MathLib::isInt(tok->next()->str()) &&
            MathLib::toLongNumber(tok->next()->str()) == 0L) {
            zerodivError(tok);
        } else if (Token::Match(tok, "div|ldiv|lldiv|imaxdiv ( %num% , %num% )") &&
                   MathLib::isInt(tok->strAt(4)) &&
                   MathLib::toLongNumber(tok->strAt(4)) == 0L) {
            zerodivError(tok);
        }
    }
}

void CheckOther::zerodivError(const Token *tok)
{
    reportError(tok, Severity::error, "zerodiv", "Division by zero.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

/** @brief Check for NaN (not-a-number) in an arithmetic expression
 *  @note e.g. double d = 1.0 / 0.0 + 100.0;
 */
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
                "Although nothing bad really happens, it is suspicious.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkMathFunctions()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->varId())
                continue;
            if (Token::Match(tok, "log|log10 ( %num% )")) {
                bool isNegative = MathLib::isNegative(tok->strAt(2));
                bool isInt = MathLib::isInt(tok->strAt(2));
                bool isFloat = MathLib::isFloat(tok->strAt(2));
                if (isNegative && isInt && MathLib::toLongNumber(tok->strAt(2)) <= 0) {
                    mathfunctionCallError(tok); // case log(-2)
                } else if (isNegative && isFloat && MathLib::toDoubleNumber(tok->strAt(2)) <= 0.) {
                    mathfunctionCallError(tok); // case log(-2.0)
                } else if (!isNegative && isFloat && MathLib::toDoubleNumber(tok->strAt(2)) <= 0.) {
                    mathfunctionCallError(tok); // case log(0.0)
                } else if (!isNegative && isInt && MathLib::toLongNumber(tok->strAt(2)) <= 0) {
                    mathfunctionCallError(tok); // case log(0)
                }
            }

            // acos( x ), asin( x )  where x is defined for interval [-1,+1], but not beyond
            else if (Token::Match(tok, "acos|asin ( %num% )") &&
                     std::fabs(MathLib::toDoubleNumber(tok->strAt(2))) > 1.0) {
                mathfunctionCallError(tok);
            }
            // sqrt( x ): if x is negative the result is undefined
            else if (Token::Match(tok, "sqrt|sqrtf|sqrtl ( %num% )") &&
                     MathLib::isNegative(tok->strAt(2))) {
                mathfunctionCallError(tok);
            }
            // atan2 ( x , y): x and y can not be zero, because this is mathematically not defined
            else if (Token::Match(tok, "atan2 ( %num% , %num% )") &&
                     MathLib::isNullValue(tok->strAt(2)) &&
                     MathLib::isNullValue(tok->strAt(4))) {
                mathfunctionCallError(tok, 2);
            }
            // fmod ( x , y) If y is zero, then either a range error will occur or the function will return zero (implementation-defined).
            else if (Token::Match(tok, "fmod ( %any%")) {
                const Token* nextArg = tok->tokAt(2)->nextArgument();
                if (nextArg && nextArg->isNumber() && MathLib::isNullValue(nextArg->str()))
                    mathfunctionCallError(tok, 2);
            }
            // pow ( x , y) If x is zero, and y is negative --> division by zero
            else if (Token::Match(tok, "pow ( %num% , %num% )") &&
                     MathLib::isNullValue(tok->strAt(2))  &&
                     MathLib::isNegative(tok->strAt(4))) {
                mathfunctionCallError(tok, 2);
            }
        }
    }
}

void CheckOther::mathfunctionCallError(const Token *tok, const unsigned int numParam)
{
    if (tok) {
        if (numParam == 1)
            reportError(tok, Severity::error, "wrongmathcall", "Passing value " + tok->strAt(2) + " to " + tok->str() + "() leads to undefined result.");
        else if (numParam == 2)
            reportError(tok, Severity::error, "wrongmathcall", "Passing values " + tok->strAt(2) + " and " + tok->strAt(4) + " to " + tok->str() + "() leads to undefined result.");
    } else
        reportError(tok, Severity::error, "wrongmathcall", "Passing value '#' to #() leads to undefined result.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkCCTypeFunctions()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->varId() == 0 &&
            Token::Match(tok, "isalnum|isalpha|iscntrl|isdigit|isgraph|islower|isprint|ispunct|isspace|isupper|isxdigit ( %num% ,|)") &&
            MathLib::isNegative(tok->strAt(2))) {
            cctypefunctionCallError(tok, tok->str(), tok->strAt(2));
        }
    }
}
void CheckOther::cctypefunctionCallError(const Token *tok, const std::string &functionName, const std::string &value)
{
    reportError(tok, Severity::error, "wrongcctypecall", "Passing value " + value + " to " + functionName + "() causes undefined behavior which may lead to a crash.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkMisusedScopedObject()
{
    // Skip this check for .c files
    if (_tokenizer->isC()) {
        return;
    }

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "[;{}] %var% (")
                && Token::simpleMatch(tok->linkAt(2), ") ;")
                && symbolDatabase->isClassOrStruct(tok->next()->str())
                && !tok->next()->function()) {
                tok = tok->next();
                misusedScopeObjectError(tok, tok->str());
                tok = tok->next();
            }
        }
    }
}

void CheckOther::misusedScopeObjectError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "unusedScopedObject", "Instance of '" + varname + "' object is destroyed immediately.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkIncorrectStringCompare()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // skip "assert(str && ..)" and "assert(.. && str)"
            if (Token::Match(tok, "%var% (") &&
                (Token::Match(tok->tokAt(2), "%str% &&") || Token::Match(tok->next()->link()->tokAt(-2), "&& %str% )")) &&
                (tok->str().find("assert")+6U==tok->str().size() || tok->str().find("ASSERT")+6U==tok->str().size()))
                tok = tok->next()->link();

            if (Token::simpleMatch(tok, ". substr (") && Token::Match(tok->tokAt(3)->nextArgument(), "%num% )")) {
                MathLib::bigint clen = MathLib::toLongNumber(tok->linkAt(2)->strAt(-1));
                const Token* begin = tok->previous();
                for (;;) { // Find start of statement
                    while (begin->link() && Token::Match(begin, "]|)|>"))
                        begin = begin->link()->previous();
                    if (Token::Match(begin->previous(), ".|::"))
                        begin = begin->tokAt(-2);
                    else
                        break;
                }
                begin = begin->previous();
                const Token* end = tok->linkAt(2)->next();
                if (Token::Match(begin->previous(), "%str% ==|!=") && begin->strAt(-2) != "+") {
                    std::size_t slen = Token::getStrLength(begin->previous());
                    if (clen != (int)slen) {
                        incorrectStringCompareError(tok->next(), "substr", begin->strAt(-1));
                    }
                } else if (Token::Match(end, "==|!= %str% !!+")) {
                    std::size_t slen = Token::getStrLength(end->next());
                    if (clen != (int)slen) {
                        incorrectStringCompareError(tok->next(), "substr", end->strAt(1));
                    }
                }
            } else if (Token::Match(tok, "&&|%oror%|( %str% &&|%oror%|)") && !Token::Match(tok, "( %str% )")) {
                incorrectStringBooleanError(tok->next(), tok->strAt(1));
            } else if (Token::Match(tok, "if|while ( %str% )")) {
                incorrectStringBooleanError(tok->tokAt(2), tok->strAt(2));
            }
        }
    }
}

void CheckOther::incorrectStringCompareError(const Token *tok, const std::string& func, const std::string &string)
{
    reportError(tok, Severity::warning, "incorrectStringCompare", "String literal " + string + " doesn't match length argument for " + func + "().");
}

void CheckOther::incorrectStringBooleanError(const Token *tok, const std::string& string)
{
    reportError(tok, Severity::warning, "incorrectStringBooleanError", "Conversion of string literal " + string + " to bool always evaluates to true.");
}

//-----------------------------------------------------------------------------
// check for duplicate expressions in if statements
// if (a) { } else if (a) { }
//-----------------------------------------------------------------------------

static bool expressionHasSideEffects(const Token *first, const Token *last)
{
    for (const Token *tok = first; tok != last->next(); tok = tok->next()) {
        // check for assignment
        if (tok->isAssignmentOp())
            return true;

        // check for inc/dec
        else if (tok->type() == Token::eIncDecOp)
            return true;

        // check for function call
        else if (Token::Match(tok, "%var% (") &&
                 !(Token::Match(tok, "c_str|string") || tok->isStandardType()))
            return true;
    }

    return false;
}

void CheckOther::checkDuplicateIf()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        const Token* const tok = scope->classDef;
        // only check if statements
        if (scope->type != Scope::eIf || !tok)
            continue;

        std::map<std::string, const Token*> expressionMap;

        // get the expression from the token stream
        std::string expression = tok->tokAt(2)->stringifyList(tok->next()->link());

        // save the expression and its location
        expressionMap.insert(std::make_pair(expression, tok));

        // find the next else if (...) statement
        const Token *tok1 = scope->classEnd;

        // check all the else if (...) statements
        while ((Token::simpleMatch(tok1, "} else if (") &&
                Token::simpleMatch(tok1->linkAt(3), ") {")) ||
               (Token::simpleMatch(tok1, "} else { if (") &&
                Token::simpleMatch(tok1->linkAt(4), ") {"))) {
            int conditionIndex=(tok1->strAt(3)=="(") ? 3 : 4;
            // get the expression from the token stream
            expression = tok1->tokAt(conditionIndex+1)->stringifyList(tok1->linkAt(conditionIndex));

            // try to look up the expression to check for duplicates
            std::map<std::string, const Token *>::iterator it = expressionMap.find(expression);

            // found a duplicate
            if (it != expressionMap.end()) {
                // check for expressions that have side effects and ignore them
                if (!expressionHasSideEffects(tok1->tokAt(conditionIndex+1), tok1->linkAt(conditionIndex)->previous()))
                    duplicateIfError(it->second, tok1->next());
            }

            // not a duplicate expression so save it and its location
            else
                expressionMap.insert(std::make_pair(expression, tok1->next()));

            // find the next else if (...) statement
            tok1 = tok1->linkAt(conditionIndex)->next()->link();
        }
    }
}

void CheckOther::duplicateIfError(const Token *tok1, const Token *tok2)
{
    std::list<const Token *> toks;
    toks.push_back(tok2);
    toks.push_back(tok1);

    reportError(toks, Severity::style, "duplicateIf", "Duplicate conditions in 'if' and related 'else if'.\n"
                "Duplicate conditions in 'if' and related 'else if'. This is suspicious and might indicate "
                "a cut and paste or logic error. Please examine this code carefully to determine "
                "if it is correct.");
}

//-----------------------------------------------------------------------------
// check for duplicate code in if and else branches
// if (a) { b = true; } else { b = true; }
//-----------------------------------------------------------------------------
void CheckOther::checkDuplicateBranch()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (scope->type != Scope::eIf && scope->type != Scope::eElseIf)
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
    std::list<const Token *> toks;
    toks.push_back(tok2);
    toks.push_back(tok1);

    reportError(toks, Severity::style, "duplicateBranch", "Found duplicate branches for 'if' and 'else'.\n"
                "Finding the same code in an 'if' and related 'else' branch is suspicious and "
                "might indicate a cut and paste or logic error. Please examine this code "
                "carefully to determine if it is correct.");
}


//-----------------------------------------------------------------------------
// Check for a free() of an invalid address
// char* p = malloc(100);
// free(p + 10);
//-----------------------------------------------------------------------------
void CheckOther::checkInvalidFree()
{
    std::map<unsigned int, bool> allocatedVariables;
    for (const Token* tok = _tokenizer->tokens(); tok; tok = tok->next()) {

        // Keep track of which variables were assigned addresses to newly-allocated memory
        if (Token::Match(tok, "%var% = malloc|g_malloc|new")) {
            allocatedVariables.insert(std::make_pair(tok->varId(), false));
        }

        // If a previously-allocated pointer is incremented or decremented, any subsequent
        // free involving pointer arithmetic may or may not be invalid, so we should only
        // report an inconclusive result.
        else if (Token::Match(tok, "%var% = %var% +|-") &&
                 tok->varId() == tok->tokAt(2)->varId() &&
                 allocatedVariables.find(tok->varId()) != allocatedVariables.end()) {
            if (_settings->inconclusive)
                allocatedVariables[tok->varId()] = true;
            else
                allocatedVariables.erase(tok->varId());
        }

        // If a previously-allocated pointer is assigned a completely new value,
        // we can't know if any subsequent free() on that pointer is valid or not.
        else if (Token::Match(tok, "%var% = ")) {
            allocatedVariables.erase(tok->varId());
        }

        // If a variable that was previously assigned a newly-allocated memory location is
        // added or subtracted from when used to free the memory, report an error.
        else if (Token::Match(tok, "free|g_free|delete ( %any% +|- %any%") ||
                 Token::Match(tok, "delete [ ] ( %any% +|- %any%") ||
                 Token::Match(tok, "delete %any% +|- %any%")) {

            const int varIdx = tok->strAt(1) == "(" ? 2 :
                               tok->strAt(3) == "(" ? 4 : 1;
            const unsigned int var1 = tok->tokAt(varIdx)->varId();
            const unsigned int var2 = tok->tokAt(varIdx + 2)->varId();
            const std::map<unsigned int, bool>::iterator alloc1 = allocatedVariables.find(var1);
            const std::map<unsigned int, bool>::iterator alloc2 = allocatedVariables.find(var2);
            if (alloc1 != allocatedVariables.end()) {
                invalidFreeError(tok, alloc1->second);
            } else if (alloc2 != allocatedVariables.end()) {
                invalidFreeError(tok, alloc2->second);
            }
        }

        // If the previously-allocated variable is passed in to another function
        // as a parameter, it might be modified, so we shouldn't report an error
        // if it is later used to free memory
        else if (Token::Match(tok, "%var% (")) {
            const Token* tok2 = Token::findmatch(tok->next(), "%var%", tok->linkAt(1));
            while (tok2 != NULL) {
                allocatedVariables.erase(tok2->varId());
                tok2 = Token::findmatch(tok2->next(), "%var%", tok->linkAt(1));
            }
        }
    }
}

void CheckOther::invalidFreeError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::error, "invalidFree", "Invalid memory address freed.", inconclusive);
}


//-----------------------------------------------------------------------------
// Check for double free
// free(p); free(p);
//-----------------------------------------------------------------------------
void CheckOther::checkDoubleFree()
{
    std::set<unsigned int> freedVariables;
    std::set<unsigned int> closeDirVariables;

    for (const Token* tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Keep track of any variables passed to "free()", "g_free()" or "closedir()",
        // and report an error if the same variable is passed twice.
        if (Token::Match(tok, "free|g_free|closedir ( %var% )")) {
            unsigned int var = tok->tokAt(2)->varId();
            if (var) {
                if (Token::Match(tok, "free|g_free")) {
                    if (freedVariables.find(var) != freedVariables.end())
                        doubleFreeError(tok, tok->strAt(2));
                    else
                        freedVariables.insert(var);
                } else if (tok->str() == "closedir") {
                    if (closeDirVariables.find(var) != closeDirVariables.end())
                        doubleCloseDirError(tok, tok->strAt(2));
                    else
                        closeDirVariables.insert(var);
                }
            }
        }

        // Keep track of any variables operated on by "delete" or "delete[]"
        // and report an error if the same variable is delete'd twice.
        else if (Token::Match(tok, "delete %var% ;") || Token::Match(tok, "delete [ ] %var% ;")) {
            int varIdx = (tok->strAt(1) == "[") ? 3 : 1;
            unsigned int var = tok->tokAt(varIdx)->varId();
            if (var) {
                if (freedVariables.find(var) != freedVariables.end())
                    doubleFreeError(tok, tok->strAt(varIdx));
                else
                    freedVariables.insert(var);
            }
        }

        // If this scope doesn't return, clear the set of previously freed variables
        else if (tok->str() == "}" && _tokenizer->IsScopeNoReturn(tok)) {
            freedVariables.clear();
            closeDirVariables.clear();
        }

        // If this scope is a "for" or "while" loop that contains "break" or "continue",
        // give up on trying to figure out the flow of execution and just clear the set
        // of previously freed variables.
        // TODO: There are false negatives. This bailout is only needed when the
        // loop will exit without free()'ing the memory on the last iteration.
        else if (tok->str() == "}" && tok->link() && tok->link()->previous() &&
                 tok->link()->linkAt(-1) &&
                 Token::Match(tok->link()->linkAt(-1)->previous(), "while|for") &&
                 Token::findmatch(tok->link()->linkAt(-1), "break|continue ;", tok) != NULL) {
            freedVariables.clear();
            closeDirVariables.clear();
        }

        // If a variable is passed to a function, remove it from the set of previously freed variables
        else if (Token::Match(tok, "%var% (") && !Token::Match(tok, "printf|sprintf|snprintf|fprintf|wprintf|swprintf|fwprintf")) {

            // If this is a new function definition, clear all variables
            if (Token::simpleMatch(tok->next()->link(), ") {")) {
                freedVariables.clear();
                closeDirVariables.clear();
            }
            // If it is a function call, then clear those variables in its argument list
            else if (Token::simpleMatch(tok->next()->link(), ") ;")) {
                for (const Token* tok2 = tok->tokAt(2); tok2 != tok->linkAt(1); tok2 = tok2->next()) {
                    if (tok2->varId()) {
                        unsigned int var = tok2->varId();
                        freedVariables.erase(var);
                        closeDirVariables.erase(var);
                    }
                }
            }
        }

        // If a pointer is assigned a new value, remove it from the set of previously freed variables
        else if (Token::Match(tok, "%var% =")) {
            unsigned int var = tok->varId();
            if (var) {
                freedVariables.erase(var);
                closeDirVariables.erase(var);
            }
        }

        // Any control statements in-between delete, free() or closedir() statements
        // makes it unclear whether any subsequent statements would be redundant.
        if (Token::Match(tok, "if|else|for|while|break|continue|goto|return|throw|switch")) {
            freedVariables.clear();
            closeDirVariables.clear();
        }
    }
}

void CheckOther::doubleFreeError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "doubleFree", "Memory pointed to by '" + varname +"' is freed twice.");
}

void CheckOther::doubleCloseDirError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "doubleCloseDir", "Directory handle '" + varname +"' closed twice.");
}

namespace {
    struct ExpressionTokens {
        const Token *start;
        const Token *end;
        int count;
        bool inconclusiveFunction;
        ExpressionTokens(const Token *s, const Token *e): start(s), end(e), count(1), inconclusiveFunction(false) {}
    };

    struct FuncFilter {
        FuncFilter(const Scope *scope, const Token *tok): _scope(scope), _tok(tok) {}

        bool operator()(const Function* func) const {
            bool matchingFunc = func->type == Function::eFunction &&
                                _tok->str() == func->token->str();
            // either a class function, or a global function with the same name
            return (_scope && _scope == func->nestedIn && matchingFunc) ||
                   (!_scope && matchingFunc);
        }
        const Scope *_scope;
        const Token *_tok;
    };

    bool inconclusiveFunctionCall(const std::list<const Function*> &constFunctions,
                                  const ExpressionTokens &tokens)
    {
        const Token *start = tokens.start;
        const Token *end = tokens.end;
        // look for function calls between start and end...
        for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
            if (tok != start && tok->str() == "(") {
                // go back to find the function call.
                const Token *prev = tok->previous();
                if (!prev)
                    continue;
                if (prev->str() == ">") {
                    // ignore template functions like boo<double>()
                    return true;
                }
                if (prev->isName()) {
                    const Variable *v = 0;
                    if (Token::Match(prev->tokAt(-2), "%var% .")) {
                        const Token *scope = prev->tokAt(-2);
                        v = scope->variable();
                    }
                    // hard coded list of safe, no-side-effect functions
                    if (v == 0 && Token::Match(prev, "strcmp|strncmp|strlen|wcscmp|wcsncmp|wcslen|memcmp|strcasecmp|strncasecmp"))
                        return false;
                    std::list<const Function*>::const_iterator it = std::find_if(constFunctions.begin(),
                            constFunctions.end(),
                            FuncFilter(v ? v->typeScope(): 0, prev));
                    if (it == constFunctions.end())
                        return true;
                }
            }
        }
        return false;
    }

    class Expressions {
    public:
        Expressions(const std::list<const Function*> &constFunctions)
            : _start(0),
              _lastTokens(0),
              _constFunctions(constFunctions) { }

        void endExpr(const Token *end) {
            const std::string &e = _expression.str();
            if (!e.empty()) {
                std::map<std::string, ExpressionTokens>::iterator it = _expressions.find(e);
                bool lastInconclusive = _lastTokens && _lastTokens->inconclusiveFunction;
                if (it == _expressions.end()) {
                    ExpressionTokens exprTokens(_start, end);
                    exprTokens.inconclusiveFunction = lastInconclusive || inconclusiveFunctionCall(
                                                          _constFunctions, exprTokens);
                    _expressions.insert(std::make_pair(e, exprTokens));
                    _lastTokens = &_expressions.find(e)->second;
                } else {
                    ExpressionTokens &expr = it->second;
                    expr.count += 1;
                    expr.inconclusiveFunction = expr.inconclusiveFunction || lastInconclusive;
                    _lastTokens = &expr;
                }
            }
            _expression.str("");
            _start = 0;
        }

        void append(const Token *tok) {
            if (!_start)
                _start = tok;
            _expression << tok->str();
        }

        std::map<std::string,ExpressionTokens> &getMap() {
            return _expressions;
        }

    private:
        std::map<std::string, ExpressionTokens> _expressions;
        std::ostringstream _expression;
        const Token *_start;
        ExpressionTokens *_lastTokens;
        const std::list<const Function*> &_constFunctions;
    };

    bool notconst(const Function* func)
    {
        return !func->isConst;
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

void CheckOther::checkExpressionRange(const std::list<const Function*> &constFunctions,
                                      const Token *start,
                                      const Token *end,
                                      const std::string &toCheck)
{
    if (!start || !end)
        return;
    Expressions expressions(constFunctions);
    std::string opName;
    int level = 0;
    for (const Token *tok = start->next(); tok && tok != end; tok = tok->next()) {
        if (tok->str() == ")")
            level--;
        else if (tok->str() == "(")
            level++;

        if (level == 0 && Token::Match(tok, toCheck.c_str())) {
            opName = tok->str();
            expressions.endExpr(tok);
        } else {
            expressions.append(tok);
        }
    }
    expressions.endExpr(end);
    std::map<std::string,ExpressionTokens>::const_iterator it = expressions.getMap().begin();
    for (; it != expressions.getMap().end(); ++it) {
        // check expression..
        bool valid = true;
        unsigned int parentheses = 0;  // ()
        unsigned int brackets = 0;     // []

        // taking address?
        if (Token::Match(it->second.end->previous(), "%op% &")) {
            continue;
        }

        for (const Token *tok = it->second.start; tok && tok != it->second.end; tok = tok->next()) {
            if (tok->str() == "(") {
                ++parentheses;
            } else if (tok->str() == ")") {
                if (parentheses == 0) {
                    valid = false;
                    break;
                }
                --parentheses;
            } else if (tok->str() == "[") {
                ++brackets;
            } else if (tok->str() == "]") {
                if (brackets == 0) {
                    valid = false;
                    break;
                }
                --brackets;
            } else if (tok->type() == Token::eIncDecOp) {
                valid = false;
                break;
            }
        }

        if (!valid || parentheses!=0 || brackets!=0)
            continue;

        const ExpressionTokens &expr = it->second;
        if (expr.count > 1 && !expr.inconclusiveFunction) {
            duplicateExpressionError(expr.start, expr.start, opName);
        }
    }
}

void CheckOther::complexDuplicateExpressionCheck(const std::list<const Function*> &constFunctions,
        const Token *classStart,
        const std::string &toCheck,
        const std::string &alt)
{
    std::string statementStart(",|=|?|:|return");
    if (!alt.empty())
        statementStart += "|" + alt;
    std::string statementEnd(";|,|?|:");
    if (!alt.empty())
        statementEnd += "|" + alt;

    for (const Token *tok = classStart; tok && tok != classStart->link(); tok = tok->next()) {
        if (!Token::Match(tok, toCheck.c_str()))
            continue;

        // look backward for the start of the statement
        const Token *start = 0;
        int level = 0;
        for (const Token *tok1 = tok->previous(); tok1 && tok1 != classStart; tok1 = tok1->previous()) {
            if (tok1->str() == ")")
                level++;
            else if (tok1->str() == "(")
                level--;

            if (level < 0 || (level == 0 && Token::Match(tok1, statementStart.c_str()))) {
                start = tok1;
                break;
            }
        }
        const Token *end = 0;
        level = 0;
        // look for the end of the statement
        for (const Token *tok1 = tok->next(); tok1 && tok1 != classStart->link(); tok1 = tok1->next()) {
            if (tok1->str() == ")")
                level--;
            else if (tok1->str() == "(")
                level++;

            if (level < 0 || (level == 0 && Token::Match(tok1, statementEnd.c_str()))) {
                end = tok1;
                break;
            }
        }
        checkExpressionRange(constFunctions, start, end, toCheck);
    }
}

//---------------------------------------------------------------------------
// check for the same expression on both sides of an operator
// (x == x), (x && x), (x || x)
// (x.y == x.y), (x.y && x.y), (x.y || x.y)
//---------------------------------------------------------------------------
void CheckOther::checkDuplicateExpression()
{
    if (!_settings->isEnabled("style"))
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

        complexDuplicateExpressionCheck(constFunctions, scope->classStart, "%or%", "");
        complexDuplicateExpressionCheck(constFunctions, scope->classStart, "%oror%", "");
        complexDuplicateExpressionCheck(constFunctions, scope->classStart, "&", "%oror%|%or%");
        complexDuplicateExpressionCheck(constFunctions, scope->classStart, "&&", "%oror%|%or%");

        for (const Token *tok = scope->classStart; tok && tok != scope->classStart->link(); tok = tok->next()) {
            if (Token::Match(tok, ",|=|return|(|&&|%oror% %var% %comp%|- %var% )|&&|%oror%|;|,") &&
                tok->strAt(1) == tok->strAt(3)) {
                // float == float and float != float are valid NaN checks
                // float - float is a valid Inf check
                if (Token::Match(tok->tokAt(2), "==|!=|-") && tok->next()->varId()) {
                    const Variable *var = tok->next()->variable();
                    if (var && var->typeStartToken() == var->typeEndToken()) {
                        if (Token::Match(var->typeStartToken(), "float|double"))
                            continue;
                    }
                }

                // If either variable token is an expanded macro then
                // don't write the warning
                if (tok->next()->isExpandedMacro() || tok->tokAt(3)->isExpandedMacro())
                    continue;

                duplicateExpressionError(tok->next(), tok->tokAt(3), tok->strAt(2));
            } else if (Token::Match(tok, ",|=|return|(|&&|%oror% %var% . %var% %comp%|- %var% . %var% )|&&|%oror%|;|,") &&
                       tok->strAt(1) == tok->strAt(5) && tok->strAt(3) == tok->strAt(7)) {

                // If either variable token is an expanded macro then
                // don't write the warning
                if (tok->next()->isExpandedMacro() || tok->tokAt(6)->isExpandedMacro())
                    continue;

                duplicateExpressionError(tok->next(), tok->tokAt(6), tok->strAt(4));
            }
        }
    }
}

void CheckOther::duplicateExpressionError(const Token *tok1, const Token *tok2, const std::string &op)
{
    std::list<const Token *> toks;
    toks.push_back(tok2);
    toks.push_back(tok1);

    reportError(toks, Severity::style, "duplicateExpression", "Same expression on both sides of \'" + op + "\'.\n"
                "Finding the same expression on both sides of an operator is suspicious and might "
                "indicate a cut and paste or logic error. Please examine this code carefully to "
                "determine if it is correct.");
}

//---------------------------------------------------------------------------
// Check for string comparison involving two static strings.
// if(strcmp("00FF00","00FF00")==0) // <- statement is always true
//---------------------------------------------------------------------------
void CheckOther::checkAlwaysTrueOrFalseStringCompare()
{
    if (!_settings->isEnabled("warning"))
        return;

    const Token *tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, "strncmp|strcmp|stricmp|strcmpi|strcasecmp|wcscmp|wcsncmp ( %str% , %str% ")) != NULL) {
        const std::string &str1 = tok->strAt(2);
        const std::string &str2 = tok->strAt(4);
        alwaysTrueFalseStringCompareError(tok, str1, str2);
        tok = tok->tokAt(5);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, "QString :: compare ( %str% , %str% )")) != NULL) {
        const std::string &str1 = tok->strAt(4);
        const std::string &str2 = tok->strAt(6);
        alwaysTrueFalseStringCompareError(tok, str1, str2);
        tok = tok->tokAt(7);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, "strncmp|strcmp|stricmp|strcmpi|strcasecmp|wcscmp|wcsncmp ( %var% , %var% ")) != NULL) {
        const std::string &str1 = tok->strAt(2);
        const std::string &str2 = tok->strAt(4);
        if (str1 == str2)
            alwaysTrueStringVariableCompareError(tok, str1, str2);
        tok = tok->tokAt(5);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, "!!+ %str% ==|!= %str% !!+")) != NULL) {
        const std::string &str1 = tok->strAt(1);
        const std::string &str2 = tok->strAt(3);
        alwaysTrueFalseStringCompareError(tok, str1, str2);
        tok = tok->tokAt(5);
    }
}

void CheckOther::alwaysTrueFalseStringCompareError(const Token *tok, const std::string& str1, const std::string& str2)
{
    const std::size_t stringLen = 10;
    const std::string string1 = (str1.size() < stringLen) ? str1 : (str1.substr(0, stringLen-2) + "..");
    const std::string string2 = (str2.size() < stringLen) ? str2 : (str2.substr(0, stringLen-2) + "..");

    reportError(tok, Severity::warning, "staticStringCompare",
                "Unnecessary comparison of static strings.\n"
                "The compared strings, '" + string1 + "' and '" + string2 + "', are always " + (str1==str2?"identical":"unequal") + ". "
                "Therefore the comparison is unnecessary and looks suspicious.");
}

void CheckOther::alwaysTrueStringVariableCompareError(const Token *tok, const std::string& str1, const std::string& str2)
{
    reportError(tok, Severity::warning, "stringCompare",
                "Comparison of identical string variables.\n"
                "The compared strings, '" + str1 + "' and '" + str2 + "', are identical. "
                "This could be a logic bug.");
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckOther::checkSuspiciousStringCompare()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->next()->type() != Token::eComparisonOp)
                continue;

            const Token* varTok = tok;
            const Token* litTok = tok->tokAt(2);

            if (varTok->strAt(-1) == "+" || litTok->strAt(1) == "+")
                continue;

            if ((varTok->type() == Token::eString || varTok->type() == Token::eVariable) && (litTok->type() == Token::eString || litTok->type() == Token::eVariable) && litTok->type() != varTok->type()) {
                if (varTok->type() == Token::eString)
                    std::swap(varTok, litTok);

                const Variable *var = varTok->variable();
                if (var) {
                    if (_tokenizer->isC() ||
                        (var->isPointer() && varTok->strAt(-1) != "*" && !Token::Match(varTok->next(), "[.([]")))
                        suspiciousStringCompareError(tok, var->name());
                }
            }
        }
    }
}

void CheckOther::suspiciousStringCompareError(const Token* tok, const std::string& var)
{
    reportError(tok, Severity::warning, "literalWithCharPtrCompare",
                "String literal compared with variable '" + var + "'. Did you intend to use strcmp() instead?");
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckOther::checkModuloAlwaysTrueFalse()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if ((Token::Match(tok, "% %num% %comp% %num%")) &&
                (!tok->tokAt(4) || !tok->tokAt(4)->isArithmeticalOp())) {
                if (MathLib::isLessEqual(tok->strAt(1), tok->strAt(3)))
                    moduloAlwaysTrueFalseError(tok, tok->strAt(1));
            }
        }
    }
}

void CheckOther::moduloAlwaysTrueFalseError(const Token* tok, const std::string& maxVal)
{
    reportError(tok, Severity::warning, "moduloAlwaysTrueFalse",
                "Comparison of modulo result is predetermined, because it is always less than " + maxVal + ".");
}

//-----------------------------------------------------------------------------
// Check for code like:
// seteuid(geteuid()) or setuid(getuid()), which first gets and then sets the
// (effective) user id to itself. Very often this indicates a copy and paste
// error.
//-----------------------------------------------------------------------------
void CheckOther::redundantGetAndSetUserId()
{
    if (_settings->isEnabled("warning")
        && _settings->standards.posix) {

        for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
            if (Token::simpleMatch(tok, "setuid ( getuid ( ) )")
                ||  Token::simpleMatch(tok, "seteuid ( geteuid ( ) )")
                ||  Token::simpleMatch(tok, "setgid ( getgid ( ) )")
                ||  Token::simpleMatch(tok, "setegid ( getegid ( ) )")) {
                redundantGetAndSetUserIdError(tok);
            }
        }
    }
}
void CheckOther::redundantGetAndSetUserIdError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "redundantGetAndSetUserId", "Redundant get and set of user id.\n"
                "Redundant statement without any effect. First the user id is retrieved"
                "by get(e)uid() and then set with set(e)uid().", false);
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
            if (Token::Match(tok, "%var% <|<= 0") && tok->varId() && !Token::Match(tok->previous(), "++|--|)|+|-|*|/|~|<<|>>") && !Token::Match(tok->tokAt(3), "+|-")) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZeroError(tok, var->name(), inconclusive);
                else if (var && var->isPointer() && tok->strAt(-1) != "*")
                    pointerLessThanZeroError(tok, inconclusive);
            } else if (Token::Match(tok, "0 >|>= %var%") && tok->tokAt(2)->varId() && !Token::Match(tok->tokAt(3), "+|-|*|/") && !Token::Match(tok->previous(), "+|-|<<|>>|~")) {
                const Variable *var = tok->tokAt(2)->variable();
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZeroError(tok, var->name(), inconclusive);
                else if (var && var->isPointer() && !Token::Match(tok->tokAt(3), "[.[]"))
                    pointerLessThanZeroError(tok, inconclusive);
            } else if (Token::Match(tok, "0 <= %var%") && tok->tokAt(2)->varId() && !Token::Match(tok->tokAt(3), "+|-|*|/") && !Token::Match(tok->previous(), "+|-|<<|>>|~")) {
                const Variable *var = tok->tokAt(2)->variable();
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedPositiveError(tok, var->name(), inconclusive);
                else if (var && var->isPointer() && !Token::Match(tok->tokAt(3), "[.[]"))
                    pointerPositiveError(tok, inconclusive);
            } else if (Token::Match(tok, "%var% >= 0") && tok->varId() && !Token::Match(tok->previous(), "++|--|)|+|-|*|/|~|<<|>>") && !Token::Match(tok->tokAt(3), "+|-")) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedPositiveError(tok, var->name(), inconclusive);
                else if (var && var->isPointer() && tok->strAt(-1) != "*")
                    pointerPositiveError(tok, inconclusive);
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
                    "this message might be a false warning.", true);
    } else {
        reportError(tok, Severity::style, "unsignedLessThanZero",
                    "Checking if unsigned variable '" + varname + "' is less than zero.\n"
                    "The unsigned variable '" + varname + "' will never be negative so it "
                    "is either pointless or an error to check if it is.");
    }
}

void CheckOther::pointerLessThanZeroError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "pointerLessThanZero",
                "A pointer can not be negative so it is either pointless or an error to check if it is.", inconclusive);
}

void CheckOther::unsignedPositiveError(const Token *tok, const std::string &varname, bool inconclusive)
{
    if (inconclusive) {
        reportError(tok, Severity::style, "unsignedPositive",
                    "Unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it.\n"
                    "The unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it. "
                    "It's not known if the used constant is a "
                    "template parameter or not and therefore this message might be a false warning", true);
    } else {
        reportError(tok, Severity::style, "unsignedPositive",
                    "Unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it.");
    }
}

void CheckOther::pointerPositiveError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "pointerPositive",
                "A pointer can not be negative so it is either pointless or an error to check if it is not.", inconclusive);
}

/*
This check rule works for checking the "const A a = getA()" usage when getA() returns "const A &" or "A &".
In most scenarios, "const A & a = getA()" will be more efficient.
*/
void CheckOther::checkRedundantCopy()
{
    if (!_settings->isEnabled("performance") || _tokenizer->isC())
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::size_t i = 0; i < symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);

        if (!var || var->isReference() || !var->isConst() || var->isPointer() || !var->type()) // bailout if var is of standard type, if it is a pointer or non-const
            continue;

        const Token* startTok = var->nameToken();
        const Token* endToken;
        if (startTok->strAt(1) == "=") // %type% %var% = ... ;
            endToken = Token::findsimplematch(startTok->tokAt(2), ";");
        else if (startTok->strAt(1) == "(") // %type% %var%(...)
            endToken = startTok->linkAt(1);
        else
            continue;

        const Token* tok = startTok->tokAt(2);
        while (tok && Token::Match(tok, "%var% .|::"))
            tok = tok->tokAt(2);
        if (!Token::Match(tok, "%var% ("))
            continue;
        if (tok->linkAt(1)->next() != endToken) // bailout for usage like "const A a = getA()+3"
            continue;

        const Function* func = tok->function();
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
                "the unnecessary data copying by converting '" + varname + "' to const reference.");
}

//---------------------------------------------------------------------------
// Checking for shift by negative values
//---------------------------------------------------------------------------

void CheckOther::checkNegativeBitwiseShift()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {

            if ((Token::Match(tok,"%var% >>|<< %num%") || Token::Match(tok,"%num% >>|<< %num%")) && !Token::Match(tok->previous(),">>|<<")) {
                if (tok->isName()) {
                    const Variable *var = tok->variable();
                    if (var && var->typeStartToken()->isStandardType() && (tok->strAt(2))[0] == '-')
                        negativeBitwiseShiftError(tok);
                } else {
                    if ((tok->strAt(2))[0] == '-')
                        negativeBitwiseShiftError(tok);
                }
            }
        }
    }
}


void CheckOther::negativeBitwiseShiftError(const Token *tok)
{
    reportError(tok, Severity::error, "shiftNegative", "Shifting by a negative value.");
}


//---------------------------------------------------------------------------
// Check for incompletely filled buffers.
//---------------------------------------------------------------------------
void CheckOther::checkIncompleteArrayFill()
{
    bool warning = _settings->isEnabled("warning");
    bool portability = _settings->isEnabled("portability");
    if (!_settings->inconclusive || (!portability && !warning))
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
                    if ((size != 1 && size != 100 && size != 0) || var->isPointer()) {
                        if (warning)
                            incompleteArrayFillError(tok, var->name(), tok->str(), false);
                    } else if (var->typeStartToken()->str() == "bool" && portability) // sizeof(bool) is not 1 on all platforms
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
                    "The array '" + buffer + "' is filled incompletely. The function '" + function + "()' needs the size given in bytes, but the type 'bool' is larger than 1 on some platforms. Did you forget to multiply the size with 'sizeof(*" + buffer + ")'?", true);
    else
        reportError(tok, Severity::warning, "incompleteArrayFill",
                    "Array '" + buffer + "' is filled incompletely. Did you forget to multiply the size given to '" + function + "()' with 'sizeof(*" + buffer + ")'?\n"
                    "The array '" + buffer + "' is filled incompletely. The function '" + function + "()' needs the size given in bytes, but an element of the given array is larger than one byte. Did you forget to multiply the size with 'sizeof(*" + buffer + ")'?", true);
}


void CheckOther::oppositeInnerCondition()
{
    // FIXME: This check is experimental because of #4170 and #4186. Fix those tickets and remove the "experimental".
    if (!_settings->isEnabled("warning") || !_settings->inconclusive || !_settings->experimental)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        const Token* const toke = scope->classDef;


        if (scope->type == Scope::eIf && toke) {

            const Token *op1Tok, *op2Tok;
            op1Tok = scope->classDef->tokAt(2);
            op2Tok = scope->classDef->tokAt(4);

            if (scope->classDef->strAt(6) == "{") {

                const char *oppositeCondition = NULL;

                if (scope->classDef->strAt(3) == "==")
                    oppositeCondition = "if ( %any% !=|<|>|<=|>= %any% )";
                else if (scope->classDef->strAt(3) == "!=")
                    oppositeCondition = "if ( %any% ==|>=|<= %any% )";
                else if (scope->classDef->strAt(3) == "<")
                    oppositeCondition = "if ( %any% >|>=|== %any% )";
                else if (scope->classDef->strAt(3) == "<=")
                    oppositeCondition = "if ( %any% > %any% )";
                else if (scope->classDef->strAt(3) == ">")
                    oppositeCondition = "if ( %any% <|<=|== %any% )";
                else if (scope->classDef->strAt(3) == ">=")
                    oppositeCondition = "if ( %any% < %any% )";

                if (oppositeCondition) {
                    int flag = 0;

                    for (const Token* tok = scope->classStart; tok != scope->classEnd && flag == 0; tok = tok->next()) {
                        if ((tok->str() == op1Tok->str() || tok->str() == op2Tok->str()) && tok->strAt(1) == "=")
                            break;
                        else if (Token::Match(tok, "%any% ( %any% )")) {
                            if ((tok->strAt(2) == op1Tok->str() || tok->strAt(2) == op2Tok->str()))
                                break;
                        } else if (Token::Match(tok, "%any% ( %any% , %any%")) {
                            for (const Token* tok2 = tok->next(); tok2 != tok->linkAt(1); tok2 = tok2->next()) {
                                if (tok2->str() == op1Tok->str()) {
                                    flag = 1;
                                    break;
                                }
                            }
                        } else if (Token::Match(tok, oppositeCondition)) {
                            if ((tok->strAt(2) == op1Tok->str() && tok->strAt(4) == op2Tok->str()) || (tok->strAt(2) == op2Tok->str() && tok->strAt(4) == op1Tok->str()))
                                oppositeInnerConditionError(toke);
                        }
                    }
                }
            }
        }
    }
}

void CheckOther::oppositeInnerConditionError(const Token *tok)
{
    reportError(tok, Severity::warning, "oppositeInnerCondition", "Opposite conditions in nested 'if' blocks lead to a dead code block.", true);
}


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
                ftok = ftok ? ftok->previous() : NULL;
                if (ftok && ftok->isName()) {
                    // If this is a variadic function then report error
                    const Function *f = ftok->function();
                    if (f && f->argCount() <= argnr) {
                        const Token *tok2 = f->argDef;
                        tok2 = tok2 ? tok2->link() : NULL; // goto ')'
                        if (Token::simpleMatch(tok2->tokAt(-3), ". . ."))
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
                "        printf(\"%018p, %s\n\", p, (long)p & 255 ? p : \"\");\n"
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
                "}");
}
