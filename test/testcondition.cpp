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

#include "tokenize.h"
#include "checkcondition.h"
#include "testsuite.h"
#include <tinyxml2.h>


class TestCondition : public TestFixture {
public:
    TestCondition() : TestFixture("TestCondition") {
    }

private:


    void run() {
        TEST_CASE(assignAndCompare);   // assignment and comparison don't match
        TEST_CASE(mismatchingBitAnd);  // overlapping bitmasks
        TEST_CASE(compare);            // mismatching LHS/RHS in comparison
        TEST_CASE(multicompare);       // mismatching comparisons
        TEST_CASE(duplicateIf);        // duplicate conditions in if and else-if

        TEST_CASE(checkBadBitmaskCheck);

        TEST_CASE(incorrectLogicOperator1);
        TEST_CASE(incorrectLogicOperator2);
        TEST_CASE(incorrectLogicOperator3);
        TEST_CASE(incorrectLogicOperator4);
        TEST_CASE(incorrectLogicOperator5); // complex expressions
        TEST_CASE(incorrectLogicOperator6); // char literals
        TEST_CASE(incorrectLogicOperator7); // opposite expressions: (expr || !expr)
        TEST_CASE(secondAlwaysTrueFalseWhenFirstTrueError);
        TEST_CASE(incorrectLogicOp_condSwapping);

        TEST_CASE(modulo);

        TEST_CASE(oppositeInnerCondition);

        TEST_CASE(clarifyCondition1);     // if (a = b() < 0)
        TEST_CASE(clarifyCondition2);     // if (a & b == c)
        TEST_CASE(clarifyCondition3);     // if (! a & b)
        TEST_CASE(clarifyCondition4);     // ticket #3110
        TEST_CASE(clarifyCondition5);     // #3609 CWinTraits<WS_CHILD|WS_VISIBLE>..
        TEST_CASE(clarifyCondition6);     // #3818
    }

