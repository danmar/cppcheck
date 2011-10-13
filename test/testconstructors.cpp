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



#include "tokenize.h"
#include "checkclass.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestConstructors : public TestFixture {
public:
    TestConstructors() : TestFixture("TestConstructors")
    { }

private:


    void check(const char code[], bool showAll = false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.inconclusive = showAll;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check class constructors..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.constructors();
    }

    void run() {
        TEST_CASE(simple1);
        TEST_CASE(simple2);
        TEST_CASE(simple3);
        TEST_CASE(simple4);
        TEST_CASE(simple5); // ticket #2560

        TEST_CASE(initvar_with_this);       // BUG 2190300
        TEST_CASE(initvar_if);              // BUG 2190290
        TEST_CASE(initvar_operator_eq1);     // BUG 2190376
        TEST_CASE(initvar_operator_eq2);     // BUG 2190376
        TEST_CASE(initvar_operator_eq3);
        TEST_CASE(initvar_operator_eq4);     // ticket #2204
        TEST_CASE(initvar_same_classname);      // BUG 2208157
        TEST_CASE(initvar_chained_assign);      // BUG 2270433
        TEST_CASE(initvar_2constructors);       // BUG 2270353
        TEST_CASE(initvar_constvar);
        TEST_CASE(initvar_staticvar);
        TEST_CASE(initvar_union);

        TEST_CASE(initvar_private_constructor);     // BUG 2354171 - private constructor
        TEST_CASE(initvar_copy_constructor); // ticket #1611
        TEST_CASE(initvar_nested_constructor); // ticket #1375
        TEST_CASE(initvar_nocopy1);            // ticket #2474
        TEST_CASE(initvar_nocopy2);            // ticket #2484

        TEST_CASE(initvar_destructor);      // No variables need to be initialized in a destructor

        TEST_CASE(operatorEqSTL);
    }


    void simple1() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) The class 'Fred' does not have a constructor.\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) The struct 'Fred' does not have a constructor.\n", errout.str());
    }


    void simple2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() : i(0) { }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { i = 0; }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred() : i(0) { }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred() { i = 0; }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred() { }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }


    void simple3() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred() :i(0)\n"
              "{ }\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ i = 0; }\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }\n");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred() :i(0)\n"
              "{ }\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ i = 0; }\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }


    void simple4() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred();\n"
              "    Fred(int _i);\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }\n"
              "Fred::Fred(int _i)\n"
              "{\n"
              "    i = _i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    Fred(int _i);\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }\n"
              "Fred::Fred(int _i)\n"
              "{\n"
              "    i = _i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }

    void simple5() { // ticket #2560
        check("namespace Nsp\n"
              "{\n"
              "    class B { };\n"
              "}\n"
              "class Altren : public Nsp::B\n"
              "{\n"
              "public:\n"
              "    Altren () : Nsp::B(), mValue(0)\n"
              "    {\n"
              "    }\n"
              "private:\n"
              "    int mValue;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_with_this() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred()\n"
              "    { this->i = 0; }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred()\n"
              "    { this->i = 0; }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_if() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        if (true)\n"
              "            i = 0;\n"
              "        else\n"
              "            i = 1;\n"
              "    }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        if (true)\n"
              "            i = 0;\n"
              "        else\n"
              "            i = 1;\n"
              "    }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_operator_eq1() {
        // Bug 2190376 - False positive, Uninitialized member variable with operator=

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "\n"
              "public:\n"
              "    Fred()\n"
              "    { i = 0; }\n"
              "\n"
              "    Fred(const Fred &fred)\n"
              "    { *this = fred; }\n"
              "\n"
              "    const Fred & operator=(const Fred &fred)\n"
              "    { i = fred.i; return *this; }\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "  public:\n"
              "  A() : i(0), j(0) {}\n"
              "\n"
              "  A &operator=(const int &value)\n"
              "  {\n"
              "    i = value;\n"
              "    return (*this);\n"
              "  }\n"
              "\n"
              "  int i;\n"
              "  int j;\n"
              "};\n"
              "\n"
              "int main() {}\n");

        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    int i;\n"
              "\n"
              "    Fred()\n"
              "    { i = 0; }\n"
              "\n"
              "    Fred(const Fred &fred)\n"
              "    { *this = fred; }\n"
              "\n"
              "    const Fred & operator=(const Fred &fred)\n"
              "    { i = fred.i; return *this; }\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());

        check("struct A\n"
              "{\n"
              "  A() : i(0), j(0) {}\n"
              "\n"
              "  A &operator=(const int &value)\n"
              "  {\n"
              "    i = value;\n"
              "    return (*this);\n"
              "  }\n"
              "\n"
              "  int i;\n"
              "  int j;\n"
              "};\n"
              "\n"
              "int main() {}\n");

        ASSERT_EQUALS("", errout.str());
    }


    void initvar_operator_eq2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { i = 0; }\n"
              "    void operator=(const Fred &fred) { }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred() { i = 0; }\n"
              "    void operator=(const Fred &fred) { }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='\n", errout.str());
    }

    void initvar_operator_eq3() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "    void operator=(const Fred &fred) { Init(); }\n"
              "private:\n"
              "    void Init() { i = 0; }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred() { Init(); }\n"
              "    void operator=(const Fred &fred) { Init(); }\n"
              "private:\n"
              "    void Init() { i = 0; }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_operator_eq4() {
        check("class Fred\n"
              "{\n"
              "    int i;\n"
              "public:\n"
              "    Fred() : i(5) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='\n", errout.str());

        check("class Fred\n"
              "{\n"
              "    int * i;\n"
              "public:\n"
              "    Fred() : i(NULL) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='\n", errout.str());

        check("class Fred\n"
              "{\n"
              "    const int * i;\n"
              "public:\n"
              "    Fred() : i(NULL) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='\n", errout.str());

        check("class Fred\n"
              "{\n"
              "    const int i;\n"
              "public:\n"
              "    Fred() : i(5) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_same_classname() {
        // Bug 2208157 - False positive: Uninitialized variable, same class name

        check("void func1()\n"
              "{\n"
              "    class Fred\n"
              "    {\n"
              "        int a;\n"
              "        Fred() { a = 0; }\n"
              "    };\n"
              "}\n"
              "\n"
              "void func2()\n"
              "{\n"
              "    class Fred\n"
              "    {\n"
              "        int b;\n"
              "        Fred() { b = 0; }\n"
              "    };\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("void func1()\n"
              "{\n"
              "    struct Fred\n"
              "    {\n"
              "        int a;\n"
              "        Fred() { a = 0; }\n"
              "    };\n"
              "}\n"
              "\n"
              "void func2()\n"
              "{\n"
              "    class Fred\n"
              "    {\n"
              "        int b;\n"
              "        Fred() { b = 0; }\n"
              "    };\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("void func1()\n"
              "{\n"
              "    struct Fred\n"
              "    {\n"
              "        int a;\n"
              "        Fred() { a = 0; }\n"
              "    };\n"
              "}\n"
              "\n"
              "void func2()\n"
              "{\n"
              "    struct Fred\n"
              "    {\n"
              "        int b;\n"
              "        Fred() { b = 0; }\n"
              "    };\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("class Foo {\n"
              "    void func1()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int a;\n"
              "            Fred() { a = 0; }\n"
              "        };\n"
              "    }\n"
              "\n"
              "    void func2()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int b;\n"
              "            Fred() { b = 0; }\n"
              "        };\n"
              "    }\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());

        check("class Foo {\n"
              "    void func1()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int a;\n"
              "            Fred() { }\n"
              "        };\n"
              "    }\n"
              "\n"
              "    void func2()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int b;\n"
              "            Fred() { }\n"
              "        };\n"
              "    }\n"
              "};\n");

        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::a' is not initialized in the constructor.\n"
                      "[test.cpp:16]: (warning) Member variable 'Fred::b' is not initialized in the constructor.\n", errout.str());
    }

    void initvar_chained_assign() {
        // Bug 2270433 - Uninitialized variable false positive on chained assigns

        check("class c\n"
              "{\n"
              "    c();\n"
              "\n"
              "    int m_iMyInt1;\n"
              "    int m_iMyInt2;\n"
              "}\n"
              "\n"
              "c::c()\n"
              "{\n"
              "    m_iMyInt1 = m_iMyInt2 = 0;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("struct c\n"
              "{\n"
              "    c();\n"
              "\n"
              "    int m_iMyInt1;\n"
              "    int m_iMyInt2;\n"
              "}\n"
              "\n"
              "c::c()\n"
              "{\n"
              "    m_iMyInt1 = m_iMyInt2 = 0;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
    }


    void initvar_2constructors() {
        check("class c\n"
              "{\n"
              "    c();\n"
              "    c(bool b);"
              "\n"
              "    void InitInt();\n"
              "\n"
              "    int m_iMyInt;\n"
              "    int m_bMyBool;\n"
              "}\n"
              "\n"
              "c::c()\n"
              "{\n"
              "    m_bMyBool = false;\n"
              "    InitInt();"
              "}\n"
              "\n"
              "c::c(bool b)\n"
              "{\n"
              "    m_bMyBool = b;\n"
              "    InitInt();\n"
              "}\n"
              "\n"
              "void c::InitInt()\n"
              "{\n"
              "    m_iMyInt = 0;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("struct c\n"
              "{\n"
              "    c();\n"
              "    c(bool b);"
              "\n"
              "    void InitInt();\n"
              "\n"
              "    int m_iMyInt;\n"
              "    int m_bMyBool;\n"
              "}\n"
              "\n"
              "c::c()\n"
              "{\n"
              "    m_bMyBool = false;\n"
              "    InitInt();"
              "}\n"
              "\n"
              "c::c(bool b)\n"
              "{\n"
              "    m_bMyBool = b;\n"
              "    InitInt();\n"
              "}\n"
              "\n"
              "void c::InitInt()\n"
              "{\n"
              "    m_iMyInt = 0;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
    }


    void initvar_constvar() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred() : s(NULL)\n"
              "{ }");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{ s = NULL; }");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::s' is not initialized in the constructor.\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred() : s(NULL)\n"
              "{ }");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{ s = NULL; }");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::s' is not initialized in the constructor.\n", errout.str());
    }


    void initvar_staticvar() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "    static void *p;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }


    void initvar_union() {
        check("class Fred\n"
              "{\n"
              "    union\n"
              "    {\n"
              "        int a;\n"
              "        char b[4];\n"
              "    } U;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        U.a = 0;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "    union\n"
              "    {\n"
              "        int a;\n"
              "        char b[4];\n"
              "    } U;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Member variable 'Fred::U' is not initialized in the constructor.\n", errout.str());
    }


    void initvar_private_constructor() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int var;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_copy_constructor() { // ticket #1611
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::string var;\n"
              "public:\n"
              "    Fred() { };\n"
              "    Fred(const Fred &) { };\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::var' is not initialized in the constructor.\n", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::string var;\n"
              "public:\n"
              "    Fred();\n"
              "    Fred(const Fred &);\n"
              "};\n"
              "Fred::Fred() { };\n"
              "Fred::Fred(const Fred &) { };\n");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Fred::var' is not initialized in the constructor.\n", errout.str());
    }

    void initvar_nested_constructor() { // ticket #1375
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        B(int x);\n"
              "        struct C {\n"
              "            C(int y);\n"
              "            struct D {\n"
              "                int d;\n"
              "                D(int z);\n"
              "            };\n"
              "            int c;\n"
              "        };\n"
              "        int b;\n"
              "    };\n"
              "private:\n"
              "    int a;\n"
              "    B b;\n"
              "};\n"
              "A::A(){}\n"
              "A::B::B(int x){}\n"
              "A::B::C::C(int y){}\n"
              "A::B::C::D::D(int z){}\n");
        ASSERT_EQUALS("[test.cpp:20]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:20]: (warning) Member variable 'A::b' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'B::b' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'C::c' is not initialized in the constructor.\n"
                      "[test.cpp:23]: (warning) Member variable 'D::d' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        B(int x);\n"
              "        struct C {\n"
              "            C(int y);\n"
              "            struct D {\n"
              "                D(const D &);\n"
              "                int d;\n"
              "            };\n"
              "            int c;\n"
              "        };\n"
              "        int b;\n"
              "    };\n"
              "private:\n"
              "    int a;\n"
              "    B b;\n"
              "};\n"
              "A::A(){}\n"
              "A::B::B(int x){}\n"
              "A::B::C::C(int y){}\n"
              "A::B::C::D::D(const A::B::C::D & d){}\n");
        ASSERT_EQUALS("[test.cpp:20]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:20]: (warning) Member variable 'A::b' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'B::b' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'C::c' is not initialized in the constructor.\n"
                      "[test.cpp:23]: (warning) Member variable 'D::d' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        B(int x);\n"
              "        struct C {\n"
              "            C(int y);\n"
              "            struct D {\n"
              "                struct E { int e; };\n"
              "                struct E d;\n"
              "                D(const E &);\n"
              "            };\n"
              "            int c;\n"
              "        };\n"
              "        int b;\n"
              "    };\n"
              "private:\n"
              "    int a;\n"
              "    B b;\n"
              "};\n"
              "A::A(){}\n"
              "A::B::B(int x){}\n"
              "A::B::C::C(int y){}\n"
              "A::B::C::D::D(const A::B::C::D::E & e){}\n");
        ASSERT_EQUALS("[test.cpp:21]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'A::b' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'B::b' is not initialized in the constructor.\n"
                      "[test.cpp:23]: (warning) Member variable 'C::c' is not initialized in the constructor.\n"
                      "[test.cpp:24]: (warning) Member variable 'D::d' is not initialized in the constructor.\n", errout.str());
    }

    void initvar_nocopy1() { // ticket #2474
        check("class B\n"
              "{\n"
              "    B (const B & Var);\n"
              "    B & operator= (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B (const B & Var);\n"
              "    B & operator= (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:12]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:13]: (warning) Member variable 'A::m_SemVar' is not assigned a value in 'A::operator='\n", errout.str());

        check("class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:7]: (warning) Member variable 'A::m_SemVar' is not assigned a value in 'A::operator='\n", errout.str());
    }

    void initvar_nocopy2() { // ticket #2484
        check("class B\n"
              "{\n"
              "    B (B & Var);\n"
              "    B & operator= (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B (B & Var);\n"
              "    B & operator= (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:12]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:13]: (warning) Member variable 'A::m_SemVar' is not assigned a value in 'A::operator='\n", errout.str());
    }

    void initvar_destructor() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int var;\n"
              "public:\n"
              "    Fred() : var(0) {}\n"
              "    ~Fred() {}\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqSTL() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::vector<int> ints;\n"
              "public:\n"
              "    Fred();\n"
              "    void operator=(const Fred &f);\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "void Fred::operator=(const Fred &f)\n"
              "{ }", true);
        ASSERT_EQUALS("[test.cpp:13]: (warning) Member variable 'Fred::ints' is not assigned a value in 'Fred::operator='\n", errout.str());
    }
};

REGISTER_TEST(TestConstructors)
