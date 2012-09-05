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
#include "checkother.h"
#include "mathlib.h"
#include "symboldatabase.h"

#include <cmath> // fabs()
#include <stack>
#include <algorithm> // find_if()
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckOther instance;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkIncrementBoolean()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% ++")) {
            if (tok->varId()) {
                const Variable *var = symbolDatabase->getVariableFromVarId(tok->varId());

                if (var && var->typeEndToken()->str() == "bool")
                    incrementBooleanError(tok);
            }
        }
    }
}

void CheckOther::incrementBooleanError(const Token *tok)
{
    reportError(
        tok,
        Severity::style,
        "incrementboolean",
        "The use of a variable of type bool with the ++ postfix operator is always true and deprecated by the C++ Standard.\n"
        "The operand of a postfix increment operator may be of type bool but it is deprecated by C++ Standard (Annex D-1) and the operand is always set to true."
    );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::clarifyCalculation()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "?" && tok->previous()) {
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
                "Clarify calculation precedence for " + op + " and ?\n"
                "Suspicious calculation. Please use parentheses to clarify the code. "
                "The code " + calc + " should be written as either " + s1 + " or " + s2 + ".");
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

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
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

    // using boolean result in bitwise operation ! x [&|^]
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "!|<|<=|==|!=|>|>=")) {
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

void CheckOther::clarifyConditionError(const Token *tok, bool assign, bool boolop)
{
    std::string errmsg;

    if (assign)
        errmsg = "Suspicious condition (assignment+comparison), it can be clarified with parentheses";

    else if (boolop)
        errmsg = "Boolean result is used in bitwise operation. Clarify expression with parentheses\n"
                 "Suspicious expression. Boolean result is used in bitwise operation. The ! operator "
                 "and the comparison operators have higher precedence than bitwise operators. "
                 "It is recommended that the expression is clarified with parentheses.";
    else
        errmsg = "Suspicious condition (bitwise operator + comparison), it can be clarified with parentheses\n"
                 "Suspicious condition. Comparison operators have higher precedence than bitwise operators. Please clarify the condition with parentheses.";

    reportError(tok,
                Severity::style,
                "clarifyCondition",
                errmsg);
}

//---------------------------------------------------------------------------
// Clarify (meaningless) statements like *foo++; with parantheses.
//---------------------------------------------------------------------------
void CheckOther::clarifyStatement()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "* %var%")) {
            const Token *tok2=tok->previous();

            while (Token::Match(tok2, "*"))
                tok2=tok2->previous();

            if (Token::Match(tok2, "[{};]")) {
                tok = tok->tokAt(2);
                while (tok) {
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
                    //TODO: change the string in order to remove the excessive spaces between the tokens.
                    clarifyStatementError(tok,
                                          tok2->next()->stringifyList(tok->tokAt(2)),
                                          "("+tok2->next()->stringifyList(tok)+")"+tok->stringifyList(tok->tokAt(2)));
            }
        }
    }
}

void CheckOther::clarifyStatementError(const Token *tok, const std::string &expr, const std::string &suggested)
{
    reportError(tok, Severity::warning, "clarifyStatement", "Ineffective statement: '" + expr + "'. Did you intend to write '" + suggested + "'?\n"
                "A statement like '*expr++;' might not do what you intended. 'operator*' is executed before postfix 'operator++'. "
                "Thus, the dereference is meaningless. Did you intend to write '(*expr)++;'?");
}

//---------------------------------------------------------------------------
// if (bool & bool) -> if (bool && bool)
// if (bool | bool) -> if (bool || bool)
//---------------------------------------------------------------------------
void CheckOther::checkBitwiseOnBoolean()
{
    if (!_settings->isEnabled("style"))
        return;

    // danmar: this is inconclusive because I don't like that there are
    //         warnings for calculations. Example: set_flag(a & b);
    if (!_settings->inconclusive)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "(|.|return|&&|%oror%|throw|, %var% [&|]")) {
            const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok->next()->varId());
            if (var && var->typeEndToken()->str() == "bool") {
                bitwiseOnBooleanError(tok->next(), var->name(), tok->strAt(2) == "&" ? "&&" : "||");
                tok = tok->tokAt(2);
            }
        } else if (Token::Match(tok, "[&|] %var% )|.|return|&&|%oror%|throw|,") && (!tok->previous() || !tok->previous()->isExtendedOp() || tok->strAt(-1) == ")")) {
            const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok->next()->varId());
            if (var && var->typeEndToken()->str() == "bool") {
                bitwiseOnBooleanError(tok->next(), var->name(), tok->str() == "&" ? "&&" : "||");
                tok = tok->tokAt(2);
            }
        }
    }
}

void CheckOther::bitwiseOnBooleanError(const Token *tok, const std::string &varname, const std::string &op)
{
    reportError(tok, Severity::style, "bitwiseOnBoolean",
                "Boolean variable '" + varname + "' is used in bitwise operation. Did you mean " + op + " ?", true);
}

void CheckOther::checkSuspiciousSemicolon()
{
    if (!_settings->inconclusive || !_settings->isEnabled("style"))
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
                SuspiciousSemicolonError(i->classStart);
            }
        }
    }
}

void CheckOther::SuspiciousSemicolonError(const Token* tok)
{
    reportError(tok, Severity::warning, "suspiciousSemicolon",
                "Suspicious use of ; at the end of 'if/for/while' statement.", true);
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
    if (!_settings->isEnabled("style") && !_settings->isEnabled("portability"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        const Token* toTok = 0;
        const Token* nextTok = 0;
        // Find cast
        if (Token::Match(tok, "( const| %type% const| * )") || Token::Match(tok, "( const| %type% %type% const| * )")) {
            toTok = tok->next();
            nextTok = tok->link()->next();
            if (nextTok && nextTok->str() == "(")
                nextTok = nextTok->next();
        } else if (Token::Match(tok, "reinterpret_cast < const| %type% const| * > (") || Token::Match(tok, "reinterpret_cast < const| %type% %type% const| * > (")) {
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
        unsigned int varid = 0;
        bool allocation = false;
        bool ref = false;
        if (Token::Match(nextTok, "new %type%"))
            allocation = true;
        else if (Token::Match(nextTok, "%var% !!["))
            varid = nextTok->varId();
        else if (Token::Match(nextTok, "& %var%") && !Token::Match(nextTok->tokAt(2), "(|[")) {
            varid = nextTok->next()->varId();
            ref = true;
        }

        const Token* fromTok = 0;

        if (allocation) {
            fromTok = nextTok->next();
        } else {
            const Variable* var = symbolDatabase->getVariableFromVarId(varid);
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

void CheckOther::invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to, bool inconclusive)
{
    if (to == "integer") { // If we cast something to int*, this can be useful to play with its binary data representation
        if (!inconclusive)
            reportError(tok, Severity::portability, "invalidPointerCast", "Casting from " + from + "* to integer* is not portable due to different binary data representations on different platforms");
        else
            reportError(tok, Severity::portability, "invalidPointerCast", "Casting from " + from + "* to char* might be not portable due to different binary data representations on different platforms", true);
    } else
        reportError(tok, Severity::warning, "invalidPointerCast", "Casting between " + from + "* and " + to + "* which have an incompatible binary data representation");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkSizeofForNumericParameter()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof ( %num% )")
            || Token::Match(tok, "sizeof %num%")
           ) {
            sizeofForNumericParameterError(tok);
        }
    }
}

void CheckOther::sizeofForNumericParameterError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "sizeofwithnumericparameter", "Using sizeof with a numeric constant as function "
                "argument might not be what you intended.\n"
                "It is unusual to use constant value with sizeof. For example, sizeof(10)"
                " returns 4 (in 32-bit systems) or 8 (in 64-bit systems) instead of 10. sizeof('A')"
                " and sizeof(char) can return different results.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkSizeofForArrayParameter()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof ( %var% )") || Token::Match(tok, "sizeof %var% !![")) {
            const Token* varTok = tok->next();
            if (varTok->str() == "(") {
                varTok = varTok->next();
            }
            if (varTok->varId() > 0) {
                const Variable *var = symbolDatabase->getVariableFromVarId(varTok->varId());
                if (var && var->isArray() && var->isArgument()) {
                    sizeofForArrayParameterError(tok);
                }
            }
        }
    }
}

