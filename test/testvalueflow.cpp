/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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

#include "testsuite.h"
#include "testutils.h"
#include "valueflow.h"
#include "tokenize.h"
#include "token.h"

#include <vector>
#include <string>

extern std::ostringstream errout;
class TestValueFlow : public TestFixture {
public:
    TestValueFlow() : TestFixture("TestValueFlow") {
    }

private:

    void run() {
        valueFlowBeforeCondition();
    }

    bool testValueOfX(const char code[], unsigned int linenr, int value) {
        Settings settings;
        settings.valueFlow = true;  // temporary flag

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                std::list<ValueFlow::Value>::const_iterator it;
                for (it = tok->values.begin(); it != tok->values.end(); ++it) {
                    if (it->intvalue == value)
                        return true;
                }
                return false;
            }
        }

        return false;
    }

    void valueFlowBeforeCondition() {
        const char code[] = "void f(int x) {\n"
                            "    int a = x;\n"
                            "    if (x == 123) {}\n"
                            "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));
    }
};

REGISTER_TEST(TestValueFlow)
