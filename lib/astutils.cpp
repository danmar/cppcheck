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
