/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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


#include "checkautovariables.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"


class TestAutoVariables : public TestFixture {
public:
    TestAutoVariables() : TestFixture("TestAutoVariables") {
    }

private:
    Settings settings;

    void check(const char code[], bool inconclusive = false, bool runSimpleChecks = true, const char* filename = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        settings.inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        CheckAutoVariables checkAutoVariables(&tokenizer, &settings, this);
        checkAutoVariables.returnReference();
        checkAutoVariables.assignFunctionArg();
        checkAutoVariables.checkVarLifetime();

        if (runSimpleChecks) {
            tokenizer.simplifyTokenList2();

            // Check auto variables
            checkAutoVariables.autoVariables();
        }
    }

    void run() override {
        settings.addEnabled("warning");
        settings.addEnabled("style");
        LOAD_LIB_2(settings.library, "std.cfg");

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
        TEST_CASE(testautovar_ptrptr); // ticket #6956
        TEST_CASE(testautovar_return1);
        TEST_CASE(testautovar_return2);
        TEST_CASE(testautovar_return3);
        TEST_CASE(testautovar_return4);
        TEST_CASE(testautovar_extern);
        TEST_CASE(testinvaliddealloc);
        TEST_CASE(testinvaliddealloc_C);
        TEST_CASE(testassign1);  // Ticket #1819
        TEST_CASE(testassign2);  // Ticket #2765

        TEST_CASE(assignAddressOfLocalArrayToGlobalPointer);
        TEST_CASE(assignAddressOfLocalVariableToGlobalPointer);

        TEST_CASE(returnLocalVariable1);
        TEST_CASE(returnLocalVariable2);
        TEST_CASE(returnLocalVariable3); // &x[0]
        TEST_CASE(returnLocalVariable4); // x+y
        TEST_CASE(returnLocalVariable5); // cast
        TEST_CASE(returnLocalVariable6); // valueflow

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
        TEST_CASE(returnReferenceLambda);
        TEST_CASE(returnReferenceInnerScope);

        // global namespace
        TEST_CASE(testglobalnamespace);

        TEST_CASE(returnParameterAddress);

        TEST_CASE(testconstructor); // ticket #5478 - crash

        TEST_CASE(variableIsUsedInScope); // ticket #5599 crash in variableIsUsedInScope()

        TEST_CASE(danglingLifetimeLambda);
        TEST_CASE(danglingLifetimeContainer);
        TEST_CASE(danglingLifetime);
        TEST_CASE(danglingLifetimeFunction);
        TEST_CASE(invalidLifetime);
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

        check("void foo(int b) {\n"
              "    b += 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Assignment of function parameter has no effect outside the function.\n", errout.str());

        check("void foo(std::string s) {\n"
              "    s = foo(b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Assignment of function parameter has no effect outside the function.\n", errout.str());

        check("void foo(char* p) {\n" // don't warn for self assignment, there is another warning for this
              "  p = p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

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

        check("void foo(int* ptr) {\n" // #3177
              "    --ptr;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?\n", errout.str());

        check("void foo(struct S* const x) {\n" // #7839
              "    ++x->n;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar11() { // #4641 - fp, assign local struct member address to function parameter
        check("struct A {\n"
              "    char (*data)[10];\n"
              "};\n"
              "void foo(char** p) {\n"
              "    struct A a = bar();\n"
              "    *p = &(*a.data)[0];\n"
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
        ASSERT_THROW(check("void f(int* a) { a = }"), InternalError);
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

    void testautovar_ptrptr() { // #6596
        check("void remove_duplicate_matches (char **matches) {\n"
              "  char dead_slot;\n"
              "  matches[0] = (char *)&dead_slot;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Address of local auto-variable assigned to a function parameter.\n", errout.str());
    }

    void testautovar_return1() {
        check("int* func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:3] -> [test.cpp:3]: (error) Returning pointer to local variable 'num' that will be invalid when returning.\n", errout.str());
    }

    void testautovar_return2() {
        check("class Fred {\n"
              "    int* func1();\n"
              "}\n"
              "int* Fred::func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:6] -> [test.cpp:6]: (error) Returning pointer to local variable 'num' that will be invalid when returning.\n", errout.str());
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
        // #8058 - FP ignore return in lambda
        check("void foo() {\n"
              "  int cond2;\n"
              "  dostuff([&cond2]() { return &cond2; });\n"
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

        check("void func1() {\n"
              "    static char tmp1[256];\n"
              "    char *p = tmp1;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Deallocation of an static variable (tmp1) results in undefined behaviour.\n", errout.str());

        check("char tmp1[256];\n"
              "void func1() {\n"
              "    char *p; if (x) p = tmp1;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Deallocation of an global variable (tmp1) results in undefined behaviour.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (error) Deallocation of an auto-variable results in undefined behaviour.\n", errout.str());

        check("int main() {\n"
              "   long *pKoeff[256];\n"
              "   free (pKoeff);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Deallocation of an auto-variable results in undefined behaviour.\n", errout.str());

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

        // #6506
        check("struct F {\n"
              "  void free(void*) {}\n"
              "};\n"
              "void foo() {\n"
              "  char c1[1];\n"
              "  F().free(c1);\n"
              "  char *c2 = 0;\n"
              "  F().free(&c2);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class foo {\n"
              "  void free(void* );\n"
              "  void someMethod() {\n"
              "    char **dst_copy = NULL;\n"
              "    free(&dst_copy);\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // #6551
        check("bool foo( ) {\n"
              "  SwTxtFld * pTxtFld = GetFldTxtAttrAt();\n"
              "  delete static_cast<SwFmtFld*>(&pTxtFld->GetAttr());\n"
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

    void assignAddressOfLocalArrayToGlobalPointer() {
        check("int *p;\n"
              "void f() {\n"
              "  int x[10];\n"
              "  p = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Address of local array x is assigned to global pointer p and not reassigned before x goes out of scope.\n", errout.str());

        check("int *p;\n"
              "void f() {\n"
              "  int x[10];\n"
              "  p = x;\n"
              "  p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assignAddressOfLocalVariableToGlobalPointer() {
        check("int *p;\n"
              "void f() {\n"
              "  int x;\n"
              "  p = &x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Address of local variable x is assigned to global pointer p and not reassigned before x goes out of scope.\n", errout.str());

        check("int *p;\n"
              "void f() {\n"
              "  int x;\n"
              "  p = &x;\n"
              "  p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnLocalVariable1() {
        check("char *foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:3] -> [test.cpp:4]: (error) Returning pointer to local variable 'str' that will be invalid when returning.\n",
            errout.str());

        check("char *foo()\n" // use ValueFlow
              "{\n"
              "    char str[100] = {0};\n"
              "    char *p = str;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:3] -> [test.cpp:5]: (error) Returning pointer to local variable 'str' that will be invalid when returning.\n",
            errout.str());

        check("class Fred {\n"
              "    char *foo();\n"
              "};\n"
              "char *Fred::foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:6] -> [test.cpp:6] -> [test.cpp:7]: (error) Returning pointer to local variable 'str' that will be invalid when returning.\n",
            errout.str());

        check("char * format_reg(char *outbuffer_start) {\n"
              "    return outbuffer_start;\n"
              "}\n"
              "void print_with_operands() {\n"
              "    char temp[42];\n"
              "    char *tp = temp;\n"
              "    tp = format_reg(tp);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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


    void returnLocalVariable3() { // &x[..]
        // #3030
        check("char *foo() {\n"
              "    char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning pointer to local variable 'q' that will be invalid when returning.\n", errout.str());

        check("char *foo()\n"
              "{\n"
              "    static char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnLocalVariable4() { // x+y
        check("char *foo() {\n"
              "    char x[10] = {0};\n"
              "    return x+5;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning pointer to local variable 'x' that will be invalid when returning.\n",
            errout.str());

        check("char *foo(int y) {\n"
              "    char x[10] = {0};\n"
              "    return (x+8)-y;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning pointer to local variable 'x' that will be invalid when returning.\n",
            errout.str());
    }

    void returnLocalVariable5() { // cast
        check("char *foo() {\n"
              "    int x[10] = {0};\n"
              "    return (char *)x;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning pointer to local variable 'x' that will be invalid when returning.\n",
            errout.str());
    }

    void returnLocalVariable6() { // valueflow
        check("int *foo() {\n"
              "    int x = 123;\n"
              "    int p = &x;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning object that points to local variable 'x' that will be invalid when returning.\n", errout.str());
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

        check("int& incValue(int& value) {\n"
              "    return ++value;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReferenceLambda() {
        // #6787
        check("const Item& foo(const Container& items) const {\n"
              "    return bar(items.begin(), items.end(),\n"
              "    [](const Item& lhs, const Item& rhs) {\n"
              "        return false;\n"
              "    });\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5844
        check("map<string,string> const &getVariableTable() {\n"
              "static map<string,string> const s_var = []{\n"
              "    map<string,string> var;\n"
              "    return var;\n"
              "  }();\n"
              "return s_var;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7583
        check("Command& foo() {\n"
              "  return f([]() -> int { return 1; });\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReferenceInnerScope() {
        // #6951
        check("const Callback& make() {\n"
              "    struct _Wrapper {\n"
              "        static ulong call(void* o, const void* f, const void*[]) {\n"
              "            return 1;\n"
              "        }\n"
              "    };\n"
              "    return _make(_Wrapper::call, pmf);\n"
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

        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1] -> [test.cpp:3]: (error) Returning pointer to local variable 'y' that will be invalid when returning.\n", errout.str());

        check("int ** foo(int * y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1] -> [test.cpp:3]: (error) Returning pointer to local variable 'y' that will be invalid when returning.\n", errout.str());

        check("const int * foo(const int & y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("int * foo(int * y)\n"
              "{\n"
              "  return y;\n"
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

    void danglingLifetimeLambda() {
        check("auto f() {\n"
              "    int a = 1;\n"
              "    auto l = [&](){ return a; };\n"
              "    return l;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", errout.str());

        check("auto f() {\n"
              "    int a = 1;\n"
              "    return [&](){ return a; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", errout.str());

        check("auto f(int a) {\n"
              "    return [&](){ return a; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1] -> [test.cpp:2]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", errout.str());

        check("auto f(int a) {\n"
              "    auto p = &a;\n"
              "    return [=](){ return p; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3] -> [test.cpp:1] -> [test.cpp:3]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", errout.str());

        check("auto g(int& a) {\n"
              "    int p = a;\n"
              "    return [&](){ return p; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning lambda that captures local variable 'p' that will be invalid when returning.\n", errout.str());

        check("auto f() {\n"
              "    return [=](){\n"
              "        int a = 1;\n"
              "        return [&](){ return a; };\n"
              "    };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3] -> [test.cpp:4]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", errout.str());

        // TODO: Variable is not set correctly for this case
        check("auto f(int b) {\n"
              "    return [=](int a){\n"
              "        a += b;\n"
              "        return [&](){ return a; };\n"
              "    };\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", "", errout.str());

        check("auto g(int& a) {\n"
              "    return [&](){ return a; };\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("auto g(int a) {\n"
              "    auto p = a;\n"
              "    return [=](){ return p; };\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("auto g(int& a) {\n"
              "    auto p = a;\n"
              "    return [=](){ return p; };\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("auto g(int& a) {\n"
              "    int& p = a;\n"
              "    return [&](){ return p; };\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("template<class F>\n"
              "void g(F);\n"
              "auto f() {\n"
              "    int x;\n"
              "    return g([&]() { return x; });\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void danglingLifetimeContainer() {
        check("auto f(const std::vector<int>& x) {\n"
              "    auto it = x.begin();\n"
              "    return it;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning iterator to local container 'x' that will be invalid when returning.\n", errout.str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto p = x.data();\n"
              "    return p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning object that points to local variable 'x' that will be invalid when returning.\n", errout.str());

        check("auto f(std::vector<int> x) {\n"
              "    auto it = x.begin();\n"
              "    return it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1] -> [test.cpp:3]: (error) Returning iterator to local container 'x' that will be invalid when returning.\n", errout.str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return std::next(it);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:4] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning object that points to local variable 'x' that will be invalid when returning.\n",
            errout.str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return it + 1;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning iterator to local container 'x' that will be invalid when returning.\n",
            errout.str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return std::next(it + 1);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:4] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning object that points to local variable 'x' that will be invalid when returning.\n",
            errout.str());

        check("std::vector<int*> f() {\n"
              "    int i = 0;\n"
              "    std::vector<int*> v;\n"
              "    v.push_back(&i);\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4] -> [test.cpp:4] -> [test.cpp:2] -> [test.cpp:5]: (error) Returning object that points to local variable 'i' that will be invalid when returning.\n",
            errout.str());

        check("std::vector<int*> f() {\n"
              "    std::vector<int*> r;\n"
              "    int i = 0;\n"
              "    std::vector<int*> v;\n"
              "    v.push_back(&i);\n"
              "    r.assign(v.begin(), v.end());\n"
              "    return r;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:5] -> [test.cpp:5] -> [test.cpp:5] -> [test.cpp:3] -> [test.cpp:7]: (error) Returning object that points to local variable 'i' that will be invalid when returning.\n",
            errout.str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    void f() {\n"
              "        int i;\n"
              "        v.push_back(&i);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS(
            "[test.cpp:5] -> [test.cpp:5] -> [test.cpp:4] -> [test.cpp:5]: (error) Non-local variable 'v' will use object that points to local variable 'i'.\n",
            errout.str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    void f() {\n"
              "        int i;\n"
              "        int * p = &i;\n"
              "        v.push_back(p);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS(
            "[test.cpp:5] -> [test.cpp:6] -> [test.cpp:4] -> [test.cpp:6]: (error) Non-local variable 'v' will use object that points to local variable 'i'.\n",
            errout.str());

        check("struct A {\n"
              "    std::vector<std::string> v;\n"
              "    void f() {\n"
              "        char s[3];\n"
              "        v.push_back(s);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<std::string> f() {\n"
              "    const char * s = \"hello\";\n"
              "    std::vector<std::string> v;\n"
              "    v.push_back(s);\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("auto f() {\n"
              "  static std::vector<int> x;\n"
              "  return x.begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::string g() {\n"
              "    std::vector<char> v;\n"
              "    return v.data();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<int>::iterator f(std::vector<int>* v) {\n"
              "    return v->begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<int>::iterator f(std::vector<int>* v) {\n"
              "    std::vector<int>* v = new std::vector<int>();\n"
              "    return v->begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(std::vector<int> v) {\n"
              "    return *v.begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int f(std::vector<int> v) {\n"
              "    return v.end() - v.begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("auto g() {\n"
              "    std::vector<char> v;\n"
              "    return {v, [v]() { return v.data(); }};\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("template<class F>\n"
              "void g(F);\n"
              "auto f() {\n"
              "    std::vector<char> v;\n"
              "    return g([&]() { return v.data(); });\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void danglingLifetime() {
        check("auto f() {\n"
              "    std::vector<int> a;\n"
              "    auto it = a.begin();\n"
              "    return [=](){ return it; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4] -> [test.cpp:2] -> [test.cpp:4]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", errout.str());

        check("auto f(std::vector<int> a) {\n"
              "    auto it = a.begin();\n"
              "    return [=](){ return it; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3] -> [test.cpp:1] -> [test.cpp:3]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning.\n", errout.str());

        check("auto f(std::vector<int>& a) {\n"
              "    auto it = a.begin();\n"
              "    return [=](){ return it; };\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int * f(int a[]) {\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Make sure we dont hang
        check("struct A;\n"
              "void f() {\n"
              "    using T = A[3];\n"
              "    A &&a = T{1, 2, 3}[1];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Make sure we dont hang
        check("struct A;\n"
              "void f() {\n"
              "    using T = A[3];\n"
              "    A &&a = T{1, 2, 3}[1]();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void danglingLifetimeFunction() {
        check("auto f() {\n"
              "    int a;\n"
              "    return std::ref(a);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning object that points to local variable 'a' that will be invalid when returning.\n",
            errout.str());

        check("auto f() {\n"
              "    int a;\n"
              "    return std::make_tuple(std::ref(a));\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:3] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning object that points to local variable 'a' that will be invalid when returning.\n",
            errout.str());

        check("auto f(int x) {\n"
              "    int a;\n"
              "    std::tie(a) = x;\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void invalidLifetime() {
        check("void foo(int a) {\n"
              "    std::function<void()> f;\n"
              "    if (a > 0) {\n"
              "        int b = a + 1;\n"
              "        f = [&]{ return b; };\n"
              "    }\n"
              "    f();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4] -> [test.cpp:7]: (error) Using lambda that captures local variable 'b' that is out of scope.\n", errout.str());

        check("void f(bool b)  {\n"
              "  int* x;\n"
              "  if(b) {\n"
              "    int y[6] = {0,1,2,3,4,5};\n"
              "    x = y;\n"
              "  }\n"
              "  x[3];\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4] -> [test.cpp:4] -> [test.cpp:7]: (error) Using pointer to local variable 'y' that is out of scope.\n",
            errout.str());

        check("void foo(int a) {\n"
              "    std::function<void()> f;\n"
              "    if (a > 0) {\n"
              "        int b = a + 1;\n"
              "        f = [&]{ return b; };\n"
              "        f();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct a {\n"
              "  b();\n"
              "  std::list<int> c;\n"
              "};\n"
              "void a::b() {\n"
              "  c.end()\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void b(char f[], char c[]) {\n"
              "  std::string d(c); {\n"
              "    std::string e;\n"
              "    b(f, e.c_str())\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool b) {\n"
              "    std::string s;\n"
              "    if(b) {\n"
              "        char buf[3];\n"
              "        s = buf;\n"
              "    }\n"
              "    std::cout << s;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int &a[];\n"
              "void b(){int *c = a};\n");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestAutoVariables)
