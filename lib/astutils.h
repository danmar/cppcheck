/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include <functional>
#include <string>
#include <vector>

#include "errorlogger.h"

class Library;
class Settings;
class Token;
class Variable;

enum class ChildrenToVisit {
    none,
    op1,
    op2,
    op1_and_op2,
    done  // found what we looked for, don't visit any more children
};

/**
 * Visit AST nodes recursively. The order is not "well defined"
 */
void visitAstNodes(const Token *ast, std::function<ChildrenToVisit(const Token *)> visitor);

/** Is expression a 'signed char' if no promotion is used */
bool astIsSignedChar(const Token *tok);
/** Is expression a 'char' if no promotion is used? */
bool astIsUnknownSignChar(const Token *tok);
/** Is expression of integral type? */
bool astIsIntegral(const Token *tok, bool unknown);
/** Is expression of floating point type? */
bool astIsFloat(const Token *tok, bool unknown);
/** Is expression of boolean type? */
bool astIsBool(const Token *tok);

bool astIsPointer(const Token *tok);

bool astIsIterator(const Token *tok);

bool astIsContainer(const Token *tok);

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

const Token * nextAfterAstRightmostLeaf(const Token * tok);

bool precedes(const Token * tok1, const Token * tok2);

bool isSameExpression(bool cpp, bool macro, const Token *tok1, const Token *tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isEqualKnownValue(const Token * const tok1, const Token * const tok2);

bool isDifferentKnownValues(const Token * const tok1, const Token * const tok2);

/**
 * Are two conditions opposite
 * @param isNot  do you want to know if cond1 is !cond2 or if cond1 and cond2 are non-overlapping. true: cond1==!cond2  false: cond1==true => cond2==false
 * @param cpp    c++ file
 * @param cond1  condition1
 * @param cond2  condition2
 * @param library files data
 * @param pure
 */
bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isOppositeExpression(bool cpp, const Token * const tok1, const Token * const tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isConstExpression(const Token *tok, const Library& library, bool pure, bool cpp);

bool isWithoutSideEffects(bool cpp, const Token* tok);

bool isUniqueExpression(const Token* tok);

/** Is scope a return scope (scope will unconditionally return) */
bool isReturnScope(const Token *endToken);

/** Is variable changed by function call?
 * In case the answer of the question is inconclusive, e.g. because the function declaration is not known
 * the return value is false and the output parameter inconclusive is set to true
 *
 * @param tok           ast tree
 * @param varid         Variable Id
 * @param settings      program settings
 * @param inconclusive  pointer to output variable which indicates that the answer of the question is inconclusive
 */
bool isVariableChangedByFunctionCall(const Token *tok, unsigned int varid, const Settings *settings, bool *inconclusive);

/** Is variable changed by function call?
 * In case the answer of the question is inconclusive, e.g. because the function declaration is not known
 * the return value is false and the output parameter inconclusive is set to true
 *
 * @param tok           token of variable in function call
 * @param settings      program settings
 * @param inconclusive pointer to output variable which indicates that the answer of the question is inconclusive
 */
bool isVariableChangedByFunctionCall(const Token *tok, const Settings *settings, bool *inconclusive);

/** Is variable changed in block of code? */
bool isVariableChanged(const Token *start, const Token *end, const unsigned int varid, bool globalvar, const Settings *settings, bool cpp);

bool isVariableChanged(const Variable * var, const Settings *settings, bool cpp);

/** Determines the number of arguments - if token is a function call or macro
 * @param start token which is supposed to be the function/macro name.
 * \return Number of arguments
 */
int numberOfArguments(const Token *start);

/**
 * Get arguments (AST)
 */
std::vector<const Token *> getArguments(const Token *ftok);

/**
 * find lambda function end token
 * \param first The [ token
 * \return nullptr or the }
 */
const Token *findLambdaEndToken(const Token *first);

/**
 * do we see a likely write of rhs through overloaded operator
 *   s >> x;
 *   a & x;
 */
bool isLikelyStreamRead(bool cpp, const Token *op);

#endif // astutilsH
