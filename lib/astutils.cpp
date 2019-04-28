/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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

#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueflow.h"

#include <list>
#include <stack>


void visitAstNodes(const Token *ast, std::function<ChildrenToVisit(const Token *)> visitor)
{
    std::stack<const Token *> tokens;
    tokens.push(ast);
    while (!tokens.empty()) {
        const Token *tok = tokens.top();
        tokens.pop();
        if (!tok)
            continue;

        ChildrenToVisit c = visitor(tok);

        if (c == ChildrenToVisit::done)
            break;
        if (c == ChildrenToVisit::op1 || c == ChildrenToVisit::op1_and_op2)
            tokens.push(tok->astOperand1());
        if (c == ChildrenToVisit::op2 || c == ChildrenToVisit::op1_and_op2)
            tokens.push(tok->astOperand2());
    }
}


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

bool astIsPointer(const Token *tok)
{
    return tok && tok->valueType() && tok->valueType()->pointer;
}

bool astIsIterator(const Token *tok)
{
    return tok && tok->valueType() && tok->valueType()->type == ValueType::Type::ITERATOR;
}

bool astIsContainer(const Token *tok)
{
    return tok && tok->valueType() && tok->valueType()->type == ValueType::Type::CONTAINER;
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
    if (tok->isName() && !tok->varId() && tok->hasKnownIntValue() && MathLib::toString(tok->values().front().intvalue) == rhs)
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
        if (tok->str() == "!") {
            ret = tok->astOperand1();
            // handle (!(x!=0)) as (x==0)
            astIsVariableComparison(ret, "!=", "0", &ret);
        }
    }
    while (ret && ret->str() == ".")
        ret = ret->astOperand2();
    if (ret && ret->str() == "=" && ret->astOperand1() && ret->astOperand1()->varId())
        ret = ret->astOperand1();
    else if (ret && ret->varId() == 0U)
        ret = nullptr;
    if (vartok)
        *vartok = ret;
    return ret;
}

static bool hasToken(const Token * startTok, const Token * stopTok, const Token * tok)
{
    for (const Token * tok2 = startTok; tok2 != stopTok; tok2 = tok2->next()) {
        if (tok2 == tok)
            return true;
    }
    return false;
}

const Token * nextAfterAstRightmostLeaf(const Token * tok)
{
    const Token * rightmostLeaf = tok;
    if (!rightmostLeaf || !rightmostLeaf->astOperand1())
        return nullptr;
    do {
        if (rightmostLeaf->astOperand2())
            rightmostLeaf = rightmostLeaf->astOperand2();
        else
            rightmostLeaf = rightmostLeaf->astOperand1();
    } while (rightmostLeaf->astOperand1());
    while (Token::Match(rightmostLeaf->next(), "]|)") && !hasToken(rightmostLeaf->next()->link(), rightmostLeaf->next(), tok))
        rightmostLeaf = rightmostLeaf->next();
    if (rightmostLeaf->str() == "{" && rightmostLeaf->link())
        rightmostLeaf = rightmostLeaf->link();
    return rightmostLeaf->next();
}

static const Token * getVariableInitExpression(const Variable * var)
{
    if (!var || !var->declEndToken())
        return nullptr;
    if (Token::Match(var->declEndToken(), "; %varid% =", var->declarationId()))
        return var->declEndToken()->tokAt(2)->astOperand2();
    return var->declEndToken()->astOperand2();
}

static bool isInLoopCondition(const Token * tok)
{
    return Token::Match(tok->astTop()->previous(), "for|while (");
}

/// If tok2 comes after tok1
bool precedes(const Token * tok1, const Token * tok2)
{
    if (!tok1)
        return false;
    if (!tok2)
        return false;
    return tok1->progressValue() < tok2->progressValue();
}

static bool isAliased(const Token * startTok, const Token * endTok, unsigned int varid)
{
    for (const Token *tok = startTok; tok != endTok; tok = tok->next()) {
        if (Token::Match(tok, "= & %varid% ;", varid))
            return true;
        if (tok->varId() == varid)
            continue;
        if (tok->varId() == 0)
            continue;
        if (!astIsPointer(tok))
            continue;
        for (const ValueFlow::Value &val : tok->values()) {
            if (!val.isLocalLifetimeValue())
                continue;
            if (val.tokvalue->varId() == varid)
                return true;
        }
    }
    return false;
}

static bool exprDependsOnThis(const Token *expr, unsigned int depth)
{
    if (!expr)
        return false;
    if (depth >= 1000)
        // Abort recursion to avoid stack overflow
        return true;
    ++depth;
    // calling nonstatic method?
    if (Token::Match(expr->previous(), "!!:: %name% (") && expr->function() && expr->function()->nestedIn && expr->function()->nestedIn->isClassOrStruct()) {
        // is it a method of this?
        const Scope *nestedIn = expr->scope()->functionOf;
        if (nestedIn && nestedIn->function)
            nestedIn = nestedIn->function->token->scope();
        while (nestedIn && nestedIn != expr->function()->nestedIn) {
            nestedIn = nestedIn->nestedIn;
        }
        return nestedIn == expr->function()->nestedIn;
    }
    return exprDependsOnThis(expr->astOperand1(), depth) || exprDependsOnThis(expr->astOperand2(), depth);
}