void CheckOther::sizeofForArrayParameterError(const Token *tok)
{
    reportError(tok, Severity::error,
                "sizeofwithsilentarraypointer", "Using sizeof for array given as function argument "
                "returns the size of pointer.\n"
                "Giving array as function parameter and then using sizeof-operator for the array "
                "argument. In this case the sizeof-operator returns the size of pointer (in the "
                "system). It does not return the size of the whole array in bytes as might be "
                "expected. For example, this code:\n"
                "     int f(char a[100]) {\n"
                "         return sizeof(a);\n"
                "     }\n"
                " returns 4 (in 32-bit systems) or 8 (in 64-bit systems) instead of 100 (the "
                "size of the array in bytes)."
               );
}

void CheckOther::checkSizeofForPointerSize()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
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
        const Variable *var = symbolDatabase->getVariableFromVarId(variable->varId());
        if (!var || !var->isPointer() || var->isArray()) {
            variable = 0;
        }

        if (variable2) {
            var = symbolDatabase->getVariableFromVarId(variable2->varId());
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

void CheckOther::sizeofForPointerError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "pointerSize",
                "Using size of pointer " + varname + " instead of size of its data.\n"
                "Using size of pointer " + varname + " instead of size of its data. "
                "This is likely to lead to a buffer overflow. You probably intend to "
                "write sizeof(*" + varname + ")", true);
}

//---------------------------------------------------------------------------
// Detect redundant assignments: x = 0; x = 4;
//---------------------------------------------------------------------------

static void eraseNotLocalArg(std::map<unsigned int, const Token*>& container, const SymbolDatabase* symbolDatabase)
{
    for (std::map<unsigned int, const Token*>::iterator i = container.begin(); i != container.end();) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i->first);
        if (!var || (!var->isLocal() && !var->isArgument())) {
            container.erase(i++);
            if (i == container.end())
                break;
        } else
            ++i;
    }
}

void CheckOther::checkRedundantAssignment()
{
    if (!_settings->isEnabled("performance"))
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (!scope->isExecutable())
            continue;

        ///std::cout << std::endl << "scope: " << i->className << std::endl;

        std::map<unsigned int, const Token*> varAssignments;
        std::map<unsigned int, const Token*> memAssignments;
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
                std::map<unsigned int, const Token*>::iterator it = varAssignments.find(tok->varId());
                if (tok->next()->isAssignmentOp() && Token::Match(tok->previous(), "[;{}]")) { // Assignment
                    ///std::cout << "assign: " << tok->varId() << std::endl;
                    if (it != varAssignments.end()) {
                        bool error = true; // Ensure that variable is not used on right side
                        for (const Token* tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                            if (tok2->str() == ";")
                                break;
                            else if (tok2->varId() == tok->varId())
                                error = false;
                        }
                        if (error) {
                            if (scope->type == Scope::eSwitch && Token::findmatch(it->second, "default|case", tok))
                                redundantAssignmentInSwitchError(it->second, tok, tok->str());
                            else
                                redundantAssignmentError(it->second, tok, tok->str());
                        }
                        it->second = tok;
                    }
                    varAssignments[tok->varId()] = tok;
                    memAssignments.erase(tok->varId());
                } else if (tok->next()->type() == Token::eIncDecOp || (tok->previous()->type() == Token::eIncDecOp && !Token::Match(tok->next(), ".|[|("))) { // Variable incremented/decremented
                    varAssignments[tok->varId()] = tok;
                    memAssignments.erase(tok->varId());
                } else if (!Token::Match(tok->tokAt(-2), "sizeof (")) { // Other usage of variable
                    ///std::cout << "use: " << tok->varId() << std::endl;
                    if (it != varAssignments.end())
                        varAssignments.erase(it);
                    if (!writtenArgumentsEnd) // Indicates that we are in the first argument of strcpy/memcpy/... function
                        memAssignments.erase(tok->varId());
                }
            } else if (Token::Match(tok, "%var% (")) { // Function call. Global variables might be used. Reset their status
                bool memfunc = Token::Match(tok, "memcpy|memmove|memset|strcpy|strncpy|sprintf|snprintf|strcat|strncat");
                if (memfunc) {
                    const Token* param1 = tok->tokAt(2);
                    writtenArgumentsEnd = param1->next();
                    if (param1->varId() && param1->strAt(1) == "," && tok->str() != "strcat" && tok->str() != "strncat") {
                        std::map<unsigned int, const Token*>::iterator it = memAssignments.find(param1->varId());
                        if (it == memAssignments.end())
                            memAssignments[param1->varId()] = tok;
                        else {
                            if (scope->type == Scope::eSwitch && Token::findmatch(it->second, "default|case", tok))
                                redundantCopyInSwitchError(it->second, tok, param1->str());
                            else
                                redundantCopyError(it->second, tok, param1->str());
                        }
                    }
                } else if (scope->type == Scope::eSwitch) { // Avoid false positives if noreturn function is called in switch
                    const Function* func = symbolDatabase->findFunctionByToken(_tokenizer->getFunctionTokenByName(tok->str().c_str()));
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
                "Buffer '" + var + "' is being written before its old content has been used. This might indicate a missing 'break;'.");
}

void CheckOther::redundantAssignmentError(const Token *tok1, const Token* tok2, const std::string& var)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok1);
    callstack.push_back(tok2);
    reportError(callstack, Severity::performance, "redundantAssignment",
                "Variable '" + var + "' is reassigned a value before the old one has been used.");
}

void CheckOther::redundantAssignmentInSwitchError(const Token *tok1, const Token* tok2, const std::string &var)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok1);
    callstack.push_back(tok2);
    reportError(callstack, Severity::warning, "redundantAssignInSwitch",
                "Variable '" + var + "' is reassigned a value before the old one has been used. This might indicate a missing 'break;'.");
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
void CheckOther::checkRedundantAssignmentInSwitch()
{
    if (!_settings->isEnabled("style"))
        return;

    const char breakPattern[] = "break|continue|return|exit|goto|throw";
    const char functionPattern[] = "%var% (";

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
                        } else if (Token::Match(tok3, functionPattern) || Token::Match(tok3, breakPattern)) {
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
            if (Token::Match(tok2, functionPattern) || Token::Match(tok2, breakPattern)) {
                varsWithBitsSet.clear();
                bitOperations.clear();
            }
        }
    }
}

void CheckOther::redundantBitwiseOperationInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "redundantBitwiseOperationInSwitch", "Redundant bitwise operation on \"" + varname + "\" in switch");
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkSwitchCaseFallThrough()
{
    if (!(_settings->isEnabled("style") && _settings->experimental))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    const char breakPattern[] = "break|continue|return|exit|goto|throw";

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
            } else if (Token::Match(tok2, breakPattern)) {
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
                "switchCaseFallThrough", "Switch falls through case without comment");
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

