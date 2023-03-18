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
#ifndef fwdanalysisH
#define fwdanalysisH
//---------------------------------------------------------------------------

#include "config.h"

#include <set>
#include <vector>

class Token;
class Library;

/**
 * Forward data flow analysis for checks
 *  - unused value
 *  - redundant assignment
 *  - valueflow analysis
 */
class FwdAnalysis {
public:
    FwdAnalysis(bool cpp, const Library &library) : mCpp(cpp), mLibrary(library) {}

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
        bool known{};
        const Token* token{};
    };

    /** Is there some possible alias for given expression */
    bool possiblyAliased(const Token *expr, const Token *startToken) const;

    std::set<nonneg int> getExprVarIds(const Token* expr, bool* localOut = nullptr, bool* unknownVarIdOut = nullptr) const;
private:
    static bool isEscapedAlias(const Token* expr);

    /** Result of forward analysis */
    struct Result {
        enum class Type { NONE, READ, WRITE, BREAK, RETURN, BAILOUT } type;
        explicit Result(Type type) : type(type) {}
        Result(Type type, const Token *token) : type(type), token(token) {}
        const Token* token{};
    };

    struct Result check(const Token *expr, const Token *startToken, const Token *endToken);
    struct Result checkRecursive(const Token *expr, const Token *startToken, const Token *endToken, const std::set<nonneg int> &exprVarIds, bool local, bool inInnerClass, int depth=0);

    // Is expression a l-value global data?
    bool isGlobalData(const Token *expr) const;

    const bool mCpp;
    const Library &mLibrary;
    enum class What { Reassign, UnusedValue, ValueFlow } mWhat = What::Reassign;
    std::vector<KnownAndToken> mValueFlow;
    bool mValueFlowKnown = true;
};

#endif // fwdanalysisH
