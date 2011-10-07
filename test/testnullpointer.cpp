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
        TEST_CASE(structDerefAndCheck);    // dereferencing struct and then checking if it's null
        TEST_CASE(pointerDerefAndCheck);
        TEST_CASE(nullpointer5);    // References should not be checked
        TEST_CASE(nullpointerExecutionPaths);
        TEST_CASE(nullpointerExecutionPathsLoop);
        TEST_CASE(nullpointer7);
        TEST_CASE(nullpointer8);
        TEST_CASE(nullpointer9);
        TEST_CASE(nullpointer10);
        TEST_CASE(nullpointer11); // ticket #2812
        TEST_CASE(pointerCheckAndDeRef);	// check if pointer is null and then dereference it
        TEST_CASE(nullConstantDereference);		// Dereference NULL constant
        TEST_CASE(gcc_statement_expression);    // Don't crash
        TEST_CASE(snprintf_with_zero_size);
        TEST_CASE(snprintf_with_non_zero_size);
    }

    void check(const char code[])
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
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: tok - otherwise it is redundant to check if tok is null at line 3\n", errout.str());

        // #2681
        check("void foo(const Token *tok)\n"
              "{\n"
              "    while (tok && tok->str() == \"=\")\n"
              "        tok = tok->next();\n"
              "\n"
              "    if (tok->str() != \";\")\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Possible null pointer dereference: tok - otherwise it is redundant to check if tok is null at line 3\n", errout.str());

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

        // ticket #2245 - sizeof doesn't dereference
        check("void f(Bar *p) {\n"
              "    if (!p) {\n"
              "        int sz = sizeof(p->x);\n"
              "    }\n"
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
    void structDerefAndCheck()
    {
        // errors..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 4\n", errout.str());

        check("void foo(struct ABC *abc) {\n"
              "    bar(abc->a);\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 3\n", errout.str());

        check("void foo(ABC *abc) {\n"
              "    abc->do_something();\n"
              "    if (abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 3\n", errout.str());

        check("void foo(ABC *abc) {\n"
              "    if (abc->a == 3) {\n"
              "        return;\n"
              "    }\n"
              "    if (abc) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 5\n", errout.str());

        check("void f(ABC *abc) {\n"
              "    if (abc->x == 0) {\n"
              "        return;\n"
              "    }\n"
              "    if (!abc);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 5\n", errout.str());

        // TODO: False negative if member of member is dereferenced
        check("void foo(ABC *abc) {\n"
              "    abc->next->a = 0;\n"
              "    if (abc->next)\n"
              "        ;\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 3\n", "", errout.str());

        check("void foo(ABC *abc) {\n"
              "    abc->a = 0;\n"
              "    if (abc && abc->b == 0)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 3\n", errout.str());

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

        // #2641 - global pointer, function call
        check("ABC *abc;\n"
              "void f() {\n"
              "    abc->a = 0;\n"
              "    do_stuff();\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS("",errout.str());

        check("Fred *fred;\n"
              "void f() {\n"
              "    fred->foo();\n"
              "    if (fred) { }\n"
              "}");
        ASSERT_EQUALS("",errout.str());

        // #2641 - local pointer, function call
        check("void f() {\n"
              "    ABC *abc;\n"
              "    abc->a = 0;\n"
              "    do_stuff();\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 5\n",errout.str());

        // #2641 - local pointer, function call
        check("void f(ABC *abc) {\n"
              "    abc->a = 0;\n"
              "    do_stuff();\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: abc - otherwise it is redundant to check if abc is null at line 4\n",errout.str());

        // #2691 - switch/break
        check("void f(ABC *abc) {\n"
              "    switch ( x ) {\n"
              "        case 14:\n"
              "            sprintf(buf, \"%d\", abc->a);\n"
              "            break;\n"
              "        case 15:\n"
              "            if ( abc ) {}\n"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3128
        check("void f(ABC *abc) {\n"
              "    x(!abc || y(abc->a));\n"
              "    if (abc) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Dereferencing a pointer and then checking if it is null
    void pointerDerefAndCheck()
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
              "    *p = 0;\n"
              "    if (p) { }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        check("void foo(int *p)\n"
              "{\n"
              "    *p = 0;\n"
              "    if (p || q) { }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        check("void foo(int *p)\n"
              "{\n"
              "    bar(*p);\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        check("void foo(char *p)\n"
              "{\n"
              "    strcpy(p, \"abc\");\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", errout.str());

        check("void foo(char *p)\n"
              "{\n"
              "    if (*p == 0) { }\n"
              "    if (!p) { }\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 4\n", "", errout.str());

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

        check("void foo(int *p)\n"
              "{\n"
              "    int var1 = x ? *p : 5;\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        // Ticket #3125
        check("void foo(ABC *p)\n"
              "{\n"
              "    int var1 = p ? (p->a) : 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(ABC *p)\n"
              "{\n"
              "    int var1 = p ? (1 + p->a) : 0;\n"
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

        // #3128 - false positive
        check("void f(int *p) {\n"
              "    assert(!p || (*p<=6));\n"
              "    if (p) { *p = 0; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int *p) {\n"
              "    assert(p && (*p<=6));\n"
              "    if (p) { *p = 0; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(x *p)\n"
              "{\n"
              "    p = p->next;\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(x *p)\n"
              "{\n"
              "    p = bar(p->next);\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(x *p)\n"
              "{\n"
              "    p = aa->bar(p->next);\n"
              "    if (!p)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    abc = abc ? abc->next : 0;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(Item *item) {\n"
              "    x = item ? ab(item->x) : 0;\n"
              "    if (item) { }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(Item *item) {\n"
              "    item->x = 0;\n"
              "    a = b ? c : d;\n"
              "    if (item) { }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Possible null pointer dereference: item - otherwise it is redundant to check if item is null at line 4\n", errout.str());

        check("BOOL GotoFlyAnchor()\n"  // #2243
              "{\n"
              "    const SwFrm* pFrm = GetCurrFrm();\n"
              "    do {\n"
              "        pFrm = pFrm->GetUpper();\n"
              "    } while( pFrm && !pFrm->IsFlyFrm() );\n"
              "\n"
              "    if( !pFrm )\n"
              "        return FALSE;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #2463
        check("struct A \n"
              "{\n"
              "    B* W;\n"
              "\n"
              "    void f() {\n"
              "        switch (InData) {\n"
              "            case 2:\n"
              "                if (!W) return;\n"
              "                W->foo();\n"
              "                break;\n"
              "            case 3:\n"
              "                f();\n"
              "                if (!W) return;\n"
              "                break;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #2525 - sizeof
        check("void f() {\n"
              "    int *test = NULL;\n"
              "    int c = sizeof(test[0]);\n"
              "    if (!test)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #3023 - checked deref
        check("void f(struct ABC *abc) {\n"
              "  WARN_ON(!abc || abc->x == 0);\n"
              "  if (!abc) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(struct ABC *abc) {\n"
              "  WARN_ON(!abc || abc->x == 7);\n"
              "  if (!abc) { }\n"
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
    void nullpointerExecutionPaths()
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
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Possible null pointer dereference: p\n",
                           "", errout.str());

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

        check("void f() {\n"
              "    Foo *p = 0;\n"
              "    bool b = (p && (p->type() == 1));\n"
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
              "        p = \"abcd\";\n"
              "    }\n"
              "    for (int i = 0; i < 3; i++) {\n"
              "        if (a && (p[i] == '1'));\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // ticket #2251: taking the address of member
        check("void f() {\n"
              "    Fred *fred = 0;\n"
              "    int x = &fred->x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Ticket #2350
    void nullpointerExecutionPathsLoop()
    {
        // No false positive:
        check("void foo() {\n"
              "    int n;\n"
              "    int *argv32;\n"
              "    if (x) {\n"
              "        n = 0;\n"
              "        argv32 = 0;\n"
              "    }\n"
              "\n"
              "    for (int i = 0; i < n; i++) {\n"
              "        argv32[i] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // No false negative:
        check("void foo() {\n"
              "    int n;\n"
              "    int *argv32;\n"
              "    if (x) {\n"
              "        n = 10;\n"
              "        argv32 = 0;\n"
              "    }\n"
              "\n"
              "    for (int i = 0; i < n; i++) {\n"
              "        argv32[i] = 0;\n"
              "    }\n"
              "}\n");
        TODO_ASSERT_EQUALS("error",
                           "", errout.str());

        // #2231 - error if assignment in loop is not used
        check("void f() {\n"
              "    char *p = 0;\n"
              "\n"
              "    for (int x = 0; x < 3; ++x) {\n"
              "        if (y[x] == 0) {\n"
              "            p = malloc(10);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Possible null pointer dereference: p\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:4]: (error) Null pointer dereference\n", errout.str());
    }

    void nullpointer10()
    {
        check("void foo()\n"
              "{\n"
              "  struct my_type* p = 0;\n"
              "  p->x = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p\n", errout.str());

        check("void foo()\n"
              "{\n"
              "  struct my_type* p;\n"
              "  p = 0; \n"
              "  p->x = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: p\n", errout.str());
    }

    void nullpointer11() // ticket #2812
    {
        check("int foo()\n"
              "{\n"
              "  my_type* p = 0;\n"
              "  return p->x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p\n", errout.str());

        check("int foo()\n"
              "{\n"
              "  struct my_type* p = 0;\n"
              "  return p->x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p\n", errout.str());

        check("int foo()\n"
              "{\n"
              "  my_type* p;\n"
              "  p = 0; \n"
              "  return p->x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: p\n", errout.str());

        check("int foo()\n"
              "{\n"
              "  struct my_type* p;\n"
              "  p = 0; \n"
              "  return p->x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: p\n", errout.str());
    }

    // Check if pointer is null and the dereference it
    void pointerCheckAndDeRef()
    {
        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        check("void foo(char *p) {\n"
              "    if (NULL == p) {\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        check("void foo(char *p) {\n"
              "    if (p == NULL) {\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        check("void foo(char *p) {\n"
              "    if (p == NULL) {\n"
              "    }\n"
              "    printf(\"%c\", *p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        check("void foo(char *p) {\n"
              "    if (p && *p == 0) {\n"
              "    }\n"
              "    printf(\"%c\", *p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        check("void foo(char *p) {\n"
              "    if (p && *p == 0) {\n"
              "    } else { *p = 0; }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        check("void foo(char *p) {\n"
              "    if (p) {\n"
              "    }\n"
              "    strcpy(p, \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        check("void foo(char *p) {\n"
              "    if (p) {\n"
              "    }\n"
              "    bar();\n"
              "    strcpy(p, \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

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
              "        (*bail)();\n"
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

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        ab.abort();\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        switch (x) { }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "    }\n"
              "    return *x;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // operator!
        check("void f() {\n"
              "    A a;\n"
              "    if (!a) {\n"
              "        a.x();\n"
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

        // ticket: #2300 - calling unknown function that may initialize the pointer
        check("Fred *fred;\n"
              "void a() {\n"
              "    if (!fred) {\n"
              "        initfred();\n"
              "        fred->x = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // ticket #1219
        check("void foo(char *p) {\n"
              "    if (p) {\n"
              "        return;\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 2\n", errout.str());

        // #2467 - unknown macro may terminate the application
        check("void f(Fred *fred) {\n"
              "    if (fred == NULL) {\n"
              "        MACRO;\n"
              "    }\n"
              "    fred->a();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2493 - switch
        check("void f(Fred *fred) {\n"
              "    if (fred == NULL) {\n"
              "        x = 0;\n"
              "    }\n"
              "    switch (x) {\n"
              "        case 1:\n"
              "            fred->a();\n"
              "            break;\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2582 - segmentation fault
        check("if()");

        // #2674 - different functions
        check("class Fred {\n"
              "public:\n"
              "    Wilma *wilma;\n"
              "    void a();\n"
              "    void b();\n"
              "};\n"
              "\n"
              "void Fred::a() {\n"
              "    if ( wilma ) { }\n"
              "}\n"
              "\n"
              "void Fred::b() {\n"
              "    wilma->Reload();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void test(int *i) {\n"
              "  if(i == NULL) {\n"
              "    int b = 1;\n"
              "  } else {\n"
              "    int b = *i;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2696 - false positives nr 1
        check("void f()\n"
              "{\n"
              "   struct foo *pFoo = NULL;\n"
              "   size_t len;\n"
              "\n"
              "   len = sizeof(*pFoo) - sizeof(pFoo->data);\n"
              "\n"
              "   if (pFoo)\n"
              "      bar();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2696 - false positives nr 2
        check("void f()\n"
              "{\n"
              "   struct foo *pFoo = NULL;\n"
              "   size_t len;\n"
              "\n"
              "   while (pFoo)\n"
              "      pFoo = pFoo->next;\n"
              "\n"
              "   len = sizeof(pFoo->data);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2696 - false positives nr 3
        check("void f()\n"
              "{\n"
              "   struct foo *pFoo = NULL;\n"
              "   size_t len;\n"
              "\n"
              "   while (pFoo)\n"
              "      pFoo = pFoo->next;\n"
              "\n"
              "   len = decltype(*pFoo);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo(struct Fred *fred) {\n"
              "    if (fred) { int a = 0; }\n"
              "    return fred->a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: fred - otherwise it is redundant to check if fred is null at line 2\n", errout.str());

        // #2789 - assign and check pointer
        check("void f() {\n"
              "    char *p;\n"
              "    if (!(p=x())) { }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference: p - otherwise it is redundant to check if p is null at line 3\n", errout.str());

        // check, assign and use
        check("void f() {\n"
              "    char *p;\n"
              "    if (p == 0 && (p = malloc(10)) != 0) {\n"
              "        *p = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Test CheckNullPointer::nullConstantDereference
    void nullConstantDereference()
    {
        // Ticket #2090
        check("void foo() {\n"
              "  char *p = 0;\n"
              "  strcpy(p, \"abcd\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Null pointer dereference\n", errout.str());

        // Ticket #2413 - it's ok to pass NULL to fflush
        check("void foo() {\n"
              "  fflush(NULL);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #3126 - don't confuse member function with standard function
        check("void f() {\n"
              "    image1.fseek(0, SEEK_SET);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void gcc_statement_expression()
    {
        // Ticket #2621
        check("void f(struct ABC *abc) {\n"
              "    ({ if (abc) dbg(); })\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf_with_zero_size()
    {
        // Ticket #2840
        check("void f() {\n"
              "    int bytes = snprintf(0, 0, \"%u\", 1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf_with_non_zero_size()
    {
        // Ticket #2840
        check("void f() {\n"
              "    int bytes = snprintf(0, 10, \"%u\", 1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Null pointer dereference\n", errout.str());
    }
};

REGISTER_TEST(TestNullPointer)

