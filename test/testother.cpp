/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "checkother.h"
#include "library.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "testsuite.h"
#include "tokenize.h"

#include <simplecpp.h>
#include <tinyxml2.h>
#include <map>
#include <string>
#include <vector>

class TestOther : public TestFixture {
public:
    TestOther() : TestFixture("TestOther") {
    }

private:
    Settings _settings;

    void run() override {
        LOAD_LIB_2(_settings.library, "std.cfg");


        TEST_CASE(emptyBrackets);

        TEST_CASE(zeroDiv1);
        TEST_CASE(zeroDiv2);
        TEST_CASE(zeroDiv3);
        TEST_CASE(zeroDiv4);
        TEST_CASE(zeroDiv5);
        TEST_CASE(zeroDiv6);
        TEST_CASE(zeroDiv7);  // #4930
        TEST_CASE(zeroDiv8);
        TEST_CASE(zeroDiv9);
        TEST_CASE(zeroDiv10);
        TEST_CASE(zeroDiv11);

        TEST_CASE(zeroDivCond); // division by zero / useless condition

        TEST_CASE(nanInArithmeticExpression);

        TEST_CASE(varScope1);
        TEST_CASE(varScope2);
        TEST_CASE(varScope3);
        TEST_CASE(varScope4);
        TEST_CASE(varScope5);
        TEST_CASE(varScope6);
        TEST_CASE(varScope7);
        TEST_CASE(varScope8);
        TEST_CASE(varScope9);       // classes may have extra side-effects
        TEST_CASE(varScope10);      // Undefined macro FOR
        TEST_CASE(varScope11);      // #2475 - struct initialization is not inner scope
        TEST_CASE(varScope12);
        TEST_CASE(varScope13);      // variable usage in inner loop
        TEST_CASE(varScope14);
        TEST_CASE(varScope15);      // #4573 if-else-if
        TEST_CASE(varScope16);
        TEST_CASE(varScope17);
        TEST_CASE(varScope18);
        TEST_CASE(varScope20);      // Ticket #5103
        TEST_CASE(varScope21);      // Ticket #5382
        TEST_CASE(varScope22);      // Ticket #5684
        TEST_CASE(varScope23);      // Ticket #6154
        TEST_CASE(varScope24);      // pointer / reference
        TEST_CASE(varScope25);      // time_t

        TEST_CASE(oldStylePointerCast);
        TEST_CASE(invalidPointerCast);

        TEST_CASE(passedByValue);
        TEST_CASE(passedByValue_nonConst);

        TEST_CASE(switchRedundantAssignmentTest);
        TEST_CASE(switchRedundantOperationTest);
        TEST_CASE(switchRedundantBitwiseOperationTest);
        TEST_CASE(unreachableCode);

        TEST_CASE(suspiciousCase);
        TEST_CASE(suspiciousEqualityComparison);

        TEST_CASE(selfAssignment);
        TEST_CASE(trac1132);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunction1);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunction2);
        TEST_CASE(testMisusedScopeObjectPicksClass);
        TEST_CASE(testMisusedScopeObjectPicksStruct);
        TEST_CASE(testMisusedScopeObjectDoesNotPickIf);
        TEST_CASE(testMisusedScopeObjectDoesNotPickConstructorDeclaration);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunctor);
        TEST_CASE(testMisusedScopeObjectDoesNotPickLocalClassConstructors);
        TEST_CASE(testMisusedScopeObjectDoesNotPickUsedObject);
        TEST_CASE(testMisusedScopeObjectDoesNotPickPureC);
        TEST_CASE(testMisusedScopeObjectDoesNotPickNestedClass);
        TEST_CASE(testMisusedScopeObjectInConstructor);
        TEST_CASE(testMisusedScopeObjectNoCodeAfter);
        TEST_CASE(trac2071);
        TEST_CASE(trac2084);
        TEST_CASE(trac3693);

        TEST_CASE(clarifyCalculation);
        TEST_CASE(clarifyStatement);

        TEST_CASE(duplicateBranch);
        TEST_CASE(duplicateBranch1); // tests extracted by http://www.viva64.com/en/b/0149/ ( Comparison between PVS-Studio and cppcheck ): Errors detected in Quake 3: Arena by PVS-Studio: Fragment 2
        TEST_CASE(duplicateBranch2); // empty macro
        TEST_CASE(duplicateBranch3);
        TEST_CASE(duplicateExpression1);
        TEST_CASE(duplicateExpression2); // ticket #2730
        TEST_CASE(duplicateExpression3); // ticket #3317
        TEST_CASE(duplicateExpression4); // ticket #3354 (++)
        TEST_CASE(duplicateExpression5); // ticket #3749 (macros with same values)
        TEST_CASE(duplicateExpression6); // ticket #4639
        TEST_CASE(duplicateExpression7);
        TEST_CASE(duplicateExpression8);
        TEST_CASE(duplicateExpressionLoop);
        TEST_CASE(duplicateValueTernary);
        TEST_CASE(duplicateExpressionTernary); // #6391
        TEST_CASE(duplicateExpressionTemplate); // #6930
        TEST_CASE(oppositeExpression);
        TEST_CASE(duplicateVarExpression);
        TEST_CASE(duplicateVarExpressionUnique);
        TEST_CASE(duplicateVarExpressionAssign);
        TEST_CASE(duplicateVarExpressionCrash);
        TEST_CASE(multiConditionSameExpression);

        TEST_CASE(checkSignOfUnsignedVariable);
        TEST_CASE(checkSignOfPointer);

        TEST_CASE(checkForSuspiciousSemicolon1);
        TEST_CASE(checkForSuspiciousSemicolon2);

        TEST_CASE(checkInvalidFree);

        TEST_CASE(checkRedundantCopy);

        TEST_CASE(checkNegativeShift);

        TEST_CASE(incompleteArrayFill);

        TEST_CASE(redundantVarAssignment);
        TEST_CASE(redundantVarAssignment_7133);
        TEST_CASE(redundantVarAssignment_stackoverflow);
        TEST_CASE(redundantVarAssignment_lambda);
        TEST_CASE(redundantVarAssignment_for);
        TEST_CASE(redundantMemWrite);

        TEST_CASE(varFuncNullUB);

        TEST_CASE(checkPipeParameterSize); // ticket #3521

        TEST_CASE(checkCastIntToCharAndBack); // ticket #160

        TEST_CASE(checkCommaSeparatedReturn);
        TEST_CASE(checkPassByReference);

        TEST_CASE(checkComparisonFunctionIsAlwaysTrueOrFalse);

        TEST_CASE(integerOverflow); // #5895

        TEST_CASE(redundantPointerOp);
        TEST_CASE(test_isSameExpression);
        TEST_CASE(raceAfterInterlockedDecrement);

        TEST_CASE(testUnusedLabel);

        TEST_CASE(testEvaluationOrder);
        TEST_CASE(testEvaluationOrderSelfAssignment);
        TEST_CASE(testEvaluationOrderMacro);
        TEST_CASE(testEvaluationOrderSequencePointsFunctionCall);
        TEST_CASE(testEvaluationOrderSequencePointsComma);
        TEST_CASE(testEvaluationOrderSizeof);

        TEST_CASE(testUnsignedLessThanZero);

        TEST_CASE(doubleMove1);
        TEST_CASE(doubleMoveMemberInitialization1);
        TEST_CASE(doubleMoveMemberInitialization2);
        TEST_CASE(moveAndAssign1);
        TEST_CASE(moveAndAssign2);
        TEST_CASE(moveAssignMoveAssign);
        TEST_CASE(moveAndReset1);
        TEST_CASE(moveAndReset2);
        TEST_CASE(moveResetMoveReset);
        TEST_CASE(moveAndFunctionParameter);
        TEST_CASE(moveAndFunctionParameterReference);
        TEST_CASE(moveAndFunctionParameterConstReference);
        TEST_CASE(moveAndFunctionParameterUnknown);
        TEST_CASE(moveAndReturn);
        TEST_CASE(moveAndClear);
        TEST_CASE(movedPointer);
        TEST_CASE(moveAndAddressOf);
        TEST_CASE(partiallyMoved);
        TEST_CASE(moveAndLambda);
        TEST_CASE(forwardAndUsed);

        TEST_CASE(funcArgNamesDifferent);
        TEST_CASE(funcArgOrderDifferent);
        TEST_CASE(cpp11FunctionArgInit); // #7846 - "void foo(int declaration = {}) {"

        TEST_CASE(shadowVariables);
    }

    void check(const char code[], const char *filename = nullptr, bool experimental = false, bool inconclusive = true, bool runSimpleChecks=true, Settings* settings = 0) {
        // Clear the error buffer..
        errout.str("");

        if (!settings) {
            settings = &_settings;
        }
        settings->addEnabled("style");
        settings->addEnabled("warning");
        settings->addEnabled("portability");
        settings->addEnabled("performance");
        settings->standards.c = Standards::CLatest;
        settings->standards.cpp = Standards::CPPLatest;
        settings->inconclusive = inconclusive;
        settings->experimental = experimental;

        // Tokenize..
        Tokenizer tokenizer(settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename ? filename : "test.cpp");

        // Check..
        CheckOther checkOther(&tokenizer, settings, this);
        checkOther.runChecks(&tokenizer, settings, this);

        if (runSimpleChecks) {
            tokenizer.simplifyTokenList2();
            checkOther.runSimplifiedChecks(&tokenizer, settings, this);
        }
    }

    void check(const char code[], Settings *s) {
        check(code,"test.cpp",false,true,true,s);
    }

    void checkP(const char code[], const char *filename = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings* settings = &_settings;
        settings->addEnabled("style");
        settings->addEnabled("warning");
        settings->addEnabled("portability");
        settings->addEnabled("performance");
        settings->standards.c = Standards::CLatest;
        settings->standards.cpp = Standards::CPPLatest;
        settings->inconclusive = true;
        settings->experimental = false;

        // Raw tokens..
        std::vector<std::string> files(1, filename);
        std::istringstream istr(code);
        const simplecpp::TokenList tokens1(istr, files, files[0]);

        // Preprocess..
        simplecpp::TokenList tokens2(files);
        std::map<std::string, simplecpp::TokenList*> filedata;
        simplecpp::preprocess(tokens2, tokens1, files, filedata, simplecpp::DUI());

        // Tokenizer..
        Tokenizer tokenizer(settings, this);
        tokenizer.createTokens(&tokens2);
        tokenizer.simplifyTokens1("");

        // Check..
        CheckOther checkOther(&tokenizer, settings, this);
        checkOther.runChecks(&tokenizer, settings, this);
        tokenizer.simplifyTokenList2();
        checkOther.runSimplifiedChecks(&tokenizer, settings, this);
    }

    void checkposix(const char code[]) {
        static Settings settings;
        settings.addEnabled("warning");
        settings.standards.posix = true;

        check(code,
              nullptr, // filename
              false,   // experimental
              false,   // inconclusive
              true,    // runSimpleChecks
              &settings);
    }

    void checkInterlockedDecrement(const char code[]) {
        static Settings settings;
        settings.platformType = Settings::Win32A;

        check(code, nullptr, false, false, true, &settings);
    }

    void emptyBrackets() {
        check("{\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void zeroDiv1() { // floating point division by zero => no error
        check("void foo() {\n"
              "    cout << 1. / 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    cout << 42 / (double)0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    cout << 42 / (float)0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    cout << 42 / (int)0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n", errout.str());
    }

    void zeroDiv2() {
        check("void foo()\n"
              "{\n"
              "    int sum = 0;\n"
              "    for(int i = 0; i < n; i ++)\n"
              "    {\n"
              "        sum += i;\n"
              "    }\n"
              "    cout<<b/sum;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDiv3() {
        check("int foo(int i) {\n"
              "    return i / 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n", errout.str());

        check("int foo(int i) {\n"
              "    return i % 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n", errout.str());

        check("void foo(int& i) {\n"
              "    i /= 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n", errout.str());

        check("void foo(int& i) {\n"
              "    i %= 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n", errout.str());

        check("uint8_t foo(uint8_t i) {\n"
              "    return i / 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n", errout.str());
    }

    void zeroDiv4() {
        check("void f()\n"
              "{\n"
              "   long a = b / 0x6;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0x0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(long b)\n"
              "{\n"
              "   long a = b / 0x0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0L;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(long b)\n"
              "{\n"
              "   long a = b / 0L;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0ul;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(long b)\n"
              "{\n"
              "   long a = b / 0ul;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        // Don't warn about floating points (gcc doesn't warn either)
        // and floating points are handled differently than integers.
        check("void f()\n"
              "{\n"
              "   long a = b / 0.0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0.5;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDiv5() {
        check("void f()\n"
              "{ { {\n"
              "   long a = b / 0;\n"
              "} } }");
        ASSERT_EQUALS("", errout.str());
        check("void f(long b)\n"
              "{ { {\n"
              "   long a = b / 0;\n"
              "} } }");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());
    }

    void zeroDiv6() {
        check("void f()\n"
              "{ { {\n"
              "   int a = b % 0;\n"
              "} } }");
        ASSERT_EQUALS("", errout.str());
        check("void f(int b)\n"
              "{ { {\n"
              "   int a = b % 0;\n"
              "} } }");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());
    }

    void zeroDiv7() {
        // unknown types for x and y --> do not warn
        check("void f() {\n"
              "  int a = x/2*3/0;\n"
              "  int b = y/2*3%0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(int x, int y) {\n"
              "  int a = x/2*3/0;\n"
              "  int b = y/2*3%0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n"
                      "[test.cpp:3]: (error) Division by zero.\n", errout.str());
    }

    void zeroDiv8() {
        // #5584 - FP when function is unknown
        check("void f() {\n"
              "  int a = 0;\n"
              "  do_something(a);\n"
              "  return 4 / a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error, inconclusive) Division by zero.\n", errout.str());
    }

    void zeroDiv9() {
        // #6403 FP zerodiv - inside protecting if-clause
        check("void foo() {\n"
              "  double fStepHelp = 0;\n"
              "   if( (rOuterValue >>= fStepHelp) ) {\n"
              "     if( fStepHelp != 0.0) {\n"
              "       double fStepMain = 1;\n"
              "       sal_Int32 nIntervalCount = static_cast< sal_Int32 >(fStepMain / fStepHelp);\n"
              "    }\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDiv10() {
        // #5402 false positive: (error) Division by zero -- with boost::format
        check("int main() {\n"
              "  std::cout\n"
              "    << boost::format(\" %d :: %s <> %s\") % 0 % \"a\" % \"b\"\n"
              "    << std::endl;\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDiv11() {
        check("void f(int a) {\n"
              "  int res = (a+2)/0;\n"
              "  int res = (a*2)/0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n"
                      "[test.cpp:3]: (error) Division by zero.\n", errout.str());
        check("void f() {\n"
              "  int res = (a+2)/0;\n"
              "  int res = (a*2)/0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDivCond() {
        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x > 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x>0' is redundant or there is division by zero at line 2.\n", errout.str());

        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x >= 1) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x>=1' is redundant or there is division by zero at line 2.\n", errout.str());

        check("void f(int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x == 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x==0' is redundant or there is division by zero at line 2.\n", errout.str());

        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x != 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x!=0' is redundant or there is division by zero at line 2.\n", errout.str());

        // function call
        check("void f1(int x, int y) { c=x/y; }\n"
              "void f2(unsigned int y) {\n"
              "    f1(123,y);\n"
              "    if (y>0){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:1]: (warning) Either the condition 'y>0' is redundant or there is division by zero at line 1.\n", errout.str());

        // avoid false positives when variable is changed after division
        check("void f() {\n"
              "  unsigned int x = do_something();\n"
              "  int y = 17 / x;\n"
              "  x = some+calculation;\n"
              "  if (x != 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        {
            // function is called that might modify global variable
            check("void do_something();\n"
                  "int x;\n"
                  "void f() {\n"
                  "  int y = 17 / x;\n"
                  "  do_something();\n"
                  "  if (x != 0) {}\n"
                  "}");
            ASSERT_EQUALS("", errout.str());

            // function is called. but don't care, variable is local
            check("void do_something();\n"
                  "void f() {\n"
                  "  int x = some + calculation;\n"
                  "  int y = 17 / x;\n"
                  "  do_something();\n"
                  "  if (x != 0) {}\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:4]: (warning) Either the condition 'x!=0' is redundant or there is division by zero at line 4.\n", errout.str());
        }

        check("void do_something(int value);\n"
              "void f(int x) {\n"
              "  int y = 17 / x;\n"
              "  do_something(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int x;\n"
              "void f() {\n"
              "  int y = 17 / x;\n"
              "  while (y || x == 0) { x--; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ticket 5033 segmentation fault (valid code) in CheckOther::checkZeroDivisionOrUselessCondition
        check("void f() {\n"
              "double* p1= new double[1];\n"
              "double* p2= new double[1];\n"
              "double* p3= new double[1];\n"
              "double* pp[3] = {p1,p2,p3};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5105 - FP
        check("int f(int a, int b) {\n"
              "  int r = a / b;\n"
              "  if (func(b)) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Unknown types for b and c --> do not warn
        check("int f(int d) {\n"
              "  int r = (a?b:c) / d;\n"
              "  if (d == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f(int a) {\n"
              "  int r = a ? 1 / a : 0;\n"
              "  if (a == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f(int a) {\n"
              "  int r = (a == 0) ? 0 : 1 / a;\n"
              "  if (a == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void nanInArithmeticExpression() {
        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0 + 1.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0 - 1.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 1.0 + 3.0 / 0.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 1.0 - 3.0 / 0.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }

    void varScope1() {
        check("unsigned short foo()\n"
              "{\n"
              "    test_client CClient;\n"
              "    try\n"
              "    {\n"
              "        if (CClient.Open())\n"
              "        {\n"
              "            return 0;\n"
              "        }\n"
              "    }\n"
              "    catch (...)\n"
              "    {\n"
              "        return 2;\n"
              "    }\n"
              "\n"
              "    try\n"
              "    {\n"
              "        CClient.Close();\n"
              "    }\n"
              "    catch (...)\n"
              "    {\n"
              "        return 2;\n"
              "    }\n"
              "\n"
              "    return 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope2() {
        check("int foo()\n"
              "{\n"
              "    Error e;\n"
              "    e.SetValue(12);\n"
              "    throw e;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope3() {
        check("void foo()\n"
              "{\n"
              "    int i;\n"
              "    int *p = 0;\n"
              "    if (abc)\n"
              "    {\n"
              "        p = &i;\n"
              "    }\n"
              "    *p = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope4() {
        check("void foo()\n"
              "{\n"
              "    int i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope5() {
        check("void f(int x)\n"
              "{\n"
              "    int i = 0;\n"
              "    if (x) {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable 'i' can be reduced.\n", errout.str());

        check("void f(int x) {\n"
              "    const unsigned char i = 0;\n"
              "    if (x) {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x)\n"
              "{\n"
              "    int i = 0;\n"
              "    if (x) {b()}\n"
              "    else {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable 'i' can be reduced.\n", errout.str());
    }

    void varScope6() {
        check("void f(int x)\n"
              "{\n"
              "    int i = x;\n"
              "    if (a) {\n"
              "        x++;\n"
              "    }\n"
              "    if (b) {\n"
              "        c(i);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n" // #5398
              "    bool success = false;\n"
              "    int notReducable(someClass.getX(&success));\n"
              "    if (success) {\n"
              "        foo(notReducable);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(Test &test) {\n"
              "  int& x = test.getData();\n"
              "  if (test.process())\n"
              "    x = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "int foo = 0;\n"
              "std::vector<int> vec(10);\n"
              "BOOST_FOREACH(int& i, vec)\n"
              "{\n"
              " foo += 1;\n"
              " if(foo == 10)\n"
              " {\n"
              "  return 0;\n"
              " }\n"
              "}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int &x)\n"
              "{\n"
              "  int n = 1;\n"
              "  do\n"
              "  {\n"
              "    ++n;\n"
              "    ++x;\n"
              "  } while (x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope7() {
        check("void f(int x)\n"
              "{\n"
              "    int y = 0;\n"
              "    b(y);\n"
              "    if (x) {\n"
              "        y++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope8() {
        check("void test() {\n"
              "    float edgeResistance=1;\n"
              "    std::vector<int> edges;\n"
              "    BOOST_FOREACH(int edge, edges) {\n"
              "        edgeResistance = (edge+1) / 2.0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'edgeResistance' can be reduced.\n", errout.str());
    }

    void varScope9() {
        // classes may have extra side effects
        check("class fred {\n"
              "public:\n"
              "    void x();\n"
              "};\n"
              "void test(int a) {\n"
              "    fred f;\n"
              "    if (a == 2) {\n"
              "        f.x();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope10() {
        check("int f()\n"
              "{\n"
              "    int x = 0;\n"
              "    FOR {\n"
              "        foo(x++);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope11() {
        check("int f() {\n"
              "    int x = 0;\n"
              "    AB ab = { x, 0 };\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    int x = 0;\n"
              "    if (a == 0) { ++x; }\n"
              "    AB ab = { x, 0 };\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    int x = 0;\n"
              "    if (a == 0) { ++x; }\n"
              "    if (a == 1) { AB ab = { x, 0 }; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope12() {
        check("void f(int x) {\n"
              "    int i[5];\n"
              "    int* j = y;\n"
              "    if (x)\n"
              "        foo(i);\n"
              "    foo(j);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'i' can be reduced.\n", errout.str());

        check("void f(int x) {\n"
              "    int i[5];\n"
              "    int* j;\n"
              "    if (x)\n"
              "        j = i;\n"
              "    foo(j);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    const bool b = true;\n"
              "    x++;\n"
              "    if (x == 5)\n"
              "        foo(b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    const bool b = x;\n"
              "    x++;\n"
              "    if (x == 5)\n"
              "        foo(b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope13() {
        // #2770
        check("void f() {\n"
              "    int i = 0;\n"
              "    forever {\n"
              "        if (i++ == 42) { break; }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope14() {
        // #3941
        check("void f() {\n"
              "    const int i( foo());\n"
              "    if(a) {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope15() {
        // #4573
        check("void f() {\n"
              "    int a,b,c;\n"
              "    if (a);\n"
              "    else if(b);\n"
              "    else if(c);\n"
              "    else;\n"
              "}", nullptr, false, false);
        ASSERT_EQUALS("", errout.str());
    }

    void varScope16() {
        check("void f() {\n"
              "    int a = 0;\n"
              "    while((++a) < 56) {\n"
              "        foo();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    do {\n"
              "        foo();\n"
              "    } while((++a) < 56);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    do {\n"
              "        a = 64;\n"
              "        foo(a);\n"
              "    } while((++a) < 56);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    do {\n"
              "        a = 64;\n"
              "        foo(a);\n"
              "    } while(z());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'a' can be reduced.\n", errout.str());
    }

    void varScope17() {
        check("void f() {\n"
              "    int x;\n"
              "    if (a) {\n"
              "        x = stuff(x);\n"
              "        morestuff(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'x' can be reduced.\n", errout.str());

        check("void f() {\n"
              "    int x;\n"
              "    if (a) {\n"
              "        x = stuff(x);\n"
              "        morestuff(x);\n"
              "    }\n"
              "    if (b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'x' can be reduced.\n", errout.str());
    }

    void varScope18() {
        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            break;\n"
              "    }\n"
              "\n"
              "    if (c) {\n"
              "        x = foo();\n"
              "        do_something(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'x' can be reduced.\n", errout.str());

        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            x = 10;\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            break;\n"
              "    }\n"
              "\n"
              "    if (c) {\n"
              "        x = foo();\n"
              "        do_something(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            if(c)\n"
              "                do_something(x);\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'x' can be reduced.\n", errout.str());

        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            if(c)\n"
              "                do_something(x);\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            if(d)\n"
              "                do_something(x);\n"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope20() { // Ticket #5103 - constant variable only used in inner scope
        check("int f(int a) {\n"
              "  const int x = 234;\n"
              "  int b = a;\n"
              "  if (b > 32) b = x;\n"
              "  return b;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope21() { // Ticket #5382 - initializing two-dimensional array
        check("int test() {\n"
              "    int test_value = 3;\n"
              "    int test_array[1][1] = { { test_value } };\n"
              "    return sizeof(test_array);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope22() { // Ticket #5684 - "The scope of the variable 'p' can be reduced" - But it can not.
        check("void foo() {\n"
              "   int* p( 42 );\n"
              "   int i = 0;\n"
              "   while ( i != 100 ) {\n"
              "      *p = i;\n"
              "      ++p;\n"
              "      ++i;\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        // try to avoid an obvious false negative after applying the fix for the example above:
        check("void foo() {\n"
              "   int* p( 42 );\n"
              "   int i = 0;\n"
              "   int dummy = 0;\n"
              "   while ( i != 100 ) {\n"
              "      p = & dummy;\n"
              "      *p = i;\n"
              "      ++p;\n"
              "      ++i;\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'p' can be reduced.\n", errout.str());
    }

    void varScope23() { // #6154: Don't suggest to reduce scope if inner scope is a lambda
        check("int main() {\n"
              "   size_t myCounter = 0;\n"
              "   Test myTest([&](size_t aX){\n"
              "       std::cout << myCounter += aX << std::endl;\n"
              "   });\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope24() {
        check("void f(Foo x) {\n"
              "   Foo &r = x;\n"
              "   if (cond) {\n"
              "       r.dostuff();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'r' can be reduced.\n", errout.str());

        check("void f(Foo x) {\n"
              "   Foo foo = x;\n"
              "   if (cond) {\n"
              "       foo.dostuff();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope25() {
        check("void f() {\n"
              "    time_t currtime;\n"
              "    if (a) {\n"
              "        currtime = time(&dummy);\n"
              "        if (currtime > t) {}\n"
              "    }\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:2]: (style) The scope of the variable 'currtime' can be reduced.\n", errout.str());
    }
    void checkOldStylePointerCast(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        static Settings settings;
        settings.addEnabled("style");
        settings.standards.cpp = Standards::CPP03; // #5560

        // Tokenize..
        Tokenizer tokenizerCpp(&settings, this);
        std::istringstream istr(code);
        tokenizerCpp.tokenize(istr, "test.cpp");

        CheckOther checkOtherCpp(&tokenizerCpp, &settings, this);
        checkOtherCpp.warningOldStylePointerCast();
    }

    void oldStylePointerCast() {
        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (Base *) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base * const) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (volatile Base *) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (volatile Base * const) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const volatile Base *) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const volatile Base * const) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) ( new Derived() );\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) new Derived();\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) new short[10];\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(B *) const = 0;\n"
                                 "}");
        ASSERT_EQUALS("", errout.str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(const B *) const = 0;\n"
                                 "}");
        ASSERT_EQUALS("", errout.str());

        // #3630
        checkOldStylePointerCast("class SomeType;\n"
                                 "class X : public Base {\n"
                                 "    X() : Base((SomeType*)7) {}\n"
                                 "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class SomeType;\n"
                                 "class X : public Base {\n"
                                 "    X() : Base((SomeType*)var) {}\n"
                                 "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class SomeType;\n"
                                 "class X : public Base {\n"
                                 "    X() : Base((SomeType*)0) {}\n"
                                 "};");
        ASSERT_EQUALS("", errout.str());

        // #5560
        checkOldStylePointerCast("class C;\n"
                                 "\n"
                                 "class B\n"
                                 "{ virtual G* createGui(S*, C*) const = 0; };\n"
                                 "\n"
                                 "class MS : public M\n"
                                 "{ virtual void addController(C*) override {} };");
        ASSERT_EQUALS("", errout.str());

        // #6164
        checkOldStylePointerCast("class Base {};\n"
                                 "class Derived: public Base {};\n"
                                 "void testCC() {\n"
                                 "  std::vector<Base*> v;\n"
                                 "  v.push_back((Base*)new Derived);\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) C-style pointer casting\n", errout.str());
    }

    void checkInvalidPointerCast(const char code[], bool portability = true, bool inconclusive = false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("warning");
        if (portability)
            settings.addEnabled("portability");
        settings.inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckOther checkOtherCpp(&tokenizer, &settings, this);
        checkOtherCpp.invalidPointerCast();
    }


    void invalidPointerCast() {
        checkInvalidPointerCast("void test() {\n"
                                "    float *f = new float[10];\n"
                                "    delete [] (double*)f;\n"
                                "    delete [] (long double const*)(new float[10]);\n"
                                "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (portability) Casting between float* and double* which have an incompatible binary data representation.\n"
                           "[test.cpp:4]: (portability) Casting between float* and const long double* which have an incompatible binary data representation.\n",
                           "[test.cpp:3]: (portability) Casting between float* and double* which have an incompatible binary data representation.\n", errout.str());

        checkInvalidPointerCast("void test(const float* f) {\n"
                                "    double *d = (double*)f;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Casting between const float* and double* which have an incompatible binary data representation.\n", errout.str());

        checkInvalidPointerCast("void test(double* d1) {\n"
                                "    long double *ld = (long double*)d1;\n"
                                "    double *d2 = (double*)ld;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Casting between double* and long double* which have an incompatible binary data representation.\n"
                      "[test.cpp:3]: (portability) Casting between long double* and double* which have an incompatible binary data representation.\n", errout.str());

        checkInvalidPointerCast("char* test(int* i) {\n"
                                "    long double *d = (long double*)(i);\n"
                                "    double *d = (double*)(i);\n"
                                "    float *f = reinterpret_cast<float*>(i);\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Casting between integer* and long double* which have an incompatible binary data representation.\n"
                      "[test.cpp:3]: (portability) Casting between integer* and double* which have an incompatible binary data representation.\n"
                      "[test.cpp:4]: (portability) Casting between integer* and float* which have an incompatible binary data representation.\n", errout.str());

        checkInvalidPointerCast("float* test(unsigned int* i) {\n"
                                "    return (float*)i;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Casting between integer* and float* which have an incompatible binary data representation.\n", errout.str());

        checkInvalidPointerCast("float* test(unsigned int* i) {\n"
                                "    return (float*)i[0];\n"
                                "}");
        ASSERT_EQUALS("", errout.str());

        checkInvalidPointerCast("float* test(double& d) {\n"
                                "    return (float*)&d;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Casting between double* and float* which have an incompatible binary data representation.\n", errout.str());

        checkInvalidPointerCast("void test(float* data) {\n"
                                "    f.write((char*)data,sizeof(float));\n"
                                "}", true, false);
        ASSERT_EQUALS("", errout.str());

        checkInvalidPointerCast("void test(float* data) {\n"
                                "    f.write((char*)data,sizeof(float));\n"
                                "}", true, true); // #3639
        ASSERT_EQUALS("[test.cpp:2]: (portability, inconclusive) Casting from float* to char* is not portable due to different binary data representations on different platforms.\n", errout.str());


        checkInvalidPointerCast("long long* test(float* f) {\n"
                                "    return (long long*)f;\n"
                                "}", false);
        ASSERT_EQUALS("", errout.str());

        checkInvalidPointerCast("long long* test(float* f, char* c) {\n"
                                "    foo((long long*)f);\n"
                                "    return reinterpret_cast<long long*>(c);\n"
                                "}", true);
        ASSERT_EQUALS("[test.cpp:2]: (portability) Casting from float* to integer* is not portable due to different binary data representations on different platforms.\n", errout.str());

        checkInvalidPointerCast("Q_DECLARE_METATYPE(int*)"); // #4135 - don't crash
    }


    void passedByValue() {
        check("void f(const std::string str) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void f(std::unique_ptr<std::string> ptr) {}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::shared_ptr<std::string> ptr) {}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::function<F> ptr) {}");
        ASSERT_EQUALS("", errout.str());

        {
            check("void f(const std::pair<int,int> x) {}");
            ASSERT_EQUALS("", errout.str());

            check("void f(const std::pair<std::string,std::string> x) {}");
            TODO_ASSERT_EQUALS("error", "", errout.str());
        }

        check("void f(const std::string::size_type x) {}");
        ASSERT_EQUALS("", errout.str());

        check("class Foo;\nvoid f(const Foo foo) {}"); // Unknown class
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Function parameter 'foo' should be passed by const reference.\n", errout.str());

        check("class Foo { std::vector<int> v; };\nvoid f(const Foo foo) {}"); // Large class (STL member)
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'foo' should be passed by const reference.\n", errout.str());

        check("class Foo { int i; };\nvoid f(const Foo foo) {}"); // Small class
        ASSERT_EQUALS("", errout.str());

        check("class Foo { int i[6]; };\nvoid f(const Foo foo) {}"); // Large class (array)
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'foo' should be passed by const reference.\n", errout.str());

        check("class Foo { std::string* s; };\nvoid f(const Foo foo) {}"); // Small class (pointer)
        ASSERT_EQUALS("", errout.str());

        check("class Foo { static std::string s; };\nvoid f(const Foo foo) {}"); // Small class (static member)
        ASSERT_EQUALS("", errout.str());

        check("class X { std::string s; }; class Foo : X { };\nvoid f(const Foo foo) {}"); // Large class (inherited)
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'foo' should be passed by const reference.\n", errout.str());

        check("class X { std::string s; }; class Foo { X x; };\nvoid f(const Foo foo) {}"); // Large class (inherited)
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'foo' should be passed by const reference.\n", errout.str());

        check("void f(const std::string &str) {}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::vector<int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by const reference.\n", errout.str());

        check("void f(const std::vector<std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by const reference.\n", errout.str());

        check("void f(const std::vector<std::string>::size_type s) {}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::vector<int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::map<int,int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::map<int,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by const reference.\n", errout.str());

        check("void f(const std::map<std::string,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by const reference.\n", errout.str());

        check("void f(const std::map<int,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by const reference.\n", errout.str());

        check("void f(const std::map<std::string,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by const reference.\n", errout.str());

        check("void f(const std::streamoff pos) {}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::initializer_list<int> i) {}");
        ASSERT_EQUALS("", errout.str());

        // #5824
        check("void log(const std::string& file, int line, const std::string& function, const std::string str, ...) {}");
        ASSERT_EQUALS("", errout.str());

        // #5534
        check("struct float3 { };\n"
              "typedef float3 vec;\n"
              "class Plane {\n"
              "    vec Refract(vec &vec) const;\n"
              "    bool IntersectLinePlane(const vec &planeNormal);\n"
              "}; ");
        ASSERT_EQUALS("", errout.str());

        check("class X {\n"
              "    virtual void func(const std::string str) {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());
    }

    void passedByValue_nonConst() {
        check("void f(std::string str) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void f(std::string str) {\n"
              "    return str + x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void f(std::string str) {\n"
              "    std::cout << str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void f(std::string str) {\n"
              "    std::cin >> str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::string str) {\n"
              "    std::string s2 = str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void f(std::string str) {\n"
              "    std::string& s2 = str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::string str) {\n"
              "    const std::string& s2 = str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void f(std::string str) {\n"
              "    str = \"\";\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::string str) {\n"
              "    foo(str);\n" // It could be that foo takes str as non-const-reference
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(const std::string& str);\n"
              "void f(std::string str) {\n"
              "    foo(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void foo(std::string str);\n"
              "void f(std::string str) {\n"
              "    foo(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("void foo(std::string& str);\n"
              "void f(std::string str) {\n"
              "    foo(str);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(std::string* str);\n"
              "void f(std::string str) {\n"
              "    foo(&str);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int& i1, const std::string& str, int& i2);\n"
              "void f(std::string str) {\n"
              "    foo((a+b)*c, str, x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("std::string f(std::string str) {\n"
              "    str += x;\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class X {\n"
              "    std::string s;\n"
              "    void func() const;\n"
              "};\n"
              "Y f(X x) {\n"
              "    x.func();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (performance) Function parameter 'x' should be passed by const reference.\n", errout.str());

        check("class X {\n"
              "    void func();\n"
              "};\n"
              "Y f(X x) {\n"
              "    x.func();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class X {\n"
              "    void func(std::string str) {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'str' should be passed by const reference.\n", errout.str());

        check("class X {\n"
              "    virtual void func(std::string str) {}\n" // Do not warn about virtual functions, if 'str' is not declared as const
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class X {\n"
              "    char a[1024];\n"
              "};\n"
              "class Y : X {\n"
              "    char b;\n"
              "};\n"
              "void f(Y y) {\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (performance) Function parameter 'y' should be passed by const reference.\n", errout.str());

        check("class X {\n"
              "    void* a;\n"
              "    void* b;\n"
              "};\n"
              "class Y {\n"
              "    void* a;\n"
              "    void* b;\n"
              "    char c;\n"
              "};\n"
              "void f(X x, Y y) {\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (performance) Function parameter 'y' should be passed by const reference.\n", errout.str());

        {
            // 8-byte data should be passed by const reference on 32-bit platform but not on 64-bit platform
            const char code[] = "class X {\n"
                                "    uint64_t a;\n"
                                "    uint64_t b;\n"
                                "};\n"
                                "void f(X x) {}";

            Settings s32(_settings);
            s32.platform(cppcheck::Platform::Unix32);
            check(code, &s32);
            ASSERT_EQUALS("[test.cpp:5]: (performance) Function parameter 'x' should be passed by const reference.\n", errout.str());

            Settings s64(_settings);
            s64.platform(cppcheck::Platform::Unix64);
            check(code, &s64);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void switchRedundantAssignmentTest() {
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:9]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y = 2;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:11]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        if (x)\n"
              "        {\n"
              "            y = 3;\n"
              "        }\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y = 2;\n"
              "        if (z)\n"
              "            printf(\"%d\", y);\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int x = a;\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        x = 2;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        break;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "            continue;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "            throw e;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        printf(\"%d\", y);\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        bar();\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:10]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());

        check("void bar() {}\n" // bar isn't noreturn
              "void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        bar();\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:11]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "    }\n"
              "}", 0, false, false, false);
        // TODO ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:8]: (warning) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n", errout.str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strncpy(str, \"a'\");\n"
              "    case 3:\n"
              "      strncpy(str, \"b'\");\n"
              "    }\n"
              "}");
        // TODO ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:8]: (warning) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n", errout.str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    int z = 0;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "      z++;\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "      z++;\n"
              "    }\n"
              "}", nullptr, false, false, false);
        // TODO ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:10]: (warning) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n", errout.str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "      break;\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "      break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "      printf(str);\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "    }\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        // Ticket #5158 "segmentation fault (valid code)"
        check("typedef struct ct_data_s {\n"
              "    union {\n"
              "        char freq;\n"
              "    } fc;\n"
              "} ct_data;\n"
              "typedef struct internal_state {\n"
              "    struct ct_data_s dyn_ltree[10];\n"
              "} deflate_state;\n"
              "void f(deflate_state *s) {\n"
              "    s->dyn_ltree[0].fc.freq++;\n"
              "}\n", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        // Ticket #6132 "crash: daca: kvirc CheckOther::checkRedundantAssignment()"
        check("void HttpFileTransfer :: transferTerminated ( bool bSuccess@1 ) {\n"
              "if ( m_szCompletionCallback . isNull ( ) ) {\n"
              "KVS_TRIGGER_EVENT ( KviEvent_OnHTTPGetTerminated , out ? out : ( g_pApp . activeConsole ( ) ) , & vParams )\n"
              "} else {\n"
              "KviKvsScript :: run ( m_szCompletionCallback , out ? out : ( g_pApp . activeConsole ( ) ) , & vParams ) ;\n"
              "}\n"
              "}\n", nullptr, false, false, true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int x;\n"
              "  switch (state) {\n"
              "  case 1: x = 3; goto a;\n"
              "  case 1: x = 6; goto a;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void switchRedundantOperationTest() {
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        ++y;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:9]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        ++y;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:11]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y;\n"
              "    case 3:\n"
              "        ++y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        ++y;\n"
              "    case 3:\n"
              "        ++y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        --y;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:9]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        --y;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:11]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y;\n"
              "    case 3:\n"
              "        --y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        --y;\n"
              "    case 3:\n"
              "        --y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:9]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y++;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:11]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        y++;\n"
              "    }\n"
              "    bar(y);\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "    case 3:\n"
              "        y++;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y--;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:9]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y--;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:11]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        y--;\n"
              "    }\n"
              "    bar(y);\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y--;\n"
              "    case 3:\n"
              "        y--;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "    case 3:\n"
              "        if (x)\n"
              "        {\n"
              "            y = 3;\n"
              "        }\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y++;\n"
              "        if (y)\n"
              "            printf(\"%d\", y);\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int x = a;\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        x++;\n"
              "    case 3:\n"
              "        y++;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "        break;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y++;\n"
              "            continue;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y++;\n"
              "            throw e;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "        printf(\"%d\", y);\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "        bar();\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:10]: (warning) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing?\n", errout.str());
    }

    void switchRedundantBitwiseOperationTest() {
        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Redundant bitwise operation on 'y' in 'switch' statement. 'break;' missing?\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "    default:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Redundant bitwise operation on 'y' in 'switch' statement. 'break;' missing?\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "    default:\n"
              "        if (z)\n"
              "            y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= z;\n"
              "        z++\n"
              "    default:\n"
              "        y |= z;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "        bar(y);\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "        y = 4;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:8]: (style) Variable 'y' is reassigned a value before the old one has been used.\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y &= 3;\n"
              "    case 3:\n"
              "        y &= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Redundant bitwise operation on 'y' in 'switch' statement. 'break;' missing?\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y ^= 3;\n"
              "    case 3:\n"
              "        y ^= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 2;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y &= 2;\n"
              "    case 3:\n"
              "        y &= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 2;\n"
              "    case 3:\n"
              "        y &= 2;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void unreachableCode() {
        check("void foo(int a) {\n"
              "    while(1) {\n"
              "        if (a++ >= 100) {\n"
              "            break;\n"
              "            continue;\n"
              "        }\n"
              "    }\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("int foo(int a) {\n"
              "    return 0;\n"
              "    return(a-1);\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("int foo(int a) {\n"
              "  A:"
              "    return(0);\n"
              "    goto A;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        Settings settings;
        settings.library.setnoreturn("exit", true);
        settings.library.functions["exit"].argumentChecks[1] = Library::ArgumentChecks();
        check("void foo() {\n"
              "    exit(0);\n"
              "    break;\n"
              "}", nullptr, false, false, false, &settings);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("class NeonSession {\n"
              "    void exit();\n"
              "};\n"
              "void NeonSession::exit()\n"
              "{\n"
              "    SAL_INFO(\"ucb.ucp.webdav\", \"neon commands cannot be aborted\");\n"
              "}", nullptr, false, false, false, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void NeonSession::exit()\n"
              "{\n"
              "    SAL_INFO(\"ucb.ucp.webdav\", \"neon commands cannot be aborted\");\n"
              "}", nullptr, false, false, false, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo() { xResAccess->exit(); }", nullptr, false, false, false, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        switch(a) {\n"
              "          case 0:\n"
              "            printf(\"case 0\");\n"
              "            break;\n"
              "            break;\n"
              "          case 1:\n"
              "            c++;\n"
              "            break;\n"
              "         }\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:7]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        switch(a) {\n"
              "          case 0:\n"
              "            printf(\"case 0\");\n"
              "            break;\n"
              "          case 1:\n"
              "            c++;\n"
              "            break;\n"
              "         }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        while(true) {\n"
              "          if (a++ >= 100) {\n"
              "            break;\n"
              "            break;\n"
              "          }\n"
              "       }\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        while(true) {\n"
              "          if (a++ >= 100) {\n"
              "            continue;\n"
              "            continue;\n"
              "          }\n"
              "          a+=2;\n"
              "       }\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        while(true) {\n"
              "          if (a++ >= 100) {\n"
              "            continue;\n"
              "          }\n"
              "          a+=2;\n"
              "       }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo() {\n"
              "    throw 0;\n"
              "    return 1;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    throw 0;\n"
              "    return;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("int foo() {\n"
              "    return 0;\n"
              "    return 1;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("int foo() {\n"
              "    return 0;\n"
              "    foo();\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Statements following return, break, continue, goto or throw will never be executed.\n", errout.str());

        check("int foo(int unused) {\n"
              "    return 0;\n"
              "    (void)unused;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        check("int foo(int unused1, int unused2) {\n"
              "    return 0;\n"
              "    (void)unused1;\n"
              "    (void)unused2;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        check("int foo(int unused1, int unused2) {\n"
              "    return 0;\n"
              "    (void)unused1;\n"
              "    (void)unused2;\n"
              "    foo();\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:5]: (style) Statements following return, break, continue, goto or throw will never be executed.\n", errout.str());

        check("int foo() {\n"
              "    if(bar)\n"
              "        return 0;\n"
              "    return 124;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo() {\n"
              "    while(bar) {\n"
              "        return 0;\n"
              "        return 0;\n"
              "        return 0;\n"
              "        return 0;\n"
              "    }\n"
              "    return 124;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        check("void foo() {\n"
              "    while(bar) {\n"
              "        return;\n"
              "        break;\n"
              "    }\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        // #5707
        check("extern int i,j;\n"
              "int foo() {\n"
              "    switch(i) {\n"
              "        default: j=1; break;\n"
              "    }\n"
              "    return 0;\n"
              "    j=2;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:7]: (style) Statements following return, break, continue, goto or throw will never be executed.\n", errout.str());

        check("int foo() {\n"
              "    return 0;\n"
              "  label:\n"
              "    throw 0;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Label 'label' is not used.\n", errout.str());

        check("void foo() {\n"
              "    wxCHECK2(state < 3 && state >= 0, return);\n"
              "    _checkboxState = state;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    virtual void foo  (P & Val) throw ();\n"
              "    virtual void foo1 (P & Val) throw ();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo() {\n"
              "    goto label;\n"
              "    while (true) {\n"
              "     bar();\n"
              "     label:\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str()); // #3457

        check("int foo() {\n"
              "    goto label;\n"
              "    do {\n"
              "     bar();\n"
              "     label:\n"
              "    } while (true);\n"
              "}");
        ASSERT_EQUALS("", errout.str()); // #3457

        check("int foo() {\n"
              "    goto label;\n"
              "    for (;;) {\n"
              "     bar();\n"
              "     label:\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str()); // #3457

        // #3383. TODO: Use preprocessor
        check("int foo() {\n"
              "\n" // #ifdef A
              "    return 0;\n"
              "\n" // #endif
              "    return 1;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());
        check("int foo() {\n"
              "\n" // #ifdef A
              "    return 0;\n"
              "\n" // #endif
              "    return 1;\n"
              "}", nullptr, false, true, false);
        ASSERT_EQUALS("[test.cpp:5]: (style, inconclusive) Consecutive return, break, continue, goto or throw statements are unnecessary.\n", errout.str());

        // #4711 lambda functions
        check("int f() {\n"
              "    return g([](int x){x+1; return x;});\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        // #4756
        check("template <>\n"
              "inline uint16_t htobe(uint16_t value) {\n"
              "     return ( __extension__ ({\n"
              "         register unsigned short int __v, __x = (unsigned short int) (value);\n"
              "         if (__builtin_constant_p (__x))\n"
              "             __v = ((unsigned short int) ((((__x) >> 8) & 0xff) | (((__x) & 0xff) << 8)));\n"
              "         else\n"
              "             __asm__ (\"rorw $8, %w0\" : \"=r\" (__v) : \"0\" (__x) : \"cc\");\n"
              "         __v;\n"
              "     }));\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        // #6008
        check("static std::function< int ( int, int ) > GetFunctor() {\n"
              "    return [](int a_, int b_) -> int {\n"
              "        int sum = a_ + b_;\n"
              "        return sum;\n"
              "    };\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        // #5789
        check("struct per_state_info {\n"
              "    uint64_t enter, exit;\n"
              "    uint64_t events;\n"
              "    per_state_info() : enter(0), exit(0), events(0) {}\n"
              "};", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        // #6664
        check("void foo() {\n"
              "    (beat < 100) ? (void)0 : exit(0);\n"
              "    bar();\n"
              "}", nullptr, false, false, false, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    (beat < 100) ? exit(0) : (void)0;\n"
              "    bar();\n"
              "}", nullptr, false, false, false, &settings);
        ASSERT_EQUALS("", errout.str());

        // #8261
        check("void foo() {\n"
              "    (beat < 100) ? (void)0 : throw(0);\n"
              "    bar();\n"
              "}", nullptr, false, false, false, &settings);
        ASSERT_EQUALS("", errout.str());
    }


    void suspiciousCase() {
        check("void foo() {\n"
              "    switch(a) {\n"
              "        case A&&B:\n"
              "            foo();\n"
              "        case (A||B):\n"
              "            foo();\n"
              "        case A||B:\n"
              "            foo();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Found suspicious case label in switch(). Operator '&&' probably doesn't work as intended.\n"
                      "[test.cpp:5]: (warning, inconclusive) Found suspicious case label in switch(). Operator '||' probably doesn't work as intended.\n"
                      "[test.cpp:7]: (warning, inconclusive) Found suspicious case label in switch(). Operator '||' probably doesn't work as intended.\n", errout.str());

        check("void foo() {\n"
              "    switch(a) {\n"
              "        case 1:\n"
              "            a=A&&B;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    switch(a) {\n"
              "        case A&&B?B:A:\n"
              "            foo();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void suspiciousEqualityComparison() {
        check("void foo(int c) {\n"
              "    if (c == 1) c == 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());

        check("void foo(int* c) {\n"
              "    if (*c == 1) *c == 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());


        check("void foo(int c) {\n"
              "    if (c == 1) {\n"
              "        c = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int c) {\n"
              "    c == 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());

        check("void foo(int c) {\n"
              "    for (int i = 0; i == 10; i ++) {\n"
              "        a ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int c) {\n"
              "    for (i == 0; i < 10; i ++) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());

        check("void foo(int c) {\n"
              "    for (i == 1; i < 10; i ++) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());

        check("void foo(int c) {\n"
              "    for (i == 2; i < 10; i ++) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());

        check("void foo(int c) {\n"
              "    for (int i = 0; i < 10; i == c) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());

        check("void foo(int c) {\n"
              "    for (; running == 1;) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int c) {\n"
              "    printf(\"%i\", ({x==0;}));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x) {\n"
              "    printf(\"%i\", ({int x = do_something(); x == 0;}));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x) {\n"
              "    printf(\"%i\", ({x == 0; x > 0 ? 10 : 20}));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead?\n", errout.str());

        check("void foo(int x) {\n"
              "    for (const Token* end = tok->link(); tok != end; tok = (tok == end) ? end : tok->next()) {\n"
              "        x++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x) {\n"
              "    for (int i = (x == 0) ? 0 : 5; i < 10; i ++) {\n"
              "        x++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x) {\n"
              "    for (int i = 0; i < 10; i += (x == 5) ? 1 : 2) {\n"
              "        x++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void selfAssignment() {
        check("void foo()\n"
              "{\n"
              "    int x = 1;\n"
              "    x = x;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Redundant assignment of 'x' to itself.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int x = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of 'x' to itself.\n", errout.str());

        check("struct A { int b; };\n"
              "void foo(A* a1, A* a2) {\n"
              "    a1->b = a1->b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of 'a1->b' to itself.\n", errout.str());

        // #4073 (segmentation fault)
        check("void Foo::myFunc( int a )\n"
              "{\n"
              "    if (a == 42)\n"
              "    a = a;\n"
              "}");

        check("void foo()\n"
              "{\n"
              "    int x = 1;\n"
              "    x = x + 1;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int *x = getx();\n"
              "        *x = x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    BAR *x = getx();\n"
              "    x = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of 'x' to itself.\n", errout.str());

        // #2502 - non-primitive type -> there might be some side effects
        check("void foo()\n"
              "{\n"
              "    Fred fred; fred = fred;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    x = (x == 0);"
              "    func(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    x = (x != 0);"
              "    func(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ticket #3001 - false positive
        check("void foo(int x) {\n"
              "    x = x ? x : 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3800 - false negative when variable is extern
        check("extern int i;\n"
              "void f() {\n"
              "    i = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of 'i' to itself.\n", errout.str());

        // #4291 - id for variables accessed through 'this'
        check("class Foo {\n"
              "    int var;\n"
              "    void func();\n"
              "};\n"
              "void Foo::func() {\n"
              "    this->var = var;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Redundant assignment of 'this->var' to itself.\n", errout.str());

        check("class Foo {\n"
              "    int var;\n"
              "    void func(int var);\n"
              "};\n"
              "void Foo::func(int var) {\n"
              "    this->var = var;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6406 - designated initializer doing bogus self assignment
        check("struct callbacks {\n"
              "    void (*s)(void);\n"
              "};\n"
              "void something(void) {}\n"
              "void f() {\n"
              "    struct callbacks ops = { .s = ops.s };\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (warning) Redundant assignment of 'something' to itself.\n", "", errout.str());

        check("class V\n"
              "{\n"
              "public:\n"
              "    V()\n"
              "    {\n"
              "        x = y = z = 0.0;\n"
              "    }\n"
              "    V( double x, const double y, const double &z )\n"
              "    {\n"
              "        x = x; y = y; z = z;\n"
              "    }\n"
              "    double x, y, z;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Redundant assignment of 'x' to itself.\n"
                      "[test.cpp:10]: (warning) Redundant assignment of 'y' to itself.\n"
                      "[test.cpp:10]: (warning) Redundant assignment of 'z' to itself.\n", errout.str());

        check("void f(int i) { i = !!i; }");
        ASSERT_EQUALS("", errout.str());

    }

    void trac1132() {
        check("class Lock\n"
              "{\n"
              "public:\n"
              "    Lock(int i)\n"
              "    {\n"
              "        std::cout << \"Lock \" << i << std::endl;\n"
              "    }\n"
              "    ~Lock()\n"
              "    {\n"
              "        std::cout << \"~Lock\" << std::endl;\n"
              "    }\n"
              "};\n"
              "int main()\n"
              "{\n"
              "    Lock(123);\n"
              "    std::cout << \"hello\" << std::endl;\n"
              "    return 0;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:15]: (style) Instance of 'Lock' object is destroyed immediately.\n", errout.str());
    }

    void trac3693() {
        check("struct A{\n"
              "  enum {\n"
              "    b = 300\n"
              "  };\n"
              "};\n"
              "const int DFLT_TIMEOUT = A::b % 1000000 ;\n", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickFunction1() {
        check("int main ( )\n"
              "{\n"
              "    CouldBeFunction ( 123 ) ;\n"
              "    return 0 ;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickFunction2() {
        check("struct error {\n"
              "    error() {}\n"
              "};\n"
              "\n"
              "class parser {\n"
              "public:\n"
              "    void error() const {}\n"
              "\n"
              "    void foo() const {\n"
              "        error();\n"
              "        do_something();\n"
              "    }\n"
              "};\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectPicksClass() {
        check("class NotAFunction ;\n"
              "int function ( )\n"
              "{\n"
              "    NotAFunction ( 123 );\n"
              "    return 0 ;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:4]: (style) Instance of 'NotAFunction' object is destroyed immediately.\n", errout.str());
    }

    void testMisusedScopeObjectPicksStruct() {
        check("struct NotAClass;\n"
              "bool func ( )\n"
              "{\n"
              "    NotAClass ( 123 ) ;\n"
              "    return true ;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:4]: (style) Instance of 'NotAClass' object is destroyed immediately.\n", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickIf() {
        check("bool func( int a , int b , int c )\n"
              "{\n"
              "    if ( a > b ) return c == a ;\n"
              "    return b == a ;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickConstructorDeclaration() {
        check("class Something : public SomethingElse\n"
              "{\n"
              "public:\n"
              "~Something ( ) ;\n"
              "Something ( ) ;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickFunctor() {
        check("class IncrementFunctor\n"
              "{\n"
              "public:\n"
              "    void operator()(int &i)\n"
              "    {\n"
              "        ++i;\n"
              "    }\n"
              "};\n"
              "\n"
              "int main()\n"
              "{\n"
              "    int a = 1;\n"
              "    IncrementFunctor()(a);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickLocalClassConstructors() {
        check("void f() {\n"
              "    class Foo {\n"
              "        Foo() { }\n"
              "        Foo(int a) { }\n"
              "        Foo(int a, int b) { }\n"
              "    };\n"
              "    Foo();\n"
              "    do_something();\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:7]: (style) Instance of 'Foo' object is destroyed immediately.\n", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickUsedObject() {
        check("struct Foo {\n"
              "    void bar() {\n"
              "    }\n"
              "};\n"
              "\n"
              "void fn() {\n"
              "    Foo().bar();\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickPureC() {
        // Ticket #2352
        const char code[] = "struct cb_watch_bool {\n"
                            "    int a;\n"
                            "};\n"
                            "\n"
                            "void f()\n"
                            "{\n"
                            "    cb_watch_bool();\n"
                            "    do_something();\n"
                            "}\n";

        check(code, "test.cpp");
        ASSERT_EQUALS("[test.cpp:7]: (style) Instance of 'cb_watch_bool' object is destroyed immediately.\n", errout.str());

        check(code, "test.c");
        ASSERT_EQUALS("", errout.str());

        // Ticket #2639
        check("struct stat { int a; int b; };\n"
              "void stat(const char *fn, struct stat *);\n"
              "\n"
              "void foo() {\n"
              "    stat(\"file.txt\", &st);\n"
              "    do_something();\n"
              "}");
        ASSERT_EQUALS("",errout.str());
    }

    void testMisusedScopeObjectDoesNotPickNestedClass() {
        const char code[] = "class ios_base {\n"
                            "public:\n"
                            "  class Init {\n"
                            "  public:\n"
                            "  };\n"
                            "};\n"
                            "class foo {\n"
                            "public:\n"
                            "  foo();\n"
                            "  void Init(int);\n"
                            "};\n"
                            "foo::foo() {\n"
                            "  Init(0);\n"
                            "  do_something();\n"
                            "}\n";

        check(code, "test.cpp");
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectInConstructor() {
        const char code[] = "class Foo {\n"
                            "public:\n"
                            "  Foo(char x) {\n"
                            "    Foo(x, 0);\n"
                            "    do_something();\n"
                            "  }\n"
                            "  Foo(char x, int y) { }\n"
                            "};\n";
        check(code, "test.cpp");
        ASSERT_EQUALS("[test.cpp:4]: (style) Instance of 'Foo' object is destroyed immediately.\n", errout.str());
    }

    void testMisusedScopeObjectNoCodeAfter() {
        check("class Foo {};\n"
              "void f() {\n"
              "  Foo();\n" // No code after class => don't warn
              "}", "test.cpp");
        ASSERT_EQUALS("", errout.str());
    }

    void trac2084() {
        check("void f()\n"
              "{\n"
              "    struct sigaction sa;\n"
              "\n"
              "    { sigaction(SIGHUP, &sa, 0); };\n"
              "    { sigaction(SIGINT, &sa, 0); };\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void trac2071() {
        check("void f() {\n"
              "    struct AB {\n"
              "        AB(int a) { }\n"
              "    };\n"
              "\n"
              "    const AB ab[3] = { AB(0), AB(1), AB(2) };\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void clarifyCalculation() {
        check("int f(char c) {\n"
              "    return 10 * (c == 0) ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '*' and '?'.\n", errout.str());

        check("void f(char c) {\n"
              "    printf(\"%i\", 10 * (c == 0) ? 1 : 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '*' and '?'.\n", errout.str());

        check("void f() {\n"
              "    return (2*a)?b:c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(char c) {\n"
              "    printf(\"%i\", 1 + 1 ? 1 : 2);\n" // "1+1" is simplified away
              "}",0,false,false,false);
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '+' and '?'.\n", "", errout.str()); // TODO: Is that really necessary, or is this pattern too unlikely?

        check("void f() {\n"
              "    std::cout << x << 1 ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '<<' and '?'.\n", errout.str());

        check("void f() {\n"
              "    int ab = a - b ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '-' and '?'.\n", errout.str());

        check("void f() {\n"
              "    int ab = a | b ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '|' and '?'.\n", errout.str());

        // ticket #195
        check("int f(int x, int y) {\n"
              "    return x >> ! y ? 8 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '>>' and '?'.\n", errout.str());

        check("int f() {\n"
              "   return shift < sizeof(int64_t)*8 ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() { a = *p ? 1 : 2; }");
        ASSERT_EQUALS("", errout.str());
    }

    void clarifyStatement() {
        check("char* f(char* c) {\n"
              "    *c++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n", errout.str());

        check("char* f(char** c) {\n"
              "    *c[5]--;\n"
              "    return *c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n", errout.str());

        check("void f(Foo f) {\n"
              "    *f.a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n", errout.str());

        check("void f(Foo f) {\n"
              "    *f.a[5].v[3]++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n", errout.str());

        check("void f(Foo f) {\n"
              "    *f.a(1, 5).v[x + y]++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n", errout.str());

        check("char* f(char* c) {\n"
              "    (*c)++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(char* c) {\n"
              "    bar(*c++);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("char*** f(char*** c) {\n"
              "    ***c++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n", errout.str());

        check("char** f(char*** c) {\n"
              "    **c[5]--;\n"
              "    return **c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective statement similar to '*A++;'. Did you intend to write '(*A)++;'?\n", errout.str());

        check("char*** f(char*** c) {\n"
              "    (***c)++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void *f(char** c) {\n"
              "    bar(**c++);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void *f(char* p) {\n"
              "    for (p = path; *p++;) ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateBranch() {
        check("void f(int a, int &b) {\n"
              "    if (a)\n"
              "        b = 1;\n"
              "    else\n"
              "        b = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) Found duplicate branches for 'if' and 'else'.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a) {\n"
              "        if (a == 1)\n"
              "            b = 2;\n"
              "        else\n"
              "            b = 2;\n"
              "    } else\n"
              "        b = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Found duplicate branches for 'if' and 'else'.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1)\n"
              "        b = 1;\n"
              "    else {\n"
              "        if (a)\n"
              "            b = 2;\n"
              "        else\n"
              "            b = 2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5]: (style, inconclusive) Found duplicate branches for 'if' and 'else'.\n", errout.str());

        check("int f(int signed, unsigned char value) {\n"
              "    int ret;\n"
              "    if (signed)\n"
              "        ret = (signed char)value;\n"  // cast must be kept so the simplifications and verification is skipped
              "    else\n"
              "        ret = (unsigned char)value;\n"
              "    return ret;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (b)\n"
              "        __asm__(\"mov ax, bx\");\n"
              "    else\n"
              "        __asm__(\"mov bx, bx\");\n"
              "}");
        ASSERT_EQUALS("", errout.str()); // #3407

        check("void f() {\n"
              "    if (b)\n"
              "        __asm__(\"mov ax, bx\");\n"
              "    else\n"
              "        __asm__(\"mov ax, bx\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) Found duplicate branches for 'if' and 'else'.\n", errout.str());
    }

    void duplicateBranch1() {

        // tests inspired by http://www.viva64.com/en/b/0149/ ( Comparison between PVS-Studio and cppcheck )
        // Errors detected in Quake 3: Arena by PVS-Studio: Fragment 2
        check("void f()\n"
              "{\n"
              "  if (front < 0)\n"
              "    frac = front/(front-back);\n"
              "  else\n"
              "    frac = front/(front-back);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Found duplicate branches for 'if' and 'else'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  if (front < 0)\n"
              "  { frac = front/(front-back);}\n"
              "  else\n"
              "    frac = front/((front-back));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Found duplicate branches for 'if' and 'else'.\n", errout.str());

        // No message about empty branches (#5354)
        check("void f()\n"
              "{\n"
              "  if (front < 0)\n"
              "  {}\n"
              "  else\n"
              "  {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateBranch2() {
        checkP("#define DOSTUFF1 ;\n"
               "#define DOSTUFF2 ;\n"
               "void f(int x) {\n" // #4329
               "  if (x)\n"
               "    DOSTUFF1\n"
               "  else\n"
               "    DOSTUFF2\n"
               "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateBranch3() {
        check("void f(bool b, int i) {\n"
              "    int j = i;\n"
              "    if (b) {\n"
              "        x = i;\n"
              "    } else {\n"
              "        x = j;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Found duplicate branches for 'if' and 'else'.\n", errout.str());

        check("void f(bool b, int i) {\n"
              "    int j = i;\n"
              "    i++;\n"
              "    if (b) {\n"
              "        x = i;\n"
              "    } else {\n"
              "        x = j;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpression1() {
        check("void foo(int a) {\n"
              "    if (a == a) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void fun(int b) {\n"
              "    return  a && a ||\n"
              "            b == b &&\n"
              "            d > d &&\n"
              "            e < e &&\n"
              "            f ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&&'.\n"
                      "[test.cpp:3]: (style) Same expression on both sides of '=='.\n"
                      "[test.cpp:4]: (style) Same expression on both sides of '>'.\n"
                      "[test.cpp:5]: (style) Same expression on both sides of '<'.\n", errout.str());

        check("void foo() {\n"
              "    return a && a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo() {\n"
              "    a = b && b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo(int b) {\n"
              "    f(a,b == b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void foo(int b) {\n"
              "    f(b == b, a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void foo() {\n"
              "    if (x!=2 || x!=2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if ((a < b) && (b > a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&&' because 'a<b' and 'b>a' represent the same value.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if ((a <= b) && (b >= a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&&' because 'a<=b' and 'b>=a' represent the same value.\n", errout.str());

        check("void foo() {\n"
              "    if (x!=2 || y!=3 || x!=2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (x!=2 && (x=y) && x!=2) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if (a && b || a && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (a && b || b && c) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if (a && b | b && c) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '|'.\n", errout.str());

        check("void foo() {\n"
              "    if ((a + b) | (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '|'.\n", errout.str());

        check("void foo() {\n"
              "    if ((a | b) & (a | b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&'.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if ((a | b) == (a | b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void foo() {\n"
              "    if (a1[a2[c & 0xff] & 0xff]) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void d(const char f, int o, int v)\n"
              "{\n"
              "     if (((f=='R') && (o == 1) && ((v < 2) || (v > 99))) ||\n"
              "         ((f=='R') && (o == 2) && ((v < 2) || (v > 99))) ||\n"
              "         ((f=='T') && (o == 2) && ((v < 200) || (v > 9999)))) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f(int x) { return x+x; }");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) { while (x+=x) ; }");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if (a && b && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo() {\n"
              "    if (a || b || b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (a / 1000 / 1000) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo(int i) {\n"
              "    return i/i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '/'.\n", errout.str());

        check("void foo() {\n"
              "    if (a << 1 << 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() { return !!y; }"); // No FP
        ASSERT_EQUALS("", errout.str());

        // make sure there are not "same expression" fp when there are different casts
        check("void f(long x) { if ((int32_t)x == (int64_t)x) {} }",
              nullptr,  // filename
              false, // experimental
              false, // inconclusive
              false, // runSimpleChecks
              nullptr   // settings
             );
        ASSERT_EQUALS("", errout.str());

        // make sure there are not "same expression" fp when there are different ({}) expressions
        check("void f(long x) { if (({ 1+2; }) == ({3+4;})) {} }");
        ASSERT_EQUALS("", errout.str());

        // #5535: Reference named like its type
        check("void foo() { UMSConfig& UMSConfig = GetUMSConfiguration(); }");
        ASSERT_EQUALS("", errout.str());

        // #3868 - false positive (same expression on both sides of |)
        check("void f(int x) {\n"
              "    a = x ? A | B | C\n"
              "          : A | B;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const Bar &bar) {\n"
              "    bool a = bar.isSet() && bar->isSet();\n"
              "    bool b = bar.isSet() && bar.isSet();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Same expression on both sides of '&&'.\n", errout.str());


        check("void foo(int a, int b) {\n"
              "    if ((b + a) | (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '|' because 'b+a' and 'a+b' represent the same value.\n", errout.str());

        check("void foo(const std::string& a, const std::string& b) {\n"
              "  return a.find(b+\"&\") || a.find(\"&\"+b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a, int b) {\n"
              "    if ((b > a) | (a > b)) {}\n" // > is not commutative
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(double a, double b) {\n"
              "    if ((b + a) > (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The expression 'b+a > a+b' is always false because 'b+a' and 'a+b' represent the same value.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x == 1) && (x == 0x00000001))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    if (Four == 4) {}"
              "}", nullptr, false, true, false);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    static_assert(Four == 4, "");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    static_assert(4 == Four, "");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    enum { FourInEnumOne = 4 };\n"
              "    enum { FourInEnumTwo = 4 };\n"
              "    if (FourInEnumOne == FourInEnumTwo) {}\n"
              "}", nullptr, false, true, false);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    enum { FourInEnumOne = 4 };\n"
              "    enum { FourInEnumTwo = 4 };\n"
              "    static_assert(FourInEnumOne == FourInEnumTwo, "");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a, int b) {\n"
              "    if (sizeof(a) == sizeof(a)) { }\n"
              "    if (sizeof(a) == sizeof(b)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("float bar(int) __attribute__((pure));\n"
              "char foo(int) __attribute__((pure));\n"
              "int test(int a, int b) {\n"
              "    if (bar(a) == bar(a)) { }\n"
              "    if (unknown(a) == unknown(a)) { }\n"
              "    if (foo(a) == foo(a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Same expression on both sides of '=='.\n", errout.str());
    }

    void duplicateExpression2() { // check if float is NaN or Inf
        check("int f(long double ldbl, double dbl, float flt) {\n" // ticket #2730
              "    if (ldbl != ldbl) have_nan = 1;\n"
              "    if (!(dbl == dbl)) have_nan = 1;\n"
              "    if (flt != flt) have_nan = 1;\n"
              "    return have_nan;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("float f(float x) { return x-x; }"); // ticket #4485 (Inf)
        ASSERT_EQUALS("", errout.str());

        check("float f(float x) { return (X double)x == (X double)x; }", nullptr, false, false, false);
        ASSERT_EQUALS("", errout.str());

        check("struct X { float f; };\n"
              "float f(struct X x) { return x.f == x.f; }");
        ASSERT_EQUALS("", errout.str());

        check("struct X { int i; };\n"
              "int f(struct X x) { return x.i == x.i; }");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        // #5284 - when type is unknown, assume it's float
        check("int f() { return x==x; }");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpression3() {
        Settings settings;
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"mystrcmp\">\n"
                               "    <pure/>\n"
                               "    <arg nr=\"1\"/>\n"
                               "    <arg nr=\"2\"/>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        settings.library.load(doc);

        check("void foo() {\n"
              "    if (x() || x()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "  void foo() const;\n"
              "  bool bar() const;\n"
              "};\n"
              "void A::foo() const {\n"
              "    if (bar() && bar()) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("struct A {\n"
              "  void foo();\n"
              "  bool bar();\n"
              "  bool bar() const;\n"
              "};\n"
              "void A::foo() {\n"
              "    if (bar() && bar()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class B {\n"
              "    void bar(int i);\n"
              "};\n"
              "class A {\n"
              "    void bar(int i) const;\n"
              "};\n"
              "void foo() {\n"
              "    B b;\n"
              "    A a;\n"
              "    if (b.bar(1) && b.bar(1)) {}\n"
              "    if (a.bar(1) && a.bar(1)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("class D { void strcmp(); };\n"
              "void foo() {\n"
              "    D d;\n"
              "    if (d.strcmp() && d.strcmp()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if ((mystrcmp(a, b) == 0) || (mystrcmp(a, b) == 0)) {}\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void GetValue() { return rand(); }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void __attribute__((const)) GetValue() { return X; }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void GetValue() __attribute__((const));\n"
              "void GetValue() { return X; }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (str == \"(\" || str == \"(\") {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (bar(a) && !strcmp(a, b) && bar(a) && !strcmp(a, b)) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5334
        check("void f(C *src) {\n"
              "    if (x<A*>(src) || x<B*>(src))\n"
              "        a++;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(A *src) {\n"
              "    if (dynamic_cast<B*>(src) || dynamic_cast<B*>(src)) {}\n"
              "}\n", "test.cpp", false, false, false); // don't run simplifications
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        // #5819
        check("Vector func(Vector vec1) {\n"
              "    return fabs(vec1 & vec1 & vec1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("Vector func(int vec1) {\n"
              "    return fabs(vec1 & vec1 & vec1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression on both sides of '&'.\n", errout.str());

    }

    void duplicateExpression4() {
        check("void foo() {\n"
              "    if (*a++ != b || *a++ != b) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if (*a-- != b || *a-- != b) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // assignment
        check("void f() {\n"
              "  while (*(a+=2)==*(b+=2) && *(a+=2)==*(b+=2)) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpression5() {  // #3749 - macros with same values
        check("void f() {\n"
              "    if ($a == $a) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpression6() {  // #4639
        check("float IsNan(float value) { return !(value == value); }\n"
              "double IsNan(double value) { return !(value == value); }\n"
              "long double IsNan(long double value) { return !(value == value); }");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpression7() {
        check("void f() {\n"
              "    const int i = sizeof(int);\n"
              "    if ( i != sizeof (int)){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'i != sizeof(int)' is always false because 'i' and 'sizeof(int)' represent the same value.\n", errout.str());

        check("void f() {\n"
              "    const int i = sizeof(int);\n"
              "    if ( sizeof (int) != i){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'sizeof(int) != i' is always false because 'sizeof(int)' and 'i' represent the same value.\n", errout.str());

        check("void f(int a = 1) { if ( a != 1){}}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'a != 1' is always false.\n", errout.str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    int b = 1;\n"
              "    if ( a != b){} \n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3] -> [test.cpp:4]: (style) The expression 'a != b' is always false because 'a' and 'b' represent the same value.\n", errout.str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    int b = a;\n"
              "    if ( a != b){} \n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) The expression 'a != b' is always false because 'a' and 'b' represent the same value.\n", errout.str());

        check("void use(int);\n"
              "void f() {\n"
              "    int a = 1;\n"
              "    int b = 1;\n"
              "    use(b);\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (style) The expression 'a != 1' is always false.\n", errout.str());

        check("void use(int);\n"
              "void f() {\n"
              "    int a = 1;\n"
              "    use(a);\n"
              "    a = 2;\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void use(int);\n"
              "void f() {\n"
              "    int a = 2;\n"
              "    use(a);\n"
              "    a = 1;\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("const int a = 1;\n"
              "    void f() {\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:3]: (style) The expression 'a != 1' is always false.\n", errout.str());

        check("int a = 1;\n"
              "    void f() {\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    static const int a = 1;\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'a != 1' is always false.\n", errout.str());

        check("void f() {\n"
              "    static int a = 1;\n"
              "    if ( a != 1){} \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    if ( a != 1){\n"
              "        a++;\n"
              "    }}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'a != 1' is always false.\n", errout.str());

        check("void f(int b) {\n"
              "    int a = 1;\n"
              "    while (b) {\n"
              "        if ( a != 1){}\n"
              "        a++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("bool f(bool a, bool b) {\n"
              "    const bool c = a;\n"
              "    return a && b && c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Same expression on both sides of '&&' because 'a' and 'c' represent the same value.\n", errout.str());
    }

    void duplicateExpression8() {
        check("void f() {\n"
              "    int a = 1;\n"
              "    int b = a;\n"
              "    a = 2;\n"
              "    if ( b != a){}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int * a, int i) { int b = a[i]; a[i] = 2; if ( b != a[i]){}}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int * a, int i) { int b = *a; *a = 2; if ( b != *a){}}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A { int f() const; };\n"
              "A g();\n"
              "void foo() {\n"
              "    for (const A x = A();;) {\n"
              "        const int a = x.f();\n"
              "        x = g();\n"
              "        if (x.f() == a) break;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(int i);\n"
              "struct A {\n"
              "    enum E { B, C };\n"
              "    bool f(E);\n"
              "};\n"
              "void foo() {\n"
              "    A a;\n"
              "    const bool x = a.f(A::B);\n"
              "    const bool y = a.f(A::C);\n"
              "    if(!x && !y) return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo() { \n"
              "    const bool x = a.f(A::B);\n"
              "    const bool y = a.f(A::C);\n"
              "    if (!x && !y) return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool * const b);\n"
              "void foo() { \n"
              "    bool x = true;\n"
              "    bool y = true;\n"
              "    f(&x);\n"
              "    if (!x && !y) return;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const int a = {};\n"
              "    if(a == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("volatile const int var = 42;\n"
              "void f() { if(var == 42) {} }\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    struct b c;\n"
              "    c.a = &a;\n"
              "    g(&c);\n"
              "    if (a == 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpressionLoop() {
        check("void f() {\n"
              "    int a = 1;\n"
              "    while ( a != 1){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'a != 1' is always false.\n", errout.str());

        check("void f() { int a = 1; while ( a != 1){ a++; }}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() { int a = 1; for ( int i=0; i < 3 && a != 1; i++){ a++; }}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int b) { int a = 1; while (b) { if ( a != 1){} b++; } a++; } \n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'i != 0' is always false.\n", errout.str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) {}\n"
              "        i++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) { i++; }\n"
              "        i++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) { i++; }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int i = 0;\n"
              "    while(i < 10) {\n"
              "        if( i != 0 ) {}\n"
              "        i++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int b) {\n"
              "    while (b) {\n"
              "        int a = 1;\n"
              "        if ( a != 1){}\n"
              "        b++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) The expression 'a != 1' is always false.\n", errout.str());
    }

    void duplicateExpressionTernary() { // #6391
        check("void f() {\n"
              "    return A ? x : x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression in both branches of ternary operator.\n", errout.str());

        check("int f(bool b, int a) {\n"
              "    const int c = a;\n"
              "    return b ? a : c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Same expression in both branches of ternary operator.\n", errout.str());

        check("void f() {\n"
              "    return A ? x : z;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(unsigned char c) {\n"
              "  x = y ? (signed char)c : (unsigned char)c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::string stringMerge(std::string const& x, std::string const& y) {\n" // #7938
              "    return ((x > y) ? (y + x) : (x + y));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6426
        check("void foo(bool flag) {\n"
              "  bar( (flag) ? ~0u : ~0ul);\n"
              "}");
        ASSERT_EQUALS((_settings.sizeof_int==_settings.sizeof_long)?"[test.cpp:2]: (style) Same value in both branches of ternary operator.\n":"",  errout.str());
    }

    void duplicateValueTernary() {
        check("void f() {\n"
              "    if( a ? (b ? false:false): false ) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f1(int a) {return (a == 1) ? (int)1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f2(int a) {return (a == 1) ? (int)1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f3(int a) {return (a == 1) ? 1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f4(int a) {return (a == 1) ? 1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f5(int a) {return (a == (int)1) ? (int)1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f6(int a) {return (a == (int)1) ? (int)1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f7(int a) {return (a == (int)1) ? 1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());

        check("int f8(int a) {return (a == (int)1) ? 1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Same value in both branches of ternary operator.\n", errout.str());
    }

    void duplicateExpressionTemplate() { // #6930
        check("template <int I> void f() {\n"
              "    if (I >= 0 && I < 3) {}\n"
              "}\n"
              "\n"
              "static auto a = f<0>();");
        ASSERT_EQUALS("", errout.str());
    }

    void oppositeExpression() {
        check("void f(bool a) { if(a && !a) {} }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '&&'.\n", errout.str());

        check("void f(bool a) { if(a != !a) {} }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '!='.\n", errout.str());

        check("void f(bool a) { if( a == !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '=='.\n", errout.str());

        check("void f(bool a) { if( a != !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '!='.\n", errout.str());

        check("void f(bool a) { if( !(a) == a ) {}}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '=='.\n", errout.str());

        check("void f(bool a) { if( !(a) != a ) {}}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '!='.\n", errout.str());

        check("void f(bool a) { if( !(!a) == !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '=='.\n", errout.str());

        check("void f(bool a) { if( !(!a) != !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '!='.\n", errout.str());

        check("void f1(bool a) {\n"
              "    const bool b = a;\n"
              "    if( a == !(b) ) {}\n"
              "    if( b == !(a) ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Opposite expression on both sides of '=='.\n"
                      "[test.cpp:2] -> [test.cpp:4]: (style) Opposite expression on both sides of '=='.\n", errout.str());

        check("void f2(bool *a) {\n"
              "    const bool b = *a;\n"
              "    if( *a == !(b) ) {}\n"
              "    if( b == !(*a) ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Opposite expression on both sides of '=='.\n"
                      "[test.cpp:2] -> [test.cpp:4]: (style) Opposite expression on both sides of '=='.\n", errout.str());

        check("void f(bool a) { a = !a; }");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a) { if( a < -a ) {}}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Opposite expression on both sides of '<'.\n", errout.str());

        check("void f(int a) { a -= -a; }");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a) { a = a / (-a); }");
        ASSERT_EQUALS("", errout.str());

        check("bool f(int i){ return !((i - 1) & i); }");
        ASSERT_EQUALS("", errout.str());

        check("bool f(unsigned i){ return (x > 0) && (x & (x-1)) == 0; }");
        ASSERT_EQUALS("", errout.str());

        check("void A::f(bool a, bool c)\n"
              "{\n"
              "    const bool b = a;\n"
              "    if(c) { a = false; } \n"
              "    if(b && !a) { }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool c) {\n"
              "    const bool b = a;\n"
              "    if(c) { a = false; } \n"
              "    if(b && !a) { }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    bool x = a;\n"
              "    dostuff();\n"
              "    if (x && a) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  const bool b = g();\n"
              "  if (!b && g()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateVarExpression() {
        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f();\n"
              "    int j = f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("struct Foo { int f() const; int g() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("struct Foo { int f() const; int g() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    Foo f2 = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:5]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = 1 + f();\n"
              "    int j = 1 + f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f() + 1;\n"
              "    int j = 1 + f();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int x = f();\n"
              "    int i = x + 1;\n"
              "    int j = f() + 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f() + f();\n"
              "    int j = f() + f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("int f(int) __attribute__((pure));\n"
              "int g(int) __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f(0);\n"
              "    int j = f(0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("int f(int) __attribute__((pure));\n"
              "int g(int) __attribute__((pure));\n"
              "void test() {\n"
              "    const int x = 0;\n"
              "    int i = f(0);\n"
              "    int j = f(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test(int * p, int * q) {\n"
              "    int i = *p;\n"
              "    int j = *p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("struct A { int x; int y; };"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (style) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("void test() {\n"
              "    int i = 0;\n"
              "    int j = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test() {\n"
              "    int i = -1;\n"
              "    int j = -1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f(int);\n"
              "void test() {\n"
              "    int i = f(0);\n"
              "    int j = f(1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f();\n"
              "int g();\n"
              "void test() {\n"
              "    int i = f() || f();\n"
              "    int j = f() && f();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct Foo {};\n"
              "void test() {\n"
              "    Foo i = Foo();\n"
              "    Foo j = Foo();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct Foo {};\n"
              "void test() {\n"
              "    Foo i = Foo{};\n"
              "    Foo j = Foo{};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct Foo { int f() const; float g() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("struct Foo { int f(); int g(); };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test() {\n"
              "    int i = f();\n"
              "    int j = f();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test(int x) {\n"
              "    int i = ++x;\n"
              "    int j = ++x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test(int x) {\n"
              "    int i = x++;\n"
              "    int j = x++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test(int x) {\n"
              "    int i = --x;\n"
              "    int j = --x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test(int x) {\n"
              "    int i = x--;\n"
              "    int j = x--;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test(int x) {\n"
              "    int i = x + 1;\n"
              "    int j = 1 + x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateVarExpressionUnique() {
        check("struct SW { int first; };\n"
              "void foo(SW* x) {\n"
              "    int start = x->first;\n"
              "    int end   = x->first;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3]: (style, inconclusive) Same expression used in consecutive assignments of 'start' and 'end'.\n", errout.str());
        check("struct SW { int first; };\n"
              "void foo(SW* x, int i, int j) {\n"
              "    int start = x->first;\n"
              "    int end   = x->first;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3]: (style, inconclusive) Same expression used in consecutive assignments of 'start' and 'end'.\n", errout.str());
        check("struct Foo { int f() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("void test(int * p) {\n"
              "    int i = *p;\n"
              "    int j = *p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("struct Foo { int f() const; int g(int) const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("struct Foo { int f() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());
    }

    void duplicateVarExpressionAssign() {
        check("struct A { int x; int y; };"
              "void use(int);\n"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "    use(i);\n"
              "    i = j;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        check("struct A { int x; int y; };"
              "void use(int);\n"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "    use(j);\n"
              "    j = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'.\n", errout.str());

        // Issue #8612
        check("struct P\n"
              "{\n"
              "    void func();\n"
              "    bool operator==(const P&) const;\n"
              "};\n"
              "struct X\n"
              "{\n"
              "    P first;\n"
              "    P second;\n"
              "};\n"
              "bool bar();\n"
              "void baz(const P&);\n"
              "void foo(const X& x)\n"
              "{\n"
              "    P current = x.first;\n"
              "    P previous = x.first;\n"
              "    while (true)\n"
              "    {\n"
              "        baz(current);\n"
              "        if (bar() && previous == current)\n"
              "        {\n"
              "            current.func();\n"
              "        }\n"
              "        previous = current;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:16] -> [test.cpp:15]: (style, inconclusive) Same expression used in consecutive assignments of 'current' and 'previous'.\n", errout.str());
    }

    void duplicateVarExpressionCrash() {
        // Issue #8624
        check("struct  X {\n"
              "    X();\n"
              "    int f() const;\n"
              "};\n"
              "void run() {\n"
              "        X x;\n"
              "        int a = x.f();\n"
              "        int b = x.f();\n"
              "        (void)a;\n"
              "        (void)b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:7]: (style, inconclusive) Same expression used in consecutive assignments of 'a' and 'b'.\n", errout.str());

        // Issue #8712
        check("void f() {\n"
              "  unsigned char d;\n"
              "  d = d % 5;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("template <typename T>\n"
              "T f() {\n"
              "  T x = T();\n"
              "}\n"
              "int &a = f<int&>();\n");
        ASSERT_EQUALS("", errout.str());

        // Issue #8713
        check("class A {\n"
              "  int64_t B = 32768;\n"
              "  P<uint8_t> m = MakeP<uint8_t>(B);\n"
              "};\n"
              "void f() {\n"
              "  uint32_t a = 42;\n"
              "  uint32_t b = uint32_t(A ::B / 1024);\n"
              "  int32_t c = int32_t(a / b);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Issue #8709
        check("a b;\n"
              "void c() {\n"
              "  switch (d) { case b:; }\n"
              "  double e(b);\n"
              "  if(e <= 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

    }

    void multiConditionSameExpression() {
        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) continue;\n"
              "  if ((val > 0)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'val < 0' is always false.\n"
                      "[test.cpp:2] -> [test.cpp:4]: (style) The expression 'val > 0' is always false.\n", errout.str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if ((val > 0)) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'val < 0' is always false.\n"
                      "[test.cpp:2] -> [test.cpp:4]: (style) The expression 'val > 0' is always false.\n", errout.str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if ((val < 0)) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The expression 'val < 0' is always false.\n"
                      "[test.cpp:2] -> [test.cpp:4]: (style) The expression 'val < 0' is always false.\n", errout.str());

        check("void f() {\n"
              "  int activate = 0;\n"
              "  int foo = 0;\n"
              "  if (activate) {}\n"
              "  else if (foo) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void checkSignOfUnsignedVariable() {
        check(
            "void foo() {\n"
            "  for(unsigned char i = 10; i >= 0; i--)"
            "    printf(\"%u\", i);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'i' can't be negative so it is unnecessary to test it.\n", errout.str());

        check(
            "void foo(bool b) {\n"
            "  for(unsigned int i = 10; b || i >= 0; i--)"
            "    printf(\"%u\", i);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'i' can't be negative so it is unnecessary to test it.\n", errout.str());

        check(
            "bool foo(unsigned int x) {\n"
            "  if (x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x) {\n"
            "  if (x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x) {\n"
            "  if (0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x) {\n"
            "  if (0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x) {\n"
            "  if (x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check(
            "bool foo(int x) {\n"
            "  if (x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());


        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x < 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (x < 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (0 > x && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (0 > x && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x >= 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (x >= 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());


        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (y && x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (y && x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (y && 0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (y && 0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (y && x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (y && x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());


        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x < 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (x < 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (0 > x || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (0 > x || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x >= 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check(
            "bool foo(int x, bool y) {\n"
            "  if (x >= 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        // #3233 - FP when template is used (template parameter is numeric constant)
        {
            const char code[] =
                "template<int n> void foo(unsigned int x) {\n"
                "  if (x <= n);\n"
                "}\n"
                "foo<0>();";
            check(code, nullptr, false, false);
            ASSERT_EQUALS("", errout.str());
            check(code, nullptr, false, true);
            ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Checking if unsigned variable 'x' is less than zero. This might be a false warning.\n", errout.str());
        }
    }

    void checkSignOfPointer() {
        check(
            "bool foo(int* x) {\n"
            "  if (x >= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check(
            "bool foo(int* x) {\n"
            "  if (*x >= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(int* x) {\n"
            "  if (x < 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check(
            "bool foo(int* x) {\n"
            "  if (*x < 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y < 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y <= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y > 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y >= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(Bar* x) {\n"
            "  if (0 <= x)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first) {\n"
            "  if (first.ptr >= 0) {} \n"
            "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if((first.ptr - second.ptr) >= 0) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first) {\n"
            "  if((first.ptr) >= 0) {} \n"
            "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 <= first.ptr - second.ptr) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 <= (first.ptr - second.ptr)) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(first.ptr - second.ptr < 0) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if((first.ptr - second.ptr) < 0) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 > first.ptr - second.ptr) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 > (first.ptr - second.ptr)) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(int* x) {\n"
            "  if (0 <= x[0])"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(Bar* x) {\n"
            "  if (0 <= x.y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(Bar* x) {\n"
            "  if (0 <= x->y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(Bar* x, Bar* y) {\n"
            "  if (0 <= x->y - y->y )"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(Bar* x) {\n"
            "  if (0 > x)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check(
            "bool foo(int* x) {\n"
            "  if (0 > x[0])"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(Bar* x) {\n"
            "  if (0 > x.y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "bool foo(Bar* x) {\n"
            "  if (0 > x->y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  int (*t)(void *a, void *b);\n"
            "  if (t(a, b) < 0)\n"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  int (*t)(void *a, void *b);\n"
            "  if (0 > t(a, b))\n"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "struct object_info { int *typep; };\n"
            "void packed_object_info(struct object_info *oi) {\n"
            "  if (oi->typep < 0);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check(
            "struct object_info { int typep[10]; };\n"
            "void packed_object_info(struct object_info *oi) {\n"
            "  if (oi->typep < 0);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check(
            "struct object_info { int *typep; };\n"
            "void packed_object_info(struct object_info *oi) {\n"
            "  if (*oi->typep < 0);\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkForSuspiciousSemicolon1() {
        check(
            "void foo() {\n"
            "  for(int i = 0; i < 10; ++i);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        // Empty block
        check(
            "void foo() {\n"
            "  for(int i = 0; i < 10; ++i); {\n"
            "  }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Suspicious use of ; at the end of 'for' statement.\n", errout.str());

        check(
            "void foo() {\n"
            "  while (!quit); {\n"
            "    do_something();\n"
            "  }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Suspicious use of ; at the end of 'while' statement.\n", errout.str());
    }

    void checkForSuspiciousSemicolon2() {
        check(
            "void foo() {\n"
            "  if (i == 1); {\n"
            "    do_something();\n"
            "  }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Suspicious use of ; at the end of 'if' statement.\n", errout.str());

        // Seen this in the wild
        check(
            "void foo() {\n"
            "  if (Match());\n"
            "  do_something();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  if (Match());\n"
            "  else\n"
            "    do_something();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  if (i == 1)\n"
            "       ;\n"
            "  {\n"
            "    do_something();\n"
            "  }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  if (i == 1);\n"
            "\n"
            "  {\n"
            "    do_something();\n"
            "  }\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }


    void checkInvalidFree() {
        check(
            "void foo(char *p) {\n"
            "  char *a; a = malloc(1024);\n"
            "  free(a + 10);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = malloc(1024);\n"
            "  free(a - 10);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = malloc(1024);\n"
            "  free(10 + a);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = new char[1024];\n"
            "  delete[] (a + 10);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = new char;\n"
            "  delete a + 10;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = new char;\n"
            "  bar(a);\n"
            "  delete a + 10;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = new char;\n"
            "  char *b; b = new char;\n"
            "  bar(a);\n"
            "  delete a + 10;\n"
            "  delete b + 10;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = new char;\n"
            "  char *b; b = new char;\n"
            "  bar(a, b);\n"
            "  delete a + 10;\n"
            "  delete b + 10;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  char *a; a = new char;\n"
            "  bar()\n"
            "  delete a + 10;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(size_t xx) {\n"
            "  char *ptr; ptr = malloc(42);\n"
            "  ptr += xx;\n"
            "  free(ptr - xx - 1);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:4]: (error, inconclusive) Invalid memory address freed.\n", errout.str());

        check(
            "void foo(size_t xx) {\n"
            "  char *ptr; ptr = malloc(42);\n"
            "  std::cout << ptr;\n"
            "  ptr = otherPtr;\n"
            "  free(otherPtr - xx - 1);\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkRedundantCopy() {
        check("const std::string& getA(){static std::string a;return a;}\n"
              "void foo() {\n"
              "    const std::string a = getA();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying.\n", errout.str());

        check("class A{public:A(){}};\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a = getA();\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying.\n", errout.str());

        check("const int& getA(){static int a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const int a = getA();\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("const int& getA(){static int a;return a;}\n"
              "int main()\n"
              "{\n"
              "    int getA = 0;\n"
              "    const int a = getA + 3;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:4]: (style) Local variable getA shadows outer function\n", errout.str());

        check("class A{public:A(){}};\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a(getA());\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying.\n", errout.str());

        check("const int& getA(){static int a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const int a(getA());\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class A{\n"
              "public:A(int a=0){_a = a;}\n"
              "A operator+(const A & a){return A(_a+a._a);}\n"
              "private:int _a;};\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a = getA() + 1;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class A{\n"
              "public:A(int a=0){_a = a;}\n"
              "A operator+(const A & a){return A(_a+a._a);}\n"
              "private:int _a;};\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a(getA()+1);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5190 - FP when creating object with constructor that takes a reference
        check("class A {};\n"
              "class B { B(const A &a); };\n"
              "const A &getA();\n"
              "void f() {\n"
              "    const B b(getA());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5618
        const char* code5618 = "class Token {\n"
                               "public:\n"
                               "    const std::string& str();\n"
                               "};\n"
                               "void simplifyArrayAccessSyntax() {\n"
                               "    for (Token *tok = list.front(); tok; tok = tok->next()) {\n"
                               "        const std::string temp = tok->str();\n"
                               "        tok->str(tok->strAt(2));\n"
                               "    }\n"
                               "}";
        check(code5618, nullptr, false, true);
        TODO_ASSERT_EQUALS("", "[test.cpp:7]: (performance, inconclusive) Use const reference for 'temp' to avoid unnecessary data copying.\n", errout.str());
        check(code5618, nullptr, false, false);
        ASSERT_EQUALS("", errout.str());

        // #5890 - crash: wesnoth desktop_util.cpp / unicode.hpp
        check("typedef std::vector<char> X;\n"
              "X f<X>(const X &in) {\n"
              "    const X s = f<X>(in);\n"
              "    return f<X>(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkNegativeShift() {
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   a << -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting by a negative value is undefined behaviour\n", errout.str());
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   a >> -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting by a negative value is undefined behaviour\n", errout.str());
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   a <<= -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting by a negative value is undefined behaviour\n", errout.str());
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   a >>= -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting by a negative value is undefined behaviour\n", errout.str());
        check("void foo()\n"
              "{\n"
              "   std::cout << -1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "   std::cout << a << -1 ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "   std::cout << 3 << -1 ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo() {\n"
              "   x = (-10+2) << 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Shifting a negative value is technically undefined behaviour\n", errout.str());

        check("x = y ? z << $-1 : 0;\n");
        ASSERT_EQUALS("", errout.str());

        // Negative LHS
        check("const int x = -1 >> 2;");
        ASSERT_EQUALS("[test.cpp:1]: (portability) Shifting a negative value is technically undefined behaviour\n", errout.str());

        // #6383 - unsigned type
        check("const int x = (unsigned int)(-1) >> 2;");
        ASSERT_EQUALS("", errout.str());

        // #7814 - UB happening in valueflowcode when it tried to compute shifts.
        check("int shift1() { return 1 >> -1 ;}\n"
              "int shift2() { return 1 << -1 ;}\n"
              "int shift3() { return -1 >> 1 ;}\n"
              "int shift4() { return -1 << 1 ;}\n");
        ASSERT_EQUALS("[test.cpp:1]: (error) Shifting by a negative value is undefined behaviour\n"
                      "[test.cpp:2]: (error) Shifting by a negative value is undefined behaviour\n"
                      "[test.cpp:3]: (portability) Shifting a negative value is technically undefined behaviour\n"
                      "[test.cpp:4]: (portability) Shifting a negative value is technically undefined behaviour\n", errout.str());
    }

    void incompleteArrayFill() {
        check("void f() {\n"
              "    int a[5];\n"
              "    memset(a, 123, 5);\n"
              "    memcpy(a, b, 5);\n"
              "    memmove(a, b, 5);\n"
              "}");
        ASSERT_EQUALS(// TODO "[test.cpp:4] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n"
            "[test.cpp:3]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'?\n"
            "[test.cpp:4]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memcpy()' with 'sizeof(*a)'?\n"
            "[test.cpp:5]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memmove()' with 'sizeof(*a)'?\n", errout.str());

        check("void f() {\n"
              "    Foo* a[5];\n"
              "    memset(a, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'?\n", errout.str());

        check("class Foo {int a; int b;};\n"
              "void f() {\n"
              "    Foo a[5];\n"
              "    memset(a, 'a', 5);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'?\n", "", errout.str());

        check("void f() {\n"
              "    Foo a[5];\n" // Size of foo is unknown
              "    memset(a, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char a[5];\n"
              "    memset(a, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a[5];\n"
              "    memset(a+15, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    bool a[5];\n"
              "    memset(a, false, 5);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability, inconclusive) Array 'a' might be filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'?\n", errout.str());
    }

    void redundantVarAssignment() {
        // Simple tests
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());


        // non-local variable => only show warning when inconclusive is used
        check("int i;\n"
              "void f() {\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        check("void f() {\n"
              "    int i;\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        check("void f() {\n"
              "    static int i;\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("void f() {\n"
              "    int i[10];\n"
              "    i[2] = 1;\n"
              "    i[2] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Variable 'i[2]' is reassigned a value before the old one has been used.\n", errout.str());

        check("void f(int x) {\n"
              "    int i[10];\n"
              "    i[x] = 1;\n"
              "    x=1;\n"
              "    i[x] = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const int x) {\n"
              "    int i[10];\n"
              "    i[x] = 1;\n"
              "    i[x] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Variable 'i[x]' is reassigned a value before the old one has been used.\n", errout.str());

        // Testing different types
        check("void f() {\n"
              "    Foo& bar = foo();\n"
              "    bar = x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    Foo& bar = foo();\n"
              "    bar = x;\n"
              "    bar = y;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("void f() {\n"
              "    Foo& bar = foo();\n" // #4425. bar might refer to something global, etc.
              "    bar = y();\n"
              "    foo();\n"
              "    bar = y();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Tests with function call between assignment
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    bar();\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (style) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        check("int i;\n"
              "void f() {\n"
              "    i = 1;\n"
              "    bar();\n" // Global variable might be accessed in bar()
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    static int i;\n"
              "    i = 1;\n"
              "    bar();\n" // bar() might call f() recursively. This could be a false positive in more complex examples (when value of i is used somewhere. See #4229)
              "    i = 2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int i;\n"
              "    i = 1;\n"
              "    bar();\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (style) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        check("void bar(int i) {}\n"
              "void f(int i) {\n"
              "    i = 1;\n"
              "    bar(i);\n" // Passed as argument
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    Foo bar = foo();\n"
              "    bar();\n" // #5568. operator() called
              "    bar = y();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Branch tests
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    if(x)\n"
              "        i = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int i) {\n"
              "    if(x)\n"
              "        i = 0;\n"
              "    i = 1;\n"
              "    i = 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:5]: (style) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        // #4513
        check("int x;\n"
              "int g() {\n"
              "    return x*x;\n"
              "}\n"
              "void f() {\n"
              "    x = 2;\n"
              "    x = g();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int g() {\n"
              "    return x*x;\n"
              "}\n"
              "void f(int x) {\n"
              "    x = 2;\n"
              "    x = g();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:6]: (style) Variable 'x' is reassigned a value before the old one has been used.\n", errout.str());

        check("void f() {\n"
              "    Foo& bar = foo();\n"
              "    bar = x;\n"
              "    bar = y();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class C {\n"
              "    int x;\n"
              "    void g() { return x * x; }\n"
              "    void f();\n"
              "};\n"
              "\n"
              "void C::f() {\n"
              "    x = 2;\n"
              "    x = g();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class C {\n"
              "    int x;\n"
              "    void g() { return x*x; }\n"
              "    void f(Foo z);\n"
              "};\n"
              "\n"
              "void C::f(Foo z) {\n"
              "    x = 2;\n"
              "    x = z.g();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // from #3103 (avoid a false negative)
        check("int foo(){\n"
              "    int x;\n"
              "    x = 1;\n"
              "    x = 1;\n"
              "    return x + 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Variable 'x' is reassigned a value before the old one has been used.\n", errout.str());

        // from #3103 (avoid a false positive)
        check("int foo(){\n"
              "    int x;\n"
              "    x = 1;\n"
              "    if (y)\n" // <-- cppcheck does not know anything about 'y'
              "        x = 2;\n"
              "    return x + 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // initialization, assignment with 0
        check("void f() {\n"  // Ticket #4356
              "    int x = 0;\n"  // <- ignore initialization with 0
              "    x = 3;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  state_t *x = NULL;\n"
              "  x = dostuff();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  state_t *x;\n"
              "  x = NULL;\n"
              "  x = dostuff();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo() {\n" // #4420
              "    int x;\n"
              "    bar(++x);\n"
              "    x = 5;\n"
              "    return bar(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // struct member..
        check("struct AB { int a; int b; };\n"
              "\n"
              "int f() {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    ab.a = 2;\n"
              "    return ab.a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:6]: (style) Variable 'ab.a' is reassigned a value before the old one has been used.\n", errout.str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f() {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    ab = do_something();\n"
              "    return ab.a;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f() {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    do_something(&ab);\n"
              "    ab.a = 2;\n"
              "    return ab.a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f(DO_SOMETHING do_something) {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    do_something(&ab);\n"
              "    ab.a = 2;\n"
              "    return ab.a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f(struct AB *ab) {\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "    ab++;\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f(struct AB *ab) {\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "    ab = x;\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(struct AB *ab) {\n" // #
              "    ab->data->x = 1;\n"
              "    ab = &ab1;\n"
              "    ab->data->x = 2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5964
        check("void func(char *buffer, const char *format, int precision, unsigned value) {\n"
              "    (precision < 0) ? sprintf(buffer, format, value) : sprintf(buffer, format, precision, value);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // don't crash
        check("struct data {\n"
              "  struct { int i; } fc;\n"
              "};\n"
              "struct state {\n"
              "  struct data d[123];\n"
              "};\n"
              "void func(struct state *s) {\n"
              "  s->foo[s->x++] = 2;\n"
              "  s->d[1].fc.i++;\n"
              "}");

        // #6525 - inline assembly
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    asm(\"foo\");\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6555
        check("void foo() {\n"
              "    char *p = 0;\n"
              "    try {\n"
              "        p = fred();\n"
              "        p = wilma();\n"
              "    }\n"
              "    catch (...) {\n"
              "        barney(p);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    char *p = 0;\n"
              "    try {\n"
              "        p = fred();\n"
              "        p = wilma();\n"
              "    }\n"
              "    catch (...) {\n"
              "        barney(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'p' can be reduced.\n",
                      errout.str());

        check("void foo() {\n"
              "    char *p = 0;\n"
              "    try {\n"
              "        if(z) {\n"
              "            p = fred();\n"
              "            p = wilma();\n"
              "        }\n"
              "    }\n"
              "    catch (...) {\n"
              "        barney(p);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Member variable pointers
        check("void podMemPtrs() {\n"
              "    int POD::*memptr;\n"
              "    memptr = &POD::a;\n"
              "    memptr = &POD::b;\n"
              "    if (memptr)\n"
              "        memptr = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style) Variable 'memptr' is reassigned a value before the old one has been used.\n", errout.str());
    }

    void redundantVarAssignment_7133() {
        // #7133
        check("sal_Int32 impl_Export() {\n"
              "   try {\n"
              "        try  {\n"
              "          uno::Sequence< uno::Any > aArgs(2);\n"
              "          beans::NamedValue aValue;\n"
              "          aValue.Name = \"DocumentHandler\";\n"
              "          aValue.Value <<= xDocHandler;\n"
              "          aArgs[0] <<= aValue;\n"
              "          aValue.Name = \"Model\";\n"
              "          aValue.Value <<= xDocumentComp;\n"
              "          aArgs[1] <<= aValue;\n"
              "        }\n"
              "        catch (const uno::Exception&) {\n"
              "        }\n"
              "   }\n"
              "   catch (const uno::Exception&)  {\n"
              "   }\n"
              "}", "test.cpp", false, true);
        ASSERT_EQUALS("", errout.str());

        check("void ConvertBitmapData(sal_uInt16 nDestBits) {\n"
              "    BitmapBuffer aSrcBuf;\n"
              "    aSrcBuf.mnBitCount = nSrcBits;\n"
              "    BitmapBuffer aDstBuf;\n"
              "    aSrcBuf.mnBitCount = nDestBits;\n"
              "    bConverted = ::ImplFastBitmapConversion( aDstBuf, aSrcBuf, aTwoRects );\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:3] -> [test.c:5]: (style) Variable 'aSrcBuf.mnBitCount' is reassigned a value before the old one has been used.\n", errout.str());
        check("void ConvertBitmapData(sal_uInt16 nDestBits) {\n"
              "    BitmapBuffer aSrcBuf;\n"
              "    aSrcBuf.mnBitCount = nSrcBits;\n"
              "    BitmapBuffer aDstBuf;\n"
              "    aSrcBuf.mnBitCount = nDestBits;\n"
              "    bConverted = ::ImplFastBitmapConversion( aDstBuf, aSrcBuf, aTwoRects );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (style) Variable 'aSrcBuf.mnBitCount' is reassigned a value before the old one has been used.\n",
                      errout.str());

        check("class C { void operator=(int x); };\n" // #8368 - assignment operator might have side effects => inconclusive
              "void f() {\n"
              "    C c;\n"
              "    c = x;\n"
              "    c = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:5]: (style, inconclusive) Variable 'c' is reassigned a value before the old one has been used if variable is no semaphore variable.\n", errout.str());
    }

    void redundantVarAssignment_stackoverflow() {
        check("typedef struct message_node {\n"
              "  char code;\n"
              "  size_t size;\n"
              "  struct message_node *next, *prev;\n"
              "} *message_list;\n"
              "static message_list remove_message_from_list(message_list m) {\n"
              "    m->prev->next = m->next;\n"
              "    m->next->prev = m->prev;\n"
              "    return m->next;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void redundantVarAssignment_lambda() {
        // #7152
        check("int foo() {\n"
              "    int x = 0, y = 0;\n"
              "    auto f = [&]() { if (x < 5) ++y; };\n"
              "    x = 2;\n"
              "    f();\n"
              "    x = 6;\n"
              "    f();\n"
              "    return y;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void redundantVarAssignment_for() {
        check("void f() {\n"
              "    char buf[10];\n"
              "    int i;\n"
              "    for (i = 0; i < 4; i++)\n"
              "        buf[i] = 131;\n"
              "    buf[i] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void redundantMemWrite() {
        return; // FIXME: temporary hack

        // Simple tests
        check("void f() {\n"
              "    char a[10];\n"
              "    memcpy(a, foo, bar);\n"
              "    memset(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        check("void f() {\n"
              "    char a[10];\n"
              "    strcpy(a, foo);\n"
              "    strncpy(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        check("void f() {\n"
              "    char a[10];\n"
              "    sprintf(a, \"foo\");\n"
              "    memmove(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        check("void f(char *filename) {\n"
              "    char *p = strrchr(filename,'.');\n"
              "    strcpy(p, \"foo\");\n"
              "    dostuff(filename);\n"
              "    strcpy(p, \"foo\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Writing to different parts of a buffer
        check("void f(void* a) {\n"
              "    memcpy(a, foo, bar);\n"
              "    memset(a+5, 0, bar);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Use variable as second argument
        check("void f(void* a, void* b) {\n"
              "    memset(a, 0, 5);\n"
              "    memcpy(b, a, 5);\n"
              "    memset(a, 1, 5);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // strcat is special
        check("void f() {\n"
              "    char a[10];\n"
              "    strcpy(a, foo);\n"
              "    strcat(a, bar);\n" // Not redundant
              "    strcpy(a, x);\n" // Redundant
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        // Tests with function call between copy
        check("void f() {\n"
              "    char a[10];\n"
              "    snprintf(a, foo, bar);\n"
              "    bar();\n"
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        check("void* a;\n"
              "void f() {\n"
              "    memset(a, 0, size);\n"
              "    bar();\n" // Global variable might be accessed in bar()
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char a[10];\n"
              "    memset(a, 0, size);\n"
              "    bar();\n"
              "    memset(a, 0, size);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n", "", errout.str());

        check("void bar(void* a) {}\n"
              "void f(void* a) {\n"
              "    memset(a, 0, size);\n"
              "    bar(a);\n" // Passed as argument
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Branch tests
        check("void f(void* a) {\n"
              "    memset(a, 0, size);\n"
              "    if(x)\n"
              "        memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4455 - initialization of local buffer
        check("void f(void) {"
              "    char buf[10];\n"
              "    memset(buf, 0, 10);\n"
              "    strcpy(buf, string);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(void) {\n"
              "    char buf[10] = {0};\n"
              "    memset(buf, 0, 10);\n"
              "    strcpy(buf, string);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'buf' is being written before its old content has been used.\n", errout.str());

        // #5689 - use return value of strcpy
        check("int f(void* a) {\n"
              "    int i = atoi(strcpy(a, foo));\n"
              "    strncpy(a, 0, bar);\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7175 - read+write
        check("void f() {\n"
              "    char buf[100];\n"
              "    strcpy(buf, x);\n"
              "    strcpy(buf, dostuff(buf));\n" // <- read + write
              "    strcpy(buf, x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char buf[100];\n"
              "    strcpy(buf, x);\n"
              "    strcpy(buf, dostuff(buf));\n"
              "    strcpy(buf, x);\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());
    }

    void varFuncNullUB() { // #4482
        check("void a(...);\n"
              "void b() { a(NULL); }");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Passing NULL after the last typed argument to a variadic function leads to undefined behaviour.\n", errout.str());

        check("void a(char *p, ...);\n"
              "void b() { a(NULL, 2); }");
        ASSERT_EQUALS("", errout.str());
    }

    void checkPipeParameterSize() { // #3521

        checkposix("void f(){\n"
                   "int pipefd[1];\n" // <--  array of two integers is needed
                   "if (pipe(pipefd) == -1) {\n"
                   "    return;\n"
                   "  }\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer 'pipefd' must have size of 2 integers if used as parameter of pipe().\n", errout.str());

        checkposix("void f(){\n"
                   "int pipefd[2];\n"
                   "if (pipe(pipefd) == -1) {\n"
                   "    return;\n"
                   "  }\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkposix("void f(){\n"
                   "int pipefd[20];\n"
                   "if (pipe(pipefd) == -1) {\n"
                   "    return;\n"
                   "  }\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkposix("void f(){\n"
                   "int pipefd[1];\n" // <--  array of two integers is needed
                   "if (pipe2(pipefd,0) == -1) {\n"
                   "    return;\n"
                   "  }\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer 'pipefd' must have size of 2 integers if used as parameter of pipe().\n", errout.str());

        checkposix("void f(){\n"
                   "int pipefd[2];\n"
                   "if (pipe2(pipefd,0) == -1) {\n"
                   "    return;\n"
                   "  }\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkposix("void f(){\n"
                   "int pipefd[20];\n"
                   "if (pipe2(pipefd,0) == -1) {\n"
                   "    return;\n"
                   "  }\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        // avoid crash with pointer variable
        check("void foo (int* arrayPtr)\n"
              "{\n"
              "  if (pipe (arrayPtr) < 0)\n"
              "  {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // avoid crash with pointer variable - for local variable on stack as well - see #4801
        check("void foo() {\n"
              "  int *cp;\n"
              "  if ( pipe (cp) == -1 ) {\n"
              "     return;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // test with unknown variable
        check("void foo() {\n"
              "  if ( pipe (cp) == -1 ) {\n"
              "     return;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // avoid crash with pointer variable - for local variable on stack as well - see #4801
        check("void foo() {\n"
              "  int *cp;\n"
              "  if ( pipe (cp) == -1 ) {\n"
              "     return;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // test with unknown variable
        check("void foo() {\n"
              "  if ( pipe (cp) == -1 ) {\n"
              "     return;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkCastIntToCharAndBack() { // #160

        // check getchar
        check("void f() {\n"
              "unsigned char c; c = getchar();\n"
              "  while( c != EOF)\n"
              "  {\n"
              "    bar(c);\n"
              "    c = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Storing getchar() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f() {\n"
              "unsigned char c = getchar();\n"
              "  while( EOF != c)\n"
              "  {\n"
              "    bar(c);\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Storing getchar() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f() {\n"
              "  unsigned char c; c = getchar();\n"
              "  while( EOF != c )\n"
              "  {\n"
              "    bar(c);\n"
              "    c = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Storing getchar() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f() {\n"
              "  int i; i = getchar();\n"
              "  while( i != EOF)\n"
              "  {\n"
              "    bar(i);\n"
              "    i = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int i; i = getchar();\n"
              "  while( EOF != i )\n"
              "  {\n"
              "    bar(i);\n"
              "    i = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        // check getc
        check("void f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = getc (pFile);\n"
              "} while (c != EOF)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing getc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = getc (pFile);\n"
              "} while (EOF != c)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing getc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = getc (pFile);\n"
              "} while (i != EOF)"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = getc (pFile);\n"
              "} while (EOF != i)"
              "}");
        ASSERT_EQUALS("", errout.str());


        // check fgetc
        check("void f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (c != EOF)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing fgetc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f (FILE * pFile){\n"
              "char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (EOF != c)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing fgetc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f (FILE * pFile){\n"
              "signed char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (EOF != c)"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = fgetc (pFile);\n"
              "} while (i != EOF)"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = fgetc (pFile);\n"
              "} while (EOF != i)"
              "}");
        ASSERT_EQUALS("", errout.str());

        // cin.get()
        check("void f(){\n"
              "   char ch; ch = std::cin.get();\n"
              "   while (EOF != ch) {\n"
              "        std::cout << ch;\n"
              "        ch = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Storing cin.get() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f(){\n"
              "   char ch; ch = std::cin.get();\n"
              "   while (ch != EOF) {\n"
              "        std::cout << ch;\n"
              "        ch = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Storing cin.get() return value in char variable and then comparing with EOF.\n", errout.str());

        check("void f(){\n"
              "   int i; i = std::cin.get();\n"
              "   while ( EOF != i ) {\n"
              "        std::cout << i;\n"
              "        i = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(){\n"
              "   int i; i = std::cin.get();\n"
              "   while ( i != EOF ) {\n"
              "        std::cout << i;\n"
              "        i = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkCommaSeparatedReturn() {
        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a++,\n"
              "  do_something();\n"
              "}", nullptr, true, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Comma is used in return statement. The comma can easily be misread as a ';'.\n", errout.str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a++, do_something();\n"
              "}", nullptr, true, false, false);
        ASSERT_EQUALS("", errout.str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a+5,\n"
              "  do_something();\n"
              "}", nullptr, true, false, false);
        ASSERT_EQUALS("[test.cpp:3]: (style) Comma is used in return statement. The comma can easily be misread as a ';'.\n", errout.str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a+5, do_something();\n"
              "}", nullptr, true, false, false);
        ASSERT_EQUALS("", errout.str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return c<int,\nint>::b;\n"
              "}", nullptr, true, false, false);
        ASSERT_EQUALS("", errout.str());

        // #4943 take care of C++11 initializer lists
        check("std::vector<Foo> Bar() {\n"
              "    return\n"
              "    {\n"
              "        { \"1\" },\n"
              "        { \"2\" },\n"
              "        { \"3\" }\n"
              "    };\n"
              "}", nullptr, true, false, false);
        ASSERT_EQUALS("", errout.str());
    }

    void checkPassByReference() {
        // #8570 passByValue when std::move is used
        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    explicit B(A a) : a(std::move(a)) {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "};", nullptr, false, false, true);
        ASSERT_EQUALS("", errout.str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    explicit B(A a) : a{std::move(a)} {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "};", nullptr, false, false, true);
        ASSERT_EQUALS("", errout.str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    B(A a, A a2) : a{std::move(a)}, a2{std::move(a2)} {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "    A a2;"
              "};", nullptr, false, false, true);
        ASSERT_EQUALS("", errout.str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    B(A a, A a2) : a{std::move(a)}, a2{a2} {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "    A a2;"
              "};", nullptr, false, false, true);
        ASSERT_EQUALS("[test.cpp:8]: (performance) Function parameter 'a2' should be passed by const reference.\n", errout.str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    B(A a, A a2) : a{std::move(a)}, a2(a2) {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "    A a2;"
              "};", nullptr, false, false, true);
        ASSERT_EQUALS("[test.cpp:8]: (performance) Function parameter 'a2' should be passed by const reference.\n", errout.str());
    }

    void checkComparisonFunctionIsAlwaysTrueOrFalse() {
        // positive test
        check("bool f(int x){\n"
              "   return isless(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with isless(x,x) always evaluates to false.\n", errout.str());

        check("bool f(int x){\n"
              "   return isgreater(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with isgreater(x,x) always evaluates to false.\n", errout.str());

        check("bool f(int x){\n"
              "   return islessgreater(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with islessgreater(x,x) always evaluates to false.\n", errout.str());

        check("bool f(int x){\n"
              "   return islessequal(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with islessequal(x,x) always evaluates to true.\n", errout.str());

        check("bool f(int x){\n"
              "   return isgreaterequal(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with isgreaterequal(x,x) always evaluates to true.\n", errout.str());

        // no warning should be reported for
        check("bool f(int x, int y){\n"
              "   return isgreaterequal(x,y) && islessequal(x,y) && islessgreater(x,y) && isgreater(x,y) && isless(x,y);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void integerOverflow() { // 5895
        // no signed integer overflow should happen
        check("void f(unsigned long long ull) {\n"
              "    if (ull == 0x89504e470d0a1a0a || ull == 0x8a4d4e470d0a1a0a) ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void redundantPointerOp() {
        check("int *f(int *x) {\n"
              "    return &*x;\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on 'x' - it's already a pointer.\n", errout.str());

        check("int *f(int *y) {\n"
              "    return &(*y);\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on 'y' - it's already a pointer.\n", errout.str());

        // no warning for bitwise AND
        check("void f(int *b) {\n"
              "    int x = 0x20 & *b;\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("", errout.str());

        // No message for double pointers to structs
        check("void f(struct foo **my_struct) {\n"
              "    char **pass_to_func = &(*my_struct)->buf;\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("", errout.str());

        // another double pointer to struct - with an array
        check("void f(struct foo **my_struct) {\n"
              "    char **pass_to_func = &(*my_struct)->buf[10];\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("", errout.str());

        // double pointer to array
        check("void f(char **ptr) {\n"
              "    int *x = &(*ptr)[10];\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("", errout.str());

        // function calls
        check("void f(Mutex *mut) {\n"
              "    pthread_mutex_lock(&*mut);\n"
              "}\n", nullptr, false, false);
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on 'mut' - it's already a pointer.\n", errout.str());

        // make sure we got the AST match for "(" right
        check("void f(char *ptr) {\n"
              "    if (&*ptr == NULL)\n"
              "        return;\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on 'ptr' - it's already a pointer.\n", errout.str());

        // no warning for macros
        check("#define MUTEX_LOCK(m) pthread_mutex_lock(&(m))\n"
              "void f(struct mutex *mut) {\n"
              "    MUTEX_LOCK(*mut);\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("", errout.str());

    }

    void test_isSameExpression() { // see #5738
        check("bool isInUnoIncludeFile(StringRef name) {"
              "   return  name.startswith(SRCDIR \"/com/\") || name.startswith(SRCDIR \"/uno/\");\n"
              "};", "test.cpp", false, false);
        ASSERT_EQUALS("", errout.str());
    }

    void raceAfterInterlockedDecrement() {
        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    whatever();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (counter)\n"
            "        return;\n"
            "    destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (!counter)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (counter > 0)\n"
            "        return;\n"
            "    destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (0 < counter)\n"
            "        return;\n"
            "    destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (counter == 0)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (0 == counter)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (0 != counter)\n"
            "        return;\n"
            "    destroy()\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (counter != 0)\n"
            "        return;\n"
            "    destroy()\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (counter <= 0)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    InterlockedDecrement(&counter);\n"
            "    if (0 >= counter)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (newCount)\n"
            "        return;\n"
            "    destroy();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (!newCount)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (newCount > 0)\n"
            "        return;\n"
            "    destroy();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (0 < newCount)\n"
            "        return;\n"
            "    destroy();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (newCount == 0)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (0 == newCount)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (0 != newCount)\n"
            "        return;\n"
            "    destroy()\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (newCount != 0)\n"
            "        return;\n"
            "    destroy()\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (newCount <= 0)\n"
            "        destroy();\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "void f() {\n"
            "    int counter = 0;\n"
            "    int newCount = InterlockedDecrement(&counter);\n"
            "    if (0 >= newCount)\n"
            "        destroy;\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInterlockedDecrement(
            "int f() {\n"
            "    int counter = 0;\n"
            "    if (InterlockedDecrement(&counter) == 0) {\n"
            "        destroy();\n"
            "        return 0;\n"
            "    } else {\n"
            "        return counter;\n"
            "    }\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "int f() {\n"
            "    int counter = 0;\n"
            "    if (::InterlockedDecrement(&counter) == 0) {\n"
            "        destroy();\n"
            "        return 0;\n"
            "    } else {\n"
            "        return counter;\n"
            "    }\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());


        checkInterlockedDecrement(
            "int f() {\n"
            "    int counter = 0;\n"
            "    if (InterlockedDecrement(&counter) == 0) {\n"
            "        destroy();\n"
            "        return 0;\n"
            "    }\n"
            "    return counter;\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "int f() {\n"
            "    int counter = 0;\n"
            "    if (::InterlockedDecrement(&counter) == 0) {\n"
            "        destroy();\n"
            "        return 0;\n"
            "    }\n"
            "    return counter;\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "int f() {\n"
            "    int counter = 0;\n"
            "    if (InterlockedDecrement(&counter) == 0) {\n"
            "        destroy();\n"
            "        return 0;\n"
            "    } else\n"
            "        return counter;\n"
            "    \n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());

        checkInterlockedDecrement(
            "int f() {\n"
            "    int counter = 0;\n"
            "    if (::InterlockedDecrement(&counter) == 0) {\n"
            "        destroy();\n"
            "        return 0;\n"
            "    } else\n"
            "        return counter;\n"
            "    \n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.\n", errout.str());
    }

    void testUnusedLabel() {
        check("void f() {\n"
              "    label:\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Label 'label' is not used.\n", errout.str());

        check("void f() {\n"
              "    label:\n"
              "    foo();\n"
              "    goto label;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    label:\n"
              "    foo();\n"
              "    goto label;\n"
              "}\n"
              "void g() {\n"
              "    label:\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Label 'label' is not used.\n", errout.str());

        check("void f() {\n"
              "    switch(a) {\n"
              "        default:\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    class X {\n"
              "        protected:\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    class X {\n"
              "        my_protected:\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int test(char art) {\n"
              "    switch (art) {\n"
              "    caseZERO:\n"
              "        return 0;\n"
              "    case1:\n"
              "        return 1;\n"
              "    case 2:\n"
              "        return 2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Label 'caseZERO' is not used. Should this be a 'case' of the enclosing switch()?\n"
                      "[test.cpp:5]: (warning) Label 'case1' is not used. Should this be a 'case' of the enclosing switch()?\n", errout.str());

        check("int test(char art) {\n"
              "    switch (art) {\n"
              "    case 2:\n"
              "        return 2;\n"
              "    }\n"
              "    label:\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Label 'label' is not used.\n", errout.str());
    }

    void testEvaluationOrder() {
        check("void f() {\n"
              "  int x = dostuff();\n"
              "  return x + x++;\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:3]: (error) Expression 'x+x++' depends on order of evaluation of side effects\n", errout.str());

        // #7226
        check("long int f1(const char *exp) {\n"
              "  return strtol(++exp, (char **)&exp, 10);\n"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());

        check("long int f1(const char *exp) {\n"
              "  return dostuff(++exp, exp, 10);\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:2]: (error) Expression '++exp,exp' depends on order of evaluation of side effects\n", errout.str());

        check("void f() {\n"
              "  int a;\n"
              "  while (a=x(), a==123) {}\n"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());

        // # 8717
        check("void f(int argc, char *const argv[]) {\n"
              "    char **local_argv = safe_malloc(sizeof (*local_argv));\n"
              "    int local_argc = 0;\n"
              "    local_argv[local_argc++] = argv[0];\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int x = 0;\n"
              "  return 0 + x++;\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x, int y) {\n"
              "  int a[10];\n"
              "  a[x+y] = a[y+x]++;;\n"
              "}\n", "test.c");
        ASSERT_EQUALS("[test.c:3]: (error) Expression 'a[x+y]=a[y+x]++' depends on order of evaluation of side effects\n", errout.str());
    }

    void testEvaluationOrderSelfAssignment() {
        // self assignment
        check("void f() {\n"
              "  int x = x = y + 1;\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:2]: (warning) Redundant assignment of 'x' to itself.\n", errout.str());
    }

    void testEvaluationOrderMacro() {
        // macro, don't bailout (#7233)
        checkP("#define X x\n"
               "void f(int x) {\n"
               "  return x + X++;\n"
               "}", "test.c");
        ASSERT_EQUALS("[test.c:3]: (error) Expression 'x+x++' depends on order of evaluation of side effects\n", errout.str());
    }

    void testEvaluationOrderSequencePointsFunctionCall() {
        // FP
        check("void f(int id) {\n"
              "  id = dostuff(id += 42);\n"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());

        // FN
        check("void f(int id) {\n"
              "  id = id + dostuff(id += 42);\n"
              "}", "test.c");
        TODO_ASSERT_EQUALS("error", "", errout.str());
    }

    void testEvaluationOrderSequencePointsComma() {
        check("int f(void) {\n"
              "  int t;\n"
              "  return (unsigned char)(t=1,t^c);\n"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());

        check("void f(void) {\n"
              "  int t;\n"
              "  dostuff(t=1,t^c);\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:3]: (error) Expression 't=1,t^c' depends on order of evaluation of side effects\n", errout.str());

        check("void f(void) {\n"
              "  int t;\n"
              "  dostuff((t=1,t),2);\n"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());

        // #8230
        check("void hprf(const char* fp) {\n"
              "    do\n"
              "        ;\n"
              "    while (++fp, (*fp) <= 0177);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());

        check("void hprf(const char* fp) {\n"
              "    do\n"
              "        ;\n"
              "    while (i++, ++fp, (*fp) <= 0177);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());

        check("void f(const char* fp) {\n"
              "    do\n"
              "        ;\n"
              "    while (f(++fp, (*fp) <= 7));\n"
              "}\n", "test.c");
        ASSERT_EQUALS("[test.c:4]: (error) Expression '++fp,(*fp)<=7' depends on order of evaluation of side effects\n", errout.str());
    }

    void testEvaluationOrderSizeof() {
        check("void f(char *buf) {\n"
              "  dostuff(buf++, sizeof(*buf));"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void testUnsignedLessThanZero() {
        check("struct d {\n"
              "  unsigned n;\n"
              "};\n"
              "void f(void) {\n"
              "  struct d d;\n"
              "  d.n = 3;\n"
              "\n"
              "  if (d.n < 0) {\n"
              "    return;\n"
              "  }\n"
              "\n"
              "  if (0 > d.n) {\n"
              "    return;\n"
              "  }\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:8]: (style) Checking if unsigned variable 'd.n' is less than zero.\n"
                      "[test.c:12]: (style) Checking if unsigned variable 'd.n' is less than zero.\n",
                      errout.str());
    }

    void doubleMove1() {
        check("void g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    g(std::move(a));\n"
              "    g(std::move(a));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void doubleMoveMemberInitialization1() {
        check("class A\n"
              "{\n"
              "    A(B && b)\n"
              "    :b1(std::move(b))\n"
              "    {\n"
              "        b2 = std::move(b);\n"
              "    }\n"
              "    B b1;\n"
              "    B b2;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Access of moved variable 'b'.\n", errout.str());
    }

    void doubleMoveMemberInitialization2() {
        check("class A\n"
              "{\n"
              "    A(B && b)\n"
              "    :b1(std::move(b)),\n"
              "     b2(std::move(b))\n"
              "    {}\n"
              "    B b1;\n"
              "    B b2;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Access of moved variable 'b'.\n", errout.str());
    }

    void moveAndAssign1() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    a = g(std::move(a));\n"
              "    a = g(std::move(a));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void moveAndAssign2() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    B b = g(std::move(a));\n"
              "    C c = g(std::move(a));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void moveAssignMoveAssign() {
        check("void h(A a);\n"
              "void f() {"
              "    A a;\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a = b;\n"
              "    h(a);\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a = b;\n"
              "    h(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Access of moved variable 'a'.\n"
                      "[test.cpp:8]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void moveAndReset1() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    a.reset(g(std::move(a)));\n"
              "    a.reset(g(std::move(a)));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void moveAndReset2() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b;\n"
              "    A c;\n"
              "    b.reset(g(std::move(a)));\n"
              "    c.reset(g(std::move(a)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void moveResetMoveReset() {
        check("void h(A a);\n"
              "void f() {"
              "    A a;\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a.reset(b);\n"
              "    h(a);\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a.reset(b);\n"
              "    h(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Access of moved variable 'a'.\n"
                      "[test.cpp:8]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void moveAndFunctionParameter() {
        check("void g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Access of moved variable 'a'.\n"
                      "[test.cpp:6]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void moveAndFunctionParameterReference() {
        check("void g(A & a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void moveAndFunctionParameterConstReference() {
        check("void g(A const & a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Access of moved variable 'a'.\n"
                      "[test.cpp:6]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void moveAndFunctionParameterUnknown() {
        check("void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning, inconclusive) Access of moved variable 'a'.\n"
                      "[test.cpp:5]: (warning, inconclusive) Access of moved variable 'a'.\n", errout.str());
    }

    void moveAndReturn() {
        check("int f(int i) {\n"
              "    A a;\n"
              "    A b;\n"
              "    g(std::move(a));\n"
              "    if (i)\n"
              "        return g(std::move(b));\n"
              "    return h(std::move(a),std::move(b));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Access of moved variable 'a'.\n", errout.str());
    }

    void moveAndClear() {
        check("void f() {\n"
              "    V v;\n"
              "    g(std::move(v));\n"
              "    v.clear();\n"
              "    if (v.empty()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void movedPointer() {
        check("void f() {\n"
              "    P p;\n"
              "    g(std::move(p));\n"
              "    x = p->x;\n"
              "    y = p->y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Access of moved variable 'p'.\n"
                      "[test.cpp:5]: (warning) Access of moved variable 'p'.\n", errout.str());
    }

    void moveAndAddressOf() {
        check("void f() {\n"
              "    std::string s1 = x;\n"
              "    std::string s2 = std::move(s1);\n"
              "    p = &s1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void partiallyMoved() {
        check("void f() {\n"
              "    A a;\n"
              "    gx(std::move(a).x());\n"
              "    gy(std::move(a).y());\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void moveAndLambda() {
        check("void f() {\n"
              "    A a;\n"
              "    auto h = [a=std::move(a)](){return g(std::move(a));};"
              "    b = a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void forwardAndUsed() {
        check("template<typename T>\n"
              "void f(T && t) {\n"
              "    g(std::forward<T>(t));\n"
              "    T s = t;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Access of forwarded variable 't'.\n", errout.str());
    }

    void funcArgNamesDifferent() {
        check("void func1(int a, int b, int c); \n"
              "void func1(int a, int b, int c) { }\n"
              "void func2(int a, int b, int c);\n"
              "void func2(int A, int B, int C) { }\n"
              "class Fred {\n"
              "    void func1(int a, int b, int c); \n"
              "    void func2(int a, int b, int c);\n"
              "    void func3(int a = 0, int b = 0, int c = 0);\n"
              "    void func4(int a = 0, int b = 0, int c = 0);\n"
              "};\n"
              "void Fred::func1(int a, int b, int c) { }\n"
              "void Fred::func2(int A, int B, int C) { }\n"
              "void Fred::func3(int a, int b, int c) { }\n"
              "void Fred::func4(int A, int B, int C) { }\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (style, inconclusive) Function 'func2' argument 1 names different: declaration 'a' definition 'A'.\n"
                      "[test.cpp:3] -> [test.cpp:4]: (style, inconclusive) Function 'func2' argument 2 names different: declaration 'b' definition 'B'.\n"
                      "[test.cpp:3] -> [test.cpp:4]: (style, inconclusive) Function 'func2' argument 3 names different: declaration 'c' definition 'C'.\n"
                      "[test.cpp:7] -> [test.cpp:12]: (style, inconclusive) Function 'func2' argument 1 names different: declaration 'a' definition 'A'.\n"
                      "[test.cpp:7] -> [test.cpp:12]: (style, inconclusive) Function 'func2' argument 2 names different: declaration 'b' definition 'B'.\n"
                      "[test.cpp:7] -> [test.cpp:12]: (style, inconclusive) Function 'func2' argument 3 names different: declaration 'c' definition 'C'.\n"
                      "[test.cpp:9] -> [test.cpp:14]: (style, inconclusive) Function 'func4' argument 1 names different: declaration 'a' definition 'A'.\n"
                      "[test.cpp:9] -> [test.cpp:14]: (style, inconclusive) Function 'func4' argument 2 names different: declaration 'b' definition 'B'.\n"
                      "[test.cpp:9] -> [test.cpp:14]: (style, inconclusive) Function 'func4' argument 3 names different: declaration 'c' definition 'C'.\n", errout.str());
    }

    void funcArgOrderDifferent() {
        check("void func1(int a, int b, int c);\n"
              "void func1(int a, int b, int c) { }\n"
              "void func2(int a, int b, int c);\n"
              "void func2(int c, int b, int a) { }\n"
              "void func3(int, int b, int c);\n"
              "void func3(int c, int b, int a) { }\n"
              "class Fred {\n"
              "    void func1(int a, int b, int c);\n"
              "    void func2(int a, int b, int c);\n"
              "    void func3(int a = 0, int b = 0, int c = 0);\n"
              "    void func4(int, int b = 0, int c = 0);\n"
              "};\n"
              "void Fred::func1(int a, int b, int c) { }\n"
              "void Fred::func2(int c, int b, int a) { }\n"
              "void Fred::func3(int c, int b, int a) { }\n"
              "void Fred::func4(int c, int b, int a) { }\n",
              nullptr, false, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Function 'func2' argument order different: declaration 'a, b, c' definition 'c, b, a'\n"
                      "[test.cpp:5] -> [test.cpp:6]: (warning) Function 'func3' argument order different: declaration ', b, c' definition 'c, b, a'\n"
                      "[test.cpp:9] -> [test.cpp:14]: (warning) Function 'func2' argument order different: declaration 'a, b, c' definition 'c, b, a'\n"
                      "[test.cpp:10] -> [test.cpp:15]: (warning) Function 'func3' argument order different: declaration 'a, b, c' definition 'c, b, a'\n"
                      "[test.cpp:11] -> [test.cpp:16]: (warning) Function 'func4' argument order different: declaration ', b, c' definition 'c, b, a'\n", errout.str());
    }

    // #7846 - Syntax error when using C++11 braced-initializer in default argument
    void cpp11FunctionArgInit() {
        // syntax error is not expected
        ASSERT_NO_THROW(check(
                            "\n void foo(int declaration = {}) {"
                            "\n   for (int i = 0; i < 10; i++) {}"
                            "\n }"
                            "\n  "
                        ));
        ASSERT_EQUALS("", errout.str());
    }

    void shadowVariables() {
        check("int x;\n"
              "void f() { int x; }\n");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:2]: (style) Local variable x shadows outer variable\n", errout.str());

        check("int x();\n"
              "void f() { int x; }\n");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:2]: (style) Local variable x shadows outer function\n", errout.str());

        check("struct C {\n"
              "    C(int x) : x(x) {}\n" // <- we do not want a FP here
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  if (cond) {int x;}\n" // <- not a shadow variable
              "  int x;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int size() {\n"
              "  int size;\n" // <- not a shadow variable
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestOther)
