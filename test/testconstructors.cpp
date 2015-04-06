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
#include "checkclass.h"
#include "testsuite.h"


class TestConstructors : public TestFixture {
public:
    TestConstructors() : TestFixture("TestConstructors") {
    }

private:


    void check(const char code[], bool showAll = false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.inconclusive = showAll;
        settings.addEnabled("style");
        settings.addEnabled("warning");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

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
        TEST_CASE(simple6); // ticket #4085 - uninstantiated template class
        TEST_CASE(simple7); // ticket #4531
        TEST_CASE(simple8);
        TEST_CASE(simple9); // ticket #4574
        TEST_CASE(simple10); // ticket #4388
        TEST_CASE(simple11); // ticket #4536, #6214
        TEST_CASE(simple12); // ticket #4620
        TEST_CASE(simple13); // #5498 - no constructor, c++11 assignments
        TEST_CASE(simple14); // #6253 template base

        TEST_CASE(noConstructor1);
        TEST_CASE(noConstructor2);
        TEST_CASE(noConstructor3);
        TEST_CASE(noConstructor4);
        TEST_CASE(noConstructor5);
        TEST_CASE(noConstructor6); // ticket #4386
        TEST_CASE(noConstructor7); // ticket #4391
        TEST_CASE(noConstructor8); // ticket #4404
        TEST_CASE(noConstructor9); // ticket #4419
        TEST_CASE(noConstructor10); // ticket #6614

        TEST_CASE(forwardDeclaration); // ticket #4290/#3190

        TEST_CASE(initvar_with_this);       // BUG 2190300
        TEST_CASE(initvar_if);              // BUG 2190290
        TEST_CASE(initvar_operator_eq1);     // BUG 2190376
        TEST_CASE(initvar_operator_eq2);     // BUG 2190376
        TEST_CASE(initvar_operator_eq3);
        TEST_CASE(initvar_operator_eq4);     // ticket #2204
        TEST_CASE(initvar_operator_eq5);     // ticket #4119
        TEST_CASE(initvar_same_classname);      // BUG 2208157
        TEST_CASE(initvar_chained_assign);      // BUG 2270433
        TEST_CASE(initvar_2constructors);       // BUG 2270353
        TEST_CASE(initvar_constvar);
        TEST_CASE(initvar_staticvar);
        TEST_CASE(initvar_union);
        TEST_CASE(initvar_delegate);       // ticket #4302

        TEST_CASE(initvar_private_constructor);     // BUG 2354171 - private constructor
        TEST_CASE(initvar_copy_constructor); // ticket #1611
        TEST_CASE(initvar_nested_constructor); // ticket #1375
        TEST_CASE(initvar_nocopy1);            // ticket #2474
        TEST_CASE(initvar_nocopy2);            // ticket #2484
        TEST_CASE(initvar_nocopy3);            // ticket #3611
        TEST_CASE(initvar_with_member_function_this); // ticket #4824

        TEST_CASE(initvar_destructor);      // No variables need to be initialized in a destructor
        TEST_CASE(initvar_func_ret_func_ptr); // ticket #4449

        TEST_CASE(operatorEqSTL);

        TEST_CASE(uninitVar1);
        TEST_CASE(uninitVar2);
        TEST_CASE(uninitVar3);
        TEST_CASE(uninitVar4);
        TEST_CASE(uninitVar5);
        TEST_CASE(uninitVar6);
        TEST_CASE(uninitVar7);
        TEST_CASE(uninitVar8);
        TEST_CASE(uninitVar9);  // ticket #1730
        TEST_CASE(uninitVar10); // ticket #1993
        TEST_CASE(uninitVar11);
        TEST_CASE(uninitVar12); // ticket #2078
        TEST_CASE(uninitVar13); // ticket #1195
        TEST_CASE(uninitVar14); // ticket #2149
        TEST_CASE(uninitVar15);
        TEST_CASE(uninitVar16);
        TEST_CASE(uninitVar17);
        TEST_CASE(uninitVar18); // ticket #2465
        TEST_CASE(uninitVar19); // ticket #2792
        TEST_CASE(uninitVar20); // ticket #2867
        TEST_CASE(uninitVar21); // ticket #2947
        TEST_CASE(uninitVar22); // ticket #3043
        TEST_CASE(uninitVar23); // ticket #3702
        TEST_CASE(uninitVar24); // ticket #3190
        TEST_CASE(uninitVar25); // ticket #4789
        TEST_CASE(uninitVar26);
        TEST_CASE(uninitVar27); // ticket #5170 - rtl::math::setNan(&d)
        TEST_CASE(uninitVar28); // ticket #6258
        TEST_CASE(uninitVar29);
        TEST_CASE(uninitVar30); // ticket #6417
        TEST_CASE(uninitVarEnum);
        TEST_CASE(uninitVarStream);
        TEST_CASE(uninitVarTypedef);
        TEST_CASE(uninitVarMemset);
        TEST_CASE(uninitVarArray1);
        TEST_CASE(uninitVarArray2);
        TEST_CASE(uninitVarArray3);
        TEST_CASE(uninitVarArray4);
        TEST_CASE(uninitVarArray5);
        TEST_CASE(uninitVarArray6);
        TEST_CASE(uninitVarArray7);
        TEST_CASE(uninitVarArray8);
        TEST_CASE(uninitVarArray2D);
        TEST_CASE(uninitVarArray3D);
        TEST_CASE(uninitVarCpp11Init1);
        TEST_CASE(uninitVarCpp11Init2);
        TEST_CASE(uninitVarStruct1);       // ticket #2172
        TEST_CASE(uninitVarStruct2);       // ticket #838
        TEST_CASE(uninitVarUnion1);        // ticket #3196
        TEST_CASE(uninitVarUnion2);
        TEST_CASE(uninitMissingFuncDef);   // can't expand function in constructor
        TEST_CASE(privateCtor1);           // If constructor is private..
        TEST_CASE(privateCtor2);           // If constructor is private..
        TEST_CASE(function);               // Function is not variable
        TEST_CASE(uninitVarHeader1);       // Class is defined in header
        TEST_CASE(uninitVarHeader2);       // Class is defined in header
        TEST_CASE(uninitVarHeader3);       // Class is defined in header
        TEST_CASE(uninitVarPublished);     // Borland C++: Variables in the published section are auto-initialized
        TEST_CASE(uninitOperator);         // No FP about uninitialized 'operator[]'
        TEST_CASE(uninitFunction1);        // No FP when initialized in function
        TEST_CASE(uninitFunction2);        // No FP when initialized in function
        TEST_CASE(uninitFunction3);        // No FP when initialized in function
        TEST_CASE(uninitFunction4);
        TEST_CASE(uninitFunction5);
        TEST_CASE(uninitSameClassName);    // No FP when two classes have the same name
        TEST_CASE(uninitFunctionOverload); // No FP when there are overloaded functions
        TEST_CASE(uninitVarOperatorEqual); // ticket #2415
        TEST_CASE(uninitVarPointer);       // ticket #3801
        TEST_CASE(uninitConstVar);
        TEST_CASE(constructors_crash1);    // ticket #5641
    }


