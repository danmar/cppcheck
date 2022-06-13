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

#ifndef configH
#define configH

#ifdef _WIN32
#  ifdef CPPCHECKLIB_EXPORT
#    define CPPCHECKLIB __declspec(dllexport)
#  elif defined(CPPCHECKLIB_IMPORT)
#    define CPPCHECKLIB __declspec(dllimport)
#  else
#    define CPPCHECKLIB
#  endif
#else
#  define CPPCHECKLIB
#endif

// MS Visual C++ memory leak debug tracing
#if !defined(DISABLE_CRTDBG_MAP_ALLOC) && defined(_MSC_VER) && defined(_DEBUG)
#  define _CRTDBG_MAP_ALLOC
#  include <crtdbg.h>
#endif

// C++11 noexcept
#if (defined(__GNUC__) && (__GNUC__ >= 5)) \
    || defined(__clang__) \
    || defined(__CPPCHECK__)
#  define NOEXCEPT noexcept
#else
#  define NOEXCEPT
#endif

// C++11 noreturn
#if (defined(__GNUC__) && (__GNUC__ >= 5)) \
    || defined(__clang__) \
    || defined(__CPPCHECK__)
#  define NORETURN [[noreturn]]
#else
#  define NORETURN
#endif

// fallthrough
#if defined(__clang__)
#  define FALLTHROUGH [[clang::fallthrough]]
#elif (defined(__GNUC__) && (__GNUC__ >= 7))
#  define FALLTHROUGH __attribute__((fallthrough))
#else
#  define FALLTHROUGH
#endif

// unused
#if defined(__GNUC__) \
    || defined(__clang__) \
    || defined(__CPPCHECK__)
#  define UNUSED __attribute__((unused))
#else
#  define UNUSED
#endif

#define REQUIRES(msg, ...) class=typename std::enable_if<__VA_ARGS__::value>::type

#include <string>
static const std::string emptyString;

// Use the nonneg macro when you want to assert that a variable/argument is not negative
#ifdef __CPPCHECK__
#define nonneg   __cppcheck_low__(0)
#elif defined(NONNEG)
// Enable non-negative values checking
// TODO : investigate using annotations/contracts for stronger value checking
#define nonneg   unsigned
#else
// Disable non-negative values checking
#define nonneg
#endif

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define ASAN 1
#endif
#endif

#ifndef ASAN
#ifdef  __SANITIZE_ADDRESS__
#define ASAN 1
#else
#define ASAN 0
#endif
#endif

#if defined(_WIN32)
#define THREADING_MODEL_THREAD
#define STDCALL __stdcall
#elif defined(USE_THREADS)
#define THREADING_MODEL_THREAD
#define STDCALL
#elif ((defined(__GNUC__) || defined(__sun)) && !defined(__MINGW32__) && !defined(__CYGWIN__)) || defined(__CPPCHECK__)
#define THREADING_MODEL_FORK
#else
#error "No threading model defined"
#endif

#define STRINGISIZE(...) #__VA_ARGS__

#ifdef __clang__
#define SUPPRESS_WARNING(warning, ...)_Pragma("clang diagnostic push") _Pragma(STRINGISIZE(clang diagnostic ignored warning)) __VA_ARGS__ _Pragma("clang diagnostic pop")
#define SUPPRESS_DEPRECATED_WARNING(...) SUPPRESS_WARNING("-Wdeprecated", __VA_ARGS__)
#define SUPPRESS_FLOAT_EQUAL_WARNING(...) SUPPRESS_WARNING("-Wfloat-equal", __VA_ARGS__)
#else
#define SUPPRESS_WARNING(warning, ...) __VA_ARGS__
#define SUPPRESS_DEPRECATED_WARNING(...) __VA_ARGS__
#define SUPPRESS_FLOAT_EQUAL_WARNING(...) __VA_ARGS__
#endif


#endif // configH
