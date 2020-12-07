/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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


#include "checkleakautovar.h"
#include "library.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <simplecpp.h>
#include <tinyxml2.h>
#include <vector>

class TestLeakAutoVar : public TestFixture {
public:
    TestLeakAutoVar() : TestFixture("TestLeakAutoVar") {
    }

private:
    Settings settings;

    void run() OVERRIDE {
        int id = 0;
        while (!Library::ismemory(++id));
        settings.library.setalloc("malloc", id, -1);
        settings.library.setrealloc("realloc", id, -1);
        settings.library.setdealloc("free", id, 1);
        while (!Library::ismemory(++id));
        settings.library.setalloc("socket", id, -1);
        settings.library.setdealloc("close", id, 1);
        while (!Library::isresource(++id));
        settings.library.setalloc("fopen", id, -1);
        settings.library.setrealloc("freopen", id, -1, 3);
        settings.library.setdealloc("fclose", id, 1);
        settings.library.smartPointers.insert("std::shared_ptr");
        settings.library.smartPointers.insert("std::unique_ptr");

        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
        "<def>\n"
        "  <podtype name=\"uint8_t\" sign=\"u\" size=\"1\"/>\n"
        "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        settings.library.load(doc);

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
        // TODO TEST_CASE(assign13); // #4237: FP. char*&ref=p; p=malloc(10); free(ref);
        TEST_CASE(assign14);
        TEST_CASE(assign15);
        TEST_CASE(assign16);
        TEST_CASE(assign17); // #9047
        TEST_CASE(assign18);
        TEST_CASE(assign19);
        TEST_CASE(assign20); // #9187

        TEST_CASE(isAutoDealloc);

        TEST_CASE(realloc1);
        TEST_CASE(realloc2);
        TEST_CASE(realloc3);
        TEST_CASE(freopen1);
        TEST_CASE(freopen2);

        TEST_CASE(deallocuse1);
        TEST_CASE(deallocuse2);
        TEST_CASE(deallocuse3);
        TEST_CASE(deallocuse4);
        // TODO TEST_CASE(deallocuse5); // #4018: FP. free(p), p = 0;
        TEST_CASE(deallocuse6); // #4034: FP. x = p = f();
        TEST_CASE(deallocuse7); // #6467, #6469, #6473
        TEST_CASE(deallocuse8); // #1765

        TEST_CASE(doublefree1);
        TEST_CASE(doublefree2);
        TEST_CASE(doublefree3); // #4914
        TEST_CASE(doublefree4); // #5451 - FP when exit is called
        TEST_CASE(doublefree5); // #5522
        TEST_CASE(doublefree6); // #7685
        TEST_CASE(doublefree7);
        TEST_CASE(doublefree8);
        TEST_CASE(doublefree9);
        TEST_CASE(doublefree10); // #8706

        // exit
        TEST_CASE(exit1);
        TEST_CASE(exit2);
        TEST_CASE(exit3);

        // handling function calls
        TEST_CASE(functioncall1);

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
        TEST_CASE(ifelse7); // #5576 - if (fd < 0)
        TEST_CASE(ifelse8); // #5747 - if (fd == -1)
        TEST_CASE(ifelse9); // #5273 - if (X(p==NULL, 0))
        TEST_CASE(ifelse10); // #8794 - if (!(x!=NULL))
        TEST_CASE(ifelse11); // #8365 - if (NULL == (p = malloc(4)))
        TEST_CASE(ifelse12); // #8340 - if ((*p = malloc(4)) == NULL)
        TEST_CASE(ifelse13); // #8392
        TEST_CASE(ifelse14); // #9130 - if (x == (char*)NULL)
        TEST_CASE(ifelse15); // #9206 - if (global_ptr = malloc(1))
        TEST_CASE(ifelse16); // #9635 - if (p = malloc(4), p == NULL)
        TEST_CASE(ifelse17); //  if (!!(!p))

        // switch
        TEST_CASE(switch1);

        // loops
        TEST_CASE(loop1);

        // mismatching allocation/deallocation
        TEST_CASE(mismatchAllocDealloc);

        TEST_CASE(smartPointerDeleter);
        TEST_CASE(smartPointerRelease);

        // Execution reaches a 'return'
        TEST_CASE(return1);
        TEST_CASE(return2);
        TEST_CASE(return3);
        TEST_CASE(return4);
        TEST_CASE(return5);
        TEST_CASE(return6); // #8282 return {p, p}
        TEST_CASE(return7); // #9343 return (uint8_t*)x
        TEST_CASE(return8);

        // General tests: variable type, allocation type, etc
        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);  // #3954 - reference pointer
        TEST_CASE(test4);  // #5923 - static pointer
        TEST_CASE(test5);  // unknown type

        // Execution reaches a 'throw'
        TEST_CASE(throw1);
        TEST_CASE(throw2);

        // Possible leak => Further configuration is needed for complete analysis
        TEST_CASE(configuration1);
        TEST_CASE(configuration2);
        TEST_CASE(configuration3);
        TEST_CASE(configuration4);

