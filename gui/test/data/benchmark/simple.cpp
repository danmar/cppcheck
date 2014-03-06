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
// Remove includes from the benchmark run
// Included files aren't found anyway
//#include "checkother.h"
//#include "mathlib.h"
//#include "symboldatabase.h"

//#include <cctype> // std::isupper
//#include <cmath> // fabs()
//#include <stack>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckOther instance;
}

//---------------------------------------------------------------------------

void CheckOther::checkIncrementBoolean()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% ++")) {
            if (tok->varId()) {
                const Token *declTok = Token::findmatch(_tokenizer->tokens(), "bool %varid%", tok->varId());
                if (declTok)
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
        "The operand of a postfix increment operator may be of type bool but it is deprecated by C++ Standard (Annex D-1) and the operand is always set to true\n"
    );
}

//---------------------------------------------------------------------------


void CheckOther::clarifyCalculation()
{
    if (!_settings->isEnabled("style"))
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->strAt(1) == "?") {
            // condition
            const Token *cond = tok;
            if (cond->isName() || cond->isNumber())
                cond = cond->previous();
            else if (cond->str() == ")")
                cond = cond->link()->previous();
            else
                continue;

            // calculation
            if (!cond->isArithmeticalOp())
                continue;

            const std::string &op = cond->str();
            cond = cond->previous();

            // skip previous multiplications..
            while (cond && cond->strAt(-1) == "*" && (cond->isName() || cond->isNumber()))
                cond = cond->tokAt(-2);

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


// Clarify condition '(x = a < 0)' into '((x = a) < 0)' or '(x = (a < 0))'
void CheckOther::clarifyCondition()
{
    if (!_settings->isEnabled("style"))
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "( %var% =")) {
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(" || tok2->str() == "[")
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "&&|%oror%|?|)"))
                    break;
                else if (Token::Match(tok2, "<|<=|==|!=|>|>= %num% )")) {
                    clarifyConditionError(tok);
                    break;
                }
            }
        }
    }
}

void CheckOther::clarifyConditionError(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "clarifyCondition",
                "Suspicious condition (assignment+comparison), it can be clarified with parentheses");
}



void CheckOther::warningOldStylePointerCast()
{
    if (!_settings->isEnabled("style") ||
        (_tokenizer->tokens() && _tokenizer->fileLine(_tokenizer->tokens()).find(".cpp") == std::string::npos))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Old style pointer casting..
        if (!Token::Match(tok, "( const| %type% * ) %var%") &&
            !Token::Match(tok, "( const| %type% * ) (| new"))
            continue;

        int addToIndex = 0;
        if (tok->tokAt(1)->str() == "const")
            addToIndex = 1;

        if (tok->tokAt(4 + addToIndex)->str() == "const")
            continue;

        // Is "type" a class?
        const std::string pattern("class " + tok->tokAt(1 + addToIndex)->str());
        if (!Token::findmatch(_tokenizer->tokens(), pattern.c_str()))
            continue;

        cstyleCastError(tok);
    }
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

void CheckOther::checkSizeofForNumericParameter()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof ( %num% )")
            || Token::Match(tok, "sizeof ( - %num% )")
            || Token::Match(tok, "sizeof %num%")
            || Token::Match(tok, "sizeof - %num%")
           ) {
            sizeofForNumericParameterError(tok);
        }
    }
}

void CheckOther::checkSizeofForArrayParameter()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof ( %var% )") || Token::Match(tok, "sizeof %var%")) {
            int tokIdx = 1;
            if (tok->tokAt(tokIdx)->str() == "(") {
                ++tokIdx;
            }
            if (tok->tokAt(tokIdx)->varId() > 0) {
                const Token *declTok = Token::findmatch(_tokenizer->tokens(), "%varid%", tok->tokAt(tokIdx)->varId());
                if (declTok) {
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
    const char switchPattern[] = "switch ( %any% ) { case";
    const char breakPattern[] = "break|continue|return|exit|goto|throw";
    const char functionPattern[] = "%var% (";

    // Find the beginning of a switch. E.g.:
    //   switch (var) { ...
    const Token *tok = Token::findmatch(_tokenizer->tokens(), switchPattern);
    while (tok) {

        // Check the contents of the switch statement
        std::map<unsigned int, const Token*> varsAssigned;
        int indentLevel = 0;
        for (const Token *tok2 = tok->tokAt(5); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                // Inside a conditional or loop. Don't mark variable accesses as being redundant. E.g.:
                //   case 3: b = 1;
                //   case 4: if (a) { b = 2; }    // Doesn't make the b=1 redundant because it's conditional
                if (Token::Match(tok2->previous(), ")|else {") && tok2->link()) {
                    const Token* endOfConditional = tok2->link();
                    for (const Token* tok3 = tok2; tok3 != endOfConditional; tok3 = tok3->next()) {
                        if (tok3->varId() != 0)
                            varsAssigned.erase(tok3->varId());
                        else if (Token::Match(tok3, functionPattern) || Token::Match(tok3, breakPattern))
                            varsAssigned.clear();
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
            }
            // Not a simple assignment so there may be good reason if this variable is assigned to twice. E.g.:
            //    case 3: b = 1;
            //    case 4: b++;
            else if (tok2->varId() != 0)
                varsAssigned.erase(tok2->varId());

            // Reset our record of assignments if there is a break or function call. E.g.:
            //    case 3: b = 1; break;
            if (Token::Match(tok2, functionPattern) || Token::Match(tok2, breakPattern))
                varsAssigned.clear();

        }

        tok = Token::findmatch(tok->next(), switchPattern);
    }
}


void CheckOther::checkSwitchCaseFallThrough()
{
    if (!(_settings->isEnabled("style") && _settings->experimental))
        return;

    const char switchPattern[] = "switch (";
    const char breakPattern[] = "break|continue|return|exit|goto|throw";

    // Find the beginning of a switch. E.g.:
    //   switch (var) { ...
    const Token *tok = Token::findmatch(_tokenizer->tokens(), switchPattern);
    while (tok) {

        // Check the contents of the switch statement
        std::stack<std::pair<Token *, bool> > ifnest;
        std::stack<Token *> loopnest;
        std::stack<Token *> scopenest;
        bool justbreak = true;
        bool firstcase = true;
        for (const Token *tok2 = tok->tokAt(1)->link()->tokAt(2); tok2; tok2 = tok2->next()) {
            if (Token::simpleMatch(tok2, "if (")) {
                tok2 = tok2->tokAt(1)->link()->next();
                if (tok2->link() == NULL) {
                    std::ostringstream errmsg;
                    errmsg << "unmatched if in switch: " << tok2->linenr();
                    reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                    break;
                }
                ifnest.push(std::make_pair(tok2->link(), false));
                justbreak = false;
            } else if (Token::simpleMatch(tok2, "while (")) {
                tok2 = tok2->tokAt(1)->link()->next();
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
                tok2 = tok2->tokAt(1);
                if (tok2->link() == NULL) {
                    std::ostringstream errmsg;
                    errmsg << "unmatched do in switch: " << tok2->linenr();
                    reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                    break;
                }
                loopnest.push(tok2->link());
                justbreak = false;
            } else if (Token::simpleMatch(tok2, "for (")) {
                tok2 = tok2->tokAt(1)->link()->next();
                if (tok2->link() == NULL) {
                    std::ostringstream errmsg;
                    errmsg << "unmatched for in switch: " << tok2->linenr();
                    reportError(_tokenizer->tokens(), Severity::debug, "debug", errmsg.str());
                    break;
                }
                loopnest.push(tok2->link());
                justbreak = false;
            } else if (Token::Match(tok2, switchPattern)) {
                // skip over nested switch, we'll come to that soon
                tok2 = tok2->tokAt(1)->link()->next()->link();
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

        tok = Token::findmatch(tok->next(), switchPattern);
    }
}


//---------------------------------------------------------------------------
//    int x = 1;
//    x = x;            // <- redundant assignment to self
//
//    int y = y;        // <- redundant initialization to self
//---------------------------------------------------------------------------
void CheckOther::checkSelfAssignment()
{
    if (!_settings->isEnabled("style"))
        return;

    // POD variables..
    std::set<unsigned int> pod;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->isStandardType() && Token::Match(tok->tokAt(2), "[,);]") && tok->next()->varId())
            pod.insert(tok->next()->varId());
    }

    const char selfAssignmentPattern[] = "%var% = %var% ;|=|)";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), selfAssignmentPattern);
    while (tok) {
        if (Token::Match(tok->previous(), "[;{}]") &&
            tok->varId() && tok->varId() == tok->tokAt(2)->varId() &&
            pod.find(tok->varId()) != pod.end()) {
            selfAssignmentError(tok, tok->str());
        }

        tok = Token::findmatch(tok->next(), selfAssignmentPattern);
    }
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
        const Token* varTok = Token::findmatch(tok->tokAt(2), "%var% --|++|+=|-=|*=|/=|&=|^=|=", endTok);
        if (varTok) {
            assignmentInAssertError(tok, varTok->str());
        } else if (NULL != (varTok = Token::findmatch(tok->tokAt(2), "--|++ %var%", endTok))) {
            assignmentInAssertError(tok, varTok->strAt(1));
        }

        tok = Token::findmatch(endTok->next(), assertPattern);
        endTok = tok ? tok->next()->link() : NULL;
    }
}

