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

#include <cctype>
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
                if (Token::Match(cond->previous(),("return|=|+|-|,|(|"+op).c_str()))
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

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "( %var% [=&|^]")) {
            for (const Token *tok2 = tok->tokAt(3); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(" || tok2->str() == "[")
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "<|<=|==|!=|>|>=")) {
                    // This might be a template
                    if (!_tokenizer->isC() && Token::Match(tok2->previous(), "%var% <"))
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
                if (Token::Match(tok, "<|>") && (Token::Match(tok2, "& ,|>") ||
                                                 Token::simpleMatch(tok2->previous(), "const &")))
                    continue;

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
        if (Token::Match(tok, "(|.|return|&&|%oror% %var% [&|]")) {
            if (tok->next()->varId()) {
                const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok->next()->varId());
                if (var && (var->typeStartToken() == var->typeEndToken()) &&
                    var->typeStartToken()->str() == "bool") {
                    bitwiseOnBooleanError(tok->next(), tok->next()->str(), tok->strAt(2) == "&" ? "&&" : "||");
                }
            }
        }
    }
}

void CheckOther::bitwiseOnBooleanError(const Token *tok, const std::string &varname, const std::string &op)
{
    reportInconclusiveError(tok, Severity::style, "bitwiseOnBoolean",
                            "Boolean variable '" + varname + "' is used in bitwise operation. Did you mean " + op + " ?");
}

void CheckOther::checkSuspiciousSemicolon()
{
    if (!_settings->inconclusive || !_settings->isEnabled("style"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Look for "if(); {}", "for(); {}" or "while(); {}"
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type == Scope::eIf || i->type == Scope::eElse || i->type == Scope::eElseIf || i->type == Scope::eWhile || i->type == Scope::eFor) {
            const Token *tok = Token::findsimplematch(i->classDef, "(", i->classEnd);
            if (!tok)
                continue;
            const Token *end = tok->link();

            // Ensure the semicolon is at the same line number as the if/for/while statement
            // and the {..} block follows it without an extra empty line.
            if (Token::simpleMatch(end, ") { ; } {") &&
                end->linenr() == end->tokAt(2)->linenr()
                && end->linenr()+1 >= end->tokAt(4)->linenr()) {
                SuspiciousSemicolonError(tok);
            }
        }
    }
}

void CheckOther::SuspiciousSemicolonError(const Token* tok)
{
    reportInconclusiveError(tok, Severity::warning, "suspiciousSemicolon",
                            "Suspicious use of ; at the end of 'if/for/while' statement.");
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckOther::warningOldStylePointerCast()
{
    if (!_settings->isEnabled("style")) {
        return;
    }

    if (!_tokenizer->isCPP()) {
        return;
    }

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Old style pointer casting..
        if (!Token::Match(tok, "( const| %type% * ) (| %var%") &&
            !Token::Match(tok, "( const| %type% * ) (| new"))
            continue;

        unsigned char addToIndex = 0;
        if (tok->strAt(1) == "const")
            addToIndex = 1;

        if (tok->strAt(4 + addToIndex) == "const")
            continue;

        // Is "type" a class?
        const std::string pattern("class " + tok->strAt(1 + addToIndex));
        if (!Token::findmatch(_tokenizer->tokens(), pattern.c_str()))
            continue;

        cstyleCastError(tok);
    }
}

void CheckOther::cstyleCastError(const Token *tok)
{
    reportError(tok, Severity::style, "cstyleCast", "C-style pointer casting");
}

//---------------------------------------------------------------------------
// fflush(stdin) <- fflush only applies to output streams in ANSI C
//---------------------------------------------------------------------------
void CheckOther::checkFflushOnInputStream()
{
    const Token *tok = _tokenizer->tokens();
    while (tok && ((tok = Token::findsimplematch(tok, "fflush ( stdin )")) != NULL)) {
        fflushOnInputStreamError(tok, tok->strAt(2));
        tok = tok->tokAt(4);
    }
}

void CheckOther::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error,
                "fflushOnInputStream", "fflush() called on input stream \"" + varname + "\" may result in undefined behaviour");
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
        if (Token::Match(tok, "sizeof ( %var% )") || Token::Match(tok, "sizeof %var%")) {
            unsigned short tokIdx = 1;
            if (tok->strAt(tokIdx) == "(") {
                ++tokIdx;
            }
            if (tok->tokAt(tokIdx)->varId() > 0) {
                const Variable *var = symbolDatabase->getVariableFromVarId(tok->tokAt(tokIdx)->varId());
                if (var) {
                    const Token *declTok = var->nameToken();
                    if (Token::simpleMatch(declTok->next(), "[")) {
                        declTok = declTok->next()->link();
                        // multidimensional array
                        while (Token::simpleMatch(declTok->next(), "[")) {
                            declTok = declTok->next()->link();
                        }
                        if (!(Token::Match(declTok->next(), "= %str%")) && !(Token::simpleMatch(declTok->next(), "= {")) && !(Token::simpleMatch(declTok->next(), ";"))) {
                            if (Token::simpleMatch(declTok->next(), ",")) {
                                declTok = declTok->next();
                                while (!Token::simpleMatch(declTok, ";")) {
                                    if (Token::simpleMatch(declTok, ")")) {
                                        sizeofForArrayParameterError(tok);
                                        break;
                                    }
                                    if (Token::Match(declTok, "(|[|{")) {
                                        declTok = declTok->link();
                                    }
                                    declTok = declTok->next();
                                }
                            }
                        }
                        if (Token::simpleMatch(declTok->next(), ")")) {
                            sizeofForArrayParameterError(tok);
                        }
                    }
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

void CheckOther::checkSizeofForStrncmpSize()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    static const char pattern0[] = "strncmp (";
    static const char pattern1[] = "sizeof ( %var% ) )";
    static const char pattern2[] = "sizeof %var% )";

    // danmar : this is inconclusive in case the size parameter is
    //          sizeof(char *) by intention.
    if (!_settings->inconclusive || !_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, pattern0)) {
            const Token *tokVar = tok->tokAt(2);
            if (tokVar)
                tokVar = tokVar->nextArgument();
            if (tokVar)
                tokVar = tokVar->nextArgument();
            if (Token::Match(tokVar, pattern1) || Token::Match(tokVar, pattern2)) {
                tokVar = tokVar->next();
                if (tokVar->str() == "(")
                    tokVar = tokVar->next();
                if (tokVar->varId() > 0) {
                    const Variable *var = symbolDatabase->getVariableFromVarId(tokVar->varId());
                    if (var && var->isPointer()) {
                        sizeofForStrncmpError(tokVar);
                    }
                }
            }
        }
    }
}

