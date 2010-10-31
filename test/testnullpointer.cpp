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
#include "checknullpointer.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestNullPointer : public TestFixture
{
public:
    TestNullPointer() : TestFixture("TestNullPointer")
    { }

private:


    void run()
    {
        TEST_CASE(nullpointer1);
        TEST_CASE(nullpointer2);
        TEST_CASE(nullpointer3);    // dereferencing struct and then checking if it's null
        TEST_CASE(nullpointer4);
        TEST_CASE(nullpointer5);    // References should not be checked
        TEST_CASE(nullpointer6);
        TEST_CASE(nullpointer7);
        TEST_CASE(nullpointer8);
        TEST_CASE(nullpointer9);
        TEST_CASE(nullpointer10);	// check if pointer is null and then dereference it
    }

    void check(const char code[])
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
        CheckNullPointer checkNullPointer(&tokenizer, &settings, this);
        checkNullPointer.nullPointer();
        tokenizer.simplifyTokenList();
        checkNullPointer.nullConstantDereference();
        checkNullPointer.executionPaths();
    }


    void nullpointer1()
    {
        check("int foo(const Token *tok)\n"
              "{\n"
              "    while (tok);\n"
              "    tok = tok->next();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: tok\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    for (const Token *tok = tokens; tok; tok = tok->next())\n"
              "    {\n"
              "        while (tok && tok->str() != \";\")\n"
              "            tok = tok->next();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: tok - otherwise it is redundant to check if tok is null at line 5\n", errout.str());

        check("void foo(Token &tok)\n"
              "{\n"
              "    for (int i = 0; i < tok.size(); i++ )\n"
              "    {\n"
              "        while (!tok)\n"
              "            char c = tok.read();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    for (const Token *tok = tokens; tok; tok = tok->next())\n"
              "    {\n"
              "        while (tok && tok->str() != \";\")\n"
              "            tok = tok->next();\n"
              "        if( !tok ) break;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    for (const Token *tok = tokens; tok; tok = tok ? tok->next() : NULL)\n"
              "    {\n"
              "        while (tok && tok->str() != \";\")\n"
              "            tok = tok->next();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(A*a)\n"
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
        check("void f(A *a)\n"
              "{\n"
              "    if (a->x == 1)\n"
              "    {\n"
              "        a = a->next;\n"
              "    }\n"
              "    else if (a->x == 2) { }\n"
              "    if (a) { }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // ticket #2134 - sizeof doesn't dereference
        check("void f() {\n"
              "    int c = 1;\n"
              "    int *list = NULL;\n"
              "    sizeof(*list);\n"
              "    if (!list)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

    }

    void nullpointer2()
    {
        // Null pointer dereference can only happen with pointers
        check("void foo()\n"
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
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 4\n", errout.str());

        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    bar(abc->a);\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 4\n", errout.str());

        // ok dereferencing in a condition
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    if (abc && abc->a);\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // ok to use a linked list..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    abc = abc->next;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // reassign struct..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    abc = abc->next;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    f(&abc);\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // goto..
        check("void foo(struct ABC *abc)\n"
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
        check("void freeAbc(struct ABC *abc)\n"
              "{\n"
              "    while (abc)\n"
              "    {\n"
              "        struct ABC *next = abc->next;\n"
              "        if (abc) delete abc;\n"
              "        abc = next;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(struct ABC *abc)\n"
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

        check("void f()\n"
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
        check("void foo(ABC *abc)\n"
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
        check("void foo(int *p)\n"
              "{\n"
              "    *p = 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        check("void foo(int *p)\n"
              "{\n"
              "    bar(*p);\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        // no error
        check("void foo()\n"
              "{\n"
              "    int *p;\n"
              "    f(&p);\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int **p = f();\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int *p)\n"
              "{\n"
              "    if (x)\n"
              "        p = 0;\n"
              "    else\n"
              "        *p = 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    int a = 2 * x;"
              "    if (x == 0)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int *p)\n"
              "{\n"
              "    int var1 = p ? *p : 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(P *p)\n"
              "{\n"
              "  while (p)\n"
              "    if (p->check())\n"
              "      break;\n"
              "    else\n"
              "      p = p->next();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(Document *doc) {\n"
              "    int x = doc && doc->x;\n"
              "    if (!doc) {\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void nullpointer5()
    {
        // errors..
        check("void foo(A &a)\n"
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
        check("static void foo()\n"
              "{\n"
              "    Foo *p = 0;\n"
              "    if (a == 1)\n"
              "        p = new FooBar;\n"
              "    else if (a == 2)\n"
              "        p = new FooCar;\n"
              "    p->abcd();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Possible null pointer dereference: p\n", errout.str());

        check("static void foo()\n"
              "{\n"
              "    int *p = 0;\n"
              "    int *q = p;\n"
              "    q[0] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: q\n", errout.str());

        check("static void foo()\n"
              "{\n"
              "    int *p = 0;\n"
              "    int &r = *p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        check("static void foo(int x)\n"
              "{\n"
              "    int *p = 0;\n"
              "    int y = 5 + *p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        check("static void foo(int x)\n"
              "{\n"
              "    Foo<int> *abc = 0;\n"
              "    abc->a();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: abc\n", errout.str());

        check("static void foo()\n"
              "{\n"
              "    int *p(0);\n"
              "    std::cout << *p;"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char *c = 0;\n"
              "    {\n"
              "        delete c;\n"
              "    }\n"
              "    c[0] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Possible null pointer dereference: c\n", errout.str());

        check("static void foo()\n"
              "{\n"
              "    int *p = 0;\n"
              "    if (3 > *p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());

        check("void f()\n"
              "{\n"
              "    if (x) {\n"
              "        char *c = 0;\n"
              "        *c = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Null pointer dereference\n", errout.str());

        // no false positive..
        check("static void foo()\n"
              "{\n"
              "    Foo *p = 0;\n"
              "    p = new Foo;\n"
              "    p->abcd();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("static void foo()\n"
              "{\n"
              "    Foo *p = 0;\n"
              "    if (!p)\n"
              "        return;\n"
              "    p->abcd();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("static void foo()\n"
              "{\n"
              "    int *p = 0;\n"
              "    exit();\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("static void foo(int a)\n"
              "{\n"
              "    Foo *p = 0;\n"
              "    if (a && p)\n"
              "        p->do_something();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int sz = sizeof((*(struct dummy *)0).x);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void get_offset(long &offset)\n"
              "{\n"
              "    mystruct * temp; temp = 0;\n"
              "    offset = (long)(&(temp->z));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #1893 - try/catch inside else
        check("int *test(int *Z)\n"
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

        check("int *test(int *Z)\n"
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

        // Ticket #2052 (false positive for 'else continue;')
        check("void f() {\n"
              "    for (int x = 0; x < 5; ++x) {"
              "        int *p = 0;\n"
              "        if (a(x)) p=b(x);\n"
              "        else continue;\n"
              "        *p = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // function pointer..
        check("void foo()\n"
              "{\n"
              "    void (*f)();\n"
              "    f = 0;\n"
              "    f();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: f\n", errout.str());

        check("static void foo()\n"
              "{\n"
              "    int *p = 0;\n"
              "    int *p2 = 0;\n"
              "    int r = *p;\n"
              "    int r2 = *p2;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Null pointer dereference\n"
                      "[test.cpp:6]: (error) Null pointer dereference\n", errout.str());

        // loops..
        check("void f() {\n"
              "    int *p = 0;\n"
              "    for (int i = 0; i < 10; ++i) {\n"
              "        int x = *p + 1;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p\n", errout.str());

        check("void f(int a) {\n"
              "    const char *p = 0;\n"
              "    if (a) {\n"
              "    	 p = \"abcd\";\n"
              "    }\n"
              "    for (int i = 0; i < 3; i++) {\n"
              "        if (a && (p[i] == '1'));\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void nullpointer7()
    {
        check("void foo()\n"
              "{\n"
              "  wxLongLong x = 0;\n"
              "  int y = x.GetValue();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void nullpointer8()
    {
        check("void foo()\n"
              "{\n"
              "  const char * x = 0;\n"
              "  strdup(x);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: x\n", errout.str());
        check("void foo()\n"
              "{\n"
              "  char const * x = 0;\n"
              "  strdup(x);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: x\n", errout.str());
    }

    void nullpointer9() //#ticket 1778
    {
        check("void foo()\n"
              "{\n"
              "  std::string * x = 0;\n"
              "  *x = \"test\";\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: x\n", errout.str());
    }

    // Check if pointer is null and the dereference it
    void nullpointer10()
    {
        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p\n", errout.str());

        check("void foo(abc *p) {\n"
              "    if (!p) {\n"
              "    }\n"
              "    else if (!p->x) {\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        abort();\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        throw x;\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if (!p) {\n"
              "        switch (x) { }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // This is why this check can't be used on the simplified token list
        check("void f(Foo *foo) {\n"
              "    if (!dynamic_cast<bar *>(foo)) {\n"
              "        *foo = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestNullPointer)

