/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#include "checkmemoryleak.h"
#include "preprocessor.h"
#include "settings.h"
#include "simplecpp.h"
#include "standards.h"
#include "symboldatabase.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <list>
#include <ostream>
#include <string>
#include <vector>

struct InternalError;


class TestMemleak : private TestFixture {
public:
    TestMemleak() : TestFixture("TestMemleak") {
    }

private:
    Settings settings;

    void run() OVERRIDE {
        TEST_CASE(testFunctionReturnType);
        TEST_CASE(open);
    }

    CheckMemoryLeak::AllocType functionReturnType(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        const CheckMemoryLeak c(&tokenizer, this, &settings);

        return c.functionReturnType(&tokenizer.getSymbolDatabase()->scopeList.front().functionList.front());
    }

    void testFunctionReturnType() {
        {
            const char code[] = "const char *foo()\n"
                                "{ return 0; }";
            ASSERT_EQUALS(CheckMemoryLeak::No, functionReturnType(code));
        }

        {
            const char code[] = "Fred *newFred()\n"
                                "{ return new Fred; }";
            ASSERT_EQUALS(CheckMemoryLeak::New, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{ return new char[100]; }";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{\n"
                                "    char *p = new char[100];\n"
                                "    return p;\n"
                                "}";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }
    }

    void open() {
        const char code[] = "class A {\n"
                            "  static int open() {\n"
                            "    return 1;\n"
                            "  }\n"
                            "\n"
                            "  A() {\n"
                            "    int ret = open();\n"
                            "  }\n"
                            "};\n";

        // Clear the error buffer..
        errout.str("");

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // there is no allocation
        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "ret =");
        const CheckMemoryLeak check(&tokenizer, 0, &settings);
        ASSERT_EQUALS(CheckMemoryLeak::No, check.getAllocationType(tok->tokAt(2), 1));
    }
};

REGISTER_TEST(TestMemleak)





class TestMemleakInClass : public TestFixture {
public:
    TestMemleakInClass() : TestFixture("TestMemleakInClass") {
    }

private:
    Settings settings;

