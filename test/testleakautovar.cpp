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
#include "checkleakautovar.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestLeakAutoVar : public TestFixture {
public:
    TestLeakAutoVar() : TestFixture("TestLeakAutoVar") {
    }

private:

    void run() {
        // Assign
        TEST_CASE(assign1);
        TEST_CASE(assign2);
        TEST_CASE(assign3);
        TEST_CASE(assign4);
        TEST_CASE(assign5);
        TEST_CASE(assign6);
        TEST_CASE(assign7);
        TEST_CASE(assign8);
        TEST_CASE(assign9);
        TEST_CASE(assign10);
        TEST_CASE(assign11); // #3942: x = a(b(p));
        TEST_CASE(assign12); // #4236: FP. bar(&x);
        TEST_CASE(assign13); // #4237: FP. char*&ref=p; p=malloc(10); free(ref);

        TEST_CASE(deallocuse1);
        TEST_CASE(deallocuse2);
        TEST_CASE(deallocuse3);
        TEST_CASE(deallocuse4);
        TEST_CASE(deallocuse5); // #4018: FP. free(p), p = 0;
        TEST_CASE(deallocuse6); // #4034: FP. x = p = f();

        TEST_CASE(doublefree1);
        TEST_CASE(doublefree2);
        TEST_CASE(doublefree3); // #4914

        // exit
        TEST_CASE(exit1);
        TEST_CASE(exit2);

        // goto
        TEST_CASE(goto1);
        TEST_CASE(goto2);

        // if/else
        TEST_CASE(ifelse1);
        TEST_CASE(ifelse2);
        TEST_CASE(ifelse3);
        TEST_CASE(ifelse4);
        TEST_CASE(ifelse5);
        TEST_CASE(ifelse6); // #3370

        // switch
        TEST_CASE(switch1);

        // loops
        TEST_CASE(loop1);

        // mismatching allocation/deallocation
        TEST_CASE(mismatch_fopen_free);

        // Execution reaches a 'return'
        TEST_CASE(return1);
        TEST_CASE(return2);
        TEST_CASE(return3);
        TEST_CASE(return4);

        // General tests: variable type, allocation type, etc
        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);  // #3954 - reference pointer

        // Execution reaches a 'throw'
        TEST_CASE(throw1);

        // Possible leak => Further configuration is needed for complete analysis
        TEST_CASE(configuration1);
        TEST_CASE(configuration2);
        TEST_CASE(configuration3);
        TEST_CASE(configuration4);

        TEST_CASE(ptrptr);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Settings settings;
        int id = 0;
        while (!settings.library.ismemory(++id));
        settings.library.setalloc("malloc",id);
        settings.library.setdealloc("free",id);
        while (!settings.library.isresource(++id));
        settings.library.setalloc("fopen",id);
        settings.library.setdealloc("fclose",id);
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.c");
        tokenizer.simplifyTokenList2();

        // Check for leaks..
        CheckLeakAutoVar c;
        settings.checkLibrary = true;
        settings.addEnabled("information");
        c.runSimplifiedChecks(&tokenizer, &settings, this);
    }

    void assign1() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p = NULL;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());
    }

    void assign2() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    char *q = p;\n"
              "    free(q);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign3() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    char *q = p + 1;\n"
              "    free(q - 1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign4() {
        check("void f() {\n"
              "    char *a = malloc(10);\n"
              "    a += 10;\n"
              "    free(a - 10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign5() {
        check("void foo()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    list += p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign6() { // #2806 - FP when there is redundant assignment
        check("void foo() {\n"
              "    char *p = malloc(10);\n"
              "    p = strcpy(p,q);\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign7() {
        check("void foo(struct str *d) {\n"
              "    struct str *p = malloc(10);\n"
              "    d->p = p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign8() {  // linux list
        check("void foo(struct str *d) {\n"
              "    struct str *p = malloc(10);\n"
              "    d->p = &p->x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign9() {
        check("void foo() {\n"
              "    char *p = x();\n"
              "    free(p);\n"
              "    p = NULL;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign10() {
        check("void foo() {\n"
              "    char *p;\n"
              "    if (x) { p = malloc(10); }\n"
              "    if (!x) { p = NULL; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign11() { // #3942 - FP for x = a(b(p));
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    x = a(b(p));\n"
              "}");
        ASSERT_EQUALS("[test.c:4]: (information) --check-library: Function b() should have <use>/<ignore> configuration\n", errout.str());
    }

    void assign12() { // #4236: FP. bar(&x)
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    free(p);\n"
              "    bar(&p);\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign13() { // #4237: FP. char *&ref=p; p=malloc(10); free(ref);
        check("void f() {\n"
              "    char *p;\n"
              "    char * &ref = p;\n"
              "    p = malloc(10);\n"
              "    free(ref);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void deallocuse1() {
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Dereferencing 'p' after it is deallocated / released\n", errout.str());

        check("void f(char *p) {\n"
              "    free(p);\n"
              "    char c = *p;\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Dereferencing 'p' after it is deallocated / released\n", errout.str());
    }

    void deallocuse2() {
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    strcpy(a, p);\n"
              "}");
        TODO_ASSERT_EQUALS("error (free,use)", "[test.c:3]: (information) --check-library: Function strcpy() should have <noreturn> configuration\n", errout.str());

        check("void f(char *p) {\n"   // #3041 - assigning pointer when it's used
              "    free(p);\n"
              "    strcpy(a, p=b());\n"
              "}");
        TODO_ASSERT_EQUALS("", "[test.c:3]: (information) --check-library: Function strcpy() should have <noreturn> configuration\n", errout.str());
    }

    void deallocuse3() {
        check("void f(struct str *p) {\n"
              "    free(p);\n"
              "    p = p->next;\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Dereferencing 'p' after it is deallocated / released\n", errout.str());
    }

    void deallocuse4() {
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Returning/dereferencing 'p' after it is deallocated / released\n", errout.str());
    }

    void deallocuse5() {  // #4018
        check("void f(char *p) {\n"
              "    free(p), p = 0;\n"
              "    *p = 0;\n"  // <- Make sure pointer info is reset. It is NOT a freed pointer dereference
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void deallocuse6() {  // #4034
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    x = p = foo();\n"  // <- p is not dereferenced
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void doublefree1() {  // #3895
        check("void f(char *p) {\n"
              "    if (x)\n"
              "        free(p);\n"
              "    else\n"
              "        p = 0;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:6]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());
    }

    void doublefree2() {  // #3891
        check("void *f(int a) {\n"
              "    char *p = malloc(10);\n"
              "    if (a == 2) { free(p); return ((void*)1); }\n"
              "    free(p);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void doublefree3() {  // #4914
        check("void foo() {\n"
              "   bool done = false;\n"
              "   do {\n"
              "       char *bar = malloc(10)\n"
              "       if(condition()) {\n"
              "           free(bar);\n"
              "           continue;\n"
              "       }\n"
              "       done = true;\n"
              "       free(bar)\n"
              "   } while(!done);\n"
              "   return;"
              "}"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void exit1() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    exit(0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void exit2() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    fatal_error();\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (information) --check-library: Function fatal_error() should have <noreturn> configuration\n"
                      "[test.c:4]: (information) --check-library: Function fatal_error() should have <use>/<ignore> configuration\n",
                      errout.str());
    }

    void goto1() {
        check("static void f() {\n"
              "    int err = -ENOMEM;\n"
              "    char *reg = malloc(100);\n"
              "    if (err) {\n"
              "        free(reg);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void goto2() { // #4231
        check("static char * f() {\n"
              "x:\n"
              "    char *p = malloc(100);\n"
              "    if (err) {\n"
              "        free(p);\n"
              "        goto x;\n"
              "    }\n"
              "    return p;\n"  // no error since there is a goto
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse1() {
        check("int f() {\n"
              "    char *p = NULL;\n"
              "    if (x) { p = malloc(10); }\n"
              "    else { return 0; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse2() {
        check("int f() {\n"
              "    char *p = NULL;\n"
              "    if (x) { p = malloc(10); }\n"
              "    else { return 0; }\n"
              "}");
        ASSERT_EQUALS("[test.c:5]: (error) Memory leak: p\n", errout.str());
    }

    void ifelse3() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (!p) { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (p) { } else { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3866 - UNLIKELY
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (UNLIKELY(!p)) { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse4() {
        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x) { p = malloc(10); }\n"
              "    if (x) { free(p); }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x) { p = malloc(10); }\n"
              "    if (!x) { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse5() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (!p && x) { p = malloc(10); }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse6() { // #3370
        check("void f(int x) {\n"
              "    int *a = malloc(20);\n"
              "    if (x)\n"
              "        free(a);\n"
              "    else\n"
              "        a = 0;\n"
              "}");
        ASSERT_EQUALS("[test.c:6]: (error) Memory leak: a\n", errout.str());
    }

    void switch1() {
        check("void f() {\n"
              "    char *p = 0;\n"
              "    switch (x) {\n"
              "    case 123: p = malloc(100); break;\n"
              "    default: return;\n"
              "    }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void loop1() {
        // test the handling of { }
        check("void f() {\n"
              "    char *p;\n"
              "    for (i=0;i<5;i++) { }\n"
              "    if (x) { free(p) }\n"
              "    else { a = p; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch_fopen_free() {
        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    free(f);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Mismatching allocation and deallocation: f\n", errout.str());
    }

    void return1() {
        check("int f() {\n"
              "    char *p = malloc(100);\n"
              "    return 123;\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());
    }

    void return2() {
        check("char *f() {\n"
              "    char *p = malloc(100);\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void return3() {
        check("struct dev * f() {\n"
              "    struct ABC *abc = malloc(100);\n"
              "    return &abc->dev;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void return4() { // ticket #3862
        // avoid false positives
        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        free(p);\n"
              "        throw 1;\n"
              "    }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        delete p;\n"
              "        throw 1;\n"
              "    }\n"
              "    delete p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        delete [] p;\n"
              "        throw 1;\n"
              "    }\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void test1() { // 3809
        check("void f(double*&p) {\n"
              "    p = malloc(0x100);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void test2() { // 3899
        check("struct Fred {\n"
              "    char *p;\n"
              "    void f1() { free(p); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void test3() { // 3954 - reference pointer
        check("void f() {\n"
              "    char *&p = x();\n"
              "    p = malloc(10);\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void throw1() { // 3987 - Execution reach a 'throw'
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    throw 123;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("void f() {\n"
              "    char *p;\n"
              "    try {\n"
              "        p = malloc(10);\n"
              "        throw 123;\n"
              "    } catch (...) { }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void configuration1() {
        // Possible leak => configuration is required for complete analysis
        // The user should be able to "white list" and "black list" functions.

        // possible leak. If the function 'x' deallocates the pointer or
        // takes the address, there is no leak.
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    x(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (information) --check-library: Function x() should have <noreturn> configuration\n"
                      "[test.c:4]: (information) --check-library: Function x() should have <use>/<ignore> configuration\n",
                      errout.str());
    }

    void configuration2() {
        // possible leak. If the function 'x' deallocates the pointer or
        // takes the address, there is no leak.
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    x(&p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (information) --check-library: Function x() should have <noreturn> configuration\n"
                      "[test.c:4]: (information) --check-library: Function x() should have <use>/<ignore> configuration\n",
                      errout.str());
    }

    void configuration3() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (set_data(p)) { }\n"
              "}");
        ASSERT_EQUALS("[test.c:4]: (information) --check-library: Function set_data() should have <use>/<ignore> configuration\n", errout.str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (set_data(p)) { return; }\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (information) --check-library: Function set_data() should have <use>/<ignore> configuration\n"
                      "[test.c:4]: (information) --check-library: Function set_data() should have <use>/<ignore> configuration\n"
                      , errout.str());
    }

    void configuration4() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    int ret = set_data(p);\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("[test.c:4]: (information) --check-library: Function set_data() should have <use>/<ignore> configuration\n", errout.str());
    }

    void ptrptr() {
        check("void f() {\n"
              "    char **p = malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());
    }
};

REGISTER_TEST(TestLeakAutoVar)
