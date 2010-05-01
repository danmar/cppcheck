/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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



// Check for unused variables..

#include "testsuite.h"
#include "tokenize.h"
#include "checkother.h"

#include <sstream>
extern std::ostringstream errout;

class TestUnusedVar : public TestFixture
{
public:
    TestUnusedVar() : TestFixture("TestUnusedVar")
    { }

private:
    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkStructMemberUsage();
    }

    void run()
    {
        TEST_CASE(structmember1);
        TEST_CASE(structmember2);
        TEST_CASE(structmember3);
        TEST_CASE(structmember4);
        TEST_CASE(structmember5);
        TEST_CASE(structmember6);
        TEST_CASE(structmember7);
        TEST_CASE(structmember8);
        TEST_CASE(structmember_extern);		// No false positives for extern structs

        TEST_CASE(localvar1);
        TEST_CASE(localvar2);
        TEST_CASE(localvar3);
        TEST_CASE(localvar4);
        TEST_CASE(localvar5);
        TEST_CASE(localvar6);
        TEST_CASE(localvar7);
        TEST_CASE(localvar8);
        TEST_CASE(localvar9); // ticket #1605
        TEST_CASE(localvar10);
        TEST_CASE(localvar11);
        TEST_CASE(localvar12);
        TEST_CASE(localvar13); // ticket #1640
        TEST_CASE(localvaralias1);
        TEST_CASE(localvaralias2); // ticket #1637
        TEST_CASE(localvarasm);

        // Don't give false positives for variables in structs/unions
        TEST_CASE(localvarStruct1);
        TEST_CASE(localvarStruct2);
        TEST_CASE(localvarStruct3);
        TEST_CASE(localvarStruct4); // Ticket #31: sigsegv on incomplete struct

        TEST_CASE(localvarOp);          // Usage with arithmetic operators
        TEST_CASE(localvarInvert);      // Usage with inverted variable
        TEST_CASE(localvarIf);          // Usage in if
        TEST_CASE(localvarIfElse);      // return tmp1 ? tmp2 : tmp3;
        TEST_CASE(localvarOpAssign);    // a |= b;
        TEST_CASE(localvarFor);         // for ( ; var; )
        TEST_CASE(localvarShift);       // 1 >> var
    }

    void structmember1()
    {
        check("struct abc\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) struct or union member 'abc::a' is never used\n"
                                  "[test.cpp:4]: (style) struct or union member 'abc::b' is never used\n"
                                  "[test.cpp:5]: (style) struct or union member 'abc::c' is never used\n"), errout.str());
    }

    void structmember2()
    {
        check("struct ABC\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    int a = abc.a;\n"
              "    int b = abc.b;\n"
              "    int c = abc.c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember3()
    {
        check("struct ABC\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n"
              "\n"
              "static struct ABC abc[] = { {1, 2, 3} };\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    int a = abc[0].a;\n"
              "    int b = abc[0].b;\n"
              "    int c = abc[0].c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember4()
    {
        check("struct ABC\n"
              "{\n"
              "    const int a;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    ABC abc;\n"
              "    if (abc.a == 2);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember5()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    void reset()\n"
              "    {\n"
              "        a = 1;\n"
              "        b = 2;\n"
              "    }\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct AB ab;\n"
              "    ab.reset();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember6()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(char *buf)\n"
              "{\n"
              "    struct AB *ab = (struct AB *)&buf[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(char *buf)\n"
              "{\n"
              "    struct AB *ab = (AB *)&buf[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember7()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(struct AB *ab)\n"
              "{\n"
              "    ab->a = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(struct AB _shuge *ab)\n"
              "{\n"
              "    ab->a = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember8()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(char *ab)\n"
              "{\n"
              "    ((AB *)ab)->b = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember_extern()
    {
        // extern struct => no false positive
        check("extern struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "} ab;\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    ab.b = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // global linkage => no false positive
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "} ab;\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    ab.b = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // static linkage => error message
        check("static struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "} ab;\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    ab.b = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) struct or union member 'AB::a' is never used\n", errout.str());
    }









    void functionVariableUsage(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.functionVariableUsage();
    }

    void localvar1()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i(0);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i(a);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int i(j);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int & i = j;\n"
                              "    j = j;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    const int & i = j;\n"
                              "    j = j;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int & i(j);\n"
                              "    j = j;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    const int & i(j);\n"
                              "    j = j;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * j = 0;\n"
                              "    int * i(j);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * j = 0;\n"
                              "    const int * i(j);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    bool i = false;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    bool i = true;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i;\n"
                              "    i = fgets();\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        // undefined variables are not reported because they may be classes with constructors
        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = undefined;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct S * i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct S * i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct S & i = j;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct S & i = j;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    undefined * i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    int j = i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'j' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i[10] = { 0 };\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo(int n)\n"
                              "{\n"
                              "    int i[n] = { 0 };\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char i[10] = \"123456789\";\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i = \"123456789\";\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());
    }

    void localvar2()
    {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        functionVariableUsage("bool foo()\n"
                              "{\n"
                              "    bool i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        // undefined variables are not reported because they may be classes with constructors
        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("undefined *foo()\n"
                              "{\n"
                              "    undefined * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        functionVariableUsage("int *foo()\n"
                              "{\n"
                              "    int * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        functionVariableUsage("const int *foo()\n"
                              "{\n"
                              "    const int * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        functionVariableUsage("struct S *foo()\n"
                              "{\n"
                              "    struct S * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        functionVariableUsage("const struct S *foo()\n"
                              "{\n"
                              "    const struct S * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0], 0);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(0, a[0]);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(0, a[0], 0);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        // f() can not write a (not supported yet)
        functionVariableUsage("void f(int i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is not assigned a value\n", errout.str());

        // f() can not write a (not supported yet)
        functionVariableUsage("void f(const int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is not assigned a value\n", errout.str());

        // f() writes a
        functionVariableUsage("void f(int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar3()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    if ( abc )\n"
                              "        ;\n"
                              "    else i = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n"), errout.str());
    }

    void localvar4()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(i);\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(&i);\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvar5()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    b = (char)a;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvar6()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = ++a;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());
    }

    void localvar7()// ticket 1253
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    i--;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &ii(i);\n"
                              "    ii--;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &ii=i;\n"
                              "    ii--;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n"), errout.str());
    }

    void localvar8()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i[2];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    A * i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct A * i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct A * i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * i[2];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const int * i[2];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i[2];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i[2];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct A * i[2];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct A * i[2];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo(int n)\n"
                              "{\n"
                              "    int i[n];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    int &j = i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n"
                      "[test.cpp:4]: (style) Variable 'j' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &j = i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n"
                      "[test.cpp:4]: (style) Variable 'j' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &j = i;\n"
                              "    j = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    A * i;\n"
                              "    i->f();\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i;\n"
                              "    if (i);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i = 0;\n"
                              "    if (i);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i = new char[10];\n"
                              "    if (i);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i;\n"
                              "    f(i);\n"
                              "}\n");

        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    return &a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *p = a;\n"
                              "    for (int i = 0; i < 10; i++)\n"
                              "        p[i] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *p = &a[0];\n"
                              "    for (int i = 0; i < 10; i++)\n"
                              "        p[i] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int x;\n"
                              "    a[0] = 0;\n"
                              "    x = a[0];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'x' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c;\n"
                              "    a = b = c = 0;\n"
                              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'c' is assigned a value that is never used\n",
            errout.str());

        functionVariableUsage("int * foo()\n"
                              "{\n"
                              "    return &undefined[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar9()
    {
        // ticket #1605
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    for (int i = 0; i < 10; )\n"
                              "        a[i++] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());
    }

    void localvar10()
    {
        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x) {\n"
                              "        int i;\n"
                              "    } else {\n"
                              "        int i;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n"
                      "[test.cpp:5]: (style) Unused variable: i\n"
                      "[test.cpp:7]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x)\n"
                              "        int i;\n"
                              "    else\n"
                              "        int i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n"
                      "[test.cpp:5]: (style) Unused variable: i\n"
                      "[test.cpp:7]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x) {\n"
                              "        int i;\n"
                              "    } else {\n"
                              "        int i = 0;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n"
                      "[test.cpp:5]: (style) Unused variable: i\n"
                      "[test.cpp:7]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x) {\n"
                              "        int i;\n"
                              "    } else {\n"
                              "        int i;\n"
                              "    }\n"
                              "    i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n"
                      "[test.cpp:5]: (style) Unused variable: i\n"
                      "[test.cpp:7]: (style) Unused variable: i\n", errout.str());
    }

    void localvar11()
    {
        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    if (x == 1)\n"
                              "    {\n"
                              "        a = 123;\n"    // redundant assignment
                              "        return;\n"
                              "    }\n"
                              "    x = a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        // The variable 'a' is initialized. But the initialized value is
        // never used. It is only initialized for security reasons.
        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    if (x == 1)\n"
                              "        a = 123;\n"
                              "    else if (x == 2)\n"
                              "        a = 456;\n"
                              "    else\n"
                              "        return;\n"
                              "    x = a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar12()
    {
        // ticket #1574
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c, d, e, f;\n"
                              "    a = b = c = d = e = f = 0;\n"
                              "\n"
                              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'c' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'd' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'e' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'f' is assigned a value that is never used\n",
            errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c = 0;\n"
                              "    a = b = c;\n"
                              "\n"
                              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n",
            errout.str());

        TODO_ASSERT_EQUALS(
            "[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'c' is assigned a value that is never used\n",
            errout.str());
    }

    void localvar13() // ticket #1640
    {
        functionVariableUsage("void foo( OBJECT *obj )\n"
                              "{\n"
                              "    int x;\n"
                              "    x = obj->ySize / 8;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'x' is assigned a value that is never used\n", errout.str());
    }

    void localvaralias1()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Unused variable: a\n"
                                  "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Unused variable: a\n"
                                  "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)&a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)&a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = static_cast<char *>(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = static_cast<const char *>(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        // a is not a local variable and b is aliased to it (not supported yet)
        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}\n");
        TODO_ASSERT_EQUALS(std::string("[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        // a is not a local variable and b is aliased to it (not supported yet)
        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}\n");
        TODO_ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        // a is not a local variable and b is aliased to it (not supported yet)
        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "    }\n"
                              "}\n");
        TODO_ASSERT_EQUALS(std::string("[test.cpp:6]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = static_cast<char *>(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = static_cast<const char *>(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("void foo(int a[10])\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a[10];\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    int *d = b;\n"
                              "    *d = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Unused variable: a\n"
                                  "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"
                                  "[test.cpp:5]: (style) Variable 'c' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    c = b;\n"
                              "    *c = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Unused variable: a\n"
                                  "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    c = b;\n"
                              "    *c = 0;\n"
                              "    c = a;\n"
                              "    *c = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
                                  "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"), errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    b[-10] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    b[-10] = 0;\n"
                              "    int * c = b - 10;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
                      "[test.cpp:5]: (style) Variable 'c' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    x = c[0];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is not assigned a value\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    c[1] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    c[1] = c[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    int d = c[0];\n"
                              "    f(d);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is not assigned a value\n", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = (struct S *)a;\n"
                              "    s->c[0] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = (struct S *)a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    const struct S * s = (const struct S *)a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = static_cast<struct S *>(a);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    const struct S * s = static_cast<const struct S *>(a);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used\n", errout.str());
    }

    void localvaralias2() // ticket 1637
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * a;\n"
                              "    a = a;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarasm()
    {
        functionVariableUsage("void foo(int &b)\n"
                              "{\n"
                              "    int a;\n"
                              "    asm();\n"
                              "    b = a;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }



    void localvarStruct1()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static const struct{ int x, y, w, h; } bounds = {1,2,3,4};\n"
                              "    return bounds.x + bounds.y + bounds.w + bounds.h;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarStruct2()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct ABC { int a, b, c; };\n"
                              "    struct ABC abc = { 1, 2, 3 };\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarStruct3()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 10;\n"
                              "    union { struct { unsigned char x; }; unsigned char z; };\n"
                              "    do {\n"
                              "        func();\n"
                              "    } while(a--);\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (style) Unused variable: z\n"), errout.str());
    }

    void localvarStruct4()
    {
        /* This must not SIGSEGV: */
        // FIXME!!
        //functionVariableUsage("void foo()\n"
        //                      "{\n"
        //                      "    struct { \n");
    }



    void localvarOp()
    {
        const char op[] = "+-*/%&|^";
        for (const char *p = op; *p; ++p)
        {
            std::string code("int main()\n"
                             "{\n"
                             "    int tmp = 10;\n"
                             "    return 123 " + std::string(1, *p) + " tmp;\n"
                             "}\n");
            functionVariableUsage(code.c_str());
            ASSERT_EQUALS(std::string(""), errout.str());
        }
    }

    void localvarInvert()
    {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    return ~tmp;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarIf()
    {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    if ( tmp )\n"
                              "        return 1;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarIfElse()
    {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int tmp1 = 1;\n"
                              "    int tmp2 = 2;\n"
                              "    int tmp3 = 3;\n"
                              "    return tmp1 ? tmp2 : tmp3;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarOpAssign()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = 2;\n"
                              "    a |= b;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());
    }

    void localvarFor()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    for (;a;);\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarShift()
    {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int var = 1;\n"
                              "    return 1 >> var;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

};

REGISTER_TEST(TestUnusedVar)