//---------------------------------------------------------------------------
//    if ((x != 1) || (x != 3))            // <- always true
//    if ((x == 1) && (x == 3))            // <- always false
//    if ((x < 1)  && (x > 3))             // <- always false
//    if ((x > 3)  || (x < 10))            // <- always true
//---------------------------------------------------------------------------
void CheckOther::checkIncorrectLogicOperator()
{
    if (!_settings->isEnabled("style"))
        return;

    const char conditionPattern[] = "if|while (";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), conditionPattern);
    const Token *endTok = tok ? tok->next()->link() : NULL;

    while (tok && endTok) {
        // Find a pair of OR'd terms, with or without parentheses
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
            op1Tok = logicTok->tokAt(1);
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
            if (Token::Match(term1Tok, "%var% %any% %num%")) {
                varId = term1Tok->varId();
                if (!varId) {
                    tok = Token::findmatch(endTok->next(), conditionPattern);
                    endTok = tok ? tok->next()->link() : NULL;
                    continue;
                }
                varFirst1 = true;
                firstConstant = term1Tok->tokAt(2)->str();
            } else if (Token::Match(term1Tok, "%num% %any% %var%")) {
                varId = term1Tok->tokAt(2)->varId();
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
                secondConstant = term2Tok->tokAt(2)->str();
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
            struct Condition {
                const char *before;
                Position   position1;
                const char *op1TokStr;
                const char *op2TokStr;
                Position   position2;
                const char *op3TokStr;
                const char *after;
                Relation   relation;
                bool       state;
            } conditions[] = {
                { "!!&&", NA,     "!=",   "||", NA,     "!=",   "!!&&", NotEqual,  true  }, // (x != 1) || (x != 3) <- always true
                { "(",    NA,     "==",   "&&", NA,     "==",   ")",    NotEqual,  false }, // (x == 1) && (x == 3) <- always false
                { "(",    First,  "<",    "&&", First,  ">",    ")",    LessEqual, false }, // (x < 1)  && (x > 3)  <- always false
                { "(",    First,  ">",    "&&", First,  "<",    ")",    MoreEqual, false }, // (x > 3)  && (x < 1)  <- always false
                { "(",    Second, ">",    "&&", First,  ">",    ")",    LessEqual, false }, // (1 > x)  && (x > 3)  <- always false
                { "(",    First,  ">",    "&&", Second, ">",    ")",    MoreEqual, false }, // (x > 3)  && (1 > x)  <- always false
                { "(",    First,  "<",    "&&", Second, "<",    ")",    LessEqual, false }, // (x < 1)  && (3 < x)  <- always false
                { "(",    Second, "<",    "&&", First,  "<",    ")",    MoreEqual, false }, // (3 < x)  && (x < 1)  <- always false
                { "(",    Second, ">",    "&&", Second, "<",    ")",    LessEqual, false }, // (1 > x)  && (3 < x)  <- always false
                { "(",    Second, "<",    "&&", Second, ">",    ")",    MoreEqual, false }, // (3 < x)  && (1 > x)  <- always false
                { "(",    First , ">|>=", "||", First,  "<|<=", ")",    Less,      true  }, // (x > 3)  || (x < 10) <- always true
                { "(",    First , "<|<=", "||", First,  ">|>=", ")",    More,      true  }, // (x < 10) || (x > 3)  <- always true
                { "(",    Second, "<|<=", "||", First,  "<|<=", ")",    Less,      true  }, // (3 < x)  || (x < 10) <- always true
                { "(",    First,  "<|<=", "||", Second, "<|<=", ")",    More,      true  }, // (x < 10) || (3 < x)  <- always true
                { "(",    First,  ">|>=", "||", Second, ">|>=", ")",    Less,      true  }, // (x > 3)  || (10 > x) <- always true
                { "(",    Second, ">|>=", "||", First,  ">|>=", ")",    More,      true  }, // (10 > x) || (x > 3)  <- always true
                { "(",    Second, "<|<=", "||", Second, ">|<=", ")",    Less,      true  }, // (3 < x)  || (10 > x) <- always true
                { "(",    Second, ">|>=", "||", Second, "<|<=", ")",    More,      true  }, // (10 > x) || (3 < x)  <- always true
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
                    (conditions[i].relation == MoreEqual && MathLib::isGreaterEqual(firstConstant, secondConstant)))
                    incorrectLogicOperatorError(term1Tok, conditions[i].state);
            }
        }

        tok = Token::findmatch(endTok->next(), conditionPattern);
        endTok = tok ? tok->next()->link() : NULL;
    }
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
    const Token *endTok = tok ? tok->tokAt(2)->link() : NULL;

    while (tok && endTok) {
        // Find a pass-by-value declaration in the catch(), excluding basic types
        // e.g. catch (std::exception err)
        const Token *tokType = Token::findmatch(tok, "%type% %var% )", endTok);
        if (tokType && !tokType->isStandardType()) {
            catchExceptionByValueError(tokType);
        }

        tok = Token::findmatch(endTok->next(), catchPattern);
        endTok = tok ? tok->tokAt(2)->link() : NULL;
    }
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
        int param = 1;
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
        while (tok2 && tok2->str() != ",")
            tok2 = tok2->next();
        if (!tok2)
            continue;

        // is any source buffer overlapping the target buffer?
        int parlevel = 0;
        while ((tok2 = tok2->next()) != NULL) {
            if (tok2->str() == "(")
                ++parlevel;
            else if (tok2->str() == ")") {
                --parlevel;
                if (parlevel < 0)
                    break;
            } else if (parlevel == 0 && Token::Match(tok2, ", %varid% [,)]", varid)) {
                sprintfOverlappingDataError(tok2->next(), tok2->next()->str());
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------

void CheckOther::invalidScanf()
{
    if (!_settings->isEnabled("style"))
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        const Token *formatToken = 0;
        if (Token::Match(tok, "scanf|vscanf ( %str% ,"))
            formatToken = tok->tokAt(2);
        else if (Token::Match(tok, "fscanf|vfscanf ( %var% , %str% ,"))
            formatToken = tok->tokAt(4);
        else
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
                invalidScanfError(tok);
                format = false;
            }
        }
    }
}

//---------------------------------------------------------------------------
//    if (!x==3) <- Probably meant to be "x!=3"
//---------------------------------------------------------------------------
void CheckOther::checkComparisonOfBoolWithInt()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "( ! %var% ==|!= %num% )")) {
            const Token *numTok = tok->tokAt(4);
            if (numTok && numTok->str() != "0") {
                comparisonOfBoolWithIntError(numTok, tok->strAt(2));
            }
        } else if (Token::Match(tok, "( %num% ==|!= ! %var% )")) {
            const Token *numTok = tok->tokAt(1);
            if (numTok && numTok->str() != "0") {
                comparisonOfBoolWithIntError(numTok, tok->strAt(4));
            }
        }
    }
}

//---------------------------------------------------------------------------
//    switch (x)
//    {
//        case 2:
//            y = a;
//            break;
//            break; // <- Redundant break
//        case 3:
//            y = b;
//    }
//---------------------------------------------------------------------------
void CheckOther::checkDuplicateBreak()
{
    if (!_settings->isEnabled("style"))
        return;

    const char breakPattern[] = "break|continue ; break|continue ;";

    // Find consecutive break or continue statements. e.g.:
    //   break; break;
    const Token *tok = Token::findmatch(_tokenizer->tokens(), breakPattern);
    while (tok) {
        duplicateBreakError(tok);
        tok = Token::findmatch(tok->next(), breakPattern);
    }
}


