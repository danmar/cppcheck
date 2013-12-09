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
// Check for assignment / condition mismatches
//---------------------------------------------------------------------------

#include "checkassignif.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckAssignIf instance;
}


void CheckAssignIf::assignIf()
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
                if (Token::Match(endToken->tokAt(-4), "* ) & %any% ;"))
                    endToken = NULL;

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
bool CheckAssignIf::assignIfParseScope(const Token * const assignTok,
                                       const Token * const startTok,
                                       const unsigned int varid,
                                       const bool islocal,
                                       const char bitop,
                                       const MathLib::bigint num)
{
    bool ret = false;

    for (const Token *tok2 = startTok; tok2; tok2 = tok2->next()) {
        if (Token::Match(tok2->tokAt(2), "%varid% %cop% %num% ;", varid) && tok2->strAt(3) == std::string(1U, bitop)) {
            const MathLib::bigint num2 = MathLib::toLongNumber(tok2->strAt(4));
            if ((bitop == '&') && (0 == (num & num2)))
                mismatchingBitAndError(assignTok, num, tok2, num2);
        }
        if (Token::Match(tok2, "%varid% =", varid)) {
            if (Token::Match(tok2->tokAt(2), "%varid% %cop% %num% ;", varid) && tok2->strAt(3) == std::string(1U, bitop)) {
                const MathLib::bigint num2 = MathLib::toLongNumber(tok2->strAt(4));
                if ((bitop == '&') && (0 == (num & num2)))
                    mismatchingBitAndError(assignTok, num, tok2, num2);
            }
            return true;
        }
        if (Token::Match(tok2, "[(,] &| %varid% [,)]", varid)) {
            unsigned int argumentNumber = 0;
            const Token *ftok;
            for (ftok = tok2; ftok && ftok->str() != "("; ftok = ftok->previous()) {
                if (ftok->str() == ")")
                    ftok = ftok->link();
                else if (ftok->str() == ",")
                    argumentNumber++;
            }
            ftok = ftok ? ftok->previous() : NULL;
            if (!(ftok && ftok->function()))
                return true;
            const Variable *par = ftok->function()->getArgumentVar(argumentNumber);
            if (par == NULL || par->isReference() || par->isPointer())
                return true;
        }
        if (tok2->str() == "}")
            return false;
        if (Token::Match(tok2, "break|continue|return"))
            ret = true;
        if (ret && tok2->str() == ";")
            return false;
        if (!islocal && Token::Match(tok2, "%var% (") && !Token::simpleMatch(tok2->next()->link(), ") {"))
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

void CheckAssignIf::assignIfError(const Token *tok1, const Token *tok2, const std::string &condition, bool result)
{
    std::list<const Token *> locations;
    locations.push_back(tok1);
    locations.push_back(tok2);

    reportError(locations,
                Severity::style,
                "assignIfError",
                "Mismatching assignment and comparison, comparison '" + condition + "' is always " + std::string(result ? "true" : "false") + ".");
}


void CheckAssignIf::mismatchingBitAndError(const Token *tok1, const MathLib::bigint num1, const Token *tok2, const MathLib::bigint num2)
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

void CheckAssignIf::comparison()
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

void CheckAssignIf::comparisonError(const Token *tok, const std::string &bitop, MathLib::bigint value1, const std::string &op, MathLib::bigint value2, bool result)
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






void CheckAssignIf::multiCondition()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type == Scope::eIf && Token::Match(i->classDef, "if ( %var% & %num% ) {")) {
            const Token* const tok = i->classDef;
            const unsigned int varid(tok->tokAt(2)->varId());
            if (varid == 0)
                continue;

            const MathLib::bigint num1 = MathLib::toLongNumber(tok->strAt(4));
            if (num1 < 0)
                continue;

            const Token *tok2 = tok->linkAt(6);
            while (Token::simpleMatch(tok2, "} else { if (")) {
                // Goto '('
                const Token * const opar = tok2->tokAt(4);

                // tok2: skip if-block
                tok2 = opar->link();
                if (Token::simpleMatch(tok2, ") {"))
                    tok2 = tok2->next()->link();

                // check condition..
                if (Token::Match(opar, "( %varid% ==|& %num% &&|%oror%|)", varid)) {
                    const MathLib::bigint num2 = MathLib::toLongNumber(opar->strAt(3));
                    if (num2 < 0)
                        continue;

                    if ((num1 & num2) == num2) {
                        multiConditionError(opar, tok->linenr());
                    }
                }
            }
        }
    }
}

void CheckAssignIf::multiConditionError(const Token *tok, unsigned int line1)
{
    std::ostringstream errmsg;
    errmsg << "Expression is always false because 'else if' condition matches previous condition at line "
           << line1 << ".";

    reportError(tok, Severity::style, "multiCondition", errmsg.str());
}