void CheckOther::checkSelfAssignment()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    const char selfAssignmentPattern[] = "%var% = %var% ;|=|)";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), selfAssignmentPattern);
    while (tok) {
        if (Token::Match(tok->previous(), "[;{}]") &&
            tok->varId() && tok->varId() == tok->tokAt(2)->varId() &&
            isTypeWithoutSideEffects(_tokenizer, symbolDatabase->getVariableFromVarId(tok->varId()))) {
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

        tok = Token::findmatch(tok->next(), selfAssignmentPattern);
    }
}

void CheckOther::selfAssignmentError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "selfAssignment", "Redundant assignment of \"" + varname + "\" to itself");
}

//---------------------------------------------------------------------------
//    int a = 1;
//    assert(a = 2);            // <- assert should not have a side-effect
//---------------------------------------------------------------------------
void CheckOther::checkAssignmentInAssert()
{
    if (!_settings->isEnabled("style"))
        return;

    const char assertPattern[] = "assert ( %any%";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), assertPattern);
    const Token *endTok = tok ? tok->next()->link() : NULL;

    while (tok && endTok) {
        for (tok = tok->tokAt(2); tok != endTok; tok = tok->next()) {
            if (tok->isName() && (tok->next()->isAssignmentOp() || tok->next()->type() == Token::eIncDecOp))
                assignmentInAssertError(tok, tok->str());
            else if (Token::Match(tok, "--|++ %var%"))
                assignmentInAssertError(tok, tok->strAt(1));
        }

        tok = Token::findmatch(endTok->next(), assertPattern);
        endTok = tok ? tok->next()->link() : NULL;
    }
}

void CheckOther::assignmentInAssertError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "assignmentInAssert", "Assert statement modifies '" + varname + "'.\n"
                "Variable '" + varname + "' is modified insert assert statement. "
                "Assert statements are removed from release builds so the code inside "
                "assert statement is not run. If the code is needed also in release "
                "builds this is a bug.");
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
    if (!_settings->isEnabled("style"))
        return;

    for (const Token* tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Find a pair of comparison expressions with or without parenthesis
        // with a shared variable and constants and with a logical operator between them.
        // e.g. if (x != 3 || x != 4)
        const Token *term1Tok = NULL, *term2Tok = NULL;
        const Token *op1Tok = NULL, *op2Tok = NULL, *op3Tok = NULL, *nextTok = NULL;

        if (Token::Match(tok, "( %any% !=|==|<|>|>=|<= %any% ) &&|%oror%")) {
            term1Tok = tok->next();
            op1Tok = tok->tokAt(2);
            op2Tok = tok->tokAt(5);
        } else if (Token::Match(tok, "%any% !=|==|<|>|>=|<= %any% &&|%oror%")) {
            term1Tok = tok;
            op1Tok = tok->next();
            op2Tok = tok->tokAt(3);
        }
        if (op2Tok) {
            if (Token::Match(op2Tok->next(), "( %any% !=|==|<|>|>=|<= %any% ) %any%")) {
                term2Tok = op2Tok->tokAt(2);
                op3Tok = op2Tok->tokAt(3);
                nextTok = op2Tok->tokAt(6);
            } else if (Token::Match(op2Tok->next(), "%any% !=|==|<|>|>=|<= %any% %any%")) {
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
                        const std::string text = cond1str + " " + op2Tok->str() + " " + cond2str;
                        incorrectLogicOperatorError(term1Tok, text, conditions[i].error == AlwaysTrue);
                    } else {
                        const std::string text = "If " + cond1str + ", the comparison " + cond2str +
                                                 " is always " + ((conditions[i].error == SecondTrue || conditions[i].error == AlwaysTrue) ? "true" : "false") + ".";
                        redundantConditionError(term1Tok, text);
                    }
                    break;
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
                    "Are these conditions necessary? Did you intend to use && instead? Are the numbers correct? Are you comparing the correct variables?");
    else
        reportError(tok, Severity::warning, "incorrectLogicOperator",
                    "Logical conjunction always evaluates to false: " + condition + ".\n"
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
        if (!Token::Match(tok, "strtol|strtoul ("))
            continue;

        tok = tok->tokAt(2);
        // Locate the third parameter of the function call..
        for (int i = 0; i < 2 && tok; i++)
            tok = tok->nextArgument();

        if (Token::Match(tok, "%num% )")) {
            const MathLib::bigint radix = MathLib::toLongNumber(tok->str());
            if (!(radix == 0 || (radix >= 2 && radix <= 36))) {
                dangerousUsageStrtolError(tok);
            }
        } else
            break;
    }

    // sprintf|snprintf overlapping data
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Get variable id of target buffer..
        unsigned int varid = 0;

        if (Token::Match(tok, "sprintf|snprintf ( %var% ,"))
            varid = tok->tokAt(2)->varId();

        else if (Token::Match(tok, "sprintf|snprintf ( %var% . %var% ,"))
            varid = tok->tokAt(4)->varId();

        if (varid == 0)
            continue;

        // goto ","
        const Token *tok2 = tok->tokAt(3);
        while (tok2->str() != ",")
            tok2 = tok2->next();

        tok2 = tok2->next(); // Jump behind ","

        if (tok->str() == "snprintf") { // Jump over second parameter for snprintf
            tok2 = tok2->nextArgument();
            if (!tok2)
                continue;
        }

        // is any source buffer overlapping the target buffer?
        do {
            if (Token::Match(tok2, "%varid% [,)]", varid)) {
                sprintfOverlappingDataError(tok2->next(), tok2->next()->str());
                break;
            }
        } while (NULL != (tok2 = tok2->nextArgument()));
    }
}

void CheckOther::dangerousUsageStrtolError(const Token *tok)
{
    reportError(tok, Severity::error, "dangerousUsageStrtol", "Invalid radix in call to strtol or strtoul. Must be 0 or 2-36");
}

void CheckOther::sprintfOverlappingDataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "sprintfOverlappingData",
                "Undefined behavior: variable is used as parameter and destination in s[n]printf().\n"
                "The variable '" + varname + "' is used both as a parameter and as a destination in "
                "s[n]printf(). The origin and destination buffers overlap. Quote from glibc (C-library) "
                "documentation (http://www.gnu.org/software/libc/manual/html_mono/libc.html#Formatted-Output-Functions): "
                "'If copying takes place between objects that overlap as a result of a call "
                "to sprintf() or snprintf(), the results are undefined.'");
}

//---------------------------------------------------------------------------
//    if (!x==3) <- Probably meant to be "x!=3"
//---------------------------------------------------------------------------