/// This takes a token that refers to a variable and it will return the token
/// to the expression that the variable is assigned to. If its not valid to
/// make such substitution then it will return the original token.
static const Token * followVariableExpression(const Token * tok, bool cpp, const Token * end = nullptr)
{
    if (!tok)
        return tok;
    // Skip following variables that is across multiple files
    if (end && end->fileIndex() != tok->fileIndex())
        return tok;
    // Skip array access
    if (Token::Match(tok, "%var% ["))
        return tok;
    // Skip pointer indirection
    if (tok->astParent() && tok->isUnaryOp("*"))
        return tok;
    // Skip following variables if it is used in an assignment
    if (Token::Match(tok->next(), "%assign%"))
        return tok;
    const Variable * var = tok->variable();
    const Token * varTok = getVariableInitExpression(var);
    if (!varTok)
        return tok;
    // Bailout. If variable value depends on value of "this".
    if (exprDependsOnThis(varTok, 0))
        return tok;
    // Skip array access
    if (Token::simpleMatch(varTok, "["))
        return tok;
    if (var->isVolatile())
        return tok;
    if (!var->isLocal() && !var->isConst())
        return tok;
    if (var->isStatic() && !var->isConst())
        return tok;
    if (var->isArgument())
        return tok;
    const Token * lastTok = precedes(tok, end) ? end : tok;
    // If this is in a loop then check if variables are modified in the entire scope
    const Token * endToken = (isInLoopCondition(tok) || isInLoopCondition(varTok) || var->scope() != tok->scope()) ? var->scope()->bodyEnd : lastTok;
    if (!var->isConst() && (!precedes(varTok, endToken) || isVariableChanged(varTok, endToken, tok->varId(), false, nullptr, cpp)))
        return tok;
    if (precedes(varTok, endToken) && isAliased(varTok, endToken, tok->varId()))
        return tok;
    // Start at beginning of initialization
    const Token * startToken = varTok;
    while (Token::Match(startToken, "%op%|.|(|{") && startToken->astOperand1())
        startToken = startToken->astOperand1();
    // Skip if the variable its referring to is modified
    for (const Token * tok2 = startToken; tok2 != endToken; tok2 = tok2->next()) {
        if (Token::simpleMatch(tok2, ";"))
            break;
        if (tok2->astParent() && tok2->isUnaryOp("*"))
            return tok;
        if (tok2->tokType() == Token::eIncDecOp ||
            tok2->isAssignmentOp() ||
            Token::Match(tok2, "%name% .|[|++|--|%assign%")) {
            return tok;
        }

        if (const Variable * var2 = tok2->variable()) {
            if (!var2->scope())
                return tok;
            const Token * endToken2 = var2->scope() != tok->scope() ? var2->scope()->bodyEnd : endToken;
            if (!var2->isLocal() && !var2->isConst() && !var2->isArgument())
                return tok;
            if (var2->isStatic() && !var2->isConst())
                return tok;
            if (!var2->isConst() && (!precedes(tok2, endToken2) || isVariableChanged(tok2, endToken2, tok2->varId(), false, nullptr, cpp)))
                return tok;
            if (precedes(tok2, endToken2) && isAliased(tok2, endToken2, tok2->varId()))
                return tok;
            // Recognized as a variable but the declaration is unknown
        } else if (tok2->varId() > 0) {
            return tok;
        } else if (tok2->tokType() == Token::eName && !Token::Match(tok2, "sizeof|decltype|typeof") && !tok2->function()) {
            return tok;
        }
    }
    return varTok;
}

static void followVariableExpressionError(const Token *tok1, const Token *tok2, ErrorPath* errors)
{
    if (!errors)
        return;
    if (!tok1)
        return;
    if (!tok2)
        return;
    ErrorPathItem item = std::make_pair(tok2, "'" + tok1->str() + "' is assigned value '" + tok2->expressionString() + "' here.");
    if (std::find(errors->begin(), errors->end(), item) != errors->end())
        return;
    errors->push_back(item);
}

bool isSameExpression(bool cpp, bool macro, const Token *tok1, const Token *tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors)
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
    // Skip double not
    if (Token::simpleMatch(tok1, "!") && Token::simpleMatch(tok1->astOperand1(), "!") && !Token::simpleMatch(tok1->astParent(), "=")) {
        return isSameExpression(cpp, macro, tok1->astOperand1()->astOperand1(), tok2, library, pure, followVar, errors);
    }
    if (Token::simpleMatch(tok2, "!") && Token::simpleMatch(tok2->astOperand1(), "!") && !Token::simpleMatch(tok2->astParent(), "=")) {
        return isSameExpression(cpp, macro, tok1, tok2->astOperand1()->astOperand1(), library, pure, followVar, errors);
    }
    // Follow variable
    if (followVar && tok1->str() != tok2->str() && (Token::Match(tok1, "%var%") || Token::Match(tok2, "%var%"))) {
        const Token * varTok1 = followVariableExpression(tok1, cpp, tok2);
        if (varTok1->str() == tok2->str()) {
            followVariableExpressionError(tok1, varTok1, errors);
            return isSameExpression(cpp, macro, varTok1, tok2, library, true, followVar, errors);
        }
        const Token * varTok2 = followVariableExpression(tok2, cpp, tok1);
        if (tok1->str() == varTok2->str()) {
            followVariableExpressionError(tok2, varTok2, errors);
            return isSameExpression(cpp, macro, tok1, varTok2, library, true, followVar, errors);
        }
        if (varTok1->str() == varTok2->str()) {
            followVariableExpressionError(tok1, varTok1, errors);
            followVariableExpressionError(tok2, varTok2, errors);
            return isSameExpression(cpp, macro, varTok1, varTok2, library, true, followVar, errors);
        }
    }
    if (tok1->varId() != tok2->varId() || tok1->str() != tok2->str() || tok1->originalName() != tok2->originalName()) {
        if ((Token::Match(tok1,"<|>")   && Token::Match(tok2,"<|>")) ||
            (Token::Match(tok1,"<=|>=") && Token::Match(tok2,"<=|>="))) {
            return isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand2(), library, pure, followVar, errors) &&
                   isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand1(), library, pure, followVar, errors);
        }
        return false;
    }
    if (macro && (tok1->isExpandedMacro() || tok2->isExpandedMacro() || tok1->isTemplateArg() || tok2->isTemplateArg()))
        return false;
    if (tok1->isComplex() != tok2->isComplex())
        return false;
    if (tok1->isLong() != tok2->isLong())
        return false;
    if (tok1->isUnsigned() != tok2->isUnsigned())
        return false;
    if (tok1->isSigned() != tok2->isSigned())
        return false;
    if (pure && tok1->isName() && tok1->next()->str() == "(" && tok1->str() != "sizeof") {
        if (!tok1->function()) {
            if (Token::simpleMatch(tok1->previous(), ".")) {
                const Token *lhs = tok1->previous();
                while (Token::Match(lhs, "(|.|["))
                    lhs = lhs->astOperand1();
                const bool lhsIsConst = (lhs->variable() && lhs->variable()->isConst()) ||
                                        (lhs->valueType() && lhs->valueType()->constness > 0) ||
                                        (Token::Match(lhs, "%var% . %name% (") && library.isFunctionConst(lhs->tokAt(2)));
                if (!lhsIsConst)
                    return false;
            } else {
                const Token * ftok = tok1;
                if (Token::simpleMatch(tok1->previous(), "::"))
                    ftok = tok1->previous();
                if (!library.isFunctionConst(ftok) && !ftok->isAttributeConst() && !ftok->isAttributePure())
                    return false;
            }
        } else {
            if (tok1->function() && !tok1->function()->isConst() && !tok1->function()->isAttributeConst() && !tok1->function()->isAttributePure())
                return false;
        }
    }
    // templates/casts
    if ((Token::Match(tok1, "%name% <") && tok1->next()->link()) ||
        (Token::Match(tok2, "%name% <") && tok2->next()->link())) {

        // non-const template function that is not a dynamic_cast => return false
        if (pure && Token::simpleMatch(tok1->next()->link(), "> (") &&
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
    // cast => assert that the casts are equal
    if (tok1->str() == "(" && tok1->previous() &&
        !tok1->previous()->isName() &&
        !(tok1->previous()->str() == ">" && tok1->previous()->link())) {
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
        isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand1(), library, pure, followVar, errors);
    noncommutativeEquals = noncommutativeEquals &&
                           isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand2(), library, pure, followVar, errors);

    if (noncommutativeEquals)
        return true;

    // in c++, a+b might be different to b+a, depending on the type of a and b
    if (cpp && tok1->str() == "+" && tok1->isBinaryOp()) {
        const ValueType* vt1 = tok1->astOperand1()->valueType();
        const ValueType* vt2 = tok1->astOperand2()->valueType();
        if (!(vt1 && (vt1->type >= ValueType::VOID || vt1->pointer) && vt2 && (vt2->type >= ValueType::VOID || vt2->pointer)))
            return false;
    }

    const bool commutative = tok1->isBinaryOp() && Token::Match(tok1, "%or%|%oror%|+|*|&|&&|^|==|!=");
    bool commutativeEquals = commutative &&
                             isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand1(), library, pure, followVar, errors);
    commutativeEquals = commutativeEquals &&
                        isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand2(), library, pure, followVar, errors);


    return commutativeEquals;
}

