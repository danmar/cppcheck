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
#ifndef cxx11emuH
#define cxx11emuH
//---------------------------------------------------------------------------

/* Emulate certain features of C++11 in a C++98-compatible way. */

#ifdef __cplusplus
#if (__GNUC__ <= 4 && __GNUC_MINOR__ < 6 && !defined(__clang__)) || (!defined(__GXX_EXPERIMENTAL_CXX0X__) && __cplusplus < 201103L)

// Null pointer literal
// Source: SC22/WG21/N2431 = J16/07-0301
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2431.pdf

const                        // this is a const object...
class {
public:
    template<class T>          // convertible to any type
    operator T*() const {    // of null non-member
        return 0;    // pointer...
    }
    template<class C, class T> // or any type of null
    operator T C::*() const { // member pointer...
        return 0;
    }
private:
    void operator&() const;    // whose address can't be taken
} cppcheck_nullptr_impl = {};  // and whose name is nullptr

// An evil workaround for the inability to disable -Wc++0x-compat using a #pragma.
// Once -std=c++0x is embraced, the above class can be renamed to nullptr and
// the define can be removed.
#define nullptr cppcheck_nullptr_impl

// Static assertions

#ifndef static_assert
#define XXJOIN(x, y) XXJOIN_AGAIN(x, y)
#define XXJOIN_AGAIN(x, y) x ## y
#define static_assert(e, msg) \
 typedef char XXJOIN(assertion_failed_at_line_, __LINE__) [(e) ? 1 : -1]
#endif // static_assert

#endif // __cplusplus < 201103L
#endif // __cplusplus

//---------------------------------------------------------------------------
#endif // cxx11emuH