void CheckOther::sizeofForNumericParameterError(const Token *tok)
{
    reportError(tok, Severity::error,
                "sizeofwithnumericparameter", "Using sizeof with a numeric constant as function "
                "argument might not be what you intended.\n"
                "It is unusual to use constant value with sizeof. For example, this code:\n"
                "     int f() {\n"
                "         return sizeof(10);\n"
                "     }\n"
                " returns 4 (in 32-bit systems) or 8 (in 64-bit systems) instead of 10."
               );
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
// Check for unsigned divisions
//---------------------------------------------------------------------------

void CheckOther::checkUnsignedDivision()
{
    if (!_settings->isEnabled("style"))
        return;

    // Check for "ivar / uvar" and "uvar / ivar"
    std::map<unsigned int, char> varsign;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[{};(,] %type% %var% [;=,)]")) {
            if (tok->tokAt(1)->isUnsigned())
                varsign[tok->tokAt(2)->varId()] = 'u';
            else
                varsign[tok->tokAt(2)->varId()] = 's';
        }

        else if (!Token::Match(tok, "[).]") && Token::Match(tok->next(), "%var% / %num%")) {
            if (tok->strAt(3)[0] == '-') {
                char sign1 = varsign[tok->tokAt(1)->varId()];
                if (sign1 == 'u') {
                    udivError(tok->next());
                }
            }
        }

        else if (Token::Match(tok, "(|[|=|%op% %num% / %var%")) {
            if (tok->strAt(1)[0] == '-') {
                char sign2 = varsign[tok->tokAt(3)->varId()];
                if (sign2 == 'u') {
                    udivError(tok->next());
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
// memset(p, y, 0 /* bytes to fill */) <- 2nd and 3rd arguments inverted
//---------------------------------------------------------------------------
void CheckOther::checkMemsetZeroBytes()
{
    const Token *tok = _tokenizer->tokens();
    while (tok && ((tok = Token::findmatch(tok, "memset ( %var% , %num% , 0 )")) != NULL)) {
        memsetZeroBytesError(tok, tok->strAt(2));
        tok = tok->tokAt(8);
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Usage of function variables
//---------------------------------------------------------------------------

/**
 * @brief This class is used to capture the control flow within a function.
 */
class ScopeInfo {
public:
    ScopeInfo() : _token(NULL), _parent(NULL) { }
    ScopeInfo(const Token *token, ScopeInfo *parent_) : _token(token), _parent(parent_) { }
    ~ScopeInfo();

    ScopeInfo *parent() {
        return _parent;
    }
    ScopeInfo *addChild(const Token *token);
    void remove(ScopeInfo *scope);

private:
    const Token *_token;
    ScopeInfo *_parent;
    std::list<ScopeInfo *> _children;
};

ScopeInfo::~ScopeInfo()
{
    while (!_children.empty()) {
        delete *_children.begin();
        _children.pop_front();
    }
}

ScopeInfo *ScopeInfo::addChild(const Token *token)
{
    ScopeInfo *temp = new ScopeInfo(token, this);

    _children.push_back(temp);

    return temp;
}

void ScopeInfo::remove(ScopeInfo *scope)
{
    std::list<ScopeInfo *>::iterator it;

    for (it = _children.begin(); it != _children.end(); ++it) {
        if (*it == scope) {
            delete *it;
            _children.erase(it);
            break;
        }
    }
}

/**
 * @brief This class is used create a list of variables within a function.
 */
class Variables {
public:
    enum VariableType { standard, array, pointer, reference, pointerArray, referenceArray, pointerPointer };

    /** Store information about variable usage */
    class VariableUsage {
    public:
        VariableUsage(const Token *name = 0,
                      VariableType type = standard,
                      ScopeInfo *scope = NULL,
                      bool read = false,
                      bool write = false,
                      bool modified = false,
                      bool allocateMemory = false) :
            _name(name),
            _type(type),
            _scope(scope),
            _read(read),
            _write(write),
            _modified(modified),
            _allocateMemory(allocateMemory) {
        }

        /** variable is used.. set both read+write */
        void use() {
            _read = true;
            _write = true;
        }

        /** is variable unused? */
        bool unused() const {
            return (_read == false && _write == false);
        }

        const Token *_name;
        VariableType _type;
        ScopeInfo *_scope;
        bool _read;
        bool _write;
        bool _modified; // read/modify/write
        bool _allocateMemory;
        std::set<unsigned int> _aliases;
        std::set<ScopeInfo *> _assignments;
    };

    typedef std::map<unsigned int, VariableUsage> VariableMap;

    void clear() {
        _varUsage.clear();
    }
    VariableMap &varUsage() {
        return _varUsage;
    }
    void addVar(const Token *name, VariableType type, ScopeInfo *scope, bool write_);
    void allocateMemory(unsigned int varid);
    void read(unsigned int varid);
    void readAliases(unsigned int varid);
    void readAll(unsigned int varid);
    void write(unsigned int varid);
    void writeAliases(unsigned int varid);
    void writeAll(unsigned int varid);
    void use(unsigned int varid);
    void modified(unsigned int varid);
    VariableUsage *find(unsigned int varid);
    void alias(unsigned int varid1, unsigned int varid2, bool replace);
    void erase(unsigned int varid) {
        _varUsage.erase(varid);
    }
    void eraseAliases(unsigned int varid);
    void eraseAll(unsigned int varid);
    void clearAliases(unsigned int varid);

private:
    VariableMap _varUsage;
};

/**
 * Alias the 2 given variables. Either replace the existing aliases if
 * they exist or merge them.  You would replace an existing alias when this
 * assignment is in the same scope as the previous assignment.  You might
 * merge the aliases when this assignment is in a different scope from the
 * previous assignment depending on the relationship of the 2 scopes.
 */
void Variables::alias(unsigned int varid1, unsigned int varid2, bool replace)
{
    VariableUsage *var1 = find(varid1);
    VariableUsage *var2 = find(varid2);

    // alias to self
    if (varid1 == varid2) {
        if (var1)
            var1->use();
        return;
    }

    std::set<unsigned int>::iterator i;

    if (replace) {
        // remove var1 from all aliases
        for (i = var1->_aliases.begin(); i != var1->_aliases.end(); ++i) {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(var1->_name->varId());
        }

        // remove all aliases from var1
        var1->_aliases.clear();
    }

    // var1 gets all var2s aliases
    for (i = var2->_aliases.begin(); i != var2->_aliases.end(); ++i) {
        if (*i != varid1)
            var1->_aliases.insert(*i);
    }

    // var2 is an alias of var1
    var2->_aliases.insert(varid1);
    var1->_aliases.insert(varid2);

    if (var2->_type == Variables::pointer)
        var2->_read = true;
}

void Variables::clearAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        // remove usage from all aliases
        std::set<unsigned int>::iterator i;

        for (i = usage->_aliases.begin(); i != usage->_aliases.end(); ++i) {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(usage->_name->varId());
        }

        // remove all aliases from usage
        usage->_aliases.clear();
    }
}

void Variables::eraseAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
            erase(*aliases);
    }
}

void Variables::eraseAll(unsigned int varid)
{
    eraseAliases(varid);
    erase(varid);
}

void Variables::addVar(const Token *name,
                       VariableType type,
                       ScopeInfo *scope,
                       bool write_)
{
    if (name->varId() > 0)
        _varUsage.insert(std::make_pair(name->varId(), VariableUsage(name, type, scope, false, write_, false)));
}

void Variables::allocateMemory(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
        usage->_allocateMemory = true;
}

void Variables::read(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
        usage->_read = true;
}

void Variables::readAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_read = true;
        }
    }
}

void Variables::readAll(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_read = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_read = true;
        }
    }
}

void Variables::write(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
        usage->_write = true;
}

void Variables::writeAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_write = true;
        }
    }
}

void Variables::writeAll(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_write = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_write = true;
        }
    }
}

void Variables::use(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->use();

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->use();
        }
    }
}

void Variables::modified(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_modified = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_modified = true;
        }
    }
}

Variables::VariableUsage *Variables::find(unsigned int varid)
{
    if (varid) {
        VariableMap::iterator i = _varUsage.find(varid);
        if (i != _varUsage.end())
            return &i->second;
    }
    return 0;
}

static int doAssignment(Variables &variables, const Token *tok, bool dereference, ScopeInfo *scope)
{
    int next = 0;

    // a = a + b;
    if (Token::Match(tok, "%var% = %var% !!;") && tok->str() == tok->strAt(2)) {
        return 2;
    }

    // check for aliased variable
    const unsigned int varid1 = tok->varId();
    Variables::VariableUsage *var1 = variables.find(varid1);

    if (var1) {
        int start = 1;

        // search for '='
        while (tok->tokAt(start)->str() != "=")
            start++;

        start++;

        if (Token::Match(tok->tokAt(start), "&| %var%") ||
            Token::Match(tok->tokAt(start), "( const| struct|union| %type% *| ) &| %var%") ||
            Token::Match(tok->tokAt(start), "( const| struct|union| %type% *| ) ( &| %var%") ||
            Token::Match(tok->tokAt(start), "%any% < const| struct|union| %type% *| > ( &| %var%")) {
            unsigned char offset = 0;
            unsigned int varid2;
            bool addressOf = false;

            if (Token::Match(tok->tokAt(start), "%var% ."))
                variables.use(tok->tokAt(start)->varId());   // use = read + write

            // check for C style cast
            if (tok->tokAt(start)->str() == "(") {
                if (tok->tokAt(start + 1)->str() == "const")
                    offset++;

                if (Token::Match(tok->tokAt(start + 1 + offset), "struct|union"))
                    offset++;

                if (tok->tokAt(start + 2 + offset)->str() == "*")
                    offset++;

                if (tok->tokAt(start + 3 + offset)->str() == "&") {
                    addressOf = true;
                    next = start + 4 + offset;
                } else if (tok->tokAt(start + 3 + offset)->str() == "(") {
                    if (tok->tokAt(start + 4 + offset)->str() == "&") {
                        addressOf = true;
                        next = start + 5 + offset;
                    } else
                        next = start + 4 + offset;
                } else
                    next = start + 3 + offset;
            }

            // check for C++ style cast
            else if (tok->tokAt(start)->str().find("cast") != std::string::npos &&
                     tok->tokAt(start + 1)->str() == "<") {
                if (tok->tokAt(start + 2)->str() == "const")
                    offset++;

                if (Token::Match(tok->tokAt(start + 2 + offset), "struct|union"))
                    offset++;

                if (tok->tokAt(start + 3 + offset)->str() == "*")
                    offset++;

                if (tok->tokAt(start + 5 + offset)->str() == "&") {
                    addressOf = true;
                    next = start + 6 + offset;
                } else
                    next = start + 5 + offset;
            }

            // check for var ? ...
            else if (Token::Match(tok->tokAt(start), "%var% ?")) {
                next = start;
            }

            // no cast
            else {
                if (tok->tokAt(start)->str() == "&") {
                    addressOf = true;
                    next = start + 1;
                } else if (tok->tokAt(start)->str() == "new")
                    return 0;
                else
                    next = start;
            }

            // check if variable is local
            varid2 = tok->tokAt(next)->varId();
            Variables::VariableUsage *var2 = variables.find(varid2);

            if (var2) { // local variable (alias or read it)
                if (var1->_type == Variables::pointer) {
                    if (dereference)
                        variables.read(varid2);
                    else {
                        if (addressOf ||
                            var2->_type == Variables::array ||
                            var2->_type == Variables::pointer) {
                            bool    replace = true;

                            // check if variable declared in same scope
                            if (scope == var1->_scope)
                                replace = true;

                            // not in same scope as declaration
                            else {
                                std::set<ScopeInfo *>::iterator assignment;

                                // check for an assignment in this scope
                                assignment = var1->_assignments.find(scope);

                                // no other assignment in this scope
                                if (assignment == var1->_assignments.end()) {
                                    // nothing to replace
                                    if (var1->_assignments.empty())
                                        replace = false;

                                    // this variable has previous assignments
                                    else {
                                        /**
                                         * @todo determine if existing aliases should be replaced or merged
                                         */

                                        replace = false;
                                    }
                                }

                                // assignment in this scope
                                else {
                                    // replace when only one other assignment
                                    if (var1->_assignments.size() == 1)
                                        replace = true;

                                    // otherwise, merge them
                                    else
                                        replace = false;
                                }
                            }

                            variables.alias(varid1, varid2, replace);
                        } else if (tok->tokAt(next + 1)->str() == "?") {
                            if (var2->_type == Variables::reference)
                                variables.readAliases(varid2);
                            else
                                variables.read(varid2);
                        }
                    }
                } else if (var1->_type == Variables::reference) {
                    variables.alias(varid1, varid2, true);
                } else {
                    if (var2->_type == Variables::pointer && tok->tokAt(next + 1)->str() == "[")
                        variables.readAliases(varid2);

                    variables.read(varid2);
                }
            } else { // not a local variable (or an unsupported local variable)
                if (var1->_type == Variables::pointer && !dereference) {
                    // check if variable declaration is in this scope
                    if (var1->_scope == scope)
                        variables.clearAliases(varid1);
                    else {
                        std::set<ScopeInfo *>::iterator assignment;

                        // check for an assignment in this scope
                        assignment = var1->_assignments.find(scope);

                        // no other assignment in this scope
                        if (assignment == var1->_assignments.end()) {
                            /**
                             * @todo determine if existing aliases should be discarded
                             */
                        }

                        // this assignment replaces the last assignment in this scope
                        else {
                            // aliased variables in a larger scope are not supported
                            // remove all aliases
                            variables.clearAliases(varid1);
                        }
                    }
                }
            }
        }

        var1->_assignments.insert(scope);
    }

    // check for alias to struct member
    // char c[10]; a.b = c;
    else if (Token::Match(tok->tokAt(-2), "%var% .")) {
        if (Token::Match(tok->tokAt(2), "%var%")) {
            unsigned int varid2 = tok->tokAt(2)->varId();
            Variables::VariableUsage *var2 = variables.find(varid2);

            // struct member aliased to local variable
            if (var2 && (var2->_type == Variables::array ||
                         var2->_type == Variables::pointer)) {
                // erase aliased variable and all variables that alias it
                // to prevent false positives
                variables.eraseAll(varid2);
            }
        }
    }

    return next;
}

