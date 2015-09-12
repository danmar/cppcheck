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
// Check for condition mismatches
//---------------------------------------------------------------------------

#include "checkcondition.h"
#include "astutils.h"
#include "checkother.h"
#include "symboldatabase.h"

#include <limits>
#include <stack>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckCondition instance;
}


void CheckCondition::assignIf()
{
    if (!_settings->isEnabled("style"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "=")
            continue;

        if (Token::Match(tok->tokAt(-2), "[;{}] %var% =")) {
            const Variable *var = tok->previous()->variable();
            if (var == 0)
                continue;

            char bitop = '\0';
            MathLib::bigint num = 0;

            if (Token::Match(tok->next(), "%num% [&|]")) {
                bitop = tok->strAt(2).at(0);
                num = MathLib::toLongNumber(tok->next()->str());
            } else {
                const Token *endToken = Token::findsimplematch(tok, ";");

                // Casting address
                if (endToken && Token::Match(endToken->tokAt(-4), "* ) & %any% ;"))
                    endToken = nullptr;

                if (endToken && Token::Match(endToken->tokAt(-2), "[&|] %num% ;")) {
                    bitop = endToken->strAt(-2).at(0);
                    num = MathLib::toLongNumber(endToken->previous()->str());
                }
            }

            if (bitop == '\0')
                continue;

            if (num < 0 && bitop == '|')
                continue;

            assignIfParseScope(tok, tok->tokAt(4), var->declarationId(), var->isLocal(), bitop, num);
        }
    }
}

static bool isParameterChanged(const Token *partok)
{
    bool addressOf = Token::Match(partok, "[(,] &");
    unsigned int argumentNumber = 0;
    const Token *ftok;
    for (ftok = partok; ftok && ftok->str() != "("; ftok = ftok->previous()) {
        if (ftok->str() == ")")
            ftok = ftok->link();
        else if (argumentNumber == 0U && ftok->str() == "&")
            addressOf = true;
        else if (ftok->str() == ",")
            argumentNumber++;
    }
    ftok = ftok ? ftok->previous() : nullptr;
    if (!(ftok && ftok->function()))
        return true;
    if (ftok->function()->isConst())
        return false;
    const Variable *par = ftok->function()->getArgumentVar(argumentNumber);
    if (!par)
        return true;
    if (par->isConst())
        return false;
    if (addressOf || par->isReference() || par->isPointer())
        return true;
    return false;
}

/** parse scopes recursively */
bool CheckCondition::assignIfParseScope(const Token * const assignTok,
                                        const Token * const startTok,
                                        const unsigned int varid,
                                        const bool islocal,
                                        const char bitop,
                                        const MathLib::bigint num)
{
    bool ret = false;

    for (const Token *tok2 = startTok; tok2; tok2 = tok2->next()) {
        if ((bitop == '&') && Token::Match(tok2->tokAt(2), "%varid% %cop% %num% ;", varid) && tok2->strAt(3) == std::string(1U, bitop)) {
            const MathLib::bigint num2 = MathLib::toLongNumber(tok2->strAt(4));
            if (0 == (num & num2))
                mismatchingBitAndError(assignTok, num, tok2, num2);
        }
        if (Token::Match(tok2, "%varid% =", varid)) {
            return true;
        }
        if (Token::Match(tok2, "++|-- %varid%", varid) || Token::Match(tok2, "%varid% ++|--", varid))
            return true;
        if (Token::Match(tok2, "[(,] &| %varid% [,)]", varid) && isParameterChanged(tok2))
            return true;
        if (tok2->str() == "}")
            return false;
        if (Token::Match(tok2, "break|continue|return"))
            ret = true;
        if (ret && tok2->str() == ";")
            return false;
        if (!islocal && Token::Match(tok2, "%name% (") && !Token::simpleMatch(tok2->next()->link(), ") {"))
            return true;
        if (Token::Match(tok2, "if|while (")) {
            if (!islocal && tok2->str() == "while")
                continue;

            // parse condition
            const Token * const end = tok2->next()->link();
            for (; tok2 != end; tok2 = tok2->next()) {
                if (Token::Match(tok2, "[(,] &| %varid% [,)]", varid)) {
                    return true;
                }
                if (Token::Match(tok2,"&&|%oror%|( %varid% %any% %num% &&|%oror%|)", varid)) {
                    const Token *vartok = tok2->next();
                    const std::string& op(vartok->strAt(1));
                    const MathLib::bigint num2 = MathLib::toLongNumber(vartok->strAt(2));
                    const std::string condition(vartok->str() + op + vartok->strAt(2));
                    if (op == "==" && (num & num2) != ((bitop=='&') ? num2 : num))
                        assignIfError(assignTok, tok2, condition, false);
                    else if (op == "!=" && (num & num2) != ((bitop=='&') ? num2 : num))
                        assignIfError(assignTok, tok2, condition, true);
                }
                if (Token::Match(tok2, "%varid% %op%", varid) && tok2->next()->isAssignmentOp()) {
                    return true;
                }
            }

            bool ret1 = assignIfParseScope(assignTok, end->tokAt(2), varid, islocal, bitop, num);
            bool ret2 = false;
            if (Token::simpleMatch(end->next()->link(), "} else {"))
                ret2 = assignIfParseScope(assignTok, end->next()->link()->tokAt(3), varid, islocal, bitop, num);
            if (ret1 || ret2)
                return true;
        }
    }
    return false;
}