void CheckOther::sizeofForStrncmpError(const Token *tok)
{
    reportInconclusiveError(tok, Severity::warning, "strncmpLen",
                            "Passing sizeof(pointer) as the last argument to strncmp.\n"
                            "Passing a pointer to sizeof returns the size of the pointer, not "
                            "the number of characters pointed to by that pointer. This "
                            "means that only 4 or 8 (on 64-bit systems) characters are "
                            "compared, which is probably not what was expected.");
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

    const char switchPattern[] = "switch ( %any% ) { case";
    const char breakPattern[] = "break|continue|return|exit|goto|throw";
    const char functionPattern[] = "%var% (";

    // Find the beginning of a switch. E.g.:
    //   switch (var) { ...
    const Token *tok = Token::findmatch(_tokenizer->tokens(), switchPattern);
    while (tok) {

        // Check the contents of the switch statement
        std::map<unsigned int, const Token*> varsAssigned;
        std::map<unsigned int, const Token*> stringsCopied;
        int indentLevel = 0;
        for (const Token *tok2 = tok->tokAt(5); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                // Inside a conditional or loop. Don't mark variable accesses as being redundant. E.g.:
                //   case 3: b = 1;
                //   case 4: if (a) { b = 2; }    // Doesn't make the b=1 redundant because it's conditional
                if (Token::Match(tok2->previous(), ")|else {") && tok2->link()) {
                    const Token* endOfConditional = tok2->link();
                    for (const Token* tok3 = tok2; tok3 != endOfConditional; tok3 = tok3->next()) {
                        if (tok3->varId() != 0) {
                            varsAssigned.erase(tok3->varId());
                            stringsCopied.erase(tok3->varId());
                        } else if (Token::Match(tok3, functionPattern) || Token::Match(tok3, breakPattern)) {
                            varsAssigned.clear();

                            if (tok3->str() != "strcpy" && tok3->str() != "strncpy")
                                stringsCopied.clear();
                        }
                    }
                    tok2 = endOfConditional;
                } else
                    ++ indentLevel;
            } else if (tok2->str() == "}") {
                -- indentLevel;

                // End of the switch block
                if (indentLevel < 0)
                    break;
            }

            // Variable assignment. Report an error if it's assigned to twice before a break. E.g.:
            //    case 3: b = 1;    // <== redundant
            //    case 4: b = 2;
            if (Token::Match(tok2->previous(), ";|{|}|: %var% = %any% ;") && tok2->varId() != 0) {
                std::map<unsigned int, const Token*>::iterator i = varsAssigned.find(tok2->varId());
                if (i == varsAssigned.end())
                    varsAssigned[tok2->varId()] = tok2;
                else
                    redundantAssignmentInSwitchError(i->second, i->second->str());

                stringsCopied.erase(tok2->varId());
            }
            // String copy. Report an error if it's copied to twice before a break. E.g.:
            //    case 3: strcpy(str, "a");    // <== redundant
            //    case 4: strcpy(str, "b");
            else if (Token::Match(tok2->previous(), ";|{|}|: strcpy|strncpy ( %var% ,") && tok2->tokAt(2)->varId() != 0) {
                std::map<unsigned int, const Token*>::iterator i = stringsCopied.find(tok2->tokAt(2)->varId());
                if (i == stringsCopied.end())
                    stringsCopied[tok2->tokAt(2)->varId()] = tok2->tokAt(2);
                else
                    redundantStrcpyInSwitchError(i->second, i->second->str());
            }
            // Not a simple assignment so there may be good reason if this variable is assigned to twice. E.g.:
            //    case 3: b = 1;
            //    case 4: b++;
            else if (tok2->varId() != 0)
                varsAssigned.erase(tok2->varId());

            // Reset our record of assignments if there is a break or function call. E.g.:
            //    case 3: b = 1; break;
            if (Token::Match(tok2, functionPattern) || Token::Match(tok2, breakPattern)) {
                varsAssigned.clear();

                if (tok2->str() != "strcpy" && tok2->str() != "strncpy")
                    stringsCopied.clear();
            }

        }

        tok = Token::findmatch(tok->next(), switchPattern);
    }
}

void CheckOther::redundantAssignmentInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "redundantAssignInSwitch", "Redundant assignment of \"" + varname + "\" in switch");
}

void CheckOther::redundantStrcpyInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "redundantStrcpyInSwitch",
                "Switch case fall-through. Redundant strcpy of \"" + varname + "\".\n"
                "Switch case fall-through. Redundant strcpy of \"" + varname + "\". The string is overwritten in a later case block.");
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
//    std::cout << std::cout;
//---------------------------------------------------------------------------
void CheckOther::checkCoutCerrMisusage()
{
    bool firstCout = false;
    unsigned int roundbraces = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "(")
            ++roundbraces;
        else if (tok->str() == ")") {
            if (!roundbraces)
                break;
            --roundbraces;
        }
        if (roundbraces)
            continue;

        if (Token::Match(tok, "std :: cout|cerr")) {
            if (firstCout && tok->strAt(-1) == "<<" && tok->strAt(3) != ".") {
                coutCerrMisusageError(tok, tok->strAt(2));
                firstCout = false;
            } else if (tok->strAt(3) == "<<")
                firstCout = true;
        } else if (firstCout && tok->str() == ";")
            firstCout = false;
    }
}

void CheckOther::coutCerrMisusageError(const Token* tok, const std::string& streamName)
{
    reportError(tok, Severity::error, "coutCerrMisusage", "Invalid usage of output stream: '<< std::" + streamName + "'.");
}

