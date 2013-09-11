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


#include "tokenize.h"
#include "checkassignif.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestAssignIf : public TestFixture {
public:
    TestAssignIf() : TestFixture("TestAssignIf") {
    }

private:


    void run() {
        TEST_CASE(assignAndCompare);   // assignment and comparison don't match
        TEST_CASE(mismatchingBitAnd);  // overlapping bitmasks
        TEST_CASE(compare);            // mismatching LHS/RHS in comparison
        TEST_CASE(multicompare);       // mismatching comparisons
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const std::string str1(tokenizer.tokens()->stringifyList(0,true));
        tokenizer.simplifyTokenList();
        const std::string str2(tokenizer.tokens()->stringifyList(0,true));

        // Ensure that the test case is not bad.
        if (str1 != str2) {
            warn(("Unsimplified code in test case. It looks like this test "
                  "should either be cleaned up or moved to TestTokenizer or "
                  "TestSimplifyTokens instead.\nstr1="+str1+"\nstr2="+str2).c_str());
        }


        // Check char variable usage..
        CheckAssignIf checkAssignIf(&tokenizer, &settings, this);
        checkAssignIf.assignIf();
        checkAssignIf.comparison();
        checkAssignIf.multiCondition();
    }

    void assignAndCompare() {
        // &
        check("void foo(int x)\n"
              "{\n"
              "    int y = x & 4;\n"
              "    if (y == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Mismatching assignment and comparison, comparison 'y==3' is always false.\n", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    int y = x & 4;\n"
              "    if (y != 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Mismatching assignment and comparison, comparison 'y!=3' is always true.\n", errout.str());

        // |
        check("void foo(int x) {\n"
              "    int y = x | 0x14;\n"
              "    if (y == 0x710);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching assignment and comparison, comparison 'y==1808' is always false.\n", errout.str());

        check("void foo(int x) {\n"
              "    int y = x | 0x14;\n"
              "    if (y == 0x71f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // various simple assignments
        check("void foo(int x) {\n"
              "    int y = (x+1) | 1;\n"
              "    if (y == 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching assignment and comparison, comparison 'y==2' is always false.\n", errout.str());

        check("void foo() {\n"
              "    int y = 1 | x();\n"
              "    if (y == 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching assignment and comparison, comparison 'y==2' is always false.\n", errout.str());

        // multiple conditions
        check("void foo(int x) {\n"
              "    int y = x & 4;\n"
              "    if ((y == 3) && (z == 1));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching assignment and comparison, comparison 'y==3' is always false.\n", errout.str());

        check("void foo(int x) {\n"
              "    int y = x & 4;\n"
              "    if ((x==123) || ((y == 3) && (z == 1)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching assignment and comparison, comparison 'y==3' is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    if (setvalue(&y) && y != 8);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // recursive checking into scopes
        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    if (z) y=0;\n"
              "    else { if (y==8); }\n" // always false
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false.\n", errout.str());

        // while
        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    while (y==8);\n" // local variable => always false
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    extern int y; y = x & 7;\n"
              "    while (y==8);\n" // non-local variable => no error
              "}");
        ASSERT_EQUALS("", errout.str());

        // calling function
        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    do_something();\n"
              "    if (y==8);\n" // local variable => always false
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    do_something(&y);\n" // passing variable => no error
              "    if (y==8);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void do_something(int);\n"
              "void f(int x) {\n"
              "    int y = x & 7;\n"
              "    do_something(y);\n"
              "    if (y==8);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    extern int y; y = x & 7;\n"
              "    do_something();\n"
              "    if (y==8);\n" // non-local variable => no error
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4434 : false positive: ?:
        check("void f(int x) {\n"
              "    x = x & 1;\n"
              "    x = x & 1 ? 1 : -1;\n"
              "    if(x != -1) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4735
        check("void f() {\n"
              "    int x = *(char*)&0x12345678;\n"
              "    if (x==18) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // bailout: no variable info
        check("void foo(int x) {\n"
              "    y = 2 | x;\n"  // y not declared => no error
              "    if(y == 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // bailout: negative number
        check("void foo(int x) {\n"
              "    int y = -2 | x;\n" // negative number => no error
              "    if (y==1) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // bailout: pass variable to function
        check("void foo(int x) {\n"
              "    int y = 2 | x;\n"
              "    bar(&y);\n"  // pass variable to function => no error
              "    if (y==1) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatchingBitAnd() {
        check("void f(int a) {\n"
              "    int b = a & 0xf0;\n"
              "    b &= 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching bitmasks. Result is always 0 (X = Y & 0xf0; Z = X & 0x1; => Z=0).\n", errout.str());

        check("void f(int a) {\n"
              "    int b = a & 0xf0;\n"
              "    int c = b & 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching bitmasks. Result is always 0 (X = Y & 0xf0; Z = X & 0x1; => Z=0).\n", errout.str());

        check("void f(int a) {\n"
              "    int b = a;"
              "    switch (x) {\n"
              "    case 1: b &= 1; break;\n"
              "    case 2: b &= 2; break;\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void compare() {
        check("void foo(int x)\n"
              "{\n"
              "    if (x & 4 == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X & 0x4) == 0x3' is always false.\n", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    if ((x & 4) == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X & 0x4) == 0x3' is always false.\n", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    if (x & 4 != 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X & 0x4) != 0x3' is always true.\n", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    if ((x | 4) == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X | 0x4) == 0x3' is always false.\n", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    if ((x | 4) != 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X | 0x4) != 0x3' is always true.\n", errout.str());

        // array
        check("void foo(int *x)\n"
              "{\n"
              "    if (x[0] & 4 == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X & 0x4) == 0x3' is always false.\n", errout.str());

        // struct member
        check("void foo(struct X *x)\n"
              "{\n"
              "    if (x->y & 4 == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X & 0x4) == 0x3' is always false.\n", errout.str());

        // expression
        check("void foo(int x)\n"
              "{\n"
              "    if ((x+2) & 4 == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X & 0x4) == 0x3' is always false.\n", errout.str());
    }

    void multicompare() {
        check("void foo(int x)\n"
              "{\n"
              "    if (x & 7);\n"
              "    else { if (x == 1); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Expression is always false because 'else if' condition matches previous condition at line 3.\n", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    if (x & 7);\n"
              "    else { if (x & 1); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Expression is always false because 'else if' condition matches previous condition at line 3.\n", errout.str());
    }
};

REGISTER_TEST(TestAssignIf)