void CheckCondition::assignIfError(const Token *tok1, const Token *tok2, const std::string &condition, bool result)
{
    std::list<const Token *> locations;
    locations.push_back(tok1);
    locations.push_back(tok2);

    reportError(locations,
                Severity::style,
                "assignIfError",
                "Mismatching assignment and comparison, comparison '" + condition + "' is always " + std::string(result ? "true" : "false") + ".");
}


void CheckCondition::mismatchingBitAndError(const Token *tok1, const MathLib::bigint num1, const Token *tok2, const MathLib::bigint num2)
{
    std::list<const Token *> locations;
    locations.push_back(tok1);
    locations.push_back(tok2);

    std::ostringstream msg;
    msg << "Mismatching bitmasks. Result is always 0 ("
        << "X = Y & 0x" << std::hex << num1 << "; Z = X & 0x" << std::hex << num2 << "; => Z=0).";

    reportError(locations,
                Severity::style,
                "mismatchingBitAnd",
                msg.str());
}


static void getnumchildren(const Token *tok, std::list<MathLib::bigint> &numchildren)
{
    if (tok->astOperand1() && tok->astOperand1()->isNumber())
        numchildren.push_back(MathLib::toLongNumber(tok->astOperand1()->str()));
    else if (tok->astOperand1() && tok->str() == tok->astOperand1()->str())
        getnumchildren(tok->astOperand1(), numchildren);
    if (tok->astOperand2() && tok->astOperand2()->isNumber())
        numchildren.push_back(MathLib::toLongNumber(tok->astOperand2()->str()));
    else if (tok->astOperand2() && tok->str() == tok->astOperand2()->str())
        getnumchildren(tok->astOperand2(), numchildren);
}

/* Return whether tok is in the body for a function returning a boolean. */
static bool inBooleanFunction(const Token *tok)
{
    const Scope *scope = tok ? tok->scope() : 0;
    while (scope && scope->isLocal())
        scope = scope->nestedIn;
    if (scope && scope->type == Scope::eFunction) {
        const Function *func = scope->function;
        if (func) {
            const Token *ret = func->retDef;
            while (ret && Token::Match(ret, "static|const"))
                ret = ret->next();
            return ret && (ret->str() == "bool");
        }
    }
    return false;
}