//---------------------------------------------------------------------------
//    int x = 1;
//    x = x;            // <- redundant assignment to self
//
//    int y = y;        // <- redundant initialization to self
//---------------------------------------------------------------------------
static bool isPOD(const Variable* var)
{
    // TODO: Implement real support for POD definition
    return(var && var->nameToken()->previous()->isStandardType());
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
            isPOD(symbolDatabase->getVariableFromVarId(tok->varId()))) {
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
            if (tok->isName() && (tok->next()->isAssignmentOp() || tok->next()->str() == "++" || tok->next()->str() == "--"))
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
void CheckOther::checkIncorrectLogicOperator()
{
    if (!_settings->isEnabled("style"))
        return;

    const char conditionPattern[] = "if|while (";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), conditionPattern);
    const Token *endTok = tok ? tok->next()->link() : NULL;

    while (tok && endTok) {
        // Find a pair of comparison expressions with or without parenthesis
        // with a shared variable and constants and with a logical operator between them.
        // e.g. if (x != 3 || x != 4)
        const Token *logicTok = NULL, *term1Tok = NULL, *term2Tok = NULL;
        const Token *op1Tok = NULL, *op2Tok = NULL, *op3Tok = NULL, *nextTok = NULL;
        if (NULL != (logicTok = Token::findmatch(tok, "( %any% !=|==|<|>|>=|<= %any% ) &&|%oror% ( %any% !=|==|<|>|>=|<= %any% ) %any%", endTok))) {
            term1Tok = logicTok->next();
            term2Tok = logicTok->tokAt(7);
            op1Tok = logicTok->tokAt(2);
            op2Tok = logicTok->tokAt(5);
            op3Tok = logicTok->tokAt(8);
            nextTok = logicTok->tokAt(11);
        } else if (NULL != (logicTok = Token::findmatch(tok, "%any% !=|==|<|>|>=|<= %any% &&|%oror% %any% !=|==|<|>|>=|<= %any% %any%", endTok))) {
            term1Tok = logicTok;
            term2Tok = logicTok->tokAt(4);
            op1Tok = logicTok->next();
            op2Tok = logicTok->tokAt(3);
            op3Tok = logicTok->tokAt(5);
            nextTok = logicTok->tokAt(7);
        }

        if (logicTok) {
            // Find the common variable and the two different-valued constants
            unsigned int variableTested = 0;
            std::string firstConstant, secondConstant;
            bool varFirst1, varFirst2;
            unsigned int varId;
            const Token *varTok = NULL;
            if (Token::Match(term1Tok, "%var% %any% %num%")) {
                varTok = term1Tok;
                varId = varTok->varId();
                if (!varId) {
                    tok = Token::findmatch(endTok->next(), conditionPattern);
                    endTok = tok ? tok->next()->link() : NULL;
                    continue;
                }
                varFirst1 = true;
                firstConstant = term1Tok->strAt(2);
            } else if (Token::Match(term1Tok, "%num% %any% %var%")) {
                varTok = term1Tok->tokAt(2);
                varId = varTok->varId();
                if (!varId) {
                    tok = Token::findmatch(endTok->next(), conditionPattern);
                    endTok = tok ? tok->next()->link() : NULL;
                    continue;
                }
                varFirst1 = false;
                firstConstant = term1Tok->str();
            } else {
                tok = Token::findmatch(endTok->next(), conditionPattern);
                endTok = tok ? tok->next()->link() : NULL;
                continue;
            }

            if (Token::Match(term2Tok, "%var% %any% %num%")) {
                const unsigned int varId2 = term2Tok->varId();
                if (!varId2 || varId != varId2) {
                    tok = Token::findmatch(endTok->next(), conditionPattern);
                    endTok = tok ? tok->next()->link() : NULL;
                    continue;
                }
                varFirst2 = true;
                secondConstant = term2Tok->strAt(2);
                variableTested = varId;
            } else if (Token::Match(term2Tok, "%num% %any% %var%")) {
                const unsigned int varId2 = term1Tok->tokAt(2)->varId();
                if (!varId2 || varId != varId2) {
                    tok = Token::findmatch(endTok->next(), conditionPattern);
                    endTok = tok ? tok->next()->link() : NULL;
                    continue;
                }
                varFirst2 = false;
                secondConstant = term2Tok->str();
                variableTested = varId;
            } else {
                tok = Token::findmatch(endTok->next(), conditionPattern);
                endTok = tok ? tok->next()->link() : NULL;
                continue;
            }

            if (variableTested == 0 || firstConstant.empty() || secondConstant.empty()) {
                tok = Token::findmatch(endTok->next(), conditionPattern);
                endTok = tok ? tok->next()->link() : NULL;
                continue;
            }

            enum Position { First, Second, NA };
            enum Relation { Equal, NotEqual, Less, LessEqual, More, MoreEqual };
            enum LogicError { Exclusion, AlwaysTrue, AlwaysFalse, AlwaysFalseOr };
            static const struct Condition {
                const char *before;
                Position   position1;
                const char *op1TokStr;
                const char *op2TokStr;
                Position   position2;
                const char *op3TokStr;
                const char *after;
                Relation   relation;
                LogicError error;
            } conditions[] = {
                { "!!&&", NA,     "!=",   "||", NA,     "!=",   "!!&&", NotEqual,  Exclusion     }, // (x != 1) || (x != 3) <- always true
                { "(",    NA,     "==",   "&&", NA,     "==",   ")",    NotEqual,  AlwaysFalseOr }, // (x == 1) && (x == 3) <- always false
                { "(",    First,  "<",    "&&", First,  ">",    ")",    LessEqual, AlwaysFalseOr }, // (x < 1)  && (x > 3)  <- always false
                { "(",    First,  ">",    "&&", First,  "<",    ")",    MoreEqual, AlwaysFalseOr }, // (x > 3)  && (x < 1)  <- always false
                { "(",    Second, ">",    "&&", First,  ">",    ")",    LessEqual, AlwaysFalseOr }, // (1 > x)  && (x > 3)  <- always false
                { "(",    First,  ">",    "&&", Second, ">",    ")",    MoreEqual, AlwaysFalseOr }, // (x > 3)  && (1 > x)  <- always false
                { "(",    First,  "<",    "&&", Second, "<",    ")",    LessEqual, AlwaysFalseOr }, // (x < 1)  && (3 < x)  <- always false
                { "(",    Second, "<",    "&&", First,  "<",    ")",    MoreEqual, AlwaysFalseOr }, // (3 < x)  && (x < 1)  <- always false
                { "(",    Second, ">",    "&&", Second, "<",    ")",    LessEqual, AlwaysFalseOr }, // (1 > x)  && (3 < x)  <- always false
                { "(",    Second, "<",    "&&", Second, ">",    ")",    MoreEqual, AlwaysFalseOr }, // (3 < x)  && (1 > x)  <- always false
                { "(",    First , ">|>=", "||", First,  "<|<=", ")",    Less,      Exclusion     }, // (x > 3)  || (x < 10) <- always true
                { "(",    First , "<|<=", "||", First,  ">|>=", ")",    More,      Exclusion     }, // (x < 10) || (x > 3)  <- always true
                { "(",    Second, "<|<=", "||", First,  "<|<=", ")",    Less,      Exclusion     }, // (3 < x)  || (x < 10) <- always true
                { "(",    First,  "<|<=", "||", Second, "<|<=", ")",    More,      Exclusion     }, // (x < 10) || (3 < x)  <- always true
                { "(",    First,  ">|>=", "||", Second, ">|>=", ")",    Less,      Exclusion     }, // (x > 3)  || (10 > x) <- always true
                { "(",    Second, ">|>=", "||", First,  ">|>=", ")",    More,      Exclusion     }, // (10 > x) || (x > 3)  <- always true
                { "(",    Second, "<|<=", "||", Second, ">|<=", ")",    Less,      Exclusion     }, // (3 < x)  || (10 > x) <- always true
                { "(",    Second, ">|>=", "||", Second, "<|<=", ")",    More,      Exclusion     }, // (10 > x) || (3 < x)  <- always true
                { "(",    First,  ">",    "&&", NA,     "!=",   ")",    More,      AlwaysTrue    }, // (x > 5)  && (x != 1) <- second expression always true
                { "(",    Second, "<",    "&&", NA,     "!=",   ")",    More,      AlwaysTrue    }, // (5 < x)  && (x != 1) <- second expression always true
                { "(",    First,  ">",    "&&", NA,     "==",   ")",    More,      AlwaysFalse   }, // (x > 5)  && (x == 1) <- second expression always false
                { "(",    Second, "<",    "&&", NA,     "==",   ")",    More,      AlwaysFalse   }, // (5 < x)  && (x == 1) <- second expression always false
            };

            for (unsigned int i = 0; i < (sizeof(conditions) / sizeof(conditions[0])); i++) {
                if (!((conditions[i].position1 == NA) || (((conditions[i].position1 == First) && varFirst1) || ((conditions[i].position1 == Second) && !varFirst1))))
                    continue;

                if (!((conditions[i].position2 == NA) || (((conditions[i].position2 == First) && varFirst2) || ((conditions[i].position2 == Second) && !varFirst2))))
                    continue;

                if (!Token::Match(op1Tok, conditions[i].op1TokStr))
                    continue;

                if (!Token::Match(op2Tok, conditions[i].op2TokStr))
                    continue;

                if (!Token::Match(op3Tok, conditions[i].op3TokStr))
                    continue;

                if (!Token::Match(logicTok->previous(), conditions[i].before))
                    continue;

                if (!Token::Match(nextTok, conditions[i].after))
                    continue;

                if ((conditions[i].relation == Equal     && MathLib::isEqual(firstConstant, secondConstant)) ||
                    (conditions[i].relation == NotEqual  && MathLib::isNotEqual(firstConstant, secondConstant)) ||
                    (conditions[i].relation == Less      && MathLib::isLess(firstConstant, secondConstant)) ||
                    (conditions[i].relation == LessEqual && MathLib::isLessEqual(firstConstant, secondConstant)) ||
                    (conditions[i].relation == More      && MathLib::isGreater(firstConstant, secondConstant)) ||
                    (conditions[i].relation == MoreEqual && MathLib::isGreaterEqual(firstConstant, secondConstant))) {
                    if (conditions[i].error == Exclusion || conditions[i].error == AlwaysFalseOr)
                        incorrectLogicOperatorError(term1Tok, conditions[i].error == Exclusion);
                    else {
                        std::string text("When " + varTok->str() + " is greater than " + firstConstant + ", the comparison " +
                                         varTok->str() + " " + conditions[i].op3TokStr + " " + secondConstant +
                                         " is always " + (conditions[i].error == AlwaysTrue ? "true." : "false."));
                        secondAlwaysTrueFalseWhenFirstTrueError(term1Tok, text);
                    }
                }
            }
        }

        tok = Token::findmatch(endTok->next(), conditionPattern);
        endTok = tok ? tok->next()->link() : NULL;
    }
}

void CheckOther::incorrectLogicOperatorError(const Token *tok, bool always)
{
    if (always)
        reportError(tok, Severity::warning,
                    "incorrectLogicOperator", "Mutual exclusion over || always evaluates to true. Did you intend to use && instead?");
    else
        reportError(tok, Severity::warning,
                    "incorrectLogicOperator", "Expression always evaluates to false. Did you intend to use || instead?");
}

void CheckOther::secondAlwaysTrueFalseWhenFirstTrueError(const Token *tok, const std::string &truefalse)
{
    reportError(tok, Severity::style, "secondAlwaysTrueFalseWhenFirstTrue", truefalse);
}

