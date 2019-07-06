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


#include "platform.h"
#include "settings.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

struct InternalError;


class TestSimplifyUsing : public TestFixture {
public:
    TestSimplifyUsing() : TestFixture("TestSimplifyUsing") {
    }


private:
    Settings settings0;
    Settings settings1;
    Settings settings2;

    void run() OVERRIDE {
        settings0.addEnabled("style");
        settings2.addEnabled("style");

        TEST_CASE(simplifyUsing1);
        TEST_CASE(simplifyUsing2);
        TEST_CASE(simplifyUsing3);
        TEST_CASE(simplifyUsing4);
        TEST_CASE(simplifyUsing5);
        TEST_CASE(simplifyUsing6);
        TEST_CASE(simplifyUsing7);
        TEST_CASE(simplifyUsing8);
        TEST_CASE(simplifyUsing9);
        TEST_CASE(simplifyUsing10);
        TEST_CASE(simplifyUsing11);
        TEST_CASE(simplifyUsing12);
        TEST_CASE(simplifyUsing13);
        TEST_CASE(simplifyUsing14);
        TEST_CASE(simplifyUsing15);
        TEST_CASE(simplifyUsing16);

        TEST_CASE(simplifyUsing8970);
        TEST_CASE(simplifyUsing8971);
        TEST_CASE(simplifyUsing8976);
        TEST_CASE(simplifyUsing9040);
        TEST_CASE(simplifyUsing9042);
        TEST_CASE(simplifyUsing9191);
    }