static bool isBool(const Variable* var)
{
    return(var && var->typeEndToken()->str() == "bool");
}
static bool isNonBoolStdType(const Variable* var)
{
    return(var && var->typeEndToken()->isStandardType() && var->typeEndToken()->str() != "bool");
}
void CheckOther::checkComparisonOfBoolWithInt()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->next() && tok->next()->type() == Token::eComparisonOp && (!tok->previous() || !tok->previous()->isArithmeticalOp()) && (!tok->tokAt(3) || !tok->tokAt(3)->isArithmeticalOp())) {
            const Token* const right = tok->tokAt(2);
            if ((tok->varId() && right->isNumber()) || (tok->isNumber() && right->varId())) { // Comparing variable with number
                const Token* varTok = tok;
                const Token* numTok = right;
                if (tok->isNumber() && right->varId()) // num with var
                    std::swap(varTok, numTok);
                if (isBool(symbolDatabase->getVariableFromVarId(varTok->varId())) && // Variable has to be a boolean
                    ((tok->strAt(1) != "==" && tok->strAt(1) != "!=") ||
                     (MathLib::toLongNumber(numTok->str()) != 0 && MathLib::toLongNumber(numTok->str()) != 1))) { // == 0 and != 0 are allowed, for C also == 1 and != 1
                    comparisonOfBoolWithIntError(varTok, numTok->str(), tok->strAt(1) == "==" || tok->strAt(1) == "!=");
                }
            } else if (tok->isBoolean() && right->varId()) { // Comparing boolean constant with variable
                if (isNonBoolStdType(symbolDatabase->getVariableFromVarId(right->varId()))) { // Variable has to be of non-boolean standard type
                    comparisonOfBoolWithIntError(right, tok->str(), false);
                } else if (tok->strAt(1) != "==" && tok->strAt(1) != "!=") {
                    comparisonOfBoolWithInvalidComparator(right, tok->str());
                }
            } else if (tok->varId() && right->isBoolean()) { // Comparing variable with boolean constant
                if (isNonBoolStdType(symbolDatabase->getVariableFromVarId(tok->varId()))) { // Variable has to be of non-boolean standard type
                    comparisonOfBoolWithIntError(tok, right->str(), false);
                } else if (tok->strAt(1) != "==" && tok->strAt(1) != "!=") {
                    comparisonOfBoolWithInvalidComparator(right, tok->str());
                }
            } else if (tok->isNumber() && right->isBoolean()) { // number constant with boolean constant
                comparisonOfBoolWithIntError(tok, right->str(), false);
            } else if (tok->isBoolean() && right->isNumber()) { // number constant with boolean constant
                comparisonOfBoolWithIntError(tok, tok->str(), false);
            } else if (tok->varId() && right->varId()) { // Comparing two variables, one of them boolean, one of them integer
                const Variable* var1 = symbolDatabase->getVariableFromVarId(right->varId());
                const Variable* var2 = symbolDatabase->getVariableFromVarId(tok->varId());
                if (isBool(var1) && isNonBoolStdType(var2)) // Comparing boolean with non-bool standard type
                    comparisonOfBoolWithIntError(tok, var1->name(), false);
                else if (isNonBoolStdType(var1) && isBool(var2)) // Comparing non-bool standard type with boolean
                    comparisonOfBoolWithIntError(tok, var2->name(), false);
            }
        }
    }
}

void CheckOther::comparisonOfBoolWithIntError(const Token *tok, const std::string &expression, bool n0o1)
{
    if (n0o1)
        reportError(tok, Severity::warning, "comparisonOfBoolWithInt",
                    "Comparison of a boolean with an integer that is neither 1 nor 0\n"
                    "The expression \"" + expression + "\" is of type 'bool' "
                    "and it is compared against a integer value that is "
                    "neither 1 nor 0.");
    else
        reportError(tok, Severity::warning, "comparisonOfBoolWithInt",
                    "Comparison of a boolean with an integer\n"
                    "The expression \"" + expression + "\" is of type 'bool' "
                    "and it is compared against a integer value.");
}

void CheckOther::comparisonOfBoolWithInvalidComparator(const Token *tok, const std::string &expression)
{
    reportError(tok, Severity::warning, "comparisonOfBoolWithInvalidComparator",
                "Comparison of a boolean value using relational (<, >, <= or >=) operator.\n"
                "The expression \"" + expression + "\" is of type 'bool' "
                "and result is of type 'bool'. Comparing 'bool' value using relational (<, >, <= or >=)"
                " operator could cause unexpected results.");
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
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
                if (tok2->str() == ";") {
                    secondBreak = tok2->next();
                    break;
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
                "The second of the two statements can never be executed, and so should be removed.", inconclusive);
}

void CheckOther::unreachableCodeError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "unreachableCode",
                "Statements following return, break, continue, goto or throw will never be executed.", inconclusive);
}

//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------
static bool isUnsigned(const Variable* var)
{
    return(var && var->typeStartToken()->isUnsigned() && !var->isPointer() && !var->isArray());
}
static bool isSigned(const Variable* var)
{
    return(var && !var->typeStartToken()->isUnsigned() && Token::Match(var->typeEndToken(), "int|char|short|long") && !var->isPointer() && !var->isArray());
}

void CheckOther::checkUnsignedDivision()
{
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    bool style = _settings->isEnabled("style");

    const Token* ifTok = 0;
    // Check for "ivar / uvar" and "uvar / ivar"
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[).]")) // Don't check members or casted variables
            continue;

        if (Token::Match(tok->next(), "%var% / %num%")) {
            if (tok->strAt(3)[0] == '-' && isUnsigned(symbolDatabase->getVariableFromVarId(tok->next()->varId()))) {
                udivError(tok->next(), false);
            }
        } else if (Token::Match(tok->next(), "%num% / %var%")) {
            if (tok->strAt(1)[0] == '-' && isUnsigned(symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId()))) {
                udivError(tok->next(), false);
            }
        } else if (Token::Match(tok->next(), "%var% / %var%") && _settings->inconclusive && style && !ifTok) {
            const Variable* var1 = symbolDatabase->getVariableFromVarId(tok->next()->varId());
            const Variable* var2 = symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId());
            if ((isUnsigned(var1) && isSigned(var2)) || (isUnsigned(var2) && isSigned(var1))) {
                udivError(tok->next(), true);
            }
        } else if (!ifTok && Token::simpleMatch(tok, "if ("))
            ifTok = tok->next()->link()->next()->link();
        else if (ifTok == tok)
            ifTok = 0;
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
    if (!_settings->isEnabled("style"))
        return;

    for (const Token* tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "memset (")) {
            const Token* lastParamTok = tok->next()->link()->previous();
            if (lastParamTok->str() == "0")
                memsetZeroBytesError(tok, tok->strAt(2));
        }
    }
}

void CheckOther::memsetZeroBytesError(const Token *tok, const std::string &varname)
{
    const std::string summary("memset() called to fill 0 bytes of \'" + varname + "\'");
    const std::string verbose(summary + ". Second and third arguments might be inverted.");
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
        lookupVar(tok, var);
    }
}

void CheckOther::lookupVar(const Token *tok, const Variable* var)
{
    // Skip the variable declaration..
    while (tok && tok->str() != ";")
        tok = tok->next();

    // Check if the variable is used in this indentlevel..
    bool used1 = false;   // used in one sub-scope -> reducable
    bool used2 = false;   // used in more sub-scopes -> not reducable
    unsigned int indentlevel = 0;
    int parlevel = 0;
    bool for_or_while = false;  // is sub-scope a "for/while/etc". anything that is not "if"
    while (tok) {
        if (tok->str() == "{") {
            if (tok->strAt(-1) == "=") {
                if (Token::findmatch(tok, "%varid%", tok->link(), var->varId())) {
                    return;
                }

                tok = tok->link();
            } else
                ++indentlevel;
        }

        else if (tok->str() == "}") {
            if (indentlevel == 0)
                break;
            --indentlevel;
            if (indentlevel == 0) {
                if (for_or_while && used2)
                    return;
                used2 |= used1;
                used1 = false;
            }
        }

        else if (tok->str() == "(") {
            ++parlevel;
        }

        else if (tok->str() == ")") {
            --parlevel;
        }

        // Bail out if references are used
        else if (Token::Match(tok, "& %varid%", var->varId())) {
            return;
        }

        else if (tok->varId() == var->varId()) {
            if (indentlevel == 0)
                return;
            if (tok->strAt(-1) == "=" && (var->isArray() || var->isPointer())) // Create a copy of array/pointer. Bailout, because the memory it points to might be necessary in outer scope
                return;
            used1 = true;
            if (for_or_while && tok->strAt(1) != "=")
                used2 = true;
            if (used1 && used2)
                return;
        }

        else if (indentlevel == 0) {
            // %unknown% ( %any% ) {
            // If %unknown% is anything except if, we assume
            // that it is a for or while loop or a macro hiding either one
            if (tok->strAt(1) == "(" &&
                Token::simpleMatch(tok->next()->link(), ") {")) {
                if (tok->str() != "if")
                    for_or_while = true;
            }

            else if (Token::simpleMatch(tok, "do {"))
                for_or_while = true;

            // possible unexpanded macro hiding for/while..
            else if (tok->str() != "else" && Token::Match(tok->previous(), "[;{}] %type% {")) {
                for_or_while = true;
            }

            if (parlevel == 0 && (tok->str() == ";"))
                for_or_while = false;
        }

        tok = tok->next();
    }

    // Warning if this variable:
    // * not used in this indentlevel
    // * used in lower indentlevel
    if (used1 || used2)
        variableScopeError(var->nameToken(), var->name());
}