void CheckCondition::checkBadBitmaskCheck()
{
    if (!_settings->isEnabled("warning"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "|" && tok->astOperand1() && tok->astOperand2() && tok->astParent()) {
            const Token* parent = tok->astParent();
            const bool isBoolean = Token::Match(parent, "&&|%oror%") ||
                                   (parent->str() == "?" && parent->astOperand1() == tok) ||
                                   (parent->str() == "=" && parent->astOperand2() == tok && parent->astOperand1() && parent->astOperand1()->variable() && parent->astOperand1()->variable()->typeStartToken()->str() == "bool") ||
                                   (parent->str() == "(" && Token::Match(parent->astOperand1(), "if|while")) ||
                                   (parent->str() == "return" && parent->astOperand1() == tok && inBooleanFunction(tok));

            const bool isTrue = (tok->astOperand1()->values.size() == 1 && tok->astOperand1()->values.front().intvalue != 0 && !tok->astOperand1()->values.front().conditional) ||
                                (tok->astOperand2()->values.size() == 1 && tok->astOperand2()->values.front().intvalue != 0 && !tok->astOperand2()->values.front().conditional);

            if (isBoolean && isTrue)
                badBitmaskCheckError(tok);
        }
    }
}

void CheckCondition::badBitmaskCheckError(const Token *tok)
{
    reportError(tok, Severity::warning, "badBitmaskCheck", "Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?");
}

void CheckCondition::comparison()
{
    if (!_settings->isEnabled("style"))
        return;

    // Experimental code based on AST
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "==|!=")) {
            const Token *expr1 = tok->astOperand1();
            const Token *expr2 = tok->astOperand2();
            if (!expr1 || !expr2)
                continue;
            if (expr1->isNumber())
                std::swap(expr1,expr2);
            if (!expr2->isNumber())
                continue;
            const MathLib::bigint num2 = MathLib::toLongNumber(expr2->str());
            if (num2 < 0)
                continue;
            if (!Token::Match(expr1,"[&|]"))
                continue;
            std::list<MathLib::bigint> numbers;
            getnumchildren(expr1, numbers);
            for (std::list<MathLib::bigint>::const_iterator num = numbers.begin(); num != numbers.end(); ++num) {
                const MathLib::bigint num1 = *num;
                if (num1 < 0)
                    continue;
                if ((expr1->str() == "&" && (num1 & num2) != num2) ||
                    (expr1->str() == "|" && (num1 | num2) != num2)) {
                    const std::string& op(tok->str());
                    comparisonError(expr1, expr1->str(), num1, op, num2, op=="==" ? false : true);
                }
            }
        }
    }
}

void CheckCondition::comparisonError(const Token *tok, const std::string &bitop, MathLib::bigint value1, const std::string &op, MathLib::bigint value2, bool result)
{
    std::ostringstream expression;
    expression << std::hex << "(X " << bitop << " 0x" << value1 << ") " << op << " 0x" << value2;

    const std::string errmsg("Expression '" + expression.str() + "' is always " + (result?"true":"false") + ".\n"
                             "The expression '" + expression.str() + "' is always " + (result?"true":"false") +
                             ". Check carefully constants and operators used, these errors might be hard to "
                             "spot sometimes. In case of complex expression it might help to split it to "
                             "separate expressions.");

    reportError(tok, Severity::style, "comparisonError", errmsg);
}

bool CheckCondition::isOverlappingCond(const Token * const cond1, const Token * const cond2, const std::set<std::string> &constFunctions) const
{
    if (!cond1 || !cond2)
        return false;

    // same expressions
    if (isSameExpression(_tokenizer->isCPP(), cond1,cond2,constFunctions))
        return true;

    // bitwise overlap for example 'x&7' and 'x==1'
    if (cond1->str() == "&" && cond1->astOperand1() && cond2->astOperand2()) {
        const Token *expr1 = cond1->astOperand1();
        const Token *num1  = cond1->astOperand2();
        if (!num1) // unary operator&
            return false;
        if (!num1->isNumber())
            std::swap(expr1,num1);
        if (!num1->isNumber() || MathLib::isNegative(num1->str()))
            return false;

        if (!Token::Match(cond2, "&|==") || !cond2->astOperand1() || !cond2->astOperand2())
            return false;
        const Token *expr2 = cond2->astOperand1();
        const Token *num2  = cond2->astOperand2();
        if (!num2->isNumber())
            std::swap(expr2,num2);
        if (!num2->isNumber() || MathLib::isNegative(num2->str()))
            return false;

        if (!isSameExpression(_tokenizer->isCPP(), expr1,expr2,constFunctions))
            return false;

        const MathLib::bigint value1 = MathLib::toLongNumber(num1->str());
        const MathLib::bigint value2 = MathLib::toLongNumber(num2->str());
        return ((value1 & value2) > 0);
    }
    return false;
}


