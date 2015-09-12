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


//---------------------------------------------------------------------------
#include "astutils.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include <set>

static bool isChar(const Variable* var)
{
    return (var && !var->isPointer() && !var->isArray() && var->typeStartToken()->str() == "char");
}

static bool isSignedChar(const Variable* var)
{
    return (isChar(var) && !var->typeStartToken()->isUnsigned());
}

bool astIsSignedChar(const Token *tok)
{
    if (!tok)
        return false;
    if (tok->str() == "*" && tok->astOperand1() && !tok->astOperand2()) {
        const Variable *var = tok->astOperand1()->variable();
        if (!var || !var->isPointer())
            return false;
        const Token *type = var->typeStartToken();
        while (type && type->str() == "const")
            type = type->next();
        return (type && type->str() == "char" && !type->isUnsigned());
    }
    return isSignedChar(tok->variable());
}

bool astIsIntegral(const Token *tok, bool unknown)
{
    // TODO: handle arrays
    if (tok->isNumber())
        return MathLib::isInt(tok->str());

    if (tok->isName()) {
        if (tok->variable())
            return tok->variable()->isIntegralType();

        return unknown;
    }
    if (tok->str() == "(") {
        // cast
        if (Token::Match(tok, "( const| float|double )"))
            return false;

        // Function call
        if (tok->previous()->function()) {
            if (Token::Match(tok->previous()->function()->retDef, "float|double"))
                return false;
            else if (Token::Match(tok->previous()->function()->retDef, "bool|char|short|int|long"))
                return true;
        }

        if (tok->strAt(-1) == "sizeof")
            return true;

        return unknown;
    }

    if (tok->astOperand2() && (tok->str() == "." || tok->str() == "::"))
        return astIsIntegral(tok->astOperand2(), unknown);

    if (tok->astOperand1() && tok->str() != "?")
        return astIsIntegral(tok->astOperand1(), unknown);

    return unknown;
}

bool astIsFloat(const Token *tok, bool unknown)
{
    // TODO: handle arrays
    if (tok->isNumber())
        return MathLib::isFloat(tok->str());

    if (tok->isName()) {
        if (tok->variable())
            return tok->variable()->isFloatingType();

        return unknown;
    }
    if (tok->str() == "(") {
        // cast
        if (Token::Match(tok, "( const| float|double )"))
            return true;

        // Function call
        if (tok->previous()->function())
            return Token::Match(tok->previous()->function()->retDef, "float|double");

        if (tok->strAt(-1) == "sizeof")
            return false;

        return unknown;
    }

    if (tok->astOperand2() && (tok->str() == "." || tok->str() == "::"))
        return astIsFloat(tok->astOperand2(), unknown);

    if (tok->astOperand1() && tok->str() != "?" && astIsFloat(tok->astOperand1(), unknown))
        return true;
    if (tok->astOperand2() && astIsFloat(tok->astOperand2(), unknown))
        return true;

    if (tok->isOp())
        return false;

    return unknown;
}

std::string astCanonicalType(const Token *expr)
{
    if (!expr)
        return "";
    if (expr->variable()) {
        const Variable *var = expr->variable();
        std::string ret;
        for (const Token *type = var->typeStartToken(); Token::Match(type,"%name%|::") && type != var->nameToken(); type = type->next()) {
            if (!Token::Match(type, "const|static"))
                ret += type->str();
        }
        return ret;

    }
    // TODO: handle expressions
    return "";
}

