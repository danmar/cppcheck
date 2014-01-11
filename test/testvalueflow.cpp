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
        TEST_CASE(valueFlowBeforeCondition);
        TEST_CASE(valueFlowForLoop);
        TEST_CASE(valueFlowSubFunction);
    }

    bool testValueOfX(const std::string &code, unsigned int linenr, int value) {
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
            }
        }

        return false;
    }


    void bailout(const char code[]) {
        Settings settings;
        settings.valueFlow = true;  // temporary flag
        settings.debugwarnings = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");
    }


    void valueFlowBeforeCondition() {
        const char *code;

        code = "void f(int x) {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));

        code = "void f(unsigned int x) {\n"
               "    int a = x;\n"
               "    if (x >= 1) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(int *x) {\n"
               "    *x = 100;\n"
               "    if (x) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "extern const int x;\n"
               "void f() {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        // assignment
        code = "void f(int x) {\n"
               "   x = 2 + x;\n"
               "   if (x == 65);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 65));


        // guarding by &&
        code = "void f(int x) {\n"
               "    if (!x || \n"  // <- x can be 0
               "        a/x) {}\n" // <- x can't be 0
               "    if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int *x) {\n"
               "  ((x=ret())&&\n"
               "   (*x==0));\n"  // <- x is not 0
               "  if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // function calls
        code = "void f(int x) {\n"
               "  a = x;\n"
               "  setx(x);\n"
               "  if (x == 1) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(std::string("void setx(int x);")+code, 2U, 1));
        ASSERT_EQUALS(false, testValueOfX(std::string("void setx(int &x);")+code, 2U, 1));
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 1));

        // bailout: ?:
        bailout("void f(int x) {\n"
                "    y = ((x<0) ? x : ((x==2)?3:4));\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: no simplification of x within ?: expression\n", errout.str());

        bailout("int f(int x) {\n"
                "  int r = x ? 1 / x : 0;\n"
                "  if (x == 0) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: no simplification of x within ?: expression\n", errout.str());

        code = "void f(int x) {\n"
               "    int a =v x;\n"
               "    a = b ? x/2 : 20/x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));

        code = "void f(int x, int y) {\n"
               "    a = x;\n"
               "    if (y){}\n"
               "    if (x==123){}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));

        // bailout: if/else/etc
        bailout("void f(int x) {\n"
                "    if (x != 123) { b = x; }\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: variable x stopping on }\n", errout.str());

        // bailout: assignment
        bailout("void f(int x) {\n"
                "    x = y;\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: assignment of x\n", errout.str());

        // bailout: global variables
        bailout("int x;\n"
                "void f() {\n"
                "    int a = x;\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:4]: (debug) ValueFlow bailout: global variable x\n", errout.str());
    }

    void valueFlowForLoop() {
        const char *code;

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x++)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 10));

        code = "void f() {\n"
               "    for (int x = 0; x < ((short)10); x++)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 10));
    }

    void valueFlowSubFunction() {
        const char *code;

        code = "void f1(int x) { return x; }\n"
               "void f2(int x) {\n"
               "    f1(123);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 123));

        code = "void f1(int x) { return x; }\n"
               "void f2(int x) {\n"
               "    f1(x);\n"
               "    if (x==0){}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 0));

        code = "void f1(int x) {\n"
               "    if (x == 0) return;\n"
               "    int y = 1234 / x;\n"
               "}\n"
               "\n"
               "void f2() {\n"
               "    f1(0);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f1(int x) { a=x; }\n"
               "void f2(int y) { f1(y<123); }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 1));

        code = "void f1(int x) { a=(abc)x; }\n"
               "void f2(int y) { f1(123); }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 123));
    }
};

REGISTER_TEST(TestValueFlow)
