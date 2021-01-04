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

#include "checkcondition.h"
#include "library.h"
#include "preprocessor.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <simplecpp.h>
#include <tinyxml2.h>
#include <map>
#include <vector>

class TestCondition : public TestFixture {
public:
    TestCondition() : TestFixture("TestCondition") {
    }

private:
    Settings settings0;
    Settings settings1;

    void run() OVERRIDE {
        LOAD_LIB_2(settings0.library, "qt.cfg");
        LOAD_LIB_2(settings0.library, "std.cfg");

        settings0.addEnabled("style");
        settings0.addEnabled("warning");

        const char cfg[] = "<?xml version=\"1.0\"?>\n"
        "<def>\n"
        "  <function name=\"bar\"> <pure/> </function>\n"
        "</def>";
        tinyxml2::XMLDocument xmldoc;
        xmldoc.Parse(cfg, sizeof(cfg));
        settings1.addEnabled("style");
        settings1.addEnabled("warning");
        settings1.library.load(xmldoc);

        TEST_CASE(assignAndCompare);   // assignment and comparison don't match
        TEST_CASE(mismatchingBitAnd);  // overlapping bitmasks
        TEST_CASE(comparison);         // CheckCondition::comparison test cases
        TEST_CASE(multicompare);       // mismatching comparisons
        TEST_CASE(overlappingElseIfCondition);  // overlapping conditions in if and else-if
        TEST_CASE(oppositeElseIfCondition); // opposite conditions in if and else-if

        TEST_CASE(checkBadBitmaskCheck);

        TEST_CASE(incorrectLogicOperator1);
        TEST_CASE(incorrectLogicOperator2);
        TEST_CASE(incorrectLogicOperator3);
        TEST_CASE(incorrectLogicOperator4);
        TEST_CASE(incorrectLogicOperator5); // complex expressions
        TEST_CASE(incorrectLogicOperator6); // char literals
        TEST_CASE(incorrectLogicOperator7); // opposite expressions: (expr || !expr)
        TEST_CASE(incorrectLogicOperator8); // !
        TEST_CASE(incorrectLogicOperator9);
        TEST_CASE(incorrectLogicOperator10); // enum
        TEST_CASE(incorrectLogicOperator11);
        TEST_CASE(incorrectLogicOperator12);
        TEST_CASE(incorrectLogicOperator13);
        TEST_CASE(incorrectLogicOperator14);
        TEST_CASE(secondAlwaysTrueFalseWhenFirstTrueError);
        TEST_CASE(incorrectLogicOp_condSwapping);
        TEST_CASE(testBug5895);
        TEST_CASE(testBug5309);

        TEST_CASE(modulo);

        TEST_CASE(oppositeInnerCondition);
        TEST_CASE(oppositeInnerConditionPointers);
        TEST_CASE(oppositeInnerConditionClass);
        TEST_CASE(oppositeInnerConditionUndeclaredVariable);
        TEST_CASE(oppositeInnerConditionAlias);
        TEST_CASE(oppositeInnerCondition2);
        TEST_CASE(oppositeInnerCondition3);
        TEST_CASE(oppositeInnerConditionAnd);
        TEST_CASE(oppositeInnerConditionEmpty);
        TEST_CASE(oppositeInnerConditionFollowVar);

        TEST_CASE(identicalInnerCondition);

        TEST_CASE(identicalConditionAfterEarlyExit);
        TEST_CASE(innerConditionModified);

        TEST_CASE(clarifyCondition1);     // if (a = b() < 0)
        TEST_CASE(clarifyCondition2);     // if (a & b == c)
        TEST_CASE(clarifyCondition3);     // if (! a & b)
        TEST_CASE(clarifyCondition4);     // ticket #3110
        TEST_CASE(clarifyCondition5);     // #3609 CWinTraits<WS_CHILD|WS_VISIBLE>..
        TEST_CASE(clarifyCondition6);     // #3818
        TEST_CASE(clarifyCondition7);
        TEST_CASE(clarifyCondition8);

        TEST_CASE(alwaysTrue);
        TEST_CASE(alwaysTrueInfer);
        TEST_CASE(multiConditionAlwaysTrue);
        TEST_CASE(duplicateCondition);

        TEST_CASE(checkInvalidTestForOverflow);
        TEST_CASE(checkConditionIsAlwaysTrueOrFalseInsideIfWhile);
        TEST_CASE(alwaysTrueFalseInLogicalOperators);
        TEST_CASE(pointerAdditionResultNotNull);
        TEST_CASE(duplicateConditionalAssign);
    }

