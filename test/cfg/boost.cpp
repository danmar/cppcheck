
// Test library configuration for boost.cfg
//
// Usage:
// $ cppcheck --check-library --library=boost --enable=style,information --inconclusive --error-exitcode=1 --disable=missingInclude --inline-suppr test/cfg/boost.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <boost/config.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>


BOOST_FORCEINLINE void boost_forceinline_test()
{}

BOOST_NOINLINE void boost_noinline_test()
{}

BOOST_NORETURN void boost_noreturn_test()
{}

void print_hello()
{
    printf("hello");
}

void valid_code(boost::function<void(void)> &pf_print_hello)
{
    if (BOOST_LIKELY(1)) {}
    if (BOOST_UNLIKELY(0)) {}

    int int1 = 5;
    boost::endian::endian_reverse_inplace(int1);
    boost::bind(print_hello)();
    pf_print_hello = boost::bind(print_hello);
}

void ignoredReturnValue()
{
    // cppcheck-suppress ignoredReturnValue
    boost::math::round(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::math::iround(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::math::lround(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::math::llround(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::endian::endian_reverse(1);
}

void uninitvar()
{
    int intUninit1;
    int intUninit2;
    // cppcheck-suppress uninitvar
    boost::endian::endian_reverse_inplace(intUninit1);
    // cppcheck-suppress uninitvar
    (void)boost::math::round(intUninit2);
}

void throwexception(int * buf)
{
    if (!buf)
        boost::throw_exception(std::bad_alloc());
    *buf = 0;
}

void throwexception2(int * buf)
{
    if (!buf)
        BOOST_THROW_EXCEPTION(std::bad_alloc());
    *buf = 0;
}

void macros()
{
#define DECL(z, n, text) text ## n = n;
    BOOST_PP_REPEAT(5, DECL, int x)

    BOOST_SCOPED_ENUM_DECLARE_BEGIN(future_errc) {
        no_state
    }
    BOOST_SCOPED_ENUM_DECLARE_END(future_errc)
}