    /**
     * Tokenize and execute leak check for given code
     * @param code Source code
     */
    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakInClass checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.check();
    }

    void run() OVERRIDE {
        settings.addEnabled("warning");
        settings.addEnabled("style");

        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(class1);
        TEST_CASE(class2);
        TEST_CASE(class3);
        TEST_CASE(class4);
        TEST_CASE(class6);
        TEST_CASE(class7);
        TEST_CASE(class8);
        TEST_CASE(class9);
        TEST_CASE(class10);
        TEST_CASE(class11);
        TEST_CASE(class12);
        TEST_CASE(class13);
        TEST_CASE(class14);
        TEST_CASE(class15);
        TEST_CASE(class16);
        TEST_CASE(class17);
        TEST_CASE(class18);
        TEST_CASE(class19); // ticket #2219
        TEST_CASE(class20);
        TEST_CASE(class21); // ticket #2517
        TEST_CASE(class22); // ticket #3012
        TEST_CASE(class23); // ticket #3303
        TEST_CASE(class24); // ticket #3806 - false positive in copy constructor
        TEST_CASE(class25); // ticket #4367 - false positive implementation for destructor is not seen

        TEST_CASE(staticvar);

        TEST_CASE(free_member_in_sub_func);

        TEST_CASE(mismatch1);
        TEST_CASE(mismatch2); // #5659

        // allocating member variable in public function
        TEST_CASE(func1);
        TEST_CASE(func2);
    }


    void class1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "    char *str2;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "    char *str2;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "        str2 = new char[10];\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        delete [] str2;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());
    }

    void class2() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "}\n"
              "\n"
              "Fred::~Fred()\n"
              "{\n"
              "    free(str1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:17]: (error) Mismatching allocation and deallocation: Fred::str1\n", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        free(str1);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:12]: (error) Mismatching allocation and deallocation: Fred::str1\n", errout.str());
    }

    void class3() {
        check("class Token;\n"
              "\n"
              "class Tokenizer\n"
              "{\n"
              "private:\n"
              "    Token *_tokens;\n"
              "\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "    void deleteTokens(Token *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "    _tokens = new Token;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(Token *tok)\n"
              "{\n"
              "    while (tok)\n"
              "    {\n"
              "        Token *next = tok->next();\n"
              "        delete tok;\n"
              "        tok = next;\n"
              "    }\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("class Token;\n"
              "\n"
              "class Tokenizer\n"
              "{\n"
              "private:\n"
              "    Token *_tokens;\n"
              "\n"
              "public:\n"
              "    Tokenizer()\n"
              "    {\n"
              "        _tokens = new Token;\n"
              "    }\n"
              "    ~Tokenizer()\n"
              "    {\n"
              "        deleteTokens(_tokens);\n"
              "    }\n"
              "    void deleteTokens(Token *tok)\n"
              "    {\n"
              "        while (tok)\n"
              "        {\n"
              "            Token *next = tok->next();\n"
              "            delete tok;\n"
              "            tok = next;\n"
              "        }\n"
              "    }\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void class4() {
        check("struct ABC;\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    void addAbc(ABC *abc);\n"
              "public:\n"
              "    void click();\n"
              "};\n"
              "\n"
              "void Fred::addAbc(ABC* abc)\n"
              "{\n"
              "    AbcPosts->Add(abc);\n"
              "}\n"
              "\n"
              "void Fred::click()\n"
              "{\n"
              "    ABC *p = new ABC;\n"
              "    addAbc( p );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct ABC;\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    void addAbc(ABC* abc)\n"
              "    {\n"
              "        AbcPosts->Add(abc);\n"
              "    }\n"
              "public:\n"
              "    void click()\n"
              "    {\n"
              "        ABC *p = new ABC;\n"
              "        addAbc( p );\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class6() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    char *str = new char[100];\n"
              "    delete [] str;\n"
              "    hello();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo()\n"
              "    {\n"
              "        char *str = new char[100];\n"
              "        delete [] str;\n"
              "        hello();\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class7() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int *i;\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    this->i = new int;\n"
              "}\n"
              "Fred::~Fred()\n"
              "{\n"
              "    delete this->i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int *i;\n"
              "    Fred()\n"
              "    {\n"
              "        this->i = new int;\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        delete this->i;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class8() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    void a();\n"
              "    void doNothing() { }\n"
              "};\n"
              "\n"
              "void A::a()\n"
              "{\n"
              "    int* c = new int(1);\n"
              "    delete c;\n"
              "    doNothing(c);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    void a()\n"
              "    {\n"
              "        int* c = new int(1);\n"
              "        delete c;\n"
              "        doNothing(c);\n"
              "    }\n"
              "    void doNothing() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class9() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "    ~A();\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{ p = new int; }\n"
              "\n"
              "A::~A()\n"
              "{ delete (p); }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A()\n"
              "    { p = new int; }\n"
              "    ~A()\n"
              "    { delete (p); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class10() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "};\n"
              "A::A()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A() { p = new int; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class11() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A() : p(new int[10])\n"
              "    { }"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "};\n"
              "A::A() : p(new int[10])\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class12() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A();\n"
              "    void cleanup();"
              "};\n"
              "\n"
              "A::A()\n"
              "{ p = new int[10]; }\n"
              "\n"
              "A::~A()\n"
              "{ }\n"
              "\n"
              "void A::cleanup()\n"
              "{ delete [] p; }");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int[10]; }\n"
              "    ~A()\n"
              "    { }\n"
              "    void cleanup()\n"
              "    { delete [] p; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class13() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A();\n"
              "    void foo();"
              "};\n"
              "\n"
              "A::A()\n"
              "{ }\n"
              "\n"
              "A::~A()\n"
              "{ }\n"
              "\n"
              "void A::foo()\n"
              "{ p = new int[10]; delete [] p; }");
        ASSERT_EQUALS("[test.cpp:17]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n", errout.str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { }\n"
              "    ~A()\n"
              "    { }\n"
              "    void foo()\n"
              "    { p = new int[10]; delete [] p; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n", errout.str());
    }

    void class14() {
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = new int[10]; }");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = new int[10]; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = new int; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = malloc(sizeof(int)*10); }");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = malloc(sizeof(int)*10); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class15() {
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { delete [] p; }\n"
              "};\n"
              "A::A()\n"
              "{ p = new int[10]; }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int[10]; }\n"
              "    ~A() { delete [] p; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { delete p; }\n"
              "};\n"
              "A::A()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int; }\n"
              "    ~A() { delete p; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { free(p); }\n"
              "};\n"
              "A::A()\n"
              "{ p = malloc(sizeof(int)*10); }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = malloc(sizeof(int)*10); }\n"
              "    ~A() { free(p); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class16() {
        // Ticket #1510
        check("class A\n"
              "{\n"
              "    int *a;\n"
              "    int *b;\n"
              "public:\n"
              "    A() { a = b = new int[10]; }\n"
              "    ~A() { delete [] a; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class17() {
        // Ticket #1557
        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void A::foo()\n"
              "{\n"
              "    A::pd = new char[12];\n"
              "    delete [] A::pd;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated.\n", errout.str());

        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo()\n"
              "    {\n"
              "        pd = new char[12];\n"
              "        delete [] pd;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated.\n", errout.str());

        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void A::foo()\n"
              "{\n"
              "    pd = new char[12];\n"
              "    delete [] pd;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated.\n", errout.str());
    }

    void class18() {
        // Ticket #853
        check("class  A : public x\n"
              "{\n"
              "public:\n"
              "  A()\n"
              "  {\n"
              "    a = new char[10];\n"
              "    foo(a);\n"
              "  }\n"
              "private:\n"
              "  char *a;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class  A : public x\n"
              "{\n"
              "public:\n"
              "  A();\n"
              "private:\n"
              "  char *a;\n"
              "};\n"
              "A::A()\n"
              "{\n"
              "  a = new char[10];\n"
              "  foo(a);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void class19() {
        // Ticket #2219
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton(this);\n"
              "    rp2 = new TRadioButton(this);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class TRadioButton { };\n"
              "class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton;\n"
              "    rp2 = new TRadioButton;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Foo' is unsafe, 'Foo::rp1' can leak by wrong usage.\n"
                      "[test.cpp:6]: (style) Class 'Foo' is unsafe, 'Foo::rp2' can leak by wrong usage.\n", errout.str());

        check("class TRadioButton { };\n"
              "class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "    ~Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton;\n"
              "    rp2 = new TRadioButton;\n"
              "}\n"
              "Foo::~Foo()\n"
              "{\n"
              "    delete rp1;\n"
              "    delete rp2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void class20() {
        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred()\n"
              "        {\n"
              "            str1 = new char[10];\n"
              "            str2 = new char[10];\n"
              "        }\n"
              "        ~Fred()\n"
              "        {\n"
              "            delete [] str2;\n"
              "        }\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred();\n"
              "        ~Fred();\n"
              "    };\n"
              "\n"
              "    Fred::Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "        str2 = new char[10];\n"
              "    }\n"
              "\n"
              "    Fred::~Fred()\n"
              "    {\n"
              "        delete [] str2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred();\n"
              "        ~Fred();\n"
              "    };\n"
              "}\n"
              "ns1::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    namespace ns2 {\n"
              "        class Fred\n"
              "        {\n"
              "        private:\n"
              "            char *str1;\n"
              "            char *str2;\n"
              "        public:\n"
              "            Fred();\n"
              "            ~Fred();\n"
              "        };\n"
              "    }\n"
              "}\n"
              "ns1::ns2::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::ns2::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    namespace ns2 {\n"
              "        namespace ns3 {\n"
              "            class Fred\n"
              "            {\n"
              "            private:\n"
              "                char *str1;\n"
              "                char *str2;\n"
              "            public:\n"
              "                Fred();\n"
              "                ~Fred();\n"
              "            };\n"
              "        }\n"
              "    }\n"
              "}\n"
              "ns1::ns2::ns3::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::ns2::ns3::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());
    }

    void class21() { // ticket #2517
        check("struct B { };\n"
              "struct C\n"
              "{\n"
              "    B * b;\n"
              "    C(B * x) : b(x) { }\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B *b;\n"
              "    C *c;\n"
              "public:\n"
              "    A() : b(new B()), c(new C(b)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage.\n"
                      "[test.cpp:10]: (style) Class 'A' is unsafe, 'A::c' can leak by wrong usage.\n", errout.str());

        check("struct B { };\n"
              "struct C\n"
              "{\n"
              "    B * b;\n"
              "    C(B * x) : b(x) { }\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B *b;\n"
              "    C *c;\n"
              "public:\n"
              "    A()\n"
              "    {\n"
              "       b = new B();\n"
              "       c = new C(b);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage.\n"
                      "[test.cpp:10]: (style) Class 'A' is unsafe, 'A::c' can leak by wrong usage.\n", errout.str());
    }

    void class22() { // ticket #3012 - false positive
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "private:\n"
              "    Fred() { a = new int; }\n"
              "    ~Fred() { (delete(a), (a)=NULL); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class23() { // ticket #3303 - false positive
        check("class CDataImpl {\n"
              "public:\n"
              "    CDataImpl() { m_refcount = 1; }\n"
              "    void Release() { if (--m_refcount == 0) delete this; }\n"
              "private:\n"
              "    int m_refcount;\n"
              "};\n"
              "\n"
              "class CData {\n"
              "public:\n"
              "    CData() : m_impl(new CDataImpl()) { }\n"
              "    ~CData() { if (m_impl) m_impl->Release(); }\n"
              "private:\n"
              "    CDataImpl *m_impl;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class24() { // ticket #3806 - false positive in copy constructor
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "public:\n"
              "    Fred(const Fred &fred) { a = new int; }\n"
              "    ~Fred() { delete a; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class25() { // ticket #4367 - false positive when implementation for destructor is not seen
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "public:\n"
              "    Fred() { a = new int; }\n"
              "    ~Fred();\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void staticvar() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    static int * p;\n"
              "public:"
              "    A()\n"
              "    {\n"
              "        if (!p)\n"
              "            p = new int[100];\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void free_member_in_sub_func() {
        // Member function
        check("class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "    static void deleteTokens(int *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Global function
        check("void deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}\n"
              "class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch1() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    A(int i);\n"
              "    ~A();\n"
              "private:\n"
              "    char* pkt_buffer;\n"
              "};\n"
              "\n"
              "A::A(int i)\n"
              "{\n"
              "    pkt_buffer = new char[8192];\n"
              "    if (i != 1) {\n"
              "        delete pkt_buffer;\n"
              "        pkt_buffer = 0;\n"
              "    }\n"
              "}\n"
              "\n"
              "A::~A() {\n"
              "    delete [] pkt_buffer;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:14]: (error) Mismatching allocation and deallocation: A::pkt_buffer\n", errout.str());
    }

    void mismatch2() { // #5659
        check("namespace NS\n"
              "{\n"
              "class Foo\n"
              "{\n"
              "public:\n"
              "  void fct();\n"
              "\n"
              "private:\n"
              "  char* data_;\n"
              "};\n"
              "}\n"
              "\n"
              "using namespace NS;\n"
              "\n"
              "void Foo::fct()\n"
              "{\n"
              "  data_ = new char[42];\n"
              "  delete data_;\n"
              "  data_ = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:17]: (warning) Possible leak in public function. The pointer 'data_' is not deallocated before it is allocated.\n"
                      "[test.cpp:18]: (error) Mismatching allocation and deallocation: Foo::data_\n", errout.str());

        check("namespace NS\n"
              "{\n"
              "class Foo\n"
              "{\n"
              "public:\n"
              "  void fct(int i);\n"
              "\n"
              "private:\n"
              "  char* data_;\n"
              "};\n"
              "}\n"
              "\n"
              "using namespace NS;\n"
              "\n"
              "void Foo::fct(int i)\n"
              "{\n"
              "  data_ = new char[42];\n"
              "  delete data_;\n"
              "  data_ = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:17]: (warning) Possible leak in public function. The pointer 'data_' is not deallocated before it is allocated.\n"
                      "[test.cpp:18]: (error) Mismatching allocation and deallocation: Foo::data_\n", errout.str());
    }

    void func1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    void xy()\n"
              "    { s = malloc(100); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    void xy()\n"
              "    { s = malloc(100); }\n"
              "private:\n"
              "    char *s;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", errout.str());
    }

    void func2() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    const Fred & operator = (const Fred &f)\n"
              "    { s = malloc(100); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", errout.str());
    }
};

