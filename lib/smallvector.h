/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#ifndef smallvectorH
#define smallvectorH

#include <cstddef>

static constexpr std::size_t DefaultSmallVectorSize = 0;

#ifdef HAVE_BOOST
#include <boost/container/small_vector.hpp>

template<typename T, std::size_t N = DefaultSmallVectorSize>
using SmallVector = boost::container::small_vector<T, N>;
#else
#include <vector>

template<class T, std::size_t N>
struct TaggedAllocator : std::allocator<T>
{
    template<class ... Ts>
    // cppcheck-suppress noExplicitConstructor
    TaggedAllocator(Ts&&... ts)
        : std::allocator<T>(std::forward<Ts>(ts)...)
    {}
};

template<typename T, std::size_t N = DefaultSmallVectorSize>
using SmallVector = std::vector<T, TaggedAllocator<T, N>>;
#endif

#endif