void CheckOther::variableScopeError(const Token *tok, const std::string &varname)
{
    reportError(tok,
                Severity::style,
                "variableScope",
                "The scope of the variable '" + varname + "' can be reduced\n"
                "The scope of the variable '" + varname + "' can be reduced. Warning: It can be unsafe "
                "to fix this message. Be careful. Especially when there are inner loops. Here is an "
                "example where cppcheck will write that the scope for 'i' can be reduced:\n"
                "void f(int x)\n"
                "{\n"
                "    int i = 0;\n"
                "    if (x) {\n"
                "        // it's safe to move 'int i = 0' here\n"
                "        for (int n = 0; n < 10; ++n) {\n"
                "            // it is possible but not safe to move 'int i = 0' here\n"
                "            do_something(&i);\n"
                "        }\n"
                "    }\n"
                "}\n"
                "When you see this message it is always safe to reduce the variable scope 1 level.");
}

//---------------------------------------------------------------------------
// Check for constant function parameters
//---------------------------------------------------------------------------
void CheckOther::checkConstantFunctionParameter()
{
    if (!_settings->isEnabled("performance"))
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
        } else if (Token::Match(tok, "std :: %type% <") && !Token::Match(tok->linkAt(3), "> ::")) {
            passedByValueError(tok, var->name());
        } else if (var->type() || symbolDatabase->isClassOrStruct(tok->str())) {  // Check if type is a struct or class.
            passedByValueError(tok, var->name());
        }
    }
}

void CheckOther::passedByValueError(const Token *tok, const std::string &parname)
{
    reportError(tok, Severity::performance, "passedByValue",
                "Function parameter '" + parname + "' should be passed by reference.\n"
                "Parameter '" +  parname + "' is passed as a value. It could be passed "
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
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if ((tok->str() != ".") && Token::Match(tok->next(), "%var% [ %var% ]")) {
            const Variable* arrayvar = symbolDatabase->getVariableFromVarId(tok->next()->varId());
            const Variable* indexvar = symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId());
            const MathLib::bigint arraysize = (arrayvar && arrayvar->isArray()) ? arrayvar->dimension(0U) : 0;
            if (isSignedChar(indexvar) && arraysize > 0x80)
                charArrayIndexError(tok->next());
        }

        else if (Token::Match(tok, "[;{}] %var% = %any% [&^|] %any% ;")) {
            // is a char variable used in the calculation?
            if (!isSignedChar(symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId())) &&
                !isSignedChar(symbolDatabase->getVariableFromVarId(tok->tokAt(5)->varId())))
                continue;

            // it's ok with a bitwise and where the other operand is 0xff or less..
            if (tok->strAt(4) == "&") {
                if (tok->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok->strAt(3)))
                    continue;
                if (tok->tokAt(5)->isNumber() && MathLib::isGreater("0x100", tok->strAt(5)))
                    continue;
            }

            // is the result stored in a short|int|long?
            const Variable *var = symbolDatabase->getVariableFromVarId(tok->next()->varId());
            if (var && Token::Match(var->typeStartToken(), "short|int|long") && !var->isPointer() && !var->isArray())
                charBitOpError(tok->tokAt(4)); // This is an error..
        }

        else if (Token::Match(tok, "[;{}] %var% = %any% [&^|] ( * %var% ) ;")) {
            const Variable* var = symbolDatabase->getVariableFromVarId(tok->tokAt(7)->varId());
            if (!var || !var->isPointer() || var->typeStartToken()->str() != "char" || var->typeStartToken()->isUnsigned())
                continue;
            // it's ok with a bitwise and where the other operand is 0xff or less..
            if (tok->strAt(4) == "&" && tok->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok->strAt(3)))
                continue;

            // is the result stored in a short|int|long?
            var = symbolDatabase->getVariableFromVarId(tok->next()->varId());
            if (var && Token::Match(var->typeStartToken(), "short|int|long") && !var->isPointer() && !var->isArray())
                charBitOpError(tok->tokAt(4)); // This is an error..
        }
    }
}

void CheckOther::charArrayIndexError(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "charArrayIndex",
                "Using char type as array index\n"
                "Using signed char type as array index. If the value "
                "can be greater than 127 there will be a buffer overflow "
                "(because of sign extension).");
}

void CheckOther::charBitOpError(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "charBitOp",
                "When using char variables in bit operations, sign extension can generate unexpected results.\n"
                "When using char variables in bit operations, sign extension can generate unexpected results. For example:\n"
                "    char c = 0x80;\n"
                "    int i = 0 | c;\n"
                "    if (i & 0x8000)\n"
                "        printf(\"not expected\");\n"
                "The 'not expected' will be printed on the screen.");
}

//---------------------------------------------------------------------------
// Incomplete statement..
//---------------------------------------------------------------------------
void CheckOther::checkIncompleteStatement()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
            if (Token::simpleMatch(tok, ") {") && Token::simpleMatch(tok->next()->link(), "} ;"))
                tok = tok->next()->link();
        }

        else if (Token::simpleMatch(tok, "= {"))
            tok = tok->next()->link();

        else if (tok->str() == "{" && Token::Match(tok->tokAt(-2), "%type% %var%"))
            tok = tok->link();

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
    reportError(tok, Severity::warning, "constStatement", "Redundant code: Found a statement that begins with " + type + " constant");
}

//---------------------------------------------------------------------------
// str plus char
//---------------------------------------------------------------------------

void CheckOther::strPlusChar()
{
    // Don't use this check for Java and C# programs..
    if (_tokenizer->isJavaOrCSharp()) {
        return;
    }

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[=(] %str% + %any%")) {
            // char constant..
            if (tok->tokAt(3)->type() == Token::eChar)
                strPlusCharError(tok->next());

            // char variable..
            unsigned int varid = tok->tokAt(3)->varId();
            if (isChar(symbolDatabase->getVariableFromVarId(varid)))
                strPlusCharError(tok->next());
        }
    }
}

