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


#include "config.h"
#include "platform.h"
#include "settings.h"
#include "templatesimplifier.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"

struct InternalError;


class TestSimplifyTemplate : public TestFixture {
public:
    TestSimplifyTemplate() : TestFixture("TestSimplifyTemplate") {
    }

private:
    Settings settings;

    void run() override {
        settings.addEnabled("portability");

        TEST_CASE(template1);
        TEST_CASE(template2);
        TEST_CASE(template3);
        TEST_CASE(template4);
        TEST_CASE(template5);
        TEST_CASE(template6);
        TEST_CASE(template7);
        TEST_CASE(template8);
        TEST_CASE(template9);
        TEST_CASE(template10);
        TEST_CASE(template11);
        TEST_CASE(template12);
        TEST_CASE(template13);
        TEST_CASE(template14);
        TEST_CASE(template15);  // recursive templates
        TEST_CASE(template16);
        TEST_CASE(template17);
        TEST_CASE(template18);
        TEST_CASE(template19);
        TEST_CASE(template20);
        TEST_CASE(template21);
        TEST_CASE(template22);
        TEST_CASE(template23);
        TEST_CASE(template24);  // #2648 - using sizeof in template parameter
        TEST_CASE(template25);  // #2648 - another test for sizeof template parameter
        TEST_CASE(template26);  // #2721 - passing 'char[2]' as template parameter
        TEST_CASE(template27);  // #3350 - removing unused template in macro call
        TEST_CASE(template28);
        TEST_CASE(template30);  // #3529 - template < template < ..
        TEST_CASE(template31);  // #4010 - reference type
        TEST_CASE(template32);  // #3818 - mismatching template not handled well
        TEST_CASE(template33);  // #3818,#4544 - inner templates in template instantiation not handled well
        TEST_CASE(template34);  // #3706 - namespace => hang
        TEST_CASE(template35);  // #4074 - A<'x'> a;
        TEST_CASE(template36);  // #4310 - passing unknown template instantiation as template argument
        TEST_CASE(template37);  // #4544 - A<class B> a;
        TEST_CASE(template38);  // #4832 - crash on C++11 right angle brackets
        TEST_CASE(template39);  // #4742 - freeze
        TEST_CASE(template40);  // #5055 - template specialization outside struct
        TEST_CASE(template41);  // #4710 - const in instantiation not handled perfectly
        TEST_CASE(template42);  // #4878 - variadic templates
        TEST_CASE(template43);  // #5097 - assert due to '>>' not treated as end of template instantiation
        TEST_CASE(template44);  // #5297 - TemplateSimplifier::simplifyCalculations not eager enough
        TEST_CASE(template45);  // #5814 - syntax error reported for valid code
        TEST_CASE(template46);  // #5816 - syntax error reported for valid code
        TEST_CASE(template47);  // #6023 - syntax error reported for valid code
        TEST_CASE(template48);  // #6134 - 100% CPU upon invalid code
        TEST_CASE(template49);  // #6237 - template instantiation
        TEST_CASE(template50);  // #4272 - simple partial specialization
        TEST_CASE(template51);  // #6172 - crash upon valid code
        TEST_CASE(template52);  // #6437 - crash upon valid code
        TEST_CASE(template53);  // #4335 - bail out for valid code
        TEST_CASE(template54);  // #6587 - memory corruption upon valid code
        TEST_CASE(template55);  // #6604 - simplify "const const" to "const" in template instantiations
        TEST_CASE(template56);  // #7117 - const ternary operator simplification as template parameter
        TEST_CASE(template57);  // #7891
        TEST_CASE(template58);  // #6021 - use after free (deleted tokens in simplifyCalculations)
        TEST_CASE(template59);  // #8051 - TemplateSimplifier::simplifyTemplateInstantiation failure
        TEST_CASE(template60);  // handling of methods outside template definition
        TEST_CASE(template61);  // daca2, kodi
        TEST_CASE(template62);  // #8314 - inner template instantiation
        TEST_CASE(template63);  // #8576 - qualified type
        TEST_CASE(template64);  // #8683
        TEST_CASE(template65);  // #8321
        TEST_CASE(template66);  // #8725
        TEST_CASE(template67);  // #8122
        TEST_CASE(template68);  // union
        TEST_CASE(template69);  // #8791
        TEST_CASE(template70);  // #5289
        TEST_CASE(template_specialization_1);  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        TEST_CASE(template_specialization_2);  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        TEST_CASE(template_enum);  // #6299 Syntax error in complex enum declaration (including template)
        TEST_CASE(template_unhandled);
        TEST_CASE(template_default_parameter);
        TEST_CASE(template_forward_declared_default_parameter);
        TEST_CASE(template_default_type);
        TEST_CASE(template_typename);
        TEST_CASE(template_constructor);    // #3152 - template constructor is removed
        TEST_CASE(syntax_error_templates_1);
        TEST_CASE(template_member_ptr); // Ticket #5786 - crash upon valid code
        TEST_CASE(template_namespace_1);
        TEST_CASE(template_namespace_2);
        TEST_CASE(template_namespace_3);
        TEST_CASE(template_namespace_4);
        TEST_CASE(template_namespace_5);
        TEST_CASE(template_namespace_6);
        TEST_CASE(template_namespace_7); // #8768

        // Test TemplateSimplifier::templateParameters
        TEST_CASE(templateParameters);

        TEST_CASE(templateNamePosition);

        TEST_CASE(expandSpecialized1);
        TEST_CASE(expandSpecialized2);
        TEST_CASE(expandSpecialized3); // #8671
        TEST_CASE(expandSpecialized4);

        TEST_CASE(templateAlias1);
        TEST_CASE(templateAlias2);
        TEST_CASE(templateAlias3); // #8315

        // Test TemplateSimplifier::instantiateMatch
        TEST_CASE(instantiateMatch);
        TEST_CASE(templateParameterWithoutName); // #8602 Template default parameter without name yields syntax error
    }