static bool nextIsStandardType(const Token *tok)
{
    tok = tok->next();

    if (tok->str() == "static")
        tok = tok->next();

    return tok->isStandardType();
}

static bool nextIsStandardTypeOrVoid(const Token *tok)
{
    tok = tok->next();

    if (tok->str() == "static")
        tok = tok->next();

    if (tok->str() == "const")
        tok = tok->next();

    return tok->isStandardType() || tok->str() == "void";
}

bool CheckOther::isRecordTypeWithoutSideEffects(const Token *tok)
{
    const Variable * var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok->varId());

    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check base class for side effects */
    /** @todo false negative: check constructors for side effects */
    if (var && var->type() && var->type()->numConstructors == 0 &&
        (var->type()->varlist.empty() || var->type()->needInitialization == Scope::True) &&
        var->type()->derivedFrom.empty())
        return true;

    return false;
}

void CheckOther::functionVariableUsage()
{
    if (!_settings->isEnabled("style"))
        return;

    // Parse all executing scopes..
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // First token for the current scope..
        const Token *const tok1 = scope->classStart;

        // varId, usage {read, write, modified}
        Variables variables;

        // scopes
        ScopeInfo scopes;
        ScopeInfo *info = &scopes;

        unsigned int indentlevel = 0;
        for (const Token *tok = tok1; tok; tok = tok->next()) {
            if (tok->str() == "{") {
                // replace the head node when found
                if (indentlevel == 0)
                    scopes = ScopeInfo(tok, NULL);
                // add the new scope
                else
                    info = info->addChild(tok);
                ++indentlevel;
            } else if (tok->str() == "}") {
                --indentlevel;

                info = info->parent();

                if (indentlevel == 0)
                    break;
            } else if (Token::Match(tok, "struct|union|class {") ||
                       Token::Match(tok, "struct|union|class %type% {|:")) {
                while (tok->str() != "{")
                    tok = tok->next();
                tok = tok->link();
                if (! tok)
                    break;
            }

            if (Token::Match(tok, "[;{}] asm ( ) ;")) {
                variables.clear();
                break;
            }

            // standard type declaration with possible initialization
            // int i; int j = 0; static int k;
            if (Token::Match(tok, "[;{}] static| %type% %var% ;|=") &&
                !Token::Match(tok->next(), "return|throw")) {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->isStandardType() || isRecordTypeWithoutSideEffects(tok->next())) {
                    variables.addVar(tok->next(), Variables::standard, info,
                                     tok->tokAt(2)->str() == "=" || isStatic);
                }
                tok = tok->next();
            }

            // standard const type declaration
            // const int i = x;
            else if (Token::Match(tok, "[;{}] const %type% %var% =")) {
                tok = tok->next()->next();

                if (tok->isStandardType() || isRecordTypeWithoutSideEffects(tok->next()))
                    variables.addVar(tok->next(), Variables::standard, info, true);

                tok = tok->next();
            }

            // std::string declaration with possible initialization
            // std::string s; std::string s = "string";
            else if (Token::Match(tok, "[;{}] static| std :: string %var% ;|=")) {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                tok = tok->tokAt(3);
                variables.addVar(tok, Variables::standard, info,
                                 tok->next()->str() == "=" || isStatic);
            }

            // standard struct type declaration with possible initialization
            // struct S s; struct S s = { 0 }; static struct S s;
            else if (Token::Match(tok, "[;{}] static| struct %type% %var% ;|=") &&
                     (isRecordTypeWithoutSideEffects(tok->strAt(1) == "static" ? tok->tokAt(4) : tok->tokAt(3)))) {
                tok = tok->next();

                bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                tok = tok->next();

                variables.addVar(tok->next(), Variables::standard, info,
                                 tok->tokAt(2)->str() == "=" || isStatic);
                tok = tok->next();
            }

            // standard type declaration and initialization using constructor
            // int i(0); static int j(0);
            else if (Token::Match(tok, "[;{}] static| %type% %var% ( %any% ) ;") &&
                     nextIsStandardType(tok)) {
                tok = tok->next();

                if (tok->str() == "static")
                    tok = tok->next();

                variables.addVar(tok->next(), Variables::standard, info, true);

                // check if a local variable is used to initialize this variable
                if (tok->tokAt(3)->varId() > 0)
                    variables.readAll(tok->tokAt(3)->varId());
                tok = tok->tokAt(4);
            }

            // standard type declaration of array of with possible initialization
            // int i[10]; int j[2] = { 0, 1 }; static int k[2] = { 2, 3 };
            else if (Token::Match(tok, "[;{}] static| const| %type% *| %var% [ %any% ] ;|=") &&
                     nextIsStandardType(tok)) {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return" && tok->str() != "throw") {
                    bool isPointer = bool(tok->strAt(1) == "*");
                    const Token * const nametok = tok->tokAt(isPointer ? 2 : 1);

                    variables.addVar(nametok, isPointer ? Variables::pointerArray : Variables::array, info,
                                     nametok->tokAt(4)->str() == "=" || isStatic);

                    // check for reading array size from local variable
                    if (nametok->tokAt(2)->varId() != 0)
                        variables.read(nametok->tokAt(2)->varId());

                    // look at initializers
                    if (Token::simpleMatch(nametok->tokAt(4), "= {")) {
                        tok = nametok->tokAt(6);
                        while (tok->str() != "}") {
                            if (Token::Match(tok, "%var%"))
                                variables.read(tok->varId());
                            tok = tok->next();
                        }
                    } else
                        tok = nametok->tokAt(3);
                }
            }

            // pointer or reference declaration with possible initialization
            // int * i; int * j = 0; static int * k = 0;
            else if (Token::Match(tok, "[;{}] static| const| %type% *|& %var% ;|=")) {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->strAt(1) == "::")
                    tok = tok->tokAt(2);

                if (tok->str() != "return" && tok->str() != "throw") {
                    Variables::VariableType type;

                    if (tok->next()->str() == "*")
                        type = Variables::pointer;
                    else
                        type = Variables::reference;

                    bool written = tok->tokAt(3)->str() == "=";

                    variables.addVar(tok->tokAt(2), type, info, written || isStatic);

                    int offset = 0;

                    // check for assignment
                    if (written)
                        offset = doAssignment(variables, tok->tokAt(2), false, info);

                    tok = tok->tokAt(2 + offset);
                }
            }

            // pointer to pointer declaration with possible initialization
            // int ** i; int ** j = 0; static int ** k = 0;
            else if (Token::Match(tok, "[;{}] static| const| %type% * * %var% ;|=")) {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return") {
                    bool written = tok->tokAt(4)->str() == "=";

                    variables.addVar(tok->tokAt(3), Variables::pointerPointer, info, written || isStatic);

                    int offset = 0;

                    // check for assignment
                    if (written)
                        offset = doAssignment(variables, tok->tokAt(3), false, info);

                    tok = tok->tokAt(3 + offset);
                }
            }

            // pointer or reference of struct or union declaration with possible initialization
            // struct s * i; struct s * j = 0; static struct s * k = 0;
            else if (Token::Match(tok, "[;{}] static| const| struct|union %type% *|& %var% ;|=")) {
                Variables::VariableType type;

                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->strAt(2) == "*")
                    type = Variables::pointer;
                else
                    type = Variables::reference;

                const bool written = tok->strAt(4) == "=";

                variables.addVar(tok->tokAt(3), type, info, written || isStatic);

                int offset = 0;

                // check for assignment
                if (written)
                    offset = doAssignment(variables, tok->tokAt(3), false, info);

                tok = tok->tokAt(3 + offset);
            }

            // pointer or reference declaration with initialization using constructor
            // int * i(j); int * k(i); static int * l(i);
            else if (Token::Match(tok, "[;{}] static| const| %type% &|* %var% ( %any% ) ;") &&
                     nextIsStandardTypeOrVoid(tok)) {
                Variables::VariableType type;

                tok = tok->next();

                if (tok->str() == "static")
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->next()->str() == "*")
                    type = Variables::pointer;
                else
                    type = Variables::reference;

                unsigned int varid = 0;

                // check for aliased variable
                if (Token::Match(tok->tokAt(4), "%var%"))
                    varid = tok->tokAt(4)->varId();

                variables.addVar(tok->tokAt(2), type, info, true);

                // check if a local variable is used to initialize this variable
                if (varid > 0) {
                    Variables::VariableUsage *var = variables.find(varid);

                    if (type == Variables::pointer) {
                        variables.use(tok->tokAt(4)->varId());

                        if (var && (var->_type == Variables::array ||
                                    var->_type == Variables::pointer))
                            var->_aliases.insert(tok->varId());
                    } else {
                        variables.readAll(tok->tokAt(4)->varId());
                        if (var)
                            var->_aliases.insert(tok->varId());
                    }
                }
                tok = tok->tokAt(5);
            }

            // array of pointer or reference declaration with possible initialization
            // int * p[10]; int * q[10] = { 0 }; static int * * r[10] = { 0 };
            else if (Token::Match(tok, "[;{}] static| const| %type% *|& %var% [ %any% ] ;|=")) {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return") {
                    variables.addVar(tok->tokAt(2),
                                     tok->next()->str() == "*" ? Variables::pointerArray : Variables::referenceArray, info,
                                     tok->tokAt(6)->str() == "=" || isStatic);

                    // check for reading array size from local variable
                    if (tok->tokAt(4)->varId() != 0)
                        variables.read(tok->tokAt(4)->varId());

                    tok = tok->tokAt(5);
                }
            }

            // array of pointer or reference of struct or union declaration with possible initialization
            // struct S * p[10]; struct T * q[10] = { 0 }; static struct S * r[10] = { 0 };
            else if (Token::Match(tok, "[;{}] static| const| struct|union %type% *|& %var% [ %any% ] ;|=")) {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                variables.addVar(tok->tokAt(3),
                                 tok->tokAt(2)->str() == "*" ? Variables::pointerArray : Variables::referenceArray, info,
                                 tok->tokAt(7)->str() == "=" || isStatic);

                // check for reading array size from local variable
                if (tok->tokAt(5)->varId() != 0)
                    variables.read(tok->tokAt(5)->varId());

                tok = tok->tokAt(6);
            }

            // Freeing memory (not considered "using" the pointer if it was also allocated in this function)
            else if (Token::Match(tok, "free|g_free|kfree|vfree ( %var% )") ||
                     Token::Match(tok, "delete %var% ;") ||
                     Token::Match(tok, "delete [ ] %var% ;")) {
                unsigned int varid = 0;
                if (tok->str() != "delete") {
                    varid = tok->tokAt(2)->varId();
                    tok = tok->tokAt(3);
                } else if (tok->strAt(1) == "[") {
                    varid = tok->tokAt(3)->varId();
                    tok = tok->tokAt(4);
                } else {
                    varid = tok->next()->varId();
                    tok = tok->tokAt(2);
                }

                Variables::VariableUsage *var = variables.find(varid);
                if (var && !var->_allocateMemory) {
                    variables.readAll(varid);
                }
            }

            else if (Token::Match(tok, "return|throw %var%"))
                variables.readAll(tok->next()->varId());

            // assignment
            else if (Token::Match(tok, "*| (| ++|--| %var% ++|--| )| =") ||
                     Token::Match(tok, "*| ( const| %type% *| ) %var% =")) {
                bool dereference = false;
                bool pre = false;
                bool post = false;

                if (tok->str() == "*") {
                    dereference = true;
                    tok = tok->next();
                }

                if (Token::Match(tok, "( const| %type% *| ) %var% ="))
                    tok = tok->link()->next();

                else if (tok->str() == "(")
                    tok = tok->next();

                if (Token::Match(tok, "++|--")) {
                    pre = true;
                    tok = tok->next();
                }

                if (Token::Match(tok->next(), "++|--"))
                    post = true;

                const unsigned int varid1 = tok->varId();
                const Token *start = tok;

                tok = tok->tokAt(doAssignment(variables, tok, dereference, info));

                if (pre || post)
                    variables.use(varid1);

                if (dereference) {
                    Variables::VariableUsage *var = variables.find(varid1);
                    if (var && var->_type == Variables::array)
                        variables.write(varid1);
                    variables.writeAliases(varid1);
                    variables.read(varid1);
                } else {
                    Variables::VariableUsage *var = variables.find(varid1);
                    if (var && var->_type == Variables::reference) {
                        variables.writeAliases(varid1);
                        variables.read(varid1);
                    }
                    // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                    else if (var && var->_type == Variables::pointer &&
                             Token::Match(start, "%var% = new|malloc|calloc|g_malloc|kmalloc|vmalloc")) {
                        bool allocate = true;

                        if (start->strAt(2) == "new") {
                            // is it a user defined type?
                            if (!start->tokAt(3)->isStandardType()) {
                                if (!isRecordTypeWithoutSideEffects(start))
                                    allocate = false;
                            }
                        }

                        if (allocate)
                            variables.allocateMemory(varid1);
                        else
                            variables.write(varid1);
                    } else if (varid1 && Token::Match(tok, "%varid% .", varid1)) {
                        variables.use(varid1);
                    } else {
                        variables.write(varid1);
                    }

                    Variables::VariableUsage *var2 = variables.find(tok->varId());
                    if (var2) {
                        if (var2->_type == Variables::reference) {
                            variables.writeAliases(tok->varId());
                            variables.read(tok->varId());
                        } else if (tok->varId() != varid1 && Token::Match(tok, "%var% ."))
                            variables.read(tok->varId());
                        else if (tok->varId() != varid1 &&
                                 var2->_type == Variables::standard &&
                                 tok->strAt(-1) != "&")
                            variables.use(tok->varId());
                    }
                }

                const Token *equal = tok->next();

                if (Token::Match(tok->next(), "[ %any% ]"))
                    equal = tok->tokAt(4);

                // checked for chained assignments
                if (tok != start && equal->str() == "=") {
                    Variables::VariableUsage *var = variables.find(tok->varId());

                    if (var && var->_type != Variables::reference)
                        var->_read = true;

                    tok = tok->previous();
                }
            }

            // assignment
            else if (Token::Match(tok, "%var% [") && Token::simpleMatch(tok->next()->link(), "] =")) {
                unsigned int varid = tok->varId();
                const Variables::VariableUsage *var = variables.find(varid);

                if (var) {
                    // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                    if (var->_type == Variables::pointer &&
                        Token::Match(tok->next()->link(), "] = new|malloc|calloc|g_malloc|kmalloc|vmalloc")) {
                        variables.allocateMemory(varid);
                    } else if (var->_type == Variables::pointer || var->_type == Variables::reference) {
                        variables.read(varid);
                        variables.writeAliases(varid);
                    } else
                        variables.writeAll(varid);
                }
            }

            else if (Token::Match(tok, ">>|& %var%"))
                variables.use(tok->next()->varId());    // use = read + write
            else if (Token::Match(tok, "[;{}] %var% >>"))
                variables.use(tok->next()->varId());    // use = read + write

            // function parameter
            else if (Token::Match(tok, "[(,] %var% ["))
                variables.use(tok->next()->varId());   // use = read + write
            else if (Token::Match(tok, "[(,] %var% [,)]") && tok->previous()->str() != "*")
                variables.use(tok->next()->varId());   // use = read + write
            else if (Token::Match(tok, "[(,] (") &&
                     Token::Match(tok->next()->link(), ") %var% [,)]"))
                variables.use(tok->next()->link()->next()->varId());   // use = read + write

            // function
            else if (Token::Match(tok, "%var% (")) {
                variables.read(tok->varId());
                if (Token::Match(tok->tokAt(2), "%var% ="))
                    variables.read(tok->tokAt(2)->varId());
            }

            else if (Token::Match(tok, "[{,] %var% [,}]"))
                variables.read(tok->next()->varId());

            else if (Token::Match(tok, "%var% ."))
                variables.use(tok->varId());   // use = read + write

            else if ((Token::Match(tok, "[(=&!]") || tok->isExtendedOp()) &&
                     (Token::Match(tok->next(), "%var%") && !Token::Match(tok->next(), "true|false|new")))
                variables.readAll(tok->next()->varId());

            else if (Token::Match(tok, "%var%") && (tok->next()->str() == ")" || tok->next()->isExtendedOp()))
                variables.readAll(tok->varId());

            else if (Token::Match(tok, "; %var% ;"))
                variables.readAll(tok->next()->varId());

            if (Token::Match(tok, "++|-- %var%")) {
                if (tok->strAt(-1) != ";")
                    variables.use(tok->next()->varId());
                else
                    variables.modified(tok->next()->varId());
            }

            else if (Token::Match(tok, "%var% ++|--")) {
                if (tok->strAt(-1) != ";")
                    variables.use(tok->varId());
                else
                    variables.modified(tok->varId());
            }

            else if (tok->isAssignmentOp()) {
                for (const Token *tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                    if (tok2->varId()) {
                        variables.read(tok2->varId());
                        if (tok2->next()->isAssignmentOp())
                            variables.write(tok2->varId());
                    }
                }
            }
        }

        // Check usage of all variables in the current scope..
        Variables::VariableMap::const_iterator it;
        for (it = variables.varUsage().begin(); it != variables.varUsage().end(); ++it) {
            const Variables::VariableUsage &usage = it->second;
            const std::string &varname = usage._name->str();

            // variable has been marked as unused so ignore it
            if (usage._name->isUnused())
                continue;

            // skip things that are only partially implemented to prevent false positives
            if (usage._type == Variables::pointerPointer ||
                usage._type == Variables::pointerArray ||
                usage._type == Variables::referenceArray)
                continue;

            // variable has had memory allocated for it, but hasn't done
            // anything with that memory other than, perhaps, freeing it
            if (usage.unused() && !usage._modified && usage._allocateMemory)
                allocatedButUnusedVariableError(usage._name, varname);

            // variable has not been written, read, or modified
            else if (usage.unused() && !usage._modified)
                unusedVariableError(usage._name, varname);

            // variable has not been written but has been modified
            else if (usage._modified & !usage._write)
                unassignedVariableError(usage._name, varname);

            // variable has been written but not read
            else if (!usage._read && !usage._modified)
                unreadVariableError(usage._name, varname);

            // variable has been read but not written
            else if (!usage._write && !usage._allocateMemory)
                unassignedVariableError(usage._name, varname);
        }
    }
}