        TEST_CASE(ptrptr);

        TEST_CASE(nestedAllocation);
        TEST_CASE(testKeywords); // #6767

        TEST_CASE(inlineFunction); // #3989

        TEST_CASE(smartPtrInContainer); // #8262

        TEST_CASE(recursiveCountLimit); // #5872 #6157 #9097

        TEST_CASE(functionCallCastConfig); // #9652
    }

    void check(const char code[], bool cpp = false) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, cpp?"test.cpp":"test.c");

        // Check for leaks..
        CheckLeakAutoVar c;
        settings.checkLibrary = true;
        settings.addEnabled("information");
        c.runChecks(&tokenizer, &settings, this);
    }

    void check(const char code[], Settings & settings) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for leaks..
        CheckLeakAutoVar c;
        settings.checkLibrary = true;
        settings.addEnabled("information");
        c.runChecks(&tokenizer, &settings, this);
    }

    void checkP(const char code[], bool cpp = false) {
        // Clear the error buffer..
        errout.str("");

        // Raw tokens..
        std::vector<std::string> files(1, cpp?"test.cpp":"test.c");
        std::istringstream istr(code);
        const simplecpp::TokenList tokens1(istr, files, files[0]);

        // Preprocess..
        simplecpp::TokenList tokens2(files);
        std::map<std::string, simplecpp::TokenList*> filedata;
        simplecpp::preprocess(tokens2, tokens1, files, filedata, simplecpp::DUI());

        // Tokenizer..
        Tokenizer tokenizer(&settings, this);
        tokenizer.createTokens(std::move(tokens2));
        tokenizer.simplifyTokens1("");

        // Check for leaks..
        CheckLeakAutoVar c;
        settings.checkLibrary = true;
        settings.addEnabled("information");
        c.runChecks(&tokenizer, &settings, this);
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
        ASSERT_EQUALS("[test.c:4]: (information) --check-library: Function b() should have <use>/<leak-ignore> configuration\n", errout.str());
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

    void assign14() {
        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = malloc(10))) { }"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());

        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = new char[10])) { }"
              "}", true);
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: p\n", errout.str());
    }

    void assign15() {
        // #8120
        check("void f() {\n"
              "   baz *p;\n"
              "   p = malloc(sizeof *p);\n"
              "   free(p);\n"
              "   p = malloc(sizeof *p);\n"
              "   free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign16() {
        check("void f() {\n"
              "   char *p = malloc(10);\n"
              "   free(p);\n"
              "   if (p=dostuff()) *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign17() { // #9047
        check("void f() {\n"
              "    char *p = (char*)malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());

        check("void f() {\n"
              "    char *p = (char*)(int*)malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());
    }

    void assign18() {
        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = (char*)malloc(10))) { }"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());

        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = (char*)(int*)malloc(10))) { }"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());
    }

    void assign19() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    free((void*)p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign20() { // #9187
        check("void f() {\n"
              "    char *p = static_cast<int>(malloc(10));\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: p\n", errout.str());
    }

    void isAutoDealloc() {
        check("void f() {\n"
              "    char *p = new char[100];"
              "}", true);
        ASSERT_EQUALS("[test.cpp:2]: (error) Memory leak: p\n", errout.str());

        check("void f() {\n"
              "    Fred *fred = new Fred;"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    std::string *str = new std::string;"
              "}", true);
        TODO_ASSERT_EQUALS("[test.cpp:2]: (error) Memory leak: str\n", "", errout.str());

        check("class TestType {\n" // #9028
              "public:\n"
              "    char ca[12];\n"
              "};\n"
              "void f() {\n"
              "    TestType *tt = new TestType();\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: tt\n", errout.str());
    }

    void realloc1() {
        check("void f() {\n"
              "    void *p = malloc(10);\n"
              "    void *q = realloc(p, 20);\n"
              "    free(q)\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc2() {
        check("void f() {\n"
              "    void *p = malloc(10);\n"
              "    void *q = realloc(p, 20);\n"
              "}");
        ASSERT_EQUALS("[test.c:4]: (error) Memory leak: q\n", errout.str());
    }

    void realloc3() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    char *q = (char*) realloc(p, 20);\n"
              "}");
        ASSERT_EQUALS("[test.c:4]: (error) Memory leak: q\n", errout.str());
    }

    void freopen1() {
        check("void f() {\n"
              "    void *p = fopen(name,a);\n"
              "    void *q = freopen(name, b, p);\n"
              "    fclose(q)\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void freopen2() {
        check("void f() {\n"
              "    void *p = fopen(name,a);\n"
              "    void *q = freopen(name, b, p);\n"
              "}");
        ASSERT_EQUALS("[test.c:4]: (error) Resource leak: q\n", errout.str());
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
        ASSERT_EQUALS("[test.c:2] -> [test.c:3]: (error) Returning/dereferencing 'p' after it is deallocated / released\n", errout.str());

        check("void f(char *p) {\n"
              "  if (!p) free(p);\n"
              "  return p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(char *p) {\n"
              "  if (!p) delete p;\n"
              "  return p;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(char *p) {\n"
              "  if (!p) delete [] p;\n"
              "  return p;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(void* p) {\n"
              "   if (a) {\n"
              "      free(p);\n"
              "       return;\n"
              "   }\n"
              "   g(p);\n"
              "   return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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

    void deallocuse7() {  // #6467, #6469, #6473, #6648
        check("struct Foo { int* ptr; };\n"
              "void f(Foo* foo) {\n"
              "    delete foo->ptr;\n"
              "    foo->ptr = new Foo; \n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("struct Foo { int* ptr; };\n"
              "void f(Foo* foo) {\n"
              "    delete foo->ptr;\n"
              "    x = *foo->ptr; \n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (error) Dereferencing 'ptr' after it is deallocated / released\n", errout.str());

        check("void parse() {\n"
              "    struct Buf {\n"
              "        Buf(uint32_t len) : m_buf(new uint8_t[len]) {}\n"
              "        ~Buf() { delete[]m_buf; }\n"
              "        uint8_t *m_buf;\n"
              "    };\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("struct Foo {\n"
              "    Foo();\n"
              "    Foo* ptr;\n"
              "    void func();\n"
              "};\n"
              "void bar(Foo* foo) {\n"
              "    delete foo->ptr;\n"
              "    foo->ptr = new Foo;\n"
              "    foo->ptr->func();\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void foo(void (*conv)(char**)) {\n"
              "  char * ptr=(char*)malloc(42);\n"
              "  free(ptr);\n"
              "  (*conv)(&ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void deallocuse8() {  // #1765
        check("void f() {\n"
              "    int *ptr = new int;\n"
              "    delete(ptr);\n"
              "    *ptr = 0;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (error) Dereferencing 'ptr' after it is deallocated / released\n", errout.str());
    }

    void doublefree1() {  // #3895
        check("void f(char *p) {\n"
              "    if (x)\n"
              "        free(p);\n"
              "    else\n"
              "        p = 0;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3] -> [test.c:6]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:3]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void foo(char *p, char *r) {\n"
            "  free(p);\n"
            "  free(r);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  free(p);\n"
            "  free(r);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  if (x < 3) free(p);\n"
            "  else { if (x > 9) free(p); }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  getNext(&p);\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  bar();\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:4]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  printf(\"Freed memory at location %x\", p);\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:4]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void foo(FILE *p) {\n"
            "  fclose(p);\n"
            "  fclose(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:3]: (error) Resource handle 'p' freed twice.\n", errout.str());

        check(
            "void foo(FILE *p, FILE *r) {\n"
            "  fclose(p);\n"
            "  fclose(r);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(FILE *p) {\n"
            "  if (x < 3) fclose(p);\n"
            "  else { if (x > 9) fclose(p); }\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(FILE *p) {\n"
            "  fclose(p);\n"
            "  gethandle(&p);\n"
            "  fclose(p);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(FILE *p) {\n"
            "  fclose(p);\n"
            "  gethandle();\n"
            "  fclose(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:4]: (error) Resource handle 'p' freed twice.\n", errout.str());

        check(
            "void foo(Data* p) {\n"
            "  free(p->a);\n"
            "  free(p->b);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void f() {\n"
            "    char *p; p = malloc(100);\n"
            "    if (x) {\n"
            "        free(p);\n"
            "        exit();\n"
            "    }\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void f() {\n"
            "    char *p; p = malloc(100);\n"
            "    if (x) {\n"
            "        free(p);\n"
            "        x = 0;\n"
            "    }\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:4] -> [test.c:7]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void f() {\n"
            "    char *p; p = do_something();\n"
            "    free(p);\n"
            "    p = do_something();\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  delete p;\n"
            "  delete p;\n"
            "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void foo(char *p, char *r) {\n"
            "  delete p;\n"
            "  delete r;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(P p) {\n"
            "  delete p.x;\n"
            "  delete p;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char **p) {\n"
            "  delete p[0];\n"
            "  delete p[1];\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  delete p;\n"
            "  getNext(&p);\n"
            "  delete p;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  delete p;\n"
            "  bar();\n"
            "  delete p;\n"
            "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void foo(char *p) {\n"
            "  delete[] p;\n"
            "  delete[] p;\n"
            "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void foo(char *p, char *r) {\n"
            "  delete[] p;\n"
            "  delete[] r;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  delete[] p;\n"
            "  getNext(&p);\n"
            "  delete[] p;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(char *p) {\n"
            "  delete[] p;\n"
            "  bar();\n"
            "  delete[] p;\n"
            "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "LineMarker::~LineMarker() {\n"
            "  delete pxpm;\n"
            "}\n"
            "LineMarker &LineMarker::operator=(const LineMarker &) {\n"
            "  delete pxpm;\n"
            "  pxpm = NULL;\n"
            "  return *this;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo()\n"
            "{\n"
            "  int* ptr; ptr = NULL;\n"
            "  try\n"
            "    {\n"
            "      ptr = new int(4);\n"
            "    }\n"
            "  catch(...)\n"
            "    {\n"
            "      delete ptr;\n"
            "      throw;\n"
            "    }\n"
            "  delete ptr;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "int foo()\n"
            "{\n"
            "   int* a; a = new int;\n"
            "   bool doDelete; doDelete = true;\n"
            "   if (a != 0)\n"
            "   {\n"
            "       doDelete = false;\n"
            "       delete a;\n"
            "   }\n"
            "   if(doDelete)\n"
            "       delete a;\n"
            "   return 0;\n"
            "}", true);
        TODO_ASSERT_EQUALS("", "[test.cpp:8] -> [test.cpp:11]: (error) Memory pointed to by 'a' is freed twice.\n", errout.str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    while(true) {\n"
            "        x = new char[100];\n"
            "        if (y++ > 100)\n"
            "            break;\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    for (int i = 0; i < 10000; i++) {\n"
            "        x = new char[100];\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", true);
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Memory pointed to by 'x' is freed twice.\n", "", errout.str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    while (isRunning()) {\n"
            "        x = new char[100];\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", true);
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Memory pointed to by 'x' is freed twice.\n", "", errout.str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    while (isRunning()) {\n"
            "        x = malloc(100);\n"
            "        free(x);\n"
            "    }\n"
            "    free(x);\n"
            "}");
        TODO_ASSERT_EQUALS("[test.c:8]: (error) Memory pointed to by 'x' is freed twice.\n", "", errout.str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    for (;;) {\n"
            "        x = new char[100];\n"
            "        if (y++ > 100)\n"
            "            break;\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    do {\n"
            "        x = new char[100];\n"
            "        if (y++ > 100)\n"
            "            break;\n"
            "        delete[] x;\n"
            "    } while (true);\n"
            "    delete[] x;\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void f()\n"
            "{\n"
            "    char *p; p = 0;\n"
            "    if (x < 100) {\n"
            "        p = malloc(10);\n"
            "        free(p);\n"
            "    }\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:6] -> [test.c:8]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());

        check(
            "void MyFunction()\n"
            "{\n"
            "    char* data; data = new char[100];\n"
            "    try\n"
            "    {\n"
            "    }\n"
            "    catch(err)\n"
            "    {\n"
            "        delete[] data;\n"
            "        MyThrow(err);\n"
            "    }\n"
            "    delete[] data;\n"
            "}\n"

            "void MyThrow(err)\n"
            "{\n"
            "    throw(err);\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check(
            "void MyFunction()\n"
            "{\n"
            "    char* data; data = new char[100];\n"
            "    try\n"
            "    {\n"
            "    }\n"
            "    catch(err)\n"
            "    {\n"
            "        delete[] data;\n"
            "        MyExit(err);\n"
            "    }\n"
            "    delete[] data;\n"
            "}\n"

            "void MyExit(err)\n"
            "{\n"
            "    exit(err);\n"
            "}", true);
        ASSERT_EQUALS("", errout.str());

        check( // #6252
            "struct Wrapper {\n"
            "    Thing* m_thing;\n"
            "    Wrapper() : m_thing(0) {\n"
            "    }\n"
            "    ~Wrapper() {\n"
            "        delete m_thing;\n"
            "    }\n"
            "    void changeThing() {\n"
            "        delete m_thing;\n"
            "        m_thing = new Thing;\n"
            "    }\n"
            "};", true);
        ASSERT_EQUALS("", errout.str());

        // #7401
        check("void pCodeLabelDestruct(pCode *pc) {\n"
              "    free(PCL(pc)->label);\n"
              "    free(pc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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

    void doublefree4() {  // #5451 - exit
        check("void f(char *p) {\n"
              "  if (x) {\n"
              "    free(p);\n"
              "    exit(1);\n"
              "  }\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void doublefree5() {  // #5522
        check("void f(char *p) {\n"
              "  free(p);\n"
              "  x = (q == p);\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:4]: (error) Memory pointed to by 'p' is freed twice.\n", errout.str());
    }

    void doublefree6() { // #7685
        check("void do_wordexp(FILE *f) {\n"
              "  free(getword(f));\n"
              "  fclose(f);\n"
              "}", /*cpp=*/false);
        ASSERT_EQUALS("", errout.str());
    }

    void doublefree7() {
        check("void f(char *p, int x) {\n"
              "    free(p);\n"
              "    if (x && (p = malloc(10)))\n"
              "        free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(char *p, int x) {\n"
              "    delete[] p;\n"
              "    if (x && (p = new char[10]))\n"
              "        delete[] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void doublefree8() {
        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x(i);\n"
              "    delete i;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Memory pointed to by 'i' is freed twice.\n", errout.str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    delete i;\n"
              "    std::unique_ptr<int> x(i);\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Memory pointed to by 'i' is freed twice.\n", errout.str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x{i};\n"
              "    delete i;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Memory pointed to by 'i' is freed twice.\n", errout.str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::shared_ptr<int> x(i);\n"
              "    delete i;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Memory pointed to by 'i' is freed twice.\n", errout.str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::shared_ptr<int> x{i};\n"
              "    delete i;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Memory pointed to by 'i' is freed twice.\n", errout.str());

        // Check for use-after-free FP
        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::shared_ptr<int> x{i};\n"
              "    *i = 123;\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int * i = new int[1];\n"
              "    std::unique_ptr<int[]> x(i);\n"
              "    delete i;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Memory pointed to by 'i' is freed twice.\n", errout.str());
    }

    void doublefree9() {
        check("struct foo {\n"
              "    int* get(int) { return new int(); }\n"
              "};\n"
              "void f(foo* b) {\n"
              "    std::unique_ptr<int> x(b->get(0));\n"
              "    std::unique_ptr<int> y(b->get(1));\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void doublefree10() {
        check("void f(char* s) {\n"
              "    char *p = malloc(strlen(s));\n"
              "    if (p != NULL) {\n"
              "        strcat(p, s);\n"
              "        if (strlen(s) != 10)\n"
              "            free(p); p = NULL;\n"
              "    }\n"
              "    if (p != NULL)\n"
              "        free(p);\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(char* s) {\n"
              "    char *p = malloc(strlen(s));\n"
              "    if (p != NULL) {\n"
              "        strcat(p, s);\n"
              "        if (strlen(s) != 10)\n"
              "            free(p), p = NULL;\n"
              "    }\n"
              "    if (p != NULL)\n"
              "        free(p);\n"
              "}\n", true);
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
                      "[test.c:4]: (information) --check-library: Function fatal_error() should have <use>/<leak-ignore> configuration\n",
                      errout.str());
    }

    void exit3() {
        check("void f() {\n"
              "  char *p = malloc(100);\n"
              "  if (x) {\n"
              "    free(p);\n"
              "    ::exit(0);\n"
              "  }"
              "  free(p);\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  char *p = malloc(100);\n"
              "  if (x) {\n"
              "    free(p);\n"
              "    std::exit(0);\n"
              "  }"
              "  free(p);\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void functioncall1() {
        check("void f(struct S *p) {\n"
              "  p->x = malloc(10);\n"
              "  free(p->x);\n"
              "  p->x = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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

        check("char * f(size_t size) {"
              "    void *p = malloc(1);"
              "    if (!p && size != 0)"
              "        return NULL;"
              "    return p;"
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

    void ifelse7() { // #5576
        check("void f() {\n"
              "    int x = malloc(20);\n"
              "    if (x < 0)\n"  // assume negative value indicates its unallocated
              "        return;\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse8() { // #5747
        check("int f() {\n"
              "    int fd = socket(AF_INET, SOCK_PACKET, 0 );\n"
              "    if (fd == -1)\n"
              "        return -1;\n"
              "    return fd;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    int fd = socket(AF_INET, SOCK_PACKET, 0 );\n"
              "    if (fd != -1)\n"
              "        return fd;\n"
              "    return -1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse9() { // #5273
        check("void f() {\n"
              "    char *p = malloc(100);\n"
              "    if (dostuff(p==NULL,0))\n"
              "        return;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse10() { // #8794
        check("void f() {\n"
              "    void *x = malloc(1U);\n"
              "    if (!(x != NULL))\n"
              "        return;\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse11() { // #8365
        check("void f() {\n"
              "    void *p;\n"
              "    if (NULL == (p = malloc(4)))\n"
              "        return;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse12() { // #8340
        check("void f(char **p) {\n"
              "    if ((*p = malloc(4)) == NULL)\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse13() { // #8392
        check("int f(int fd, const char *mode) {\n"
              "    char *path;\n"
              "    if (fd == -1 || (path = (char *)malloc(10)) == NULL)\n"
              "        return 1;\n"
              "    free(path);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f(int fd, const char *mode) {\n"
              "    char *path;\n"
              "    if ((path = (char *)malloc(10)) == NULL || fd == -1)\n"
              "        return 1;\n" // <- memory leak
              "    free(path);\n"
              "    return 0;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4] memory leak", "", errout.str());
    }

    void ifelse14() { // #9130
        check("char* f() {\n"
              "    char* buf = malloc(10);\n"
              "    if (buf == (char*)NULL)\n"
              "        return NULL;\n"
              "    return buf;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse15() { // #9206
        check("struct SSS { int a; };\n"
              "SSS* global_ptr;\n"
              "void test_alloc() {\n"
              "   if ( global_ptr = new SSS()) {}\n"
              "   return;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("FILE* hFile;\n"
              "int openFile( void ) {\n"
              "   if ((hFile = fopen(\"1.txt\", \"wb\" )) == NULL) return 0;\n"
              "   return 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse16() { // #9635
        check("void f(void) {\n"
              "    char *p;\n"
              "    if(p = malloc(4), p == NULL)\n"
              "        return;\n"
              "    free(p);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(void) {\n"
              "    char *p, q;\n"
              "    if(p = malloc(4), q = 1, p == NULL)\n"
              "        return;\n"
              "    free(p);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse17() {
        check("int *f() {\n"
              "    int *p = realloc(nullptr, 10);\n"
              "    if (!p)\n"
              "        return NULL;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int *f() {\n"
              "    int *p = realloc(nullptr, 10);\n"
              "    if (!!(!p))\n"
              "        return NULL;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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

    void mismatchAllocDealloc() {
        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    free(f);\n"
              "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:3]: (error) Mismatching allocation and deallocation: f\n", errout.str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    free((void*)f);\n"
              "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:3]: (error) Mismatching allocation and deallocation: f\n", errout.str());

        check("void f() {\n"
              "    char *cPtr = new char[100];\n"
              "    delete[] cPtr;\n"
              "    cPtr = new char[100]('x');\n"
              "    delete[] cPtr;\n"
              "    cPtr = new char[100];\n"
              "    delete cPtr;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:7]: (error) Mismatching allocation and deallocation: cPtr\n", errout.str());

        check("void f() {\n"
              "    char *cPtr = new char[100];\n"
              "    free(cPtr);\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: cPtr\n", errout.str());

        check("void f() {\n"
              "    char *cPtr = new (buf) char[100];\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int * i = new int[1];\n"
              "    std::unique_ptr<int> x(i);\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: i\n", errout.str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int[]> x(i);\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: i\n", errout.str());

        check("void f() {\n"
              "   void* a = malloc(1);\n"
              "   void* b = freopen(f, p, a);\n"
              "   free(b);\n"
              "}");
        ASSERT_EQUALS("[test.c:2] -> [test.c:3]: (error) Mismatching allocation and deallocation: a\n"
                      "[test.c:3] -> [test.c:4]: (error) Mismatching allocation and deallocation: b\n", errout.str());

        check("void f() {\n"
              "   void* a;\n"
              "   void* b = realloc(a, 10);\n"
              "   free(b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "   int * i = new int;\n"
              "   int * j = realloc(i, 2 * sizeof(int));\n"
              "   delete[] j;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: i\n"
                      "[test.cpp:3] -> [test.cpp:4]: (error) Mismatching allocation and deallocation: j\n", errout.str());
    }

    void smartPointerDeleter() {
        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::unique_ptr<FILE> fp{f};\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: f\n", errout.str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::unique_ptr<FILE, decltype(&fclose)> fp{f, &fclose};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, &fclose};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("struct deleter { void operator()(FILE* f) { fclose(f); }};\n"
              "void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::unique_ptr<FILE, deleter> fp{f};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("int * create();\n"
              "void destroy(int * x);\n"
              "void f() {\n"
              "    int x * = create()\n"
              "    std::unique_ptr<int, decltype(&destroy)> xp{x, &destroy()};\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());

        check("int * create();\n"
              "void destroy(int * x);\n"
              "void f() {\n"
              "    int x * = create()\n"
              "    std::unique_ptr<int, decltype(&destroy)> xp(x, &destroy());\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, [](FILE* x) { fclose(x); }};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, +[](FILE* x) { fclose(x); }};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, [](FILE* x) { free(f); }};\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: f\n", errout.str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, [](FILE* x) {}};\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: f\n", errout.str());

        check("class C;\n"
              "void f() {\n"
              "  C* c = new C{};\n"
              "  std::shared_ptr<C> a{c, [](C*) {}};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("class C;\n"
              "void f() {\n"
              "  C* c = new C{};\n"
              "  std::shared_ptr<C> a{c, [](C* x) { delete x; }};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }
    void smartPointerRelease() {
        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x(i);\n"
              "    x.release();\n"
              "    delete i;\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x(i);\n"
              "    x.release();\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: i\n", errout.str());
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
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        delete p;\n"
              "        throw 1;\n"
              "    }\n"
              "    delete p;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        delete [] p;\n"
              "        throw 1;\n"
              "    }\n"
              "    delete [] p;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void return5() { // ticket #6397 - conditional allocation/deallocation and conditional return
        // avoid false positives
        check("void f(int *p, int x) {\n"
              "    if (x != 0) {\n"
              "        free(p);\n"
              "    }\n"
              "    if (x != 0) {\n"
              "        return;\n"
              "    }\n"
              "    *p = 0;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void return6() { // #8282
        check("std::pair<char*, char*> f(size_t n) {\n"
              "   char* p = (char* )malloc(n);\n"
              "   return {p, p};\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void return7() { // #9343
        check("uint8_t *f() {\n"
              "    void *x = malloc(1);\n"
              "    return (uint8_t *)x;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("uint8_t f() {\n"
              "    void *x = malloc(1);\n"
              "    return (uint8_t)x;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: x\n", errout.str());

        check("void** f() {\n"
              "    void *x = malloc(1);\n"
              "    return (void**)x;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (long long)x;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (void*)(short)x;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: x\n", errout.str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (mytype)x;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void* f() {\n" // Do not crash
              "    void *x = malloc(1);\n"
              "    return (mytype)y;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: x\n", errout.str());
    }

    void return8() {
        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (x);\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return ((x));\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return ((((x))));\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("char* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (char*)(x);\n"
              "}", true);
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

    void test4() { // 5923 - static pointer
        check("void f() {\n"
              "    static char *p;\n"
              "    if (!p) p = malloc(10);\n"
              "    if (x) { free(p); p = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void test5() { // unknown type
        check("void f() { Fred *p = malloc(10); }", true);
        ASSERT_EQUALS("[test.cpp:1]: (error) Memory leak: p\n", errout.str());

        check("void f() { Fred *p = malloc(10); }", false);
        ASSERT_EQUALS("[test.c:1]: (error) Memory leak: p\n", errout.str());

        check("void f() { Fred *p = new Fred; }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { Fred fred = malloc(10); }", true);
        ASSERT_EQUALS("", errout.str());
    }

    void throw1() { // 3987 - Execution reach a 'throw'
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    throw 123;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: p\n", errout.str());

        check("void f() {\n"
              "    char *p;\n"
              "    try {\n"
              "        p = malloc(10);\n"
              "        throw 123;\n"
              "    } catch (...) { }\n"
              "    free(p);\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void throw2() { // do not miss ::NS::Except()
        check("namespace NS {\n"
              "    class Except {\n"
              "    };\n"
              "}\n"
              "void foo(int i)\n"
              "{\n"
              "    int *pi = new int;\n"
              "    if (i == 42) {\n"
              "        delete pi;\n"
              "        throw ::NS::Except();\n"
              "    }\n"
              "    delete pi;\n"
              "}", true);
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
                      "[test.c:4]: (information) --check-library: Function x() should have <use>/<leak-ignore> configuration\n",
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
                      "[test.c:4]: (information) --check-library: Function x() should have <use>/<leak-ignore> configuration\n",
                      errout.str());
    }

    void configuration3() {
        const char * code = "void f() {\n"
                            "    char *p = malloc(10);\n"
                            "    if (set_data(p)) { }\n"
                            "}";
        check(code);
        ASSERT_EQUALS("[test.c:4]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration\n", errout.str());
        check(code, true);
        ASSERT_EQUALS("[test.cpp:4]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration\n", errout.str());

        code = "void f() {\n"
               "    char *p = malloc(10);\n"
               "    if (set_data(p)) { return; }\n"
               "}";
        check(code);
        ASSERT_EQUALS("[test.c:3]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration\n"
                      "[test.c:4]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration\n"
                      , errout.str());
        check(code, true);
        ASSERT_EQUALS("[test.cpp:3]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration\n"
                      "[test.cpp:4]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration\n"
                      , errout.str());
    }

    void configuration4() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    int ret = set_data(p);\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("[test.c:4]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration\n", errout.str());
    }

    void ptrptr() {
        check("void f() {\n"
              "    char **p = malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.c:3]: (error) Memory leak: p\n", errout.str());
    }

    void nestedAllocation() {
        check("void QueueDSMCCPacket(unsigned char *data, int length) {\n"
              "    unsigned char *dataCopy = malloc(length * sizeof(unsigned char));\n"
              "    m_dsmccQueue.enqueue(new DSMCCPacket(dataCopy));\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (information) --check-library: Function DSMCCPacket() should have <use>/<leak-ignore> configuration\n", errout.str());

        check("void QueueDSMCCPacket(unsigned char *data, int length) {\n"
              "    unsigned char *dataCopy = malloc(length * sizeof(unsigned char));\n"
              "    m_dsmccQueue.enqueue(new DSMCCPacket(somethingunrelated));\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: dataCopy\n", errout.str());

        check("void f() {\n"
              "  char *buf = new char[1000];\n"
              "  clist.push_back(new (std::nothrow) C(buf));\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (information) --check-library: Function C() should have <use>/<leak-ignore> configuration\n", errout.str());
    }

    void testKeywords() {
        check("int main(int argc, char **argv) {\n"
              "  double *new = malloc(1*sizeof(double));\n"
              "  free(new);\n"
              "  return 0;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void inlineFunction() {
        check("int test() {\n"
              "  char *c;\n"
              "  int ret() {\n"
              "        free(c);\n"
              "        return 0;\n"
              "    }\n"
              "    c = malloc(128);\n"
              "    return ret();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // #8262
    void smartPtrInContainer() {
        check("std::list< std::shared_ptr<int> > mList;\n"
              "void test(){\n"
              "  int *pt = new int(1);\n"
              "  mList.push_back(std::shared_ptr<int>(pt));\n"
              "}\n",
              true
             );
        ASSERT_EQUALS("", errout.str());
    }

    void recursiveCountLimit() { // #5872 #6157 #9097
        ASSERT_THROW(checkP("#define ONE     else if (0) { }\n"
                            "#define TEN     ONE ONE ONE ONE ONE ONE ONE ONE ONE ONE\n"
                            "#define HUN     TEN TEN TEN TEN TEN TEN TEN TEN TEN TEN\n"
                            "#define THOU    HUN HUN HUN HUN HUN HUN HUN HUN HUN HUN\n"
                            "void foo() {\n"
                            "  if (0) { }\n"
                            "  THOU THOU\n"
                            "}"), InternalError);
        ASSERT_NO_THROW(checkP("#define ONE     if (0) { }\n"
                               "#define TEN     ONE ONE ONE ONE ONE ONE ONE ONE ONE ONE\n"
                               "#define HUN     TEN TEN TEN TEN TEN TEN TEN TEN TEN TEN\n"
                               "#define THOU    HUN HUN HUN HUN HUN HUN HUN HUN HUN HUN\n"
                               "void foo() {\n"
                               "  if (0) { }\n"
                               "  THOU THOU\n"
                               "}"));
    }

    void functionCallCastConfig() { // #9652
        Settings settingsFunctionCall = settings;

        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def format=\"2\">\n"
                               "  <function name=\"free_func\">\n"
                               "    <noreturn>false</noreturn>\n"
                               "    <arg nr=\"1\">\n"
                               "      <not-uninit/>\n"
                               "    </arg>\n"
                               "    <arg nr=\"2\">\n"
                               "      <not-uninit/>\n"
                               "    </arg>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        settingsFunctionCall.library.load(doc);
        check("void test_func()\n"
              "{\n"
              "    char * buf = malloc(4);\n"
              "    free_func((void *)(1), buf);\n"
              "}", settingsFunctionCall);
        ASSERT_EQUALS("[test.cpp:5]: (information) --check-library: Function free_func() should have <use>/<leak-ignore> configuration\n", errout.str());
    }
};

REGISTER_TEST(TestLeakAutoVar)


class TestLeakAutoVarStrcpy: public TestFixture {
public:
    TestLeakAutoVarStrcpy() : TestFixture("TestLeakAutoVarStrcpy") {
    }

private:
    Settings settings;

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for leaks..
        CheckLeakAutoVar checkLeak;
        settings.checkLibrary = true;
        settings.addEnabled("information");
        checkLeak.runChecks(&tokenizer, &settings, this);
    }

    void run() OVERRIDE {
        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(returnedValue); // #9298
        TEST_CASE(fclose_false_positive); // #9575
    }

    void returnedValue() { // #9298
        check("char *m;\n"
              "void strcpy_returnedvalue(const char* str)\n"
              "{\n"
              "    char* ptr = new char[strlen(str)+1];\n"
              "    m = strcpy(ptr, str);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void fclose_false_positive() { // #9575
        check("int  f(FILE *fp) { return fclose(fp); }");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestLeakAutoVarStrcpy)


class TestLeakAutoVarWindows : public TestFixture {
public:
    TestLeakAutoVarWindows() : TestFixture("TestLeakAutoVarWindows") {
    }

private:
    Settings settings;

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.c");

        // Check for leaks..
        CheckLeakAutoVar checkLeak;
        checkLeak.runChecks(&tokenizer, &settings, this);
    }

    void run() OVERRIDE {
        LOAD_LIB_2(settings.library, "windows.cfg");

        TEST_CASE(heapDoubleFree);
    }

    void heapDoubleFree() {
        check("void f() {"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  HeapFree(MyHeap, 0, a);"
              "  HeapFree(MyHeap, 0, b);"
              "  HeapDestroy(MyHeap);"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {"
              "  int *a = HeapAlloc(GetProcessHeap(), 0, sizeof(int));"
              "  int *b = HeapAlloc(GetProcessHeap(), 0, sizeof(int));"
              "  HeapFree(GetProcessHeap(), 0, a);"
              "  HeapFree(GetProcessHeap(), 0, b);"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  HeapFree(MyHeap, 0, a);"
              "  HeapDestroy(MyHeap);"
              "}");
        ASSERT_EQUALS("[test.c:1]: (error) Memory leak: b\n", errout.str());

        check("void f() {"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  HeapFree(MyHeap, 0, a);"
              "  HeapFree(MyHeap, 0, b);"
              "}");
        TODO_ASSERT_EQUALS("[test.c:1] (error) Resource leak: MyHeap",
                           "", errout.str());

        check("void f() {"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));"
              "  HeapFree(MyHeap, 0, a);"
              "}");
        TODO_ASSERT_EQUALS("[test.c:1] (error) Memory leak: MyHeap\n"
                           "[test.c:1] (error) Memory leak: b",
                           "[test.c:1]: (error) Memory leak: b\n", errout.str());
    }
};

REGISTER_TEST(TestLeakAutoVarWindows)
