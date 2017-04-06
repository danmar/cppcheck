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
#include "astutils.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"

static bool astIsCharWithSign(const Token *tok, ValueType::Sign sign)
{
    if (!tok)
        return false;
    const ValueType *valueType = tok->valueType();
    if (!valueType)
        return false;
    return valueType->type == ValueType::Type::CHAR && valueType->pointer == 0U && valueType->sign == sign;
}

bool astIsSignedChar(const Token *tok)
{
    return astIsCharWithSign(tok, ValueType::Sign::SIGNED);
}

bool astIsUnknownSignChar(const Token *tok)
{
    return astIsCharWithSign(tok, ValueType::Sign::UNKNOWN_SIGN);
}

bool astIsIntegral(const Token *tok, bool unknown)
{
    const ValueType *vt = tok ? tok->valueType() : nullptr;
    if (!vt)
        return unknown;
    return vt->isIntegral() && vt->pointer == 0U;
}

bool astIsFloat(const Token *tok, bool unknown)
{
    const ValueType *vt = tok ? tok->valueType() : nullptr;
    if (!vt)
        return unknown;
    return vt->type >= ValueType::Type::FLOAT && vt->pointer == 0U;
}

bool astIsBool(const Token *tok)
{
    return tok && (tok->isBoolean() || (tok->valueType() && tok->valueType()->type == ValueType::Type::BOOL && !tok->valueType()->pointer));
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

static bool match(const Token *tok, const std::string &rhs)
{
    if (tok->str() == rhs)
        return true;
    if (tok->isName() && !tok->varId() && tok->values().size() == 1U && tok->values().front().isKnown() && MathLib::toString(tok->values().front().intvalue) == rhs)
        return true;
    return false;
}

const Token * astIsVariableComparison(const Token *tok, const std::string &comp, const std::string &rhs, const Token **vartok)
{
    if (!tok)
        return nullptr;

    const Token *ret = nullptr;
    if (tok->isComparisonOp()) {
        if (tok->astOperand1() && match(tok->astOperand1(), rhs)) {
            // Invert comparator
            std::string s = tok->str();
            if (s[0] == '>')
                s[0] = '<';
            else if (s[0] == '<')
                s[0] = '>';
            if (s == comp) {
                ret = tok->astOperand2();
            }
        } else if (tok->str() == comp && tok->astOperand2() && match(tok->astOperand2(), rhs)) {
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

bool isSameExpression(bool cpp, bool macro, const Token *tok1, const Token *tok2, const Library& library, bool pure)
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
    if (tok1->varId() != tok2->varId() || tok1->str() != tok2->str() || tok1->originalName() != tok2->originalName()) {
        if ((Token::Match(tok1,"<|>")   && Token::Match(tok2,"<|>")) ||
            (Token::Match(tok1,"<=|>=") && Token::Match(tok2,"<=|>="))) {
            return isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand2(), library, pure) &&
                   isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand1(), library, pure);
        }
        return false;
    }
    if (macro && (tok1->isExpandedMacro() || tok2->isExpandedMacro()))
        return false;
    if (tok1->isComplex() != tok2->isComplex())
        return false;
    if (tok1->isLong() != tok2->isLong())
        return false;
    if (tok1->isUnsigned() != tok2->isUnsigned())
        return false;
    if (tok1->isSigned() != tok2->isSigned())
        return false;
    if (tok1->isName() && tok1->next()->str() == "(" && tok1->str() != "sizeof") {
        if (!tok1->function() && !Token::Match(tok1->previous(), ".|::") && pure && !library.isFunctionConst(tok1->str(), true) && !tok1->isAttributeConst() && !tok1->isAttributePure())
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
        while (t1 && t2 &&
               t1->str() == t2->str() &&
               t1->isLong() == t2->isLong() &&
               t1->isUnsigned() == t2->isUnsigned() &&
               t1->isSigned() == t2->isSigned() &&
               (t1->isName() || t1->str() == "*")) {
            t1 = t1->next();
            t2 = t2->next();
        }
        if (!t1 || !t2 || t1->str() != ")" || t2->str() != ")")
            return false;
    }
    bool noncommutativeEquals =
        isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand1(), library, pure);
    noncommutativeEquals = noncommutativeEquals &&
                           isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand2(), library, pure);

    if (noncommutativeEquals)
        return true;

    // in c++, a+b might be different to b+a, depending on the type of a and b
    if (cpp && tok1->str() == "+") {
        const ValueType* vt1 = tok1->astOperand1()->valueType();
        const ValueType* vt2 = tok1->astOperand1()->valueType();
        if (!(vt1 && (vt1->type >= ValueType::VOID || vt1->pointer) && vt2 && (vt2->type >= ValueType::VOID || vt2->pointer)))
            return false;
    }

    const bool commutative = tok1->astOperand1() && tok1->astOperand2() && Token::Match(tok1, "%or%|%oror%|+|*|&|&&|^|==|!=");
    bool commutativeEquals = commutative &&
                             isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand1(), library, pure);
    commutativeEquals = commutativeEquals &&
                        isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand2(), library, pure);


    return commutativeEquals;
}

bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const Library& library, bool pure)
{
    if (!cond1 || !cond2)
        return false;

    if (cond1->str() == "!") {
        if (cond2->str() == "!=") {
            if (cond2->astOperand1() && cond2->astOperand1()->str() == "0")
                return isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand2(), library, pure);
            if (cond2->astOperand2() && cond2->astOperand2()->str() == "0")
                return isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure);
        }
        return isSameExpression(cpp, true, cond1->astOperand1(), cond2, library, pure);
    }

    if (cond2->str() == "!")
        return isOppositeCond(isNot, cpp, cond2, cond1, library, pure);

    if (!cond1->isComparisonOp() || !cond2->isComparisonOp())
        return false;

    const std::string &comp1 = cond1->str();

    // condition found .. get comparator
    std::string comp2;
    if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure) &&
        isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand2(), library, pure)) {
        comp2 = cond2->str();
    } else if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand2(), library, pure) &&
               isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand1(), library, pure)) {
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

bool isConstExpression(const Token *tok, const Library& library, bool pure)
{
    if (!tok)
        return true;
    if (tok->isName() && tok->next()->str() == "(") {
        if (!tok->function() && !Token::Match(tok->previous(), ".|::") && !library.isFunctionConst(tok->str(), pure))
            return false;
        else if (tok->function() && !tok->function()->isConst())
            return false;
    }
    if (tok->tokType() == Token::eIncDecOp)
        return false;
    // bailout when we see ({..})
    if (tok->str() == "{")
        return false;
    return isConstExpression(tok->astOperand1(), library, pure) && isConstExpression(tok->astOperand2(), library, pure);
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

bool isReturnScope(const Token * const endToken)
{
    if (!endToken || endToken->str() != "}")
        return false;

    const Token *prev = endToken->previous();
    while (prev && Token::simpleMatch(prev->previous(), "; ;"))
        prev = prev->previous();
    if (prev && Token::simpleMatch(prev->previous(), "} ;"))
        prev = prev->previous();

    if (Token::simpleMatch(prev, "}")) {
        if (Token::simpleMatch(prev->link()->tokAt(-2), "} else {"))
            return isReturnScope(prev) && isReturnScope(prev->link()->tokAt(-2));
        if (Token::simpleMatch(prev->link()->previous(), ") {") &&
            Token::simpleMatch(prev->link()->linkAt(-1)->previous(), "switch (") &&
            !Token::findsimplematch(prev->link(), "break", prev)) {
            return true;
        }
        if (Token::simpleMatch(prev->link()->previous(), ") {") &&
            Token::simpleMatch(prev->link()->linkAt(-1)->previous(), "return (")) {
            return true;
        }
        if (Token::Match(prev->link()->previous(), "[;{}] {"))
            return isReturnScope(prev);
    } else if (Token::simpleMatch(prev, ";")) {
        // noreturn function
        if (Token::simpleMatch(prev->previous(), ") ;") && Token::Match(prev->linkAt(-1)->tokAt(-2), "[;{}] %name% ("))
            return true;
        // return/goto statement
        prev = prev->previous();
        while (prev && !Token::Match(prev, ";|{|}|return|goto|throw|continue|break"))
            prev = prev->previous();
        return prev && prev->isName();
    }
    return false;
}

bool isVariableChangedByFunctionCall(const Token *tok, const Settings *settings, bool *inconclusive)
{
    if (!tok)
        return false;

    // address of variable
    const bool addressOf = tok && Token::simpleMatch(tok->previous(), "&");

    // passing variable to subfunction?
    if (Token::Match(tok->tokAt(-2), ") & %name% [,)]") && Token::Match(tok->linkAt(-2)->previous(), "[,(] ("))
        ;
    else if (Token::Match(tok->tokAt(addressOf?-2:-1), "[(,] &| %name% [,)]"))
        ;
    else
        return false;

    // reinterpret_cast etc..
    if (Token::Match(tok->tokAt(-3), "> ( & %name% ) [,)]") &&
        tok->linkAt(-3) &&
        Token::Match(tok->linkAt(-3)->tokAt(-2), "[,(] %type% <"))
        tok = tok->linkAt(-3);

    // goto start of function call and get argnr
    unsigned int argnr = 0;
    while (tok && tok->str() != "(") {
        if (tok->str() == ",")
            ++argnr;
        else if (tok->str() == ")")
            tok = tok->link();
        tok = tok->previous();
    }
    tok = tok ? tok->previous() : nullptr;
    if (tok && tok->link() && tok->str() == ">")
        tok = tok->link()->previous();
    if (!Token::Match(tok, "%name% ("))
        return false; // not a function => variable not changed

    if (tok->varId())
        return false; // Constructor call of tok => variable probably not changed by constructor call

    if (!tok->function()) {
        // if the library says 0 is invalid
        // => it is assumed that parameter is an in parameter (TODO: this is a bad heuristic)
        if (!addressOf && settings->library.isnullargbad(tok, 1+argnr))
            return false;
        // addressOf => inconclusive
        if (!addressOf) {
            if (inconclusive != nullptr)
                *inconclusive = true;
            return false;
        }
        return true;
    }

    const Variable *arg = tok->function()->getArgumentVar(argnr);

    if (addressOf && !(arg && arg->isConst()))
        return true;

    return arg && !arg->isConst() && arg->isReference();
}

bool isVariableChanged(const Token *start, const Token *end, const unsigned int varid, const Settings *settings)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() == varid) {
            if (Token::Match(tok, "%name% %assign%|++|--"))
                return true;

            if (Token::Match(tok->previous(), "++|-- %name%"))
                return true;

            bool inconclusive = false;
            bool isChanged = isVariableChangedByFunctionCall(tok, settings, &inconclusive);
            isChanged |= inconclusive;
            if (isChanged)
                return true;

            const Token *parent = tok->astParent();
            while (Token::Match(parent, ".|::"))
                parent = parent->astParent();
            if (parent && parent->tokType() == Token::eIncDecOp)
                return true;
        }
    }
    return false;
}

int numberOfArguments(const Token *start)
{
    int arguments=0;
    const Token* const openBracket = start->next();
    if (openBracket && openBracket->str()=="(" && openBracket->next() && openBracket->next()->str()!=")") {
        const Token* argument=openBracket->next();
        while (argument) {
            ++arguments;
            argument = argument->nextArgument();
        }
    }
    return arguments;
}