void CheckOther::strPlusCharError(const Token *tok)
{
    reportError(tok, Severity::error, "strPlusChar", "Unusual pointer arithmetic");
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
    reportError(tok, Severity::error, "zerodiv", "Division by zero");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkMathFunctions()
{
    const SymbolDatabase *db = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;
    for (scope = db->scopeList.begin(); scope != db->scopeList.end(); ++scope) {
        if (scope->type != Scope::eFunction)
            continue;

        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
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

            // acos( x ), asin( x )  where x is defined for intervall [-1,+1], but not beyound
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
            reportError(tok, Severity::error, "wrongmathcall", "Passing value " + tok->strAt(2) + " to " + tok->str() + "() leads to undefined result");
        else if (numParam == 2)
            reportError(tok, Severity::error, "wrongmathcall", "Passing value " + tok->strAt(2) + " and " + tok->strAt(4) + " to " + tok->str() + "() leads to undefined result");
    } else
        reportError(tok, Severity::error, "wrongmathcall", "Passing value " " to " "() leads to undefined result");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkCCTypeFunctions()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->varId() == 0 &&
            Token::Match(tok, "isalnum|isalpha|iscntrl|isdigit|isgraph|islower|isprint|ispunct|isspace|isupper|isxdigit ( %num% ,|)") &&
            MathLib::isNegative(tok->strAt(2))) {
            cctypefunctionCallError(tok, tok->str(), tok->tokAt(2)->str());
        }
    }
}
void CheckOther::cctypefunctionCallError(const Token *tok, const std::string &functionName, const std::string &value)
{
    reportError(tok, Severity::error, "wrongcctypecall", "Passing value " + value + " to " + functionName + "() cause undefined behavior, which may lead to a crash");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/** Is there a function with given name? */
static bool isFunction(const std::string &name, const Token *startToken)
{
    const std::string pattern1(name + " (");
    for (const Token *tok = startToken; tok; tok = tok->next()) {
        // skip executable scopes etc
        if (tok->str() == "(") {
            tok = tok->link();
            if (Token::simpleMatch(tok, ") {"))
                tok = tok->next()->link();
            else if (Token::simpleMatch(tok, ") const {"))
                tok = tok->linkAt(2);
        }

        // function declaration/implementation found
        if ((tok->str() == "*" || (tok->isName() && tok->str().find(":") ==std::string::npos))
            && Token::simpleMatch(tok->next(), pattern1.c_str()))
            return true;
    }
    return false;
}

void CheckOther::checkMisusedScopedObject()
{
    // Skip this check for .c files
    if (_tokenizer->isC()) {
        return;
    }

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "[;{}] %var% (")
                && Token::simpleMatch(tok->linkAt(2), ") ;")
                && symbolDatabase->isClassOrStruct(tok->next()->str())
                && !isFunction(tok->next()->str(), _tokenizer->tokens())) {
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
                "unusedScopedObject", "Instance of \"" + varname + "\" object destroyed immediately.");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::checkIncorrectStringCompare()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, ". substr ( %any% , %num% ) ==|!= %str%")) {
            MathLib::bigint clen = MathLib::toLongNumber(tok->strAt(5));
            std::size_t slen = Token::getStrLength(tok->tokAt(8));
            if (clen != (int)slen) {
                incorrectStringCompareError(tok->next(), "substr", tok->strAt(8), tok->strAt(5));
            }
        } else if (Token::Match(tok, "%str% ==|!= %var% . substr ( %any% , %num% )")) {
            MathLib::bigint clen = MathLib::toLongNumber(tok->strAt(8));
            std::size_t slen = Token::getStrLength(tok);
            if (clen != (int)slen) {
                incorrectStringCompareError(tok->next(), "substr", tok->str(), tok->strAt(8));
            }
        } else if (Token::Match(tok, "&&|%oror% %str% &&|%oror%|)")) {
            // assert(condition && "debug message") would be considered a fp.
            if (tok->str() == "&&" && tok->strAt(2) == ")" && tok->linkAt(2)->previous()->str() == "assert")
                continue;
            incorrectStringBooleanError(tok->next(), tok->strAt(1));
        } else if (Token::Match(tok, "if|while|assert ( %str% &&|%oror%|)")) {
            // assert("debug message" && condition) would be considered a fp.
            if (tok->strAt(3) == "&&" && tok->str() == "assert")
                continue;
            incorrectStringBooleanError(tok->tokAt(2), tok->strAt(2));
        }
    }
}

void CheckOther::incorrectStringCompareError(const Token *tok, const std::string& func, const std::string &string, const std::string &len)
{
    reportError(tok, Severity::warning, "incorrectStringCompare", "String literal " + string + " doesn't match length argument for " + func + "(" + len + ").");
}

void CheckOther::incorrectStringBooleanError(const Token *tok, const std::string& string)
{
    reportError(tok, Severity::warning, "incorrectStringBooleanError", "A boolean comparison with the string literal " + string + " is always true.");
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
        while (Token::simpleMatch(tok1, "} else if (") &&
               Token::simpleMatch(tok1->linkAt(3), ") {")) {
            // get the expression from the token stream
            expression = tok1->tokAt(4)->stringifyList(tok1->linkAt(3));

            // try to look up the expression to check for duplicates
            std::map<std::string, const Token *>::iterator it = expressionMap.find(expression);

            // found a duplicate
            if (it != expressionMap.end()) {
                // check for expressions that have side effects and ignore them
                if (!expressionHasSideEffects(tok1->tokAt(4), tok1->linkAt(3)->previous()))
                    duplicateIfError(it->second, tok1->next());
            }

            // not a duplicate expression so save it and its location
            else
                expressionMap.insert(std::make_pair(expression, tok1->next()));

            // find the next else if (...) statement
            tok1 = tok1->linkAt(3)->next()->link();
        }
    }
}