    void check(const char code[], bool validate=true, const char* filename = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.addEnabled("warning");

        CheckCondition checkCondition;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        checkCondition.runChecks(&tokenizer, &settings, this);
        const std::string str1(tokenizer.tokens()->stringifyList(0,true));
        tokenizer.simplifyTokenList2();
        const std::string str2(tokenizer.tokens()->stringifyList(0,true));
        checkCondition.runSimplifiedChecks(&tokenizer, &settings, this);

        // Ensure that the test case is not bad.
        if (validate && str1 != str2) {
            warnUnsimplified(str1, str2);
        }
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

        // no crash on unary operator& (#5643)
        check("SdrObject* ApplyGraphicToObject() {\n"
              "    if (&rHitObject) {}\n"
              "    else if (rHitObject.IsClosedObj() && !&rHitObject) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5695: increment
        check("void f(int a0, int n) {\n"
              "  int c = a0 & 3;\n"
              "  for (int a = 0; a < n; a++) {\n"
              "    c++;\n"
              "    if (c == 4)\n"
              "      c  = 0;\n"
              "  }\n"
              "}\n");
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
              "    if ((x & 4) == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression '(X & 0x4) == 0x3' is always false.\n", errout.str());

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

        check("void foo(int x)\n"
              "{\n"
              "    if ((x & y & 4 & z ) == 3);\n"
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

        check("extern int bar() __attribute__((pure));\n"
              "void foo(int x)\n"
              "{\n"
              "    if ( bar() >1 && b) {}\n"
              "    else if (bar() >1 && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Expression is always false because 'else if' condition matches previous condition at line 4.\n", errout.str());

        checkPureFunction("extern int bar();\n"
                          "void foo(int x)\n"
                          "{\n"
                          "    if ( bar() >1 && b) {}\n"
                          "    else if (bar() >1 && b) {}\n"
                          "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Expression is always false because 'else if' condition matches previous condition at line 4.\n", errout.str());
    }

    void checkPureFunction(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        const char cfg[] = "<?xml version=\"1.0\"?>\n"
                           "<def>\n"
                           "  <function name=\"bar\"> <pure/> </function>\n"
                           "</def>";
        tinyxml2::XMLDocument xmldoc;
        xmldoc.Parse(cfg, sizeof(cfg));

        Settings settings;
        settings.addEnabled("style");
        settings.addEnabled("warning");
        settings.library.load(xmldoc);

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckCondition checkCondition;
        checkCondition.runChecks(&tokenizer, &settings, this);
        const std::string str1(tokenizer.tokens()->stringifyList(0,true));
        tokenizer.simplifyTokenList2();
        const std::string str2(tokenizer.tokens()->stringifyList(0,true));
        checkCondition.runSimplifiedChecks(&tokenizer, &settings, this);

        // Ensure that the test case is not bad.
        if (str1 != str2) {
            warnUnsimplified(str1, str2);
        }
    }
    void duplicateIf() {
        check("void f(int a, int &b) {\n"
              "    if (a) { b = 1; }\n"
              "    else { if (a) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a) { b = 1; }\n"
              "    else { if (a) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else { if (a == 2) { b = 2; }\n"
              "    else { if (a == 1) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else { if (a == 2) { b = 2; }\n"
              "    else { if (a == 2) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Expression is always false because 'else if' condition matches previous condition at line 3.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a++) { b = 1; }\n"
              "    else { if (a++) { b = 2; }\n"
              "    else { if (a++) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (!strtok(NULL, \" \")) { b = 1; }\n"
              "    else { if (!strtok(NULL, \" \")) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a, int &b) {\n"
              "   x = x / 2;\n"
              "   if (x < 100) { b = 1; }\n"
              "   else { x = x / 2; if (x < 100) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int i) {\n"
              "   if(i == 0x02e2000000 || i == 0xa0c6000000)\n"
              "       foo(i);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ticket 3689 ( avoid false positive )
        check("int fitInt(long long int nValue){\n"
              "    if( nValue < 0x7fffffffLL )\n"
              "    {\n"
              "        return 32;\n"
              "    }\n"
              "    if( nValue < 0x7fffffffffffLL )\n"
              "    {\n"
              "        return 48;\n"
              "    }\n"
              "    else {\n"
              "        if( nValue < 0x7fffffffffffffffLL )\n"
              "        {\n"
              "            return 64;\n"
              "        } else\n"
              "        {\n"
              "            return -1;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(WIDGET *widget) {\n"
              "  if (dynamic_cast<BUTTON*>(widget)){}\n"
              "  else if (dynamic_cast<LABEL*>(widget)){}\n"
              "}",false);
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n" // #6482
              "  if (x & 1) {}\n"
              "  else if (x == 0) {}\n"
              "}",false);
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "  if (x & 15) {}\n"
              "  else if (x == 40) {}\n"
              "}",false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());
    }

