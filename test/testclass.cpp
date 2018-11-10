/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include <tinyxml2.h>

#include "checkclass.h"
#include "library.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"


class TestClass : public TestFixture {
public:
    TestClass() : TestFixture("TestClass") {
    }

private:
    Settings settings0;
    Settings settings1;

    void run() override {
        settings0.addEnabled("style");
        settings1.addEnabled("warning");

        // Load std.cfg configuration
        {
            const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <memory>\n"
                                   "    <alloc init=\"false\">malloc</alloc>\n"
                                   "    <dealloc>free</dealloc>\n"
                                   "  </memory>\n"
                                   "</def>";
            tinyxml2::XMLDocument doc;
            doc.Parse(xmldata, sizeof(xmldata));
            settings0.library.load(doc);
            settings1.library.load(doc);
        }


        TEST_CASE(virtualDestructor1);      // Base class not found => no error
        TEST_CASE(virtualDestructor2);      // Base class doesn't have a destructor
        TEST_CASE(virtualDestructor3);      // Base class has a destructor, but it's not virtual
        TEST_CASE(virtualDestructor4);      // Derived class doesn't have a destructor => no error
        TEST_CASE(virtualDestructor5);      // Derived class has empty destructor => no error
        TEST_CASE(virtualDestructor6);      // only report error if base class pointer that points at derived class is deleted
        TEST_CASE(virtualDestructorProtected);
        TEST_CASE(virtualDestructorInherited);
        TEST_CASE(virtualDestructorTemplate);

        TEST_CASE(virtualDestructorInconclusive); // ticket # 5807

        TEST_CASE(copyConstructor1);
        TEST_CASE(copyConstructor2); // ticket #4458
        TEST_CASE(copyConstructor3); // defaulted/deleted
        TEST_CASE(copyConstructor4); // base class with private constructor
        TEST_CASE(noOperatorEq); // class with memory management should have operator eq
        TEST_CASE(noDestructor); // class with memory management should have destructor

        TEST_CASE(operatorEq1);
        TEST_CASE(operatorEq2);
        TEST_CASE(operatorEq3); // ticket #3051
        TEST_CASE(operatorEq4); // ticket #3114
        TEST_CASE(operatorEq5); // ticket #3296
        TEST_CASE(operatorEqRetRefThis1);
        TEST_CASE(operatorEqRetRefThis2); // ticket #1323
        TEST_CASE(operatorEqRetRefThis3); // ticket #1405
        TEST_CASE(operatorEqRetRefThis4); // ticket #1451
        TEST_CASE(operatorEqRetRefThis5); // ticket #1550
        TEST_CASE(operatorEqRetRefThis6); // ticket #2479
        TEST_CASE(operatorEqRetRefThis7); // ticket #5782 endless recursion
        TEST_CASE(operatorEqToSelf1);   // single class
        TEST_CASE(operatorEqToSelf2);   // nested class
        TEST_CASE(operatorEqToSelf3);   // multiple inheritance
        TEST_CASE(operatorEqToSelf4);   // nested class with multiple inheritance
        TEST_CASE(operatorEqToSelf5);   // ticket # 1233
        TEST_CASE(operatorEqToSelf6);   // ticket # 1550
        TEST_CASE(operatorEqToSelf7);
        TEST_CASE(operatorEqToSelf8);   // ticket #2179
        TEST_CASE(operatorEqToSelf9);   // ticket #2592

        TEST_CASE(memsetOnStruct);
        TEST_CASE(memsetVector);
        TEST_CASE(memsetOnClass);
        TEST_CASE(memsetOnInvalid);    // Ticket #5425: Crash upon invalid
        TEST_CASE(memsetOnStdPodType); // Ticket #5901 - std::uint8_t
        TEST_CASE(memsetOnFloat);      // Ticket #5421
        TEST_CASE(memsetOnUnknown);    // Ticket #7183
        TEST_CASE(mallocOnClass);

        TEST_CASE(this_subtraction);    // warn about "this-x"

        // can member function be made const
        TEST_CASE(const1);
        TEST_CASE(const2);
        TEST_CASE(const3);
        TEST_CASE(const4);
        TEST_CASE(const5); // ticket #1482
        TEST_CASE(const6); // ticket #1491
        TEST_CASE(const7);
        TEST_CASE(const8); // ticket #1517
        TEST_CASE(const9); // ticket #1515
        TEST_CASE(const10); // ticket #1522
        TEST_CASE(const11); // ticket #1529
        TEST_CASE(const12); // ticket #1552
        TEST_CASE(const13); // ticket #1519
        TEST_CASE(const14);
        TEST_CASE(const15);
        TEST_CASE(const16); // ticket #1551
        TEST_CASE(const17); // ticket #1552
        TEST_CASE(const18);
        TEST_CASE(const19); // ticket #1612
        TEST_CASE(const20); // ticket #1602
        TEST_CASE(const21); // ticket #1683
        TEST_CASE(const22);
        TEST_CASE(const23); // ticket #1699
        TEST_CASE(const24); // ticket #1708
        TEST_CASE(const25); // ticket #1724
        TEST_CASE(const26); // ticket #1847
        TEST_CASE(const27); // ticket #1882
        TEST_CASE(const28); // ticket #1883
        TEST_CASE(const29); // ticket #1922
        TEST_CASE(const30);
        TEST_CASE(const31);
        TEST_CASE(const32); // ticket #1905 - member array is assigned
        TEST_CASE(const33);
        TEST_CASE(const34); // ticket #1964
        TEST_CASE(const35); // ticket #2001
        TEST_CASE(const36); // ticket #2003
        TEST_CASE(const37); // ticket #2081 and #2085
        TEST_CASE(const38); // ticket #2135
        TEST_CASE(const39);
        TEST_CASE(const40); // ticket #2228
        TEST_CASE(const41); // ticket #2255
        TEST_CASE(const42); // ticket #2282
        TEST_CASE(const43); // ticket #2377
        TEST_CASE(const44); // ticket #2595
        TEST_CASE(const45); // ticket #2664
        TEST_CASE(const46); // ticket #2636
        TEST_CASE(const47); // ticket #2670
        TEST_CASE(const48); // ticket #2672
        TEST_CASE(const49); // ticket #2795
        TEST_CASE(const50); // ticket #2943
        TEST_CASE(const51); // ticket #3040
        TEST_CASE(const52); // ticket #3048
        TEST_CASE(const53); // ticket #3049
        TEST_CASE(const54); // ticket #3052
        TEST_CASE(const55);
        TEST_CASE(const56); // ticket #3149
        TEST_CASE(const57); // tickets #2669 and #2477
        TEST_CASE(const58); // ticket #2698
        TEST_CASE(const59); // ticket #4646
        TEST_CASE(const60); // ticket #3322
        TEST_CASE(const61); // ticket #5606
        TEST_CASE(const62); // ticket #5701
        TEST_CASE(const63); // ticket #5983
        TEST_CASE(const64); // ticket #6268
        TEST_CASE(const65); // ticket #8693
        TEST_CASE(const_handleDefaultParameters);
        TEST_CASE(const_passThisToMemberOfOtherClass);
        TEST_CASE(assigningPointerToPointerIsNotAConstOperation);
        TEST_CASE(assigningArrayElementIsNotAConstOperation);
        TEST_CASE(constoperator1);  // operator< can often be const
        TEST_CASE(constoperator2);  // operator<<
        TEST_CASE(constoperator3);
        TEST_CASE(constoperator4);
        TEST_CASE(constoperator5); // ticket #3252
        TEST_CASE(constoperator6); // ticket #8669
        TEST_CASE(constincdec);     // increment/decrement => non-const
        TEST_CASE(constassign1);
        TEST_CASE(constassign2);
        TEST_CASE(constincdecarray);     // increment/decrement array element => non-const
        TEST_CASE(constassignarray);
        TEST_CASE(constReturnReference);
        TEST_CASE(constDelete);     // delete member variable => not const
        TEST_CASE(constLPVOID);     // a function that returns LPVOID can't be const
        TEST_CASE(constFunc); // a function that calls const functions can be const
        TEST_CASE(constVirtualFunc);
        TEST_CASE(constIfCfg);  // ticket #1881 - fp when there are #if
        TEST_CASE(constFriend); // ticket #1921 - fp for friend function
        TEST_CASE(constUnion);  // ticket #2111 - fp when there is a union
        TEST_CASE(constArrayOperator); // #4406
        TEST_CASE(constRangeBasedFor); // #5514
        TEST_CASE(const_shared_ptr);
        TEST_CASE(constPtrToConstPtr);

        TEST_CASE(initializerListOrder);
        TEST_CASE(initializerListUsage);
        TEST_CASE(selfInitialization);

        TEST_CASE(virtualFunctionCallInConstructor);
        TEST_CASE(pureVirtualFunctionCall);
        TEST_CASE(pureVirtualFunctionCallOtherClass);
        TEST_CASE(pureVirtualFunctionCallWithBody);
        TEST_CASE(pureVirtualFunctionCallPrevented);

        TEST_CASE(duplInheritedMembers);
        TEST_CASE(explicitConstructors);
        TEST_CASE(copyCtorAndEqOperator);

        TEST_CASE(unsafeClassDivZero);

        TEST_CASE(override1);
        TEST_CASE(overrideCVRefQualifiers);
    }

    void checkCopyCtorAndEqOperator(const char code[]) {
        // Clear the error log
        errout.str("");
        Settings settings;
        settings.addEnabled("warning");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.checkCopyCtorAndEqOperator();
    }