//---------------------------------------------------------------------------
//    try {} catch (std::exception err) {} <- Should be "std::exception& err"
//---------------------------------------------------------------------------
void CheckOther::checkCatchExceptionByValue()
{
    if (!_settings->isEnabled("style"))
        return;

    const char catchPattern[] = "} catch (";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), catchPattern);
    const Token *endTok = tok ? tok->linkAt(2) : NULL;

    while (tok && endTok) {
        // Find a pass-by-value declaration in the catch(), excluding basic types
        // e.g. catch (std::exception err)
        const Token *tokType = Token::findmatch(tok, "%type% %var% )", endTok);
        if (tokType && !tokType->isStandardType()) {
            catchExceptionByValueError(tokType);
        }

        tok = Token::findmatch(endTok->next(), catchPattern);
        endTok = tok ? tok->linkAt(2) : NULL;
    }
}

void CheckOther::catchExceptionByValueError(const Token *tok)
{
    reportError(tok, Severity::style,
                "catchExceptionByValue", "Exception should be caught by reference.\n"
                "The exception is caught as a value. It could be caught "
                "as a (const) reference which is usually recommended in C++.");
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

        // Locate the third parameter of the function call..
        unsigned int param = 1;
        for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(")
                tok2 = tok2->link();
            else if (tok2->str() == ")")
                break;
            else if (tok2->str() == ",") {
                ++param;
                if (param == 3) {
                    if (Token::Match(tok2, ", %num% )")) {
                        const MathLib::bigint radix = MathLib::toLongNumber(tok2->next()->str());
                        if (!(radix == 0 || (radix >= 2 && radix <= 36))) {
                            dangerousUsageStrtolError(tok2);
                        }
                    }
                    break;
                }
            }
        }
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
//---------------------------------------------------------------------------
void CheckOther::invalidScanf()
{
    if (!_settings->isEnabled("style"))
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        const Token *formatToken = 0;
        if (Token::Match(tok, "scanf|vscanf ( %str% ,"))
            formatToken = tok->tokAt(2);
        else if (Token::Match(tok, "sscanf|vsscanf|fscanf|vfscanf (")) {
            const Token* nextArg = tok->tokAt(2)->nextArgument();
            if (nextArg && Token::Match(nextArg, "%str%"))
                formatToken = nextArg;
            else
                continue;
        } else
            continue;

        bool format = false;

        // scan the string backwards, so we dont need to keep states
        const std::string &formatstr(formatToken->str());
        for (unsigned int i = 1; i < formatstr.length(); i++) {
            if (formatstr[i] == '%')
                format = !format;

            else if (!format)
                continue;

            else if (std::isdigit(formatstr[i])) {
                format = false;
            }

            else if (std::isalpha(formatstr[i])) {
                if (formatstr[i] != 'c')  // #3490 - field width limits are not necessary for %c
                    invalidScanfError(tok);
                format = false;
            }
        }
    }
}

void CheckOther::invalidScanfError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "invalidscanf", "scanf without field width limits can crash with huge input data\n"
                "scanf without field width limits can crash with huge input data. To fix this error "
                "message add a field width specifier:\n"
                "    %s => %20s\n"
                "    %i => %3i\n"
                "\n"
                "Sample program that can crash:\n"
                "\n"
                "#include <stdio.h>\n"
                "int main()\n"
                "{\n"
                "    int a;\n"
                "    scanf(\"%i\", &a);\n"
                "    return 0;\n"
                "}\n"
                "\n"
                "To make it crash:\n"
                "perl -e 'print \"5\"x2100000' | ./a.out");
}

//---------------------------------------------------------------------------
//    printf("%u", "xyz"); // Wrong argument type
//    printf("%u%s", 1); // Too few arguments
//    printf("", 1); // Too much arguments
//---------------------------------------------------------------------------
static bool isComplexType(const Variable* var, const Token* varTypeTok)
{
    if (var->type())
        return(true);

    static std::set<std::string> knownTypes;
    if (knownTypes.empty()) {
        knownTypes.insert("struct"); // If a type starts with the struct keyword, its a complex type
        knownTypes.insert("string");
    }

    if (varTypeTok->str() == "std")
        varTypeTok = varTypeTok->tokAt(2);
    return(knownTypes.find(varTypeTok->str()) != knownTypes.end() && !var->isPointer() && !var->isArray());
}

static bool isKnownType(const Variable* var, const Token* varTypeTok)
{
    return(varTypeTok->isStandardType() || varTypeTok->next()->isStandardType() || isComplexType(var, varTypeTok));
}

void CheckOther::checkWrongPrintfScanfArguments()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isName()) continue;

        const Token* argListTok = 0; // Points to first va_list argument
        std::string formatString;

        if (Token::Match(tok, "printf|scanf ( %str%")) {
            formatString = tok->strAt(2);
            if (tok->strAt(3) == ",") {
                argListTok = tok->tokAt(4);
            } else if (tok->strAt(3) == ")") {
                argListTok = 0;
            } else {
                continue;
            }
        } else if (Token::Match(tok, "sprintf|fprintf|sscanf|fscanf ( %any%")) {
            const Token* formatStringTok = tok->tokAt(2)->nextArgument(); // Find second parameter (format string)
            if (Token::Match(formatStringTok, "%str% ,")) {
                argListTok = formatStringTok->nextArgument(); // Find third parameter (first argument of va_args)
                formatString = formatStringTok->str();
            } else if (Token::Match(formatStringTok, "%str% )")) {
                argListTok = 0; // Find third parameter (first argument of va_args)
                formatString = formatStringTok->str();
            } else {
                continue;
            }
        } else if (Token::Match(tok, "snprintf|fnprintf ( %any%")) {
            const Token* formatStringTok = tok->tokAt(2);
            for (int i = 0; i < 2 && formatStringTok; i++) {
                formatStringTok = formatStringTok->nextArgument(); // Find third parameter (format string)
            }
            if (Token::Match(formatStringTok, "%str% ,")) {
                argListTok = formatStringTok->nextArgument(); // Find fourth parameter (first argument of va_args)
                formatString = formatStringTok->str();
            }
            if (Token::Match(formatStringTok, "%str% )")) {
                argListTok = 0; // Find fourth parameter (first argument of va_args)
                formatString = formatStringTok->str();
            } else {
                continue;
            }
        } else {
            continue;
        }

        // Count format string parameters..
        bool scan = Token::Match(tok, "sscanf|fscanf|scanf");
        unsigned int numFormat = 0;
        bool percent = false;
        const Token* argListTok2 = argListTok;
        for (std::string::iterator i = formatString.begin(); i != formatString.end(); ++i) {
            if (*i == '%') {
                percent = !percent;
            } else if (percent && *i == '[') {
                while (i != formatString.end()) {
                    if (*i == ']') {
                        numFormat++;
                        if (argListTok)
                            argListTok = argListTok->nextArgument();
                        percent = false;
                        break;
                    }
                    ++i;
                }
                if (i == formatString.end())
                    break;
            } else if (percent) {
                percent = false;

                bool _continue = false;
                while (i != formatString.end() && *i != ']' && !std::isalpha(*i)) {
                    if (*i == '*') {
                        if (scan)
                            _continue = true;
                        else {
                            numFormat++;
                            if (argListTok)
                                argListTok = argListTok->nextArgument();
                        }
                    }
                    ++i;
                }
                if (i == formatString.end())
                    break;
                if (_continue)
                    continue;

                if (scan || *i != 'm') { // %m is a non-standard extension that requires no parameter on print functions.
                    numFormat++;

                    // Perform type checks
                    if (_settings->isEnabled("style") && Token::Match(argListTok, "%any% ,|)")) { // We can currently only check the type of arguments matching this simple pattern.
                        const Variable* variableInfo = symbolDatabase->getVariableFromVarId(argListTok->varId());
                        const Token* varTypeTok = variableInfo ? variableInfo->typeStartToken() : NULL;
                        if (varTypeTok && varTypeTok->str() == "static")
                            varTypeTok = varTypeTok->next();

                        if (scan && varTypeTok) {
                            if ((!variableInfo->isPointer() && !variableInfo->isArray()) || varTypeTok->str() == "const")
                                invalidScanfArgTypeError(tok, tok->str(), numFormat);
                        } else if (!scan) {
                            switch (*i) {
                            case 's':
                                if (variableInfo && !Token::Match(argListTok, "%str%") && isKnownType(variableInfo, varTypeTok) && (!variableInfo->isPointer() && !variableInfo->isArray()))
                                    invalidPrintfArgTypeError_s(tok, numFormat);
                                break;
                            case 'n':
                                if ((varTypeTok && isKnownType(variableInfo, varTypeTok) && ((!variableInfo->isPointer() && !variableInfo->isArray()) || varTypeTok->str() == "const")) || Token::Match(argListTok, "%str%"))
                                    invalidPrintfArgTypeError_n(tok, numFormat);
                                break;
                            case 'c':
                            case 'd':
                            case 'i':
                            case 'u':
                            case 'x':
                            case 'X':
                            case 'o':
                                if (varTypeTok && varTypeTok->str() == "const")
                                    varTypeTok = varTypeTok->next();
                                if ((varTypeTok && isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "unsigned|signed| bool|short|long|int|char|size_t|unsigned|signed") && !variableInfo->isPointer() && !variableInfo->isArray()))
                                    invalidPrintfArgTypeError_int(tok, numFormat, *i);
                                else if (Token::Match(argListTok, "%str%"))
                                    invalidPrintfArgTypeError_int(tok, numFormat, *i);
                                break;
                            case 'p':
                                if (varTypeTok && varTypeTok->str() == "const")
                                    varTypeTok = varTypeTok->next();
                                if (varTypeTok && isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "unsigned|signed| short|long|int|size_t|unsigned|signed") && !variableInfo->isPointer() && !variableInfo->isArray())
                                    invalidPrintfArgTypeError_p(tok, numFormat);
                                else if (Token::Match(argListTok, "%str%"))
                                    invalidPrintfArgTypeError_p(tok, numFormat);
                                break;
                            case 'e':
                            case 'E':
                            case 'f':
                            case 'g':
                            case 'G':
                                if (varTypeTok && varTypeTok->str() == "const")
                                    varTypeTok = varTypeTok->next();
                                if (varTypeTok && ((isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "float|double")) || variableInfo->isPointer() || variableInfo->isArray()))
                                    invalidPrintfArgTypeError_float(tok, numFormat, *i);
                                else if (Token::Match(argListTok, "%str%"))
                                    invalidPrintfArgTypeError_float(tok, numFormat, *i);
                                break;
                            default:
                                break;
                            }
                        }
                    }

                    if (argListTok)
                        argListTok = argListTok->nextArgument(); // Find next argument
                }
            }
        }

        // Count printf/scanf parameters..
        unsigned int numFunction = 0;
        while (argListTok2) {
            numFunction++;
            argListTok2 = argListTok2->nextArgument(); // Find next argument
        }

        // Mismatching number of parameters => warning
        if (numFormat != numFunction)
            wrongPrintfScanfArgumentsError(tok, tok->str(), numFormat, numFunction);
    }
}