    void check(const char code[], const char* filename = "test.cpp", bool inconclusive = false) {
        // Clear the error buffer..
        errout.str("");

        settings0.inconclusive = inconclusive;

        // Raw tokens..
        std::vector<std::string> files(1, filename);
        std::istringstream istr(code);
        const simplecpp::TokenList tokens1(istr, files, files[0]);

        // Preprocess..
        simplecpp::TokenList tokens2(files);
        std::map<std::string, simplecpp::TokenList*> filedata;
        simplecpp::preprocess(tokens2, tokens1, files, filedata, simplecpp::DUI());

        Preprocessor preprocessor(settings0, nullptr);
        preprocessor.setDirectives(tokens1);

        // Tokenizer..
        Tokenizer tokenizer(&settings0, this);
        tokenizer.createTokens(std::move(tokens2));
        tokenizer.simplifyTokens1("");
        tokenizer.setPreprocessor(&preprocessor);

        // Run checks..
        CheckCondition checkCondition;
        checkCondition.runChecks(&tokenizer, &settings0, this);
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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Mismatching assignment and comparison, comparison 'y==0x710' is always false.\n", errout.str());

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

        check("void f(int x) {\n"
              "    int a = 100;\n"
              "    while (x) {\n"
              "        int y = 16 | a;\n"
              "        while (y != 0) y--;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void g(int x);\n"
              "void f(int x) {\n"
              "    int a = 100;\n"
              "    while (x) {\n"
              "        int y = 16 | a;\n"
              "        while (y != 0) g(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5] -> [test.cpp:6]: (style) Mismatching assignment and comparison, comparison 'y!=0' is always true.\n",
            errout.str());

        check("void g(int &x);\n"
              "void f(int x) {\n"
              "    int a = 100;\n"
              "    while (x) {\n"
              "        int y = 16 | a;\n"
              "        while (y != 0) g(y);\n"
              "    }\n"
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

        check("void f(int a) {\n" // #6662
              "  int x = a & 1;\n"
              "  while (x <= 4) {\n"
              "    if (x != 5) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Mismatching assignment and comparison, comparison 'x!=5' is always true.\n", errout.str());

        check("void f(int a) {\n" // #6662
              "  int x = a & 1;\n"
              "  while ((x += 4) < 10) {\n"
              "    if (x != 5) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int x = 100;\n"
              "    while (x) {\n"
              "        g(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void g(int x);\n"
              "void f() {\n"
              "    int x = 100;\n"
              "    while (x) {\n"
              "        g(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition 'x' is always true\n", errout.str());

        check("void g(int & x);\n"
              "void f() {\n"
              "    int x = 100;\n"
              "    while (x) {\n"
              "        g(x);\n"
              "    }\n"
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

    void comparison() {
        // CheckCondition::comparison test cases
        // '=='
        check("void f(int a) {\n assert( (a & 0x07) == 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X & 0x7) == 0x8' is always false.\n",errout.str());
        check("void f(int a) {\n assert( (a & b & 4 & c ) == 3 );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X & 0x4) == 0x3' is always false.\n", errout.str());
        check("void f(int a) {\n assert( (a | 0x07) == 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X | 0x7) == 0x8' is always false.\n",errout.str());
        check("void f(int a) {\n assert( (a & 0x07) == 7U );\n}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int a) {\n assert( (a | 0x01) == -15 );\n}");
        ASSERT_EQUALS("", errout.str());
        // '!='
        check("void f(int a) {\n assert( (a & 0x07) != 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X & 0x7) != 0x8' is always true.\n",errout.str());
        check("void f(int a) {\n assert( (a | 0x07) != 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X | 0x7) != 0x8' is always true.\n",errout.str());
        check("void f(int a) {\n assert( (a & 0x07) != 7U );\n}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int a) {\n assert( (a | 0x07) != 7U );\n}");
        ASSERT_EQUALS("", errout.str());
        // '>='
        check("void f(int a) {\n assert( (a & 0x07) >= 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X & 0x7) >= 0x8' is always false.\n",errout.str());
        check("void f(unsigned int a) {\n assert( (a | 0x7) >= 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X | 0x7) >= 0x7' is always true.\n",errout.str());
        check("void f(int a) {\n assert( (a & 0x07) >= 7U );\n}");
        ASSERT_EQUALS("",errout.str());
        check("void f(int a) {\n assert( (a | 0x07) >= 8U );\n}");
        ASSERT_EQUALS("",errout.str()); //correct for negative 'a'
        // '>'
        check("void f(int a) {\n assert( (a & 0x07) > 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X & 0x7) > 0x7' is always false.\n",errout.str());
        check("void f(unsigned int a) {\n assert( (a | 0x7) > 6U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X | 0x7) > 0x6' is always true.\n",errout.str());
        check("void f(int a) {\n assert( (a & 0x07) > 6U );\n}");
        ASSERT_EQUALS("",errout.str());
        check("void f(int a) {\n assert( (a | 0x07) > 7U );\n}");
        ASSERT_EQUALS("",errout.str()); //correct for negative 'a'
        // '<='
        check("void f(int a) {\n assert( (a & 0x07) <= 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X & 0x7) <= 0x7' is always true.\n",errout.str());
        check("void f(unsigned int a) {\n assert( (a | 0x08) <= 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X | 0x8) <= 0x7' is always false.\n",errout.str());
        check("void f(int a) {\n assert( (a & 0x07) <= 6U );\n}");
        ASSERT_EQUALS("",errout.str());
        check("void f(int a) {\n assert( (a | 0x08) <= 7U );\n}");
        ASSERT_EQUALS("",errout.str()); //correct for negative 'a'
        // '<'
        check("void f(int a) {\n assert( (a & 0x07) < 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X & 0x7) < 0x8' is always true.\n",errout.str());
        check("void f(unsigned int a) {\n assert( (a | 0x07) < 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression '(X | 0x7) < 0x7' is always false.\n",errout.str());
        check("void f(int a) {\n assert( (a & 0x07) < 3U );\n}");
        ASSERT_EQUALS("",errout.str());
        check("void f(int a) {\n assert( (a | 0x07) < 7U );\n}");
        ASSERT_EQUALS("",errout.str()); //correct for negative 'a'
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

        // 7284
        check("void foo() {\n"
              "    if (a) {}\n"
              "    else if (!!a) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());
    }

    void checkPureFunction(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckCondition checkCondition;
        checkCondition.runChecks(&tokenizer, &settings1, this);
    }

    void overlappingElseIfCondition() {
        check("void f(int a, int &b) {\n"
              "    if (a) { b = 1; }\n"
              "    else { if (a) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Condition 'a' is always false\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a) { b = 1; }\n"
              "    else { if (a) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Condition 'a' is always false\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else { if (a == 2) { b = 2; }\n"
              "    else { if (a == 1) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Condition 'a==1' is always false\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else { if (a == 2) { b = 2; }\n"
              "    else { if (a == 2) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'a==2' is always false\n", errout.str());

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

        {
            check("void f(Class &c) {\n"
                  "    if (c.dostuff() == 3) {}\n"
                  "    else { if (c.dostuff() == 3) {} }\n"
                  "}");
            ASSERT_EQUALS("", errout.str());

            check("void f(const Class &c) {\n"
                  "    if (c.dostuff() == 3) {}\n"
                  "    else { if (c.dostuff() == 3) {} }\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());
        }

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
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n" // #6482
              "  if (x & 1) {}\n"
              "  else if (x == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "  if (x & 15) {}\n"
              "  else if (x == 40) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(int x) {\n"
              "  if (x == sizeof(double)) {}\n"
              "  else { if (x == sizeof(long double)) {} }"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "  if (x & 0x08) {}\n"
              "  else if (x & 0xF8) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "  if (x & 0xF8) {}\n"
              "  else if (x & 0x08) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!b && !!a){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!b && a){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( b && !!a){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( b && !(!a)){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!b && !(!a)){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Expression is always false because 'else if' condition matches previous condition at line 2.\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!(b) && !!(a+b)){}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #8168
        check("enum MaskValues\n"
              "{\n"
              "    Value1 = 0x00000001,\n"
              "    Value2 = 0x00000002\n"
              "};\n"
              "void TestFunction(int value) {\n"
              "    if ( value & (int)Value1 ) {}\n"
              "    else if ( value & (int)Value2 ) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(size_t x) {\n"
              "    if (x == sizeof(int)) {}\n"
              "    else { if (x == sizeof(long))} {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(size_t x) {\n"
              "    if (x == sizeof(long)) {}\n"
              "    else { if (x == sizeof(long long))} {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeElseIfCondition() {
        setMultiline();

        check("void f(int x) {\n"
              "    if (x) {}\n"
              "    else if (!x) {}\n"
              "}");
        ASSERT_EQUALS("test.cpp:3:style:Condition '!x' is always true\n"
                      "test.cpp:2:note:condition 'x'\n"
                      "test.cpp:3:note:Condition '!x' is always true\n", errout.str());

        check("void f(int x) {\n"
              "    int y = x;\n"
              "    if (x) {}\n"
              "    else if (!y) {}\n"
              "}");
        ASSERT_EQUALS("test.cpp:4:style:Expression is always true because 'else if' condition is opposite to previous condition at line 3.\n"
                      "test.cpp:2:note:'y' is assigned value 'x' here.\n"
                      "test.cpp:3:note:first condition\n"
                      "test.cpp:4:note:else if condition is opposite to first condition\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("bool f(int x) {\n"
              "  if (x) {\n"
              "    return x | 0x02;\n"
              "  }\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("const bool f(int x) {\n"
              "    return x | 0x02;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("struct F {\n"
              "  static const bool f(int x) {\n"
              "      return x | 0x02;\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

        check("struct F {\n"
              "  typedef bool b_t;\n"
              "};\n"
              "F::b_t f(int x) {\n"
              "  return x | 0x02;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?\n", errout.str());

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

        check("void f(unsigned a, unsigned b) {\n"
              "  unsigned cmd1 = b & 0x0F;\n"
              "  if (cmd1 | a) {\n"
              "    if (b == 0x0C) {}\n"
              "  }\n"
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

        check("void f(int x) {\n"
              "  if (x<0 && !x) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x < 0 && !x.\n", errout.str());

        check("void f(int x) {\n"
              "  if (x==0 && x) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x == 0 && x.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Condition 'x!=3' is always true\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Condition 'x==0x00000001' is always true\n", errout.str());

        check("void f(int x) {\n"
              "    if (x == 1 && x == 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x == 1 && x == 3.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x == 1.0 && x == 3.0)\n"
              "        a++;\n"
              "}");
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

        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x == 3', the comparison 'x != 4' is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x!=4) && (x==3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x == 3', the comparison 'x != 4' is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x==3) || (x!=4))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x == 3', the comparison 'x != 4' is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x!=4) || (x==3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x == 3', the comparison 'x != 4' is always true.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x == 6', the comparison 'x > 5' is always true.\n", errout.str());

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
        check("#define ZERO 0\n"
              "void f(int x) {\n"
              "  if (x && x != ZERO) {}\n"
              "}");
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
              "}", "test.cpp", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(char x) {\n"
              "  if (x == '1' && x == '2') {}\n"
              "}", "test.cpp", true);
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: x == '1' && x == '2'.\n", errout.str());

        check("int f(char c) {\n"
              "  return (c >= 'a' && c <= 'z');\n"
              "}", "test.cpp", true);
        ASSERT_EQUALS("", errout.str());

        check("int f(char c) {\n"
              "  return (c <= 'a' && c >= 'z');\n"
              "}", "test.cpp", true);
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Logical conjunction always evaluates to false: c <= 'a' && c >= 'z'.\n", errout.str());

        check("int f(char c) {\n"
              "  return (c <= 'a' && c >= 'z');\n"
              "}", "test.cpp", false);
        ASSERT_EQUALS("", errout.str());
    }

    void incorrectLogicOperator7() { // opposite expressions
        check("void f(int i) {\n"
              "  if (i || !i) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: i || !(i).\n", errout.str());

        check("void f(int a, int b) {\n"
              "  if (a>b || a<=b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical disjunction always evaluates to true: a > b || a <= b.\n", errout.str());

        check("void f(int a, int b) {\n"
              "  if (a>b || a<b) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6064 False positive incorrectLogicOperator - invalid assumption about template type?
        check("template<typename T> T icdf( const T uniform ) {\n"
              "   if ((0<uniform) && (uniform<1))\n"
              "     {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6081 False positive: incorrectLogicOperator, with close negative comparisons
        check("double neg = -1.0 - 1.0e-13;\n"
              "void foo() {\n"
              "    if ((neg < -1.0) && (neg > -1.0 - 1.0e-12))\n"
              "        return;\n"
              "    else\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void incorrectLogicOperator8() { // opposite expressions
        check("void f(int i) {\n"
              "  if (!(i!=10) && !(i!=20)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Logical conjunction always evaluates to false: !(i != 10) && !(i != 20).\n", errout.str());
    }

    void incorrectLogicOperator9() { //  #6069 "False positive incorrectLogicOperator due to dynamic_cast"
        check("class MyType;\n"
              "class OtherType;\n"
              "void foo (OtherType* obj) { \n"
              "    assert((!obj) || dynamic_cast<MyType*>(obj));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void incorrectLogicOperator10() { //  #7794 - enum
        check("typedef enum { A, B } Type_t;\n"
              "void f(Type_t t) {\n"
              "    if ((t == A) && (t == B))\n"
              "    {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Logical conjunction always evaluates to false: t == 0 && t == 1.\n", errout.str());
    }

    void incorrectLogicOperator11() {
        check("void foo(int i, const int n) { if ( i < n && i == n ) {} }");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Logical conjunction always evaluates to false: i < n && i == n.\n", errout.str());

        check("void foo(int i, const int n) { if ( i > n && i == n ) {} }");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Logical conjunction always evaluates to false: i > n && i == n.\n", errout.str());

        check("void foo(int i, const int n) { if ( i == n && i > n ) {} }");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Logical conjunction always evaluates to false: i == n && i > n.\n", errout.str());

        check("void foo(int i, const int n) { if ( i == n && i < n ) {} }");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Logical conjunction always evaluates to false: i == n && i < n.\n", errout.str());
    }

    void incorrectLogicOperator12() { // #8696
        check("struct A {\n"
              "    void f() const;\n"
              "};\n"
              "void foo(A a) {\n"
              "  A x = a;\n"
              "  A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:6] -> [test.cpp:8]: (warning) Logical conjunction always evaluates to false: a > x && a < y.\n", errout.str());

        check("struct A {\n"
              "    void f();\n"
              "};\n"
              "void foo(A a) {\n"
              "  A x = a;\n"
              "  A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(A a) {\n"
              "  A x = a;\n"
              "  A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(A a) {\n"
              "  const A x = a;\n"
              "  const A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3] -> [test.cpp:5]: (warning) Logical conjunction always evaluates to false: a > x && a < y.\n", errout.str());
    }

    void incorrectLogicOperator13() {
        // 8780
        check("void f(const int &v) {\n"
              "    const int x=v;\n"
              "    if ((v == 1) && (x == 2)) {;}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Logical conjunction always evaluates to false: v == 1 && x == 2.\n", errout.str());

        check("void f2(const int *v) {\n"
              "    const int *x=v;\n"
              "    if ((*v == 1) && (*x == 2)) {;}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Logical conjunction always evaluates to false: *(v) == 1 && *(x) == 2.\n", errout.str());
    }

    void incorrectLogicOperator14() {
        check("static const std ::string h;\n"
              "class i {\n"
              "public:\n"
              "  struct j {\n"
              "    std ::string k;\n"
              "    std ::string l;\n"
              "  };\n"
              "  struct a {\n"
              "    enum { m = 1 };\n"
              "  };\n"
              "} b;\n"
              "namespace n {\n"
              "class c;\n"
              "}\n"
              "struct o {\n"
              "  enum { p, d, q, r };\n"
              "  enum { e, f };\n"
              "\n"
              "public:\n"
              "  class j {\n"
              "  public:\n"
              "    class s {\n"
              "      std ::string a;\n"
              "    };\n"
              "  };\n"
              "};\n"
              "namespace n {\n"
              "class b;\n"
              "}\n"
              "namespace aa {\n"
              "class d {\n"
              "public:\n"
              "  char t;\n"
              "  enum {} u;\n"
              "};\n"
              "} // namespace aa\n"
              "namespace aa {\n"
              "struct e {};\n"
              "} // namespace aa\n"
              "class a;\n"
              "class w {\n"
              "public:\n"
              "  enum { x };\n"
              "  struct {\n"
              "  } y;\n"
              "  std ::string z;\n"
              "};\n"
              "class ab {\n"
              "  friend class c;\n"
              "\n"
              "public:\n"
              "  class ac {\n"
              "    void e(const ac &v) const;\n"
              "  };\n"
              "};\n"
              "class f;\n"
              "class ad {\n"
              "  friend class e;\n"
              "  enum { e, ae, ag, ah, ai, aj, ak, a, b };\n"
              "  class c {};\n"
              "  class d {\n"
              "    enum am { f, an, ao, ap, aq, ar, b, as, at, c, au };\n"
              "    enum av { aw, ax, ay, az, e, ba, bb, bc, bd, a };\n"
              "    struct b {\n"
              "      am action;\n"
              "      av c;\n"
              "    };\n"
              "  };\n"
              "  class e {\n"
              "  public:\n"
              "    std ::string e;\n"
              "    class f {\n"
              "    } f;\n"
              "    class be {\n"
              "    public:\n"
              "    };\n"
              "    std ::vector<be> bf;\n"
              "    enum { bg, b } c;\n"
              "  };\n"
              "  struct bh {\n"
              "    std ::map<int, d> b;\n"
              "  };\n"
              "  std ::map<std ::string, bh> bi;\n"
              "  struct {\n"
              "    int b;\n"
              "    char bj;\n"
              "  } bk;\n"
              "  class a {\n"
              "  public:\n"
              "    std ::set<std ::string> b;\n"
              "  };\n"
              "};\n"
              "class bl;\n"
              "class al;\n"
              "class bm;\n"
              "class f;\n"
              "class b;\n"
              "class bn;\n"
              "namespace bo {\n"
              "class bp {\n"
              "public:\n"
              "  typedef std ::pair<const f *, std ::string> bq;\n"
              "  typedef std ::list<bq> br;\n"
              "};\n"
              "const bo ::bp *dg(const f *a, const al *b);\n"
              "} // namespace bo\n"
              "const bn *dh(const f *d, bo ::bp ::br &bs);\n"
              "class f {\n"
              "public:\n"
              "  struct bt {};\n"
              "  std ::vector<a> f;\n"
              "};\n"
              "class bu;\n"
              "class a;\n"
              "class c;\n"
              "struct bv {};\n"
              "class af {\n"
              "private:\n"
              "public:\n"
              "  enum { b, d, e, f, c, bw };\n"
              "  void a(int c);\n"
              "  af *bx() const;\n"
              "};\n"
              "namespace by {\n"
              "class b;\n"
              "}\n"
              "class b {\n"
              "public:\n"
              "  bool d, c;\n"
              "};\n"
              "class bz;\n"
              "class f;\n"
              "class ca {\n"
              "  friend class b;\n"
              "\n"
              "public:\n"
              "  const bm *cb() const { return cc; }\n"
              "  f *d(f *e, bool f) const;\n"
              "  int e() { return ++cd; }\n"
              "  bl *const c;\n"
              "  bm *cc;\n"
              "  std ::map<std ::string, int> ce;\n"
              "  int cd;\n"
              "  bz *a;\n"
              "};\n"
              "namespace n {\n"
              "class c;\n"
              "class d;\n"
              "} // namespace n\n"
              "class cf {\n"
              "public:\n"
              "  explicit cf(const std ::string &aname);\n"
              "  cf(const std ::string &aname, const ca *cg, const al *ch, bl *ci)\n"
              "      : cj(cg), ck(ch), cl(ci), cn(aname) {}\n"
              "\n"
              "protected:\n"
              "  const ca *const cj;\n"
              "  const al *const ck;\n"
              "  bl *const cl;\n"
              "  const std ::string cn;\n"
              "};\n"
              "class cm : public cf {\n"
              "public:\n"
              "  void cp();\n"
              "  std ::string d() const;\n"
              "};\n"
              "struct co {\n"
              "  co();\n"
              "  const bu *a;\n"
              "  enum f {};\n"
              "  enum {\n"
              "    b = (1 << 0),\n"
              "    c = (1 << 1),\n"
              "  };\n"
              "  void d(bool e);\n"
              "};\n"
              "class bu {\n"
              "  friend class e;\n"
              "\n"
              "public:\n"
              "  struct f {};\n"
              "  enum { d, cr, cq, ct, cs, e, a, b, c, dd, cu, cv, cw, cx, cy, cz, da };\n"
              "  const f *db;\n"
              "  const af *dc;\n"
              "} f{};\n"
              "class bm {\n"
              "public:\n"
              "  std ::list<bu> df;\n"
              "  std ::vector<const bu *> de;\n"
              "  mutable std ::set<std ::string> f;\n"
              "};\n"
              "void cm ::cp() {\n"
              "  const bm *a = cj->cb();\n"
              "  for (const bu *b : a->de)\n"
              "    for (af *c = b->dc->bx();;) {\n"
              "      af *d = c;\n"
              "      af *e = c;\n"
              "      bool f(d);\n"
              "      bool g(e);\n"
              "      if (f && g)\n"
              "        ;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void secondAlwaysTrueFalseWhenFirstTrueError() {
        check("void f(int x) {\n"
              "    if (x > 5 && x != 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x > 5', the comparison 'x != 1' is always true.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x > 5', the comparison 'x != 1' is always true.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x == 4', the comparison 'x > 3' is always true.\n"
                      "[test.cpp:3]: (style) Redundant condition: If 'x == 4', the comparison 'x < 5' is always true.\n"
                      "[test.cpp:4]: (style) Redundant condition: If 'x == 4', the comparison 'x >= 3' is always true.\n"
                      "[test.cpp:5]: (style) Redundant condition: If 'x == 4', the comparison 'x <= 5' is always true.\n", errout.str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 5 || x != 1;\n"
              "    c = x < 1 || x != 3;\n"
              "    d = x >= 5 || x != 1;\n"
              "    e = x <= 1 || x != 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x > 5', the comparison 'x != 1' is always true.\n"
                      "[test.cpp:3]: (style) Redundant condition: If 'x < 1', the comparison 'x != 3' is always true.\n"
                      "[test.cpp:4]: (style) Redundant condition: If 'x >= 5', the comparison 'x != 1' is always true.\n"
                      "[test.cpp:5]: (style) Redundant condition: If 'x <= 1', the comparison 'x != 3' is always true.\n", errout.str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 6 && x > 5;\n"
              "    c = x > 5 || x > 6;\n"
              "    d = x < 6 && x < 5;\n"
              "    e = x < 5 || x < 6;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: If 'x > 6', the comparison 'x > 5' is always true.\n"
                      "[test.cpp:3]: (style) Redundant condition: If 'x > 6', the comparison 'x > 5' is always true.\n"
                      "[test.cpp:4]: (style) Redundant condition: If 'x < 5', the comparison 'x < 6' is always true.\n"
                      "[test.cpp:5]: (style) Redundant condition: If 'x < 5', the comparison 'x < 6' is always true.\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("bool foo(int a, int b) {\n"
              "    if(a==b)\n"
              "        return a!=b;\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'return' condition leads to a dead code block.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if(a==b)\n"
              "        if(b!=a)\n"
              "            cout << a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void foo(int a) {\n"
              "    if(a >= 50) {\n"
              "        if(a < 50)\n"
              "            cout << a;\n"
              "        else\n"
              "            cout << 100;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void foo(const int &i);\n"
              "void bar(int i) {\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void foo(int i);\n"
              "void bar() {\n"
              "    int i; i = func();\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("class C { void f(int &i) const; };\n" // #7028 - variable is changed by const method
              "void foo(C c, int i) {\n"
              "  if (i==5) {\n"
              "    c.f(i);\n"
              "    if (i != 5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:7]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());


        // #5874 - array
        check("void testOppositeConditions2() {\n"
              "  int array[2] = { 0, 0 };\n"
              "  if (array[0] < 2) {\n"
              "    array[0] += 5;\n"
              "    if (array[0] > 2) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6227 - FP caused by simplifications of casts and known variables
        check("void foo(A *a) {\n"
              "   if(a) {\n"
              "       B *b = dynamic_cast<B*>(a);\n"
              "       if(!b) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a) {\n"
              "   if(a) {\n"
              "       int b = a;\n"
              "       if(!b) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:4]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void foo(unsigned u) {\n"
              "  if (u != 0) {\n"
              "    for (int i=0; i<32; i++) {\n"
              "      if (u == 0) {}\n"  // <- don't warn
              "      u = x;\n"
              "    }\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #8186
        check("void f() {\n"
              "  for (int i=0;i<4;i++) {\n"
              "    if (i==5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        // #8938
        check("void Delete(SS_CELLCOORD upperleft) {\n"
              "    if ((upperleft.Col == -1) && (upperleft.Row == -1)) {\n"
              "        GetActiveCell(&(upperleft.Col), &(upperleft.Row));\n"
              "        if (upperleft.Row == -1) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeInnerConditionPointers() {
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

        check("void Fred::f() {\n" // daca: ace
              "  if (this->next_ == map_man_->table_) {\n"
              "    this->next_ = n;\n"
              "    if (this->next_ != map_man_->table_) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test(float *f) {\n" // #7405
              "  if(*f>10) {\n"
              "    (*f) += 0.1f;\n"
              "    if(*f<10) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int * f(int * x, int * y) {\n"
              "    if(!x) return x;\n"
              "    return y;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeInnerConditionClass() {
        // #6095 - calling member function that might change the state
        check("void f() {\n"
              "  const Fred fred;\n" // <- fred is const, warn
              "  if (fred.isValid()) {\n"
              "    fred.dostuff();\n"
              "    if (!fred.isValid()) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("class Fred { public: bool isValid() const; void dostuff() const; };\n"
              "void f() {\n"
              "  Fred fred;\n"
              "  if (fred.isValid()) {\n"
              "    fred.dostuff();\n" // <- dostuff() is const, warn
              "    if (!fred.isValid()) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

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

        check("bool f(std::ofstream &CFileStream) {\n" // #8198
              "  if(!CFileStream.good()) { return; }\n"
              "  CFileStream << \"abc\";\n"
              "  if (!CFileStream.good()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeInnerConditionUndeclaredVariable() {
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

        // #6574 - another fp when undeclared variable is used
        check("void foo() {\n"
              "   if(i) {\n"
              "       i++;\n"
              "       if(!i) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // undeclared array
        check("void f(int x) {\n"
              "  if (a[x] > 0) {\n"
              "    a[x] -= dt;\n"
              "    if (a[x] < 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6313 - false positive: opposite conditions in nested if blocks when condition changed
        check("void Foo::Bar() {\n"
              "   if(var){\n"
              "      --var;\n"
              "      if(!var){}\n"
              "      else {}\n"
              "   }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // daca hyphy
        check("bool f() {\n"
              "  if (rec.lLength==0) {\n"
              "    rec.Delete(i);\n"
              "    if (rec.lLength!=0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeInnerConditionAlias() {
        check("void f() {\n"
              "  struct S s;\n"
              "  bool hasFailed = false;\n"
              "  s.status = &hasFailed;\n"
              "\n"
              "  if (! hasFailed) {\n"
              "    doStuff(&s);\n"
              "    if (hasFailed) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Condition '!hasFailed' is always true\n", errout.str());
    }

    void oppositeInnerCondition2() {
        // first comparison: <
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x==5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Opposite inner 'if' condition leads to a dead code block.\n",
                      errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x!=5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x!=5' is always true\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x>5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Opposite inner 'if' condition leads to a dead code block.\n",
                      errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x>=5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Opposite inner 'if' condition leads to a dead code block.\n",
                      errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x<5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x<5' is always true\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x<=5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x<=5' is always true\n", errout.str());

        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x==4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x!=4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x!=6) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x!=6' is always true\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x>4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x>4' is always false\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x>=4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x<4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x<=4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x<=4' is always true\n", errout.str());

        // first comparison: >
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x==5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x>5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x>=5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x>=5' is always true\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x<5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x<5' is always false\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x<=5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x==4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Opposite inner 'if' condition leads to a dead code block.\n",
                      errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x>4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x>4' is always true\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x>=4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Condition 'x>=4' is always true\n", errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x<4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Opposite inner 'if' condition leads to a dead code block.\n",
                      errout.str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x<=4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Opposite inner 'if' condition leads to a dead code block.\n",
                      errout.str());

        check("void f(int x) {\n"
              "  if (x < 4) {\n"
              "    if (10 < x) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());
    }

    void oppositeInnerCondition3() {
        check("void f3(char c) { if(c=='x') if(c=='y') {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f4(char *p) { if(*p=='x') if(*p=='y') {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f5(const char * const p) { if(*p=='x') if(*p=='y') {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f5(const char * const p) { if('x'==*p) if('y'==*p) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f6(char * const p) { if(*p=='x') if(*p=='y') {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f7(const char * p) { if(*p=='x') if(*p=='y') {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f8(int i) { if(i==4) if(i==2) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f9(int *p) { if (*p==4) if(*p==2) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f10(int * const p) { if (*p==4) if(*p==2) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f11(const int *p) { if (*p==4) if(*p==2) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f12(const int * const p) { if (*p==4) if(*p==2) {}}");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("struct foo {\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "void f(foo x) { if(x.a==4) if(x.b==2) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("struct foo {\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "void f(foo x) { if(x.a==4) if(x.b==4) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f3(char a, char b) { if(a==b) if(a==0) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) { if (x == 1) if (x != 1) {} }");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());
    }

    void oppositeInnerConditionAnd() {
        check("void f(int x) {\n"
              "  if (a>3 && x > 100) {\n"
              "    if (x < 10) {}\n"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f(bool x, const int a, const int b) {\n"
              "        if(x && a < b)\n"
              "            if( x && a > b){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());
    }

    void oppositeInnerConditionEmpty() {
        check("void f1(const std::string &s) { if(s.size() > 42) if(s.empty()) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f1(const std::string &s) { if(s.size() > 0) if(s.empty()) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f1(const std::string &s) { if(s.size() < 0) if(s.empty()) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f1(const std::string &s) { if(s.empty()) if(s.size() > 42) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("template<class T> void f1(const T &s) { if(s.size() > 42) if(s.empty()) {}} ");
        ASSERT_EQUALS("", errout.str()); //We don't know the type of T so we don't know the relationship between size() and empty(). e.g. s might be a 50 tonne truck with nothing in it.

        check("void f2(const std::wstring &s) { if(s.empty()) if(s.size() > 42) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f1(QString s) { if(s.isEmpty()) if(s.length() > 42) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Opposite inner 'if' condition leads to a dead code block.\n", errout.str());

        check("void f1(const std::string &s, bool b) { if(s.empty() || ((s.size() == 1) && b)) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string &x, const std::string &y) { if(x.size() > 42) if(y.empty()) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string &x, const std::string &y) { if(y.empty()) if(x.size() > 42) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string v[10]) { if(v[0].size() > 42) if(v[1].empty()) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string &s) { if(s.size() <= 1) if(s.empty()) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string &s) { if(s.size() <= 2) if(s.empty()) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string &s) { if(s.size() < 2) if(s.empty()) {}} ");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string &s) { if(s.size() >= 0) if(s.empty()) {}} ");
        ASSERT_EQUALS("", errout.str());

        // TODO: These are identical condition since size cannot be negative
        check("void f1(const std::string &s) { if(s.size() <= 0) if(s.empty()) {}} ");
        ASSERT_EQUALS("", errout.str());

        // TODO: These are identical condition since size cannot be negative
        check("void f1(const std::string &s) { if(s.size() < 1) if(s.empty()) {}} ");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeInnerConditionFollowVar() {
        check("struct X {\n"
              "    void f() {\n"
              "        const int flag = get();\n"
              "        if (flag) {\n"
              "            bar();\n"
              "            if (!get()) {}\n"
              "        }\n"
              "    }\n"
              "    void bar();\n"
              "    int get() const;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class C {\n"
              "public:\n"
              "  bool f() const { return x > 0; }\n"
              "  void g();\n"
              "  int x = 0;\n"
              "};\n"
              "\n"
              "void C::g() {\n"
              "  bool b = f();\n"
              "  x += 1;\n"
              "  if (!b && f()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void identicalInnerCondition() {
        check("void f1(int a, int b) { if(a==b) if(a==b) {}}");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Identical inner 'if' condition is always true.\n", errout.str());

        check("void f2(int a, int b) { if(a!=b) if(a!=b) {}}");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (warning) Identical inner 'if' condition is always true.\n", errout.str());

        // #6645 false negative: condition is always false
        check("void f(bool a, bool b) {\n"
              "  if(a && b) {\n"
              "     if(a) {}\n"
              "     else  {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Identical inner 'if' condition is always true.\n", errout.str());

        check("bool f(int a, int b) {\n"
              "    if(a == b) { return a == b; }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (warning) Identical inner 'return' condition is always true.\n", errout.str());

        check("bool f(bool a) {\n"
              "    if(a) { return a; }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int* f(int* a, int * b) {\n"
              "    if(a) { return a; }\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int* f(std::shared_ptr<int> a, std::shared_ptr<int> b) {\n"
              "    if(a.get()) { return a.get(); }\n"
              "    return b.get();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A { int * x; };\n"
              "int* f(A a, int * b) {\n"
              "    if(a.x) { return a.x; }\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    uint32_t value;\n"
              "    get_value(&value);\n"
              "    int opt_function_capable = (value >> 28) & 1;\n"
              "    if (opt_function_capable) {\n"
              "        value = 0;\n"
              "        get_value (&value);\n"
              "        if ((value >> 28) & 1) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void identicalConditionAfterEarlyExit() {
        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  if (x > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Identical condition 'x>100', second condition is always false\n", errout.str());

        check("bool f(int x) {\n"
              "  if (x > 100) { return false; }\n"
              "  return x > 100;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Identical condition and return expression 'x>100', return value is always false\n", errout.str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  if (x > 100 || y > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Identical condition 'x>100', second condition is always false\n", errout.str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  if (x > 100 && y > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Identical condition 'x>100', second condition is always false\n", errout.str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  if (abc) {}\n"
              "  if (x > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (warning) Identical condition 'x>100', second condition is always false\n", errout.str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  while (abc) { y = x; }\n"
              "  if (x > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (warning) Identical condition 'x>100', second condition is always false\n", errout.str());

        check("void f(int x) {\n"  // #8217 - crash for incomplete code
              "  if (x > 100) { return; }\n"
              "  X(do);\n"
              "  if (x > 100) {}\n"
              "}");
        // TODO: we should probably throw unknownMacro InternalError. Complain that the macro X must be defined. We can't check the code well without the definition.
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Condition 'x>100' is always false\n", errout.str());

        check("void f(const int *i) {\n"
              "  if (!i) return;\n"
              "  if (!num1tok) { *num1 = *num2; }\n"
              "  if (!i) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (warning) Identical condition '!i', second condition is always false\n", errout.str());

        check("void C::f(Tree &coreTree) {\n" // daca
              "  if(!coreTree.build())\n"
              "    return;\n"
              "  coreTree.dostuff();\n"
              "  if(!coreTree.build()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct C { void f(const Tree &coreTree); };\n"
              "void C::f(const Tree &coreTree) {\n"
              "  if(!coreTree.build())\n"
              "    return;\n"
              "  coreTree.dostuff();\n"
              "  if(!coreTree.build()) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (warning) Identical condition '!coreTree.build()', second condition is always false\n", errout.str());

        check("void f(int x) {\n" // daca: labplot
              "  switch(type) {\n"
              "  case 1:\n"
              "    if (x == 0) return 1;\n"
              "    else return 2;\n"
              "  case 2:\n"
              "    if (x == 0) return 3;\n"
              "    else return 4;\n"
              "  }\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static int failed = 0;\n"
              "void f() {\n"
              "  if (failed) return;\n"
              "  checkBuffer();\n"
              "  if (failed) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // daca icu
        check("void f(const uint32_t *section, int32_t  start) {\n"
              "  if(10<=section[start]) { return; }\n"
              "  if(++start<100 && 10<=section[start]) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // daca iqtree
        check("void readNCBITree(std::istream &in) {\n"
              "  char ch;\n"
              "  in >> ch;\n"
              "  if (ch != '|') return;\n"
              "  in >> ch;\n"
              "  if (ch != '|') {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #8924
        check("struct A {\n"
              "    void f() {\n"
              "        if (this->FileIndex >= 0) return;\n"
              "        this->FileIndex = 1 ;\n"
              "        if (this->FileIndex < 0) return;\n"
              "    }\n"
              "    int FileIndex;  \n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        // #8858 - #if
        check("short Do() {\n"
              "    short ret = bar1();\n"
              "    if ( ret )\n"
              "        return ret;\n"
              "#ifdef FEATURE\n"
              "    ret = bar2();\n"
              "#endif\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void innerConditionModified() {
        check("void f(int x, int y) {\n"
              "  if (x == 0) {\n"
              "    x += y;\n"
              "    if (x == 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "  if (x == 0) {\n"
              "    x += y;\n"
              "    if (x == 1) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int * x, int * y) {\n"
              "  if (x[*y] == 0) {\n"
              "    (*y)++;\n"
              "    if (x[*y] == 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // clarify conditions with = and comparison
    void clarifyCondition1() {
        check("void f() {\n"
              "    if (x = b() < 0) {}\n" // don't simplify and verify this code
              "}");
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
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int y = rand(), z = rand();\n"
              "    if (y || (!y && z));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition: !y. 'y || (!y && z)' is equivalent to 'y || z'\n", errout.str());

        check("void f() {\n"
              "    int y = rand(), z = rand();\n"
              "    if (y || !y && z);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition: !y. 'y || (!y && z)' is equivalent to 'y || z'\n", errout.str());

        check("void f() {\n"
              "    if (!a || a && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: a. '!a || (a && b)' is equivalent to '!a || b'\n", errout.str());


        check("void f(const Token *tok) {\n"
              "    if (!tok->next()->function() || \n"
              "        (tok->next()->function() && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: tok->next()->function(). '!A || (A && B)' is equivalent to '!A || B'\n", errout.str());

        check("void f() {\n"
              "    if (!tok->next()->function() || \n"
              "        (!tok->next()->function() && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (!tok->next()->function() || \n"
              "        (!tok2->next()->function() && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const Token *tok) {\n"
              "    if (!tok->next(1)->function(1) || \n"
              "        (tok->next(1)->function(1) && tok->next(1)->function(1)->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: tok->next(1)->function(1). '!A || (A && B)' is equivalent to '!A || B'\n", errout.str());

        check("void f() {\n"
              "    if (!tok->next()->function(1) || \n"
              "        (tok->next()->function(2) && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "   int y = rand(), z = rand();\n"
              "   if (y==0 || y!=0 && z);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition: y!=0. 'y==0 || (y!=0 && z)' is equivalent to 'y==0 || z'\n", errout.str());

        check("void f() {\n"
              "  if (x>0 || (x<0 && y)) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Test Token::expressionString, TODO move this test
        check("void f() {\n"
              "  if (!dead || (dead && (*it).ticks > 0)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: dead. '!dead || (dead && (*it).ticks>0)' is equivalent to '!dead || (*it).ticks>0'\n", errout.str());

        check("void f() {\n"
              "  if (!x || (x && (2>(y-1)))) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: x. '!x || (x && 2>(y-1))' is equivalent to '!x || 2>(y-1)'\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if (a || (a && b)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: a. 'a || (a && b)' is equivalent to 'a'\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if (a && (a || b)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant condition: a. 'a && (a || b)' is equivalent to 'a'\n", errout.str());
    }

    // clarify conditions with bitwise operator and comparison
    void clarifyCondition2() {
        check("void f() {\n"
              "    if (x & 3 == 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses.\n"
                      "[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n"
                      "[test.cpp:2]: (style) Condition 'x&3==2' is always false\n", errout.str());

        check("void f() {\n"
              "    if (a & fred1.x == fred2.y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses.\n"
                      "[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n"
                      , errout.str());
    }

    // clarify condition that uses ! operator and then bitwise operator
    void clarifyCondition3() {
        check("void f(int w) {\n"
              "    if(!w & 0x8000) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n", errout.str());

        check("void f(int w) {\n"
              "    if((!w) & 0x8000) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (x == foo() & 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n", errout.str());

        check("void f() {\n"
              "    if (2 & x == foo()) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n", errout.str());

        check("void f() {\n"
              "    if (2 & (x == foo())) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::list<int> &ints) { }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { A<x &> a; }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { a(x<y|z,0); }", "test.c");  // filename is c => there are never templates
        ASSERT_EQUALS("[test.c:1]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses.\n", errout.str());

        check("class A<B&,C>;", "test.cpp");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (result != (char *)&inline_result) { }\n" // don't simplify and verify cast
              "}");
        ASSERT_EQUALS("", errout.str());

        // #8495
        check("void f(bool a, bool b) {\n"
              "    C & a & b;\n"
              "}");
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
        check("template<bool B> struct CWinTraits;\n"
              "CWinTraits<WS_CHILD|WS_VISIBLE>::GetWndStyle(0);");
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

    void clarifyCondition7() {
        // Ensure that binary and unary &, and & in declarations are distinguished properly
        check("void f(bool error) {\n"
              "    bool & withoutSideEffects=found.first->second;\n" // Declaring a reference to a boolean; & is no operator at all
              "    execute(secondExpression, &programMemory, &result, &error);\n" // Unary &
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void clarifyCondition8() {
        // don't warn when boolean result comes from function call, array index, etc
        // the operator precedence is not unknown then
        check("bool a();\n"
              "bool f(bool b) {\n"
              "    return (a() & b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool f(bool *a, bool b) {\n"
              "    return (a[10] & b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct A { bool a; };\n"
              "bool f(struct A a, bool b) {\n"
              "    return (a.a & b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct A { bool a; };\n"
              "bool f(struct A a, bool b) {\n"
              "    return (A::a & b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testBug5895() {
        check("void png_parse(uint64_t init, int buf_size) {\n"
              "    if (init == 0x89504e470d0a1a0a || init == 0x8a4d4e470d0a1a0a)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testBug5309() {
        check("extern uint64_t value;\n"
              "void foo() {\n"
              "    if( ( value >= 0x7ff0000000000001ULL )\n"
              "            && ( value <= 0x7fffffffffffffffULL ) );\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void alwaysTrue() {
        check("void f() {\n" // #4842
              "  int x = 0;\n"
              "  if (a) { return; }\n" // <- this is just here to fool simplifyKnownVariabels
              "  if (!x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition '!x' is always true\n", errout.str());

        check("bool f(int x) {\n"
              "  if(x == 0) { x++; return x == 0; } \n"
              "  return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Condition 'x==0' is always false\n", errout.str());

        check("void f() {\n" // #6898 (Token::expressionString)
              "  int x = 0;\n"
              "  A(x++ == 1);\n"
              "  A(x++ == 2);\n"
              "}");
        TODO_ASSERT_EQUALS("function argument is always true? however is code really weird/suspicious?", "", errout.str());

        check("bool foo(int bar) {\n"
              "  bool ret = false;\n"
              "  if (bar == 1)\n"
              "    return ret;\n" // <- #9326 - FP condition is always false
              "  if (bar == 2)\n"
              "    ret = true;\n"
              "  return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f1(const std::string &s) { if(s.empty()) if(s.size() == 0) {}} ");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:1]: (style) Condition 's.size()==0' is always true\n", errout.str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   if( buf != 0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Condition 'buf!=0' is always true\n", errout.str()); // #8924

        check("void f() {\n"
              "   int buf[42];\n"
              "   if( !buf ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Condition '!buf' is always false\n", errout.str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   bool b = buf;\n"
              "   if( b ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition 'b' is always true\n", errout.str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   bool b = buf;\n"
              "   if( !b ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition '!b' is always false\n", errout.str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   int * p = nullptr;\n"
              "   if( buf == p ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition 'buf==p' is always false\n", errout.str());

        check("void f(bool x) {\n"
              "   int buf[42];\n"
              "   if( buf || x ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Condition 'buf' is always true\n", errout.str());

        check("void f(int * p) {\n"
              "   int buf[42];\n"
              "   if( buf == p ) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   int p[42];\n"
              "   if( buf == p ) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   if( buf == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Avoid FP when condition comes from macro
        check("#define NOT !\n"
              "void f() {\n"
              "  int x = 0;\n"
              "  if (a) { return; }\n" // <- this is just here to fool simplifyKnownVariabels
              "  if (NOT x) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("#define M  x != 0\n"
              "void f() {\n"
              "  int x = 0;\n"
              "  if (a) { return; }\n" // <- this is just here to fool simplifyKnownVariabels
              "  if (M) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("#define IF(X)  if (X && x())\n"
              "void f() {\n"
              "  IF(1) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Avoid FP for sizeof condition
        check("void f() {\n"
              "  if (sizeof(char) != 123) {}\n"
              "  if (123 != sizeof(char)) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int x = 123;\n"
              "  if (sizeof(char) != x) {}\n"
              "  if (x != sizeof(char)) {}\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Condition 'sizeof(char)!=x' is always true\n"
                           "[test.cpp:4]: (style) Condition 'x!=sizeof(char)' is always true\n", "", errout.str());

        // Don't warn in assertions. Condition is often 'always true' by intention.
        // If platform,defines,etc cause an 'always false' assertion then that is not very dangerous neither
        check("void f() {\n"
              "  int x = 0;\n"
              "  assert(x == 0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #9363 - do not warn about value passed to function
        check("void f(bool b) {\n"
              "    if (b) {\n"
              "        if (bar(!b)) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());


        // #7783  FP knownConditionTrueFalse on assert(0 && "message")
        check("void foo(int x) {\n"
              "    if (x<0)\n"
              "    {\n"
              "        assert(0 && \"bla\");\n"
              "        ASSERT(0 && \"bla\");\n"
              "        assert_foo(0 && \"bla\");\n"
              "        ASSERT_FOO(0 && \"bla\");\n"
              "        assert((int)(0==0));\n"
              "        assert((int)(0==0) && \"bla\");\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #7750 char literals in boolean expressions
        check("void f() {\n"
              "  if('a'){}\n"
              "  if(L'b'){}\n"
              "  if(1 && 'c'){}\n"
              "  int x = 'd' ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Skip literals
        check("void f() { if(true) {} }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { if(false) {} }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { if(!true) {} }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { if(!false) {} }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { if(0) {} }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { if(1) {} }");
        ASSERT_EQUALS("", errout.str());

        check("void f(int i) {\n"
              "    bool b = false;\n"
              "    if (i == 0) b = true;\n"
              "    else if (!b && i == 1) {}\n"
              "    if (b)\n"
              "    {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition '!b' is always true\n", errout.str());

        check("bool f() { return nullptr; }");
        ASSERT_EQUALS("", errout.str());

        check("enum E { A };\n"
              "bool f() { return A; }\n");
        ASSERT_EQUALS("", errout.str());

        check("bool f() { \n"
              "    const int x = 0;\n"
              "    return x; \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(void){return 1/abs(10);}");
        ASSERT_EQUALS("", errout.str());

        check("bool f() { \n"
              "    int x = 0;\n"
              "    return x; \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("bool f() {\n"
              "    const int a = 50;\n"
              "    const int b = 52;\n"
              "    return a+b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    int a = 50;\n"
              "    int b = 52;\n"
              "    a++;\n"
              "    b++;\n"
              "    return a+b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("bool& g();\n"
              "bool f() {\n"
              "    bool & b = g();\n"
              "    b = false;\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    bool b;\n"
              "    bool f() {\n"
              "        b = false;\n"
              "        return b;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("bool f(long maxtime) {\n"
              "  if (std::time(0) > maxtime)\n"
              "    return std::time(0) > maxtime;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(double param) {\n"
              "  while(bar()) {\n"
              "    if (param<0.)\n"
              "       return;\n"
              "  }\n"
              "  if (param<0.)\n"
              "    return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int i) {\n"
              "  if (i==42)\n"
              "  {\n"
              "    bar();\n"
              "  }\n"
              "  if (cond && (42==i))\n"
              "    return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // 8842 crash
        check("class a {\n"
              "  int b;\n"
              "  c(b);\n"
              "  void f() {\n"
              "    if (b) return;\n"
              "  }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(const char* x, const char* t) {\n"
              "    if (!(strcmp(x, y) == 0)) { return; }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(const int a[]){ if (a == 0){} }");
        ASSERT_EQUALS("", errout.str());

        check("struct S {\n"
              "  bool operator<(const S&);\n"
              "};\n"
              "int main() {\n"
              "  S s;\n"
              "  bool c = s<s;\n"
              "  if (c) return 0;\n"
              "  else return 42;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("long X::g(bool unknown, int& result) {\n"
              "    long ret = 0;\n"
              "    bool f = false;\n"
              "    f = f || unknown;\n"
              "    f ? result = 42 : ret = -1;\n"
              "    return ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(void *handle) {\n"
              "    if (!handle) return 0;\n"
              "    if (handle) return 1;\n"
              "    else return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Condition 'handle' is always true\n", errout.str());

        check("int f(void *handle) {\n"
              "    if (handle == 0) return 0;\n"
              "    if (handle) return 1;\n"
              "    else return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Condition 'handle' is always true\n", errout.str());

        check("int f(void *handle) {\n"
              "    if (handle != 0) return 0;\n"
              "    if (handle) return 1;\n"
              "    else return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Identical condition 'handle!=0', second condition is always false\n", errout.str());

        check("void f(void* x, void* y) {\n"
              "    if (x == nullptr && y == nullptr)\n"
              "        return;\n"
              "    if (x == nullptr || y == nullptr)\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void* g();\n"
              "void f(void* a, void* b) {\n"
              "    while (a) {\n"
              "        a = g();\n"
              "        if (a == b)\n"
              "            break;\n"
              "    }\n"
              "    if (a) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void* g();\n"
              "void f(void* a, void* b) {\n"
              "    while (a) {\n"
              "        a = g();\n"
              "    }\n"
              "    if (a) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'a' is always false\n", errout.str());

        check("void f(int * x, bool b) {\n"
              "    if (!x && b) {}\n"
              "    else if (x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const std::string x=\"xyz\";\n"
              "    if(!x.empty()){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Condition '!x.empty()' is always true\n", errout.str());

        check("std::string g();\n"
              "void f() {\n"
              "    const std::string msg = g();\n"
              "    if(!msg.empty()){}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int *array, int size ) {\n"
              "    for(int i = 0; i < size; ++i) {\n"
              "        if(array == 0)\n"
              "            continue;\n"
              "        if(array){}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (style) Condition 'array' is always true\n", errout.str());

        check("void f(int *array, int size ) {\n"
              "    for(int i = 0; i < size; ++i) {\n"
              "        if(array == 0)\n"
              "            continue;\n"
              "        else if(array){}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (style) Condition 'array' is always true\n", errout.str());

        // #9277
        check("int f() {\n"
              "    constexpr bool x = true;\n"
              "    if constexpr (x)\n"
              "        return 0;\n"
              "    else\n"
              "        return 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9319
        check("struct S {\n"
              "  int a;\n"
              "  int b;\n"
              "};\n"
              "void g(S s, bool& x);\n"
              "void f() {\n"
              "  bool x = false;\n"
              "  g({0, 1}, x);\n"
              "  if (x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9318
        check("class A {};\n"
              "class B : public A {};\n"
              "void f(A* x) {\n"
              "  if (!x)\n"
              "    return;\n"
              "  auto b = dynamic_cast<B*>(x);\n"
              "  if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int foo() {\n"
              "    auto x = getX();\n"
              "    if (x == nullptr)\n"
              "        return 1;\n"
              "    auto y = dynamic_cast<Y*>(x)\n"
              "    if (y == nullptr)\n"
              "        return 2;\n"
              "    return 3;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // handleKnownValuesInLoop
        check("bool g();\n"
              "void f(bool x) {\n"
              "    if (x) while(x) x = g();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // isLikelyStream
        check("void f(std::istringstream& iss) {\n"
              "   std::string x;\n"
              "   while (iss) {\n"
              "       iss >> x;\n"
              "       if (!iss) break;\n"
              "   }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9332
        check("struct A { void* g(); };\n"
              "void f() {\n"
              "    A a;\n"
              "    void* b = a.g();\n"
              "    if (!b) return;\n"
              "    void* c = a.g();\n"
              "    if (!c) return;\n"
              "    bool compare = c == b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9361
        check("void f(char c) {\n"
              "    if (c == '.') {}\n"
              "    else if (isdigit(c) != 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9351
        check("int f(int x) {\n"
              "    const bool b = x < 42;\n"
              "    if(b) return b?0:-1;\n"
              "    return 42;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:3]: (style) Condition 'b' is always true\n", errout.str());

        // #9362
        check("uint8_t g();\n"
              "void f() {\n"
              "    const uint8_t v = g();\n"
              "    if((v != 0x00)) {\n"
              "        if( (v & 0x01) == 0x00) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9367
        check("void f(long x) {\n"
              "    if (x <= 0L)\n"
              "        return;\n"
              "    if (x % 360L == 0)\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(int a, int b) {\n"
              "    static const int x = 10;\n"
              "    return x == 1 ? a : b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("const bool x = false;\n"
              "void f() {\n"
              "    if (x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("const bool x = false;\n"
              "void f() {\n"
              "    if (!x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9709
        check("void f(int a) {\n"
              "    bool ok = false;\n"
              "    const char * r = nullptr;\n"
              "    do_something(&r);\n"
              "    if (r != nullptr)\n"
              "        ok = a != 0;\n"
              "    if (ok) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9816
        check("bool g();\n"
              "void f() {\n"
              "    bool b = false;\n"
              "    do {\n"
              "        do {\n"
              "            if (g())\n"
              "                break;\n"
              "            b = true;\n"
              "        } while(false);\n"
              "    } while(!b);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9865
        check("void f(const std::string &s) {\n"
              "    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {\n"
              "        const unsigned char c = static_cast<unsigned char>(*it);\n"
              "        if (c == '0') {}\n"
              "        else if ((c == 'a' || c == 'A')\n"
              "                 || (c == 'b' || c == 'B')) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9711
        check("int main(int argc, char* argv[]) {\n"
              "  int foo = 0;\n"
              "  struct option options[] = {\n"
              "    {\"foo\", no_argument, &foo, \'f\'},\n"
              "    {NULL, 0, NULL, 0},\n"
              "  };\n"
              "  getopt_long(argc, argv, \"f\", options, NULL);\n"
              "  if (foo) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct a {\n"
              "  int *b();\n"
              "};\n"
              "bool g(a c, a* d) {\n"
              "  a *v, *e = v = &c;\n"
              "  if (!v)\n"
              "    return true;\n"
              "  int *f = v->b();\n"
              "  if (f)\n"
              "    v = nullptr;\n"
              "  if (v == nullptr && e) {}\n"
              "  return d;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #10037
        check("struct a {\n"
              "    int* p;\n"
              "};\n"
              "void g(a*);\n"
              "void f() {\n"
              "    struct a b;\n"
              "    uint32_t p = (uint32_t) -1;\n"
              "    b.p = (void *) &p;\n"
              "    int r = g(&b);\n"
              "    if (r == 0)\n"
              "        if (p != (uint32_t) -1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void alwaysTrueInfer() {
        check("void f(int x) {\n"
              "    if (x > 5) {\n"
              "        x++;\n"
              "        if (x == 1) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Condition 'x==1' is always false\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 5) {\n"
              "        x++;\n"
              "        if (x != 1) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Condition 'x!=1' is always true\n", errout.str());

        // #6890
        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x == -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'x==-1' is always false\n", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x != -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'x!=-1' is always true\n", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x >= -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'x>=-1' is always true\n", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x > -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'x>-1' is always true\n", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x < -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'x<-1' is always false\n", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x <= -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'x<=-1' is always false\n", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x > 7) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) Condition 'x>7' is always true\n", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x > 9) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x; \n"
              "        if (x > 10) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9541
        check("int f(int pos, int a) {\n"
              "    if (pos <= 0)\n"
              "        pos = 0;\n"
              "    else if (pos < a)\n"
              "        if(pos > 0)\n"
              "            --pos;\n"
              "    return pos;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:5]: (style) Condition 'pos>0' is always true\n", errout.str());

        // #9721
        check("void f(int x) {\n"
              "    if (x > 127) {\n"
              "        if ( (x>255) || (-128>x) )\n"
              "            return;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Condition '-128>x' is always false\n", errout.str());
    }

    void alwaysTrueContainer() {
        // #9329
        check("void c1(std::vector<double>&);\n"
              "void c2(std::vector<double>&);\n"
              "void foo(int flag) {\n"
              "    std::vector<double> g;\n"
              "    if (flag)\n"
              "        c1(g );\n"
              "    else\n"
              "        c2(g );\n"
              "    if ( !g.empty() )\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int flag) {\n"
              "    std::vector<double> g;\n"
              "    if (flag)\n"
              "        c1(g );\n"
              "    else\n"
              "        c2(g );\n"
              "    if ( !g.empty() )\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    std::vector<int> v;\n"
              "    void g();\n"
              "    void f(bool b) {\n"
              "        v.clear();\n"
              "        g();\n"
              "        return !v.empty();\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void multiConditionAlwaysTrue() {
        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) continue;\n"
              "  if (val > 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if (val > 0) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if (val < 0) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int activate = 0;\n"
              "  int foo = 0;\n"
              "  if (activate) {}\n"
              "  else if (foo) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition 'activate' is always false\n"
                      "[test.cpp:5]: (style) Condition 'foo' is always false\n", errout.str());

        // #6904
        check("void f() {\n"
              "  const int b[2] = { 1,0 };\n"
              "  if(b[1] == 2) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Condition 'b[1]==2' is always false\n", errout.str());

        // #9878
        check("void f(bool a, bool b) {\n"
              "    if (a && b){;}\n"
              "    else if (!a && b){;}\n"
              "    else if (!a && !b){;}\n"
              "    else {;}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateCondition() {
        check("void f(bool x) {\n"
              "    if(x) {}\n"
              "    if(x) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The if condition is the same as the previous if condition\n",
                      errout.str());

        check("void f(int x) {\n"
              "    if(x == 1) {}\n"
              "    if(x == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The if condition is the same as the previous if condition\n",
                      errout.str());

        check("void f(int x) {\n"
              "    if(x == 1) {}\n"
              "    if(x == 2) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if(x == 1) {}\n"
              "    if(x != 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool x) {\n"
              "    if(x) {}\n"
              "    g();\n"
              "    if(x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if(x == 1) { x++; }\n"
              "    if(x == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #8996
        check("void g(int** v);\n"
              "void f() {\n"
              "  int a = 0;\n"
              "  int b = 0;\n"
              "  int* d[] = {&a, &b};\n"
              "  g(d);\n"
              "  if (a) {}\n"
              "  if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9311
        check("struct c {\n"
              "  int* p;\n"
              "};\n"
              "void g(struct c* v);\n"
              "void f() {\n"
              "  int a = 0;\n"
              "  int b = 0;\n"
              "  struct c d[] = {{&a}, {&b}};\n"
              "  g(d);\n"
              "  if (a) {}\n"
              "  if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #8993
        check("void f(const std::string& x) {\n"
              "  auto y = x;\n"
              "  if (x.empty()) y = \"1\";\n"
              "  if (y.empty()) return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #9106
        check("struct A {int b;};\n"
              "void f(A a, int c) {\n"
              "    if (a.b) a.b = c;\n"
              "    if (a.b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    int a;\n"
              "    void b() const {\n"
              "        return a == 1;\n"
              "    }\n"
              "    void c();\n"
              "    void d() {\n"
              "        if(b()) {\n"
              "            c();\n"
              "        }\n"
              "        if (b()) {\n"
              "            a = 3;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    int a;\n"
              "    void b() const {\n"
              "        return a == 1;\n"
              "    }\n"
              "    void d() {\n"
              "        if(b()) {\n"
              "            a = 2;\n"
              "        }\n"
              "        if (b()) {\n"
              "            a = 3;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    int a;\n"
              "    void b() const {\n"
              "        return a == 1;\n"
              "    }\n"
              "    void d() {\n"
              "        if(b()) {\n"
              "        }\n"
              "        if (b()) {\n"
              "            a = 3;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:9]: (style) The if condition is the same as the previous if condition\n",
                      errout.str());

        check("void f(bool a, bool b) {\n"
              "    auto g = [&] { b = !a; };\n"
              "    if (b)\n"
              "        g();\n"
              "    if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void g(bool& a);\n"
              "void f(bool b) {\n"
              "    auto h = std::bind(&g, std::ref(b));\n"
              "    if (b)\n"
              "        h();\n"
              "    if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int *i) {\n"
              "  if (*i == 0) {\n"
              "    *i = 1;\n"
              "  }\n"
              "  if (*i == 0) {\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // do not crash
        check("void assign(const MMA& other) {\n"
              "    if (mPA.cols != other.mPA.cols || mPA.rows != other.mPA.rows)\n"
              "        ;\n"
              "    if (other.mPA.cols > 0 && other.mPA.rows > 0)\n"
              "        ;\n"
              "}");
    }

    void checkInvalidTestForOverflow() {
        check("void f(char *p, unsigned int x) {\n"
              "    assert((p + x) < p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Invalid test for overflow '(p+x)<p'. Condition is always false unless there is overflow, and overflow is undefined behaviour.\n", errout.str());

        check("void f(char *p, unsigned int x) {\n"
              "    assert((p + x) >= p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Invalid test for overflow '(p+x)>=p'. Condition is always true unless there is overflow, and overflow is undefined behaviour.\n", errout.str());

        check("void f(char *p, unsigned int x) {\n"
              "    assert(p > (p + x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Invalid test for overflow 'p>(p+x)'. Condition is always false unless there is overflow, and overflow is undefined behaviour.\n", errout.str());

        check("void f(char *p, unsigned int x) {\n"
              "    assert(p <= (p + x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Invalid test for overflow 'p<=(p+x)'. Condition is always true unless there is overflow, and overflow is undefined behaviour.\n", errout.str());

        check("void f(signed int x) {\n"
              "    assert(x + 100 < x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Invalid test for overflow 'x+100<x'. Condition is always false unless there is overflow, and overflow is undefined behaviour.\n", errout.str());

        check("void f(signed int x) {\n" // unsigned overflow => don't warn
              "    assert(x + 100U < x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkConditionIsAlwaysTrueOrFalseInsideIfWhile() {
        check("void f() {\n"
              "    enum states {A,B,C};\n"
              "    const unsigned g_flags = B|C;\n"
              "    if(g_flags & A) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Condition 'g_flags&A' is always false\n", errout.str());

        check("void f() {\n"
              "    int a = 5;"
              "    if(a) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Condition 'a' is always true\n", errout.str());

        check("void f() {\n"
              "    int a = 5;"
              "    while(a + 1) { a--; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 5;"
              "    while(a + 1) { return; }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Condition 'a+1' is always true\n", errout.str());
    }

    void alwaysTrueFalseInLogicalOperators() {
        check("bool f();\n"
              "void foo() { bool x = true; if(x||f()) {}}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Condition 'x' is always true\n", errout.str());

        check("void foo(bool b) { bool x = true; if(x||b) {}}\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) Condition 'x' is always true\n", errout.str());

        check("void foo(bool b) { if(true||b) {}}\n");
        ASSERT_EQUALS("", errout.str());

        check("bool f();\n"
              "void foo() { bool x = false; if(x||f()) {}}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Condition 'x' is always false\n", errout.str());

        check("bool f();\n"
              "void foo() { bool x = false; if(x&&f()) {}}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Condition 'x' is always false\n", errout.str());

        check("void foo(bool b) { bool x = false; if(x&&b) {}}\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) Condition 'x' is always false\n", errout.str());

        check("void foo(bool b) { if(false&&b) {}}\n");
        ASSERT_EQUALS("", errout.str());

        check("bool f();\n"
              "void foo() { bool x = true; if(x&&f()) {}}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Condition 'x' is always true\n", errout.str());

        // #9578
        check("bool f(const std::string &s) {\n"
              "        return s.size()>2U && s[0]=='4' && s[0]=='2';\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Condition 's[0]=='2'' is always false\n", errout.str());
    }

    void pointerAdditionResultNotNull() {
        check("void f(char *ptr) {\n"
              "  if (ptr + 1 != 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison is wrong. Result of 'ptr+1' can't be 0 unless there is pointer overflow, and pointer overflow is undefined behaviour.\n", errout.str());
    }

    void duplicateConditionalAssign() {
        setMultiline();

        check("void f(int& x, int y) {\n"
              "    if (x == y)\n"
              "        x = y;\n"
              "}\n");
        ASSERT_EQUALS("test.cpp:3:style:Assignment 'x=y' is redundant with condition 'x==y'.\n"
                      "test.cpp:2:note:Condition 'x==y'\n"
                      "test.cpp:3:note:Assignment 'x=y' is redundant\n", errout.str());

        check("void f(int& x, int y) {\n"
              "    if (x != y)\n"
              "        x = y;\n"
              "}\n");
        ASSERT_EQUALS("test.cpp:2:style:The statement 'if (x!=y) x=y' is logically equivalent to 'x=y'.\n"
                      "test.cpp:3:note:Assignment 'x=y'\n"
                      "test.cpp:2:note:Condition 'x!=y' is redundant\n", errout.str());

        check("void f(int& x, int y) {\n"
              "    if (x == y)\n"
              "        x = y;\n"
              "    else\n"
              "        x = 1;\n"
              "}\n");
        ASSERT_EQUALS("test.cpp:3:style:Assignment 'x=y' is redundant with condition 'x==y'.\n"
                      "test.cpp:2:note:Condition 'x==y'\n"
                      "test.cpp:3:note:Assignment 'x=y' is redundant\n", errout.str());

        check("void f(int& x, int y) {\n"
              "    if (x != y)\n"
              "        x = y;\n"
              "    else\n"
              "        x = 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int& x, int y) {\n"
              "    if (x == y)\n"
              "        x = y + 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void g();\n"
              "void f(int& x, int y) {\n"
              "    if (x == y) {\n"
              "        x = y;\n"
              "        g();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestCondition)
