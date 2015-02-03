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

#include "testsuite.h"
#include "tokenize.h"
#include "token.h"
#include "settings.h"

#include <sstream>

extern std::ostringstream errout;


class TestVarID : public TestFixture {
public:
    TestVarID() : TestFixture("TestVarID") {
    }

private:

    void run() {
        TEST_CASE(varid1);
        TEST_CASE(varid2);
        TEST_CASE(varid3);
        TEST_CASE(varid4);
        TEST_CASE(varid5);
        TEST_CASE(varid6);
        TEST_CASE(varid7);
        TEST_CASE(varidReturn1);
        TEST_CASE(varidReturn2);
        TEST_CASE(varid8);
        TEST_CASE(varid9);
        TEST_CASE(varid10);
        TEST_CASE(varid11);
        TEST_CASE(varid12);
        TEST_CASE(varid13);
        TEST_CASE(varid14);
        TEST_CASE(varid15);
        TEST_CASE(varid16);
        TEST_CASE(varid17);   // ticket #1810
        TEST_CASE(varid18);
        TEST_CASE(varid19);
        TEST_CASE(varid20);
        TEST_CASE(varid24);
        TEST_CASE(varid25);
        TEST_CASE(varid26);   // ticket #1967 (list of function pointers)
        TEST_CASE(varid27);   // Ticket #2280 (same name for namespace and variable)
        TEST_CASE(varid28);   // ticket #2630
        TEST_CASE(varid29);   // ticket #1974
        TEST_CASE(varid30);   // ticket #2614
        TEST_CASE(varid31);   // ticket #2831 (segmentation fault)
        TEST_CASE(varid32);   // ticket #2835 (segmentation fault)
        TEST_CASE(varid33);   // ticket #2875 (segmentation fault)
        TEST_CASE(varid34);   // ticket #2825
        TEST_CASE(varid35);   // ticket #2937
        TEST_CASE(varid36);   // ticket #2980 (segmentation fault)
        TEST_CASE(varid37);   // ticket #3092 (varid for 'Bar bar(*this);')
        TEST_CASE(varid38);   // ticket #3272 (varid for 'FOO class C;')
        TEST_CASE(varid39);   // ticket #3279 (varid for 'FOO::BAR const')
        TEST_CASE(varid40);   // ticket #3279
        TEST_CASE(varid41);   // ticket #3340 (varid for union type)
        TEST_CASE(varid42);   // ticket #3316 (varid for array)
        TEST_CASE(varid43);
        TEST_CASE(varid44);
        TEST_CASE(varid45); // #3466
        TEST_CASE(varid46); // struct varname
        TEST_CASE(varid47); // function parameters
        TEST_CASE(varid48); // #3785 - return (a*b)
        TEST_CASE(varid49); // #3799 - void f(std::vector<int>)
        TEST_CASE(varid50); // #3760 - explicit
        TEST_CASE(varid51); // don't set varid for template function
        TEST_CASE(varid52); // Set varid for nested templates
        TEST_CASE(varid53); // #4172 - Template instantiation: T<&functionName> list[4];
        TEST_CASE(varid54); // hang
        TEST_CASE(varid55); // #5868: Function::addArgument with varid 0 for argument named the same as a typedef
        TEST_CASE(varid_cpp_keywords_in_c_code);
        TEST_CASE(varid_cpp_keywords_in_c_code2); // #5373: varid=0 for argument called "delete"
        TEST_CASE(varidFunctionCall1);
        TEST_CASE(varidFunctionCall2);
        TEST_CASE(varidFunctionCall3);
        TEST_CASE(varidFunctionCall4);  // ticket #3280
        TEST_CASE(varidStl);
        TEST_CASE(varid_newauto);       // not declaration: new const auto(0);
        TEST_CASE(varid_delete);
        TEST_CASE(varid_functions);
        TEST_CASE(varid_sizeof);
        TEST_CASE(varid_reference_to_containers);
        TEST_CASE(varid_in_class1);
        TEST_CASE(varid_in_class2);
        TEST_CASE(varid_in_class3);     // #3092 - shadow variable in member function
        TEST_CASE(varid_in_class4);     // #3271 - public: class C;
        TEST_CASE(varid_in_class5);     // #3584 - std::vector<::FOO::B> b;
        TEST_CASE(varid_in_class6);     // #3755
        TEST_CASE(varid_in_class7);     // set variable id for struct members
        TEST_CASE(varid_in_class8);     // unknown macro in class
        TEST_CASE(varid_in_class9);     // #4291 - id for variables accessed through 'this'
        TEST_CASE(varid_in_class10);
        TEST_CASE(varid_in_class11);    // #4277 - anonymous union
        TEST_CASE(varid_in_class12);    // #4637 - method
        TEST_CASE(varid_in_class13);    // #4637 - method
        TEST_CASE(varid_in_class14);
        TEST_CASE(varid_in_class15);    // #5533 - functions
        TEST_CASE(varid_in_class16);
        TEST_CASE(varid_in_class17);    // #6056 - no varid for member functions
        TEST_CASE(varid_initList);
        TEST_CASE(varid_operator);
        TEST_CASE(varid_throw);
        TEST_CASE(varid_unknown_macro);     // #2638 - unknown macro is not type
        TEST_CASE(varid_using);  // ticket #3648
        TEST_CASE(varid_catch);
        TEST_CASE(varid_functionPrototypeTemplate);
        TEST_CASE(varid_templatePtr); // #4319
        TEST_CASE(varid_templateNamespaceFuncPtr); // #4172
        TEST_CASE(varid_templateArray);
        TEST_CASE(varid_cppcast); // #6190
        TEST_CASE(varid_variadicFunc);
        TEST_CASE(varid_typename); // #4644
        TEST_CASE(varid_rvalueref);
        TEST_CASE(varid_arrayFuncPar); // #5294
        TEST_CASE(varid_sizeofPassed); // #5295
        TEST_CASE(varid_classInFunction); // #5293
        TEST_CASE(varid_pointerToArray); // #2645
        TEST_CASE(varid_cpp11initialization); // #4344
        TEST_CASE(varid_inheritedMembers); // #4101
        TEST_CASE(varid_header); // #6386

        TEST_CASE(varidclass1);
        TEST_CASE(varidclass2);
        TEST_CASE(varidclass3);
        TEST_CASE(varidclass4);
        TEST_CASE(varidclass5);
        TEST_CASE(varidclass6);
        TEST_CASE(varidclass7);
        TEST_CASE(varidclass8);
        TEST_CASE(varidclass9);
        TEST_CASE(varidclass10);  // variable declaration below usage
        TEST_CASE(varidclass11);  // variable declaration below usage
        TEST_CASE(varidclass12);
        TEST_CASE(varidclass13);
        TEST_CASE(varidclass14);
        TEST_CASE(varidclass15);  // initializer list
        TEST_CASE(varidclass16);  // #4577
        TEST_CASE(varidclass17);  // #6073
        TEST_CASE(varid_classnameshaddowsvariablename); // #3990

        TEST_CASE(varidnamespace1);
    }

    std::string tokenize(const char code[], bool simplify = false, const char filename[] = "test.cpp") {
        errout.str("");

        Settings settings;
        settings.standards.c   = Standards::C89;
        settings.standards.cpp = Standards::CPP11;

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        if (simplify)
            tokenizer.simplifyTokenList2();

        // result..
        return tokenizer.tokens()->stringifyList(true);
    }

