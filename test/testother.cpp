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

#include "preprocessor.h"
#include "tokenize.h"
#include "symboldatabase.h"
#include "checkother.h"
#include "testsuite.h"
#include "testutils.h"
#include <tinyxml2.h>


class TestOther : public TestFixture {
public:
    TestOther() : TestFixture("TestOther") {
    }

private:
    void run() {
        TEST_CASE(emptyBrackets);

        TEST_CASE(zeroDiv1);
        TEST_CASE(zeroDiv2);
        TEST_CASE(zeroDiv3);
        TEST_CASE(zeroDiv4);
        TEST_CASE(zeroDiv5);
        TEST_CASE(zeroDiv6);
        TEST_CASE(zeroDiv7);  // #4930
        TEST_CASE(zeroDiv8);

        TEST_CASE(zeroDivCond); // division by zero / useless condition

        TEST_CASE(nanInArithmeticExpression);

        TEST_CASE(invalidFunctionUsage1);

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
        TEST_CASE(varScope19);      // Ticket #4994
        TEST_CASE(varScope20);      // Ticket #5103
        TEST_CASE(varScope21);      // Ticket #5382
        TEST_CASE(varScope22);      // Ticket #5684
        TEST_CASE(varScope23);      // Ticket #6154

        TEST_CASE(oldStylePointerCast);
        TEST_CASE(invalidPointerCast);

        TEST_CASE(passedByValue);

        TEST_CASE(mathfunctionCall_fmod);
        TEST_CASE(mathfunctionCall_sqrt);
        TEST_CASE(mathfunctionCall_log);
        TEST_CASE(mathfunctionCall_acos);
        TEST_CASE(mathfunctionCall_asin);
        TEST_CASE(mathfunctionCall_pow);
        TEST_CASE(mathfunctionCall_atan2);
        TEST_CASE(mathfunctionCall_precision);

        TEST_CASE(switchRedundantAssignmentTest);
        TEST_CASE(switchRedundantOperationTest);
        TEST_CASE(switchRedundantBitwiseOperationTest);
        TEST_CASE(switchFallThroughCase);
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

        TEST_CASE(memsetZeroBytes);
        TEST_CASE(memsetInvalid2ndParam);

        TEST_CASE(clarifyCalculation);
        TEST_CASE(clarifyStatement);

        TEST_CASE(duplicateBranch);
        TEST_CASE(duplicateBranch1); // tests extracted by http://www.viva64.com/en/b/0149/ ( Comparison between PVS-Studio and cppcheck ): Errors detected in Quake 3: Arena by PVS-Studio: Fragement 2
        TEST_CASE(duplicateBranch2); // empty macro
        TEST_CASE(duplicateExpression1);
        TEST_CASE(duplicateExpression2); // ticket #2730
        TEST_CASE(duplicateExpression3); // ticket #3317
        TEST_CASE(duplicateExpression4); // ticket #3354 (++)
        TEST_CASE(duplicateExpression5); // ticket #3749 (macros with same values)
        TEST_CASE(duplicateExpression6); // ticket #4639
        TEST_CASE(duplicateExpressionTernary); // #6391

        TEST_CASE(checkSignOfUnsignedVariable);
        TEST_CASE(checkSignOfPointer);

        TEST_CASE(checkForSuspiciousSemicolon1);
        TEST_CASE(checkForSuspiciousSemicolon2);

        TEST_CASE(checkInvalidFree);

        TEST_CASE(checkRedundantCopy);

        TEST_CASE(checkNegativeShift);

        TEST_CASE(incompleteArrayFill);

        TEST_CASE(redundantVarAssignment);
        TEST_CASE(redundantMemWrite);

        TEST_CASE(varFuncNullUB);

        TEST_CASE(checkPipeParameterSize); // ticket #3521

        TEST_CASE(checkCastIntToCharAndBack); // ticket #160

        TEST_CASE(checkCommaSeparatedReturn);

        TEST_CASE(checkComparisonFunctionIsAlwaysTrueOrFalse);

        TEST_CASE(integerOverflow); // #5895

        TEST_CASE(checkIgnoredReturnValue);

        TEST_CASE(redundantPointerOp);
    }

    void check(const char raw_code[], const char *filename = nullptr, bool experimental = false, bool inconclusive = true, bool runSimpleChecks=true, Settings* settings = 0, bool verify = true) {
        // Clear the error buffer..
        errout.str("");

        if (!settings) {
            static Settings _settings;
            settings = &_settings;
        }
        settings->addEnabled("style");
        settings->addEnabled("warning");
        settings->addEnabled("portability");
        settings->addEnabled("performance");
        settings->inconclusive = inconclusive;
        settings->experimental = experimental;

        // Preprocess file..
        Preprocessor preprocessor(settings);
        std::list<std::string> configurations;
        std::string filedata = "";
        std::istringstream fin(raw_code);
        preprocessor.preprocess(fin, filedata, configurations, "", settings->_includePaths);
        const std::string code = preprocessor.getcode(filedata, "", "");

        // Tokenize..
        Tokenizer tokenizer(settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename ? filename : "test.cpp");

        // Check..
        CheckOther checkOther(&tokenizer, settings, this);
        checkOther.runChecks(&tokenizer, settings, this);

        if (runSimpleChecks) {
            const std::string str1(tokenizer.tokens()->stringifyList(0,true));
            tokenizer.simplifyTokenList2();
            const std::string str2(tokenizer.tokens()->stringifyList(0,true));
            if (verify && str1 != str2)
                warnUnsimplified(str1, str2);
            checkOther.runSimplifiedChecks(&tokenizer, settings, this);
        }
    }

    void checkposix(const char code[]) {
        Settings settings;
        settings.addEnabled("warning");
        settings.standards.posix = true;

        check(code,
              nullptr, // filename
              false,   // experimental
              false,   // inconclusive
              true,    // runSimpleChecks
              &settings);
    }