    void copyCtorAndEqOperator() {
        checkCopyCtorAndEqOperator("class A \n"
                                   "{ \n"
                                   "    A(const A& other) { } \n"
                                   "    A& operator=(const A& other) { return *this; }\n"
                                   "};");
        ASSERT_EQUALS("", errout.str());


        checkCopyCtorAndEqOperator("class A \n"
                                   "{ \n"
                                   "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyCtorAndEqOperator("class A \n"
                                   "{ \n"
                                   "    A(const A& other) { } \n"
                                   "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyCtorAndEqOperator("class A \n"
                                   "{ \n"
                                   "    A& operator=(const A& other) { return *this; }\n"
                                   "};");
        ASSERT_EQUALS("", errout.str());


        checkCopyCtorAndEqOperator("class A \n"
                                   "{ \n"
                                   "    A(const A& other) { } \n"
                                   "    int x;\n"
                                   "};");
        TODO_ASSERT_EQUALS("[test.cpp:1]: (warning) The class 'A' has 'copy constructor' but lack of 'operator='.\n", "", errout.str());
        // TODO the error message should be clarified. It should say something like 'copy constructor is empty and will not assign i and therefore the behaviour is different to the default assignment operator'

        checkCopyCtorAndEqOperator("class A \n"
                                   "{ \n"
                                   "    A& operator=(const A& other) { return *this; }\n"
                                   "    int x;\n"
                                   "};");
        TODO_ASSERT_EQUALS("[test.cpp:1]: (warning) The class 'A' has 'operator=' but lack of 'copy constructor'.\n", "", errout.str());
        // TODO the error message should be clarified. It should say something like 'assignment operator does not assign i and therefore the behaviour is different to the default copy constructor'

        checkCopyCtorAndEqOperator("class A \n"
                                   "{ \n"
                                   "    A& operator=(const int &x) { this->x = x; return *this; }\n"
                                   "    int x;\n"
                                   "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyCtorAndEqOperator("class A {\n"
                                   "public:\n"
                                   "    A() : x(0) { }\n"
                                   "    A(const A & a) { x = a.x; }\n"
                                   "    A & operator = (const A & a) {\n"
                                   "        x = a.x;\n"
                                   "        return *this;\n"
                                   "    }\n"
                                   "private:\n"
                                   "    int x;\n"
                                   "};\n"
                                   "class B : public A {\n"
                                   "public:\n"
                                   "    B() { }\n"
                                   "    B(const B & b) :A(b) { }\n"
                                   "private:\n"
                                   "    static int i;\n"
                                   "};");
        ASSERT_EQUALS("", errout.str());

        // #7987 - Don't show warning when there is a move constructor
        checkCopyCtorAndEqOperator("struct S {\n"
                                   "  std::string test;\n"
                                   "  S(S&& s) : test(std::move(s.test)) { }\n"
                                   "  S& operator = (S &&s) {\n"
                                   "    test = std::move(s.test);\n"
                                   "    return *this;\n"
                                   "  }\n"
                                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // #8337 - False positive in copy constructor detection
        checkCopyCtorAndEqOperator("struct StaticListNode {\n"
                                   "  StaticListNode(StaticListNode*& prev) : m_next(0) {}\n"
                                   "  StaticListNode* m_next;\n"
                                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void checkExplicitConstructors(const char code[]) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings0, this);
        checkClass.checkExplicitConstructors();
    }

    void explicitConstructors() {
        checkExplicitConstructors("class Class {\n"
                                  "    Class() = delete;\n"
                                  "    Class(const Class& other) { }\n"
                                  "    Class(Class&& other) { }\n"
                                  "    explicit Class(int i) { }\n"
                                  "    explicit Class(const std::string&) { }\n"
                                  "    Class(int a, int b) { }\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkExplicitConstructors("class Class {\n"
                                  "    Class() = delete;\n"
                                  "    explicit Class(const Class& other) { }\n"
                                  "    explicit Class(Class&& other) { }\n"
                                  "    virtual int i() = 0;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkExplicitConstructors("class Class {\n"
                                  "    Class() = delete;\n"
                                  "    Class(const Class& other) = delete;\n"
                                  "    Class(Class&& other) = delete;\n"
                                  "    virtual int i() = 0;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkExplicitConstructors("class Class {\n"
                                  "    Class(int i) { }\n"
                                  "};");
        ASSERT_EQUALS("[test.cpp:2]: (style) Class 'Class' has a constructor with 1 argument that is not explicit.\n", errout.str());

        checkExplicitConstructors("class Class {\n"
                                  "    Class(const Class& other) { }\n"
                                  "    virtual int i() = 0;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkExplicitConstructors("class Class {\n"
                                  "    Class(Class&& other) { }\n"
                                  "    virtual int i() = 0;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        // #6585
        checkExplicitConstructors("class Class {\n"
                                  "    private: Class(const Class&);\n"
                                  "    virtual int i() = 0;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkExplicitConstructors("class Class {\n"
                                  "    public: Class(const Class&);\n"
                                  "    virtual int i() = 0;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        // #7465: Error properly reported in templates
        checkExplicitConstructors("template <class T> struct Test {\n"
                                  "  Test(int) : fData(0) {}\n"
                                  "  T fData;\n"
                                  "};\n"
                                  "int main() {\n"
                                  "  Test <int> test;\n"
                                  "  return 0;\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Struct 'Test < int >' has a constructor with 1 argument that is not explicit.\n", errout.str());

        // #7465: No error for copy or move constructors
        checkExplicitConstructors("template <class T> struct Test {\n"
                                  "  Test() : fData(0) {}\n"
                                  "  Test (const Test<T>& aOther) : fData(aOther.fData) {}\n"
                                  "  Test (Test<T>&& aOther) : fData(std::move(aOther.fData)) {}\n"
                                  "  T fData;\n"
                                  "};\n"
                                  "int main() {\n"
                                  "  Test <int> test;\n"
                                  "  return 0;\n"
                                  "}");
        ASSERT_EQUALS("", errout.str());

        // #8600
        checkExplicitConstructors("struct A { struct B; };\n"
                                  "struct A::B {\n"
                                  "    B() = default;\n"
                                  "    B(const B&) {}\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());
    }

    void checkDuplInheritedMembers(const char code[]) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings1, this);
        checkClass.checkDuplInheritedMembers();
    }

    void duplInheritedMembers() {
        checkDuplInheritedMembers("class Base {\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct Derived : Base {\n"
                                  "   int x;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkDuplInheritedMembers("class Base {\n"
                                  "   protected:\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct Derived : Base {\n"
                                  "   int x;\n"
                                  "};");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (warning) The struct 'Derived' defines member variable with name 'x' also defined in its parent class 'Base'.\n", errout.str());

        checkDuplInheritedMembers("class Base {\n"
                                  "   protected:\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct Derived : public Base {\n"
                                  "   int x;\n"
                                  "};");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:6]: (warning) The struct 'Derived' defines member variable with name 'x' also defined in its parent class 'Base'.\n", errout.str());

        checkDuplInheritedMembers("class Base0 {\n"
                                  "   int x;\n"
                                  "};\n"
                                  "class Base1 {\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct Derived : Base0, Base1 {\n"
                                  "   int x;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkDuplInheritedMembers("class Base0 {\n"
                                  "   protected:\n"
                                  "   int x;\n"
                                  "};\n"
                                  "class Base1 {\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct Derived : Base0, Base1 {\n"
                                  "   int x;\n"
                                  "};");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:9]: (warning) The struct 'Derived' defines member variable with name 'x' also defined in its parent class 'Base0'.\n", errout.str());

        checkDuplInheritedMembers("class Base0 {\n"
                                  "   protected:\n"
                                  "   int x;\n"
                                  "};\n"
                                  "class Base1 {\n"
                                  "   public:\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct Derived : Base0, Base1 {\n"
                                  "   int x;\n"
                                  "};");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:10]: (warning) The struct 'Derived' defines member variable with name 'x' also defined in its parent class 'Base0'.\n"
                      "[test.cpp:7] -> [test.cpp:10]: (warning) The struct 'Derived' defines member variable with name 'x' also defined in its parent class 'Base1'.\n", errout.str());

        checkDuplInheritedMembers("class Base {\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct Derived : Base {\n"
                                  "   int y;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkDuplInheritedMembers("class A {\n"
                                  "   int x;\n"
                                  "};\n"
                                  "struct B {\n"
                                  "   int x;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        // Unknown 'Base' class
        checkDuplInheritedMembers("class Derived : public UnknownBase {\n"
                                  "  int x;\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        checkDuplInheritedMembers("class Base {\n"
                                  "   int x;\n"
                                  "};\n"
                                  "class Derived : public Base {\n"
                                  "};");
        ASSERT_EQUALS("", errout.str());

        // #6692
        checkDuplInheritedMembers("namespace test1 {\n"
                                  "   struct SWibble{};\n"
                                  "   typedef SWibble wibble;\n"
                                  "}\n"
                                  "namespace test2 {\n"
                                  "   struct SWibble : public test1::wibble {\n"
                                  "   int Value;\n"
                                  "   };\n"
                                  "}");
        ASSERT_EQUALS("", errout.str());

    }

    void checkCopyConstructor(const char code[]) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings0, this);
        checkClass.copyconstructors();
    }

    void copyConstructor1() {
        checkCopyConstructor("class F\n"
                             "{\n"
                             "   public:\n"
                             "   char *c,*p,*d;\n"
                             "   F(const F &f) : p(f.p), c(f.c)\n"
                             "   {\n"
                             "      p=(char *)malloc(strlen(f.p)+1);\n"
                             "      strcpy(p,f.p);\n"
                             "   }\n"
                             "   F(char *str)\n"
                             "   {\n"
                             "      p=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(p,str);\n"
                             "   }\n"
                             "   F&operator=(const F&);\n"
                             "   ~F();\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:5]: (style) Value of pointer 'p', which points to allocated memory, is copied in copy constructor instead of allocating new memory.\n", errout.str());

        checkCopyConstructor("class F {\n"
                             "   char *p;\n"
                             "   F(const F &f) {\n"
                             "      p = f.p;\n"
                             "   }\n"
                             "   F(char *str) {\n"
                             "      p = malloc(strlen(str)+1);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Value of pointer 'p', which points to allocated memory, is copied in copy constructor instead of allocating new memory.\n"
                           "[test.cpp:3] -> [test.cpp:7]: (warning) Copy constructor does not allocate memory for member 'p' although memory has been allocated in other constructors.\n",
                           "[test.cpp:4]: (style) Value of pointer 'p', which points to allocated memory, is copied in copy constructor instead of allocating new memory.\n"
                           , errout.str());

        checkCopyConstructor("class F\n"
                             "{\n"
                             "   public:\n"
                             "   char *c,*p,*d;\n"
                             "   F(const F &f) :p(f.p)\n"
                             "   {\n"
                             "   }\n"
                             "   F(char *str)\n"
                             "   {\n"
                             "      p=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(p,str);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (style) Value of pointer 'p', which points to allocated memory, is copied in copy constructor instead of allocating new memory.\n"
                           "[test.cpp:5] -> [test.cpp:10]: (warning) Copy constructor does not allocate memory for member 'p' although memory has been allocated in other constructors.\n",
                           "[test.cpp:5]: (style) Value of pointer 'p', which points to allocated memory, is copied in copy constructor instead of allocating new memory.\n"
                           , errout.str());

        checkCopyConstructor("class kalci\n"
                             "{\n"
                             "   public:\n"
                             "   char *c,*p,*d;\n"
                             "   kalci()\n"
                             "   {\n"
                             "      p=(char *)malloc(100);\n"
                             "      strcpy(p,\"hello\");\n"
                             "      c=(char *)malloc(100);\n"
                             "      strcpy(p,\"hello\");\n"
                             "      d=(char *)malloc(100);\n"
                             "      strcpy(p,\"hello\");\n"
                             "   }\n"
                             "   kalci(const kalci &f)\n"
                             "   {\n"
                             "      p=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(p,f.p);\n"
                             "      c=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(p,f.p);\n"
                             "      d=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(p,f.p);\n"
                             "   }\n"
                             "   ~kalci();\n"
                             "   kalci& operator=(const kalci&kalci);\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("class F\n"
                             "{\n"
                             "   public:\n"
                             "   char *c,*p,*d;\n"
                             "   F(char *str,char *st,char *string)\n"
                             "   {\n"
                             "      p=(char *)malloc(100);\n"
                             "      strcpy(p,str);\n"
                             "      c=(char *)malloc(100);\n"
                             "      strcpy(p,st);\n"
                             "      d=(char *)malloc(100);\n"
                             "      strcpy(p,string);\n"
                             "   }\n"
                             "   F(const F &f)\n"
                             "   {\n"
                             "      p=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(p,f.p);\n"
                             "      c=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(p,f.p);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        TODO_ASSERT_EQUALS("[test.cpp:14] -> [test.cpp:11]: (warning) Copy constructor does not allocate memory for member 'd' although memory has been allocated in other constructors.\n", "", errout.str());

        checkCopyConstructor("class F {\n"
                             "   char *c;\n"
                             "   F(char *str,char *st,char *string) {\n"
                             "      p=(char *)malloc(100);\n"
                             "   }\n"
                             "   F(const F &f)\n"
                             "      : p(malloc(size))\n"
                             "   {\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("class F {\n"
                             "   char *c;\n"
                             "   F(char *str,char *st,char *string)\n"
                             "      : p(malloc(size))\n"
                             "   {\n"
                             "   }\n"
                             "   F(const F &f)\n"
                             "   {\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        TODO_ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:4]: (warning) Copy constructor does not allocate memory for member 'd' although memory has been allocated in other constructors.\n", "", errout.str());

        checkCopyConstructor("class F\n"
                             "{\n"
                             "   public:\n"
                             "   char *c,*p,*d;\n"
                             "   F()\n"
                             "   {\n"
                             "      p=(char *)malloc(100);\n"
                             "      c=(char *)malloc(100);\n"
                             "      d=(char*)malloc(100);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:8]: (style) Class 'F' does not have a copy constructor which is recommended since it has dynamic memory/resource allocation(s).\n", errout.str());

        checkCopyConstructor("class F\n"
                             "{\n"
                             "   public:\n"
                             "   char *c;\n"
                             "   const char *p,*d;\n"
                             "   F(char *str,char *st,char *string)\n"
                             "   {\n"
                             "      p=str;\n"
                             "      d=st;\n"
                             "      c=(char *)malloc(strlen(string)+1);\n"
                             "      strcpy(d,string);\n"
                             "   }\n"
                             "   F(const F &f)\n"
                             "   {\n"
                             "      p=f.p;\n"
                             "      d=f.d;\n"
                             "      c=(char *)malloc(strlen(str)+1);\n"
                             "      strcpy(d,f.p);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("class F : E\n"
                             "{\n"
                             "   char *p;\n"
                             "   F() {\n"
                             "      p = malloc(100);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("class E { E(E&); };\n" // non-copyable
                             "class F : E\n"
                             "{\n"
                             "   char *p;\n"
                             "   F() {\n"
                             "      p = malloc(100);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("class E {};\n"
                             "class F : E {\n"
                             "   char *p;\n"
                             "   F() {\n"
                             "      p = malloc(100);\n"
                             "   }\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'F' does not have a copy constructor which is recommended since it has dynamic memory/resource allocation(s).\n", errout.str());

        checkCopyConstructor("class F {\n"
                             "   char *p;\n"
                             "   F() {\n"
                             "      p = malloc(100);\n"
                             "   }\n"
                             "   F(F& f);\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("class F {\n"
                             "   char *p;\n"
                             "   F() : p(malloc(100)) {}\n"
                             "   ~F();\n"
                             "   F& operator=(const F&f);\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Class 'F' does not have a copy constructor which is recommended since it has dynamic memory/resource allocation(s).\n", errout.str());

        // #7198
        checkCopyConstructor("struct F {\n"
                             "   static char* c;\n"
                             "   F() {\n"
                             "      p = malloc(100);\n"
                             "   }\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void copyConstructor2() { // ticket #4458
        checkCopyConstructor("template <class _Tp>\n"
                             "class Vector\n"
                             "{\n"
                             "public:\n"
                             "    Vector() {\n"
                             "        _M_finish = new _Tp[ 42 ];\n"
                             "    }\n"
                             "    Vector( const Vector<_Tp>& v ) {\n"
                             "    }\n"
                             "     ~Vector();\n"
                             "     Vector& operator=(const Vector&v);\n"
                             "    _Tp* _M_finish;\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void copyConstructor3() {
        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f) = delete;\n"
                             "   F&operator=(const F &f);\n"
                             "   ~F();\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f) = default;\n"
                             "   F&operator=(const F &f);\n"
                             "   ~F();\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Struct 'F' has dynamic memory/resource allocation(s). The copy constructor is explicitly defaulted but the default copy constructor does not work well. It is recommended to define or delete the copy constructor.\n", errout.str());
    }

    void copyConstructor4() {
        checkCopyConstructor("class noncopyable {\n"
                             "protected:\n"
                             "    noncopyable() {}\n"
                             "    ~noncopyable() {}\n"
                             "\n"
                             "private:\n"
                             "    noncopyable( const noncopyable& );\n"
                             "    const noncopyable& operator=( const noncopyable& );\n"
                             "};\n"
                             "\n"
                             "class Base : private noncopyable {};\n"
                             "\n"
                             "class Foo : public Base {\n"
                             "public:\n"
                             "    Foo() : m_ptr(new int) {}\n"
                             "    ~Foo() { delete m_ptr; }\n"
                             "private:\n"
                             "    int* m_ptr;\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void noOperatorEq() {
        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f);\n"
                             "   ~F();\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Struct 'F' does not have a operator= which is recommended since it has dynamic memory/resource allocation(s).\n", errout.str());

        // defaulted operator=
        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f);\n"
                             "   F &operator=(const F &f) = default;\n"
                             "   ~F();\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Struct 'F' has dynamic memory/resource allocation(s). The operator= is explicitly defaulted but the default operator= does not work well. It is recommended to define or delete the operator=.\n", errout.str());

        // deleted operator=
        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f);\n"
                             "   F &operator=(const F &f) = delete;\n"
                             "   ~F();\n"
                             "};");
        ASSERT_EQUALS("", errout.str());

        // base class deletes operator=
        checkCopyConstructor("struct F : NonCopyable {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f);\n"
                             "   ~F();\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void noDestructor() {
        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f);\n"
                             "   F&operator=(const F&);"
                             "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Struct 'F' does not have a destructor which is recommended since it has dynamic memory/resource allocation(s).\n", errout.str());

        checkCopyConstructor("struct F {\n"
                             "   C* c;\n"
                             "   F() { c = new C; }\n"
                             "   F(const F &f);\n"
                             "   F&operator=(const F&);"
                             "};");
        ASSERT_EQUALS("", errout.str());

        checkCopyConstructor("struct Data { int x; int y; };\n"
                             "struct F {\n"
                             "   Data* c;\n"
                             "   F() { c = new Data; }\n"
                             "   F(const F &f);\n"
                             "   F&operator=(const F&);"
                             "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Struct 'F' does not have a destructor which is recommended since it has dynamic memory/resource allocation(s).\n", errout.str());

        // defaulted destructor
        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f);\n"
                             "   F &operator=(const F &f);\n"
                             "   ~F() = default;\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Struct 'F' has dynamic memory/resource allocation(s). The destructor is explicitly defaulted but the default destructor does not work well. It is recommended to define the destructor.\n", errout.str());

        // deleted destructor
        checkCopyConstructor("struct F {\n"
                             "   char* c;\n"
                             "   F() { c = malloc(100); }\n"
                             "   F(const F &f);\n"
                             "   F &operator=(const F &f);\n"
                             "   ~F() = delete;\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    // Check the operator Equal
    void checkOpertorEq(const char code[]) {
        // Clear the error log
        errout.str("");

        settings0.inconclusive = true;

        // Tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings0, this);
        checkClass.operatorEq();
    }

    void operatorEq1() {
        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}"
                       "    void operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'.\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}"
                       "    void operator=(const A&)=delete;\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}"
                       "    void operator=(A&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'.\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "private:\n"
                       "    void operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "protected:\n"
                       "    void operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "private:\n"
                       "    void operator=(const A&)=delete;\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "    void operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}\n"
                       "private:\n"
                       "    void operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void operator=(const A&);\n"
                       "};\n"
                       "class B\n"
                       "{\n"
                       "public:\n"
                       "    void operator=(const B&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'.\n"
                      "[test.cpp:9]: (style) 'B::operator=' should return 'B &'.\n", errout.str());

        checkOpertorEq("struct A\n"
                       "{\n"
                       "    void operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'A::operator=' should return 'A &'.\n", errout.str());

        checkOpertorEq("struct A\n"
                       "{\n"
                       "    void operator=(const A&)=delete;\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        // Ticket #7017
        checkOpertorEq("template<class T> struct X {\n"
                       "  inline X(const X& Rhs);\n"
                       "  inline X<T>& operator =(const X& Rhs);\n"
                       "};");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEq2() {
        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void * operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'.\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    A * operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'.\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    const A & operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'.\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    B & operator=(const A&);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'.\n", errout.str());
    }

    void operatorEq3() { // ticket #3051
        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    A * operator=(const A*);\n"
                       "};");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEq4() { // ticket #3114 (infinite loop)
        checkOpertorEq("struct A {\n"
                       "    A& operator=(A const& a) { return operator=(&a); }\n"
                       "    A& operator=(const A*) { return *this; }\n"
                       "};");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEq5() { // ticket #3296 (virtual operator)
        checkOpertorEq(
            "class A {\n"
            "    virtual A& operator=(const A &a) {return *this};\n"
            "};");
        ASSERT_EQUALS("", errout.str());
    }

    // Check that operator Equal returns reference to this
    void checkOpertorEqRetRefThis(const char code[]) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings0, this);
        checkClass.operatorEqRetRefThis();
    }

    void operatorEqRetRefThis1() {
        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return a; }\n"
            "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return a; }");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a) { return a; }");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return *this; }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return b; }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("[test.cpp:7]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return *this; }");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return b; }");
        ASSERT_EQUALS("[test.cpp:10]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class B;\n"
            "};\n"
            "class A::B\n"
            "{\n"
            "  B & operator=(const B & b) { return b; }\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class B;\n"
            "};\n"
            "class A::B\n"
            "{\n"
            "  B & operator=(const B &);\n"
            "};\n"
            "A::B & A::B::operator=(const A::B & b) { return b; }\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class B;\n"
            "};\n"
            "class A::B\n"
            "{\n"
            "  A::B & operator=(const A::B & b) { return b; }\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class B;\n"
            "};\n"
            "class A::B\n"
            "{\n"
            "  A::B & operator=(const A::B &);\n"
            "};\n"
            "A::B & A::B::operator=(const A::B & b) { return b; }\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace A {\n"
            "    class B;\n"
            "}\n"
            "class A::B\n"
            "{\n"
            "  B & operator=(const B & b) { return b; }\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace A {\n"
            "    class B;\n"
            "}\n"
            "class A::B\n"
            "{\n"
            "  B & operator=(const B &);\n"
            "};\n"
            "A::B & A::B::operator=(const A::B & b) { return b; }\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace A {\n"
            "    class B;\n"
            "}\n"
            "class A::B\n"
            "{\n"
            "  A::B & operator=(const A::B & b) { return b; }\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace A {\n"
            "    class B;\n"
            "}\n"
            "class A::B\n"
            "{\n"
            "  A::B & operator=(const A::B &);\n"
            "};\n"
            "A::B & A::B::operator=(const A::B & b) { return b; }\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());
    }

    void operatorEqRetRefThis2() {
        // ticket # 1323
        checkOpertorEqRetRefThis(
            "class szp\n"
            "{\n"
            "  szp &operator =(int *other) {}\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class szp\n"
            "{\n"
            "  szp &operator =(int *other);\n"
            "};\n"
            "szp &szp::operator =(int *other) {}");
        ASSERT_EQUALS("[test.cpp:5]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace NS {\n"
            "    class szp;\n"
            "}\n"
            "class NS::szp\n"
            "{\n"
            "  szp &operator =(int *other) {}\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace NS {\n"
            "    class szp;\n"
            "}\n"
            "class NS::szp\n"
            "{\n"
            "  szp &operator =(int *other);\n"
            "};\n"
            "NS::szp &NS::szp::operator =(int *other) {}");
        ASSERT_EQUALS("[test.cpp:8]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace NS {\n"
            "    class szp;\n"
            "}\n"
            "class NS::szp\n"
            "{\n"
            "  NS::szp &operator =(int *other) {}\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "namespace NS {\n"
            "    class szp;\n"
            "}\n"
            "class NS::szp\n"
            "{\n"
            "  NS::szp &operator =(int *other);\n"
            "};\n"
            "NS::szp &NS::szp::operator =(int *other) {}");
        ASSERT_EQUALS("[test.cpp:8]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class szp;\n"
            "};\n"
            "class A::szp\n"
            "{\n"
            "  szp &operator =(int *other) {}\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class szp;\n"
            "};\n"
            "class A::szp\n"
            "{\n"
            "  szp &operator =(int *other);\n"
            "};\n"
            "A::szp &A::szp::operator =(int *other) {}");
        ASSERT_EQUALS("[test.cpp:8]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class szp;\n"
            "};\n"
            "class A::szp\n"
            "{\n"
            "  A::szp &operator =(int *other) {}\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "    class szp;\n"
            "};\n"
            "class A::szp\n"
            "{\n"
            "  A::szp &operator =(int *other);\n"
            "};\n"
            "A::szp &A::szp::operator =(int *other) {}");
        ASSERT_EQUALS("[test.cpp:8]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());
    }

    void operatorEqRetRefThis3() {
        // ticket # 1405
        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  inline A &operator =(int *other) { return (*this); };\n"
            "  inline A &operator =(long *other) { return (*this = 0); };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  A &operator =(int *other);\n"
            "  A &operator =(long *other);\n"
            "};\n"
            "A &A::operator =(int *other) { return (*this); };\n"
            "A &A::operator =(long *other) { return (*this = 0); };");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  inline A &operator =(int *other) { return (*this); };\n"
            "  inline A &operator =(long *other) { return operator = (*(int *)other); };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  A &operator =(int *other);\n"
            "  A &operator =(long *other);\n"
            "};\n"
            "A &A::operator =(int *other) { return (*this); };\n"
            "A &A::operator =(long *other) { return operator = (*(int *)other); };");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  A &operator =(int *other);\n"
            "  A &operator =(long *other);\n"
            "};\n"
            "A &A::operator =(int *other) { return (*this); };\n"
            "A &A::operator =(long *other) { return this->operator = (*(int *)other); };");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqRetRefThis4() {
        // ticket # 1451
        checkOpertorEqRetRefThis(
            "P& P::operator = (const P& pc)\n"
            "{\n"
            "  return (P&)(*this += pc);\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqRetRefThis5() {
        // ticket # 1550
        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "    A & operator=(const A &a) { }\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "protected:\n"
            "    A & operator=(const A &a) {}\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "private:\n"
            "    A & operator=(const A &a) {}\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should return reference to 'this' instance.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "    A & operator=(const A &a) {\n"
            "        rand();\n"
            "        throw std::exception();\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should either return reference to 'this' instance or be declared private and left unimplemented.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "    A & operator=(const A &a) {\n"
            "        rand();\n"
            "        abort();\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should either return reference to 'this' instance or be declared private and left unimplemented.\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A :: operator=(const A &a) { }");
        ASSERT_EQUALS("[test.cpp:5]: (error) No 'return' statement in non-void function causes undefined behavior.\n", errout.str());
    }

    void operatorEqRetRefThis6() { // ticket #2478 (segmentation fault)
        checkOpertorEqRetRefThis(
            "class UString {\n"
            "public:\n"
            "    UString& assign( const char* c_str );\n"
            "    UString& operator=( const UString& s );\n"
            "};\n"
            "UString& UString::assign( const char* c_str ) {\n"
            "    std::string tmp( c_str );\n"
            "    return assign( tmp );\n"
            "}\n"
            "UString& UString::operator=( const UString& s ) {\n"
            "    return assign( s );\n"
            "}");
    }

    void operatorEqRetRefThis7() { // ticket #5782 Endless recursion in CheckClass::checkReturnPtrThis()
        checkOpertorEqRetRefThis(
            "class basic_fbstring {\n"
            "  basic_fbstring& operator=(int il) {\n"
            "    return assign();\n"
            "  }\n"
            "  basic_fbstring& assign() {\n"
            "    return replace();\n"
            "  }\n"
            "  basic_fbstring& replaceImplDiscr() {\n"
            "    return replace();\n"
            "  }\n"
            "  basic_fbstring& replace() {\n"
            "    return replaceImplDiscr();\n"
            "  }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Check that operator Equal checks for assignment to self
    void checkOpertorEqToSelf(const char code[]) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings1, this);
        checkClass.operatorEqToSelf();
    }

    void operatorEqToSelf1() {
        // this test has an assignment test but it is not needed
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { if (&a != this) { } return *this; }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but it is not needed
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test and has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if (&a != this)\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this class needs an assignment test but doesn't have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("[test.cpp:5]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());

        // this test has an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { if (&a != this) { } return *this; }");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test and has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if (&a != this)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but doesn’t have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(a.s);\n"
            "    return *this;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());

        // ticket #1224
        checkOpertorEqToSelf(
            "const SubTree &SubTree::operator= (const SubTree &b)\n"
            "{\n"
            "    CodeTree *oldtree = tree;\n"
            "    tree = new CodeTree(*b.tree);\n"
            "    delete oldtree;\n"
            "    return *this;\n"
            "}\n"
            "const SubTree &SubTree::operator= (const CodeTree &b)\n"
            "{\n"
            "    CodeTree *oldtree = tree;\n"
            "    tree = new CodeTree(b);\n"
            "    delete oldtree;\n"
            "    return *this;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

    }

    void operatorEqToSelf2() {
        // this test has an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { if (&b != this) { } return *this; }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return *this; }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char *s;\n"
            "        B & operator=(const B &b)\n"
            "        {\n"
            "            if (&b != this)\n"
            "            {\n"
            "            }\n"
            "            return *this;\n"
            "        }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but doesn't have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char *s;\n"
            "        B & operator=(const B &b)\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(b.s);\n"
            "            return *this;\n"
            "        }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("[test.cpp:8]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());

        // this test has an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { if (&b != this) { } return *this; }");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return *this; }");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test and has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b)\n"
            "{\n"
            "    if (&b != this)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(b.s);\n"
            "    }\n"
            "    return *this;\n"
            " }");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but doesn't have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(b.s);\n"
            "    return *this;\n"
            " }");
        ASSERT_EQUALS("[test.cpp:11]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());
    }

    void operatorEqToSelf3() {
        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(a.s);\n"
            "    return *this;\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf4() {
        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return *this; }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &b)\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(b.s);\n"
            "            return *this;\n"
            "        }\n"
            "    };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return *this; }");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(b.s);\n"
            "    return *this;\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf5() {
        // ticket # 1233
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if((&a!=this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if((this!=&a))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(!(&a==this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(!(this==&a))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(false==(&a==this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(false==(this==&a))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(true!=(&a==this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(true!=(this==&a))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if((&a!=this))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if((this!=&a))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if(!(&a==this))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if(!(this==&a))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if(false==(&a==this))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if(false==(this==&a))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if(true!=(&a==this))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    if(true!=(this==&a))\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "struct A {\n"
            "    char *s;\n"
            "    A& operator=(const B &b);\n"
            "};\n"
            "A& A::operator=(const B &b) {\n"
            "    free(s);\n"
            "    s = strdup(a.s);\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf6() {
        // ticket # 1550
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        delete [] data;\n"
            "        data = new char[strlen(a.data) + 1];\n"
            "        strcpy(data, a.data);\n"
            "        return *this;\n"
            "    }\n"
            "private:\n"
            "    char * data;\n"
            "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "private:\n"
            "    char * data;\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    delete [] data;\n"
            "    data = new char[strlen(a.data) + 1];\n"
            "    strcpy(data, a.data);\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("[test.cpp:8]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        delete data;\n"
            "        data = new char;\n"
            "        *data  = *a.data;\n"
            "        return *this;\n"
            "    }\n"
            "private:\n"
            "    char * data;\n"
            "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "private:\n"
            "    char * data;\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    delete data;\n"
            "    data = new char;\n"
            "    *data = *a.data;\n"
            "    return *this;\n"
            "};");
        ASSERT_EQUALS("[test.cpp:8]: (warning) 'operator=' should check for assignment to self to avoid problems with dynamic memory.\n", errout.str());
    }

    void operatorEqToSelf7() {
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & assign(const A & a)\n"
            "    {\n"
            "        return *this;\n"
            "    }\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        return assign(a);\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf8() {
        checkOpertorEqToSelf(
            "class FMat\n"
            "{\n"
            "public:\n"
            "    FMat& copy(const FMat& rhs);\n"
            "    FMat& operator=(const FMat& in);\n"
            "};\n"
            "FMat& FMat::copy(const FMat& rhs)\n"
            "{\n"
            "    return *this;\n"
            "}\n"
            "FMat& FMat::operator=(const FMat& in)\n"
            "{\n"
            "    return copy(in);\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf9() {
        checkOpertorEqToSelf(
            "class Foo\n"
            "{\n"
            "public:\n"
            "    Foo& operator=(Foo* pOther);\n"
            "    Foo& operator=(Foo& other);\n"
            "};\n"
            "Foo& Foo::operator=(Foo* pOther)\n"
            "{\n"
            "    return *this;\n"
            "}\n"
            "Foo& Foo::operator=(Foo& other)\n"
            "{\n"
            "    return Foo::operator=(&other);\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Check that base classes have virtual destructors
    void checkVirtualDestructor(const char code[], bool inconclusive = false) {
        // Clear the error log
        errout.str("");

        settings0.inconclusive = inconclusive;
        settings0.addEnabled("warning");

        // Tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings0, this);
        checkClass.virtualDestructor();
    }

    void virtualDestructor1() {
        // Base class not found

        checkVirtualDestructor("class Derived : public Base { };\n"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Derived : Base { };\n"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor2() {
        // Base class doesn't have a destructor

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class 'Base' which is inherited by class 'Derived' does not have a virtual destructor.\n", errout.str());

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : protected Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class 'Base' which is inherited by class 'Derived' does not have a virtual destructor.\n", errout.str());

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : private Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor3() {
        // Base class has a destructor, but it's not virtual

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class 'Base' which is inherited by class 'Derived' does not have a virtual destructor.\n", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : protected Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class 'Base' which is inherited by class 'Derived' does not have a virtual destructor.\n", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class 'Base' which is inherited by class 'Derived' does not have a virtual destructor.\n", errout.str());
    }

    void virtualDestructor4() {
        // Derived class doesn't have a destructor => no error

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor5() {
        // Derived class has empty destructor => no error

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() {} };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived(); }; Derived::~Derived() {}"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor6() {
        // Only report error if base class pointer is deleted that
        // points at derived class

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructorProtected() {
        // Base class has protected destructor, it makes Base *p = new Derived(); fail
        // during compilation time, so error is not possible. => no error
        checkVirtualDestructor("class A\n"
                               "{\n"
                               "protected:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructorInherited() {
        // class A inherits virtual destructor from class Base -> no error
        checkVirtualDestructor("class Base\n"
                               "{\n"
                               "public:\n"
                               "virtual ~Base() {}\n"
                               "};\n"
                               "class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};");
        ASSERT_EQUALS("", errout.str());

        // class A inherits virtual destructor from struct Base -> no error
        // also notice that public is not given, but destructor is public, because
        // we are using struct instead of class
        checkVirtualDestructor("struct Base\n"
                               "{\n"
                               "virtual ~Base() {}\n"
                               "};\n"
                               "class A : public Base\n"
                               "{\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};");
        ASSERT_EQUALS("", errout.str());

        // Unknown Base class -> it could have virtual destructor, so ignore
        checkVirtualDestructor("class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};");
        ASSERT_EQUALS("", errout.str());

        // Virtual destructor is inherited -> no error
        checkVirtualDestructor("class Base2\n"
                               "{\n"
                               "virtual ~Base2() {}\n"
                               "};\n"
                               "class Base : public Base2\n"
                               "{\n"
                               "};\n"
                               "class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};");
        ASSERT_EQUALS("", errout.str());

        // class A doesn't inherit virtual destructor from class Base -> error
        checkVirtualDestructor("class Base\n"
                               "{\n"
                               "public:\n"
                               "    ~Base() {}\n"
                               "};\n"
                               "class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Class 'Base' which is inherited by class 'B' does not have a virtual destructor.\n",
                           "", errout.str());
    }

    void virtualDestructorTemplate() {
        checkVirtualDestructor("template <typename T> class A\n"
                               "{\n"
                               " public:\n"
                               " virtual ~A(){}\n"
                               "};\n"
                               "template <typename T> class AA\n"
                               "{\n"
                               " public:\n"
                               " ~AA(){}\n"
                               "};\n"
                               "class B : public A<int>, public AA<double>\n"
                               "{\n"
                               " public:\n"
                               " ~B(){int a;}\n"
                               "};\n"
                               "\n"
                               "AA<double> *p = new B; delete p;");
        ASSERT_EQUALS("[test.cpp:9]: (error) Class 'AA < double >' which is inherited by class 'B' does not have a virtual destructor.\n", errout.str());
    }

    void virtualDestructorInconclusive() {
        checkVirtualDestructor("class Base {\n"
                               "public:\n"
                               "    ~Base(){}\n"
                               "    virtual void foo(){}\n"
                               "};\n", true);
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Class 'Base' which has virtual members does not have a virtual destructor.\n", errout.str());

        checkVirtualDestructor("class Base {\n"
                               "public:\n"
                               "    ~Base(){}\n"
                               "    virtual void foo(){}\n"
                               "};\n"
                               "class Derived : public Base {\n"
                               "public:\n"
                               "    ~Derived() { bar(); }\n"
                               "};\n"
                               "void foo() {\n"
                               "    Base * base = new Derived();\n"
                               "    delete base;\n"
                               "}\n", true);
        ASSERT_EQUALS("[test.cpp:3]: (error) Class 'Base' which is inherited by class 'Derived' does not have a virtual destructor.\n", errout.str());
    }

    void checkNoMemset(const char code[]) {
        Settings settings;
        settings.addEnabled("warning");
        settings.addEnabled("portability");
        checkNoMemset(code,settings);
    }

    void checkNoMemset(const char code[], const Settings &settings) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.checkMemset();
    }

    void memsetOnClass() {
        checkNoMemset("class Fred\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    static std::string b;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string * b;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string b;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a 'std::string'.\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    mutable std::string b;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a 'std::string'.\n", errout.str());

        checkNoMemset("class Fred {\n"
                      "    std::string b;\n"
                      "    void f();\n"
                      "};\n"
                      "void Fred::f() {\n"
                      "    memset(this, 0, sizeof(*this));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Using 'memset' on class that contains a 'std::string'.\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string s;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a 'std::string'.\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string s;\n"
                      "};\n"
                      "class Pebbles: public Fred {};\n"
                      "void f()\n"
                      "{\n"
                      "    Pebbles pebbles;\n"
                      "    memset(&pebbles, 0, sizeof(pebbles));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Using 'memset' on class that contains a 'std::string'.\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    virtual ~Fred();\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a virtual method.\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    virtual ~Fred();\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    static Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a virtual method.\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "};\n"
                      "class Wilma\n"
                      "{\n"
                      "    virtual ~Wilma();\n"
                      "};\n"
                      "class Pebbles: public Fred, Wilma {};\n"
                      "void f()\n"
                      "{\n"
                      "    Pebbles pebbles;\n"
                      "    memset(&pebbles, 0, sizeof(pebbles));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:12]: (error) Using 'memset' on class that contains a virtual method.\n", errout.str());

        // Fred not defined in scope
        checkNoMemset("namespace n1 {\n"
                      "    class Fred\n"
                      "    {\n"
                      "        std::string b;\n"
                      "    };\n"
                      "}\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        // Fred with namespace qualifier
        checkNoMemset("namespace n1 {\n"
                      "    class Fred\n"
                      "    {\n"
                      "        std::string b;\n"
                      "    };\n"
                      "}\n"
                      "void f()\n"
                      "{\n"
                      "    n1::Fred fred;\n"
                      "    memset(&fred, 0, sizeof(n1::Fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Using 'memset' on class that contains a 'std::string'.\n", errout.str());

        // Fred with namespace qualifier
        checkNoMemset("namespace n1 {\n"
                      "    class Fred\n"
                      "    {\n"
                      "        std::string b;\n"
                      "    };\n"
                      "}\n"
                      "void f()\n"
                      "{\n"
                      "    n1::Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Using 'memset' on class that contains a 'std::string'.\n", errout.str());

        checkNoMemset("class A {\n"
                      "  virtual ~A() { }\n"
                      "  std::string s;\n"
                      "};\n"
                      "int f() {\n"
                      "  const int N = 10;\n"
                      "  A** arr = new A*[N];\n"
                      "  memset(arr, 0, N * sizeof(A*));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class A {\n" // #5116 - nested class data is mixed in the SymbolDatabase
                      "  std::string s;\n"
                      "  struct B { int x; };\n"
                      "};\n"
                      "void f(A::B *b) {\n"
                      "  memset(b,0,4);\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        // #4461 Warn about memset/memcpy on class with references as members
        checkNoMemset("class A {\n"
                      "  std::string &s;\n"
                      "};\n"
                      "void f() {\n"
                      "  A a;\n"
                      "  memset(&a, 0, sizeof(a)); \n"
                      "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Using 'memset' on class that contains a reference.\n", errout.str());
        checkNoMemset("class A {\n"
                      "  const B&b;\n"
                      "};\n"
                      "void f() {\n"
                      "  A a;\n"
                      "  memset(&a, 0, sizeof(a)); \n"
                      "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Using 'memset' on class that contains a reference.\n", errout.str());

        // #7456
        checkNoMemset("struct A {\n"
                      "  A() {}\n"
                      "  virtual ~A() {}\n"
                      "};\n"
                      "struct B {\n"
                      "  A* arr[4];\n"
                      "};\n"
                      "void func() {\n"
                      "  B b[4];\n"
                      "  memset(b, 0, sizeof(b));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());
    }

    void memsetOnInvalid() { // Ticket #5425
        checkNoMemset("union ASFStreamHeader {\n"
                      "  struct AVMPACKED {\n"
                      "    union  {\n"
                      "      struct AVMPACKED {\n"
                      "        int width;\n"
                      "      } vid;\n"
                      "    };\n"
                      "  } hdr;\n"
                      "};"
                      "void parseHeader() {\n"
                      "  ASFStreamHeader strhdr;\n"
                      "  memset(&strhdr, 0, sizeof(strhdr));\n"
                      "}");
    }

    void memsetOnStruct() {
        checkNoMemset("struct A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    struct A a;\n"
                      "    memset(&a, 0, sizeof(struct A));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    struct A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("void f()\n"
                      "{\n"
                      "    struct sockaddr_in6 fail;\n"
                      "    memset(&fail, 0, sizeof(struct sockaddr_in6));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      " void g( struct sockaddr_in6& a);\n"
                      "private:\n"
                      " std::string b;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      " struct A fail;\n"
                      " memset(&fail, 0, sizeof(struct A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Using 'memset' on struct that contains a 'std::string'.\n", errout.str());

        checkNoMemset("struct Fred\n"
                      "{\n"
                      "     std::string s;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on struct that contains a 'std::string'.\n", errout.str());

        checkNoMemset("struct Stringy {\n"
                      "    std::string inner;\n"
                      "};\n"
                      "struct Foo {\n"
                      "    Stringy s;\n"
                      "};\n"
                      "int main() {\n"
                      "    Foo foo;\n"
                      "    memset(&foo, 0, sizeof(Foo));\n"
                      "}");

        ASSERT_EQUALS("[test.cpp:9]: (error) Using 'memset' on struct that contains a 'std::string'.\n", errout.str());
    }

    void memsetVector() {
        checkNoMemset("class A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on class that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(struct A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(a));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("class A\n"
                      "{ std::vector< std::vector<int> > ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on class that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector< std::vector<int> > ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector< std::vector<int> > ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(a));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int *> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'.\n", errout.str());

        checkNoMemset("struct A {\n"
                      "     std::vector<int *> buf;\n"
                      "     operator int*() {return &buf[0];}\n"
                      "};\n"
                      "void f() {\n"
                      "    A a;\n"
                      "    memset(a, 0, 100);\n"
                      "}");
        ASSERT_EQUALS("", errout.str()); // #4460

        checkNoMemset("struct C {\n"
                      "    std::string s;\n"
                      "};\n"
                      "int foo() {\n"
                      "    C* c1[10][10];\n"
                      "    C* c2[10];\n"
                      "    C c3[10][10];\n"
                      "    C** c4 = new C*[10];\n"
                      "    memset(**c1, 0, 10);\n"
                      "    memset(*c1, 0, 10);\n"
                      "    memset(*c2, 0, 10);\n"
                      "    memset(*c3, 0, 10);\n"
                      "    memset(*c4, 0, 10);\n"
                      "    memset(c2, 0, 10);\n"
                      "    memset(c3, 0, 10);\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Using 'memset' on struct that contains a 'std::string'.\n"
                      "[test.cpp:11]: (error) Using 'memset' on struct that contains a 'std::string'.\n"
                      "[test.cpp:12]: (error) Using 'memset' on struct that contains a 'std::string'.\n"
                      "[test.cpp:13]: (error) Using 'memset' on struct that contains a 'std::string'.\n", errout.str());

        // Ticket #6953
        checkNoMemset("typedef float realnum;\n"
                      "struct multilevel_data {\n"
                      "  realnum *GammaInv;\n"
                      "  realnum data[1];\n"
                      "};\n"
                      "void *new_internal_data() const {\n"
                      "  multilevel_data *d = (multilevel_data *) malloc(sizeof(multilevel_data));\n"
                      "  memset(d, 0, sizeof(multilevel_data));\n"
                      "  return (void*) d;\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:8]: (portability) Using memset() on struct which contains a floating point number.\n", errout.str());
    }

    void memsetOnStdPodType() { // Ticket #5901
        Settings settings;
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <podtype name=\"uint8_t\" sign=\"u\" size=\"1\"/>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        settings.library.load(doc);

        checkNoMemset("class A {\n"
                      "    std::array<int, 10> ints;\n"
                      "};\n"
                      "void f() {\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("", errout.str()); // std::array is POD (#5481)

        checkNoMemset("struct st {\n"
                      "  std::uint8_t a;\n"
                      "  std::uint8_t b;\n"
                      "  std::uint8_t c;\n"
                      "};\n"
                      "\n"
                      "void f() {\n"
                      "  st s;\n"
                      "  std::memset(&s, 0, sizeof(st));\n"
                      "}", settings);
        ASSERT_EQUALS("", errout.str());
    }

    void memsetOnFloat() {
        checkNoMemset("struct A {\n"
                      "    float f;\n"
                      "};\n"
                      "void f() {\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:6]: (portability) Using memset() on struct which contains a floating point number.\n", errout.str());

        checkNoMemset("struct A {\n"
                      "    float f[4];\n"
                      "};\n"
                      "void f() {\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:6]: (portability) Using memset() on struct which contains a floating point number.\n", errout.str());

        checkNoMemset("struct A {\n"
                      "    float f[4];\n"
                      "};\n"
                      "void f(const A& b) {\n"
                      "    A a;\n"
                      "    memcpy(&a, &b, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A {\n"
                      "    float* f;\n"
                      "};\n"
                      "void f() {\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());
    }

    void memsetOnUnknown() {
        checkNoMemset("void clang_tokenize(CXToken **Tokens) {\n"
                      "  *Tokens = (CXToken *)malloc(sizeof(CXToken) * CXTokens.size());\n"
                      "  memmove(*Tokens, CXTokens.data(), sizeof(CXToken) * CXTokens.size());\n"
                      "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mallocOnClass() {
        checkNoMemset("class C { C() {} };\n"
                      "void foo(C*& p) {\n"
                      "    p = malloc(sizeof(C));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (warning) Memory for class instance allocated with malloc(), but class provides constructors.\n", errout.str());

        checkNoMemset("class C { C(int z, Foo bar) { bar(); } };\n"
                      "void foo(C*& p) {\n"
                      "    p = malloc(sizeof(C));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (warning) Memory for class instance allocated with malloc(), but class provides constructors.\n", errout.str());

        checkNoMemset("struct C { C() {} };\n"
                      "void foo(C*& p) {\n"
                      "    p = realloc(p, sizeof(C));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (warning) Memory for class instance allocated with realloc(), but class provides constructors.\n", errout.str());

        checkNoMemset("struct C { virtual void bar(); };\n"
                      "void foo(C*& p) {\n"
                      "    p = malloc(sizeof(C));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (error) Memory for class instance allocated with malloc(), but class contains a virtual method.\n", errout.str());

        checkNoMemset("struct C { std::string s; };\n"
                      "void foo(C*& p) {\n"
                      "    p = malloc(sizeof(C));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (error) Memory for class instance allocated with malloc(), but class contains a 'std::string'.\n", errout.str());

        checkNoMemset("class C { };\n" // C-Style class/struct
                      "void foo(C*& p) {\n"
                      "    p = malloc(sizeof(C));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct C { C() {} };\n"
                      "void foo(C*& p) {\n"
                      "    p = new C();\n"
                      "}");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class C { C() {} };\n"
                      "void foo(D*& p) {\n" // Unknown type
                      "    p = malloc(sizeof(C));\n"
                      "}");
        ASSERT_EQUALS("", errout.str());
    }


    void checkThisSubtraction(const char code[]) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckClass checkClass(&tokenizer, &settings1, this);
        checkClass.thisSubtraction();
    }

    void this_subtraction() {
        checkThisSubtraction("; this-x ;");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Suspicious pointer subtraction. Did you intend to write '->'?\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;");
        ASSERT_EQUALS("", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious pointer subtraction. Did you intend to write '->'?\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;\n"
                             "this-x ;\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious pointer subtraction. Did you intend to write '->'?\n"
                      "[test.cpp:3]: (warning) Suspicious pointer subtraction. Did you intend to write '->'?\n", errout.str());
    }

    void checkConst(const char code[], Settings *s = 0, bool inconclusive = true) {
        // Clear the error log
        errout.str("");

        // Check..
        if (!s)
            s = &settings0;
        s->inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(s, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        CheckClass checkClass(&tokenizer, s, this);
        checkClass.checkConst();
    }

    void const1() {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int getA() { return a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    const std::string foo() { return \"\"; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'Fred::foo' can be static.\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    const std::string & foo() { return \"\"; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::foo' can be static.\n", errout.str());

        // constructors can't be const..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "public:\n"
                   "    Fred() { }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment through |=..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int setA() { a |= true; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // functions with a call to a member function can only be const, if that member function is const, too.. (#1305)
        checkConst("class foo {\n"
                   "public:\n"
                   "    int x;\n"
                   "    void a() { x = 1; }\n"
                   "    void b() { a(); }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "public:\n"
                   "    int x;\n"
                   "    int a() const { return x; }\n"
                   "    void b() { a(); }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:5]: (style, inconclusive) Technically the member function 'Fred::b' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "public:\n"
                   "    int x;\n"
                   "    void b() { a(); }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance, inconclusive) Technically the member function 'Fred::b' can be static.\n", errout.str());

        // static functions can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    static unsigned get()\n"
                   "    { return 0; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    const std::string foo() const throw() { return \"\"; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'Fred::foo' can be static.\n", errout.str());
    }

    void const2() {
        // ticket 1344
        // assignment to variable can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo() { s = \"\"; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument reference can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a) { a = s; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a) { s = a; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument references can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { a = s; b = s; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { s = a; s = b; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { s = a; b = a; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { a = s; s = b; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const3() {
        // assignment to function argument pointer can be const
        checkConst("class Fred {\n"
                   "    int s;\n"
                   "    void foo(int * a) { *a = s; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    int s;\n"
                   "    void foo(int * a) { s = *a; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument pointers can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { *a = s; *b = s; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { s = *a; s = *b; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { s = *a; *b = s; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { *a = s; s = b; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const4() {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int getA();\n"
                   "};\n"
                   "int Fred::getA() { return a; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    const std::string & foo();\n"
                   "};\n"
                   "const std::string & Fred::foo() { return \"\"; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::foo' can be static.\n", errout.str());

        // functions with a function call to a non-const member can't be const.. (#1305)
        checkConst("class Fred\n"
                   "{\n"
                   "public:\n"
                   "    int x;\n"
                   "    void a() { x = 1; }\n"
                   "    void b();\n"
                   "};\n"
                   "void Fred::b() { a(); }");
        ASSERT_EQUALS("", errout.str());

        // static functions can't be const..
        checkConst("class Fred\n"
                   "{\n"
                   "public:\n"
                   "    static unsigned get();\n"
                   "};\n"
                   "static unsigned Fred::get() { return 0; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo();\n"
                   "};\n"
                   "void Fred::foo() { s = \"\"; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument reference can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a);\n"
                   "};\n"
                   "void Fred::foo(std::string & a) { a = s; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a);\n"
                   "};\n"
                   "void Fred::foo(std::string & a) { s = a; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument references can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b);\n"
                   "};\n"
                   "void Fred::foo(std::string & a, std::string & b) { a = s; b = s; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b);\n"
                   "};\n"
                   "void Fred::foo(std::string & a, std::string & b) { s = a; s = b; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b);\n"
                   "};\n"
                   "void Fred::foo(std::string & a, std::string & b) { s = a; b = a; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b);\n"
                   "};\n"
                   "void Fred::foo(std::string & a, std::string & b) { a = s; s = b; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument pointer can be const
        checkConst("class Fred {\n"
                   "    int s;\n"
                   "    void foo(int * a);\n"
                   "};\n"
                   "void Fred::foo(int * a) { *a = s; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    int s;\n"
                   "    void foo(int * a);\n"
                   "};\n"
                   "void Fred::foo(int * a) { s = *a; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument pointers can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b);\n"
                   "};\n"
                   "void Fred::foo(std::string * a, std::string * b) { *a = s; *b = s; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b);\n"
                   "};\n"
                   "void Fred::foo(std::string * a, std::string * b) { s = *a; s = *b; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b);\n"
                   "};\n"
                   "void Fred::foo(std::string * a, std::string * b) { s = *a; *b = s; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b);\n"
                   "};\n"
                   "void Fred::foo(std::string * a, std::string * b) { *a = s; s = b; }");
        ASSERT_EQUALS("", errout.str());

        // check functions with same name
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo();\n"
                   "    void foo(std::string & a);\n"
                   "    void foo(const std::string & a);\n"
                   "};\n"
                   "void Fred::foo() { }"
                   "void Fred::foo(std::string & a) { a = s; }"
                   "void Fred::foo(const std::string & a) { s = a; }");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::foo' can be static.\n"
                      "[test.cpp:7] -> [test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // check functions with different or missing parameter names
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo1(int, int);\n"
                   "    void foo2(int a, int b);\n"
                   "    void foo3(int, int b);\n"
                   "    void foo4(int a, int);\n"
                   "    void foo5(int a, int b);\n"
                   "};\n"
                   "void Fred::foo1(int a, int b) { }\n"
                   "void Fred::foo2(int c, int d) { }\n"
                   "void Fred::foo3(int a, int b) { }\n"
                   "void Fred::foo4(int a, int b) { }\n"
                   "void Fred::foo5(int, int) { }");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::foo1' can be static.\n"
                      "[test.cpp:10] -> [test.cpp:4]: (performance, inconclusive) Technically the member function 'Fred::foo2' can be static.\n"
                      "[test.cpp:11] -> [test.cpp:5]: (performance, inconclusive) Technically the member function 'Fred::foo3' can be static.\n"
                      "[test.cpp:12] -> [test.cpp:6]: (performance, inconclusive) Technically the member function 'Fred::foo4' can be static.\n"
                      "[test.cpp:13] -> [test.cpp:7]: (performance, inconclusive) Technically the member function 'Fred::foo5' can be static.\n", errout.str());

        // check nested classes
        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA() { return a; }\n"
                   "    };\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::A::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA();\n"
                   "    };\n"
                   "    int A::getA() { return a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::A::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA();\n"
                   "    };\n"
                   "};\n"
                   "int Fred::A::getA() { return a; }");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::A::getA' can be const.\n", errout.str());

        // check deeply nested classes
        checkConst("class Fred {\n"
                   "    class B {\n"
                   "        int b;\n"
                   "        int getB() { return b; }\n"
                   "        class A {\n"
                   "            int a;\n"
                   "            int getA() { return a; }\n"
                   "        };\n"
                   "    };\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:7]: (style, inconclusive) Technically the member function 'Fred::B::A::getA' can be const.\n"
                      , errout.str());

        checkConst("class Fred {\n"
                   "    class B {\n"
                   "        int b;\n"
                   "        int getB();\n"
                   "        class A {\n"
                   "            int a;\n"
                   "            int getA();\n"
                   "        };\n"
                   "        int A::getA() { return a; }\n"
                   "    };\n"
                   "    int B::getB() { return b; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:9] -> [test.cpp:7]: (style, inconclusive) Technically the member function 'Fred::B::A::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    class B {\n"
                   "        int b;\n"
                   "        int getB();\n"
                   "        class A {\n"
                   "            int a;\n"
                   "            int getA();\n"
                   "        };\n"
                   "    };\n"
                   "    int B::A::getA() { return a; }\n"
                   "    int B::getB() { return b; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:10] -> [test.cpp:7]: (style, inconclusive) Technically the member function 'Fred::B::A::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    class B {\n"
                   "        int b;\n"
                   "        int getB();\n"
                   "        class A {\n"
                   "            int a;\n"
                   "            int getA();\n"
                   "        };\n"
                   "    };\n"
                   "};\n"
                   "int Fred::B::A::getA() { return a; }\n"
                   "int Fred::B::getB() { return b; }");
        ASSERT_EQUALS("[test.cpp:12] -> [test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:11] -> [test.cpp:7]: (style, inconclusive) Technically the member function 'Fred::B::A::getA' can be const.\n", errout.str());
    }

    // operator< can often be const
    void constoperator1() {
        checkConst("struct Fred {\n"
                   "    int a;\n"
                   "    bool operator<(const Fred &f) { return a < f.a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::operator<' can be const.\n", errout.str());
    }

    // operator<<
    void constoperator2() {
        checkConst("struct Foo {\n"
                   "    void operator<<(int);\n"
                   "};\n"
                   "struct Fred {\n"
                   "    Foo foo;\n"
                   "    void x()\n"
                   "    {\n"
                   "        foo << 123;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct Foo {\n"
                   "    void operator<<(int);\n"
                   "};\n"
                   "struct Fred {\n"
                   "    Foo foo;\n"
                   "    void x()\n"
                   "    {\n"
                   "        std::cout << foo << 123;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:6]: (style, inconclusive) Technically the member function 'Fred::x' can be const.\n", errout.str());
    }

    void constoperator3() {
        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    int const & operator [] (unsigned int index) const { return array[index]; }\n"
                   "    int & operator [] (unsigned int index) { return array[index]; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    int const & operator [] (unsigned int index) { return array[index]; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::operator[]' can be const.\n", errout.str());
    }

    void constoperator4() {
        // #7953
        checkConst("class A {\n"
                   "    int c;\n"
                   "public:\n"
                   "    operator int*() { return &c; };\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "    int c;\n"
                   "public:\n"
                   "    operator const int*() { return &c; };\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::operatorconstint*' can be const.\n", errout.str());

        // #2375
        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    typedef int* (Fred::*UnspecifiedBoolType);\n"
                   "    operator UnspecifiedBoolType() { };\n"
                   "};");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::operatorint**' can be const.\n", "", errout.str());

        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    typedef int* (Fred::*UnspecifiedBoolType);\n"
                   "    operator UnspecifiedBoolType() { array[0] = 0; };\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void constoperator5() { // ticket #3252
        checkConst("class A {\n"
                   "    int c;\n"
                   "public:\n"
                   "    operator int& () {return c}\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "    int c;\n"
                   "public:\n"
                   "    operator const int& () {return c}\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::operatorconstint&' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "    int c;\n"
                   "public:\n"
                   "    operator int () {return c}\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::operatorint' can be const.\n", errout.str());
    }

    void constoperator6() { // ticket #8669
        checkConst("class A {\n"
                   "    int c;\n"
                   "    void f() { os >> *this; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const5() {
        // ticket #1482
        checkConst("class A {\n"
                   "    int a;\n"
                   "    bool foo(int i)\n"
                   "    {\n"
                   "        bool same;\n"
                   "        same = (i == a);\n"
                   "        return same;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'A::foo' can be const.\n", errout.str());
    }

    void const6() {
        // ticket #1491
        checkConst("class foo {\n"
                   "public:\n"
                   "};\n"
                   "void bar() {}");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred\n"
                   "{\n"
                   "public:\n"
                   "    void foo() { }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance, inconclusive) Technically the member function 'Fred::foo' can be static.\n", errout.str());

        checkConst("struct fast_string\n"
                   "{\n"
                   "    union\n"
                   "    {\n"
                   "        char buff[100];\n"
                   "    };\n"
                   "    void set_type(char t);\n"
                   "};\n"
                   "inline void fast_string::set_type(char t)\n"
                   "{\n"
                   "    buff[10] = t;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());
    }

    void const7() {
        checkConst("class foo {\n"
                   "    int a;\n"
                   "public:\n"
                   "    void set(int i) { a = i; }\n"
                   "    void set(const foo & f) { *this = f; }\n"
                   "};\n"
                   "void bar() {}");
        ASSERT_EQUALS("", errout.str());
    }

    void const8() {
        // ticket #1517
        checkConst("class A {\n"
                   "public:\n"
                   "    A():m_strValue(\"\"){}\n"
                   "    std::string strGetString() { return m_strValue; }\n"
                   "private:\n"
                   "    std::string m_strValue;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::strGetString' can be const.\n", errout.str());
    }

    void const9() {
        // ticket #1515
        checkConst("class wxThreadInternal {\n"
                   "public:\n"
                   "    void SetExitCode(wxThread::ExitCode exitcode) { m_exitcode = exitcode; }\n"
                   "private:\n"
                   "    wxThread::ExitCode m_exitcode;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const10() {
        // ticket #1522
        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { return x = 0; }\n"
                   "private:\n"
                   "    int x;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { return x ? x : x = 0; }\n"
                   "private:\n"
                   "    int x;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { return x ? x = 0 : x; }\n"
                   "private:\n"
                   "    int x;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const11() {
        // ticket #1529
        checkConst("class A {\n"
                   "public:\n"
                   "    void set(struct tm time) { m_time = time; }\n"
                   "private:\n"
                   "    struct tm m_time;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const12() {
        // ticket #1525
        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { x = 0; }\n"
                   "private:\n"
                   "    mutable int x;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'A::foo' can be const.\n", errout.str());
    }

    void const13() {
        // ticket #1519
        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::vector<int> GetVec()  {return m_vec;}\n"
                   "    std::pair<int,double> GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::vector<int> m_vec;\n"
                   "    std::pair<int,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetVec' can be const.\n"
                      "[test.cpp:5]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::vector<int> & GetVec()  {return m_vec;}\n"
                   "    const std::pair<int,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::vector<int> m_vec;\n"
                   "    std::pair<int,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetVec' can be const.\n"
                      "[test.cpp:5]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());
    }

    void const14() {
        // extends ticket 1519
        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair<std::vector<int>,double> GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair<std::vector<int>,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair<std::vector<int>,double>& GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair<std::vector<int>,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair<std::vector<int>,double>& GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair<std::vector<int>,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair<int ,double> GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair<int ,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair<int ,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair<int ,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair<int ,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair<int ,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int,std::vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< int,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, int >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< vector<int>, int >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, int >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< std::vector<int>,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::vector<int>,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());



        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::pair < int, char > , int >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< std::pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int , std::pair < int, char > >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< int , std::pair < int, char > >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int , std::pair < int, char > >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    vector<int>  GetVec() {return m_Vec;}\n"
                   "private:\n"
                   "    vector<int>  m_Vec;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetVec' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const vector<int>&  GetVec() {return m_Vec;}\n"
                   "private:\n"
                   "    vector<int>  m_Vec;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::GetVec' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    vector<int>&  GetVec() {return m_Vec;}\n"
                   "private:\n"
                   "    vector<int>  m_Vec;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("class A {\n"
                   "public:\n"
                   "    int * * foo() { return &x; }\n"
                   "private:\n"
                   "    const int * x;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    const int ** foo() { return &x; }\n"
                   "private:\n"
                   "    const int * x;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'A::foo' can be const.\n", errout.str());
    }

    void const15() {
        checkConst("class Fred {\n"
                   "    unsigned long long int a;\n"
                   "    unsigned long long int getA() { return a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::getA' can be const.\n", errout.str());

        // constructors can't be const..
        checkConst("class Fred {\n"
                   "    unsigned long long int a;\n"
                   "public:\n"
                   "    Fred() { }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // assignment through |=..
        checkConst("class Fred {\n"
                   "    unsigned long long int a;\n"
                   "    unsigned long long int setA() { a |= true; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // static functions can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    static unsigned long long int get()\n"
                   "    { return 0; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const16() {
        // ticket #1551
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void set(int i) { Fred::a = i; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const17() {
        // ticket #1552
        checkConst("class Fred {\n"
                   "public:\n"
                   "    void set(int i, int j) { a[i].k = i; }\n"
                   "private:\n"
                   "    struct { int k; } a[4];\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const18() {
        checkConst("class Fred {\n"
                   "static int x;\n"
                   "public:\n"
                   "    void set(int i) { x = i; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance, inconclusive) Technically the member function 'Fred::set' can be static.\n", errout.str());
    }

    void const19() {
        // ticket #1612
        checkConst("using namespace std;\n"
                   "class Fred {\n"
                   "private:\n"
                   "    std::string s;\n"
                   "public:\n"
                   "    void set(std::string ss) { s = ss; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const20() {
        // ticket #1602
        checkConst("class Fred {\n"
                   "    int x : 3;\n"
                   "public:\n"
                   "    void set(int i) { x = i; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    list<int *> x;\n"
                   "public:\n"
                   "    list<int *> get() { return x; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    list<const int *> x;\n"
                   "public:\n"
                   "    list<const int *> get() { return x; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::get' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::list<std::string &> x;\n"
                   "public:\n"
                   "    std::list<std::string &> get() { return x; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    std::list<const std::string &> x;\n"
                   "public:\n"
                   "    std::list<const std::string &> get() { return x; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::get' can be const.\n", errout.str());
    }

    void const21() {
        // ticket #1683
        checkConst("class A\n"
                   "{\n"
                   "private:\n"
                   "    const char * l1[10];\n"
                   "public:\n"
                   "    A()\n"
                   "    {\n"
                   "        for (int i = 0 ; i < 10; l1[i] = NULL, i++);\n"
                   "    }\n"
                   "    void f1() { l1[0] = \"Hello\"; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const22() {
        checkConst("class A\n"
                   "{\n"
                   "private:\n"
                   "    B::C * v1;\n"
                   "public:\n"
                   "    void f1() { v1 = 0; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A\n"
                   "{\n"
                   "private:\n"
                   "    B::C * v1[0];\n"
                   "public:\n"
                   "    void f1() { v1[0] = 0; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const23() {
        checkConst("class Class {\n"
                   "public:\n"
                   "    typedef Template<double> Type;\n"
                   "    typedef Template2<Type> Type2;\n"
                   "    void set_member(Type2 m) { _m = m; }\n"
                   "private:\n"
                   "    Type2 _m;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const24() {
        checkConst("class Class {\n"
                   "public:\n"
                   "void Settings::SetSetting(QString strSetting, QString strNewVal)\n"
                   "{\n"
                   "    (*m_pSettings)[strSetting] = strNewVal;\n"
                   "}\n"
                   "private:\n"
                   "    std::map<QString, QString> *m_pSettings;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }


    void const25() { // ticket #1724
        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVal=\"\";}\n"
                   "std::string strGetString() const\n"
                   "{return m_strVal.c_str();}\n"
                   "const std::string strGetString1() const\n"
                   "{return m_strVal.c_str();}\n"
                   "private:\n"
                   "std::string m_strVal;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("", errout.str());

        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVal=\"\";}\n"
                   "std::string strGetString()\n"
                   "{return m_strVal.c_str();}\n"
                   "private:\n"
                   "std::string m_strVal;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::strGetString' can be const.\n", errout.str());

        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVal=\"\";}\n"
                   "const std::string strGetString1()\n"
                   "{return m_strVal.c_str();}\n"
                   "private:\n"
                   "std::string m_strVal;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::strGetString1' can be const.\n", errout.str());

        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVec.push_back(\"\");}\n"
                   "size_t strGetSize()\n"
                   "{return m_strVec.size();}\n"
                   "private:\n"
                   "std::vector<std::string> m_strVec;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::strGetSize' can be const.\n", errout.str());

        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVec.push_back(\"\");}\n"
                   "bool strGetEmpty()\n"
                   "{return m_strVec.empty();}\n"
                   "private:\n"
                   "std::vector<std::string> m_strVec;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'A::strGetEmpty' can be const.\n", errout.str());
    }

    void const26() { // ticket #1847
        checkConst("class DelayBase {\n"
                   "public:\n"
                   "void swapSpecificDelays(int index1, int index2) {\n"
                   "    std::swap<float>(delays_[index1], delays_[index2]);\n"
                   "}\n"
                   "float delays_[4];\n"
                   "};\n"
                  );
        ASSERT_EQUALS("", errout.str());

        checkConst("struct DelayBase {\n"
                   "    float swapSpecificDelays(int index1) {\n"
                   "        return delays_[index1];\n"
                   "    }\n"
                   "    float delays_[4];\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Technically the member function 'DelayBase::swapSpecificDelays' can be const.\n", errout.str());
    }

    void const27() { // ticket #1882
        checkConst("class A {\n"
                   "public:\n"
                   "    A(){m_d=1.0; m_iRealVal=2.0;}\n"
                   "    double dGetValue();\n"
                   "private:\n"
                   "    double m_d;\n"
                   "    double m_iRealVal;\n"
                   "};\n"
                   "double  A::dGetValue() {\n"
                   "    double dRet = m_iRealVal;\n"
                   "    if( m_d != 0 )\n"
                   "        return m_iRealVal / m_d;\n"
                   "    return dRet;\n"
                   "};", nullptr, true);
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:4]: (style, inconclusive) Technically the member function 'A::dGetValue' can be const.\n", errout.str());
    }

    void const28() { // ticket #1883
        checkConst("class P {\n"
                   "public:\n"
                   "    P() { x=0.0; y=0.0; }\n"
                   "    double x,y;\n"
                   "};\n"
                   "class A : public P {\n"
                   "public:\n"
                   "    A():P(){}\n"
                   "    void SetPos(double xPos, double yPos) {\n"
                   "        x=xPos;\n"
                   "        y=yPos;\n"
                   "    }\n"
                   "};\n"
                  );
        ASSERT_EQUALS("", errout.str());

        checkConst("class AA : public P {\n"
                   "public:\n"
                   "    AA():P(){}\n"
                   "    inline void vSetXPos(int x_)\n"
                   "    {\n"
                   "        UnknownScope::x = x_;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class AA {\n"
                   "public:\n"
                   "    AA():P(){}\n"
                   "    inline void vSetXPos(int x_)\n"
                   "    {\n"
                   "        UnknownScope::x = x_;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance, inconclusive) Technically the member function 'AA::vSetXPos' can be static.\n", errout.str());

    }

    void const29() { // ticket #1922
        checkConst("class test {\n"
                   "  public:\n"
                   "    test();\n"
                   "    const char* get() const;\n"
                   "    char* get();\n"
                   "  private:\n"
                   "    char* value_;\n"
                   "};\n"
                   "test::test()\n"
                   "{\n"
                   "  value_ = 0;\n"
                   "}\n"
                   "const char* test::get() const\n"
                   "{\n"
                   "  return value_;\n"
                   "}\n"
                   "char* test::get()\n"
                   "{\n"
                   "  return value_;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());
    }

    void const30() {
        // check for false negatives
        checkConst("class Base {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Derived : public Base {\n"
                   "public:\n"
                   "    int get() {\n"
                   "        return a;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:7]: (style, inconclusive) Technically the member function 'Derived::get' can be const.\n", errout.str());

        checkConst("class Base1 {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Base2 {\n"
                   "public:\n"
                   "    int b;\n"
                   "};\n"
                   "class Derived : public Base1, public Base2 {\n"
                   "public:\n"
                   "    int getA() {\n"
                   "        return a;\n"
                   "    }\n"
                   "    int getB() {\n"
                   "        return b;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:11]: (style, inconclusive) Technically the member function 'Derived::getA' can be const.\n"
                      "[test.cpp:14]: (style, inconclusive) Technically the member function 'Derived::getB' can be const.\n", errout.str());

        checkConst("class Base {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Derived1 : public Base { };\n"
                   "class Derived2 : public Derived1 {\n"
                   "public:\n"
                   "    int get() {\n"
                   "        return a;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:8]: (style, inconclusive) Technically the member function 'Derived2::get' can be const.\n", errout.str());

        checkConst("class Base {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Derived1 : public Base { };\n"
                   "class Derived2 : public Derived1 { };\n"
                   "class Derived3 : public Derived2 { };\n"
                   "class Derived4 : public Derived3 {\n"
                   "public:\n"
                   "    int get() {\n"
                   "        return a;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:10]: (style, inconclusive) Technically the member function 'Derived4::get' can be const.\n", errout.str());

        // check for false positives
        checkConst("class Base {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Derived : public Base {\n"
                   "public:\n"
                   "    int get() const {\n"
                   "        return a;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Base1 {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Base2 {\n"
                   "public:\n"
                   "    int b;\n"
                   "};\n"
                   "class Derived : public Base1, public Base2 {\n"
                   "public:\n"
                   "    int getA() const {\n"
                   "        return a;\n"
                   "    }\n"
                   "    int getB() const {\n"
                   "        return b;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Base {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Derived1 : public Base { };\n"
                   "class Derived2 : public Derived1 {\n"
                   "public:\n"
                   "    int get() const {\n"
                   "        return a;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Base {\n"
                   "public:\n"
                   "    int a;\n"
                   "};\n"
                   "class Derived1 : public Base { };\n"
                   "class Derived2 : public Derived1 { };\n"
                   "class Derived3 : public Derived2 { };\n"
                   "class Derived4 : public Derived3 {\n"
                   "public:\n"
                   "    int get() const {\n"
                   "        return a;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const31() {
        checkConst("namespace std { }\n"
                   "class Fred {\n"
                   "public:\n"
                   "    int a;\n"
                   "    int get() { return a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:5]: (style, inconclusive) Technically the member function 'Fred::get' can be const.\n", errout.str());
    }

    void const32() {
        checkConst("class Fred {\n"
                   "public:\n"
                   "    std::string a[10];\n"
                   "    void seta() { a[0] = \"\"; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const33() {
        checkConst("class derived : public base {\n"
                   "public:\n"
                   "    void f(){}\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const34() { // ticket #1964
        checkConst("class Bar {\n"
                   "    void init(Foo * foo) {\n"
                   "        foo.bar = this;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const35() { // ticket #2001
        checkConst("namespace N\n"
                   "{\n"
                   "        class Base\n"
                   "        {\n"
                   "        };\n"
                   "}\n"
                   "namespace N\n"
                   "{\n"
                   "        class Derived : public Base\n"
                   "        {\n"
                   "        public:\n"
                   "                int getResourceName() { return var; }\n"
                   "                int var;\n"
                   "        };\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:12]: (style, inconclusive) Technically the member function 'N::Derived::getResourceName' can be const.\n", errout.str());

        checkConst("namespace N\n"
                   "{\n"
                   "        class Base\n"
                   "        {\n"
                   "        public:\n"
                   "                int getResourceName();\n"
                   "                int var;\n"
                   "        };\n"
                   "}\n"
                   "int N::Base::getResourceName() { return var; }");
        ASSERT_EQUALS("[test.cpp:10] -> [test.cpp:6]: (style, inconclusive) Technically the member function 'N::Base::getResourceName' can be const.\n", errout.str());

        checkConst("namespace N\n"
                   "{\n"
                   "        class Base\n"
                   "        {\n"
                   "        public:\n"
                   "                int getResourceName();\n"
                   "                int var;\n"
                   "        };\n"
                   "}\n"
                   "namespace N\n"
                   "{\n"
                   "        int Base::getResourceName() { return var; }\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:12] -> [test.cpp:6]: (style, inconclusive) Technically the member function 'N::Base::getResourceName' can be const.\n", errout.str());

        checkConst("namespace N\n"
                   "{\n"
                   "        class Base\n"
                   "        {\n"
                   "        public:\n"
                   "                int getResourceName();\n"
                   "                int var;\n"
                   "        };\n"
                   "}\n"
                   "using namespace N;\n"
                   "int Base::getResourceName() { return var; }");
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:6]: (style, inconclusive) Technically the member function 'N::Base::getResourceName' can be const.\n", errout.str());
    }

    void const36() { // ticket #2003
        checkConst("class Foo {\n"
                   "public:\n"
                   "    Blue::Utility::Size m_MaxQueueSize;\n"
                   "    void SetMaxQueueSize(Blue::Utility::Size a_MaxQueueSize)\n"
                   "    {\n"
                   "        m_MaxQueueSize = a_MaxQueueSize;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const37() { // ticket #2081 and #2085
        checkConst("class A\n"
                   "{\n"
                   "public:\n"
                   "    A(){};\n"
                   "    std::string operator+(const char *c)\n"
                   "    {\n"
                   "        return m_str+std::string(c);\n"
                   "    }\n"
                   "private:\n"
                   "    std::string m_str;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:5]: (style, inconclusive) Technically the member function 'A::operator+' can be const.\n", errout.str());

        checkConst("class Fred\n"
                   "{\n"
                   "private:\n"
                   "    long x;\n"
                   "public:\n"
                   "    Fred() {\n"
                   "        x = 0;\n"
                   "    }\n"
                   "    bool isValid() {\n"
                   "        return (x == 0x11224488);\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:9]: (style, inconclusive) Technically the member function 'Fred::isValid' can be const.\n", errout.str());
    }

    void const38() { // ticket #2135
        checkConst("class Foo {\n"
                   "public:\n"
                   "    ~Foo() { delete oArq; }\n"
                   "    Foo(): oArq(new std::ofstream(\"...\")) {}\n"
                   "    void MyMethod();\n"
                   "private:\n"
                   "    std::ofstream *oArq;\n"
                   "};\n"
                   "void Foo::MyMethod()\n"
                   "{\n"
                   "    (*oArq) << \"</table>\";\n"
                   "}");
        ASSERT_EQUALS("", errout.str());
    }

    void const39() {
        checkConst("class Foo\n"
                   "{\n"
                   "    int * p;\n"
                   "public:\n"
                   "    Foo () : p(0) { }\n"
                   "    int * f();\n"
                   "    const int * f() const;\n"
                   "};\n"
                   "const int * Foo::f() const\n"
                   "{\n"
                   "    return p;\n"
                   "}\n"
                   "int * Foo::f()\n"
                   "{\n"
                   "    return p;\n"
                   "}");

        ASSERT_EQUALS("", errout.str());
    }

    void const40() { // ticket #2228
        checkConst("class SharedPtrHolder\n"
                   "{\n"
                   "  private:\n"
                   "   std::tr1::shared_ptr<int> pView;\n"
                   "  public:\n"
                   "   SharedPtrHolder()\n"
                   "   { }\n"
                   "   void SetView(const std::shared_ptr<int> & aView)\n"
                   "   {\n"
                   "      pView = aView;\n"
                   "   }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());
    }

    void const41() { // ticket #2255
        checkConst("class Fred\n"
                   "{\n"
                   "   ::std::string m_name;\n"
                   "public:\n"
                   "   void SetName(const ::std::string & name)\n"
                   "   {\n"
                   "      m_name = name;\n"
                   "   }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());

        checkConst("class SharedPtrHolder\n"
                   "{\n"
                   "   ::std::tr1::shared_ptr<int> pNum;\n"
                   "  public :\n"
                   "   void SetNum(const ::std::tr1::shared_ptr<int> & apNum)\n"
                   "   {\n"
                   "      pNum = apNum;\n"
                   "   }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());

        checkConst("class SharedPtrHolder2\n"
                   "{\n"
                   "  public:\n"
                   "   typedef ::std::tr1::shared_ptr<int> IntSharedPtr;\n"
                   "  private:\n"
                   "   IntSharedPtr pNum;\n"
                   "  public :\n"
                   "   void SetNum(const IntSharedPtr & apNum)\n"
                   "   {\n"
                   "      pNum = apNum;\n"
                   "   }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());

        checkConst("struct IntPtrTypes\n"
                   "{\n"
                   "   typedef ::std::tr1::shared_ptr<int> Shared;\n"
                   "};\n"
                   "class SharedPtrHolder3\n"
                   "{\n"
                   "  private:\n"
                   "   IntPtrTypes::Shared pNum;\n"
                   "  public :\n"
                   "   void SetNum(const IntPtrTypes::Shared & apNum)\n"
                   "   {\n"
                   "      pNum = apNum;\n"
                   "   }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());

        checkConst("template <typename T>\n"
                   "struct PtrTypes\n"
                   "{\n"
                   "   typedef ::std::tr1::shared_ptr<T> Shared;\n"
                   "};\n"
                   "class SharedPtrHolder4\n"
                   "{\n"
                   "  private:\n"
                   "   PtrTypes<int>::Shared pNum;\n"
                   "  public :\n"
                   "   void SetNum(const PtrTypes<int>::Shared & apNum)\n"
                   "   {\n"
                   "      pNum = apNum;\n"
                   "   }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());
    }

    void const42() { // ticket #2282
        checkConst("class Fred\n"
                   "{\n"
                   "public:\n"
                   "    struct AB { };\n"
                   "    bool f(AB * ab);\n"
                   "};\n"
                   "bool Fred::f(Fred::AB * ab)\n"
                   "{\n"
                   "}");

        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5]: (performance, inconclusive) Technically the member function 'Fred::f' can be static.\n", errout.str());

        checkConst("class Fred\n"
                   "{\n"
                   "public:\n"
                   "    struct AB {\n"
                   "        struct CD { };\n"
                   "    };\n"
                   "    bool f(AB::CD * cd);\n"
                   "};\n"
                   "bool Fred::f(Fred::AB::CD * cd)\n"
                   "{\n"
                   "}");

        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:7]: (performance, inconclusive) Technically the member function 'Fred::f' can be static.\n", errout.str());

        checkConst("namespace NS {\n"
                   "    class Fred\n"
                   "    {\n"
                   "    public:\n"
                   "        struct AB {\n"
                   "            struct CD { };\n"
                   "        };\n"
                   "        bool f(AB::CD * cd);\n"
                   "    };\n"
                   "    bool Fred::f(Fred::AB::CD * cd)\n"
                   "    {\n"
                   "    }\n"
                   "}");

        ASSERT_EQUALS("[test.cpp:10] -> [test.cpp:8]: (performance, inconclusive) Technically the member function 'NS::Fred::f' can be static.\n", errout.str());

        checkConst("namespace NS {\n"
                   "    class Fred\n"
                   "    {\n"
                   "    public:\n"
                   "        struct AB {\n"
                   "            struct CD { };\n"
                   "        };\n"
                   "        bool f(AB::CD * cd);\n"
                   "    };\n"
                   "}\n"
                   "bool NS::Fred::f(NS::Fred::AB::CD * cd)\n"
                   "{\n"
                   "}");

        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:8]: (performance, inconclusive) Technically the member function 'NS::Fred::f' can be static.\n", errout.str());

        checkConst("class Foo {\n"
                   "    class Fred\n"
                   "    {\n"
                   "    public:\n"
                   "        struct AB {\n"
                   "            struct CD { };\n"
                   "        };\n"
                   "        bool f(AB::CD * cd);\n"
                   "    };\n"
                   "};\n"
                   "bool Foo::Fred::f(Foo::Fred::AB::CD * cd)\n"
                   "{\n"
                   "}");

        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:8]: (performance, inconclusive) Technically the member function 'Foo::Fred::f' can be static.\n", errout.str());
    }

    void const43() { // ticket 2377
        checkConst("class A\n"
                   "{\n"
                   "public:\n"
                   "    void foo( AA::BB::CC::DD b );\n"
                   "    AA::BB::CC::DD a;\n"
                   "};\n"
                   "void A::foo( AA::BB::CC::DD b )\n"
                   "{\n"
                   "    a = b;\n"
                   "}");

        ASSERT_EQUALS("", errout.str());

        checkConst("namespace AA\n"
                   "{\n"
                   "    namespace BB\n"
                   "    {\n"
                   "        namespace CC\n"
                   "        {\n"
                   "            struct DD\n"
                   "            {};\n"
                   "        }\n"
                   "    }\n"
                   "}\n"
                   "class A\n"
                   "{\n"
                   "    public:\n"
                   "\n"
                   "    AA::BB::CC::DD a;\n"
                   "    void foo(AA::BB::CC::DD b)\n"
                   "    {\n"
                   "        a = b;\n"
                   "    }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());

        checkConst("namespace ZZ\n"
                   "{\n"
                   "    namespace YY\n"
                   "    {\n"
                   "        struct XX\n"
                   "        {};\n"
                   "    }\n"
                   "}\n"
                   "class B\n"
                   "{\n"
                   "    public:\n"
                   "    ZZ::YY::XX a;\n"
                   "    void foo(ZZ::YY::XX b)\n"
                   "    {\n"
                   "        a = b;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const44() { // ticket 2595
        checkConst("class A\n"
                   "{\n"
                   "public:\n"
                   "    bool bOn;\n"
                   "    bool foo()\n"
                   "    {\n"
                   "        return 0 != (bOn = bOn);\n"
                   "    }\n"
                   "};");

        ASSERT_EQUALS("", errout.str());
    }

    void const45() { // ticket 2664
        checkConst("namespace wraps {\n"
                   "    class BaseLayout {};\n"
                   "}\n"
                   "namespace tools {\n"
                   "    class WorkspaceControl :\n"
                   "        public wraps::BaseLayout\n"
                   "    {\n"
                   "        int toGrid(int _value)\n"
                   "        {\n"
                   "        }\n"
                   "    };\n"
                   "}");

        ASSERT_EQUALS("[test.cpp:8]: (performance, inconclusive) Technically the member function 'tools::WorkspaceControl::toGrid' can be static.\n", errout.str());
    }

    void const46() { // ticket 2663
        checkConst("class Altren {\n"
                   "public:\n"
                   "    int fun1() {\n"
                   "        int a;\n"
                   "        a++;\n"
                   "    }\n"
                   "    int fun2() {\n"
                   "        b++;\n"
                   "    }\n"
                   "};");

        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Altren::fun1' can be static.\n"
                      "[test.cpp:7]: (performance, inconclusive) Technically the member function 'Altren::fun2' can be static.\n", errout.str());
    }

    void const47() { // ticket 2670
        checkConst("class Altren {\n"
                   "public:\n"
                   "  void foo() { delete this; }\n"
                   "  void foo(int i) const { }\n"
                   "  void bar() { foo(); }\n"
                   "};");

        ASSERT_EQUALS("[test.cpp:4]: (performance, inconclusive) Technically the member function 'Altren::foo' can be static.\n", errout.str());

        checkConst("class Altren {\n"
                   "public:\n"
                   "  void foo() { delete this; }\n"
                   "  void foo(int i) const { }\n"
                   "  void bar() { foo(1); }\n"
                   "};");

        ASSERT_EQUALS("[test.cpp:4]: (performance, inconclusive) Technically the member function 'Altren::foo' can be static.\n"
                      "[test.cpp:5]: (style, inconclusive) Technically the member function 'Altren::bar' can be const.\n", errout.str());
    }

    void const48() { // ticket 2672
        checkConst("class S0 {\n"
                   "    class S1 {\n"
                   "        class S2 {\n"
                   "            class S3 {\n"
                   "                class S4 { };\n"
                   "            };\n"
                   "        };\n"
                   "    };\n"
                   "};\n"
                   "class TextIterator {\n"
                   "    S0::S1::S2::S3::S4 mCurrent, mSave;\n"
                   "public:\n"
                   "    bool setTagColour();\n"
                   "};\n"
                   "bool TextIterator::setTagColour() {\n"
                   "    mSave = mCurrent;\n"
                   "}");

        ASSERT_EQUALS("", errout.str());
    }

    void const49() { // ticket 2795
        checkConst("class A {\n"
                   "    private:\n"
                   "         std::map<unsigned int,unsigned int> _hash;\n"
                   "    public:\n"
                   "         A() : _hash() {}\n"
                   "         unsigned int fetch(unsigned int key)\n" // cannot be 'const'
                   "         {\n"
                   "             return _hash[key];\n"
                   "         }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const50() { // ticket 2943
        checkConst("class Altren\n"
                   "{\n"
                   "        class SubClass : public std::vector<int>\n"
                   "        {\n"
                   "        };\n"
                   "};\n"
                   "void _setAlign()\n"
                   "{\n"
                   "        if (mTileSize.height > 0) return;\n"
                   "        if (mEmptyView) return;\n"
                   "}");

        ASSERT_EQUALS("", errout.str());
    }

    void const51() { // ticket 3040
        checkConst("class PSIPTable {\n"
                   "public:\n"
                   "    PSIPTable() : _pesdata(0) { }\n"
                   "    const unsigned char* pesdata() const { return _pesdata; }\n"
                   "    unsigned char* pesdata()             { return _pesdata; }\n"
                   "    void SetSection(uint num) { pesdata()[6] = num; }\n"
                   "private:\n"
                   "    unsigned char *_pesdata;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class PESPacket {\n"
                   "public:\n"
                   "    PESPacket() : _pesdata(0) { }\n"
                   "    const unsigned char* pesdata() const { return _pesdata; }\n"
                   "    unsigned char* pesdata()             { return _pesdata; }\n"
                   "private:\n"
                   "    unsigned char *_pesdata;\n"
                   "};\n"
                   "class PSIPTable : public PESPacket\n"
                   "{\n"
                   "public:\n"
                   "    void SetSection(uint num) { pesdata()[6] = num; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const52() { // ticket 3048
        checkConst("class foo {\n"
                   "    void DoSomething(int &a) const { a = 1; }\n"
                   "    void DoSomethingElse() { DoSomething(bar); }\n"
                   "private:\n"
                   "    int bar;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'foo::DoSomething' can be static.\n", errout.str());
    }

    void const53() { // ticket 3049
        checkConst("class A {\n"
                   "  public:\n"
                   "    A() : foo(false) {};\n"
                   "    virtual bool One(bool b = false) { foo = b; return false; }\n"
                   "  private:\n"
                   "    bool foo;\n"
                   "};\n"
                   "class B : public A {\n"
                   "  public:\n"
                   "    B() {};\n"
                   "    bool One(bool b = false) { return false; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const54() { // ticket 3052
        checkConst("class Example {\n"
                   "  public:\n"
                   "    void Clear(void) { Example tmp; (*this) = tmp; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const55() {
        checkConst("class MyObject {\n"
                   "    int tmp;\n"
                   "    MyObject() : tmp(0) {}\n"
                   "public:\n"
                   "    void set(std::stringstream &in) { in >> tmp; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const56() { // ticket #3149
        checkConst("class MyObject {\n"
                   "public:\n"
                   "    void foo(int x) {\n"
                   "    switch (x) { }\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'MyObject::foo' can be static.\n", errout.str());

        checkConst("class A\n"
                   "{\n"
                   "    protected:\n"
                   "        unsigned short f (unsigned short X);\n"
                   "    public:\n"
                   "        A ();\n"
                   "};\n"
                   "\n"
                   "unsigned short A::f (unsigned short X)\n"
                   "{\n"
                   "    enum ERetValues {RET_NOK = 0, RET_OK = 1};\n"
                   "    enum ETypes     {FLOAT_TYPE = 1, INT_TYPE = 2};\n"
                   "\n"
                   "    try\n"
                   "    {\n"
                   "        switch (X)\n"
                   "        {\n"
                   "            case FLOAT_TYPE:\n"
                   "            {\n"
                   "                return RET_OK;\n"
                   "            }\n"
                   "            case INT_TYPE:\n"
                   "            {\n"
                   "                return RET_OK;\n"
                   "            }\n"
                   "            default:\n"
                   "            {\n"
                   "                return RET_NOK;\n"
                   "            }\n"
                   "        }\n"
                   "    }\n"
                   "    catch (...)\n"
                   "    {\n"
                   "        return RET_NOK;\n"
                   "    }\n"
                   "\n"
                   "    return RET_NOK;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:4]: (performance, inconclusive) Technically the member function 'A::f' can be static.\n", errout.str());

        checkConst("class MyObject {\n"
                   "public:\n"
                   "    void foo(int x) {\n"
                   "    for (int i = 0; i < 5; i++) { }\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'MyObject::foo' can be static.\n", errout.str());
    }

    void const57() { // tickets #2669 and #2477
        checkConst("namespace MyGUI\n"
                   "{\n"
                   "  namespace types\n"
                   "  {\n"
                   "    struct TSize {};\n"
                   "    struct TCoord {\n"
                   "      TSize size() const { }\n"
                   "    };\n"
                   "  }\n"
                   "  typedef types::TSize IntSize;\n"
                   "  typedef types::TCoord IntCoord;\n"
                   "}\n"
                   "class SelectorControl\n"
                   "{\n"
                   "  MyGUI::IntSize getSize()\n"
                   "  {\n"
                   "    return mCoordValue.size();\n"
                   "  }\n"
                   "private:\n"
                   "  MyGUI::IntCoord mCoordValue;\n"
                   "};");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (performance, inconclusive) Technically the member function 'MyGUI::types::TCoord::size' can be static.\n"
                           "[test.cpp:15]: (style, inconclusive) Technically the member function 'SelectorControl::getSize' can be const.\n",
                           "[test.cpp:7]: (performance, inconclusive) Technically the member function 'MyGUI::types::TCoord::size' can be static.\n", errout.str());

        checkConst("struct Foo {\n"
                   "    Bar b;\n"
                   "    void foo(Foo f) {\n"
                   "        b.run();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct Bar {\n"
                   "    int i = 0;\n"
                   "    void run() { i++; }\n"
                   "};\n"
                   "struct Foo {\n"
                   "    Bar b;\n"
                   "    void foo(Foo f) {\n"
                   "        b.run();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct Bar {\n"
                   "    void run() const { }\n"
                   "};\n"
                   "struct Foo {\n"
                   "    Bar b;\n"
                   "    void foo(Foo f) {\n"
                   "        b.run();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'Bar::run' can be static.\n"
                      "[test.cpp:6]: (style, inconclusive) Technically the member function 'Foo::foo' can be const.\n", errout.str());
    }

    void const58() {
        checkConst("struct MyObject {\n"
                   "    void foo(Foo f) {\n"
                   "        f.clear();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'MyObject::foo' can be static.\n", errout.str());

        checkConst("struct MyObject {\n"
                   "    int foo(Foo f) {\n"
                   "        return f.length();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'MyObject::foo' can be static.\n", errout.str());

        checkConst("struct MyObject {\n"
                   "    Foo f;\n"
                   "    int foo() {\n"
                   "        return f.length();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct MyObject {\n"
                   "    std::string f;\n"
                   "    int foo() {\n"
                   "        return f.length();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'MyObject::foo' can be const.\n", errout.str());
    }

    void const59() { // ticket #4646
        checkConst("class C {\n"
                   "public:\n"
                   "    inline void operator += (const int &x ) { re += x; }\n"
                   "    friend inline void exp(C & c, const C & x) { }\n"
                   "protected:\n"
                   "    int   re;\n"
                   "    int   im;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const60() { // ticket #3322
        checkConst("class MyString {\n"
                   "public:\n"
                   "    MyString() : m_ptr(0){}\n"
                   "    MyString& operator+=( const MyString& rhs ) {\n"
                   "            delete m_ptr;\n"
                   "            m_ptr = new char[42];\n"
                   "    }\n"
                   "    MyString append( const MyString& str )\n"
                   "    {       return operator+=( str ); } \n"
                   "    char *m_ptr;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
        checkConst("class MyString {\n"
                   "public:\n"
                   "    MyString() : m_ptr(0){}\n"
                   "    MyString& operator+=( const MyString& rhs );\n"
                   "    MyString append( const MyString& str )\n"
                   "    {       return operator+=( str ); } \n"
                   "    char *m_ptr;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const61() { // ticket #5606 - don't crash
        checkConst("class MixerParticipant : public MixerParticipant {\n"
                   "    int GetAudioFrame();\n"
                   "};\n"
                   "int MixerParticipant::GetAudioFrame() {\n"
                   "    return 0;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkConst("class MixerParticipant : public MixerParticipant {\n"
                   "    bool InitializeFileReader() {\n"
                   "       printf(\"music\");\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        // Based on an example from SVN source code causing an endless recursion within CheckClass::isConstMemberFunc()
        // A more complete example including a template declaration like
        //     template<typename K> class Hash{/* ... */};
        // didn't .
        checkConst("template<>\n"
                   "class Hash<void> {\n"
                   "protected:\n"
                   "  typedef Key::key_type key_type;\n"
                   "  void set(const Key& key);\n"
                   "};\n"
                   "template<typename K, int KeySize>\n"
                   "class Hash : private Hash<void> {\n"
                   "  typedef Hash<void> inherited;\n"
                   "  void set(const Key& key) {\n"
                   "      inherited::set(inherited::Key(key));\n"
                   "  }\n"
                   "};\n", 0, false);
        ASSERT_EQUALS("", errout.str());
    }

    void const62() {
        checkConst("class A {\n"
                   "    private:\n"
                   "         std::unordered_map<unsigned int,unsigned int> _hash;\n"
                   "    public:\n"
                   "         A() : _hash() {}\n"
                   "         unsigned int fetch(unsigned int key)\n" // cannot be 'const'
                   "         {\n"
                   "             return _hash[key];\n"
                   "         }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const63() {
        checkConst("struct A {\n"
                   "    std::string s;\n"
                   "    void clear() {\n"
                   "         std::string* p = &s;\n"
                   "         p->clear();\n"
                   "     }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A {\n"
                   "    std::string s;\n"
                   "    void clear() {\n"
                   "         std::string& r = s;\n"
                   "         r.clear();\n"
                   "     }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A {\n"
                   "    std::string s;\n"
                   "    void clear() {\n"
                   "         std::string& r = sth; r = s;\n"
                   "         r.clear();\n"
                   "     }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'A::clear' can be const.\n", errout.str());

        checkConst("struct A {\n"
                   "    std::string s;\n"
                   "    void clear() {\n"
                   "         const std::string* p = &s;\n"
                   "         p->somefunction();\n"
                   "     }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'A::clear' can be const.\n", errout.str());

        checkConst("struct A {\n"
                   "    std::string s;\n"
                   "    void clear() {\n"
                   "         const std::string& r = s;\n"
                   "         r.somefunction();\n"
                   "     }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Technically the member function 'A::clear' can be const.\n", errout.str());
    }

    void const64() {
        checkConst("namespace B {\n"
                   "    namespace D {\n"
                   "        typedef int DKIPtr;\n"
                   "    }\n"
                   "    class ZClass  {\n"
                   "        void set(const ::B::D::DKIPtr& p) {\n"
                   "            membervariable = p;\n"
                   "        }\n"
                   "        ::B::D::DKIPtr membervariable;\n"
                   "    };\n"
                   "}");
        ASSERT_EQUALS("", errout.str());
    }

    void const65() {
        checkConst("template <typename T>\n"
                   "class TemplateClass {\n"
                   "public:\n"
                   "   TemplateClass() { }\n"
                   "};\n"
                   "template <>\n"
                   "class TemplateClass<float> {\n"
                   "public:\n"
                   "   TemplateClass() { }\n"
                   "};\n"
                   "int main() {\n"
                   "    TemplateClass<int> a;\n"
                   "    TemplateClass<float> b;\n"
                   "    return 0;\n"
                   "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const_handleDefaultParameters() {
        checkConst("struct Foo {\n"
                   "    void foo1(int i, int j = 0) {\n"
                   "        return func(this);\n"
                   "    }\n"
                   "    int bar1() {\n"
                   "        return foo1(1);\n"
                   "    }\n"
                   "    int bar2() {\n"
                   "        return foo1(1, 2);\n"
                   "    }\n"
                   "    int bar3() {\n"
                   "        return foo1(1, 2, 3);\n"
                   "    }\n"
                   "    int bar4() {\n"
                   "        return foo1();\n"
                   "    }\n"
                   "    void foo2(int i = 0) {\n"
                   "        return func(this);\n"
                   "    }\n"
                   "    int bar5() {\n"
                   "        return foo2();\n"
                   "    }\n"
                   "    void foo3() {\n"
                   "        return func(this);\n"
                   "    }\n"
                   "    int bar6() {\n"
                   "        return foo3();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:11]: (performance, inconclusive) Technically the member function 'Foo::bar3' can be static.\n"
                      "[test.cpp:14]: (performance, inconclusive) Technically the member function 'Foo::bar4' can be static.\n", errout.str());
    }

    void const_passThisToMemberOfOtherClass() {
        checkConst("struct Foo {\n"
                   "    void foo() {\n"
                   "        Bar b;\n"
                   "        b.takeFoo(this);\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct Foo {\n"
                   "    void foo() {\n"
                   "        Foo f;\n"
                   "        f.foo();\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'Foo::foo' can be static.\n", errout.str());

        checkConst("struct A;\n" // #5839 - operator()
                   "struct B {\n"
                   "    void operator()(A *a);\n"
                   "};\n"
                   "struct A {\n"
                   "    void dostuff() {\n"
                   "        B()(this);\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void assigningPointerToPointerIsNotAConstOperation() {
        checkConst("struct s\n"
                   "{\n"
                   "    int** v;\n"
                   "    void f()\n"
                   "    {\n"
                   "        v = 0;\n"
                   "    }\n"
                   "};\n"
                  );
        ASSERT_EQUALS("", errout.str());
    }

    void assigningArrayElementIsNotAConstOperation() {
        checkConst("struct s\n"
                   "{\n"
                   "    ::std::string v[3];\n"
                   "    void f()\n"
                   "    {\n"
                   "        v[0] = \"Happy new year!\";\n"
                   "    }\n"
                   "};\n"
                  );
        ASSERT_EQUALS("", errout.str());
    }

    // increment/decrement => not const
    void constincdec() {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return ++a; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return --a; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a++; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a--; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return ++a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return --a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a++; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a--; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());
    }

    void constassign1() {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a-=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a+=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a*=-1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a/=-2; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a-=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a+=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a*=-1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a/=-2; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());
    }

    void constassign2() {
        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a-=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a+=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a*=-1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a-=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a+=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a*=-1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a/=-2; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a-=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a+=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a*=-1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a/=-2; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    // increment/decrement array element => not const
    void constincdecarray() {
        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return ++a[0]; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return --a[0]; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]++; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]--; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return ++a[0]; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return --a[0]; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]++; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]--; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());
    }

    void constassignarray() {
        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]-=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]+=1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]*=-1; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]/=-2; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]-=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]+=1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]*=-1; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]/=-2; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'Fred::nextA' can be static.\n", errout.str());
    }

    // return pointer/reference => not const
    void constReturnReference() {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int &getR() { return a; }\n"
                   "    int *getP() { return &a; }"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    // delete member variable => not const (but technically it can, it compiles without errors)
    void constDelete() {
        checkConst("class Fred {\n"
                   "    int *a;\n"
                   "    void clean() { delete a; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    // A function that returns unknown types can't be const (#1579)
    void constLPVOID() {
        checkConst("class Fred {\n"
                   "    UNKNOWN a() { return 0; };\n"
                   "};");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'Fred::a' can be static.\n", "", errout.str());

        // #1579 - HDC
        checkConst("class Fred {\n"
                   "    foo bar;\n"
                   "    UNKNOWN a() { return b; };\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    // a function that calls const functions can be const
    void constFunc() {
        checkConst("class Fred {\n"
                   "    void f() const { };\n"
                   "    void a() { f(); };\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:2]: (performance, inconclusive) Technically the member function 'Fred::f' can be static.\n"
                      "[test.cpp:3]: (style, inconclusive) Technically the member function 'Fred::a' can be const.\n", errout.str());

        // ticket #1593
        checkConst("class A\n"
                   "{\n"
                   "   std::vector<int> m_v;\n"
                   "public:\n"
                   "   A(){}\n"
                   "   unsigned int GetVecSize()  {return m_v.size();}\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:6]: (style, inconclusive) Technically the member function 'A::GetVecSize' can be const.\n", errout.str());

        checkConst("class A\n"
                   "{\n"
                   "   std::vector<int> m_v;\n"
                   "public:\n"
                   "   A(){}\n"
                   "   bool GetVecEmpty()  {return m_v.empty();}\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:6]: (style, inconclusive) Technically the member function 'A::GetVecEmpty' can be const.\n", errout.str());
    }

    void constVirtualFunc() {
        // base class has no virtual function
        checkConst("class A { };\n"
                   "class B : public A {\n"
                   "   int b;\n"
                   "public:\n"
                   "   B() : b(0) { }\n"
                   "   int func() { return b; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:6]: (style, inconclusive) Technically the member function 'B::func' can be const.\n", errout.str());

        checkConst("class A { };\n"
                   "class B : public A {\n"
                   "   int b;\n"
                   "public:\n"
                   "   B() : b(0) { }\n"
                   "   int func();\n"
                   "};\n"
                   "int B::func() { return b; }");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:6]: (style, inconclusive) Technically the member function 'B::func' can be const.\n", errout.str());

        // base class has no virtual function
        checkConst("class A {\n"
                   "public:\n"
                   "    int func();\n"
                   "};\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func() { return b; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:9]: (style, inconclusive) Technically the member function 'B::func' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    int func();\n"
                   "};\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int B::func() { return b; }");
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:9]: (style, inconclusive) Technically the member function 'B::func' can be const.\n", errout.str());

        // base class has virtual function
        checkConst("class A {\n"
                   "public:\n"
                   "    virtual int func();\n"
                   "};\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func() { return b; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    virtual int func();\n"
                   "};\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int B::func() { return b; }");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    virtual int func() = 0;\n"
                   "};\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int B::func() { return b; }");
        ASSERT_EQUALS("", errout.str());

        // base class has no virtual function
        checkConst("class A {\n"
                   "    int a;\n"
                   "public:\n"
                   "    A() : a(0) { }\n"
                   "    int func() { return a; }\n"
                   "};\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func() { return b; }\n"
                   "};\n"
                   "class C : public B {\n"
                   "    int c;\n"
                   "public:\n"
                   "    C() : c(0) { }\n"
                   "    int func() { return c; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:5]: (style, inconclusive) Technically the member function 'A::func' can be const.\n"
                      "[test.cpp:11]: (style, inconclusive) Technically the member function 'B::func' can be const.\n"
                      "[test.cpp:17]: (style, inconclusive) Technically the member function 'C::func' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "    int a;\n"
                   "public:\n"
                   "    A() : a(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int A::func() { return a; }\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int B::func() { return b; }\n"
                   "class C : public B {\n"
                   "    int c;\n"
                   "public:\n"
                   "    C() : c(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int C::func() { return c; }");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5]: (style, inconclusive) Technically the member function 'A::func' can be const.\n"
                      "[test.cpp:14] -> [test.cpp:12]: (style, inconclusive) Technically the member function 'B::func' can be const.\n"
                      "[test.cpp:21] -> [test.cpp:19]: (style, inconclusive) Technically the member function 'C::func' can be const.\n", errout.str());

        // base class has virtual function
        checkConst("class A {\n"
                   "    int a;\n"
                   "public:\n"
                   "    A() : a(0) { }\n"
                   "    virtual int func() { return a; }\n"
                   "};\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func() { return b; }\n"
                   "};\n"
                   "class C : public B {\n"
                   "    int c;\n"
                   "public:\n"
                   "    C() : c(0) { }\n"
                   "    int func() { return c; }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "    int a;\n"
                   "public:\n"
                   "    A() : a(0) { }\n"
                   "    virtual int func();\n"
                   "};\n"
                   "int A::func() { return a; }\n"
                   "class B : public A {\n"
                   "    int b;\n"
                   "public:\n"
                   "    B() : b(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int B::func() { return b; }\n"
                   "class C : public B {\n"
                   "    int c;\n"
                   "public:\n"
                   "    C() : c(0) { }\n"
                   "    int func();\n"
                   "};\n"
                   "int C::func() { return c; }");
        ASSERT_EQUALS("", errout.str());

        // ticket #1311
        checkConst("class X {\n"
                   "    int x;\n"
                   "public:\n"
                   "    X(int x) : x(x) { }\n"
                   "    int getX() { return x; }\n"
                   "};\n"
                   "class Y : public X {\n"
                   "    int y;\n"
                   "public:\n"
                   "    Y(int x, int y) : X(x), y(y) { }\n"
                   "    int getY() { return y; }\n"
                   "};\n"
                   "class Z : public Y {\n"
                   "    int z;\n"
                   "public:\n"
                   "    Z(int x, int y, int z) : Y(x, y), z(z) { }\n"
                   "    int getZ() { return z; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:5]: (style, inconclusive) Technically the member function 'X::getX' can be const.\n"
                      "[test.cpp:11]: (style, inconclusive) Technically the member function 'Y::getY' can be const.\n"
                      "[test.cpp:17]: (style, inconclusive) Technically the member function 'Z::getZ' can be const.\n", errout.str());

        checkConst("class X {\n"
                   "    int x;\n"
                   "public:\n"
                   "    X(int x) : x(x) { }\n"
                   "    int getX();\n"
                   "};\n"
                   "int X::getX() { return x; }\n"
                   "class Y : public X {\n"
                   "    int y;\n"
                   "public:\n"
                   "    Y(int x, int y) : X(x), y(y) { }\n"
                   "    int getY();\n"
                   "};\n"
                   "int Y::getY() { return y; }\n"
                   "class Z : public Y {\n"
                   "    int z;\n"
                   "public:\n"
                   "    Z(int x, int y, int z) : Y(x, y), z(z) { }\n"
                   "    int getZ();\n"
                   "};\n"
                   "int Z::getZ() { return z; }");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5]: (style, inconclusive) Technically the member function 'X::getX' can be const.\n"
                      "[test.cpp:14] -> [test.cpp:12]: (style, inconclusive) Technically the member function 'Y::getY' can be const.\n"
                      "[test.cpp:21] -> [test.cpp:19]: (style, inconclusive) Technically the member function 'Z::getZ' can be const.\n", errout.str());
    }

    void constIfCfg() {
        const char code[] = "struct foo {\n"
                            "    int i;\n"
                            "    void f() {\n"
                            //"#ifdef ABC\n"
                            //"        i = 4;\n"
                            //"endif\n"
                            "    }\n"
                            "};";

        checkConst(code, &settings0, true);
        ASSERT_EQUALS("[test.cpp:3]: (performance, inconclusive) Technically the member function 'foo::f' can be static.\n", errout.str());

        checkConst(code, &settings0, false); // TODO: Set inconclusive to true (preprocess it)
        ASSERT_EQUALS("", errout.str());
    }

    void constFriend() { // ticket #1921
        const char code[] = "class foo {\n"
                            "    friend void f() { }\n"
                            "};";
        checkConst(code);
        ASSERT_EQUALS("", errout.str());
    }

    void constUnion() { // ticket #2111
        checkConst("class foo {\n"
                   "public:\n"
                   "    union {\n"
                   "        int i;\n"
                   "        float f;\n"
                   "    } d;\n"
                   "    void setf(float x) {\n"
                   "        d.f = x;\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void constArrayOperator() {
        checkConst("struct foo {\n"
                   "    int x;\n"
                   "    int y[5][724];\n"
                   "    T a() {\n"
                   "        return y[x++][6];\n"
                   "    }\n"
                   "    T b() {\n"
                   "        return y[1][++x];\n"
                   "    }\n"
                   "    T c() {\n"
                   "        return y[1][6];\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:10]: (style, inconclusive) Technically the member function 'foo::c' can be const.\n", errout.str());
    }

    void constRangeBasedFor() { // #5514
        checkConst("class Fred {\n"
                   "    int array[256];\n"
                   "public:\n"
                   "    void f1() {\n"
                   "        for (auto & e : array)\n"
                   "            foo(e);\n"
                   "    }\n"
                   "    void f2() {\n"
                   "        for (const auto & e : array)\n"
                   "            foo(e);\n"
                   "    }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:8]: (style, inconclusive) Technically the member function 'Fred::f2' can be const.\n", errout.str());
    }

    void const_shared_ptr() { // #8674
        checkConst("class Fred {\n"
                   "public:\n"
                   "    std::shared_ptr<Data> getData();\n"
                   "private:\n"
                   "     std::shared_ptr<Data> data;\n"
                   "};\n"
                   "\n"
                   "std::shared_ptr<Data> Fred::getData() { return data; }");
        ASSERT_EQUALS("", errout.str());
    }

    void constPtrToConstPtr() {
        checkConst("class Fred {\n"
                   "public:\n"
                   "    const char *const *data;\n"
                   "    const char *const *getData() { return data; }\n}");
        ASSERT_EQUALS("[test.cpp:4]: (style, inconclusive) Technically the member function 'Fred::getData' can be const.\n", errout.str());
    }

    void checkInitializerListOrder(const char code[]) {
        // Clear the error log
        errout.str("");

        // Check..
        settings0.inconclusive = true;

        // Tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        CheckClass checkClass(&tokenizer, &settings0, this);
        checkClass.initializerListOrder();
    }

    void initializerListOrder() {
        checkInitializerListOrder("class Fred {\n"
                                  "    int a, b, c;\n"
                                  "public:\n"
                                  "    Fred() : c(0), b(0), a(0) { }\n"
                                  "};");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) Member variable 'Fred::b' is in the wrong place in the initializer list.\n"
                      "[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) Member variable 'Fred::a' is in the wrong place in the initializer list.\n", errout.str());

        checkInitializerListOrder("class Fred {\n"
                                  "    int a, b, c;\n"
                                  "public:\n"
                                  "    Fred() : c{0}, b{0}, a{0} { }\n"
                                  "};");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) Member variable 'Fred::b' is in the wrong place in the initializer list.\n"
                      "[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) Member variable 'Fred::a' is in the wrong place in the initializer list.\n", errout.str());
    }

    void checkInitializationListUsage(const char code[]) {
        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings.addEnabled("performance");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.initializationListUsage();
    }

    void initializerListUsage() {
        checkInitializationListUsage("enum Enum { C = 0 };\n"
                                     "class Fred {\n"
                                     "    int a;\n"  // No message for builtin types: No performance gain
                                     "    int* b;\n" // No message for pointers: No performance gain
                                     "    Enum c;\n" // No message for enums: No performance gain
                                     "    Fred() { a = 0; b = 0; c = C; }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class Fred {\n"
                                     "    std::string s;\n"
                                     "    Fred() { a = 0; s = \"foo\"; }\n"
                                     "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Variable 's' is assigned in constructor body. Consider performing initialization in initialization list.\n", errout.str());

        checkInitializationListUsage("class Fred {\n"
                                     "    std::string& s;\n" // Message is invalid for references, since their initialization in initializer list is required anyway and behaves different from assignment (#5004)
                                     "    Fred(const std::string& s_) : s(s_) { s = \"foo\"; }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class Fred {\n"
                                     "    std::vector<int> v;\n"
                                     "    Fred() { v = unknown; }\n"
                                     "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Variable 'v' is assigned in constructor body. Consider performing initialization in initialization list.\n", errout.str());

        checkInitializationListUsage("class C { std::string s; };\n"
                                     "class Fred {\n"
                                     "    C c;\n"
                                     "    Fred() { c = unknown; }\n"
                                     "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance) Variable 'c' is assigned in constructor body. Consider performing initialization in initialization list.\n", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C c;\n"
                                     "    Fred() { c = unknown; }\n"
                                     "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance) Variable 'c' is assigned in constructor body. Consider performing initialization in initialization list.\n", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C c;\n"
                                     "    Fred(Fred const & other) { c = other.c; }\n"
                                     "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance) Variable 'c' is assigned in constructor body. Consider performing initialization in initialization list.\n", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C c;\n"
                                     "    Fred(Fred && other) { c = other.c; }\n"
                                     "};");
        ASSERT_EQUALS("[test.cpp:4]: (performance) Variable 'c' is assigned in constructor body. Consider performing initialization in initialization list.\n", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C a;\n"
                                     "    Fred() { initB(); a = b; }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C a;\n"
                                     "    Fred() : a(0) { if(b) a = 0; }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C a[5];\n"
                                     "    Fred() { for(int i = 0; i < 5; i++) a[i] = 0; }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C a; int b;\n"
                                     "    Fred() : b(5) { a = b; }\n" // Don't issue a message here: You actually could move it to the initialization list, but it would cause problems if you change the order of the variable declarations.
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class C;\n"
                                     "class Fred {\n"
                                     "    C a;\n"
                                     "    Fred() { try { a = new int; } catch(...) {} }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class Fred {\n"
                                     "    std::string s;\n"
                                     "    Fred() { s = toString((size_t)this); }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class Fred {\n"
                                     "    std::string a;\n"
                                     "    std::string foo();\n"
                                     "    Fred() { a = foo(); }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class Fred {\n"
                                     "    std::string a;\n"
                                     "    Fred() { a = foo(); }\n"
                                     "};");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Variable 'a' is assigned in constructor body. Consider performing initialization in initialization list.\n", errout.str());

        checkInitializationListUsage("class Fred {\n" // #4332
                                     "    static std::string s;\n"
                                     "    Fred() { s = \"foo\"; }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class Fred {\n" // #5640
                                     "    std::string s;\n"
                                     "    Fred() {\n"
                                     "        char str[2];\n"
                                     "        str[0] = c;\n"
                                     "        str[1] = 0;\n"
                                     "        s = str;\n"
                                     "    }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class B {\n" // #5640
                                     "    std::shared_ptr<A> _d;\n"
                                     "    B(const B& other) : _d(std::make_shared<A>()) {\n"
                                     "        *_d = *other._d;\n"
                                     "    }\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("class Bar {\n" // #8466
                                     "public:\n"
                                     "    explicit Bar(const Bar &bar) : Bar{bar.s} {}\n"
                                     "    explicit Bar(const char s) : s{s} {}\n"
                                     "private:\n"
                                     "    char s;\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());

        checkInitializationListUsage("unsigned bar(std::string);\n" // #8291
                                     "class Foo {\n"
                                     "public:\n"
                                     "    int a_, b_;\n"
                                     "    Foo(int a, int b) : a_(a), b_(b) {}\n"
                                     "    Foo(int a, const std::string& b) : Foo(a, bar(b)) {}\n"
                                     "};");
        ASSERT_EQUALS("", errout.str());
    }


    void checkSelfInitialization(const char code []) {
        // Clear the error log
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        CheckClass checkClass(&tokenizer, &settings0, this);
        checkClass.checkSelfInitialization();
    }

    void selfInitialization() {
        checkSelfInitialization("class Fred {\n"
                                "    int i;\n"
                                "    Fred() : i(i) {\n"
                                "    }\n"
                                "};");
        ASSERT_EQUALS("[test.cpp:3]: (error) Member variable 'i' is initialized by itself.\n", errout.str());

        checkSelfInitialization("class Fred {\n"
                                "    int i;\n"
                                "    Fred() : i{i} {\n"
                                "    }\n"
                                "};");
        ASSERT_EQUALS("[test.cpp:3]: (error) Member variable 'i' is initialized by itself.\n", errout.str());

        checkSelfInitialization("class Fred {\n"
                                "    int i;\n"
                                "    Fred();\n"
                                "};\n"
                                "Fred::Fred() : i(i) {\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Member variable 'i' is initialized by itself.\n", errout.str());

        checkSelfInitialization("class Fred {\n"
                                "    std::string s;\n"
                                "    Fred() : s(s) {\n"
                                "    }\n"
                                "};");
        ASSERT_EQUALS("[test.cpp:3]: (error) Member variable 's' is initialized by itself.\n", errout.str());

        checkSelfInitialization("class Fred {\n"
                                "    int x;\n"
                                "    Fred(int x);\n"
                                "};\n"
                                "Fred::Fred(int x) : x(x) { }\n"
                               );
        ASSERT_EQUALS("", errout.str());

        checkSelfInitialization("class Fred {\n"
                                "    int x;\n"
                                "    Fred(int x);\n"
                                "};\n"
                                "Fred::Fred(int x) : x{x} { }\n"
                               );
        ASSERT_EQUALS("", errout.str());

        checkSelfInitialization("class Fred {\n"
                                "    std::string s;\n"
                                "    Fred(const std::string& s) : s(s) {\n"
                                "    }\n"
                                "};");
        ASSERT_EQUALS("", errout.str());

        checkSelfInitialization("class Fred {\n"
                                "    std::string s;\n"
                                "    Fred(const std::string& s) : s{s} {\n"
                                "    }\n"
                                "};");
        ASSERT_EQUALS("", errout.str());

        checkSelfInitialization("struct Foo : Bar {\n"
                                "    int i;\n"
                                "    Foo(int i)\n"
                                "        : Bar(""), i(i) {}\n"
                                "};");
        ASSERT_EQUALS("", errout.str());

        checkSelfInitialization("struct Foo : std::Bar {\n" // #6073
                                "    int i;\n"
                                "    Foo(int i)\n"
                                "        : std::Bar(""), i(i) {}\n"
                                "};");
        ASSERT_EQUALS("", errout.str());

        checkSelfInitialization("struct Foo : std::Bar {\n" // #6073
                                "    int i;\n"
                                "    Foo(int i)\n"
                                "        : std::Bar(""), i{i} {}\n"
                                "};");
        ASSERT_EQUALS("", errout.str());
    }

    void checkVirtualFunctionCall(const char code[], Settings *s = 0, bool inconclusive = true) {
        // Clear the error log
        errout.str("");

        // Check..
        if (!s) {
            static Settings settings_;
            s = &settings_;
            s->addEnabled("warning");
        }
        s->inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(s, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        CheckClass checkClass(&tokenizer, s, this);
        checkClass.checkVirtualFunctionCallInConstructor();
    }

    void virtualFunctionCallInConstructor() {
        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual int f() { return 1; }\n"
                                 "    A();\n"
                                 "};\n"
                                 "A::A()\n"
                                 "{f();}\n");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:3]: (warning) Virtual function 'f' is called from constructor 'A()' at line 7. Dynamic binding is not used.\n", errout.str());

        checkVirtualFunctionCall("class A {\n"
                                 "    virtual int f();\n"
                                 "    A() {f();}\n"
                                 "};\n"
                                 "int A::f() { return 1; }\n");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Virtual function 'f' is called from constructor 'A()' at line 3. Dynamic binding is not used.\n", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    A() { A::f(); }\n"
                                 "    virtual void f() {}\n"
                                 "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void pureVirtualFunctionCall() {
        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual void pure()=0;\n"
                                 "    A();\n"
                                 "};\n"
                                 "A::A()\n"
                                 "{pure();}\n");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in constructor.\n", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual int pure()=0;\n"
                                 "    A();\n"
                                 "    int m;\n"
                                 "};\n"
                                 "A::A():m(A::pure())\n"
                                 "{}\n");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in constructor.\n", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual void pure()=0; \n"
                                 "    virtual ~A(); \n"
                                 "    int m; \n"
                                 "};\n"
                                 "A::~A()\n"
                                 "{pure();}\n");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in destructor.\n", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual void pure()=0;\n"
                                 "    void nonpure()\n"
                                 "    {pure();}\n"
                                 "    A(); \n"
                                 "};\n"
                                 "A::A()\n"
                                 "{nonpure();}\n");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:5] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in constructor.\n", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual int pure()=0;\n"
                                 "    int nonpure()\n"
                                 "    {return pure();}\n"
                                 "    A(); \n"
                                 "    int m;\n"
                                 "};\n"
                                 "A::A():m(nonpure())\n"
                                 "{}\n");
        TODO_ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:5] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in constructor.\n", "", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual void pure()=0; \n"
                                 "    void nonpure()\n"
                                 "    {pure();}\n"
                                 "    virtual ~A();\n"
                                 "    int m;\n"
                                 "};\n"
                                 "A::~A()\n"
                                 "{nonpure();}\n");
        ASSERT_EQUALS("[test.cpp:10] -> [test.cpp:5] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in destructor.\n", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual void pure()=0;\n"
                                 "    A(bool b);\n"
                                 "};\n"
                                 "A::A(bool b)\n"
                                 "{if (b) pure();}\n");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in constructor.\n", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual void pure()=0;\n"
                                 "    virtual ~A();\n"
                                 "    int m;\n"
                                 "};\n"
                                 "A::~A()\n"
                                 "{if (b) pure();}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:3]: (warning) Call of pure virtual function 'pure' in destructor.\n", errout.str());

        // #5831
        checkVirtualFunctionCall("class abc {\n"
                                 "public:\n"
                                 "  virtual ~abc() throw() {}\n"
                                 "  virtual void def(void* g) throw () = 0;\n"
                                 "};");
        ASSERT_EQUALS("", errout.str());

        // #4992
        checkVirtualFunctionCall("class CMyClass {\n"
                                 "    std::function< void(void) > m_callback;\n"
                                 "public:\n"
                                 "    CMyClass() {\n"
                                 "        m_callback = [this]() { return VirtualMethod(); };\n"
                                 "    }\n"
                                 "    virtual void VirtualMethod() = 0;\n"
                                 "};");
        ASSERT_EQUALS("", errout.str());
    }

    void pureVirtualFunctionCallOtherClass() {
        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual void pure()=0;\n"
                                 "    A(const A & a);\n"
                                 "};\n"
                                 "A::A(const A & a)\n"
                                 "{a.pure();}\n");
        ASSERT_EQUALS("", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual void pure()=0;\n"
                                 "    A();\n"
                                 "};\n"
                                 "class B\n"
                                 "{\n"
                                 "    virtual void pure()=0;\n"
                                 "};\n"
                                 "A::A()\n"
                                 "{B b; b.pure();}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void pureVirtualFunctionCallWithBody() {
        checkVirtualFunctionCall("class A\n"
                                 "{\n"
                                 "    virtual void pureWithBody()=0;\n"
                                 "    A();\n"
                                 "};\n"
                                 "A::A()\n"
                                 "{pureWithBody();}\n"
                                 "void A::pureWithBody()\n"
                                 "{}\n");
        ASSERT_EQUALS("", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual void pureWithBody()=0;\n"
                                 "    void nonpure()\n"
                                 "    {pureWithBody();}\n"
                                 "    A(); \n"
                                 "};\n"
                                 "A::A()\n"
                                 "{nonpure();}\n"
                                 "void A::pureWithBody()\n"
                                 "{}\n");
        ASSERT_EQUALS("", errout.str());

    }

    void pureVirtualFunctionCallPrevented() {
        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual void pure()=0;\n"
                                 "    void nonpure(bool bCallPure)\n"
                                 "    { if (bCallPure) pure();}\n"
                                 "    A(); \n"
                                 "};\n"
                                 "A::A()\n"
                                 "{nonpure(false);}\n");
        ASSERT_EQUALS("", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual void pure()=0;\n"
                                 "    void nonpure(bool bCallPure)\n"
                                 "    { if (!bCallPure) ; else pure();}\n"
                                 "    A(); \n"
                                 "};\n"
                                 "A::A()\n"
                                 "{nonpure(false);}\n");
        ASSERT_EQUALS("", errout.str());

        checkVirtualFunctionCall("class A\n"
                                 " {\n"
                                 "    virtual void pure()=0;\n"
                                 "    void nonpure(bool bCallPure)\n"
                                 "    {\n"
                                 "        switch (bCallPure) {\n"
                                 "        case true: pure(); break;\n"
                                 "        }\n"
                                 "    }\n"
                                 "    A(); \n"
                                 "};\n"
                                 "A::A()\n"
                                 "{nonpure(false);}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void checkUnsafeClassDivZero(const char code[]) {
        // Clear the error log
        errout.str("");
        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.checkUnsafeClassDivZero(true);
    }

    void unsafeClassDivZero() {
        checkUnsafeClassDivZero("class A {\n"
                                "public:\n"
                                "  void dostuff(int x);\n"
                                "}\n"
                                "void A::dostuff(int x) { int a = 1000 / x; }");
        ASSERT_EQUALS("[test.cpp:5]: (style) Public interface of A is not safe. When calling A::dostuff(), if parameter x is 0 that leads to division by zero.\n", errout.str());

        checkUnsafeClassDivZero("class A {\n"
                                "public:\n"
                                "  void f1();\n"
                                "  void f2(int x);\n"
                                "}\n"
                                "void A::f1() {}\n"
                                "void A::f2(int x) { int a = 1000 / x; }");
        ASSERT_EQUALS("[test.cpp:7]: (style) Public interface of A is not safe. When calling A::f2(), if parameter x is 0 that leads to division by zero.\n", errout.str());

        checkUnsafeClassDivZero("class A {\n"
                                "public:\n"
                                "  void operator/(int x);\n"
                                "}\n"
                                "void A::operator/(int x) { int a = 1000 / x; }");
        ASSERT_EQUALS("", errout.str());
    }

    void checkOverride(const char code[]) {
        // Clear the error log
        errout.str("");
        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.checkOverride();
    }

    void override1() {
        checkOverride("class Base { virtual void f(); };\n"
                      "class Derived : Base { virtual void f(); };");
        ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:2]: (style) The function 'f' overrides a function in a base class but is not marked with a 'override' specifier.\n", errout.str());

        checkOverride("class Base { virtual void f(); };\n"
                      "class Derived : Base { virtual void f() override; };");
        ASSERT_EQUALS("", errout.str());

        checkOverride("class Base { virtual void f(); };\n"
                      "class Derived : Base { virtual void f() final; };");
        ASSERT_EQUALS("", errout.str());
    }

    void overrideCVRefQualifiers() {
        checkOverride("class Base { virtual void f(); };\n"
                      "class Derived : Base { void f() const; }");
        ASSERT_EQUALS("", errout.str());

        checkOverride("class Base { virtual void f(); };\n"
                      "class Derived : Base { void f() volatile; }");
        ASSERT_EQUALS("", errout.str());

        checkOverride("class Base { virtual void f(); };\n"
                      "class Derived : Base { void f() &; }");
        ASSERT_EQUALS("", errout.str());

        checkOverride("class Base { virtual void f(); };\n"
                      "class Derived : Base { void f() &&; }");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestClass)
