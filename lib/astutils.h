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
#ifndef astutilsH
#define astutilsH
//---------------------------------------------------------------------------

#include <functional>
#include <string>
#include <vector>

#include "errorlogger.h"
#include "utils.h"

class Library;
class Settings;
class Scope;
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

Token* astParentSkipParens(Token* tok);
const Token* astParentSkipParens(const Token* tok);

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
 * @param pure boolean
 */
bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isOppositeExpression(bool cpp, const Token * const tok1, const Token * const tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors=nullptr);

bool isConstExpression(const Token *tok, const Library& library, bool pure, bool cpp);

bool isWithoutSideEffects(bool cpp, const Token* tok);

bool isUniqueExpression(const Token* tok);

/** Is scope a return scope (scope will unconditionally return) */
bool isReturnScope(const Token *endToken, const Settings * settings = nullptr, bool functionScope=false);

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
bool isVariableChanged(const Token *start, const Token *end, const nonneg int varid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);

bool isVariableChanged(const Token *tok, int indirect, const Settings *settings, bool cpp, int depth = 20);

bool isVariableChanged(const Variable * var, const Settings *settings, bool cpp, int depth = 20);

const Token* findVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int varid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);
Token* findVariableChanged(Token *start, const Token *end, int indirect, const nonneg int varid, bool globalvar, const Settings *settings, bool cpp, int depth = 20);

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

bool isLikelyStream(bool cpp, const Token *stream);

/**
 * do we see a likely write of rhs through overloaded operator
 *   s >> x;
 *   a & x;
 */
bool isLikelyStreamRead(bool cpp, const Token *op);

bool isCPPCast(const Token* tok);

bool isConstVarExpression(const Token *tok);

const Variable *getLHSVariable(const Token *tok);

struct PathAnalysis {
    enum class Progress {
        Continue,
        Break
    };
    PathAnalysis(const Token* start, const Library& library)
        : start(start), library(&library)
    {}
    const Token * start;
    const Library * library;

    struct Info {
        const Token* tok;
        ErrorPath errorPath;
        bool known;
    };

    void forward(const std::function<Progress(const Info&)>& f) const;
    template<class F>
    void forwardAll(F f) {
        forward([&](const Info& info) {
            f(info);
            return Progress::Continue;
        });
    }
    template<class Predicate>
    Info forwardFind(Predicate pred) {
        Info result{};
        forward([&](const Info& info) {
            if (pred(info)) {
                result = info;
                return Progress::Break;
            }
            return Progress::Continue;
        });
        return result;
    }
private:

    Progress forwardRecursive(const Token* tok, Info info, const std::function<PathAnalysis::Progress(const Info&)>& f) const;
    Progress forwardRange(const Token* startToken, const Token* endToken, Info info, const std::function<Progress(const Info&)>& f) const;

    static const Scope* findOuterScope(const Scope * scope);

    static std::pair<bool, bool> checkCond(const Token * tok, bool& known);
};

/**
 * @brief Returns true if there is a path between the two tokens
 *
 * @param start Starting point of the path
 * @param dest The path destination
 * @param errorPath Adds the path traversal to the errorPath
 */
bool reaches(const Token * start, const Token * dest, const Library& library, ErrorPath* errorPath);

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

    static bool isNullOperand(const Token *expr);
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
    struct Result checkRecursive(const Token *expr, const Token *startToken, const Token *endToken, const std::set<int> &exprVarIds, bool local, bool inInnerClass);

    // Is expression a l-value global data?
    bool isGlobalData(const Token *expr) const;

    const bool mCpp;
    const Library &mLibrary;
    enum class What { Reassign, UnusedValue, ValueFlow } mWhat;
    std::vector<KnownAndToken> mValueFlow;
    bool mValueFlowKnown;
};

#endif // astutilsH
