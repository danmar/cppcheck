/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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


// This file is for integration tests for trac tickets.
//
// The intention is mostly to put tests here for false positives that
// are fixed by fixing AST/ValueFlow/SymbolDatabase/etc. The check itself
// worked as it should but there was problem with the input.
//
// These tests are typically "black box" tests and don't consider how the
// checker works internally.

#include "checkbool.h"
#include "checknullpointer.h"
#include "checkother.h"
#include "testsuite.h"
#include "settings.h"

class TestTrac : public TestFixture {
public:
    TestTrac() : TestFixture("TestTrac") {
    }

private:

    void run() OVERRIDE {
        TEST_CASE(ticket_7798);
        TEST_CASE(ticket_9573);
        TEST_CASE(ticket_9700);
    }

    template<class C>
    void check(const char code[], const char *filename = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.addEnabled("warning");
        settings.addEnabled("portability");
        settings.addEnabled("performance");
        settings.standards.c = Standards::CLatest;
        settings.standards.cpp = Standards::CPPLatest;
        settings.inconclusive = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // Check..
        C c(&tokenizer, &settings, this);
        c.runChecks(&tokenizer, &settings, this);
    }

    void ticket_7798() {
        // checkComparisonOfFuncReturningBool (overloaded functions)
        check<CheckBool>("bool eval(double *) { return false; }\n"
                         "double eval(char *) { return 1.0; }\n"
                         "int main(int argc, char *argv[])\n"
                         "{\n"
                         "  if ( eval(argv[1]) > eval(argv[2]) )\n"
                         "    return 1;\n"
                         "  return 0;\n"
                         "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ticket_9573() {
        // nullpointer (valueflow)
        check<CheckNullPointer>("int foo (int **array, size_t n_array) {\n"
                                "    size_t i;\n"
                                "    for (i = 0; i < n_array; ++i) {\n"
                                "        if (*array[i] == 1)\n"
                                "            return 1;\n"
                                "    }\n"
                                "    return 0;\n"
                                "} \n"
                                "int bar() {\n"
                                "    int **array = NULL; \n"
                                "    foo (array, 0);\n"
                                "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void ticket_9700() {
        // FP: duplicateBranch
        check<CheckOther>("void* f(bool b) {\n"
                          "    if (b) {\n"
                          "        return new A::Y(true);\n"
                          "    } else {\n"
                          "        return new A::Z(true);\n"
                          "    }\n"
                          "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestTrac)
