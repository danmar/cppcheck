/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include "checkunusedvar.h"

#include <sstream>
extern std::ostringstream errout;

class TestUnusedVar : public TestFixture {
public:
    TestUnusedVar() : TestFixture("TestUnusedVar")
    { }

private:
    void run() {
        TEST_CASE(structmember1);
        TEST_CASE(structmember2);
        TEST_CASE(structmember3);
        TEST_CASE(structmember4);
        TEST_CASE(structmember5);
        TEST_CASE(structmember6);
        TEST_CASE(structmember7);
        TEST_CASE(structmember8);
        TEST_CASE(structmember9);  // #2017 - struct is inherited
        TEST_CASE(structmember_extern);		// No false positives for extern structs
        TEST_CASE(structmember10);

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
        TEST_CASE(localvar14); // ticket #5
        TEST_CASE(localvar15);
        TEST_CASE(localvar16); // ticket #1709
        TEST_CASE(localvar17); // ticket #1720
        TEST_CASE(localvar18); // ticket #1723
        TEST_CASE(localvar19); // ticket #1776
        TEST_CASE(localvar20); // ticket #1799
        TEST_CASE(localvar21); // ticket #1807
        TEST_CASE(localvar22); // ticket #1811
        TEST_CASE(localvar23); // ticket #1808
        TEST_CASE(localvar24); // ticket #1803
        TEST_CASE(localvar25); // ticket #1729
        TEST_CASE(localvar26); // ticket #1894
        TEST_CASE(localvar27); // ticket #2160
        TEST_CASE(localvar28); // ticket #2205
        TEST_CASE(localvar29); // ticket #2206 (array initialization)
        TEST_CASE(localvar30);
        TEST_CASE(localvar31); // ticket #2286
        TEST_CASE(localvar32); // ticket #2330
        TEST_CASE(localvar33); // ticket #2346
        TEST_CASE(localvar34); // ticket #2368
        TEST_CASE(localvar35); // ticket #2535
        TEST_CASE(localvar36); // ticket #2805
        TEST_CASE(localvar37); // ticket #3078
        TEST_CASE(localvaralias1);
        TEST_CASE(localvaralias2); // ticket #1637
        TEST_CASE(localvaralias3); // ticket #1639
        TEST_CASE(localvaralias4); // ticket #1643
        TEST_CASE(localvaralias5); // ticket #1647
        TEST_CASE(localvaralias6); // ticket #1729
        TEST_CASE(localvaralias7); // ticket #1732
        TEST_CASE(localvaralias8);
        TEST_CASE(localvaralias9); // ticket #1996
        TEST_CASE(localvaralias10); // ticket #2004
        TEST_CASE(localvarasm);
        TEST_CASE(localvarstatic);
        TEST_CASE(localvardynamic1);
        TEST_CASE(localvardynamic2); // ticket #2904
        TEST_CASE(localvararray1);  // ticket #2780
        TEST_CASE(localvarstring1);
        TEST_CASE(localvarstring2); // ticket #2929
        TEST_CASE(localvarconst);

        // Don't give false positives for variables in structs/unions
        TEST_CASE(localvarStruct1);
        TEST_CASE(localvarStruct2);
        TEST_CASE(localvarStruct3);
        TEST_CASE(localvarStruct4); // Ticket #31: sigsegv on incomplete struct
        TEST_CASE(localvarStruct5);
        TEST_CASE(localvarStruct6);

        TEST_CASE(localvarOp);          // Usage with arithmetic operators
        TEST_CASE(localvarInvert);      // Usage with inverted variable
        TEST_CASE(localvarIf);          // Usage in if
        TEST_CASE(localvarIfElse);      // return tmp1 ? tmp2 : tmp3;
        TEST_CASE(localvarOpAssign);    // a |= b;
        TEST_CASE(localvarFor);         // for ( ; var; )
        TEST_CASE(localvarShift1);      // 1 >> var
        TEST_CASE(localvarShift2);      // x = x >> 1
        TEST_CASE(localvarCast);
        TEST_CASE(localvarClass);
        TEST_CASE(localvarUnused);
        TEST_CASE(localvarFunction); // ticket #1799
        TEST_CASE(localvarIfNOT);    // #3104 - if ( NOT var )
    }

