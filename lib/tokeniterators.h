/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#ifndef tokeniterators_h__
#define tokeniterators_h__


#include "symboldatabase.h"
#include "tokenlist.h"
#include "tokenize.h"

#define ITERATE_TOKENS_2(loop_var_name, tokBegin, tokEnd) const Token* loop_var_name = tokBegin; loop_var_name && loop_var_name != tokEnd; loop_var_name = loop_var_name->next()
#define ITERATE_TOKENS_1(loop_var_name, ...) ITERATE_TOKENS_2(loop_var_name, IterHelpers::GetBeginToken(__VA_ARGS__), IterHelpers::GetEndToken(__VA_ARGS__))

#define ITERATE_TOKENS(loop_var_name, ...) ITERATE_TOKENS_1(loop_var_name, __VA_ARGS__)



/**
 * @brief  Provides GetBeginToken() and GetEndToken() for various token-range objects to be used in ITERATE_TOKENS macro
 */
class IterHelpers {
public:
    // start and end token
    inline static const Token* GetBeginToken(const Token* tokStart, const Token* /* tokEnd */ = NULL) {
        return tokStart;
    };

    inline static const Token* GetEndToken(const Token* /* tokStart */, const Token* tokEnd = NULL) {
        return tokEnd;
    };

public:
    // std::pair of tokens
    inline static const Token* GetBeginToken(const std::pair<const Token*, const Token*>& tokenPair) {
        return tokenPair.first;
    };

    inline static const Token* GetEndToken(const std::pair<const Token*, const Token*>& tokenPair) {
        return tokenPair.second;
    };

public:
    // Scope
    inline static const Token* GetBeginToken(const Scope* scope) {
        return (scope->bodyStart) ? (scope->bodyStart->next()) : nullptr;
    };

    inline static const Token* GetEndToken(const Scope* scope) {
        return scope->bodyEnd;
    };
public:
    class ScopeIncludeBraces {};
    // Scope
    inline static const Token* GetBeginToken(const Scope* scope, ScopeIncludeBraces) {
        return scope->bodyStart;
    };

    inline static const Token* GetEndToken(const Scope* scope, ScopeIncludeBraces) {
        return scope->bodyEnd;
    };

public:
    // TokenList
    inline static const Token* GetBeginToken(const TokenList& tokenList) {
        return tokenList.front();
    };

    inline static const Token* GetEndToken(const TokenList& tokenList) {
        return tokenList.back();
    };
public:
    // Tokenizer
    inline static const Token* GetBeginToken(const Tokenizer* tokenizer) {
        return GetBeginToken(tokenizer->list);
    };

    inline static const Token* GetEndToken(const Tokenizer* tokenizer) {
        return GetEndToken(tokenizer->list);
    };
};


/**
 * @brief  Syntax sugar to be used in c++11 range-based loops
 */
class IterateTokens {

public:

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = iterator;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = iterator;
    public:
        explicit iterator(const Token* tok)
            : mTok(tok)
        {};

        iterator(const iterator& it)
            : mTok(it.mTok)
        {};

        ~iterator() {};

        iterator(iterator&& it) noexcept = default;

        iterator& operator=(const iterator& it) = delete;

        iterator& operator=(const Token* tok) = delete;

        iterator& operator++() {
            if (!mTok) {
                throw std::runtime_error("Null pointer");
            }
            mTok = mTok->next();
            return *this;
        };

        friend bool operator!=(const iterator& it1, const iterator& it2) {
            if (!it1.mTok) {
                // null value should stop the loop
                return false;
            }
            return it1.mTok != it2.mTok;
        };

        const Token* operator->() const {
            return mTok;
        };

        operator const Token* () const {
            return mTok;
        };

        const iterator& operator*() const {
            // return `iterator` instead of `Token*` to handle assignments correcty
            return *this;
        };
    private:
        const Token* mTok;
    };
public:
    explicit IterateTokens(const Token* tokBegin, const Token* tokEnd = nullptr)
        : mTokBegin(tokBegin)
        , mTokEnd(tokEnd)
    {};
    explicit IterateTokens(const std::pair<const Token*, const Token*> &tokenPair)
        : IterateTokens(tokenPair.first, tokenPair.second)
    {}
    explicit IterateTokens(const TokenList& tokenList)
        : IterateTokens(tokenList.front(), tokenList.back())
    {}
    explicit IterateTokens(const Tokenizer* tokenList)
        : IterateTokens(tokenList->list)
    {}
    explicit IterateTokens(const Scope* scope)
        : IterateTokens((scope->bodyStart) ? (scope->bodyStart->next()) : nullptr, scope->bodyEnd)
    {}

public:
    static IterateTokens UntilLinked(const Token* tokBegin) {
        return IterateTokens(tokBegin, tokBegin->link());
    };
    static IterateTokens UntilScopeEnd(const Token* tokBegin) {
        return IterateTokens(tokBegin, tokBegin->scope()->bodyEnd);
    };

    // Should be used to start scope iteration with opening brace
    static IterateTokens ScopeIncludeBraces(const Scope* scope) {
        return IterateTokens(scope->bodyStart, scope->bodyEnd);
    };

public:
    iterator &begin() {
        return mTokBegin;
    };
    iterator &end() {
        return mTokEnd;
    };
private:
    iterator mTokBegin;
    iterator mTokEnd;
};

#endif // tokeniterators_h__