    void simple1() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:1]: (style) The class 'Fred' does not have a constructor.\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:1]: (style) The struct 'Fred' does not have a constructor.\n", errout.str());
    }


    void simple2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() : i(0) { }\n"
              "    Fred(Fred const & other) : i(other.i) {}\n"
              "    Fred(Fred && other) : i(other.i) {}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { i = 0; }\n"
              "    Fred(Fred const & other) {i=other.i}\n"
              "    Fred(Fred && other) {i=other.i}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "    Fred(Fred const & other) {}\n"
              "    Fred(Fred && other) {}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n"
                      "[test.cpp:6]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred() { }\n"
              "    Fred(Fred const & other) {}\n"
              "    Fred(Fred && other) {}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n"
                      "[test.cpp:4]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }


    void simple3() {
        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred() :i(0)\n"
              "{ }");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ i = 0; }");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }


    void simple4() {
        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    explicit Fred(int _i);\n"
              "    Fred(Fred const & other);\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }\n"
              "Fred::Fred(int _i)\n"
              "{\n"
              "    i = _i;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:8]: (warning, inconclusive) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
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

    void simple6() { // ticket #4085 - uninstantiated template class
        check("template <class T> struct A {\n"
              "    A<T>() { x = 0; }\n"
              "    A<T>(const T & t) { x = t.x; }\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("template <class T> struct A {\n"
              "    A<T>() : x(0) { }\n"
              "    A<T>(const T & t) : x(t.x) { }\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("template <class T> struct A {\n"
              "    A<T>() : x(0) { }\n"
              "    A<T>(const T & t) : x(t.x) { }\n"
              "private:\n"
              "    int x;\n"
              "    int y;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Member variable 'A::y' is not initialized in the constructor.\n"
                      "[test.cpp:3]: (warning) Member variable 'A::y' is not initialized in the constructor.\n", errout.str());
    }

    void simple7() { // ticket #4531
        check("class Fred;\n"
              "struct Fred {\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void simple8() {
        check("struct Fred { int x; };\n"
              "class Barney { Fred fred; };\n"
              "class Wilma { struct Betty { int x; } betty; };");
        ASSERT_EQUALS("[test.cpp:2]: (style) The class 'Barney' does not have a constructor.\n"
                      "[test.cpp:3]: (style) The class 'Wilma' does not have a constructor.\n", errout.str());
    }

    void simple9() { // ticket #4574
        check("class Unknown::Fred {\n"
              "public:\n"
              "    Fred() : x(0) { }\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void simple10() { // ticket #4388
        check("class Fred {\n"
              "public:\n"
              "    Fred() = default;\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void simple11() { // ticket #4536, #6214
        check("class Fred {\n"
              "public:\n"
              "    Fred() {}\n"
              "private:\n"
              "    int x = 0;\n"
              "    int y = f();\n"
              "    int z{0};\n"
              "    int (*pf[2])(){nullptr, nullptr};\n"
              "    int a[2][3] = {{1,2,3},{4,5,6}};\n"
              "    int b{1}, c{2};\n"
              "    int d, e{3};\n"
              "    int f{4}, g;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n"
                      "[test.cpp:3]: (warning) Member variable 'Fred::g' is not initialized in the constructor.\n", errout.str());
    }

    void simple12() { // ticket #4620
        check("class Fred {\n"
              "    int x;\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "    void Init(int i = 0);\n"
              "};\n"
              "void Fred::Init(int i) { x = i; }");
        ASSERT_EQUALS("", errout.str());

        check("class Fred {\n"
              "    int x;\n"
              "    int y;\n"
              "public:\n"
              "    Fred() { Init(0); }\n"
              "    void Init(int i, int j = 0);\n"
              "};\n"
              "void Fred::Init(int i, int j) { x = i; y = j; }");
        ASSERT_EQUALS("", errout.str());
    }

    void simple13() { // #5498
        check("class Fred {\n"
              "    int x=1;\n"
              "    int *y=0;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void simple14() { // #6253 template base
        check("class Fred : public Base<A, B> {"
              "public:"
              "    Fred()\n"
              "    :Base<A, B>(1),\n"
              "     x(1)\n"
              "    {}\n"
              "private:\n"
              "    int x;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred : public Base<A, B> {"
              "public:"
              "    Fred()\n"
              "    :Base<A, B>{1},\n"
              "     x{1}\n"
              "    {}\n"
              "private:\n"
              "    int x;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor1() {
        // There are nonstatic member variables - constructor is needed
        check("class Fred\n"
              "{\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:1]: (style) The class 'Fred' does not have a constructor.\n", errout.str());
    }

    void noConstructor2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    static void foobar();\n"
              "};\n"
              "\n"
              "void Fred::foobar()\n"
              "{ }");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor3() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    static int foobar;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor4() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int foobar;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor5() {
        check("namespace Foo\n"
              "{\n"
              "    int i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor6() {
        // ticket #4386
        check("class Ccpucycles {\n"
              "    friend class foo::bar;\n"
              "    Ccpucycles() :\n"
              "    m_v(0), m_b(true)\n"
              "    {}\n"
              "private:\n"
              "    cpucyclesT m_v;\n"
              "    bool m_b;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor7() {
        // ticket #4391
        check("short bar;\n"
              "class foo;\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor8() {
        // ticket #4404
        check("class LineSegment;\n"
              "class PointArray  { };\n"
              "void* tech_ = NULL;\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor9() {
        // ticket #4419
        check("class CGreeting : public CGreetingBase<char> {\n"
              "public:\n"
              " CGreeting() : CGreetingBase<char>(), MessageSet(false) {}\n"
              "private:\n"
              " bool MessageSet;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor10() {
        // ticket #6614
        check("class A : public wxDialog\n"
              "{\n"
              "private:\n"
              "    DECLARE_EVENT_TABLE()\n"
              "public:\n"
              "    A(wxWindow *parent,\n"
              "      wxWindowID id = 1,\n"
              "      const wxString &title = wxT(""),\n"
              "      const wxPoint& pos = wxDefaultPosition,\n"
              "      const wxSize& size = wxDefaultSize,\n"
              "      long style = wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX);\n"
              "    virtual ~A();\n"
              "private:\n"
              "    wxTimer *WxTimer1;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // ticket #4290 "False Positive: style (noConstructor): The class 'foo' does not have a constructor."
    // ticket #3190 "SymbolDatabase: Parse of sub class constructor fails"
    void forwardDeclaration() {
        check("class foo;\n"
              "int bar;\n");
        ASSERT_EQUALS("", errout.str());

        check("class foo;\n"
              "class foo;\n");
        ASSERT_EQUALS("", errout.str());

        check("class foo{};\n"
              "class foo;\n");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_with_this() {
        check("struct Fred\n"
              "{\n"
              "    Fred()\n"
              "    { this->i = 0; }\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_if() {
        check("struct Fred\n"
              "{\n"
              "    Fred()\n"
              "    {\n"
              "        if (true)\n"
              "            i = 0;\n"
              "        else\n"
              "            i = 1;\n"
              "    }\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_operator_eq1() {
        // Bug 2190376 and #3820 - False positive, Uninitialized member variable with operator=

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
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred {\n"
              "    int i;\n"
              "\n"
              "    Fred(const Fred &fred)\n"
              "    { (*this) = fred; }\n"
              "\n"
              "    const Fred & operator=(const Fred &fred)\n"
              "    { i = fred.i; return *this; }\n"
              "};");
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
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void initvar_operator_eq2() {
        check("struct Fred\n"
              "{\n"
              "    Fred() { i = 0; }\n"
              "    void operator=(const Fred &fred) { }\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='.\n", errout.str());
    }

    void initvar_operator_eq3() {
        check("struct Fred\n"
              "{\n"
              "    Fred() { Init(); }\n"
              "    void operator=(const Fred &fred) { Init(); }\n"
              "private:\n"
              "    void Init() { i = 0; }\n"
              "    int i;\n"
              "};");
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
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='.\n", errout.str());

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
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='.\n", errout.str());

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
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='.\n", errout.str());

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
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_operator_eq5() { // #4119 - false positive when using swap to assign variables
        check("class Fred {\n"
              "    int i;\n"
              "public:\n"
              "    Fred() : i(5) { }\n"
              "    ~Fred() { }\n"
              "    Fred(const Fred &fred) : i(fred.i) { }\n"
              "    Fred & operator=(const Fred &rhs) {\n"
              "        Fred(rhs).swap(*this);\n"
              "        return *this;\n"
              "    }\n"
              "};");
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
              "}");

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
              "}");

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
              "}");

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
              "};");

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
              "};");

        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::a' is not initialized in the constructor.\n"
                      "[test.cpp:16]: (warning) Member variable 'Fred::b' is not initialized in the constructor.\n", errout.str());
    }

    void initvar_chained_assign() {
        // Bug 2270433 - Uninitialized variable false positive on chained assigns

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
              "}");

        ASSERT_EQUALS("", errout.str());
    }


    void initvar_2constructors() {
        check("struct c\n"
              "{\n"
              "    c();\n"
              "    explicit c(bool b);"
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
              "}");

        ASSERT_EQUALS("", errout.str());
    }


    void initvar_constvar() {
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
              "};");
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
              "};");
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
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (warning) Member variable 'Fred::U' is not initialized in the constructor.\n", "", errout.str());
    }


    void initvar_delegate() {
        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { }\n"
              "    A() : A(42) {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'A::number' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning) Member variable 'A::number' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { number = n; }\n"
              "    A() : A(42) {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A() { }\n"
              "    A() {}\n"
              "};", true);
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'A::number' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning, inconclusive) Member variable 'A::number' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A() { }\n"
              "    A() { number = 42; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { }\n"
              "    A() : A{42} {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'A::number' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning) Member variable 'A::number' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { number = n; }\n"
              "    A() : A{42} {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A{} { }\n"
              "    A() {}\n"
              "};", true);
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'A::number' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning, inconclusive) Member variable 'A::number' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A{} { }\n"
              "    A() { number = 42; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::string var;\n"
              "public:\n"
              "    Fred() { };\n"
              "    Fred(const Fred &) { };\n"
              "};", true);
        ASSERT_EQUALS("[test.cpp:7]: (warning, inconclusive) Member variable 'Fred::var' is not initialized in the constructor.\n", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::string var;\n"
              "public:\n"
              "    Fred();\n"
              "    Fred(const Fred &);\n"
              "};\n"
              "Fred::Fred() { };\n"
              "Fred::Fred(const Fred &) { };\n", true);
        ASSERT_EQUALS("[test.cpp:10]: (warning, inconclusive) Member variable 'Fred::var' is not initialized in the constructor.\n", errout.str());
    }

    void initvar_nested_constructor() { // ticket #1375
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        explicit B(int x);\n"
              "        struct C {\n"
              "            explicit C(int y);\n"
              "            struct D {\n"
              "                int d;\n"
              "                explicit D(int z);\n"
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
              "A::B::C::D::D(int z){}");
        // Note that the example code is not compilable. The A constructor must
        // explicitly initialize A::b. A warning for A::b is not necessary.
        ASSERT_EQUALS("[test.cpp:20]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'B::b' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'C::c' is not initialized in the constructor.\n"
                      "[test.cpp:23]: (warning) Member variable 'D::d' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        explicit B(int x);\n"
              "        struct C {\n"
              "            explicit C(int y);\n"
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
              "A::B::C::D::D(const A::B::C::D & d){}");
        // Note that the example code is not compilable. The A constructor must
        // explicitly initialize A::b. A warning for A::b is not necessary.
        ASSERT_EQUALS("[test.cpp:20]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'B::b' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'C::c' is not initialized in the constructor.\n"
                      "[test.cpp:23]: (warning) Member variable 'D::d' is not initialized in the constructor.\n", errout.str());

        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        explicit B(int x);\n"
              "        struct C {\n"
              "            explicit C(int y);\n"
              "            struct D {\n"
              "                struct E { int e; };\n"
              "                struct E d;\n"
              "                explicit D(const E &);\n"
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
              "A::B::C::D::D(const A::B::C::D::E & e){}");
        // Note that the example code is not compilable. The A constructor must
        // explicitly initialize A::b. A warning for A::b is not necessary.
        ASSERT_EQUALS("[test.cpp:21]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'B::b' is not initialized in the constructor.\n"
                      "[test.cpp:23]: (warning) Member variable 'C::c' is not initialized in the constructor.\n"
                      "[test.cpp:24]: (warning) Member variable 'D::d' is not initialized in the constructor.\n", errout.str());
    }

    void initvar_nocopy1() { // ticket #2474
        check("class B\n"
              "{\n"
              "    B (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class B\n"
              "{\n"
              "    B (B && Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class B\n"
              "{\n"
              "    B & operator= (const B & Var);\n"
              "public:\n"
              "    B ();\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:12]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:13]: (warning) Member variable 'A::m_SemVar' is not assigned a value in 'A::operator='.\n", errout.str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B (B && Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:12]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n", errout.str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B ();\n"
              "    B & operator= (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:12]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:13]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:14]: (warning) Member variable 'A::m_SemVar' is not assigned a value in 'A::operator='.\n", errout.str());

        check("class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout.str());
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
              "};");
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
              "};");
        ASSERT_EQUALS("[test.cpp:12]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor.\n"
                      "[test.cpp:13]: (warning) Member variable 'A::m_SemVar' is not assigned a value in 'A::operator='.\n", errout.str());
    }

    void initvar_nocopy3() { // #3611 - unknown type is non-copyable
        check("struct A {\n"
              "    B b;\n"
              "    A() {}\n"
              "    A(const A& rhs) {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    B b;\n"
              "    A() {}\n"
              "    A(const A& rhs) {}\n"
              "};", true);
        ASSERT_EQUALS("[test.cpp:4]: (warning, inconclusive) Member variable 'A::b' is not initialized in the constructor.\n", errout.str());
    }

    void initvar_with_member_function_this() {
        check("struct Foo {\n"
              "  Foo(int m) { this->setMember(m); }\n"
              "  void setMember(int m) { member = m; }\n"
              "  int member;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_destructor() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int var;\n"
              "public:\n"
              "    Fred() : var(0) {}\n"
              "    ~Fred() {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void initvar_func_ret_func_ptr() { // ticket #4449 (segmentation fault)
        check("class something {\n"
              "    int * ( something :: * process()) () { return 0; }\n"
              "    something() { process(); }\n"
              "};");
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
        ASSERT_EQUALS("[test.cpp:13]: (warning, inconclusive) Member variable 'Fred::ints' is not assigned a value in 'Fred::operator='.\n", errout.str());
    }

    void uninitVar1() {
        check("enum ECODES\n"
              "{\n"
              "    CODE_1 = 0,\n"
              "    CODE_2 = 1\n"
              "};\n"
              "\n"
              "class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() {}\n"
              "\n"
              "private:\n"
              "    ECODES _code;\n"
              "};");

        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Fred::_code' is not initialized in the constructor.\n", errout.str());


        check("class A{};\n"
              "\n"
              "class B : public A\n"
              "{\n"
              "public:\n"
              "  B() {}\n"
              "private:\n"
              "  float f;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'B::f' is not initialized in the constructor.\n", errout.str());

        check("class C\n"
              "{\n"
              "    FILE *fp;\n"
              "\n"
              "public:\n"
              "    explicit C(FILE *fp);\n"
              "};\n"
              "\n"
              "C::C(FILE *fp) {\n"
              "    C::fp = fp;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar2() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { (*this).i = 0; }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar3() {
        // No FP when struct has constructor
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { }\n"
              "private:\n"
              "    struct Bar {\n"
              "        Bar();\n"
              "    };\n"
              "    Bar bars[2];\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // Using struct that doesn't have constructor
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { }\n"
              "private:\n"
              "    struct Bar {\n"
              "        int x;\n"
              "    };\n"
              "    Bar bars[2];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Foo::bars' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar4() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { bar.x = 0; }\n"
              "private:\n"
              "    struct Bar {\n"
              "        int x;\n"
              "    };\n"
              "    struct Bar bar;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar5() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { }\n"
              "    Foo &operator=(const Foo &)\n"
              "    { return *this; }\n"
              "    static int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar6() {
        check("class Foo : public Bar\n"
              "{\n"
              "public:\n"
              "    explicit Foo(int i) : Bar(mi=i) { }\n"
              "private:\n"
              "    int mi;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Foo : public Bar\n"
              "{\n"
              "public:\n"
              "    explicit Foo(int i) : Bar{mi=i} { }\n"
              "private:\n"
              "    int mi;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar7() {
        check("class Foo {\n"
              "    int a;\n"
              "public:\n"
              "    Foo() : a(0) {}\n"
              "    Foo& operator=(const Foo&);\n"
              "    void Swap(Foo& rhs);\n"
              "};\n"
              "\n"
              "void Foo::Swap(Foo& rhs) {\n"
              "    std::swap(a,rhs.a);\n"
              "}\n"
              "\n"
              "Foo& Foo::operator=(const Foo& rhs) {\n"
              "    Foo copy(rhs);\n"
              "    copy.Swap(*this);\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar8() {
        check("class Foo {\n"
              "    int a;\n"
              "public:\n"
              "    Foo() : a(0) {}\n"
              "    Foo& operator=(const Foo&);\n"
              "};\n"
              "\n"
              "Foo& Foo::operator=(const Foo& rhs) {\n"
              "    if (&rhs != this)\n"
              "    {\n"
              "    }\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Member variable 'Foo::a' is not assigned a value in 'Foo::operator='.\n", errout.str());
    }

    void uninitVar9() { // ticket #1730
        check("class Prefs {\n"
              "private:\n"
              "    int xasd;\n"
              "public:\n"
              "    explicit Prefs(wxSize size);\n"
              "};\n"
              "Prefs::Prefs(wxSize size)\n"
              "{\n"
              "    SetMinSize( wxSize( 48,48 ) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Prefs::xasd' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar10() { // ticket #1993
        check("class A {\n"
              "public:\n"
              "        A();\n"
              "private:\n"
              "        int var1;\n"
              "        int var2;\n"
              "};\n"
              "A::A() : var1(0) { }");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Member variable 'A::var2' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar11() {
        check("class A {\n"
              "public:\n"
              "        explicit A(int a = 0);\n"
              "private:\n"
              "        int var;\n"
              "};\n"
              "A::A(int a) { }");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'A::var' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar12() { // ticket #2078
        check("class Point\n"
              "{\n"
              "public:\n"
              "    Point()\n"
              "    {\n"
              "        Point(0, 0);\n"
              "    }\n"
              "    Point(int x, int y)\n"
              "        : x(x), y(y)\n"
              "    {}\n"
              "    int x, y;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Point::x' is not initialized in the constructor.\n"
                      "[test.cpp:4]: (warning) Member variable 'Point::y' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar13() { // ticket #1195
        check("class A {\n"
              "private:\n"
              "    std::vector<int> *ints;\n"
              "public:\n"
              "    A()\n"
              "    {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'A::ints' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar14() { // ticket #2149
        // no namespace
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo();\n"
              "private:\n"
              "    bool mMember;\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // single namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "    Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor outside namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "Foo::Foo()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // constructor outside namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "Output::Foo::Foo()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor in separate namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "namespace Output\n"
              "{\n"
              "    Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:13]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor in different separate namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "namespace Input\n"
              "{\n"
              "    Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // constructor in different separate namespace (won't compile)
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "namespace Input\n"
              "{\n"
              "    Output::Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // constructor in nested separate namespace
        check("namespace A\n"
              "{\n"
              "    namespace Output\n"
              "    {\n"
              "        class Foo\n"
              "        {\n"
              "        public:\n"
              "            Foo();\n"
              "        private:\n"
              "            bool mMember;\n"
              "        };\n"
              "    }\n"
              "    namespace Output\n"
              "    {\n"
              "        Foo::Foo()\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:15]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor in nested different separate namespace
        check("namespace A\n"
              "{\n"
              "    namespace Output\n"
              "    {\n"
              "        class Foo\n"
              "        {\n"
              "        public:\n"
              "            Foo();\n"
              "        private:\n"
              "            bool mMember;\n"
              "        };\n"
              "    }\n"
              "    namespace Input\n"
              "    {\n"
              "        Foo::Foo()\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // constructor in nested different separate namespace
        check("namespace A\n"
              "{\n"
              "    namespace Output\n"
              "    {\n"
              "        class Foo\n"
              "        {\n"
              "        public:\n"
              "            Foo();\n"
              "        private:\n"
              "            bool mMember;\n"
              "        };\n"
              "    }\n"
              "    namespace Input\n"
              "    {\n"
              "        Output::Foo::Foo()\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar15() {
        check("class Fred\n"
              "{\n"
              "    int a;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "Fred::~Fred()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar16() {
        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "    void set(int x) { a = x; }\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo;\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "        foo.set(0);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "    void set(int x) { a = x; }\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo;\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Bar::foo' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar17() {
        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo[10];\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "        foo[0].a = 0;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo[10];\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Member variable 'Bar::foo' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar18() { // ticket #2465
        check("struct Altren\n"
              "{\n"
              "    explicit Altren(int _a = 0) : value(0) { }\n"
              "    int value;\n"
              "};\n"
              "class A\n"
              "{\n"
              "public:\n"
              "    A() { }\n"
              "private:\n"
              "    Altren value;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar19() { // ticket #2792
        check("class mystring\n"
              "{\n"
              "    char* m_str;\n"
              "    int m_len;\n"
              "public:\n"
              "    explicit mystring(const char* str)\n"
              "    {\n"
              "        m_len = strlen(str);\n"
              "        m_str = (char*) malloc(m_len+1);\n"
              "        memcpy(m_str, str, m_len+1);\n"
              "    }\n"
              "    mystring& operator=(const mystring& copy)\n"
              "    {\n"
              "        return (*this = copy.m_str);\n"
              "    }\n"
              "    ~mystring()\n"
              "    {\n"
              "        free(m_str);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar20() { // ticket #2867
        check("Object::MemFunc() {\n"
              "    class LocalClass {\n"
              "    public:\n"
              "        LocalClass() : dataLength_(0) {}\n"
              "        std::streamsize dataLength_;\n"
              "        double bitsInData_;\n"
              "    } obj;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor.\n", errout.str());

        check("Object::MemFunc() {\n"
              "    class LocalClass : public copy_protected {\n"
              "    public:\n"
              "        LocalClass() : copy_protected(1), dataLength_(0) {}\n"
              "        std::streamsize dataLength_;\n"
              "        double bitsInData_;\n"
              "    } obj;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor.\n", errout.str());

        check("Object::MemFunc() {\n"
              "    class LocalClass : ::copy_protected {\n"
              "    public:\n"
              "        LocalClass() : copy_protected(1), dataLength_(0) {}\n"
              "        std::streamsize dataLength_;\n"
              "        double bitsInData_;\n"
              "    } obj;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar21() { // ticket #2947
        check("class Fred {\n"
              "private:\n"
              "    int a[23];\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred() {\n"
              "    a[x::y] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar22() { // ticket #3043
        check("class Fred {\n"
              "  public:\n"
              "    Fred & operator=(const Fred &);\n"
              "    virtual Fred & clone(const Fred & other);\n"
              "    int x;\n"
              "};\n"
              "Fred & Fred::operator=(const Fred & other) {\n"
              "    return clone(other);\n"
              "}\n"
              "Fred & Fred::clone(const Fred & other) {\n"
              "    x = 0;\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred {\n"
              "  public:\n"
              "    Fred & operator=(const Fred &);\n"
              "    virtual Fred & clone(const Fred & other);\n"
              "    int x;\n"
              "};\n"
              "Fred & Fred::operator=(const Fred & other) {\n"
              "    return clone(other);\n"
              "}\n"
              "Fred & Fred::clone(const Fred & other) {\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::x' is not assigned a value in 'Fred::operator='.\n", errout.str());
    }

    void uninitVar23() { // ticket #3702
        check("class Fred {\n"
              "    int x;\n"
              "public:\n"
              "    Fred(struct A a, struct B b);\n"
              "    Fred(C c, struct D d);\n"
              "    Fred(struct E e, F f);\n"
              "    Fred(struct G, struct H);\n"
              "    Fred(I, J);\n"
              "};\n"
              "Fred::Fred(A a, B b) { }\n"
              "Fred::Fred(struct C c, D d) { }\n"
              "Fred::Fred(E e, struct F f) { }\n"
              "Fred::Fred(G g, H h) { }\n"
              "Fred::Fred(struct I i, struct J j) { }\n"
             );
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Fred::x' is not initialized in the constructor.\n"
                      "[test.cpp:11]: (warning) Member variable 'Fred::x' is not initialized in the constructor.\n"
                      "[test.cpp:12]: (warning) Member variable 'Fred::x' is not initialized in the constructor.\n"
                      "[test.cpp:13]: (warning) Member variable 'Fred::x' is not initialized in the constructor.\n"
                      "[test.cpp:14]: (warning) Member variable 'Fred::x' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar24() { // ticket #3190
        check("class Foo;\n"
              "class Bar;\n"
              "class Sub;\n"
              "class Foo { class Sub; };\n"
              "class Bar { class Sub; };\n"
              "class Bar::Sub {\n"
              "    int b;\n"
              "public:\n"
              "    Sub() { }\n"
              "    Sub(int);\n"
              "};\n"
              "Bar::Sub::Sub(int) { };\n"
              "class ::Foo::Sub {\n"
              "    int f;\n"
              "public:\n"
              "    ~Sub();\n"
              "    Sub();\n"
              "};\n"
              "::Foo::Sub::~Sub() { }\n"
              "::Foo::Sub::Sub() { }\n"
              "class Foo;\n"
              "class Bar;\n"
              "class Sub;\n", true);

        ASSERT_EQUALS("[test.cpp:9]: (warning, inconclusive) Member variable 'Sub::b' is not initialized in the constructor.\n"
                      "[test.cpp:12]: (warning) Member variable 'Sub::b' is not initialized in the constructor.\n"
                      "[test.cpp:20]: (warning) Member variable 'Sub::f' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar25() { // ticket #4789
        check("struct A {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    A(int x = 0, int y = 0, int z = 0);\n"
              "};\n"
              "A::A(int x = 0, int y = 0, int z = 0) { } \n"
              "struct B {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    B(int x = 0, int y = 0, int z = 0);\n"
              "};\n"
              "B::B(int x, int y, int z) { } \n"
              "struct C {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    C(int, int, int);\n"
              "};\n"
              "C::C(int x = 0, int y = 0, int z = 0) { } \n"
              "struct D {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    D(int, int, int);\n"
              "};\n"
              "D::D(int x, int y, int z) { } \n"
              "struct E {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    E(int x, int y, int z);\n"
              "};\n"
              "E::E(int, int, int) { } \n"
              "struct F {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    F(int x = 0, int y = 0, int z = 0);\n"
              "};\n"
              "F::F(int, int, int) { }\n", true);
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:7]: (warning) Member variable 'A::b' is not initialized in the constructor.\n"
                      "[test.cpp:7]: (warning) Member variable 'A::c' is not initialized in the constructor.\n"
                      "[test.cpp:14]: (warning) Member variable 'B::a' is not initialized in the constructor.\n"
                      "[test.cpp:14]: (warning) Member variable 'B::b' is not initialized in the constructor.\n"
                      "[test.cpp:14]: (warning) Member variable 'B::c' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'C::a' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'C::b' is not initialized in the constructor.\n"
                      "[test.cpp:21]: (warning) Member variable 'C::c' is not initialized in the constructor.\n"
                      "[test.cpp:28]: (warning) Member variable 'D::a' is not initialized in the constructor.\n"
                      "[test.cpp:28]: (warning) Member variable 'D::b' is not initialized in the constructor.\n"
                      "[test.cpp:28]: (warning) Member variable 'D::c' is not initialized in the constructor.\n"
                      "[test.cpp:35]: (warning) Member variable 'E::a' is not initialized in the constructor.\n"
                      "[test.cpp:35]: (warning) Member variable 'E::b' is not initialized in the constructor.\n"
                      "[test.cpp:35]: (warning) Member variable 'E::c' is not initialized in the constructor.\n"
                      "[test.cpp:42]: (warning) Member variable 'F::a' is not initialized in the constructor.\n"
                      "[test.cpp:42]: (warning) Member variable 'F::b' is not initialized in the constructor.\n"
                      "[test.cpp:42]: (warning) Member variable 'F::c' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar26() {
        check("class A {\n"
              "    int * v;\n"
              "    int sz;\n"
              "public:\n"
              "    A(int s) {\n"
              "        v = new int [sz = s];\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar27() {
        check("class A {\n"
              "    double d;\n"
              "public:\n"
              "    A() {\n"
              "        rtl::math::setNan(&d);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
        check("class A {\n"
              "    double d;\n"
              "public:\n"
              "    A() {\n"
              "        ::rtl::math::setNan(&d);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar28() {
        check("class Fred {\n"
              "    int i;\n"
              "    float f;\n"
              "public:\n"
              "    Fred() {\n"
              "        foo(1);\n"
              "        foo(1.0f);\n"
              "    }\n"
              "    void foo(int a) { i = a; }\n"
              "    void foo(float a) { f = a; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar29() {
        check("class A {\n"
              "    int i;\n"
              "public:\n"
              "    A() { foo(); }\n"
              "    void foo() const { };\n"
              "    void foo() { i = 0; }\n"
              "};\n"
              "class B {\n"
              "    int i;\n"
              "public:\n"
              "    B() { foo(); }\n"
              "    void foo() { i = 0; }\n"
              "    void foo() const { }\n"
              "};\n"
              "class C {\n"
              "    int i;\n"
              "public:\n"
              "    C() { foo(); }\n"
              "    void foo() const { i = 0; }\n"
              "    void foo() { }\n"
              "};\n"
              "class D {\n"
              "    int i;\n"
              "public:\n"
              "    D() { foo(); }\n"
              "	void foo() { }\n"
              "	void foo() const { i = 0; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:18]: (warning) Member variable 'C::i' is not initialized in the constructor.\n"
                      "[test.cpp:25]: (warning) Member variable 'D::i' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar30() { // ticket #6417
        check("namespace NS {\n"
              "    class MyClass {\n"
              "    public:\n"
              "        MyClass();\n"
              "        ~MyClass();\n"
              "    private:\n"
              "        bool SomeVar;\n"
              "    };\n"
              "}\n"
              "using namespace NS;\n"
              "MyClass::~MyClass() { }\n"
              "MyClass::MyClass() : SomeVar(false) { }\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray1() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() {}\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'John::name' is not initialized in the constructor.\n", errout.str());

        check("class John\n"
              "{\n"
              "public:\n"
              "    John() {John::name[0] = '\\0';}\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { strcpy(name, \"\"); }\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { }\n"
              "\n"
              "    double  operator[](const unsigned long i);\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A;\n"
              "class John\n"
              "{\n"
              "public:\n"
              "    John() { }\n"
              "    A a[5];\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A;\n"
              "class John\n"
              "{\n"
              "public:\n"
              "    John() { }\n"
              "    A *a[5];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'John::a' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarArray2() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { *name = 0; }\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // #5754
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() {*this->name = '\\0';}\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray3() {
        check("class John\n"
              "{\n"
              "private:\n"
              "    int a[100];\n"
              "    int b[100];\n"
              "\n"
              "public:\n"
              "    John()\n"
              "    {\n"
              "        memset(a,0,sizeof(a));\n"
              "        memset(b,0,sizeof(b));\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray4() {
        check("class John\n"
              "{\n"
              "private:\n"
              "    int a[100];\n"
              "    int b[100];\n"
              "\n"
              "public:\n"
              "    John()\n"
              "    {\n"
              "        if (snprintf(a,10,\"a\")) { }\n"
              "        if (snprintf(b,10,\"b\")) { }\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray5() {
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    Bar bars[10];\n"
              "public:\n"
              "    Foo()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray6() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo();\n"
              "    static const char STR[];\n"
              "};\n"
              "const char Foo::STR[] = \"abc\";\n"
              "Foo::Foo() { }");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray7() {
        check("class Foo\n"
              "{\n"
              "    int array[10];\n"
              "public:\n"
              "    Foo() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'Foo::array' is not initialized in the constructor.\n", errout.str());

        check("class Foo\n"
              "{\n"
              "    int array[10];\n"
              "public:\n"
              "    Foo() { memset(array, 0, sizeof(array)); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Foo\n"
              "{\n"
              "    int array[10];\n"
              "public:\n"
              "    Foo() { ::memset(array, 0, sizeof(array)); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray8() {
        check("class Foo {\n"
              "    char a[10];\n"
              "public:\n"
              "    Foo() { ::ZeroMemory(a); }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray2D() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { a[0][0] = 0; }\n"
              "\n"
              "private:\n"
              "    char a[2][2];\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray3D() {
        check("class John\n"
              "{\n"
              "private:\n"
              "    char a[2][2][2];\n"
              "public:\n"
              "    John() { a[0][0][0] = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarCpp11Init1() {
        check("class Foo {\n"
              "    std::vector<std::string> bar;\n"
              "public:\n"
              "    Foo()\n"
              "        : bar({\"a\", \"b\"})\n"
              "    {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarCpp11Init2() {
        check("class Fred {\n"
              "    struct Foo {\n"
              "        int a;\n"
              "        bool b;\n"
              "    };\n"
              "    Foo f;\n"
              "    float g;\n"
              "public:\n"
              "    Fred() : f{0, true} { }\n"
              "    float get() const\n"
              "};\n"
              "float Fred::get() const { return g; }");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Member variable 'Fred::g' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarStruct1() { // ticket #2172
        check("class A\n"
              "{\n"
              "private:\n"
              "    struct B {\n"
              "        std::string str1;\n"
              "        std::string str2;\n"
              "    }\n"
              "    struct B b;\n"
              "public:\n"
              "    A() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    struct B {\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    }\n"
              "    struct B b;\n"
              "public:\n"
              "    A() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'A::b' is not initialized in the constructor.\n", errout.str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    struct B {\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "        B() : str1(NULL), str2(NULL) { }\n"
              "    }\n"
              "    struct B b;\n"
              "public:\n"
              "    A() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarStruct2() { // ticket #838
        check("struct POINT\n"
              "{\n"
              "    int x;\n"
              "    int y;\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Member variable 'Fred::p' is not initialized in the constructor.\n", errout.str());

        check("struct POINT\n"
              "{\n"
              "    int x;\n"
              "    int y;\n"
              "    POINT();\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct POINT\n"
              "{\n"
              "    int x;\n"
              "    int y;\n"
              "    POINT() :x(0), y(0) { }\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // non static data-member initialization
        check("struct POINT\n"
              "{\n"
              "    int x=0;\n"
              "    int y=0;\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarUnion1() { // ticket #3196
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    union { int a; int b; };\n"
              "public:\n"
              "    Fred()\n"
              "    { a = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarUnion2() {
        // If the "data_type" is 0 it means union member "data" is invalid.
        // So it's ok to not initialize "data".
        // related forum: http://sourceforge.net/apps/phpbb/cppcheck/viewtopic.php?f=3&p=1806
        check("union Data { int id; int *ptr; };\n"
              "\n"
              "class Fred {\n"
              "private:\n"
              "    int data_type;\n"
              "    Data data;\n"
              "public:\n"
              "    Fred() : data_type(0)\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitMissingFuncDef() {
        // Unknown member function
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    void Init();"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function (friend class)
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    friend ABC;\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function (is Init a virtual function?)
        check("class Fred : private ABC\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        // Unknown non-member function
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        // Unknown non-member function
        check("class ABC { };\n"
              "class Fred : private ABC\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

    }

    void uninitVarEnum() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    enum abc {a,b,c};\n"
              "    Fred() {}\n"
              "private:\n"
              "    unsigned int i;\n"
              "};");

        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarStream() {
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    int foo;\n"
              "public:\n"
              "    explicit Foo(std::istream &in)\n"
              "    {\n"
              "        if(!(in >> foo))\n"
              "            throw 0;\n"
              "    }\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarTypedef() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    typedef int * pointer;\n"
              "    Foo() : a(0) {}\n"
              "    pointer a;\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarMemset() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    int * pointer;\n"
              "    Foo() { memset(this, 0, sizeof(*this)); }\n"
              "};");

        ASSERT_EQUALS("", errout.str());

        check("class Foo\n"
              "{\n"
              "public:\n"
              "    int * pointer;\n"
              "    Foo() { ::memset(this, 0, sizeof(*this)); }\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void privateCtor1() {
        check("class Foo {\n"
              "    int foo;\n"
              "    Foo() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void privateCtor2() {
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    int foo;\n"
              "    Foo() { }\n"
              "public:\n"
              "    explicit Foo(int _i) { }\n"
              "};");

        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Foo::foo' is not initialized in the constructor.\n", errout.str());
    }


    void function() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    A();\n"
              "    int* f(int*);\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{\n"
              "}\n"
              "\n"
              "int* A::f(int* p)\n"
              "{\n"
              "    return p;\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }


    void uninitVarHeader1() {
        check("#file \"fred.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int i;\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "#endfile");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarHeader2() {
        check("#file \"fred.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int i;\n"
              "public:\n"
              "    Fred() { }\n"
              "};\n"
              "#endfile");
        ASSERT_EQUALS("[fred.h:6]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarHeader3() {
        check("#file \"fred.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    mutable int i;\n"
              "public:\n"
              "    Fred() { }\n"
              "};\n"
              "#endfile");
        ASSERT_EQUALS("[fred.h:6]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }

    // Borland C++: No FP for published pointers - they are automatically initialized
    void uninitVarPublished() {
        check("class Fred\n"
              "{\n"
              "__published:\n"
              "    int *i;\n"
              "public:\n"
              "    Fred() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitOperator() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "    int *operator [] (int index) { return 0; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitFunction1() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(*this); }\n"
              "\n"
              "    static void init(Fred &f)\n"
              "    { f.d = 0; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(*this); }\n"
              "\n"
              "    static void init(Fred &f)\n"
              "    { }\n"
              "\n"
              "    double d;\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", "", errout.str());
    }

    void uninitFunction2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init(*this)); }\n"
              "\n"
              "    static bool init(Fred &f)\n"
              "    { f.d = 0; return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init(*this)); }\n"
              "\n"
              "    static bool init(Fred &f)\n"
              "    { return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", "",  errout.str());
    }

    void uninitFunction3() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init()); }\n"
              "\n"
              "    bool init()\n"
              "    { d = 0; return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init()); }\n"
              "\n"
              "    bool init()\n"
              "    { return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", errout.str());
    }

    void uninitFunction4() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(this); }\n"
              "\n"
              "    init(Fred *f)\n"
              "    { f.d = 0; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(this); }\n"
              "\n"
              "    init(Fred *f)\n"
              "    { }\n"
              "\n"
              "    double d;\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", "", errout.str());
    }

    void uninitFunction5() { // #4072 - FP about struct that is initialized in function
        check("struct Structure {\n"
              "    int C;\n"
              "};\n"
              "\n"
              "class A {\n"
              "    Structure B;\n"
              "public:\n"
              "    A() { Init( B ); };\n"
              "    void Init( Structure& S ) { S.C = 0; };\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitSameClassName() {
        check("class B\n"
              "{\n"
              "public:\n"
              "    B();\n"
              "    int j;\n"
              "};\n"
              "\n"
              "class A\n"
              "{\n"
              "    class B\n"
              "    {\n"
              "    public:\n"
              "        B();\n"
              "        int i;\n"
              "    };\n"
              "};\n"
              "\n"
              "A::B::B()\n"
              "{\n"
              "    i = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B();\n"
              "    int j;\n"
              "};\n"
              "\n"
              "class A\n"
              "{\n"
              "    class B\n"
              "    {\n"
              "    public:\n"
              "        B();\n"
              "        int i;\n"
              "    };\n"
              "};\n"
              "\n"
              "B::B()\n"
              "{\n"
              "}\n"
              "\n"
              "A::B::B()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[test.cpp:18]: (warning) Member variable 'B::j' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'B::i' is not initialized in the constructor.\n", errout.str());

        // Ticket #1700
        check("namespace n1\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo() : i(0) { }\n"
              "private:\n"
              "    int i;\n"
              "};\n"
              "}\n"
              "\n"
              "namespace n2\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo() { }\n"
              "};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("namespace n1\n"
              "{\n"
              "class Foo {\n"
              "public:\n"
              "    Foo();\n"
              "private:\n"
              "    int i;\n"
              "};\n"
              "}\n"
              "\n"
              "n1::Foo::Foo()\n"
              "{ }\n"
              "\n"
              "namespace n2\n"
              "{\n"
              "class Foo {\n"
              "public:\n"
              "    Foo() { }\n"
              "};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Member variable 'Foo::i' is not initialized in the constructor.\n", errout.str());

        check("namespace n1\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo();\n"
              "private:\n"
              "    int i;\n"
              "};\n"
              "}\n"
              "\n"
              "n1::Foo::Foo() : i(0)\n"
              "{ }\n"
              "\n"
              "namespace n2\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo() { }\n"
              "};\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitFunctionOverload() {
        // Ticket #1783 - overloaded "init" functions
        check("class A\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "\n"
              "public:\n"
              "    A()\n"
              "    {\n"
              "        init();\n"
              "    }\n"
              "\n"
              "    void init() { init(0); }\n"
              "\n"
              "    void init(int value)\n"
              "    { i = value; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "\n"
              "public:\n"
              "    A()\n"
              "    {\n"
              "        init();\n"
              "    }\n"
              "\n"
              "    void init() { init(0); }\n"
              "\n"
              "    void init(int value)\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'A::i' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarOperatorEqual() { // ticket #2415
        check("struct A {\n"
              "    int a;\n"
              "    A() { a=0; }\n"
              "    A(A const &a) { operator=(a); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    int a;\n"
              "    A() { a=0; }\n"
              "    A(A const &a) { operator=(a); }\n"
              "    A & operator = (const A & rhs) {\n"
              "        a = rhs.a;\n"
              "        return *this;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("struct A {\n"
              "    int a;\n"
              "    A() { a=0; }\n"
              "    A(A const &a) { operator=(a); }\n"
              "    A & operator = (const A & rhs) {\n"
              "        return *this;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning) Member variable 'A::a' is not assigned a value in 'A::operator='.\n", errout.str());
    }

    void uninitVarPointer() { // #3801
        check("struct A {\n"
              "    int a;\n"
              "};\n"
              "struct B {\n"
              "    A* a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'B::a' is not initialized in the constructor.\n", errout.str());

        check("struct A;\n"
              "struct B {\n"
              "    A* a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'B::a' is not initialized in the constructor.\n", errout.str());

        check("struct A;\n"
              "struct B {\n"
              "    const A* a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'B::a' is not initialized in the constructor.\n", errout.str());

        check("struct A;\n"
              "struct B {\n"
              "    A* const a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'B::a' is not initialized in the constructor.\n", errout.str());
    }

    void uninitConstVar() {
        check("struct A;\n"
              "struct B {\n"
              "    A* const a;\n"
              "    B() { }\n"
              "    B(B& b) { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'B::a' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning) Member variable 'B::a' is not initialized in the constructor.\n", errout.str());

        check("struct A;\n"
              "struct B {\n"
              "    A* const a;\n"
              "    B& operator=(const B& r) { }\n"
              "};");
        ASSERT_EQUALS("", errout.str()); // #3804

        check("struct B {\n"
              "    const int a;\n"
              "    B() { }\n"
              "    B(B& b) { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Member variable 'B::a' is not initialized in the constructor.\n"
                      "[test.cpp:4]: (warning) Member variable 'B::a' is not initialized in the constructor.\n", errout.str());

        check("struct B {\n"
              "    const int a;\n"
              "    B& operator=(const B& r) { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    // Ticket #5641 "Regression. Crash for 'C() _STLP_NOTHROW {}'"
    void constructors_crash1() {
        check("class C {\n"
              "public:\n"
              "  C() _STLP_NOTHROW {}\n"
              "  C(const C&) _STLP_NOTHROW {}\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestConstructors)
