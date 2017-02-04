/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "checkassert.h"
#include "testsuite.h"


class TestAssert : public TestFixture {
public:
    TestAssert() : TestFixture("TestAssert") {}

private:
    Settings settings;

    void check(const char code[], const char *filename = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // Check..
        CheckAssert checkAssert(&tokenizer, &settings, this);
        checkAssert.runSimplifiedChecks(&tokenizer, &settings, this);
    }

    void run() {
        settings.addEnabled("warning");

        TEST_CASE(assignmentInAssert);
        TEST_CASE(functionCallInAssert);
        TEST_CASE(memberFunctionCallInAssert);
        TEST_CASE(safeFunctionCallInAssert);
    }


    void safeFunctionCallInAssert() {
        check(
            "int a;\n"
            "bool b = false;\n"
            "int foo() {\n"
            "   if (b) { a = 1+2 };\n"
            "   return a;\n"
            "}\n"
            "assert(foo() == 3); \n"
        );
        ASSERT_EQUALS("", errout.str());

        check(
            "int foo(int a) {\n"
            "    int b=a+1;\n"
            "    return b;\n"
            "}\n"
            "assert(foo(1) == 2); \n"
        );
        ASSERT_EQUALS("", errout.str());
    }

    void functionCallInAssert() {
        check(
            "int a;\n"
            "int foo() {\n"
            "    a = 1+2;\n"
            "    return a;\n"
            "}\n"
            "assert(foo() == 3); \n"
        );
        ASSERT_EQUALS("[test.cpp:6]: (warning) Assert statement calls a function which may have desired side effects: 'foo'.\n", errout.str());

        //  Ticket #4937 "false positive: Assert calls a function which may have desired side effects"
        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square &sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square *sq ) {\n"
              "      *sq &= 0x38;\n"
              "      return *sq == 0 || *sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'.\n", errout.str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square *sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void memberFunctionCallInAssert() {
        check("struct SquarePack {\n"
              "   void Foo();\n"
              "};\n"
              "void foo(SquarePack s) {\n"
              "   assert( s.Foo(); );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Assert statement calls a function which may have desired side effects: 'Foo'.\n", errout.str());

        check("struct SquarePack {\n"
              "   void Foo() const;\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo(); );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct SquarePack {\n"
              "   static void Foo();\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo(); );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct SquarePack {\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo(); );\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assignmentInAssert() {
        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a = 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f(int a) {\n"
              "    assert(a == 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int a, int b) {\n"
              "    assert(a == 2 && b = 1);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Assert statement modifies 'b'.\n", errout.str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a += 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a *= 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a -= 2);\n"
              "    return a;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a--);\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Assert statement modifies 'a'.\n", errout.str());

        check("void f() {\n"
              "  assert(std::all_of(first, last, []() {\n"
              "                  auto tmp = x.someValue();\n"
              "                  auto const expected = someOtherValue;\n"
              "                  return tmp == expected;\n"
              "                }));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestAssert)
