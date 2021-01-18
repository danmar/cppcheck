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
#ifndef tokenrangeH
#define tokenrangeH
//---------------------------------------------------------------------------

#include "config.h"

template<typename T>
class TokenRange
{
    static_assert(std::is_convertible<T*, const Token*>::value, "T must be a Token to use TokenRange");

    T* mFront;
    T* mBack;

public:
    TokenRange(T* front, T* back) : mFront(front), mBack(back) {}

    struct TokenIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type = Token*;
        using difference_type = std::ptrdiff_t;
        using pointer = Token**;
        using reference = Token*&;

        T* mt;
        TokenIterator() : mt(nullptr) {}
        TokenIterator(T* t) : mt(t) {}
        TokenIterator& operator++() { mt = mt->next(); return *this; }
        bool operator==(const TokenIterator& b) { return mt == b.mt; }
        bool operator!=(const TokenIterator& b) { return mt != b.mt; }
        T* operator*() { return mt; }
    };

    TokenIterator begin() { return TokenIterator(mFront); }
    TokenIterator end() { return TokenIterator(mBack); }
};

#endif // tokenrangeH
