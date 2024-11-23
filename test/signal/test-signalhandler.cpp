/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "config.h"

#if defined(USE_UNIX_SIGNAL_HANDLING)
#include "signalhandler.h"

#include <cassert>
#include <cfenv>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// static functions are omitted from trace

/*static*/ NORETURN void my_assert() // NOLINT(misc-use-internal-linkage)
{
    assert(false);
}

/*static*/ NORETURN void my_abort() // NOLINT(misc-use-internal-linkage)
{
    abort();
}

/*static*/ void my_segv() // NOLINT(misc-use-internal-linkage)
{
    // cppcheck-suppress nullPointer
    ++*(int*)nullptr;
}

/*static*/ void my_fpe() // NOLINT(misc-use-internal-linkage)
{
#if !defined(__APPLE__)
    feenableexcept(FE_ALL_EXCEPT); // TODO: check result
#endif
    std::feraiseexcept(FE_UNDERFLOW | FE_DIVBYZERO); // TODO: check result
    // TODO: to generate this via code
}
#endif

int main(int argc, const char * const argv[])
{
#if defined(USE_UNIX_SIGNAL_HANDLING)
    if (argc != 2)
        return 1;

    register_signal_handler(stdout);

    if (strcmp(argv[1], "assert") == 0)
        my_assert();
    else if (strcmp(argv[1], "abort") == 0)
        my_abort();
    else if (strcmp(argv[1], "fpe") == 0)
        my_fpe();
    else if (strcmp(argv[1], "segv") == 0)
        my_segv();

    return 0;
#else
    (void)argc;
    (void)argv;
    return 1;
#endif
}