void CheckCondition::multiCondition()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eIf)
            continue;

        const Token * const cond1 = i->classDef->next()->astOperand2();

        const Token * tok2 = i->classDef->next();
        for (;;) {
            tok2 = tok2->link();
            if (!Token::simpleMatch(tok2, ") {"))
                break;
            tok2 = tok2->linkAt(1);
            if (!Token::simpleMatch(tok2, "} else { if ("))
                break;
            tok2 = tok2->tokAt(4);

            if (isOverlappingCond(cond1, tok2->astOperand2(), _settings->library.functionpure))
                multiConditionError(tok2, cond1->linenr());
        }
    }
}

void CheckCondition::multiConditionError(const Token *tok, unsigned int line1)
{
    std::ostringstream errmsg;
    errmsg << "Expression is always false because 'else if' condition matches previous condition at line "
           << line1 << ".";

    reportError(tok, Severity::style, "multiCondition", errmsg.str());
}

//---------------------------------------------------------------------------
// Detect oppositing inner and outer conditions
//---------------------------------------------------------------------------

void CheckCondition::oppositeInnerCondition()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (scope->type != Scope::eIf)
            continue;

        if (!Token::simpleMatch(scope->classDef->linkAt(1), ") {"))
            continue;

        bool nonlocal = false; // nonlocal variable used in condition
        std::set<unsigned int> vars; // variables used in condition
        for (const Token *cond = scope->classDef->linkAt(1); cond != scope->classDef; cond = cond->previous()) {
            if (cond->varId()) {
                vars.insert(cond->varId());
                const Variable *var = cond->variable();
                nonlocal |= (var && (!var->isLocal() || var->isStatic()) && !var->isArgument());
                // TODO: if var is pointer check what it points at
                nonlocal |= (var && (var->isPointer() || var->isReference()));
            } else if (!nonlocal && cond->isName()) {
                // varid is 0. this is possibly a nonlocal variable..
                nonlocal = Token::Match(cond->astParent(), "%cop%|(");
            }
        }

        // parse until inner condition is reached..
        const Token *ifToken = nullptr;
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::simpleMatch(tok, "if (")) {
                ifToken = tok;
                break;
            }
            if (Token::Match(tok, "%type% (") && nonlocal) // function call -> bailout if there are nonlocal variables
                break;
            else if ((tok->varId() && vars.find(tok->varId()) != vars.end()) ||
                     (!tok->varId() && nonlocal)) {
                if (Token::Match(tok, "%name% ++|--|="))
                    break;
                if (Token::Match(tok, "%name% [")) {
                    const Token *tok2 = tok->linkAt(1);
                    while (Token::simpleMatch(tok2, "] ["))
                        tok2 = tok2->linkAt(1);
                    if (Token::simpleMatch(tok2, "] ="))
                        break;
                }
                if (Token::Match(tok->previous(), "++|--|& %name%"))
                    break;
                if (tok->variable() &&
                    !tok->variable()->isConst() &&
                    Token::Match(tok, "%name% . %name% (")) {
                    const Function* function = tok->tokAt(2)->function();
                    if (!function || !function->isConst())
                        break;
                }
                if (Token::Match(tok->previous(), "[(,] %name% [,)]") && isParameterChanged(tok))
                    break;
            }
        }
        if (!ifToken)
            continue;

        // Condition..
        const Token *cond1 = scope->classDef->next()->astOperand2();
        const Token *cond2 = ifToken->next()->astOperand2();

        if (isOppositeCond(false, _tokenizer->isCPP(), cond1, cond2, _settings->library.functionpure))
            oppositeInnerConditionError(scope->classDef, cond2);
    }
}