void CheckOther::wrongPrintfScanfArgumentsError(const Token* tok,
        const std::string &functionName,
        unsigned int numFormat,
        unsigned int numFunction)
{
    Severity::SeverityType severity = numFormat > numFunction ? Severity::error : Severity::warning;
    if (severity != Severity::error && !_settings->isEnabled("style"))
        return;

    std::ostringstream errmsg;
    errmsg << functionName
           << " format string has "
           << numFormat
           << " parameters but "
           << (numFormat > numFunction ? "only " : "")
           << numFunction
           << " are given";

    reportError(tok, severity, "wrongPrintfScanfArgNum", errmsg.str());
}

void CheckOther::invalidScanfArgTypeError(const Token* tok, const std::string &functionName, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << functionName << " argument no. " << numFormat << ": requires non-const pointers or arrays as arguments";
    reportError(tok, Severity::warning, "invalidScanfArgType", errmsg.str());
}
void CheckOther::invalidPrintfArgTypeError_s(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%s in format string (no. " << numFormat << ") requires a char* given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_s", errmsg.str());
}
void CheckOther::invalidPrintfArgTypeError_n(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%n in format string (no. " << numFormat << ") requires a pointer to an non-const integer given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_n", errmsg.str());
}
void CheckOther::invalidPrintfArgTypeError_p(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%p in format string (no. " << numFormat << ") requires an integer or pointer given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_p", errmsg.str());
}
void CheckOther::invalidPrintfArgTypeError_int(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires an integer given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_int", errmsg.str());
}
void CheckOther::invalidPrintfArgTypeError_float(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires a floating point number given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_float", errmsg.str());
}

//---------------------------------------------------------------------------
//    if (!x==3) <- Probably meant to be "x!=3"
//---------------------------------------------------------------------------
void CheckOther::checkComparisonOfBoolWithInt()
{
    if (!_settings->isEnabled("style"))
        return;

    std::map<unsigned int, bool> boolvars; // Contains all declarated standard type variables and indicates whether its a bool or not.
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[{};(,] %type% %var% [;=,)]") && tok->next()->isStandardType() && tok->tokAt(2)->varId() > 0) { // Declaration of standard type variable
            boolvars[tok->tokAt(2)->varId()] = (tok->strAt(1) == "bool");
        } else if (Token::Match(tok, "%var% >|>=|==|!=|<=|< %num%")) { // Comparing variable with number
            const Token *varTok = tok;
            const Token *numTok = tok->tokAt(2);
            std::map<unsigned int, bool>::const_iterator iVar = boolvars.find(varTok->varId());
            if (iVar != boolvars.end() && iVar->second && // Variable has to be a boolean
                ((tok->strAt(1) != "==" && tok->strAt(1) != "!=") ||
                 (MathLib::toLongNumber(numTok->str()) != 0 && MathLib::toLongNumber(numTok->str()) != 1))) { // == 0 and != 0 are allowed, for C also == 1 and != 1
                comparisonOfBoolWithIntError(varTok, numTok->str());
            }
        } else if (Token::Match(tok, "%num% >|>=|==|!=|<=|< %var%")) { // Comparing number with variable
            const Token *varTok = tok->tokAt(2);
            const Token *numTok = tok;
            std::map<unsigned int, bool>::const_iterator iVar = boolvars.find(varTok->varId());
            if (iVar != boolvars.end() && iVar->second && // Variable has to be a boolean
                ((tok->strAt(1) != "==" && tok->strAt(1) != "!=") ||
                 (MathLib::toLongNumber(numTok->str()) != 0 && MathLib::toLongNumber(numTok->str()) != 1))) { // == 0 and != 0 are allowed, for C also == 1 and != 1
                comparisonOfBoolWithIntError(varTok, numTok->str());
            }
        } else if (Token::Match(tok, "true|false >|>=|==|!=|<=|< %var%")) { // Comparing boolean constant with variable
            const Token *varTok = tok->tokAt(2);
            const Token *constTok = tok;
            std::map<unsigned int, bool>::const_iterator iVar = boolvars.find(varTok->varId());
            if (iVar != boolvars.end() && !iVar->second) { // Variable has to be of non-boolean standard type
                comparisonOfBoolWithIntError(varTok, constTok->str());
            }
        } else if (Token::Match(tok, "%var% >|>=|==|!=|<=|< true|false")) { // Comparing variable with boolean constant
            const Token *varTok = tok;
            const Token *constTok = tok->tokAt(2);
            std::map<unsigned int, bool>::const_iterator iVar = boolvars.find(varTok->varId());
            if (iVar != boolvars.end() && !iVar->second) { // Variable has to be of non-boolean standard type
                comparisonOfBoolWithIntError(varTok, constTok->str());
            }
        } else if (Token::Match(tok, "%var% >|>=|==|!=|<=|< %var%") &&
                   !Token::Match(tok->tokAt(3), ".|::|(")) { // Comparing two variables, one of them boolean, one of them integer
            const Token *var1Tok = tok->tokAt(2);
            const Token *var2Tok = tok;
            std::map<unsigned int, bool>::const_iterator iVar1 = boolvars.find(var1Tok->varId());
            std::map<unsigned int, bool>::const_iterator iVar2 = boolvars.find(var2Tok->varId());
            if (iVar1 != boolvars.end() && iVar2 != boolvars.end()) {
                if (iVar1->second && !iVar2->second) // Comparing boolean with non-bool standard type
                    comparisonOfBoolWithIntError(var2Tok, var1Tok->str());
                else if (!iVar1->second && iVar2->second) // Comparing non-bool standard type with boolean
                    comparisonOfBoolWithIntError(var2Tok, var2Tok->str());
            }
        } else if (Token::Match(tok, "( ! %var% ==|!= %num% )")) {
            const Token *numTok = tok->tokAt(4);
            if (numTok && numTok->str() != "0" && numTok->str() != "1") {
                comparisonOfBoolWithIntError(numTok, "!"+tok->strAt(2));
            }
        } else if (Token::Match(tok, "( %num% ==|!= ! %var% )")) {
            const Token *numTok = tok->next();
            if (numTok && numTok->str() != "0" && numTok->str() != "1") {
                comparisonOfBoolWithIntError(numTok, "!"+tok->strAt(4));
            }
        }
    }
}

