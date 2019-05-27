
// Test library configuration for boost.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/boost.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <boost/config.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/endian/conversion.hpp>


BOOST_FORCEINLINE void boost_forceinline_test()
{
}

BOOST_NOINLINE void boost_noinline_test()
{
}

BOOST_NORETURN void boost_noreturn_test()
{
}

void valid_code()
{
    if (BOOST_LIKELY(1)) {
    }
    if (BOOST_UNLIKELY(0)) {
    }

    int int1 = 5;
    boost::endian::endian_reverse_inplace(int1);
}

void ignoredReturnValue(char * buf)
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
