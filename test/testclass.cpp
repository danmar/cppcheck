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

class TestClass : public TestFixture
{
public:
    TestClass() : TestFixture("TestClass")
    { }

private:

    void run()
    {
        TEST_CASE(virtualDestructor1);      // Base class not found => no error
        TEST_CASE(virtualDestructor2);      // Base class doesn't have a destructor
        TEST_CASE(virtualDestructor3);      // Base class has a destructor, but it's not virtual
        TEST_CASE(virtualDestructor4);      // Derived class doesn't have a destructor => no error
        TEST_CASE(virtualDestructor5);      // Derived class has empty destructor => no error
        TEST_CASE(virtualDestructor6);      // only report error if base class pointer that points at derived class is deleted
        TEST_CASE(virtualDestructorProtected);
        TEST_CASE(virtualDestructorInherited);
        TEST_CASE(virtualDestructorTemplate);

        TEST_CASE(uninitVar1);
        TEST_CASE(uninitVar2);
        TEST_CASE(uninitVar3);
        TEST_CASE(uninitVar4);
        TEST_CASE(uninitVar5);
        TEST_CASE(uninitVar6);
        TEST_CASE(uninitVar7);
        TEST_CASE(uninitVar8);
        TEST_CASE(uninitVar9); // ticket #1730
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
        TEST_CASE(uninitVarArray2D);
        TEST_CASE(uninitVarStruct1); // ticket #2172
        TEST_CASE(uninitVarStruct2); // ticket #838
        TEST_CASE(uninitMissingFuncDef);	// can't expand function in constructor
        TEST_CASE(privateCtor1);        	// If constructor is private..
        TEST_CASE(privateCtor2);        	// If constructor is private..
        TEST_CASE(function);            	// Function is not variable
        TEST_CASE(uninitVarHeader1);    	// Class is defined in header
        TEST_CASE(uninitVarHeader2);    	// Class is defined in header
        TEST_CASE(uninitVarHeader3);    	// Class is defined in header
        TEST_CASE(uninitVarPublished);  	// Borland C++: Variables in the published section are auto-initialized
        TEST_CASE(uninitOperator);      	// No FP about uninitialized 'operator[]'
        TEST_CASE(uninitFunction1);			// No FP when initialized in function
        TEST_CASE(uninitFunction2);			// No FP when initialized in function
        TEST_CASE(uninitFunction3);			// No FP when initialized in function
        TEST_CASE(uninitFunction4);
        TEST_CASE(uninitSameClassName);		// No FP when two classes have the same name
        TEST_CASE(uninitFunctionOverload); 	// No FP when there are overloaded functions
        TEST_CASE(uninitJava);              // Java: no FP when variable is initialized in declaration
        TEST_CASE(uninitVarOperatorEqual);  // ticket #2415

        TEST_CASE(noConstructor1);
        TEST_CASE(noConstructor2);
        TEST_CASE(noConstructor3);
        TEST_CASE(noConstructor4);
        TEST_CASE(noConstructor5);

        TEST_CASE(operatorEq1);
        TEST_CASE(operatorEq2);
        TEST_CASE(operatorEq3); // ticket #3051
        TEST_CASE(operatorEq4); // ticket #3114
        TEST_CASE(operatorEqRetRefThis1);
        TEST_CASE(operatorEqRetRefThis2); // ticket #1323
        TEST_CASE(operatorEqRetRefThis3); // ticket #1405
        TEST_CASE(operatorEqRetRefThis4); // ticket #1451
        TEST_CASE(operatorEqRetRefThis5); // ticket #1550
        TEST_CASE(operatorEqRetRefThis6); // ticket #2479
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
        TEST_CASE(const18); // ticket #1563
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
        TEST_CASE(const52); // ticket #3049
        TEST_CASE(const53); // ticket #3052
        TEST_CASE(const54);
        TEST_CASE(const55); // ticket #3149
        TEST_CASE(assigningPointerToPointerIsNotAConstOperation);
        TEST_CASE(assigningArrayElementIsNotAConstOperation);
        TEST_CASE(constoperator1);  // operator< can often be const
        TEST_CASE(constoperator2);	// operator<<
        TEST_CASE(constoperator3);
        TEST_CASE(constoperator4);
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
        TEST_CASE(constUnion);	// ticket #2111 - fp when there are union

        TEST_CASE(initializerList);
    }

    // Check the operator Equal
    void checkOpertorEq(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.operatorEq();
    }

