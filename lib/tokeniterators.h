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


#define ITERATE_TOKENS(loop_var_name, ...) const Token* loop_var_name = IterHelpers::GetBeginToken(__VA_ARGS__); loop_var_name && loop_var_name != IterHelpers::GetEndToken(__VA_ARGS__); loop_var_name = loop_var_name->next()



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


#endif // tokeniterators_h__
