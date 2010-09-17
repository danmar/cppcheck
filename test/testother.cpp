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

        TEST_CASE(nullpointer1);
        TEST_CASE(nullpointer2);
        TEST_CASE(nullpointer3);    // dereferencing struct and then checking if it's null
        TEST_CASE(nullpointer4);
        TEST_CASE(nullpointer5);    // References should not be checked
        TEST_CASE(nullpointer6);
        TEST_CASE(nullpointer7);
        TEST_CASE(nullpointer8);
        TEST_CASE(nullpointer9);

        TEST_CASE(uninitvar1);
        TEST_CASE(uninitvar_alloc);     // data is allocated but not initialized
        TEST_CASE(uninitvar_arrays);    // arrays
        TEST_CASE(uninitvar_class);     // class/struct
        TEST_CASE(uninitvar_enum);      // enum variables
        TEST_CASE(uninitvar_if);        // handling if
        TEST_CASE(uninitvar_loops);     // handling for/while
        TEST_CASE(uninitvar_switch);    // handling switch
        TEST_CASE(uninitvar_references); // references
        TEST_CASE(uninitvar_strncpy);   // strncpy doesn't always 0-terminate
        TEST_CASE(uninitvar_func);      // analyse functions

        TEST_CASE(oldStylePointerCast);

        TEST_CASE(postIncrementDecrementStl);
        TEST_CASE(postIncrementDecrementClass);

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

        // Simplify token list..
        tokenizer.simplifyTokenList();

        checkOther.checkZeroDivision();
        checkOther.checkMathFunctions();
        checkOther.checkEmptyStringTest();
        checkOther.checkFflushOnInputStream();
        checkOther.checkSelfAssignment();
        checkOther.invalidScanf();
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


    void checkNullPointer(const char code[])
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
        checkOther.nullPointer();

        tokenizer.simplifyTokenList();
        checkOther.nullConstantDereference();
        checkOther.executionPaths();
    }


    void nullpointer1()
    {
        checkNullPointer("int foo(const Token *tok)\n"
                         "{\n"
                         "    while (tok);\n"
                         "    tok = tok->next();\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: tok\n", errout.str());

        checkNullPointer("void foo()\n"
                         "{\n"
                         "    for (const Token *tok = tokens; tok; tok = tok->next())\n"
                         "    {\n"
                         "        while (tok && tok->str() != \";\")\n"
                         "            tok = tok->next();\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: tok - otherwise it is redundant to check if tok is null at line 5\n", errout.str());

        checkNullPointer("void foo(Token &tok)\n"
                         "{\n"
                         "    for (int i = 0; i < tok.size(); i++ )\n"
                         "    {\n"
                         "        while (!tok)\n"
                         "            char c = tok.read();\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo()\n"
                         "{\n"
                         "    for (const Token *tok = tokens; tok; tok = tok->next())\n"
                         "    {\n"
                         "        while (tok && tok->str() != \";\")\n"
                         "            tok = tok->next();\n"
                         "        if( !tok ) break;\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo()\n"
                         "{\n"
                         "    for (const Token *tok = tokens; tok; tok = tok ? tok->next() : NULL)\n"
                         "    {\n"
                         "        while (tok && tok->str() != \";\")\n"
                         "            tok = tok->next();\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(A*a)\n"
                         "{\n"
                         "  switch (a->b()) {\n"
                         "    case 1:\n"
                         "      while( a ){\n"
                         "        a = a->next;\n"
                         "      }\n"
                         "    break;\n"
                         "    case 2:\n"
                         "      a->b();\n"
                         "      break;\n"
                         "  }\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // ticket #1923 - no false positive when using else if
        checkNullPointer("void f(A *a)\n"
                         "{\n"
                         "    if (a->x == 1)\n"
                         "    {\n"
                         "        a = a->next;\n"
                         "    }\n"
                         "    else if (a->x == 2) { }\n"
                         "    if (a) { }\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void nullpointer2()
    {
        // Null pointer dereference can only happen with pointers
        checkNullPointer("void foo()\n"
                         "{\n"
                         "    Fred fred;\n"
                         "    while (fred);\n"
                         "    fred.hello();\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Dereferencing a struct and then checking if it is null
    // This is checked by this function:
    //        CheckOther::nullPointerStructByDeRefAndChec
    void nullpointer3()
    {
        // errors..
        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    int a = abc->a;\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 4\n", errout.str());

        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    bar(abc->a);\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 4\n", errout.str());

        // ok dereferencing in a condition
        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    if (abc && abc->a);\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // ok to use a linked list..
        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    abc = abc->next;\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // reassign struct..
        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    int a = abc->a;\n"
                         "    abc = abc->next;\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    int a = abc->a;\n"
                         "    f(&abc);\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // goto..
        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    int a;\n"
                         "    if (!abc)\n"
                         "        goto out;"
                         "    a = abc->a;\n"
                         "    return;\n"
                         "out:\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // loops..
        checkNullPointer("void freeAbc(struct ABC *abc)\n"
                         "{\n"
                         "    while (abc)\n"
                         "    {\n"
                         "        struct ABC *next = abc->next;\n"
                         "        if (abc) delete abc;\n"
                         "        abc = next;\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    int a = abc->a;"
                         "    do\n"
                         "    {\n"
                         "        if (abc)\n"
                         "            abc = abc->next;\n"
                         "        --a;\n"
                         "    }\n"
                         "    while (a > 0);\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void f()\n"
                         "{\n"
                         "    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())\n"
                         "    {\n"
                         "        while (tok && tok->str() != \"{\")\n"
                         "            tok = tok->next();\n"
                         "        if (!tok)\n"
                         "            return;\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // dynamic_cast..
        checkNullPointer("void foo(ABC *abc)\n"
                         "{\n"
                         "    int a = abc->a;\n"
                         "    if (!dynamic_cast<DEF *>(abc))\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Dereferencing a pointer and then checking if it is null
    void nullpointer4()
    {
        // errors..
        checkNullPointer("void foo(int *p)\n"
                         "{\n"
                         "    *p = 0;\n"
                         "    if (!p)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        checkNullPointer("void foo(int *p)\n"
                         "{\n"
                         "    bar(*p);\n"
                         "    if (!p)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        // no error
        checkNullPointer("void foo()\n"
                         "{\n"
                         "    int *p;\n"
                         "    f(&p);\n"
                         "    if (!p)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo()\n"
                         "{\n"
                         "    int **p = f();\n"
                         "    if (!p)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(int *p)\n"
                         "{\n"
                         "    if (x)\n"
                         "        p = 0;\n"
                         "    else\n"
                         "        *p = 0;\n"
                         "    if (!p)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(int x)\n"
                         "{\n"
                         "    int a = 2 * x;"
                         "    if (x == 0)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(int *p)\n"
                         "{\n"
                         "    int var1 = p ? *p : 0;\n"
                         "    if (!p)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(P *p)\n"
                         "{\n"
                         "  while (p)\n"
                         "    if (p->check())\n"
                         "      break;\n"
                         "    else\n"
                         "      p = p->next();\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void nullpointer5()
    {
        // errors..
        checkNullPointer("void foo(A &a)\n"
                         "{\n"
                         " char c = a.c();\n"
                         " if (!a)\n"
                         "   return;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Execution paths..
    void nullpointer6()
    {
        // errors..
        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    Foo *p = 0;\n"
                         "    if (a == 1)\n"
                         "        p = new FooBar;\n"
                         "    else if (a == 2)\n"
                         "        p = new FooCar;\n"
                         "    p->abcd();\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Possible null pointer dereference: p\n", errout.str());

        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    int *p = 0;\n"
                         "    int *q = p;\n"
                         "    q[0] = 0;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: q\n", errout.str());

        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    int *p = 0;\n"
                         "    int &r = *p;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        checkNullPointer("static void foo(int x)\n"
                         "{\n"
                         "    int *p = 0;\n"
                         "    int y = 5 + *p;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        checkNullPointer("static void foo(int x)\n"
                         "{\n"
                         "    Foo<int> *abc = 0;\n"
                         "    abc->a();\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: abc\n", errout.str());

        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    int *p(0);\n"
                         "    std::cout << *p;"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        checkNullPointer("void f()\n"
                         "{\n"
                         "    char *c = 0;\n"
                         "    {\n"
                         "        delete c;\n"
                         "    }\n"
                         "    c[0] = 0;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Possible null pointer dereference: c\n", errout.str());

        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    int *p = 0;\n"
                         "    if (3 > *p);\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        checkNullPointer("void f()\n"
                         "{\n"
                         "    if (x) {\n"
                         "        char *c = 0;\n"
                         "        *c = 0;\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Null pointer dereference\n", errout.str());

        // no false positive..
        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    Foo *p = 0;\n"
                         "    p = new Foo;\n"
                         "    p->abcd();\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    Foo *p = 0;\n"
                         "    if (!p)\n"
                         "        return;\n"
                         "    p->abcd();\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    int *p = 0;\n"
                         "    exit();\n"
                         "    *p = 0;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("static void foo(int a)\n"
                         "{\n"
                         "    Foo *p = 0;\n"
                         "    if (a && p)\n"
                         "        p->do_something();\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo()\n"
                         "{\n"
                         "    int sz = sizeof((*(struct dummy *)0).x);\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void get_offset(long &offset)\n"
                         "{\n"
                         "    mystruct * temp; temp = 0;\n"
                         "    offset = (long)(&(temp->z));\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #1893 - try/catch inside else
        checkNullPointer("int *test(int *Z)\n"
                         "{\n"
                         "    int *Q=NULL;\n"
                         "    if (Z) {\n"
                         "        Q = Z;\n"
                         "    }\n"
                         "    else {\n"
                         "        Z = new int;\n"
                         "        try {\n"
                         "        } catch(...) {\n"
                         "        }\n"
                         "        Q = Z;\n"
                         "    }\n"
                         "    *Q=1;\n"
                         "    return Q;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("int *test(int *Z)\n"
                         "{\n"
                         "    int *Q=NULL;\n"
                         "    if (Z) {\n"
                         "        Q = Z;\n"
                         "    }\n"
                         "    else {\n"
                         "        try {\n"
                         "        } catch(...) {\n"
                         "        }\n"
                         "    }\n"
                         "    *Q=1;\n"
                         "    return Q;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:12]: (error) Possible null pointer dereference: Q\n", errout.str());

        // function pointer..
        checkNullPointer("void foo()\n"
                         "{\n"
                         "    void (*f)();\n"
                         "    f = 0;\n"
                         "    f();\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: f\n", errout.str());

        checkNullPointer("static void foo()\n"
                         "{\n"
                         "    int *p = 0;\n"
                         "    int *p2 = 0;\n"
                         "    int r = *p;\n"
                         "    int r2 = *p2;\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Null pointer dereference\n"
                      "[test.cpp:6]: (error) Null pointer dereference\n", errout.str());

    }

    void nullpointer7()
    {
        checkNullPointer("void foo()\n"
                         "{\n"
                         "  wxLongLong x = 0;\n"
                         "  int y = x.GetValue();\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void nullpointer8()
    {
        checkNullPointer("void foo()\n"
                         "{\n"
                         "  const char * x = 0;\n"
                         "  strdup(x);\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: x\n", errout.str());
        checkNullPointer("void foo()\n"
                         "{\n"
                         "  char const * x = 0;\n"
                         "  strdup(x);\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: x\n", errout.str());
    }

    void nullpointer9() //#ticket 1778
    {
        checkNullPointer("void foo()\n"
                         "{\n"
                         "  std::string * x = 0;\n"
                         "  *x = \"test\";\n"
                         "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: x\n", errout.str());
    }

    void checkUninitVar(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.executionPaths();
    }

    void uninitvar1()
    {
        // dereferencing uninitialized pointer..
        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    p->abcd();\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo<int> *p;\n"
                       "    p->abcd();\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("void f(Foo *p)\n"
                       "{\n"
                       "    int a;\n"
                       "    p->a = malloc(4 * a);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *p;\n"
                       "    delete p;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *p;\n"
                       "    delete [] p;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *p;\n"
                       "    *p = 135;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *x;\n"
                       "    int y = *x;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *x;\n"
                       "    int &y(*x);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    int x;\n"
                       "    int *y = &x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    int x = xyz::x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("static int foo()\n"
                       "{\n"
                       "    int ret;\n"
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: ret\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    a = 5 + a;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    bar(4 * a);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (i);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: i\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    for (int x = 0; i < 10; x++);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: i\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    for (int x = 0; x < 10; i++);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: i\n", errout.str());

        checkUninitVar("static int foo(int x)\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "        i = 0;\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: i\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int ar[10];\n"
                       "    int i;\n"
                       "    ar[i] = 0;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: i\n", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int x, y;\n"
                       "    x = (y = 10);\n"
                       "    int z = y * 2;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo p;\n"
                       "    p.abcd();\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo p;\n"
                       "    int x = p.abcd();\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("A a()\n"
                       "{\n"
                       "    A ret;\n"
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int a()\n"
                       "{\n"
                       "    int x;\n"
                       "    return x;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x[10];\n"
                       "    int *y = x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x;\n"
                       "    int *y = &x;\n"
                       "    *y = 0;\n"
                       "    x++;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    char x[10], y[10];\n"
                       "    char *z = x;\n"
                       "    memset(z, 0, sizeof(x));\n"
                       "    memcpy(y, x, sizeof(x));\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int a()\n"
                       "{\n"
                       "    int ret;\n"
                       "    std::cin >> ret;\n"
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int a()\n"
                       "{\n"
                       "    int ret;\n"
                       "    asm();\n"
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x[10];\n"
                       "    struct xyz xyz1 = { .x = x };\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    struct S *s;\n"
                       "    s->x = 0;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: s\n", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    struct S *s;\n"
                       "    FOREACH() { }\n"
                       "    s->x = 0;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: s\n", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    struct S *s1;\n"
                       "    struct S *s2;\n"
                       "    FOREACH(s1) { }\n"
                       "    s2->x = 0;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: s2\n", errout.str());

        // #1533
        checkUninitVar("char a()\n"
                       "{\n"
                       "    char key;\n"
                       "    struct A msg = { .buf = {&key} };\n"
                       "    init(&msg);\n"
                       "    return key;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "   char *buf = malloc(100);\n"
                       "   struct ABC *abc = buf;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Fred {\n"
                       "public:\n"
                       "    FILE *f;\n"
                       "    ~Fred();\n"
                       "}\n"
                       "Fred::~Fred()\n"
                       "{\n"
                       "    fclose(f);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int c;\n"
                       "    ab(sizeof(xyz), &c);\n"
                       "    if (c);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int c;\n"
                       "    a = (f2(&c));\n"
                       "    c++;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void f(int a)\n"
                         "{\n"
                         "    if (a) {\n"
                         "        char *p;\n"
                         "        *p = 0;\n"
                         "    }\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: p\n"   // <-- duplicate
                      "[test.cpp:5]: (error) Uninitialized variable: p\n", errout.str());

        // +=
        checkUninitVar("void f()\n"
                       "{\n"
                       "    int c;\n"
                       "    c += 2;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: c\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s = malloc(100);\n"
                       "    *s += 10;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: s\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a[10];\n"
                       "    a[0] += 10;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        // goto/setjmp/longjmp..
        checkUninitVar("void foo(int x)\n"
                       "{\n"
                       "    long b;\n"
                       "    if (g()) {\n"
                       "        b =2;\n"
                       "        goto found;\n"
                       "    }\n"
                       "\n"
                       "    return;\n"
                       "\n"
                       "found:\n"
                       "    int a = b;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    jmp_buf env;\n"
                       "    int a;\n"
                       "    int val = setjmp(env);\n"
                       "    if(val)\n"
                       "        return a;\n"
                       "    a = 1;\n"
                       "    longjmp(env, 1);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // macro_for..
        checkUninitVar("int foo()\n"
                       "{\n"
                       "  int retval;\n"
                       "  if (condition) {\n"
                       "    for12(1,2) { }\n"
                       "    retval = 1;\n"
                       "  }\n"
                       "  else\n"
                       "    retval = 2;\n"
                       "  return retval;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    goto exit;\n"
                       "    i++;\n"
                       "exit:\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // if..
    void uninitvar_if()
    {
        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    if (x)\n"
                       "        p = new Foo;\n"
                       "    p->abcd();\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("static void foo(int x)\n"
                       "{\n"
                       "    int a;\n"
                       "    if (x==1);\n"
                       "    if (x==2);\n"
                       "    x = a;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "        i = 22;\n"
                       "    else\n"
                       "        i = 33;\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "        i = 22;\n"
                       "    else\n"
                       "    {\n"
                       "        char *y = {0};\n"
                       "        i = 33;\n"
                       "    }\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "    {\n"
                       "        struct abc abc1 = (struct abc) { .a=0, .b=0, .c=0 };\n"
                       "        i = 22;\n"
                       "    }\n"
                       "    else\n"
                       "    {\n"
                       "        i = 33;\n"
                       "    }\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("static void foo(int x)\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    if (x)\n"
                       "        p = new Foo;\n"
                       "    if (x)\n"
                       "        p->abcd();\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo(int a)\n"
                       "{\n"
                       "    int n;\n"
                       "    int condition;\n"
                       "    if(a == 1) {\n"
                       "        n=0;\n"
                       "        condition=0;\n"
                       "    }\n"
                       "    else {\n"
                       "        n=1;\n"
                       "    }\n"
                       "\n"
                       "    if( n == 0) {\n"
                       "        a=condition;\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    C *c;\n"
                       "    if (fun(&c));\n"
                       "    c->Release();\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo(int x)\n"
                       "{\n"
                       "    int i;\n"
                       "    if (one())\n"
                       "        i = 1;\n"
                       "    else\n"
                       "        return 3;\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int ret;\n"
                       "    if (one())\n"
                       "        ret = 1;\n"
                       "    else\n"
                       "        throw 3;\n"
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int f(int a)\n"
                       "{\n"
                       "    int ret;\n"
                       "    if (a == 1)\n"
                       "        ret = 1;\n"
                       "    else\n"
                       "        XYZ ret = 2;\n"  // XYZ may be an unexpanded macro so bailout the checking of "ret".
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int f(int a, int b)\n"
                       "{\n"
                       "   int x;\n"
                       "   if (a)\n"
                       "      x = a;\n"
                       "   else {\n"
                       "      do { } while (f2());\n"
                       "      x = b;\n"
                       "   }\n"
                       "   return x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo(long verbose,bool bFlag)\n"
                       "{\n"
                       "  double t;\n"
                       "  if (bFlag)\n"
                       "  {\n"
                       "    if (verbose)\n"
                       "      t = 1;\n"
                       "    if (verbose)\n"
                       "      std::cout << (12-t);\n"
                       "  }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // ? :
        checkUninitVar("static void foo(int v)\n"
                       "{\n"
                       "    int x;\n"
                       "    if (v > 0)\n"
                       "        v = func(&x);\n"
                       "    x = v <= 0 ? -1 : x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    const char *msgid1, *msgid2;\n"
                       "    int ret = bar(&msgid1);\n"
                       "    if (ret > 0) {\n"
                       "        ret = bar(&msgid2);\n"
                       "    }\n"
                       "    ret = ret <= 0 ? -1 :\n"
                       "          strcmp(msgid1, msgid2) == 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo(int a, int b)\n"
                       "{\n"
                       "    int x; x = (a<b) ? 1 : 0;\n"
                       "    int y = y;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: y\n", errout.str());

        // ; { .. }
        checkUninitVar("int foo()\n"
                       "{\n"
                       "  int retval;\n"
                       "  if (condition) {\n"
                       "    { }\n"
                       "    retval = 1; }\n"
                       "  else\n"
                       "    retval = 2;\n"
                       "  return retval;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "  {\n"
                       "    for (int i = 0; i < 10; ++i)\n"
                       "    { }\n"
                       "  }\n"
                       "\n"
                       "  { }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    // handling for/while loops..
    void uninitvar_loops()
    {
        // for..
        checkUninitVar("void f()\n"
                       "{\n"
                       "    for (int i = 0; i < 4; ++i) {\n"
                       "        int a;\n"
                       "        b(4*a);\n"
                       "    }"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: a\n", errout.str());

        // while..
        checkUninitVar("int f()\n"
                       "{\n"
                       "    int i;\n"
                       "    while (fgets())\n"
                       "        i = 1;\n"
                       "    return i;"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: i\n", errout.str());

        checkUninitVar("void f(int i)\n"
                       "{\n"
                       "    int a;\n"
                       "    while (i < 10)\n"
                       "        i++;\n"
                       "    a++;"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: a\n", errout.str());
    }

    // switch..
    void uninitvar_switch()
    {
        checkUninitVar("void f(int x)\n"
                       "{\n"
                       "    short c;\n"
                       "    switch(x) {\n"
                       "    case 1:\n"
                       "        c++;\n"
                       "        break;\n"
                       "    };\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: c\n", errout.str());

        checkUninitVar("char * f()\n"
                       "{\n"
                       "    static char ret[200];\n"
                       "    memset(ret, 0, sizeof(ret));\n"
                       "    switch (x)\n"
                       "    {\n"
                       "        case 1: return ret;\n"
                       "        case 2: return ret;\n"
                       "    }\n"
                       "    return 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo(const int iVar, unsigned int slot, unsigned int pin)\n"
                       "{\n"
                       "    int i;\n"
                       "\n"
                       "    if (iVar == 0)\n"
                       "    {\n"
                       "        switch (slot)\n"
                       "        {\n"
                       "            case 4:  return 5;\n"
                       "            default: return -1;\n"
                       "        }\n"
                       "    }\n"
                       "    else\n"
                       "    {\n"
                       "        switch (pin)\n"
                       "        {\n"
                       "            case 0:   i =  2; break;\n"
                       "            default:  i = -1; break;\n"
                       "        }\n"
                       "    }\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // No segmentation fault
        checkUninitVar("void a() try\n"
                       "{\n"
                       "    {\n"
                       "        while (1) {\n"
                       "            switch (1) {\n"
                       "                case 1: {\n"
                       "                    int i;\n"
                       "                }\n"
                       "            }\n"
                       "        }\n"
                       "    } catch (...) {\n"
                       "    }\n"
                       "}\n");

        // #1855 - switch(foo(&x))
        checkUninitVar("int a()\n"
                       "{\n"
                       "    int x;\n"
                       "    switch (foo(&x))\n"
                       "    {\n"
                       "        case 1:\n"
                       "            return x;\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // arrays..
    void uninitvar_arrays()
    {
        checkUninitVar("int f()\n"
                       "{\n"
                       "    char a[10];\n"
                       "    a[a[0]] = 0;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("int f()\n"
                       "{\n"
                       "    char a[10];\n"
                       "    char c = *a;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("int f()\n"
                       "{\n"
                       "    char a[10];\n"
                       "    *a = '\\0';\n"
                       "    int i = strlen(a);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a, b[10];\n"
                       "    a = b[0] = 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[10], b[10];\n"
                       "    a[0] = b[0] = 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[10], *p;\n"
                       "    *(p = a) = 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char c[50] = \"\";\n"
                       "    strcat(c, \"test\");\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char s[20];\n"
                       "    strcpy(s2, s);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: s\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char s[20];\n"
                       "    strcat(s, \"abc\");\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: s\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char s[20];\n"
                       "    strchr(s, ' ');\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: s\n", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "        int y[2];\n"
                       "        int s;\n"
                       "        GetField( y + 0, \n"
                       "                       y + 1 );\n"
                       "        s = y[0]*y[1];\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // alloc..
    void uninitvar_alloc()
    {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s = malloc(100);\n"
                       "    strcat(s, \"abc\");\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: s\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s1 = new char[10];\n"
                       "    char *s2 = new char[strlen(s1)];\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: s1\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *p = malloc(64);\n"
                       "    int x = p[0];\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: p\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *p = malloc(64);\n"
                       "    if (p[0]) { }\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: p\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *p = malloc(64);\n"
                       "    return p[0];\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: p\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    Fred *fred = new Fred;\n"
                       "    fred->foo();\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo(char *s)\n"
                       "{\n"
                       "    char *a = malloc(100);\n"
                       "    *a = *s;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    char *a;\n"
                       "    if (a);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    char *a = malloc(100);\n"
                       "    if (a);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    abc->a = 123;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    abc->a.word = 123;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    abc->a = 123;\n"
                       "    abc->a += 123;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    free(abc);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s = malloc(100);\n"
                       "    if (!s)\n"
                       "        return;\n"
                       "    char c = *s;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Data is allocated but not initialized: s\n", errout.str());
    }

    // class / struct..
    void uninitvar_class()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "    int i;\n"
                       "    int a() { return i; }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    struct Relative {\n"
                       "        Surface *surface;\n"
                       "        void MoveTo(int x, int y) {\n"
                       "            surface->MoveTo();\n"
                       "        }\n"
                       "    };\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    static const struct ab {\n"
                       "        int a,b;\n"
                       "        int get_a() { return a; }"
                       "    } = { 0, 0 };\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int i;\n"
                       "    {\n"
                       "        union ab {\n"
                       "            int a,b;\n"
                       "        }\n"
                       "        i = 0;\n"
                       "    }\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // enum..
    void uninitvar_enum()
    {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    enum AB { a, b };\n"
                       "    AB ab;\n"
                       "    if (ab);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: ab\n", errout.str());
    }

    // references..
    void uninitvar_references()
    {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    int &b = a;\n"
                       "    b = 0;\n"
                       "    int x = a;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f(struct blame_entry *ent)\n"
                       "{\n"
                       "    struct origin *suspect = ent->suspect;\n"
                       "    char hex[41];\n"
                       "    strcpy(hex, sha1_to_hex(suspect->commit->object.sha1));\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    const std::string s(x());\n"
                       "    strchr(s.c_str(), ',');\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // strncpy doesn't always 0-terminate..
    void uninitvar_strncpy()
    {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[100];\n"
                       "    strncpy(a, s, 20);\n"
                       "    strncat(a, s, 20);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dangerous usage of 'a' (strncpy doesn't always 0-terminate it)\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[100];\n"
                       "    strncpy(a, \"hello\", 3);\n"
                       "    strncat(a, \"world\", 20);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dangerous usage of 'a' (strncpy doesn't always 0-terminate it)\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[100];\n"
                       "    strncpy(a, \"hello\", sizeof(a));\n"
                       "    strncat(a, \"world\", 20);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    std::string analyseFunctions(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        std::set<std::string> f;
        const CheckOther checkOther((const Tokenizer *)0, (const Settings *)0, (ErrorLogger *)0);
        checkOther.analyse(tokenizer.tokens(), f);

        std::string ret;
        for (std::set<std::string>::const_iterator it = f.begin(); it != f.end(); ++it)
            ret += (ret.empty() ? "" : " ") + *it;
        return ret;
    }

    void uninitvar_func()
    {
        // function analysis..
        ASSERT_EQUALS("foo", analyseFunctions("void foo(int x) { }"));
        ASSERT_EQUALS("foo", analyseFunctions("void foo(const int &x) { }"));
        ASSERT_EQUALS("foo", analyseFunctions("void foo(int &x) { ++x; }"));
        ASSERT_EQUALS("", analyseFunctions("void foo(int &x) { x = 0; }"));
        ASSERT_EQUALS("", analyseFunctions("void foo(s x) { }"));

        // function calls..
        checkUninitVar("void assignOne(int &x)\n"
                       "{ x = 1; }\n"
                       "\n"
                       "int f()\n"
                       "{\n"
                       "    int i;\n"
                       "    assignOne(i);\n"
                       "    return i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int f(int (*assign)(int *p))\n"
                       "{\n"
                       "    int i;\n"
                       "    (*assign)(&i);\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int f()\n"
                       "{\n"
                       "    char s[10];\n"
                       "    return bar(s);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    FILE *f;\n"
                       "    fflush(f);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: f\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    Abc *p;\n"
                       "    int sz = sizeof(*p);\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    x = bar(sizeof(*p));\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    x = bar(p->begin());\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("int foo(int x) { return x; }\n"
                       "void f2()\n"
                       "{\n"
                       "    int x;\n"
                       "    foo(x);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar("void foo(const char *s)\n"
                       "{\n"
                       "    char *p;\n"
                       "    memcpy(p, s, 100);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("void foo(const char *s)\n"
                       "{\n"
                       "    char *p = malloc(100);\n"
                       "    memcpy(p, s, 100);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // using uninitialized function pointer..
        checkUninitVar("void foo()\n"
                       "{\n"
                       "    void (*f)();\n"
                       "    f();\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: f\n", errout.str());

        // calling noreturn function..
        checkUninitVar("int foo(int a) {\n"
                       "    int x;\n"
                       "    if (a==1)\n"
                       "        g();\n"    // might be a noreturn function
                       "    else\n"
                       "        x = 3;\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo(int a) {\n"
                       "    int x;\n"
                       "    if (a==1)\n"
                       "        g(1);\n"    // might be a noreturn function
                       "    else\n"
                       "        x = 3;\n"
                       "    return x;\n"
                       "}");
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

    void checkpostIncrementDecrement(const char code[])
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
        settings._checkCodingStyle = true;
        settings.inconclusive = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.postIncrement();
    }

    void postIncrementDecrementStl()
    {
        checkpostIncrementDecrement("void f1()\n"
                                    "{\n"
                                    "    std::list<int>::iterator it;\n"
                                    "    for (it = ab.begin(); it != ab.end(); it++)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Pre-Incrementing variable 'it' is preferred to Post-Incrementing\n", errout.str());

        checkpostIncrementDecrement("void f1()\n"
                                    "{\n"
                                    "    std::list<int>::iterator it;\n"
                                    "    for (it = ab.begin(); it != ab.end(); it++)\n"
                                    "        ;\n"
                                    "    for (it = ab.begin(); it != ab.end(); it++)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Pre-Incrementing variable 'it' is preferred to Post-Incrementing\n"
                      "[test.cpp:6]: (style) Pre-Incrementing variable 'it' is preferred to Post-Incrementing\n", errout.str());

        checkpostIncrementDecrement("void f2()\n"
                                    "{\n"
                                    "    std::list<int>::iterator it;\n"
                                    "    for (it = ab.end(); it != ab.begin(); it--)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Pre-Decrementing variable 'it' is preferred to Post-Decrementing\n", errout.str());

        checkpostIncrementDecrement("void f2()\n"
                                    "{\n"
                                    "    std::list<int>::iterator it;\n"
                                    "    for (it = ab.end(); it != ab.begin(); it--)\n"
                                    "        ;\n"
                                    "    for (it = ab.end(); it != ab.begin(); it--)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Pre-Decrementing variable 'it' is preferred to Post-Decrementing\n"
                      "[test.cpp:6]: (style) Pre-Decrementing variable 'it' is preferred to Post-Decrementing\n", errout.str());

        checkpostIncrementDecrement("void f1()\n"
                                    "{\n"
                                    "    std::list<std::vector<int> >::iterator it;\n"
                                    "    for (it = ab.begin(); it != ab.end(); it++)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Pre-Incrementing variable 'it' is preferred to Post-Incrementing\n", errout.str());

        checkpostIncrementDecrement("void f1()\n"
                                    "{\n"
                                    "    std::map<int, std::vector<int> >::iterator it;\n"
                                    "    for (it = ab.begin(); it != ab.end(); it++)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Pre-Incrementing variable 'it' is preferred to Post-Incrementing\n", errout.str());
    }

    void postIncrementDecrementClass()
    {
        checkpostIncrementDecrement("class TestClass;\n"
                                    "void f1()\n"
                                    "{\n"
                                    "    TestClass tClass;\n"
                                    "    for (tClass = TestClass.begin(); tClass != TestClass.end(); tClass++)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Pre-Incrementing variable 'tClass' is preferred to Post-Incrementing\n", errout.str());

        checkpostIncrementDecrement("class TestClass;\n"
                                    "void f1()\n"
                                    "{\n"
                                    "    TestClass tClass;\n"
                                    "    for (tClass = TestClass.end(); tClass != TestClass.begin(); tClass--)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Pre-Decrementing variable 'tClass' is preferred to Post-Decrementing\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:1]: (style) Function parameter 'str' is passed by value. It could be passed by reference instead.\n", errout.str());

        testPassedByValue("class Foo;\nvoid f(const Foo foo) {}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Function parameter 'foo' is passed by value. It could be passed by reference instead.\n", errout.str());

        testPassedByValue("void f(const std::string &str) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::vector<int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Function parameter 'v' is passed by value. It could be passed by reference instead.\n", errout.str());

        testPassedByValue("void f(const std::vector<std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Function parameter 'v' is passed by value. It could be passed by reference instead.\n", errout.str());

        testPassedByValue("void f(const std::vector<int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::map<int,int> &v) {}");
        ASSERT_EQUALS("", errout.str());

        testPassedByValue("void f(const std::map<int,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Function parameter 'v' is passed by value. It could be passed by reference instead.\n", errout.str());

        testPassedByValue("void f(const std::map<std::string,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Function parameter 'v' is passed by value. It could be passed by reference instead.\n", errout.str());

        testPassedByValue("void f(const std::map<int,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Function parameter 'v' is passed by value. It could be passed by reference instead.\n", errout.str());

        testPassedByValue("void f(const std::map<std::string,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1]: (style) Function parameter 'v' is passed by value. It could be passed by reference instead.\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (style) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (!strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (strlen(str) == 0) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (strlen(str) > 0) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (strlen(str) != 0) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (0 != strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());

        check("if (0 == strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Empty string test can be simplified to \"*str == '\\0'\"\n", errout.str());

        check("if (0 < strlen(str)) { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) Non-empty string test can be simplified to \"*str != '\\0'\"\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (style) Suspicious code 'sizeof sizeof ..', most likely there should only be one sizeof. The current code is equivalent to 'sizeof(size_t)'.\n", errout.str());
    }

    void sizeofCalculation()
    {
        check("sizeof(a+b)");
        ASSERT_EQUALS("[test.cpp:1]: (style) Found calculation inside sizeof()\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:7]: (style) Redundant assignment of \"y\" in switch\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:8]: (style) Redundant assignment of \"y\" in switch\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) Redundant assignment of \"x\" to itself\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        int x = x;\n"
              "        return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant assignment of \"x\" to itself\n", errout.str());

        check("void foo()\n"
              "{\n"
              "        std::string var = var = \"test\";\n"
              "        return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant assignment of \"var\" to itself\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:6]: (style) scanf without field width limits can crash with huge input data\n"
                      "[test.cpp:7]: (style) scanf without field width limits can crash with huge input data\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:6]: (style) scanf without field width limits can crash with huge input data\n"
                      "[test.cpp:7]: (style) scanf without field width limits can crash with huge input data\n", errout.str());
    }
};

REGISTER_TEST(TestOther)

