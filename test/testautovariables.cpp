/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "checkautovariables.h"
#include "testsuite.h"


class TestAutoVariables : public TestFixture {
public:
    TestAutoVariables() : TestFixture("TestAutoVariables") {
    }

private:


    void check(const char code[], bool inconclusive = false, bool runSimpleChecks = true, const char* filename = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.inconclusive = inconclusive;
        settings.addEnabled("warning");
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        CheckAutoVariables checkAutoVariables(&tokenizer, &settings, this);
        checkAutoVariables.returnReference();
        checkAutoVariables.assignFunctionArg();

        if (runSimpleChecks) {
            const std::string str1(tokenizer.tokens()->stringifyList(0,true));
            tokenizer.simplifyTokenList2();
            const std::string str2(tokenizer.tokens()->stringifyList(0,true));
            if (str1 != str2)
                warnUnsimplified(str1, str2);

            // Check auto variables
            checkAutoVariables.autoVariables();
            checkAutoVariables.returnPointerToLocalArray();
        }
    }

    void run() {
        TEST_CASE(testautovar1);
        TEST_CASE(testautovar2);
        TEST_CASE(testautovar3); // ticket #2925
        TEST_CASE(testautovar4); // ticket #2928
        TEST_CASE(testautovar5); // ticket #2926
        TEST_CASE(testautovar6); // ticket #2931
        TEST_CASE(testautovar7); // ticket #3066
        TEST_CASE(testautovar8);
        TEST_CASE(testautovar9);
        TEST_CASE(testautovar10); // ticket #2930 - void f(char *p) { p = '\0'; }
        TEST_CASE(testautovar11); // ticket #4641 - fp, assign local struct member address to function parameter
        TEST_CASE(testautovar12); // ticket #5024 - crash
        TEST_CASE(testautovar13); // ticket #5537 - crash
        TEST_CASE(testautovar14); // ticket #4776 - assignment of function parameter, goto
        TEST_CASE(testautovar15); // ticket #6538
        TEST_CASE(testautovar_array1);
        TEST_CASE(testautovar_array2);
        TEST_CASE(testautovar_return1);
        TEST_CASE(testautovar_return2);
        TEST_CASE(testautovar_return3);
        TEST_CASE(testautovar_return4); // ticket #3030
        TEST_CASE(testautovar_extern);
        TEST_CASE(testinvaliddealloc);
        TEST_CASE(testinvaliddealloc_C);
        TEST_CASE(testassign1);  // Ticket #1819
        TEST_CASE(testassign2);  // Ticket #2765

        TEST_CASE(returnLocalVariable1);
        TEST_CASE(returnLocalVariable2);

        // return reference..
        TEST_CASE(returnReference1);
        TEST_CASE(returnReference2);
        TEST_CASE(returnReference3);
        TEST_CASE(returnReference4);
        TEST_CASE(returnReference5);
        TEST_CASE(returnReference6);
        TEST_CASE(returnReference7);
        TEST_CASE(returnReferenceLiteral);
        TEST_CASE(returnReferenceCalculation);

        // global namespace
        TEST_CASE(testglobalnamespace);

        TEST_CASE(returnParameterAddress);

        TEST_CASE(testconstructor); // ticket #5478 - crash

        TEST_CASE(variableIsUsedInScope); // ticket #5599 crash in variableIsUsedInScope()
    }



