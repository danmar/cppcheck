
// Test library configuration for googletest.cfg
//
// Usage:
// $ cppcheck --check-library --library=googletest --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/googletest.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <gmock/gmock-generated-matchers.h>
#include <gtest/gtest.h>


namespace ExampleNamespace {
    constexpr long long TOLERANCE = 10;

    // #9397 syntaxError when MATCHER_P is not known
    MATCHER_P(ExampleMatcherPTest, expected, "")
    {
        return ((arg <= (expected + TOLERANCE)) && (arg >= (expected - TOLERANCE)));
    }

    // syntaxError when MATCHER is not known
    MATCHER(ExampleMatcherTest, "")
    {
        return (arg == TOLERANCE);
    }
}

TEST(ASSERT, ASSERT)
{
    int *a = (int*)calloc(10,sizeof(int));
    ASSERT_TRUE(a != nullptr);

    a[0] = 10;

    free(a);
}

// Avoid syntax error: https://sourceforge.net/p/cppcheck/discussion/general/thread/6ccc7283e2/
TEST(test_cppcheck, cppcheck)
{
    TestStruct<int> it;
    ASSERT_THROW(it.operator->(), std::out_of_range);
}

// #9964 - avoid compareBoolExpressionWithInt false positive
TEST(Test, assert_false_fp)
{
    // cppcheck-suppress checkLibraryNoReturn
    ASSERT_FALSE(errno < 0);
}
