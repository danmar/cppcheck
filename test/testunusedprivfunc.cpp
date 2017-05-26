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

#include "checkclass.h"
#include "platform.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <simplecpp.h>
#include <map>
#include <vector>

class TestUnusedPrivateFunction : public TestFixture {
public:
    TestUnusedPrivateFunction() : TestFixture("TestUnusedPrivateFunction") {
    }

private:
    Settings settings;

    void run() {
        settings.addEnabled("style");

        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);
        TEST_CASE(test4);
        TEST_CASE(test5);
        TEST_CASE(test6); // ticket #2602

        // [ 2236547 ] False positive --style unused function, called via pointer
        TEST_CASE(func_pointer1);
        TEST_CASE(func_pointer2);
        TEST_CASE(func_pointer3);
        TEST_CASE(func_pointer4); // ticket #2807
        TEST_CASE(func_pointer5); // ticket #2233
        TEST_CASE(func_pointer6); // ticket #4787

        TEST_CASE(ctor);
        TEST_CASE(ctor2);

        TEST_CASE(classInClass);
        TEST_CASE(sameFunctionNames);
        TEST_CASE(incompleteImplementation);

        TEST_CASE(derivedClass);   // skip warning for derived classes. It might be a virtual function.

        TEST_CASE(friendClass);

        TEST_CASE(borland1);     // skip FP when using __property
        TEST_CASE(borland2);     // skip FP when using __published

        // No false positives when there are "unused" templates that are removed in the simplified token list
        TEST_CASE(template1);

        // #2407 - FP when called from operator()
        TEST_CASE(fp_operator);
        TEST_CASE(testDoesNotIdentifyMethodAsFirstFunctionArgument); // #2480
        TEST_CASE(testDoesNotIdentifyMethodAsMiddleFunctionArgument);
        TEST_CASE(testDoesNotIdentifyMethodAsLastFunctionArgument);

        TEST_CASE(multiFile);
        TEST_CASE(unknownBaseTemplate); // ticket #2580
        TEST_CASE(hierarchie_loop); // ticket 5590