void CheckOther::duplicateIfError(const Token *tok1, const Token *tok2)
{
    std::list<const Token *> toks;
    toks.push_back(tok2);
    toks.push_back(tok1);

    reportError(toks, Severity::style, "duplicateIf", "Found duplicate if expressions.\n"
                "Finding the same expression more than once is suspicious and might indicate "
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

//-----------------------------------------------------------------------------
// Check for double free
// free(p); free(p);
//-----------------------------------------------------------------------------
void CheckOther::checkDoubleFree()
{
    std::set<unsigned int> freedVariables;
    std::set<unsigned int> closeDirVariables;

    for (const Token* tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        bool isUnknown = true;

        // Keep track of any variables passed to "free()", "g_free()" or "closedir()",
        // and report an error if the same variable is passed twice.
        if (Token::Match(tok, "free|g_free|closedir ( %var% )")) {
            unsigned int var = tok->tokAt(2)->varId();
            if (var) {
                if (Token::Match(tok, "free|g_free")) {
                    if (freedVariables.find(var) != freedVariables.end())
                        doubleFreeError(tok, tok->tokAt(2)->str());
                    else
                        freedVariables.insert(var);
                } else if (tok->str() == "closedir") {
                    if (closeDirVariables.find(var) != closeDirVariables.end())
                        doubleCloseDirError(tok, tok->tokAt(2)->str());
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
                    doubleFreeError(tok, tok->tokAt(varIdx)->str());
                else
                    freedVariables.insert(var);
            }
        }

        // If this scope doesn't return, clear the set of previously freed variables
        else if (tok->str() == "}" && _tokenizer->IsScopeNoReturn(tok, &isUnknown) && !isUnknown) {
            freedVariables.clear();
            closeDirVariables.clear();
        }

        // If this scope is a "for" or "while" loop that contains "break" or "continue",
        // give up on trying to figure out the flow of execution and just clear the set
        // of previously freed variables.
        // TODO: There are false negatives. This bailout is only needed when the
        // loop will exit without free()'ing the memory on the last iteration.
        else if (tok->str() == "}" && tok->link() && tok->link()->linkAt(-1) &&
                 Token::Match(tok->link()->linkAt(-1)->previous(), "while|for") &&
                 Token::findmatch(tok->link()->linkAt(-1), "break|continue ;", tok) != NULL) {
            freedVariables.clear();
            closeDirVariables.clear();
        }

        // If a variable is passed to a function, remove it from the set of previously freed variables
        else if (Token::Match(tok, "%var% (") && !Token::Match(tok, "printf|sprintf|snprintf|fprintf")) {

            // If this is a new function definition, clear all variables
            if (Token::simpleMatch(tok->next()->link(), ") {")) {
                freedVariables.clear();
                closeDirVariables.clear();
            }
            // If it is a function call, then clear those variables in its argument list
            else if (Token::simpleMatch(tok->next()->link(), ") ;")) {
                for (const Token* tok2 = tok->tokAt(2); tok2 != tok->linkAt(1); tok2 = tok2->next()) {
                    if (Token::Match(tok2, "%var%")) {
                        unsigned int var = tok2->varId();
                        if (var) {
                            freedVariables.erase(var);
                            closeDirVariables.erase(var);
                        }
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

void CheckOther::duplicateBranchError(const Token *tok1, const Token *tok2)
{
    std::list<const Token *> toks;
    toks.push_back(tok2);
    toks.push_back(tok1);

    reportError(toks, Severity::style, "duplicateBranch", "Found duplicate branches for if and else.\n"
                "Finding the same code for an if branch and an else branch is suspicious and "
                "might indicate a cut and paste or logic error. Please examine this code "
                "carefully to determine if it is correct.");
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

    bool inconclusiveFunctionCall(const SymbolDatabase *symbolDatabase,
                                  const std::list<const Function*> &constFunctions,
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
                        v = symbolDatabase->getVariableFromVarId(scope->varId());
                    }
                    // hard coded list of safe, no-side-effect functions
                    if (v == 0 && Token::Match(prev, "strcmp|strncmp|strlen|memcmp|strcasecmp|strncasecmp"))
                        return false;
                    std::list<const Function*>::const_iterator it = std::find_if(constFunctions.begin(),
                            constFunctions.end(),
                            FuncFilter(v ? v->type(): 0, prev));
                    if (it == constFunctions.end())
                        return true;
                }
            }
        }
        return false;
    }

    class Expressions {
    public:
        Expressions(const SymbolDatabase *symbolDatabase, const
                    std::list<const Function*> &constFunctions)
            : _start(0),
              _lastTokens(0),
              _symbolDatabase(symbolDatabase),
              _constFunctions(constFunctions) { }

        void endExpr(const Token *end) {
            const std::string &e = _expression.str();
            if (!e.empty()) {
                std::map<std::string, ExpressionTokens>::iterator it = _expressions.find(e);
                bool lastInconclusive = _lastTokens && _lastTokens->inconclusiveFunction;
                if (it == _expressions.end()) {
                    ExpressionTokens exprTokens(_start, end);
                    exprTokens.inconclusiveFunction = lastInconclusive || inconclusiveFunctionCall(
                                                          _symbolDatabase, _constFunctions, exprTokens);
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
        const SymbolDatabase *_symbolDatabase;
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
    Expressions expressions(_tokenizer->getSymbolDatabase(), constFunctions);
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
        unsigned int parenthesis = 0;  // ()
        unsigned int brackets = 0;     // []

        // taking address?
        if (Token::Match(it->second.end->previous(), "%op% &")) {
            continue;
        }

        for (const Token *tok = it->second.start; tok && tok != it->second.end; tok = tok->next()) {
            if (tok->str() == "(") {
                ++parenthesis;
            } else if (tok->str() == ")") {
                if (parenthesis == 0) {
                    valid = false;
                    break;
                }
                --parenthesis;
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

        if (!valid || parenthesis!=0 || brackets!=0)
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
            if (Token::Match(tok, ",|=|return|(|&&|%oror% %var% ==|!=|<=|>=|<|>|- %var% )|&&|%oror%|;|,") &&
                tok->strAt(1) == tok->strAt(3)) {
                // float == float and float != float are valid NaN checks
                if (Token::Match(tok->tokAt(2), "==|!=") && tok->next()->varId()) {
                    const Variable * var = symbolDatabase->getVariableFromVarId(tok->next()->varId());
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
            } else if (Token::Match(tok, ",|=|return|(|&&|%oror% %var% . %var% ==|!=|<=|>=|<|>|- %var% . %var% )|&&|%oror%|;|,") &&
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
    if (!_settings->isEnabled("style"))
        return;

    const char pattern1[] = "strncmp|strcmp|stricmp|strcmpi|strcasecmp|wcscmp ( %str% , %str% ";
    const char pattern2[] = "QString :: compare ( %str% , %str% )";
    const char pattern3[] = "strncmp|strcmp|stricmp|strcmpi|strcasecmp|wcscmp ( %var% , %var% ";

    const Token *tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern1)) != NULL) {
        const std::string &str1 = tok->strAt(2);
        const std::string &str2 = tok->strAt(4);
        alwaysTrueFalseStringCompareError(tok, str1, str2);
        tok = tok->tokAt(5);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern2)) != NULL) {
        const std::string &str1 = tok->strAt(4);
        const std::string &str2 = tok->strAt(6);
        alwaysTrueFalseStringCompareError(tok, str1, str2);
        tok = tok->tokAt(7);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern3)) != NULL) {
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
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token* tok = _tokenizer->list.front(); tok && tok->tokAt(3); tok = tok->next()) {
        if (tok->next()->type() != Token::eComparisonOp)
            continue;

        const Token* varTok = tok;
        const Token* litTok = tok->tokAt(2);

        if (varTok->strAt(-1) == "+" || litTok->strAt(1) == "+")
            continue;

        if ((varTok->type() == Token::eString || varTok->type() == Token::eVariable) && (litTok->type() == Token::eString || litTok->type() == Token::eVariable) && litTok->type() != varTok->type()) {
            if (varTok->type() == Token::eString)
                std::swap(varTok, litTok);

            const Variable* var = symbolDatabase->getVariableFromVarId(varTok->varId());
            if (var && var->isPointer())
                suspiciousStringCompareError(tok, var->name());
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
    for (const Token* tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "% %num% ==|!=|<=|<|>|>= %num%") && (!tok->tokAt(4) || !tok->tokAt(4)->isArithmeticalOp())) {
            if (MathLib::isLessEqual(tok->strAt(1), tok->strAt(3)))
                moduloAlwaysTrueFalseError(tok, tok->strAt(1));
        }
    }
}

void CheckOther::moduloAlwaysTrueFalseError(const Token* tok, const std::string& maxVal)
{
    reportError(tok, Severity::warning, "moduloAlwaysTrueFalse",
                "Comparison of modulo result is predetermined, because it is always less than " + maxVal + ".");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckOther::sizeofsizeof()
{
    if (!_settings->isEnabled("style"))
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof (| sizeof")) {
            sizeofsizeofError(tok);
            tok = tok->next();
        }
    }
}

void CheckOther::sizeofsizeofError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "sizeofsizeof", "Calling sizeof for 'sizeof'.\n"
                "Calling sizeof for 'sizeof looks like a suspicious code and "
                "most likely there should be just one 'sizeof'. The current "
                "code is equivalent to 'sizeof(size_t)'");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckOther::sizeofCalculation()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            const Token* const end = tok->linkAt(1);
            for (const Token *tok2 = tok->tokAt(2); tok2 != end; tok2 = tok2->next()) {
                if (tok2->isOp() && (!tok2->isExpandedMacro() || _settings->inconclusive) && !Token::Match(tok2, ">|<|&") && (Token::Match(tok2->previous(), "%var%") || !Token::Match(tok2, "*"))) {
                    if (!(Token::Match(tok2->previous(), "%type%") || Token::Match(tok2->next(), "%type%"))) {
                        sizeofCalculationError(tok2, tok2->isExpandedMacro());
                        break;
                    }
                }
            }
        }
    }
}

void CheckOther::sizeofCalculationError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::warning,
                "sizeofCalculation", "Found calculation inside sizeof()", inconclusive);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckOther::checkAssignBoolToPointer()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "!!* %var% = %bool% ;")) {
            const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

            const Variable *var1(symbolDatabase->getVariableFromVarId(tok->next()->varId()));

            // Is variable a pointer?
            if (var1 && var1->isPointer())
                assignBoolToPointerError(tok->next());
        }
    }
}

