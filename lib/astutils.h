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
#ifndef astutilsH
#define astutilsH
//---------------------------------------------------------------------------

#include <set>
#include <string>

class Token;

/** Is expression a 'signed char' if no promotion is used */
bool astIsSignedChar(const Token *tok);
/** Is expression of integral type? */
bool astIsIntegral(const Token *tok, bool unknown);
/** Is expression of floating point type? */
bool astIsFloat(const Token *tok, bool unknown);

/**
 * Get canonical type of expression. const/static/etc are not included and neither *&.
 * For example:
 * Expression type      Return
 * std::string          std::string
 * int *                int
 * static const int     int
 * std::vector<T>       std::vector
 */
std::string astCanonicalType(const Token *expr);

/** Is given syntax tree a variable comparison against value */
const Token * astIsVariableComparison(const Token *tok, const std::string &comp, const std::string &rhs, const Token **vartok=nullptr);

bool isSameExpression(bool cpp, const Token *tok1, const Token *tok2, const std::set<std::string> &constFunctions);

/**
 * Are two conditions opposite
 * @param isNot  do you want to know if cond1 is !cond2 or if cond1 and cond2 are non-overlapping. true: cond1==!cond2  false: cond1==true => cond2==false
 * @param cpp    c++ file
 * @param cond1  condition1
 * @param cond2  condition2
 * @param constFunctions  constFunctions
 */
bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const std::set<std::string> &constFunctions);

bool isConstExpression(const Token *tok, const std::set<std::string> &constFunctions);

bool isWithoutSideEffects(bool cpp, const Token* tok);

/** Is variable changed in block of code? */
bool isVariableChanged(const Token *start, const Token *end, const unsigned int varid);

#endif // astutilsH