REGISTER_TEST(TestMemleakInClass)







class TestMemleakStructMember : public TestFixture {
public:
    TestMemleakStructMember() : TestFixture("TestMemleakStructMember") {
    }

private:
    Settings settings;

    void check(const char code[], bool isCPP = true) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, isCPP ? "test.cpp" : "test.c");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakStructMember checkMemoryLeakStructMember(&tokenizer, &settings, this);
        checkMemoryLeakStructMember.check();
    }

    void run() OVERRIDE {
        LOAD_LIB_2(settings.library, "std.cfg");
        LOAD_LIB_2(settings.library, "posix.cfg");

        // testing that errors are detected
        TEST_CASE(err);

        // handle / bail out when "goto" is found
        TEST_CASE(goto_);

        // Don't report errors if the struct is returned
        TEST_CASE(ret1);
        TEST_CASE(ret2);

        // assignments
        TEST_CASE(assign1);
        TEST_CASE(assign2);
        TEST_CASE(assign3);

        // Failed allocation
        TEST_CASE(failedAllocation);

        TEST_CASE(function1);   // Deallocating in function
        TEST_CASE(function2);   // #2848: Taking address in function
        TEST_CASE(function3);   // #3024: kernel list
        TEST_CASE(function4);   // #3038: Deallocating in function

        // Handle if-else
        TEST_CASE(ifelse);

        // Linked list
        TEST_CASE(linkedlist);

        // struct variable is a global variable
        TEST_CASE(globalvar);

        // local struct variable
        TEST_CASE(localvars);

        // struct variable is a reference variable
        TEST_CASE(refvar);

        // Segmentation fault in CheckMemoryLeakStructMember
        TEST_CASE(trac5030);

        TEST_CASE(varid); // #5201: Analysis confused by (variable).attribute notation
        TEST_CASE(varid_2); // #5315: Analysis confused by ((variable).attribute) notation

        TEST_CASE(customAllocation);
    }

    void err() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", errout.str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", errout.str());

        check("static ABC * foo()\n"
              "{\n"
              "    ABC *abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "    abc->b = malloc(10);\n"
              "    if (abc->b == 0)\n"
              "    {\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: abc.a\n", errout.str());

        check("static void foo(int a)\n"
              "{\n"
              "    ABC *abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (a == 1)\n"
              "    {\n"
              "        free(abc->a);\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: abc.a\n", errout.str());
    }

    void goto_() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (abc->a)\n"
              "    { goto out; }\n"
              "    free(abc);\n"
              "    return;\n"
              "out:\n"
              "    free(abc->a);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ret1() {
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void foo(struct ABC *abc)\n"
              "{\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7302
        check("void* foo() {\n"
              "    struct ABC abc;\n"
              "    abc.a = malloc(10);\n"
              "    return abc.a;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());

        check("void* foo() {\n"
              "    struct ABC abc;\n"
              "    abc.a = malloc(10);\n"
              "    return abc.b;\n"
              "}", false);
        ASSERT_EQUALS("[test.c:4]: (error) Memory leak: abc.a\n", errout.str());
    }

    void ret2() {
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return &abc->self;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign1() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = abc1;\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc;\n"
              "    abc1 = abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void foo()\n"
              "{\n"
              " struct msn_entry *ptr;\n"
              " ptr = malloc(sizeof(struct msn_entry));\n"
              " ptr->msn = malloc(100);\n"
              " back = ptr;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }

    void assign2() {
        check("static void foo() {\n"
              "    struct ABC *abc = malloc(123);\n"
              "    abc->a = abc->b = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign3() {
        check("void f(struct s *f1) {\n"
              "    struct s f2;\n"
              "    f2.a = malloc(100);\n"
              "    *f1 = f2;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void failedAllocation() {
        check("static struct ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (!abc->a)\n"
              "    {\n"
              "        free(abc);\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void function1() {
        // Not found function => assume that the function may deallocate
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    func(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abclist.push_back(abc);\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // #2848: Taking address in function 'assign'
    void function2() {
        check("void f() {\n"
              "  A a = { 0 };\n"
              "  a.foo = (char *) malloc(10);\n"
              "  assign(&a);\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    // #3024: kernel list
    void function3() {
        check("void f() {\n"
              "  struct ABC *abc = malloc(100);\n"
              "  abc.a = (char *) malloc(10);\n"
              "  list_add_tail(&abc->list, head);\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    // #3038: deallocating in function
    void function4() {
        check("void a(char *p) { char *x = p; free(x); }\n"
              "void b() {\n"
              "  struct ABC abc;\n"
              "  abc.a = (char *) malloc(10);\n"
              "  a(abc.a);\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    if (x)"
              "    {\n"
              "        abc->a = malloc(10);\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        free(abc);\n"
              "        return;\n"
              "    }\n"
              "    free(abc->a);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void linkedlist() {
        // #3904 - false positive when linked list is used
        check("static void foo() {\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->next = malloc(sizeof(struct ABC));\n"
              "    abc->next->next = NULL;\n"
              "\n"
              "    while (abc) {\n"
              "        struct ABC *next = abc->next;\n"
              "        free(abc);\n"
              "        abc = next;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void globalvar() {
        check("struct ABC *abc;\n"
              "\n"
              "static void foo()\n"
              "{\n"
              "    abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Ticket #933 Leaks with struct members not detected
    void localvars() {
        // Test error case
        const char code1[] = "struct A {\n"
                             "    FILE* f;\n"
                             "    char* c;\n"
                             "    void* m;\n"
                             "};\n"
                             "\n"
                             "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "    a.c = new char[12];\n"
                             "    a.m = malloc(12);\n"
                             "}";

        check(code1, true);
        ASSERT_EQUALS("[test.cpp:12]: (error) Memory leak: a.f\n"
                      "[test.cpp:12]: (error) Memory leak: a.c\n"
                      "[test.cpp:12]: (error) Memory leak: a.m\n", errout.str());
        check(code1, false);
        ASSERT_EQUALS("[test.c:12]: (error) Memory leak: a.f\n"
                      "[test.c:12]: (error) Memory leak: a.m\n", errout.str());

        // Test OK case
        const char code2[] = "struct A {\n"
                             "    FILE* f;\n"
                             "    char* c;\n"
                             "    void* m;\n"
                             "};\n"
                             "\n"
                             "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "    a.c = new char[12];\n"
                             "    a.m = malloc(12);\n"
                             "    fclose(a.f);\n"
                             "    delete [] a.c;\n"
                             "    free(a.m);\n"
                             "}";

        check(code2, true);
        ASSERT_EQUALS("", errout.str());
        check(code2, false);
        ASSERT_EQUALS("", errout.str());

        // Test unknown struct. In C++, it might have a destructor
        const char code3[] = "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "}";

        check(code3, true);
        ASSERT_EQUALS("", errout.str());
        check(code3, false);
        ASSERT_EQUALS("[test.c:4]: (error) Memory leak: a.f\n", errout.str());

        // Test struct with destructor
        const char code4[] = "struct A {\n"
                             "    FILE* f;\n"
                             "    ~A();\n"
                             "};\n"
                             "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "}";

        check(code4, true);
        ASSERT_EQUALS("", errout.str());
    }

    void refvar() { // #8116
        check("struct Test\n"
              "{\n"
              "  int* data;\n"
              "};\n"
              "\n"
              "void foo(Test* x)\n"
              "{\n"
              "  Test& y = *x;\n"
              "  y.data = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // don't crash
    void trac5030() {
        check("bool bob( char const **column_ptrs ) {\n"
              "unique_ptr<char[]>otherbuffer{new char[otherbufsize+1]};\n"
              "char *const oldbuffer = otherbuffer.get();\n"
              "int const oldbufsize = otherbufsize;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varid() { // #5201
        check("struct S {\n"
              "  void *state_check_buff;\n"
              "};\n"
              "void f() {\n"
              "  S s;\n"
              "  (s).state_check_buff = (void* )malloc(1);\n"
              "  if (s.state_check_buff == 0)\n"
              "    return;\n"
              "}", false);
        ASSERT_EQUALS("[test.c:9]: (error) Memory leak: s.state_check_buff\n", errout.str());
    }

    void varid_2() { // #5315
        check("typedef struct foo { char *realm; } foo;\n"
              "void build_principal() {\n"
              "  foo f;\n"
              "  ((f)->realm) = strdup(realm);\n"
              "  if(f->realm == NULL) {}\n"
              "}", false);
        ASSERT_EQUALS("[test.c:6]: (error) Memory leak: f.realm\n", errout.str());
    }

    void customAllocation() { // #4770
        check("char *myalloc(void) {\n"
              "    return malloc(100);\n"
              "}\n"
              "void func() {\n"
              "    struct ABC abc;\n"
              "    abc.a = myalloc();\n"
              "}", false);
        ASSERT_EQUALS("[test.c:7]: (error) Memory leak: abc.a\n", errout.str());
    }
};

REGISTER_TEST(TestMemleakStructMember)





class TestMemleakNoVar : public TestFixture {
public:
    TestMemleakNoVar() : TestFixture("TestMemleakNoVar") {
    }

private:
    Settings settings;

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakNoVar checkMemoryLeakNoVar(&tokenizer, &settings, this);
        checkMemoryLeakNoVar.check();
    }

    void run() OVERRIDE {
        settings.inconclusive = true;
        settings.standards.posix = true;
        settings.addEnabled("warning");

        LOAD_LIB_2(settings.library, "std.cfg");
        LOAD_LIB_2(settings.library, "posix.cfg");

        // pass allocated memory to function..
        TEST_CASE(functionParameter);

        // never use leakable resource
        TEST_CASE(missingAssignment);

        // pass allocated memory to function using a smart pointer
        TEST_CASE(smartPointerFunctionParam);
        TEST_CASE(resourceLeak);

        // Test getAllocationType for subfunction
        TEST_CASE(getAllocationType);
    }

    void functionParameter() {
        // standard function..
        check("void x() {\n"
              "    strcpy(a, strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Allocation with strdup, strcpy doesn't release it.\n", errout.str());

        check("char *x() {\n"
              "    char *ret = strcpy(malloc(10), \"abc\");\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void x() {\n"
              "    free(malloc(10));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // user function..
        check("void set_error(const char *msg) {\n"
              "}\n"
              "\n"
              "void x() {\n"
              "    set_error(strdup(p));\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Allocation with strdup, set_error doesn't release it.\n", "", errout.str());

        check("void f()\n"
              "{\n"
              "    int fd;\n"
              "    fd = mkstemp(strdup(\"/tmp/file.XXXXXXXX\"));\n"
              "    close(fd);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Allocation with strdup, mkstemp doesn't release it.\n", "", errout.str());

        check("void f()\n"
              "{\n"
              "    if(TRUE || strcmp(strdup(a), b));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with strdup, strcmp doesn't release it.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    if(!strcmp(strdup(a), b) == 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with strdup, strcmp doesn't release it.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    42, strcmp(strdup(a), b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with strdup, strcmp doesn't release it.\n", errout.str());
    }

    void missingAssignment() {
        check("void x()\n"
              "{\n"
              "    malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function 'malloc' is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    calloc(10, 1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function 'calloc' is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    strdup(\"Test\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function 'strdup' is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    (char*) malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function 'malloc' is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    char* ptr = malloc(10);\n"
              "    foo(ptr);\n"
              "    free(ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("char** x(const char* str) {\n"
              "    char* ptr[] = { malloc(10), malloc(5), strdup(str) };\n"
              "    return ptr;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void x()\n"
              "{\n"
              "    42,malloc(42);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function 'malloc' is not stored.\n", errout.str());

        check("void *f()\n"
              "{\n"
              "    return malloc(10);\n"
              "}\n"
              "void x()\n"
              "{\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Return value of allocation function 'f' is not stored.\n", errout.str());

        check("void f()\n" // #8100
              "{\n"
              "    auto lambda = [](){return malloc(10);};\n"
              "}\n"
              "void x()\n"
              "{\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void x()\n"
              "{\n"
              "    if(!malloc(5)) fail();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function 'malloc' is not stored.\n", errout.str());

        check("FOO* factory() {\n"
              "    FOO* foo = new (std::nothrow) FOO;\n"
              "    return foo;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #6536
        check("struct S { S(int) {} };\n"
              "void foo(int i) {\n"
              "  S socket(i);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #6693
        check("struct CTest {\n"
              "    void Initialise();\n"
              "    void malloc();\n"
              "};\n"
              "void CTest::Initialise() {\n"
              "    malloc();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n" // #7348 - cast
              "    p = (::X*)malloc(42);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7182 "crash: CheckMemoryLeak::functionReturnType()"
        check("template<typename... Ts> auto unary_right_comma (Ts... ts) { return (ts , ...); }\n"
              "template<typename T, typename... Ts> auto binary_left_comma (T x, Ts... ts) { return (x , ... , ts); }\n"
              "int main() {\n"
              "  unary_right_comma (a);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void smartPointerFunctionParam() {
        check("void x() {\n"
              "    f(shared_ptr<int>(new int(42)), g());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    h(12, f(shared_ptr<int>(new int(42)), g()));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(unique_ptr<int>(new int(42)), g());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_unique<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(g(), shared_ptr<int>(new int(42)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(g(), unique_ptr<int>(new int(42)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_unique<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(shared_ptr<char>(new char), make_unique<int>(32));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If make_unique<int>() throws, memory could be leaked. Use make_shared<char>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(g(124), h(\"test\", 234), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If h() throws, memory could be leaked. Use make_shared<char>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(shared_ptr<std::string>(new std::string(\"\")), g<std::string>());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g<std::string>() throws, memory could be leaked. Use make_shared<std::string>() instead.\n", errout.str());

        check("void g(int x) throw() { }\n"
              "void x() {\n"
              "    f(g(124), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void __declspec(nothrow) g(int x) { }\n"
              "void x() {\n"
              "    f(g(124), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
    void resourceLeak() {
        check("void foo() {\n"
              "  fopen(\"file.txt\", \"r\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Return value of allocation function 'fopen' is not stored.\n", errout.str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder h ( fopen(\"file.txt\", \"r\"));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder ( fopen(\"file.txt\", \"r\"));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder h { fopen(\"file.txt\", \"r\")};\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder h = fopen(\"file.txt\", \"r\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder { fopen(\"file.txt\", \"r\")};\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Holder {\n"
              "  Holder(int i, FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder { 0, fopen(\"file.txt\", \"r\")};\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void getAllocationType() {
        // #7845
        check("class Thing { Thing(); };\n"
              "Thing * makeThing() { Thing *thing = new Thing; return thing; }\n"
              "\n"
              "void f() {\n"
              "  makeThing();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};
REGISTER_TEST(TestMemleakNoVar)