const Token * astIsVariableComparison(const Token *tok, const std::string &comp, const std::string &rhs, const Token **vartok)
{
    if (!tok)
        return nullptr;

    const Token *ret = nullptr;
    if (tok->isComparisonOp()) {
        if (tok->astOperand1() && tok->astOperand1()->str() == rhs) {
            // Invert comparator
            std::string s = tok->str();
            if (s[0] == '>')
                s[0] = '<';
            else if (s[0] == '<')
                s[0] = '>';
            if (s == comp) {
                ret = tok->astOperand2();
            }
        } else if (tok->str() == comp && tok->astOperand2() && tok->astOperand2()->str() == rhs) {
            ret = tok->astOperand1();
        }
    } else if (comp == "!=" && rhs == std::string("0")) {
        ret = tok;
    } else if (comp == "==" && rhs == std::string("0")) {
        if (tok->str() == "!")
            ret = tok->astOperand1();
    }
    while (ret && ret->str() == ".")
        ret = ret->astOperand2();
    if (ret && ret->varId() == 0U)
        ret = nullptr;
    if (vartok)
        *vartok = ret;
    return ret;
}

bool isSameExpression(bool cpp, const Token *tok1, const Token *tok2, const std::set<std::string> &constFunctions)
{
    if (tok1 == nullptr && tok2 == nullptr)
        return true;
    if (tok1 == nullptr || tok2 == nullptr)
        return false;
    if (cpp) {
        if (tok1->str() == "." && tok1->astOperand1() && tok1->astOperand1()->str() == "this")
            tok1 = tok1->astOperand2();
        if (tok2->str() == "." && tok2->astOperand1() && tok2->astOperand1()->str() == "this")
            tok2 = tok2->astOperand2();
    }
    if (tok1->varId() != tok2->varId() || tok1->str() != tok2->str()) {
        if ((Token::Match(tok1,"<|>")   && Token::Match(tok2,"<|>")) ||
            (Token::Match(tok1,"<=|>=") && Token::Match(tok2,"<=|>="))) {
            return isSameExpression(cpp, tok1->astOperand1(), tok2->astOperand2(), constFunctions) &&
                   isSameExpression(cpp, tok1->astOperand2(), tok2->astOperand1(), constFunctions);
        }
        return false;
    }
    if (tok1->str() == "." && tok1->originalName() != tok2->originalName())
        return false;
    if (tok1->isExpandedMacro() || tok2->isExpandedMacro())
        return false;
    if (tok1->isName() && tok1->next()->str() == "(" && tok1->str() != "sizeof") {
        if (!tok1->function() && !Token::Match(tok1->previous(), ".|::") && constFunctions.find(tok1->str()) == constFunctions.end() && !tok1->isAttributeConst() && !tok1->isAttributePure())
            return false;
        else if (tok1->function() && !tok1->function()->isConst() && !tok1->function()->isAttributeConst() && !tok1->function()->isAttributePure())
            return false;
    }
    // templates/casts
    if ((Token::Match(tok1, "%name% <") && tok1->next()->link()) ||
        (Token::Match(tok2, "%name% <") && tok2->next()->link())) {

        // non-const template function that is not a dynamic_cast => return false
        if (Token::simpleMatch(tok1->next()->link(), "> (") &&
            !(tok1->function() && tok1->function()->isConst()) &&
            tok1->str() != "dynamic_cast")
            return false;

        // some template/cast stuff.. check that the template arguments are same
        const Token *t1 = tok1->next();
        const Token *t2 = tok2->next();
        const Token *end1 = t1->link();
        const Token *end2 = t2->link();
        while (t1 && t2 && t1 != end1 && t2 != end2) {
            if (t1->str() != t2->str())
                return false;
            t1 = t1->next();
            t2 = t2->next();
        }
        if (t1 != end1 || t2 != end2)
            return false;
    }
    if (tok1->tokType() == Token::eIncDecOp || tok1->isAssignmentOp())
        return false;
    // bailout when we see ({..})
    if (tok1->str() == "{")
        return false;
    if (tok1->str() == "(" && tok1->previous() && !tok1->previous()->isName()) { // cast => assert that the casts are equal
        const Token *t1 = tok1->next();
        const Token *t2 = tok2->next();
        while (t1 && t2 && t1->str() == t2->str() && (t1->isName() || t1->str() == "*")) {
            t1 = t1->next();
            t2 = t2->next();
        }
        if (!t1 || !t2 || t1->str() != ")" || t2->str() != ")")
            return false;
    }
    bool noncommuative_equals =
        isSameExpression(cpp, tok1->astOperand1(), tok2->astOperand1(), constFunctions);
    noncommuative_equals = noncommuative_equals &&
                           isSameExpression(cpp, tok1->astOperand2(), tok2->astOperand2(), constFunctions);

    if (noncommuative_equals)
        return true;

    const bool commutative = tok1->astOperand1() && tok1->astOperand2() && Token::Match(tok1, "%or%|%oror%|+|*|&|&&|^|==|!=");
    bool commuative_equals = commutative &&
                             isSameExpression(cpp, tok1->astOperand2(), tok2->astOperand1(), constFunctions);
    commuative_equals = commuative_equals &&
                        isSameExpression(cpp, tok1->astOperand1(), tok2->astOperand2(), constFunctions);

    return commuative_equals;
}

bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const std::set<std::string> &constFunctions)
{
    if (!cond1 || !cond2)
        return false;

    if (cond1->str() == "!") {
        if (cond2->str() == "!=") {
            if (cond2->astOperand1() && cond2->astOperand1()->str() == "0")
                return isSameExpression(cpp, cond1->astOperand1(), cond2->astOperand2(), constFunctions);
            if (cond2->astOperand2() && cond2->astOperand2()->str() == "0")
                return isSameExpression(cpp, cond1->astOperand1(), cond2->astOperand1(), constFunctions);
        }
        return isSameExpression(cpp, cond1->astOperand1(), cond2, constFunctions);
    }

    if (cond2->str() == "!")
        return isOppositeCond(isNot, cpp, cond2, cond1, constFunctions);

    if (!cond1->isComparisonOp() || !cond2->isComparisonOp())
        return false;

    const std::string &comp1 = cond1->str();

    // condition found .. get comparator
    std::string comp2;
    if (isSameExpression(cpp, cond1->astOperand1(), cond2->astOperand1(), constFunctions) &&
        isSameExpression(cpp, cond1->astOperand2(), cond2->astOperand2(), constFunctions)) {
        comp2 = cond2->str();
    } else if (isSameExpression(cpp, cond1->astOperand1(), cond2->astOperand2(), constFunctions) &&
               isSameExpression(cpp, cond1->astOperand2(), cond2->astOperand1(), constFunctions)) {
        comp2 = cond2->str();
        if (comp2[0] == '>')
            comp2[0] = '<';
        else if (comp2[0] == '<')
            comp2[0] = '>';
    }

    // is condition opposite?
    return ((comp1 == "==" && comp2 == "!=") ||
            (comp1 == "!=" && comp2 == "==") ||
            (comp1 == "<"  && comp2 == ">=") ||
            (comp1 == "<=" && comp2 == ">") ||
            (comp1 == ">"  && comp2 == "<=") ||
            (comp1 == ">=" && comp2 == "<") ||
            (!isNot && ((comp1 == "<" && comp2 == ">") ||
                        (comp1 == ">" && comp2 == "<"))));
}

bool isConstExpression(const Token *tok, const std::set<std::string> &constFunctions)
{
    if (!tok)
        return true;
    if (tok->isName() && tok->next()->str() == "(") {
        if (!tok->function() && !Token::Match(tok->previous(), ".|::") && constFunctions.find(tok->str()) == constFunctions.end())
            return false;
        else if (tok->function() && !tok->function()->isConst())
            return false;
    }
    if (tok->tokType() == Token::eIncDecOp)
        return false;
    // bailout when we see ({..})
    if (tok->str() == "{")
        return false;
    return isConstExpression(tok->astOperand1(),constFunctions) && isConstExpression(tok->astOperand2(),constFunctions);
}

bool isWithoutSideEffects(bool cpp, const Token* tok)
{
    if (!cpp)
        return true;

    while (tok && tok->astOperand2() && tok->astOperand2()->str() != "(")
        tok = tok->astOperand2();
    if (tok && tok->varId()) {
        const Variable* var = tok->variable();
        return var && (!var->isClass() || var->isPointer() || var->isStlType());
    }
    return true;
}

