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
#include "checkother.h"
#include "symboldatabase.h"

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
        if (Token::Match(tok2, "[(,] &| %varid% [,)]", varid)) {
            unsigned int argumentNumber = 0;
            const Token *ftok;
            for (ftok = tok2; ftok && ftok->str() != "("; ftok = ftok->previous()) {
                if (ftok->str() == ")")
                    ftok = ftok->link();
                else if (ftok->str() == ",")
                    argumentNumber++;
            }
            ftok = ftok ? ftok->previous() : nullptr;
            if (!(ftok && ftok->function()))
                return true;
            const Variable *par = ftok->function()->getArgumentVar(argumentNumber);
            if (par == nullptr || par->isReference() || par->isPointer())
                return true;
        }
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
                                   (parent->str() == "(" && Token::Match(parent->astOperand1(), "if|while"));

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

static bool isOverlappingCond(const Token * const cond1, const Token * const cond2, const std::set<std::string> &constFunctions)
{
    if (!cond1 || !cond2)
        return false;

    // same expressions
    if (isSameExpression(cond1,cond2,constFunctions))
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

        if (!isSameExpression(expr1,expr2,constFunctions))
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

static bool isOppositeCond(const Token * const cond1, const Token * const cond2, const std::set<std::string> &constFunctions)
{
    if (!cond1 || !cond2)
        return false;

    if (cond1->str() == "!")
        return isSameExpression(cond1->astOperand1(), cond2, constFunctions);

    if (cond2->str() == "!")
        return isSameExpression(cond1, cond2->astOperand1(), constFunctions);

    if (!cond1->isComparisonOp() || !cond2->isComparisonOp())
        return false;

    const std::string &comp1 = cond1->str();

    // condition found .. get comparator
    std::string comp2;
    if (isSameExpression(cond1->astOperand1(), cond2->astOperand1(), constFunctions) &&
        isSameExpression(cond1->astOperand2(), cond2->astOperand2(), constFunctions)) {
        comp2 = cond2->str();
    } else if (isSameExpression(cond1->astOperand1(), cond2->astOperand2(), constFunctions) &&
               isSameExpression(cond1->astOperand2(), cond2->astOperand1(), constFunctions)) {
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
            (comp1 == "<"  && comp2 == ">") ||
            (comp1 == "<=" && comp2 == ">") ||
            (comp1 == ">"  && comp2 == "<=") ||
            (comp1 == ">"  && comp2 == "<") ||
            (comp1 == ">=" && comp2 == "<"));
}

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
        for (const Token *cond = scope->classDef; cond != scope->classDef->linkAt(1); cond = cond->next()) {
            if (cond->varId()) {
                vars.insert(cond->varId());
                const Variable *var = cond->variable();
                nonlocal |= (var && (!var->isLocal() || var->isStatic()) && !var->isArgument());
                // TODO: if var is pointer check what it points at
                nonlocal |= (var && (var->isPointer() || var->isReference()));
            } else if (cond->isName()) {
                // varid is 0. this is possibly a nonlocal variable..
                nonlocal |= (cond->astParent() && cond->astParent()->isConstOp());
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
                    Token::Match(tok, "%name% . %name% (") &&
                    !tok->variable()->isConst()) {
                    const Function* function = tok->tokAt(2)->function();
                    if (!function || !function->isConst())
                        break;
                }
                if (Token::Match(tok->previous(), "[(,] %name% [,)]")) {
                    // is variable unchanged? default is false..
                    bool unchanged = false;

                    // locate start parentheses in function call..
                    unsigned int argumentNumber = 0;
                    const Token *start = tok->previous();
                    while (start && start->str() == ",") {
                        start = start->astParent();
                        ++argumentNumber;
                    }

                    start = start ? start->previous() : nullptr;
                    if (start && start->function()) {
                        const Variable *arg = start->function()->getArgumentVar(argumentNumber);
                        if (arg && !arg->isPointer() && !arg->isReference())
                            unchanged = true;
                    }

                    if (!unchanged)
                        break;
                }
            }
        }
        if (!ifToken)
            continue;

        // Condition..
        const Token *cond1 = scope->classDef->next()->astOperand2();
        const Token *cond2 = ifToken->next()->astOperand2();

        if (isOppositeCond(cond1, cond2, _settings->library.functionpure))
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

static bool checkIntRelation(const std::string &op, const MathLib::bigint value1, const MathLib::bigint value2)
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

