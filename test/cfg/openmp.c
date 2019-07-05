
// Test library configuration for openmp.cfg
//
// Usage:
// $ cppcheck --check-library --library=openmp --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/openmp.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <omp.h>
#include <stdio.h>

void validCode()
{
    int arr[20] = { 0 };
    #pragma omp parallel for
    for (int i = 0; i < 20; ++i) {
        arr[i] = i * i;
    }

    char * pChars = (char *) omp_target_alloc(4, 1);
    printf("pChars: %p", pChars);
    omp_target_free(pChars, 1);
}

void memleak_omp_target_alloc()
{
    char * pChars = (char *) omp_target_alloc(2, 0);
    printf("pChars: %p", pChars);
    // cppcheck-suppress memleak
}