void CheckCondition::oppositeInnerConditionError(const Token *tok1, const Token* tok2)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok1);
    callstack.push_back(tok2);
    reportError(callstack, Severity::warning, "oppositeInnerCondition", "Opposite conditions in nested 'if' blocks lead to a dead code block.");
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

static std::string invertOperatorForOperandSwap(std::string s)
{
    if (s[0] == '<')
        s[0] = '>';
    else if (s[0] == '>')
        s[0] = '<';
    return s;
}

template <typename T>
static bool checkIntRelation(const std::string &op, const T value1, const T value2)
{
    return (op == "==" && value1 == value2) ||
           (op == "!=" && value1 != value2) ||
           (op == ">"  && value1 >  value2) ||
           (op == ">=" && value1 >= value2) ||
           (op == "<"  && value1 <  value2) ||
           (op == "<=" && value1 <= value2);
}

static bool checkFloatRelation(const std::string &op, const double value1, const double value2)
{
    return (op == ">"  && value1 >  value2) ||
           (op == ">=" && value1 >= value2) ||
           (op == "<"  && value1 <  value2) ||
           (op == "<=" && value1 <= value2);
}

template<class T>
T getvalue3(const T value1, const T value2)
{
    const T min = std::min(value1, value2);
    if (min== std::numeric_limits<T>::max())
        return min;
    else
        return min+1; // see #5895
}

template<>
double getvalue3(const double value1, const double value2)
{
    return (value1 + value2) / 2.0f;
}


template<class T>
static inline T getvalue(const int test, const T value1, const T value2)
{
    // test:
    // 1 => return value that is less than both value1 and value2
    // 2 => return value1
    // 3 => return value that is between value1 and value2
    // 4 => return value2
    // 5 => return value that is larger than both value1 and value2
    switch (test) {
    case 1: {
        const T ret = std::min(value1, value2);
        if ((ret - (T)1) < ret)
            return ret - (T)1;
        else if ((ret / (T)2) < ret)
            return ret / (T)2;
        else if ((ret * (T)2) < ret)
            return ret * (T)2;
        return ret;
    }
    case 2:
        return value1;
    case 3:
        return getvalue3<T>(value1, value2);
    case 4:
        return value2;
    case 5: {
        const T ret = std::max(value1, value2);
        if ((ret + (T)1) > ret)
            return ret + (T)1;
        else if ((ret / (T)2) > ret)
            return ret / (T)2;
        else if ((ret * (T)2) > ret)
            return ret * (T)2;
        return ret;
    }
    };
    return 0;
}

static bool parseComparison(const Token *comp, bool *not1, std::string *op, std::string *value, const Token **expr)
{
    *not1 = false;
    while (comp && comp->str() == "!") {
        *not1 = !(*not1);
        comp = comp->astOperand1();
    }

    if (!comp)
        return false;

    if (!comp->isComparisonOp() || !comp->astOperand1() || !comp->astOperand2()) {
        *op = "!=";
        *value = "0";
        *expr = comp;
    } else if (comp->astOperand1()->isLiteral()) {
        if (comp->astOperand1()->isExpandedMacro())
            return false;
        *op = invertOperatorForOperandSwap(comp->str());
        *value = comp->astOperand1()->str();
        *expr = comp->astOperand2();
    } else if (comp->astOperand2()->isLiteral()) {
        if (comp->astOperand2()->isExpandedMacro())
            return false;
        *op = comp->str();
        *value = comp->astOperand2()->str();
        *expr = comp->astOperand1();
    } else {
        *op = "!=";
        *value = "0";
        *expr = comp;
    }

    // Only float and int values are currently handled
    if (!MathLib::isInt(*value) && !MathLib::isFloat(*value))
        return false;

    return true;
}

static std::string conditionString(bool not1, const Token *expr1, const std::string &op, const std::string &value1)
{
    if (expr1->astParent()->isComparisonOp())
        return std::string(not1 ? "!(" : "") +
               (expr1->isName() ? expr1->str() : std::string("EXPR")) +
               " " +
               op +
               " " +
               value1 +
               (not1 ? ")" : "");

    return std::string(not1 ? "!" : "") +
           (expr1->isName() ? expr1->str() : std::string("EXPR"));
}

