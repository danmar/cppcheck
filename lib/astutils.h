/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#include <set>
#include <string>
#include <vector>

#include "errortypes.h"
#include "utils.h"

class Library;
class Scope;
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
void visitAstNodes(Token *ast, std::function<ChildrenToVisit(Token *)> visitor);

const Token* findAstNode(const Token* ast, const std::function<bool(const Token*)>& pred);

std::vector<const Token*> astFlatten(const Token* tok, const char* op);

bool astHasToken(const Token* root, const Token * tok);

bool astHasVar(const Token * tok, nonneg int varid);

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

bool astIsSmartPointer(const Token* tok);

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

bool isTemporary(bool cpp, const Token* tok, const Library* library, bool unknown = false);

const Token* previousBeforeAstLeftmostLeaf(const Token* tok);
Token* previousBeforeAstLeftmostLeaf(Token* tok);

const Token * nextAfterAstRightmostLeaf(const Token * tok);
Token* nextAfterAstRightmostLeaf(Token* tok);

Token* astParentSkipParens(Token* tok);
const Token* astParentSkipParens(const Token* tok);

const Token* getParentMember(const Token * tok);

const Token* getParentLifetime(const Token* tok);

bool astIsLHS(const Token* tok);
bool astIsRHS(const Token* tok);

Token* getCondTok(Token* tok);
const Token* getCondTok(const Token* tok);

Token* getCondTokFromEnd(Token* endBlock);
const Token* getCondTokFromEnd(const Token* endBlock);

/// For a "break" token, locate the next token to execute. The token will
/// be either a "}" or a ";".
const Token *findNextTokenFromBreak(const Token *breakToken);

/**
 * Extract for loop values: loopvar varid, init value, step value, last value (inclusive)
 */
bool extractForLoopValues(const Token *forToken,
                          nonneg int * const varid,
                          bool * const knownInitValue,
                          long long * const initValue,
                          bool * const partialCond,
                          long long * const stepValue,
                          long long * const lastValue);

bool precedes(const Token * tok1, const Token * tok2);

bool exprDependsOnThis(const Token* expr, nonneg int depth = 0);

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
 * @param pure boolean
 */
bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isOppositeExpression(bool cpp, const Token * const tok1, const Token * const tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isConstExpression(const Token *tok, const Library& library, bool pure, bool cpp);

bool isWithoutSideEffects(bool cpp, const Token* tok);

bool isUniqueExpression(const Token* tok);

bool isEscapeFunction(const Token* ftok, const Library* library);

/** Is scope a return scope (scope will unconditionally return) */
bool isReturnScope(const Token* const endToken,
                   const Library* library = nullptr,
                   const Token** unknownFunc = nullptr,
                   bool functionScope = false);

/// Return the token to the function and the argument number
const Token * getTokenArgumentFunction(const Token * tok, int& argn);

/** Is variable changed by function call?
 * In case the answer of the question is inconclusive, e.g. because the function declaration is not known
 * the return value is false and the output parameter inconclusive is set to true
 *
 * @param tok           ast tree
 * @param varid         Variable Id
 * @param settings      program settings
 * @param inconclusive  pointer to output variable which indicates that the answer of the question is inconclusive
 */
bool isVariableChangedByFunctionCall(const Token *tok, int indirect, nonneg int varid, const Settings *settings, bool *inconclusive);

/** Is variable changed by function call?
 * In case the answer of the question is inconclusive, e.g. because the function declaration is not known
 * the return value is false and the output parameter inconclusive is set to true
 *
 * @param tok           token of variable in function call
 * @param settings      program settings
 * @param inconclusive pointer to output variable which indicates that the answer of the question is inconclusive
 */
bool isVariableChangedByFunctionCall(const Token *tok, int indirect, const Settings *settings, bool *inconclusive);

/** Is variable changed in block of code? */
bool isVariableChanged(const Token *start, const Token *end, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);
bool isVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);

bool isVariableChanged(const Token *tok, int indirect, const Settings *settings, bool cpp, int depth = 20);

bool isVariableChanged(const Variable * var, const Settings *settings, bool cpp, int depth = 20);

bool isVariablesChanged(const Token* start,
                        const Token* end,
                        int indirect,
                        std::vector<const Variable*> vars,
                        const Settings* settings,
                        bool cpp);