    void testautovar1() {
        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    *res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());

        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?\n", errout.str());

        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    foo.res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar2() {
        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    *res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());

        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?\n", errout.str());

        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    foo.res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar3() { // ticket #2925
        check("void foo(int **p)\n"
              "{\n"
              "    int x[100];\n"
              "    *p = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar4() { // ticket #2928
        check("void foo(int **p)\n"
              "{\n"
              "    static int x[100];\n"
              "    *p = x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar5() { // ticket #2926
        check("void foo(struct AB *ab)\n"
              "{\n"
              "    char a;\n"
              "    ab->a = &a;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());

        check("void foo(struct AB *ab)\n"
              "{\n"
              "    char a;\n"
              "    ab->a = &a;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (error, inconclusive) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar6() { // ticket #2931
        check("void foo(struct X *x)\n"
              "{\n"
              "    char a[10];\n"
              "    x->str = a;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());

        check("void foo(struct X *x)\n"
              "{\n"
              "    char a[10];\n"
              "    x->str = a;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (error, inconclusive) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar7() { // ticket #3066
        check("struct txt_scrollpane_s * TXT_NewScrollPane(struct txt_widget_s * target)\n"
              "{\n"
              "    struct txt_scrollpane_s * scrollpane;\n"
              "    target->parent = &scrollpane->widget;\n"
              "    return scrollpane;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar8() {
        check("void foo(int*& p) {\n"
              "    int i = 0;\n"
              "    p = &i;\n"
              "}", false);
        ASSERT_EQUALS("[test.cpp:3]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());

        check("void foo(std::string& s) {\n"
              "    s = foo;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar9() {
        check("struct FN {int i;};\n"
              "struct FP {FN* f};\n"
              "void foo(int*& p, FN* p_fp) {\n"
              "    FN fn;\n"
              "    FP fp;\n"
              "    p = &fn.i;\n"
              "    p = &p_fp->i;\n"
              "    p = &fp.f->i;\n"
              "}", false);
        ASSERT_EQUALS("[test.cpp:6]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar10() { // #2930 - assignment of function parameter
        check("void foo(char* p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?\n", errout.str());

        check("void foo(int b) {\n"
              "    b = foo(b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Assignment of function parameter has no effect outside the function.\n", errout.str());

        check("void foo(char* p) {\n"
              "    if (!p) p = buf;\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* p) {\n"
              "    if (!p) p = buf;\n"
              "    do_something(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* p) {\n"
              "    while (!p) p = buf;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* p) {\n"
              "    p = 0;\n"
              "    asm(\"somecmd\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(Foo* p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?\n", errout.str());

        check("class Foo {};\n"
              "void foo(Foo p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Assignment of function parameter has no effect outside the function.\n", errout.str());

        check("void foo(Foo p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int& p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("double foo(double d) {\n" // #5005
              "    int i = d;\n"
              "    d = i;\n"
              "    return d;"
              "}",false,false);
        ASSERT_EQUALS("", errout.str());

        check("void foo(int* ptr) {\n" // #4793
              "    ptr++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?\n", errout.str());
    }

    void testautovar11() { // #4641 - fp, assign local struct member address to function parameter
        check("struct A {\n"
              "    char *data[10];\n"
              "};\n"
              "void foo(char** p) {\n"
              "    struct A a = bar();\n"
              "    *p = &a.data[0];\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    char data[10];\n"
              "};\n"
              "void foo(char** p) {\n"
              "    struct A a = bar();\n"
              "    *p = &a.data[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());

        // #4998
        check("void f(s8**out) {\n"
              "  s8 *p;\n"  // <- p is pointer => no error
              "  *out = &p[1];\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(s8**out) {\n"
              "  s8 p[10];\n"  // <- p is array => error
              "  *out = &p[1];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar12() { // Ticket #5024, #5050 - Crash on invalid input
        check("void f(int* a) { a = }");
        check("struct custom_type { custom_type(int) {} };\n"
              "void func(int) {}\n"
              "int var;\n"
              "void init() { func(var); }\n"
              "UNKNOWN_MACRO_EXPANDING_TO_SIGNATURE { custom_type a(var); }");
    }

    void testautovar13() { // Ticket #5537
        check("class FileManager {\n"
              "  FileManager() : UniqueRealDirs(*new UniqueDirContainer())\n"
              "  {}\n"
              "  ~FileManager() {\n"
              "    delete &UniqueRealDirs;\n"
              "   }\n"
              "};\n");
    }

    void testautovar14() { // Ticket #4776
        check("void f(int x) {\n"
              "label:"
              "  if (x>0) {\n"
              "    x = x >> 1;\n"
              "    goto label;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar15() { // Ticket #6538
        check("static const float4  darkOutline(0.05f, 0.05f, 0.05f, 0.95f);\n"
              "static const float darkLuminosity = 0.05 +\n"
              "                                    0.0722f * math::powf(darkOutline[2], 2.2);\n"
              "const float4* ChooseOutlineColor(const float4& textColor)  {\n"
              "    const float lumdiff = something;\n"
              "    if (lumdiff > 5.0f)\n"
              "        return &darkOutline;\n"
              "    return 0;\n"
              "}", false, false);
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar_array1() {
        check("void func1(int* arr[2])\n"
              "{\n"
              "    int num=2;"
              "    arr[0]=&num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar_array2() {
        check("class Fred {\n"
              "    void func1(int* arr[2]);\n"
              "}\n"
              "void Fred::func1(int* arr[2])\n"
              "{\n"
              "    int num=2;"
              "    arr[0]=&num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar_return1() {
        check("int* func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Address of an auto-variable returned.\n", errout.str());
    }

    void testautovar_return2() {
        check("class Fred {\n"
              "    int* func1()\n"
              "}\n"
              "int* Fred::func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Address of an auto-variable returned.\n", errout.str());
    }

    void testautovar_return3() {
        // #2975 - FP
        check("void** f()\n"
              "{\n"
              "    void *&value = tls[id];"
              "    return &value;"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar_return4() {
        // #3030
        check("char *foo()\n"
              "{\n"
              "    char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Address of an auto-variable returned.\n", errout.str());

        check("char *foo()\n"
              "{\n"
              "    static char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar_extern() {
        check("struct foo *f()\n"
              "{\n"
              "    extern struct foo f;\n"
              "    return &f;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testinvaliddealloc() {
        check("void func1() {\n"
              "    char tmp1[256];\n"
              "    free(tmp1);\n"
              "    char tmp2[256];\n"
              "    delete tmp2;\n"
              "    char tmp3[256];\n"
              "    delete tmp3;\n"
              "    char tmp4[256];\n"
              "    delete[] (tmp4);\n"
              "    char tmp5[256];\n"
              "    delete[] tmp5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Deallocation of an auto-variable results in undefined behaviour.\n"
                      "[test.cpp:5]: (error) Deallocation of an auto-variable results in undefined behaviour.\n"
                      "[test.cpp:7]: (error) Deallocation of an auto-variable results in undefined behaviour.\n"
                      "[test.cpp:9]: (error) Deallocation of an auto-variable results in undefined behaviour.\n"
                      "[test.cpp:11]: (error) Deallocation of an auto-variable results in undefined behaviour.\n", errout.str());

        check("void func1() {\n"
              "    char* tmp1[256];\n"
              "    init(tmp1);\n"
              "    delete tmp1[34];\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char psz_title[10];\n"
              "    {\n"
              "        char *psz_title = 0;\n"
              "        abc(0, psz_title);\n"
              "        free(psz_title);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2298 new check: passing stack-address to free()
        check("int main() {\n"
              "   int *p = malloc(4);\n"
              "   free(&p);\n"
              "   return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Deallocation of an auto-variable results in undefined behaviour.\n", errout.str());
        check("int main() {\n"
              "   int i;\n"
              "   free(&i);\n"
              "   return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Deallocation of an auto-variable results in undefined behaviour.\n", errout.str());

        // #5732
        check("int main() {\n"
              "   long (*pKoeff)[256] = new long[9][256];\n"
              "   delete[] pKoeff;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int main() {\n"
              "   long *pKoeff[256];\n"
              "   delete[] pKoeff;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Deallocation of an auto-variable results in undefined behaviour.\n", "", errout.str());

        check("int main() {\n"
              "   long *pKoeff[256];\n"
              "   free (pKoeff);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Deallocation of an auto-variable results in undefined behaviour.\n", "", errout.str());

        check("void foo() {\n"
              "   const intPtr& intref = Getter();\n"
              "   delete intref;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void test() {\n"
              "   MyObj& obj = *new MyObj;\n"
              "   delete &obj;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }

    void testinvaliddealloc_C() {
        // #5691
        check("void svn_repos_dir_delta2() {\n"
              "  struct context c;\n"
              "      SVN_ERR(delete(&c, root_baton, src_entry, pool));\n"
              "}\n", false, true, "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void testassign1() { // Ticket #1819
        check("void f(EventPtr *eventP, ActionPtr **actionsP) {\n"
              "    EventPtr event = *eventP;\n"
              "    *actionsP = &event->actions;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testassign2() { // Ticket #2765
        check("static void function(unsigned long **datap) {\n"
              "    struct my_s *mr = global_structure_pointer;\n"
              "    *datap = &mr->value;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnLocalVariable1() {
        check("char *foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Pointer to local array variable returned.\n", errout.str());

        check("class Fred {\n"
              "    char *foo();\n"
              "};\n"
              "char *Fred::foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Pointer to local array variable returned.\n", errout.str());
    }

    void returnLocalVariable2() {
        check("std::string foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred {\n"
              "    std::string foo();\n"
              "};\n"
              "std::string Fred::foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReference1() {
        check("std::string &foo()\n"
              "{\n"
              "    std::string s;\n"
              "    return s;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Reference to auto variable returned.\n", errout.str());

        check("std::vector<int> &foo()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Reference to auto variable returned.\n", errout.str());

        check("std::vector<int> &foo()\n"
              "{\n"
              "    static std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "\n"
              "std::string &f()\n"
              "{\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Reference to temporary returned.\n", errout.str());

        // make sure scope is used in function lookup
        check("class Fred {\n"
              "    std::string hello() {\n"
              "        return std::string();\n"
              "    }\n"
              "};\n"
              "std::string &f() {\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::string hello() {\n"
              "     return std::string();\n"
              "}\n"
              "\n"
              "std::string &f() {\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Reference to temporary returned.\n", errout.str());

        check("std::string hello() {\n"
              "     return \"foo\";\n"
              "}\n"
              "\n"
              "std::string &f() {\n"
              "    return hello().substr(1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Foo;\n"
              "Foo hello() {\n"
              "     return Foo();\n"
              "}\n"
              "\n"
              "Foo& f() {\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Reference to temporary returned.\n", errout.str());

        // make sure function overloads are handled properly
        check("class Foo;\n"
              "Foo & hello(bool) {\n"
              "     static Foo foo;\n"
              "     return foo;\n"
              "}\n"
              "Foo hello() {\n"
              "     return Foo();\n"
              "}\n"
              "\n"
              "Foo& f() {\n"
              "    return hello(true);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("Foo hello() {\n"
              "     return Foo();\n"
              "}\n"
              "\n"
              "Foo& f() {\n" // Unknown type - might be a reference
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReference2() {
        check("class Fred {\n"
              "    std::string &foo();\n"
              "}\n"
              "std::string &Fred::foo()\n"
              "{\n"
              "    std::string s;\n"
              "    return s;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Reference to auto variable returned.\n", errout.str());

        check("class Fred {\n"
              "    std::vector<int> &foo();\n"
              "};\n"
              "std::vector<int> &Fred::foo()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Reference to auto variable returned.\n", errout.str());

        check("class Fred {\n"
              "    std::vector<int> &foo();\n"
              "};\n"
              "std::vector<int> &Fred::foo()\n"
              "{\n"
              "    static std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred {\n"
              "    std::string &f();\n"
              "};\n"
              "std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "std::string &Fred::f()\n"
              "{\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Reference to temporary returned.\n", errout.str());

        check("class Fred {\n"
              "    std::string hello();\n"
              "    std::string &f();\n"
              "};\n"
              "std::string Fred::hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "std::string &Fred::f()\n"
              "{\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Reference to temporary returned.\n", errout.str());

        check("class Bar;\n"
              "Bar foo() {\n"
              "     return something;\n"
              "}\n"
              "Bar& bar() {\n"
              "    return foo();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Reference to temporary returned.\n", errout.str());

        check("std::map<int, string> foo() {\n"
              "     return something;\n"
              "}\n"
              "std::map<int, string>& bar() {\n"
              "    return foo();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Reference to temporary returned.\n", errout.str());

        check("Bar foo() {\n"
              "     return something;\n"
              "}\n"
              "Bar& bar() {\n" // Unknown type - might be a typedef to a reference type
              "    return foo();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Don't crash with function in unknown scope (#4076)
        check("X& a::Bar() {}"
              "X& foo() {"
              "    return Bar();"
              "}");
    }

    void returnReference3() {
        check("double & f(double & rd) {\n"
              "    double ret = getValue();\n"
              "    rd = ret;\n"
              "    return rd;\n"
              "}", false, false);
        ASSERT_EQUALS("", errout.str());
    }

    // Returning reference to global variable
    void returnReference4() {
        check("double a;\n"
              "double & f() {\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReference5() {
        check("struct A {\n"
              "    int i;\n"
              "};\n"

              "struct B {\n"
              "    A a;\n"
              "};\n"

              "struct C {\n"
              "    B *b;\n"
              "    const A& a() const {\n"
              "        const B *pb = b;\n"
              "        const A &ra = pb->a;\n"
              "        return ra;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReference6() {
        check("Fred & create() {\n"
              "    Fred &fred(*new Fred);\n"
              "    return fred;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReference7() {  // 3791 - false positive for overloaded function
        check("std::string a();\n"
              "std::string &a(int);\n"
              "std::string &b() {\n"
              "    return a(12);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::string &a(int);\n"
              "std::string a();\n"
              "std::string &b() {\n"
              "    return a(12);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReferenceLiteral() {
        check("const std::string &a() {\n"
              "    return \"foo\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Reference to temporary returned.\n", errout.str());

        check("const std::string a() {\n"
              "    return \"foo\";\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReferenceCalculation() {
        check("const std::string &a(const std::string& str) {\n"
              "    return \"foo\" + str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Reference to temporary returned.\n", errout.str());

        check("int& operator<<(int out, int path) {\n"
              "    return out << path;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Reference to temporary returned.\n", errout.str());

        check("std::ostream& operator<<(std::ostream& out, const std::string& path) {\n"
              "    return out << path;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::ostream& operator<<(std::ostream* out, const std::string& path) {\n"
              "    return *out << path;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("Unknown1& operator<<(Unknown1 out, Unknown2 path) {\n"
              "    return out << path;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int& a(int b) {\n"
              "    return 2*(b+1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Reference to temporary returned.\n", errout.str());

        check("const std::string &a(const std::string& str) {\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("const std::string &a(int bar) {\n"
              "    return foo(bar + 1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("const std::string a(const std::string& str) {\n"
              "    return \"foo\" + str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void testglobalnamespace() {
        check("class SharedPtrHolder\n"
              "{\n"
              "   ::std::tr1::shared_ptr<int> pNum;\n"
              "public:\n"
              "   void SetNum(const ::std::tr1::shared_ptr<int> & apNum)\n"
              "   {\n"
              "      pNum = apNum;\n"
              "   }\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void returnParameterAddress() {
        check("int* foo(int y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3]: (error) Address of function parameter 'y' returned.\n", errout.str());

        check("int ** foo(int * y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3]: (error) Address of function parameter 'y' returned.\n", errout.str());

        check("const int * foo(const int & y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void testconstructor() { // Ticket #5478 - crash while checking a constructor
        check("class const_tree_iterator {\n"
              "  const_tree_iterator(bool (*_incream)(node_type*&)) {}\n"
              "  const_tree_iterator& parent() {\n"
              "    return const_tree_iterator(foo);\n"
              "  }\n"
              "};");
    }

    void variableIsUsedInScope() {
        check("void removed_cb (GList *uids) {\n"
              "for (; uids; uids = uids->next) {\n"
              "}\n"
              "}\n"
              "void opened_cb () {\n"
              "	g_signal_connect (G_CALLBACK (removed_cb));\n"
              "}");
    }

};

REGISTER_TEST(TestAutoVariables)