void CheckCondition::checkIncorrectLogicOperator()
{
    const bool printStyle = _settings->isEnabled("style");
    const bool printWarning = _settings->isEnabled("warning");
    if (!printWarning && !printStyle)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t ii = 0; ii < functions; ++ii) {
        const Scope * scope = symbolDatabase->functionScopes[ii];

        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%oror%|&&") || !tok->astOperand1() || !tok->astOperand2())
                continue;

            // Opposite comparisons around || or && => always true or always false
            if ((tok->astOperand1()->isName() || tok->astOperand2()->isName()) &&
                isOppositeCond(true, _tokenizer->isCPP(), tok->astOperand1(), tok->astOperand2(), _settings->library.functionpure)) {

                const bool alwaysTrue(tok->str() == "||");
                incorrectLogicOperatorError(tok, tok->expressionString(), alwaysTrue);
                continue;
            }


            // 'A && (!A || B)' is equivalent with 'A && B'
            // 'A || (!A && B)' is equivalent with 'A || B'
            if (printStyle && tok->astOperand1() && tok->astOperand2() &&
                ((tok->str() == "||" && tok->astOperand2()->str() == "&&") ||
                 (tok->str() == "&&" && tok->astOperand2()->str() == "||"))) {
                const Token* tok2 = tok->astOperand2()->astOperand1();
                if (isOppositeCond(true, _tokenizer->isCPP(), tok->astOperand1(), tok2, _settings->library.functionpure)) {
                    std::string expr1(tok->astOperand1()->expressionString());
                    std::string expr2(tok->astOperand2()->astOperand1()->expressionString());
                    std::string expr3(tok->astOperand2()->astOperand2()->expressionString());

                    if (expr1.length() + expr2.length() + expr3.length() > 50U) {
                        if (expr1[0] == '!' && expr2[0] != '!') {
                            expr1 = "!A";
                            expr2 = "A";
                        } else {
                            expr1 = "A";
                            expr2 = "!A";
                        }

                        expr3 = "B";
                    }

                    const std::string cond1 = expr1 + " " + tok->str() + " (" + expr2 + " " + tok->astOperand2()->str() + " " + expr3 + ")";
                    const std::string cond2 = expr1 + " " + tok->str() + " " + expr3;

                    redundantConditionError(tok, tok2->expressionString() + ". '" + cond1 + "' is equivalent to '" + cond2 + "'");
                    continue;
                }
            }

            // Comparison #1 (LHS)
            const Token *comp1 = tok->astOperand1();
            if (comp1 && comp1->str() == tok->str())
                comp1 = comp1->astOperand2();

            // Comparison #2 (RHS)
            const Token *comp2 = tok->astOperand2();

            // Parse LHS
            bool not1;
            std::string op1, value1;
            const Token *expr1;
            if (!parseComparison(comp1, &not1, &op1, &value1, &expr1))
                continue;

            // Parse RHS
            bool not2;
            std::string op2, value2;
            const Token *expr2;
            if (!parseComparison(comp2, &not2, &op2, &value2, &expr2))
                continue;

            if (isSameExpression(_tokenizer->isCPP(), comp1, comp2, _settings->library.functionpure))
                continue; // same expressions => only report that there are same expressions
            if (!isSameExpression(_tokenizer->isCPP(), expr1, expr2, _settings->library.functionpure))
                continue;

            const bool isfloat = astIsFloat(expr1, true) || MathLib::isFloat(value1) || astIsFloat(expr2, true) || MathLib::isFloat(value2);

            // don't check floating point equality comparisons. that is bad
            // and deserves different warnings.
            if (isfloat && (op1 == "==" || op1 == "!=" || op2 == "==" || op2 == "!="))
                continue;

            const double d1 = (isfloat) ? MathLib::toDoubleNumber(value1) : 0;
            const double d2 = (isfloat) ? MathLib::toDoubleNumber(value2) : 0;
            const MathLib::bigint i1 = (isfloat) ? 0 : MathLib::toLongNumber(value1);
            const MathLib::bigint i2 = (isfloat) ? 0 : MathLib::toLongNumber(value2);
            const bool useUnsignedInt = (std::numeric_limits<MathLib::bigint>::max()==i1)||(std::numeric_limits<MathLib::bigint>::max()==i2);
            const MathLib::biguint u1 = (useUnsignedInt) ? MathLib::toLongNumber(value1) : 0;
            const MathLib::biguint u2 = (useUnsignedInt) ? MathLib::toLongNumber(value2) : 0;
            // evaluate if expression is always true/false
            bool alwaysTrue = true, alwaysFalse = true;
            bool firstTrue = true, secondTrue = true;
            for (int test = 1; test <= 5; ++test) {
                // test:
                // 1 => testvalue is less than both value1 and value2
                // 2 => testvalue is value1
                // 3 => testvalue is between value1 and value2
                // 4 => testvalue value2
                // 5 => testvalue is larger than both value1 and value2
                bool result1, result2;
                if (isfloat) {
                    const double testvalue = getvalue<double>(test, d1, d2);
                    result1 = checkFloatRelation(op1, testvalue, d1);
                    result2 = checkFloatRelation(op2, testvalue, d2);
                } else if (useUnsignedInt) {
                    const MathLib::biguint testvalue = getvalue<MathLib::biguint>(test, u1, u2);
                    result1 = checkIntRelation(op1, testvalue, u1);
                    result2 = checkIntRelation(op2, testvalue, u2);
                } else {
                    const MathLib::bigint testvalue = getvalue<MathLib::bigint>(test, i1, i2);
                    result1 = checkIntRelation(op1, testvalue, i1);
                    result2 = checkIntRelation(op2, testvalue, i2);
                }
                if (not1)
                    result1 = !result1;
                if (not2)
                    result2 = !result2;
                if (tok->str() == "&&") {
                    alwaysTrue &= (result1 && result2);
                    alwaysFalse &= !(result1 && result2);
                } else {
                    alwaysTrue &= (result1 || result2);
                    alwaysFalse &= !(result1 || result2);
                }
                firstTrue &= !(!result1 && result2);
                secondTrue &= !(result1 && !result2);
            }

            const std::string cond1str = conditionString(not1, expr1, op1, value1);
            const std::string cond2str = conditionString(not2, expr2, op2, value2);
            if (printWarning && (alwaysTrue || alwaysFalse)) {
                const std::string text = cond1str + " " + tok->str() + " " + cond2str;
                incorrectLogicOperatorError(tok, text, alwaysTrue);
            } else if (printStyle && secondTrue) {
                const std::string text = "If '" + cond1str + "', the comparison '" + cond2str +
                                         "' is always " + (secondTrue ? "true" : "false") + ".";
                redundantConditionError(tok, text);
            } else if (printStyle && firstTrue) {
                //const std::string text = "The comparison " + cond1str + " is always " +
                //                         (firstTrue ? "true" : "false") + " when " +
                //                         cond2str + ".";
                const std::string text = "If '" + cond2str + "', the comparison '" + cond1str +
                                         "' is always " + (firstTrue ? "true" : "false") + ".";
                redundantConditionError(tok, text);
            }
        }
    }
}