    void operatorEq1()
    {
        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "private:\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}\n"
                       "private:\n"
                       "    void operator=(const A&);\n"
                       "};\n");
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
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'\n"
                      "[test.cpp:9]: (style) 'B::operator=' should return 'B &'\n", errout.str());

        checkOpertorEq("struct A\n"
                       "{\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'A::operator=' should return 'A &'\n", errout.str());
    }

    void operatorEq2()
    {
        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void * operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    A * operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    const A & operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    B & operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'A::operator=' should return 'A &'\n", errout.str());
    }

    void operatorEq3() // ticket #3051
    {
        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    A * operator=(const A*);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEq4() // ticket #3114 (infinite loop)
    {
        checkOpertorEq("struct A {\n"
                       "    A& operator=(A const& a) { return operator=(&a); }\n"
                       "    A& operator=(const A*) { return *this; }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Check that operator Equal returns reference to this
    void checkOpertorEqRetRefThis(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.operatorEqRetRefThis();
    }

    void operatorEqRetRefThis1()
    {
        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return a; }\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return a; }\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a) { return a; }\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return *this; }\n"
            "    };\n"
            "};\n");
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
            "};\n");
        ASSERT_EQUALS("[test.cpp:7]: (style) 'operator=' should return reference to self\n", errout.str());

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
            "A::B & A::B::operator=(const A::B &b) { return *this; }\n");
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
            "A::B & A::B::operator=(const A::B &b) { return b; }\n");
        ASSERT_EQUALS("[test.cpp:10]: (style) 'operator=' should return reference to self\n", errout.str());
    }

    void operatorEqRetRefThis2()
    {
        // ticket # 1323
        checkOpertorEqRetRefThis(
            "class szp\n"
            "{\n"
            "  szp &operator =(int *other) {};\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class szp\n"
            "{\n"
            "  szp &operator =(int *other);\n"
            "};\n"
            "szp &szp::operator =(int *other) {}");
        ASSERT_EQUALS("[test.cpp:5]: (style) 'operator=' should return reference to self\n", errout.str());
    }

    void operatorEqRetRefThis3()
    {
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

    void operatorEqRetRefThis4()
    {
        // ticket # 1451
        checkOpertorEqRetRefThis(
            "P& P::operator = (const P& pc)\n"
            "{\n"
            "  return (P&)(*this += pc);\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqRetRefThis5()
    {
        // ticket # 1550
        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "    A & operator=(const A &a) { }\n"
            "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A :: operator=(const A &a) { }");
        ASSERT_EQUALS("[test.cpp:5]: (style) 'operator=' should return reference to self\n", errout.str());
    }

    void operatorEqRetRefThis6() // ticket #2478 (segmentation fault)
    {
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
            "}\n");
    }

    // Check that operator Equal checks for assignment to self
    void checkOpertorEqToSelf(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.operatorEqToSelf();
    }

    void operatorEqToSelf1()
    {
        // this test has an assignment test but it is not needed
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { if (&a != this) { } return *this; }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but it is not needed
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};\n");
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
            "};\n");
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
            "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (warning) 'operator=' should check for assignment to self\n", errout.str());

        // this test has an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { if (&a != this) { } return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
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
            "}\n");
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
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (warning) 'operator=' should check for assignment to self\n", errout.str());

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

    void operatorEqToSelf2()
    {
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
            "};\n");
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
            "};\n");
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
            "};\n");
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
            "};\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) 'operator=' should check for assignment to self\n", errout.str());

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
            "A::B & A::B::operator=(const A::B &b) { if (&b != this) { } return *this; }\n");
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
            "A::B & A::B::operator=(const A::B &b) { return *this; }\n");
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
            " }\n");
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
            " }\n");
        ASSERT_EQUALS("[test.cpp:11]: (warning) 'operator=' should check for assignment to self\n", errout.str());
    }

    void operatorEqToSelf3()
    {
        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};\n");
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
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
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
            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf4()
    {
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
            "};\n");
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
            "};\n");
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
            "A::B & A::B::operator=(const A::B &b) { return *this; }\n");
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
            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf5()
    {
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
    }

    void operatorEqToSelf6()
    {
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
        ASSERT_EQUALS("[test.cpp:4]: (warning) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:8]: (warning) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (warning) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:8]: (warning) 'operator=' should check for assignment to self\n", errout.str());
    }

    void operatorEqToSelf7()
    {
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

    void operatorEqToSelf8()
    {
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

    void operatorEqToSelf9()
    {
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
            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Check that base classes have virtual destructors
    void checkVirtualDestructor(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.virtualDestructor();
    }

    void virtualDestructor1()
    {
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

    void virtualDestructor2()
    {
        // Base class doesn't have a destructor

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : protected Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

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

    void virtualDestructor3()
    {
        // Base class has a destructor, but it's not virtual

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : protected Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());
    }

    void virtualDestructor4()
    {
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

    void virtualDestructor5()
    {
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

    void virtualDestructor6()
    {
        // Only report error if base class pointer is deleted that
        // points at derived class

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };"
                               "Base *base = new Derived;\n"
                               "delete base;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructorProtected()
    {
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
                               "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructorInherited()
    {
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
                               "};\n");
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
                               "};\n");
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
                               "};\n");
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
                               "};\n");
        ASSERT_EQUALS("", errout.str());

        // class A doesn't inherit virtual destructor from class Base -> error
        checkVirtualDestructor("class Base\n"
                               "{\n"
                               "public:\n"
                               "~Base() {}\n"
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
                               "};\n");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Class A which is inherited by class B does not have a virtual destructor\n",
                           "", errout.str());
    }

    void virtualDestructorTemplate()
    {
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
        ASSERT_EQUALS("[test.cpp:9]: (error) Class AA<double> which is inherited by class B does not have a virtual destructor\n", errout.str());
    }

    void checkUninitVar(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.constructors();
    }

    void uninitVar1()
    {
        checkUninitVar("enum ECODES\n"
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
                       "};\n");

        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Fred::_code' is not initialized in the constructor.\n", errout.str());


        checkUninitVar("class A{};\n"
                       "\n"
                       "class B : public A\n"
                       "{\n"
                       "public:\n"
                       "  B() {}\n"
                       "private:\n"
                       "  float f;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Member variable 'B::f' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("class C\n"
                       "{\n"
                       "    FILE *fp;\n"
                       "\n"
                       "public:\n"
                       "    C(FILE *fp);\n"
                       "};\n"
                       "\n"
                       "C::C(FILE *fp) {\n"
                       "    C::fp = fp;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar2()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { (*this).i = 0; }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar3()
    {
        // No FP when struct has constructor
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    Foo() { }\n"
                       "private:\n"
                       "    struct Bar {\n"
                       "        Bar();\n"
                       "    };\n"
                       "    Bar bars[2];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        // Using struct that doesn't have constructor
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    Foo() { }\n"
                       "private:\n"
                       "    struct Bar {\n"
                       "        int x;\n"
                       "    };\n"
                       "    Bar bars[2];\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Foo::bars' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar4()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    Foo() { bar.x = 0; }\n"
                       "private:\n"
                       "    struct Bar {\n"
                       "        int x;\n"
                       "    };\n"
                       "    struct Bar bar;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar5()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    Foo() { }\n"
                       "    Foo &operator=(const Foo &)\n"
                       "    { return *this; }\n"
                       "    static int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar6()
    {
        checkUninitVar("class Foo : public Bar\n"
                       "{\n"
                       "public:\n"
                       "    Foo(int i) : Bar(mi=i) { }\n"
                       "private:\n"
                       "    int mi;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar7()
    {
        checkUninitVar("class Foo {\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar8()
    {
        checkUninitVar("class Foo {\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Member variable 'Foo::a' is not assigned a value in 'Foo::operator='\n", errout.str());
    }

    void uninitVar9() // ticket #1730
    {
        checkUninitVar("class Prefs {\n"
                       "private:\n"
                       "    int xasd;\n"
                       "public:\n"
                       "    Prefs(wxSize size);\n"
                       "};\n"
                       "Prefs::Prefs(wxSize size)\n"
                       "{\n"
                       "    SetMinSize( wxSize( 48,48 ) );\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Prefs::xasd' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar10() // ticket #1993
    {
        checkUninitVar("class A {\n"
                       "public:\n"
                       "        A();\n"
                       "private:\n"
                       "        int var1;\n"
                       "        int var2;\n"
                       "};\n"
                       "A::A() : var1(0) { }\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Member variable 'A::var2' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar11()
    {
        checkUninitVar("class A {\n"
                       "public:\n"
                       "        A(int a = 0);\n"
                       "private:\n"
                       "        int var;\n"
                       "};\n"
                       "A::A(int a) { }\n");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'A::var' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar12() // ticket #2078
    {
        checkUninitVar("class Point\n"
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
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Point::x' is not initialized in the constructor.\n"
                      "[test.cpp:4]: (warning) Member variable 'Point::y' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar13() // ticket #1195
    {
        checkUninitVar("class A {\n"
                       "private:\n"
                       "    std::vector<int> *ints;\n"
                       "public:\n"
                       "    A()\n"
                       "    {}\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'A::ints' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar14() // ticket #2149
    {
        // no namespace
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    Foo();\n"
                       "private:\n"
                       "    bool mMember;\n"
                       "};\n"
                       "Foo::Foo()\n"
                       "{\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // single namespace
        checkUninitVar("namespace Output\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor outside namespace
        checkUninitVar("namespace Output\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // constructor outside namespace
        checkUninitVar("namespace Output\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor in separate namespace
        checkUninitVar("namespace Output\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:13]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor in different separate namespace
        checkUninitVar("namespace Output\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // constructor in different separate namespace (won't compile)
        checkUninitVar("namespace Output\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // constructor in nested separate namespace
        checkUninitVar("namespace A\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:15]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor.\n", errout.str());

        // constructor in nested different separate namespace
        checkUninitVar("namespace A\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        // constructor in nested different separate namespace
        checkUninitVar("namespace A\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar15()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "    int a;\n"
                       "public:\n"
                       "    Fred();\n"
                       "    ~Fred();\n"
                       "};\n"
                       "Fred::~Fred()\n"
                       "{\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar16()
    {
        checkUninitVar("struct Foo\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("struct Foo\n"
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
                       "};\n");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'Bar::foo' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar17()
    {
        checkUninitVar("struct Foo\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("struct Foo\n"
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
                       "};\n");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Member variable 'Bar::foo' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar18() // ticket #2465
    {
        checkUninitVar("struct Altren\n"
                       "{\n"
                       "    Altren(int _a = 0) : value(0) { }\n"
                       "    int value;\n"
                       "};\n"
                       "class A\n"
                       "{\n"
                       "public:\n"
                       "    A() { }\n"
                       "private:\n"
                       "    Altren value;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("struct Altren\n"
                       "{\n"
                       "    Altren(int _a) : value(0) { }\n"
                       "    int value;\n"
                       "};\n"
                       "class A\n"
                       "{\n"
                       "public:\n"
                       "    A() { }\n"
                       "private:\n"
                       "    Altren value;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Member variable 'A::value' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar19() // ticket #2792
    {
        checkUninitVar("class mystring\n"
                       "{\n"
                       "    char* m_str;\n"
                       "    int m_len;\n"
                       "public:\n"
                       "    mystring(const char* str)\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar20() // ticket #2867
    {
        checkUninitVar("Object::MemFunc() {\n"
                       "    class LocalClass {\n"
                       "    public:\n"
                       "        LocalClass() : dataLength_(0) {}\n"
                       "        std::streamsize dataLength_;\n"
                       "        double bitsInData_;\n"
                       "    } obj;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("Object::MemFunc() {\n"
                       "    class LocalClass : public copy_protected {\n"
                       "    public:\n"
                       "        LocalClass() : copy_protected(1), dataLength_(0) {}\n"
                       "        std::streamsize dataLength_;\n"
                       "        double bitsInData_;\n"
                       "    } obj;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("Object::MemFunc() {\n"
                       "    class LocalClass : ::copy_protected {\n"
                       "    public:\n"
                       "        LocalClass() : copy_protected(1), dataLength_(0) {}\n"
                       "        std::streamsize dataLength_;\n"
                       "        double bitsInData_;\n"
                       "    } obj;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVar21() // ticket #2947
    {
        checkUninitVar("class Fred {\n"
                       "private:\n"
                       "    int a[23];\n"
                       "public:\n"
                       "    Fred(); \n"
                       "};\n"
                       "Fred::Fred() {\n"
                       "    a[x::y] = 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar22() // ticket #3043
    {
        checkUninitVar("class Fred {\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Fred {\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Fred::x' is not assigned a value in 'Fred::operator='\n", errout.str());
    }

    void uninitVarArray1()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() {}\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'John::name' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() {John::name[0] = '\\0';}\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { strcpy(name, ""); }\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { }\n"
                       "\n"
                       "    double  operator[](const unsigned long i);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class A;\n"
                       "class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { }\n"
                       "    A a[5];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class A;\n"
                       "class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { }\n"
                       "    A *a[5];\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'John::a' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarArray2()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { *name = 0; }\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray3()
    {
        checkUninitVar("class John\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray4()
    {
        checkUninitVar("class John\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray5()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "private:\n"
                       "    Bar bars[10];\n"
                       "public:\n"
                       "    Foo()\n"
                       "    { }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray6()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    Foo();\n"
                       "    static const char STR[];\n"
                       "};\n"
                       "const char Foo::STR[] = \"abc\";\n"
                       "Foo::Foo() { }");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray7()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "    int array[10];\n"
                       "public:\n"
                       "    Foo() { }\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'Foo::array' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("class Foo\n"
                       "{\n"
                       "    int array[10];\n"
                       "public:\n"
                       "    Foo() { memset(array, 0, sizeof(array)); }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Foo\n"
                       "{\n"
                       "    int array[10];\n"
                       "public:\n"
                       "    Foo() { ::memset(array, 0, sizeof(array)); }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray2D()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { a[0][0] = 0; }\n"
                       "\n"
                       "private:\n"
                       "    char a[2][2];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarStruct1() // ticket #2172
    {
        checkUninitVar("class A\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class A\n"
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
                       "};\n");
        ASSERT_EQUALS("[test.cpp:10]: (warning) Member variable 'A::b' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("class A\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarStruct2() // ticket #838
    {
        checkUninitVar("struct POINT\n"
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
                       "};\n");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Member variable 'Fred::p' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("struct POINT\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("struct POINT\n"
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
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitMissingFuncDef()
    {
        // Unknown member function
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    void Init();"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function (friend class)
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    friend ABC;\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function (is Init a virtual function?)
        checkUninitVar("class Fred : private ABC\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        // Unknown non-member function
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

        // Unknown non-member function
        checkUninitVar("class ABC { };\n"
                       "class Fred : private ABC\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());

    }

    void uninitVarEnum()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    enum abc {a,b,c};\n"
                       "    Fred() {}\n"
                       "private:\n"
                       "    unsigned int i;\n"
                       "};\n");

        ASSERT_EQUALS("[test.cpp:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarStream()
    {
        checkUninitVar("#include <fstream>\n"
                       "class Foo\n"
                       "{\n"
                       "private:\n"
                       "    int foo;\n"
                       "public:\n"
                       "    Foo(std::istream &in)\n"
                       "    {\n"
                       "        if(!(in >> foo))\n"
                       "            throw 0;\n"
                       "    }\n"
                       "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarTypedef()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    typedef int * pointer;\n"
                       "    Foo() : a(0) {}\n"
                       "    pointer a;\n"
                       "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarMemset()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    int * pointer;\n"
                       "    Foo() { memset(this, 0, sizeof(*this)); }\n"
                       "};\n");

        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    int * pointer;\n"
                       "    Foo() { ::memset(this, 0, sizeof(*this)); }\n"
                       "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void privateCtor1()
    {
        checkUninitVar("class Foo {\n"
                       "    int foo;\n"
                       "    Foo() { }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void privateCtor2()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "private:\n"
                       "    int foo;\n"
                       "    Foo() { }\n"
                       "public:\n"
                       "    Foo(int _i) { }\n"
                       "};\n");

        ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'Foo::foo' is not initialized in the constructor.\n", errout.str());
    }


    void function()
    {
        checkUninitVar("class A\n"
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
                       "}\n");

        ASSERT_EQUALS("", errout.str());
    }


    void uninitVarHeader1()
    {
        checkUninitVar("#file \"fred.h\"\n"
                       "class Fred\n"
                       "{\n"
                       "private:\n"
                       "    unsigned int i;\n"
                       "public:\n"
                       "    Fred();\n"
                       "};\n"
                       "#endfile\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarHeader2()
    {
        checkUninitVar("#file \"fred.h\"\n"
                       "class Fred\n"
                       "{\n"
                       "private:\n"
                       "    unsigned int i;\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "};\n"
                       "#endfile\n");
        ASSERT_EQUALS("[fred.h:6]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }

    void uninitVarHeader3()
    {
        checkUninitVar("#file \"fred.h\"\n"
                       "class Fred\n"
                       "{\n"
                       "private:\n"
                       "    mutable int i;\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "};\n"
                       "#endfile\n");
        ASSERT_EQUALS("[fred.h:6]: (warning) Member variable 'Fred::i' is not initialized in the constructor.\n", errout.str());
    }

    // Borland C++: No FP for published pointers - they are automatically initialized
    void uninitVarPublished()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "__published:\n"
                       "    int *i;\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitOperator()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "    int *operator [] (int index) { return 0; }\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitFunction1()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { init(*this); }\n"
                       "\n"
                       "    static void init(Fred &f)\n"
                       "    { f.d = 0; }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { init(*this); }\n"
                       "\n"
                       "    static void init(Fred &f)\n"
                       "    { }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", "", errout.str());
    }

    void uninitFunction2()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { if (!init(*this)); }\n"
                       "\n"
                       "    static bool init(Fred &f)\n"
                       "    { f.d = 0; return true; }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { if (!init(*this)); }\n"
                       "\n"
                       "    static bool init(Fred &f)\n"
                       "    { return true; }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", "",  errout.str());
    }

    void uninitFunction3()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { if (!init()); }\n"
                       "\n"
                       "    bool init()\n"
                       "    { d = 0; return true; }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { if (!init()); }\n"
                       "\n"
                       "    bool init()\n"
                       "    { return true; }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", errout.str());
    }

    void uninitFunction4()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { init(this); }\n"
                       "\n"
                       "    init(Fred *f)\n"
                       "    { f.d = 0; }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { init(this); }\n"
                       "\n"
                       "    init(Fred *f)\n"
                       "    { }\n"
                       "\n"
                       "    double d;\n"
                       "};\n");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'Fred::d' is not initialized in the constructor.\n", "", errout.str());
    }

    void uninitSameClassName()
    {
        checkUninitVar("class B\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class B\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:18]: (warning) Member variable 'B::j' is not initialized in the constructor.\n"
                      "[test.cpp:22]: (warning) Member variable 'B::i' is not initialized in the constructor.\n", errout.str());

        // Ticket #1700
        checkUninitVar("namespace n1\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("namespace n1\n"
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
                       "}\n");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Member variable 'Foo::i' is not initialized in the constructor.\n", errout.str());

        checkUninitVar("namespace n1\n"
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitFunctionOverload()
    {
        // Ticket #1783 - overloaded "init" functions
        checkUninitVar("class A\n"
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

        checkUninitVar("class A\n"
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
        TODO_ASSERT_EQUALS("[test.cpp:7]: (warning) Member variable 'A::i' is not initialized in the constructor.\n", "", errout.str());
    }

    void checkUninitVarJava(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.java");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.constructors();
    }

    void uninitJava()
    {
        checkUninitVarJava("class A {\n"
                           "    private: int i = 0;\n"
                           "    public: A() { }\n"
                           "};");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarOperatorEqual()  // ticket #2415
    {
        checkUninitVar("struct A {\n"
                       "    int a;\n"
                       "    A() { a=0; }\n"
                       "    A(A const &a) { operator=(a); }\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("struct A {\n"
                       "    int a;\n"
                       "    A() { a=0; }\n"
                       "    A(A const &a) { operator=(a); }\n"
                       "    A & operator = (const A & rhs) {\n"
                       "        a = rhs.a;\n"
                       "        return *this;\n"
                       "    }\n"
                       "};");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("struct A {\n"
                       "    int a;\n"
                       "    A() { a=0; }\n"
                       "    A(A const &a) { operator=(a); }\n"
                       "    A & operator = (const A & rhs) {\n"
                       "        return *this;\n"
                       "    }\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Member variable 'A::a' is not initialized in the constructor.\n"
                      "[test.cpp:5]: (warning) Member variable 'A::a' is not assigned a value in 'A::operator='\n", errout.str());
    }

    void checkNoConstructor(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.constructors();
    }

    void noConstructor1()
    {
        // There are nonstatic member variables - constructor is needed
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "    int i;\n"
                           "};\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) The class 'Fred' does not have a constructor.\n", errout.str());
    }

    void noConstructor2()
    {
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "public:\n"
                           "    static void foobar();\n"
                           "};\n"
                           "\n"
                           "void Fred::foobar()\n"
                           "{ }\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor3()
    {
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "private:\n"
                           "    static int foobar;\n"
                           "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor4()
    {
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "public:\n"
                           "    int foobar;\n"
                           "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor5()
    {
        checkNoConstructor("namespace Foo\n"
                           "{\n"
                           "    int i;\n"
                           "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void checkNoMemset(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.noMemset();
    }

    void memsetOnClass()
    {
        checkNoMemset("class Fred\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    static std::string b;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string * b; \n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string b; \n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a 'std::string'\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    mutable std::string b; \n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a 'std::string'\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string s;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a 'std::string'\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    std::string s;\n"
                      "};\n"
                      "class Pebbles: public Fred {};\n"
                      "void f()\n"
                      "{\n"
                      "    Pebbles pebbles;\n"
                      "    memset(&pebbles, 0, sizeof(pebbles));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:9]: (error) Using 'memset' on class that contains a 'std::string'\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    virtual ~Fred();\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a virtual method\n", errout.str());

        checkNoMemset("class Fred\n"
                      "{\n"
                      "    virtual ~Fred();\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    static Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on class that contains a virtual method\n", errout.str());

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
                      "}\n");
        ASSERT_EQUALS("[test.cpp:12]: (error) Using 'memset' on class that contains a virtual method\n", errout.str());

        // Fred not defined in scope
        checkNoMemset("namespace n1 {\n"
                      "    class Fred\n"
                      "    {\n"
                      "        std::string b; \n"
                      "    };\n"
                      "}\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(Fred));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        // Fred with namespace qualifier
        checkNoMemset("namespace n1 {\n"
                      "    class Fred\n"
                      "    {\n"
                      "        std::string b; \n"
                      "    };\n"
                      "}\n"
                      "void f()\n"
                      "{\n"
                      "    n1::Fred fred;\n"
                      "    memset(&fred, 0, sizeof(n1::Fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (error) Using 'memset' on class that contains a 'std::string'\n", errout.str());

        // Fred with namespace qualifier
        checkNoMemset("namespace n1 {\n"
                      "    class Fred\n"
                      "    {\n"
                      "        std::string b; \n"
                      "    };\n"
                      "}\n"
                      "void f()\n"
                      "{\n"
                      "    n1::Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (error) Using 'memset' on class that contains a 'std::string'\n", errout.str());
    }

    void memsetOnStruct()
    {
        checkNoMemset("struct A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    struct A a;\n"
                      "    memset(&a, 0, sizeof(struct A));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    struct A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("void f()\n"
                      "{\n"
                      "    struct sockaddr_in6 fail;\n"
                      "    memset(&fail, 0, sizeof(struct sockaddr_in6));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      " void g( struct sockaddr_in6& a);\n"
                      "private:\n"
                      " std::string b; \n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      " struct A fail;\n"
                      " memset(&fail, 0, sizeof(struct A));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (error) Using 'memset' on struct that contains a 'std::string'\n", errout.str());

        checkNoMemset("struct Fred\n"
                      "{\n"
                      "     std::string s;\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      "    Fred fred;\n"
                      "    memset(&fred, 0, sizeof(fred));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Using 'memset' on struct that contains a 'std::string'\n", errout.str());

        checkNoMemset("struct Stringy {\n"
                      "    std::string inner;\n"
                      "};\n"
                      "struct Foo {\n"
                      "    Stringy s;\n"
                      "};\n"
                      "int main() {\n"
                      "    Foo foo;\n"
                      "    memset(&foo, 0, sizeof(Foo));\n"
                      "}\n");

        ASSERT_EQUALS("[test.cpp:9]: (error) Using 'memset' on struct that contains a 'std::string'\n", errout.str());
    }

    void memsetVector()
    {
        checkNoMemset("class A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on class that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(struct A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(a));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());

        checkNoMemset("class A\n"
                      "{ std::vector< std::vector<int> > ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on class that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector< std::vector<int> > ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector< std::vector<int> > ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(a));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int *> ints; };\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(&a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());
    }


    void checkThisSubtraction(const char code[])
    {
        // Clear the error log
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.thisSubtraction();
    }

    void this_subtraction()
    {
        checkThisSubtraction("; this-x ;");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Suspicious pointer subtraction\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;");
        ASSERT_EQUALS("", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious pointer subtraction\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;\n"
                             "this-x ;\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious pointer subtraction\n"
                      "[test.cpp:3]: (warning) Suspicious pointer subtraction\n", errout.str());
    }

    void checkConst(const char code[], const Settings *s = 0)
    {
        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        if (s)
            settings = *s;
        else
            settings.addEnabled("information");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.checkConst();
    }

    void const1()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int getA() { return a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    const std::string foo() { return ""; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    const std::string & foo() { return ""; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // constructors can't be const..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "public:\n"
                   "    Fred() { }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment through |=..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int setA() { a |= true; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // functions with a function call can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    int x;\n"
                   "    void b() { a(); }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // static functions can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    static unsigned get()\n"
                   "    { return 0; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    const std::string foo() const throw() { return ""; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const2()
    {
        // ticket 1344
        // assignment to variable can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo() { s = ""; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument reference can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a) { a = s; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a) { s = a; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument references can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { a = s; b = s; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { s = a; s = b; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { s = a; b = s; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a, std::string & b) { a = s; s = b; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const3()
    {
        // assignment to function argument pointer can be const
        checkConst("class Fred {\n"
                   "    int s;\n"
                   "    void foo(int * a) { *a = s; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    int s;\n"
                   "    void foo(int * a) { s = *a; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument pointers can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { *a = s; *b = s; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { s = *a; s = *b; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { s = *a; *b = s; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable, can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string * a, std::string * b) { *a = s; s = b; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const4()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int getA();\n"
                   "};\n"
                   "int Fred::getA() { return a; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) Technically the member function 'Fred::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    const std::string foo();\n"
                   "};\n"
                   "const std::string Fred::foo() { return ""; }");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    const std::string & foo();\n"
                   "};\n"
                   "const std::string & Fred::foo() { return ""; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

        // constructors can't be const..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "public:\n"
                   "    Fred()\n"
                   "};\n"
                   "Fred::Fred() { }");
        ASSERT_EQUALS("", errout.str());

        // assignment through |=..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int setA();\n"
                   "};\n"
                   "int Fred::setA() { a |= true; }");
        ASSERT_EQUALS("", errout.str());

        // functions with a function call can't be const..
        checkConst("class Fred\n"
                   "{\n"
                   "public:\n"
                   "    int x;\n"
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
                   "void Fred::foo() { s = ""; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument reference can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a);\n"
                   "};\n"
                   "void Fred::foo(std::string & a) { a = s; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

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
                   "void Fred::foo(std::string & a, std::string & b) { s = a; b = s; }");
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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:3]: (style) Technically the member function 'Fred::foo' can be const.\n"
                      "[test.cpp:7] -> [test.cpp:4]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (style) Technically the member function 'Fred::foo1' can be const.\n"
                      "[test.cpp:10] -> [test.cpp:4]: (style) Technically the member function 'Fred::foo2' can be const.\n"
                      "[test.cpp:11] -> [test.cpp:5]: (style) Technically the member function 'Fred::foo3' can be const.\n"
                      "[test.cpp:12] -> [test.cpp:6]: (style) Technically the member function 'Fred::foo4' can be const.\n"
                      "[test.cpp:13] -> [test.cpp:7]: (style) Technically the member function 'Fred::foo5' can be const.\n", errout.str());

        // check nested classes
        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA() { return a; }\n"
                   "    };\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'Fred::A::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA();\n"
                   "    };\n"
                   "    int A::getA() { return a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:4]: (style) Technically the member function 'Fred::A::getA' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA();\n"
                   "    };\n"
                   "};\n"
                   "int Fred::A::getA() { return a; }");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:4]: (style) Technically the member function 'Fred::A::getA' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:7]: (style) Technically the member function 'Fred::B::A::getA' can be const.\n"
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
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:4]: (style) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:9] -> [test.cpp:7]: (style) Technically the member function 'Fred::B::A::getA' can be const.\n" , errout.str());

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
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:4]: (style) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:10] -> [test.cpp:7]: (style) Technically the member function 'Fred::B::A::getA' can be const.\n" , errout.str());

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
                   "int Fred::B::getB() { return b; }\n");
        ASSERT_EQUALS("[test.cpp:12] -> [test.cpp:4]: (style) Technically the member function 'Fred::B::getB' can be const.\n"
                      "[test.cpp:11] -> [test.cpp:7]: (style) Technically the member function 'Fred::B::A::getA' can be const.\n" , errout.str());
    }

    // operator< can often be const
    void constoperator1()
    {
        checkConst("struct Fred {\n"
                   "    int a;\n"
                   "    bool operator<(const Fred &f) { return (a < f.a); }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::operator<' can be const.\n", errout.str());
    }

    // operator<<
    void constoperator2()
    {
        checkConst("struct Foo {\n"
                   "    void operator<<(int);\n"
                   "};\n"
                   "struct Fred {\n"
                   "    Foo foo;\n"
                   "    void x()\n"
                   "    {\n"
                   "        foo << 123;\n"
                   "    }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void constoperator3()
    {
        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    int const & operator [] (unsigned int index) const { return array[index]; }\n"
                   "    int & operator [] (unsigned int index) { return array[index]; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    int const & operator [] (unsigned int index) { return array[index]; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::operator[]' can be const.\n", errout.str());
    }

    void constoperator4()
    {
        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    typedef int* (Fred::*UnspecifiedBoolType);\n"
                   "    operator UnspecifiedBoolType() { };\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'Fred::operatorint**' can be const.\n", errout.str());

        checkConst("struct Fred {\n"
                   "    int array[10];\n"
                   "    typedef int* (Fred::*UnspecifiedBoolType);\n"
                   "    operator UnspecifiedBoolType() { array[0] = 0; };\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const5()
    {
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
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'A::foo' can be const.\n", errout.str());
    }

    void const6()
    {
        // ticket # 1491
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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'Fred::foo' can be const.\n", errout.str());

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

    void const7()
    {
        checkConst("class foo {\n"
                   "    int a;\n"
                   "public:\n"
                   "    void set(int i) { a = i; }\n"
                   "    void set(const foo & f) { *this = f; }\n"
                   "};\n"
                   "void bar() {}");
        ASSERT_EQUALS("", errout.str());
    }

    void const8()
    {
        // ticket #1517
        checkConst("class A {\n"
                   "public:\n"
                   "    A():m_strValue(""){}\n"
                   "    std::string strGetString() { return m_strValue; }\n"
                   "private:\n"
                   "    std::string m_strValue;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::strGetString' can be const.\n", errout.str());
    }

    void const9()
    {
        // ticket #1515
        checkConst("class wxThreadInternal {\n"
                   "public:\n"
                   "    void SetExitCode(wxThread::ExitCode exitcode) { m_exitcode = exitcode; }\n"
                   "private:\n"
                   "    wxThread::ExitCode m_exitcode;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const10()
    {
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
                   "    int foo() { return (x ? x : x = 0); }\n"
                   "private:\n"
                   "    int x;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { return (x ? x = 0 : x); }\n"
                   "private:\n"
                   "    int x;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const11()
    {
        // ticket #1529
        checkConst("class A {\n"
                   "public:\n"
                   "    void set(struct tm time) { m_time = time; }\n"
                   "private:\n"
                   "    struct tm m_time;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());
    }

    void const12()
    {
        // ticket #1525
        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { x = 0; }\n"
                   "private:\n"
                   "    mutable int x;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'A::foo' can be const.\n", errout.str());
    }

    void const13()
    {
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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetVec' can be const.\n"
                      "[test.cpp:5]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::vector<int> & GetVec()  {return m_vec;}\n"
                   "    const std::pair<int,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::vector<int> m_vec;\n"
                   "    std::pair<int,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetVec' can be const.\n"
                      "[test.cpp:5]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());
    }

    void const14()
    {
        // extends ticket 1519
        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair<std::vector<int>,double> GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair<std::vector<int>,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair<std::vector<int>,double>& GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair<std::vector<int>,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair<int ,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair<int ,double> m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< int,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

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
                   "    pair< int,vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int,vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< int,vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int,vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< int,vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int,vector<int> >  m_pair;\n"
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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< vector<int>, int >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< std::vector<int>,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::vector<int>,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< vector<int>, vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, vector<int> >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());



        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::pair < int, char > , int >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< std::pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< pair < int, char > , int >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< pair < int, char > , int >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< int , pair < int, char > >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int , pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< int , pair < int, char > > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int , pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< int , pair < int, char > > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int , pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int , std::pair < int, char > >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< int , std::pair < int, char > >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetPair' can be const.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetVec' can be const.\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const vector<int>&  GetVec() {return m_Vec;}\n"
                   "private:\n"
                   "    vector<int>  m_Vec;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::GetVec' can be const.\n", errout.str());

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
                   "    int * const * foo() { return &x; }\n"
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
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'A::foo' can be const.\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    const int * const * foo() { return &x; }\n"
                   "private:\n"
                   "    const int * x;\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'A::foo' can be const.\n", errout.str());
    }

    void const15()
    {
        checkConst("class Fred {\n"
                   "    unsigned long long int a;\n"
                   "    unsigned long long int getA() { return a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::getA' can be const.\n", errout.str());

        // constructors can't be const..
        checkConst("class Fred {\n"
                   "    unsigned long long int a;\n"
                   "public:\n"
                   "    Fred() { }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment through |=..
        checkConst("class Fred {\n"
                   "    unsigned long long int a;\n"
                   "    unsigned long long int setA() { a |= true; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // functions with a function call can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    unsigned long long int x;\n"
                   "    void b() { a(); }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // static functions can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    static unsigned long long int get()\n"
                   "    { return 0; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const16()
    {
        // ticket #1551
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void set(int i) { Fred::a = i; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const17()
    {
        // ticket #1552
        checkConst("class Fred {\n"
                   "public:\n"
                   "    void set(int i, int j) { a[i].k = i; }\n"
                   "private:\n"
                   "    struct { int k; } a[4];\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const18()
    {
        // ticket #1563
        checkConst("class Fred {\n"
                   "static int x;\n"
                   "public:\n"
                   "    void set(int i) { x = i; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const19()
    {
        // ticket #1612
        checkConst("using namespace std;\n"
                   "class Fred {\n"
                   "private:\n"
                   "    std::string s;\n"
                   "public:\n"
                   "    void set(std::string ss) { s = ss; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const20()
    {
        // ticket #1602
        checkConst("class Fred {\n"
                   "    int x : 3;\n"
                   "public:\n"
                   "    void set(int i) { x = i; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    list<int *> x;\n"
                   "public:\n"
                   "    list<int *> get() { return x; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    list<const int *> x;\n"
                   "public:\n"
                   "    list<const int *> get() { return x; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'Fred::get' can be const.\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::list<std::string &> x;\n"
                   "public:\n"
                   "    std::list<std::string &> get() { return x; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    std::list<const std::string &> x;\n"
                   "public:\n"
                   "    std::list<const std::string &> get() { return x; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'Fred::get' can be const.\n", errout.str());
    }

    void const21()
    {
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
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const22()
    {
        checkConst("class A\n"
                   "{\n"
                   "private:\n"
                   "    B::C * v1;\n"
                   "public:\n"
                   "    void f1() { v1 = 0; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A\n"
                   "{\n"
                   "private:\n"
                   "    B::C * v1[0];\n"
                   "public:\n"
                   "    void f1() { v1[0] = 0; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const23()
    {
        checkConst("class Class {\n"
                   "public:\n"
                   "    typedef Template<double> Type;\n"
                   "    typedef Template2<Type> Type2;\n"
                   "    void set_member(Type2 m) { _m = m; }\n"
                   "private:\n"
                   "    Type2 _m;\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const24()
    {
        checkConst("class Class {\n"
                   "public:\n"
                   "void Settings::SetSetting(QString strSetting, QString strNewVal)\n"
                   "{\n"
                   "    (*m_pSettings)[strSetting] = strNewVal;\n"
                   "}\n"
                   "private:\n"
                   "    std::map<QString, QString> *m_pSettings;\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }


    void const25() // ticket #1724
    {
        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVal="";}\n"
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
                   "A(){m_strVal="";}\n"
                   "std::string strGetString()\n"
                   "{return m_strVal.c_str();}\n"
                   "private:\n"
                   "std::string m_strVal;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::strGetString' can be const.\n", errout.str());

        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVal="";}\n"
                   "const std::string strGetString1()\n"
                   "{return m_strVal.c_str();}\n"
                   "private:\n"
                   "std::string m_strVal;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::strGetString1' can be const.\n", errout.str());

        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVec.push_back("");}\n"
                   "size_t strGetSize()\n"
                   "{return m_strVec.size();}\n"
                   "private:\n"
                   "std::vector<std::string> m_strVec;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::strGetSize' can be const.\n", errout.str());

        checkConst("class A{\n"
                   "public:\n"
                   "A(){m_strVec.push_back("");}\n"
                   "bool strGetEmpty()\n"
                   "{return m_strVec.empty();}\n"
                   "private:\n"
                   "std::vector<std::string> m_strVec;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'A::strGetEmpty' can be const.\n", errout.str());
    }

    void const26() // ticket #1847
    {
        checkConst("class DelayBase {\n"
                   "public:\n"
                   "void swapSpecificDelays(int index1, int index2) {\n"
                   "    std::swap<float>(delays_[index1], delays_[index2]);\n"
                   "}\n"
                   "float delays_[4];\n"
                   "};\n"
                  );
        ASSERT_EQUALS("", errout.str());
    }

    void const27() // ticket #1882
    {
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
                   "        return dRet / m_d;\n"
                   "    return dRet;\n"
                   "};\n"
                  );
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:4]: (style) Technically the member function 'A::dGetValue' can be const.\n", errout.str());
    }

    void const28() // ticket #1883
    {
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
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class AA {\n"
                   "public:\n"
                   "    AA():P(){}\n"
                   "    inline void vSetXPos(int x_)\n"
                   "    {\n"
                   "        UnknownScope::x = x_;\n"
                   "    }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Technically the member function 'AA::vSetXPos' can be const.\n", errout.str());

    }

    void const29() // ticket #1922
    {
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
                   "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const30()
    {
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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:7]: (style) Technically the member function 'Derived::get' can be const.\n", errout.str());

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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:11]: (style) Technically the member function 'Derived::getA' can be const.\n"
                      "[test.cpp:14]: (style) Technically the member function 'Derived::getB' can be const.\n", errout.str());

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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:8]: (style) Technically the member function 'Derived2::get' can be const.\n", errout.str());

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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:10]: (style) Technically the member function 'Derived4::get' can be const.\n", errout.str());

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
                   "};\n");
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
                   "};\n");
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
                   "};\n");
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
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const31()
    {
        checkConst("namespace std { }\n"
                   "class Fred {\n"
                   "public:\n"
                   "    int a;\n"
                   "    int get() { return a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Technically the member function 'Fred::get' can be const.\n", errout.str());
    }

    void const32()
    {
        checkConst("class Fred {\n"
                   "public:\n"
                   "    std::string a[10];\n"
                   "    void seta() { a[0] = \"\"; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const33()
    {
        checkConst("class derived : public base {\n"
                   "public:\n"
                   "    void f(){}\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const34() // ticket #1964
    {
        checkConst("class Bar {\n"
                   "    void init(Foo * foo) {\n"
                   "        foo.bar = this;\n"
                   "    }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const35() // ticket #2001
    {
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
                   "}\n");
        ASSERT_EQUALS("[test.cpp:12]: (style) Technically the member function 'N::Derived::getResourceName' can be const.\n", errout.str());

        checkConst("namespace N\n"
                   "{\n"
                   "        class Base\n"
                   "        {\n"
                   "        public:\n"
                   "                int getResourceName();\n"
                   "                int var;\n"
                   "        };\n"
                   "}\n"
                   "int N::Base::getResourceName() { return var; }\n");
        ASSERT_EQUALS("[test.cpp:10] -> [test.cpp:6]: (style) Technically the member function 'N::Base::getResourceName' can be const.\n", errout.str());

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
                   "}\n");
        ASSERT_EQUALS("[test.cpp:12] -> [test.cpp:6]: (style) Technically the member function 'N::Base::getResourceName' can be const.\n", errout.str());

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
                   "int Base::getResourceName() { return var; }\n");
        TODO_ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:6]: (style) Technically the member function 'N::Base::getResourceName' can be const.\n",
                           "", errout.str());
    }

    void const36() // ticket #2003
    {
        checkConst("class Foo {\n"
                   "public:\n"
                   "    Blue::Utility::Size m_MaxQueueSize;\n"
                   "    void SetMaxQueueSize(Blue::Utility::Size a_MaxQueueSize)\n"
                   "    {\n"
                   "        m_MaxQueueSize = a_MaxQueueSize;\n"
                   "    }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const37() // ticket #2081 and #2085
    {
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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Technically the member function 'A::operator+' can be const.\n", errout.str());

        checkConst("class Fred\n"
                   "{\n"
                   "private:\n"
                   "    long x;\n"
                   "public:\n"
                   "    Fred() {\n"
                   "        x = 0;\n"
                   "    }\n"
                   "    bool isValid() {\n"
                   "        return bool(x == 0x11224488);\n"
                   "    }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:9]: (style) Technically the member function 'Fred::isValid' can be const.\n", errout.str());
    }

    void const38() // ticket #2135
    {
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
                   "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const39()
    {
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
                   "}\n");

        ASSERT_EQUALS("", errout.str());
    }

    void const40() // ticket #2228
    {
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
                   "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void const41() // ticket #2255
    {
        checkConst("class Fred\n"
                   "{\n"
                   "   ::std::string m_name;\n"
                   "public:\n"
                   "   void SetName(const ::std::string & name)\n"
                   "   {\n"
                   "      m_name = name;\n"
                   "   }\n"
                   "};\n");

        ASSERT_EQUALS("", errout.str());

        checkConst("class SharedPtrHolder\n"
                   "{\n"
                   "   ::std::tr1::shared_ptr<int> pNum;\n"
                   "  public :\n"
                   "   void SetNum(const ::std::tr1::shared_ptr<int> & apNum)\n"
                   "   {\n"
                   "      pNum = apNum;\n"
                   "   }\n"
                   "};\n");

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
                   "};\n");

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
                   "};\n");

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
                   "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void const42() // ticket #2282
    {
        checkConst("class Fred\n"
                   "{\n"
                   "public:\n"
                   "    struct AB { };\n"
                   "    bool f(AB * ab);\n"
                   "};\n"
                   "bool Fred::f(Fred::AB * ab)\n"
                   "{\n"
                   "}\n");

        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5]: (style) Technically the member function 'Fred::f' can be const.\n", errout.str());

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
                   "}\n");

        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:7]: (style) Technically the member function 'Fred::f' can be const.\n", errout.str());

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
                   "}\n");

        ASSERT_EQUALS("[test.cpp:10] -> [test.cpp:8]: (style) Technically the member function 'NS::Fred::f' can be const.\n", errout.str());

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
                   "}\n");

        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:8]: (style) Technically the member function 'NS::Fred::f' can be const.\n", errout.str());

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
                   "}\n");

        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:8]: (style) Technically the member function 'Foo::Fred::f' can be const.\n", errout.str());
    }

    void const43() // ticket 2377
    {
        checkConst("class A\n"
                   "{\n"
                   "public:\n"
                   "    void foo( AA::BB::CC::DD b );\n"
                   "    AA::BB::CC::DD a;\n"
                   "};\n"
                   "void A::foo( AA::BB::CC::DD b )\n"
                   "{\n"
                   "    a = b;\n"
                   "}\n");

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
                   "    \n"
                   "    AA::BB::CC::DD a;\n"
                   "    void foo(AA::BB::CC::DD b)\n"
                   "    {\n"
                   "        a = b;\n"
                   "    }\n"
                   "};\n");

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
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const44() // ticket 2595
    {
        checkConst("class A\n"
                   "{\n"
                   "public:\n"
                   "    bool bOn;\n"
                   "    bool foo()\n"
                   "    {\n"
                   "        return 0 != (bOn = bOn && true);\n"
                   "    }\n"
                   "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void const45() // ticket 2664
    {
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
                   "}\n");

        ASSERT_EQUALS("[test.cpp:8]: (style) Technically the member function 'tools::WorkspaceControl::toGrid' can be const.\n", errout.str());
    }

    void const46() // ticket 2663
    {
        checkConst("class Altren {\n"
                   "public:\n"
                   "    int fun1() {\n"
                   "        int a;\n"
                   "        a++;\n"
                   "    }\n"
                   "    int fun2() {\n"
                   "        b++;\n"
                   "    }\n"
                   "};\n");

        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Altren::fun1' can be const.\n"
                      "[test.cpp:7]: (style) Technically the member function 'Altren::fun2' can be const.\n", errout.str());
    }

    void const47() // ticket 2670
    {
        checkConst("class Altren {\n"
                   "public:\n"
                   "  void foo() { delete this; }\n"
                   "  void foo(int i) const { }\n"
                   "  void bar() { foo(); }\n"
                   "};\n");

        ASSERT_EQUALS("", errout.str());

        checkConst("class Altren {\n"
                   "public:\n"
                   "  void foo() { delete this; }\n"
                   "  void foo(int i) const { }\n"
                   "  void bar() { foo(1); }\n"
                   "};\n");

        ASSERT_EQUALS("[test.cpp:5]: (style) Technically the member function 'Altren::bar' can be const.\n", errout.str());
    }

    void const48() // ticket 2672
    {
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
                   "}\n");

        ASSERT_EQUALS("", errout.str());
    }

    void const49() // ticket 2795
    {
        checkConst("class A {\n"
                   "    private:\n"
                   "         std::map<unsigned int,unsigned int> _hash;\n"
                   "    public:\n"
                   "         A() : _hash() {}\n"
                   "         unsigned int fetch(unsigned int key) // cannot be 'const'\n"
                   "         {\n"
                   "             return _hash[key];\n"
                   "         }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const50() // ticket 2943
    {
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
                   "}\n");

        ASSERT_EQUALS("", errout.str());
    }

    void const51() // ticket 3040
    {
        checkConst("class PSIPTable {\n"
                   "public:\n"
                   "    PSIPTable() : _pesdata(0) { }\n"
                   "    const unsigned char* pesdata() const { return _pesdata; }\n"
                   "    unsigned char* pesdata()             { return _pesdata; }\n"
                   "    void SetSection(uint num) { pesdata()[6] = num; }\n"
                   "private:\n"
                   "    unsigned char *_pesdata;\n"
                   "};\n");
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
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const52() // ticket 3049
    {
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
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const53() // ticket 3052
    {
        checkConst("class Example {\n"
                   "  public:\n"
                   "    void Clear(void) { Example tmp; (*this) = tmp; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const54()
    {
        checkConst("class MyObject {\n"
                   "    int tmp;\n"
                   "    MyObject() : tmp(0) {}\n"
                   "public:\n"
                   "    void set(std::stringstream &in) { in >> tmp; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void const55() // ticket #3149
    {
        checkConst("class MyObject {\n"
                   "public:\n"
                   "    void foo(int x) {\n"
                   "    switch (x) { }\n"
                   "    }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'MyObject::foo' can be const.\n", errout.str());

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
                   "                break;\n"
                   "            }\n"
                   "            case INT_TYPE:\n"
                   "            {\n"
                   "                return RET_OK;\n"
                   "                break;\n"
                   "            }\n"
                   "            default:\n"
                   "            {\n"
                   "                return RET_NOK;\n"
                   "                break;\n"
                   "            }\n"
                   "        }\n"
                   "    }\n"
                   "    catch (...)\n"
                   "    {\n"
                   "        return RET_NOK;\n"
                   "    }\n"
                   "\n"
                   "    return RET_NOK;\n"
                   "}\n");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:4]: (style) Technically the member function 'A::f' can be const.\n", errout.str());

        checkConst("class MyObject {\n"
                   "public:\n"
                   "    void foo(int x) {\n"
                   "    for (int i = 0; i < 5; i++) { }\n"
                   "    }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'MyObject::foo' can be const.\n", errout.str());
    }

    void assigningPointerToPointerIsNotAConstOperation()
    {
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

    void assigningArrayElementIsNotAConstOperation()
    {
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
    void constincdec()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return ++a; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return --a; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a++; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a--; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return ++a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return --a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a++; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a--; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());
    }

    void constassign1()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a-=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a+=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a*=-1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return a/=-2; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a-=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a+=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a*=-1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a;\n"
                   "class Fred {\n"
                   "    void nextA() { return a/=-2; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());
    }

    void constassign2()
    {
        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a-=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a+=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    struct A { int a; } s;\n"
                   "    void nextA() { return s.a*=-1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a-=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a+=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a*=-1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("struct A { int a; } s;\n"
                   "class Fred {\n"
                   "    void nextA() { return s.a/=-2; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a-=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a+=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a*=-1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("struct A { int a; };\n"
                   "class Fred {\n"
                   "    A s;\n"
                   "    void nextA() { return s.a/=-2; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // increment/decrement array element => not const
    void constincdecarray()
    {
        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return ++a[0]; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return --a[0]; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]++; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]--; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return ++a[0]; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return --a[0]; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]++; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]--; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());
    }

    void constassignarray()
    {
        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]-=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]+=1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]*=-1; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("class Fred {\n"
                   "    int a[2];\n"
                   "    void nextA() { return a[0]/=-2; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]-=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]+=1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]*=-1; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());

        checkConst("int a[2];\n"
                   "class Fred {\n"
                   "    void nextA() { return a[0]/=-2; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::nextA' can be const.\n", errout.str());
    }

    // return pointer/reference => not const
    void constReturnReference()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int &getR() { return a; }\n"
                   "    int *getP() { return &a; }"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // delete member variable => not const (but technically it can, it compiles without errors)
    void constDelete()
    {
        checkConst("class Fred {\n"
                   "    int *a;\n"
                   "    void clean() { delete a; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // A function that returns LPVOID can't be const
    void constLPVOID()
    {
        checkConst("class Fred {\n"
                   "    LPVOID a() { return 0; };\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // #1579 - HDC
        checkConst("class Fred {\n"
                   "    HDC a() { return 0; };\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // a function that calls const functions can be const
    void constFunc()
    {
        checkConst("class Fred {\n"
                   "    void f() const { };\n"
                   "    void a() { f(); };\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Technically the member function 'Fred::a' can be const.\n", errout.str());

        // ticket #1593
        checkConst("#include <vector>\n"
                   "class A\n"
                   "{\n"
                   "   std::vector<int> m_v;\n"
                   "public:\n"
                   "   A(){}\n"
                   "   unsigned int GetVecSize()  {return m_v.size();}\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:7]: (style) Technically the member function 'A::GetVecSize' can be const.\n", errout.str());

        checkConst("#include <vector>\n"
                   "class A\n"
                   "{\n"
                   "   std::vector<int> m_v;\n"
                   "public:\n"
                   "   A(){}\n"
                   "   bool GetVecEmpty()  {return m_v.empty();}\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:7]: (style) Technically the member function 'A::GetVecEmpty' can be const.\n", errout.str());
    }

    void constVirtualFunc()
    {
        // base class has no virtual function
        checkConst("class A { };\n"
                   "class B : public A {\n"
                   "   int b;\n"
                   "public:\n"
                   "   B() : b(0) { }\n"
                   "   int func() { return b; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) Technically the member function 'B::func' can be const.\n", errout.str());

        checkConst("class A { };\n"
                   "class B : public A {\n"
                   "   int b;\n"
                   "public:\n"
                   "   B() : b(0) { }\n"
                   "   int func();\n"
                   "};\n"
                   "int B::func() { return b; }\n");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:6]: (style) Technically the member function 'B::func' can be const.\n", errout.str());

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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:9]: (style) Technically the member function 'B::func' can be const.\n", errout.str());

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
                   "int B::func() { return b; }\n");
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:9]: (style) Technically the member function 'B::func' can be const.\n", errout.str());

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
                   "};\n");
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
                   "int B::func() { return b; }\n");
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
                   "int B::func() { return b; }\n");
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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Technically the member function 'A::func' can be const.\n"
                      "[test.cpp:11]: (style) Technically the member function 'B::func' can be const.\n"
                      "[test.cpp:17]: (style) Technically the member function 'C::func' can be const.\n", errout.str());

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
                   "int C::func() { return c; }\n");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5]: (style) Technically the member function 'A::func' can be const.\n"
                      "[test.cpp:14] -> [test.cpp:12]: (style) Technically the member function 'B::func' can be const.\n"
                      "[test.cpp:21] -> [test.cpp:19]: (style) Technically the member function 'C::func' can be const.\n", errout.str());

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
                   "};\n");
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
                   "int C::func() { return c; }\n");
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
                   "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Technically the member function 'X::getX' can be const.\n"
                      "[test.cpp:11]: (style) Technically the member function 'Y::getY' can be const.\n"
                      "[test.cpp:17]: (style) Technically the member function 'Z::getZ' can be const.\n", errout.str());

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
                   "int Z::getZ() { return z; }\n");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5]: (style) Technically the member function 'X::getX' can be const.\n"
                      "[test.cpp:14] -> [test.cpp:12]: (style) Technically the member function 'Y::getY' can be const.\n"
                      "[test.cpp:21] -> [test.cpp:19]: (style) Technically the member function 'Z::getZ' can be const.\n", errout.str());
    }

    void constIfCfg()
    {
        const char code[] = "class foo {\n"
                            "    void f() { }\n"
                            "};";

        Settings settings;
        settings.addEnabled("information");

        settings.ifcfg = false;
        checkConst(code, &settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) Technically the member function 'foo::f' can be const.\n", errout.str());

        settings.ifcfg = true;
        checkConst(code, &settings);
        ASSERT_EQUALS("", errout.str());
    }

    void constFriend()	// ticket #1921
    {
        const char code[] = "class foo {\n"
                            "    friend void f() { }\n"
                            "};";
        checkConst(code);
        ASSERT_EQUALS("", errout.str());
    }

    void constUnion() // ticket #2111
    {
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

    void checkInitializerList(const char code[])
    {
        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings.addEnabled("style");
        settings.inconclusive = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.initializerList();
    }

    void initializerList()
    {
        checkInitializerList("class Fred {\n"
                             "    int a, b, c;\n"
                             "public:\n"
                             "    Fred() : c(0), b(0), a(0) { }\n"
                             "};");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style) Member variable 'Fred::b' is in the wrong order in the initializer list.\n"
                      "[test.cpp:4] -> [test.cpp:2]: (style) Member variable 'Fred::a' is in the wrong order in the initializer list.\n", errout.str());
    }

};

REGISTER_TEST(TestClass)