    void varid1() {
        {
            const std::string actual = tokenize(
                                           "static int i = 1;\n"
                                           "void f()\n"
                                           "{\n"
                                           "    int i = 2;\n"
                                           "    for (int i = 0; i < 10; ++i)\n"
                                           "        i = 3;\n"
                                           "    i = 4;\n"
                                           "}\n", false, "test.c");

            const std::string expected("\n\n##file 0\n"
                                       "1: static int i@1 = 1 ;\n"
                                       "2: void f ( )\n"
                                       "3: {\n"
                                       "4: int i@2 ; i@2 = 2 ;\n"
                                       "5: for ( int i@3 = 0 ; i@3 < 10 ; ++ i@3 ) {\n"
                                       "6: i@3 = 3 ; }\n"
                                       "7: i@2 = 4 ;\n"
                                       "8: }\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                                           "static int i = 1;\n"
                                           "void f()\n"
                                           "{\n"
                                           "    int i = 2;\n"
                                           "    for (int i = 0; i < 10; ++i)\n"
                                           "    {\n"
                                           "      i = 3;\n"
                                           "    }\n"
                                           "    i = 4;\n"
                                           "}\n", false, "test.c");

            const std::string expected("\n\n##file 0\n"
                                       "1: static int i@1 = 1 ;\n"
                                       "2: void f ( )\n"
                                       "3: {\n"
                                       "4: int i@2 ; i@2 = 2 ;\n"
                                       "5: for ( int i@3 = 0 ; i@3 < 10 ; ++ i@3 )\n"
                                       "6: {\n"
                                       "7: i@3 = 3 ;\n"
                                       "8: }\n"
                                       "9: i@2 = 4 ;\n"
                                       "10: }\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid2() {
        const std::string actual = tokenize(
                                       "void f()\n"
                                       "{\n"
                                       "    struct ABC abc;\n"
                                       "    abc.a = 3;\n"
                                       "    i = abc.a;\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: struct ABC abc@1 ;\n"
                                   "4: abc@1 . a@2 = 3 ;\n"
                                   "5: i = abc@1 . a@2 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid3() {
        const std::string actual = tokenize(
                                       "static char str[4];\n"
                                       "void f()\n"
                                       "{\n"
                                       "    char str[10];\n"
                                       "    str[0] = 0;\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: static char str@1 [ 4 ] ;\n"
                                   "2: void f ( )\n"
                                   "3: {\n"
                                   "4: char str@2 [ 10 ] ;\n"
                                   "5: str@2 [ 0 ] = 0 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid4() {
        const std::string actual = tokenize(
                                       "void f(const unsigned int a[])\n"
                                       "{\n"
                                       "    int i = *(a+10);\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( const int a@1 [ ] )\n"
                                   "2: {\n"
                                   "3: int i@2 ; i@2 = * ( a@1 + 10 ) ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid5() {
        const std::string actual = tokenize(
                                       "void f()\n"
                                       "{\n"
                                       "    int a,b;\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: int a@1 ; int b@2 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varid6() {
        const std::string actual = tokenize(
                                       "int f(int a, int b)\n"
                                       "{\n"
                                       "    return a+b;\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: int f ( int a@1 , int b@2 )\n"
                                   "2: {\n"
                                   "3: return a@1 + b@2 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varid7() {
        const std::string actual = tokenize(
                                       "void func() {\n"
                                       "    char a[256] = \"test\";\n"
                                       "    {\n"
                                       "        char b[256] = \"test\";\n"
                                       "    }\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void func ( ) {\n"
                                   "2: char a@1 [ 256 ] = \"test\" ;\n"
                                   "3: {\n"
                                   "4: char b@2 [ 256 ] = \"test\" ;\n"
                                   "5: }\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidReturn1() {
        const std::string actual = tokenize(
                                       "int f()\n"
                                       "{\n"
                                       "    int a;\n"
                                       "    return a;\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: int f ( )\n"
                                   "2: {\n"
                                   "3: int a@1 ;\n"
                                   "4: return a@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidReturn2() {
        const std::string actual = tokenize(
                                       "void foo()\n"
                                       "{\n"
                                       "    unsigned long mask = (1UL << size_) - 1;\n"
                                       "    return (abits_val_ & mask);\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: long mask@1 ; mask@1 = ( 1UL << size_ ) - 1 ;\n"
                                   "4: return ( abits_val_ & mask@1 ) ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid8() {
        const std::string actual = tokenize(
                                       "void func()\n"
                                       "{\n"
                                       "    std::string str(\"test\");\n"
                                       "    str.clear();\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void func ( )\n"
                                   "2: {\n"
                                   "3: std :: string str@1 ( \"test\" ) ;\n"
                                   "4: str@1 . clear ( ) ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid9() {
        const std::string actual = tokenize(
                                       "typedef int INT32;\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid10() {
        const std::string actual = tokenize(
                                       "void foo()\n"
                                       "{\n"
                                       "    int abc;\n"
                                       "    struct abc abc1;\n"
                                       "}", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: int abc@1 ;\n"
                                   "4: struct abc abc1@2 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid11() {
        const std::string actual = tokenize(
                                       "class Foo;\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Foo ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid12() {
        const std::string actual = tokenize(
                                       "static void a()\n"
                                       "{\n"
                                       "    class Foo *foo;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: static void a ( )\n"
                                   "2: {\n"
                                   "3: class Foo * foo@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid13() {
        const std::string actual = tokenize(
                                       "void f()\n"
                                       "{\n"
                                       "    int a; int b;\n"
                                       "    a = a;\n"
                                       "}\n", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: int a@1 ; int b@2 ;\n"
                                   "4: a@1 = a@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid14() {
        // Overloaded operator*
        const std::string actual = tokenize(
                                       "void foo()\n"
                                       "{\n"
                                       "A a;\n"
                                       "B b;\n"
                                       "b * a;\n"
                                       "}", false, "test.c");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: A a@1 ;\n"
                                   "4: B b@2 ;\n"
                                   "5: b@2 * a@1 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid15() {
        {
            const std::string actual = tokenize(
                                           "struct S {\n"
                                           "    struct T {\n"
                                           "    } t;\n"
                                           "} s;", false, "test.c");

            const std::string expected("\n\n##file 0\n"
                                       "1: struct S {\n"
                                       "2: struct T {\n"
                                       "3: } ; struct T t@1 ;\n"
                                       "4: } ; struct S s@2 ;\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                                           "struct S {\n"
                                           "    struct T {\n"
                                           "    } t;\n"
                                           "};", false, "test.c");

            const std::string expected("\n\n##file 0\n"
                                       "1: struct S {\n"
                                       "2: struct T {\n"
                                       "3: } ; struct T t@1 ;\n"
                                       "4: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid16() {
        const char code[] ="void foo()\n"
                           "{\n"
                           "    int x = 1;\n"
                           "    y = (z * x);\n"
                           "}\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: int x@1 ; x@1 = 1 ;\n"
                                   "4: y = z * x@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, tokenize(code, false, "test.c"));
    }

    void varid17() { // ticket #1810
        const char code[] ="char foo()\n"
                           "{\n"
                           "    char c('c');\n"
                           "    return c;\n"
                           "}\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: char foo ( )\n"
                                   "2: {\n"
                                   "3: char c@1 ( 'c' ) ;\n"
                                   "4: return c@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, tokenize(code, false, "test.c"));
    }

    void varid18() {
        const char code[] ="char foo(char c)\n"
                           "{\n"
                           "    bar::c = c;\n"
                           "}\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: char foo ( char c@1 )\n"
                                   "2: {\n"
                                   "3: bar :: c = c@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid19() {
        const char code[] ="void foo()\n"
                           "{\n"
                           "    std::pair<std::vector<double>, int> x;\n"
                           "}\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: std :: pair < std :: vector < double > , int > x@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid20() {
        const char code[] ="void foo()\n"
                           "{\n"
                           "    pair<vector<int>, vector<double> > x;\n"
                           "}\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: pair < vector < int > , vector < double > > x@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid24() {
        const char code[] ="class foo()\n"
                           "{\n"
                           "public:\n"
                           "    ;\n"
                           "private:\n"
                           "    static int i;\n"
                           "};\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: class foo ( )\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: ;\n"
                                   "5: private:\n"
                                   "6: static int i@1 ;\n"
                                   "7: } ;\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid25() {
        const char code[] ="class foo()\n"
                           "{\n"
                           "public:\n"
                           "    ;\n"
                           "private:\n"
                           "    mutable int i;\n"
                           "};\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: class foo ( )\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: ;\n"
                                   "5: private:\n"
                                   "6: mutable int i@1 ;\n"
                                   "7: } ;\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid26() {
        const char code[] ="list<int (*)()> functions;\n";
        const std::string expected("\n\n##file 0\n"
                                   "1: list < int ( * ) ( ) > functions@1 ;\n");
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid27() {
        const char code[] ="int fooled_ya;\n"
                           "fooled_ya::iterator iter;\n";
        const std::string expected("\n\n##file 0\n"
                                   "1: int fooled_ya@1 ;\n"
                                   "2: fooled_ya :: iterator iter@2 ;\n");
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid28() { // ticket #2630 (segmentation fault)
        tokenize("template <typedef A>\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varid29() {
        const char code[] ="class A {\n"
                           "    B<C<1>,1> b;\n"
                           "};\n";
        const std::string expected("\n\n##file 0\n"
                                   "1: class A {\n"
                                   "2: B < C < 1 > , 1 > b@1 ;\n"
                                   "3: } ;\n");
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid30() { // ticket #2614
        const char code1[] = "void f(EventPtr *eventP, ActionPtr **actionsP)\n"
                             "{\n"
                             "    EventPtr event = *eventP;\n"
                             "    *actionsP = &event->actions;\n"
                             "}\n";
        const std::string expected1("\n\n##file 0\n"
                                    "1: void f ( EventPtr * eventP@1 , ActionPtr * * actionsP@2 )\n"
                                    "2: {\n"
                                    "3: EventPtr event@3 ; event@3 = * eventP@1 ;\n"
                                    "4: * actionsP@2 = & event@3 . actions@4 ;\n"
                                    "5: }\n");
        ASSERT_EQUALS(expected1, tokenize(code1, false, "test.c"));

        const char code2[] = "void f(int b, int c) {\n"
                             "    x(a*b*c,10);\n"
                             "}\n";
        const std::string expected2("\n\n##file 0\n"
                                    "1: void f ( int b@1 , int c@2 ) {\n"
                                    "2: x ( a * b@1 * c@2 , 10 ) ;\n"
                                    "3: }\n");
        ASSERT_EQUALS(expected2, tokenize(code2, false, "test.c"));

        const char code3[] = "class Nullpointer : public ExecutionPath\n"
                             " {\n"
                             "    Nullpointer(Check *c, const unsigned int id, const std::string &name)\n"
                             "        : ExecutionPath(c, id)\n"
                             "    {\n"
                             "    }\n"
                             "}\n";
        const std::string expected3("\n\n##file 0\n"
                                    "1: class Nullpointer : public ExecutionPath\n"
                                    "2: {\n"
                                    "3: Nullpointer ( Check * c@1 , const int id@2 , const std :: string & name@3 )\n"
                                    "4: : ExecutionPath ( c@1 , id@2 )\n"
                                    "5: {\n"
                                    "6: }\n"
                                    "7: }\n");
        ASSERT_EQUALS(expected3, tokenize(code3));
    }

    void varid31() { // ticket #2831 (segmentation fault)
        const char code[] ="z<y<x>";
        tokenize(code);
        ASSERT_EQUALS("", errout.str());
    }

    void varid32() { // ticket #2835 (segmentation fault)
        const char code[] ="><,f<i,";
        tokenize(code);
        ASSERT_EQUALS("", errout.str());
    }

    void varid33() { // ticket #2875 (segmentation fault)
        const char code[] ="0; (a) < (a)";
        tokenize(code, true);
        ASSERT_EQUALS("", errout.str());
    }

    void varid34() { // ticket #2825
        const char code[] ="class Fred : public B1, public B2\n"
                           "{\n"
                           "public:\n"
                           "    Fred() { a = 0; }\n"
                           "private:\n"
                           "    int a;\n"
                           "};\n";
        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred : public B1 , public B2\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: Fred ( ) { a@1 = 0 ; }\n"
                                   "5: private:\n"
                                   "6: int a@1 ;\n"
                                   "7: } ;\n");
        ASSERT_EQUALS(expected, tokenize(code));
        ASSERT_EQUALS("", errout.str());
    }

    void varid35() { // ticket #2937
        const char code[] ="int foo() {\n"
                           "    int f(x);\n"
                           "    return f;\n"
                           "}\n";
        const std::string expected("\n\n##file 0\n"
                                   "1: int foo ( ) {\n"
                                   "2: int f@1 ( x ) ;\n"
                                   "3: return f@1 ;\n"
                                   "4: }\n");
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid36() { // ticket #2980 (segmentation fault)
        const char code[] ="#elif A\n"
                           "A,a<b<x0\n";
        tokenize(code);
        ASSERT_EQUALS("", errout.str());
    }

    void varid37() {
        {
            const char code[] = "void blah() {"
                                "    Bar bar(*x);"
                                "}";
            ASSERT_EQUALS("\n\n##file 0\n1: "
                          "void blah ( ) { Bar bar@1 ( * x ) ; }\n",
                          tokenize(code));
        }
        {
            const char code[] = "void blah() {"
                                "    Bar bar(&x);"
                                "}";
            ASSERT_EQUALS("\n\n##file 0\n1: "
                          "void blah ( ) { Bar bar@1 ( & x ) ; }\n",
                          tokenize(code));
        }
    }

    void varid38() {
        const char code[] = "FOO class C;\n";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: FOO class C ;\n",
                      tokenize(code));
    }

    void varid39() {
        // const..
        {
            const char code[] = "void f(FOO::BAR const);\n";
            ASSERT_EQUALS("\n\n##file 0\n"
                          "1: void f ( FOO :: BAR const ) ;\n",
                          tokenize(code));
        }
        {
            const char code[] = "static int const SZ = 22;\n";
            ASSERT_EQUALS("\n\n##file 0\n"
                          "1: static const int SZ@1 = 22 ;\n",
                          tokenize(code, false, "test.c"));
        }
    }

    void varid40() {
        const char code[] ="extern \"C\" int (*a())();";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int * a ( ) ;\n",
                      tokenize(code));
    }

    void varid41() {
        const char code1[] = "union evt; void f(const evt & event);";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: union evt ; void f ( const evt & event@1 ) ;\n",
                      tokenize(code1, false, "test.c"));

        const char code2[] = "struct evt; void f(const evt & event);";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct evt ; void f ( const evt & event@1 ) ;\n",
                      tokenize(code2, false, "test.c"));
    }

    void varid42() {
        const char code[] ="namespace fruit { struct banana {}; };\n"
                           "class Fred {\n"
                           "public:\n"
                           "     struct fruit::banana Bananas[25];\n"
                           "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: namespace fruit { struct banana { } ; } ;\n"
                      "2: class Fred {\n"
                      "3: public:\n"
                      "4: struct fruit :: banana Bananas@1 [ 25 ] ;\n"
                      "5: } ;\n",
                      tokenize(code));
    }

    void varid43() {
        const char code[] ="int main(int flag) { if(a & flag) { return 1; } }";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int main ( int flag@1 ) { if ( a & flag@1 ) { return 1 ; } }\n",
                      tokenize(code, false, "test.c"));
    }

    void varid44() {
        const char code[] ="class A:public B,public C,public D {};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A : public B , public C , public D { } ;\n",
                      tokenize(code));
    }

    void varid45() { // #3466
        const char code[] ="void foo() { B b(this); A a(this, b); }";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void foo ( ) { B b@1 ( this ) ; A a@2 ( this , b@1 ) ; }\n",
                      tokenize(code));
    }

    void varid46() { // #3756
        const char code[] ="void foo() { int t; x = (struct t *)malloc(); f(t); }";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void foo ( ) { int t@1 ; x = ( struct t * ) malloc ( ) ; f ( t@1 ) ; }\n",
                      tokenize(code, false, "test.c"));
    }

    void varid47() { // function parameters
        // #3768
        {
            const char code[] ="void f(std::string &string, std::string &len) {}";
            ASSERT_EQUALS("\n\n##file 0\n"
                          "1: void f ( std :: string & string@1 , std :: string & len@2 ) { }\n",
                          tokenize(code, false, "test.cpp"));
        }

        // #4729
        {
            const char code[] = "int x;\n"
                                "void a(int x);\n"
                                "void b() { x = 0; }\n";
            ASSERT_EQUALS("\n\n##file 0\n"
                          "1: int x@1 ;\n"
                          "2: void a ( int x@2 ) ;\n"
                          "3: void b ( ) { x@1 = 0 ; }\n",
                          tokenize(code));
        }
    }

    void varid48() {  // #3785 - return (a*b)
        const char code[] ="int X::f(int b) const { return(a*b); }";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int X :: f ( int b@1 ) const { return ( a * b@1 ) ; }\n",
                      tokenize(code, false, "test.c"));
    }

    void varid49() {  // #3799 - void f(std::vector<int>)
        const char code[] ="void f(std::vector<int>)";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void f ( std :: vector < int > )\n",
                      tokenize(code, false, "test.cpp"));
    }

    void varid50() {  // #3760 - explicit
        const char code[] ="class A { explicit A(const A&); };";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A { explicit A ( const A & ) ; } ;\n",
                      tokenize(code, false, "test.cpp"));
    }

    void varid51() {  // don't set varid on template function
        const char code[] ="T t; t.x<0>();";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: T t@1 ; t@1 . x < 0 > ( ) ;\n",
                      tokenize(code, false, "test.cpp"));
    }

    void varid52() {
        const char code[] ="A<B<C>::D> e;\n"
                           "B< C<> > b[10];\n"
                           "B<C<>> c[10];";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: A < B < C > :: D > e@1 ;\n"
                      "2: B < C < > > b@2 [ 10 ] ;\n"
                      "3: B < C < > > c@3 [ 10 ] ;\n",
                      tokenize(code, false, "test.cpp"));
    }

    void varid53() { // #4172 - Template instantiation: T<&functionName> list[4];
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: A < & f > list@1 [ 4 ] ;\n",
                      tokenize("A<&f> list[4];", false, "test.cpp"));
    }

    void varid54() { // hang
        // Original source code: libgc
        tokenize("STATIC ptr_t GC_approx_sp(void) { word sp; sp = (word)&sp; return((ptr_t)sp); }",true);
    }

    void varid55() { // Ticket #5868
        const char code[] =     "typedef struct foo {} foo; "
                                "void bar1(struct foo foo) {} "
                                "void baz1(foo foo) {} "
                                "void bar2(struct foo& foo) {} "
                                "void baz2(foo& foo) {} "
                                "void bar3(struct foo* foo) {} "
                                "void baz3(foo* foo) {}";
        const char expected[] = "\n\n##file 0\n1: struct foo { } ; "
                                "void bar1 ( struct foo foo@1 ) { } "
                                "void baz1 ( struct foo foo@2 ) { } "
                                "void bar2 ( struct foo & foo@3 ) { } "
                                "void baz2 ( struct foo & foo@4 ) { } "
                                "void bar3 ( struct foo * foo@5 ) { } "
                                "void baz3 ( struct foo * foo@6 ) { }\n";
        ASSERT_EQUALS(expected, tokenize(code, false, "test.cpp"));
    }

    void varid_cpp_keywords_in_c_code() {
        const char code[] = "void f() {\n"
                            "    delete d;\n"
                            "    throw t;\n"
                            "}";

        const char expected[] = "\n\n##file 0\n"
                                "1: void f ( ) {\n"
                                "2: delete d@1 ;\n"
                                "3: throw t@2 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, tokenize(code,false,"test.c"));
    }

    void varid_cpp_keywords_in_c_code2() { // #5373
        const char code[] = "int clear_extent_bit(struct extent_io_tree *tree, u64 start, u64 end, "
                            "unsigned long bits, int wake, int delete, struct extent_state **cached_state, "
                            "gfp_t mask) {\n"
                            "  struct extent_state *state;\n"
                            "}"
                            "int clear_extent_dirty() {\n"
                            "  return clear_extent_bit(tree, start, end, EXTENT_DIRTY | EXTENT_DELALLOC | "
                            "                          EXTENT_DO_ACCOUNTING, 0, 0, NULL, mask);\n"
                            "}";
        tokenize(code, false, "test.c");
    }

    void varidFunctionCall1() {
        const char code[] ="void f() {\n"
                           "    int x;\n"
                           "    x = a(y*x,10);\n"
                           "}";
        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( ) {\n"
                                   "2: int x@1 ;\n"
                                   "3: x@1 = a ( y * x@1 , 10 ) ;\n"
                                   "4: }\n");
        ASSERT_EQUALS(expected, tokenize(code, false, "test.c"));
    }

    void varidFunctionCall2() {
        // #2491
        const char code[] ="void f(int b) {\n"
                           "    x(a*b,10);\n"
                           "}";
        const std::string expected1("\n\n##file 0\n"
                                    "1: void f ( int b@1 ) {\n"
                                    "2: x ( a * b");
        const std::string expected2(" , 10 ) ;\n"
                                    "3: }\n");
        ASSERT_EQUALS(expected1+"@1"+expected2, tokenize(code,false,"test.c"));
    }

    void varidFunctionCall3() {
        // Ticket #2339
        const char code[] ="void f() {\n"
                           "    int a = 0;\n"
                           "    int b = c - (foo::bar * a);\n"
                           "}";

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( ) {\n"
                                   "2: int a@1 ; a@1 = 0 ;\n"
                                   "3: int b@2 ; b@2 = c - ( foo :: bar * a@1 ) ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidFunctionCall4() {
        // Ticket #3280
        const char code1[] = "void f() { int x; fun(a,b*x); }";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void f ( ) { int x@1 ; fun ( a , b * x@1 ) ; }\n",
                      tokenize(code1, false, "test.c"));
        const char code2[] = "void f(int a) { int x; fun(a,b*x); }";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void f ( int a@1 ) { int x@2 ; fun ( a@1 , b * x@2 ) ; }\n",
                      tokenize(code2, false, "test.c"));
    }


    void varidStl() {
        const std::string actual = tokenize(
                                       "list<int> ints;\n"
                                       "list<int>::iterator it;\n"
                                       "std::vector<std::string> dirs;\n"
                                       "std::map<int, int> coords;\n"
                                       "std::tr1::unordered_map<int, int> xy;\n"
                                       "std::list<boost::wave::token_id> tokens;\n"
                                       "static std::vector<CvsProcess*> ex1;\n"
                                       "extern std::vector<CvsProcess*> ex2;\n"
                                       "std::map<int, 1> m;\n"
                                   );

        const std::string expected("\n\n##file 0\n"
                                   "1: list < int > ints@1 ;\n"
                                   "2: list < int > :: iterator it@2 ;\n"
                                   "3: std :: vector < std :: string > dirs@3 ;\n"
                                   "4: std :: map < int , int > coords@4 ;\n"
                                   "5: std :: unordered_map < int , int > xy@5 ;\n"
                                   "6: std :: list < boost :: wave :: token_id > tokens@6 ;\n"
                                   "7: static std :: vector < CvsProcess * > ex1@7 ;\n"
                                   "8: extern std :: vector < CvsProcess * > ex2@8 ;\n"
                                   "9: std :: map < int , 1 > m@9 ;\n"
                                  );

        ASSERT_EQUALS(expected, actual);
    }

    void varid_newauto() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void f ( ) { const new auto ( 0 ) ; }\n",
                      tokenize("void f(){new const auto(0);}"));
    }

    void varid_delete() {
        const std::string actual = tokenize(
                                       "void f()\n"
                                       "{\n"
                                       "  int *a;\n"
                                       "  delete a;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: int * a@1 ;\n"
                                   "4: delete a@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid_functions() {
        {
            const std::string actual = tokenize(
                                           "void f();\n"
                                           "void f(){}\n", false, "test.c");

            const std::string expected("\n\n##file 0\n"
                                       "1: void f ( ) ;\n"
                                       "2: void f ( ) { }\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                                           "A f(3);\n"
                                           "A f2(true);\n"
                                           "A g();\n"
                                           "A e(int c);\n", false, "test.c");

            const std::string expected("\n\n##file 0\n"
                                       "1: A f@1 ( 3 ) ;\n"
                                       "2: A f2@2 ( true ) ;\n"
                                       "3: A g ( ) ;\n"
                                       "4: A e ( int c@3 ) ;\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                                           "void f1(int &p)\n"
                                           "{\n"
                                           "    p = 0;\n"
                                           "}\n"
                                           "void f2(std::string &str)\n"
                                           "{\n"
                                           "   str.clear();\n"
                                           "}\n"
                                           "void f3(const std::string &s)\n"
                                           "{\n"
                                           "    s.size();\n"
                                           "}\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: void f1 ( int & p@1 )\n"
                                       "2: {\n"
                                       "3: p@1 = 0 ;\n"
                                       "4: }\n"
                                       "5: void f2 ( std :: string & str@2 )\n"
                                       "6: {\n"
                                       "7: str@2 . clear ( ) ;\n"
                                       "8: }\n"
                                       "9: void f3 ( const std :: string & s@3 )\n"
                                       "10: {\n"
                                       "11: s@3 . size ( ) ;\n"
                                       "12: }\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize("void f(struct foobar);", false, "test.c");
            const std::string expected("\n\n##file 0\n"
                                       "1: void f ( struct foobar ) ;\n");
            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize("bool f(X x, int=3);", false, "test.cpp");
            const std::string expected("\n\n##file 0\n"
                                       "1: bool f ( X x@1 , int = 3 ) ;\n");
            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_sizeof() {
        const char code[] = "x = sizeof(a*b);";
        const char expected[] = "\n\n##file 0\n"
                                "1: x = sizeof ( a * b ) ;\n";
        ASSERT_EQUALS(expected, tokenize(code,false,"test.c"));
    }

    void varid_reference_to_containers() {
        const std::string actual = tokenize(
                                       "void f()\n"
                                       "{\n"
                                       "    std::vector<int> b;\n"
                                       "    std::vector<int> &a = b;\n"
                                       "    std::vector<int> *c = &b;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: std :: vector < int > b@1 ;\n"
                                   "4: std :: vector < int > & a@2 = b@1 ;\n"
                                   "5: std :: vector < int > * c@3 ; c@3 = & b@1 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid_in_class1() {
        {
            const std::string actual = tokenize(
                                           "class Foo\n"
                                           "{\n"
                                           "public:\n"
                                           "    std::string name1;\n"
                                           "    std::string name2;\n"
                                           "};\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: class Foo\n"
                                       "2: {\n"
                                       "3: public:\n"
                                       "4: std :: string name1@1 ;\n"
                                       "5: std :: string name2@2 ;\n"
                                       "6: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                                           "class foo\n"
                                           "{\n"
                                           "public:\n"
                                           "    void do_something(const int x, const int y);\n"
                                           "    void bar();\n"
                                           "};\n"
                                           "\n"
                                           "void foo::bar()\n"
                                           "{\n"
                                           "    POINT pOutput = { 0 , 0 };\n"
                                           "    int x = pOutput.x;\n"
                                           "    int y = pOutput.y;\n"
                                           "}\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: class foo\n"
                                       "2: {\n"
                                       "3: public:\n"
                                       "4: void do_something ( const int x@1 , const int y@2 ) ;\n"
                                       "5: void bar ( ) ;\n"
                                       "6: } ;\n"
                                       "7:\n"
                                       "8: void foo :: bar ( )\n"
                                       "9: {\n"
                                       "10: POINT pOutput@3 ; pOutput@3 = { 0 , 0 } ;\n"
                                       "11: int x@4 ; x@4 = pOutput@3 . x@5 ;\n"
                                       "12: int y@6 ; y@6 = pOutput@3 . y@7 ;\n"
                                       "13: }\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_in_class2() {
        const std::string actual = tokenize(
                                       "struct Foo {\n"
                                       "    int x;\n"
                                       "};\n"
                                       "\n"
                                       "struct Bar {\n"
                                       "    Foo foo;\n"
                                       "    int x;\n"
                                       "    void f();\n"
                                       "};\n"
                                       "\n"
                                       "void Bar::f()\n"
                                       "{\n"
                                       "    foo.x = x;\n"
                                       "}\n");
        const std::string expected("\n\n##file 0\n"
                                   "1: struct Foo {\n"
                                   "2: int x@1 ;\n"
                                   "3: } ;\n"
                                   "4:\n"
                                   "5: struct Bar {\n"
                                   "6: Foo foo@2 ;\n"
                                   "7: int x@3 ;\n"
                                   "8: void f ( ) ;\n"
                                   "9: } ;\n"
                                   "10:\n"
                                   "11: void Bar :: f ( )\n"
                                   "12: {\n"
                                   "13: foo@2 . x@4 = x@3 ;\n"
                                   "14: }\n");
        ASSERT_EQUALS(expected, actual);
    }

    void varid_in_class3() {
        const char code[] = "class Foo {\n"
                            "    void blah() {\n"
                            "        Bar x(*this);\n"  // <- ..
                            "    }\n"
                            "    int x;\n"   // <- .. don't assign same varid
                            "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Foo {\n"
                      "2: void blah ( ) {\n"
                      "3: Bar x@1 ( * this ) ;\n"
                      "4: }\n"
                      "5: int x@2 ;\n"
                      "6: } ;\n", tokenize(code));
    }

    void varid_in_class4() {
        const char code[] = "class Foo {\n"
                            "public: class C;\n"
                            "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Foo {\n"
                      "2: public: class C ;\n"
                      "3: } ;\n",
                      tokenize(code));
    }

    void varid_in_class5() {
        const char code[] = "struct Foo {\n"
                            "    std::vector<::X> v;\n"
                            "}";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct Foo {\n"
                      "2: std :: vector < :: X > v@1 ;\n"
                      "3: }\n",
                      tokenize(code));
    }

    void varid_in_class6() {
        const char code[] = "class A {\n"
                            "    void f(const char *str) const {\n"
                            "        std::stringstream sst;\n"
                            "        sst.str();\n"
                            "    }\n"
                            "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: void f ( const char * str@1 ) const {\n"
                      "3: std :: stringstream sst@2 ;\n"
                      "4: sst@2 . str ( ) ;\n"
                      "5: }\n"
                      "6: } ;\n",
                      tokenize(code));
    }

    void varid_in_class7() {
        const char code[] = "class A {\n"
                            "    void f() {\n"
                            "        abc.a = 0;\n"
                            "    }\n"
                            "    struct ABC abc;\n"
                            "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: void f ( ) {\n"
                      "3: abc@1 . a@2 = 0 ;\n"
                      "4: }\n"
                      "5: struct ABC abc@1 ;\n"
                      "6: } ;\n",
                      tokenize(code));
    }

    void varid_in_class8() {  // #3776 - unknown macro
        const char code[] = "class A {\n"
                            "  UNKNOWN_MACRO(A)\n"
                            "private:\n"
                            "  int x;\n"
                            "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: UNKNOWN_MACRO ( A )\n"
                      "3: private:\n"
                      "4: int x@1 ;\n"
                      "5: } ;\n",
                      tokenize(code));
    }

    void varid_in_class9() {  // #4291 - id for variables accessed through 'this'
        const char code1[] = "class A {\n"
                             "  int var;\n"
                             "public:\n"
                             "  void setVar();\n"
                             "};\n"
                             "void A::setVar() {\n"
                             "  this->var = var;\n"
                             "}";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: int var@1 ;\n"
                      "3: public:\n"
                      "4: void setVar ( ) ;\n"
                      "5: } ;\n"
                      "6: void A :: setVar ( ) {\n"
                      "7: this . var@1 = var@1 ;\n"
                      "8: }\n",
                      tokenize(code1));

        const char code2[] = "class Foo : public FooBase {\n"
                             "    void Clone(FooBase& g);\n"
                             "    short m_bar;\n"
                             "};\n"
                             "void Foo::Clone(FooBase& g) {\n"
                             "    g->m_bar = m_bar;\n"
                             "}";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Foo : public FooBase {\n"
                      "2: void Clone ( FooBase & g@1 ) ;\n"
                      "3: short m_bar@2 ;\n"
                      "4: } ;\n"
                      "5: void Foo :: Clone ( FooBase & g@3 ) {\n"
                      "6: g@3 . m_bar@4 = m_bar@2 ;\n"
                      "7: }\n",
                      tokenize(code2)); // #4311
    }

    void varid_in_class10() {
        const char code[] = "class Foo : public FooBase {\n"
                            "    void Clone(FooBase& g);\n"
                            "    short m_bar;\n"
                            "};\n"
                            "void Foo::Clone(FooBase& g) {\n"
                            "    ((FooBase)g)->m_bar = m_bar;\n"
                            "}";
        TODO_ASSERT_EQUALS("\n\n##file 0\n"
                           "1: class Foo : public FooBase {\n"
                           "2: void Clone ( FooBase & g@1 ) ;\n"
                           "3: short m_bar@2 ;\n"
                           "4: } ;\n"
                           "5: void Foo :: Clone ( FooBase & g@3 ) {\n"
                           "6: ( ( FooBase ) g@3 ) . m_bar@4 = m_bar@2 ;\n"
                           "7: }\n",
                           "\n\n##file 0\n"
                           "1: class Foo : public FooBase {\n"
                           "2: void Clone ( FooBase & g@1 ) ;\n"
                           "3: short m_bar@2 ;\n"
                           "4: } ;\n"
                           "5: void Foo :: Clone ( FooBase & g@3 ) {\n"
                           "6: ( ( FooBase ) g@3 ) . m_bar = m_bar@2 ;\n"
                           "7: }\n",
                           tokenize(code));
    }

    void varid_in_class11() { // #4277 - anonymous union
        const char code1[] = "class Foo {\n"
                             "    union { float a; int b; };\n"
                             "    void f() { a=0; }\n"
                             "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Foo {\n"
                      "2: union { float a@1 ; int b@2 ; } ;\n"
                      "3: void f ( ) { a@1 = 0 ; }\n"
                      "4: } ;\n",
                      tokenize(code1));

        const char code2[] = "class Foo {\n"
                             "    void f() { a=0; }\n"
                             "    union { float a; int b; };\n"
                             "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Foo {\n"
                      "2: void f ( ) { a@1 = 0 ; }\n"
                      "3: union { float a@1 ; int b@2 ; } ;\n"
                      "4: } ;\n",
                      tokenize(code2));
    }

    void varid_in_class12() { // #4637 - method
        const char code[] = "class Foo {\n"
                            "private:\n"
                            "    void f(void);\n"
                            "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Foo {\n"
                      "2: private:\n"
                      "3: void f ( ) ;\n"
                      "4: } ;\n",
                      tokenize(code));
    }

    void varid_in_class13() {
        const char code1[] = "struct a { char typename; };";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct a { char typename@1 ; } ;\n",
                      tokenize(code1, false, "test.c"));
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct a { char typename ; } ;\n",  // not valid C++ code
                      tokenize(code1, false, "test.cpp"));

        const char code2[] = "struct a { char typename[2]; };";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct a { char typename@1 [ 2 ] ; } ;\n",
                      tokenize(code2, false, "test.c"));
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct a { char typename [ 2 ] ; } ;\n",  // not valid C++ code
                      tokenize(code2, false, "test.cpp"));
    }

    void varid_in_class14() {
        const char code[] = "class Tokenizer { TokenList list; };\n"
                            "\n"
                            "void Tokenizer::f() {\n"
                            "  std::list<int> x;\n"               // <- not member variable
                            "  list.do_something();\n"            // <- member variable
                            "  Tokenizer::list.do_something();\n" // <- redundant scope info
                            "}\n";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Tokenizer { TokenList list@1 ; } ;\n"
                      "2:\n"
                      "3: void Tokenizer :: f ( ) {\n"
                      "4: std :: list < int > x@2 ;\n"
                      "5: list@1 . do_something ( ) ;\n"
                      "6: Tokenizer :: list@1 . do_something ( ) ;\n"
                      "7: }\n", tokenize(code, false, "test.cpp"));
    }

    void varid_in_class15() { // #5533 - functions
        const char code[] = "class Fred {\n"
                            "  void x(int a) const;\n"
                            "  void y() { a=0; }\n" // <- unknown variable
                            "}\n";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Fred {\n"
                      "2: void x ( int a@1 ) const ;\n"
                      "3: void y ( ) { a = 0 ; }\n"
                      "4: }\n", tokenize(code, false, "test.cpp"));
    }

    void varid_in_class16() { // Set varId for inline member functions
        const char code[] = "class Fred {\n"
                            "    int x;\n"
                            "    void foo(int x) { this->x = x; }\n"
                            "};\n";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Fred {\n"
                      "2: int x@1 ;\n"
                      "3: void foo ( int x@2 ) { this . x@1 = x@2 ; }\n"
                      "4: } ;\n", tokenize(code, false, "test.cpp"));
    }

    void varid_in_class17() { // #6056 - Set no varid for member functions
        const char code1[] = "class Fred {\n"
                             "    int method_with_internal(X&);\n"
                             "    int method_with_internal(X*);\n"
                             "    int method_with_internal(int&);\n"
                             "    int method_with_internal(A* b, X&);\n"
                             "    int method_with_internal(X&, A* b);\n"
                             "    int method_with_internal(const B &, int);\n"
                             "    void Set(BAR);\n"
                             "    FOO Set(BAR);\n"
                             "    int method_with_class(B<B> b);\n"
                             "    bool function(std::map<int, int, MYless> & m);\n"
                             "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Fred {\n"
                      "2: int method_with_internal ( X & ) ;\n"
                      "3: int method_with_internal ( X * ) ;\n"
                      "4: int method_with_internal ( int & ) ;\n"
                      "5: int method_with_internal ( A * b@1 , X & ) ;\n"
                      "6: int method_with_internal ( X & , A * b@2 ) ;\n"
                      "7: int method_with_internal ( const B & , int ) ;\n"
                      "8: void Set ( BAR ) ;\n"
                      "9: FOO Set ( BAR ) ;\n"
                      "10: int method_with_class ( B < B > b@3 ) ;\n"
                      "11: bool function ( std :: map < int , int , MYless > & m@4 ) ;\n"
                      "12: } ;\n", tokenize(code1, false, "test.cpp"));

        const char code2[] = "int i;\n"
                             "SomeType someVar1(i, i);\n"
                             "SomeType someVar2(j, j);\n"
                             "SomeType someVar3(j, 1);\n"
                             "SomeType someVar4(new bar);";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int i@1 ;\n"
                      "2: SomeType someVar1@2 ( i@1 , i@1 ) ;\n"
                      "3: SomeType someVar2 ( j , j ) ;\n" // This one could be a function
                      "4: SomeType someVar3@3 ( j , 1 ) ;\n"
                      "5: SomeType someVar4@4 ( new bar ) ;\n", tokenize(code2, false, "test.cpp"));
    }

    void varid_initList() {
        const char code1[] = "class A {\n"
                             "  A() : x(0) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: A ( ) : x@1 ( 0 ) { }\n"
                      "3: int x@1 ;\n"
                      "4: } ;\n",
                      tokenize(code1));

        const char code2[] = "class A {\n"
                             "  A(int x) : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: A ( int x@1 ) : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code2));

        const char code3[] = "class A {\n"
                             "  A(int x);\n"
                             "  int x;\n"
                             "};\n"
                             "A::A(int x) : x(x) {}";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: A ( int x@1 ) ;\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n"
                      "5: A :: A ( int x@3 ) : x@2 ( x@3 ) { }\n",
                      tokenize(code3));

        const char code4[] = "struct A {\n"
                             "  int x;\n"
                             "  A(int x) : x(x) {}\n"
                             "};\n";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct A {\n"
                      "2: int x@1 ;\n"
                      "3: A ( int x@2 ) : x@1 ( x@2 ) { }\n"
                      "4: } ;\n",
                      tokenize(code4));

        const char code5[] = "class A {\n"
                             "  A(int x) noexcept : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: A ( int x@1 ) noexcept : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code5));

        const char code6[] = "class A {\n"
                             "  A(int x) noexcept(true) : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: A ( int x@1 ) noexcept ( true ) : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code6));
    }

    void varid_operator() {
        {
            const std::string actual = tokenize(
                                           "class Foo\n"
                                           "{\n"
                                           "public:\n"
                                           "    void operator=(const Foo &);\n"
                                           "};\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: class Foo\n"
                                       "2: {\n"
                                       "3: public:\n"
                                       "4: void operator= ( const Foo & ) ;\n"
                                       "5: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }
        {
            const std::string actual = tokenize(
                                           "struct Foo {\n"
                                           "    void * operator new [](int);\n"
                                           "};\n");
            const std::string expected("\n\n##file 0\n"
                                       "1: struct Foo {\n"
                                       "2: void * operatornew[] ( int ) ;\n"
                                       "3: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_throw() {  // ticket #1723
        const std::string actual = tokenize(
                                       "UserDefinedException* pe = new UserDefinedException();\n"
                                       "throw pe;\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: UserDefinedException * pe@1 ; pe@1 = new UserDefinedException ( ) ;\n"
                                   "2: throw pe@1 ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid_unknown_macro() {
        // #2638 - unknown macro
        const char code[] = "void f() {\n"
                            "    int a[10];\n"
                            "    AAA\n"
                            "    a[0] = 0;\n"
                            "}";
        const char expected[] = "\n\n##file 0\n"
                                "1: void f ( ) {\n"
                                "2: int a@1 [ 10 ] ;\n"
                                "3: AAA\n"
                                "4: a@1 [ 0 ] = 0 ;\n"
                                "5: }\n";
        ASSERT_EQUALS(expected, tokenize(code, false, "test.c"));
    }

    void varid_using() {
        // #3648
        const char code[] = "using std::size_t;";
        const char expected[] = "\n\n##file 0\n"
                                "1: using long ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_catch() {
        const char code[] = "void f() {\n"
                            "    try { dostuff(); }\n"
                            "    catch (exception &e) { }\n"
                            "}";
        const char expected[] = "\n\n##file 0\n"
                                "1: void f ( ) {\n"
                                "2: try { dostuff ( ) ; }\n"
                                "3: catch ( exception & e@1 ) { }\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_functionPrototypeTemplate() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: function < void ( ) > fptr@1 ;\n", tokenize("function<void(void)> fptr;"));
    }

    void varid_templatePtr() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: std :: map < int , FooTemplate < int > * > dummy_member@1 [ 1 ] ;\n", tokenize("std::map<int, FooTemplate<int>*> dummy_member[1];"));
    }

    void varid_templateNamespaceFuncPtr() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: KeyListT < float , & NIFFile :: getFloat > mKeyList@1 [ 4 ] ;\n", tokenize("KeyListT<float, &NIFFile::getFloat> mKeyList[4];"));
    }

    void varid_templateArray() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: VertexArrayIterator < float [ 2 ] > attrPos@1 ; attrPos@1 = m_AttributePos . GetIterator < float [ 2 ] > ( ) ;\n",
                      tokenize("VertexArrayIterator<float[2]> attrPos = m_AttributePos.GetIterator<float[2]>();"));
    }

    void varid_cppcast() {
        ASSERT_EQUALS("\n\n##file 0\n1: const_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("const_cast<int *>(code)[0] = 0;"));

        ASSERT_EQUALS("\n\n##file 0\n1: dynamic_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("dynamic_cast<int *>(code)[0] = 0;"));

        ASSERT_EQUALS("\n\n##file 0\n1: reinterpret_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("reinterpret_cast<int *>(code)[0] = 0;"));

        ASSERT_EQUALS("\n\n##file 0\n1: static_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("static_cast<int *>(code)[0] = 0;"));
    }

    void varid_variadicFunc() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int foo ( . . . ) ;\n", tokenize("int foo(...);"));
    }

    void varid_typename() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: template < int d , class A , class B >\n", tokenize("template<int d, class A, class B>"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: template < int d , typename A , typename B >\n", tokenize("template<int d, typename A, typename B>"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: typename A a@1 ;\n", tokenize("typename A a;"));
    }

    void varid_rvalueref() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int & & a@1 ;\n", tokenize("int&& a;"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void foo ( int & & a@1 ) { }\n", tokenize("void foo(int&& a) {}"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class C {\n"
                      "2: C ( int & & a@1 ) ;\n"
                      "3: } ;\n",
                      tokenize("class C {\n"
                               "    C(int&& a);\n"
                               "};"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void foo ( int & & ) ;\n", tokenize("void foo(int&&);"));
    }

    void varid_arrayFuncPar() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void check ( const char fname@1 [ ] = 0 ) { }\n", tokenize("void check( const char fname[] = 0) { }"));
    }

    void varid_sizeofPassed() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void which_test ( ) {\n"
                      "2: const char * argv@1 [ 2 ] = { \"./test_runner\" , \"TestClass\" } ;\n"
                      "3: options args@2 ( sizeof argv@1 / sizeof ( argv@1 [ 0 ] ) , argv@1 ) ;\n"
                      "4: args@2 . which_test ( ) ;\n"
                      "5: }\n",
                      tokenize("void which_test() {\n"
                               "    const char* argv[] = { \"./test_runner\", \"TestClass\" };\n"
                               "    options args(sizeof argv / sizeof argv[0], argv);\n"
                               "    args.which_test();\n"
                               "}"));
    }

    void varid_classInFunction() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void AddSuppression ( ) {\n"
                      "2: class QErrorLogger {\n"
                      "3: void reportErr ( ErrorLogger :: ErrorMessage & msg@1 ) {\n"
                      "4: }\n"
                      "5: } ;\n"
                      "6: }\n",
                      tokenize("void AddSuppression() {\n"
                               "    class QErrorLogger {\n"
                               "        void reportErr(ErrorLogger::ErrorMessage &msg) {\n"
                               "        }\n"
                               "    }; \n"
                               "}"));
    }

    void varid_pointerToArray() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int * a1@1 [ 10 ] ;\n"
                      "2: void f1 ( ) {\n"
                      "3: int * a2@2 [ 10 ] ;\n"
                      "4: int ( & a3@3 ) [ 10 ] ;\n"
                      "5: }\n"
                      "6: struct A {\n"
                      "7: int ( & a4@4 ) [ 10 ] ;\n"
                      "8: int f2 ( int i@5 ) { return a4@4 [ i@5 ] ; }\n"
                      "9: int f3 ( int ( & a5@6 ) [ 10 ] , int i@7 ) { return a5@6 [ i@7 ] ; }\n"
                      "10: } ;\n"
                      "11: int f4 ( int ( & a6@8 ) [ 10 ] , int i@9 ) { return a6@8 [ i@9 ] ; }\n",
                      tokenize("int (*a1)[10];\n" // pointer to array of 10 ints
                               "void f1() {\n"
                               "    int(*a2)[10];\n"
                               "    int(&a3)[10];\n"
                               "}\n"
                               "struct A {\n"
                               "    int(&a4)[10];\n"
                               "    int f2(int i) { return a4[i]; }\n"
                               "    int f3(int(&a5)[10], int i) { return a5[i]; }\n"
                               "};\n"
                               "int f4(int(&a6)[10], int i) { return a6[i]; }"));
    }

    void varid_cpp11initialization() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int i@1 { 1 } ;\n"
                      "2: std :: vector < int > vec@2 { 1 , 2 , 3 } ;\n"
                      "3: namespace n { int z@3 ; } ;\n"
                      "4: int & j@4 { i@1 } ;\n"
                      "5: int k@5 { 1 } ; int l@6 { 2 } ;\n",
                      tokenize("int i{1};\n"
                               "std::vector<int> vec{1, 2, 3};\n"
                               "namespace n { int z; };\n"
                               "int& j{i};\n"
                               "int k{1}, l{2};"));

        // #6030
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: struct S3 : public S1 , public S2 { } ;\n",
                      tokenize("struct S3 : public S1, public S2 { };"));

        // #6058
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class Scope { } ;\n",
                      tokenize("class CPPCHECKLIB Scope { };"));

        // #6073
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A : public B , public C :: D {\n"
                      "2: int i@1 ;\n"
                      "3: A ( int i@2 ) : B { i@2 } , C :: D { i@2 } , i@1 { i@2 } {\n"
                      "4: int j@3 { i@2 } ;\n"
                      "5: }\n"
                      "6: } ;\n",
                      tokenize("class A: public B, public C::D {\n"
                               "    int i;\n"
                               "    A(int i): B{i}, C::D{i}, i{i} {\n"
                               "        int j{i};\n"
                               "    }\n"
                               "};"));
    }

    void varid_inheritedMembers() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : public A {\n"
                      "5: void f ( ) ;\n"
                      "6: } ;\n"
                      "7: void B :: f ( ) {\n"
                      "8: a@1 = 0 ;\n"
                      "9: }\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : public A {\n"
                               "    void f();\n"
                               "};\n"
                               "void B::f() {\n"
                               "    a = 0;\n"
                               "}"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : A {\n"
                      "5: void f ( ) ;\n"
                      "6: } ;\n"
                      "7: void B :: f ( ) {\n"
                      "8: a@1 = 0 ;\n"
                      "9: }\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : A {\n"
                               "    void f();\n"
                               "};\n"
                               "void B::f() {\n"
                               "    a = 0;\n"
                               "}"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : protected B , public A {\n"
                      "5: void f ( ) ;\n"
                      "6: } ;\n"
                      "7: void B :: f ( ) {\n"
                      "8: a@1 = 0 ;\n"
                      "9: }\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : protected B, public A {\n"
                               "    void f();\n"
                               "};\n"
                               "void B::f() {\n"
                               "    a = 0;\n"
                               "}"));

        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : public A {\n"
                      "5: void f ( ) {\n"
                      "6: a@1 = 0 ;\n"
                      "7: }\n"
                      "8: } ;\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : public A {\n"
                               "    void f() {\n"
                               "        a = 0;\n"
                               "    }\n"
                               "};"));
    }

    void varid_header() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: class A ;\n"
                      "2: struct B {\n"
                      "3: void setData ( const A & a@1 ) ;\n"
                      "4: } ;\n",
                      tokenize("class A;\n"
                               "struct B {\n"
                               "    void setData(const A & a);\n"
                               "}; ", false, "test.h"));
    }

    void varidclass1() {
        const std::string actual = tokenize(
                                       "class Fred\n"
                                       "{\n"
                                       "private:\n"
                                       "    int i;\n"
                                       "\n"
                                       "    void foo1();\n"
                                       "    void foo2()\n"
                                       "    {\n"
                                       "        ++i;\n"
                                       "    }\n"
                                       "}\n"
                                       "\n"
                                       "Fred::foo1()\n"
                                       "{\n"
                                       "    i = 0;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: {\n"
                                   "3: private:\n"
                                   "4: int i@1 ;\n"
                                   "5:\n"
                                   "6: void foo1 ( ) ;\n"
                                   "7: void foo2 ( )\n"
                                   "8: {\n"
                                   "9: ++ i@1 ;\n"
                                   "10: }\n"
                                   "11: }\n"
                                   "12:\n"
                                   "13: Fred :: foo1 ( )\n"
                                   "14: {\n"
                                   "15: i@1 = 0 ;\n"
                                   "16: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass2() {
        const std::string actual = tokenize(
                                       "class Fred\n"
                                       "{ void f(); };\n"
                                       "\n"
                                       "void A::foo1()\n"
                                       "{\n"
                                       "    int i = 0;\n"
                                       "}\n"
                                       "\n"
                                       "void Fred::f()\n"
                                       "{\n"
                                       "    i = 0;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: { void f ( ) ; } ;\n"
                                   "3:\n"
                                   "4: void A :: foo1 ( )\n"
                                   "5: {\n"
                                   "6: int i@1 ; i@1 = 0 ;\n"
                                   "7: }\n"
                                   "8:\n"
                                   "9: void Fred :: f ( )\n"
                                   "10: {\n"
                                   "11: i = 0 ;\n"
                                   "12: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass3() {
        const std::string actual = tokenize(
                                       "class Fred\n"
                                       "{ int i; void f(); };\n"
                                       "\n"
                                       "void Fred::f()\n"
                                       "{\n"
                                       "    i = 0;\n"
                                       "}\n"
                                       "\n"
                                       "void A::f()\n"
                                       "{\n"
                                       "    i = 0;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: { int i@1 ; void f ( ) ; } ;\n"
                                   "3:\n"
                                   "4: void Fred :: f ( )\n"
                                   "5: {\n"
                                   "6: i@1 = 0 ;\n"
                                   "7: }\n"
                                   "8:\n"
                                   "9: void A :: f ( )\n"
                                   "10: {\n"
                                   "11: i = 0 ;\n"
                                   "12: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass4() {
        const std::string actual = tokenize(
                                       "class Fred\n"
                                       "{ int i; void f(); };\n"
                                       "\n"
                                       "void Fred::f()\n"
                                       "{\n"
                                       "    if (i) { }\n"
                                       "    i = 0;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: { int i@1 ; void f ( ) ; } ;\n"
                                   "3:\n"
                                   "4: void Fred :: f ( )\n"
                                   "5: {\n"
                                   "6: if ( i@1 ) { }\n"
                                   "7: i@1 = 0 ;\n"
                                   "8: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass5() {
        const std::string actual = tokenize(
                                       "class A { };\n"
                                       "class B\n"
                                       "{\n"
                                       "    A *a;\n"
                                       "    B() : a(new A)\n"
                                       "    { }\n"
                                       "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class A { } ;\n"
                                   "2: class B\n"
                                   "3: {\n"
                                   "4: A * a@1 ;\n"
                                   "5: B ( ) : a@1 ( new A )\n"
                                   "6: { }\n"
                                   "7: } ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass6() {
        const std::string actual = tokenize(
                                       "class A\n"
                                       "{\n"
                                       "  public:\n"
                                       "  static char buf[20];\n"
                                       "};\n"
                                       "char A::buf[20];\n"
                                       "int main()\n"
                                       "{\n"
                                       "  char buf[2];\n"
                                       "  A::buf[10] = 0;\n"
                                       "}");

        const std::string wanted("\n\n##file 0\n"
                                 "1: class A\n"
                                 "2: {\n"
                                 "3: public:\n"
                                 "4: static char buf@1 [ 20 ] ;\n"
                                 "5: } ;\n"
                                 "6: char A :: buf@1 [ 20 ] ;\n"
                                 "7: int main ( )\n"
                                 "8: {\n"
                                 "9: char buf@2 [ 2 ] ;\n"
                                 "10: A :: buf@1 [ 10 ] = 0 ;\n"
                                 "11: }\n");

        ASSERT_EQUALS(wanted, actual);
    }

    void varidclass7() {
        const std::string actual = tokenize(
                                       "int main()\n"
                                       "{\n"
                                       "  char buf[2];\n"
                                       "  A::buf[10] = 0;\n"
                                       "}");

        const std::string expected("\n\n##file 0\n"
                                   "1: int main ( )\n"
                                   "2: {\n"
                                   "3: char buf@1 [ 2 ] ;\n"
                                   "4: A :: buf [ 10 ] = 0 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass8() {
        const char code[] ="class Fred {\n"
                           "public:\n"
                           "    void foo(int d) {\n"
                           "        int i = bar(x * d);\n"
                           "    }\n"
                           "    int x;\n"
                           "}\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: public:\n"
                                   "3: void foo ( int d@1 ) {\n"
                                   "4: int i@2 ; i@2 = bar ( x@3 * d@1 ) ;\n"
                                   "5: }\n"
                                   "6: int x@3 ;\n"
                                   "7: }\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass9() {
        const char code[] ="typedef char Str[10];"
                           "class A {\n"
                           "public:\n"
                           "    void f(Str &cl);\n"
                           "    void g(Str cl);\n"
                           "}\n"
                           "void Fred::f(Str &cl) {\n"
                           "    sizeof(cl);\n"
                           "}";

        const std::string expected("\n\n"
                                   "##file 0\n"
                                   "1: class A {\n"
                                   "2: public:\n"
                                   "3: void f ( char ( & cl@1 ) [ 10 ] ) ;\n"
                                   "4: void g ( char cl@2 [ 10 ] ) ;\n"
                                   "5: }\n"
                                   "6: void Fred :: f ( char ( & cl@3 ) [ 10 ] ) {\n"
                                   "7: sizeof ( cl@3 ) ;\n"
                                   "8: }\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass10() {
        const char code[] ="class A {\n"
                           "    void f() {\n"
                           "        a = 3;\n"
                           "    }\n"
                           "    int a;\n"
                           "};\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: class A {\n"
                                   "2: void f ( ) {\n"
                                   "3: a@1 = 3 ;\n"
                                   "4: }\n"
                                   "5: int a@1 ;\n"
                                   "6: } ;\n");
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass11() {
        const char code[] ="class Fred {\n"
                           "    int a;\n"
                           "    void f();\n"
                           "};\n"
                           "class Wilma {\n"
                           "    int a;\n"
                           "    void f();\n"
                           "};\n"
                           "void Fred::f() { a = 0; }\n"
                           "void Wilma::f() { a = 0; }\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: int a@1 ;\n"
                                   "3: void f ( ) ;\n"
                                   "4: } ;\n"
                                   "5: class Wilma {\n"
                                   "6: int a@2 ;\n"
                                   "7: void f ( ) ;\n"
                                   "8: } ;\n"
                                   "9: void Fred :: f ( ) { a@1 = 0 ; }\n"
                                   "10: void Wilma :: f ( ) { a@2 = 0 ; }\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass12() {
        const char code[] ="class Fred {\n"
                           "    int a;\n"
                           "    void f() { Fred::a = 0; }\n"
                           "};\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: int a@1 ;\n"
                                   "3: void f ( ) { Fred :: a@1 = 0 ; }\n"
                                   "4: } ;\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass13() {
        const char code[] ="class Fred {\n"
                           "    int a;\n"
                           "    void f() { Foo::Fred::a = 0; }\n"
                           "};\n";

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: int a@1 ;\n"
                                   "3: void f ( ) { Foo :: Fred :: a = 0 ; }\n"
                                   "4: } ;\n");

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass14() {
        // don't give friend classes varid
        {
            const char code[] ="class A {\n"
                               "friend class B;\n"
                               "}";

            const std::string expected("\n\n##file 0\n"
                                       "1: class A {\n"
                                       "2: friend class B ;\n"
                                       "3: }\n");

            ASSERT_EQUALS(expected, tokenize(code));
        }

        {
            const char code[] ="class A {\n"
                               "private: friend class B;\n"
                               "}";

            const std::string expected("\n\n##file 0\n"
                                       "1: class A {\n"
                                       "2: private: friend class B ;\n"
                                       "3: }\n");

            ASSERT_EQUALS(expected, tokenize(code));
        }
    }

    void varidclass15() {
        const char code[] = "class A {\n"
                            "    int a;\n"
                            "    int b;\n"
                            "    A();\n"
                            "};\n"
                            "A::A() : a(0) { b = 1; }";
        const char expected[] = "\n\n##file 0\n"
                                "1: class A {\n"
                                "2: int a@1 ;\n"
                                "3: int b@2 ;\n"
                                "4: A ( ) ;\n"
                                "5: } ;\n"
                                "6: A :: A ( ) : a@1 ( 0 ) { b@2 = 1 ; }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass16() {
        const char code[] = "struct A;\n"
                            "typedef bool (A::* FuncPtr)();\n"
                            "struct A {\n"
                            "    FuncPtr pFun;\n"
                            "    void setPFun(int mode);\n"
                            "    bool funcNorm();\n"
                            "};\n"
                            "void A::setPFun(int mode) {\n"
                            "    pFun = &A::funcNorm;\n"
                            "}";
        const char expected[] = "\n\n##file 0\n"
                                "1: struct A ;\n"
                                "2:\n"
                                "3: struct A {\n"
                                "4: bool * pFun@1 ;\n"
                                "5: void setPFun ( int mode@2 ) ;\n"
                                "6: bool funcNorm ( ) ;\n"
                                "7: } ;\n"
                                "8: void A :: setPFun ( int mode@3 ) {\n"
                                "9: pFun@1 = & A :: funcNorm ;\n"
                                "10: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass17() {
        const char code[] = "class A: public B, public C::D {\n"
                            "    int i;\n"
                            "    A(int i): B(i), C::D(i), i(i) {\n"
                            "        int j(i);\n"
                            "    }\n"
                            "};";
        const char expected[] = "\n\n##file 0\n"
                                "1: class A : public B , public C :: D {\n"
                                "2: int i@1 ;\n"
                                "3: A ( int i@2 ) : B ( i@2 ) , C :: D ( i@2 ) , i@1 ( i@2 ) {\n"
                                "4: int j@3 ; j@3 = i@2 ;\n"
                                "5: }\n"
                                "6: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_classnameshaddowsvariablename() {
        const char code[] = "class Data;\n"
                            "void strange_declarated(const Data& Data);\n"
                            "void handleData(const Data& data) {\n"
                            "    strange_declarated(data);\n"
                            "}\n";
        const char expected[] = "\n\n##file 0\n"
                                "1: class Data ;\n"
                                "2: void strange_declarated ( const Data & Data@1 ) ;\n"
                                "3: void handleData ( const Data & data@2 ) {\n"
                                "4: strange_declarated ( data@2 ) ;\n"
                                "5: }\n";
        ASSERT_EQUALS(expected, tokenize(code));

    }

    void varidnamespace1() {
        const char code[] = "namespace A {\n"
                            "    char buf[20];\n"
                            "}\n"
                            "int main() {\n"
                            "    return foo(A::buf);\n"
                            "}";

        const char expected[] = "\n\n##file 0\n"
                                "1: namespace A {\n"
                                "2: char buf@1 [ 20 ] ;\n"
                                "3: }\n"
                                "4: int main ( ) {\n"
                                "5: return foo ( A :: buf@1 ) ;\n"
                                "6: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }
};

REGISTER_TEST(TestVarID)