void CheckCondition::incorrectLogicOperatorError(const Token *tok, const std::string &condition, bool always)
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

void CheckCondition::redundantConditionError(const Token *tok, const std::string &text)
{
    reportError(tok, Severity::style, "redundantCondition", "Redundant condition: " + text);
}

//-----------------------------------------------------------------------------
// Detect "(var % val1) > val2" where val2 is >= val1.
//-----------------------------------------------------------------------------
void CheckCondition::checkModuloAlwaysTrueFalse()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isComparisonOp())
                continue;
            const Token *num, *modulo;
            if (Token::simpleMatch(tok->astOperand1(), "%") && Token::Match(tok->astOperand2(), "%num%")) {
                modulo = tok->astOperand1();
                num = tok->astOperand2();
            } else if (Token::Match(tok->astOperand1(), "%num%") && Token::simpleMatch(tok->astOperand2(), "%")) {
                num = tok->astOperand1();
                modulo = tok->astOperand2();
            } else {
                continue;
            }

            if (Token::Match(modulo->astOperand2(), "%num%") &&
                MathLib::isLessEqual(modulo->astOperand2()->str(), num->str()))
                moduloAlwaysTrueFalseError(tok, modulo->astOperand2()->str());
        }
    }
}

void CheckCondition::moduloAlwaysTrueFalseError(const Token* tok, const std::string& maxVal)
{
    reportError(tok, Severity::warning, "moduloAlwaysTrueFalse",
                "Comparison of modulo result is predetermined, because it is always less than " + maxVal + ".");
}