void CheckOther::unusedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedVariable", "Unused variable: " + varname);
}

void CheckOther::allocatedButUnusedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedAllocatedMemory", "Variable '" + varname + "' is allocated memory that is never used");
}

void CheckOther::unreadVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unreadVariable", "Variable '" + varname + "' is assigned a value that is never used");
}

void CheckOther::unassignedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unassignedVariable", "Variable '" + varname + "' is not assigned a value");
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Check scope of variables..
//---------------------------------------------------------------------------

void CheckOther::checkVariableScope()
{
    if (!_settings->isEnabled("information"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // Walk through all tokens..
        int indentlevel = 0;
        for (const Token *tok = scope->classStart; tok; tok = tok->next()) {
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

            else if (tok->str() == "{") {
                ++indentlevel;
            } else if (tok->str() == "}") {
                --indentlevel;
                if (indentlevel == 0)
                    break;
            }

            if (indentlevel > 0 && Token::Match(tok, "[{};]")) {
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
//---------------------------------------------------------------------------

void CheckOther::lookupVar(const Token *tok1, const std::string &varname)
{
    const Token *tok = tok1;

    // Skip the variable declaration..
    while (tok && tok->str() != ";")
        tok = tok->next();

    // Check if the variable is used in this indentlevel..
    bool used1 = false;   // used in one sub-scope -> reducable
    bool used2 = false;   // used in more sub-scopes -> not reducable
    int indentlevel = 0;
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
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for constant function parameters
//---------------------------------------------------------------------------

void CheckOther::checkConstantFunctionParameter()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // TODO: False negatives. This pattern only checks for string.
        //       Investigate if there are other classes in the std
        //       namespace and add them to the pattern. There are
        //       streams for example (however it seems strange with
        //       const stream parameter).
        if (Token::Match(tok, "[,(] const std :: string %var% [,)]")) {
            passedByValueError(tok, tok->strAt(5));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < %type% > %var% [,)]")) {
            passedByValueError(tok, tok->strAt(8));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < std :: %type% > %var% [,)]")) {
            passedByValueError(tok, tok->strAt(10));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < std :: %type% , std :: %type% > %var% [,)]")) {
            passedByValueError(tok, tok->strAt(14));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < %type% , std :: %type% > %var% [,)]")) {
            passedByValueError(tok, tok->strAt(12));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < std :: %type% , %type% > %var% [,)]")) {
            passedByValueError(tok, tok->strAt(12));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < %type% , %type% > %var% [,)]")) {
            passedByValueError(tok, tok->strAt(10));
        }

        else if (Token::Match(tok, "[,(] const %type% %var% [,)]")) {
            // Check if type is a struct or class.
            if (symbolDatabase->isClassOrStruct(tok->strAt(2))) {
                passedByValueError(tok, tok->strAt(3));
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check that all struct members are used
//---------------------------------------------------------------------------

void CheckOther::checkStructMemberUsage()
{
    if (!_settings->isEnabled("style"))
        return;

    std::string structname;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->fileIndex() != 0)
            continue;

        if (Token::Match(tok, "struct|union %type% {")) {
            structname.clear();
            if (Token::simpleMatch(tok->previous(), "extern"))
                continue;
            if ((!tok->previous() || Token::simpleMatch(tok->previous(), ";")) && Token::Match(tok->tokAt(2)->link(), ("} ; " + tok->strAt(1) + " %var% ;").c_str()))
                continue;

            structname = tok->strAt(1);

            // Bail out if struct/union contain any functions
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(") {
                    structname.clear();
                    break;
                }

                if (tok2->str() == "}")
                    break;
            }

            // bail out if struct is inherited
            if (!structname.empty() && Token::findmatch(tok, (",|private|protected|public " + structname).c_str()))
                structname.clear();

            // Bail out if some data is casted to struct..
            const std::string s("( struct| " + tok->next()->str() + " * ) & %var% [");
            if (Token::findmatch(tok, s.c_str()))
                structname.clear();

            // Try to prevent false positives when struct members are not used directly.
            if (Token::findmatch(tok, (structname + " *").c_str()))
                structname.clear();
            else if (Token::findmatch(tok, (structname + " %type% *").c_str()))
                structname = "";
        }

        if (tok->str() == "}")
            structname.clear();

        if (!structname.empty() && Token::Match(tok, "[{;]")) {
            // Declaring struct variable..
            std::string varname;

            // declaring a POD variable?
            if (!tok->next()->isStandardType())
                continue;

            if (Token::Match(tok->next(), "%type% %var% [;[]"))
                varname = tok->strAt(2);
            else if (Token::Match(tok->next(), "%type% %type% %var% [;[]"))
                varname = tok->strAt(3);
            else if (Token::Match(tok->next(), "%type% * %var% [;[]"))
                varname = tok->strAt(3);
            else if (Token::Match(tok->next(), "%type% %type% * %var% [;[]"))
                varname = tok->strAt(4);
            else
                continue;

            // Check if the struct variable is used anywhere in the file
            const std::string usagePattern(". " + varname);
            bool used = false;
            for (const Token *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next()) {
                if (Token::simpleMatch(tok2, usagePattern.c_str())) {
                    used = true;
                    break;
                }
            }

            if (! used) {
                unusedStructMemberError(tok->next(), structname, varname);
            }
        }
    }
}