void CheckOther::comparisonOfBoolWithIntError(const Token *tok, const std::string &expression)
{
    reportError(tok, Severity::warning, "comparisonOfBoolWithInt",
                "Comparison of a boolean with integer that is neither 1 nor 0\n"
                "The expression \"" + expression + "\" is of type 'bool' "
                "and it is compared against a integer value that is "
                "neither 1 nor 0.");
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
            labelName = tok->tokAt(1);
        }

        if (secondBreak) {
            if (Token::Match(secondBreak, "continue|goto|throw") ||
                (secondBreak->str() == "return" && (tok->str() == "return" || secondBreak->strAt(1) == ";"))) { // return with value after statements like throw can be necessary to make a function compile
                duplicateBreakError(secondBreak);
                tok = Token::findmatch(secondBreak, "[}:]");
            } else if (secondBreak->str() == "break") { // break inside switch as second break statement should not issue a warning
                if (tok->str() == "break") // If the previous was a break, too: Issue warning
                    duplicateBreakError(secondBreak);
                else {
                    unsigned int indent = 0;
                    for (const Token* tok2 = tok; tok2; tok2 = tok2->previous()) { // Check, if the enclosing scope is a switch (TODO: Can we use SymbolDatabase here?)
                        if (tok2->str() == "}")
                            indent++;
                        else if (indent == 0 && tok2->str() == "{" && tok2->strAt(-1) == ")") {
                            if (tok2->previous()->link()->strAt(-1) != "switch") {
                                duplicateBreakError(secondBreak);
                                break;
                            } else
                                break;
                        } else if (tok2->str() == "{")
                            indent--;
                    }
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
                    unreachableCodeError(secondBreak);
                tok = Token::findmatch(secondBreak, "[}:]");
            } else
                tok = secondBreak;

            if (!tok)
                break;
        }
    }
}

void CheckOther::duplicateBreakError(const Token *tok)
{
    reportError(tok, Severity::style, "duplicateBreak",
                "Consecutive return, break, continue, goto or throw statements are unnecessary.\n"
                "The second of the two statements can never be executed, and so should be removed.");
}

void CheckOther::unreachableCodeError(const Token *tok)
{
    reportError(tok, Severity::style, "unreachableCode",
                "Statements following return, break, continue, goto or throw will never be executed.");
}

//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------
static bool isUnsigned(const Variable* var)
{
    return(var && var->typeStartToken()->isUnsigned() && var->typeStartToken() == var->typeEndToken());
}

void CheckOther::checkUnsignedDivision()
{
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    // Check for "ivar / uvar" and "uvar / ivar"
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "[).]") && Token::Match(tok->next(), "%var% / %num%")) {
            if (tok->strAt(3)[0] == '-' && isUnsigned(symbolDatabase->getVariableFromVarId(tok->next()->varId()))) {
                udivError(tok->next());
            }
        }

        else if (Token::Match(tok, "(|[|=|%op% %num% / %var%")) {
            if (tok->strAt(1)[0] == '-' && isUnsigned(symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId()))) {
                udivError(tok->next());
            }
        }
    }
}

void CheckOther::udivError(const Token *tok)
{
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

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // Walk through all tokens..
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            // Skip function local class and struct declarations..
            if ((tok->str() == "class") || (tok->str() == "struct") || (tok->str() == "union")) {
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "{") {
                        tok = tok2->link();
                        break;
                    }
                    if (Token::Match(tok2, "[,);]")) {
                        break;
                    }
                }
                if (! tok)
                    break;
            }

            if (Token::Match(tok, "[{};]")) {
                // First token of statement..
                const Token *tok1 = tok->next();
                if (! tok1)
                    continue;

                if ((tok1->str() == "return") ||
                    (tok1->str() == "throw") ||
                    (tok1->str() == "delete") ||
                    (tok1->str() == "goto") ||
                    (tok1->str() == "else"))
                    continue;

                // Variable declaration?
                if (Token::Match(tok1, "%type% %var% ; %var% = %num% ;")) {
                    // Tokenizer modify "int i = 0;" to "int i; i = 0;",
                    // so to handle this situation we just skip
                    // initialization (see ticket #272).
                    const unsigned int firstVarId = tok1->next()->varId();
                    const unsigned int secondVarId = tok1->tokAt(3)->varId();
                    if (firstVarId > 0 && firstVarId == secondVarId) {
                        lookupVar(tok1->tokAt(6), tok1->strAt(1));
                    }
                } else if (tok1->isStandardType() && Token::Match(tok1, "%type% %var% [;=]")) {
                    lookupVar(tok1, tok1->strAt(1));
                }
            }
        }
    }
}