bool isEqualKnownValue(const Token * const tok1, const Token * const tok2)
{
    return tok1->hasKnownValue() && tok2->hasKnownValue() && tok1->values() == tok2->values();
}

bool isDifferentKnownValues(const Token * const tok1, const Token * const tok2)
{
    return tok1->hasKnownValue() && tok2->hasKnownValue() && tok1->values() != tok2->values();
}

static bool isZeroBoundCond(const Token * const cond)
{
    if (cond == nullptr)
        return false;
    // Assume unsigned
    // TODO: Handle reverse conditions
    const bool isZero = cond->astOperand2()->getValue(0);
    if (cond->str() == "==" || cond->str() == ">=")
        return isZero;
    if (cond->str() == "<=")
        return true;
    if (cond->str() == "<")
        return !isZero;
    if (cond->str() == ">")
        return false;
    return false;
}

bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const Library& library, bool pure, bool followVar, ErrorPath* errors)
{
    if (!cond1 || !cond2)
        return false;

    if (cond1->str() == "!") {
        if (cond2->str() == "!=") {
            if (cond2->astOperand1() && cond2->astOperand1()->str() == "0")
                return isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand2(), library, pure, followVar, errors);
            if (cond2->astOperand2() && cond2->astOperand2()->str() == "0")
                return isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure, followVar, errors);
        }
        return isSameExpression(cpp, true, cond1->astOperand1(), cond2, library, pure, followVar, errors);
    }

    if (cond2->str() == "!")
        return isOppositeCond(isNot, cpp, cond2, cond1, library, pure, followVar, errors);

    if (!isNot) {
        if (cond1->str() == "==" && cond2->str() == "==") {
            if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure, followVar, errors))
                return isDifferentKnownValues(cond1->astOperand2(), cond2->astOperand2());
            if (isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand2(), library, pure, followVar, errors))
                return isDifferentKnownValues(cond1->astOperand1(), cond2->astOperand1());
        }
        // TODO: Handle reverse conditions
        if (Library::isContainerYield(cond1, Library::Container::EMPTY, "empty") &&
            Library::isContainerYield(cond2->astOperand1(), Library::Container::SIZE, "size") &&
            cond1->astOperand1()->astOperand1()->varId() == cond2->astOperand1()->astOperand1()->astOperand1()->varId()) {
            return !isZeroBoundCond(cond2);
        }

        if (Library::isContainerYield(cond2, Library::Container::EMPTY, "empty") &&
            Library::isContainerYield(cond1->astOperand1(), Library::Container::SIZE, "size") &&
            cond2->astOperand1()->astOperand1()->varId() == cond1->astOperand1()->astOperand1()->astOperand1()->varId()) {
            return !isZeroBoundCond(cond1);
        }
    }


    if (!cond1->isComparisonOp() || !cond2->isComparisonOp())
        return false;

    const std::string &comp1 = cond1->str();

    // condition found .. get comparator
    std::string comp2;
    if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure, followVar, errors) &&
        isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand2(), library, pure, followVar, errors)) {
        comp2 = cond2->str();
    } else if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand2(), library, pure, followVar, errors) &&
               isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand1(), library, pure, followVar, errors)) {
        comp2 = cond2->str();
        if (comp2[0] == '>')
            comp2[0] = '<';
        else if (comp2[0] == '<')
            comp2[0] = '>';
    }

    if (!isNot && comp2.empty()) {
        const Token *expr1 = nullptr, *value1 = nullptr, *expr2 = nullptr, *value2 = nullptr;
        std::string op1 = cond1->str(), op2 = cond2->str();
        if (cond1->astOperand2()->hasKnownIntValue()) {
            expr1 = cond1->astOperand1();
            value1 = cond1->astOperand2();
        } else if (cond1->astOperand1()->hasKnownIntValue()) {
            expr1 = cond1->astOperand2();
            value1 = cond1->astOperand1();
            if (op1[0] == '>')
                op1[0] = '<';
            else if (op1[0] == '<')
                op1[0] = '>';
        }
        if (cond2->astOperand2()->hasKnownIntValue()) {
            expr2 = cond2->astOperand1();
            value2 = cond2->astOperand2();
        } else if (cond2->astOperand1()->hasKnownIntValue()) {
            expr2 = cond2->astOperand2();
            value2 = cond2->astOperand1();
            if (op2[0] == '>')
                op2[0] = '<';
            else if (op2[0] == '<')
                op2[0] = '>';
        }
        if (!expr1 || !value1 || !expr2 || !value2) {
            return false;
        }
        if (!isSameExpression(cpp, true, expr1, expr2, library, pure, followVar, errors))
            return false;

        const ValueFlow::Value &rhsValue1 = value1->values().front();
        const ValueFlow::Value &rhsValue2 = value2->values().front();

        if (op1 == "<" || op1 == "<=")
            return (op2 == "==" || op2 == ">" || op2 == ">=") && (rhsValue1.intvalue < rhsValue2.intvalue);
        else if (op1 == ">=" || op1 == ">")
            return (op2 == "==" || op2 == "<" || op2 == "<=") && (rhsValue1.intvalue > rhsValue2.intvalue);

        return false;
    }

    // is condition opposite?
    return ((comp1 == "==" && comp2 == "!=") ||
            (comp1 == "!=" && comp2 == "==") ||
            (comp1 == "<"  && comp2 == ">=") ||
            (comp1 == "<=" && comp2 == ">") ||
            (comp1 == ">"  && comp2 == "<=") ||
            (comp1 == ">=" && comp2 == "<") ||
            (!isNot && ((comp1 == "<" && comp2 == ">") ||
                        (comp1 == ">" && comp2 == "<") ||
                        (comp1 == "==" && (comp2 == "!=" || comp2 == ">" || comp2 == "<")) ||
                        ((comp1 == "!=" || comp1 == ">" || comp1 == "<") && comp2 == "==")
                       )));
}

