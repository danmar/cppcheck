/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include <cstdio>
#include <cstdlib>
#include "testsuite.h"
#include "options.h"

int main(int argc, char *argv[])
{
    // MS Visual C++ memory leak debug tracing
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

#ifdef NDEBUG
    try {
#endif
        options args(argc, const_cast<const char**>(argv));

        std::size_t failedTestsCount = TestFixture::runTests(args);
        return (failedTestsCount == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
#ifdef NDEBUG
    } catch (const InternalError& e) {
        printf("%s\n", e.errorMessage.c_str());
    } catch (const std::exception& error) {
        printf("%s\n", error.what());
    } catch (...) {
        printf("Unknown exception\n");
    }
    return EXIT_FAILURE;
#endif
}
