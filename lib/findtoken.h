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
#ifndef findtokenH
#define findtokenH
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

inline std::vector<MathLib::bigint> evaluateKnownValues(const Token* tok)
{
    if (!tok->hasKnownIntValue())
        return {};
    return {tok->getKnownIntValue()};
}

template<class T, class Predicate, class Found, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
void findTokensImpl(T* start, const Token* end, const Predicate& pred, Found found)
{
    for (T* tok = start; precedes(tok, end); tok = tok->next()) {
        if (pred(tok)) {
            if (found(tok))
                break;
        }
    }
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokens(T* start, const Token* end, const Predicate& pred)
{
    std::vector<T*> result;
    findTokensImpl(start, end, pred, [&](T* tok) {
        result.push_back(tok);
        return false;
    });
    return result;
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
T* findToken(T* start, const Token* end, const Predicate& pred)
{
    T* result = nullptr;
    findTokensImpl(start, end, pred, [&](T* tok) {
        result = tok;
        return true;
    });
    return result;
}

template<class T,
         class Predicate,
         class Found,
         class Evaluate,
         REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
bool findTokensSkipDeadCodeImpl(const Library* library,
                                T* start,
                                const Token* end,
                                const Predicate& pred,
                                Found found,
                                const Evaluate& evaluate)
{
    for (T* tok = start; precedes(tok, end); tok = tok->next()) {
        if (pred(tok)) {
            if (found(tok))
                return true;
        }
        if (Token::Match(tok, "if|for|while (") && Token::simpleMatch(tok->next()->link(), ") {")) {
            const Token* condTok = getCondTok(tok);
            if (!condTok)
                continue;
            auto result = evaluate(condTok);
            if (result.empty())
                continue;
            if (findTokensSkipDeadCodeImpl(library, tok->next(), tok->linkAt(1), pred, found, evaluate))
                return true;
            T* thenStart = tok->linkAt(1)->next();
            T* elseStart = nullptr;
            if (Token::simpleMatch(thenStart->link(), "} else {"))
                elseStart = thenStart->link()->tokAt(2);

            int r = result.front();
            if (r == 0) {
                if (elseStart) {
                    if (findTokensSkipDeadCodeImpl(library, elseStart, elseStart->link(), pred, found, evaluate))
                        return true;
                    if (isReturnScope(elseStart->link(), library))
                        return true;
                    tok = elseStart->link();
                } else {
                    tok = thenStart->link();
                }
            } else {
                if (findTokensSkipDeadCodeImpl(library, thenStart, thenStart->link(), pred, found, evaluate))
                    return true;
                if (isReturnScope(thenStart->link(), library))
                    return true;
                tok = thenStart->link();
            }
        } else if (Token::Match(tok->astParent(), "&&|?|%oror%") && astIsLHS(tok)) {
            auto result = evaluate(tok);
            if (result.empty())
                continue;
            const bool cond = result.front() != 0;
            T* next = nullptr;
            if ((cond && Token::simpleMatch(tok->astParent(), "||")) ||
                (!cond && Token::simpleMatch(tok->astParent(), "&&"))) {
                next = nextAfterAstRightmostLeaf(tok->astParent());
            } else if (Token::simpleMatch(tok->astParent(), "?")) {
                T* colon = tok->astParent()->astOperand2();
                if (!cond) {
                    next = colon;
                } else {
                    if (findTokensSkipDeadCodeImpl(library, tok->astParent()->next(), colon, pred, found, evaluate))
                        return true;
                    next = nextAfterAstRightmostLeaf(colon);
                }
            }
            if (next)
                tok = next;
        } else if (Token::simpleMatch(tok, "} else {")) {
            const Token* condTok = getCondTokFromEnd(tok);
            if (!condTok)
                continue;
            auto result = evaluate(condTok);
            if (result.empty())
                continue;
            if (isReturnScope(tok->link(), library))
                return true;
            int r = result.front();
            if (r != 0) {
                tok = tok->linkAt(2);
            }
        } else if (Token::simpleMatch(tok, "[") && Token::Match(tok->link(), "] (|{")) {
            T* afterCapture = tok->link()->next();
            if (Token::simpleMatch(afterCapture, "(") && afterCapture->link())
                tok = afterCapture->link()->next();
            else
                tok = afterCapture;
        }
    }
    return false;
}

template<class T, class Predicate, class Evaluate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokensSkipDeadCode(const Library* library,
                                       T* start,
                                       const Token* end,
                                       const Predicate& pred,
                                       const Evaluate& evaluate)
{
    std::vector<T*> result;
    findTokensSkipDeadCodeImpl(
        library,
        start,
        end,
        pred,
        [&](T* tok) {
        result.push_back(tok);
        return false;
    },
        evaluate);
    return result;
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokensSkipDeadCode(const Library* library, T* start, const Token* end, const Predicate& pred)
{
    return findTokensSkipDeadCode(library, start, end, pred, &evaluateKnownValues);
}

template<class T, class Predicate, class Evaluate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
T* findTokenSkipDeadCode(const Library* library, T* start, const Token* end, const Predicate& pred, const Evaluate& evaluate)
{
    T* result = nullptr;
    findTokensSkipDeadCodeImpl(
        library,
        start,
        end,
        pred,
        [&](T* tok) {
        result = tok;
        return true;
    },
        evaluate);
    return result;
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
T* findTokenSkipDeadCode(const Library* library, T* start, const Token* end, const Predicate& pred)
{
    return findTokenSkipDeadCode(library, start, end, pred, &evaluateKnownValues);
}

#endif // findtokenH