    void checkStructMemberUsage(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for unused variables..
        CheckUnusedVar checkUnusedVar(&tokenizer, &settings, this);
        checkUnusedVar.checkStructMemberUsage();
    }

    void structmember1() {
        checkStructMemberUsage("struct abc\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "    int c;\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) struct or union member 'abc::a' is never used\n"
                      "[test.cpp:4]: (style) struct or union member 'abc::b' is never used\n"
                      "[test.cpp:5]: (style) struct or union member 'abc::c' is never used\n", errout.str());
    }

    void structmember2() {
        checkStructMemberUsage("struct ABC\n"
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

    void structmember3() {
        checkStructMemberUsage("struct ABC\n"
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

    void structmember4() {
        checkStructMemberUsage("struct ABC\n"
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

    void structmember5() {
        checkStructMemberUsage("struct AB\n"
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

    void structmember6() {
        checkStructMemberUsage("struct AB\n"
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

        checkStructMemberUsage("struct AB\n"
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

    void structmember7() {
        checkStructMemberUsage("struct AB\n"
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

        checkStructMemberUsage("struct AB\n"
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

    void structmember8() {
        checkStructMemberUsage("struct AB\n"
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

    void structmember9() {
        checkStructMemberUsage("struct base {\n"
                               "    int a;\n"
                               "};\n"
                               "\n"
                               "struct derived : public base {"
                               "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember10() {
        // Fred may have some useful side-effects
        checkStructMemberUsage("struct abc {\n"
                               "    Fred fred;\n"
                               "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember_extern() {
        // extern struct => no false positive
        checkStructMemberUsage("extern struct AB\n"
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
        checkStructMemberUsage("struct AB\n"
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
        checkStructMemberUsage("static struct AB\n"
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

    void functionVariableUsage(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check for unused variables..
        CheckUnusedVar checkUnusedVar(&tokenizer, &settings, this);
        checkUnusedVar.checkFunctionVariableUsage();
    }

    void localvar1() {
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

        // if a is undefined then Cppcheck can't determine if "int i(a)" is a
        // * variable declaration
        // * function declaration
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i(a);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

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

    void localvar2() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        functionVariableUsage("bool foo()\n"
                              "{\n"
                              "    bool i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        // undefined variables are not reported because they may be classes with constructors
        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("undefined *foo()\n"
                              "{\n"
                              "    undefined * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        functionVariableUsage("int *foo()\n"
                              "{\n"
                              "    int * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        functionVariableUsage("const int *foo()\n"
                              "{\n"
                              "    const int * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        functionVariableUsage("struct S *foo()\n"
                              "{\n"
                              "    struct S * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        functionVariableUsage("const struct S *foo()\n"
                              "{\n"
                              "    const struct S * i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

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
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is not assigned a value\n",
                           "", errout.str());

        // f() can not write a (not supported yet)
        functionVariableUsage("void f(const int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is not assigned a value\n",
                           "", errout.str());

        // f() writes a
        functionVariableUsage("void f(int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar3() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    if ( abc )\n"
                              "        ;\n"
                              "    else i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());
    }

    void localvar4() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(i);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(&i);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar5() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    b = (char)a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar6() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = ++a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());
    }

    void localvar7() { // ticket 1253
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    i--;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &ii(i);\n"
                              "    ii--;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &ii=i;\n"
                              "    ii--;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value\n", errout.str());
    }

    void localvar8() {
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
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const int * i[2];\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i[2];\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i[2];\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct A * i[2];\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct A * i[2];\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

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
                              "    a = b = c = f();\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("int * foo()\n"
                              "{\n"
                              "    return &undefined[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar9() {
        // ticket #1605
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    for (int i = 0; i < 10; )\n"
                              "        a[i++] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());
    }

    void localvar10() {
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

    void localvar11() {
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

    void localvar12() {
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

        TODO_ASSERT_EQUALS(
            "[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'c' is assigned a value that is never used\n",

            "[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
            "[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n",
            errout.str());
    }

    void localvar13() { // ticket #1640
        functionVariableUsage("void foo( OBJECT *obj )\n"
                              "{\n"
                              "    int x;\n"
                              "    x = obj->ySize / 8;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'x' is assigned a value that is never used\n", errout.str());
    }

    void localvar14() {
        // ticket #5
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n", errout.str());
    }

    void localvar15() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    int b[a];\n"
                              "    b[0] = 0;\n"
                              "    return b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    int * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return *b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    const int * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct B * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    struct B * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("const struct B * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    const struct B * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar16() { // ticket #1709
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    char buf[5];\n"
                              "    char *ptr = buf;\n"
                              "    *(ptr++) = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    char buf[5];\n"
                              "    char *ptr = buf - 1;\n"
                              "    *(++ptr) = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar17() { // ticket #1720
        // Don't crash when checking the code below!
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct DATA *data;\n"
                              "    char *k = data->req;\n"
                              "    char *ptr;\n"
                              "    char *line_start;\n"
                              "    ptr = data->buffer;\n"
                              "    line_start = ptr;\n"
                              "    data->info = k;\n"
                              "    line_start = ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'line_start' is assigned a value that is never used\n", errout.str());
    }

    void localvar18() { // ticket #1723
        functionVariableUsage("A::A(int iValue) {\n"
                              "    UserDefinedException* pe = new UserDefinedException();\n"
                              "    throw pe;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar19() { // ticket #1776
        functionVariableUsage("void foo() {\n"
                              "    int a[10];\n"
                              "    int c;\n"
                              "    c = *(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'a' is not assigned a value\n"
                      "[test.cpp:3]: (style) Variable 'c' is assigned a value that is never used\n", errout.str());
    }

    void localvar20() { // ticket #1799
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char c1 = 'c';\n"
                              "    char c2[] = { c1 };\n"
                              "    a(c2);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar21() { // ticket #1807
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buffer[1024];\n"
                              "    bar((void *)buffer);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar22() { // ticket #1811
        functionVariableUsage("int foo(int u, int v)\n"
                              "{\n"
                              "    int h, i;\n"
                              "    h = 0 ? u : v;\n"
                              "    i = 1 ? u : v;\n"
                              "    return h + i;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar23() { // ticket #1808
        functionVariableUsage("int foo(int c)\n"
                              "{\n"
                              "    int a;\n"
                              "    int b[10];\n"
                              "    a = b[c] = 0;\n"
                              "    return a;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar24() { // ticket #1803
        functionVariableUsage("class MyException\n"
                              "{\n"
                              "    virtual void raise() const\n"
                              "    {\n"
                              "        throw *this;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar25() { // ticket #1729
        functionVariableUsage("int main() {\n"
                              "    int ppos = 1;\n"
                              "    int pneg = 0;\n"
                              "    const char*edge = ppos? \" +\" : pneg ? \" -\" : \"\";\n"
                              "    printf(\"This should be a '+' -> %s\n\", edge);\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar26() { // ticket #1894
        functionVariableUsage("int main() {\n"
                              "    const Fred &fred = getfred();\n"
                              "    int *p = fred.x();\n"
                              "    *p = 0;"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar27() { // ticket #2160
        functionVariableUsage("void f(struct s *ptr) {\n"
                              "    int param = 1;\n"
                              "    ptr->param = param++;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar28() { // ticket #2205
        functionVariableUsage("void f(char* buffer, int value) {\n"
                              "    char* pos = buffer;\n"
                              "    int size = value;\n"
                              "    *(int*)pos = size;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar29() { // ticket #2206
        functionVariableUsage("void f() {\n"
                              "    float s_ranges[] = { 0, 256 };\n"
                              "    float* ranges[] = { s_ranges };\n"
                              "    cout << ranges[0][0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar30() { // ticket #2264
        functionVariableUsage("void f() {\n"
                              "    Engine *engine = e;\n"
                              "    x->engine = engine->clone();\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar31() { // ticket #2286
        functionVariableUsage("void f() {\n"
                              "    int x = 0;\n"
                              "    a.x = x - b;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar32() { // ticket #2330
        functionVariableUsage("void f() {\n"
                              "    int x;\n"
                              "    fstream &f = getfile();\n"
                              "    f >> x;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar33() { // ticket #2345
        functionVariableUsage("void f() {\n"
                              "    Abc* abc = getabc();\n"
                              "    while (0 != (abc = abc->next())) {\n"
                              "        ++nOldNum;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar34() { // ticket #2368
        functionVariableUsage("void f() {\n"
                              "    int i = 0;\n"
                              "    if (false) {\n"
                              "    } else {\n"
                              "        j -= i;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar35() { // ticket #2535
        functionVariableUsage("void f() {\n"
                              "    int a, b;\n"
                              "    x(1,a,b);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar36() { // ticket #2805
        functionVariableUsage("int f() {\n"
                              "    int a, b;\n"
                              "    a = 2 * (b = 3);\n"
                              "    return a + b;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar37() { // ticket #3078
        functionVariableUsage("void f() {\n"
                              "    int a = 2;\n"
                              "    ints.at(a) = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)&a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)&a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = static_cast<char *>(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = static_cast<const char *>(&a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = static_cast<char *>(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = static_cast<const char *>(a);\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c;\n"
                              "    *c = b[0];\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'c' is not assigned a value\n", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int c = b[0];\n"
                              "    c = c;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int c = b[0];\n"
                              "    c = c;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a[0];\n"
                              "    a[0] = b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = &a[0];\n"
                              "    a[0] = b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    a[0] = b[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo(int a[10])\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a[10];\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    int *d = b;\n"
                              "    *d = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"
                      "[test.cpp:5]: (style) Variable 'c' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    c = b;\n"
                              "    *c = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());

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

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    int c[10];\n"
                              "    int *d;\n"
                              "    d = b;\n"
                              "    d = a;\n"
                              "    d = c;\n"
                              "    *d = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: b\n"
                      "[test.cpp:5]: (style) Variable 'c' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    int c[10];\n"
                              "    int *d;\n"
                              "    d = b; *d = 0;\n"
                              "    d = a; *d = 0;\n"
                              "    d = c; *d = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n"
                      "[test.cpp:5]: (style) Variable 'c' is assigned a value that is never used\n", errout.str());
    }

    void localvaralias2() { // ticket 1637
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * a;\n"
                              "    a = a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias3() { // ticket 1639
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    BROWSEINFO    info;\n"
                              "    char          szDisplayName[MAX_PATH];\n"
                              "    info.pszDisplayName = szDisplayName;\n"
                              "    SHBrowseForFolder(&info);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias4() { // ticket 1643
        functionVariableUsage("struct AB { int a; int b; } ab;\n"
                              "void foo()\n"
                              "{\n"
                              "    int * a = &ab.a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("struct AB { int a; int b; } ab;\n"
                              "void foo()\n"
                              "{\n"
                              "    int * a = &ab.a;\n"
                              "    *a = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct AB { int a; int b; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct AB ab;\n"
                              "    int * a = &ab.a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ab' is not assigned a value\n"
                      "[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("struct AB { int a; int b; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct AB ab;\n"
                              "    int * a = &ab.a;\n"
                              "    *a = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias5() { // ticket 1647
        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[0];\n"
                              "    *p++ = 0;\n"
                              "    return buf[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[1];\n"
                              "    *p-- = 0;\n"
                              "    return buf[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[0];\n"
                              "    *++p = 0;\n"
                              "    return buf[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[1];\n"
                              "    *--p = 0;\n"
                              "    return buf[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias6() { // ticket 1729
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    } else {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'buf' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'buf' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: buf\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    srcdata = buf;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    } else {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'buf' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'buf' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: buf\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    srcdata = buf;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Unused variable: vdata\n", errout.str());
    }

    void localvaralias7() { // ticket 1732
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *c[10];\n"
                              "    char **cp;\n"
                              "    cp = c;\n"
                              "    *cp = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias8() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    else\n"
                              "        pb = b4;\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    else {\n"
                              "        pb = b1;\n"
                              "        pb = b4;\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    else {\n"
                              "        pb = b1;\n"
                              "        pb = b2;\n"
                              "        pb = b3;\n"
                              "        pb = b4;\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());


        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    pb = b4;\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: b1\n"
                      "[test.cpp:4]: (style) Unused variable: b2\n"
                      "[test.cpp:5]: (style) Unused variable: b3\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "            else\n"
                              "                pb = b4;\n"
                              "        }\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "            else {\n"
                              "                pb = b1;\n"
                              "                pb = b4;\n"
                              "            }\n"
                              "        }\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "            else {\n"
                              "                pb = b1;\n"
                              "                pb = b2;\n"
                              "                pb = b3;\n"
                              "                pb = b4;\n"
                              "            }\n"
                              "        }\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "        }\n"
                              "    }\n"
                              "    pb = b4;\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: b1\n"
                      "[test.cpp:4]: (style) Unused variable: b2\n"
                      "[test.cpp:5]: (style) Unused variable: b3\n", errout.str());
    }

    void localvaralias9() { // ticket 1996
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Foo foo;\n"
                              "    Foo &ref = foo;\n"
                              "    ref[0] = 123;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias10() { // ticket 2004
        functionVariableUsage("void foo(Foo &foo)\n"
                              "{\n"
                              "    Foo &ref = foo;\n"
                              "    int *x = &ref.x();\n"
                              "    *x = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarasm() {
        functionVariableUsage("void foo(int &b)\n"
                              "{\n"
                              "    int a;\n"
                              "    asm();\n"
                              "    b = a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarStruct1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static const struct{ int x, y, w, h; } bounds = {1,2,3,4};\n"
                              "    return bounds.x + bounds.y + bounds.w + bounds.h;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarStruct2() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct ABC { int a, b, c; };\n"
                              "    struct ABC abc = { 1, 2, 3 };\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'abc' is assigned a value that is never used\n", errout.str());
    }

    void localvarStruct3() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 10;\n"
                              "    union { struct { unsigned char x; }; unsigned char z; };\n"
                              "    do {\n"
                              "        func();\n"
                              "    } while(a--);\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: x\n"
                           "[test.cpp:4]: (style) Unused variable: z\n",

                           "", errout.str());
    }

    void localvarStruct4() {
        /* This must not SIGSEGV: */
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct { \n");
    }

    void localvarStruct5() {
        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    a.i = 0;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    a.i = 0;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a = { 0 };\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a = { 0 };\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("class A { int i; public: A(); { } };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n", errout.str());

        functionVariableUsage("class A { int i; public: A(); { } };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A { unknown i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A : public Fred { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarStruct6() {
        functionVariableUsage("class Type { };\n"
                              "class A {\n"
                              "public:\n"
                              "    Type & get() { return t; }\n"
                              "private:\n"
                              "    Type t;\n"
                              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarOp() {
        const char op[] = "+-*/%&|^";
        for (const char *p = op; *p; ++p) {
            std::string code("int main()\n"
                             "{\n"
                             "    int tmp = 10;\n"
                             "    return 123 " + std::string(1, *p) + " tmp;\n"
                             "}\n");
            functionVariableUsage(code.c_str());
            ASSERT_EQUALS("", errout.str());
        }
    }

    void localvarInvert() {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    return ~tmp;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarIf() {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    if ( tmp )\n"
                              "        return 1;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarIfElse() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int tmp1 = 1;\n"
                              "    int tmp2 = 2;\n"
                              "    int tmp3 = 3;\n"
                              "    return tmp1 ? tmp2 : tmp3;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarOpAssign() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = 2;\n"
                              "    a |= b;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    int a = 1;\n"
                              "    (b).x += a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarFor() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    for (;a;);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarShift1() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int var = 1;\n"
                              "    return 1 >> var;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarShift2() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int var = 1;\n"
                              "    while (var = var >> 1) { }\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarCast() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = static_cast<int>(a);\n"
                              "    return b;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarClass() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    class B : public A {\n"
                              "        int a;\n"
                              "        int f() { return a; }\n"
                              "    } b;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarUnused() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((unused));\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((unused)) = true;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((unused)) test;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((unused)) test = true;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarFunction() {
        functionVariableUsage("void check_dlsym(void*& h)\n"
                              "{\n"
                              "  typedef void (*function_type) (void);\n"
                              "  function_type fn;\n"
                              "  fn = reinterpret_cast<function_type>(dlsym(h, \"try_allocation\"));\n"
                              "  fn();\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarstatic() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i(0);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        // If "a" is undefined then Cppcheck can't determine whether
        // "static int i(a);" is a variable declaration or a function
        // declaration.
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i(a);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int j = 0;\n"
                              "    static int i(j);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("int * foo(int x)\n"
                              "{\n"
                              "    static int a[] = { 3, 4, 5, 6 };\n"
                              "    static int b[] = { 4, 5, 6, 7 };\n"
                              "    static int c[] = { 5, 6, 7, 8 };\n"
                              "    b[1] = 1;\n"
                              "    return x ? a : c;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());
    }

    void localvardynamic1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = malloc(16);\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = g_malloc(16);\n"
                              "    g_free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = kmalloc(16, GFP_KERNEL);\n"
                              "    kfree(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = vmalloc(16, GFP_KERNEL);\n"
                              "    vfree(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());


        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char[16];\n"
                              "    delete[] ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new ( nothrow ) char[16];\n"
                              "    delete[] ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new ( std::nothrow ) char[16];\n"
                              "    delete[] ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char;\n"
                              "    delete ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = malloc(16);\n"
                              "    ptr[0] = 123;\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char[16];\n"
                              "    ptr[0] = 123;\n"
                              "    delete[] ptr;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int a; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'fred' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("struct Fred { int a; Fred() : a(0) {} };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Fred* fred = malloc(sizeof(Fred));\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    free(fred);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'fred' is allocated memory that is never used\n", errout.str());


        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = names[i];\n"
                              "    delete[] ptr;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvardynamic2() {
        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    delete ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new (nothrow ) Fred();\n"
                              "    delete ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new (std::nothrow) Fred();\n"
                              "    delete ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    delete ptr;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = new Fred();\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    delete ptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used\n", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    delete ptr;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvararray1() {
        functionVariableUsage("void foo() {\n"
                              "    int p[5];\n"
                              "    *p = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarstring1() { // ticket #1597
        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unused variable: s\n", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "    s = \"foo\";\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 's' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    std::string s = \"foo\";\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 's' is assigned a value that is never used\n", errout.str());

        functionVariableUsage("std::string foo() {\n"
                              "    std::string s;\n"
                              "    return s;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 's' is not assigned a value\n", errout.str());

        functionVariableUsage("std::string foo() {\n"
                              "    std::string s = \"foo\";\n"
                              "    return s;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarstring2() { // ticket #2929
        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "    int i;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unused variable: s\n"
                      "[test.cpp:3]: (style) Unused variable: i\n", errout.str());
    }

    void localvarconst() {
        functionVariableUsage("void foo() {\n"
                              "    const bool b = true;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'b' is assigned a value that is never used\n", errout.str());
    }

    // ticket #3104 - false positive when variable is read with "if (NOT var)"
    void localvarIfNOT() {
        functionVariableUsage("void f() {\n"
                              "    bool x = foo();\n"
                              "    if (NOT x) { }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestUnusedVar)