bool isOppositeExpression(bool cpp, const Token * const tok1, const Token * const tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors)
{
    if (!tok1 || !tok2)
        return false;
    if (isOppositeCond(true, cpp, tok1, tok2, library, pure, followVar, errors))
        return true;
    if (tok1->isUnaryOp("-"))
        return isSameExpression(cpp, true, tok1->astOperand1(), tok2, library, pure, followVar, errors);
    if (tok2->isUnaryOp("-"))
        return isSameExpression(cpp, true, tok2->astOperand1(), tok1, library, pure, followVar, errors);
    return false;
}

bool isConstExpression(const Token *tok, const Library& library, bool pure, bool cpp)
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
    if (tok->isAssignmentOp())
        return false;
    if (isLikelyStreamRead(cpp, tok))
        return false;
    // bailout when we see ({..})
    if (tok->str() == "{")
        return false;
    return isConstExpression(tok->astOperand1(), library, pure, cpp) && isConstExpression(tok->astOperand2(), library, pure, cpp);
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

bool isUniqueExpression(const Token* tok)
{
    if (!tok)
        return true;
    if (tok->function()) {
        const Function * fun = tok->function();
        const Scope * scope = fun->nestedIn;
        if (!scope)
            return true;
        const std::string returnType = fun->retType ? fun->retType->name() : fun->retDef->stringifyList(fun->tokenDef);
        for (const Function& f:scope->functionList) {
            if (f.type != Function::eFunction)
                continue;

            const std::string freturnType = f.retType ? f.retType->name() : f.retDef->stringifyList(f.tokenDef);
            if (f.argumentList.size() == fun->argumentList.size() &&
                returnType == freturnType &&
                f.name() != fun->name()) {
                return false;
            }
        }
    } else if (tok->variable()) {
        const Variable * var = tok->variable();
        const Scope * scope = var->scope();
        if (!scope)
            return true;
        const Type * varType = var->type();
        // Iterate over the variables in scope and the parameters of the function if possible
        const Function * fun = scope->function;
        const std::list<Variable>* setOfVars[] = {&scope->varlist, fun ? &fun->argumentList : nullptr};

        for (const std::list<Variable>* vars:setOfVars) {
            if (!vars)
                continue;
            bool other = std::any_of(vars->cbegin(), vars->cend(), [=](const Variable &v) {
                if (varType)
                    return v.type() && v.type()->name() == varType->name() && v.name() != var->name();
                return v.isFloatingType() == var->isFloatingType() &&
                       v.isEnumType() == var->isEnumType() &&
                       v.isClass() == var->isClass() &&
                       v.isArray() == var->isArray() &&
                       v.isPointer() == var->isPointer() &&
                       v.name() != var->name();
            });
            if (other)
                return false;
        }
    } else if (!isUniqueExpression(tok->astOperand1())) {
        return false;
    }

    return isUniqueExpression(tok->astOperand2());
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
        if (Token::Match(prev->link()->astTop(), "return|throw"))
            return true;
        if (Token::Match(prev->link()->previous(), "[;{}] {"))
            return isReturnScope(prev);
    } else if (Token::simpleMatch(prev, ";")) {
        // noreturn function
        if (Token::simpleMatch(prev->previous(), ") ;") && Token::Match(prev->linkAt(-1)->tokAt(-2), "[;{}] %name% ("))
            return true;
        if (Token::simpleMatch(prev->previous(), ") ;") && prev->previous()->link() &&
            Token::Match(prev->previous()->link()->astTop(), "return|throw"))
            return true;
        if (Token::Match(prev->previous()->astTop(), "return|throw"))
            return true;
        // return/goto statement
        prev = prev->previous();
        while (prev && !Token::Match(prev, ";|{|}|return|goto|throw|continue|break"))
            prev = prev->previous();
        return prev && prev->isName();
    }
    return false;
}

bool isVariableChangedByFunctionCall(const Token *tok, unsigned int varid, const Settings *settings, bool *inconclusive)
{
    if (!tok)
        return false;
    if (tok->varId() == varid)
        return isVariableChangedByFunctionCall(tok, settings, inconclusive);
    return isVariableChangedByFunctionCall(tok->astOperand1(), varid, settings, inconclusive) ||
           isVariableChangedByFunctionCall(tok->astOperand2(), varid, settings, inconclusive);
}

