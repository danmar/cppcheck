/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include "checkuninitvar.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestUninitVar : public TestFixture {
public:
    TestUninitVar() : TestFixture("TestUninitVar")
    { }

private:


    void run() {
        TEST_CASE(uninitvar1);
        TEST_CASE(uninitvar_bitop);		// using uninitialized operand in bit operation
        TEST_CASE(uninitvar_alloc);     // data is allocated but not initialized
        TEST_CASE(uninitvar_arrays);    // arrays
        TEST_CASE(uninitvar_class);     // class/struct
        TEST_CASE(uninitvar_enum);      // enum variables
        TEST_CASE(uninitvar_if);        // handling if
        TEST_CASE(uninitvar_loops);     // handling for/while
        TEST_CASE(uninitvar_switch);    // handling switch
        TEST_CASE(uninitvar_references); // references
        TEST_CASE(uninitvar_return);    // return
        TEST_CASE(uninitvar_strncpy);   // strncpy doesn't always 0-terminate
        TEST_CASE(uninitvar_memset);    // not 0-terminated
        TEST_CASE(uninitvar_func);      // analyse functions
        TEST_CASE(func_uninit_var);     // analyse function calls for: 'int a(int x) { return x+x; }'
        TEST_CASE(func_uninit_pointer); // analyse function calls for: 'void a(int *p) { *p = 0; }'
        TEST_CASE(uninitvar_typeof);    // typeof

        // checking for uninitialized variables without using the
        // ExecutionPath functionality
        TEST_CASE(uninitvar2);
        TEST_CASE(uninitvar3);          // #3844
        TEST_CASE(uninitvar4);          // #3869 (reference)
        TEST_CASE(uninitvar5);          // #3861
        TEST_CASE(uninitvar6);          // handling unknown types in C and C++ files
    }

    void checkUninitVar(const char code[], const char filename[] = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyTokenList();

        // Check code..
        CheckUninitVar check(&tokenizer, &settings, this);
        check.executionPaths();
    }

    void uninitvar1() {
        // Ticket #2207 - False negative
        checkUninitVar("void foo() {\n"
                       "    int a;\n"
                       "    b = c - a;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("void foo() {\n"
                       "    int a;\n"
                       "    b = a - c;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("void foo() {\n"
                       "    int *p;\n"
                       "    realloc(p,10);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: p\n", errout.str());

        checkUninitVar("void foo() {\n"
                       "    int *p = NULL;\n"
                       "    realloc(p,10);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

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
                       "    int *p;\n"
                       "    p[0] = 135;\n"
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
                       "    int *x;\n"
                       "    int *&y = x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    int x = xyz::x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

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
                       "    extern int a;\n"
                       "    a++;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

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
                       "    i++;\n"
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

        // Ticket #3597
        checkUninitVar("int f() {\n"
                       "	int a;\n"
                       "	int b = 1;\n"
                       "	(b += a) = 1;\n"
                       "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n","", errout.str());

        checkUninitVar("int f() {\n"
                       "	int a,b,c;\n"
                       "	a = b = c;\n"
                       "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: c\n", "", errout.str());

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

        // Unknown types
        {
            checkUninitVar("void a()\n"
                           "{\n"
                           "    A ret;\n"
                           "    return ret;\n"
                           "}\n");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar("void a()\n"
                           "{\n"
                           "    A ret;\n"
                           "    return ret;\n"
                           "}\n",
                           "test.c");
            TODO_ASSERT_EQUALS("[test.c:4]: (error) Uninitialized variable: ret\n", "", errout.str());

            // #3916 - avoid false positive
            checkUninitVar("void f(float x) {\n"
                           "  union lf { long l; float f; } u_lf;\n"
                           "  float hx = (u_lf.f = (x), u_lf.l);\n"
                           "}\n",
                           "test.c");
            ASSERT_EQUALS("", errout.str());
        }

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

        // Handling >> and <<
        {
            checkUninitVar("int a() {\n"
                           "    int ret;\n"
                           "    std::cin >> ret;\n"
                           "    ret++;\n"
                           "}\n");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar("int a() {\n"
                           "    int ret;\n"
                           "    int a = value >> ret;\n"
                           "    ret++;\n"
                           "}\n",
                           "test.c");
            ASSERT_EQUALS("[test.c:3]: (error) Uninitialized variable: ret\n", errout.str());

            checkUninitVar("void foo() {\n"   // #3707
                           "    Node node;\n"
                           "    int x;\n"
                           "    node[\"abcd\"] >> x;\n"
                           "}\n");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar("int a(FArchive &arc) {\n"   // #3060 (initialization through operator<<)
                           "    int *p;\n"
                           "    arc << p;\n"
                           "    return *p;\n"
                           "}\n");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar("int a() {\n"
                           "    int ret;\n"
                           "    int a = value << ret;\n"
                           "    return ret;\n"
                           "}\n",
                           "test.c");
            ASSERT_EQUALS("[test.c:3]: (error) Uninitialized variable: ret\n", errout.str());
        }

        checkUninitVar("void a() {\n"   // asm
                       "    int x;\n"
                       "    asm();\n"
                       "    x++;\n"
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
                       "    key++;\n"
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

        checkUninitVar("void f(int a)\n"
                       "{\n"
                       "    if (a) {\n"
                       "        char *p;\n"
                       "        *p = 0;\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: p\n", errout.str());

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
                       "    a[0] = 10 - a[1];\n"
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

        // Ticket #3480 - Don't crash garbage code
        checkUninitVar("int f()\n"
                       "{\n"
                       "    return if\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #3873 (false positive)
        checkUninitVar("MachineLoopRange *MachineLoopRanges::getLoopRange(const MachineLoop *Loop) {\n"
                       "  MachineLoopRange *&Range = Cache[Loop];\n"
                       "  if (!Range)\n"
                       "	Range = new MachineLoopRange(Loop, Allocator, *Indexes);\n"
                       "  return Range;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitvar3() { // #3844
        // avoid false positive
        checkUninitVar("namespace std _GLIBCXX_VISIBILITY(default)\n"
                       "{\n"
                       "_GLIBCXX_BEGIN_NAMESPACE_CONTAINER\n"
                       "  typedef unsigned long _Bit_type;\n"
                       "  struct _Bit_reference\n"
                       "  {\n"
                       "	 _Bit_type * _M_p;\n"
                       "	 _Bit_type _M_mask;\n"
                       "	 _Bit_reference(_Bit_type * __x, _Bit_type __y)\n"
                       "	 : _M_p(__x), _M_mask(__y) { }\n"
                       "  };\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitvar_bitop() {
        checkUninitVar("void foo() {\n"
                       "    int b;\n"
                       "    c = a | b;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: b\n", errout.str());

        checkUninitVar("void foo() {\n"
                       "    int b;\n"
                       "    c = b | a;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: b\n", errout.str());
    }

    // if..
    void uninitvar_if() {
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

        checkUninitVar("void f() {\n"
                       "   char a[10];\n"
                       "   if (a[0] = x){}\n"
                       "}");
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

        // Ticket #2207 - False positive
        checkUninitVar("void foo(int x) {\n"
                       "    int a;\n"
                       "    if (x)\n"
                       "        a = 1;\n"
                       "    if (!x)\n"
                       "        return;\n"
                       "    b = (c - a);\n"
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

        checkUninitVar("int test(int cond1, int cond2) {\n"
                       "  int foo;\n"
                       "  if (cond1 || cond2) {\n"
                       "     if (cond2)\n"
                       "        foo = 0;\n"
                       "  }\n"
                       "  if (cond2) {\n"
                       "    int t = foo*foo;\n"
                       "  }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // ? :
        checkUninitVar("static void foo(int v)\n"
                       "{\n"
                       "    int x;\n"
                       "    x = v <= 0 ? -1 : x;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: x\n", errout.str());

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
    void uninitvar_loops() {
        // for..
        checkUninitVar("void f()\n"
                       "{\n"
                       "    for (int i = 0; i < 4; ++i) {\n"
                       "        int a;\n"
                       "        b(4*a);\n"
                       "    }"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("void f() {\n"
                       "    int k;\n"
                       "    for (int i = 0; i < 4; ++i) {\n"
                       "        k = k + 2;\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: k\n", errout.str());

        checkUninitVar("void f() {\n"
                       "    gchar sel[10];\n"
                       "    for (int i = 0; i < 4; ++i) {\n"
                       "        int sz = sizeof(sel);\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("enum ABCD { A, B, C, D };\n"
                       "\n"
                       "static void f(char *str ) {\n"
                       "    enum ABCD i;\n"
                       "    for (i = 0; i < D; i++) {\n"
                       "        str[i] = 0;\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void x() {\n"
                       "    do  {\n"
                       "        Token * tok;\n"
                       "        for (tok = a; tok; tok = tok->next())\n"
                       "        {\n"
                       "        }\n"
                       "    } while (tok2);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

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

        // Ticket #2226: C++0x loop
        checkUninitVar("void f() {\n"
                       "    container c;\n"
                       "    for (iterator it : c) {\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #2345: False positive in sub-condition in if inside a loop
        checkUninitVar("void f(int x) {\n"
                       "    const PoolItem* pItem;\n"
                       "    while (x > 0) {\n"
                       "        if (GetItem(&pItem) && (*pItem != rPool))\n"
                       "        { }\n"
                       "        x--;\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
        checkUninitVar("void f(int x) {\n"
                       "    const PoolItem* pItem;\n"
                       "    while (x > 0) {\n"
                       "        if (*pItem != rPool)\n"
                       "        { }\n"
                       "        x--;\n"
                       "    }\n"
                       "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: pItem\n",
                           "", errout.str());

        // #2231 - conditional initialization in loop..
        checkUninitVar("int foo(char *a) {\n"
                       "    int x;\n"
                       "\n"
                       "    for (int i = 0; i < 10; ++i) {\n"
                       "        if (a[i] == 'x') {\n"
                       "            x = i;\n"
                       "            break;\n"
                       "        }\n"
                       "    }\n"
                       "\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Uninitialized variable: x\n", errout.str());

        // Ticket #2796
        checkUninitVar("void foo() {\n"
                       "    while (true) {\n"
                       "        int x;\n"
                       "        if (y) x = 0;\n"
                       "        else break;\n"
                       "        return x;\n"   // <- x is initialized
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        // Assignment in for. Ticket #3287
        checkUninitVar("int foo(char* in, bool b) {\n"
                       "    char* c;\n"
                       "    if (b) for (c = in; *c == 0; ++c) {}\n"
                       "    else c = in + strlen(in) - 1;\n"
                       "    *c = 0;\n"
                       "}\n");
        TODO_ASSERT_EQUALS("", "[test.cpp:5]: (error) Uninitialized variable: c\n", errout.str());
    }

    // switch..
    void uninitvar_switch() {
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

        // #3231 - ({ switch .. })
        checkUninitVar("void f() {\n"
                       "    int a;\n"
                       "    ({\n"
                       "    switch(sizeof(int)) {\n"
                       "    case 4:\n"
                       "    default:\n"
                       "        (a)=0;\n"
                       "        break;\n"
                       "    };\n"
                       "    })\n"
                       "}");
        ASSERT_EQUALS("", errout.str());
    }

    // arrays..
    void uninitvar_arrays() {
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
                       "    char a[SIZE+10];\n"
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

        checkUninitVar("void f() {\n"
                       "    char a[10], *p;\n"
                       "    p = &(a[10]);\n"
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

        checkUninitVar("void foo()\n"
                       "{\n"
                       "        int a[2];\n"
                       "        init(a - 1);\n"
                       "        int b = a[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "        Fred a[2];\n"
                       "        Fred b = a[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #2320
        checkUninitVar("void foo() {\n"
                       "        char a[2];\n"
                       "        char *b = (a+2) & 7;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f() {\n"   // Ticket #3050
                       "    char a[2];\n"
                       "    printf(\"%s\", a);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("void f() {\n"    // Ticket #3497
                       "    char header[1];\n"
                       "    *((unsigned char*)(header)) = 0xff;\n"
                       "    return header[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f() {\n"    // Ticket #3497
                       "    char header[1];\n"
                       "    *((unsigned char*)((unsigned char *)(header))) = 0xff;\n"
                       "    return header[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f() {\n"
                       "    ABC abc;\n"
                       "    int a[1];\n"
                       "\n"
                       "    abc.a = a;\n"
                       "    init(&abc);\n"
                       "    return a[0];\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        // ticket #3344
        checkUninitVar("void f(){\n"
                       "   char *strMsg = \"This is a message\";\n"
                       "   char *buffer=(char*)malloc(128*sizeof(char));\n"
                       "   strcpy(strMsg,buffer);\n"
                       "   free(buffer);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: buffer\n", errout.str());

        // #3845
        checkUninitVar("int foo() {\n"
                       "    int a[1] = {5};\n"
                       "    return a[0];\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo() {\n"
                       "    int a[2][2] = {{3,4}, {5,6}};\n"
                       "    return a[0][1];\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("int foo() {\n"
                       "    int a[1];\n"
                       "    return a[0];\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar("int foo() {\n"
                       "    int a[2][2];\n"
                       "    return a[0][1];\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", "", errout.str());
    }

    // alloc..
    void uninitvar_alloc() {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s = malloc(100);\n"
                       "    strcat(s, \"abc\");\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Data is allocated but not initialized: s\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s = malloc(100);\n"
                       "    perror(s);\n"
                       "};");
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

        checkUninitVar("void f()\n"
                       "{\n"
                       "    Fred *fred = malloc(sizeof(Fred));\n"
                       "    x(&fred->f);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    Fred *fred = malloc(sizeof(Fred));\n"
                       "    x(fred->f);\n"
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

        // #3708 - false positive when using ptr typedef
        checkUninitVar("void f() {\n"
                       "    uintptr_t x = malloc(100);\n"
                       "    uintptr_t y = x + 10;\n"  // <- not bad usage
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // class / struct..
    void uninitvar_class() {
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

        checkUninitVar("void f(int x) {\n"
                       "    struct AB ab;\n"
                       "    x = ab.x = 12;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // enum..
    void uninitvar_enum() {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    enum AB { a, b };\n"
                       "    AB ab;\n"
                       "    if (ab);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: ab\n", errout.str());
    }

    // references..
    void uninitvar_references() {
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

    void uninitvar_return() {

        checkUninitVar("static int foo()\n"
                       "{\n"
                       "    int ret;\n"
                       "    return ret;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: ret\n", errout.str());

        checkUninitVar("static int foo()\n"
                       "{\n"
                       "    int ret;\n"
                       "    return ret+5;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: ret\n", errout.str());

        checkUninitVar("static int foo() {\n"
                       "    int ret;\n"
                       "    return ret = 5;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        {
            checkUninitVar("static int foo() {\n"
                           "    int ret;\n"
                           "    return cin >> ret;\n"
                           "}\n");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar("static int foo() {\n"
                           "    int ret;\n"
                           "    return cin >> ret;\n"
                           "}\n", "test.c");
            ASSERT_EQUALS("[test.c:3]: (error) Uninitialized variable: ret\n", errout.str());
        }

        // Ticket #2146 - False negative
        checkUninitVar("int f(int x) {\n"
                       "    int y;\n"
                       "    return x ? 1 : y;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Uninitialized variable: y\n", errout.str());

        // Ticket #3106 - False positive
        {
            checkUninitVar("int f() {\n"
                           "    int i;\n"
                           "    return x(&i) ? i : 0;\n"
                           "}\n");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar("int f() {\n"
                           "    int i;\n"
                           "    return x() ? i : 0;\n"
                           "}\n");
            ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: i\n", errout.str());
        }

    }

    // strncpy doesn't always 0-terminate..
    void uninitvar_strncpy() {
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

        // #3245 - false positive
        {
            checkUninitVar("void f() {\n"
                           "    char a[100];\n"
                           "    strncpy(a,p,10);\n"
                           "    memcmp(a,q,10);\n"
                           "}");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar("void f() {\n"
                           "    char a[100];\n"
                           "    strncpy(a,p,10);\n"
                           "    if (memcmp(a,q,10)==0);\n"
                           "}");
            ASSERT_EQUALS("", errout.str());
        }

        // Using strncpy isn't necessarily dangerous usage
        checkUninitVar("void f(const char dev[], char *str) {\n"
                       "    char buf[10];\n"
                       "    strncpy(buf, dev, 10);\n"
                       "    strncpy(str, buf, 10);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // initialization with memset (not 0-terminating string)..
    void uninitvar_memset() {
        checkUninitVar("void f() {\n"
                       "    char a[20];\n"
                       "    memset(a, 'a', 20);\n"
                       "    strcat(a, s);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Dangerous usage of 'a' (not 0-terminated)\n", errout.str());
    }

    std::string analyseFunctions(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        std::set<std::string> f;
        const CheckUninitVar check((const Tokenizer *)0, (const Settings *)0, (ErrorLogger *)0);
        check.analyse(tokenizer.tokens(), f);

        std::string ret;
        for (std::set<std::string>::const_iterator it = f.begin(); it != f.end(); ++it)
            ret += (ret.empty() ? "" : " ") + *it;
        return ret;
    }

    void uninitvar_func() {
        // function analysis..
        ASSERT_EQUALS("foo", analyseFunctions("void foo(int x) { }"));
        ASSERT_EQUALS("foo", analyseFunctions("void foo(int x);"));
        ASSERT_EQUALS("foo", analyseFunctions("void foo(const int &x) { }"));
        ASSERT_EQUALS("foo", analyseFunctions("void foo(int &x) { ++x; }"));
        ASSERT_EQUALS("rename", analyseFunctions("int rename (const char* oldname, const char* newname);"));	// Ticket #914
        ASSERT_EQUALS("rename", analyseFunctions("int rename (const char oldname[], const char newname[]);"));
        ASSERT_EQUALS("", analyseFunctions("void foo(int &x) { x = 0; }"));
        ASSERT_EQUALS("", analyseFunctions("void foo(s x) { }"));
        // TODO: it's ok to pass a valid pointer to "foo". See #2775 and #2946
        TODO_ASSERT_EQUALS("foo", "", analyseFunctions("void foo(Fred *fred) { fred->x = 0; }"));
        ASSERT_EQUALS("", analyseFunctions("void foo(int *x) { x[0] = 0; }"));

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
                       "    int i;\n"
                       "    x(i+2);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: i\n", errout.str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *p = malloc(10);\n"
                       "    read(p + 1);\n"
                       "    return p;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

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

        checkUninitVar("int calc(const int *p, int n);\n"
                       "void f() {\n"
                       "    int x[10];\n"
                       "    calc(x,10);\n"
                       "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: x\n",
                           "", errout.str());

        // #2401 - unknown function/macro might init the variable
        checkUninitVar("int f() {\n"
                       "    int x;\n"
                       "    INIT(x);\n"
                       "    return x;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // #3222 - calling function through function pointer
        checkUninitVar("char f() {\n"
                       "    char buffer[100];\n"
                       "    (foo.init)(buffer);\n"
                       "    return buffer[0];\n"
                       "}");
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

        checkUninitVar("void (*init)(char *str);\n"
                       "\n"
                       "char x() {\n"
                       "    char cmd[10];\n"
                       "    init(cmd);\n"
                       "    return cmd[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("char fn(FILE *f) {\n"
                       "    char buf[10];\n"
                       "    fread(buf, 1, 10, f);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // #2775 - uninitialized struct pointer in subfunction
        checkUninitVar("void a(struct Fred *fred) {\n"
                       "    fred->x = 0;\n"
                       "}\n"
                       "\n"
                       "void b() {\n"
                       "    struct Fred *p;\n"
                       "    a(p);\n"
                       "}\n");
        // TODO: See #2946
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Uninitialized variable: p\n", "", errout.str());

        // #2946 - FP array is initialized in subfunction
        checkUninitVar("void a(char *buf) {\n"
                       "    buf[0] = 0;\n"
                       "}\n"
                       "void b() {\n"
                       "    char buf[10];\n"
                       "    a(buf);\n"
                       "    buf[1] = buf[0];\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        // #3159 - initialization by function
        checkUninitVar("static int isnumber(const char *arg) {\n"
                       "    char *p;\n"
                       "    return strtod(arg, &p) != 0 || p != arg;\n"
                       "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("static int isnumber(const char *arg) {\n"
                       "    char *p;\n"
                       "    return strtod(&arg) != 0 || p != arg;\n"
                       "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());
    }

    // valid and invalid use of 'int a(int x) { return x + x; }'
    void func_uninit_var() {
        const std::string funca("int a(int x) { return x + x; }\n");

        checkUninitVar((funca +
                        "void b() {\n"
                        "    int x;\n"
                        "    a(x);\n"
                        "}").c_str());
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar((funca +
                        "void b() {\n"
                        "    int *p;\n"
                        "    a(*p);\n"
                        "}").c_str());
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());
    }


    // valid and invalid use of 'void a(int *p) { *p = 0; }'
    void func_uninit_pointer() {
        const std::string funca("void a(int *p) { *p = 0; }\n");

        // ok - initialized pointer
        checkUninitVar((funca +
                        "void b() {\n"
                        "    int buf[10];\n"
                        "    a(buf);\n"
                        "}\n").c_str());
        ASSERT_EQUALS("", errout.str());

        // not ok - uninitialized pointer
        checkUninitVar((funca +
                        "void b() {\n"
                        "    int *p;\n"
                        "    a(p);\n"
                        "}\n").c_str());
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: p\n", errout.str());
    }

    void uninitvar_typeof() {
        checkUninitVar("void f() {\n"
                       "    struct Fred *fred;\n"
                       "    typeof(fred->x);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f() {\n"
                       "    struct SData * s;\n"
                       "    ab(typeof(s->status));\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("void f() {\n"
                       "    struct SData * s;\n"
                       "    TYPEOF(s->status);\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }



    /** New checking that doesn't rely on ExecutionPath */
    void checkUninitVar2(const char code[], const char fname[] = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, fname);
        tokenizer.simplifyTokenList();

        // Check for redundant code..
        CheckUninitVar checkuninitvar(&tokenizer, &settings, this);
        checkuninitvar.check();
    }

    void uninitvar2() {
        // using uninit var
        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    x++;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    str[x] = 0;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    int y = x & 3;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    int y = 3 & x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar2("void f() {\n"
                        "    struct ABC *abc;\n"
                        "    abc->a = 0;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: abc\n", errout.str());

        checkUninitVar2("int f() {\n"
                        "    static int x;\n"
                        "    return ++x;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("int f() {\n"
                        "    extern int x;\n"
                        "    return ++x;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"  // #3926 - weird cast.
                        "    int x;\n"
                        "    *(((char *)&x) + 0) = 0;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // using uninit var in condition
        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    if (1 == (3 & x)) { }\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: x\n", errout.str());

        // ?:
        checkUninitVar2("int f(int *ptr) {\n"
                        "    int a;\n"
                        "    int *p = ptr ? ptr : &a;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // = { .. }
        checkUninitVar2("int f() {\n"
                        "    int a;\n"
                        "    int *p[] = { &a };\n"
                        "    *p[0] = 0;\n"
                        "    return a;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // >> => initialization / usage
        {
            const char code[] = "void f() {\n"
                                "    int x;\n"
                                "    if (i >> x) { }\n"
                                "}";

            checkUninitVar2(code, "test.cpp");
            ASSERT_EQUALS("", errout.str());

            checkUninitVar2(code, "test.c");
            ASSERT_EQUALS("[test.c:3]: (error) Uninitialized variable: x\n", errout.str());
        }

        checkUninitVar2("void f() {\n"
                        "    int i, i2;\n"
                        "    strm >> i >> i2;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        // unconditional initialization
        checkUninitVar2("int f() {\n"
                        "    int ret;\n"
                        "    if (a) { ret = 1; }\n"
                        "    else { {} ret = 2; }\n"
                        "    return ret;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // conditional initialization
        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { x = 1; }\n"
                        "    else if (y == 2) { x = 1; }\n"
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { x = 1; }\n"
                        "    else if (y == 2) { x = 1; }\n"
                        "    if (y == 3) { }\n"   // <- ignore condition
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Uninitialized variable: x\n", errout.str());

        // initialization in condition
        checkUninitVar2("void f() {\n"
                        "    int a;\n"
                        "    if (init(&a)) { }\n"
                        "    a++;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        // return, break, continue, goto
        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { return; }\n"
                        "    if (y == 2) { break; }\n"
                        "    if (y == 3) { continue; }\n"
                        "    else { x = 1; }\n"
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    return;\n"
                        "err:\n"
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int i;\n"
                        "    if (x) {\n"
                        "        i = 1;\n"
                        "    } else {\n"
                        "        goto out;\n"
                        "    }\n"
                        "    i++;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("int f() {\n"
                        "    int i,x;\n"
                        "    for (i=0;i<9;++i)\n"
                        "        if (foo) break;\n"
                        "    return x;\n"
                        "}\n");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        // if goto is simplified there might be conditions that are always true
        checkUninitVar2("void f() {\n"
                        "    int i;\n"
                        "    if (x) {\n"
                        "        int y = -ENOMEM;\n"
                        "        if (y != 0) return;\n"
                        "        i++;\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int i, y;\n"
                        "    if (x) {\n"
                        "        y = -ENOMEM;\n"
                        "        if (y != 0) return;\n"
                        "        i++;\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        // for, while
        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    while (a) {\n"
                        "        if (b) x++;\n"
                        "        else x = 0;\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    for (int i = 0; i < 10; i += x) {\n"
                        "        x = y;\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    for (int i = 0; i < 10; i += x) { }\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: x\n", errout.str());

        checkUninitVar2("int f() {\n"
                        "    int i;\n"
                        "    for (i=0;i<9;++i)\n"
                        "        if (foo()) goto out;\n"
                        "out:\n"
                        "    return i;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        // try
        checkUninitVar2("void f() {\n"
                        "    int i, *p = &i;\n"
                        "    try { }\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // exit
        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { exit(0); }\n"
                        "    else { x = 1; }\n"
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        // strange code.. don't crash (#3415)
        checkUninitVar2("void foo() {\n"
                        "    int i;\n"
                        "    ({ if (0); });\n"
                        "    for_each(i) { }\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // if, if
        checkUninitVar2("void f(int a) {\n"
                        "    int i;\n"
                        "    if (a) i = 0;\n"
                        "    if (a) i++;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int a,b=0;\n"
                        "    if (x) {\n"
                        "        if (y) {\n"
                        "            a = 0;\n"
                        "            b = 1;\n"
                        "        }\n"
                        "    }\n"
                        "    if (b) a++;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int a=0, b;\n"
                        "    if (x) { }\n"
                        "    else if (y==2) { a=1; b=2; }\n"
                        "    if (a) { ++b; }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("static void f(int x, int y) {\n"
                        "    int a;\n"
                        "    if (x == 0) { a = y; }\n"
                        "    if (x == 0 && (a == 1)) { }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("static void f(int x, int y) {\n"
                        "    int a;\n"
                        "    if (x == 0 && (a == 1)) { }\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int a;\n"
                        "    if (x) { a = 0; }\n"
                        "    if (x) { if (y) { a++; } }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int a;\n"
                        "    if (x) { a = 0; }\n"
                        "    if (x) { if (y) { } else { a++; } }\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        // asm
        checkUninitVar2("void f() {\n"
                        "    int x;\n"
                        "    asm();\n"
                        "    x++;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // sizeof / typeof / offsetof / etc
        checkUninitVar2("void f() {\n"
                        "    int i;\n"
                        "    sizeof(i+1);\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int i;\n"
                        "    if (100 == sizeof(i+1));\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    struct ABC *abc;\n"
                        "    int i = ARRAY_SIZE(abc.a);"
                        "}");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar2("void f() {\n"
                        "    int *abc;\n"
                        "    typeof(*abc);\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #3486 - Don't crash garbage code
        checkUninitVar2("void f()\n"
                        "{\n"
                        "  (\n"
                        "    x;\n"
                        "    int a, a2, a2*x; if () ;\n"
                        "  )\n"
                        "}\n");

        // Ticket #3890 - False positive for std::map
        checkUninitVar2("void f() {\n"
                        "    std::map<int,bool> x;\n"
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());

        // Ticket #3906 - False positive for std::vector pointer
        checkUninitVar2("void f() {\n"
                        "    std::vector<int> *x = NULL;\n"
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // #3869 - reference variable
    void uninitvar4() {
        checkUninitVar2("void f() {\n"
                        "    int buf[10];\n"
                        "    int &x = buf[0];\n"
                        "    buf[0] = 0;\n"
                        "    x++;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());
    }

    // #3861
    void uninitvar5() {
        // ensure there is no false positive
        checkUninitVar2("void f() {\n"
                        "    x<char> c;\n"
                        "    c << 2345;\n"
                        "}");
        ASSERT_EQUALS("", errout.str());

        // ensure there is no false negative
        checkUninitVar2("void f() {\n"
                        "    char c;\n"
                        "    char a = c << 2;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: c\n", errout.str());
    }

    // Handling of unknown types. Assume they are POD in C.
    void uninitvar6() {
        const char code[] = "void f() {\n"
                            "    dfs a;\n"
                            "    return a;\n"
                            "}";

        // Assume dfs is a non POD type if file is C++
        checkUninitVar2(code, "test.cpp");
        ASSERT_EQUALS("", errout.str());

        // Assume dfs is a POD type if file is C
        checkUninitVar2(code, "test.c");
        ASSERT_EQUALS("[test.c:3]: (error) Uninitialized variable: a\n", errout.str());
    }
};

REGISTER_TEST(TestUninitVar)

