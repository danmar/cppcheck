/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "cppcheck.h"
#include "testsuite.h"

#include <algorithm>
#include <map>
#include <string>
#include <stdexcept>

extern std::ostringstream errout;
extern std::ostringstream output;

class TestThreadExecutor : public TestFixture
{
public:
    TestThreadExecutor() : TestFixture("TestThreadExecutor")
    { }

private:

    void check(const std::string &data)
    {
        errout.str("");
        output.str("");
    }

    void run()
    {
        TEST_CASE(jobs);
    }

    void jobs()
    {
        errout.str("");
        output.str("");
    }
};

REGISTER_TEST(TestThreadExecutor)
