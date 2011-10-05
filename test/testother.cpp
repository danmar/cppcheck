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

#include "preprocessor.h"
#include "tokenize.h"
#include "checkother.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestOther : public TestFixture
{
public:
    TestOther() : TestFixture("TestOther")
    { }

private:


    void run()
    {
        TEST_CASE(assignBoolToPointer);

        TEST_CASE(zeroDiv1);
        TEST_CASE(zeroDiv2);
        TEST_CASE(zeroDiv3);
        TEST_CASE(zeroDiv4);
        TEST_CASE(zeroDiv5);
        TEST_CASE(zeroDiv6);

        TEST_CASE(sprintf1);        // Dangerous usage of sprintf
        TEST_CASE(sprintf2);
        TEST_CASE(sprintf3);
        TEST_CASE(sprintf4);        // struct member

        TEST_CASE(strPlusChar1);     // "/usr" + '/'
        TEST_CASE(strPlusChar2);     // "/usr" + ch
        TEST_CASE(strPlusChar3);     // ok: path + "/sub" + '/'

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
        TEST_CASE(varScope12);      // variable usage in inner loop

        TEST_CASE(oldStylePointerCast);

        TEST_CASE(dangerousStrolUsage);

        TEST_CASE(passedByValue);

        TEST_CASE(mathfunctionCall1);

        TEST_CASE(fflushOnInputStreamTest);

        TEST_CASE(sizeofsizeof);
        TEST_CASE(sizeofCalculation);

        TEST_CASE(switchRedundantAssignmentTest);
        TEST_CASE(switchFallThroughCase);
        TEST_CASE(duplicateBreak);

        TEST_CASE(selfAssignment);
        TEST_CASE(testScanf1);
        TEST_CASE(testScanf2);

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
        TEST_CASE(trac2071);
        TEST_CASE(trac2084);

        TEST_CASE(assignmentInAssert);

        TEST_CASE(incorrectLogicOperator1);
        TEST_CASE(incorrectLogicOperator2);
        TEST_CASE(secondAlwaysTrueFalseWhenFirstTrueError);

        TEST_CASE(catchExceptionByValue);

        TEST_CASE(memsetZeroBytes);

        TEST_CASE(sizeofForArrayParameter);
        TEST_CASE(sizeofForNumericParameter);

        TEST_CASE(clarifyCalculation);

        TEST_CASE(clarifyCondition1);     // if (a = b() < 0)
        TEST_CASE(clarifyCondition2);     // if (a & b == c)
        TEST_CASE(clarifyCondition3);     // if (! a & b)
        TEST_CASE(clarifyCondition4);     // ticket #3110
        TEST_CASE(bitwiseOnBoolean);      // if (bool & bool)

        TEST_CASE(incorrectStringCompare);

        TEST_CASE(incrementBoolean);
        TEST_CASE(comparisonOfBoolWithInt1);
        TEST_CASE(comparisonOfBoolWithInt2);
        TEST_CASE(comparisonOfBoolWithInt3);
        TEST_CASE(comparisonOfBoolWithInt4);

        TEST_CASE(duplicateIf);
        TEST_CASE(duplicateBranch);
        TEST_CASE(duplicateExpression1);
        TEST_CASE(duplicateExpression2); // ticket #2730

        TEST_CASE(alwaysTrueFalseStringCompare);
        TEST_CASE(checkSignOfUnsignedVariable);
    }

    void check(const char code[], const char *filename = NULL)
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.inconclusive = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename ? filename : "test.cpp");

        // Check..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.sizeofsizeof();
        checkOther.sizeofCalculation();
        checkOther.checkRedundantAssignmentInSwitch();
        checkOther.checkAssignmentInAssert();
        checkOther.checkSizeofForArrayParameter();
        checkOther.checkSizeofForNumericParameter();
        checkOther.clarifyCondition();
        checkOther.checkDuplicateIf();
        checkOther.checkDuplicateBranch();
        checkOther.checkDuplicateExpression();
        checkOther.checkBitwiseOnBoolean();

        // Simplify token list..
        tokenizer.simplifyTokenList();

        checkOther.checkZeroDivision();
        checkOther.checkMathFunctions();
        checkOther.checkFflushOnInputStream();
        checkOther.checkSelfAssignment();
        checkOther.invalidScanf();
        checkOther.checkMisusedScopedObject();
        checkOther.checkIncorrectLogicOperator();
        checkOther.checkCatchExceptionByValue();
        checkOther.checkMemsetZeroBytes();
        checkOther.clarifyCalculation();
        checkOther.checkIncorrectStringCompare();
        checkOther.checkIncrementBoolean();
        checkOther.checkComparisonOfBoolWithInt();
        checkOther.checkDuplicateBreak();
        checkOther.checkAssignBoolToPointer();
    }

    class SimpleSuppressor: public ErrorLogger
    {
    public:
        SimpleSuppressor(Settings &settings, ErrorLogger *next)
            : _settings(settings), _next(next)
        { }
        virtual void reportOut(const std::string &outmsg)
        {
            _next->reportOut(outmsg);
        }
        virtual void reportErr(const ErrorLogger::ErrorMessage &msg)
        {
            if (!msg._callStack.empty() && !_settings.nomsg.isSuppressed(msg._id, msg._callStack.begin()->getfile(), msg._callStack.begin()->line))
                _next->reportErr(msg);
        }
    private:
        Settings &_settings;
        ErrorLogger *_next;
    };

    void check_preprocess_suppress(const char precode[], const char *filename = NULL)
    {
        // Clear the error buffer..
        errout.str("");

        if (filename == NULL)
            filename = "test.cpp";

        Settings settings;
        settings.addEnabled("style");
        settings.addEnabled("performance");
        settings.experimental = true;

        // Preprocess file..
        Preprocessor preprocessor(&settings, this);
        std::list<std::string> configurations;
        std::string filedata = "";
        std::istringstream fin(precode);
        preprocessor.preprocess(fin, filedata, configurations, filename, settings._includePaths);
        SimpleSuppressor logger(settings, this);
        const std::string code = Preprocessor::getcode(filedata, "", filename, &settings, &logger);

        // Tokenize..
        Tokenizer tokenizer(&settings, &logger);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyGoto();

        // Check..
        CheckOther checkOther(&tokenizer, &settings, &logger);
        checkOther.checkSwitchCaseFallThrough();
        checkOther.checkAlwaysTrueOrFalseStringCompare();

        logger.reportUnmatchedSuppressions(settings.nomsg.getUnmatchedLocalSuppressions(filename));
    }


    void assignBoolToPointer()
    {
        check("void foo(bool *p) {\n"
              "    p = false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Assigning bool value to pointer (converting bool value to address)\n", errout.str());
    }


    void zeroDiv1()
    {
        check("void foo()\n"
              "{\n"
              "    int a = 0;\n"
              "    double b = 1.;\n"
              "    cout<<b/a;\n"
              "}");


        ASSERT_EQUALS("[test.cpp:5]: (error) Division by zero\n", errout.str());
    }

    void zeroDiv2()
    {
        check("void foo()\n"
              "{\n"
              "    int sum = 0;\n"
              "    int n = 100;\n"
              "    for(int i = 0; i < n; i ++)\n"
              "    {\n"
              "        sum += i; \n"
              "    }\n"
              "    cout<<b/sum;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int a = 0 ? (2/0) : 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDiv3()
    {
        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero\n", errout.str());
    }

    void zeroDiv4()
    {
        check("void f()\n"
              "{\n"
              "   long a = b / 0x6;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0x0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero\n", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0L;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero\n", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0ul;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero\n", errout.str());

        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0L);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero\n", errout.str());

        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0x5);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Don't warn about floating points (gcc doesn't warn either)
        // and floating points are handled differently than integers.
        check("void f()\n"
              "{\n"
              "   long a = b / 0.0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0.5;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Don't warn about 0.0
        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0.0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "   div_t divresult = div (1,0.5);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void zeroDiv5()
    {
        check("void f()\n"
              "{ { {\n"
              "   long a = b / 0;\n"
              "} } }\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero\n", errout.str());
    }

    void zeroDiv6()
    {
        check("void f()\n"
              "{ { {\n"
              "   int a = b % 0;\n"
              "} } }\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Division by zero\n", errout.str());
    }


    void sprintfUsage(const char code[])
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        //tokenizer.tokens()->printOut( "tokens" );

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.invalidFunctionUsage();
    }

    void sprintf1()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%s\",buf);\n"
                     "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Undefined behavior: variable is used as parameter and destination in s[n]printf().\n", errout.str());
    }

    void sprintf2()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%i\",sizeof(buf));\n"
                     "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf3()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%i\",sizeof(buf));\n"
                     "    if (buf[0]);\n"
                     "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf4()
    {
        sprintfUsage("struct A\n"
                     "{\n"
                     "    char filename[128];\n"
                     "};\n"
                     "\n"
                     "void foo()\n"
                     "{\n"
                     "    const char* filename = \"hello\";\n"
                     "    struct A a;\n"
                     "    snprintf(a.filename, 128, \"%s\", filename);\n"
                     "}\n");
        ASSERT_EQUALS("", errout.str());
    }





    void strPlusChar(const char code[])
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.strPlusChar();
    }

    void strPlusChar1()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    const char *p = \"/usr\" + '/';\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Unusual pointer arithmetic\n", errout.str());
    }

    void strPlusChar2()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    char ch = 1;\n"
                    "    const char *p = ch + \"/usr\";\n"
                    "}\n");
        ASSERT_EQUALS("", errout.str());

        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    int i = 1;\n"
                    "    const char* psz = \"Bla\";\n"
                    "    const std::string str = i + psz;\n"
                    "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void strPlusChar3()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    std::string temp = \"/tmp\";\n"
                    "    std::string path = temp + '/' + \"sub\" + '/';\n"
                    "}\n");
        ASSERT_EQUALS("", errout.str());
    }



    void varScope(const char code[])
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("information");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkVariableScope();
    }

    void varScope1()
    {
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
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope2()
    {
        varScope("int foo()\n"
                 "{\n"
                 "    Error e;\n"
                 "    e.SetValue(12);\n"
                 "    throw e;\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope3()
    {
        varScope("void foo()\n"
                 "{\n"
                 "    int i;\n"
                 "    int *p = 0;\n"
                 "    if (abc)\n"
                 "    {\n"
                 "        p = &i;\n"
                 "    }\n"
                 "    *p = 1;\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope4()
    {
        varScope("void foo()\n"
                 "{\n"
                 "    int i;\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope5()
    {
        varScope("void f(int x)\n"
                 "{\n"
                 "    int i = 0;\n"
                 "    if (x) {\n"
                 "        for ( ; i < 10; ++i) ;\n"
                 "    }\n"
                 "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable 'i' can be reduced\n", errout.str());

        varScope("void f(int x)\n"
                 "{\n"
                 "    int i = 0;\n"
                 "    if (x) {b()}\n"
                 "    else {\n"
                 "        for ( ; i < 10; ++i) ;\n"
                 "    }\n"
                 "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable 'i' can be reduced\n", errout.str());
    }

    void varScope6()
    {
        varScope("void f(int x)\n"
                 "{\n"
                 "    int i = x;\n"
                 "    if (a) {\n"
                 "        x++;\n"
                 "    }\n"
                 "    if (b) {\n"
                 "        c(i);\n"
                 "    }\n"
                 "}\n");
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
                 "}\n");
        ASSERT_EQUALS("", errout.str());

        varScope("void f(int &x)\n"
                 "{\n"
                 "  int n = 1;\n"
                 "  do\n"
                 "  {\n"
                 "    ++n;\n"
                 "    ++x;\n"
                 "  } while (x);\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope7()
    {
        varScope("void f(int x)\n"
                 "{\n"
                 "    int y = 0;\n"
                 "    b(y);\n"
                 "    if (x) {\n"
                 "        y++;\n"
                 "    }\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope8()
    {
        varScope("void test() {\n"
                 "    float edgeResistance=1;\n"
                 "    std::vector<int> edges;\n"
                 "    BOOST_FOREACH(int edge, edges) {\n"
                 "        edgeResistance = (edge+1) / 2.0;\n"
                 "    }\n"
                 "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable 'edgeResistance' can be reduced\n", errout.str());
    }

    void varScope9()
    {
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
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope10()
    {
        // classes may have extra side effects
        varScope("int f()\n"
                 "{\n"
                 "    int x = 0;\n"
                 "    FOR {\n"
                 "        foo(x++);\n"
                 "    }\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope11()
    {
        varScope("int f() {\n"
                 "    int x = 0;\n"
                 "    AB ab = { x, 0 };\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());

        varScope("int f() {\n"
                 "    int x = 0;\n"
                 "    if (a == 0) { ++x; }\n"
                 "    AB ab = { x, 0 };\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());

        varScope("int f() {\n"
                 "    int x = 0;\n"
                 "    if (a == 0) { ++x; }\n"
                 "    if (a == 1) { AB ab = { x, 0 }; }\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope12()
    {
        // #2770
        varScope("void f() {\n"
                 "    int i = 0;\n"
                 "    forever {\n"
                 "        if (i++ == 42) { break; }\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkOldStylePointerCast(const char code[])
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizerCpp(&settings, this);
        std::istringstream istr(code);
        tokenizerCpp.tokenize(istr, "test.cpp");
        tokenizerCpp.setVarId();

        Tokenizer tokenizerC(&settings, this);
        std::istringstream istr2(code);
        tokenizerC.tokenize(istr2, "test.c");
        tokenizerC.setVarId();

        CheckOther checkOtherCpp(&tokenizerCpp, &settings, this);
        checkOtherCpp.warningOldStylePointerCast();

        CheckOther checkOtherC(&tokenizerC, &settings, this);
        checkOtherC.warningOldStylePointerCast();
    }

    void oldStylePointerCast()
    {
        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (Base *) derived;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) derived;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) ( new Derived() );\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) new Derived();\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) new short[10];\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(B *) const = 0;\n"
                                 "}\n");
        ASSERT_EQUALS("", errout.str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(const B *) const = 0;\n"
                                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void dangerousStrolUsage()
    {
        {
            sprintfUsage("int f(const char *num)\n"
                         "{\n"
                         "    return strtol(num, NULL, 1);\n"
                         "}\n");

            ASSERT_EQUALS("[test.cpp:3]: (error) Invalid radix in call to strtol or strtoul. Must be 0 or 2-36\n", errout.str());
        }

        {
            sprintfUsage("int f(const char *num)\n"
                         "{\n"
                         "    return strtol(num, NULL, 10);\n"
                         "}\n");

            ASSERT_EQUALS("", errout.str());
        }
    }

    void testPassedByValue(const char code[])
    {
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

    void passedByValue()
    {
        testPassedByValue("void f(const std::string str) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' should be passed by reference.\n", errout.str());

        testPassedByValue("class Foo;\nvoid f(const Foo foo) {}");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'foo' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::string &str) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::vector<int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

        testPassedByValue("void f(const std::vector<std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' should be passed by reference.\n", errout.str());

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

    }

    void mathfunctionCall1()
    {
        // log|log10
        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-2) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value -2 to log() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1.) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value -1. to log() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value -1.0 to log() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-0.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value -0.1 to log() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value 0 to log() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(0.) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value 0. to log() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(0.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value 0.0 to log() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(1.0E+3) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(1.0E-3) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(1E-3) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::string *log(0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(2.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        // acos
        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(1)        << std::endl;\n"
              "    std::cout <<  acos(-1)       << std::endl;\n"
              "    std::cout <<  acos(0.1)      << std::endl;\n"
              "    std::cout <<  acos(0.0001)   << std::endl;\n"
              "    std::cout <<  acos(0.01)     << std::endl;\n"
              "    std::cout <<  acos(1.0E-1)   << std::endl;\n"
              "    std::cout <<  acos(-1.0E-1)  << std::endl;\n"
              "    std::cout <<  acos(+1.0E-1)  << std::endl;\n"
              "    std::cout <<  acos(0.1E-1)   << std::endl;\n"
              "    std::cout <<  acos(+0.1E-1)  << std::endl;\n"
              "    std::cout <<  acos(-0.1E-1)  << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value 1.1 to acos() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value -1.1 to acos() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(-110) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value -110 to acos() leads to undefined result\n", errout.str());


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
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  atan2(0,0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value 0 and 0 to atan2() leads to undefined result\n", errout.str());


        // fmod
        check("void foo()\n"
              "{\n"
              "    std::cout <<  fmod(1.0,0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value 1.0 and 0 to fmod() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  fmod(1.0,1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        // pow
        check("void foo()\n"
              "{\n"
              "    std::cout <<  pow(0,-10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value 0 and -10 to pow() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  pow(0,10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // sqrt
        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(-1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Passing value -1 to sqrt() leads to undefined result\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


    }

    void fflushOnInputStreamTest()
    {
        check("void foo()\n"
              "{\n"
              "    fflush(stdin);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) fflush() called on input stream \"stdin\" may result in undefined behaviour\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    fflush(stdout);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sizeofsizeof()
    {
        check("void foo()\n"
              "{\n"
              "    int i = sizeof sizeof char;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Calling sizeof for 'sizeof'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int i = sizeof (sizeof long);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Calling sizeof for 'sizeof'.\n", errout.str());

        check("void foo(long *p)\n"
              "{\n"
              "    int i = sizeof (sizeof (p));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Calling sizeof for 'sizeof'.\n", errout.str());
    }

    void sizeofCalculation()
    {
        check("sizeof(a+b)");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Found calculation inside sizeof()\n", errout.str());

        check("sizeof(-a)");
        ASSERT_EQUALS("", errout.str());

        check("sizeof(void * const)");
        ASSERT_EQUALS("", errout.str());
    }

    void switchRedundantAssignmentTest()
    {
        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (a)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Redundant assignment of \"y\" in switch\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (a)\n"
              "        {\n"
              "        case 2:\n"
              "          {\n"
              "            y = 2;\n"
              "          }\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Redundant assignment of \"y\" in switch\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (a)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "        case 3:\n"
              "            if (x)\n"
              "            {\n"
              "                y = 3;\n"
              "            }\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (a)\n"
              "        {\n"
              "        case 2:\n"
              "          {\n"
              "            y = 2;\n"
              "            if (y)\n"
              "                printf(\"%d\", y);\n"
              "          }\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int x = a;\n"
              "        int y = 1;\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            x = 2;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "          {\n"
              "            int y = 2;\n"
              "          }\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "            break;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
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
              "    }\n"
              "}\n");
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
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "            printf(\"%d\", y);\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int y = 1;\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "            bar();\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void switchFallThroughCase()
    {
        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            break;\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}\n");
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
            "}\n");
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
            "}\n");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Switch falls through case without comment\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "        default:\n"
            "            break;\n"
            "    }\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Switch falls through case without comment\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "        case 1:\n"
            "            g();\n"
            "            // fall through\n"
            "        case 2:\n"
            "            break;\n"
            "    }\n"
            "}\n");
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
            "}\n");
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
            "}\n");
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
            "}\n");
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
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (style) Switch falls through case without comment\n", errout.str());

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
            "}\n");
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
            "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) Switch falls through case without comment\n", errout.str());

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
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (style) Switch falls through case without comment\n", errout.str());

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
            "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) Switch falls through case without comment\n", errout.str());

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
            "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Switch falls through case without comment\n", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    switch (a) {\n"
            "            int x;\n"
            "        case 1:\n"
            "            break;\n"
            "    }\n"
            "}\n");
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
            "}\n");
        // This fails because the switch parsing code currently doesn't understand
        // that all paths after g() actually return. It's a pretty unusual case
        // (no pun intended).
        TODO_ASSERT_EQUALS("",
                           "[test.cpp:11]: (style) Switch falls through case without comment\n", errout.str());

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
            "}\n");
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
            "}\n");
        // This fails because Tokenizer::simplifyGoto() copies the "leave:" block
        // into where the goto is, but because it contains a "return", it omits
        // copying a final return after the block.
        TODO_ASSERT_EQUALS("",
                           "[test.cpp:5]: (style) Switch falls through case without comment\n", errout.str());

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
            "}\n");
        ASSERT_EQUALS("", errout.str());

        check_preprocess_suppress(
            "void foo() {\n"
            "    // unrelated comment saying 'fall through'\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateBreak()
    {
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
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) Consecutive break or continue statements are unnecessary\n", errout.str());

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
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        while(1) {\n"
              "          if (a++ >= 100) {\n"
              "            break;\n"
              "            break;\n"
              "          }\n"
              "       }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Consecutive break or continue statements are unnecessary\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        while(1) {\n"
              "          if (a++ >= 100) {\n"
              "            continue;\n"
              "            continue;\n"
              "          }\n"
              "          a+=2;\n"
              "       }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Consecutive break or continue statements are unnecessary\n", errout.str());

        check("void foo(int a)\n"
              "{\n"
              "        while(1) {\n"
              "          if (a++ >= 100) {\n"
              "            continue;\n"
              "          }\n"
              "          a+=2;\n"
              "       }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void selfAssignment()
    {
        check("void foo()\n"
              "{\n"
              "        int x = 1;\n"
              "        x = x;\n"
              "        return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Redundant assignment of \"x\" to itself\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        int x = x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of \"x\" to itself\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        std::string var = var = \"test\";\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of \"var\" to itself\n", "", errout.str());

        check("void foo()\n"
              "{\n"
              "        int x = 1;\n"
              "        x = x + 1;\n"
              "        return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "        int *x = getx();\n"
              "        *x = x;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // non-primitive type -> there might be some side effects
        check("void foo()\n"
              "{\n"
              "        Fred fred; fred = fred;\n"
              "}\n");
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
    }

    void testScanf1()
    {
        check("#include <stdio.h>\n"
              "int main(int argc, char **argv)\n"
              "{\n"
              "    int a, b;\n"
              "    FILE *file = fopen(\"test\", \"r\");\n"
              "    b = fscanf(file, \"aa %ds\", &a);\n"
              "    c = scanf(\"aa %ds\", &a);\n"
              "    b = fscanf(file, \"aa%%ds\", &a);\n"
              "    fclose(file);\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) scanf without field width limits can crash with huge input data\n"
                      "[test.cpp:7]: (warning) scanf without field width limits can crash with huge input data\n", errout.str());
    }

    void testScanf2()
    {
        check("#include <stdio.h>\n"
              "int main(int argc, char **argv)\n"
              "{\n"
              "    int a, b;\n"
              "    FILE *file = fopen(\"test\", \"r\");\n"
              "    b = fscanf(file, \"aa%%%ds\", &a);\n"
              "    c = scanf(\"aa %%%ds\", &a);\n"
              "    b = fscanf(file, \"aa%%ds\", &a);\n"
              "    fclose(file);\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) scanf without field width limits can crash with huge input data\n"
                      "[test.cpp:7]: (warning) scanf without field width limits can crash with huge input data\n", errout.str());
    }

    void trac1132()
    {
        std::istringstream code("#include <iostream>\n"
                                "class Lock\n"
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

        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
        tokenizer.tokenize(code, "trac1132.cpp");
        tokenizer.simplifyTokenList();

        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkMisusedScopedObject();

        ASSERT_EQUALS("[trac1132.cpp:16]: (error) instance of \"Lock\" object destroyed immediately\n", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickFunction1()
    {
        check("int main ( )\n"
              "{\n"
              "    CouldBeFunction ( 123 ) ;\n"
              "    return 0 ;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickFunction2()
    {
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
              "    }\n"
              "};\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectPicksClass()
    {
        check("class NotAFunction ;\n"
              "int function ( )\n"
              "{\n"
              "    NotAFunction ( 123 );\n"
              "    return 0 ;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:4]: (error) instance of \"NotAFunction\" object destroyed immediately\n", errout.str());
    }

    void testMisusedScopeObjectPicksStruct()
    {
        check("struct NotAClass;\n"
              "bool func ( )\n"
              "{\n"
              "    NotAClass ( 123 ) ;\n"
              "    return true ;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:4]: (error) instance of \"NotAClass\" object destroyed immediately\n", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickIf()
    {
        check("bool func( int a , int b , int c )\n"
              "{\n"
              "    if ( a > b ) return c == a ;\n"
              "    return b == a ;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickConstructorDeclaration()
    {
        check("class Something : public SomthingElse\n"
              "{\n"
              "public:\n"
              "~Something ( ) ;\n"
              "Something ( ) ;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickFunctor()
    {
        check("\n"
              "#include <algorithm>\n"
              "\n"
              "class IncrementFunctor\n"
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

    void testMisusedScopeObjectDoesNotPickLocalClassConstructors()
    {
        check("void f() {\n"
              "    class Foo {\n"
              "        Foo() { }\n"
              "        Foo(int a) { }\n"
              "        Foo(int a, int b) { }\n"
              "    };\n"
              "    Foo();\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:7]: (error) instance of \"Foo\" object destroyed immediately\n", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickUsedObject()
    {
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

    void testMisusedScopeObjectDoesNotPickPureC()
    {
        // Ticket #2352
        const char code[] = "struct cb_watch_bool {\n"
                            "    int a;\n"
                            "};\n"
                            "\n"
                            "void f()\n"
                            "{\n"
                            "    cb_watch_bool();\n"
                            "}\n";

        check(code, "test.cpp");
        ASSERT_EQUALS("[test.cpp:7]: (error) instance of \"cb_watch_bool\" object destroyed immediately\n", errout.str());

        check(code, "test.c");
        ASSERT_EQUALS("", errout.str());

        // Ticket #2639
        check("struct stat { int a; int b; };\n"
              "void stat(const char *fn, struct stat *);\n"
              "\n"
              "void foo() {\n"
              "    stat(\"file.txt\", &st);\n"
              "}\n");
        ASSERT_EQUALS("",errout.str());
    }

    void testMisusedScopeObjectDoesNotPickNestedClass()
    {
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
                            "}\n";

        check(code, "test.cpp");
        ASSERT_EQUALS("", errout.str());
    }

    void trac2084()
    {
        check("#include <signal.h>\n"
              "\n"
              "void f()\n"
              "{\n"
              "    struct sigaction sa;\n"
              "\n"
              "    { sigaction(SIGHUP, &sa, 0); };\n"
              "    { sigaction(SIGINT, &sa, 0); };\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void trac2071()
    {
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

    void assignmentInAssert()
    {
        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a = 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a == 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    int b = 0;\n"
              "    assert(a == 2 && b = 1);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:4]: (warning) Assert statement modifies 'b'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a += 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a *= 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a -= 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a --);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());
    }

    void incorrectLogicOperator1()
    {
        check("void f(int x) {\n"
              "    if ((x != 1) || (x != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x != 1 || x != 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (1 != x || 3 != x)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

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
              "    const int ERR1 = 5;\n"
              "    const int ERR2 = 6;\n"
              "    if ((x != ERR1) || (x != ERR2))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:4]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f(int x, int y) {\n"
              "    const int ERR1 = 5;\n"
              "    if ((x != ERR1) || (y != ERR1))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

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

    void incorrectLogicOperator2()
    {
        check("void f(int x) {\n"
              "    if ((x == 1) && (x == 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x == 1.0) && (x == 3.0))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(float x) {\n"
              "    if ((x == 1) && (x == 1.0))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x == 1) && (x == 0x00000001))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x == 1 && x == 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x == 1.0 && x == 3.0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(float x) {\n"
              "    if (x == 1 && x == 1.0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1.0 && x > 1.0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 1.0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(float x) {\n"
              "    if (x < 1.0 && x > 3.0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (1 > x && 3 < x)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression always evaluates to false. Did you intend to use || instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x < 3 && x > 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x < 10)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x <= 10)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x < 10)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x <= 10)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x < 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x <= 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x < 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x > 3 || x <= 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x > 10 || x < 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void secondAlwaysTrueFalseWhenFirstTrueError()
    {
        check("void f(int x) {\n"
              "    if (x > 5 && x != 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x != 1 is always true.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x != 1 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x > 5) && (x != 6))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (5 < x && x != 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x != 1 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if (5 < x && x != 6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((5 < x) && (x != 1))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x != 1 is always true.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((5 < x) && (x != 6))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (x > 5 && x == 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x == 1 is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x > 5 && x == 6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x > 5) && (x == 1))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x == 1 is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x > 5) && (x == 6))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (5 < x && x == 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x == 1 is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    if (5 < x && x == 6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((5 < x) && (x == 1))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (style) When x is greater than 5, the comparison x == 1 is always false.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((5 < x) && (x == 6))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void catchExceptionByValue()
    {
        check("void f() {\n"
              "    try\n"
              "    {\n"
              "        foo();\n"
              "    }\n"
              "    catch( ::std::exception err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:6]: (style) Exception should be caught by reference.\n", errout.str());

        check("void f() {\n"
              "    try\n"
              "    {\n"
              "        foo();\n"
              "    }\n"
              "    catch(const exception err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:6]: (style) Exception should be caught by reference.\n", errout.str());

        check("void f() {\n"
              "    try\n"
              "    {\n"
              "        foo();\n"
              "    }\n"
              "    catch( ::std::exception& err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try\n"
              "    {\n"
              "        foo();\n"
              "    }\n"
              "    catch(exception* err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try\n"
              "    {\n"
              "        foo();\n"
              "    }\n"
              "    catch(const exception& err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try\n"
              "    {\n"
              "        foo();\n"
              "    }\n"
              "    catch(int err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try\n"
              "    {\n"
              "        foo();\n"
              "    }\n"
              "    catch(exception* const err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void memsetZeroBytes()
    {
        check("void f() {\n"
              "    memset(p, 10, 0)\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) memset() called to fill 0"
                      " bytes of \'p\'\n", errout.str());

        check("void f() {\n"
              "    memset(p, sizeof(p), 0)\n"
              "}\n"
             );
        TODO_ASSERT_EQUALS("[test.cpp:2]: (warning) memset() called to fill 0"
                           " bytes of \"p\". Second and third arguments might be inverted.\n",

                           "", errout.str());
    }

    void sizeofForArrayParameter()
    {
        check("void f() {\n"
              "    int a[10];\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    unsigned int a = 2;\n"
              "    unsigned int b = 2;\n"
              "    int c[(a+b)];\n"
              "    std::cout << sizeof(c) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    unsigned int a = { 2 };\n"
              "    unsigned int b[] = { 0 };\n"
              "    int c[a[b[0]]];\n"
              "    std::cout << sizeof(c) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());


        check("void f() {\n"
              "    unsigned int a[] = { 1 };\n"
              "    unsigned int b = 2;\n"
              "    int c[(a[0]+b)];\n"
              "    std::cout << sizeof(c) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a[] = { 1, 2, 3 };\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a[3] = { 1, 2, 3 };\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f( int a[]) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof for array given as "
                      "function argument returns the size of pointer.\n", errout.str());

        check("void f( int a[]) {\n"
              "    std::cout << sizeof a / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof for array given as "
                      "function argument returns the size of pointer.\n", errout.str());

        check("void f( int a[3] ) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof for array given as "
                      "function argument returns the size of pointer.\n", errout.str());

        check("void f(int *p) {\n"
              "    p[0] = 0;\n"
              "    sizeof(p);\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char p[] = \"test\";\n"
              "    sizeof(p);\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        // ticket #2495
        check("void f() {\n"
              "    static float col[][3]={\n"
              "      {1,0,0},\n"
              "      {0,0,1},\n"
              "      {0,1,0},\n"
              "      {1,0,1},\n"
              "      {1,0,1},\n"
              "      {1,0,1},\n"
              "    };\n"
              "    const int COL_MAX=sizeof(col)/sizeof(col[0]);\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        // ticket #155
        check("void f() {\n"
              "    char buff1[1024*64],buff2[sizeof(buff1)*2];\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        // ticket #2510
        check("void f( int a[], int b) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof for array given as "
                      "function argument returns the size of pointer.\n", errout.str());

        // ticket #2510
        check("void f( int a[3] , int b[2] ) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof for array given as "
                      "function argument returns the size of pointer.\n", errout.str());

        // ticket #2510
        check("void f() {\n"
              "    char buff1[1024*64],buff2[sizeof(buff1)*(2+1)];\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

    }

    void sizeofForNumericParameter()
    {
        check("void f() {\n"
              "    std::cout << sizeof(10) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof with a numeric constant as function argument might not be what you intended.\n", errout.str());

        check("void f() {\n"
              "    std::cout << sizeof(-10) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof with a numeric constant as function argument might not be what you intended.\n", errout.str());

        check("void f() {\n"
              "    std::cout << sizeof 10  << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof with a numeric constant as function argument might not be what you intended.\n", errout.str());

        check("void f() {\n"
              "    std::cout << sizeof -10  << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using sizeof with a numeric constant as function argument might not be what you intended.\n", errout.str());


    }

    void clarifyCalculation()
    {
        check("int f(char c) {\n"
              "    return 10 * (c == 0) ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for * and ?\n", errout.str());

        check("void f(char c) {\n"
              "    printf(\"%i\", 10 * (c == 0) ? 1 : 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for * and ?\n", errout.str());

        // Ticket #2585 - segmentation fault for invalid code
        check("abcdef?" "?<"
              "123456?" "?>"
              "+?" "?=");
        ASSERT_EQUALS("", errout.str());

        check("void f(char c) {\n"
              "    printf(\"%i\", 1 + 1 ? 1 : 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for + and ?\n", errout.str());

        check("void f() {\n"
              "    std::cout << x << 1 ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for << and ?\n", errout.str());

        check("void f() {\n"
              "    int ab = a - b ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for - and ?\n", errout.str());

        // ticket #195
        check("int f(int x, int y) {\n"
              "    return x >> ! y ? 8 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Clarify calculation precedence for >> and ?\n", errout.str());
    }

    // clarify conditions with = and comparison
    void clarifyCondition1()
    {
        check("void f() {\n"
              "    if (x = b() < 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (assignment+comparison), it can be clarified with parentheses\n", errout.str());

        check("void f(int i) {\n"
              "    for (i = 0; i < 10; i++) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (x = a<int>()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }

    // clarify conditions with bitwise operator and comparison
    void clarifyCondition2()
    {
        check("void f() {\n"
              "    if (x & 2 == 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (bitwise operator + comparison), it can be clarified with parentheses\n", errout.str());

        check("void f() {\n"
              "    if (a & fred1.x == fred2.y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious condition (bitwise operator + comparison), it can be clarified with parentheses\n", errout.str());
    }

    // clarify condition that uses ! operator and then bitwise operator
    void clarifyCondition3()
    {
        check("void f(int w) {\n"
              "    if(!w & 0x8000) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses\n", errout.str());

        check("void f() {\n"
              "    if (x == foo() & 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses\n", errout.str());

        check("void f(std::list<int> &ints) { }");
        ASSERT_EQUALS("", errout.str());

        check("void f() { A<x &> a; }");
        ASSERT_EQUALS("", errout.str());

        check("class A<B&,C>;", "test.C");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (result != (char *)&inline_result) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void clarifyCondition4() // ticket #3110
    {
        check("typedef double SomeType;\n"
              "typedef std::pair<std::string,SomeType> PairType;\n"
              "struct S\n"
              "{\n"
              "     bool operator()\n"
              "         ( PairType const & left\n"
              "         , PairType const & right) const\n"
              "     {\n"
              "         return (left.first < right.first);\n"
              "     }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void bitwiseOnBoolean() // 3062
    {
        check("void f(_Bool a, _Bool b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean && ?\n", errout.str());

        check("void f(_Bool a, _Bool b) {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean || ?\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean && ?\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if(a & !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean && ?\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean || ?\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if(a | !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean || ?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean && ?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a & !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean && ?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean || ?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a | !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Boolean variable 'a' is used in bitwise operation. Did you mean || ?\n", errout.str());
    }

    void incorrectStringCompare()
    {
        check("int f() {\n"
              "    return test.substr( 0 , 4 ) == \"Hello\" ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) String literal \"Hello\" doesn't match length argument for substr(4).\n", errout.str());

        check("int f() {\n"
              "    return test.substr( 0 , 5 ) == \"Hello\" ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              " return \"Hello\" == test.substr( 0 , 4 ) ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) String literal \"Hello\" doesn't match length argument for substr(4).\n", errout.str());

        check("int f() {\n"
              " return \"Hello\" == test.substr( 0 , 5 ) ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void incrementBoolean()
    {
        check("bool bValue = true;\n"
              "bValue++;\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) The use of a variable of type bool with the ++ postfix operator is always true and deprecated by the C++ Standard.\n", errout.str());

        check("_Bool bValue = true;\n"
              "bValue++;\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) The use of a variable of type bool with the ++ postfix operator is always true and deprecated by the C++ Standard.\n", errout.str());

        check("void f(bool test){\n"
              "    test++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The use of a variable of type bool with the ++ postfix operator is always true and deprecated by the C++ Standard.\n", errout.str());

        check("void f(int test){\n"
              "    test++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt1()
    {
        check("void f(bool x) {\n"
              "    if (x < 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(bool x) {\n"
              "    if (10 >= x) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(bool x) {\n"
              "    if (x != 0) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool x) {\n"
              "    if (x != 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(bool x) {\n"
              "    if (x == 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(bool x) {\n"
              "    if (x == 0) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt2()
    {
        check("void f(bool x, int y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(int x, bool y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(bool x, bool y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool x, fooClass y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt3()
    {
        check("void f(int y) {\n"
              "    if (y > false) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(int y) {\n"
              "    if (true == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(bool y) {\n"
              "    if (y == true) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt4()
    {
        check("void f(int x) {\n"
              "    if (!x == 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(int x) {\n"
              "    if (!x != 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(int x) {\n"
              "    if (x != 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (10 == !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(int x) {\n"
              "    if (10 != !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with a non-zero integer\n", errout.str());

        check("void f(int x) {\n"
              "    if (10 != x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateIf()
    {
        check("void f(int a, int &b) {\n"
              "    if (a) { b = 1; }\n"
              "    else if (a) { b = 2; }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (style) Found duplicate if expressions.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else if (a == 2) { b = 2; }\n"
              "    else if (a == 1) { b = 3; }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style) Found duplicate if expressions.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else if (a == 2) { b = 2; }\n"
              "    else if (a == 2) { b = 3; }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3]: (style) Found duplicate if expressions.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) {\n"
              "        b = 1;\n"
              "        if (b == 1) { }\n"
              "        else if (b == 1) { }\n"
              "    } else if (a == 2) { b = 2; }\n"
              "    else if (a == 2) { b = 3; }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6]: (style) Found duplicate if expressions.\n"
                      "[test.cpp:5] -> [test.cpp:4]: (style) Found duplicate if expressions.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a++) { b = 1; }\n"
              "    else if (a++) { b = 2; }\n"
              "    else if (a++) { b = 3; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (!strtok(NULL," ")) { b = 1; }\n"
              "    else if (!strtok(NULL," ")) { b = 2; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a, int &b) {\n"
              "   if ((x = x / 2) < 100) { b = 1; }\n"
              "   else if ((x = x / 2) < 100) { b = 2; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateBranch()
    {
        check("void f(int a, int &b) {\n"
              "    if (a)\n"
              "        b = 1;\n"
              "    else\n"
              "        b = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style) Found duplicate branches for if and else.\n", errout.str());

        check("void f(int a, int &b) {\n"
              "    if (a) {\n"
              "        if (a == 1)\n"
              "            b = 2;\n"
              "        else\n"
              "            b = 2;\n"
              "    } else\n"
              "        b = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) Found duplicate branches for if and else.\n", errout.str());

        check("int f(int signed, unsigned char value) {\n"
              "    int ret;\n"
              "    if (signed)\n"
              "        ret = (signed char)value;\n"
              "    else\n"
              "        ret = (unsigned char)value;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void duplicateExpression1()
    {
        check("voif foo() {\n"
              "    if (a == a) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '=='.\n", errout.str());

        check("void fun() {\n"
              "    return (a && a ||\n"
              "            b == b &&\n"
              "            c - c &&\n"
              "            d > d &&\n"
              "            e < e &&\n"
              "            f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:2]: (style) Same expression on both sides of '&&'.\n"
                      "[test.cpp:3] -> [test.cpp:3]: (style) Same expression on both sides of '=='.\n"
                      "[test.cpp:4] -> [test.cpp:4]: (style) Same expression on both sides of '-'.\n"
                      "[test.cpp:5] -> [test.cpp:5]: (style) Same expression on both sides of '>'.\n"
                      "[test.cpp:6] -> [test.cpp:6]: (style) Same expression on both sides of '<'.\n", errout.str());
    }

    void duplicateExpression2() // ticket #2730
    {
        check("int main()\n"
              "{\n"
              "    long double ldbl;\n"
              "    double dbl, in;\n"
              "    float  flt;\n"
              "    int have_nan = 0;\n"
              "    ldbl = sqrtl(-1.0);\n"
              "    dbl = sqrt(-1.0);\n"
              "    flt = sqrtf(-1.0);\n"
              "    if (ldbl != ldbl) have_nan = 1;\n"
              "    if (!(dbl == dbl)) have_nan = 1;\n"
              "    if (flt != flt) have_nan = 1;\n"
              "    return have_nan;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Passing value -1.0 to sqrtl() leads to undefined result\n"
                      "[test.cpp:8]: (error) Passing value -1.0 to sqrt() leads to undefined result\n"
                      "[test.cpp:9]: (error) Passing value -1.0 to sqrtf() leads to undefined result\n", errout.str());
    }

    void alwaysTrueFalseStringCompare()
    {
        check_preprocess_suppress(
            "#define MACRO \"00FF00\"\n"
            "int main()\n"
            "{\n"
            "  if (strcmp(MACRO,\"00FF00\") == 0)"
            "  {"
            "    std::cout << \"Equal\n\""
            "  }"
            "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Comparison of always identical static strings.\n", errout.str());

        check_preprocess_suppress(
            "int main()\n"
            "{\n"
            "  if (stricmp(\"hotdog\",\"HOTdog\") == 0)"
            "  {"
            "    std::cout << \"Equal\n\""
            "  }"
            "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Unnecessary comparison of static strings.\n", errout.str());

        check_preprocess_suppress(
            "#define MACRO \"Hotdog\"\n"
            "int main()\n"
            "{\n"
            "  if (QString::compare(\"Hamburger\", MACRO) == 0)"
            "  {"
            "    std::cout << \"Equal\n\""
            "  }"
            "}");
        ASSERT_EQUALS("[test.cpp:4]: (performance) Unnecessary comparison of static strings.\n", errout.str());

        check_preprocess_suppress(
            "int main()\n"
            "{\n"
            "  if (QString::compare(argv[2], \"hotdog\") == 0)"
            "  {"
            "    std::cout << \"Equal\n\""
            "  }"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void check_signOfUnsignedVariable(const char code[])
    {
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
        checkOther.checkSignOfUnsignedVariable();
    }

    void checkSignOfUnsignedVariable()
    {
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
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is positive is always true.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is positive is always true.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is positive is always true.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (style) Checking if unsigned variable 'x' is positive is always true.\n", errout.str());

        check_signOfUnsignedVariable(
            "bool foo(int x, bool y) {\n"
            "  if (x >= 0 || y)"
            "    return true;\n"
            "  return false;\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestOther)

