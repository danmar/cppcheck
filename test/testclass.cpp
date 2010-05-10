/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
        TEST_CASE(virtualDestructorProtected);
        TEST_CASE(virtualDestructorInherited);
        TEST_CASE(virtualDestructorTemplate);

        TEST_CASE(uninitVar1);
        TEST_CASE(uninitVar2);
        TEST_CASE(uninitVar3);
        TEST_CASE(uninitVar4);
        TEST_CASE(uninitVar5);
        TEST_CASE(uninitVarEnum);
        TEST_CASE(uninitVarStream);
        TEST_CASE(uninitVarTypedef);
        TEST_CASE(uninitVarMemset);
        TEST_CASE(uninitVarArray1);
        TEST_CASE(uninitVarArray2);
        TEST_CASE(uninitVarArray3);
        TEST_CASE(uninitVarArray4);
        TEST_CASE(uninitVarArray2D);
        TEST_CASE(uninitMissingFuncDef);// can't expand function in constructor
        TEST_CASE(privateCtor1);        // If constructor is private..
        TEST_CASE(privateCtor2);        // If constructor is private..
        TEST_CASE(function);            // Function is not variable
        TEST_CASE(uninitVarHeader1);    // Class is defined in header
        TEST_CASE(uninitVarHeader2);    // Class is defined in header
        TEST_CASE(uninitVarHeader3);    // Class is defined in header
        TEST_CASE(uninitVarPublished);  // Borland C++: Variables in the published section are auto-initialized
        TEST_CASE(uninitProperty);		// Borland C++: No FP for properties
        TEST_CASE(uninitOperator);      // No FP about uninitialized 'operator[]'
        TEST_CASE(uninitFunction1);		// No FP when initialized in function
        TEST_CASE(uninitFunction2);		// No FP when initialized in function
        TEST_CASE(uninitSameClassName);	// No FP when two classes have the same name

        TEST_CASE(noConstructor1);
        TEST_CASE(noConstructor2);
        TEST_CASE(noConstructor3);
        TEST_CASE(noConstructor4);

        TEST_CASE(operatorEq1);
        TEST_CASE(operatorEqRetRefThis1);
        TEST_CASE(operatorEqRetRefThis2); // ticket #1323
        TEST_CASE(operatorEqRetRefThis3); // ticket #1405
        TEST_CASE(operatorEqRetRefThis4); // ticket #1451
        TEST_CASE(operatorEqRetRefThis5); // ticket #1550
        TEST_CASE(operatorEqToSelf1);   // single class
        TEST_CASE(operatorEqToSelf2);   // nested class
        TEST_CASE(operatorEqToSelf3);   // multiple inheritance
        TEST_CASE(operatorEqToSelf4);   // nested class with multiple inheritance
        TEST_CASE(operatorEqToSelf5);   // ticket # 1233
        TEST_CASE(operatorEqToSelf6);   // ticket # 1550
        TEST_CASE(memsetOnStruct);
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
        TEST_CASE(constoperator1);  // operator< can often be const
        TEST_CASE(constoperator2);	// operator<<
        TEST_CASE(constincdec);     // increment/decrement => non-const
        TEST_CASE(constReturnReference);
        TEST_CASE(constDelete);     // delete member variable => not const
        TEST_CASE(constLPVOID);     // a function that returns LPVOID can't be const
        TEST_CASE(constFunc); // a function that calls const functions can be const
    }

    // Check the operator Equal
    void checkOpertorEq(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
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
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should return something\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should return something\n"
                      "[test.cpp:9]: (style) 'operator=' should return something\n", errout.str());

        checkOpertorEq("struct A\n"
                       "{\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should return something\n", errout.str());
    }

    // Check that operator Equal returns reference to this
    void checkOpertorEqRetRefThis(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
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
            "    A & operator=(const A &)\n"
            "};\n"
            "A & A::operator=(const A &a) { return a; }\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a)\n"
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
            "szp &szp::operator =(int *other) {};");
        ASSERT_EQUALS("[test.cpp:5]: (style) 'operator=' should return reference to self\n", errout.str());
    }

    void operatorEqRetRefThis3()
    {
        // ticket # 1405
        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  inline A &operator =(int *other) { return (*this;) };\n"
            "  inline A &operator =(long *other) { return (*this = 0;) };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  A &operator =(int *other);\n"
            "  A &operator =(long *other);\n"
            "};\n"
            "A &A::operator =(int *other) { return (*this;) };\n"
            "A &A::operator =(long *other) { return (*this = 0;) };");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  inline A &operator =(int *other) { return (*this;) };\n"
            "  inline A &operator =(long *other) { return operator = (*(int *)other); };\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A {\n"
            "public:\n"
            "  A &operator =(int *other);\n"
            "  A &operator =(long *other);\n"
            "};\n"
            "A &A::operator =(int *other) { return (*this;) };\n"
            "A &A::operator =(long *other) { return operator = (*(int *)other); };");
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

    // Check that operator Equal checks for assignment to self
    void checkOpertorEqToSelf(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings.inconclusive = true;
        settings._checkCodingStyle = true;
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
        ASSERT_EQUALS("[test.cpp:5]: (style) 'operator=' should check for assignment to self\n", errout.str());

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
            "    A & operator=(const A &)\n"
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
            "A & operator=(const A &a)\n"
            "{\n"
            "    if (&a != this)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but doesnt have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & operator=(const A &a)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(a.s);\n"
            "    return *this;\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (style) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:8]: (style) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:11]: (style) 'operator=' should check for assignment to self\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:8]: (style) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should check for assignment to self\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:8]: (style) 'operator=' should check for assignment to self\n", errout.str());
    }

    // Check that base classes have virtual destructors
    void checkVirtualDestructor(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.virtualDestructor();
    }

    void virtualDestructor1()
    {
        // Base class not found

        checkVirtualDestructor("class Derived : public Base { };");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Derived : Base { };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor2()
    {
        // Base class doesn't have a destructor

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor3()
    {
        // Base class has a destructor, but it's not virtual

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());
    }

    void virtualDestructor4()
    {
        // Derived class doesn't have a destructor => no error

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { };");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor5()
    {
        // Derived class has empty destructor => no error

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() {} };");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived(); }; Derived::~Derived() {}");
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
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Class A which is inherited by class B does not have a virtual destructor\n", errout.str());
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
                               "};\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Class AA<double> which is inherited by class B does not have a virtual destructor\n", errout.str());
    }

    void checkUninitVar(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings.inconclusive = true;
        settings._checkCodingStyle = true;
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

        ASSERT_EQUALS("[test.cpp:10]: (style) Member variable not initialized in the constructor 'Fred::_code'\n", errout.str());


        checkUninitVar("class A{};\n"
                       "\n"
                       "class B : public A\n"
                       "{\n"
                       "public:\n"
                       "  B() {}\n"
                       "private:\n"
                       "  float f;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) Member variable not initialized in the constructor 'B::f'\n", errout.str());

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
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Member variable not initialized in the constructor 'Foo::bars'\n", errout.str());
        ASSERT_EQUALS("", errout.str());	// So we notice if something is reported.
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
        ASSERT_EQUALS("[test.cpp:4]: (style) Member variable not initialized in the constructor 'John::name'\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5]: (style) Member variable not initialized in the constructor 'John::a'\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
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

        ASSERT_EQUALS("[test.cpp:5]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
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
    }

    void privateCtor1()
    {
        checkUninitVar("class Foo {\n"
                       "    int foo;\n"
                       "    Foo() { }\n"
                       "};\n");

        ASSERT_EQUALS("[test.cpp:3]: (possible style) Member variable not initialized in the constructor 'Foo::foo'\n", errout.str());
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

        // actual results - "possible style" for both messages
        ASSERT_EQUALS("[test.cpp:5]: (possible style) Member variable not initialized in the constructor 'Foo::foo'\n"
                      "[test.cpp:7]: (possible style) Member variable not initialized in the constructor 'Foo::foo'\n", errout.str());

        // wanted results - "style" for the public constructor
        TODO_ASSERT_EQUALS("[test.cpp:5]: (possible style) Member variable not initialized in the constructor 'Foo::foo'\n"
                           "[test.cpp:7]: (style) Member variable not initialized in the constructor 'Foo::foo'\n", errout.str());
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
        ASSERT_EQUALS("[fred.h:6]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
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
        ASSERT_EQUALS("[fred.h:6]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
    }

    // Borland C++: No FP for published pointers - they are automaticly initialized
    void uninitVarPublished()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "__published:\n"
                       "    int *i;\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class Fred\n"
                       "{\n"
                       "__published:\n"
                       "    int * i_;\n"
                       "    __property int * i = {read=i_, write=i_};\n"
                       "public:\n"
                       "    Fred() { i_ = 0; }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Borland C++: No FP for properties
    void uninitProperty()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "private:\n"
                       "    int * i_;\n"
                       "public:\n"
                       "    Fred() { i_ = 0; }\n"
                       "    __property int * i = {read=i_, write=i_};\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitOperator()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "    int *operator [] (int index) { return 0; }\n"
                       "}\n");
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());
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
                       "}\n");
        ASSERT_EQUALS("", errout.str());
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
    }


    void checkNoConstructor(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
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
        ASSERT_EQUALS("[test.cpp:1]: (style) The class 'Fred' has no constructor. Member variables not initialized.\n", errout.str());
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

    void checkNoMemset(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");


        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.noMemset();
    }

    void memsetOnClass()
    {
        checkNoMemset("class A\n"
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
                      "    memset(&a, 0, sizeof(A));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void memsetOnStruct()
    {
        checkNoMemset("class A\n"
                      "{\n"
                      " void g( struct sockaddr_in6& a);\n"
                      "private:\n"
                      " std::string b; \n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      " struct sockaddr_in6 fail;\n"
                      " memset(&fail, 0, sizeof(struct sockaddr_in6));\n"
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
    }

    void memsetVector()
    {
        checkNoMemset("struct A\n"
                      "{ std::vector<int> ints; }\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector< std::vector<int> > ints; }\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());

        checkNoMemset("struct A\n"
                      "{ std::vector<int *> ints; }\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    A a;\n"
                      "    memset(a, 0, sizeof(A));\n"
                      "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on struct that contains a 'std::vector'\n", errout.str());
    }


    void checkThisSubtraction(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        settings.inconclusive = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.thisSubtraction();
    }

    void this_subtraction()
    {
        checkThisSubtraction("; this-x ;");
        ASSERT_EQUALS("[test.cpp:1]: (style) Suspicious pointer subtraction\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;");
        ASSERT_EQUALS("", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious pointer subtraction\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;\n"
                             "this-x ;\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Suspicious pointer subtraction\n"
                      "[test.cpp:3]: (style) Suspicious pointer subtraction\n", errout.str());
    }

    void checkConst(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.checkConst();
    }

    void const1()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int getA() { return a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::getA' can be const\n", errout.str());

        checkConst("class Fred {\n"
                   "    const std::string foo() { return ""; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) The function 'Fred::foo' can be const\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    const std::string & foo() { return ""; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) The function 'Fred::getA' can be const\n", errout.str());

        checkConst("class Fred {\n"
                   "    const std::string foo();\n"
                   "};\n"
                   "const std::string Fred::foo() { return ""; }");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style) The function 'Fred::foo' can be const\n", errout.str());

        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    const std::string & foo();\n"
                   "};\n"
                   "const std::string & Fred::foo() { return ""; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    int x;\n"
                   "    void b();\n"
                   "};\n"
                   "void Fred::b() { a(); }");
        ASSERT_EQUALS("", errout.str());

        // static functions can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    static unsigned get();\n"
                   "};\n"
                   "static unsigned Fred::get() { return 0; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to variable can't be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo()\n"
                   "};\n"
                   "void Fred::foo() { s = ""; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument reference can be const
        checkConst("class Fred {\n"
                   "    std::string s;\n"
                   "    void foo(std::string & a);\n"
                   "};\n"
                   "void Fred::foo(std::string & a) { a = s; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
                   "void foo(std::string & a, std::string & b) { a = s; s = b; }");
        ASSERT_EQUALS("", errout.str());

        // assignment to function argument pointer can be const
        checkConst("class Fred {\n"
                   "    int s;\n"
                   "    void foo(int * a);\n"
                   "};\n"
                   "void Fred::foo(int * a) { *a = s; }");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:3]: (style) The function 'Fred::foo' can be const\n"
                      "[test.cpp:7] -> [test.cpp:4]: (style) The function 'Fred::foo' can be const\n", errout.str());

        // check functions with different or missing paramater names
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
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (style) The function 'Fred::foo1' can be const\n"
                      "[test.cpp:10] -> [test.cpp:4]: (style) The function 'Fred::foo2' can be const\n"
                      "[test.cpp:11] -> [test.cpp:5]: (style) The function 'Fred::foo3' can be const\n"
                      "[test.cpp:12] -> [test.cpp:6]: (style) The function 'Fred::foo4' can be const\n"
                      "[test.cpp:13] -> [test.cpp:7]: (style) The function 'Fred::foo5' can be const\n", errout.str());

        // check nested classes
        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA() { return a; }\n"
                   "    };\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'Fred::A::getA' can be const\n", errout.str());

        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA();\n"
                   "    };\n"
                   "    int A::getA() { return a; }\n"
                   "};");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:4]: (style) The function 'Fred::A::getA' can be const\n", errout.str());

        checkConst("class Fred {\n"
                   "    class A {\n"
                   "        int a;\n"
                   "        int getA();\n"
                   "    };\n"
                   "};\n"
                   "int Fred::A::getA() { return a; }");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:4]: (style) The function 'Fred::A::getA' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'Fred::B::getB' can be const\n"
                      "[test.cpp:7]: (style) The function 'Fred::B::A::getA' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:4]: (style) The function 'Fred::B::getB' can be const\n"
                      "[test.cpp:9] -> [test.cpp:7]: (style) The function 'Fred::B::A::getA' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:12] -> [test.cpp:4]: (style) The function 'Fred::B::getB' can be const\n"
                      "[test.cpp:11] -> [test.cpp:7]: (style) The function 'Fred::B::A::getA' can be const\n", errout.str());
    }

    // operator< can often be const
    void constoperator1()
    {
        checkConst("struct Fred {\n"
                   "    int a;\n"
                   "    bool operator<(const Fred &f) { return (a < f.a); }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::operator<' can be const\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'A::foo' can be const\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'Fred::foo' can be const\n", errout.str());

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
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::strGetString' can be const\n", errout.str());
    }

    void const9()
    {
        // ticket #1515
        checkConst("class wxThreadInternal {\n"
                   "public:\n"
                   "    void SetExitCode(wxThread::ExitCode exitcode) { m_exitcode = exitcode; }\n"
                   "private:\n"
                   "    wxThread::ExitCode m_exitcode;\n"
                   "}");
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
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { return (x ? x : x = 0); }\n"
                   "private:\n"
                   "    int x;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    int foo() { return (x ? x = 0 : x); }\n"
                   "private:\n"
                   "    int x;\n"
                   "}");
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
                   "}");
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
                   "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'A::foo' can be const\n", errout.str());
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
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetVec' can be const\n"
                      "[test.cpp:5]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::vector<int> & GetVec()  {return m_vec;}\n"
                   "    const std::pair<int,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::vector<int> m_vec;\n"
                   "    std::pair<int,double> m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetVec' can be const\n"
                      "[test.cpp:5]: (style) The function 'A::GetPair' can be const\n", errout.str());
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
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair<std::vector<int>,double>& GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair<std::vector<int>,double> m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair<std::vector<int>,double>& GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair<std::vector<int>,double> m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair<int ,double> GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair<int ,double> m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair<int ,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair<int ,double> m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair<int ,double> & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair<int ,double> m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int,std::vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int,std::vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< int,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int,std::vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int,std::vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< int,vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int,vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< int,vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int,vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< int,vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int,vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, int >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< vector<int>, int >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, int >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< std::vector<int>,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::vector<int>,std::vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::vector<int>,std::vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, vector<int> >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< vector<int>, vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< vector<int>, vector<int> >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< vector<int>, vector<int> >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());



        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::pair < int, char > , int >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< std::pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< std::pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< std::pair < int, char > , int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< pair < int, char > , int >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< pair < int, char > , int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< pair < int, char > , int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< pair < int, char > , int > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< pair < int, char > , int >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< int , pair < int, char > >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int , pair < int, char > >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const pair< int , pair < int, char > > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int , pair < int, char > >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    pair< int , pair < int, char > > & GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    pair< int , pair < int, char > >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int , std::pair < int, char > >  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const std::pair< int , std::pair < int, char > >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetPair' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    std::pair< int , std::pair < int, char > >&  GetPair() {return m_pair;}\n"
                   "private:\n"
                   "    std::pair< int , std::pair < int, char > >  m_pair;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    vector<int>  GetVec() {return m_Vec;}\n"
                   "private:\n"
                   "    vector<int>  m_Vec;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetVec' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    const vector<int>&  GetVec() {return m_Vec;}\n"
                   "private:\n"
                   "    vector<int>  m_Vec;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'A::GetVec' can be const\n", errout.str());

        checkConst("using namespace std;"
                   "class A {\n"
                   "public:\n"
                   "    A(){}\n"
                   "    vector<int>&  GetVec() {return m_Vec;}\n"
                   "private:\n"
                   "    vector<int>  m_Vec;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());


        checkConst("class A {\n"
                   "public:\n"
                   "    int * const * foo() { return &x; }\n"
                   "private:\n"
                   "    const int * x;\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    const int ** foo() { return &x; }\n"
                   "private:\n"
                   "    const int * x;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'A::foo' can be const\n", errout.str());

        checkConst("class A {\n"
                   "public:\n"
                   "    const int * const * foo() { return &x; }\n"
                   "private:\n"
                   "    const int * x;\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'A::foo' can be const\n", errout.str());
    }

    void const15()
    {
        checkConst("class Fred {\n"
                   "    unsigned long long int a;\n"
                   "    unsigned long long int getA() { return a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::getA' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'Fred::get' can be const\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (style) The function 'Fred::get' can be const\n", errout.str());
    }

    // increment/decrement => not const
    void constincdec()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return ++a; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
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
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::a' can be const\n", errout.str());

        // ticket #1593
        checkConst("#include <vector>\n"
                   "class A\n"
                   "{\n"
                   "   std::vector<int> m_v;\n"
                   "public:\n"
                   "   A(){}\n"
                   "   unsigned int GetVecSize()  {return m_v.size();}\n"
                   "}");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (style) The function 'A::GetVecSize' can be const\n", errout.str());
    }
};

REGISTER_TEST(TestClass)