void CheckOther::lookupVar(const Token *tok1, const std::string &varname)
{
    const Token *tok = tok1;

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
                if (Token::findmatch(tok, varname.c_str(), tok->link())) {
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
        else if (Token::simpleMatch(tok, (std::string("& ") + varname).c_str())) {
            return;
        }

        else if (tok->str() == varname) {
            if (indentlevel == 0)
                return;
            used1 = true;
            if (for_or_while && !Token::simpleMatch(tok->next(), "="))
                used2 = true;
            if (used1 && used2)
                return;
        }

        else if (indentlevel == 0) {
            // %unknown% ( %any% ) {
            // If %unknown% is anything except if, we assume
            // that it is a for or while loop or a macro hiding either one
            if (Token::simpleMatch(tok->next(), "(") &&
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
        variableScopeError(tok1, varname);
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

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type == Scope::eFunction && i->function) {
            for (const Token* tok = i->function->arg->next(); tok; tok = tok->nextArgument()) {
                // TODO: False negatives. This pattern only checks for string.
                //       Investigate if there are other classes in the std
                //       namespace and add them to the pattern. There are
                //       streams for example (however it seems strange with
                //       const stream parameter).
                if (Token::Match(tok, "const std :: string %var% [,)]")) {
                    passedByValueError(tok, tok->strAt(4));
                } else if (Token::Match(tok, "const std :: %type% < std| ::| %type% > %var% [,)]")) {
                    passedByValueError(tok, Token::findsimplematch(tok->tokAt(5), ">")->strAt(1));
                } else if (Token::Match(tok, "const std :: %type% < std| ::| %type% , std| ::| %type% > %var% [,)]")) {
                    passedByValueError(tok, Token::findsimplematch(tok->tokAt(7), ">")->strAt(1));
                } else if (Token::Match(tok, "const %type% %var% [,)]")) {
                    // Check if type is a struct or class.
                    if (symbolDatabase->isClassOrStruct(tok->strAt(1))) {
                        passedByValueError(tok, tok->strAt(2));
                    }
                }
            }
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
static bool isSignedChar(const Variable* var)
{
    return(var && var->nameToken()->previous()->str() == "char" && !var->nameToken()->previous()->isUnsigned() && var->nameToken()->next()->str() != "[");
}

void CheckOther::checkCharVariable()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if ((tok->str() != ".") && Token::Match(tok->next(), "%var% [ %var% ]")) {
            const Variable* var = symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId());
            if (isSignedChar(var))
                charArrayIndexError(tok->next());
        }

        else if (Token::Match(tok, "[;{}] %var% = %any% [&|] %any% ;")) {
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
            if (!(var && Token::Match(var->typeEndToken(), "short|int|long")))
                continue;

            // This is an error..
            charBitOpError(tok->tokAt(4));
        }

        else if (Token::Match(tok, "[;{}] %var% = %any% [&|] ( * %var% ) ;")) {
            const Variable* var = symbolDatabase->getVariableFromVarId(tok->tokAt(7)->varId());
            if (!var || !var->isPointer())
                continue;
            // it's ok with a bitwise and where the other operand is 0xff or less..
            if (tok->strAt(4) == "&" && tok->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok->strAt(3)))
                continue;

            // is the result stored in a short|int|long?
            var = symbolDatabase->getVariableFromVarId(tok->next()->varId());
            if (!(var && Token::Match(var->typeEndToken(), "short|int|long")))
                continue;

            // This is an error..
            charBitOpError(tok->tokAt(4));
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
static bool isChar(const Variable* var)
{
    return(var && var->nameToken()->previous()->str() == "char" && var->nameToken()->next()->str() != "[");
}

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
            if (tok->strAt(3)[0] == '\'')
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
            if (tok->varId() == 0 && Token::Match(tok, "log|log10 ( %num% )")) {
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
            else if (tok->varId() == 0 &&
                     Token::Match(tok, "acos|asin ( %num% )") &&
                     std::fabs(MathLib::toDoubleNumber(tok->strAt(2))) > 1.0) {
                mathfunctionCallError(tok);
            }
            // sqrt( x ): if x is negative the result is undefined
            else if (tok->varId() == 0 &&
                     Token::Match(tok, "sqrt|sqrtf|sqrtl ( %num% )") &&
                     MathLib::isNegative(tok->strAt(2))) {
                mathfunctionCallError(tok);
            }
            // atan2 ( x , y): x and y can not be zero, because this is mathematically not defined
            else if (tok->varId() == 0 &&
                     Token::Match(tok, "atan2 ( %num% , %num% )") &&
                     MathLib::isNullValue(tok->strAt(2)) &&
                     MathLib::isNullValue(tok->strAt(4))) {
                mathfunctionCallError(tok, 2);
            }
            // fmod ( x , y) If y is zero, then either a range error will occur or the function will return zero (implementation-defined).
            else if (tok->varId() == 0 &&
                     Token::Match(tok, "fmod ( %num% , %num% )") &&
                     MathLib::isNullValue(tok->strAt(4))) {
                mathfunctionCallError(tok, 2);
            }
            // pow ( x , y) If x is zero, and y is negative --> division by zero
            else if (tok->varId() == 0 &&
                     Token::Match(tok, "pow ( %num% , %num% )") &&
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
                "unusedScopedObject", "instance of \"" + varname + "\" object destroyed immediately");
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
        else if (Token::Match(tok, "++|--"))
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
        std::string expression = tok->tokAt(2)->stringify(tok->next()->link());

        // save the expression and its location
        expressionMap.insert(std::make_pair(expression, tok));

        // find the next else if (...) statement
        const Token *tok1 = tok->next()->link()->next()->link();

        // check all the else if (...) statements
        while (Token::simpleMatch(tok1, "} else if (") &&
               Token::simpleMatch(tok1->linkAt(3), ") {")) {
            // get the expression from the token stream
            expression = tok1->tokAt(4)->stringify(tok1->linkAt(3));

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
        const Token* tok = scope->classDef;

        if ((scope->type != Scope::eIf && scope->type != Scope::eElseIf) || !tok)
            continue;

        if (tok->str() == "else")
            tok = tok->next();

        // check all the code in the function for if (..) else
        if (tok && tok->next() && Token::simpleMatch(tok->next()->link(), ") {") &&
            Token::simpleMatch(tok->next()->link()->next()->link(), "} else {")) {
            // save if branch code
            std::string branch1 = tok->next()->link()->tokAt(2)->stringify(tok->next()->link()->next()->link());

            // find else branch
            const Token *tok1 = tok->next()->link()->next()->link();

            // save else branch code
            std::string branch2 = tok1->tokAt(3)->stringify(tok1->linkAt(2));

            // check for duplicates
            if (branch1 == branch2)
                duplicateBranchError(tok, tok1->tokAt(2));
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
        else if (tok->str() == "}" && _tokenizer->IsScopeNoReturn(tok)) {
            freedVariables.clear();
            closeDirVariables.clear();
        }

        // If a variable is passed to a function, remove it from the set of previously freed variables
        else if (Token::Match(tok, "%var% (") && !Token::Match(tok, "printf|sprintf|snprintf|fprintf")) {
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
        if (Token::Match(tok, "else|break|continue|goto|return")) {
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

        bool operator()(const Function &func) {
            bool matchingFunc = func.type == Function::eFunction &&
                                _tok->str() == func.token->str();
            // either a class function, or a global function with the same name
            return (_scope && _scope == func.functionScope && matchingFunc) ||
                   (!_scope && matchingFunc);
        }
        const Scope *_scope;
        const Token *_tok;
    };

    bool inconclusiveFunctionCall(const SymbolDatabase *symbolDatabase,
                                  const std::list<Function> &constFunctions,
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
                    std::list<Function>::const_iterator it = std::find_if(constFunctions.begin(),
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
                    std::list<Function> &constFunctions)
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
        const std::list<Function> &_constFunctions;
    };

    bool notconst(const Function &func)
    {
        return !func.isConst;
    }

    void getConstFunctions(const SymbolDatabase *symbolDatabase, std::list<Function> &constFunctions)
    {
        std::list<Scope>::const_iterator scope;
        for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
            std::list<Function>::const_iterator func;
            // only add const functions that do not have a non-const overloaded version
            // since it is pretty much impossible to tell which is being called.
            typedef std::map<std::string, std::list<Function> > StringFunctionMap;
            StringFunctionMap functionsByName;
            for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                StringFunctionMap::iterator it = functionsByName.find(func->token->str());
                Scope *currScope = const_cast<Scope*>(&*scope);
                if (it == functionsByName.end()) {
                    std::list<Function> tmp;
                    tmp.push_back(*func);
                    tmp.back().functionScope = currScope;
                    functionsByName[func->token->str()] = tmp;
                } else {
                    it->second.push_back(*func);
                    it->second.back().functionScope = currScope;
                }
            }
            for (StringFunctionMap::iterator it = functionsByName.begin();
                 it != functionsByName.end(); ++it) {
                std::list<Function>::const_iterator nc = std::find_if(it->second.begin(), it->second.end(), notconst);
                if (nc == it->second.end()) {
                    // ok to add all of them
                    constFunctions.splice(constFunctions.end(), it->second);
                }
            }
        }
    }

}

void CheckOther::checkExpressionRange(const std::list<Function> &constFunctions,
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
        unsigned int parantheses = 0;  // ()
        unsigned int brackets = 0;     // []

        // taking address?
        if (Token::Match(it->second.end->previous(), "%op% &")) {
            continue;
        }

        for (const Token *tok = it->second.start; tok && tok != it->second.end; tok = tok->next()) {
            if (tok->str() == "(") {
                ++parantheses;
            } else if (tok->str() == ")") {
                if (parantheses == 0) {
                    valid = false;
                    break;
                }
                --parantheses;
            } else if (tok->str() == "[") {
                ++brackets;
            } else if (tok->str() == "]") {
                if (brackets == 0) {
                    valid = false;
                    break;
                }
                --brackets;
            } else if (tok->str() == "++" || tok->str() == "--") {
                valid = false;
                break;
            }
        }

        if (!valid || parantheses!=0 || brackets!=0)
            continue;

        const ExpressionTokens &expr = it->second;
        if (expr.count > 1 && !expr.inconclusiveFunction) {
            duplicateExpressionError(expr.start, expr.start, opName);
        }
    }
}

void CheckOther::complexDuplicateExpressionCheck(const std::list<Function> &constFunctions,
        const Token *classStart,
        const std::string &toCheck,
        const std::string &alt)
{
    std::string statementStart(",|=|return");
    if (!alt.empty())
        statementStart += "|" + alt;
    std::string statementEnd(";|,");
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
    std::list<Function> constFunctions;
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

                duplicateExpressionError(tok->next(), tok->tokAt(3), tok->strAt(2));
            } else if (Token::Match(tok, ",|=|return|(|&&|%oror% %var% . %var% ==|!=|<=|>=|<|>|- %var% . %var% )|&&|%oror%|;|,") &&
                       tok->strAt(1) == tok->strAt(5) && tok->strAt(3) == tok->strAt(7)) {
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
    if (!_settings->isEnabled("style") && !_settings->isEnabled("performance"))
        return;

    const char pattern1[] = "strncmp|strcmp|stricmp|strcmpi|strcasecmp|wcscmp ( %str% , %str% ";
    const char pattern2[] = "QString :: compare ( %str% , %str% )";
    const char pattern3[] = "strncmp|strcmp|stricmp|strcmpi|strcasecmp|wcscmp ( %var% , %var% ";

    const Token *tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern1)) != NULL) {
        alwaysTrueFalseStringCompareError(tok, tok->strAt(2), tok->strAt(4));
        tok = tok->tokAt(5);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern2)) != NULL) {
        alwaysTrueFalseStringCompareError(tok, tok->strAt(4), tok->strAt(6));
        tok = tok->tokAt(7);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern3)) != NULL) {
        const Token *var1 = tok->tokAt(2);
        const Token *var2 = tok->tokAt(4);
        const std::string &str1 = var1->str();
        const std::string &str2 = var2->str();
        if (str1 == str2)
            alwaysTrueStringVariableCompareError(tok, str1, str2);
        tok = tok->tokAt(5);
    }
}