//---------------------------------------------------------------------------
// Clarify condition '(x = a < 0)' into '((x = a) < 0)' or '(x = (a < 0))'
// Clarify condition '(a & b == c)' into '((a & b) == c)' or '(a & (b == c))'
//---------------------------------------------------------------------------
void CheckCondition::clarifyCondition()
{
    if (!_settings->isEnabled("style"))
        return;

    const bool isC = _tokenizer->isC();

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "( %name% [=&|^]")) {
                for (const Token *tok2 = tok->tokAt(3); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(" || tok2->str() == "[")
                        tok2 = tok2->link();
                    else if (tok2->tokType() == Token::eComparisonOp) {
                        // This might be a template
                        if (!isC && tok2->link())
                            break;
                        if (Token::simpleMatch(tok2->astParent(), "?"))
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

                while (tok2 && (tok2->isName() || tok2->isNumber() || Token::Match(tok2, ".|(|["))) {
                    if (Token::Match(tok2, "(|["))
                        tok2 = tok2->link();
                    tok2 = tok2->next();
                }

                if (Token::Match(tok2, "[&|^]")) {
                    // don't write false positives when templates are used
                    if (Token::Match(tok2, "&|* ,|>") || Token::simpleMatch(tok2->previous(), "const &"))
                        continue;

                    // #3609 - CWinTraits<WS_CHILD|WS_VISIBLE>::..
                    if (!isC && Token::Match(tok->previous(), "%name% <")) {
                        const Token *tok3 = tok2;
                        while (Token::Match(tok3, "[&|^] %name%"))
                            tok3 = tok3->tokAt(2);
                        if (Token::Match(tok3, ",|>"))
                            continue;
                    }

                    clarifyConditionError(tok, false, true);
                }
            }
        }
    }
}

void CheckCondition::clarifyConditionError(const Token *tok, bool assign, bool boolop)
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


void CheckCondition::alwaysTrueFalse()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();

    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%comp%|!"))
                continue;
            if (tok->link()) // don't write false positives when templates are used
                continue;
            if (tok->values.size() != 1U)
                continue;
            if (!tok->values.front().isKnown())
                continue;
            if (!tok->astParent() || !Token::Match(tok->astParent()->previous(), "%name% ("))
                continue;

            // Don't warn when there are expanded macros..
            bool isExpandedMacro = false;
            std::stack<const Token*> tokens;
            tokens.push(tok);
            while (!tokens.empty()) {
                const Token *tok2 = tokens.top();
                tokens.pop();
                if (!tok2)
                    continue;
                tokens.push(tok2->astOperand1());
                tokens.push(tok2->astOperand2());
                if (tok2->isExpandedMacro()) {
                    isExpandedMacro = true;
                    break;
                }
            }
            if (isExpandedMacro)
                continue;

            alwaysTrueFalseError(tok, tok->values.front().intvalue != 0);
        }
    }
}

void CheckCondition::alwaysTrueFalseError(const Token *tok, bool knownResult)
{
    const std::string expr = tok ? tok->expressionString() : std::string("x");

    reportError(tok,
                Severity::style,
                "knownConditionTrueFalse",
                "Condition '" + expr + "' is always " + (knownResult ? "true" : "false"));
}
