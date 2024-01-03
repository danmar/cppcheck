/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "checkunusedfunctions.h"
#include "errortypes.h"
#include "helpers.h"
#include "platform.h"
#include "preprocessor.h"
#include "settings.h"
#include "fixture.h"
#include "tokenize.h"

#include <sstream>
#include <string>

class TestUnusedFunctions : public TestFixture {
public:
    TestUnusedFunctions() : TestFixture("TestUnusedFunctions") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::style).build();

    void run() override {
        TEST_CASE(incondition);
        TEST_CASE(return1);
        TEST_CASE(return2);
        TEST_CASE(return3);
        TEST_CASE(callback1);
        TEST_CASE(callback2);
        TEST_CASE(else1);
        TEST_CASE(functionpointer);
        TEST_CASE(template1);
        TEST_CASE(template2);
        TEST_CASE(template3);
        TEST_CASE(template4); // #9805
        TEST_CASE(template5);
        TEST_CASE(template6); // #10475 crash
        TEST_CASE(template7); // #9766 crash
        TEST_CASE(template8);
        TEST_CASE(template9);
        TEST_CASE(template10);
        TEST_CASE(throwIsNotAFunction);
        TEST_CASE(unusedError);
        TEST_CASE(unusedMain);
        TEST_CASE(initializationIsNotAFunction);
        TEST_CASE(operator1);   // #3195
        TEST_CASE(operator2);   // #7974
        TEST_CASE(returnRef);
        TEST_CASE(attribute); // #3471 - FP __attribute__(constructor)
        TEST_CASE(initializer_list);
        TEST_CASE(member_function_ternary);
        TEST_CASE(boost);
        TEST_CASE(enumValues);

        TEST_CASE(multipleFiles);   // same function name in multiple files

        TEST_CASE(lineNumber); // Ticket 3059

        TEST_CASE(ignore_declaration); // ignore declaration

        TEST_CASE(operatorOverload);

        TEST_CASE(entrypointsWin);
        TEST_CASE(entrypointsWinU);
        TEST_CASE(entrypointsUnix);

        TEST_CASE(includes);
    }

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    void check_(const char* file, int line, const char code[], Platform::Type platform = Platform::Type::Native, const Settings *s = nullptr) {
        // Clear the error buffer..
        errout.str("");

        const Settings settings1 = settingsBuilder(s ? *s : settings).platform(platform).build();

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, "test.cpp"), file, line);

        // Check for unused functions..
        CheckUnusedFunctions checkUnusedFunctions(&tokenizer, &settings1, this);
        checkUnusedFunctions.parseTokens(tokenizer, "someFile.c", &settings1);
        // check() returns error if and only if errout is not empty.
        if ((checkUnusedFunctions.check)(this, settings1)) {
            ASSERT(!errout.str().empty());
        } else {
            ASSERT_EQUALS("", errout.str());
        }
    }

    void incondition() {
        check("int f1()\n"
              "{\n"
              "    if (f1())\n"
              "    { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void return1() {
        check("int f1()\n"
              "{\n"
              "    return f1();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void return2() {
        check("char * foo()\n"
              "{\n"
              "    return *foo();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void return3() {
        check("typedef void (*VoidFunc)();\n" // #9602
              "void sayHello() {\n"
              "  printf(\"Hello World\\n\");\n"
              "}\n"
              "VoidFunc getEventHandler() {\n"
              "  return sayHello;\n"
              "}\n"
              "void indirectHello() {\n"
              "  VoidFunc handler = getEventHandler();\n"
              "  handler();\n"
              "}\n"
              "int main() {\n"
              "  indirectHello();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void callback1() {
        check("void f1()\n"
              "{\n"
              "    void (*f)() = cond ? f1 : NULL;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void callback2() { // #8677
        check("class C {\n"
              "public:\n"
              "    void callback();\n"
              "    void start();\n"
              "};\n"
              "\n"
              "void C::callback() {}\n" // <- not unused
              "\n"
              "void C::start() { ev.set<C, &C::callback>(this); }");
        ASSERT_EQUALS("[test.cpp:9]: (style) The function 'start' is never used.\n", errout.str());
    }

    void else1() {
        check("void f1()\n"
              "{\n"
              "    if (cond) ;\n"
              "    else f1();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void functionpointer() {
        check("void foo() { }\n"
              "int main() {\n"
              "    f(&foo);\n"
              "    return 0\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() { }\n"
              "int main() {\n"
              "    f(&::foo);\n"
              "    return 0\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("namespace abc {\n"
              "    void foo() { }\n"
              "};\n"
              "int main() {\n"
              "    f(&abc::foo);\n"
              "    return 0\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("namespace abc {\n"
              "    void foo() { }\n"
              "};\n"
              "int main() {\n"
              "    f = &abc::foo;\n"
              "    return 0\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("namespace abc {\n"
              "    void foo() { }\n"
              "};\n"
              "int main() {\n"
              "    f = &::abc::foo;\n"
              "    return 0\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("namespace abc {\n"  // #3875
              "    void foo() { }\n"
              "};\n"
              "int main() {\n"
              "    f(abc::foo);\n"
              "    return 0\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void template1() {
        check("template<class T> void foo() { }\n"
              "\n"
              "int main()\n"
              "{\n"
              "    foo<int>();\n"
              "    return 0\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void template2() {
        check("void f() { }\n"
              "\n"
              "template<class T> void g()\n"
              "{\n"
              "    f();\n"
              "}\n"
              "\n"
              "void h() { g<int>(); h(); }");
        ASSERT_EQUALS("", errout.str());
    }

    void template3() { // #4701
        check("class X {\n"
              "public:\n"
              "    void bar() { foo<int>(0); }\n"
              "private:\n"
              "    template<typename T> void foo( T t ) const;\n"
              "};\n"
              "template<typename T> void X::foo( T t ) const { }");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'bar' is never used.\n", errout.str());
    }

    void template4() { // #9805
        check("struct A {\n"
              "    int a = 0;\n"
              "    void f() { a = 1; }\n"
              "    template <typename T, typename... Args> auto call(const Args &... args) -> T {\n"
              "        a = 2;\n"
              "        return T{};\n"
              "    }\n"
              "};\n"
              "\n"
              "struct B : public A {\n"
              "    void test() {\n"
              "        f();\n"
              "        call<int>(1, 2, 3);\n"
              "        test();\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void template5() { // #9220
        check("void f(){}\n"
              "\n"
              "typedef void(*Filter)();\n"
              "\n"
              "template <Filter fun>\n"
              "void g() { fun(); }\n"
              "\n"
              "int main() { g<f>(); return 0;}");
        ASSERT_EQUALS("", errout.str());
    }

    void template6() { // #10475
        check("template<template<typename...> class Ref, typename... Args>\n"
              "struct Foo<Ref<Args...>, Ref> : std::true_type {};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void template7()
    { // #9766
        check("void f() {\n"
              "    std::array<std::array<double,3>,3> array;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'f' is never used.\n", errout.str());
    }

    void template8() { // #11485
        check("struct S {\n"
              "    template<typename T>\n"
              "    void tf(const T&) { }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'tf' is never used.\n", errout.str());

        check("struct S {\n"
              "    template<typename T>\n"
              "    void tf(const T&) { }\n"
              "};\n"
              "int main() {\n"
              "    C c;\n"
              "    c.tf(1.5);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct S {\n"
              "    template<typename T>\n"
              "    void tf(const T&) { }\n"
              "};\n"
              "int main() {\n"
              "    C c;\n"
              "    c.tf<int>(1);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void template9() {
        check("template<class T>\n" // #7739
              "void f(T const& t) {}\n"
              "template<class T>\n"
              "void g(T const& t) {\n"
              "    f(t);\n"
              "}\n"
              "template<>\n"
              "void f<double>(double const& d) {}\n"
              "int main() {\n"
              "    g(2);\n"
              "    g(3.14);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("template <typename T> T f(T);\n" // #9222
              "template <typename T> T f(T i) { return i; }\n"
              "template int f<int>(int);\n"
              "int main() { return f(int(2)); }\n");
        ASSERT_EQUALS("", errout.str());
    }

    void template10() {
        check("template<typename T>\n" // #12013, don't crash
              "struct S {\n"
              "    static const int digits = std::numeric_limits<T>::digits;\n"
              "    using type = std::conditional<digits < 32, std::int32_t, std::int64_t>::type;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void throwIsNotAFunction() {
        check("struct A {void f() const throw () {}}; int main() {A a; a.f();}");
        ASSERT_EQUALS("", errout.str());
    }

    void unusedError() {
        check("void foo() {}\n"
              "int main()");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'foo' is never used.\n", errout.str());

        check("void foo() const {}\n"
              "int main()");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'foo' is never used.\n", errout.str());

        check("void foo() const throw() {}\n"
              "int main()");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'foo' is never used.\n", errout.str());

        check("void foo() throw() {}\n"
              "int main()");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'foo' is never used.\n", errout.str());
    }

    void unusedMain() {
        check("int main() { }");
        ASSERT_EQUALS("", errout.str());
    }

    void initializationIsNotAFunction() {
        check("struct B: N::A {\n"
              "  B(): N::A() {};\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void operator1() {
        check("struct Foo { void operator()(int a) {} };");
        ASSERT_EQUALS("", errout.str());

        check("struct Foo { operator std::string(int a) {} };");
        ASSERT_EQUALS("", errout.str());
    }

    void operator2() { // #7974
        check("bool operator==(const data_t& a, const data_t& b) {\n"
              "    return (a.fd == b.fd);\n"
              "}\n"
              "bool operator==(const event& a, const event& b) {\n"
              "    return ((a.events == b.events) && (a.data == b.data));\n"
              "}\n"
              "int main(event a, event b) {\n"
              "    return a == b;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void returnRef() {
        check("int& foo() {return x;}");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'foo' is never used.\n", errout.str());
    }

    void attribute() { // #3471 - FP __attribute__((constructor))
        check("void __attribute__((constructor)) f() {}");
        ASSERT_EQUALS("", errout.str());

        check("void __attribute__((constructor(1000))) f() {}");
        ASSERT_EQUALS("", errout.str());

        check("void __attribute__((destructor)) f() {}");
        ASSERT_EQUALS("", errout.str());

        check("void __attribute__((destructor(1000))) f() {}");
        ASSERT_EQUALS("", errout.str());

        // alternate syntax
        check("__attribute__((constructor)) void f() {}");
        ASSERT_EQUALS("", errout.str());

        check("__attribute__((constructor(1000))) void f() {}");
        ASSERT_EQUALS("", errout.str());

        check("__attribute__((destructor)) void f() {}");
        ASSERT_EQUALS("", errout.str());

        check("__attribute__((destructor(1000))) void f() {}");
        ASSERT_EQUALS("", errout.str());

        // alternate syntax
        check("void f() __attribute__((constructor));\n"
              "void f() { }");
        ASSERT_EQUALS("", errout.str());

        check("void f() __attribute__((constructor(1000)));\n"
              "void f() { }");
        ASSERT_EQUALS("", errout.str());

        check("void f() __attribute__((destructor));\n"
              "void f() { }");
        ASSERT_EQUALS("", errout.str());

        check("void f() __attribute__((destructor(1000)));\n"
              "void f() { }");
        ASSERT_EQUALS("", errout.str());

        // Don't crash on wrong syntax
        check("int x __attribute__((constructor));\n"
              "int y __attribute__((destructor));");

        // #10661
        check("extern \"C\" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize) { return 0; }\n");
        ASSERT_EQUALS("", errout.str());
    }

    void initializer_list() {
        check("int foo() { return 0; }\n"
              "struct A {\n"
              "    A() : m_i(foo())\n"
              "    {}\n"
              "int m_i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // #8580
        check("int foo() { return 12345; }\n"
              "int bar(std::function<int()> func) { return func(); }\n"
              "\n"
              "class A {\n"
              "public:\n"
              "  A() : a(bar([] { return foo(); })) {}\n"
              "  const int a;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void member_function_ternary() {
        check("struct Foo {\n"
              "    void F1() {}\n"
              "    void F2() {}\n"
              "};\n"
              "int main(int argc, char *argv[]) {\n"
              "    Foo foo;\n"
              "    void (Foo::*ptr)();\n"
              "    ptr = (argc > 1 && !strcmp(argv[1], \"F2\")) ? &Foo::F2 : &Foo::F1;\n"
              "    (foo.*ptr)();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void boost() {
        check("static void _xy(const char *b, const char *e)\n"
              "{}\n"
              "parse(line, blanks_p >> ident[&_xy] >> blanks_p >> eol_p).full");
        ASSERT_EQUALS("", errout.str());
    }

    void enumValues() { // #11486
        check("enum E1 { Break1 };\n"
              "struct S {\n"
              "    enum class E { Break };\n"
              "    void Break() {}\n"
              "    void Break1() {}\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'Break' is never used.\n"
                      "[test.cpp:5]: (style) The function 'Break1' is never used.\n",
                      errout.str());
    }

    void multipleFiles() {
        Tokenizer tokenizer(&settings, this);
        CheckUnusedFunctions c(&tokenizer, &settings, nullptr);

        // Clear the error buffer..
        errout.str("");

        const char code[] = "static void f() { }";

        for (int i = 1; i <= 2; ++i) {
            std::ostringstream fname;
            fname << "test" << i << ".cpp";

            // Clear the error buffer..
            errout.str("");

            Tokenizer tokenizer2(&settings, this);
            std::istringstream istr(code);
            ASSERT(tokenizer2.tokenize(istr, fname.str().c_str()));

            c.parseTokens(tokenizer2, "someFile.c", &settings);
        }

        // Check for unused functions..
        (c.check)(this, settings);

        ASSERT_EQUALS("[test1.cpp:1]: (style) The function 'f' is never used.\n", errout.str());
    }

    void lineNumber() {
        check("void foo();\n"
              "void bar() {}\n"
              "int main() {}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The function 'bar' is never used.\n", errout.str());
    }

    void ignore_declaration() {
        check("void f();\n"
              "void f() {}");
        ASSERT_EQUALS("[test.cpp:2]: (style) The function 'f' is never used.\n", errout.str());

        check("void f(void) {}\n"
              "void (*list[])(void) = {f}");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorOverload() {
        check("class A {\n"
              "private:\n"
              "    friend std::ostream & operator<<(std::ostream &, const A&);\n"
              "};\n"
              "std::ostream & operator<<(std::ostream &os, const A&) {\n"
              "    os << \"This is class A\";\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class A{};\n"
              "A operator + (const A &, const A &){ return A(); }\n"
              "A operator - (const A &, const A &){ return A(); }\n"
              "A operator * (const A &, const A &){ return A(); }\n"
              "A operator / (const A &, const A &){ return A(); }\n"
              "A operator % (const A &, const A &){ return A(); }\n"
              "A operator & (const A &, const A &){ return A(); }\n"
              "A operator | (const A &, const A &){ return A(); }\n"
              "A operator ~ (const A &){ return A(); }\n"
              "A operator ! (const A &){ return A(); }\n"
              "bool operator < (const A &, const A &){ return true; }\n"
              "bool operator > (const A &, const A &){ return true; }\n"
              "A operator += (const A &, const A &){ return A(); }\n"
              "A operator -= (const A &, const A &){ return A(); }\n"
              "A operator *= (const A &, const A &){ return A(); }\n"
              "A operator /= (const A &, const A &){ return A(); }\n"
              "A operator %= (const A &, const A &){ return A(); }\n"
              "A operator &= (const A &, const A &){ return A(); }\n"
              "A operator ^= (const A &, const A &){ return A(); }\n"
              "A operator |= (const A &, const A &){ return A(); }\n"
              "A operator << (const A &, const int){ return A(); }\n"
              "A operator >> (const A &, const int){ return A(); }\n"
              "A operator <<= (const A &, const int){ return A(); }\n"
              "A operator >>= (const A &, const int){ return A(); }\n"
              "bool operator == (const A &, const A &){ return true; }\n"
              "bool operator != (const A &, const A &){ return true; }\n"
              "bool operator <= (const A &, const A &){ return true; }\n"
              "bool operator >= (const A &, const A &){ return true; }\n"
              "A operator && (const A &, const int){ return A(); }\n"
              "A operator || (const A &, const int){ return A(); }\n"
              "A operator ++ (const A &, const int){ return A(); }\n"
              "A operator ++ (const A &){ return A(); }\n"
              "A operator -- (const A &, const int){ return A(); }\n"
              "A operator -- (const A &){ return A(); }\n"
              "A operator , (const A &, const A &){ return A(); }");
        ASSERT_EQUALS("", errout.str());


        check("class A {\n"
              "public:\n"
              "    static void * operator new(std::size_t);\n"
              "    static void * operator new[](std::size_t);\n"
              "};\n"
              "void * A::operator new(std::size_t s) {\n"
              "    return malloc(s);\n"
              "}\n"
              "void * A::operator new[](std::size_t s) {\n"
              "    return malloc(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void entrypointsWin() {
        check("int WinMain() { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'WinMain' is never used.\n", errout.str());

        check("int _tmain() { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function '_tmain' is never used.\n", errout.str());

        const Settings s = settingsBuilder(settings).library("windows.cfg").build();

        check("int WinMain() { }", Platform::Type::Native, &s);
        ASSERT_EQUALS("", errout.str());

        check("int _tmain() { }", Platform::Type::Native, &s);
        ASSERT_EQUALS("", errout.str());
    }

    void entrypointsWinU() {
        check("int wWinMain() { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function 'wWinMain' is never used.\n", errout.str());

        check("int _tmain() { }");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function '_tmain' is never used.\n", errout.str());

        const Settings s = settingsBuilder(settings).library("windows.cfg").build();

        check("int wWinMain() { }", Platform::Type::Native, &s);
        ASSERT_EQUALS("", errout.str());

        check("int _tmain() { }", Platform::Type::Native, &s);
        ASSERT_EQUALS("", errout.str());
    }

    void entrypointsUnix() {
        check("int _init() { }\n"
              "int _fini() { }\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) The function '_init' is never used.\n"
                      "[test.cpp:2]: (style) The function '_fini' is never used.\n", errout.str());

        const Settings s = settingsBuilder(settings).library("gnu.cfg").build();

        check("int _init() { }\n"
              "int _fini() { }\n", Platform::Type::Native, &s);
        ASSERT_EQUALS("", errout.str());
    }

    // TODO: fails because the location information is not be preserved by PreprocessorHelper::getcode()
    void includes()
    {
        // #11483
        const char inc[] = "class A {\n"
                           "public:\n"
                           "    void f() {}\n"
                           "};";
        const char code[] = R"(#include "test.h")";
        ScopedFile header("test.h", inc);
        Preprocessor preprocessor(settings, this);
        const std::string processed = PreprocessorHelper::getcode(preprocessor, code, "", "test.cpp");
        check(processed.c_str());
        TODO_ASSERT_EQUALS("[test.h:3]: (style) The function 'f' is never used.\n", "[test.cpp:3]: (style) The function 'f' is never used.\n", errout.str());
    }
};

REGISTER_TEST(TestUnusedFunctions)