    std::string tok(const char code[], bool simplify = true, bool debugwarnings = false, Settings::PlatformType type = Settings::Native) {
        errout.str("");

        settings.debugwarnings = debugwarnings;
        settings.platform(type);
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, !simplify);
    }

    std::string tok(const char code[], const char filename[]) {
        errout.str("");

        settings.debugwarnings = false;
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, false);
    }

    void template1() {
        const char code[] = "template <class T> void f(T val) { T a; }\n"
                            "f<int>(10);";

        const char expected[] = "f<int> ( 10 ) ; "
                                "void f<int> ( int val ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template2() {
        const char code[] = "template <class T> class Fred { T a; };\n"
                            "Fred<int> fred;";

        const char expected[] = "Fred<int> fred ; "
                                "class Fred<int> { int a ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template3() {
        const char code[] = "template <class T, int sz> class Fred { T data[sz]; };\n"
                            "Fred<float,4> fred;";

        const char expected[] = "Fred<float,4> fred ; "
                                "class Fred<float,4> { float data [ 4 ] ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template4() {
        const char code[] = "template <class T> class Fred { Fred(); };\n"
                            "Fred<float> fred;";

        const char expected[] = "Fred<float> fred ; "
                                "class Fred<float> { Fred<float> ( ) ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template5() {
        const char code[] = "template <class T> class Fred { };\n"
                            "template <class T> Fred<T>::Fred() { }\n"
                            "Fred<float> fred;";

        const char expected[] = "Fred<float> fred ; "
                                "class Fred<float> { } ; "
                                "Fred<float> :: Fred<float> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template6() {
        const char code[] = "template <class T> class Fred { };\n"
                            "Fred<float> fred1;\n"
                            "Fred<float> fred2;";

        const char expected[] = "Fred<float> fred1 ; "
                                "Fred<float> fred2 ; "
                                "class Fred<float> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template7() {
        // A template class that is not used => no simplification
        {
            const char code[] = "template <class T>\n"
                                "class ABC\n"
                                "{\n"
                                "public:\n"
                                "    typedef ABC<T> m;\n"
                                "};\n";

            const char expected[] = "template < class T > class ABC { public: } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <typename T> class ABC {\n"
                                "public:\n"
                                "    typedef std::vector<T> type;\n"
                                "};\n"
                                "int main() {\n"
                                "    ABC<int>::type v;\n"
                                "    v.push_back(4);\n"
                                "    return 0;\n"
                                "}\n";

            const char wanted[] = "template < typename T > class ABC { public: } ; "
                                  "int main ( ) { "
                                  "std :: vector < int > v ; "
                                  "v . push_back ( 4 ) ; "
                                  "return 0 ; "
                                  "}";

            const char current[] = "template < typename T > class ABC { public: } ; "
                                   "int main ( ) { "
                                   "ABC < int > :: type v ; "
                                   "v . push_back ( 4 ) ; "
                                   "return 0 ; "
                                   "}";

            TODO_ASSERT_EQUALS(wanted, current, tok(code));
        }

        {
            const char code[] = "template <typename T> class ABC {\n"
                                "public:\n"
                                "    typedef std::vector<T> type;\n"
                                "    void f()\n"
                                "    {\n"
                                "      ABC<int>::type v;\n"
                                "      v.push_back(4);\n"
                                "    }\n"
                                "};\n";

            const char expected[] = "template < typename T > class ABC { "
                                    "public: void f ( ) { "
                                    "ABC < int > :: type v ; "
                                    "v . push_back ( 4 ) ; "
                                    "} "
                                    "} ;";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    // Template definitions but no usage => no expansion
    void template8() {
        const char code[] = "template<typename T> class A;\n"
                            "template<typename T> class B;\n"
                            "\n"
                            "typedef A<int> x;\n"
                            "typedef B<int> y;\n"
                            "\n"
                            "template<typename T> class A {\n"
                            "    void f() {\n"
                            "        B<T> a = B<T>::g();\n"
                            "        T b = 0;\n"
                            "        if (b)\n"
                            "            b = 0;\n"
                            "    }\n"
                            "};\n"
                            "\n"
                            "template<typename T> inline B<T> h() { return B<T>(); }\n";

        ASSERT_EQUALS("template < typename T > class A ; "
                      "template < typename T > class B ; "
                      "template < typename T > class A { void f ( ) { B < T > a ; a = B < T > :: g ( ) ; T b ; b = 0 ; } } ; "
                      "template < typename T > B < T > h ( ) { return B < T > ( ) ; }", tok(code));

        ASSERT_EQUALS("class A { template < typename T > int foo ( T d ) ; } ;", tok("class A{ template<typename T> int foo(T d);};"));
    }

    void template9() {
        const char code[] = "template < typename T > class A { } ;\n"
                            "\n"
                            "void f ( ) {\n"
                            "    A < int > a ;\n"
                            "}\n"
                            "\n"
                            "template < typename T >\n"
                            "class B {\n"
                            "    void g ( ) {\n"
                            "        A < T > b = A < T > :: h ( ) ;\n"
                            "    }\n"
                            "} ;\n";

        // The expected result..
        const char expected[] = "void f ( ) { A<int> a ; } "
                                "template < typename T > class B { void g ( ) { A < T > b ; b = A < T > :: h ( ) ; } } ; "
                                "class A<int> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template10() {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    foo<3,int>();\n"
                            "}\n";

        // The expected result..
        const char expected[] = "void f ( ) "
                                "{"
                                " foo<3,int> ( ) ; "
                                "} "
                                "int * foo<3,int> ( ) { return new int [ 3 ] ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template11() {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char * p = foo<3,char>();\n"
                            "}\n";

        // The expected result..
        const char expected[] = "void f ( ) "
                                "{"
                                " char * p ; p = foo<3,char> ( ) ; "
                                "} "
                                "char * foo<3,char> ( ) { return new char [ 3 ] ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template12() {
        const char code[] = "template <int x, int y, int z>\n"
                            "class A : public B<x, y, (x - y) ? ((y < z) ? 1 : -1) : 0>\n"
                            "{ };\n"
                            "\n"
                            "void f()\n"
                            "{\n"
                            "    A<12,12,11> a;\n"
                            "}\n";

        // The expected result..
        const char expected[] = "void f ( ) "
                                "{"
                                " A<12,12,11> a ; "
                                "} "
                                "class A<12,12,11> : public B < 12 , 12 , 0 > "
                                "{ } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template13() {
        const char code[] = "class BB {};\n"
                            "\n"
                            "template <class T>\n"
                            "class AA\n"
                            "{\n"
                            "public:\n"
                            "    static AA<T> create(T* newObject);\n"
                            "    static int size();\n"
                            "};\n"
                            "\n"
                            "class CC { public: CC(AA<BB>, int) {} };\n"
                            "\n"
                            "class XX {\n"
                            "    AA<CC> y;\n"
                            "public:\n"
                            "    XX();\n"
                            "};\n"
                            "\n"
                            "XX::XX():\n"
                            "    y(AA<CC>::create(new CC(AA<BB>(), 0)))\n"
                            "    {}\n"
                            "\n"
                            "int yy[AA<CC>::size()];";

        // Just run it and check that there are not assertions.
        tok(code);
    }

    void template14() {
        const char code[] = "template <> void foo<int *>()\n"
                            "{ x(); }\n"
                            "\n"
                            "int main()\n"
                            "{\n"
                            "foo<int*>();\n"
                            "}\n";

        // The expected result..
        const char expected[] = "void foo<int*> ( ) "
                                "{ x ( ) ; } "
                                "int main ( ) "
                                "{ foo<int*> ( ) ; }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template15() { // recursive templates  #3130 etc
        const char code[] = "template <unsigned int i> void a()\n"
                            "{\n"
                            "    a<i-1>();\n"
                            "}\n"
                            "\n"
                            "template <> void a<0>()\n"
                            "{ }\n"
                            "\n"
                            "int main()\n"
                            "{\n"
                            "    a<2>();\n"
                            "    return 0;\n"
                            "}\n";

        // The expected result..
        const char expected[] = "void a<0> ( ) { } "
                                "int main ( ) "
                                "{ a<2> ( ) ; return 0 ; } "
                                "void a<2> ( ) { a<1> ( ) ; } "
                                "void a<1> ( ) { a<0> ( ) ; }";

        ASSERT_EQUALS(expected, tok(code));

        // #3130
        const char code2[] = "template <int n> struct vec {\n"
                             "  vec() {}\n"
                             "  vec(const vec<n-1>& v) {}\n" // <- never used don't instantiate
                             "};\n"
                             "\n"
                             "vec<4> v;";
        const char expected2[] = "vec<4> v ; "
                                 "struct vec<4> { "
                                 "vec<4> ( ) { } "
                                 "vec<4> ( const vec < 4 - 1 > & v ) { } "
                                 "} ;";

        ASSERT_EQUALS(expected2, tok(code2));
    }

    void template16() {
        const char code[] = "template <unsigned int i> void a()\n"
                            "{ }\n"
                            "\n"
                            "template <unsigned int i> void b()\n"
                            "{ a<i>(); }\n"
                            "\n"
                            "int main()\n"
                            "{\n"
                            "    b<2>();\n"
                            "    return 0;\n"
                            "}\n";

        const char expected[] = "int main ( ) { b<2> ( ) ; return 0 ; } "
                                "void b<2> ( ) { a<2> ( ) ; } "
                                "void a<2> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template17() {
        const char code[] = "template<class T>\n"
                            "class Fred\n"
                            "{\n"
                            "    template<class T>\n"
                            "    static shared_ptr< Fred<T> > CreateFred()\n"
                            "    {\n"
                            "    }\n"
                            "};\n"
                            "\n"
                            "shared_ptr<int> i;\n";

        // Assert that there is no segmentation fault..
        tok(code);
    }

    void template18() {
        const char code[] = "template <class T> class foo { T a; };\n"
                            "foo<int> *f;";

        const char expected[] = "foo<int> * f ; "
                                "class foo<int> { int a ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template19() {
        const char code[] = "template <typename T> T & foo()\n"
                            "{ static T temp; return temp; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char p = foo<char>();\n"
                            "}\n";

        // The expected result..
        const char expected[] = "void f ( ) "
                                "{"
                                " char p ; p = foo<char> ( ) ; "
                                "} "
                                "char & foo<char> ( ) { static char temp ; return temp ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template20() {
        // Ticket #1788 - the destructor implementation is lost
        const char code[] = "template <class T> class A { public:  ~A(); };\n"
                            "template <class T> A<T>::~A() {}\n"
                            "A<int> a;\n";

        // The expected result..
        const char expected[] = "A<int> a ; "
                                "class A<int> { public: ~ A<int> ( ) ; } ; "
                                "A<int> :: ~ A<int> ( ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template21() {
        {
            const char code[] = "template <class T> struct Fred { T a; };\n"
                                "Fred<int> fred;";

            const char expected[] = "Fred<int> fred ; "
                                    "struct Fred<int> { int a ; } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <class T, int sz> struct Fred { T data[sz]; };\n"
                                "Fred<float,4> fred;";

            const char expected[] = "Fred<float,4> fred ; "
                                    "struct Fred<float,4> { float data [ 4 ] ; } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <class T> struct Fred { Fred(); };\n"
                                "Fred<float> fred;";

            const char expected[] = "Fred<float> fred ; "
                                    "struct Fred<float> { Fred<float> ( ) ; } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <class T> struct Fred { };\n"
                                "Fred<float> fred1;\n"
                                "Fred<float> fred2;";

            const char expected[] = "Fred<float> fred1 ; "
                                    "Fred<float> fred2 ; "
                                    "struct Fred<float> { } ;";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void template22() {
        const char code[] = "template <class T> struct Fred { T a; };\n"
                            "Fred<std::string> fred;";

        const char expected[] = "Fred<std::string> fred ; "
                                "struct Fred<std::string> { std :: string a ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template23() {
        const char code[] = "template <class T> void foo() { }\n"
                            "void bar() {\n"
                            "    std::cout << (foo<double>());\n"
                            "}";

        const char expected[] = "void bar ( ) {"
                                " std :: cout << ( foo<double> ( ) ) ; "
                                "} "
                                "void foo<double> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template24() {
        // #2648
        const char code[] = "template<int n> struct B\n"
                            "{\n"
                            "  int a[n];\n"
                            "};\n"
                            "\n"
                            "template<int x> class bitset: B<sizeof(int)>\n"
                            "{};\n"
                            "\n"
                            "bitset<1> z;";
        const char expected[] = "class bitset<1> ; "
                                "bitset<1> z ; "
                                "class bitset<1> : B<4> { } ; "
                                "struct B<4> { int a [ 4 ] ; } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template25() {
        const char code[] = "template<int n> struct B\n"
                            "{\n"
                            "  int a[n];\n"
                            "};\n"
                            "\n"
                            "template<int x> class bitset: B<((sizeof(int)) ? : 1)>\n"
                            "{};\n"
                            "\n"
                            "bitset<1> z;";

        const char actual[] = "template < int n > struct B { int a [ n ] ; } ; "
                              "class bitset<1> ; "
                              "bitset<1> z ; "
                              "class bitset<1> : B < 4 > { } ;";

        const char expected[] = "class bitset<1> ; "
                                "bitset<1> z ; "
                                "class bitset<1> : B < 4 > { } ; "
                                "struct B < 4 > { int a [ 4 ] ; } ;";

        TODO_ASSERT_EQUALS(expected, actual, tok(code));
    }

    void template26() {
        // #2721
        const char code[] = "template<class T>\n"
                            "class A { public: T x; };\n"
                            "\n"
                            "template<class M>\n"
                            "class C: public A<char[M]> {};\n"
                            "\n"
                            "C<2> a;\n";
        // TODO: expand A also
        ASSERT_EQUALS("template < class T > class A { public: T x ; } ; class C<2> ; C<2> a ; class C<2> : public A < char [ 2 ] > { } ;", tok(code));
    }

    void template27() {
        // #3350 - template inside macro call
        const char code[] = "X(template<class T> class Fred);";
        ASSERT_THROW(tok(code), InternalError);
    }

    void template28() {
        // #3226 - inner template
        const char code[] = "template<class A, class B> class Fred {};\n"
                            "Fred<int,Fred<int,int> > x;\n";
        ASSERT_EQUALS("Fred<int,Fred<int,int>> x ; class Fred<int,int> { } ; class Fred<int,Fred<int,int>> { } ;", tok(code));
    }

    void template30() {
        // #3529 - template < template < ..
        const char code[] = "template<template<class> class A, class B> void f(){}";
        ASSERT_EQUALS("template < template < class > class A , class B > void f ( ) { }", tok(code));
    }

    void template31() {
        // #4010 - template reference type
        const char code[] = "template<class T> struct A{}; A<int&> a;";
        ASSERT_EQUALS("A<int&> a ; struct A<int&> { } ;", tok(code));

        // #7409 - rvalue
        const char code2[] = "template<class T> struct A{}; A<int&&> a;";
        ASSERT_EQUALS("A<int&&> a ; struct A<int&&> { } ;", tok(code2));
    }

    void template32() {
        // #3818 - mismatching template not handled well
        const char code[] = "template <class T1, class T2, class T3, class T4 > struct A { };\n"
                            "\n"
                            "template <class T>\n"
                            "struct B\n"
                            "{\n"
                            "    public:\n"
                            "        A < int, Pair<T, int>, int > a;\n"  // mismatching parameters => don't instantiate
                            "};\n"
                            "\n"
                            "B<int> b;\n";
        ASSERT_EQUALS("template < class T1 , class T2 , class T3 , class T4 > struct A { } ; "
                      "struct B<int> ; "
                      "B<int> b ; "
                      "struct B<int> { public: A < int , Pair < int , int > , int > a ; } ;", tok(code));
    }

    void template33() {
        {
            // #3818 - inner templates in template instantiation not handled well
            const char code[] = "template<class T> struct A { };\n"
                                "template<class T> struct B { };\n"
                                "template<class T> struct C { A<B<X<T> > > ab; };\n"
                                "C<int> c;";
            ASSERT_EQUALS("struct B<X<int>> ; "
                          "struct C<int> ; "
                          "C<int> c ; "
                          "struct C<int> { A<B<X<int>>> ab ; } ; "
                          "struct B<X<int>> { } ; "  // <- redundant.. but nevermind
                          "struct A<B<X<int>>> { } ;", tok(code));
        }

        {
            // #4544
            const char code[] = "struct A { };\n"
                                "template<class T> struct B { };\n"
                                "template<class T> struct C { };\n"
                                "C< B<A> > c;";
            ASSERT_EQUALS("struct A { } ; "
                          "template < class T > struct B { } ; "  // <- redundant.. but nevermind
                          "struct C<B<A>> ; "
                          "C<B<A>> c ; "
                          "struct C<B<A>> { } ;",
                          tok(code));
        }
    }

    void template34() {
        // #3706 - namespace => hang
        const char code[] = "namespace abc {\n"
                            "template <typename T> struct X { void f(X<T> &x) {} };\n"
                            "}\n"
                            "template <> int X<int>::Y(0);";
        tok(code);
    }

    void template35() { // #4074 - "A<'x'> a;" is not recognized as template instantiation
        const char code[] = "template <char c> class A {};\n"
                            "A <'x'> a;";
        ASSERT_EQUALS("A<'x'> a ; class A<'x'> { } ;", tok(code));
    }

    void template36() { // #4310 - Passing unknown template instantiation as template argument
        const char code[] = "template <class T> struct X { T t; };\n"
                            "template <class C> struct Y { Foo < X< Bar<C> > > _foo; };\n" // <- Bar is unknown
                            "Y<int> bar;";
        ASSERT_EQUALS("struct Y<int> ; "
                      "Y<int> bar ; "
                      "struct Y<int> { Foo < X<Bar<int>> > _foo ; } ; "
                      "struct X<Bar<int>> { Bar < int > t ; } ;",
                      tok(code));
    }

    void template37() { // #4544 - A<class B> a;
        {
            const char code[] = "class A { };\n"
                                "template<class T> class B {};\n"
                                "B<class A> b1;\n"
                                "B<A> b2;";
            ASSERT_EQUALS("class A { } ; class B<A> ; B<A> b1 ; B<A> b2 ; class B<A> { } ;",
                          tok(code));
        }
        {
            const char code[] = "struct A { };\n"
                                "template<class T> class B {};\n"
                                "B<struct A> b1;\n"
                                "B<A> b2;";
            ASSERT_EQUALS("struct A { } ; class B<A> ; B<A> b1 ; B<A> b2 ; class B<A> { } ;",
                          tok(code));
        }
        {
            const char code[] = "enum A { };\n"
                                "template<class T> class B {};\n"
                                "B<enum A> b1;\n"
                                "B<A> b2;";
            ASSERT_EQUALS("enum A { } ; class B<A> ; B<A> b1 ; B<A> b2 ; class B<A> { } ;",
                          tok(code));
        }
    }

    void template_unhandled() {
        // An unhandled template usage should be simplified..
        ASSERT_EQUALS("x<int> ( ) ;", tok("x<int>();"));
    }

    void template38() { // #4832 - Crash on C++11 right angle brackets
        const char code[] = "template <class T> class A {\n"
                            "  T mT;\n"
                            "public:\n"
                            "  void foo() {}\n"
                            "};\n"
                            "\n"
                            "int main() {\n"
                            "    A<A<BLA>>   gna1;\n"
                            "    A<BLA>      gna2;\n"
                            "}\n";
        tok(code); // Don't crash or freeze
    }

    void template39() { // #4742 - Used to freeze in 1.60
        const char code[] = "template<typename T> struct vector {"
                            "  operator T() const;"
                            "};"
                            "void f() {"
                            "  vector<vector<int>> v;"
                            "  const vector<int> vi = static_cast<vector<int>>(v);"
                            "}";
        tok(code);
    }

    void template40() { // #5055 - false negatives when there is template specialization outside struct
        const char code[] = "struct A {"
                            "  template<typename T> struct X { T t; };"
                            "};"
                            "template<> struct A::X<int> { int *t; };";
        ASSERT_EQUALS("struct A { template < typename T > struct X { T t ; } ; } ;", tok(code));
    }

    void template41() { // #4710 - const in template instantiation not handled perfectly
        const char code1[] = "template<class T> struct X { };\n"
                             "void f(const X<int> x) { }";
        ASSERT_EQUALS("void f ( const X<int> x ) { } struct X<int> { } ;", tok(code1));

        const char code2[] = "template<class T> T f(T t) { return t; }\n"
                             "int x() { return f<int>(123); }";
        ASSERT_EQUALS("int x ( ) { return f<int> ( 123 ) ; } int f<int> ( int t ) { return t ; }", tok(code2));
    }

    void template42() { // #4878 cpcheck aborts in ext-blocks.cpp (clang testcode)
        const char code[] = "template<typename ...Args>\n"
                            "int f0(Args ...args) {\n"
                            "  return ^ {\n"
                            "    return sizeof...(Args);\n"
                            "  }() + ^ {\n"
                            "    return sizeof...(args);\n"
                            "  }();\n"
                            "}";
        tok(code);
    }

    void template43() { // #5097 - Assert due to '>>' in 'B<A<C>>' not being treated as end of template instantation
        const char code[] = "template <typename T> struct C { };"
                            "template <typename T> struct D { static int f() { return C<T>::f(); } };"
                            "template <typename T> inline int f2() { return D<T>::f(); }"
                            "template <typename T> int f1(int x, T *) { int id = f2<T>(); return id; }"
                            "template <> struct C < B < A >> {"
                            "  static int f() {"
                            "    return f1 < B < A >> (0, reinterpret_cast< B<A> *>(E<void *>::Int(-1)));"
                            "  }"
                            "};";
        tok(code); // Don't assert
    }

    void template44() { // #5297
        tok("template<class T> struct StackContainer {"
            "  void foo(int i) {"
            "    if (0 >= 1 && i<0) {}"
            "  }"
            "};"
            "template<class T> class ZContainer : public StackContainer<T> {};"
            "struct FGSTensor {};"
            "class FoldedZContainer : public ZContainer<FGSTensor> {};");
    }

    void template45() { // #5814
        tok("namespace Constants { const int fourtytwo = 42; } "
            "template <class T, int U> struct TypeMath { "
            "  static const int mult = sizeof(T) * U; "
            "}; "
            "template <class T> struct FOO { "
            "  enum { value = TypeMath<T, Constants::fourtytwo>::something }; "
            "};");
        ASSERT_EQUALS("", errout.str());
    }

    void template46() { // #5816
        tok("template<class T, class U> struct A { static const int value = 0; }; "
            "template <class T> struct B { "
            "  enum { value = A<typename T::type, int>::value }; "
            "};");
        ASSERT_EQUALS("", errout.str());
        tok("template <class T, class U> struct A {}; "
            "enum { e = sizeof(A<int, int>) }; "
            "template <class T, class U> struct B {};");
        ASSERT_EQUALS("", errout.str());
        tok("template<class T, class U> struct A { static const int value = 0; }; "
            "template<class T> struct B { typedef int type; }; "
            "template <class T> struct C { "
            "  enum { value = A<typename B<T>::type, int>::value }; "
            "};");
        ASSERT_EQUALS("", errout.str());
    }

    void template47() { // #6023
        tok("template <typename T1, typename T2 = T3<T1> > class C1 {}; "
            "class C2 : public C1<C2> {};");
        ASSERT_EQUALS("", errout.str());
    }

    void template48() { // #6134
        tok("template <int> int f( {  } ); "
            "int foo = f<1>(0);");
        ASSERT_EQUALS("", errout.str());
    }

    void template49() { // #6237
        const char code[] = "template <class T> class Fred { void f(); void g(); };\n"
                            "template <class T> void Fred<T>::f() { }\n"
                            "template <class T> void Fred<T>::g() { }\n"
                            "template void Fred<float>::f();\n"
                            "template void Fred<int>::g();\n";

        const char expected[] = "template void Fred<float> :: f ( ) ; "
                                "template void Fred<int> :: g ( ) ; "
                                "class Fred<float> { void f ( ) ; void g ( ) ; } ; "
                                "void Fred<float> :: f ( ) { } "
                                "void Fred<float> :: g ( ) { } "
                                "class Fred<int> { void f ( ) ; void g ( ) ; } ; "
                                "void Fred<int> :: f ( ) { } "
                                "void Fred<int> :: g ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template50() { // #4272
        const char code[] = "template <class T> class Fred { void f(); };\n"
                            "template <class T> void Fred<T>::f() { }\n"
                            "template<> void Fred<float>::f() { }\n"
                            "template<> void Fred<int>::g() { }\n";

        const char expected[] = "template < class T > class Fred { void f ( ) ; } ; "
                                "template < class T > void Fred < T > :: f ( ) { } "
                                "template < > void Fred < float > :: f ( ) { } "
                                "template < > void Fred < int > :: g ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template51() { // #6172
        tok("template<int N, int ... M> struct A { "
            "  static void foo() { "
            "    int i = N; "
            "  } "
            "}; "
            "void bar() { "
            "  A<0>::foo(); "
            "}");
    }

    void template52() { // #6437
        tok("template <int value> int sum() { "
            "  return value + sum<value/2>(); "
            "} "
            "template<int x, int y> int calculate_value() { "
            "  return sum<x - y>(); "
            "} "
            "int value = calculate_value<1,1>();");
    }

    void template53() { // #4335
        tok("template<int N> struct Factorial { "
            "  enum { value = N * Factorial<N - 1>::value }; "
            "};"
            "template <> struct Factorial<0> { "
            "  enum { value = 1 }; "
            "};"
            "const int x = Factorial<4>::value;", /*simplify=*/true, /*debugwarnings=*/true);
        ASSERT_EQUALS("", errout.str());
    }

    void template54() { // #6587
        tok("template<typename _Tp> _Tp* fn(); "
            "template <class T> struct A { "
            "  template <class U, class S = decltype(fn<T>())> "
            "  struct B { }; "
            "}; "
            "A<int> a;");
    }

    void template55() { // #6604
        // Avoid constconstconst in macro instantiations
        ASSERT_EQUALS(
            "template < class T > class AtSmartPtr : public ConstCastHelper < AtSmartPtr < const T > , T > { "
            "friend struct ConstCastHelper < AtSmartPtr < const T > , T > ; "
            "AtSmartPtr ( const AtSmartPtr < T > & r ) ; "
            "} ;",
            tok("template<class T> class AtSmartPtr : public ConstCastHelper<AtSmartPtr<const T>, T>\n"
                "{\n"
                "    friend struct ConstCastHelper<AtSmartPtr<const T>, T>;\n"
                "    AtSmartPtr(const AtSmartPtr<T>& r);\n"
                "};"));

        // Similar problem can also happen with ...
        ASSERT_EQUALS(
            "A<int> a ( 0 ) ; struct A<int> { "
            "A<int> ( int * p ) { p ; } "
            "} ; "
            "struct A<int...> { "
            "A<int...> ( int * p ) { "
            "p ; "
            "} } ;",
            tok("template <typename... T> struct A\n"
                "{\n"
                "    A(T* p) {\n"
                "        (A<T...>*)(p);\n"
                "    }\n"
                "};\n"
                "A<int> a(0);"));
    }

    void template56() { // #7117
        tok("template<bool B> struct Foo { "
            "  std::array<int, B ? 1 : 2> mfoo; "
            "}; "
            "void foo() { "
            "  Foo<true> myFoo; "
            "}", /*simplify=*/true, /*debugwarnings=*/true);
        ASSERT_EQUALS("", errout.str());
    }

    void template57() { // #7891
        const char code[] = "template<class T> struct Test { Test(T); };\n"
                            "Test<unsigned long> test( 0 );";
        const char exp [] = "Test<unsignedlong> test ( 0 ) ; "
                            "struct Test<unsignedlong> { Test<unsignedlong> ( long ) ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template58() { // #6021
        const char code[] = "template <typename A>\n"
                            "void TestArithmetic() {\n"
                            "  x(1 * CheckedNumeric<A>());\n"
                            "}\n"
                            "void foo() {\n"
                            "  TestArithmetic<int>();\n"
                            "}";
        const char exp[] = "void foo ( ) {"
                           " TestArithmetic<int> ( ) ; "
                           "} "
                           "void TestArithmetic<int> ( ) {"
                           " x ( CheckedNumeric < int > ( ) ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template59() { // #8051
        const char code[] = "template<int N>\n"
                            "struct Factorial {\n"
                            "    enum FacHelper { value = N * Factorial<N - 1>::value };\n"
                            "};\n"
                            "template <>\n"
                            "struct Factorial<0> {\n"
                            "    enum FacHelper { value = 1 };\n"
                            "};\n"
                            "template<int DiagonalDegree>\n"
                            "int diagonalGroupTest() {\n"
                            "    return Factorial<DiagonalDegree>::value;\n"
                            "}\n"
                            "int main () {\n"
                            "    return diagonalGroupTest<4>();\n"
                            "}";
        const char exp[] = "struct Factorial<0> { enum FacHelper { value = 1 } ; } ; "
                           "int main ( ) { return diagonalGroupTest<4> ( ) ; } "
                           "int diagonalGroupTest<4> ( ) { return Factorial<4> :: value ; } "
                           "struct Factorial<4> { enum FacHelper { value = 4 * Factorial<3> :: value } ; } ; "
                           "struct Factorial<3> { enum FacHelper { value = 3 * Factorial<2> :: value } ; } ; "
                           "struct Factorial<2> { enum FacHelper { value = 2 * Factorial<1> :: value } ; } ; "
                           "struct Factorial<1> { enum FacHelper { value = Factorial<0> :: value } ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template60() { // Extracted from Clang testfile
        const char code[] = "template <typename T> struct S { typedef int type; };\n"
                            "template <typename T> void f() {}\n"
                            "template <typename T> void h() { f<typename S<T>::type(0)>(); }\n"
                            "\n"
                            "void j() { h<int>(); }";
        const char exp[] = "template < typename T > void f ( ) { } " // <- TODO: This template is not expanded
                           "void j ( ) { h<int> ( ) ; } "
                           "void h<int> ( ) { f < S<int> :: type ( 0 ) > ( ) ; } "
                           "struct S<int> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template61() { // hang in daca, code extracted from kodi
        const char code[] = "template <typename T> struct Foo {};\n"
                            "template <typename T> struct Bar {\n"
                            "  void f1(Bar<T> x) {}\n"
                            "  Foo<Bar<T>> f2() { }\n"
                            "};\n"
                            "Bar<int> c;";
        const char exp[] = "struct Bar<int> ; "
                           "Bar<int> c ; "
                           "struct Bar<int> {"
                           " void f1 ( Bar<int> x ) { }"
                           " Foo<Bar<int>> f2 ( ) { } "
                           "} ; "
                           "struct Foo<Bar<int>> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template62() { // #8314
        const char code[] = "template <class T> struct C1 {};\n"
                            "template <class T> void f() { x = y ? C1<int>::allocate(1) : 0; }\n"
                            "template <class T, unsigned S> class C3 {};\n"
                            "template <class T, unsigned S> C3<T, S>::C3(const C3<T, S> &v) { C1<T *> c1; }\n"
                            "C3<int,6> c3;";
        const char exp[] = "template < class T > void f ( ) { x = y ? ( C1 < int > :: allocate ( 1 ) ) : 0 ; } "
                           "class C3<int,6> ; "
                           "C3<int,6> c3 ; "
                           "class C3<int,6> { } ; "
                           "C3<int,6> :: C3<int,6> ( const C3<int,6> & v ) { C1<int*> c1 ; } "
                           "struct C1<int*> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template63() { // #8576
        const char code[] = "template<class T> struct TestClass { T m_hi; }; TestClass<std::auto_ptr<v>> objTest3;";
        const char exp[] = "TestClass<std::auto_ptr<v>> objTest3 ; struct TestClass<std::auto_ptr<v>> { std :: auto_ptr < v > m_hi ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template64() { // #8683
        const char code[] = "template <typename T>\n"
                            "bool foo(){return true;}\n"
                            "struct A {\n"
                            "template<int n>\n"
                            "void t_func()\n"
                            "{\n"
                            "     if( n != 0 || foo<int>());\n"
                            "}\n"
                            "void t_caller()\n"
                            "{\n"
                            "    t_func<0>();\n"
                            "    t_func<1>();\n"
                            "}\n"
                            "};";
        tok(code); // don't crash
    }

    void template65() { // #8321
        const char code[] = "namespace bpp\n"
                            "{\n"
                            "template<class N, class E, class DAGraphImpl>\n"
                            "class AssociationDAGraphImplObserver :\n"
                            "  public AssociationGraphImplObserver<N, E, DAGraphImpl>\n"
                            "{};\n"
                            "template<class N, class E>\n"
                            "using AssociationDAGlobalGraphObserver =  AssociationDAGraphImplObserver<N, E, DAGlobalGraph>;\n"
                            "}\n"
                            "using namespace bpp;\n"
                            "using namespace std;\n"
                            "int main() {\n"
                            "  AssociationDAGlobalGraphObserver<string,unsigned int> grObs;\n"
                            " return 1;\n"
                            "}";
        tok(code); // don't crash
    }

    void template66() { // #8725
        const char code[] = "template <class T> struct Fred {\n"
                            "    const int ** foo();\n"
                            "};\n"
                            "template <class T> const int ** Fred<T>::foo() { return nullptr; }\n"
                            "Fred<int> fred;";
        const char exp [] = "Fred<int> fred ; struct Fred<int> { "
                            "const int * * foo ( ) ; "
                            "} ; "
                            "const int * * Fred<int> :: foo ( ) { return nullptr ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template67() { // ticket #8122
        const char code[] = "template <class T> struct Containter {\n"
                            "  Containter();\n"
                            "  Containter(const Containter &);\n"
                            "  Containter & operator = (const Containter &);\n"
                            "  ~Containter();\n"
                            "  T* mElements;\n"
                            "  const Containter * c;\n"
                            "};\n"
                            "template <class T> Containter<T>::Containter() : mElements(nullptr), c(nullptr) {}\n"
                            "template <class T> Containter<T>::Containter(const Containter & x) { nElements = x.nElements; c = x.c; }\n"
                            "template <class T> Containter<T> & Containter<T>::operator = (const Containter & x) { mElements = x.mElements; c = x.c; return *this; }\n"
                            "template <class T> Containter<T>::~Containter() {}\n"
                            "Containter<int> intContainer;";

        const char expected[] = "Containter<int> intContainer ; "
                                "struct Containter<int> { "
                                "Containter<int> ( ) ; "
                                "Containter<int> ( const Containter<int> & ) ; "
                                "Containter<int> & operator= ( const Containter<int> & ) ; "
                                "~ Containter<int> ( ) ; "
                                "int * mElements ; "
                                "const Containter<int> * c ; "
                                "} ; "
                                "Containter<int> :: Containter<int> ( ) : mElements ( nullptr ) , c ( nullptr ) { } "
                                "Containter<int> :: Containter<int> ( const Containter<int> & x ) { nElements = x . nElements ; c = x . c ; } "
                                "Containter<int> & Containter<int> :: operator= ( const Containter<int> & x ) { mElements = x . mElements ; c = x . c ; return * this ; } "
                                "Containter<int> :: ~ Containter<int> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template68() {
        const char code[] = "template <class T> union Fred {\n"
                            "    char dummy[sizeof(T)];\n"
                            "    T value;\n"
                            "};\n"
                            "Fred<int> fred;";
        const char exp [] = "Fred<int> fred ; union Fred<int> { "
                            "char dummy [ 4 ] ; "
                            "int value ; "
                            "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template69() { // #8791
        const char code[] = "class Test {\n"
                            "    int test;\n"
                            "    template <class T> T lookup() { return test; }\n"
                            "    int Fun() { return lookup<int>(); }\n"
                            "};";
        const char exp [] = "class Test { "
                            "int test ; "
                            "int Fun ( ) { return lookup<int> ( ) ; } "
                            "} ; "
                            "int Test :: lookup<int> ( ) { return test ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template70() { // #5289
        const char code[] = "template<typename T, typename V, int KeySize = 0> class Bar;\n"
                            "template<>\n"
                            "class Bar<void, void> {\n"
                            "};\n"
                            "template<typename K, typename V, int KeySize>\n"
                            "class Bar : private Bar<void, void> {\n"
                            "   void foo() { }\n"
                            "};";
        const char exp [] = "template < typename T , typename V , int KeySize = 0 > class Bar ; "
                            "class Bar<void,void> { "
                            "} ; "
                            "template < typename K , typename V , int KeySize = 0 > "
                            "class Bar : private Bar<void,void> { "
                            "void foo ( ) { } "
                            "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template_specialization_1() {  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        const char code[] = "template <typename T> struct C {};\n"
                            "template <typename T> struct S {a};\n"
                            "template <typename T> struct S<C<T>> {b};\n"
                            "S<int> s;";
        const char exp[]  = "template < typename T > struct C { } ; struct S<int> ; template < typename T > struct S < C < T > > { b } ; S<int> s ; struct S<int> { a } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template_specialization_2() {  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        const char code[] = "template <typename T> struct C {};\n"
                            "template <typename T> struct S {a};\n"
                            "template <typename T> struct S<C<T>> {b};\n"
                            "S<C<int>> s;";
        const char exp[]  = "template < typename T > struct C { } ; template < typename T > struct S { a } ; struct S<C<int>> ; S<C<int>> s ; struct S<C<int>> { b } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template_enum() {
        const char code1[] = "template <class T>\n"
                             "struct Unconst {\n"
                             "    typedef T type;\n"
                             "};\n"
                             "template <class T>\n"
                             "struct Unconst<const T> {\n"
                             "    typedef T type;\n"
                             "};\n"
                             "template <class T>\n"
                             "struct Unconst<const T&> {\n"
                             "    typedef T& type;\n"
                             "};\n"
                             "template <class T>\n"
                             "struct Unconst<T* const> {\n"
                             "    typedef T* type;\n"
                             "};\n"
                             "template <class T1, class T2>\n"
                             "struct type_equal {\n"
                             "    enum {  value = 0   };\n"
                             "};\n"
                             "template <class T>\n"
                             "struct type_equal<T, T> {\n"
                             "    enum {  value = 1   };\n"
                             "};\n"
                             "template<class T>\n"
                             "struct template_is_const\n"
                             "{\n"
                             "    enum {value = !type_equal<T, typename Unconst<T>::type>::value  };\n"
                             "};";
        const char exp1[] = "template < class T > struct Unconst { } ; "
                            "template < class T > struct Unconst < const T > { } ; "
                            "template < class T > struct Unconst < const T & > { } ; "
                            "template < class T > struct Unconst < T * const > { } ; "
                            "template < class T1 , class T2 > struct type_equal { enum Anonymous0 { value = 0 } ; } ; "
                            "template < class T > struct type_equal < T , T > { enum Anonymous1 { value = 1 } ; } ; "
                            "template < class T > struct template_is_const { enum Anonymous2 { value = ! type_equal < T , Unconst < T > :: type > :: value } ; } ;";
        ASSERT_EQUALS(exp1, tok(code1));
    }

    void template_default_parameter() {
        {
            const char code[] = "template <class T, int n=3>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";

            // The expected result..
            const char expected[] = "void f ( ) "
                                    "{"
                                    " A<int,2> a1 ;"
                                    " A<int,3> a2 ; "
                                    "} "
                                    "class A<int,2> "
                                    "{ int ar [ 2 ] ; } ; "
                                    "class A<int,3> "
                                    "{ int ar [ 3 ] ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <class T, int n1=3, int n2=2>\n"
                                "class A\n"
                                "{ T ar[n1+n2]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int> a1;\n"
                                "    A<int,3> a2;\n"
                                "}\n";

            // The expected result..
            const char expected[] = "void f ( ) "
                                    "{"
                                    " A<int,3,2> a1 ;"
                                    " A<int,3,2> a2 ; "
                                    "} "
                                    "class A<int,3,2> "
                                    "{ int ar [ 5 ] ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <class T, int n=3>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,(int)2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";

            const char wanted[] = "template < class T , int n >"
                                  " class A"
                                  " { T ar [ n ] ; } ;"
                                  " void f ( )"
                                  " {"
                                  " A<int,(int)2> a1 ;"
                                  " A<int,3> a2 ;"
                                  " }"
                                  " class A<int,2>"
                                  " { int ar [ 2 ] ; }"
                                  " class A<int,3>"
                                  " { int ar [ 3 ] ; }";

            const char current[] = "void f ( ) "
                                   "{ "
                                   "A < int , ( int ) 2 > a1 ; "
                                   "A<int,3> a2 ; "
                                   "} "
                                   "class A<int,3> "
                                   "{ int ar [ 3 ] ; } ;";
            TODO_ASSERT_EQUALS(wanted, current, tok(code));
        }
        {
            const char code[] = "class A { }; "
                                "template<class T> class B { }; "
                                "template<class T1, class T2 = B<T1>> class C { }; "
                                "template<class T1 = A, typename T2 = B<A>> class D { };";
            ASSERT_EQUALS("class A { } ; "
                          "template < class T > class B { } ; "
                          "template < class T1 , class T2 = B < T1 > > class C { } ; "
                          "template < class T1 = A , typename T2 = B < A > > class D { } ;", tok(code));
        }
        {
            // #7548
            const char code[] = "template<class T, class U> class DefaultMemory {}; "
                                "template<class Key, class Val, class Mem=DefaultMemory<Key,Val> > class thv_table_c  {}; "
                                "thv_table_c<void *,void *> id_table_m;";
            const char exp [] = "template < class T , class U > class DefaultMemory { } ; "
                                "thv_table_c<void * , void * , DefaultMemory < void * , void *>> id_table_m ; "
                                "class thv_table_c<void * , void * , DefaultMemory < void * , void * >> { } ;";
            const char curr[] = "template < class T , class U > class DefaultMemory { } ; "
                                "class thv_table_c<void*,void*,DefaultMemory<Key,Val>> ; "
                                "thv_table_c<void*,void*,DefaultMemory<Key,Val>> id_table_m ; "
                                "class thv_table_c<void*,void*,DefaultMemory<Key,Val>> { } ;";
            TODO_ASSERT_EQUALS(exp, curr, tok(code));
        }
    }

    void template_forward_declared_default_parameter() {
        {
            const char code[] = "template <class T, int n=3> class A;\n"
                                "template <class T, int n>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";

            const char wanted[] = "class A<int,2> ; "
                                  "class A<int,3> ; "
                                  "void f ( ) "
                                  "{"
                                  " A<int,2> a1 ;"
                                  " A<int,3> a2 ; "
                                  "} "
                                  "class A<int,2> "
                                  "{ int ar [ 2 ] ; } ; "
                                  "class A<int,3> "
                                  "{ int ar [ 3 ] ; } ;";
            const char current[] = "template < class T , int n = 3 > class A ; "
                                   "class A<int,2> ; "
                                   "class A<int,3> ; "
                                   "void f ( ) "
                                   "{"
                                   " A<int,2> a1 ;"
                                   " A<int,3> a2 ; "
                                   "} "
                                   "class A<int,2> "
                                   "{ int ar [ 2 ] ; } ; "
                                   "class A<int,3> "
                                   "{ int ar [ 3 ] ; } ;";
            TODO_ASSERT_EQUALS(wanted, current, tok(code));
        }
        {
            const char code[] = "template <class, int = 3> class A;\n"
                                "template <class T, int n>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";

            const char wanted[] = "class A<int,2> ; "
                                  "class A<int,3> ; "
                                  "void f ( ) "
                                  "{"
                                  " A<int,2> a1 ;"
                                  " A<int,3> a2 ; "
                                  "} "
                                  "class A<int,2> "
                                  "{ int ar [ 2 ] ; } ; "
                                  "class A<int,3> "
                                  "{ int ar [ 3 ] ; } ;";
            const char current[] = "template < class , int = 3 > class A ; "
                                   "class A<int,2> ; "
                                   "class A<int,3> ; "
                                   "void f ( ) "
                                   "{"
                                   " A<int,2> a1 ;"
                                   " A<int,3> a2 ; "
                                   "} "
                                   "class A<int,2> "
                                   "{ int ar [ 2 ] ; } ; "
                                   "class A<int,3> "
                                   "{ int ar [ 3 ] ; } ;";
            TODO_ASSERT_EQUALS(wanted, current, tok(code));
        }
    }

    void template_default_type() {
        const char code[] = "template <typename T, typename U=T>\n"
                            "class A\n"
                            "{\n"
                            "public:\n"
                            "  void foo() {\n"
                            "    int a;\n"
                            "    a = static_cast<U>(a);\n"
                            "  }\n"
                            "};\n"
                            "\n"
                            "template <typename T>\n"
                            "class B\n"
                            "{\n"
                            "protected:\n"
                            "  A<int> a;\n"
                            "};\n"
                            "\n"
                            "class C\n"
                            "  : public B<int>\n"
                            "{\n"
                            "};\n";

        tok(code);

        //ASSERT_EQUALS("[file1.cpp:15]: (error) Internal error: failed to instantiate template. The checking continues anyway.\n", errout.str());
        ASSERT_EQUALS("", errout.str());
    }

    void template_typename() {
        {
            const char code[] = "template <class T>\n"
                                "void foo(typename T::t *)\n"
                                "{ }";

            // The expected result..
            const char expected[] = "template < class T > void foo ( T :: t * ) { }";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f() {\n"
                                "    x(sizeof typename);\n"
                                "    type = 0;\n"
                                "}";

            ASSERT_EQUALS("void f ( ) { x ( sizeof ( typename ) ) ; type = 0 ; }", tok(code));
        }
    }

    void template_constructor() {
        // #3152 - if template constructor is removed then there might be
        //         "no constructor" false positives
        const char code[] = "class Fred {\n"
                            "    template<class T> explicit Fred(T t) { }\n"
                            "}";
        ASSERT_EQUALS("class Fred { template < class T > explicit Fred ( T t ) { } }", tok(code));

        // #3532
        const char code2[] = "class Fred {\n"
                             "    template<class T> Fred(T t) { }\n"
                             "}";
        ASSERT_EQUALS("class Fred { template < class T > Fred ( T t ) { } }", tok(code2));
    }

    void syntax_error_templates_1() {
        // ok code.. using ">" for a comparison
        tok("x<y>z> xyz;\n");
        ASSERT_EQUALS("", errout.str());

        // ok code
        tok("template<class T> operator<(T a, T b) { }\n");
        ASSERT_EQUALS("", errout.str());

        // ok code (ticket #1984)
        tok("void f(a) int a;\n"
            "{ ;x<y; }");
        ASSERT_EQUALS("", errout.str());

        // ok code (ticket #1985)
        tok("void f()\n"
            "try { ;x<y; }");
        ASSERT_EQUALS("", errout.str());

        // ok code (ticket #3183)
        tok("MACRO(({ i < x }))");
        ASSERT_EQUALS("", errout.str());

        // bad code.. missing ">"
        ASSERT_THROW(tok("x<y<int> xyz;\n"), InternalError);

        // bad code
        ASSERT_THROW(tok("typedef\n"
                         "    typename boost::mpl::if_c<\n"
                         "          _visitableIndex < boost::mpl::size< typename _Visitables::ConcreteVisitables >::value\n"
                         "          , ConcreteVisitable\n"
                         "          , Dummy< _visitableIndex >\n"
                         "    >::type ConcreteVisitableOrDummy;\n"), InternalError);

        // code is ok, don't show syntax error
        tok("struct A {int a;int b};\n"
            "class Fred {"
            "public:\n"
            "    Fred() : a({1,2}) {\n"
            "        for (int i=0;i<6;i++);\n" // <- no syntax error
            "    }\n"
            "private:\n"
            "    A a;\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void template_member_ptr() { // Ticket #5786
        tok("struct A {}; "
            "struct B { "
            "template <void (A::*)() const> struct BB {}; "
            "template <bool BT> static bool foo(int) { return true; } "
            "void bar() { bool b = foo<true>(0); }"
            "};");
        tok("struct A {}; "
            "struct B { "
            "template <void (A::*)() volatile> struct BB {}; "
            "template <bool BT> static bool foo(int) { return true; } "
            "void bar() { bool b = foo<true>(0); }"
            "};");
        tok("struct A {}; "
            "struct B { "
            "template <void (A::*)() const volatile> struct BB {}; "
            "template <bool BT> static bool foo(int) { return true; } "
            "void bar() { bool b = foo<true>(0); }"
            "};");
        tok("struct A {}; "
            "struct B { "
            "template <void (A::*)() volatile const> struct BB {}; "
            "template <bool BT> static bool foo(int) { return true; } "
            "void bar() { bool b = foo<true>(0); }"
            "};");
    }

    void template_namespace_1() {
        // #6570
        const char code[] = "namespace {\n"
                            "  template<class T> void Fred(T value) { }\n"
                            "}\n"
                            "Fred<int>(123);";
        ASSERT_EQUALS("namespace { } "
                      "Fred<int> ( 123 ) ; "
                      "void Fred<int> ( int value ) { }", tok(code));
    }

    void template_namespace_2() {
        // #8283
        const char code[] = "namespace X {\n"
                            "  template<class T> struct S { };\n"
                            "}\n"
                            "X::S<int> s;";
        ASSERT_EQUALS("namespace X { "
                      "struct S<int> ; "
                      "} "
                      "X :: S<int> s ; "
                      "struct X :: S<int> { } ;", tok(code));
    }

    void template_namespace_3() {
        const char code[] = "namespace test16 {\n"
                            "  template <class T> struct foo {\n"
                            "    static void *bar();\n"
                            "  };\n"
                            "  void *test() { return foo<int>::bar(); }\n"
                            "}";
        ASSERT_EQUALS("namespace test16 {"
                      " struct foo<int> ;"
                      " void * test ( ) {"
                      " return foo<int> :: bar ( ) ;"
                      " } "
                      "} "
                      "struct test16 :: foo<int> {"
                      " static void * bar ( ) ; "
                      "} ;", tok(code));
    }

    void template_namespace_4() {
        const char code[] = "namespace foo {\n"
                            "  template<class T> class A { void dostuff() {} };\n"
                            "  struct S : public A<int> {\n"
                            "    void f() {\n"
                            "      A<int>::dostuff();\n"
                            "    }\n"
                            "  };\n"
                            "}";
        ASSERT_EQUALS("namespace foo {"
                      " class A<int> ;"
                      " struct S : public A<int> {"
                      " void f ( ) {"
                      " A<int> :: dostuff ( ) ;"
                      " }"
                      " } ; "
                      "} "
                      "class foo :: A<int> { void dostuff ( ) { } } ;", tok(code));
    }

    void template_namespace_5() {
        const char code[] = "template<class C> struct S {};\n"
                            "namespace X { S<int> s; }";
        ASSERT_EQUALS("namespace X { S<int> s ; } struct S<int> { } ;", tok(code));
    }

    void template_namespace_6() {
        const char code[] = "namespace NS {\n"
                            "template <typename T> union C {\n"
                            "  char dummy[sizeof(T)];\n"
                            "  T value;\n"
                            "  C();\n"
                            "  ~C();\n"
                            "  C(const C &);\n"
                            "  C & operator = (const C &);\n"
                            "};\n"
                            "}\n"
                            "NS::C<int> intC;\n"
                            "template <typename T> NS::C<T>::C() {}\n"
                            "template <typename T> NS::C<T>::~C() {}\n"
                            "template <typename T> NS::C<T>::C(const NS::C<T> &) {}\n"
                            "template <typename T> NS::C<T> & NS::C<T>::operator=(const NS::C<T> &) {}";
        ASSERT_EQUALS("namespace NS { "
                      "union C<int> ; "
                      "} "
                      "NS :: C<int> intC ; union NS :: C<int> { "
                      "char dummy [ 4 ] ; "
                      "int value ; "
                      "C<int> ( ) ; "
                      "~ C<int> ( ) ; "
                      "C<int> ( const NS :: C<int> & ) ; "
                      "NS :: C<int> & operator= ( const NS :: C<int> & ) ; "
                      "} ; "
                      "NS :: C<int> :: C<int> ( ) { } "
                      "NS :: C<int> :: ~ C<int> ( ) { } "
                      "NS :: C<int> :: C<int> ( const NS :: C<int> & ) { } "
                      "NS :: C<int> & NS :: C<int> :: operator= ( const NS :: C<int> & ) { }", tok(code));
    }

    void template_namespace_7() { // #8768
        const char code[] = "namespace N1 {\n"
                            "namespace N2 {\n"
                            "    struct C { };\n"
                            "    template <class T> struct CT { };\n"
                            "    C c1;\n"
                            "    CT<int> ct1;\n"
                            "}\n"
                            "N2::C c2;\n"
                            "N2::CT<int> ct2;\n"
                            "}\n"
                            "N1::N2::C c3;\n"
                            "N1::N2::CT<int> ct3;";
        ASSERT_EQUALS("namespace N1 { "
                      "namespace N2 { "
                      "struct C { } ; "
                      "struct CT<int> ; "
                      "C c1 ; "
                      "CT<int> ct1 ; "
                      "} "
                      "N2 :: C c2 ; "
                      "N2 :: CT<int> ct2 ; "
                      "} "
                      "N1 :: N2 :: C c3 ; "
                      "N1 :: N2 :: CT<int> ct3 ; struct N1 :: N2 :: CT<int> { } ;", tok(code));
    }

    unsigned int templateParameters(const char code[]) {
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp", "");

        return TemplateSimplifier::templateParameters(tokenizer.tokens()->next());
    }

    void templateParameters() {
        // Test that the function TemplateSimplifier::templateParameters works
        ASSERT_EQUALS(1U, templateParameters("X<struct C> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<union C> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<const int> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<int const *> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<const struct C> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<len>>x;"));
        ASSERT_EQUALS(1U, templateParameters("X<typename> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<...> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<class T...> x;")); // Invalid syntax
        ASSERT_EQUALS(1U, templateParameters("X<class... T> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<class, typename T...> x;")); // Invalid syntax
        ASSERT_EQUALS(2U, templateParameters("X<class, typename... T> x;"));
        ASSERT_EQUALS(2U, templateParameters("X<int(&)(), class> x;"));
        ASSERT_EQUALS(3U, templateParameters("X<char, int(*)(), bool> x;"));
        TODO_ASSERT_EQUALS(1U, 0U, templateParameters("X<int...> x;")); // Mishandled valid syntax
        TODO_ASSERT_EQUALS(2U, 0U, templateParameters("X<class, typename...> x;")); // Mishandled valid syntax
        ASSERT_EQUALS(2U, templateParameters("X<1, T> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<i == 0> x;"));
        ASSERT_EQUALS(2U, templateParameters("X<int, i>=0> x;"));
        ASSERT_EQUALS(3U, templateParameters("X<int, i>=0, i - 2> x;"));
    }

    // Helper function to unit test TemplateSimplifier::getTemplateNamePosition
    int templateNamePositionHelper(const char code[], unsigned offset = 0, bool onlyCreateTokens = false) {
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        if (onlyCreateTokens)
            tokenizer.createTokens(istr, "test.cpp");
        else
            tokenizer.tokenize(istr, "test.cpp", emptyString);

        const Token *_tok = tokenizer.tokens();
        for (unsigned i = 0 ; i < offset ; ++i)
            _tok = _tok->next();
        return TemplateSimplifier::getTemplateNamePosition(_tok);
    }

    void templateNamePosition() {
        // Template class
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> class A {};", 4));
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> struct A {};", 4));
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> class A : B {};", 4));
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> struct A : B {};", 4));
        // Template function definitions
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> unsigned foo() { return 0; }", 4));
        ASSERT_EQUALS(3, templateNamePositionHelper("template<class T> unsigned* foo() { return 0; }", 4));
        ASSERT_EQUALS(3, templateNamePositionHelper("template<class T> const unsigned foo() { return 0; }", 4));
        ASSERT_EQUALS(4, templateNamePositionHelper("template<class T> const unsigned& foo() { return 0; }", 4));
        // Class template members
        ASSERT_EQUALS(4, templateNamePositionHelper("class A { template<class T> unsigned foo(); }; "
                      "template<class T> unsigned A::foo() { return 0; }", 19));
        ASSERT_EQUALS(5, templateNamePositionHelper("class A { template<class T> const unsigned foo(); }; "
                      "template<class T> const unsigned A::foo() { return 0; }", 20));
        TODO_ASSERT_EQUALS(7, -1, templateNamePositionHelper("class A { class B { template<class T> const unsigned foo(); }; } ; "
                           "template<class T> const unsigned A::B::foo() { return 0; }", 25));
        // Template class member
        ASSERT_EQUALS(6, templateNamePositionHelper("template<class T> class A { A(); }; "
                      "template<class T> A<T>::A() {}", 18));
        ASSERT_EQUALS(8, templateNamePositionHelper("template<class T, class U> class A { A(); }; "
                      "template<class T, class U> A<T, U>::A() {}", 24));
        ASSERT_EQUALS(7, templateNamePositionHelper("template<class T> class A { unsigned foo(); }; "
                      "template<class T> unsigned A<T>::foo() { return 0; }", 19));
        ASSERT_EQUALS(9, templateNamePositionHelper("template<class T, class U> class A { unsigned foo(); }; "
                      "template<class T, class U> unsigned A<T, U>::foo() { return 0; }", 25));
        ASSERT_EQUALS(9, templateNamePositionHelper("template<class T, class U> class A { unsigned foo(); }; "
                      "template<class T, class U> unsigned A<T, U>::foo() { return 0; }", 25, /*onlyCreateTokens=*/true));
        ASSERT_EQUALS(12, templateNamePositionHelper("template<class T> class v {}; "
                      "template<class T, class U> class A { unsigned foo(); }; "
                      "template<> unsigned A<int, v<char> >::foo() { return 0; }", 30, /*onlyCreateTokens=*/true));
    }

    void expandSpecialized1() {
        ASSERT_EQUALS("class A<int> { } ;", tok("template<> class A<int> {};"));
        ASSERT_EQUALS("class A<int> : public B { } ;", tok("template<> class A<int> : public B {};"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) ; ~ A<int> ( ) ; } ;", tok("template<> class A<int> { A(); ~A(); };"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) { } ~ A<int> ( ) { } } ;", tok("template<> class A<int> { A() {} ~A() {} };"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) ; ~ A<int> ( ) ; } ; A<int> :: A<int> ( ) { } ~ A<int> :: A<int> ( ) { }",
                      tok("template<> class A<int> { A(); ~A(); }; A<int>::A() { } ~A<int>::A() {}"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) ; A<int> ( const A<int> & ) ; A<int> foo ( ) ; } ; A<int> :: A<int> ( ) { } A<int> :: A<int> ( const A<int> & ) { } A<int> A<int> :: foo ( ) { A<int> a ; return a ; }",
                      tok("template<> class A<int> { A(); A(const A &) ; A foo(); }; A<int>::A() { } A<int>::A(const A &) { } A<int> A<int>::foo() { A a; return a; }"));
    }

    void expandSpecialized2() {
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C() { }\n"
                                "   C(const C &) { }\n"
                                "   ~C() { }\n"
                                "   C & operator=(const C &) { return *this; }\n"
                                "};\n"
                                "C<float> b;\n";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) { } "
                                    "C<float> ( const C<float> & ) { } "
                                    "~ C<float> ( ) { } "
                                    "C<float> & operator= ( const C<float> & ) { return * this ; } "
                                    "} ; "
                                    "C<float> b ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C() { }\n"
                                "   C(const C &) { }\n"
                                "   ~C() { }\n"
                                "   C & operator=(const C &) { return *this; }\n"
                                "};";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) { } "
                                    "C<float> ( const C<float> & ) { } "
                                    "~ C<float> ( ) { } "
                                    "C<float> & operator= ( const C<float> & ) { return * this ; } "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C();\n"
                                "   C(const C &);\n"
                                "   ~C();\n"
                                "   C & operator=(const C &);\n"
                                "};\n"
                                "C::C() { }\n"
                                "C::C(const C &) { }\n"
                                "C::~C() { }\n"
                                "C & C::operator=(const C &) { return *this; }\n"
                                "C<float> b;\n";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) ; "
                                    "C<float> ( const C<float> & ) ; "
                                    "~ C<float> ( ) ; "
                                    "C<float> & operator= ( const C<float> & ) ; "
                                    "} ; "
                                    "C<float> :: C<float> ( ) { } "
                                    "C<float> :: C<float> ( const C<float> & ) { } "
                                    "C<float> :: ~ C<float> ( ) { } "
                                    "C<float> & C<float> :: operator= ( const C<float> & ) { return * this ; } "
                                    "C<float> b ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C();\n"
                                "   C(const C &);\n"
                                "   ~C();\n"
                                "   C & operator=(const C &);\n"
                                "};\n"
                                "C::C() { }\n"
                                "C::C(const C &) { }\n"
                                "C::~C() { }\n"
                                "C & C::operator=(const C &) { return *this; }";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) ; "
                                    "C<float> ( const C<float> & ) ; "
                                    "~ C<float> ( ) ; "
                                    "C<float> & operator= ( const C<float> & ) ; "
                                    "} ; "
                                    "C<float> :: C<float> ( ) { } "
                                    "C<float> :: C<float> ( const C<float> & ) { } "
                                    "C<float> :: ~ C<float> ( ) { } "
                                    "C<float> & C<float> :: operator= ( const C<float> & ) { return * this ; }";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void expandSpecialized3() { // #8671
        const char code[] = "template <> struct OutputU16<unsigned char> final {\n"
                            "    explicit OutputU16(std::basic_ostream<unsigned char> &t) : outputStream_(t) {}\n"
                            "    void operator()(unsigned short) const;\n"
                            "private:\n"
                            "    std::basic_ostream<unsigned char> &outputStream_;\n"
                            "};";
        const char expected[] = "struct OutputU16<unsignedchar> final { "
                                "explicit OutputU16<unsignedchar> ( std :: basic_ostream < char > & t ) : outputStream_ ( t ) { } "
                                "void operator() ( short ) const ; "
                                "private: "
                                "std :: basic_ostream < char > & outputStream_ ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void expandSpecialized4() {
        {
            const char code[] = "template<> class C<char> { };\n"
                                "map<int> m;";
            const char expected[] = "class C<char> { } ; "
                                    "map < int > m ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<> class C<char> { };\n"
                                "map<int> m;\n"
                                "C<char> c;";
            const char expected[] = "class C<char> { } ; "
                                    "map < int > m ; "
                                    "C<char> c ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<typename T> class C { };\n"
                                "template<> class C<char> { };\n"
                                "map<int> m;\n";
            const char expected[] = "template < typename T > class C { } ; "
                                    "class C<char> { } ; "
                                    "map < int > m ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<typename T> class C { };\n"
                                "template<> class C<char> { };\n"
                                "map<int> m;\n"
                                "C<int> i;";
            const char expected[] = "class C<char> { } ; "
                                    "map < int > m ; "
                                    "C<int> i ; "
                                    "class C<int> { } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<typename T> class C { };\n"
                                "template<> class C<char> { };\n"
                                "map<int> m;\n"
                                "C<int> i;\n"
                                "C<char> c;";
            const char expected[] = "class C<char> { } ; "
                                    "map < int > m ; "
                                    "C<int> i ; "
                                    "C<char> c ; "
                                    "class C<int> { } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void templateAlias1() {
        const char code[] = "template<class T, int N> struct Foo {};\n"
                            "template<class T> using Bar = Foo<T,3>;\n"
                            "Bar<int> b;\n";

        const char expected[] = "; Foo<int,3> b ; struct Foo<int,3> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void templateAlias2() {
        const char code[] = "namespace A { template<class T, int N> struct Foo {}; }\n"
                            "template<class T> using Bar = A::Foo<T,3>;\n"
                            "Bar<int> b;\n";

        const char expected[] = "namespace A { struct Foo<int,3> ; } "
                                "; "
                                "A :: Foo<int,3> b ; "
                                "struct A :: Foo<int,3> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void templateAlias3() { // #8315
        const char code[] = "template <int> struct Tag {};\n"
                            "template <int ID> using SPtr = std::shared_ptr<void(Tag<ID>)>;\n"
                            "SPtr<0> s;";
        const char expected[] = "; std :: shared_ptr < void ( Tag<0> ) > s ; struct Tag<0> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    unsigned int instantiateMatch(const char code[], const std::size_t numberOfArguments, const char patternAfter[]) {
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp", "");

        return TemplateSimplifier::instantiateMatch(tokenizer.tokens(), numberOfArguments, patternAfter);
    }

    void instantiateMatch() {
        // Ticket #8175
        ASSERT_EQUALS(false,
                      instantiateMatch("ConvertHelper < From, To > c ;",
                                       2, ":: %name% ("));
        ASSERT_EQUALS(true,
                      instantiateMatch("ConvertHelper < From, To > :: Create ( ) ;",
                                       2, ":: %name% ("));
        ASSERT_EQUALS(false,
                      instantiateMatch("integral_constant < bool, sizeof ( ConvertHelper < From, To > :: Create ( ) ) > ;",
                                       2, ":: %name% ("));
        ASSERT_EQUALS(false,
                      instantiateMatch("integral_constant < bool, sizeof ( ns :: ConvertHelper < From, To > :: Create ( ) ) > ;",
                                       2, ":: %name% ("));
    }

    void templateParameterWithoutName() {
        ASSERT_EQUALS(1U, templateParameters("template<typename = void> struct s;"));
        ASSERT_EQUALS(1U, templateParameters("template<template<typename = float> typename T> struct A {\n"
                                             "    void f();n"
                                             "    void g();\n"
                                             "};n"));
    }
};

REGISTER_TEST(TestSimplifyTemplate)
