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
#if defined(_MSC_VER) && defined(_DEBUG)
#  define _CRTDBG_MAP_ALLOC
#  include <crtdbg.h>
#endif

// C++11 override
#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ >= 5)) \
  || (defined(__clang__) && (defined (__cplusplus)) && (__cplusplus >= 201103L)) \
  || defined(__CPPCHECK__)
#  define OVERRIDE override
#else
#  define OVERRIDE
#endif

// C++11 noexcept
#if (defined(__GNUC__) && (__GNUC__ >= 5)) \
  || (defined(__clang__) && (defined (__cplusplus)) && (__cplusplus >= 201103L)) \
  || defined(__CPPCHECK__)
#  define NOEXCEPT noexcept
#else
#  define NOEXCEPT
#endif

// C++11 noreturn
#if (defined(__GNUC__) && (__GNUC__ >= 5)) \
  || (defined(__clang__) && (defined (__cplusplus)) && (__cplusplus >= 201103L)) \
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

#define REQUIRES(msg, ...) class=typename std::enable_if<__VA_ARGS__::value>::type

#include <string>
static const std::string emptyString;

#endif // configH
