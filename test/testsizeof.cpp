/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "checksizeof.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestSizeof : public TestFixture {
public:
    TestSizeof() : TestFixture("TestSizeof") {
    }

private:


    void run() {
        TEST_CASE(sizeofsizeof);
        TEST_CASE(sizeofCalculation);
        TEST_CASE(checkPointerSizeof);
        TEST_CASE(sizeofForArrayParameter);
        TEST_CASE(sizeofForNumericParameter);
        TEST_CASE(suspiciousSizeofCalculation);
        TEST_CASE(sizeofVoid);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("warning");
        settings.addEnabled("portability");
        settings.inconclusive = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check...
        CheckSizeof checkSizeof(&tokenizer, &settings, this);
        checkSizeof.runChecks(&tokenizer, &settings, this);
    }

    void sizeofsizeof() {
        check("void foo()\n"
              "{\n"
              "    int i = sizeof sizeof char;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Calling 'sizeof' on 'sizeof'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int i = sizeof (sizeof long);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Calling 'sizeof' on 'sizeof'.\n", errout.str());

        check("void foo(long *p)\n"
              "{\n"
              "    int i = sizeof (sizeof (p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Calling 'sizeof' on 'sizeof'.\n", errout.str());
    }

    void sizeofCalculation() {
        check("int a, b; int a,sizeof(a+b)");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Found calculation inside sizeof().\n", errout.str());

        check("int a, b; sizeof(a*b)");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Found calculation inside sizeof().\n", errout.str());

        check("int a, b; sizeof(-a)");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Found calculation inside sizeof().\n", errout.str());

        check("int a, b; sizeof(*a)");
        ASSERT_EQUALS("", errout.str());

        check("sizeof(void * const)");
        ASSERT_EQUALS("", errout.str());

        check("sizeof(foo++)");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Found calculation inside sizeof().\n", errout.str());

        check("sizeof(--foo)");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Found calculation inside sizeof().\n", errout.str());
    }

    void sizeofForArrayParameter() {
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
        ASSERT_EQUALS("[test.cpp:2]: (error) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer.\n", errout.str());

        check("void f( int a[]) {\n"
              "    std::cout << sizeof a / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer.\n", errout.str());

        check("void f( int a[3] ) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer.\n", errout.str());

        check("void f(int *p) {\n"
              "    p[0] = 0;\n"
              "    int unused = sizeof(p);\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char p[] = \"test\";\n"
              "    int unused = sizeof(p);\n"
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
        ASSERT_EQUALS("[test.cpp:2]: (error) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer.\n", errout.str());

        // ticket #2510
        check("void f( int a[3] , int b[2] ) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (error) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer.\n", errout.str());

        // ticket #2510
        check("void f() {\n"
              "    char buff1[1024*64],buff2[sizeof(buff1)*(2+1)];\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

    }

    void sizeofForNumericParameter() {
        check("void f() {\n"
              "    std::cout << sizeof(10) << std::endl;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter.\n", errout.str());

        check("void f() {\n"
              "    std::cout << sizeof(-10) << std::endl;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter.\n", errout.str());

        check("void f() {\n"
              "    std::cout << sizeof 10  << std::endl;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter.\n", errout.str());

        check("void f() {\n"
              "    std::cout << sizeof -10  << std::endl;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter.\n", errout.str());
    }

    void suspiciousSizeofCalculation() {
        check("int* p;\n"
              "return sizeof(p)/5;");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Division of result of sizeof() on pointer type.\n", errout.str());

        check("unknown p;\n"
              "return sizeof(p)/5;");
        ASSERT_EQUALS("", errout.str());

        check("return sizeof(unknown)/5;");
        ASSERT_EQUALS("", errout.str());

        check("int p;\n"
              "return sizeof(p)/5;");
        ASSERT_EQUALS("", errout.str());

        check("int* p[5];\n"
              "return sizeof(p)/5;");
        ASSERT_EQUALS("", errout.str());


        check("return sizeof(foo)*sizeof(bar);");
        ASSERT_EQUALS("[test.cpp:1]: (warning, inconclusive) Multiplying sizeof() with sizeof() indicates a logic error.\n", errout.str());

        check("return (foo)*sizeof(bar);");
        ASSERT_EQUALS("", errout.str());

        check("return sizeof(foo)*bar;");
        ASSERT_EQUALS("", errout.str());
    }

    void checkPointerSizeof() {
        check("void f() {\n"
              "    char *x = malloc(10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(*x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(&x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(100 * sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(x) * 100);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof *x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(100 * sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof(*x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof *x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof(int));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char x[10];\n"
              "    memset(x, 0, sizeof(x));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char* x[10];\n"
              "    memset(x, 0, sizeof(x));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char x[10];\n"
              "    memset(x, 0, sizeof x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof(int));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof(*x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof *x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof(x) * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof x * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Size of pointer 'x' used instead of size of its data.\n", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof(*x) * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof *x * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof(int) * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "int fun(const char *buf1)\n"
            "{\n"
            "  const char *buf1_ex = \"foobarbaz\";\n"
            "  return strncmp(buf1, buf1_ex, sizeof(buf1_ex)) == 0;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning, inconclusive) Size of pointer 'buf1_ex' used instead of size of its data.\n", errout.str());

        check(
            "int fun(const char *buf1) {\n"
            "  return strncmp(buf1, foo(buf2), sizeof(buf1)) == 0;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Size of pointer 'buf1' used instead of size of its data.\n", errout.str());

        // #ticket 3874
        check("void f()\n"
              "{\n"
              " int * pIntArray[10];\n"
              " memset(pIntArray, 0, sizeof(pIntArray));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sizeofVoid() {
        check("void f() {\n"
              "  int size = sizeof(void);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Behaviour of 'sizeof(void)' is not covered by the ISO C standard.\n", errout.str());

        check("void f() {\n"
              "  void* p;\n"
              "  int size = sizeof(*p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) '*p' is of type 'void', the behaviour of 'sizeof(void)' is not covered by the ISO C standard.\n", errout.str());

        check("void f() {\n"
              "  void* p = malloc(10);\n"
              "  int* p2 = p + 4;\n"
              "  int* p3 = p - 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) 'p' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n"
                      "[test.cpp:4]: (portability) 'p' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());

        check("void f() {\n"
              "  void* p1 = malloc(10);\n"
              "  void* p2 = malloc(5);\n"
              "  p1--;\n"
              "  p2++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (portability) 'p1' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n"
                      "[test.cpp:5]: (portability) 'p2' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());

        check("void f() {\n"
              "  void** p1;\n"
              "  int j = sizeof(*p1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  void* p1[5];\n"
              "  int j = sizeof(*p1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Calculations on void* with casts

        check("void f(void *data) {\n"
              "  *((unsigned char *)data + 1) = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(void *data) {\n"
              "  *((unsigned char *)(data) + 1) = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(void *data) {\n"
              "  unsigned char* c = (unsigned char *)(data + 1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());

        check("void f(void *data) {\n"
              "  unsigned char* c = (unsigned char *)data++;\n"
              "  unsigned char* c2 = (unsigned char *)++data;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n"
                      "[test.cpp:3]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());

        check("void f(void *data) {\n"
              "  void* data2 = (void *)data + 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());

        // #4908 (void pointer as a member of a struct/class)
        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "char f(struct FOO foo) {\n"
              "  char x = *((char*)(foo.data+1));\n"
              "  foo.data++;\n"
              "  return x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (portability) 'foo.data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n"
                      "[test.cpp:6]: (portability) 'foo.data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());

        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "char f(struct FOO foo) {\n"
              "  char x = *((char*)foo.data+1);\n"
              "  return x;\n"
              "}\n"
              "char f2(struct FOO foo) {\n"
              "  char x = *((char*)((FOO)foo).data + 1);\n"
              "  return x;\n"
              "}\n"
              "char f3(struct FOO* foo) {\n"
              "  char x = *((char*)foo->data + 1);\n"
              "  return x;\n"
              "}\n"
              "struct BOO {\n"
              "  FOO data;\n"
              "};\n"
              "void f4(struct BOO* boo) {\n"
              "  char c = *((char*)boo->data.data + 1);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "char f(struct FOO* foo) {\n"
              "  *(foo[1].data + 1) = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (portability) 'foo[1].data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());

        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "void f2(struct FOO* foo) {\n"
              "  (foo[0]).data++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (portability) '(foo[0]).data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.\n", errout.str());
    }

};

REGISTER_TEST(TestSizeof)