//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------

void CheckOther::checkCharVariable()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Declaring the variable..
        if (Token::Match(tok, "[{};(,] const| char *| %var% [;=,)]") ||
            Token::Match(tok, "[{};(,] const| char %var% [")) {
            // goto 'char' token
            tok = tok->next();
            if (tok->str() == "const")
                tok = tok->next();

            // Check for unsigned char
            if (tok->isUnsigned())
                continue;

            // Set tok to point to the variable name
            tok = tok->next();
            const bool isPointer(tok->str() == "*" || tok->strAt(1) == "[");
            if (tok->str() == "*")
                tok = tok->next();

            // Check usage of char variable..
            int indentlevel = 0;
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "{")
                    ++indentlevel;

                else if (tok2->str() == "}") {
                    --indentlevel;
                    if (indentlevel <= 0)
                        break;
                }

                if (!isPointer) {
                    std::string temp = "%var% [ " + tok->str() + " ]";
                    if ((tok2->str() != ".") && Token::Match(tok2->next(), temp.c_str())) {
                        charArrayIndexError(tok2->next());
                        break;
                    }
                }

                if (Token::Match(tok2, "[;{}] %var% = %any% [&|] %any% ;")) {
                    // is the char variable used in the calculation?
                    if (tok2->tokAt(3)->varId() != tok->varId() && tok2->tokAt(5)->varId() != tok->varId())
                        continue;

                    // it's ok with a bitwise and where the other operand is 0xff or less..
                    if (tok2->strAt(4) == "&") {
                        if (tok2->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok2->strAt(3)))
                            continue;
                        if (tok2->tokAt(5)->isNumber() && MathLib::isGreater("0x100", tok2->strAt(5)))
                            continue;
                    }

                    // is the result stored in a short|int|long?
                    if (!Token::findmatch(_tokenizer->tokens(), "short|int|long %varid%", tok2->next()->varId()))
                        continue;

                    // This is an error..
                    charBitOpError(tok2);
                    break;
                }

                if (isPointer && Token::Match(tok2, "[;{}] %var% = %any% [&|] ( * %varid% ) ;", tok->varId())) {
                    // it's ok with a bitwise and where the other operand is 0xff or less..
                    if (tok2->strAt(4) == "&" && tok2->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok2->strAt(3)))
                        continue;

                    // is the result stored in a short|int|long?
                    if (!Token::findmatch(_tokenizer->tokens(), "short|int|long %varid%", tok2->next()->varId()))
                        continue;

                    // This is an error..
                    charBitOpError(tok2);
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------






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
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// str plus char
//---------------------------------------------------------------------------

