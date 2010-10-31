/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
        TEST_CASE(zeroDiv1);
        TEST_CASE(zeroDiv2);
        TEST_CASE(zeroDiv3);
        TEST_CASE(zeroDiv4);

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
        TEST_CASE(varScope9);		// classes may have extra side-effects
        TEST_CASE(varScope10);      // Undefined macro FOR

        TEST_CASE(oldStylePointerCast);

        TEST_CASE(dangerousStrolUsage);

        TEST_CASE(passedByValue);

        TEST_CASE(mathfunctionCall1);

        TEST_CASE(emptyStringTest);

        TEST_CASE(fflushOnInputStreamTest);

        TEST_CASE(sizeofsizeof);
        TEST_CASE(sizeofCalculation);

        TEST_CASE(switchRedundantAssignmentTest);

        TEST_CASE(selfAssignment);
        TEST_CASE(testScanf1);
        TEST_CASE(testScanf2);

        TEST_CASE(trac1132);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunction);
        TEST_CASE(testMisusedScopeObjectPicksClass);
        TEST_CASE(testMisusedScopeObjectPicksStruct);
        TEST_CASE(testMisusedScopeObjectDoesNotPickIf);
        TEST_CASE(testMisusedScopeObjectDoesNotPickConstructorDeclaration);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunctor);
        TEST_CASE(testMisusedScopeObjectDoesNotPickLocalClassConstructors);
        TEST_CASE(testMisusedScopeObjectDoesNotPickUsedObject);
        TEST_CASE(trac2071);
        TEST_CASE(trac2084);

        TEST_CASE(assignmentInAssert);
        TEST_CASE(incorrectLogicOperator);
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);

        // Clear the error buffer..
        errout.str("");

        checkOther.sizeofsizeof();
        checkOther.sizeofCalculation();
        checkOther.checkRedundantAssignmentInSwitch();
        checkOther.checkAssignmentInAssert();

        // Simplify token list..
        tokenizer.simplifyTokenList();

        checkOther.checkZeroDivision();
        checkOther.checkMathFunctions();
        checkOther.checkEmptyStringTest();
        checkOther.checkFflushOnInputStream();
        checkOther.checkSelfAssignment();
        checkOther.invalidScanf();
        checkOther.checkMisusedScopedObject();
        checkOther.checkIncorrectLogicOperator();
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


    void sprintfUsage(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        //tokenizer.tokens()->printOut( "tokens" );

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
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
        ASSERT_EQUALS("[test.cpp:4]: (error) Undefined behaviour: buf is used wrong in call to sprintf or snprintf. Quote: If copying takes place between objects that overlap as a result of a call to sprintf() or snprintf(), the results are undefined.\n", errout.str());
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
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
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
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        settings._checkCodingStyle = true;
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
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable i can be reduced\n", errout.str());

        varScope("void f(int x)\n"
                 "{\n"
                 "    int i = 0;\n"
                 "    if (x) {b()}\n"
                 "    else {\n"
                 "        for ( ; i < 10; ++i) ;\n"
                 "    }\n"
                 "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable i can be reduced\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:2]: (style) The scope of the variable edgeResistance can be reduced\n", errout.str());
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


    void checkOldStylePointerCast(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizerCpp;
        std::istringstream istr(code);
        tokenizerCpp.tokenize(istr, "test.cpp");
        tokenizerCpp.setVarId();

        Tokenizer tokenizerC;
        std::istringstream istr2(code);
        tokenizerC.tokenize(istr2, "test.c");
        tokenizerC.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        settings._checkCodingStyle = true;

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
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkConstantFunctionParameter();
    }

    void passedByValue()
    {
        testPassedByValue("void f(const std::string str) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'str' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());

        testPassedByValue("class Foo;\nvoid f(const Foo foo) {}");
        ASSERT_EQUALS("[test.cpp:2]: (performance) Function parameter 'foo' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());

        testPassedByValue("void f(const std::string &str) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::vector<int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());

        testPassedByValue("void f(const std::vector<std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());

        testPassedByValue("void f(const std::vector<int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::map<int,int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::map<int,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());

        testPassedByValue("void f(const std::map<std::string,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());

        testPassedByValue("void f(const std::map<int,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());

        testPassedByValue("void f(const std::map<std::string,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Function parameter 'v' is passed by value. It could be passed by reference instead, to make it faster.\n", errout.str());
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

    void emptyStringTest()
    {
        check("void foo()\n"
              "{\n"
              "    if (strlen(str) == 0)\n"
              "    {\n"
              "        std::cout << str;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (!strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (strlen(str) == 0) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (strlen(str) > 0) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (strlen(str) != 0) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (0 != strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (0 == strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (0 < strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious code 'sizeof sizeof ..', most likely there should only be one sizeof. The current code is equivalent to 'sizeof(size_t)'.\n", errout.str());
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
              "        return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of \"x\" to itself\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        std::string var = var = \"test\";\n"
              "        return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant assignment of \"var\" to itself\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        int x = 1;\n"
              "        x = x + 1;\n"
              "        return 0;\n"
              "}\n");
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
        errout.str("");

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

        Tokenizer tokenizer;
        tokenizer.tokenize(code, "trac1132.cpp");
        tokenizer.simplifyTokenList();

        Settings settings;

        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkMisusedScopedObject();

        ASSERT_EQUALS("[trac1132.cpp:16]: (error) instance of \"Lock\" object destroyed immediately\n", errout.str());
    }

    void testMisusedScopeObjectDoesNotPickFunction()
    {
        check("int main ( )\n"
              "{\n"
              "    CouldBeFunction ( 123 ) ;\n"
              "    return 0 ;\n"
              "}\n"
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
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'. If the modification is needed in release builds there is a bug.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (warning) Assert statement modifies 'b'. If the modification is needed in release builds there is a bug.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a += 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'. If the modification is needed in release builds there is a bug.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a *= 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'. If the modification is needed in release builds there is a bug.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a -= 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'. If the modification is needed in release builds there is a bug.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a --);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'. If the modification is needed in release builds there is a bug.\n", errout.str());
    }

    void incorrectLogicOperator()
    {
        check("void f() {\n"
              "    if ((x != 1) || (x != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f() {\n"
              "    if (x != 1 || x != 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f() {\n"
              "    if (1 != x || 3 != x)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Mutual exclusion over || always evaluates to true. Did you intend to use && instead?\n", errout.str());

        check("void f() {\n"
              "    if (x != 1 || y != 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if ((y == 1) && (x != 1) || (x != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if ((x != 1) || (x != 3) && (y == 1))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if ((x != 1) && (x != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if ((x == 1) || (x == 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if ((x != 1) || (y != 3))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestOther)

