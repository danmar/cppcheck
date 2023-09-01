
// Test library configuration for kde.cfg
//
// Usage:
// $ cppcheck --check-library --library=kde --enable=style,information --inconclusive --error-exitcode=1 --disable=missingInclude --inline-suppr test/cfg/kde.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <cstdio>
#include <KDE/KGlobal>
#include <KDE/KConfigGroup>

class k_global_static_testclass1 {};
K_GLOBAL_STATIC(k_global_static_testclass1, k_global_static_testinstance1);

void valid_code(const KConfigGroup& cfgGroup)
{
    const k_global_static_testclass1 * pk_global_static_testclass1 = k_global_static_testinstance1;
    printf("%p", pk_global_static_testclass1);

    bool entryTest = cfgGroup.readEntry("test", false);
    if (entryTest) {}
}

void ignoredReturnValue(const KConfigGroup& cfgGroup)
{
    // cppcheck-suppress ignoredReturnValue
    cfgGroup.readEntry("test", "default");
    // cppcheck-suppress ignoredReturnValue
    cfgGroup.readEntry("test");
}
