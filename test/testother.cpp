/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

        TEST_CASE(delete1);
        TEST_CASE(delete2);

        TEST_CASE(unreachable1);

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

        TEST_CASE(nullpointer1);
        TEST_CASE(nullpointer2);
        TEST_CASE(nullpointer3);    // dereferencing struct and then checking if it's null
        TEST_CASE(nullpointer4);
        TEST_CASE(nullpointer5);    // References should not be checked
        TEST_CASE(nullpointer6);
        TEST_CASE(nullpointer7);

        TEST_CASE(uninitvar1);

        TEST_CASE(oldStylePointerCast);

        TEST_CASE(postIncrementDecrementStl);
        TEST_CASE(postIncrementDecrementClass);

        TEST_CASE(dangerousStrolUsage);

        TEST_CASE(passedByValue);
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Simplify token list..
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.warningRedundantCode();
        checkOther.checkZeroDivision();
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

    void delete1()
    {
        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "    {\n"
              "        delete p;\n"
              "        p = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "        delete p;\n"
              "    else\n"
              "        p = new P();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (0 != g->p)\n"
              "        delete this->p;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (0 != this->g->a)\n"
              "        delete this->p->a;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void delete2()
    {
        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "    {\n"
              "        delete p;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "        delete p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p != NULL)\n"
              "        delete p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "        delete [] p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (0 != this->p)\n"
              "        delete this->p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (0 != this->p->a)\n"
              "        delete this->p->a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());


        check("void Foo::deleteInstance()\n"
              "{\n"
              "    if (Foo::instance != NULL)\n"
              "        delete Foo::instance;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());
    }

    void unreachable1()
    {
        check("void foo()\n"
              "{\n"
              "    switch (p)\n"
              "    {\n"
              "    default:\n"
              "        return 0;\n"
              "        break;\n"
              "    }\n"
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
        ASSERT_EQUALS("[test.cpp:4]: (error) Overlapping data buffer buf\n", errout.str());
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
                    "    char ch = '/';\n"
                    "    const char *p = \"/usr\" + ch;\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Unusual pointer arithmetic\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable i can be limited\n", errout.str());

        varScope("void f(int x)\n"
                 "{\n"
                 "    int i = 0;\n"
                 "    if (x) {b()}\n"
                 "    else {\n"
                 "        for ( ; i < 10; ++i) ;\n"
                 "    }\n"
                 "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The scope of the variable i can be limited\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: tok\n", errout.str());

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
    void nullpointer3()
    {
        // errors..
        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    int *a = abc->a;\n"
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
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 4\n", errout.str());

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
                         "    a = abc->a;\n"
                         "    abc = abc->next;\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
                         "    a = abc->a;\n"
                         "    f(&abc);\n"
                         "    if (!abc)\n"
                         "        ;\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());

        // goto..
        checkNullPointer("void foo(struct ABC *abc)\n"
                         "{\n"
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
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p\n", errout.str());

        checkNullPointer("void foo(int *p)\n"
                         "{\n"
                         "    bar(*p);\n"
                         "    if (!p)\n"
                         "        ;\n"
                         "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p\n", errout.str());

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

    void checkUninitVar(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.uninitvar();
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
                       "    Foo p;\n"
                       "    p.abcd();\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("A a()\n"
                       "{\n"
                       "    A ret;\n"
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x;\n"
                       "    int y = x;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x[10];\n"
                       "    int *y = x;\n"
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

        // goto..
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


        // if..
        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    if (x)\n"
                       "        p = new Foo;\n"
                       "    p->abcd();\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: p\n", errout.str());

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

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    if (x)\n"
                       "        p = new Foo;\n"
                       "    if (x)\n"
                       "        p->abcd();\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    C *c;\n"
                       "    if (fun(&c));\n"
                       "    c->Release();\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // switch..
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


        // member variables..
        checkUninitVar("class Fred\n"
                       "{\n"
                       "    int i;\n"
                       "    int a() { return i; }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

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

        // arrays..
        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[10], b[10];\n"
                       "    a[0] = b[0] = 0;\n"
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
                       "    strncpy(s, \"abcde\", 2);\n"
                       "    strcat(s, \"abc\");\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: s\n", errout.str());
    }


    void checkOldStylePointerCast(const char code[])
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
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.warningOldStylePointerCast();
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
        ASSERT_EQUALS("[test.cpp:4]: (possible style) Pre-Incrementing variable 'it' is preferred to Post-Incrementing\n", errout.str());

        checkpostIncrementDecrement("void f2()\n"
                                    "{\n"
                                    "    std::list<int>::iterator it;\n"
                                    "    for (it = ab.end(); it != ab.begin(); it--)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible style) Pre-Decrementing variable 'it' is preferred to Post-Decrementing\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:5]: (possible style) Pre-Incrementing variable 'tClass' is preferred to Post-Incrementing\n", errout.str());

        checkpostIncrementDecrement("class TestClass;\n"
                                    "void f1()\n"
                                    "{\n"
                                    "    TestClass tClass;\n"
                                    "    for (tClass = TestClass.end(); tClass != TestClass.begin(); tClass--)\n"
                                    "        ;\n"
                                    "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible style) Pre-Decrementing variable 'tClass' is preferred to Post-Decrementing\n", errout.str());
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

};

REGISTER_TEST(TestOther)