template<class T> static T getvalue(const int test, const T value1, const T value2)
{
    // test:
    // 1 => return value that is less than both value1 and value2
    // 2 => return value1
    // 3 => return value that is between value1 and value2
    // 4 => return value2
    // 5 => return value that is larger than both value1 and value2
    switch (test) {
    case 1: {
        T ret = std::min(value1, value2);
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
        return (value1 + value2) / (T)2;
    case 4:
        return value2;
    case 5: {
        T ret = std::max(value1, value2);
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
            // Opposite comparisons
            if (Token::Match(tok, "%oror%|&&") &&
                tok->astOperand1() &&
                tok->astOperand2() &&
                (tok->astOperand1()->isName() || tok->astOperand2()->isName()) &&
                isOppositeCond(tok->astOperand1(), tok->astOperand2(), _settings->library.functionpure)) {

                const bool alwaysTrue(tok->str() == "||");
                incorrectLogicOperatorError(tok, tok->expressionString(), alwaysTrue);
            }

            else if (Token::Match(tok, "&&|%oror%")) {
                // Comparison #1 (LHS)
                const Token *comp1 = tok->astOperand1();
                if (comp1 && comp1->str() == tok->str())
                    comp1 = comp1->astOperand2();

                // Comparison #2 (RHS)
                const Token *comp2 = tok->astOperand2();

                if (!comp1 || !comp1->isComparisonOp() || !comp1->astOperand1() || !comp1->astOperand2())
                    continue;
                if (!comp2 || !comp2->isComparisonOp() || !comp2->astOperand1() || !comp2->astOperand2())
                    continue;

                std::string op1, value1;
                const Token *expr1;
                if (comp1->astOperand1()->isLiteral()) {
                    op1 = invertOperatorForOperandSwap(comp1->str());
                    value1 = comp1->astOperand1()->str();
                    expr1 = comp1->astOperand2();
                } else if (comp1->astOperand2()->isLiteral()) {
                    op1 = comp1->str();
                    value1 = comp1->astOperand2()->str();
                    expr1 = comp1->astOperand1();
                } else {
                    continue;
                }

                std::string op2, value2;
                const Token *expr2;
                if (comp2->astOperand1()->isLiteral()) {
                    op2 = invertOperatorForOperandSwap(comp2->str());
                    value2 = comp2->astOperand1()->str();
                    expr2 = comp2->astOperand2();
                } else if (comp2->astOperand2()->isLiteral()) {
                    op2 = comp2->str();
                    value2 = comp2->astOperand2()->str();
                    expr2 = comp2->astOperand1();
                } else {
                    continue;
                }

                // Only float and int values are currently handled
                if (!MathLib::isInt(value1) && !MathLib::isFloat(value1))
                    continue;
                if (!MathLib::isInt(value2) && !MathLib::isFloat(value2))
                    continue;

                if (isSameExpression(comp1, comp2, _settings->library.functionpure))
                    continue; // same expressions => only report that there are same expressions
                if (!isSameExpression(expr1, expr2, _settings->library.functionpure))
                    continue;

                const bool isfloat = astIsFloat(expr1, true) || MathLib::isFloat(value1) || astIsFloat(expr2, true) || MathLib::isFloat(value2);

                // don't check floating point equality comparisons. that is bad
                // and deserves different warnings.
                if (isfloat && (op1 == "==" || op1 == "!=" || op2 == "==" || op2 == "!="))
                    continue;

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
                        const double d1 = MathLib::toDoubleNumber(value1);
                        const double d2 = MathLib::toDoubleNumber(value2);
                        const double testvalue = getvalue<double>(test, d1, d2);
                        result1 = checkFloatRelation(op1, testvalue, d1);
                        result2 = checkFloatRelation(op2, testvalue, d2);
                    } else {
                        const MathLib::bigint i1 = MathLib::toLongNumber(value1);
                        const MathLib::bigint i2 = MathLib::toLongNumber(value2);
                        const MathLib::bigint testvalue = getvalue<MathLib::bigint>(test, i1, i2);
                        result1 = checkIntRelation(op1, testvalue, i1);
                        result2 = checkIntRelation(op2, testvalue, i2);
                    }
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

                const std::string cond1str = (expr1->isName() ? expr1->str() : "EXPR") + " " + op1 + " " + value1;
                const std::string cond2str = (expr2->isName() ? expr2->str() : "EXPR") + " " + op2 + " " + value2;
                if (printWarning && (alwaysTrue || alwaysFalse)) {
                    const std::string text = cond1str + " " + tok->str() + " " + cond2str;
                    incorrectLogicOperatorError(tok, text, alwaysTrue);
                } else if (printStyle && secondTrue) {
                    const std::string text = "If " + cond1str + ", the comparison " + cond2str +
                                             " is always " + (secondTrue ? "true" : "false") + ".";
                    redundantConditionError(tok, text);
                } else if (printStyle && firstTrue) {
                    //const std::string text = "The comparison " + cond1str + " is always " +
                    //                         (firstTrue ? "true" : "false") + " when " +
                    //                         cond2str + ".";
                    const std::string text = "If " + cond2str + ", the comparison " + cond1str +
                                             " is always " + (firstTrue ? "true" : "false") + ".";
                    redundantConditionError(tok, text);
                }
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
                    else if (tok2->type() == Token::eComparisonOp) {
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