void CheckOther::strPlusChar()
{
    // Don't use this check for Java and C# programs..
    if (_tokenizer->isJavaOrCSharp()) {
        return;
    }

    bool charVars[10000] = {0};

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Declaring char variable..
        if (Token::Match(tok, "char|int|short %var% [;=]")) {
            unsigned int varid = tok->next()->varId();
            if (varid > 0 && varid < 10000)
                charVars[varid] = true;
        }

        //
        else if (Token::Match(tok, "[=(] %str% + %any%")) {
            // char constant..
            const std::string s = tok->strAt(3);
            if (s[0] == '\'')
                strPlusChar(tok->next());

            // char variable..
            unsigned int varid = tok->tokAt(3)->varId();
            if (varid > 0 && varid < 10000 && charVars[varid])
                strPlusChar(tok->next());
        }
    }
}

void CheckOther::checkZeroDivision()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {

        if (Token::Match(tok, "/ %num%") &&
            MathLib::isInt(tok->next()->str()) &&
            MathLib::toLongNumber(tok->next()->str()) == 0L) {
            zerodivError(tok);
        } else if (Token::Match(tok, "div|ldiv|lldiv|imaxdiv ( %num% , %num% )") &&
                   MathLib::isInt(tok->tokAt(4)->str()) &&
                   MathLib::toLongNumber(tok->tokAt(4)->str()) == 0L) {
            zerodivError(tok);
        }
    }
}


void CheckOther::checkMathFunctions()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // case log(-2)
        if (tok->varId() == 0 &&
            Token::Match(tok, "log|log10 ( %num% )") &&
            MathLib::isNegative(tok->tokAt(2)->str()) &&
            MathLib::isInt(tok->tokAt(2)->str()) &&
            MathLib::toLongNumber(tok->tokAt(2)->str()) <= 0) {
            mathfunctionCallError(tok);
        }
        // case log(-2.0)
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "log|log10 ( %num% )") &&
                 MathLib::isNegative(tok->tokAt(2)->str()) &&
                 MathLib::isFloat(tok->tokAt(2)->str()) &&
                 MathLib::toDoubleNumber(tok->tokAt(2)->str()) <= 0.) {
            mathfunctionCallError(tok);
        }

        // case log(0.0)
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "log|log10 ( %num% )") &&
                 !MathLib::isNegative(tok->tokAt(2)->str()) &&
                 MathLib::isFloat(tok->tokAt(2)->str()) &&
                 MathLib::toDoubleNumber(tok->tokAt(2)->str()) <= 0.) {
            mathfunctionCallError(tok);
        }

        // case log(0)
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "log|log10 ( %num% )") &&
                 !MathLib::isNegative(tok->tokAt(2)->str()) &&
                 MathLib::isInt(tok->tokAt(2)->str()) &&
                 MathLib::toLongNumber(tok->tokAt(2)->str()) <= 0) {
            mathfunctionCallError(tok);
        }
        // acos( x ), asin( x )  where x is defined for interval [-1,+1], but not beyond
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "acos|asin ( %num% )") &&
                 std::fabs(MathLib::toDoubleNumber(tok->tokAt(2)->str())) > 1.0) {
            mathfunctionCallError(tok);
        }
        // sqrt( x ): if x is negative the result is undefined
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "sqrt|sqrtf|sqrtl ( %num% )") &&
                 MathLib::isNegative(tok->tokAt(2)->str())) {
            mathfunctionCallError(tok);
        }
        // atan2 ( x , y): x and y can not be zero, because this is mathematically not defined
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "atan2 ( %num% , %num% )") &&
                 MathLib::isNullValue(tok->tokAt(2)->str()) &&
                 MathLib::isNullValue(tok->tokAt(4)->str())) {
            mathfunctionCallError(tok, 2);
        }
        // fmod ( x , y) If y is zero, then either a range error will occur or the function will return zero (implementation-defined).
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "fmod ( %num% , %num% )") &&
                 MathLib::isNullValue(tok->tokAt(4)->str())) {
            mathfunctionCallError(tok, 2);
        }
        // pow ( x , y) If x is zero, and y is negative --> division by zero
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "pow ( %num% , %num% )") &&
                 MathLib::isNullValue(tok->tokAt(2)->str())  &&
                 MathLib::isNegative(tok->tokAt(4)->str())) {
            mathfunctionCallError(tok, 2);
        }

    }
}

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
                tok = tok->tokAt(2)->link();
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
    {
        const std::string fname = _tokenizer->getFiles()->at(0);
        size_t position         = fname.rfind(".");

        if (position != std::string::npos) {
            const std::string ext = fname.substr(position);
            if (ext == ".c" || ext == ".C")
                return;
        }
    }

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        unsigned int depth = 0;

        for (const Token *tok = scope->classStart; tok; tok = tok->next()) {
            if (tok->str() == "{") {
                ++depth;
            } else if (tok->str() == "}") {
                --depth;
                if (depth == 0)
                    break;
            }

            if (Token::Match(tok, "[;{}] %var% (")
                && Token::simpleMatch(tok->tokAt(2)->link(), ") ;")
                && symbolDatabase->isClassOrStruct(tok->next()->str())
                && !isFunction(tok->next()->str(), _tokenizer->tokens())) {
                tok = tok->next();
                misusedScopeObjectError(tok, tok->str());
                tok = tok->next();
            }
        }
    }
}

void CheckOther::checkIncorrectStringCompare()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, ". substr ( %any% , %num% ) ==|!= %str%")) {
            size_t clen = MathLib::toLongNumber(tok->tokAt(5)->str());
            size_t slen = Token::getStrLength(tok->tokAt(8));
            if (clen != slen) {
                incorrectStringCompareError(tok->next(), "substr", tok->tokAt(8)->str(), tok->tokAt(5)->str());
            }
        }
        if (Token::Match(tok, "%str% ==|!= %var% . substr ( %any% , %num% )")) {
            size_t clen = MathLib::toLongNumber(tok->tokAt(8)->str());
            size_t slen = Token::getStrLength(tok);
            if (clen != slen) {
                incorrectStringCompareError(tok->next(), "substr", tok->str(), tok->tokAt(8)->str());
            }
        }
    }
}

//-----------------------------------------------------------------------------
// check for duplicate expressions in if statements
// if (a) { } else if (a) { }
//-----------------------------------------------------------------------------

static const std::string stringifyTokens(const Token *start, const Token *end)
{
    const Token *tok = start;
    std::string stringified;

    if (tok->isUnsigned())
        stringified.append("unsigned ");
    else if (tok->isSigned())
        stringified.append("signed ");

    if (tok->isLong())
        stringified.append("long ");

    stringified.append(tok->str());

    while (tok && tok->next() && tok != end) {
        if (tok->isUnsigned())
            stringified.append("unsigned ");
        else if (tok->isSigned())
            stringified.append("signed ");

        if (tok->isLong())
            stringified.append("long ");

        tok = tok->next();
        stringified.append(" ");
        stringified.append(tok->str());
    }

    return stringified;
}

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

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // check all the code in the function for if (...) and else if (...) statements
        for (const Token *tok = scope->classStart; tok && tok != scope->classStart->link(); tok = tok->next()) {
            if (Token::simpleMatch(tok, "if (") && tok->strAt(-1) != "else" &&
                Token::simpleMatch(tok->next()->link(), ") {")) {
                std::map<std::string, const Token*> expressionMap;

                // get the expression from the token stream
                std::string expression = stringifyTokens(tok->tokAt(2), tok->next()->link()->previous());

                // save the expression and its location
                expressionMap.insert(std::make_pair(expression, tok));

                // find the next else if (...) statement
                const Token *tok1 = tok->next()->link()->next()->link();

                // check all the else if (...) statements
                while (Token::simpleMatch(tok1, "} else if (") &&
                       Token::simpleMatch(tok1->tokAt(3)->link(), ") {")) {
                    // get the expression from the token stream
                    expression = stringifyTokens(tok1->tokAt(4), tok1->tokAt(3)->link()->previous());

                    // try to look up the expression to check for duplicates
                    std::map<std::string, const Token *>::iterator it = expressionMap.find(expression);

                    // found a duplicate
                    if (it != expressionMap.end()) {
                        // check for expressions that have side effects and ignore them
                        if (!expressionHasSideEffects(tok1->tokAt(4), tok1->tokAt(3)->link()->previous()))
                            duplicateIfError(it->second, tok1->next());
                    }

                    // not a duplicate expression so save it and its location
                    else
                        expressionMap.insert(std::make_pair(expression, tok1->next()));

                    // find the next else if (...) statement
                    tok1 = tok1->tokAt(3)->link()->next()->link();
                }

                tok = tok->next()->link()->next();
            }
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

    if (!_settings->inconclusive)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // check all the code in the function for if (..) else
        for (const Token *tok = scope->classStart; tok && tok != scope->classStart->link(); tok = tok->next()) {
            if (Token::simpleMatch(tok, "if (") && tok->strAt(-1) != "else" &&
                Token::simpleMatch(tok->next()->link(), ") {") &&
                Token::simpleMatch(tok->next()->link()->next()->link(), "} else {")) {
                // save if branch code
                std::string branch1 = stringifyTokens(tok->next()->link()->tokAt(2), tok->next()->link()->next()->link()->previous());

                // find else branch
                const Token *tok1 = tok->next()->link()->next()->link();

                // save else branch code
                std::string branch2 = stringifyTokens(tok1->tokAt(3), tok1->tokAt(2)->link()->previous());

                // check for duplicates
                if (branch1 == branch2)
                    duplicateBranchError(tok, tok1->tokAt(2));

                tok = tok->next()->link()->next();
            }
        }
    }
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

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        for (const Token *tok = scope->classStart; tok && tok != scope->classStart->link(); tok = tok->next()) {
            if (Token::Match(tok, "(|&&|%oror% %var% &&|%oror%|==|!=|<=|>=|<|>|-|%or% %var% )|&&|%oror%") &&
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
            } else if (Token::Match(tok, "(|&&|%oror% %var% . %var% &&|%oror%|==|!=|<=|>=|<|>|-|%or% %var% . %var% )|&&|%oror%") &&
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
    if (!_settings->isEnabled("style"))
        return;

    const char pattern1[] = "strcmp|stricmp|strcmpi|strcasecmp|wcscmp ( %str% , %str% )";
    const char pattern2[] = "QString :: compare ( %str% , %str% )";

    const Token *tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern1)) != NULL) {
        alwaysTrueFalseStringCompare(tok, tok->strAt(2), tok->strAt(4));
        tok = tok->tokAt(5);
    }

    tok = _tokenizer->tokens();
    while (tok && (tok = Token::findmatch(tok, pattern2)) != NULL) {
        alwaysTrueFalseStringCompare(tok, tok->strAt(4), tok->strAt(6));
        tok = tok->tokAt(7);
    }
}