void CheckOther::assignBoolToPointerError(const Token *tok)
{
    reportError(tok, Severity::error, "assignBoolToPointer",
                "Assigning bool value to pointer (converting bool value to address)");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckOther::checkComparisonOfBoolExpressionWithInt()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Skip template parameters
        if (tok->str() == "<" && tok->link()) {
            tok = tok->link();
            continue;
        }

        const Token* numTok = 0;
        const Token* opTok = 0;
        char op = 0;
        if (Token::Match(tok, "&&|%oror% %any% ) >|>=|==|!=|<=|< %any%")) {
            numTok = tok->tokAt(4);
            opTok = tok->tokAt(3);
            if (Token::Match(opTok, "<|>"))
                op = opTok->str()[0];
        } else if (Token::Match(tok, "%any% >|>=|==|!=|<=|< ( %any% &&|%oror%")) {
            numTok = tok;
            opTok = tok->next();
            if (Token::Match(opTok, "<|>"))
                op = opTok->str()[0]=='>'?'<':'>';
        }

        else if (Token::Match(tok, "! %var% >|>=|==|!=|<=|< %any%")) {
            numTok = tok->tokAt(3);
            opTok = tok->tokAt(2);
            if (Token::Match(opTok, "<|>"))
                op = opTok->str()[0];
        } else if (Token::Match(tok, "%any% >|>=|==|!=|<=|< ! %var%")) {
            numTok = tok;
            opTok = tok->next();
            if (Token::Match(opTok, "<|>"))
                op = opTok->str()[0]=='>'?'<':'>';
        }

        if (numTok && opTok) {
            if (numTok->isNumber()) {
                if (((numTok->str() != "0" && numTok->str() != "1") || !Token::Match(opTok, "!=|==")) && !((op == '<' && numTok->str() == "1") || (op == '>' && numTok->str() == "0")))
                    comparisonOfBoolExpressionWithIntError(tok, true);
            } else if (isNonBoolStdType(symbolDatabase->getVariableFromVarId(numTok->varId())))
                comparisonOfBoolExpressionWithIntError(tok, false);
        }
    }
}

void CheckOther::comparisonOfBoolExpressionWithIntError(const Token *tok, bool n0o1)
{
    if (n0o1)
        reportError(tok, Severity::warning, "compareBoolExpressionWithInt",
                    "Comparison of a boolean expression with an integer other than 0 or 1.");
    else
        reportError(tok, Severity::warning, "compareBoolExpressionWithInt",
                    "Comparison of a boolean expression with an integer.");
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

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // check all the code in the function
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% <|<= 0") && tok->varId() && !Token::Match(tok->previous(), "++|--|)|+|-|*|/|~|<<|>>") && !Token::Match(tok->tokAt(3), "+|-")) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZeroError(tok, var->name(), inconclusive);
                else if (var && var->isPointer() && tok->strAt(-1) != "*")
                    pointerLessThanZeroError(tok, inconclusive);
            } else if (Token::Match(tok, "0 >|>= %var%") && tok->tokAt(2)->varId() && !Token::Match(tok->tokAt(3), "+|-|*|/") && !Token::Match(tok->previous(), "+|-|<<|>>|~")) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->tokAt(2)->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZeroError(tok, var->name(), inconclusive);
                else if (var && var->isPointer() && !Token::Match(tok->tokAt(3), "[.[]"))
                    pointerLessThanZeroError(tok, inconclusive);
            } else if (Token::Match(tok, "0 <= %var%") && tok->tokAt(2)->varId() && !Token::Match(tok->tokAt(3), "+|-|*|/") && !Token::Match(tok->previous(), "+|-|<<|>>|~")) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->tokAt(2)->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedPositiveError(tok, var->name(), inconclusive);
                else if (var && var->isPointer() && !Token::Match(tok->tokAt(3), "[.[]"))
                    pointerPositiveError(tok, inconclusive);
            } else if (Token::Match(tok, "%var% >= 0") && tok->varId() && !Token::Match(tok->previous(), "++|--|)|+|-|*|/|~|<<|>>") && !Token::Match(tok->tokAt(3), "+|-")) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->varId());
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
                    "this message might be a false warning", true);
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
                    "An unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it. This might be a false warning.\n"
                    "An unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it. "
                    "It's not known if the used constant is a "
                    "template parameter or not and therefore this message might be a false warning", true);
    } else {
        reportError(tok, Severity::style, "unsignedPositive",
                    "An unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it.");
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
    if (!_settings->isEnabled("performance"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok=tok->next()) {
        const char *expect_end_token;
        if (Token::Match(tok, "const %type% %var% =")) {
            //match "const A a =" usage
            expect_end_token = ";";
        } else if (Token::Match(tok, "const %type% %var% (")) {
            //match "const A a (" usage
            expect_end_token = ")";
        } else {
            continue;
        }

        if (tok->strAt(1) == tok->strAt(4)) //avoid "const A a = A();"
            continue;
        if (!symbolDatabase->isClassOrStruct(tok->next()->str())) //avoid when %type% is standard type
            continue;
        const Token *var_tok = tok->tokAt(2);
        tok = tok->tokAt(4);
        while (tok &&Token::Match(tok,"%var% ."))
            tok = tok->tokAt(2);
        if (!Token::Match(tok, "%var% ("))
            break;
        const Token *match_end = (tok->next()->link()!=NULL)?tok->next()->link()->next():NULL;
        if (match_end==NULL || !Token::Match(match_end,expect_end_token)) //avoid usage like "const A a = getA()+3"
            break;
        const Token *fToken = _tokenizer->getFunctionTokenByName(tok->str().c_str());
        if (fToken &&fToken->previous() && fToken->previous()->str() == "&") {
            redundantCopyError(var_tok,var_tok->str());
        }
    }
}
void CheckOther::redundantCopyError(const Token *tok,const std::string& varname)
{
    reportError(tok, Severity::performance,"redundantCopyLocalConst",
                "Use const reference for "+varname+" to avoid unnecessary data copying.\n"
                "The const "+varname+" gets a copy of the data since const reference is not used. You can avoid the unnecessary data copying by converting "+varname+" to const reference instead of just const.");
}

//---------------------------------------------------------------------------
// Checking for shift by negative values
//---------------------------------------------------------------------------

void CheckOther::checkNegativeBitwiseShift()
{
    for (const Token *tok = _tokenizer->tokens(); tok ; tok = tok->next()) {
        if (Token::Match(tok,"%var% >>|<< %num%") || Token::Match(tok,"%num >>|<< %num%")) {
            if ((tok->strAt(2))[0] == '-')
                negativeBitwiseShiftError(tok);
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
    if (!_settings->inconclusive || !_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token* tok = _tokenizer->list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "memset|memcpy|memmove ( %var% ,") && Token::Match(tok->linkAt(1)->tokAt(-2), ", %num% )")) {
            const Variable* var = symbolDatabase->getVariableFromVarId(tok->tokAt(2)->varId());
            if (!var || !var->isArray() || var->dimensions().empty() || !var->dimension(0))
                continue;

            if (MathLib::toLongNumber(tok->linkAt(1)->strAt(-1)) == var->dimension(0)) {
                unsigned int size = _tokenizer->sizeOfType(var->typeStartToken());
                if ((size != 1 && size != 100 && size != 0) || Token::Match(var->typeEndToken(), "*"))
                    incompleteArrayFillError(tok, var->name(), tok->str(), false);
                else if (var->typeStartToken()->str() == "bool" && _settings->isEnabled("portability")) // sizeof(bool) is not 1 on all platforms
                    incompleteArrayFillError(tok, var->name(), tok->str(), true);
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