    void checkBadBitmaskCheck() {
        check("bool f(int x) {\n"
              "    bool b = x | 0x02;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("bool f(int x) {\n"
              "    bool b = 0x02 | x;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("int f(int x) {\n"
              "    int b = x | 0x02;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool f(int x) {\n"
              "    bool b = x & 0x02;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool f(int x) {\n"
              "    if(x | 0x02)\n"
              "        return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("bool f(int x) {\n"
              "    int y = 0x1;\n"
              "    if(b) y = 0;\n"
              "    if(x | y)\n"
              "        return b;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool f(int x) {\n"
              "    foo(a && (x | 0x02));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("int f(int x) {\n"
              "    return (x | 0x02) ? 0 : 5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("int f(int x) {\n"
              "    return x ? (x | 0x02) : 5;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool f(int x) {\n"
              "    return x | 0x02;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", "", errout.str());

        check("int f(int x) {\n"
              "    return x | 0x02;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void create_rop_masks_4( rop_mask_bits *bits) {\n"
              "DWORD mask_offset;\n"
              "BYTE *and_bits = bits->and;\n"
              "rop_mask *rop_mask;\n"
              "and_bits[mask_offset] |= (rop_mask->and & 0x0f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }


    void incorrectLogicOperator1() {
        check("void f(int x) {\n"
              "    if ((x != 1) || (x != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x != 1 || x != 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (1 != x || 3 != x)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x != 1 || x != 3.\n", errout.str());

        check("void f(int x) {\n" // ast..
              "    if (y == 1 && x == 1 && x == 7) { }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x == 1 && x == 7.\n", errout.str());

        check("void f(int x, int y) {\n"
              "    if (x != 1 || y != 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x, int y) {\n"
              "    if ((y == 1) && (x != 1) || (x != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x, int y) {\n"
              "    if ((x != 1) || (x != 3) && (y == 1))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x != 1) && (x != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x == 1) || (x == 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x, int y) {\n"
              "    if ((x != 1) || (y != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x, int y) {\n"
              "    if ((x != hotdog) || (y != hotdog))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x, int y) {\n"
              "    if ((x != 5) || (y != 5))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());


        check("void f(int x) {\n"
              "    if ((x != 5) || (x != 6))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x != 5 || x != 6.\n", errout.str());

        check("void f(unsigned int a, unsigned int b, unsigned int c) {\n"
              "    if((a != b) || (c != b) || (c != a))\n"
              "    {\n"
              "        return true;\n"
              "    }\n"
              "    return false;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void incorrectLogicOperator2() {
        check("void f(float x) {\n"
              "    if ((x == 1) && (x == 1.0))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x == 1) && (x == 0x00000001))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x == 1 && x == 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x == 1 && x == 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x == 1.0 && x == 3.0)\n"
              "        a++;\n"
              "}");
        //ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x == 1.0 && x == 3.0.\n", errout.str());
        ASSERT_EQUALS("", errout.str()); // float comparisons with == and != are not checked right now - such comparison is a bad idea

        check("void f(float x) {\n"
              "    if (x == 1 && x == 1.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void bar(float f) {\n" // #5246
              "    if ((f > 0) && (f < 1)) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1.0 && x > 1.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1.0 && x > 1.0.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 1.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 1.0.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 1.0 && x <= 1.001)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3.\n", errout.str());

        check("void f(float x) {\n"
              "    if (x < 1.0 && x > 3.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1.0 && x > 3.0.\n", errout.str());

        check("void f(int x) {\n"
              "    if (1 > x && 3 < x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 3 && x > 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x < 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x > 3 || x < 10.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x <= 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x >= 3 || x <= 10.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x < 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x >= 3 || x < 10.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x <= 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x > 3 || x <= 10.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x < 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x <= 3)\n"
              "        a++;\n"
              "}"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x >= 3 || x <= 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x < 3)\n"
              "        a++;\n"
              "}"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x >= 3 || x < 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x <= 3)\n"
              "        a++;\n"
              "}"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x > 3 || x <= 3.\n", errout.str());

        check("void f(int x) {\n"
              "   if((x==3) && (x!=4))\n"
              "        a++;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x == 3, the comparison x != 4 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x!=4) && (x==3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x == 3, the comparison x != 4 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x==3) || (x!=4))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x == 3, the comparison x != 4 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x!=4) || (x==3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x == 3, the comparison x != 4 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x==3) && (x!=3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x == 3 && x != 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x==6) || (x!=6))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: x == 6 || x != 6.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 10 || x < 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x > 5 && x == 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x > 5 && x == 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 5 && x == 6)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x == 6, the comparison x > 5 is always true.\n", errout.str());

        // #3419
        check("void f() {\n"
              "    if ( &q != &a && &q != &b ) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3676
        check("void f(int m_x2, int w, int x) {\n"
              "    if (x + w - 1 > m_x2 || m_x2 < 0 )\n"
              "        m_x2 = x + w - 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(float x) {\n" // x+1 => x
              "  if (x <= 1.0e20 && x >= -1.0e20) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(float x) {\n" // x+1 => x
              "  if (x >= 1.0e20 && x <= 1.0e21) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(float x) {\n" // x+1 => x
              "  if (x <= -1.0e20 && x >= -1.0e21) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void incorrectLogicOperator3() {
        check("void f(int x, bool& b) {\n"
              "    b = x > 5 && x == 1;\n"
              "    c = x < 1 && x == 3;\n"
              "    d = x >= 5 && x == 1;\n"
              "    e = x <= 1 && x == 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x > 5 && x == 1.\n"
                      "[test.cpp:3]: (warning) Logical conjunction always evaluates to false: x < 1 && x == 3.\n"
                      "[test.cpp:4]: (warning) Logical conjunction always evaluates to false: x >= 5 && x == 1.\n"
                      "[test.cpp:5]: (warning) Logical conjunction always evaluates to false: x <= 1 && x == 3.\n", errout.str());
    }

    void incorrectLogicOperator4() {
        check("void f(int x) {\n"
              "  if (x && x != $0) {}\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void incorrectLogicOperator5() { // complex expressions
        check("void f(int x) {\n"
              "  if (x+3 > 2 || x+3 < 10) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: EXPR > 2 || EXPR < 10.\n", errout.str());
    }

    void incorrectLogicOperator6() { // char literals
        check("void f(char x) {\n"
              "  if (x == '1' || x == '2') {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(char x) {\n"
              "  if (x == '1' && x == '2') {}\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());
    }

    void incorrectLogicOperator7() { // opposite expressions
        check("void f(int i) {\n"
              "  if (i || !i) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: i||!i.\n", errout.str());
    }

    void secondAlwaysTrueFalseWhenFirstTrueError() {
        check("void f(int x) {\n"
              "    if (x > 5 && x != 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x > 5, the comparison x != 1 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 5 && x != 6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x > 5) && (x != 1))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x > 5, the comparison x != 1 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x > 5) && (x != 6))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 3 || x == 4;\n"
              "    c = x < 5 || x == 4;\n"
              "    d = x >= 3 || x == 4;\n"
              "    e = x <= 5 || x == 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x == 4, the comparison x > 3 is always true.\n"
                      "[test.cpp:3]: (style) Redundant condition: If x == 4, the comparison x < 5 is always true.\n"
                      "[test.cpp:4]: (style) Redundant condition: If x == 4, the comparison x >= 3 is always true.\n"
                      "[test.cpp:5]: (style) Redundant condition: If x == 4, the comparison x <= 5 is always true.\n", errout.str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 5 || x != 1;\n"
              "    c = x < 1 || x != 3;\n"
              "    d = x >= 5 || x != 1;\n"
              "    e = x <= 1 || x != 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x > 5, the comparison x != 1 is always true.\n"
                      "[test.cpp:3]: (style) Redundant condition: If x < 1, the comparison x != 3 is always true.\n"
                      "[test.cpp:4]: (style) Redundant condition: If x >= 5, the comparison x != 1 is always true.\n"
                      "[test.cpp:5]: (style) Redundant condition: If x <= 1, the comparison x != 3 is always true.\n", errout.str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 6 && x > 5;\n"
              "    c = x > 5 || x > 6;\n"
              "    d = x < 6 && x < 5;\n"
              "    e = x < 5 || x < 6;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If x > 6, the comparison x > 5 is always true.\n"
                      "[test.cpp:3]: (style) Redundant condition: If x > 6, the comparison x > 5 is always true.\n"
                      "[test.cpp:4]: (style) Redundant condition: If x < 5, the comparison x < 6 is always true.\n"
                      "[test.cpp:5]: (style) Redundant condition: If x < 5, the comparison x < 6 is always true.\n", errout.str());
    }

    void incorrectLogicOp_condSwapping() {
        check("void f(int x) {\n"
              "    if (x < 1 && x > 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (1 > x && x > 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && 3 < x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (1 > x && 3 < x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 && x < 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (3 < x && x < 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 && 1 > x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (3 < x && 1 > x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1.\n", errout.str());
    }

    void modulo() {
        check("bool f(bool& b1, bool& b2, bool& b3) {\n"
              "    b1 = a % 5 == 4;\n"
              "    b2 = a % c == 100000;\n"
              "    b3 = a % 5 == c;\n"
              "    return a % 5 == 5-p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool f(bool& b1, bool& b2, bool& b3, bool& b4, bool& b5) {\n"
              "    b1 = a % 5 < 5;\n"
              "    b2 = a % 5 <= 5;\n"
              "    b3 = a % 5 == 5;\n"
              "    b4 = a % 5 != 5;\n"
              "    b5 = a % 5 >= 5;\n"
              "    return a % 5 > 5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n"
                      "[test.cpp:3]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n"
                      "[test.cpp:4]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n"
                      "[test.cpp:5]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n"
                      "[test.cpp:6]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n"
                      "[test.cpp:7]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n", errout.str());

        check("void f(bool& b1, bool& b2) {\n"
              "    b1 = bar() % 5 < 889;\n"
              "    if(x[593] % 5 <= 5)\n"
              "        b2 = x.a % 5 == 5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n"
                      "[test.cpp:3]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n"
                      "[test.cpp:4]: (warning) Comparison of modulo result is predetermined, because it is always less than 5.\n", errout.str());

        check("void f() {\n"
              "    if (a % 2 + b % 2 == 2)\n"
              "        foo();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeInnerCondition() {
        check("void foo(int a, int b) {\n"
              "    if(a==b)\n"
              "        if(a!=b)\n"
              "            cout << a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if(a==b)\n"
              "        if(b!=a)\n"
              "            cout << a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

        check("void foo(int a) {\n"
              "    if(a >= 50) {\n"
              "        if(a < 50)\n"
              "            cout << a;\n"
              "        else\n"
              "            cout << 100;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

        // #4186
        check("void foo(int a) {\n"
              "    if(a >= 50) {\n"
              "        if(a > 50)\n"
              "            cout << a;\n"
              "        else\n"
              "            cout << 100;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // 4170
        check("class foo {\n"
              "    void bar() {\n"
              "        if (tok == '(') {\n"
              "            next();\n"
              "            if (tok == ',') {\n"
              "                next();\n"
              "                if (tok != ',') {\n"
              "                    op->reg2 = asm_parse_reg();\n"
              "                }\n"
              "                skip(',');\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "    void next();\n"
              "    const char *tok;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int i)\n"
              "{\n"
              "   if(i > 5) {\n"
              "       i = bar();\n"
              "       if(i < 5) {\n"
              "           cout << a;\n"
              "       }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        check("void foo(int& i) {\n"
              "    i=6;\n"
              "}\n"
              "void bar(int i) {\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int& i);\n"
              "void bar() {\n"
              "    int i; i = func();\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int i);\n"
              "void bar(int i) {\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

        check("void foo(int i);\n"
              "void bar() {\n"
              "    int i; i = func();\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

        // see linux revision 1f80c0cc
        check("int generic_write_sync(int,int,int);\n"
              "\n"
              "void cifs_writev(int i) {\n"
              "   int rc = __generic_file_aio_write();\n"
              "   if (rc > 0){\n"
              "       err = generic_write_sync(file, iocb->ki_pos - rc, rc);\n"
              "       if(rc < 0) {\n"  // <- condition is always false
              "           err = rc;\n"
              "       }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:7]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

        check("void f(struct ABC *abc) {\n"
              "   struct AB *ab = abc->ab;\n"
              "   if (ab->a == 123){\n"
              "       do_something(abc);\n" // might change ab->a
              "       if (ab->a != 123) {\n"
              "           err = rc;\n"
              "       }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        {
            // #6095 - calling member function that might change the state
            check("void f() {\n"
                  "  const Fred fred;\n" // <- fred is const, warn
                  "  if (fred.isValid()) {\n"
                  "    fred.dostuff();\n"
                  "    if (!fred.isValid()) {}\n"
                  "  }\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

            check("class Fred { public: void dostuff() const; };\n"
                  "void f() {\n"
                  "  Fred fred;\n"
                  "  if (fred.isValid()) {\n"
                  "    fred.dostuff();\n" // <- dostuff() is const, warn
                  "    if (!fred.isValid()) {}\n"
                  "  }\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (warning) Opposite conditions in nested 'if' blocks lead to a dead code block.\n", errout.str());

            check("void f() {\n"
                  "  Fred fred;\n"
                  "  if (fred.isValid()) {\n"
                  "    fred.dostuff();\n"
                  "    if (!fred.isValid()) {}\n"
                  "  }\n"
                  "}");
            ASSERT_EQUALS("", errout.str());

            // #6385 "crash in Variable::getFlag()"
            check("class TranslationHandler {\n"
                  "QTranslator *mTranslator;\n"
                  "void SetLanguage() {\n"
                  "   if (mTranslator) {\n"
                  "             qApp->removeTranslator(mTranslator);\n"
                  "        }\n"
                  "   }\n"
                  "};");
            ASSERT_EQUALS("", errout.str()); // just don't crash...

        }

        // #5731 - fp when undeclared variable is used
        check("void f() {\n"
              "   if (x == -1){\n"
              "       x = do_something();\n"
              "       if (x != -1) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5750 - another fp when undeclared variable is used
        check("void f() {\n"
              "   if (r < w){\n"
              "       r += 3;\n"
              "       if (r > w) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5874 - array
        check("void testOppositeConditions2() {\n"
              "  int array[2] = { 0, 0 };\n"
              "  if (array[0] < 2) {\n"
              "    array[0] += 5;\n"
              "    if (array[0] > 2) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

// clarify conditions with = and comparison
    void clarifyCondition1() {
        check("void f() {\n"
              "    if (x = b() < 0) {}\n" // don't simplify and verify this code
              "}", false);
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (assignment + comparison); Clarify expression with parentheses.\n", errout.str());

        check("void f(int i) {\n"
              "    for (i = 0; i < 10; i++) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    x = a<int>(); if (x) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (x = b < 0 ? 1 : 2) {}\n" // don't simplify and verify this code
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

// clarify conditions with bitwise operator and comparison
    void clarifyCondition2() {
        check("void f() {\n"
              "    if (x & 3 == 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses.\n", errout.str());

        check("void f() {\n"
              "    if (a & fred1.x == fred2.y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses.\n", errout.str());
    }

// clarify condition that uses ! operator and then bitwise operator
    void clarifyCondition3() {
        check("void f(int w) {\n"
              "    if(!w & 0x8000) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n", errout.str());

        check("void f() {\n"
              "    if (x == foo() & 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n", errout.str());

        check("void f(std::list<int> &ints) { }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { A<x &> a; }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { a(x<y|z,0); }", true, "test.c");  // filename is c => there are never templates
        ASSERT_EQUALS("[test.c:1]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n", errout.str());

        check("class A<B&,C>;", true, "test.cpp");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (result != (char *)&inline_result) { }\n" // don't simplify and verify cast
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void clarifyCondition4() { // ticket #3110
        check("typedef double SomeType;\n"
              "typedef std::pair<std::string,SomeType> PairType;\n"
              "struct S\n"
              "{\n"
              "     bool operator()\n"
              "         ( PairType const & left\n"
              "         , PairType const & right) const\n"
              "     {\n"
              "         return left.first < right.first;\n"
              "     }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void clarifyCondition5() { // ticket #3609 (using | in template instantiation)
        check("CWinTraits<WS_CHILD|WS_VISIBLE>::GetWndStyle(0);");
        ASSERT_EQUALS("", errout.str());
    }

    void clarifyCondition6() {
        check("template<class Y>\n"
              "SharedPtr& operator=( SharedPtr<Y> const & r ) {\n"
              "    px = r.px;\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestCondition)
