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
#include "checkunusedvar.h"


class TestUnusedVar : public TestFixture {
public:
    TestUnusedVar() : TestFixture("TestUnusedVar") {
    }

private:
    void run() {
        TEST_CASE(emptyclass);  // #5355 - False positive: Variable is not assigned a value.
        TEST_CASE(emptystruct);  // #5355 - False positive: Variable is not assigned a value.

        TEST_CASE(structmember1);
        TEST_CASE(structmember2);
        TEST_CASE(structmember3);
        TEST_CASE(structmember4);
        TEST_CASE(structmember5);
        TEST_CASE(structmember6);
        TEST_CASE(structmember7);
        TEST_CASE(structmember8);
        TEST_CASE(structmember9);  // #2017 - struct is inherited
        TEST_CASE(structmember_extern); // No false positives for extern structs
        TEST_CASE(structmember10);
        TEST_CASE(structmember11); // #4168 - initialization with {} / passed by address to unknown function

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
        TEST_CASE(localvar38);
        TEST_CASE(localvar39); // ticket #3454
        TEST_CASE(localvar40); // ticket #3473
        TEST_CASE(localvar41); // ticket #3603
        TEST_CASE(localvar42); // ticket #3742
        TEST_CASE(localvar43); // ticket #3602
        TEST_CASE(localvar44); // ticket #4020
        TEST_CASE(localvar45); // ticket #4899
        TEST_CASE(localvar46); // ticket #5491 (C++11 style initialization)
        TEST_CASE(localvar47); // ticket #6603
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
        TEST_CASE(localvaralias11); // ticket #4423 - iterator
        TEST_CASE(localvaralias12); // ticket #4394
        TEST_CASE(localvaralias13); // ticket #4487
        TEST_CASE(localvarasm);
        TEST_CASE(localvarstatic);
        TEST_CASE(localvarextern);
        TEST_CASE(localvardynamic1);
        TEST_CASE(localvardynamic2); // ticket #2904
        TEST_CASE(localvardynamic3); // ticket #3467
        TEST_CASE(localvararray1);  // ticket #2780
        TEST_CASE(localvararray2);  // ticket #3438
        TEST_CASE(localvararray3);  // ticket #3980
        TEST_CASE(localvarstring1);
        TEST_CASE(localvarstring2); // ticket #2929
        TEST_CASE(localvarconst1);
        TEST_CASE(localvarconst2);

        TEST_CASE(localvarthrow); // ticket #3687

        TEST_CASE(localVarStd);

        // Don't give false positives for variables in structs/unions
        TEST_CASE(localvarStruct1);
        TEST_CASE(localvarStruct2);
        TEST_CASE(localvarStruct3);
        TEST_CASE(localvarStruct5);
        TEST_CASE(localvarStruct6);
        TEST_CASE(localvarStructArray);

        TEST_CASE(localvarOp);          // Usage with arithmetic operators
        TEST_CASE(localvarInvert);      // Usage with inverted variable
        TEST_CASE(localvarIf);          // Usage in if
        TEST_CASE(localvarIfElse);      // return tmp1 ? tmp2 : tmp3;
        TEST_CASE(localvarOpAssign);    // a |= b;
        TEST_CASE(localvarFor);         // for ( ; var; )
        TEST_CASE(localvarForEach);     // #4155 - BOOST_FOREACH, hlist_for_each, etc
        TEST_CASE(localvarShift1);      // 1 >> var
        TEST_CASE(localvarShift2);      // x = x >> 1
        TEST_CASE(localvarShift3);      // x << y
        TEST_CASE(localvarCast);
        TEST_CASE(localvarClass);
        TEST_CASE(localvarUnused);
        TEST_CASE(localvarFunction); // ticket #1799
        TEST_CASE(localvarIfNOT);    // #3104 - if ( NOT var )
        TEST_CASE(localvarAnd);      // #3672
        TEST_CASE(localvarSwitch);   // #3744 - false positive when localvar is used in switch
        TEST_CASE(localvarNULL);     // #4203 - Setting NULL value is not redundant - it is safe
        TEST_CASE(localvarUnusedGoto);    // #4447, #4558 goto

        TEST_CASE(localvarCpp11Initialization);

        TEST_CASE(chainedAssignment); // #5466

        TEST_CASE(crash1);
        TEST_CASE(crash2);
        TEST_CASE(usingNamespace);     // #4585
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

    // #5355 - False positive: Variable is not assigned a value.
    void emptyclass() {
        functionVariableUsage("class Carla {\n"
                              "};\n"
                              "class Fred : Carla {\n"
                              "};\n"
                              "void foo() {\n"
                              "    Fred fred;\n"
                              "    throw fred;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // #5355 - False positive: Variable is not assigned a value.
    void emptystruct() {
        functionVariableUsage("struct Fred {\n"
                              "};\n"
                              "void foo() {\n"
                              "    Fred fred;\n"
                              "    throw fred;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember1() {
        checkStructMemberUsage("struct abc\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "    int c;\n"
                               "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) struct or union member 'abc::a' is never used.\n"
                      "[test.cpp:4]: (style) struct or union member 'abc::b' is never used.\n"
                      "[test.cpp:5]: (style) struct or union member 'abc::c' is never used.\n", errout.str());
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
                               "}");
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
                               "}");
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
                               "}");
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
                               "}");
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
                               "}");
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
                               "}");
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
                               "}");
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
                               "}");
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
                               "}");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember9() {
        checkStructMemberUsage("struct base {\n"
                               "    int a;\n"
                               "};\n"
                               "\n"
                               "struct derived : public base {"
                               "}");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember10() {
        // Fred may have some useful side-effects
        checkStructMemberUsage("struct abc {\n"
                               "    Fred fred;\n"
                               "};");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember11() { // #4168
        checkStructMemberUsage("struct abc { int x; };\n"
                               "struct abc s = {0};\n"
                               "void f() { do_something(&s); }");
        ASSERT_EQUALS("", errout.str());

        checkStructMemberUsage("struct abc { int x; };\n"
                               "struct abc s = {0};\n"
                               "void f() { }");
        TODO_ASSERT_EQUALS("abc::x is not used", "", errout.str());
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
                               "}");
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
                               "}");
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
                               "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) struct or union member 'AB::a' is never used.\n", errout.str());
    }

    void functionVariableUsage(const char code[], const char filename[]="test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        if (!tokenizer.tokenize(istr, filename))
            return;

        // Check for unused variables..
        CheckUnusedVar checkUnusedVar(&tokenizer, &settings, this);
        checkUnusedVar.checkFunctionVariableUsage();
    }

    void localvar1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i(0);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        // if a is undefined then Cppcheck can't determine if "int i(a)" is a
        // * variable declaration
        // * function declaration
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int i(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int & i = j;\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    const int & i = j;\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int & i(j);\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    const int & i(j);\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * j = 0;\n"
                              "    int * i(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * j = 0;\n"
                              "    const int * i(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    bool i = false;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    bool i = true;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i;\n"
                              "    i = fgets();\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        // undefined variables are not reported because they may be classes with constructors
        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i = 0;\n"
                              "}\n",
                              "test.c");
        ASSERT_EQUALS("[test.c:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = undefined;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct S * i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct S * i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct S & i = j;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct S & i = j;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    undefined * i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    int j = i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'j' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i[10] = { 0 };\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo(int n)\n"
                              "{\n"
                              "    int i[n] = { 0 };\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char i[10] = \"123456789\";\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i = \"123456789\";\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10,d=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        d = code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'd' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10,d=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        g(d);\n"
                              "        d = code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10,d=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        if (i == 3) {\n"
                              "            return d;\n"
                              "        }\n"
                              "        d = code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,a=10,b=20;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<a<<std::endl;\n"
                              "        int tmp=a;\n"
                              "        a=b;\n"
                              "        b=tmp;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        d += code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'd' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        g(d);\n"
                              "        d += code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        if (i == 3) {\n"
                              "            return d;\n"
                              "        }\n"
                              "        d += code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a=10,b=20;\n"
                              "    while(a != 30) {\n"
                              "        std::cout<<a<<std::endl;\n"
                              "        int tmp=a;\n"
                              "        a=b;\n"
                              "        b=tmp;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        d += code;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'd' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        g(d);\n"
                              "        d += code;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        if (i == 3) {\n"
                              "            return d;\n"
                              "        }\n"
                              "        d += code;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a=10,b=20;\n"
                              "    do {\n"
                              "        std::cout<<a<<std::endl;\n"
                              "        int tmp=a;\n"
                              "        a=b;\n"
                              "        b=tmp;\n"
                              "    } while( a!=30 );\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    for(int i=0; i < 10; i++) {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "            code += 2;\n"
                              "        }\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    for(int i=0; i < 10; i++) {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "        }\n"
                              "        code += 2;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    while(code < 20) {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "            code += 2;\n"
                              "        }\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    do {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "            code += 2;\n"
                              "        }\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo(int j = 0) {\n" // #5985 - default function parameters should not affect checking results
                              "    int i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());
    }

    void localvar2() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("bool foo()\n"
                              "{\n"
                              "    bool i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        // undefined variables are not reported because they may be classes with constructors
        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i;\n"
                              "    return i;\n"
                              "}\n",
                              "test.c");
        ASSERT_EQUALS("[test.c:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("undefined *foo()\n"
                              "{\n"
                              "    undefined * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("int *foo()\n"
                              "{\n"
                              "    int * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("const int *foo()\n"
                              "{\n"
                              "    const int * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("struct S *foo()\n"
                              "{\n"
                              "    struct S * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("const struct S *foo()\n"
                              "{\n"
                              "    const struct S * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0], 0);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(0, a[0]);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(0, a[0], 0);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        // f() can not write a (not supported yet)
        functionVariableUsage("void f(int i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is not assigned a value.\n",
                           "", errout.str());

        // f() can not write a (not supported yet)
        functionVariableUsage("void f(const int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is not assigned a value.\n",
                           "", errout.str());

        // f() writes a
        functionVariableUsage("void f(int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar3() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    if ( abc )\n"
                              "        ;\n"
                              "    else i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());
    }

    void localvar4() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(i);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(&i);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar5() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    b = (char)a;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar6() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = ++a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());
    }

    void localvar7() { // ticket 1253
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    i--;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &ii(i);\n"
                              "    ii--;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &ii=i;\n"
                              "    ii--;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is not assigned a value.\n", errout.str());
    }

    void localvar8() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i[2];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    A * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct A * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct A * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const int * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct A * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct A * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", "", errout.str());

        functionVariableUsage("void foo(int n)\n"
                              "{\n"
                              "    int i[n];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    int &j = i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n"
                      "[test.cpp:4]: (style) Variable 'j' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &j = i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n"
                      "[test.cpp:4]: (style) Variable 'j' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &j = i;\n"
                              "    j = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("double foo()\n"
                              "{\n"
                              "    double i = 0.0;\n"
                              "    const double j = i;\n"
                              "    return j;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    A * i;\n"
                              "    i->f();\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i;\n"
                              "    if (i);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i = 0;\n"
                              "    if (i);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i = new char[10];\n"
                              "    if (i);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i;\n"
                              "    f(i);\n"
                              "}");

        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    return &a;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *p = a;\n"
                              "    for (int i = 0; i < 10; i++)\n"
                              "        p[i] = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *p = &a[0];\n"
                              "    for (int i = 0; i < 10; i++)\n"
                              "        p[i] = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int x;\n"
                              "    a[0] = 0;\n"
                              "    x = a[0];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'x' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c;\n"
                              "    a = b = c = f();\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used.\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used.\n"
                      "[test.cpp:4]: (style) Variable 'c' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("int * foo()\n"
                              "{\n"
                              "    return &undefined[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar9() {
        // ticket #1605
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    for (int i = 0; i < 10; )\n"
                              "        a[i++] = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());
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
                              "}");
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
                              "}");
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
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: i\n"
                      "[test.cpp:5]: (style) Unused variable: i\n"
                      "[test.cpp:7]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x) {\n"
                              "        int i;\n"
                              "    } else {\n"
                              "        int i;\n"
                              "    }\n"
                              "    i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:9]: (style) Variable 'i' is assigned a value that is never used.\n"
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
                              "}");
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
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar12() {
        // ticket #1574
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c, d, e, f;\n"
                              "    a = b = c = d = e = f = 0;\n"
                              "\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used.\n"
            "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used.\n"
            "[test.cpp:4]: (style) Variable 'c' is assigned a value that is never used.\n"
            "[test.cpp:4]: (style) Variable 'd' is assigned a value that is never used.\n"
            "[test.cpp:4]: (style) Variable 'e' is assigned a value that is never used.\n"
            "[test.cpp:4]: (style) Variable 'f' is assigned a value that is never used.\n",
            errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c = 0;\n"
                              "    a = b = c;\n"
                              "\n"
                              "}");

        TODO_ASSERT_EQUALS(
            "[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used.\n"
            "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used.\n"
            "[test.cpp:3]: (style) Variable 'c' is assigned a value that is never used.\n",

            "[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used.\n"
            "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used.\n",
            errout.str());
    }

    void localvar13() { // ticket #1640
        functionVariableUsage("void foo( OBJECT *obj )\n"
                              "{\n"
                              "    int x;\n"
                              "    x = obj->ySize / 8;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'x' is assigned a value that is never used.\n", errout.str());
    }

    void localvar14() {
        // ticket #5
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n", errout.str());
    }

    void localvar15() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    int b[a];\n"
                              "    b[0] = 0;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    int * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return *b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    const int * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct B * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    struct B * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("const struct B * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    const struct B * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar16() { // ticket #1709
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    char buf[5];\n"
                              "    char *ptr = buf;\n"
                              "    *(ptr++) = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'buf' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    char buf[5];\n"
                              "    char *ptr = buf - 1;\n"
                              "    *(++ptr) = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'buf' is not assigned a value.\n", errout.str());

        // #3910
        functionVariableUsage("int foo() {\n"
                              "    char buf[5];\n"
                              "    char *data[2];\n"
                              "    data[0] = buf;\n"
                              "    do_something(data);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo() {\n"
                              "    char buf1[5];\n"
                              "    char buf2[5];\n"
                              "    char *data[2];\n"
                              "    data[0] = buf1;\n"
                              "    data[1] = buf2;\n"
                              "    do_something(data);\n"
                              "}");
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
                              "}");
        ASSERT_EQUALS("[test.cpp:10]: (style) Variable 'line_start' is assigned a value that is never used.\n", errout.str());
    }

    void localvar18() { // ticket #1723
        functionVariableUsage("A::A(int iValue) {\n"
                              "    UserDefinedException* pe = new UserDefinedException();\n"
                              "    throw pe;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar19() { // ticket #1776
        functionVariableUsage("void foo() {\n"
                              "    int a[10];\n"
                              "    int c;\n"
                              "    c = *(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'a' is not assigned a value.\n"
                      "[test.cpp:4]: (style) Variable 'c' is assigned a value that is never used.\n", errout.str());
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
                              "}");
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
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());
    }

    void localvar24() { // ticket #1803
        functionVariableUsage("class MyException\n"
                              "{\n"
                              "    virtual void raise() const\n"
                              "    {\n"
                              "        throw *this;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar25() { // ticket #1729
        functionVariableUsage("int main() {\n"
                              "    int ppos = 1;\n"
                              "    int pneg = 0;\n"
                              "    const char*edge = ppos? \" +\" : pneg ? \" -\" : \"\";\n"
                              "    printf(\"This should be a '+' -> %s\n\", edge);\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar26() { // ticket #1894
        functionVariableUsage("int main() {\n"
                              "    const Fred &fred = getfred();\n"
                              "    int *p = fred.x();\n"
                              "    *p = 0;"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar27() { // ticket #2160
        functionVariableUsage("void f(struct s *ptr) {\n"
                              "    int param = 1;\n"
                              "    ptr->param = param++;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar28() { // ticket #2205
        functionVariableUsage("void f(char* buffer, int value) {\n"
                              "    char* pos = buffer;\n"
                              "    int size = value;\n"
                              "    *(int*)pos = size;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar29() { // ticket #2206
        functionVariableUsage("void f() {\n"
                              "    float s_ranges[] = { 0, 256 };\n"
                              "    float* ranges[] = { s_ranges };\n"
                              "    cout << ranges[0][0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar30() { // ticket #2264
        functionVariableUsage("void f() {\n"
                              "    Engine *engine = e;\n"
                              "    x->engine = engine->clone();\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar31() { // ticket #2286
        functionVariableUsage("void f() {\n"
                              "    int x = 0;\n"
                              "    a.x = x - b;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar32() {
        // ticket #2330 - fstream >> x
        functionVariableUsage("void f() {\n"
                              "    int x;\n"
                              "    fstream &f = getfile();\n"
                              "    f >> x;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        // ticket #4596 - if (c >>= x) {}
        functionVariableUsage("void f() {\n"
                              "    int x;\n"
                              "    C c;\n" // possibly some stream class
                              "    if (c >>= x) {}\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void f() {\n"
                              "    int x;\n"
                              "    C c;\n"
                              "    if (c >>= x) {}\n"
                              "}", "test.c");
        TODO_ASSERT_EQUALS("[test.c:2]: (style) Variable 'x' is not assigned a value.\n",
                           "[test.c:2]: (style) Variable 'x' is not assigned a value.\n"
                           "[test.c:3]: (style) Variable 'c' is not assigned a value.\n", errout.str());

        functionVariableUsage("void f(int c) {\n"
                              "    int x;\n"
                              "    if (c >> x) {}\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'x' is not assigned a value.\n", errout.str());

        functionVariableUsage("void f() {\n"
                              "    int x, y;\n"
                              "    std::cin >> x >> y;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void f() {\n"
                              "    int x, y;\n"
                              "    std::cin >> (x >> y);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'x' is not assigned a value.\n"
                      "[test.cpp:2]: (style) Variable 'y' is not assigned a value.\n", errout.str());
    }

    void localvar33() { // ticket #2345
        functionVariableUsage("void f() {\n"
                              "    Abc* abc = getabc();\n"
                              "    while (0 != (abc = abc->next())) {\n"
                              "        ++nOldNum;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar34() { // ticket #2368
        functionVariableUsage("void f() {\n"
                              "    int i = 0;\n"
                              "    if (false) {\n"
                              "    } else {\n"
                              "        j -= i;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar35() { // ticket #2535
        functionVariableUsage("void f() {\n"
                              "    int a, b;\n"
                              "    x(1,a,b);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar36() { // ticket #2805
        functionVariableUsage("int f() {\n"
                              "    int a, b;\n"
                              "    a = 2 * (b = 3);\n"
                              "    return a + b;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int f() {\n" // ticket #4318
                              "    int a,b;\n"
                              "    x(a, b=2);\n"  // <- if param2 is passed-by-reference then b might be used in x
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar37() { // ticket #3078
        functionVariableUsage("void f() {\n"
                              "    int a = 2;\n"
                              "    ints.at(a) = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar38() {
        functionVariableUsage("std::string f() {\n"
                              "    const char code[] = \"foo\";\n"
                              "    const std::string s1(sizeof_(code));\n"
                              "    const std::string s2 = sizeof_(code);\n"
                              "    return(s1+s2);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar39() {
        functionVariableUsage("void f() {\n"
                              "    int a = 1;\n"
                              "    foo(x*a);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar40() {
        functionVariableUsage("int f() {\n"
                              "    int a = 1;\n"
                              "    return x & a;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar41() {
        // #3603 - false positive 'x is assigned a value that is never used'
        functionVariableUsage("int f() {\n"
                              "    int x = 1;\n"
                              "    int y = FOO::VALUE * x;\n"
                              "    return y;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar42() { // #3742
        functionVariableUsage("float g_float = 1;\n"
                              "extern void SomeTestFunc(float);\n"
                              "void MyFuncError()\n"
                              "{\n"
                              "    const float floatA = 2.2f;\n"
                              "    const float floatTot = g_float * floatA;\n"
                              "    SomeTestFunc(floatTot);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("float g_float = 1;\n"
                              "extern void SomeTestFunc(float);\n"
                              "void MyFuncNoError()\n"
                              "{\n"
                              "    const float floatB = 2.2f;\n"
                              "    const float floatTot = floatB * g_float;\n"
                              "    SomeTestFunc(floatTot);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("float g_float = 1;\n"
                              "extern void SomeTestFunc(float);\n"
                              "void MyFuncNoError2()\n"
                              "{\n"
                              "    const float floatC = 2.2f;\n"
                              "    float floatTot = g_float * floatC;\n"
                              "    SomeTestFunc(floatTot);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar43() { // ticket #3602 (false positive)
        functionVariableUsage("void bar()\n"
                              "{\n"
                              "    int * piArray = NULL;\n"
                              "    unsigned int uiArrayLength = 2048;\n"
                              "    unsigned int uiIndex;\n"
                              "\n"
                              "    try\n"
                              "    {\n"
                              "        piArray = new int[uiArrayLength];\n" // Allocate memory
                              "    }\n"
                              "    catch (...)\n"
                              "    {\n"
                              "        SOME_MACRO\n"
                              "        delete [] piArray;\n"
                              "        return;\n"
                              "    }\n"
                              "    for (uiIndex = 0; uiIndex < uiArrayLength; uiIndex++)\n"
                              "    {\n"
                              "        piArray[uiIndex] = -1234;\n"
                              "    }\n"
                              "    delete [] piArray;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar44() { // #4020 - FP
        functionVariableUsage("void func() {\n"
                              "    int *sp_mem[2] = { 0x00, 0x00 };\n"
                              "    int src = 1, dst = 2;\n"
                              "    sp_mem[(dst + i)][3] = src;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar45() { // #4899 - FP
        functionVariableUsage("int func() {\n"
                              "    int a = 123;\n"
                              "    int b = (short)-a;;\n"
                              "    return b;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar46() { // #5491/#5494/#6301
        functionVariableUsage("int func() {\n"
                              "    int i = 0;\n"
                              "    int j{i};\n"
                              "    return j;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int func() {\n"
                              "    std::mutex m;\n"
                              "    std::unique_lock<std::mutex> l{ m };\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int func() {\n"
                              "    std::shared_lock<std::shared_timed_mutex> lock( m );\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvar47() { // #6603
        functionVariableUsage("void f() {\n"
                              "    int (SfxUndoManager::*retrieveCount)(bool) const\n"
                              "        = (flag) ? &SfxUndoManager::foo : &SfxUndoManager::bar;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'retrieveCount' is assigned a value that is never used.\n", errout.str());
    }

    void localvaralias1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)&a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)(&a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)&a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)(&a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = static_cast<char *>(&a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = static_cast<const char *>(&a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)(a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)(a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = static_cast<char *>(a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = static_cast<const char *>(a);\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c;\n"
                              "    *c = b[0];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'c' is not assigned a value.\n", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int c = b[0];\n"
                              "    x(c);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int c = b[0];\n"
                              "    x(c);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a[0];\n"
                              "    a[0] = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = &a[0];\n"
                              "    a[0] = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    a[0] = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo(int a[10])\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a[10];\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    int *d = b;\n"
                              "    *d = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:7]: (style) Variable 'b' is assigned a value that is never used.\n"
                      "[test.cpp:5]: (style) Variable 'c' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    c = b;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n"
                      "[test.cpp:7]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    c = b;\n"
                              "    *c = 0;\n"
                              "    c = a;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:9]: (style) Variable 'a' is assigned a value that is never used.\n"
                      "[test.cpp:7]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    b[-10] = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    b[-10] = 0;\n"
                              "    int * c = b - 10;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used.\n"
                      "[test.cpp:5]: (style) Variable 'c' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    x = c[0];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is not assigned a value.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    c[1] = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    c[1] = c[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    int d = c[0];\n"
                              "    f(d);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is not assigned a value.\n", errout.str());

        functionVariableUsage("void foo() {\n" // #4022 - FP (a is assigned a value that is never used)
                              "    int a[2], *b[2];\n"
                              "    a[0] = 123;\n"
                              "    b[0] = &a[0];\n"
                              "    int *d = b[0];\n"
                              "    return *d;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo() {\n" // #4022 - FP (a is assigned a value that is never used)
                              "    entry a[2], *b[2];\n"
                              "    a[0].value = 123;\n"
                              "    b[0] = &a[0];\n"
                              "    int d = b[0].value;\n"
                              "    return d;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = (struct S *)a;\n"
                              "    s->c[0] = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = (struct S *)a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    const struct S * s = (const struct S *)a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = static_cast<struct S *>(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    const struct S * s = static_cast<const struct S *>(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n"
                      "[test.cpp:5]: (style) Variable 's' is assigned a value that is never used.\n", errout.str());

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
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: b\n"
                      "[test.cpp:10]: (style) Variable 'c' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    int c[10];\n"
                              "    int *d;\n"
                              "    d = b; *d = 0;\n"
                              "    d = a; *d = 0;\n"
                              "    d = c; *d = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'b' is assigned a value that is never used.\n"
                      "[test.cpp:9]: (style) Variable 'c' is assigned a value that is never used.\n", errout.str());
    }

    void localvaralias2() { // ticket 1637
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * a;\n"
                              "    x(a);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias3() { // ticket 1639
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    BROWSEINFO    info;\n"
                              "    char          szDisplayName[MAX_PATH];\n"
                              "    info.pszDisplayName = szDisplayName;\n"
                              "    SHBrowseForFolder(&info);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias4() { // ticket 1643
        functionVariableUsage("struct AB { int a; int b; } ab;\n"
                              "void foo()\n"
                              "{\n"
                              "    int * a = &ab.a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("struct AB { int a; int b; } ab;\n"
                              "void foo()\n"
                              "{\n"
                              "    int * a = &ab.a;\n"
                              "    *a = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct AB { int a; int b; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct AB ab;\n"
                              "    int * a = &ab.a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ab' is not assigned a value.\n"
                      "[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("struct AB { int a; int b; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct AB ab;\n"
                              "    int * a = &ab.a;\n"
                              "    *a = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias5() { // ticket 1647
        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[0];\n"
                              "    *p++ = 0;\n"
                              "    return buf[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[1];\n"
                              "    *p-- = 0;\n"
                              "    return buf[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[0];\n"
                              "    *++p = 0;\n"
                              "    return buf[0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[1];\n"
                              "    *--p = 0;\n"
                              "    return buf[0];\n"
                              "}");
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
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'buf' is assigned a value that is never used.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'buf' is assigned a value that is never used.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'buf' is assigned a value that is never used.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'buf' is assigned a value that is never used.\n", errout.str());

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

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Foo foo;\n"
                              "    Foo &ref = foo;\n"
                              "    ref[0] = 123;\n"
                              "}",
                              "test.c");
        ASSERT_EQUALS("[test.c:5]: (style) Variable 'foo' is assigned a value that is never used.\n", errout.str());
    }

    void localvaralias10() { // ticket 2004
        functionVariableUsage("void foo(Foo &foo)\n"
                              "{\n"
                              "    Foo &ref = foo;\n"
                              "    int *x = &ref.x();\n"
                              "    *x = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo(Foo &foo)\n"
                              "{\n"
                              "    Foo &ref = foo;\n"
                              "    int *x = &ref.x;\n"
                              "    *x = 0;\n"
                              "}",
                              "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias11() { // #4423 - iterator
        functionVariableUsage("void f(Foo &foo) {\n"
                              "    std::set<int>::iterator x = foo.dostuff();\n"
                              "    *(x) = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias12() { // #4394
        functionVariableUsage("void f(void) {\n"
                              "    int a[4];\n"
                              "    int *b = (int*)((int*)a+1);\n"
                              "    x(b);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int f(void) {\n" // #4628
                              "    int x=1,y;\n"
                              "    y = (x * a) / 100;\n"
                              "    return y;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvaralias13() { // #4487
        functionVariableUsage("void f(char *p) {\n"
                              "    char a[4];\n"
                              "    p = a;\n"
                              "    strcpy(p, \"x\");\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void f(char *p) {\n"
                              "    char a[4];\n"
                              "    p = a;\n"
                              "    strcpy(p, \"x\");\n"
                              "}");
        TODO_ASSERT_EQUALS("a is assigned value that is never used", "", errout.str());
    }

    void localvarasm() {
        functionVariableUsage("void foo(int &b)\n"
                              "{\n"
                              "    int a;\n"
                              "    asm();\n"
                              "    b = a;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarStruct1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static const struct{ int x, y, w, h; } bounds = {1,2,3,4};\n"
                              "    return bounds.x + bounds.y + bounds.w + bounds.h;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarStruct2() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct ABC { int a, b, c; };\n"
                              "    struct ABC abc = { 1, 2, 3 };\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'abc' is assigned a value that is never used.\n", errout.str());
    }

    void localvarStruct3() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 10;\n"
                              "    union { struct { unsigned char x; }; unsigned char z; };\n"
                              "    do {\n"
                              "        func();\n"
                              "    } while(a--);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: x\n"
                           "[test.cpp:4]: (style) Unused variable: z\n", "", errout.str());
    }

    void localvarStruct5() {
        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n",
                              "test.c");
        ASSERT_EQUALS("[test.c:2]: (style) Unused variable: a\n", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    a.i = 0;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    a.i = 0;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a = { 0 };\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a = { 0 };\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("class A { int i; public: A(); { } };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n", errout.str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused variable: a\n", errout.str());

        functionVariableUsage("class A { int i; public: A(); { } };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A { unknown i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class A : public Fred { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class Fred {char c;};\n"
                              "class A : public Fred { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: a\n", errout.str());
    }

    void localvarStruct6() {
        functionVariableUsage("class Type { };\n"
                              "class A {\n"
                              "public:\n"
                              "    Type & get() { return t; }\n"
                              "private:\n"
                              "    Type t;\n"
                              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarStructArray() {
        // #3633 - detect that struct array is assigned a value
        functionVariableUsage("void f() {\n"
                              "    struct X x[10];\n"
                              "    x[0].a = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'x' is assigned a value that is never used.\n", errout.str());
    }

    void localvarOp() {
        const char op[] = "+-*/%&|^";
        for (const char *p = op; *p; ++p) {
            std::string code("int main()\n"
                             "{\n"
                             "    int tmp = 10;\n"
                             "    return 123 " + std::string(1, *p) + " tmp;\n"
                             "}");
            functionVariableUsage(code.c_str());
            ASSERT_EQUALS("", errout.str());
        }
    }

    void localvarInvert() {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    return ~tmp;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarIf() {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    if ( tmp )\n"
                              "        return 1;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarIfElse() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int tmp1 = 1;\n"
                              "    int tmp2 = 2;\n"
                              "    int tmp3 = 3;\n"
                              "    return tmp1 ? tmp2 : tmp3;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarOpAssign() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = 2;\n"
                              "    a |= b;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'a' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    int a = 1;\n"
                              "    (b).x += a;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarFor() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    for (;a;);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    for (int i = 0; (pci = cdi_list_get(pciDevices, i)); i++)\n"
                              "    {}\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarForEach() { // #4155 - foreach
        functionVariableUsage("void foo() {\n"
                              "    int i = -1;\n"
                              "    int a[] = {1,2,3};\n"
                              "    FOREACH_X (int x, a) {\n"
                              "        if (i==x) return x;\n"
                              "        i = x;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    int i = -1;\n"
                              "    int a[] = {1,2,3};\n"
                              "    FOREACH_X (int x, a) {\n"
                              "        i = x;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    int i = -1;\n"
                              "    int a[] = {1,2,3};\n"
                              "    X (int x, a) {\n"
                              "        if (i==x) return x;\n"
                              "        i = x;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        // #4956 - assignment in for_each
        functionVariableUsage("void f(std::vector<int> ints) {\n"
                              "  int x = 0;\n"
                              "  std::for_each(ints.begin(), ints.end(), [&x](int i){ dostuff(x); x = i; });\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void f(std::vector<int> ints) {\n"
                              "  int x = 0;\n"
                              "  std::for_each(ints.begin(), ints.end(), [&x](int i){ x += i; });\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'x' is assigned a value that is never used.\n", errout.str());

        // #5154 - MSVC 'for each'
        functionVariableUsage("void f() {\n"
                              "  std::map<int,int> ints;\n"
                              "  ints[0]= 1;\n"
                              "  for each(std::pair<int,int> i in ints) { x += i.first; }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarShift1() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int var = 1;\n"
                              "    return 1 >> var;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarShift2() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int var = 1;\n"
                              "    while (var = var >> 1) { }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarShift3() {  // #3509
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    QList<int *> ints;\n"
                              "    ints << 1;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo() {\n" // #4320
                              "    int x;\n"
                              "    x << 1;\n"
                              "    return x;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarCast() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = static_cast<int>(a);\n"
                              "    return b;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarClass() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    class B : public A {\n"
                              "        int a;\n"
                              "        int f() { return a; }\n"
                              "    } b;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarUnused() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((unused));\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((unused)) = true;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((unused)) test;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((unused)) test = true;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((used));\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((used)) test;\n"
                              "}");
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
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i(0);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int j = 0;\n"
                              "    static int i(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'i' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("int * foo(int x)\n"
                              "{\n"
                              "    static int a[] = { 3, 4, 5, 6 };\n"
                              "    static int b[] = { 4, 5, 6, 7 };\n"
                              "    static int c[] = { 5, 6, 7, 8 };\n"
                              "    b[1] = 1;\n"
                              "    return x ? a : c;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i = 0;\n"
                              "    if(i < foo())\n"
                              "        i += 5;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarextern() {
        functionVariableUsage("void foo() {\n"
                              "    extern int i;\n"
                              "    i = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvardynamic1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = malloc(16);\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = g_malloc(16);\n"
                              "    g_free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = kmalloc(16, GFP_KERNEL);\n"
                              "    kfree(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = vmalloc(16, GFP_KERNEL);\n"
                              "    vfree(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());


        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char[16];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new ( nothrow ) char[16];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new ( std::nothrow ) char[16];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char;\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = malloc(16);\n"
                              "    ptr[0] = 123;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char[16];\n"
                              "    ptr[0] = 123;\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int a; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'fred' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("struct Fred { int a; Fred() : a(0) {} };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Fred* fred = malloc(sizeof(Fred));\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    free(fred);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 'fred' is allocated memory that is never used.\n", errout.str());


        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = names[i];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvardynamic2() {
        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new (nothrow ) Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new (std::nothrow) Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = new Fred();\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'ptr' is allocated memory that is never used.\n", errout.str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvardynamic3() {
        // Ticket #3477 - False positive that 'data' is not assigned a value
        functionVariableUsage("void foo() {\n"
                              "    int* data = new int[100];\n"
                              "    int* p = data;\n"
                              "    for ( int i = 0; i < 10; ++i )\n"
                              "        p++;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvararray1() {
        functionVariableUsage("void foo() {\n"
                              "    int p[5];\n"
                              "    *p = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvararray2() {
        functionVariableUsage("int foo() {\n"
                              "    int p[5][5];\n"
                              "    p[0][0] = 0;\n"
                              "    return p[0][0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvararray3() {
        functionVariableUsage("int foo() {\n"
                              "    int p[5][5];\n"
                              "    *((int*)p[0]) = 0;\n"
                              "    return p[0][0];\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarstring1() { // ticket #1597
        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unused variable: s\n", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "    s = \"foo\";\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Variable 's' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void foo() {\n"
                              "    std::string s = \"foo\";\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 's' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("std::string foo() {\n"
                              "    std::string s;\n" // Class instances are initialized. Assignment is not necessary
                              "    return s;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("std::string foo() {\n"
                              "    std::string s = \"foo\";\n"
                              "    return s;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarstring2() { // ticket #2929
        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "    int i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unused variable: s\n"
                      "[test.cpp:3]: (style) Unused variable: i\n", errout.str());
    }

    void localvarconst1() {
        functionVariableUsage("void foo() {\n"
                              "    const bool b = true;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'b' is assigned a value that is never used.\n", errout.str());
    }

    void localvarconst2() {
        functionVariableUsage("void foo() {\n"
                              "    const int N = 10;\n"
                              "    struct X { int x[N]; };\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarthrow() { // ticket #3687
        functionVariableUsage("void foo() {\n"
                              "    try {}"
                              "    catch(Foo& bar) {}\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localVarStd() {
        functionVariableUsage("void f() {\n"
                              "    std::string x = foo();\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'x' is assigned a value that is never used.\n", errout.str());

        functionVariableUsage("void f() {\n"
                              "    std::vector<int> x;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unused variable: x\n", errout.str());

        functionVariableUsage("void f() {\n"
                              "    std::vector<int> x(100);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'x' is assigned a value that is never used.\n", "", errout.str());

        functionVariableUsage("void f() {\n"
                              "    std::vector<MyClass> x;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Unused variable: x\n", errout.str());

        functionVariableUsage("void f() {\n"
                              "    std::vector<MyClass> x(100);\n" // Might have a side-effect
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void f() {\n"
                              "    std::lock_guard<MyClass> lock(mutex_);\n" // Has a side-effect #4385
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void f() {\n"
                              "    pLocker = std::shared_ptr<jfxLocker>(new jfxLocker(m_lock, true));\n" // Could have side-effects (#4355)
                              "}");
        ASSERT_EQUALS("", errout.str());

        functionVariableUsage("void f() {\n"
                              "    std::mutex m;\n"
                              "    std::unique_lock<std::mutex> lock(m);\n" // #4624
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // ticket #3104 - false positive when variable is read with "if (NOT var)"
    void localvarIfNOT() {
        functionVariableUsage("void f() {\n"
                              "    bool x = foo();\n"
                              "    if (NOT x) { }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarAnd() { // #3672
        functionVariableUsage("int main() {\n"
                              "    unsigned flag = 0x1 << i;\n"
                              "    if (m_errorflags & flag) {\n"
                              "        return 1;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarSwitch() { // #3744 - false positive when used in switch
        functionVariableUsage("const char *f(int x) {\n"
                              "    const char a[] = \"abc\";\n"
                              "    const char def[] = \"def\";\n"
                              "    const char *ptr;\n"
                              "    switch(x) {\n"
                              "        case 1:  ptr=a; break;\n"
                              "        default: ptr=def; break;\n"
                              "    }\n"
                              "    return ptr;\n"
                              "}");

        // Don't write an error that "a" is not used
        ASSERT_EQUALS("", errout.str());
    }

    void localvarNULL() { // #4203 - Setting NULL value is not redundant - it is safe
        functionVariableUsage("void f() {\n"
                              "    char *p = malloc(100);\n"
                              "    foo(p);\n"
                              "    free(p);\n"
                              "    p = NULL;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarUnusedGoto() {
        // #4447
        functionVariableUsage("bool f(const int &i) {\n"
                              " int X = i;\n"
                              "label:\n"
                              " if ( X == 0 ) {\n"
                              "    X -= 101;\n"
                              "    return true;\n"
                              " }\n"
                              " if ( X < 1001 )  {\n"
                              "    X += 1;\n"
                              "    goto label;\n"
                              " }\n"
                              " return false;\n"
                              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #4558
        functionVariableUsage("int f() {\n"
                              " int i,j=0;\n"
                              " start:\n"
                              " i=j;\n"
                              " i++;\n"
                              " j=i;\n"
                              " if (i<3)\n"
                              "     goto start;\n"
                              " return i;\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void localvarCpp11Initialization() {
        // #6160
        functionVariableUsage("void foo() {\n"
                              "    int myNewValue{ 3u };\n"
                              "    myManager.theDummyTable.addRow(UnsignedIndexValue{ myNewValue }, DummyRowData{ false });\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void chainedAssignment() {
        // #5466
        functionVariableUsage("void NotUsed(double* pdD, int n) {\n"
                              "    double sum = 0.0;\n"
                              "    for (int i = 0; i<n; ++i)\n"
                              "        pdD[i] = (sum += pdD[i]);\n"
                              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void crash1() {
        functionVariableUsage("SAL_WNODEPRECATED_DECLARATIONS_PUSH\n"
                              "void convertToTokenArray() {\n"
                              "}\n"
                              "SAL_WNODEPRECATED_DECLARATIONS_POP"); // #4033
    }

    void crash2() {
        functionVariableUsage("template<unsigned dim>\n"
                              "struct Y: Y<dim-1> { };\n"
                              "template<>\n"
                              "struct Y<0> {};\n"
                              "void f() {\n"
                              "    Y y;\n"
                              "}"); // #4695
    }

    void usingNamespace() {
        functionVariableUsage("int foo() {\n"
                              "   using namespace ::com::sun::star::i18n;\n"
                              "   bool b = false;\n"
                              "   int j = 0;\n"
                              "   for (int i = 0; i < 3; i++) {\n"
                              "          if (!b) {\n"
                              "             j = 3;\n"
                              "             b = true;\n"
                              "          }\n"
                              "   }\n"
                              "   return j;\n"
                              "}"); // #4585
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestUnusedVar)
