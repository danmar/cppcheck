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
#include "checkautovariables.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestAutoVariables : public TestFixture {
public:
    TestAutoVariables() : TestFixture("TestAutoVariables") {
    }

private:



    void check(const char code[], bool inconclusive=false, bool runSimpleChecks=true) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.inconclusive = inconclusive;
        settings.addEnabled("warning");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckAutoVariables checkAutoVariables(&tokenizer, &settings, this);
        checkAutoVariables.returnReference();

        if (runSimpleChecks) {
            const std::string str1(tokenizer.tokens()->stringifyList(0,true));
            tokenizer.simplifyTokenList();
            const std::string str2(tokenizer.tokens()->stringifyList(0,true));
            if (str1 != str2)
                warn(("Unsimplified code in test case. It looks like this test "
                      "should either be cleaned up or moved to TestTokenizer or "
                      "TestSimplifyTokens instead.\nstr1="+str1+"\nstr2="+str2).c_str());


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
        TEST_CASE(testautovar_array1);
        TEST_CASE(testautovar_array2);
        TEST_CASE(testautovar_return1);
        TEST_CASE(testautovar_return2);
        TEST_CASE(testautovar_return3);
        TEST_CASE(testautovar_return4); // ticket #3030
        TEST_CASE(testautovar_extern);
        TEST_CASE(testinvaliddealloc);
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

        // global namespace
        TEST_CASE(testglobalnamespace);

        TEST_CASE(returnParameterAddress);
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
        ASSERT_EQUALS("[test.cpp:4]: (warning) Assignment of function parameter has no effect outside the function.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:7]: (warning) Assignment of function parameter has no effect outside the function.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assignment of function parameter has no effect outside the function.\n", errout.str());

        check("void foo(int b) {\n"
              "    b = foo(b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assignment of function parameter has no effect outside the function.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assignment of function parameter has no effect outside the function.\n", errout.str());

        check("class Foo {};\n"
              "void foo(Foo p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assignment of function parameter has no effect outside the function.\n", errout.str());

        check("void foo(Foo p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int& p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS(std::string(""), errout.str());
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

};

REGISTER_TEST(TestAutoVariables)
