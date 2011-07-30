/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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


#include "tokenize.h"
#include "checkassignif.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestAssignIf : public TestFixture
{
public:
    TestAssignIf() : TestFixture("TestAssignIf")
    { }

private:


    void run()
    {
        TEST_CASE(testand);
    }

    void check(const char code[])
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings._checkCodingStyle = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        // Check char variable usage..
        CheckAssignIf CheckAssignIf(&tokenizer, &settings, this);
        CheckAssignIf.check();
    }

    void testand()
    {
        check("void foo(int x)\n"
              "{\n"
              "    int y = x & 4;\n"
              "    if (y == 3);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Mismatching assignment and condition, condition is always false\n", errout.str());
    }
};

REGISTER_TEST(TestAssignIf)

