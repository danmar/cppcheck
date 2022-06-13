/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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


#include "checkassert.h"
#include "errortypes.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <iosfwd>


class TestAssert : public TestFixture {
public:
    TestAssert() : TestFixture("TestAssert") {}

private:
    Settings settings;

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    void check_(const char* file, int line, const char code[], const char *filename = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, filename), file, line);

        // Check..
        CheckAssert checkAssert;
        checkAssert.runChecks(&tokenizer, &settings, this);
    }

    void run() override {
        settings.severity.enable(Severity::warning);

        TEST_CASE(assignmentInAssert);
        TEST_CASE(functionCallInAssert);
        TEST_CASE(memberFunctionCallInAssert);
        TEST_CASE(constructorCallInAssert);
        TEST_CASE(safeConstructorCallInAssert);
        TEST_CASE(safeFunctionCallInAssert);
        TEST_CASE(crash);
    }

    void safeFunctionCallInAssert() {
        check(
            "int b = 0;\n"
            "int foo() {\n"
            "   int a = 3;\n"
            "   if (b) { a = a+b; }\n"
            "   return a;\n"
            "}\n"
            "assert(foo() == 3);");
        ASSERT_EQUALS("", errout.str());

        check(
            "int foo(int a) {\n"
            "    int b=a+1;\n"
            "    return b;\n"
            "}\n"
            "assert(foo(1) == 2);");
        ASSERT_EQUALS("", errout.str());
    }

    void functionCallInAssert() {
        check(
            "int a;\n"
            "bool b = false;\n"
            "int foo() {\n"
            "   if (b) { a = 1+2; }\n"
            "   return a;\n"
            "}\n"
            "assert(foo() == 3);");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Assert statement calls a function which may have desired side effects: 'foo'.\n", errout.str());

        check(
            "int a;\n"
            "int foo() {\n"
            "    a = 1+2;\n"
            "    return a;\n"
            "}\n"
            "assert(foo() == 3);");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Assert statement calls a function which may have desired side effects: 'foo'.\n", errout.str());

        check(
            "int a;\n"
            "int foo() {\n"
            "    a = 1+2;\n"
            "    return a;\n"
            "}\n"
            "int bar() {\n"
            "    return foo();\n"
            "}\n"
            "assert(bar() == 3);");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Assert statement calls a function which may have desired side effects: 'bar'.\n", errout.str());

        //  Ticket #4937 "false positive: Assert calls a function which may have
        //  desired side effects"
        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square &sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square *sq ) {\n"
              "      *sq &= 0x38;\n"
              "      return *sq == 0 || *sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( int *sq ) {\n"
              "      *sq++;\n"
              "      return *sq == 0 || *sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( int *sq ) {\n"
              "      sq[0]++;\n"
              "      return *sq == 0 || *sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", errout.str());

        check("struct SquarePack {\n"
              "    static bool func2( int& sq2, int& sq3 ) {\n"
              "      sq3++;\n"
              "      return true;\n"
              "    }\n"
              "   static bool func1( int& sq1 ) {\n"
              "      int sq11 = 0;"
              "      return func2(sq11, sq1);\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   int push2 = 0;\n"
              "   assert( !SquarePack::func1(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:12]: (warning) Assert statement calls a function which may have desired side effects: 'func1'.\n", errout.str());

        check("struct SquarePack {\n"
              "    static bool func2( int& sq2, int& sq3 ) {\n"
              "      sq3++;\n"
              "      return true;\n"
              "    }\n"
              "   static bool func1( int& sq1 ) {\n"
              "      int sq11 = 0;"
              "      return func2(sq1,sq11);\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   int push2 = 0;\n"
              "   assert( !SquarePack::func1(push2) );\n"
              "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:12]: (warning) Assert statement calls a function which may have desired side effects: 'func1'.\n", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( int *sq ) {\n"
              "      sq[0] = 4;\n"
              "      return *sq == 0 || *sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", errout.str());


        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square *sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square *sq ) {\n"
              "      auto lsq = sq;\n"
              "      *lsq &= 0x38;\n"
              "      return *sq == 0 || *sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", "", errout.str());
        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square &sq ) {\n"
              "      Square& ref= sq;\n"
              "      ref &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", "", errout.str());
    }

    void memberFunctionCallInAssert() {
        check("struct SquarePack {\n"
              "   void Foo();\n"
              "};\n"
              "void foo(SquarePack s) {\n"
              "   assert( s.Foo() );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Assert statement calls a function which may have desired side effects: 'Foo'.\n", errout.str());

        check("struct SquarePack {\n"
              "   void Foo() const;\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo() );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct SquarePack {\n"
              "   static void Foo();\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo() );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct SquarePack {\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo() );\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void constructorCallInAssert() {
        check("static int i = 0;\n"
              "class SquarePack {\n"
              "  SquarePack(){\n"
              "    i++;\n"
              "  }\n"
              "  bool foo() const { return true; }\n"
              "};\n"
              "void foo() {\n"
              "   assert( SquarePack().foo() );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Assert statement calls a function which may have desired side effects: 'SquarePack'.\n", errout.str());

        check("class SquarePack {\n"
              "  SquarePack(int i, int& ii) : a(i++),b(ii++){}\n"
              "  bool foo() const;\n"
              "  int a;\n"
              "  int b;\n"
              "};\n"
              "void foo() {\n"
              "   int x = 2; int xx = 3;\n"
              "   assert( SquarePack(x,xx).foo() );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Assert statement calls a function which may have desired side effects: 'SquarePack'.\n", errout.str());

        check("class SquarePack {\n"
              "  SquarePack(){}\n"
              "  SquarePack(SquarePack& i){\n"
              "    i.bar();\n"
              "  }\n"
              "  bool foo() const { return true; }\n"
              "  bool bar() { return true; }\n"
              "};\n"
              "void foo() {\n"
              "   SquarePack a;\n"
              "   assert( SquarePack(a).foo() );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Assert statement calls a function which may have desired side effects: 'SquarePack'.\n", errout.str());

        check("class SquarePack {\n"
              "  SquarePack(int* i) : a(*i++){}\n"
              "  bool foo() const;\n"
              "  int a;\n"
              "};\n"
              "void foo() {\n"
              "   int i = 2;\n"
              "   assert( SquarePack(i).foo() );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'SquarePack'.\n", errout.str());

        check("class SquarePack {\n"
              "  SquarePack(int* i) : a(i[0]++){}\n"
              "  bool foo() const;\n"
              "  int a;\n"
              "};\n"
              "void foo() {\n"
              "   int i = 2;\n"
              "   assert( SquarePack(i).foo() );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'SquarePack'.\n", errout.str());

        check("static int i = 0;\n"
              "class SquarePack {\n"
              "  SquarePack(){\n"
              "    i++;\n"
              "  }\n"
              "  bool foo() const { return true; }\n"
              "};\n"
              "void foo() {\n"
              "   assert( SquarePack{}.foo() );\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (warning) Assert statement calls a function which may have desired side effects: 'SquarePack'.\n", "", errout.str());

        check("class SquarePack {\n"
              "  SquarePack(int& i){\n"
              "    i++;\n"
              "  }\n"
              "  bool foo() const { return true; }\n"
              "};\n"
              "void foo() {\n"
              "  int a = 0;"
              "  assert( SquarePack{a}.foo() );\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (warning) Assert statement calls a function which may have desired side effects: 'SquarePack'.\n", "", errout.str());
    }

    void safeConstructorCallInAssert() {
        check("class SquarePack {\n"
              "  SquarePack(){\n"
              "    a = 4;\n"
              "  }\n"
              "  int a;\n"
              "  bool foo() const { return true; }"
              "};\n"
              "void foo() {\n"
              "   assert( SquarePack().foo() );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class SquarePack {\n"
              "  SquarePack(int c) : a(c++){}\n"
              "  int a;\n"
              "  bool foo() const { return true; }"
              "};\n"
              "void foo() {\n"
              "   int b = 4;"
              "   assert( SquarePack(b).foo() );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class SquarePack {\n"
              "  constexpr SquarePack(){}\n"
              "  bool foo() const;"
              "};\n"
              "void foo() {\n"
              "   assert( SquarePack().foo() );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class SquarePack {\n"
              "  constexpr SquarePack(int i) : i(i){}\n"
              "  bool foo() const;\n"
              "  int i;\n"
              "};\n"
              "void foo() {\n"
              "   assert( SquarePack().foo() );\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assignmentInAssert() {
        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a = 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f(int a) {\n"
              "    assert(a == 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a, int b) {\n"
              "    assert(a == 2 && (b = 1));\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assert statement modifies 'b'.\n", errout.str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a += 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a *= 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a -= 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a--);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0; int b = 0;\n"
              "    assert(--a);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f(int* a, int b) {\n"
              "    assert(*a == 2 && (a[b] = 1));\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a[3]; int b = 0; a[0] = 0;\n"
              "    assert(a[b] += 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "  assert(std::all_of(first, last, []() {\n"
              "                  auto tmp = x.someValue();\n"
              "                  auto const expected = someOtherValue;\n"
              "                  return tmp == expected;\n"
              "                }));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void crash() {
        check("void foo() {\n"
              "  assert(sizeof(struct { int a[x++]; })==sizeof(int));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n" // #9790
              "  assert(kad_bucket_hash(&(kad_guid) { .bytes = { 0 } }, & (kad_guid){.bytes = { 0 }}) == -1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestAssert)