bool isVariableChangedByFunctionCall(const Token *tok, const Settings *settings, bool *inconclusive)
{
    if (!tok)
        return false;

    const Token * const tok1 = tok;

    // address of variable
    const bool addressOf = tok->astParent() && tok->astParent()->isUnaryOp("&");

    {
        const Token *parent = tok->astParent();
        if (parent && parent->isUnaryOp("&"))
            parent = parent->astParent();
        while (parent && parent->isCast())
            parent = parent->astParent();

        // passing variable to subfunction?
        if (Token::Match(parent, "[(,]"))
            ;
        else if (Token::simpleMatch(parent, ":")) {
            while (Token::Match(parent, "[?:]"))
                parent = parent->astParent();
            while (Token::simpleMatch(parent, ","))
                parent = parent->astParent();
            if (!parent || parent->str() != "(")
                return false;
        } else
            return false;
    }

    // goto start of function call and get argnr
    unsigned int argnr = 0;
    while (tok && !Token::Match(tok, "[;{}]")) {
        if (tok->str() == ",")
            ++argnr;
        else if (tok->str() == ")")
            tok = tok->link();
        else if (Token::Match(tok->previous(), "%name% ("))
            break;
        else if (Token::simpleMatch(tok->previous(), "> (") && tok->previous()->link())
            break;
        tok = tok->previous();
    }
    if (!tok || tok->str() != "(")
        return false;
    const bool possiblyPassedByReference = (tok->next() == tok1 || Token::Match(tok1->previous(), ", %name% [,)]"));
    tok = tok->previous();
    if (tok && tok->link() && tok->str() == ">")
        tok = tok->link()->previous();
    if (!Token::Match(tok, "%name% [(<]"))
        return false; // not a function => variable not changed

    // Constructor call
    if (tok->variable() && tok->variable()->nameToken() == tok) {
        // Find constructor..
        const unsigned int argCount = numberOfArguments(tok);
        const Scope *typeScope = tok->variable()->typeScope();
        if (typeScope) {
            for (const Function &function : typeScope->functionList) {
                if (!function.isConstructor() || function.argCount() < argCount)
                    continue;
                const Variable *arg = function.getArgumentVar(argnr);
                if (arg && arg->isReference() && !arg->isConst())
                    return true;
            }
            return false;
        }
        if (inconclusive)
            *inconclusive = true;
        return false;
    }

    if (!tok->function()) {
        // Check if direction (in, out, inout) is specified in the library configuration and use that
        if (!addressOf && settings) {
            const Library::ArgumentChecks::Direction argDirection = settings->library.getArgDirection(tok, 1 + argnr);
            if (argDirection == Library::ArgumentChecks::Direction::DIR_IN)
                return false;
            else if (argDirection == Library::ArgumentChecks::Direction::DIR_OUT ||
                     argDirection == Library::ArgumentChecks::Direction::DIR_INOUT) {
                // With out or inout the direction of the content is specified, not a pointer itself, so ignore pointers for now
                const ValueType * const valueType = tok1->valueType();
                if (valueType && !valueType->pointer) {
                    return true;
                }
            }
        }

        // if the library says 0 is invalid
        // => it is assumed that parameter is an in parameter (TODO: this is a bad heuristic)
        if (!addressOf && settings && settings->library.isnullargbad(tok, 1+argnr))
            return false;
        // possible pass-by-reference => inconclusive
        if (possiblyPassedByReference) {
            if (inconclusive != nullptr)
                *inconclusive = true;
            return false;
        }
        // Safe guess: Assume that parameter is changed by function call
        return true;
    }

    const Variable *arg = tok->function()->getArgumentVar(argnr);

    if (addressOf) {
        if (!(arg && arg->isConst()))
            return true;
        // If const is applied to the pointer, then the value can still be modified
        if (arg && Token::simpleMatch(arg->typeEndToken(), "* const"))
            return true;
    }

    return arg && !arg->isConst() && arg->isReference();
}

bool isVariableChanged(const Token *start, const Token *end, const unsigned int varid, bool globalvar, const Settings *settings, bool cpp)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() != varid) {
            if (globalvar && Token::Match(tok, "%name% ("))
                // TODO: Is global variable really changed by function call?
                return true;
            continue;
        }

        const Token *tok2 = tok;
        while (Token::simpleMatch(tok2->astParent(), "*"))
            tok2 = tok2->astParent();

        if (Token::Match(tok2->astParent(), "++|--"))
            return true;

        if (tok2->astParent() && tok2->astParent()->isAssignmentOp() && tok2 == tok2->astParent()->astOperand1())
            return true;

        if (isLikelyStreamRead(cpp, tok->previous()))
            return true;

        // Member function call
        if (Token::Match(tok, "%name% . %name% (")) {
            const Variable * var = tok->variable();
            bool isConst = var && var->isConst();
            if (!isConst && var) {
                const ValueType * valueType = var->valueType();
                isConst = (valueType && valueType->pointer == 1 && valueType->constness == 1);
            }

            const Token *ftok = tok->tokAt(2);
            const Function * fun = ftok->function();
            if (!isConst && (!fun || !fun->isConst()))
                return true;
        }

        const Token *ftok = tok;
        while (ftok && (!Token::Match(ftok, "[({[]") || ftok->isCast()))
            ftok = ftok->astParent();

        if (ftok && Token::Match(ftok->link(), ") !!{")) {
            bool inconclusive = false;
            bool isChanged = isVariableChangedByFunctionCall(tok, settings, &inconclusive);
            isChanged |= inconclusive;
            if (isChanged)
                return true;
        }

        const Token *parent = tok->astParent();
        while (Token::Match(parent, ".|::"))
            parent = parent->astParent();
        if (parent && parent->tokType() == Token::eIncDecOp)
            return true;
    }
    return false;
}

