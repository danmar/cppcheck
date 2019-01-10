
// Test library configuration for boost.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/boost.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <boost/config.hpp>


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
}