        TEST_CASE(staticVariable); //ticket #5566
    }


    void check(const char code[], Settings::PlatformType platform = Settings::Native) {
        // Clear the error buffer..
        errout.str("");

        settings.platform(platform);

        // Raw tokens..
        std::vector<std::string> files;
        files.push_back("test.cpp");
        std::istringstream istr(code);
        const simplecpp::TokenList tokens1(istr, files, files[0]);

        // Preprocess..
        simplecpp::TokenList tokens2(files);
        std::map<std::string, simplecpp::TokenList*> filedata;
        simplecpp::preprocess(tokens2, tokens1, files, filedata, simplecpp::DUI());

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        tokenizer.createTokens(&tokens2);
        tokenizer.simplifyTokens1("");
        tokenizer.simplifyTokenList2();

        // Check for unused private functions..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.privateFunctions();
    }



    void test1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int f();\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "unsigned int Fred::f()\n"
              "{ }");

        ASSERT_EQUALS("[test.cpp:4]: (style) Unused private function: 'Fred::f'\n", errout.str());

        check("#line 1 \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int f();\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "#line 1 \"p.cpp\"\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "unsigned int Fred::f()\n"
              "{ }");

        ASSERT_EQUALS("[p.h:4]: (style) Unused private function: 'Fred::f'\n", errout.str());

        check("#line 1 \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "void f();\n"
              "};\n"
              "\n"
              "\n"
              "#line 1 \"p.cpp\"\n"
              "\n"
              "void Fred::f()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[p.h:4]: (style) Unused private function: 'Fred::f'\n", errout.str());

        // Don't warn about include files which implementation we don't see
        check("#line 1 \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "void f();\n"
              "void g() {}\n"
              "};\n"
              "\n"
              "#line 1 \"p.cpp\"\n"
              "\n"
              "int main()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void test2() {
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "\n"
              "    void a() const\n"
              "    { b(); }\n"
              "private:\n"
              "    void b( ) const\n"
              "    { }\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{ }");
        ASSERT_EQUALS("", errout.str());
    }


    void test3() {
        check("class A {\n"
              "public:\n"
              "    A() { }\n"
              "    ~A();\n"
              "private:\n"
              "    void B() { }\n"
              "};\n"
              "\n"
              "A::~A()\n"
              "{ B(); }");
        ASSERT_EQUALS("", errout.str());
    }


    void test4() {
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "private:\n"
              "    bool _owner;\n"
              "    void b() { }\n"
              "};\n"
              "\n"
              "A::A() : _owner(false)\n"
              "{ b(); }");
        ASSERT_EQUALS("", errout.str());
    }


    void test5() {
        check("class A {\n"
              "private:\n"
              "    A() : lock(new Lock())\n"
              "    { }\n"
              "    Lock *lock;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void test6() { // ticket #2602 segmentation fault
        check("class A {\n"
              "    A& operator=(const A&);\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }





    void func_pointer1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    typedef void (*testfp)();\n"
              "\n"
              "    testfp get()\n"
              "    {\n"
              "        return test;\n"
              "    }\n"
              "\n"
              "    static void test()\n"
              "    { }\n"
              "\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{}");

        ASSERT_EQUALS("[test.cpp:6]: (style) Unused private function: 'Fred::get'\n", errout.str());
    }



    void func_pointer2() {
        check("class UnusedPrivateFunctionMemberPointer\n"
              "{\n"
              "public:\n"
              "    UnusedPrivateFunctionMemberPointer()\n"
              "    :   mObserver(this, &UnusedPrivateFunctionMemberPointer::callback)\n"
              "    {}\n"
              "\n"
              "private:\n"
              "    void callback(const& unsigned) const {}\n"
              "\n"
              "    Observer<UnusedPrivateFunctionMemberPointer, unsigned> mObserver;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void func_pointer3() {
        check("class c1\n"
              "{\n"
              "public:\n"
              "    c1()\n"
              "    { sigc::mem_fun(this, &c1::f1); }\n"
              "\n"
              "private:\n"
              "    void f1() const {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void func_pointer4() { // ticket #2807
        check("class myclass {\n"
              "public:\n"
              "    myclass();\n"
              "private:\n"
              "    static void f();\n"
              "    void (*fptr)();\n"
              "};\n"
              "myclass::myclass() { fptr = &f; }\n"
              "void myclass::f() {}");
        ASSERT_EQUALS("", errout.str());
    }


    void func_pointer5() {
        check("class A {\n"
              "public:\n"
              "    A() { f = A::func; }\n"
              "    void (*f)();\n"
              "private:\n"
              "    static void func() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void func_pointer6() { // #4787
        check("class Test {\n"
              "private:\n"
              "    static void a(const char* p) { }\n"
              "public:\n"
              "    void test(void* f = a) {\n"
              "        f(\"test\");\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void ctor() {
        check("class PrivateCtor\n"
              "{\n"
              "private:\n"
              "    PrivateCtor(int threadNum) :\n"
              "        numOfThreads(threadNum)\n"
              "    {\n"
              "    }\n"
              "\n"
              "    int numOfThreads;\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void ctor2() {
        check("struct State {\n"
              "  State(double const totalWeighting= TotalWeighting()) :\n"
              "    totalWeighting_(totalWeighting) {}\n"
              "private:\n"
              "  double TotalWeighting() { return 123.0; }\n"  // called from constructor
              "public:\n"
              "  double totalWeighting_;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void classInClass() {
        check("class A\n"
              "{\n"
              "public:\n"
              "\n"
              "    class B\n"
              "    {\n"
              "    private:\n"
              "    };\n"
              "private:\n"
              "    static void f()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10]: (style) Unused private function: 'A::f'\n", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    A()\n"
              "    { }\n"
              "\n"
              "private:\n"
              "    void f()\n"
              "    { }\n"
              "\n"
              "    class B\n"
              "    {\n"
              "    public:\n"
              "        B(A *a)\n"
              "        { a->f(); }\n"
              "    };\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A {\n"  // #6968 - outer definition
              "public:\n"
              "  class B;\n"
              "private:\n"
              "  void f() {}\n"
              "}\n"
              "class A::B {"
              "  B() { A a; a.f(); }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void sameFunctionNames() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    void a()\n"
              "    {\n"
              "        f(1);\n"
              "    }\n"
              "\n"
              "private:\n"
              "    void f() { }\n"
              "    void f(int) { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void incompleteImplementation() {
        // The implementation for "A::a" is missing - so don't check if
        // "A::b" is used or not
        check("#file \"test.h\"\n"
              "class A\n"
              "{\n"
              "public:\n"
              "    A();\n"
              "    void a();\n"
              "private:\n"
              "    void b();\n"
              "};\n"
              "#endfile\n"
              "A::A() { }\n"
              "void A::b() { }");
        ASSERT_EQUALS("", errout.str());
    }

    void derivedClass() {
        // skip warning in derived classes in case the base class is invisible
        check("class derived : public base\n"
              "{\n"
              "public:\n"
              "    derived() : base() { }\n"
              "private:\n"
              "    void f();\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class base {\n"
              "public:\n"
              "    virtual void foo();\n"
              "    void bar();\n"
              "};\n"
              "class derived : public base {\n"
              "private:\n"
              "    void foo() {}\n" // Skip for overrides of virtual functions of base
              "    void bar() {}\n" // Don't skip if no function is overridden
              "};");
        ASSERT_EQUALS("[test.cpp:9]: (style) Unused private function: 'derived::bar'\n", errout.str());

        check("class Base {\n"
              "private:\n"
              "    virtual void func() = 0;\n"
              "public:\n"
              "    void public_func() {\n"
              "        func();\n"
              "    };\n"
              "};\n"
              "\n"
              "class Derived1: public Base {\n"
              "private:\n"
              "    void func() {}\n"
              "};\n"
              "class Derived2: public Derived1 {\n"
              "private:\n"
              "    void func() {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Base {\n"
              "public:\n"
              "    void dostuff() {\n"
              "      f();\n"
              "    }\n"
              "\n"
              "private:\n"
              "    virtual Base* f() = 0;\n"
              "};\n"
              "\n"
              "class Derived : public Base {\n"
              "private:\n"
              "    Derived* f() {\n"
              "      return 0;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void friendClass() {
        // ticket #2459 - friend class
        check("class Foo {\n"
              "private:\n"
              "    friend Bar;\n" // Unknown friend class
              "    void f() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct Bar {\n"
              "    void g() { f(); }\n" // Friend class seen, but f not seen
              "};\n"
              "class Foo {\n"
              "private:\n"
              "    friend Bar;\n"
              "    void f();\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct Bar {\n"
              "    void g() { f(); }\n" // Friend class seen, but f() used in it
              "};\n"
              "class Foo {\n"
              "private:\n"
              "    friend Bar;\n"
              "    void f() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Bar {\n" // Friend class seen, f() not used in it
              "};\n"
              "class Foo {\n"
              "    friend Bar;\n"
              "    void f() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (style) Unused private function: 'Foo::f'\n", errout.str());
    }

    void borland1() {
        // ticket #2034 - Borland C++ __property
        check("class Foo {\n"
              "private:\n"
              "    int getx() {\n"
              "        return 123;\n"
              "    }\n"
              "public:\n"
              "    Foo() { }\n"
              "    __property int x = {read=getx}\n"
              "};", Settings::Win32A);
        ASSERT_EQUALS("", errout.str());
    }

    void borland2() {
        // ticket #3661 - Borland C++ __published
        check("class Foo {\n"
              "__published:\n"
              "    int getx() {\n"
              "        return 123;\n"
              "    }\n"
              "public:\n"
              "    Foo() { }\n"
              "};", Settings::Win32A);
        ASSERT_EQUALS("", errout.str());
    }

    void template1() {
        // ticket #2067 - Template methods do not "use" private ones
        check("class A {\n"
              "public:\n"
              "    template <class T>\n"
              "    T getVal() const;\n"
              "\n"
              "private:\n"
              "    int internalGetVal() const { return 8000; }\n"
              "};\n"
              "\n"
              "template <class T>\n"
              "T A::getVal() const {\n"
              "    return internalGetVal();\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void fp_operator() {
        // #2407 - FP when function is called from operator()
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void operator()(int x) {\n"
              "        startListening();\n"
              "    }\n"
              "\n"
              "private:\n"
              "    void startListening() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void operator()(int x) {\n"
              "    }\n"
              "\n"
              "private:\n"
              "    void startListening() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:8]: (style) Unused private function: 'Fred::startListening'\n", errout.str());

        // #5059
        check("class Fred {\n"
              "    void* operator new(size_t obj_size, size_t buf_size) {}\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Unused private function: 'Fred::operatornew'\n", "", errout.str()); // No message for operators - we currently cannot check their usage

        check("class Fred {\n"
              "    void* operator new(size_t obj_size, size_t buf_size) {}\n"
              "public:\n"
              "    void* foo() { return new(size) Fred(); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void testDoesNotIdentifyMethodAsFirstFunctionArgument() {
        check("void callback(void (*func)(int), int arg)"
              "{"
              "    (*func)(arg);"
              "}"
              "class MountOperation"
              "{"
              "    static void Completed(int i);"
              "public:"
              "    MountOperation(int i);"
              "};"
              "void MountOperation::Completed(int i)"
              "{"
              "    std::cerr << i << std::endl;"
              "}"
              "MountOperation::MountOperation(int i)"
              "{"
              "    callback(MountOperation::Completed, i);"
              "}"
              "int main(void)"
              "{"
              "    MountOperation aExample(10);"
              "}"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testDoesNotIdentifyMethodAsMiddleFunctionArgument() {
        check("void callback(char, void (*func)(int), int arg)"
              "{"
              "    (*func)(arg);"
              "}"
              "class MountOperation"
              "{"
              "    static void Completed(int i);"
              "public:"
              "    MountOperation(int i);"
              "};"
              "void MountOperation::Completed(int i)"
              "{"
              "    std::cerr << i << std::endl;"
              "}"
              "MountOperation::MountOperation(int i)"
              "{"
              "    callback('a', MountOperation::Completed, i);"
              "}"
              "int main(void)"
              "{"
              "    MountOperation aExample(10);"
              "}"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void testDoesNotIdentifyMethodAsLastFunctionArgument() {
        check("void callback(int arg, void (*func)(int))"
              "{"
              "    (*func)(arg);"
              "}"
              "class MountOperation"
              "{"
              "    static void Completed(int i);"
              "public:"
              "    MountOperation(int i);"
              "};"
              "void MountOperation::Completed(int i)"
              "{"
              "    std::cerr << i << std::endl;"
              "}"
              "MountOperation::MountOperation(int i)"
              "{"
              "    callback(i, MountOperation::Completed);"
              "}"
              "int main(void)"
              "{"
              "    MountOperation aExample(10);"
              "}"
             );
        ASSERT_EQUALS("", errout.str());
    }

    void multiFile() { // ticket #2567
        check("#file \"test.h\"\n"
              "struct Fred\n"
              "{\n"
              "    Fred()\n"
              "    {\n"
              "        Init();\n"
              "    }\n"
              "private:\n"
              "    void Init();\n"
              "};\n"
              "#endfile\n"
              "void Fred::Init()\n"
              "{\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void unknownBaseTemplate() { // ticket #2580
        check("class Bla : public Base2<Base> {\n"
              "public:\n"
              "    Bla() {}\n"
              "private:\n"
              "    virtual void F() const;\n"
              "};\n"
              "void Bla::F() const { }");

        ASSERT_EQUALS("", errout.str());
    }

    void hierarchie_loop() {
        check("class InfiniteB : InfiniteA {\n"
              "    class D {\n"
              "    };\n"
              "};\n"
              "namespace N {\n"
              "    class InfiniteA : InfiniteB {\n"
              "    };\n"
              "}\n"
              "class InfiniteA : InfiniteB {\n"
              "    void foo();\n"
              "};\n"
              "void InfiniteA::foo() {\n"
              "    C a;\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void staticVariable() {
        check("class Foo {\n"
              "    static int i;\n"
              "    static int F() const { return 1; }\n"
              "};\n"
              "int Foo::i = Foo::F();");
        ASSERT_EQUALS("", errout.str());

        check("class Foo {\n"
              "    static int i;\n"
              "    int F() const { return 1; }\n"
              "};\n"
              "Foo f;\n"
              "int Foo::i = f.F();");
        ASSERT_EQUALS("", errout.str());

        check("class Foo {\n"
              "    static int i;\n"
              "    static int F() const { return 1; }\n"
              "};\n"
              "int Foo::i = sth();"
              "int i = F();");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unused private function: 'Foo::F'\n", errout.str());
    }
};

REGISTER_TEST(TestUnusedPrivateFunction)