bool isVariableChanged(const Variable * var, const Settings *settings, bool cpp)
{
    if (!var)
        return false;
    if (!var->scope())
        return false;
    const Token * start = var->declEndToken();
    if (!start)
        return false;
    if (Token::Match(start, "; %varid% =", var->declarationId()))
        start = start->tokAt(2);
    return isVariableChanged(start->next(), var->scope()->bodyEnd, var->declarationId(), var->isGlobal(), settings, cpp);
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

static void getArgumentsRecursive(const Token *tok, std::vector<const Token *> *arguments, unsigned int depth)
{
    ++depth;
    if (!tok || depth >= 100)
        return;
    if (tok->str() == ",") {
        getArgumentsRecursive(tok->astOperand1(), arguments, depth);
        getArgumentsRecursive(tok->astOperand2(), arguments, depth);
    } else {
        arguments->push_back(tok);
    }
}

std::vector<const Token *> getArguments(const Token *ftok)
{
    std::vector<const Token *> arguments;
    const Token *tok = ftok->next();
    if (!Token::Match(tok, "(|{"))
        tok = ftok;
    const Token *startTok = tok->astOperand2();
    if (!startTok && Token::simpleMatch(tok->astOperand1(), ","))
        startTok = tok->astOperand1();
    getArgumentsRecursive(startTok, &arguments, 0);
    return arguments;
}

const Token *findLambdaStartToken(const Token *last)
{
    if (!last || last->str() != "}")
        return nullptr;
    const Token* tok = last->link();
    if (Token::simpleMatch(tok->astParent(), "("))
        tok = tok->astParent();
    if (Token::simpleMatch(tok->astParent(), "["))
        return tok->astParent();
    return nullptr;
}

const Token *findLambdaEndToken(const Token *first)
{
    if (!first || first->str() != "[")
        return nullptr;
    if (!Token::Match(first->link(), "] (|{"))
        return nullptr;
    if (first->astOperand1() != first->link()->next())
        return nullptr;
    const Token * tok = first;

    if (tok->astOperand1() && tok->astOperand1()->str() == "(")
        tok = tok->astOperand1();
    if (tok->astOperand1() && tok->astOperand1()->str() == "{")
        return tok->astOperand1()->link();
    return nullptr;
}

bool isLikelyStreamRead(bool cpp, const Token *op)
{
    if (!cpp)
        return false;

    if (!Token::Match(op, "&|>>") || !op->isBinaryOp())
        return false;

    if (!Token::Match(op->astOperand2(), "%name%|.|*|[") && op->str() != op->astOperand2()->str())
        return false;

    const Token *parent = op;
    while (parent->astParent() && parent->astParent()->str() == op->str())
        parent = parent->astParent();
    if (parent->astParent() && !Token::Match(parent->astParent(), "%oror%|&&|(|,|!"))
        return false;
    if (op->str() == "&" && parent->astParent())
        return false;
    if (!parent->astOperand1() || !parent->astOperand2())
        return false;
    return (!parent->astOperand1()->valueType() || !parent->astOperand1()->valueType()->isIntegral());
}

bool isCPPCast(const Token* tok)
{
    return tok && Token::simpleMatch(tok->previous(), "> (") && tok->astOperand2() && tok->astOperand1() && tok->astOperand1()->str().find("_cast") != std::string::npos;
}

bool isConstVarExpression(const Token *tok)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok->previous(), "sizeof ("))
        return true;
    if (Token::Match(tok->previous(), "%name% (")) {
        std::vector<const Token *> args = getArguments(tok);
        return std::all_of(args.begin(), args.end(), &isConstVarExpression);
    }
    if (isCPPCast(tok)) {
        return isConstVarExpression(tok->astOperand2());
    }
    if (Token::Match(tok, "( %type%"))
        return isConstVarExpression(tok->astOperand1());
    if (Token::Match(tok, "%cop%|[|.")) {
        if (tok->astOperand1() && !isConstVarExpression(tok->astOperand1()))
            return false;
        if (tok->astOperand2() && !isConstVarExpression(tok->astOperand2()))
            return false;
        return true;
    }
    if (Token::Match(tok, "%bool%|%num%|%str%|%char%|nullptr|NULL"))
        return true;
    if (tok->isEnumerator())
        return true;
    if (tok->variable())
        return tok->variable()->isConst();
    return false;
}

static bool nonLocal(const Variable* var, bool deref)
{
    return !var || (!var->isLocal() && !var->isArgument()) || (deref && var->isArgument() && var->isPointer()) || var->isStatic() || var->isReference() || var->isExtern();
}

static bool hasFunctionCall(const Token *tok)
{
    if (!tok)
        return false;
    if (Token::Match(tok, "%name% ("))
        // todo, const/pure function?
        return true;
    return hasFunctionCall(tok->astOperand1()) || hasFunctionCall(tok->astOperand2());
}

