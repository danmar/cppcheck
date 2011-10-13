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

class TestUnusedPrivateFunction : public TestFixture {
public:
    TestUnusedPrivateFunction() : TestFixture("TestUnusedPrivateFunction")
    { }

private:
    void run() {
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

        TEST_CASE(ctor);

        TEST_CASE(classInClass);
        TEST_CASE(sameFunctionNames);
        TEST_CASE(incompleteImplementation);

        TEST_CASE(derivedClass);   // skip warning for derived classes. It might be a virtual function.

        TEST_CASE(friendClass);

        TEST_CASE(borland);     // skip FP when using __property

        // No false positives when there are "unused" templates that are removed in the simplified token list
        TEST_CASE(template1);

        // #2407 - FP when called from operator()
        TEST_CASE(fp_operator);
        TEST_CASE(testDoesNotIdentifyMethodAsFirstFunctionArgument); // #2480
        TEST_CASE(testDoesNotIdentifyMethodAsMiddleFunctionArgument);
        TEST_CASE(testDoesNotIdentifyMethodAsLastFunctionArgument);

        TEST_CASE(multiFile);
        TEST_CASE(unknownBaseTemplate); // ticket #2580
    }


    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

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
              "{ }\n");

        ASSERT_EQUALS("[test.cpp:4]: (style) Unused private function 'Fred::f'\n", errout.str());

        check("#file \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int f();\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "#endfile\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "unsigned int Fred::f()\n"
              "{ }\n");

        ASSERT_EQUALS("[p.h:4]: (style) Unused private function 'Fred::f'\n", errout.str());

        check("#file \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "void f();\n"
              "};\n"
              "\n"
              "\n"
              "#endfile\n"
              "\n"
              "void Fred::f()\n"
              "{\n"
              "}\n"
              "\n");
        ASSERT_EQUALS("[p.h:4]: (style) Unused private function 'Fred::f'\n", errout.str());

        // Don't warn about include files which implementation we don't see
        check("#file \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "void f();\n"
              "void g() {}\n"
              "};\n"
              "\n"
              "#endfile\n"
              "\n"
              "int main()\n"
              "{\n"
              "}\n"
              "\n");
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
              "{ }\n");
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
              "{ B(); }\n");
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
              "{ b(); }\n");
        ASSERT_EQUALS("", errout.str());
    }


    void test5() {
        check("class A {\n"
              "private:\n"
              "    A() : lock(new Lock())\n"
              "    { }\n"
              "    Lock *lock;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void test6() { // ticket #2602 segmentation fault
        check("class A {\n"
              "    A& operator=(const A&);\n"
              "};\n");
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
              "{}\n");

        ASSERT_EQUALS("[test.cpp:6]: (style) Unused private function 'Fred::get'\n", errout.str());
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
              "};\n");
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
              "};\n");
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
              "void myclass::f() {}\n");
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
              "};\n");

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
              "\n"
              "    static void f()\n"
              "    { }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

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
              "};\n");
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
              "    A()\n"
              "    void a();\n"
              "private:\n"
              "    void b();\n"
              "};\n"
              "#endfile\n"
              "A::A() { }\n"
              "void A::b() { }\n");
        ASSERT_EQUALS("", errout.str());
    }

    void derivedClass() {
        // skip warning in derived classes in case the function is virtual
        check("class derived : public base\n"
              "{\n"
              "public:\n"
              "    derived() : base() { }\n"
              "private:\n"
              "    void f();\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void friendClass() {
        // ticket #2459 - friend class
        check("class Foo {\n"
              "private:\n"
              "    friend Bar;\n"
              "    void f() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void borland() {
        // ticket #2034 - Borland C++ __property
        check("class Foo {\n"
              "private:\n"
              "    int getx() {\n"
              "        return 123;\n"
              "    }\n"
              "public:\n"
              "    Foo() { }\n"
              "    __property int x = {read=getx}\n"
              "};");
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
              "};\n");
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
              "};\n");
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
              "};\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) Unused private function 'Fred::startListening'\n", errout.str());
    }

    void testDoesNotIdentifyMethodAsFirstFunctionArgument() {
        check("#include <iostream>"
              "void callback(void (*func)(int), int arg)"
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
        check("#include <iostream>"
              "void callback(char, void (*func)(int), int arg)"
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
        check("#include <iostream>"
              "void callback(int arg, void (*func)(int))"
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
              "}\n");

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

};

REGISTER_TEST(TestUnusedPrivateFunction)