void CheckOther::alwaysTrueFalseStringCompareError(const Token *tok, const std::string& str1, const std::string& str2)
{
    const std::size_t stringLen = 10;
    const std::string string1 = (str1.size() < stringLen) ? str1 : (str1.substr(0, stringLen-2) + "..");
    const std::string string2 = (str2.size() < stringLen) ? str2 : (str2.substr(0, stringLen-2) + "..");

    if (str1 == str2) {
        reportError(tok, Severity::warning, "staticStringCompare",
                    "Comparison of always identical static strings.\n"
                    "The compared strings, '" + string1 + "' and '" + string2 + "', are always identical. "
                    "If the purpose is to compare these two strings, the comparison is unnecessary. "
                    "If the strings are supposed to be different, then there is a bug somewhere.");
    } else if (_settings->isEnabled("performance")) {
        reportError(tok, Severity::performance, "staticStringCompare",
                    "Unnecessary comparison of static strings.\n"
                    "The compared strings, '" + string1 + "' and '" + string2 + "', are static and always different. "
                    "If the purpose is to compare these two strings, the comparison is unnecessary.");
    }
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
            unsigned int parlevel = 0;
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    ++parlevel;
                else if (tok2->str() == ")") {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                } else if (Token::Match(tok2, "+|/")) {
                    sizeofCalculationError(tok2);
                    break;
                }
            }
        }
    }
}

void CheckOther::sizeofCalculationError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "sizeofCalculation", "Found calculation inside sizeof()");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckOther::checkAssignBoolToPointer()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] %var% = %bool% ;")) {
            const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

            const Variable *var1(symbolDatabase->getVariableFromVarId(tok->next()->varId()));

            // Is variable a pointer?
            if (var1 && var1->nameToken()->strAt(-1) == "*")
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

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "&&|%oror% %any% ) ==|!=|>|< %num%")) {
            const std::string& op = tok->strAt(3);
            const std::string& num = tok->strAt(4);
            if ((op == "<" || num != "0") && (op == ">" || num != "1")) {
                comparisonOfBoolExpressionWithIntError(tok->next());
            }
        }

        else if (Token::Match(tok, "%num% ==|!=|>|< ( %any% &&|%oror%")) {
            const std::string& op = tok->strAt(1);
            const std::string& num = tok->str();
            if ((op == ">" || num != "0") && (op == "<" || num != "1")) {
                comparisonOfBoolExpressionWithIntError(tok->next());
            }
        }
    }
}

void CheckOther::comparisonOfBoolExpressionWithIntError(const Token *tok)
{
    reportError(tok, Severity::warning, "compareBoolExpressionWithInt",
                "Comparison of a boolean expression with an integer other than 0 or 1.");
}


//---------------------------------------------------------------------------
// Check testing sign of unsigned variables.
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
        for (const Token *tok = scope->classStart; tok && tok != scope->classStart->link(); tok = tok->next()) {
            if (Token::Match(tok, ";|(|&&|%oror% %var% <|<= 0 ;|)|&&|%oror%") && tok->next()->varId()) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->next()->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZeroError(tok->next(), tok->next()->str(), inconclusive);
            } else if (Token::Match(tok, ";|(|&&|%oror% 0 > %var% ;|)|&&|%oror%") && tok->tokAt(3)->varId()) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZeroError(tok->tokAt(3), tok->strAt(3), inconclusive);
            } else if (Token::Match(tok, ";|(|&&|%oror% 0 <= %var% ;|)|&&|%oror%") && tok->tokAt(3)->varId()) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedPositiveError(tok->tokAt(3), tok->strAt(3), inconclusive);
            } else if (Token::Match(tok, ";|(|&&|%oror% %var% >= 0 ;|)|&&|%oror%") && tok->next()->varId()) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->next()->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedPositiveError(tok->next(), tok->next()->str(), inconclusive);
            }
        }
    }
}

void CheckOther::unsignedLessThanZeroError(const Token *tok, const std::string &varname, bool inconclusive)
{
    if (inconclusive) {
        reportInconclusiveError(tok, Severity::style, "unsignedLessThanZero",
                                "Checking if unsigned variable '" + varname + "' is less than zero. This might be a false warning.\n"
                                "Checking if unsigned variable '" + varname + "' is less than zero. An unsigned "
                                "variable will never be negative so it is either pointless or an error to check if it is. "
                                "It's not known if the used constant is a template parameter or not and therefore "
                                "this message might be a false warning");
    } else {
        reportError(tok, Severity::style, "unsignedLessThanZero",
                    "Checking if unsigned variable '" + varname + "' is less than zero.\n"
                    "An unsigned variable will never be negative so it is either pointless or "
                    "an error to check if it is.");
    }
}

void CheckOther::unsignedPositiveError(const Token *tok, const std::string &varname, bool inconclusive)
{
    if (inconclusive) {
        reportInconclusiveError(tok, Severity::style, "unsignedPositive",
                                "An unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it. This might be a false warning.\n"
                                "An unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it. "
                                "It's not known if the used constant is a "
                                "template parameter or not and therefore this message might be a false warning");
    } else {
        reportError(tok, Severity::style, "unsignedPositive",
                    "An unsigned variable '" + varname + "' can't be negative so it is unnecessary to test it.");
    }
}
