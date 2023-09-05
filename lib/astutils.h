/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
#include <stack>
#include <string>
#include <type_traits>
#include <vector>

#include "config.h"
#include "errortypes.h"
#include "library.h"
#include "smallvector.h"
#include "symboldatabase.h"
#include "token.h"

class Settings;

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
template<class T, class TFunc, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
void visitAstNodes(T *ast, const TFunc &visitor)
{
    if (!ast)
        return;

    // the size of 8 was determined in tests to be sufficient to avoid excess allocations. also add 1 as a buffer.
    // we might need to increase that value in the future.
    std::stack<T *, SmallVector<T *, 8 + 1>> tokens;
    T *tok = ast;
    do {
        const ChildrenToVisit c = visitor(tok);

        if (c == ChildrenToVisit::done)
            break;
        if (c == ChildrenToVisit::op2 || c == ChildrenToVisit::op1_and_op2) {
            T *t2 = tok->astOperand2();
            if (t2)
                tokens.push(t2);
        }
        if (c == ChildrenToVisit::op1 || c == ChildrenToVisit::op1_and_op2) {
            T *t1 = tok->astOperand1();
            if (t1)
                tokens.push(t1);
        }

        if (tokens.empty())
            break;

        tok = tokens.top();
        tokens.pop();
    } while (true);
}

template<class TFunc>
const Token* findAstNode(const Token* ast, const TFunc& pred)
{
    const Token* result = nullptr;
    visitAstNodes(ast, [&](const Token* tok) {
        if (pred(tok)) {
            result = tok;
            return ChildrenToVisit::done;
        }
        return ChildrenToVisit::op1_and_op2;
    });
    return result;
}

template<class TFunc>
const Token* findParent(const Token* tok, const TFunc& pred)
{
    if (!tok)
        return nullptr;
    const Token* parent = tok->astParent();
    while (parent && !pred(parent)) {
        parent = parent->astParent();
    }
    return parent;
}

const Token* findExpression(const nonneg int exprid,
                            const Token* start,
                            const Token* end,
                            const std::function<bool(const Token*)>& pred);
const Token* findExpression(const Token* start, const nonneg int exprid);

std::vector<const Token*> astFlatten(const Token* tok, const char* op);
std::vector<Token*> astFlatten(Token* tok, const char* op);

nonneg int astCount(const Token* tok, const char* op, int depth = 100);

bool astHasToken(const Token* root, const Token * tok);

bool astHasVar(const Token * tok, nonneg int varid);

bool astIsPrimitive(const Token* tok);
/** Is expression a 'signed char' if no promotion is used */
bool astIsSignedChar(const Token *tok);
/** Is expression a 'char' if no promotion is used? */
bool astIsUnknownSignChar(const Token *tok);
/** Is expression a char according to valueType? */
bool astIsGenericChar(const Token* tok);
/** Is expression of integral type? */
bool astIsIntegral(const Token *tok, bool unknown);
bool astIsUnsigned(const Token* tok);
/** Is expression of floating point type? */
bool astIsFloat(const Token *tok, bool unknown);
/** Is expression of boolean type? */
bool astIsBool(const Token *tok);

bool astIsPointer(const Token *tok);

bool astIsSmartPointer(const Token* tok);
bool astIsUniqueSmartPointer(const Token* tok);

bool astIsIterator(const Token *tok);

bool astIsContainer(const Token *tok);

bool astIsContainerView(const Token* tok);
bool astIsContainerOwned(const Token* tok);
bool astIsContainerString(const Token* tok);

Library::Container::Action astContainerAction(const Token* tok, const Token** ftok = nullptr);
Library::Container::Yield astContainerYield(const Token* tok, const Token** ftok = nullptr);

Library::Container::Yield astFunctionYield(const Token* tok, const Settings* settings, const Token** ftok = nullptr);

/** Is given token a range-declaration in a range-based for loop */
bool astIsRangeBasedForDecl(const Token* tok);

/**
 * Get canonical type of expression. const/static/etc are not included and neither *&.
 * For example:
 * Expression type      Return
 * std::string          std::string
 * int *                int
 * static const int     int
 * std::vector<T>       std::vector
 */
