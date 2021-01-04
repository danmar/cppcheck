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

#include "checkother.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <simplecpp.h>
#include <map>
#include <vector>

class TestIncompleteStatement : public TestFixture {
public:
    TestIncompleteStatement() : TestFixture("TestIncompleteStatement") {
    }

private:
    Settings settings;

    void check(const char code[], bool inconclusive = false) {
        // Clear the error buffer..
        errout.str("");

        settings.inconclusive = inconclusive;

        // Raw tokens..
        std::vector<std::string> files(1, "test.cpp");
        std::istringstream istr(code);
        const simplecpp::TokenList tokens1(istr, files, files[0]);

        // Preprocess..
        simplecpp::TokenList tokens2(files);
        std::map<std::string, simplecpp::TokenList*> filedata;
        simplecpp::preprocess(tokens2, tokens1, files, filedata, simplecpp::DUI());

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        tokenizer.createTokens(std::move(tokens2));
        tokenizer.simplifyTokens1("");

        // Check for incomplete statements..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkIncompleteStatement();
    }

    void run() OVERRIDE {
        settings.addEnabled("warning");

        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);
        TEST_CASE(test4);
        TEST_CASE(test5);
        TEST_CASE(test6);
        TEST_CASE(test7);
        TEST_CASE(test_numeric);
        TEST_CASE(void0); // #6327: No fp for statement "(void)0;"
        TEST_CASE(intarray);
        TEST_CASE(structarraynull);
        TEST_CASE(structarray);
        TEST_CASE(conditionalcall);     // ; 0==x ? X() : Y();
        TEST_CASE(structinit);          // #2462 : ABC abc{1,2,3};
        TEST_CASE(returnstruct);
        TEST_CASE(cast);                // #3009 : (struct Foo *)123.a = 1;
        TEST_CASE(increment);           // #3251 : FP for increment
        TEST_CASE(cpp11init);           // #5493 : int i{1};
        TEST_CASE(cpp11init2);          // #8449
        TEST_CASE(cpp11init3);          // #8995
        TEST_CASE(block);               // ({ do_something(); 0; })
        TEST_CASE(mapindex);
        TEST_CASE(commaoperator1);
        TEST_CASE(commaoperator2);
        TEST_CASE(redundantstmts);
        TEST_CASE(vardecl);
        TEST_CASE(archive);             // ar & x
        TEST_CASE(ast);
        TEST_CASE(oror);                // dostuff() || x=32;
    }

    void test1() {
        check("void foo()\n"
              "{\n"
              "    const char def[] =\n"
              "    \"abc\";\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void test2() {
        check("void foo()\n"
              "{\n"
              "    \"abc\";\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant code: Found a statement that begins with string constant.\n", errout.str());
    }

    void test3() {
        check("void foo()\n"
              "{\n"
              "    const char *str[] = { \"abc\" };\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void test4() {
        check("void foo()\n"
              "{\n"
              "const char *a =\n"
              "{\n"
              "\"hello \"\n"
              "\"more \"\n"
              "\"world\"\n"
              "};\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void test5() {
        check("void foo()\n"
              "{\n"
              "    50;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3]: (warning) Redundant code: Found a statement that begins with numeric constant.\n", errout.str());
    }

    void test6() {
        // don't crash
        check("void f() {\n"
              "  1 == (two + three);\n"
              "  2 != (two + three);\n"
              "  (one + two) != (two + three);\n"
              "}");
    }

    void test7() { // #9335
        check("namespace { std::string S = \"\"; }\n"
              "\n"
              "class C {\n"
              "public:\n"
              "  explicit C(const std::string& s);\n"
              "};\n"
              "\n"
              "void f() {\n"
              "  for (C c(S); ; ) {\n"
              "    (void)c;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void test_numeric() {
        check("struct P {\n"
              "    double a;\n"
              "    double b;\n"
              "};\n"
              "void f() {\n"
              "    const P values[2] =\n"
              "    {\n"
              "        { 346.1,114.1 }, { 347.1,111.1 }\n"
              "    };\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void void0() { // #6327
        check("void f() { (void*)0; }");
        ASSERT_EQUALS("", errout.str());

        check("#define X  0\n"
              "void f() { X; }");
        ASSERT_EQUALS("", errout.str());
    }

    void intarray() {
        check("int arr[] = { 100/2, 1*100 };");
        ASSERT_EQUALS("", errout.str());
    }

    void structarraynull() {
        check("struct st arr[] = {\n"
              "    { 100/2, 1*100 }\n"
              "    { 90, 70 }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void structarray() {
        check("struct st arr[] = {\n"
              "    { 100/2, 1*100 }\n"
              "    { 90, 70 }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void conditionalcall() {
        check("void f() {\n"
              "    0==x ? X() : Y();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void structinit() {
        // #2462 - C++11 struct initialization
        check("void f() {\n"
              "    ABC abc{1,2,3};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6260 - C++11 array initialization
        check("void foo() {\n"
              "    static const char* a[][2] {\n"
              "        {\"b\", \"\"},\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2482 - false positive for empty struct
        check("struct A {};");
        ASSERT_EQUALS("", errout.str());

        // #4387 - C++11 initializer list
        check("A::A() : abc{0} {}");
        ASSERT_EQUALS("", errout.str());

        // #5042 - C++11 initializer list
        check("A::A() : abc::def<int>{0} {}");
        ASSERT_EQUALS("", errout.str());

        // #4503 - vector init
        check("void f() { vector<int> v{1}; }");
        ASSERT_EQUALS("", errout.str());
    }

    void returnstruct() {
        check("struct s foo() {\n"
              "    return (struct s){0,0};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4754
        check("unordered_map<string, string>  foo() {\n"
              "    return {\n"
              "        {\"hi\", \"there\"},\n"
              "        {\"happy\", \"sad\"}\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct s foo() {\n"
              "  return (struct s){0};\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void cast() {
        check("void f() {\n"
              "    ((struct foo *)(0x1234))->xy = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void increment() {
        check("void f() {\n"
              "    int x = 1;\n"
              "    x++, x++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void cpp11init() {
        check("void f() {\n"
              "    int x{1};\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void cpp11init2() {
        check("x<string> handlers{\n"
              "  { \"mode2\", []() { return 2; } },\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void cpp11init3() {
        check("struct A { void operator()(int); };\n"
              "void f() {\n"
              "A{}(0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("template<class> struct A { void operator()(int); };\n"
              "void f() {\n"
              "A<int>{}(0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void block() {
        check("void f() {\n"
              "    ({ do_something(); 0; });\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "out:\n"
              "    ({ do_something(); 0; });\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mapindex() {
        check("void f() {\n"
              "  map[{\"1\",\"2\"}]=0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // #8827
    void commaoperator1() {
        check("void foo(int,const char*,int);\n"
              "void f(int value) {\n"
              "    foo(42,\"test\",42),(value&42);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found suspicious operator ','\n", errout.str());
    }

    void commaoperator2() {
        check("void f() {\n"
              "    for(unsigned int a=0, b; a<10; a++ ) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // #8451
    void redundantstmts() {
        check("void f1(int x) {\n"
              "    1;\n"
              "    (1);\n"
              "    (char)1;\n"
              "    ((char)1);\n"
              "    !x;\n"
              "    (!x);\n"
              "    (unsigned int)!x;\n"
              "    ~x;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:2]: (warning) Redundant code: Found a statement that begins with numeric constant.\n"
                      "[test.cpp:3]: (warning) Redundant code: Found a statement that begins with numeric constant.\n"
                      "[test.cpp:4]: (warning) Redundant code: Found a statement that begins with numeric constant.\n"
                      "[test.cpp:5]: (warning) Redundant code: Found a statement that begins with numeric constant.\n"
                      "[test.cpp:6]: (warning, inconclusive) Found suspicious operator '!'\n"
                      "[test.cpp:7]: (warning, inconclusive) Found suspicious operator '!'\n"
                      "[test.cpp:9]: (warning, inconclusive) Found suspicious operator '~'\n", errout.str());

        check("void f1(int x) { x; }", true);
        ASSERT_EQUALS("[test.cpp:1]: (warning) Unused variable value 'x'\n", errout.str());

    }

    void vardecl() {
        // #8984
        check("void f() { a::b *c = d(); }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { std::vector<b> *c; }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { a::b &c = d(); }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { std::vector<b> &c; }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { a::b &&c = d(); }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { std::vector<b> &&c; }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { char * const * a, * const * b; }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { char * const * a = 0, * volatile restrict * b; }", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() { char * const * a = 0, * volatile const * b; }", true);
        ASSERT_EQUALS("", errout.str());
    }

    void archive() {
        check("void f(Archive &ar) {\n"
              "  ar & x;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(int ar) {\n"
              "  ar & x;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Found suspicious operator '&'\n", errout.str());
    }

    void ast() {
        check("struct c { void a() const { for (int x=0; x;); } };", true);
        ASSERT_EQUALS("", errout.str());
    }

    void oror() {
        check("void foo() {\n"
              "    params_given (params, \"overrides\") || (overrides = \"1\");\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestIncompleteStatement)