    void check_preprocess_suppress(const char precode[], const char *filename = nullptr) {
        // Clear the error buffer..
        errout.str("");

        if (filename == nullptr)
            filename = "test.cpp";

        Settings settings;
        settings.addEnabled("warning");
        settings.addEnabled("style");
        settings.addEnabled("performance");
        settings.experimental = true;

        // Preprocess file..
        SimpleSuppressor logger(settings, this);
        Preprocessor preprocessor(&settings, &logger);
        std::list<std::string> configurations;
        std::string filedata = "";
        std::istringstream fin(precode);
        preprocessor.preprocess(fin, filedata, configurations, filename, settings._includePaths);
        const std::string code = preprocessor.getcode(filedata, "", filename);

        // Tokenize..
        Tokenizer tokenizer(&settings, &logger);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // Check..
        CheckOther checkOther(&tokenizer, &settings, &logger);
        checkOther.checkSwitchCaseFallThrough();

        logger.reportUnmatchedSuppressions(settings.nomsg.getUnmatchedLocalSuppressions(filename, false));
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
        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        // #4929 - if there is a user function with the name "div" don't warn
        check("void div(int a, int b);\n"
              "void f() {\n"
              "   div (1,0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0L;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0ul;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0L);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0x5);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

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

        // Don't warn about 0.0
        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0.0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0.5);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDiv5() {
        check("void f()\n"
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
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero.\n", errout.str());
    }

    void zeroDiv7() {
        check("void f() {\n"
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

    void zeroDivCond() {
        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x > 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x>0' is useless or there is division by zero at line 2.\n", errout.str());

        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x >= 1) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x>=1' is useless or there is division by zero at line 2.\n", errout.str());

        check("void f(int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x == 0) {}\n"
              "}", nullptr, false, true, true, nullptr, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x==0' is useless or there is division by zero at line 2.\n", errout.str());

        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x != 0) {}\n"
              "}", nullptr, false, true, true, nullptr, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'x!=0' is useless or there is division by zero at line 2.\n", errout.str());

        // function call
        check("void f1(int x, int y) { c=x/y; }\n"
              "void f2(unsigned int y) {\n"
              "    f1(123,y);\n"
              "    if (y>0){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:1]: (warning) Either the condition 'y>0' is useless or there is division by zero at line 1.\n", errout.str());

        // avoid false positives when variable is changed after division
        check("void f() {\n"
              "  unsigned int x = do_something();\n"
              "  int y = 17 / x;\n"
              "  x = some+calculation;\n"
              "  if (x != 0) {}\n"
              "}", nullptr, false, true, true, nullptr, false);
        ASSERT_EQUALS("", errout.str());

        {
            // function is called that might modify global variable
            check("void do_something();\n"
                  "int x;\n"
                  "void f() {\n"
                  "  int y = 17 / x;\n"
                  "  do_something();\n"
                  "  if (x != 0) {}\n"
                  "}", nullptr, false, true, true, nullptr, false);
            ASSERT_EQUALS("", errout.str());

            // function is called. but don't care, variable is local
            check("void do_something();\n"
                  "void f() {\n"
                  "  int x = some + calculation;\n"
                  "  int y = 17 / x;\n"
                  "  do_something();\n"
                  "  if (x != 0) {}\n"
                  "}", nullptr, false, true, true, nullptr, false);
            ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:4]: (warning) Either the condition 'x!=0' is useless or there is division by zero at line 4.\n", errout.str());
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
              "}", nullptr, false, true, true, nullptr, false);
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

        // ?:
        check("int f(int d) {\n"
              "  int r = (a?b:c) / d;\n"
              "  if (d == 0) {}\n"
              "}", nullptr, false, true, true, nullptr, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Either the condition 'd==0' is useless or there is division by zero at line 2.\n", errout.str());

        check("int f(int a) {\n"
              "  int r = a ? 1 / a : 0;\n"
              "  if (a == 0) {}\n"
              "}", nullptr, false, true, true, nullptr, false);
        ASSERT_EQUALS("", errout.str());

        check("int f(int a) {\n"
              "  int r = (a == 0) ? 0 : 1 / a;\n"
              "  if (a == 0) {}\n"
              "}", nullptr, false, true, true, nullptr, false);
        ASSERT_EQUALS("", errout.str());
    }

    void nanInArithmeticExpression() {
        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0 + 1.0\n"
              "   printf(\"%f\n\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0 - 1.0\n"
              "   printf(\"%f\n\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 1.0 + 3.0 / 0.0\n"
              "   printf(\"%f\n\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 1.0 - 3.0 / 0.0\n"
              "   printf(\"%f\n\", x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Using NaN/Inf in a computation.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0\n"
              "   printf(\"%f\n\", x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }

    void invalidFunctionUsage(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        const char cfg[] = "<?xml version=\"1.0\"?>\n"
                           "<def>\n"
                           "  <function name=\"memset\"> <arg nr=\"3\"><not-bool/></arg> </function>\n"
                           "  <function name=\"strtol\"> <arg nr=\"3\"><valid>0,2:36</valid></arg> </function>\n"
                           "</def>";
        tinyxml2::XMLDocument xmldoc;
        xmldoc.Parse(cfg, sizeof(cfg));

        Settings settings;
        settings.library.load(xmldoc);

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.invalidFunctionUsage();
    }

    void invalidFunctionUsage1() {
        invalidFunctionUsage("int f() { memset(a,b,sizeof(a)!=12); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        invalidFunctionUsage("int f() { memset(a,b,sizeof(a)!=0); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        invalidFunctionUsage("int f() { strtol(a,b,sizeof(a)!=12); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strtol() argument nr 3. The value is 0 or 1 (comparison result) but the valid values are '0,2:36'.\n", errout.str());

        invalidFunctionUsage("int f() { strtol(a,b,1); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strtol() argument nr 3. The value is 1 but the valid values are '0,2:36'.\n", errout.str());

        invalidFunctionUsage("int f() { strtol(a,b,10); }");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkVariableScope();
    }

    void varScope1() {
        varScope("unsigned short foo()\n"
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
        varScope("int foo()\n"
                 "{\n"
                 "    Error e;\n"
                 "    e.SetValue(12);\n"
                 "    throw e;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope3() {
        varScope("void foo()\n"
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
        varScope("void foo()\n"
                 "{\n"
                 "    int i;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope5() {
        varScope("void f(int x)\n"
                 "{\n"
                 "    int i = 0;\n"
                 "    if (x) {\n"
                 "        for ( ; i < 10; ++i) ;\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable 'i' can be reduced.\n", errout.str());

        varScope("void f(int x) {\n"
                 "    const unsigned char i = 0;\n"
                 "    if (x) {\n"
                 "        for ( ; i < 10; ++i) ;\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("void f(int x)\n"
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
        varScope("void f(int x)\n"
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

        varScope("void f()\n"
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

        varScope("void f(int &x)\n"
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
        varScope("void f(int x)\n"
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
        varScope("void test() {\n"
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
        varScope("class fred {\n"
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
        varScope("int f()\n"
                 "{\n"
                 "    int x = 0;\n"
                 "    FOR {\n"
                 "        foo(x++);\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope11() {
        varScope("int f() {\n"
                 "    int x = 0;\n"
                 "    AB ab = { x, 0 };\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("int f() {\n"
                 "    int x = 0;\n"
                 "    if (a == 0) { ++x; }\n"
                 "    AB ab = { x, 0 };\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("int f() {\n"
                 "    int x = 0;\n"
                 "    if (a == 0) { ++x; }\n"
                 "    if (a == 1) { AB ab = { x, 0 }; }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope12() {
        varScope("void f(int x) {\n"
                 "    int i[5];\n"
                 "    int* j = y;\n"
                 "    if (x)\n"
                 "        foo(i);\n"
                 "    foo(j);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'i' can be reduced.\n", errout.str());

        varScope("void f(int x) {\n"
                 "    int i[5];\n"
                 "    int* j;\n"
                 "    if (x)\n"
                 "        j = i;\n"
                 "    foo(j);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("void f(int x) {\n"
                 "    const bool b = true;\n"
                 "    x++;\n"
                 "    if (x == 5)\n"
                 "        foo(b);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("void f(int x) {\n"
                 "    const bool b = x;\n"
                 "    x++;\n"
                 "    if (x == 5)\n"
                 "        foo(b);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope13() {
        // #2770
        varScope("void f() {\n"
                 "    int i = 0;\n"
                 "    forever {\n"
                 "        if (i++ == 42) { break; }\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope14() {
        // #3941
        varScope("void f() {\n"
                 "    const int i( foo());\n"
                 "    if(a) {\n"
                 "        for ( ; i < 10; ++i) ;\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope15() {
        // #4573
        varScope("void f() {\n"
                 "    int a,b,c;\n"
                 "    if (a);\n"
                 "    else if(b);\n"
                 "    else if(c);\n"
                 "    else;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope16() {
        varScope("void f() {\n"
                 "    int a = 0;\n"
                 "    while((++a) < 56) {\n"
                 "        foo();\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("void f() {\n"
                 "    int a = 0;\n"
                 "    do {\n"
                 "        foo();\n"
                 "    } while((++a) < 56);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("void f() {\n"
                 "    int a = 0;\n"
                 "    do {\n"
                 "        a = 64;\n"
                 "        foo(a);\n"
                 "    } while((++a) < 56);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        varScope("void f() {\n"
                 "    int a = 0;\n"
                 "    do {\n"
                 "        a = 64;\n"
                 "        foo(a);\n"
                 "    } while(z());\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'a' can be reduced.\n", errout.str());
    }

    void varScope17() {
        varScope("void f() {\n"
                 "    int x;\n"
                 "    if (a) {\n"
                 "        x = stuff(x);\n"
                 "        morestuff(x);\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'x' can be reduced.\n", errout.str());

        varScope("void f() {\n"
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
        varScope("void f() {\n"
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

        varScope("void f() {\n"
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

        varScope("void f() {\n"
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

        varScope("void f() {\n"
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

    void varScope19() { // Ticket #4994
        varScope("long f () {\n"
                 "  return a >> extern\n"
                 "}\n"
                 "long a = 1 ;\n"
                 "long b = 2 ;");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope20() { // Ticket #5103 - constant variable only used in inner scope
        varScope("int f(int a) {\n"
                 "  const int x = 234;\n"
                 "  int b = a;\n"
                 "  if (b > 32) b = x;\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope21() { // Ticket #5382 - initializing two-dimensional array
        varScope("int test() {\n"
                 "    int test_value = 3;\n"
                 "    int test_array[1][1] = { { test_value } };\n"
                 "    return sizeof(test_array);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope22() { // Ticket #5684 - "The scope of the variable 'p' can be reduced" - But it can not.
        varScope("void foo() {\n"
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
        varScope("void foo() {\n"
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
        varScope("int main() {\n"
                 "   size_t myCounter = 0;\n"
                 "   Test myTest([&](size_t aX){\n"
                 "       std::cout << myCounter += aX << std::endl;\n"
                 "   });\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkOldStylePointerCast(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.standards.cpp = Standards::CPP03; // #5560

        // Tokenize..
        Tokenizer tokenizerCpp(&settings, this);
        std::istringstream istr(code);
        tokenizerCpp.tokenize(istr, "test.cpp");

        Tokenizer tokenizerC(&settings, this);
        std::istringstream istr2(code);
        tokenizerC.tokenize(istr2, "test.c");

        CheckOther checkOtherCpp(&tokenizerCpp, &settings, this);
        checkOtherCpp.warningOldStylePointerCast();

        CheckOther checkOtherC(&tokenizerC, &settings, this);
        checkOtherC.warningOldStylePointerCast();
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
        ASSERT_EQUALS("[test.cpp:3]: (portability) Casting between float* and double* which have an incompatible binary data representation.\n"
                      "[test.cpp:4]: (portability) Casting between float* and long double* which have an incompatible binary data representation.\n", errout.str());

        checkInvalidPointerCast("void test(const float* f) {\n"
                                "    double *d = (double*)f;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Casting between float* and double* which have an incompatible binary data representation.\n", errout.str());

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

    void testPassedByValue(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("performance");

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkConstantFunctionParameter();
    }

    void passedByValue() {
        testPassedByValue("void f(const std::string str) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::string::size_type x) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("class Foo;\nvoid f(const Foo foo) {}");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'foo' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::string &str) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::vector<int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::vector<std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::vector<std::string>::size_type s) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::vector<int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::map<int,int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::map<int,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::map<std::string,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::map<int,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::map<std::string,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::streamoff pos) {}");
        ASSERT_EQUALS("", errout.str());

        // #5824
        testPassedByValue("void log(const std::string& file, int line, const std::string& function, const std::string str, ...) {}");
        ASSERT_EQUALS("", errout.str());

        // #5534
        testPassedByValue("struct float3 { };\n"
                          "typedef float3 vec;\n"
                          "class Plane {\n"
                          "    vec Refract(vec &vec) const;\n"
                          "    bool IntersectLinePlane(const vec &planeNormal);\n"
                          "}; ");
        ASSERT_EQUALS("", errout.str());

    }

    void mathfunctionCall_sqrt() {
        // sqrt, sqrtf, sqrtl
        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(-1) << std::endl;\n"
              "    std::cout <<  sqrtf(-1) << std::endl;\n"
              "    std::cout <<  sqrtl(-1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1 to sqrt() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1 to sqrtf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1 to sqrtl() leads to implementation-defined result.\n", errout.str());

        // implementation-defined behaviour for "finite values of x<0" only:
        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(-0.) << std::endl;\n"
              "    std::cout <<  sqrtf(-0.) << std::endl;\n"
              "    std::cout <<  sqrtl(-0.) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(1) << std::endl;\n"
              "    std::cout <<  sqrtf(1) << std::endl;\n"
              "    std::cout <<  sqrtl(1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_log() {
        // log,log10,logf,logl,log10f,log10l
        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-2) << std::endl;\n"
              "    std::cout <<  logf(-2) << std::endl;\n"
              "    std::cout <<  logl(-2) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -2 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -2 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -2 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1) << std::endl;\n"
              "    std::cout <<  logf(-1) << std::endl;\n"
              "    std::cout <<  logl(-1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1.0) << std::endl;\n"
              "    std::cout <<  logf(-1.0) << std::endl;\n"
              "    std::cout <<  logl(-1.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1.0 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1.0 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1.0 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-0.1) << std::endl;\n"
              "    std::cout <<  logf(-0.1) << std::endl;\n"
              "    std::cout <<  logl(-0.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -0.1 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -0.1 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -0.1 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(0) << std::endl;\n"
              "    std::cout <<  logf(0.) << std::endl;\n"
              "    std::cout <<  logl(0.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value 0 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value 0. to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value 0.0 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(1E-3)    << std::endl;\n"
              "    std::cout <<  logf(1E-3)   << std::endl;\n"
              "    std::cout <<  logl(1E-3)   << std::endl;\n"
              "    std::cout <<  log(1.0E-3)  << std::endl;\n"
              "    std::cout <<  logf(1.0E-3) << std::endl;\n"
              "    std::cout <<  logl(1.0E-3) << std::endl;\n"
              "    std::cout <<  log(1.0E+3)  << std::endl;\n"
              "    std::cout <<  logf(1.0E+3) << std::endl;\n"
              "    std::cout <<  logl(1.0E+3) << std::endl;\n"
              "    std::cout <<  log(2.0)     << std::endl;\n"
              "    std::cout <<  logf(2.0)    << std::endl;\n"
              "    std::cout <<  logf(2.0f)   << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::string *log(0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3473 - no warning if "log" is a variable
        check("Fred::Fred() : log(0) { }");
        ASSERT_EQUALS("", errout.str());

        // #5748
        check("void f() { foo.log(0); }");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_acos() {
        // acos, acosf, acosl
        check("void foo()\n"
              "{\n"
              " return acos(-1)      \n"
              "    + acos(0.1)       \n"
              "    + acos(0.0001)    \n"
              "    + acos(0.01)      \n"
              "    + acos(1.0E-1)    \n"
              "    + acos(-1.0E-1)   \n"
              "    + acos(+1.0E-1)   \n"
              "    + acos(0.1E-1)    \n"
              "    + acos(+0.1E-1)   \n"
              "    + acos(-0.1E-1)   \n"
              "    + acosf(-1)       \n"
              "    + acosf(0.1)      \n"
              "    + acosf(0.0001)   \n"
              "    + acosf(0.01)     \n"
              "    + acosf(1.0E-1)   \n"
              "    + acosf(-1.0E-1)  \n"
              "    + acosf(+1.0E-1)  \n"
              "    + acosf(0.1E-1)   \n"
              "    + acosf(+0.1E-1)  \n"
              "    + acosf(-0.1E-1)  \n"
              "    + acosl(-1)       \n"
              "    + acosl(0.1)      \n"
              "    + acosl(0.0001)   \n"
              "    + acosl(0.01)     \n"
              "    + acosl(1.0E-1)   \n"
              "    + acosl(-1.0E-1)  \n"
              "    + acosl(+1.0E-1)  \n"
              "    + acosl(0.1E-1)   \n"
              "    + acosl(+0.1E-1)  \n"
              "    + acosl(-0.1E-1); \n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(1.1) << std::endl;\n"
              "    std::cout <<  acosf(1.1) << std::endl;\n"
              "    std::cout <<  acosl(1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value 1.1 to acos() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value 1.1 to acosf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value 1.1 to acosl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(-1.1) << std::endl;\n"
              "    std::cout <<  acosf(-1.1) << std::endl;\n"
              "    std::cout <<  acosl(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1.1 to acos() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1.1 to acosf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1.1 to acosl() leads to implementation-defined result.\n", errout.str());
    }

    void mathfunctionCall_asin() {
        // asin, asinf, asinl
        check("void foo()\n"
              "{\n"
              " return asin(1)       \n"
              "    + asin(-1)        \n"
              "    + asin(0.1)       \n"
              "    + asin(0.0001)    \n"
              "    + asin(0.01)      \n"
              "    + asin(1.0E-1)    \n"
              "    + asin(-1.0E-1)   \n"
              "    + asin(+1.0E-1)   \n"
              "    + asin(0.1E-1)    \n"
              "    + asin(+0.1E-1)   \n"
              "    + asin(-0.1E-1)   \n"
              "    + asinf(1)        \n"
              "    + asinf(-1)       \n"
              "    + asinf(0.1)      \n"
              "    + asinf(0.0001)   \n"
              "    + asinf(0.01)     \n"
              "    + asinf(1.0E-1)   \n"
              "    + asinf(-1.0E-1)  \n"
              "    + asinf(+1.0E-1)  \n"
              "    + asinf(0.1E-1)   \n"
              "    + asinf(+0.1E-1)  \n"
              "    + asinf(-0.1E-1)  \n"
              "    + asinl(1)        \n"
              "    + asinl(-1)       \n"
              "    + asinl(0.1)      \n"
              "    + asinl(0.0001)   \n"
              "    + asinl(0.01)     \n"
              "    + asinl(1.0E-1)   \n"
              "    + asinl(-1.0E-1)  \n"
              "    + asinl(+1.0E-1)  \n"
              "    + asinl(0.1E-1)   \n"
              "    + asinl(+0.1E-1)  \n"
              "    + asinl(-0.1E-1); \n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  asin(1.1) << std::endl;\n"
              "    std::cout <<  asinf(1.1) << std::endl;\n"
              "    std::cout <<  asinl(1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value 1.1 to asin() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value 1.1 to asinf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value 1.1 to asinl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  asin(-1.1) << std::endl;\n"
              "    std::cout <<  asinf(-1.1) << std::endl;\n"
              "    std::cout <<  asinl(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1.1 to asin() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1.1 to asinf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1.1 to asinl() leads to implementation-defined result.\n", errout.str());
    }

    void mathfunctionCall_pow() {
        // pow, powf, powl
        check("void foo()\n"
              "{\n"
              "    std::cout <<  pow(0,-10) << std::endl;\n"
              "    std::cout <<  powf(0,-10) << std::endl;\n"
              "    std::cout <<  powl(0,-10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing values 0 and -10 to pow() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing values 0 and -10 to powf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing values 0 and -10 to powl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  pow(0,10) << std::endl;\n"
              "    std::cout <<  powf(0,10) << std::endl;\n"
              "    std::cout <<  powl(0,10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_atan2() {
        // atan2
        check("void foo()\n"
              "{\n"
              "    std::cout <<  atan2(1,1)        << std::endl;\n"
              "    std::cout <<  atan2(-1,-1)      << std::endl;\n"
              "    std::cout <<  atan2(0.1,1)      << std::endl;\n"
              "    std::cout <<  atan2(0.0001,100) << std::endl;\n"
              "    std::cout <<  atan2(0.01m-1)    << std::endl;\n"
              "    std::cout <<  atan2(1.0E-1,-3)  << std::endl;\n"
              "    std::cout <<  atan2(-1.0E-1,+2) << std::endl;\n"
              "    std::cout <<  atan2(+1.0E-1,0)  << std::endl;\n"
              "    std::cout <<  atan2(0.1E-1,3)   << std::endl;\n"
              "    std::cout <<  atan2(+0.1E-1,1)  << std::endl;\n"
              "    std::cout <<  atan2(-0.1E-1,8)  << std::endl;\n"
              "    std::cout <<  atan2f(1,1)        << std::endl;\n"
              "    std::cout <<  atan2f(-1,-1)      << std::endl;\n"
              "    std::cout <<  atan2f(0.1,1)      << std::endl;\n"
              "    std::cout <<  atan2f(0.0001,100) << std::endl;\n"
              "    std::cout <<  atan2f(0.01m-1)    << std::endl;\n"
              "    std::cout <<  atan2f(1.0E-1,-3)  << std::endl;\n"
              "    std::cout <<  atan2f(-1.0E-1,+2) << std::endl;\n"
              "    std::cout <<  atan2f(+1.0E-1,0)  << std::endl;\n"
              "    std::cout <<  atan2f(0.1E-1,3)   << std::endl;\n"
              "    std::cout <<  atan2f(+0.1E-1,1)  << std::endl;\n"
              "    std::cout <<  atan2f(-0.1E-1,8)  << std::endl;\n"
              "    std::cout <<  atan2l(1,1)        << std::endl;\n"
              "    std::cout <<  atan2l(-1,-1)      << std::endl;\n"
              "    std::cout <<  atan2l(0.1,1)      << std::endl;\n"
              "    std::cout <<  atan2l(0.0001,100) << std::endl;\n"
              "    std::cout <<  atan2l(0.01m-1)    << std::endl;\n"
              "    std::cout <<  atan2l(1.0E-1,-3)  << std::endl;\n"
              "    std::cout <<  atan2l(-1.0E-1,+2) << std::endl;\n"
              "    std::cout <<  atan2l(+1.0E-1,0)  << std::endl;\n"
              "    std::cout <<  atan2l(0.1E-1,3)   << std::endl;\n"
              "    std::cout <<  atan2l(+0.1E-1,1)  << std::endl;\n"
              "    std::cout <<  atan2l(-0.1E-1,8)  << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  atan2(0,0) << std::endl;\n"
              "    std::cout <<  atan2f(0,0) << std::endl;\n"
              "    std::cout <<  atan2l(0,0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing values 0 and 0 to atan2() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing values 0 and 0 to atan2f() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing values 0 and 0 to atan2l() leads to implementation-defined result.\n", errout.str());
    }

    void mathfunctionCall_fmod() {
        // fmod, fmodl, fmodf
        check("void foo()\n"
              "{\n"
              "    std::cout <<  fmod(1.0,0) << std::endl;\n"
              "    std::cout <<  fmodf(1.0,0) << std::endl;\n"
              "    std::cout <<  fmodl(1.0,0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing values 1.0 and 0 to fmod() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing values 1.0 and 0 to fmodf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing values 1.0 and 0 to fmodl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  fmod(1.0,1) << std::endl;\n"
              "    std::cout <<  fmodf(1.0,1) << std::endl;\n"
              "    std::cout <<  fmodl(1.0,1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_precision() {
        check("void foo() {\n"
              "    print(exp(x) - 1);\n"
              "    print(log(1 + x));\n"
              "    print(1 - erf(x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression 'exp(x) - 1' can be replaced by 'expm1(x)' to avoid loss of precision.\n"
                      "[test.cpp:3]: (style) Expression 'log(1 + x)' can be replaced by 'log1p(x)' to avoid loss of precision.\n"
                      "[test.cpp:4]: (style) Expression '1 - erf(x)' can be replaced by 'erfc(x)' to avoid loss of precision.\n", errout.str());

        check("void foo() {\n"
              "    print(exp(x) - 1.0);\n"
              "    print(log(1.0 + x));\n"
              "    print(1.0 - erf(x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression 'exp(x) - 1' can be replaced by 'expm1(x)' to avoid loss of precision.\n"
                      "[test.cpp:3]: (style) Expression 'log(1 + x)' can be replaced by 'log1p(x)' to avoid loss of precision.\n"
                      "[test.cpp:4]: (style) Expression '1 - erf(x)' can be replaced by 'erfc(x)' to avoid loss of precision.\n", errout.str());

        check("void foo() {\n"
              "    print(exp(3 + x*f(a)) - 1);\n"
              "    print(log(x*4 + 1));\n"
              "    print(1 - erf(34*x + f(x) - c));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression 'exp(x) - 1' can be replaced by 'expm1(x)' to avoid loss of precision.\n"
                      "[test.cpp:3]: (style) Expression 'log(1 + x)' can be replaced by 'log1p(x)' to avoid loss of precision.\n"
                      "[test.cpp:4]: (style) Expression '1 - erf(x)' can be replaced by 'erfc(x)' to avoid loss of precision.\n", errout.str());

        check("void foo() {\n"
              "    print(2*exp(x) - 1);\n"
              "    print(1 - erf(x)/2.0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());

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

        check("void foo(char *str, int a)\n"
              "{\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "    }\n"
              "}", 0, false, false, false);
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:8]: (warning) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n", errout.str());

        check("void foo(char *str, int a)\n"
              "{\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strncpy(str, \"a'\");\n"
              "    case 3:\n"
              "      strncpy(str, \"b'\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:8]: (warning) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n", errout.str());

        check("void foo(char *str, int a)\n"
              "{\n"
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
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:10]: (warning) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n", errout.str());

        check("void foo(char *str, int a)\n"
              "{\n"
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

        check("void foo(char *str, int a)\n"
              "{\n"
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());

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

    void switchFallThroughCase() {
        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            break;\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            break;\n"
            "        case 2:\n"
            "            continue;\n"
            "        case 3:\n"
            "            return;\n"
            "        case 4:\n"
            "            exit(1);\n"
            "        case 5:\n"
            "            goto end;\n"
            "        case 6:\n"
            "            throw e;\n"
            "        case 7:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 0:\n"
            "        case 1:\n"
            "            break;\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "        default:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "            // fall through\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "            /* FALLTHRU */\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "            break;\n"
            "            // fall through\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7]: (information) Unmatched suppression: switchCaseFallThrough\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            {\n"
            "                break;\n"
            "            }\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            for (;;) {\n"
            "                break;\n"
            "            }\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            if (b) {\n"
            "                break;\n"
            "            } else {\n"
            "                break;\n"
            "            }\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            if (b) {\n"
            "                break;\n"
            "            } else {\n"
            "            }\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:8]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            if (b) {\n"
            "                break;\n"
            "            }\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            if (b) {\n"
            "            } else {\n"
            "                break;\n"
            "            }\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:8]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            if (b) {\n"
            "        case 2:\n"
            "            } else {\n"
            "                break;\n"
            "            }\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "            int x;\n"
            "        case 1:\n"
            "            break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "    case 1:\n"
            "        g();\n"
            "        switch (b) {\n"
            "            case 1:\n"
            "                return;\n"
            "            default:\n"
            "                return;\n"
            "        }\n"
            "    case 2:\n"
            "        break;\n"
            "    }\n"
            "}");
        // This fails because the switch parsing code currently doesn't understand
        // that all paths after g() actually return. It's a pretty unusual case
        // (no pun intended).
        TODO_ASSERT_EQUALS("",
                           "[test.cpp:11]: (style) Switch falls through case without comment. 'break;' missing?\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "    case 1:\n"
            "#ifndef A\n"
            "        g();\n"
            "        // fall through\n"
            "#endif\n"
            "    case 2:\n"
            "        break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "    case 1:\n"
            "        goto leave;\n"
            "    case 2:\n"
            "        break;\n"
            "    }\n"
            "leave:\n"
            "    if (x) {\n"
            "        g();\n"
            "        return;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "    case 1:\n"
            "        g();\n"
            "        // fall through\n"
            "    case 2:\n"
            "        g();\n"
            "        // falls through\n"
            "    case 3:\n"
            "        g();\n"
            "        // fall-through\n"
            "    case 4:\n"
            "        g();\n"
            "        // drop through\n"
            "    case 5:\n"
            "        g();\n"
            "        // pass through\n"
            "    case 5:\n"
            "        g();\n"
            "        // no break\n"
            "    case 5:\n"
            "        g();\n"
            "        // fallthru\n"
            "    case 6:\n"
            "        g();\n"
            "        /* fall */\n"
            "    default:\n"
            "        break;\n"
            "    }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    // unrelated comment saying 'fall through'\n"
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
        settings.library.argumentChecks["exit"][1] = Library::ArgumentChecks();
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
        ASSERT_EQUALS("", errout.str());

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

        check("%: return ; ()"); // Don't crash. #3441.

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

        // Garbage code - don't crash
        check("namespace pr16989 {\n"
              "    class C {\n"
              "        C tpl_mem(T *) { return }\n"
              "    };\n"
              "}");
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
              "}", nullptr, false, true, true, nullptr, false);
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
              "    printf(\"%i\n\", ({x==0;}));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x) {\n"
              "    printf(\"%i\n\", ({int x = do_something(); x == 0;}));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x) {\n"
              "    printf(\"%i\n\", ({x == 0; x > 0 ? 10 : 20}));\n"
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
              "}", nullptr, false, true, true, nullptr, false);
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

        check("void foo()\n"
              "{\n"
              "    std::string var = var = \"test\";\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of 'var' to itself.\n", "", errout.str());

        check("struct A { int b; };\n"
              "void foo(A* a1, A* a2) {\n"
              "    a1->b = a1->b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of 'a1.b' to itself.\n", errout.str());

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
              "}", nullptr, false, true, true, nullptr, false);
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
        ASSERT_EQUALS("[test.cpp:6]: (warning) Redundant assignment of 'this.var' to itself.\n", errout.str());

        check("class Foo {\n"
              "    int var;\n"
              "    void func(int var);\n"
              "};\n"
              "void Foo::func(int var) {\n"
              "    this->var = var;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6406 - false positive for struct with designated initializer
        check("struct callbacks {\n"
              "    void (*something)(void);\n"
              "};\n"
              "void something(void) {}\n"
              "void f() {\n"
              "    struct callbacks ops = { .something = something };\n"
              "}\n");
        TODO_ASSERT_EQUALS("", "[test.cpp:6]: (warning) Redundant assignment of 'something' to itself.\n", errout.str());

        // #6406 - designated initializer doing bogus self assignment
        check("struct callbacks {\n"
              "    void (*something)(void);\n"
              "};\n"
              "void something(void) {}\n"
              "void f() {\n"
              "    struct callbacks ops = { .something = ops.something };\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (warning) Redundant assignment of 'something' to itself.\n", "", errout.str());

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
        ASSERT_EQUALS("[test.cpp:15]: (error) Instance of 'Lock' object is destroyed immediately.\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:4]: (error) Instance of 'NotAFunction' object is destroyed immediately.\n", errout.str());
    }

    void testMisusedScopeObjectPicksStruct() {
        check("struct NotAClass;\n"
              "bool func ( )\n"
              "{\n"
              "    NotAClass ( 123 ) ;\n"
              "    return true ;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:4]: (error) Instance of 'NotAClass' object is destroyed immediately.\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:7]: (error) Instance of 'Foo' object is destroyed immediately.\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:7]: (error) Instance of 'cb_watch_bool' object is destroyed immediately.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (error) Instance of 'Foo' object is destroyed immediately.\n", errout.str());
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

    void memsetZeroBytes() {
        check("void f() {\n"
              "    memset(p, 10, 0x0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) memset() called to fill 0 bytes of 'p'.\n", errout.str());

        check("void f() {\n"
              "    memset(p, sizeof(p), 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) memset() called to fill 0 bytes of 'p'.\n", errout.str());

        check("void f() {\n"
              "    memset(p, sizeof(p), i);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void memsetInvalid2ndParam() {
        check("void f() {\n"
              "    int* is = new int[10];\n"
              "    memset(is, 1.0f, 40);\n"
              "    int* is2 = new int[10];\n"
              "    memset(is2, 0.1f, 40);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (portability) The 2nd memset() argument '1.0f' is a float, its representation is implementation defined.\n"
                      "[test.cpp:5]: (portability) The 2nd memset() argument '0.1f' is a float, its representation is implementation defined.\n", errout.str());

        check("void f() {\n"
              "    int* is = new int[10];\n"
              "    float g = computeG();\n"
              "    memset(is, g, 40);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (portability) The 2nd memset() argument 'g' is a float, its representation is implementation defined.\n", errout.str());

        check("void f() {\n"
              "    int* is = new int[10];\n"
              "    memset(is, 0.0f, 40);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n" // FP
              "    float x = 2.3f;\n"
              "    memset(a, (x?64:0), 40);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    short ss[] = {1, 2};\n"
              "    memset(ss, 256, 4);\n"
              "    short ss2[2];\n"
              "    memset(ss2, -129, 4);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) The 2nd memset() argument '256' doesn't fit into an 'unsigned char'.\n"
                      "[test.cpp:5]: (warning) The 2nd memset() argument '-129' doesn't fit into an 'unsigned char'.\n", errout.str());

        check("void f() {\n"
              "    int is[10];\n"
              "    memset(is, 0xEE, 40);\n"
              "    unsigned char* cs = malloc(256);\n"
              "    memset(cs, -1, 256);\n"
              "    short* ss[30];\n"
              "    memset(ss, -128, 60);\n"
              "    char cs2[30];\n"
              "    memset(cs2, 255, 30);\n"
              "    char cs3[30];\n"
              "    memset(cs3, 0, 30);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int is[10];\n"
              "    const int i = g();\n"
              "    memset(is, 1.0f + i, 40);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (portability) The 2nd memset() argument '1.0f+i' is a float, its representation is implementation defined.\n", errout.str());
    }

    void clarifyCalculation() {
        check("int f(char c) {\n"
              "    return 10 * (c == 0) ? 1 : 2;\n"
              "}", nullptr, false, false, true, nullptr, false);
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '*' and '?'.\n", errout.str());

        check("void f(char c) {\n"
              "    printf(\"%i\", 10 * (c == 0) ? 1 : 2);\n"
              "}", nullptr, false, false, true, nullptr, false);
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for '*' and '?'.\n", errout.str());

        check("void f() {\n"
              "    return (2*a)?b:c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #2585 - segmentation fault for invalid code
        check("abcdef?""?<"
              "123456?""?>"
              "+?""?=");
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

        // #5166 segmentation fault (invalid code) in lib/checkother.cpp:329 ( void * f { } void b ( ) { * f } )
        check("void * f { } void b ( ) { * f }");
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
        // Errors detected in Quake 3: Arena by PVS-Studio: Fragement 2
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
        Preprocessor::macroChar = '$';

        check("void f(int x) {\n" // #4329
              "  if (x)\n"
              "    $;\n"
              "  else\n"
              "    $;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpression1() {
        check("void foo(int a) {\n"
              "    if (a == a) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void fun(int b) {\n"
              "    return  a && a ||\n"
              "            b == b &&\n"
              "            d > d &&\n"
              "            e < e &&\n"
              "            f ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n"
                      "[test.cpp:3] -> [test.cpp:3]: (style) Same expression on both sides of '=='.\n"
                      "[test.cpp:4] -> [test.cpp:4]: (style) Same expression on both sides of '>'.\n"
                      "[test.cpp:5] -> [test.cpp:5]: (style) Same expression on both sides of '<'.\n", errout.str());

        check("void foo() {\n"
              "    return a && a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo() {\n"
              "    a = b && b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo(int b) {\n"
              "    f(a,b == b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void foo(int b) {\n"
              "    f(b == b, a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void foo() {\n"
              "    if (x!=2 || x!=2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if ((a < b) && (b > a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if ((a <= b) && (b >= a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo() {\n"
              "    if (x!=2 || y!=3 || x!=2) {}\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("void foo() {\n"
              "    if (x!=2 && (x=y) && x!=2) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if (a && b || a && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (a && b || b && c) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if (a && b | b && c) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '|'.\n", errout.str());

        check("void foo() {\n"
              "    if ((a + b) | (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '|'.\n", errout.str());

        check("void foo() {\n"
              "    if ((a | b) & (a | b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&'.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    if ((a | b) == (a | b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

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

        check("void foo() {\n"
              "    if (a && b && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void foo() {\n"
              "    if (a || b || b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (a / 1000 / 1000) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo(int i) {\n"
              "    return i/i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '/'.\n", errout.str());

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
        check("void f(long x) { if (({ 1+2; }) == ({3+4};)) {} }");
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

        check("void f() {\n"
              "    bool a = bar.isSet() && bar->isSet();\n"
              "    bool b = bar.isSet() && bar.isSet();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:3]: (style) Same expression on both sides of '&&'.\n", errout.str());


        check("void foo() {\n"
              "    if ((b + a) | (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '|'.\n", errout.str());

        check("void foo() {\n"
              "    if ((b > a) | (a > b)) {}\n" // > is not commutative
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if ((b + a) > (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '>'.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x == 1) && (x == 0x00000001))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    if (Four == 4) {}"
              "}", nullptr, false, true, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:3]: (style) Same expression on both sides of '=='.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:4]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void f() {\n"
              "    enum { FourInEnumOne = 4 };\n"
              "    enum { FourInEnumTwo = 4 };\n"
              "    static_assert(FourInEnumOne == FourInEnumTwo, "");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:6]: (style) Same expression on both sides of '&&'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:11]: (style) Same expression on both sides of '&&'.\n", errout.str());

        check("class D { void strcmp(); };\n"
              "void foo() {\n"
              "    D d;\n"
              "    if (d.strcmp() && d.strcmp()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if ((mystrcmp(a, b) == 0) || (mystrcmp(a, b) == 0)) {}\n"
              "}", "test.cpp", false, false, true, &settings, false);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void GetValue() { return rand(); }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void __attribute__((const)) GetValue() { return X; }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:3]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void GetValue() __attribute__((const));\n"
              "void GetValue() { return X; }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:4]: (style) Same expression on both sides of '||'.\n", errout.str());

        check("void foo() {\n"
              "    if (str == \"(\" || str == \"(\") {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '||'.\n", errout.str());

        // #5819
        check("Vector func(Vector vec1) {\n"
              "    return fabs(vec1 & vec1 & vec1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("Vector func(int vec1) {\n"
              "    return fabs(vec1 & vec1 & vec1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&'.\n", errout.str());

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
        Preprocessor::macroChar = '$';
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

    void duplicateExpressionTernary() { // #6391
        check("void f() {\n"
              "    return A ? x : x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression in both branches of ternary operator.\n", errout.str());

        check("void f() {\n"
              "    if( a ? (b ? false:false): false ) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same expression in both branches of ternary operator.\n", errout.str());

        check("void f() {\n"
              "    return A ? x : z;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void check_signOfUnsignedVariable(const char code[], bool inconclusive=false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkSignOfUnsignedVariable();
    }

    void checkSignOfUnsignedVariable() {
        check_signOfUnsignedVariable(
            "void foo() {\n"
            "  for(unsigned char i = 10; i >= 0; i--)"
            "    printf(\"%u\", i);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'i' can't be negative so it is unnecessary to test it.\n", errout.str());

        check_signOfUnsignedVariable(
            "void foo(bool b) {\n"
            "  for(unsigned int i = 10; b || i >= 0; i--)"
            "    printf(\"%u\", i);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'i' can't be negative so it is unnecessary to test it.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x) {\n"
            "  if (x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x) {\n"
            "  if (x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x) {\n"
            "  if (0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x) {\n"
            "  if (0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x) {\n"
            "  if (x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x) {\n"
            "  if (x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());


        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x < 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (x < 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (0 > x && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (0 > x && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x >= 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (x >= 0 && y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());


        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (y && x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (y && x < 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (y && 0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (y && 0 > x)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (y && x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (y && x >= 0)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());


        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x < 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (x < 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (0 > x || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is less than zero.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (0 > x || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(unsigned int x, bool y) {\n"
            "  if (x >= 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unsigned variable 'x' can't be negative so it is unnecessary to test it.\n", errout.str());

        check_signOfUnsignedVariable(
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
            check_signOfUnsignedVariable(code, false);
            ASSERT_EQUALS("", errout.str());
            check_signOfUnsignedVariable(code, true);
            ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Checking if unsigned variable 'x' is less than zero. This might be a false warning.\n", errout.str());
        }
    }

    void checkSignOfPointer() {
        check_signOfUnsignedVariable(
            "bool foo(int* x) {\n"
            "  if (x >= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x) {\n"
            "  if (*x >= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x) {\n"
            "  if (x < 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x) {\n"
            "  if (*x < 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y < 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y <= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y > 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x, int* y) {\n"
            "  if (x - y >= 0)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(Bar* x) {\n"
            "  if (0 <= x)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first) {\n"
            "  if (first.ptr >= 0) {} \n"
            "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if((first.ptr - second.ptr) >= 0) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first) {\n"
            "  if((first.ptr) >= 0) {} \n"
            "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not.\n", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 <= first.ptr - second.ptr) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 <= (first.ptr - second.ptr)) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(first.ptr - second.ptr < 0) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if((first.ptr - second.ptr) < 0) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 > first.ptr - second.ptr) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "struct S {\n"
            "  int* ptr;\n"
            "};\n"
            "void foo(S* first, S* second) {\n"
            "  if(0 > (first.ptr - second.ptr)) {} \n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x) {\n"
            "  if (0 <= x[0])"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(Bar* x) {\n"
            "  if (0 <= x.y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(Bar* x) {\n"
            "  if (0 <= x->y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(Bar* x, Bar* y) {\n"
            "  if (0 <= x->y - y->y )"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(Bar* x) {\n"
            "  if (0 > x)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int* x) {\n"
            "  if (0 > x[0])"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(Bar* x) {\n"
            "  if (0 > x.y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(Bar* x) {\n"
            "  if (0 > x->y)"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "void foo() {\n"
            "  int (*t)(void *a, void *b);\n"
            "  if (t(a, b) < 0)\n"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "void foo() {\n"
            "  int (*t)(void *a, void *b);\n"
            "  if (0 > t(a, b))\n"
            "    bar();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check_signOfUnsignedVariable(
            "struct object_info { int *typep; };\n"
            "void packed_object_info(struct object_info *oi) {\n"
            "  if (oi->typep < 0);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check_signOfUnsignedVariable(
            "struct object_info { int typep[10]; };\n"
            "void packed_object_info(struct object_info *oi) {\n"
            "  if (oi->typep < 0);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) A pointer can not be negative so it is either pointless or an error to check if it is.\n", errout.str());

        check_signOfUnsignedVariable(
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

    void check_redundant_copy(const char code[], bool inconclusive = true) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("performance");
        settings.inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Simplify token list..
        CheckOther checkOther(&tokenizer, &settings, this);
        tokenizer.simplifyTokenList2();
        checkOther.checkRedundantCopy();
    }
    void checkRedundantCopy() {
        check_redundant_copy("const std::string& getA(){static std::string a;return a;}\n"
                             "void foo() {\n"
                             "    const std::string a = getA();\n"
                             "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying.\n", errout.str());

        check_redundant_copy("class A{public:A(){}};\n"
                             "const A& getA(){static A a;return a;}\n"
                             "int main()\n"
                             "{\n"
                             "    const A a = getA();\n"
                             "    return 0;\n"
                             "}");
        ASSERT_EQUALS("[test.cpp:5]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying.\n", errout.str());

        check_redundant_copy("const int& getA(){static int a;return a;}\n"
                             "int main()\n"
                             "{\n"
                             "    const int a = getA();\n"
                             "    return 0;\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        check_redundant_copy("const int& getA(){static int a;return a;}\n"
                             "int main()\n"
                             "{\n"
                             "    int getA = 0;\n"
                             "    const int a = getA + 3;\n"
                             "    return 0;\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        check_redundant_copy("class A{public:A(){}};\n"
                             "const A& getA(){static A a;return a;}\n"
                             "int main()\n"
                             "{\n"
                             "    const A a(getA());\n"
                             "    return 0;\n"
                             "}");
        ASSERT_EQUALS("[test.cpp:5]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying.\n", errout.str());

        check_redundant_copy("const int& getA(){static int a;return a;}\n"
                             "int main()\n"
                             "{\n"
                             "    const int a(getA());\n"
                             "    return 0;\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        check_redundant_copy("class A{\n"
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

        check_redundant_copy("class A{\n"
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
        check_redundant_copy("class A {};\n"
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
        check_redundant_copy(code5618);
        TODO_ASSERT_EQUALS("", "[test.cpp:7]: (performance, inconclusive) Use const reference for 'temp' to avoid unnecessary data copying.\n", errout.str());
        check_redundant_copy(code5618, false);
        ASSERT_EQUALS("", errout.str());

        // #5890 - crash: wesnoth desktop_util.cpp / unicode.hpp
        check_redundant_copy("typedef std::vector<char> X;\n"
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

        check("x = y ? z << $-1 : 0;\n");
        ASSERT_EQUALS("", errout.str());
    }

    void incompleteArrayFill() {
        check("void f() {\n"
              "    int a[5];\n"
              "    memset(a, 123, 5);\n"
              "    memcpy(a, b, 5);\n"
              "    memmove(a, b, 5);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n"
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
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'?\n", "", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (performance) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        {
            // non-local variable => only show warning when inconclusive is used
            const char code[] = "int i;\n"
                                "void f() {\n"
                                "    i = 1;\n"
                                "    i = 1;\n"
                                "}";
            check(code, "test.cpp", false, false); // inconclusive = false
            ASSERT_EQUALS("", errout.str());
            check(code, "test.cpp", false, true); // inconclusive = true
            ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance, inconclusive) Variable 'i' is reassigned a value before the old one has been used if variable is no semaphore variable.\n", errout.str());
        }

        check("void f() {\n"
              "    int i;\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        check("void f() {\n"
              "    static int i;\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance, inconclusive) Variable 'i' is reassigned a value before the old one has been used if variable is no semaphore variable.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance, inconclusive) Variable 'bar' is reassigned a value before the old one has been used if variable is no semaphore variable.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (performance) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

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
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        check("void bar(int i) {}\n"
              "void f(int i) {\n"
              "    i = 1;\n"
              "    bar(i);\n" // Passed as argument
              "    i = 1;\n"
              "}", nullptr, false, false, false);
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
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:5]: (performance) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:6]: (performance) Variable 'x' is reassigned a value before the old one has been used.\n", errout.str());

        check("void f() {\n"
              "    Foo& bar = foo();\n"
              "    bar = x;\n"
              "    bar = y();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class C {\n"
              "    int x;\n"
              "    void g() { return x*x; }\n"
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
              "    void f();\n"
              "};\n"
              "\n"
              "void C::f(Foo z) {\n"
              "    x = 2;\n"
              "    x = z.g();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:9]: (performance, inconclusive) Variable 'x' is reassigned a value before the old one has been used if variable is no semaphore variable.\n", errout.str());

        // from #3103 (avoid a false negative)
        check("int foo(){\n"
              "    int x;\n"
              "    x = 1;\n"
              "    x = 1;\n"
              "    return x + 1;\n"
              "}", nullptr, false, false, false);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Variable 'x' is reassigned a value before the old one has been used.\n", errout.str());

        // from #3103 (avoid a false positive)
        check("int foo(){\n"
              "    int x;\n"
              "    x = 1;\n"
              "    if (y)\n" // <-- cppcheck does not know anything about 'y'
              "        x = 2;\n"
              "    return x + 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"  // Ticket #4356
              "    int x = 0;\n"  // <- ignore assignment with 0
              "    x = 3;\n"
              "}", 0, false, false, false);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int i = 54;\n"
              "    i = 0;\n"
              "}", 0, false, false, false);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (performance) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

        check("void f() {\n"
              "    int i = 54;\n"
              "    i = 1;\n"
              "}", 0, false, false, false);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (performance) Variable 'i' is reassigned a value before the old one has been used.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:6]: (performance, inconclusive) Variable 'a' is reassigned a value before the old one has been used if variable is no semaphore variable.\n", errout.str());

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
    }

    void redundantMemWrite() {
        // Simple tests
        check("void f(void* a) {\n"
              "    memcpy(a, foo, bar);\n"
              "    memset(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        check("void* a;\n"
              "void f() {\n"
              "    strcpy(a, foo);\n"
              "    strncpy(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        check("void f() {\n"
              "    void* a = foo();\n"
              "    sprintf(a, foo);\n"
              "    memmove(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

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
        check("void f(void* a) {\n"
              "    strcpy(a, foo);\n"
              "    strcat(a, bar);\n" // Not redundant
              "    strcpy(a, x);\n" // Redundant
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        // Tests with function call between copy
        check("void f(void* a) {\n"
              "    snprintf(a, foo, bar);\n"
              "    bar();\n"
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

        check("void* a;\n"
              "void f() {\n"
              "    memset(a, 0, size);\n"
              "    bar();\n" // Global variable might be accessed in bar()
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    void* a = foo();\n"
              "    memset(a, 0, size);\n"
              "    bar();\n"
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout.str());

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
        check("voif f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = getc (pFile);\n"
              "} while (c != EOF)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing getc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("voif f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = getc (pFile);\n"
              "} while (EOF != c)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing getc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("voif f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = getc (pFile);\n"
              "} while (i != EOF)"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("voif f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = getc (pFile);\n"
              "} while (EOF != i)"
              "}");
        ASSERT_EQUALS("", errout.str());


        // check fgetc
        check("voif f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (c != EOF)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing fgetc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("voif f (FILE * pFile){\n"
              "char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (EOF != c)"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Storing fgetc() return value in char variable and then comparing with EOF.\n", errout.str());

        check("voif f (FILE * pFile){\n"
              "signed char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (EOF != c)"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("voif f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = fgetc (pFile);\n"
              "} while (i != EOF)"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("voif f (FILE * pFile){\n"
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

        // ticket #4927 Segfault in CheckOther::checkCommaSeparatedReturn() on invalid code
        check("int main() {\n"
              "   return 0\n"
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

    void checkComparisonFunctionIsAlwaysTrueOrFalse() {
        // positive test
        check("bool f(int x){\n"
              "   return isless(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with isless(x,x) evaluates always to false.\n", errout.str());

        check("bool f(int x){\n"
              "   return isgreater(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with isgreater(x,x) evaluates always to false.\n", errout.str());

        check("bool f(int x){\n"
              "   return islessgreater(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with islessgreater(x,x) evaluates always to false.\n", errout.str());

        check("bool f(int x){\n"
              "   return islessequal(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with islessequal(x,x) evaluates always to true.\n", errout.str());

        check("bool f(int x){\n"
              "   return isgreaterequal(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of two identical variables with isgreaterequal(x,x) evaluates always to true.\n", errout.str());

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

    void checkIgnoredReturnValue() {
        Settings settings;
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"mystrcmp\">\n"
                               "    <use-retval/>\n"
                               "    <arg nr=\"1\"/>\n"
                               "    <arg nr=\"2\"/>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        settings.library.load(doc);

        check("void foo() {\n"
              "  mystrcmp(a, b);\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("[test.cpp:2]: (warning) Return value of function mystrcmp() is not used.\n", errout.str());

        check("bool mystrcmp(char* a, char* b);\n" // cppcheck sees a custom strcmp definition, but it returns a value. Assume it is the one specified in the library.
              "void foo() {\n"
              "    mystrcmp(a, b);\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("[test.cpp:3]: (warning) Return value of function mystrcmp() is not used.\n", errout.str());

        check("void mystrcmp(char* a, char* b);\n" // cppcheck sees a custom strcmp definition which returns void!
              "void foo() {\n"
              "    mystrcmp(a, b);\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    class mystrcmp { mystrcmp() {} };\n" // strcmp is a constructor definition here
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    return mystrcmp(a, b);\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if(mystrcmp(a, b));\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    bool b = mystrcmp(a, b);\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("", errout.str());

        // #6194
        check("void foo() {\n"
              "    MyStrCmp mystrcmp(x, y);\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("", errout.str());

        // #6197
        check("void foo() {\n"
              "    abc::def.mystrcmp(a,b);\n"
              "}", "test.cpp", false, false, true, &settings);
        ASSERT_EQUALS("", errout.str());

        // #6233
        check("int main() {\n"
              "    auto lambda = [](double value) {\n"
              "        double rounded = floor(value + 0.5);\n"
              "        printf(\"Rounded value = %f\\n\", rounded);\n"
              "    };\n"
              "    lambda(13.3);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void redundantPointerOp() {
        check("int *f(int *x) {\n"
              "    return &*x;\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on x - it's already a pointer.\n", errout.str());

        check("int *f(int *y) {\n"
              "    return &(*y);\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on y - it's already a pointer.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on mut - it's already a pointer.\n", errout.str());

        // make sure we got the AST match for "(" right
        check("void f(char *ptr) {\n"
              "    if (&*ptr == NULL)\n"
              "        return;\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("[test.cpp:2]: (style) Redundant pointer operation on ptr - it's already a pointer.\n", errout.str());

        // no warning for macros
        check("#define MUTEX_LOCK(m) pthread_mutex_lock(&(m))\n"
              "void f(struct mutex *mut) {\n"
              "    MUTEX_LOCK(*mut);\n"
              "}\n", nullptr, false, true);
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestOther)