std::string astCanonicalType(const Token *expr, bool pointedToType);

/** Is given syntax tree a variable comparison against value */
const Token * astIsVariableComparison(const Token *tok, const std::string &comp, const std::string &rhs, const Token **vartok=nullptr);

bool isVariableDecl(const Token* tok);
bool isStlStringType(const Token* tok);

bool isTemporary(bool cpp, const Token* tok, const Library* library, bool unknown = false);

const Token* previousBeforeAstLeftmostLeaf(const Token* tok);
Token* previousBeforeAstLeftmostLeaf(Token* tok);

CPPCHECKLIB const Token * nextAfterAstRightmostLeaf(const Token * tok);
Token* nextAfterAstRightmostLeaf(Token* tok);

Token* astParentSkipParens(Token* tok);
const Token* astParentSkipParens(const Token* tok);

const Token* getParentMember(const Token * tok);

const Token* getParentLifetime(const Token* tok);
const Token* getParentLifetime(bool cpp, const Token* tok, const Library* library);

std::vector<ValueType> getParentValueTypes(const Token* tok,
                                           const Settings* settings = nullptr,
                                           const Token** parent = nullptr);

bool astIsLHS(const Token* tok);
bool astIsRHS(const Token* tok);

Token* getCondTok(Token* tok);
const Token* getCondTok(const Token* tok);

Token* getInitTok(Token* tok);
const Token* getInitTok(const Token* tok);

Token* getStepTok(Token* tok);
const Token* getStepTok(const Token* tok);

Token* getCondTokFromEnd(Token* endBlock);
const Token* getCondTokFromEnd(const Token* endBlock);

/// For a "break" token, locate the next token to execute. The token will
/// be either a "}" or a ";".
const Token *findNextTokenFromBreak(const Token *breakToken);

/**
 * Extract for loop values: loopvar varid, init value, step value, last value (inclusive)
 */
bool extractForLoopValues(const Token *forToken,
                          nonneg int &varid,
                          bool &knownInitValue,
                          long long &initValue,
                          bool &partialCond,
                          long long &stepValue,
                          long long &lastValue);

bool precedes(const Token * tok1, const Token * tok2);
bool succeeds(const Token* tok1, const Token* tok2);

bool exprDependsOnThis(const Token* expr, bool onVar = true, nonneg int depth = 0);

struct ReferenceToken {
    const Token* token;
    ErrorPath errors;
};

SmallVector<ReferenceToken> followAllReferences(const Token* tok,
                                                bool temporary = true,
                                                bool inconclusive = true,
                                                ErrorPath errors = ErrorPath{},
                                                int depth = 20);
const Token* followReferences(const Token* tok, ErrorPath* errors = nullptr);

CPPCHECKLIB bool isSameExpression(bool cpp, bool macro, const Token *tok1, const Token *tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isEqualKnownValue(const Token * const tok1, const Token * const tok2);

bool isStructuredBindingVariable(const Variable* var);

const Token* isInLoopCondition(const Token* tok);

/**
 * Is token used as boolean, that is to say cast to a bool, or used as a condition in a if/while/for
 */
CPPCHECKLIB bool isUsedAsBool(const Token* const tok, const Settings* settings = nullptr);

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

bool isConstFunctionCall(const Token* ftok, const Library& library);

bool isConstExpression(const Token *tok, const Library& library, bool cpp);

bool isWithoutSideEffects(bool cpp, const Token* tok, bool checkArrayAccess = false, bool checkReference = true);

bool isUniqueExpression(const Token* tok);

bool isEscapeFunction(const Token* ftok, const Library* library);

/** Is scope a return scope (scope will unconditionally return) */
CPPCHECKLIB bool isReturnScope(const Token* const endToken,
                               const Library* library = nullptr,
                               const Token** unknownFunc = nullptr,
                               bool functionScope = false);

/** Is tok within a scope of the given type, nested within var's scope? */
bool isWithinScope(const Token* tok,
                   const Variable* var,
                   Scope::ScopeType type);

/// Return the token to the function and the argument number
const Token * getTokenArgumentFunction(const Token * tok, int& argn);
Token* getTokenArgumentFunction(Token* tok, int& argn);

std::vector<const Variable*> getArgumentVars(const Token* tok, int argnr);

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
CPPCHECKLIB bool isVariableChangedByFunctionCall(const Token *tok, int indirect, const Settings *settings, bool *inconclusive);

/** Is variable changed in block of code? */
CPPCHECKLIB bool isVariableChanged(const Token *start, const Token *end, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);
bool isVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);

