/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2018 Cppcheck team.
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
#ifndef foreachH
#define foreachH
//---------------------------------------------------------------------------

#include <cstdlib>

#ifndef CPPCHECK_HAS_RANGE_FOR
#if (defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 6) || defined(_MSC_VER)
#define CPPCHECK_HAS_RANGE_FOR 0
#else
#define CPPCHECK_HAS_RANGE_FOR 1
#endif
#endif

#if !CPPCHECK_HAS_RANGE_FOR

template<class Range>
typename Range::iterator range_begin(Range& r)
{
    return r.begin();
}

template<class Range>
typename Range::iterator range_end(Range& r)
{
    return r.end();
}

template<class Range>
typename Range::const_iterator range_begin(const Range& r)
{
    return r.begin();
}

template<class Range>
typename Range::const_iterator range_end(const Range& r)
{
    return r.end();
}

template< class T, std::size_t N >
T* range_begin(T(&array)[N])
{
    return array;
}

template< class T, std::size_t N >
T* range_end(T(&array)[N])
{
    return array+N;
}

#define CPPCHECK_PRIVATE_VAR_CAT_IMPL(x, y) x ## y
#define CPPCHECK_PRIVATE_VAR_CAT(x, y) CPPCHECK_PRIVATE_VAR_CAT_IMPL(x, y)
#define CPPCHECK_PRIVATE_VAR(x) CPPCHECK_PRIVATE_VAR_CAT(_cppcheck_ ## x, __LINE__)

#define FOREACH(VAR, ...)                                                                                                                   \
    if(bool CPPCHECK_PRIVATE_VAR(done) = false) {}                                                                                          \
    else for(auto && CPPCHECK_PRIVATE_VAR(rng) = (__VA_ARGS__); !CPPCHECK_PRIVATE_VAR(done);)                                               \
        for(auto CPPCHECK_PRIVATE_VAR(begin) = range_begin(CPPCHECK_PRIVATE_VAR(rng)); !CPPCHECK_PRIVATE_VAR(done);                               \
                CPPCHECK_PRIVATE_VAR(done) = true)                                                                                          \
            for(auto CPPCHECK_PRIVATE_VAR(end) = range_end(CPPCHECK_PRIVATE_VAR(rng));                                                            \
                    !CPPCHECK_PRIVATE_VAR(done) && CPPCHECK_PRIVATE_VAR(begin) != CPPCHECK_PRIVATE_VAR(end); ++CPPCHECK_PRIVATE_VAR(begin)) \
                if(!(CPPCHECK_PRIVATE_VAR(done) = true)) {}                                                                                 \
                else for(VAR = *CPPCHECK_PRIVATE_VAR(begin); CPPCHECK_PRIVATE_VAR(done); CPPCHECK_PRIVATE_VAR(done) = false)                \

#else
#define FOREACH(VAR, ...) for(VAR : (__VA_ARGS__))
#endif

#endif