    std::string tok(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Native, bool debugwarnings = true) {
        errout.str("");

        settings0.inconclusive = true;
        settings0.debugwarnings = debugwarnings;
        settings0.platform(type);
        Tokenizer tokenizer(&settings0, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(nullptr, !simplify);
    }

    void simplifyUsing1() {
        const char code[] = "class A\n"
                            "{\n"
                            "public:\n"
                            " using duplicate = wchar_t;\n"
                            " void foo() {}\n"
                            "};\n"
                            "using duplicate = A;\n"
                            "int main()\n"
                            "{\n"
                            " duplicate a;\n"
                            " a.foo();\n"
                            " A::duplicate c = 0;\n"
                            "}";

        const char expected[] =
            "class A "
            "{ "
            "public: "
            ""
            "void foo ( ) { } "
            "} ; "
            "int main ( ) "
            "{ "
            "A a ; "
            "a . foo ( ) ; "
            "wchar_t c ; c = 0 ; "
            "}";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing2() {
        const char code[] = "class A;\n"
                            "using duplicate = A;\n"
                            "class A\n"
                            "{\n"
                            "public:\n"
                            "using duplicate = wchar_t;\n"
                            "duplicate foo() { wchar_t b; return b; }\n"
                            "};";

        const char expected[] =
            "class A ; "
            "class A "
            "{ "
            "public: "
            ""
            "wchar_t foo ( ) { wchar_t b ; return b ; } "
            "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing3() {
        const char code[] = "class A {};\n"
                            "using duplicate = A;\n"
                            "wchar_t foo()\n"
                            "{\n"
                            "using duplicate = wchar_t;\n"
                            "duplicate b;\n"
                            "return b;\n"
                            "}\n"
                            "int main()\n"
                            "{\n"
                            "duplicate b;\n"
                            "}";

        const char expected[] =
            "class A { } ; "
            "wchar_t foo ( ) "
            "{ "
            ""
            "wchar_t b ; "
            "return b ; "
            "} "
            "int main ( ) "
            "{ "
            "A b ; "
            "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing4() {
        const char code[] = "using s32 = int;\n"
                            "using u32 = unsigned int;\n"
                            "void f()\n"
                            "{\n"
                            "    s32 ivar = -2;\n"
                            "    u32 uvar = 2;\n"
                            "    return uvar / ivar;\n"
                            "}";

        const char expected[] =
            "void f ( ) "
            "{ "
            "int ivar ; ivar = -2 ; "
            "unsigned int uvar ; uvar = 2 ; "
            "return uvar / ivar ; "
            "}";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing5() {
        const char code[] =
            "using YY_BUFFER_STATE = struct yy_buffer_state *;\n"
            "void f()\n"
            "{\n"
            "    YY_BUFFER_STATE state;\n"
            "}";

        const char expected[] =
            "void f ( ) "
            "{ "
            "struct yy_buffer_state * state ; "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing6() {
        const char code[] =
            "namespace VL {\n"
            "    using float_t = float;\n"
            "    inline VL::float_t fast_atan2(VL::float_t y, VL::float_t x){}\n"
            "}";

        const char expected[] =
            "namespace VL { "
            ""
            "float fast_atan2 ( float y , float x ) { } "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing7() {
        const char code[] = "using abc = int; "
                            "Fred :: abc f ;";
        const char expected[] = "Fred :: abc f ;";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing8() {
        const char code[] = "using INT = int;\n"
                            "using UINT = unsigned int;\n"
                            "using PINT = int *;\n"
                            "using PUINT = unsigned int *;\n"
                            "using RINT = int &;\n"
                            "using RUINT = unsigned int &;\n"
                            "using RCINT = const int &;\n"
                            "using RCUINT = const unsigned int &;\n"
                            "INT ti;\n"
                            "UINT tui;\n"
                            "PINT tpi;\n"
                            "PUINT tpui;\n"
                            "RINT tri;\n"
                            "RUINT trui;\n"
                            "RCINT trci;\n"
                            "RCUINT trcui;";

        const char expected[] =
            "int ti ; "
            "unsigned int tui ; "
            "int * tpi ; "
            "unsigned int * tpui ; "
            "int & tri ; "
            "unsigned int & trui ; "
            "const int & trci ; "
            "const unsigned int & trcui ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing9() {
        const char code[] = "using S = struct s;\n"
                            "using PS = S *;\n"
                            "using T = struct t { int a; };\n"
                            "using TP = T *;\n"
                            "using U = struct { int a; };\n"
                            "using V = U *;\n"
                            "using W = struct { int a; } *;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;\n"
                            "W w;";

        const char expected[] =
            "struct t { int a ; } ; "
            "struct U { int a ; } ; "
            "struct Unnamed0 { int a ; } ; "
            "struct s s ; "
            "struct s * ps ; "
            "struct t t ; "
            "struct t * tp ; "
            "struct U u ; "
            "struct U * v ; "
            "struct Unnamed0 * w ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing10() {
        const char code[] = "using S = union s;\n"
                            "using PS = S *;\n"
                            "using T = union t { int a; float b ; };\n"
                            "using TP = T *;\n"
                            "using U = union { int a; float b; };\n"
                            "using V = U *;\n"
                            "using W = union { int a; float b; } *;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;\n"
                            "W w;";

        const char expected[] =
            "union t { int a ; float b ; } ; "
            "union U { int a ; float b ; } ; "
            "union Unnamed0 { int a ; float b ; } ; "
            "union s s ; "
            "union s * ps ; "
            "union t t ; "
            "union t * tp ; "
            "union U u ; "
            "union U * v ; "
            "union Unnamed0 * w ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing11() {
        const char code[] = "using abc = enum { a = 0 , b = 1 , c = 2 };\n"
                            "using XYZ = enum xyz { x = 0 , y = 1 , z = 2 };\n"
                            "abc e1;\n"
                            "XYZ e2;";

        const char expected[] = "enum abc { a = 0 , b = 1 , c = 2 } ; "
                                "enum xyz { x = 0 , y = 1 , z = 2 } ; "
                                "enum abc e1 ; "
                                "enum xyz e2 ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing12() {
        const char code[] = "using V1 = vector<int>;\n"
                            "using V2 = std::vector<int>;\n"
                            "using V3 = std::vector<std::vector<int> >;\n"
                            "using IntListIterator = std::list<int>::iterator;\n"
                            "V1 v1;\n"
                            "V2 v2;\n"
                            "V3 v3;\n"
                            "IntListIterator iter;";

        const char expected[] = "vector < int > v1 ; "
                                "std :: vector < int > v2 ; "
                                "std :: vector < std :: vector < int > > v3 ; "
                                "std :: list < int > :: iterator iter ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing13() {
        const char code[] = "using Func = std::pair<int(*)(void*), void*>;\n"
                            "using CallQueue = std::vector<Func>;\n"
                            "int main() {\n"
                            " CallQueue q;\n"
                            "}";

        const char expected[] = "int main ( ) { "
                                "std :: vector < std :: pair < int ( * ) ( void * ) , void * > > q ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyUsing14() {
        const char code[] = "template <typename F, unsigned int N> struct E"
                            "{"
                            "    using v = E<F,(N>0)?(N-1):0>;"
                            "    using val = typename add<v,v>::val;"
                            "    FP_M(val);"
                            "};"
                            "template <typename F> struct E <F,0>"
                            "{"
                            "    using nal = typename D<1>::val;"
                            "    FP_M(val);"
                            "};";

        tok(code, true, Settings::Native, false);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyUsing15() {
        {
            const char code[] = "using frame = char [10];\n"
                                "frame f;";

            const char expected[] = "char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "using frame = unsigned char [10];\n"
                                "frame f;";

            const char expected[] = "unsigned char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyUsing16() {
        const char code[] = "using MOT8 = char;\n"
                            "using CHFOO = MOT8 [4096];\n"
                            "using STRFOO = struct {\n"
                            "   CHFOO freem;\n"
                            "};\n"
                            "STRFOO s;";

        const char expected[] = "struct STRFOO { "
                                "char freem [ 4096 ] ; "
                                "} ; "
                                "struct STRFOO s ;";

        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyUsing8970() {
        const char code[] = "using V = std::vector<int>;\n"
                            "struct A {\n"
                            "    V p;\n"
                            "};";

        const char expected[] = "struct A { "
                                "std :: vector < int > p ; "
                                "} ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing8971() {
        const char code[] = "class A {\n"
                            "public:\n"
                            "    using V = std::vector<double>;\n"
                            "};\n"
                            "using V = std::vector<int>;\n"
                            "class I {\n"
                            "private:\n"
                            "    A::V v_;\n"
                            "    V v2_;\n"
                            "};";

        const char expected[] = "class A { "
                                "public: "
                                "} ; "
                                "class I { "
                                "private: "
                                "std :: vector < double > v_ ; "
                                "std :: vector < int > v2_ ; "
                                "} ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyUsing8976() {
        const char code[] = "using mystring = std::string;";

        const char exp[] = ";";

        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing9040() {
        const char code[] = "using BOOL = unsigned; int i;";

        const char exp[] = "int i ;";

        ASSERT_EQUALS(exp, tok(code, true, Settings::Unix32));
        ASSERT_EQUALS(exp, tok(code, true, Settings::Unix64));
        ASSERT_EQUALS(exp, tok(code, true, Settings::Win32A));
        ASSERT_EQUALS(exp, tok(code, true, Settings::Win32W));
        ASSERT_EQUALS(exp, tok(code, true, Settings::Win64));
    }

    void simplifyUsing9042() {
        const char code[] = "template <class T>\n"
                            "class c {\n"
                            "    int i = 0;\n"
                            "    c() { i--; }\n"
                            "};\n"
                            "template <class T>\n"
                            "class s {};\n"
                            "using BOOL = char;";

        const char exp[] = "template < class T > "
                           "class c { "
                           "int i ; i = 0 ; "
                           "c ( ) { i -- ; } "
                           "} ;";

        ASSERT_EQUALS(exp, tok(code, true, Settings::Win64));
    }

    void simplifyUsing9191() {
        const char code[] = "namespace NS1 {\n"
                            "  namespace NS2 {\n"
                            "    using _LONG = signed long long;\n"
                            "  }\n"
                            "}\n"
                            "void f1() {\n"
                            "  using namespace NS1;\n"
                            "  NS2::_LONG A;\n"
                            "}\n"
                            "void f2() {\n"
                            "  using namespace NS1::NS2;\n"
                            "  _LONG A;\n"
                            "}";

        const char exp[] = "namespace NS1 { "
                           "} "
                           "void f1 ( ) { "
                           "using namespace NS1 ; "
                           "signed long long A ; "
                           "} "
                           "void f2 ( ) { "
                           "using namespace NS1 :: NS2 ; "
                           "signed long long A ; "
                           "}";

        ASSERT_EQUALS(exp, tok(code, false));
    }

};

REGISTER_TEST(TestSimplifyUsing)
