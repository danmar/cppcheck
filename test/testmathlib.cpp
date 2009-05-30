/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



#include "../src/mathlib.h"
#include "testsuite.h"

class TestMathLib : public TestFixture
{
public:
    TestMathLib() : TestFixture("TestMathLib")
    { }

private:

    void run()
    {
        TEST_CASE(calculate);
        TEST_CASE(convert);
    }

    void calculate()
    {
        ASSERT_EQUALS(std::string("256"), MathLib::add("0xff", "1"));
        ASSERT_EQUALS(std::string("-0.003"), MathLib::multiply("-1e-3", "3"));
        ASSERT_EQUALS(std::string("5"), MathLib::divide("25.5", "5.1"));
        ASSERT_EQUALS(std::string("-11.96"), MathLib::multiply("-2.3", "5.2"));
        ASSERT_EQUALS(std::string("7"), MathLib::divide("21.", "3"));
        ASSERT_EQUALS(std::string("1"), MathLib::divide("3", "2"));
    }

    void convert()
    {
        ASSERT_EQUALS(10, MathLib::toLongNumber("0xa"));
        ASSERT_EQUALS(8, MathLib::toLongNumber("010"));
        ASSERT_EQUALS(10, MathLib::toLongNumber("10"));
    }
};

REGISTER_TEST(TestMathLib)