struct FwdAnalysis::Result FwdAnalysis::checkRecursive(const Token *expr, const Token *startToken, const Token *endToken, const std::set<unsigned int> &exprVarIds, bool local)
{
    // Parse the given tokens
    for (const Token *tok = startToken; tok != endToken; tok = tok->next()) {
        if (Token::simpleMatch(tok, "try {")) {
            // TODO: handle try
            return Result(Result::Type::BAILOUT);
        }

        if (Token::simpleMatch(tok, "break ;")) {
            return Result(Result::Type::BREAK, tok);
        }

        if (Token::simpleMatch(tok, "goto"))
            return Result(Result::Type::BAILOUT);

        if (tok->str() == "continue")
            // TODO
            return Result(Result::Type::BAILOUT);

        if (const Token *lambdaEndToken = findLambdaEndToken(tok)) {
            tok = lambdaEndToken;
            const Result lambdaResult = checkRecursive(expr, lambdaEndToken->link()->next(), lambdaEndToken, exprVarIds, local);
            if (lambdaResult.type == Result::Type::READ || lambdaResult.type == Result::Type::BAILOUT)
                return lambdaResult;
        }

        if (Token::Match(tok, "return|throw")) {
            // TODO: Handle these better
            // Is expr variable used in expression?
            const Token *end = tok->findExpressionStartEndTokens().second->next();
            for (const Token *tok2 = tok; tok2 != end; tok2 = tok2->next()) {
                if (!local && Token::Match(tok2, "%name% ("))
                    return Result(Result::Type::READ);
                if (tok2->varId() && exprVarIds.find(tok2->varId()) != exprVarIds.end())
                    return Result(Result::Type::READ);
            }

            if (!local && mWhat == What::Reassign)
                return Result(Result::Type::BAILOUT);

            return Result(Result::Type::RETURN);
        }

        if (tok->str() == "}") {
            // Known value => possible value
            if (tok->scope() == expr->scope())
                mValueFlowKnown = false;

            Scope::ScopeType scopeType = tok->scope()->type;
            if (scopeType == Scope::eWhile || scopeType == Scope::eFor || scopeType == Scope::eDo) {
                // check condition
                const Token *conditionStart = nullptr;
                const Token *conditionEnd = nullptr;
                if (Token::simpleMatch(tok->link()->previous(), ") {")) {
                    conditionEnd = tok->link()->previous();
                    conditionStart = conditionEnd->link();
                } else if (Token::simpleMatch(tok->link()->previous(), "do {") && Token::simpleMatch(tok, "} while (")) {
                    conditionStart = tok->tokAt(2);
                    conditionEnd = conditionStart->link();
                }
                if (conditionStart && conditionEnd) {
                    bool used = false;
                    for (const Token *condTok = conditionStart; condTok != conditionEnd; condTok = condTok->next()) {
                        if (exprVarIds.find(condTok->varId()) != exprVarIds.end())
                            used = true;
                    }
                    if (used)
                        return Result(Result::Type::BAILOUT);
                }

                // check loop body again..
                const struct FwdAnalysis::Result &result = checkRecursive(expr, tok->link(), tok, exprVarIds, local);
                if (result.type == Result::Type::BAILOUT || result.type == Result::Type::READ)
                    return result;
            }
        }

        if (Token::simpleMatch(tok, "else {"))
            tok = tok->linkAt(1);

        if (Token::simpleMatch(tok, "asm ("))
            return Result(Result::Type::BAILOUT);

        if (mWhat == What::ValueFlow && Token::Match(tok, "while|for (")) {
            // TODO: only bailout if expr is reassigned in loop
            return Result(Result::Type::BAILOUT);
        }

        if (!local && Token::Match(tok, "%name% (") && !Token::simpleMatch(tok->linkAt(1), ") {")) {
            // TODO: this is a quick bailout
            return Result(Result::Type::BAILOUT);
        }

        if (expr->isName() && Token::Match(tok, "%name% (") && tok->str().find("<") != std::string::npos && tok->str().find(expr->str()) != std::string::npos)
            return Result(Result::Type::BAILOUT);


        if (exprVarIds.find(tok->varId()) != exprVarIds.end()) {
            const Token *parent = tok;
            bool other = false;
            bool same = tok->astParent() && isSameExpression(mCpp, false, expr, tok, mLibrary, false, false, nullptr);
            while (!same && Token::Match(parent->astParent(), "*|.|::|[")) {
                parent = parent->astParent();
                if (parent && isSameExpression(mCpp, false, expr, parent, mLibrary, false, false, nullptr)) {
                    same = true;
                    if (mWhat == What::ValueFlow) {
                        KnownAndToken v;
                        v.known = mValueFlowKnown;
                        v.token = parent;
                        mValueFlow.push_back(v);
                    }
                }
                if (Token::Match(parent, ". %var%") && parent->next()->varId() && exprVarIds.find(parent->next()->varId()) == exprVarIds.end()) {
                    other = true;
                    break;
                }
            }
            if (mWhat != What::ValueFlow && same && Token::simpleMatch(parent->astParent(), "[") && parent == parent->astParent()->astOperand2()) {
                return Result(Result::Type::READ);
            }
            if (other)
                continue;
            if (Token::simpleMatch(parent->astParent(), "=") && parent == parent->astParent()->astOperand1()) {
                if (!local && hasFunctionCall(parent->astParent()->astOperand2())) {
                    // TODO: this is a quick bailout
                    return Result(Result::Type::BAILOUT);
                }
                if (hasOperand(parent->astParent()->astOperand2(), expr)) {
                    if (mWhat == What::Reassign)
                        return Result(Result::Type::READ);
                    continue;
                }
                const bool reassign = isSameExpression(mCpp, false, expr, parent, mLibrary, false, false, nullptr);
                if (reassign)
                    return Result(Result::Type::WRITE, parent->astParent());
                return Result(Result::Type::READ);
            } else if (Token::Match(parent->astParent(), "%assign%") && !parent->astParent()->astParent() && parent == parent->astParent()->astOperand1()) {
                continue;
            } else {
                // TODO: this is a quick bailout
                return Result(Result::Type::BAILOUT, parent->astParent());
            }
        }

        if (Token::simpleMatch(tok, ") {")) {
            if (Token::simpleMatch(tok->link()->previous(), "switch ("))
                // TODO: parse switch
                return Result(Result::Type::BAILOUT);
            const Result &result1 = checkRecursive(expr, tok->tokAt(2), tok->linkAt(1), exprVarIds, local);
            if (result1.type == Result::Type::READ || result1.type == Result::Type::BAILOUT)
                return result1;
            if (mWhat == What::ValueFlow && result1.type == Result::Type::WRITE)
                mValueFlowKnown = false;
            if (Token::simpleMatch(tok->linkAt(1), "} else {")) {
                const Token *elseStart = tok->linkAt(1)->tokAt(2);
                const Result &result2 = checkRecursive(expr, elseStart, elseStart->link(), exprVarIds, local);
                if (mWhat == What::ValueFlow && result2.type == Result::Type::WRITE)
                    mValueFlowKnown = false;
                if (result2.type == Result::Type::READ || result2.type == Result::Type::BAILOUT)
                    return result2;
                if (result1.type == Result::Type::WRITE && result2.type == Result::Type::WRITE)
                    return result1;
                tok = elseStart->link();
            } else {
                tok = tok->linkAt(1);
            }
        }
    }

    return Result(Result::Type::NONE);
}

bool FwdAnalysis::isGlobalData(const Token *expr) const
{
    bool globalData = false;
    visitAstNodes(expr,
    [&](const Token *tok) {
        if (tok->varId() && !tok->variable()) {
            // Bailout, this is probably global
            globalData = true;
            return ChildrenToVisit::none;
        }
        if (tok->originalName() == "->") {
            // TODO check if pointer points at local data
            globalData = true;
            return ChildrenToVisit::none;
        } else if (Token::Match(tok, "[*[]") && tok->astOperand1() && tok->astOperand1()->variable()) {
            // TODO check if pointer points at local data
            const Variable *lhsvar = tok->astOperand1()->variable();
            const ValueType *lhstype = tok->astOperand1()->valueType();
            if (lhsvar->isPointer()) {
                globalData = true;
                return ChildrenToVisit::none;
            } else if (lhsvar->isArgument() && lhsvar->isArray()) {
                globalData = true;
                return ChildrenToVisit::none;
            } else if (lhsvar->isArgument() && (!lhstype || (lhstype->type <= ValueType::Type::VOID && !lhstype->container))) {
                globalData = true;
                return ChildrenToVisit::none;
            }
        }
        if (tok->varId() == 0 && tok->isName() && tok->previous()->str() != ".") {
            globalData = true;
            return ChildrenToVisit::none;
        }
        if (tok->variable()) {
            // TODO : Check references
            if (tok->variable()->isReference() && tok != tok->variable()->nameToken()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->variable()->isExtern()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->previous()->str() != "." && !tok->variable()->isLocal() && !tok->variable()->isArgument()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->variable()->isArgument() && tok->variable()->isPointer() && tok != expr) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->variable()->isPointerArray()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
        }
        // Unknown argument type => it might be some reference type..
        if (mCpp && tok->str() == "." && tok->astOperand1() && tok->astOperand1()->variable() && !tok->astOperand1()->valueType()) {
            globalData = true;
            return ChildrenToVisit::none;
        }
        if (Token::Match(tok, ".|["))
            return ChildrenToVisit::op1;
        return ChildrenToVisit::op1_and_op2;
    });
    return globalData;
}