void CheckOther::alwaysTrueFalseStringCompare(const Token *tok, const std::string& str1, const std::string& str2)
{
    const size_t stringLen = 10;
    const std::string string1 = (str1.size() < stringLen) ? str1 : (str1.substr(0, stringLen-2) + "..");
    const std::string string2 = (str2.size() < stringLen) ? str2 : (str2.substr(0, stringLen-2) + "..");

    if (str1 == str2) {
        reportError(tok, Severity::warning, "staticStringCompare",
                    "Comparison of always identical static strings.\n"
                    "The compared strings, '" + string1 + "' and '" + string2 + "', are always identical. "
                    "If the purpose is to compare these two strings, the comparison is unnecessary. "
                    "If the strings are supposed to be different, then there is a bug somewhere.");
    } else {
        reportError(tok, Severity::performance, "staticStringCompare",
                    "Unnecessary comparison of static strings.\n"
                    "The compared strings, '" + string1 + "' and '" + string2 + "', are static and always different. "
                    "If the purpose is to compare these two strings, the comparison is unnecessary.");
    }
}

//-----------------------------------------------------------------------------

void CheckOther::cstyleCastError(const Token *tok)
{
    reportError(tok, Severity::style, "cstyleCast", "C-style pointer casting");
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

void CheckOther::udivError(const Token *tok)
{
    reportError(tok, Severity::error, "udivError", "Unsigned division. The result will be wrong.");
}

void CheckOther::unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedStructMember", "struct or union member '" + structname + "::" + varname + "' is never used");
}

void CheckOther::passedByValueError(const Token *tok, const std::string &parname)
{
    reportError(tok, Severity::performance, "passedByValue",
                "Function parameter '" + parname + "' should be passed by reference.\n"
                "Parameter '" +  parname + "' is passed as a value. It could be passed "
                "as a (const) reference which is usually faster and recommended in C++.");
}

void CheckOther::constStatementError(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::warning, "constStatement", "Redundant code: Found a statement that begins with " + type + " constant");
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

void CheckOther::variableScopeError(const Token *tok, const std::string &varname)
{
    reportError(tok,
                Severity::information,
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

void CheckOther::conditionAlwaysTrueFalse(const Token *tok, const std::string &truefalse)
{
    reportError(tok, Severity::style, "conditionAlwaysTrueFalse", "Condition is always " + truefalse);
}

void CheckOther::strPlusChar(const Token *tok)
{
    reportError(tok, Severity::error, "strPlusChar", "Unusual pointer arithmetic");
}

void CheckOther::zerodivError(const Token *tok)
{
    reportError(tok, Severity::error, "zerodiv", "Division by zero");
}

void CheckOther::mathfunctionCallError(const Token *tok, const unsigned int numParam)
{
    if (tok) {
        if (numParam == 1)
            reportError(tok, Severity::error, "wrongmathcall", "Passing value " + tok->tokAt(2)->str() + " to " + tok->str() + "() leads to undefined result");
        else if (numParam == 2)
            reportError(tok, Severity::error, "wrongmathcall", "Passing value " + tok->tokAt(2)->str() + " and " + tok->tokAt(4)->str() + " to " + tok->str() + "() leads to undefined result");
    } else
        reportError(tok, Severity::error, "wrongmathcall", "Passing value " " to " "() leads to undefined result");
}

void CheckOther::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error,
                "fflushOnInputStream", "fflush() called on input stream \"" + varname + "\" may result in undefined behaviour");
}

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

void CheckOther::redundantAssignmentInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "redundantAssignInSwitch", "Redundant assignment of \"" + varname + "\" in switch");
}

void CheckOther::switchCaseFallThrough(const Token *tok)
{
    reportError(tok, Severity::style,
                "switchCaseFallThrough", "Switch falls through case without comment");
}

void CheckOther::selfAssignmentError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "selfAssignment", "Redundant assignment of \"" + varname + "\" to itself");
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

void CheckOther::incorrectLogicOperatorError(const Token *tok, bool always)
{
    if (always)
        reportError(tok, Severity::warning,
                    "incorrectLogicOperator", "Mutual exclusion over || always evaluates to true. Did you intend to use && instead?");
    else
        reportError(tok, Severity::warning,
                    "incorrectLogicOperator", "Expression always evaluates to false. Did you intend to use || instead?");
}

void CheckOther::misusedScopeObjectError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "unusedScopedObject", "instance of \"" + varname + "\" object destroyed immediately");
}

void CheckOther::catchExceptionByValueError(const Token *tok)
{
    reportError(tok, Severity::style,
                "catchExceptionByValue", "Exception should be caught by reference.\n"
                "The exception is caught as a value. It could be caught "
                "as a (const) reference which is usually recommended in C++.");
}

void CheckOther::memsetZeroBytesError(const Token *tok, const std::string &varname)
{
    const std::string summary("memset() called to fill 0 bytes of \'" + varname + "\'");
    const std::string verbose(summary + ". Second and third arguments might be inverted.");
    reportError(tok, Severity::warning, "memsetZeroBytes", summary + "\n" + verbose);
}

void CheckOther::incorrectStringCompareError(const Token *tok, const std::string& func, const std::string &string, const std::string &len)
{
    reportError(tok, Severity::warning, "incorrectStringCompare", "String literal " + string + " doesn't match length argument for " + func + "(" + len + ").");
}

void CheckOther::comparisonOfBoolWithIntError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "comparisonOfBoolWithInt",
                "Comparison of a boolean with a non-zero integer\n"
                "The expression \"!" + varname + "\" is of type 'bool' but is compared against a non-zero 'int'.");
}

void CheckOther::duplicateBreakError(const Token *tok)
{
    reportError(tok, Severity::style, "duplicateBreak",
                "Consecutive break or continue statements are unnecessary\n"
                "The second of the two statements can never be executed, and so should be removed\n");
}


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

//---------------------------------------------------------------------------
// Check testing sign of unsigned variables.
//---------------------------------------------------------------------------

void CheckOther::checkSignOfUnsignedVariable()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // check all the code in the function
        for (const Token *tok = scope->classStart; tok && tok != scope->classStart->link(); tok = tok->next()) {
            if (Token::Match(tok, "( %var% <|<= 0 )") && tok->next()->varId()) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->next()->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZero(tok->next(), tok->next()->str());
            } else if (Token::Match(tok, "( 0 > %var% )") && tok->tokAt(3)->varId()) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedLessThanZero(tok->tokAt(3), tok->strAt(3));
            } else if (Token::Match(tok, "( 0 <= %var% )") && tok->tokAt(3)->varId()) {
                const Variable * var = symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId());
                if (var && var->typeEndToken()->isUnsigned())
                    unsignedPositive(tok->tokAt(3), tok->strAt(3));
            }
        }
    }
}

void CheckOther::unsignedLessThanZero(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unsignedLessThanZero",
                "Checking if unsigned variable '" + varname + "' is less than zero.\n"
                "An unsigned variable will never be negative so it is either pointless or "
                "an error to check if it is.");
}

void CheckOther::unsignedPositive(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unsignedPositive",
                "Checking if unsigned variable '" + varname + "' is positive is always true.\n"
                "An unsigned variable can't be negative so it is either pointless or "
                "an error to check if it is.");
}
