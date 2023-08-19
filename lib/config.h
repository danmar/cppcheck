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

#ifndef configH
#define configH

#ifdef MAXTIME
#error "MAXTIME is no longer supported - please use command-line options --checks-max-time=, --template-max-time= and --typedef-max-time= instead"
#endif

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
#elif defined(__GNUC__)
#  define NORETURN __attribute__((noreturn))
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

// warn_unused
#if (defined(__clang__) && (__clang_major__ >= 15))
#  define WARN_UNUSED [[gnu::warn_unused]]
#else
#  define WARN_UNUSED
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
#elif ((defined(__GNUC__) || defined(__sun)) && !defined(__MINGW32__)) || defined(__CPPCHECK__)
#define THREADING_MODEL_FORK
#define STDCALL
#else
#error "No threading model defined"
#endif

#define STRINGISIZE(...) #__VA_ARGS__

#ifdef __clang__
#define SUPPRESS_WARNING_PUSH(warning) _Pragma("clang diagnostic push") _Pragma(STRINGISIZE(clang diagnostic ignored warning))
#define SUPPRESS_WARNING_POP _Pragma("clang diagnostic pop")
#define SUPPRESS_WARNING_GCC_PUSH(warning)
#define SUPPRESS_WARNING_GCC_POP
#define SUPPRESS_WARNING_CLANG_PUSH(warning) SUPPRESS_WARNING_PUSH(warning)
#define SUPPRESS_WARNING_CLANG_POP SUPPRESS_WARNING_POP
#elif defined(__GNUC__)
#define SUPPRESS_WARNING_PUSH(warning) _Pragma("GCC diagnostic push") _Pragma(STRINGISIZE(GCC diagnostic ignored warning))
#define SUPPRESS_WARNING_POP _Pragma("GCC diagnostic pop")
#define SUPPRESS_WARNING_GCC_PUSH(warning) SUPPRESS_WARNING_PUSH(warning)
#define SUPPRESS_WARNING_GCC_POP SUPPRESS_WARNING_POP
#define SUPPRESS_WARNING_CLANG_PUSH(warning)
#define SUPPRESS_WARNING_CLANG_POP
#else
#define SUPPRESS_WARNING_PUSH(warning)
#define SUPPRESS_WARNING_POP
#define SUPPRESS_WARNING_GCC_PUSH(warning)
#define SUPPRESS_WARNING_GCC_POP
#define SUPPRESS_WARNING_CLANG_PUSH(warning)
#define SUPPRESS_WARNING_CLANG_POP
#endif

#if !defined(NO_WINDOWS_SEH) && defined(_WIN32) && defined(_MSC_VER)
#define USE_WINDOWS_SEH
#endif

#if !defined(NO_UNIX_BACKTRACE_SUPPORT) && defined(__GNUC__) && defined(__GLIBC__) && !defined(__CYGWIN__) && !defined(__MINGW32__) && !defined(__NetBSD__) && !defined(__SVR4) && !defined(__QNX__)
#define USE_UNIX_BACKTRACE_SUPPORT
#endif

#if !defined(NO_UNIX_SIGNAL_HANDLING) && defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OS2__)
#define USE_UNIX_SIGNAL_HANDLING
#endif

#endif // configH