FwdAnalysis::Result FwdAnalysis::check(const Token *expr, const Token *startToken, const Token *endToken)
{
    // all variable ids in expr.
    std::set<unsigned int> exprVarIds;
    bool local = true;
    bool unknownVarId = false;
    visitAstNodes(expr,
    [&](const Token *tok) {
        if (tok->varId() == 0 && tok->isName() && tok->previous()->str() != ".") {
            // unknown variable
            unknownVarId = true;
            return ChildrenToVisit::none;
        }
        if (tok->varId() > 0) {
            exprVarIds.insert(tok->varId());
            if (!Token::simpleMatch(tok->previous(), ".")) {
                const Variable *var = tok->variable();
                if (var && var->isReference() && var->isLocal() && Token::Match(var->nameToken(), "%var% [=(]") && !isGlobalData(var->nameToken()->next()->astOperand2()))
                    return ChildrenToVisit::none;
                const bool deref = tok->astParent() && (tok->astParent()->isUnaryOp("*") || (tok->astParent()->str() == "[" && tok == tok->astParent()->astOperand1()));
                local &= !nonLocal(tok->variable(), deref);
            }
        }
        return ChildrenToVisit::op1_and_op2;
    });

    if (unknownVarId)
        return Result(FwdAnalysis::Result::Type::BAILOUT);

    if (mWhat == What::Reassign && isGlobalData(expr))
        local = false;

    // In unused values checking we do not want to check assignments to
    // global data.
    if (mWhat == What::UnusedValue && isGlobalData(expr))
        return Result(FwdAnalysis::Result::Type::BAILOUT);

    Result result = checkRecursive(expr, startToken, endToken, exprVarIds, local);

    // Break => continue checking in outer scope
    while (result.type == FwdAnalysis::Result::Type::BREAK) {
        const Scope *s = result.token->scope();
        while (s->type == Scope::eIf)
            s = s->nestedIn;
        if (s->type != Scope::eSwitch && s->type != Scope::eWhile && s->type != Scope::eFor)
            break;
        result = checkRecursive(expr, s->bodyEnd->next(), endToken, exprVarIds, local);
    }

    return result;
}

bool FwdAnalysis::hasOperand(const Token *tok, const Token *lhs) const
{
    if (!tok)
        return false;
    if (isSameExpression(mCpp, false, tok, lhs, mLibrary, false, false, nullptr))
        return true;
    return hasOperand(tok->astOperand1(), lhs) || hasOperand(tok->astOperand2(), lhs);
}

const Token *FwdAnalysis::reassign(const Token *expr, const Token *startToken, const Token *endToken)
{
    mWhat = What::Reassign;
    Result result = check(expr, startToken, endToken);
    return result.type == FwdAnalysis::Result::Type::WRITE ? result.token : nullptr;
}

bool FwdAnalysis::unusedValue(const Token *expr, const Token *startToken, const Token *endToken)
{
    mWhat = What::UnusedValue;
    Result result = check(expr, startToken, endToken);
    return (result.type == FwdAnalysis::Result::Type::NONE || result.type == FwdAnalysis::Result::Type::RETURN) && !possiblyAliased(expr, startToken);
}

std::vector<FwdAnalysis::KnownAndToken> FwdAnalysis::valueFlow(const Token *expr, const Token *startToken, const Token *endToken)
{
    mWhat = What::ValueFlow;
    mValueFlowKnown = true;
    check(expr, startToken, endToken);
    return mValueFlow;
}

bool FwdAnalysis::possiblyAliased(const Token *expr, const Token *startToken) const
{
    if (expr->isUnaryOp("*"))
        return true;

    const bool macro = false;
    const bool pure = false;
    const bool followVar = false;
    for (const Token *tok = startToken; tok; tok = tok->previous()) {
        if (tok->str() == "{" && tok->scope()->type == Scope::eFunction)
            break;

        if (Token::Match(tok, "%name% (") && !Token::Match(tok, "if|while|for")) {
            // Is argument passed by reference?
            const std::vector<const Token*> args = getArguments(tok);
            for (unsigned int argnr = 0; argnr < args.size(); ++argnr) {
                if (!Token::Match(args[argnr], "%name%|.|::"))
                    continue;
                if (tok->function() && tok->function()->getArgumentVar(argnr) && !tok->function()->getArgumentVar(argnr)->isReference() && !tok->function()->isConst())
                    continue;
                for (const Token *subexpr = expr; subexpr; subexpr = subexpr->astOperand1()) {
                    if (isSameExpression(mCpp, macro, subexpr, args[argnr], mLibrary, pure, followVar))
                        return true;
                }
            }
            continue;
        }

        const Token *addrOf = nullptr;
        if (Token::Match(tok, "& %name% ="))
            addrOf = tok->tokAt(2)->astOperand2();
        else if (tok->isUnaryOp("&"))
            addrOf = tok->astOperand1();
        else if (Token::simpleMatch(tok, "std :: ref ("))
            addrOf = tok->tokAt(3)->astOperand2();
        else
            continue;

        for (const Token *subexpr = expr; subexpr; subexpr = subexpr->astOperand1()) {
            if (isSameExpression(mCpp, macro, subexpr, addrOf, mLibrary, pure, followVar))
                return true;
        }
    }
    return false;
}

bool FwdAnalysis::isNullOperand(const Token *expr)
{
    if (!expr)
        return false;
    if (Token::Match(expr, "( %name% %name%| * )") && Token::Match(expr->astOperand1(), "0|NULL|nullptr"))
        return true;
    return Token::Match(expr, "NULL|nullptr");
}
