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
#include "check64bit.h"
#include "testsuite.h"


class Test64BitPortability : public TestFixture {
public:
    Test64BitPortability() : TestFixture("Test64BitPortability") {
    }

private:


    void run() {
        TEST_CASE(novardecl);
        TEST_CASE(functionpar);
        TEST_CASE(structmember);
        TEST_CASE(ptrcompare);
        TEST_CASE(ptrarithmetic);
        TEST_CASE(returnIssues);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("portability");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check char variable usage..
        Check64BitPortability check64BitPortability(&tokenizer, &settings, this);
        check64BitPortability.pointerassignment();
    }

    void novardecl() {
        // if the variable declarations can't be seen then skip the warning
        check("void foo()\n"
              "{\n"
              "    a = p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void functionpar() {
        check("int foo(int *p)\n"
              "{\n"
              "    int a = p;\n"
              "    return a + 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning a pointer to an integer is not portable.\n", errout.str());

        check("int foo(int p[])\n"
              "{\n"
              "    int a = p;\n"
              "    return a + 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning a pointer to an integer is not portable.\n", errout.str());

        check("int foo(int p[])\n"
              "{\n"
              "    int *a = p;\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (portability) Returning an address value in a function with integer return type is not portable.\n", errout.str());

        check("void foo(int x)\n"
              "{\n"
              "    int *p = x;\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning an integer to a pointer is not portable.\n", errout.str());

        check("int f(const char *p) {\n" // #4659
              "    return 6 + p[2] * 256;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo(int *p) {\n" // #6096
              "    bool a = p;\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember() {
        check("struct Foo { int *p; };\n"
              "void f(struct Foo *foo) {\n"
              "    int i = foo->p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Assigning a pointer to an integer is not portable.\n", errout.str());
    }

    void ptrcompare() {
        // Ticket #2892
        check("void foo(int *p) {\n"
              "    int a = (p != NULL);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ptrarithmetic() {
        // #3073
        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = p + x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = x + p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = x * x;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("void foo(int *start, int *end) {\n"
              "    int len;\n"
              "    int len = end + 10 - start;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void returnIssues() {
        check("void* foo(int i) {\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Returning an integer in a function with pointer return type is not portable.\n", errout.str());

        check("void* foo(int* i) {\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void* foo() {\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo(int i) {\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo(char* c) {\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Returning an address value in a function with integer return type is not portable.\n", errout.str());

        check("int foo(char* c) {\n"
              "    return 1+c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (portability) Returning an address value in a function with integer return type is not portable.\n", errout.str());

        check("std::string foo(char* c) {\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int foo(char *a, char *b) {\n" // #4486
              "    return a + 1 - b;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct s {\n" // 4642
              "   int i;\n"
              "};\n"
              "int func(struct s *p) {\n"
              " return 1 + p->i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void __iomem *f(unsigned int port_no) {\n"
              "  void __iomem *mmio = hpriv->mmio;\n"
              "  return mmio + (port_no * 0x80);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(Test64BitPortability)