bool isThisChanged(const Token* start, const Token* end, int indirect, const Settings* settings, bool cpp);

const Token* findVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);
Token* findVariableChanged(Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);

bool isExpressionChanged(const Token* expr,
                         const Token* start,
                         const Token* end,
                         const Settings* settings,
                         bool cpp,
                         int depth = 20);

/// If token is an alias if another variable
bool isAliasOf(const Token *tok, nonneg int varid, bool* inconclusive = nullptr);

bool isAliased(const Variable *var);

/** Determines the number of arguments - if token is a function call or macro
 * @param start token which is supposed to be the function/macro name.
 * \return Number of arguments
 */
int numberOfArguments(const Token *start);

/**
 * Get arguments (AST)
 */
std::vector<const Token *> getArguments(const Token *ftok);

const Token *findLambdaStartToken(const Token *last);

/**
 * find lambda function end token
 * \param first The [ token
 * \return nullptr or the }
 */
const Token *findLambdaEndToken(const Token *first);
Token* findLambdaEndToken(Token* first);

bool isLikelyStream(bool cpp, const Token *stream);

/**
 * do we see a likely write of rhs through overloaded operator
 *   s >> x;
 *   a & x;
 */
bool isLikelyStreamRead(bool cpp, const Token *op);

bool isCPPCast(const Token* tok);

bool isConstVarExpression(const Token *tok, const char * skipMatch = nullptr);

const Variable *getLHSVariable(const Token *tok);

std::vector<const Variable*> getLHSVariables(const Token* tok);

bool isScopeBracket(const Token* tok);

bool isNullOperand(const Token *expr);

bool isGlobalData(const Token *expr, bool cpp);
/**
 * Forward data flow analysis for checks
 *  - unused value
 *  - redundant assignment
 *  - valueflow analysis
 */
class FwdAnalysis {
public:
    FwdAnalysis(bool cpp, const Library &library) : mCpp(cpp), mLibrary(library), mWhat(What::Reassign), mValueFlowKnown(true) {}

    bool hasOperand(const Token *tok, const Token *lhs) const;

    /**
     * Check if "expr" is reassigned. The "expr" can be a tree (x.y[12]).
     * @param expr Symbolic expression to perform forward analysis for
     * @param startToken First token in forward analysis
     * @param endToken Last token in forward analysis
     * @return Token where expr is reassigned. If it's not reassigned then nullptr is returned.
     */
    const Token *reassign(const Token *expr, const Token *startToken, const Token *endToken);

    /**
     * Check if "expr" is used. The "expr" can be a tree (x.y[12]).
     * @param expr Symbolic expression to perform forward analysis for
     * @param startToken First token in forward analysis
     * @param endToken Last token in forward analysis
     * @return true if expr is used.
     */
    bool unusedValue(const Token *expr, const Token *startToken, const Token *endToken);

    struct KnownAndToken {
        bool known;
        const Token *token;
    };

    std::vector<KnownAndToken> valueFlow(const Token *expr, const Token *startToken, const Token *endToken);

    /** Is there some possible alias for given expression */
    bool possiblyAliased(const Token *expr, const Token *startToken) const;

    std::set<int> getExprVarIds(const Token* expr, bool* localOut = nullptr, bool* unknownVarIdOut = nullptr) const;
private:
    static bool isEscapedAlias(const Token* expr);

    /** Result of forward analysis */
    struct Result {
        enum class Type { NONE, READ, WRITE, BREAK, RETURN, BAILOUT } type;
        explicit Result(Type type) : type(type), token(nullptr) {}
        Result(Type type, const Token *token) : type(type), token(token) {}
        const Token *token;
    };

    struct Result check(const Token *expr, const Token *startToken, const Token *endToken);
    struct Result checkRecursive(const Token *expr, const Token *startToken, const Token *endToken, const std::set<int> &exprVarIds, bool local, bool inInnerClass, int depth=0);

    // Is expression a l-value global data?
    bool isGlobalData(const Token *expr) const;

    const bool mCpp;
    const Library &mLibrary;
    enum class What { Reassign, UnusedValue, ValueFlow } mWhat;
    std::vector<KnownAndToken> mValueFlow;
    bool mValueFlowKnown;
};

#endif // astutilsH
