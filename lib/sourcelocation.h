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

#ifndef sourcelocationH
#define sourcelocationH

#ifdef __CPPCHECK__
#define CPPCHECK_HAS_SOURCE_LOCATION 0
#define CPPCHECK_HAS_SOURCE_LOCATION_TS 0
#elif defined(__has_include)
#if __has_include(<source_location>) && __cplusplus >= 202003L
#define CPPCHECK_HAS_SOURCE_LOCATION 1
#else
#define CPPCHECK_HAS_SOURCE_LOCATION 0
#endif
#if __has_include(<experimental/source_location>) && __cplusplus >= 201402L
#define CPPCHECK_HAS_SOURCE_LOCATION_TS 1
#else
#define CPPCHECK_HAS_SOURCE_LOCATION_TS 0
#endif
#else
#define CPPCHECK_HAS_SOURCE_LOCATION 0
#define CPPCHECK_HAS_SOURCE_LOCATION_TS 0
#endif

#if CPPCHECK_HAS_SOURCE_LOCATION
#include <source_location>
using SourceLocation = std::source_location;
#elif CPPCHECK_HAS_SOURCE_LOCATION_TS
#include <experimental/source_location>
using SourceLocation = std::experimental::source_location;
#else
#include <cstdint>
struct SourceLocation {
    static SourceLocation current() {
        return SourceLocation();
    }
    std::uint_least32_t m_line = 0;
    std::uint_least32_t m_column = 0;
    const char* m_file_name = "";
    const char* m_function_name = "";
    std::uint_least32_t line() const {
        return m_line;
    }
    std::uint_least32_t column() const {
        return m_column;
    }
    const char* file_name() const {
        return m_file_name;
    }
    const char* function_name() const {
        return m_function_name;
    }
};
#endif

#endif
