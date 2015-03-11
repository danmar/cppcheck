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

#include "timer.h"
#include "testsuite.h"
#include <cmath>


class TestTimer : public TestFixture {
public:
    TestTimer() : TestFixture("TestTimer") {
    }

private:

    void run() {
        TEST_CASE(result);
    }

    void result() const {
        TimerResultsData t1;
        t1._clocks = ~(std::clock_t)0;
        ASSERT(t1.seconds() > 100.0);

        t1._clocks = CLOCKS_PER_SEC * 5 / 2;
        ASSERT(std::fabs(t1.seconds()-2.5) < 0.01);
    }
};

REGISTER_TEST(TestTimer)