bool isVariableChanged(const Token *tok, int indirect, const Settings *settings, bool cpp, int depth = 20);

bool isVariableChanged(const Variable * var, const Settings *settings, bool cpp, int depth = 20);

bool isVariablesChanged(const Token* start,
                        const Token* end,
                        int indirect,
                        const std::vector<const Variable*> &vars,
                        const Settings* settings,
                        bool cpp);

bool isThisChanged(const Token* tok, int indirect, const Settings* settings, bool cpp);
bool isThisChanged(const Token* start, const Token* end, int indirect, const Settings* settings, bool cpp);

const Token* findVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);
Token* findVariableChanged(Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);

CPPCHECKLIB bool isExpressionChanged(const Token* expr,
                                     const Token* start,
                                     const Token* end,
                                     const Settings* settings,
                                     bool cpp,
                                     int depth = 20);

bool isExpressionChangedAt(const Token* expr,
                           const Token* tok,
                           int indirect,
                           bool globalvar,
                           const Settings* settings,
                           bool cpp,
                           int depth = 20);

/// If token is an alias if another variable
bool isAliasOf(const Token *tok, nonneg int varid, bool* inconclusive = nullptr);

bool isAliasOf(const Token* tok, const Token* expr, int* indirect = nullptr, bool* inconclusive = nullptr);

bool isAliased(const Variable *var);

const Token* getArgumentStart(const Token* ftok);

/** Determines the number of arguments - if token is a function call or macro
 * @param ftok start token which is supposed to be the function/macro name.
 * @return Number of arguments
 */
int numberOfArguments(const Token* ftok);

/// Get number of arguments without using AST
int numberOfArgumentsWithoutAst(const Token* start);

/**
 * Get arguments (AST)
 */
std::vector<const Token *> getArguments(const Token *ftok);

int getArgumentPos(const Variable* var, const Function* f);

const Token* getIteratorExpression(const Token* tok);

/**
 * Are the arguments a pair of iterators/pointers?
 */
bool isIteratorPair(std::vector<const Token*> args);

CPPCHECKLIB const Token *findLambdaStartToken(const Token *last);

/**
 * find lambda function end token
 * \param first The [ token
 * \return nullptr or the }
 */
CPPCHECKLIB const Token *findLambdaEndToken(const Token *first);
CPPCHECKLIB Token* findLambdaEndToken(Token* first);

bool isLikelyStream(bool cpp, const Token *stream);

/**
 * do we see a likely write of rhs through overloaded operator
 *   s >> x;
 *   a & x;
 */
bool isLikelyStreamRead(bool cpp, const Token *op);

bool isCPPCast(const Token* tok);

bool isConstVarExpression(const Token* tok, std::function<bool(const Token*)> skipPredicate = nullptr);

enum class ExprUsage { None, NotUsed, PassedByReference, Used, Inconclusive };

ExprUsage getExprUsage(const Token* tok, int indirect, const Settings* settings, bool cpp);

const Variable *getLHSVariable(const Token *tok);

const Token* getLHSVariableToken(const Token* tok);

std::vector<const Variable*> getLHSVariables(const Token* tok);

/** Find a allocation function call in expression, so result of expression is allocated memory/resource. */
const Token* findAllocFuncCallToken(const Token *expr, const Library &library);

bool isScopeBracket(const Token* tok);

CPPCHECKLIB bool isNullOperand(const Token *expr);

bool isGlobalData(const Token *expr, bool cpp);

bool isUnevaluated(const Token *tok);

#endif // astutilsH
