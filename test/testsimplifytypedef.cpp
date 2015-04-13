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

#include "testsuite.h"
#include "tokenize.h"
#include "token.h"
#include "settings.h"


class TestSimplifyTypedef : public TestFixture {
public:
    TestSimplifyTypedef() : TestFixture("TestSimplifyTypedef") {
    }


private:

    void run() {
        TEST_CASE(simplifyTypedef1)
        TEST_CASE(simplifyTypedef2)
        TEST_CASE(simplifyTypedef3)
        TEST_CASE(simplifyTypedef4)
        TEST_CASE(simplifyTypedef5)
        TEST_CASE(simplifyTypedef6)
        TEST_CASE(simplifyTypedef7);
        TEST_CASE(simplifyTypedef8);
        TEST_CASE(simplifyTypedef9);
        TEST_CASE(simplifyTypedef10);
        TEST_CASE(simplifyTypedef11);
        TEST_CASE(simplifyTypedef12);
        TEST_CASE(simplifyTypedef13);
        TEST_CASE(simplifyTypedef14);
        TEST_CASE(simplifyTypedef15);
        TEST_CASE(simplifyTypedef16);
        TEST_CASE(simplifyTypedef17);
        TEST_CASE(simplifyTypedef18);       // typedef vector<int[4]> a;
        TEST_CASE(simplifyTypedef19);
        TEST_CASE(simplifyTypedef20);
        TEST_CASE(simplifyTypedef21);
        TEST_CASE(simplifyTypedef22);
        TEST_CASE(simplifyTypedef23);
        TEST_CASE(simplifyTypedef24);
        TEST_CASE(simplifyTypedef25);
        TEST_CASE(simplifyTypedef26);
        TEST_CASE(simplifyTypedef27);
        TEST_CASE(simplifyTypedef28);
        TEST_CASE(simplifyTypedef29);
        TEST_CASE(simplifyTypedef30);
        TEST_CASE(simplifyTypedef31);
        TEST_CASE(simplifyTypedef32);
        TEST_CASE(simplifyTypedef33);
        TEST_CASE(simplifyTypedef34); // ticket #1411
        TEST_CASE(simplifyTypedef35);
        TEST_CASE(simplifyTypedef36); // ticket #1434
        TEST_CASE(simplifyTypedef37); // ticket #1449
        TEST_CASE(simplifyTypedef38);
        TEST_CASE(simplifyTypedef39);
        TEST_CASE(simplifyTypedef40);
        TEST_CASE(simplifyTypedef41); // ticket #1488
        TEST_CASE(simplifyTypedef42); // ticket #1506
        TEST_CASE(simplifyTypedef43); // ticket #1588
        TEST_CASE(simplifyTypedef44);
        TEST_CASE(simplifyTypedef45); // ticket #1613
        TEST_CASE(simplifyTypedef46);
        TEST_CASE(simplifyTypedef47);
        TEST_CASE(simplifyTypedef48); // ticket #1673
        TEST_CASE(simplifyTypedef49); // ticket #1691
        TEST_CASE(simplifyTypedef50);
        TEST_CASE(simplifyTypedef51);
        TEST_CASE(simplifyTypedef52); // ticket #1782
        TEST_CASE(simplifyTypedef53); // ticket #1801
        TEST_CASE(simplifyTypedef54); // ticket #1814
        TEST_CASE(simplifyTypedef55);
        TEST_CASE(simplifyTypedef56); // ticket #1829
        TEST_CASE(simplifyTypedef57); // ticket #1846
        TEST_CASE(simplifyTypedef58); // ticket #1963
        TEST_CASE(simplifyTypedef59); // ticket #2011
        TEST_CASE(simplifyTypedef60); // ticket #2035
        TEST_CASE(simplifyTypedef61); // ticket #2074 and 2075
        TEST_CASE(simplifyTypedef62); // ticket #2082
        TEST_CASE(simplifyTypedef63); // ticket #2175 'typedef float x[3];'
        TEST_CASE(simplifyTypedef64);
        TEST_CASE(simplifyTypedef65); // ticket #2314
        TEST_CASE(simplifyTypedef66); // ticket #2341
        TEST_CASE(simplifyTypedef67); // ticket #2354
        TEST_CASE(simplifyTypedef68); // ticket #2355
        TEST_CASE(simplifyTypedef69); // ticket #2348
        TEST_CASE(simplifyTypedef70); // ticket #2348
        TEST_CASE(simplifyTypedef71); // ticket #2348
        TEST_CASE(simplifyTypedef72); // ticket #2375
        TEST_CASE(simplifyTypedef73); // ticket #2412
        TEST_CASE(simplifyTypedef74); // ticket #2414
        TEST_CASE(simplifyTypedef75); // ticket #2426
        TEST_CASE(simplifyTypedef76); // ticket #2453
        TEST_CASE(simplifyTypedef77); // ticket #2554
        TEST_CASE(simplifyTypedef78); // ticket #2568
        TEST_CASE(simplifyTypedef79); // ticket #2348
        TEST_CASE(simplifyTypedef80); // ticket #2587
        TEST_CASE(simplifyTypedef81); // ticket #2603
        TEST_CASE(simplifyTypedef82); // ticket #2403
        TEST_CASE(simplifyTypedef83); // ticket #2620
        TEST_CASE(simplifyTypedef84); // ticket #2630
        TEST_CASE(simplifyTypedef85); // ticket #2651
        TEST_CASE(simplifyTypedef86); // ticket #2581
        TEST_CASE(simplifyTypedef87); // ticket #2651
        TEST_CASE(simplifyTypedef88); // ticket #2675
        TEST_CASE(simplifyTypedef89); // ticket #2717
        TEST_CASE(simplifyTypedef90); // ticket #2718
        TEST_CASE(simplifyTypedef91); // ticket #2716
        TEST_CASE(simplifyTypedef92); // ticket #2736
        TEST_CASE(simplifyTypedef93); // ticket #2738
        TEST_CASE(simplifyTypedef94); // ticket #1982
        TEST_CASE(simplifyTypedef95); // ticket #2844
        TEST_CASE(simplifyTypedef96); // ticket #2886
        TEST_CASE(simplifyTypedef97); // ticket #2983 (segmentation fault)
        TEST_CASE(simplifyTypedef98); // ticket #2963
        TEST_CASE(simplifyTypedef99); // ticket #2999
        TEST_CASE(simplifyTypedef100); // ticket #3000
        TEST_CASE(simplifyTypedef101); // ticket #3003 (segmentation fault)
        TEST_CASE(simplifyTypedef102); // ticket #3004
        TEST_CASE(simplifyTypedef103); // ticket #3007
        TEST_CASE(simplifyTypedef104); // ticket #3070
        TEST_CASE(simplifyTypedef105); // ticket #3616
        TEST_CASE(simplifyTypedef106); // ticket #3619
        TEST_CASE(simplifyTypedef107); // ticket #3963 - bad code => segmentation fault
        TEST_CASE(simplifyTypedef108); // ticket #4777
        TEST_CASE(simplifyTypedef109); // ticket #1823 - rvalue reference
        TEST_CASE(simplifyTypedef110); // ticket #6268

        TEST_CASE(simplifyTypedefFunction1);
        TEST_CASE(simplifyTypedefFunction2); // ticket #1685
        TEST_CASE(simplifyTypedefFunction3);
        TEST_CASE(simplifyTypedefFunction4);
        TEST_CASE(simplifyTypedefFunction5);
        TEST_CASE(simplifyTypedefFunction6);
        TEST_CASE(simplifyTypedefFunction7);
        TEST_CASE(simplifyTypedefFunction8);
        TEST_CASE(simplifyTypedefFunction9);
        TEST_CASE(simplifyTypedefFunction10); // #5191

        TEST_CASE(simplifyTypedefShadow);  // #4445 - shadow variable
    }

    std::string tok(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Unspecified, bool debugwarnings = true) {
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.inconclusive = true;
        settings.debugwarnings = debugwarnings;   // show warnings about unhandled typedef
        settings.platform(type);
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, !simplify);
    }

    std::string simplifyTypedef(const char code[]) {
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.list.createTokens(istr);
        tokenizer.createLinks();
        tokenizer.simplifyTypedef();

        return tokenizer.tokens()->stringifyList(0, false);
    }



    void simplifyTypedef1() {
        const char code[] = "class A\n"
                            "{\n"
                            "public:\n"
                            " typedef wchar_t duplicate;\n"
                            " void foo() {}\n"
                            "};\n"
                            "typedef A duplicate;\n"
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

    void simplifyTypedef2() {
        const char code[] = "class A;\n"
                            "typedef A duplicate;\n"
                            "class A\n"
                            "{\n"
                            "public:\n"
                            "typedef wchar_t duplicate;\n"
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

    void simplifyTypedef3() {
        const char code[] = "class A {};\n"
                            "typedef A duplicate;\n"
                            "wchar_t foo()\n"
                            "{\n"
                            "typedef wchar_t duplicate;\n"
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

    void simplifyTypedef4() {
        const char code[] = "typedef int s32;\n"
                            "typedef unsigned int u32;\n"
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

    void simplifyTypedef5() {
        // ticket #780
        const char code[] =
            "typedef struct yy_buffer_state *YY_BUFFER_STATE;\n"
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

    void simplifyTypedef6() {
        // ticket #983
        const char code[] =
            "namespace VL {\n"
            "    typedef float float_t ;\n"
            "    inline VL::float_t fast_atan2(VL::float_t y, VL::float_t x){}\n"
            "}";

        const char expected[] =
            "namespace VL { "
            ""
            "float fast_atan2 ( float y , float x ) { } "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef7() {
        const char code[] = "typedef int abc ; "
                            "Fred :: abc f ;";
        const char expected[] = "Fred :: abc f ;";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef8() {
        const char code[] = "typedef int INT;\n"
                            "typedef unsigned int UINT;\n"
                            "typedef int * PINT;\n"
                            "typedef unsigned int * PUINT;\n"
                            "typedef int & RINT;\n"
                            "typedef unsigned int & RUINT;\n"
                            "typedef const int & RCINT;\n"
                            "typedef const unsigned int & RCUINT;\n"
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

    void simplifyTypedef9() {
        const char code[] = "typedef struct s S, * PS;\n"
                            "typedef struct t { int a; } T, *TP;\n"
                            "typedef struct { int a; } U;\n"
                            "typedef struct { int a; } * V;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;";

        const char expected[] =
            "struct t { int a ; } ; "
            "struct U { int a ; } ; "
            "struct Unnamed0 { int a ; } ; "
            "struct s s ; "
            "struct s * ps ; "
            "struct t t ; "
            "struct t * tp ; "
            "struct U u ; "
            "struct Unnamed0 * v ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef10() {
        const char code[] = "typedef union s S, * PS;\n"
                            "typedef union t { int a; float b ; } T, *TP;\n"
                            "typedef union { int a; float b; } U;\n"
                            "typedef union { int a; float b; } * V;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;";

        const char expected[] =
            "union t { int a ; float b ; } ; "
            "union U { int a ; float b ; } ; "
            "union Unnamed1 { int a ; float b ; } ; "
            "union s s ; "
            "union s * ps ; "
            "union t t ; "
            "union t * tp ; "
            "union U u ; "
            "union Unnamed1 * v ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef11() {
        const char code[] = "typedef enum { a = 0 , b = 1 , c = 2 } abc;\n"
                            "typedef enum xyz { x = 0 , y = 1 , z = 2 } XYZ;\n"
                            "abc e1;\n"
                            "XYZ e2;";

        const char expected[] = "int e1 ; "
                                "int e2 ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef12() {
        const char code[] = "typedef vector<int> V1;\n"
                            "typedef std::vector<int> V2;\n"
                            "typedef std::vector<std::vector<int> > V3;\n"
                            "typedef std::list<int>::iterator IntListIterator;\n"
                            "V1 v1;\n"
                            "V2 v2;\n"
                            "V3 v3;\n"
                            "IntListIterator iter;";

        const char expected[] =
            "vector < int > v1 ; "
            "std :: vector < int > v2 ; "
            "std :: vector < std :: vector < int > > v3 ; "
            "std :: list < int > :: iterator iter ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef13() {
        // ticket # 1167
        const char code[] = "typedef std::pair<int(*)(void*), void*> Func;"
                            "typedef std::vector<Func> CallQueue;"
                            "int main() {}";

        // Tokenize and check output..
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef14() {
        // ticket # 1232
        const char code[] = "template <typename F, unsigned int N> struct E"
                            "{"
                            "    typedef E<F,(N>0)?(N-1):0> v;"
                            "    typedef typename add<v,v>::val val;"
                            "    FP_M(val);"
                            "};"
                            "template <typename F> struct E <F,0>"
                            "{"
                            "    typedef typename D<1>::val val;"
                            "    FP_M(val);"
                            "};";

        // Tokenize and check output..
        tok(code, true, Settings::Unspecified, false);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef15() {
        {
            const char code[] = "typedef char frame[10];\n"
                                "frame f;";

            const char expected[] = "char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef unsigned char frame[10];\n"
                                "frame f;";

            const char expected[] = "unsigned char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef16() {
        // ticket # 1252
        const char code[] = "typedef char MOT8;\n"
                            "typedef  MOT8 CHFOO[4096];\n"
                            "typedef struct {\n"
                            "   CHFOO freem;\n"
                            "} STRFOO;";

        // Tokenize and check output..
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef17() {
        const char code[] = "typedef char * PCHAR, CHAR;\n"
                            "PCHAR pc;\n"
                            "CHAR c;";

        const char expected[] =
            "char * pc ; "
            "char c ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef18() {
        const char code[] = "typedef vector<int[4]> a;\n"
                            "a b;";

        // Clear the error buffer..
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList2();

        tokenizer.validate();
    }

    void simplifyTypedef19() {
        {
            // ticket #1275
            const char code[] = "typedef struct {} A, *B, **C;\n"
                                "A a;\n"
                                "B b;\n"
                                "C c;";

            const char expected[] =
                "struct A { } ; "
                "struct A a ; "
                "struct A * b ; "
                "struct A * * c ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef struct {} A, *********B;\n"
                                "A a;\n"
                                "B b;";

            const char expected[] =
                "struct A { } ; "
                "struct A a ; "
                "struct A * * * * * * * * * b ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef struct {} **********A, *B, C;\n"
                                "A a;\n"
                                "B b;\n"
                                "C c;";

            const char expected[] =
                "struct Unnamed2 { } ; "
                "struct Unnamed2 * * * * * * * * * * a ; "
                "struct Unnamed2 * b ; "
                "struct Unnamed2 c ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef20() {
        // ticket #1284
        const char code[] = "typedef jobject invoke_t (jobject, Proxy *, Method *, JArray< jobject > *);";

        // Clear the error buffer..
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList2();

        tokenizer.validate();
    }

    void simplifyTypedef21() {
        const char code[] = "typedef void (* PF)();\n"
                            "typedef void * (* PFV)(void *);\n"
                            "PF pf;\n"
                            "PFV pfv;";

        const char expected[] =
            ""
            ""
            "void ( * pf ) ( ) ; "
            "void * ( * pfv ) ( void * ) ;";

        ASSERT_EQUALS(expected, simplifyTypedef(code));
    }

    void simplifyTypedef22() {
        {
            const char code[] = "class Fred {\n"
                                "    typedef void (*testfp)();\n"
                                "    testfp get() { return test; }\n"
                                "    static void test() { }\n"
                                "};";

            const char expected[] =
                "class Fred { "
                ""
                "void * get ( ) { return test ; } "
                "static void test ( ) { } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef void * (*testfp)(void *);\n"
                                "    testfp get() { return test; }\n"
                                "    static void * test(void * p) { return p; }\n"
                                "};";

            const char expected[] =
                "class Fred { "
                ""
                "void * * get ( ) { return test ; } "
                "static void * test ( void * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef unsigned int * (*testfp)(unsigned int *);\n"
                                "    testfp get() { return test; }\n"
                                "    static unsigned int * test(unsigned int * p) { return p; }\n"
                                "};";

            const char expected[] =
                "class Fred { "
                ""
                "unsigned int * * get ( ) { return test ; } "
                "static unsigned int * test ( unsigned int * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef const unsigned int * (*testfp)(const unsigned int *);\n"
                                "    testfp get() { return test; }\n"
                                "    static const unsigned int * test(const unsigned int * p) { return p; }\n"
                                "};";

            // static const gets changed to const static
            const char expected[] =
                "class Fred { "
                ""
                "const unsigned int * * get ( ) { return test ; } "
                "const static unsigned int * test ( const unsigned int * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef void * (*testfp)(void *);\n"
                                "    testfp get(int i) { return test; }\n"
                                "    static void * test(void * p) { return p; }\n"
                                "};";

            const char expected[] =
                "class Fred { "
                ""
                "void * * get ( int i ) { return test ; } "
                "static void * test ( void * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef23() {
        const char code[] = "typedef bool (*Callback) (int i);\n"
                            "void    addCallback(Callback callback) { }\n"
                            "void    addCallback1(Callback callback, int j) { }";

        const char expected[] =
            "void addCallback ( bool * callback ) { } "
            "void addCallback1 ( bool * callback , int j ) { }";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef24() {
        {
            const char code[] = "typedef int (*fp)();\n"
                                "void g( fp f )\n"
                                "{\n"
                                "  fp f2 = (fp)f;\n"
                                "}";

            const char expected[] =
                "void g ( int * f ) "
                "{ "
                "int * f2 ; f2 = ( int * ) f ; "
                "}";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef int (*fp)();\n"
                                "void g( fp f )\n"
                                "{\n"
                                "  fp f2 = static_cast<fp>(f);\n"
                                "}";

            const char expected[] =
                "void g ( int * f ) "
                "{ "
                "int * f2 ; f2 = static_cast < int * > ( f ) ; "
                "}";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef25() {
        {
            // ticket #1298
            const char code[] = "typedef void (*fill_names_f) (const char *);\n"
                                "struct vfs_class {\n"
                                "    void (*fill_names) (struct vfs_class *me, fill_names_f);\n"
                                "}";

            const char expected[] =
                "struct vfs_class { "
                "void ( * fill_names ) ( struct vfs_class * me , void ( * ) ( const char * ) ) ; "
                "}";

            ASSERT_EQUALS(expected, simplifyTypedef(code));
        }

        {
            const char code[] = "typedef void (*fill_names_f) (const char *);\n"
                                "struct vfs_class {\n"
                                "    void (*fill_names) (fill_names_f, struct vfs_class *me);\n"
                                "}";

            const char expected[] =
                "struct vfs_class { "
                "void ( * fill_names ) ( void ( * ) ( const char * ) , struct vfs_class * me ) ; "
                "}";

            ASSERT_EQUALS(expected, simplifyTypedef(code));
        }
    }

    void simplifyTypedef26() {
        {
            const char code[] = "typedef void (*Callback) ();\n"
                                "void    addCallback(Callback (*callback)());";

            const char expected[] = "void addCallback ( void ( * ( * callback ) ( ) ) ( ) ) ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            // ticket # 1307
            const char code[] = "typedef void (*pc_video_update_proc)(bitmap_t *bitmap,\n"
                                "struct mscrtc6845 *crtc);\n"
                                "\n"
                                "struct mscrtc6845 *pc_video_start(pc_video_update_proc (*choosevideomode)(running_machine *machine, int *width, int *height, struct mscrtc6845 *crtc));";

            const char expected[] = "struct mscrtc6845 * pc_video_start ( void ( * ( * choosevideomode ) ( running_machine * machine , int * width , int * height , struct mscrtc6845 * crtc ) ) ( bitmap_t * bitmap , struct mscrtc6845 * crtc ) ) ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef27() {
        // ticket #1316
        const char code[] = "int main()\n"
                            "{\n"
                            "    typedef int (*func_ptr)(float, double);\n"
                            "    VERIFY((is_same<result_of<func_ptr(char, float)>::type, int>::value));\n"
                            "}";

        const char expected[] =
            "int main ( ) "
            "{ "
            ""
            "VERIFY ( is_same < result_of < int ( * ( char , float ) ) ( float , double ) > :: type , int > :: value ) ; "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef28() {
        const char code[] = "typedef std::pair<double, double> (*F)(double);\n"
                            "F f;";

        const char expected[] = "std :: pair < double , double > ( * f ) ( double ) ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef29() {
        const char code[] = "typedef int array [ice_or<is_int<int>::value, is_int<UDT>::value>::value ? 1 : -1];\n"
                            "typedef int array1 [N];\n"
                            "typedef int array2 [N][M];\n"
                            "typedef int int_t, int_array[N];\n"
                            "array a;\n"
                            "array1 a1;\n"
                            "array2 a2;\n"
                            "int_t t;\n"
                            "int_array ia;";

        const char expected[] =
            "int a [ ice_or < is_int < int > :: value , is_int < UDT > :: value > :: value ? 1 : -1 ] ; "
            "int a1 [ N ] ; "
            "int a2 [ N ] [ M ] ; "
            "int t ; "
            "int ia [ N ] ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef30() {
        const char code[] = "typedef ::std::list<int> int_list;\n"
                            "typedef ::std::list<int>::iterator int_list_iterator;\n"
                            "typedef ::std::list<int> int_list_array[10];\n"
                            "int_list il;\n"
                            "int_list_iterator ili;\n"
                            "int_list_array ila;";

        const char expected[] =
            ":: std :: list < int > il ; "
            ":: std :: list < int > :: iterator ili ; "
            ":: std :: list < int > ila [ 10 ] ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef31() {
        {
            const char code[] = "class A {\n"
                                "public:\n"
                                "    typedef int INT;\n"
                                "    INT get() const;\n"
                                "    void put(INT x) { a = x; }\n"
                                "    INT a;\n"
                                "};\n"
                                "A::INT A::get() const { return a; }\n"
                                "A::INT i = A::a;";

            const char expected[] = "class A { "
                                    "public: "
                                    ""
                                    "int get ( ) const ; "
                                    "void put ( int x ) { a = x ; } "
                                    "int a ; "
                                    "} ; "
                                    "int A :: get ( ) const { return a ; } "
                                    "int i ; i = A :: a ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct A {\n"
                                "    typedef int INT;\n"
                                "    INT get() const;\n"
                                "    void put(INT x) { a = x; }\n"
                                "    INT a;\n"
                                "};\n"
                                "A::INT A::get() const { return a; }\n"
                                "A::INT i = A::a;";

            const char expected[] = "struct A { "
                                    ""
                                    "int get ( ) const ; "
                                    "void put ( int x ) { a = x ; } "
                                    "int a ; "
                                    "} ; "
                                    "int A :: get ( ) const { return a ; } "
                                    "int i ; i = A :: a ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef32() {
        const char code[] = "typedef char CHAR;\n"
                            "typedef CHAR * LPSTR;\n"
                            "typedef const CHAR * LPCSTR;\n"
                            "CHAR c;\n"
                            "LPSTR cp;\n"
                            "LPCSTR ccp;";

        const char expected[] =
            "char c ; "
            "char * cp ; "
            "const char * ccp ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef33() {
        const char code[] = "class A {\n"
                            "public:\n"
                            "    typedef char CHAR_A;\n"
                            "    CHAR_A funA();\n"
                            "    class B {\n"
                            "    public:\n"
                            "        typedef short SHRT_B;\n"
                            "        SHRT_B funB();\n"
                            "        class C {\n"
                            "        public:\n"
                            "            typedef int INT_C;\n"
                            "            INT_C funC();\n"
                            "            struct D {\n"
                            "                typedef long LONG_D;\n"
                            "                LONG_D funD();\n"
                            "                LONG_D d;\n"
                            "            };\n"
                            "            INT_C c;\n"
                            "        };\n"
                            "        SHRT_B b;\n"
                            "    };\n"
                            "    CHAR_A a;\n"
                            "};\n"
                            "A::CHAR_A A::funA() { return a; }\n"
                            "A::B::SHRT_B A::B::funB() { return b; }\n"
                            "A::B::C::INT_C A::B::C::funC() { return c; }"
                            "A::B::C::D::LONG_D A::B::C::D::funD() { return d; }";

        const char expected[] =
            "class A { "
            "public: "
            ""
            "char funA ( ) ; "
            "class B { "
            "public: "
            ""
            "short funB ( ) ; "
            "class C { "
            "public: "
            ""
            "int funC ( ) ; "
            "struct D { "
            ""
            "long funD ( ) ; "
            "long d ; "
            "} ; "
            "int c ; "
            "} ; "
            "short b ; "
            "} ; "
            "char a ; "
            "} ; "
            "char A :: funA ( ) { return a ; } "
            "short A :: B :: funB ( ) { return b ; } "
            "int A :: B :: C :: funC ( ) { return c ; } "
            "long A :: B :: C :: D :: funD ( ) { return d ; }";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef34() {
        // ticket #1411
        const char code[] = "class X { };\n"
                            "typedef X (*foofunc)(const X&);\n"
                            "int main()\n"
                            "{\n"
                            "    foofunc *Foo = new foofunc[2];\n"
                            "}";
        const char expected[] =
            "class X { } ; "
            "int main ( ) "
            "{ "
            "X ( * * Foo ) ( const X & ) ; Foo = new X ( * ) ( const X & ) [ 2 ] ; "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    // Check simplifyTypedef
    void checkSimplifyTypedef(const char code[]) {
        errout.str("");
        // Tokenize..
        Settings settings;
        settings.inconclusive = true;
        settings.addEnabled("style");
        settings.debugwarnings = true;   // show warnings about unhandled typedef
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
    }

    void simplifyTypedef35() {
        const char code[] = "typedef int A;\n"
                            "class S\n"
                            "{\n"
                            "public:\n"
                            "    typedef float A;\n"
                            "    A a;\n"
                            "    virtual void fun(A x);\n"
                            "};\n"
                            "void S::fun(S::A) { };\n"
                            "class S1 : public S\n"
                            "{\n"
                            "public:\n"
                            "    void fun(S::A) { }\n"
                            "};\n"
                            "struct T\n"
                            "{\n"
                            "    typedef A B;\n"
                            "    B b;\n"
                            "};\n"
                            "float fun1(float A) { return A; }\n"
                            "float fun2(float a) { float A = a++; return A; }\n"
                            "float fun3(int a)\n"
                            "{\n"
                            "    typedef struct { int a; } A;\n"
                            "    A s; s.a = a;\n"
                            "    return s.a;\n"
                            "}\n"
                            "int main()\n"
                            "{\n"
                            "    A a = 0;\n"
                            "    S::A s = fun1(a) + fun2(a) - fun3(a);\n"
                            "    return a + s;\n"
                            "}";

        const char expected[] = "class S "
                                "{ "
                                "public: "
                                ""
                                "float a ; "
                                "virtual void fun ( float x ) ; "
                                "} ; "
                                "void S :: fun ( float ) { } ; "
                                "class S1 : public S "
                                "{ "
                                "public: "
                                "void fun ( float ) { } "
                                "} ; "
                                "struct T "
                                "{ "
                                ""
                                "int b ; "
                                "} ; "
                                "float fun1 ( float A ) { return A ; } "
                                "float fun2 ( float a ) { float A ; A = a ++ ; return A ; } "
                                "float fun3 ( int a ) "
                                "{ "
                                "struct A { int a ; } ; "
                                "struct A s ; s . a = a ; "
                                "return s . a ; "
                                "} "
                                "int main ( ) "
                                "{ "
                                "int a ; a = 0 ; "
                                "float s ; s = fun1 ( a ) + fun2 ( a ) - fun3 ( a ) ; "
                                "return a + s ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:1]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n"
                      "[test.cpp:20] -> [test.cpp:1]: (style, inconclusive) The function parameter 'A' hides a typedef with the same name.\n"
                      "[test.cpp:21] -> [test.cpp:1]: (style, inconclusive) The variable 'A' hides a typedef with the same name.\n"
                      "[test.cpp:24] -> [test.cpp:1]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n"
                      "[test.cpp:21]: (debug) ValueFlow bailout: increment/decrement of a\n", errout.str());
    }

    void simplifyTypedef36() {
        // ticket #1434
        const char code[] = "typedef void (*TIFFFaxFillFunc)();\n"
                            "void f(va_list ap)\n"
                            "{\n"
                            "    *va_arg(ap, TIFFFaxFillFunc*) = 0;\n"
                            "}";
        const char expected[] = "void f ( va_list ap ) "
                                "{ "
                                "* va_arg ( ap , void ( * * ) ( ) ) = 0 ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef37() {
        {
            // ticket #1449
            const char code[] = "template <class T> class V {};\n"
                                "typedef V<int> A;\n"
                                "typedef int B;\n"
                                "typedef V<int> A;\n"
                                "typedef int B;";

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n"
                          "[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) The typedef 'B' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef int INT;\n"
                                "void f()\n"
                                "{\n"
                                "    INT i; { }\n"
                                "}";
            const char expected[] = "void f ( ) "
                                    "{ "
                                    "int i ; { } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef38() {
        const char code[] = "typedef C A;\n"
                            "struct AB : public A, public B { };";
        const char expected[] = "struct AB : public C , public B { } ;";
        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef39() {
        const char code[] = "typedef int A;\n"
                            "template <const A, volatile A>::value;";
        const char expected[] = "template < const int , int > :: value ;";
        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef40() {
        const char code[] = "typedef int A;\n"
                            "typedef int B;\n"
                            "template <class A, class B> class C { };";
        const char expected[] = "template < class A , class B > class C { } ;";
        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (style, inconclusive) The template parameter 'A' hides a typedef with the same name.\n"
                      "[test.cpp:3] -> [test.cpp:2]: (style, inconclusive) The template parameter 'B' hides a typedef with the same name.\n", errout.str());

        checkSimplifyTypedef("typedef tuple<double&, const double&, const double, double*, const double*> t2;\n"
                             "void ordering_test()\n"
                             "{\n"
                             "  tuple<short, float> t2(5, 3.3f);\n"
                             "  BOOST_CHECK(t3 > t2);\n"
                             "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:1]: (style, inconclusive) The template instantiation 't2' hides a typedef with the same name.\n", errout.str());

        checkSimplifyTypedef("class MyOverflowingUnsigned\n"
                             "{\n"
                             "public:\n"
                             "    typedef unsigned self_type::*  bool_type;\n"
                             "    operator bool_type() const  { return this->v_ ? &self_type::v_ : 0; }\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        checkSimplifyTypedef("typedef int (*fptr_type)(int, int);\n"
                             "struct which_one {\n"
                             "  typedef fptr_type (*result_type)(bool x);\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        checkSimplifyTypedef("class my_configuration\n"
                             "{\n"
                             "public:\n"
                             "    template < typename T >\n"
                             "    class hook\n"
                             "    {\n"
                             "    public:\n"
                             "        typedef ::boost::rational<T>  rational_type;\n"
                             "    public:\n"
                             "        rational_type  ( &r_ )[ 9 ];\n"
                             "    };\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        checkSimplifyTypedef("class A\n"
                             "{\n"
                             "    typedef B b;\n"
                             "    friend b;\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef41() {
        // ticket #1488
        checkSimplifyTypedef("class Y;\n"
                             "class X\n"
                             "{\n"
                             "    typedef Y type;\n"
                             "    friend class type;\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef42() {
        // ticket #1506
        checkSimplifyTypedef("typedef struct A { } A;\n"
                             "struct A;");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style) The struct 'A' forward declaration is unnecessary. Type struct is already declared earlier.\n", errout.str());

        checkSimplifyTypedef("typedef union A { int i; float f; } A;\n"
                             "union A;");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style) The union 'A' forward declaration is unnecessary. Type union is already declared earlier.\n", errout.str());

        const char code [] = "typedef std::map<std::string, int> A;\n"
                             "class A;";
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style) The class 'A' forward declaration is unnecessary. Type class is already declared earlier.\n", errout.str());
        TODO_ASSERT_EQUALS("class A ;", "class std :: map < std :: string , int > ;", tok(code));
    }

    void simplifyTypedef43() {
        // ticket #1588
        {
            const char code[] = "typedef struct foo A;\n"
                                "struct A\n"
                                "{\n"
                                "    int alloclen;\n"
                                "};";

            // The expected result..
            const char expected[] = "struct A "
                                    "{ "
                                    "int alloclen ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The struct 'A' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef union foo A;\n"
                                "union A\n"
                                "{\n"
                                "    int alloclen;\n"
                                "};";

            // The expected result..
            const char expected[] = "union A "
                                    "{ "
                                    "int alloclen ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The union 'A' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef class foo A;\n"
                                "class A\n"
                                "{\n"
                                "    int alloclen;\n"
                                "};";

            // The expected result..
            const char expected[] = "class A "
                                    "{ "
                                    "int alloclen ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The class 'A' hides a typedef with the same name.\n", errout.str());
        }
    }

    void simplifyTypedef44() {
        {
            const char code[] = "typedef std::map<std::string, int> Map;\n"
                                "class MyMap : public Map\n"
                                "{\n"
                                "};";

            // The expected result..
            const char expected[] = "class MyMap : public std :: map < std :: string , int > "
                                    "{ "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef std::map<std::string, int> Map;\n"
                                "class MyMap : protected Map\n"
                                "{\n"
                                "};";

            // The expected result..
            const char expected[] = "class MyMap : protected std :: map < std :: string , int > "
                                    "{ "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef std::map<std::string, int> Map;\n"
                                "class MyMap : private Map\n"
                                "{\n"
                                "};";

            // The expected result..
            const char expected[] = "class MyMap : private std :: map < std :: string , int > "
                                    "{ "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef struct foo { } A;\n"
                                "struct MyA : public A\n"
                                "{\n"
                                "};";

            // The expected result..
            const char expected[] = "struct foo { } ; "
                                    "struct MyA : public foo "
                                    "{ "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef class foo { } A;\n"
                                "class MyA : public A\n"
                                "{\n"
                                "};";

            // The expected result..
            const char expected[] = "class foo { } ; "
                                    "class MyA : public foo "
                                    "{ "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef45() {
        // ticket # 1613
        const char code[] = "void fn() {\n"
                            "    typedef foo<> bar;\n"
                            "    while (0 > bar(1)) {}\n"
                            "}";

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef46() {
        const char code[] = "typedef const struct A { int a; } * AP;\n"
                            "AP ap;";

        // The expected result..
        const char expected[] = "struct A { int a ; } ; "
                                "const struct A * ap ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef47() {
        {
            const char code[] = "typedef std::pair<int, int> const I;\n"
                                "I i;";

            // The expected result..
            const char expected[] = "std :: pair < int , int > const i ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "typedef void (X:: *F)();\n"
                                "F f;";

            // The expected result..
            const char expected[] = "void * f ;";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void simplifyTypedef48() { // ticket #1673
        const char code[] = "typedef struct string { } string;\n"
                            "void foo (LIST *module_name)\n"
                            "{\n"
                            "    bar(module_name ? module_name->string : 0);\n"
                            "}";

        // The expected result..
        const char expected[] = "struct string { } ; "
                                "void foo ( LIST * module_name ) "
                                "{ "
                                "bar ( module_name ? module_name . string : 0 ) ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef49() { // ticket #1691
        const char code[] = "class Class2 {\n"
                            "typedef const Class & Const_Reference;\n"
                            "void some_method (Const_Reference x) const {}\n"
                            "void another_method (Const_Reference x) const {}\n"
                            "}";

        // The expected result..
        const char expected[] = "class Class2 { "
                                ""
                                "void some_method ( const Class & x ) const { } "
                                "void another_method ( const Class & x ) const { } "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef50() {
        const char code[] = "typedef char (* type1)[10];\n"
                            "typedef char (& type2)[10];\n"
                            "typedef char (& type3)[x];\n"
                            "typedef char (& type4)[x + 2];\n"
                            "type1 t1;\n"
                            "type1 (*tp1)[2];\n"
                            "type2 t2;\n"
                            "type3 t3;\n"
                            "type4 t4;";

        // The expected result..
        const char expected[] = "char * t1 [ 10 ] ; "
                                "char ( * ( * tp1 ) [ 2 ] ) [ 10 ] ; "
                                "char ( & t2 ) [ 10 ] ; "
                                "char ( & t3 ) [ x ] ; "
                                "char ( & t4 ) [ x + 2 ] ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef51() {
        const char code[] = "class A { public: int i; };\n"
                            "typedef const char (A :: * type1);\n"
                            "type1 t1 = &A::i;";

        // The expected result..
        const char expected[] = "class A { public: int i ; } ; "
                                "const char ( A :: * t1 ) ; t1 = & A :: i ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef52() { // ticket #1782
        {
            const char code[] = "typedef char (* type1)[10];\n"
                                "type1 foo() { }";

            // The expected result..
            const char expected[] = "char ( * foo ( ) ) [ 10 ] { }";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef char (* type1)[10];\n"
                                "LOCAL(type1) foo() { }";

            // this is invalid C so just make sure it doesn't generate an internal error
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef53() { // ticket #1801
        {
            const char code[] = "typedef int ( * int ( * ) ( ) ) ( ) ;";

            // this is invalid C so just make sure it doesn't crash
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:1]: (debug) Failed to parse 'typedef int ( * int ( * ) ( ) ) ( ) ;'. The checking continues anyway.\n", errout.str());
        }

        {
            const char code[] = "typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);\n"
                                "typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);";

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The typedef 'PPDMarkOption' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef int * A;\n"
                                "typedef int * A;";
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n", errout.str());
        }
    }

    void simplifyTypedef54() { // ticket #1814
        const char code[] = "void foo()\n"
                            "{\n"
                            "    typedef std::basic_string<char, traits_type, allocator_type> string_type;\n"
                            "    try\n"
                            "    {\n"
                            "        throw string_type(\"leak\");\n"
                            "    }\n"
                            "    catch (const string_type&)\n"
                            "    {\n"
                            "        pthread_exit (0);\n"
                            "    }\n"
                            "}";

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef55() {
        const char code[] = "typedef volatile unsigned long * const hwreg_t ;\n"
                            "typedef void *const t1[2];\n"
                            "typedef int*const *_Iterator;\n"
                            "hwreg_t v1;\n"
                            "t1 v2;\n"
                            "_Iterator v3;";

        // The expected result..
        const char expected[] = "long * v1 ; "
                                "void * v2 [ 2 ] ; "
                                "int * * v3 ;";
        ASSERT_EQUALS(expected, tok(code));

        // Check for output..
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef56() { // ticket #1829
        const char code[] = "struct C {\n"
                            "    typedef void (*fptr)();\n"
                            "    const fptr pr;\n"
                            "    operator const fptr& () { return pr; }\n"
                            "};";

        // The expected result..
        const char expected[] = "struct C { "
                                ""
                                "const void * pr ; " // this gets simplified to a regular pointer
                                "operatorconstvoid(*)()& ( ) { return pr ; } "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef57() { // ticket #1846
        const char code[] = "void foo() {\n"
                            "    typedef int A;\n"
                            "    A a = A(1) * A(2);\n"
                            "};";

        // The expected result..
        const char expected[] = "void foo ( ) { "
                                ""
                                "int a ; a = int ( 1 ) * int ( 2 ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef58() { // ticket #1963
        {
            const char code[] = "typedef int vec2_t[2];\n"
                                "vec2_t coords[4] = {1,2,3,4,5,6,7,8};";

            // The expected result..
            const char expected[] = "int coords [ 4 ] [ 2 ] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 } ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef int vec2_t[2];\n"
                                "vec2_t coords[4][5][6+1] = {1,2,3,4,5,6,7,8};";

            // The expected result..
            const char expected[] = "int coords [ 4 ] [ 5 ] [ 7 ] [ 2 ] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 } ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef59() { // ticket #2011
        const char code[] = "template<typename DISPATCHER> class SomeTemplateClass {\n"
                            "    typedef void (SomeTemplateClass<DISPATCHER>::*MessageDispatcherFunc)(SerialInputMessage&);\n"
                            "};";
        // The expected result..
        const char expected[] = "template < typename DISPATCHER > class SomeTemplateClass { } ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef60() { // ticket #2035
        const char code[] = "typedef enum {qfalse, qtrue} qboolean;\n"
                            "typedef qboolean (*localEntitiyAddFunc_t) (struct le_s * le, entity_t * ent);\n"
                            "void f()\n"
                            "{\n"
                            "    qboolean b;\n"
                            "    localEntitiyAddFunc_t f;\n"
                            "}";
        // The expected result..
        const char expected[] = "void f ( ) { int b ; int * f ; }";
        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef61() { // ticket #2074 and 2075
        const char code1[] = "typedef       unsigned char   (*Mf_GetIndexByte_Func)          (void);\n"
                             "typedef const unsigned char * (*Mf_GetPointerToCurrentPos_Func)(void);";

        // Check for output..
        checkSimplifyTypedef(code1);
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "typedef unsigned long uint32_t;\n"
                             "typedef uint32_t (*write_type_t) (uint32_t);";

        // Check for output..
        checkSimplifyTypedef(code2);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef62() { // ticket #2082
        const char code1[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a, b;\n"
                             "}";

        // The expected tokens..
        const char expected1[] = "void f ( ) { char a [ 256 ] ; char b [ 256 ] ; }";
        ASSERT_EQUALS(expected1, tok(code1, false));
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a = { 0 }, b = { 0 };\n"
                             "}";

        // The expected tokens..
        const char expected2[] = "void f ( ) { char a [ 256 ] ; a = { 0 } ; char b [ 256 ] ; b = { 0 } ; }";
        ASSERT_EQUALS(expected2, tok(code2, false));
        ASSERT_EQUALS("", errout.str());

        const char code3[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a = \"\", b = \"\";\n"
                             "}";

        // The expected tokens..
        const char expected3[] = "void f ( ) { char a [ 256 ] ; a = \"\" ; char b [ 256 ] ; b = \"\" ; }";
        ASSERT_EQUALS(expected3, tok(code3, false));
        ASSERT_EQUALS("", errout.str());

        const char code4[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a = \"1234\", b = \"5678\";\n"
                             "}";

        // The expected tokens..
        const char expected4[] = "void f ( ) { char a [ 256 ] ; a = \"1234\" ; char b [ 256 ] ; b = \"5678\" ; }";
        ASSERT_EQUALS(expected4, tok(code4, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef63() { // ticket #2175 'typedef float x[3];'
        const char code[] = "typedef float x[3];\n"
                            "x a,b,c;";
        const std::string actual(tok(code));
        ASSERT_EQUALS("float a [ 3 ] ; float b [ 3 ] ; float c [ 3 ] ;", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef64() {
        const char code[] = "typedef __typeof__(__type1() + __type2()) __type;"
                            "__type t;";
        const std::string actual(tok(code));
        ASSERT_EQUALS("__typeof__ ( __type1 ( ) + __type2 ( ) ) t ;", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef65() { // ticket #2314
        const char code[] = "typedef BAR<int> Foo;\n"
                            "int main() {\n"
                            "    Foo b(0);\n"
                            "    return b > Foo(10);\n"
                            "}";
        const std::string actual(tok(code, true, Settings::Unspecified, false));
        ASSERT_EQUALS("int main ( ) { BAR < int > b ( 0 ) ; return b > BAR < int > ( 10 ) ; }", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef66() { // ticket #2341
        const char code[] = "typedef long* GEN;\n"
                            "extern GEN (*foo)(long);";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef67() { // ticket #2354
        const char code[] = "typedef int ( * Function ) ( ) ;\n"
                            "void f ( ) {\n"
                            "    ((Function * (*) (char *, char *, int, int)) global[6]) ( \"assoc\", \"eggdrop\", 106, 0);\n"
                            "}";
        const char expected[] = "void f ( ) { "
                                "( ( int ( * * ( * ) ( char * , char * , int , int ) ) ( ) ) global [ 6 ] ) ( \"assoc\" , \"eggdrop\" , 106 , 0 ) ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef68() { // ticket #2355
        const char code[] = "typedef FMAC1 void (* a) ();\n"
                            "void *(*b) ();";
        const std::string actual(tok(code));
        ASSERT_EQUALS("void * * b ;", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef69() { // ticket #2348
        const char code[] = "typedef int (*CompilerHook)();\n"
                            "typedef struct VirtualMachine\n"
                            "{\n"
                            "    CompilerHook *(*compilerHookVector)(void);\n"
                            "}VirtualMachine;";
        const char expected[] = "struct VirtualMachine "
                                "{ "
                                "int ( * * ( * compilerHookVector ) ( void ) ) ( ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef70() { // ticket #2348
        const char code[] = "typedef int pread_f ( int ) ;\n"
                            "pread_f *(*test_func)(char *filename);";
        const char expected[] = "int ( * ( * test_func ) ( char * filename ) ) ( int ) ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef71() { // ticket #2348
        {
            const char code[] = "typedef int RexxFunctionHandler();\n"
                                "RexxFunctionHandler *(efuncs[1]);";
            const char expected[] = "int ( * ( efuncs [ 1 ] ) ) ( ) ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        {
            const char code[] = "typedef int RexxFunctionHandler();\n"
                                "RexxFunctionHandler *(efuncs[]) = { NULL, NULL };";
            const char expected[] = "int ( * ( efuncs [ ] ) ) ( ) = { 0 , 0 } ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef72() { // ticket #2374
        // inline operator
        {
            const char code[] = "class Fred {\n"
                                "    typedef int* (Fred::*F);\n"
                                "    operator F() const { }\n"
                                "};";
            const char expected[] = "class Fred { "
                                    ""
                                    "operatorint** ( ) const { } "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        // inline local variable
        {
            const char code[] = "class Fred {\n"
                                "    typedef int INT;\n"
                                "    void f1() const { INT i; }\n"
                                "};";
            const char expected[] = "class Fred { "
                                    ""
                                    "void f1 ( ) const { int i ; } "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        // out of line member variable
        {
            const char code[] = "class Fred {\n"
                                "    typedef int INT;\n"
                                "    void f1() const;\n"
                                "};\n"
                                "void Fred::f1() const { INT i; f(i); }";
            const char expected[] = "class Fred { "
                                    ""
                                    "void f1 ( ) const ; "
                                    "} ; "
                                    "void Fred :: f1 ( ) const { int i ; f ( i ) ; }";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        // out of line operator
        {
            const char code[] = "class Fred {\n"
                                "    typedef int* (Fred::*F);\n"
                                "    operator F() const;\n"
                                "};\n"
                                "Fred::operator F() const { }";
            const char expected[] = "class Fred { "
                                    ""
                                    "operatorint** ( ) const ; "
                                    "} ; "
                                    "Fred :: operatorint** ( ) const { }";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef73() { // ticket #2412
        const char code[] = "struct B {};\n"
                            "typedef struct A : public B {\n"
                            "    void f();\n"
                            "} a, *aPtr;";
        const char expected[] = "struct B { } ; "
                                "struct A : public B { "
                                "void f ( ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef74() { // ticket #2414
        const char code[] = "typedef long (*state_func_t)(void);\n"
                            "typedef state_func_t (*state_t)(void);\n"
                            "state_t current_state = death;\n"
                            "static char get_runlevel(const state_t);";
        const char expected[] = "long ( * ( * current_state ) ( void ) ) ( void ) ; current_state = death ; "
                                "static char get_runlevel ( const long ( * ( * ) ( void ) ) ( void ) ) ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef75() { // ticket #2426
        const char code[] = "typedef _Packed struct S { long l; };";
        ASSERT_EQUALS("", tok(code, true, Settings::Unspecified, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef76() { // ticket #2453 segmentation fault
        const char code[] = "void f1(typedef int x) {}";
        const char expected[] = "void f1 ( typedef int x ) { }";
        ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef77() { // ticket #2554
        const char code[] = "typedef char Str[10]; int x = sizeof(Str);";
        const char expected[] = "int x ; x = 10 ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef78() { // ticket #2568
        const char code[] = "typedef struct A A_t;\n"
                            "A_t a;\n"
                            "typedef struct A { } A_t;\n"
                            "A_t a1;";
        const char expected[] = "struct A a ; struct A { } ; struct A a1 ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef79() { // ticket #2348
        const char code[] = "typedef int (Tcl_ObjCmdProc) (int x);\n"
                            "typedef struct LangVtab\n"
                            "{\n"
                            "    Tcl_ObjCmdProc * (*V_LangOptionCommand);\n"
                            "} LangVtab;";
        const char expected[] = "struct LangVtab "
                                "{ "
                                "int ( * ( * V_LangOptionCommand ) ) ( int x ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef80() { // ticket #2587
        const char code[] = "typedef struct s { };\n"
                            "void f() {\n"
                            "    sizeof(struct s);\n"
                            "};";
        const char expected[] = "struct s { } ; "
                                "void f ( ) { "
                                "sizeof ( struct s ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef81() { // ticket #2603 segmentation fault
        ASSERT_THROW(checkSimplifyTypedef("typedef\n"), InternalError);

        checkSimplifyTypedef("typedef constexpr\n");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef82() { // ticket #2403
        checkSimplifyTypedef("class A {\n"
                             "public:\n"
                             "  typedef int F(int idx);\n"
                             "};\n"
                             "class B {\n"
                             "public:\n"
                             "  A::F ** f;\n"
                             "};\n"
                             "int main()\n"
                             "{\n"
                             "  B * b = new B;\n"
                             "  b->f = new A::F * [ 10 ];\n"
                             "}");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef83() { // ticket #2620
        const char code[] = "typedef char Str[10];\n"
                            "void f(Str &cl) { }";

        // The expected result..
        const char expected[] = "void f ( char ( & cl ) [ 10 ] ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef84() { // ticket #2630 (segmentation fault)
        const char code1[] = "typedef y x () x";
        ASSERT_THROW(checkSimplifyTypedef(code1), InternalError);

        const char code2[] = "typedef struct template <>";
        ASSERT_THROW(checkSimplifyTypedef(code2), InternalError);

        const char code3[] = "typedef ::<>";
        ASSERT_THROW(checkSimplifyTypedef(code3), InternalError);
    }

    void simplifyTypedef85() { // ticket #2651
        const char code[] = "typedef FOO ((BAR)(void, int, const int, int*));";
        const char expected[] = ";";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef86() { // ticket #2581
        const char code[] = "class relational {\n"
                            "    typedef void (safe_bool_helper::*safe_bool)();\n"
                            "public:\n"
                            "    operator safe_bool() const;\n"
                            "    safe_bool operator!() const;\n"
                            "};";
        const char expected[] = "class relational { "
                                ""
                                "public: "
                                "operatorsafe_bool ( ) const ; "
                                "safe_bool operator! ( ) const ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef87() { // ticket #2651
        const char code[] = "typedef FOO (*(*BAR)(void, int, const int, int*));";
        const char expected[] = ";";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef88() { // ticket #2675
        const char code[] = "typedef short int (*x)(...);";
        const char expected[] = ";";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef89() { // ticket #2717
        const char code[] = "class Fred {\n"
                            "    typedef void f(int) const;\n"
                            "    f func;\n"
                            "};";
        const char expected[] = "class Fred { void func ( int ) const ; } ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef90() { // ticket #2718
        const char code[] = "typedef int IA[2];\n"
                            "void f(const IA&) {};";
        const char expected[] = "void f ( const int ( & ) [ 2 ] ) { } ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef91() { // ticket #2716
        const char code1[] = "namespace NS {\n"
                             "    typedef int (*T)();\n"
                             "    class A {\n"
                             "        T f();\n"
                             "    };\n"
                             "}\n"
                             "namespace NS {\n"
                             "    T A::f() {}\n"
                             "}";
        const char expected1[] = "namespace NS { "
                                 ""
                                 "class A { "
                                 "int * f ( ) ; "
                                 "} ; "
                                 "} "
                                 "namespace NS { "
                                 "int * A :: f ( ) { } "
                                 "}";
        ASSERT_EQUALS(expected1, tok(code1));
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "namespace NS {\n"
                             "    typedef int (*T)();\n"
                             "    class A {\n"
                             "        T f();\n"
                             "    };\n"
                             "}\n"
                             "NS::T NS::A::f() {}";
        const char expected2[] = "namespace NS { "
                                 ""
                                 "class A { "
                                 "int * f ( ) ; "
                                 "} ; "
                                 "} "
                                 "int * NS :: A :: f ( ) { }";
        ASSERT_EQUALS(expected2, tok(code2));
        ASSERT_EQUALS("", errout.str());

        const char code3[] = "namespace NS1 {\n"
                             "    namespace NS2 {\n"
                             "        typedef int (*T)();\n"
                             "        class A {\n"
                             "            T f();\n"
                             "        };\n"
                             "    }\n"
                             "}\n"
                             "namespace NS1 {\n"
                             "    namespace NS2 {\n"
                             "        T A::f() {}\n"
                             "    }\n"
                             "}";
        const char expected3[] = "namespace NS1 { "
                                 "namespace NS2 { "
                                 ""
                                 "class A { "
                                 "int * f ( ) ; "
                                 "} ; "
                                 "} "
                                 "} "
                                 "namespace NS1 { "
                                 "namespace NS2 { "
                                 "int * A :: f ( ) { } "
                                 "} "
                                 "}";
        ASSERT_EQUALS(expected3, tok(code3));
        ASSERT_EQUALS("", errout.str());

        const char code4[] = "namespace NS1 {\n"
                             "    namespace NS2 {\n"
                             "        typedef int (*T)();\n"
                             "        class A {\n"
                             "            T f();\n"
                             "        };\n"
                             "    }\n"
                             "}\n"
                             "namespace NS1 {\n"
                             "    NS2::T NS2::A::f() {}\n"
                             "}";
        const char expected4[] = "namespace NS1 { "
                                 "namespace NS2 { "
                                 ""
                                 "class A { "
                                 "int * f ( ) ; "
                                 "} ; "
                                 "} "
                                 "} "
                                 "namespace NS1 { "
                                 "int * NS2 :: A :: f ( ) { } "
                                 "}";
        ASSERT_EQUALS(expected4, tok(code4));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef92() { // ticket #2736 (segmentation fault)
        const char code[] = "typedef long Long;\n"
                            "namespace NS {\n"
                            "}";
        ASSERT_EQUALS("", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef93() { // ticket #2738 (syntax error)
        const char code[] = "struct s { double x; };\n"
                            "typedef struct s (*binop) (struct s, struct s);";
        const char expected[] = "struct s { double x ; } ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef94() { // ticket #1982
        const char code1[] = "class A {\n"
                             "public:\n"
                             "  typedef struct {\n"
                             "    int a[4];\n"
                             "  } data;\n"
                             "};\n"
                             "A::data d;";
        const char expected1[] = "class A { "
                                 "public: "
                                 "struct data { "
                                 "int a [ 4 ] ; "
                                 "} ; "
                                 "} ; "
                                 "struct A :: data d ;";
        ASSERT_EQUALS(expected1, tok(code1));
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "class A {\n"
                             "public:\n"
                             "  typedef struct {\n"
                             "    int a[4];\n"
                             "  } data;\n"
                             "};\n"
                             "::A::data d;";
        const char expected2[] = "class A { "
                                 "public: "
                                 "struct data { "
                                 "int a [ 4 ] ; "
                                 "} ; "
                                 "} ; "
                                 "struct :: A :: data d ;";
        ASSERT_EQUALS(expected2, tok(code2));
        ASSERT_EQUALS("", errout.str());

        const char code3[] = "class A {\n"
                             "public:\n"
                             "  typedef struct {\n"
                             "    int a[4];\n"
                             "  } data;\n"
                             "};\n"
                             "class B : public ::A::data { };";
        const char expected3[] = "class A { "
                                 "public: "
                                 "struct data { "
                                 "int a [ 4 ] ; "
                                 "} ; "
                                 "} ; "
                                 "class B : public :: A :: data { } ;";
        ASSERT_EQUALS(expected3, tok(code3));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef95() { // ticket #2844
        const char code[] = "class symbol_table {\n"
                            "public:\n"
                            "  typedef expression_error::error_code (*valid_func)(void *cbparam, const char *name, expression_space space);\n"
                            "  valid_func f;\n"
                            "};";
        const char expected[] = "class symbol_table { "
                                "public: "
                                "expression_error :: error_code * f ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef96() { // ticket #2886 (segmentation fault)
        const char code[] = "typedef struct x { }";
        ASSERT_THROW(tok(code), InternalError);
    }

    void simplifyTypedef97() { // ticket #2983 (segmentation fault)
        const char code[] = "typedef x y\n"
                            "(A); y";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef98() { // ticket #2963
        const char code[] = "typedef int type ## __LINE__;\n"
                            "typedef int type ## __LINE__;\n"
                            "type1 x;\n"
                            "type2 y;";
        ASSERT_EQUALS("int x ; int y ;", tok(code));
    }

    void simplifyTypedef99() { // ticket #2999
        const char code[] = "typedef struct Fred Fred;\n"
                            "struct Fred { };";
        tok(code);
        ASSERT_EQUALS("", errout.str());

        const char code1[] = "struct Fred { };\n"
                             "typedef struct Fred Fred;";
        tok(code1);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef100() { // ticket #3000
        const char code[] = "typedef struct Fred { } Fred;\n"
                            "Fred * foo() {\n"
                            "    Fred *fred;\n"
                            "    fred = se_alloc(sizeof(struct Fred));\n"
                            "    return fred;\n"
                            "}";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef101() { // ticket #3003 (segmentation fault)
        const char code[] = "typedef a x[];\n"
                            "y = x";
        ASSERT_THROW(tok(code), InternalError);
    }

    void simplifyTypedef102() { // ticket #3004
        const char code[] = "typedef struct { } Fred;\n"
                            "void foo()\n"
                            "{\n"
                            "    Fred * Fred;\n"
                            "}";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef103() { // ticket #3007
        const char code[] = "typedef struct { } Fred;\n"
                            "void foo()\n"
                            "{\n"
                            "    Fred Fred;\n"
                            "}";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef104() { // ticket #3070
        const char code[] = "typedef int (*in_func) (void FAR *, unsigned char FAR * FAR *);";
        ASSERT_EQUALS(";", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef105() { // ticket #3616 (segmentation fault)
        const char code[] = "( int typedef char x; ){}";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef106() { // ticket #3619 (segmentation fault)
        const char code[] = "typedef void f ();\ntypedef { f }";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef107() { // ticket #3963 (bad code => segmentation fault)
        const char code[] = "typedef int x[]; int main() { return x }";
        ASSERT_THROW(tok(code), InternalError);
    }

    void simplifyTypedef108() { // ticket #4777
        const char code[] = "typedef long* GEN;\n"
                            "void sort_factor(GEN *y, long n) {\n"
                            "    GEN a, b;\n"
                            "    foo(a, b);\n"
                            "}";
        const char expected[] = "void sort_factor ( long * * y , long n ) { "
                                "long * a ; long * b ; "
                                "foo ( a , b ) ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef109() {
        const char code[] = "typedef int&& rref;\n"
                            "rref var;";
        const char expected[] = "int & & var ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef110() {
        const char code[] = "namespace A {\n"
                            "    namespace B {\n"
                            "        namespace D {\n"
                            "            typedef int DKIPtr;\n"
                            "        }\n"
                            "        struct ZClass {\n"
                            "            void set1(const A::B::D::DKIPtr& p) {\n"
                            "                membervariable1 = p;\n"
                            "            }\n"
                            "            void set2(const ::A::B::D::DKIPtr& p) {\n"
                            "                membervariable2 = p;\n"
                            "            }\n"
                            "            void set3(const B::D::DKIPtr& p) {\n"
                            "                membervariable3 = p;\n"
                            "            }\n"
                            "            void set4(const ::B::D::DKIPtr& p) {\n"
                            "                membervariable4 = p;\n"
                            "            }\n"
                            "            void set5(const C::D::DKIPtr& p) {\n"
                            "                membervariable5 = p;\n"
                            "            }\n"
                            "            A::B::D::DKIPtr membervariable1;\n"
                            "            ::A::B::D::DKIPtr membervariable2;\n"
                            "            B::D::DKIPtr membervariable3;\n"
                            "            ::B::D::DKIPtr membervariable4;\n"
                            "            C::D::DKIPtr membervariable5;\n"
                            "        };\n"
                            "    }\n"
                            "}";
        const char expected[] = "namespace A { "
                                "namespace B { "
                                "struct ZClass { "
                                "void set1 ( const int & p ) { "
                                "membervariable1 = p ; "
                                "} "
                                "void set2 ( const int & p ) { "
                                "membervariable2 = p ; "
                                "} "
                                "void set3 ( const int & p ) { "
                                "membervariable3 = p ; "
                                "} "
                                "void set4 ( const :: B :: D :: DKIPtr & p ) { "
                                "membervariable4 = p ; "
                                "} "
                                "void set5 ( const C :: D :: DKIPtr & p ) { "
                                "membervariable5 = p ; "
                                "} "
                                "int membervariable1 ; "
                                "int membervariable2 ; "
                                "int membervariable3 ; "
                                ":: B :: D :: DKIPtr membervariable4 ; "
                                "C :: D :: DKIPtr membervariable5 ; "
                                "} ; "
                                "} "
                                "}";
        ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction1() {
        {
            const char code[] = "typedef void (*my_func)();\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (*my_func)(void);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( void ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (*my_func)(int);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (*my_func)(int*);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            // ticket # 1615
            const char code[] = "typedef void (*my_func)(arg_class*);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( arg_class * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }


        {
            const char code[] = "typedef void (my_func)();\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(void);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( void ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(int);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(int*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(arg_class*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( arg_class * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }


        {
            const char code[] = "typedef void my_func();\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(void);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( void ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(int);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(int*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(arg_class*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( arg_class * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }


        {
            const char code[] = "typedef void (my_func());\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(void));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( void ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(int));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(int*));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( int * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(arg_class*));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const char expected[] = "std :: queue < void ( * ) ( arg_class * ) > func_queue ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedefFunction2() { // ticket #1685
        const char code[] = "typedef void voidfn (int);\n"
                            "voidfn xxx;";

        // The expected result..
        const char expected[] = "void xxx ( int ) ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedefFunction3() {
        {
            const char code[] = "typedef C func1();\n"
                                "typedef C (* func2)();\n"
                                "typedef C (& func3)();\n"
                                "typedef C (C::* func4)();\n"
                                "typedef C (C::* func5)() const;\n"
                                "typedef C (C::* func6)() volatile;\n"
                                "typedef C (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const char expected[] = "C f1 ( ) ; "
                                    "C * f2 ; " // this gets simplified to a regular pointer
                                    "C ( & f3 ) ( ) ; "
                                    "C * f4 ; " // this gets simplified to a regular pointer
                                    "C * f5 ; " // this gets simplified to a regular pointer
                                    "C * f6 ; " // this gets simplified to a regular pointer
                                    "C * f7 ;"; // this gets simplified to a regular pointer
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef C const func1();\n"
                                "typedef C const (* func2)();\n"
                                "typedef C const (& func3)();\n"
                                "typedef C const (C::* func4)();\n"
                                "typedef C const (C::* func5)() const;\n"
                                "typedef C const (C::* func6)() volatile;\n"
                                "typedef C const (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            // C const -> const C
            const char expected[] = "const C f1 ( ) ; "
                                    "const C * f2 ; " // this gets simplified to a regular pointer
                                    "const C ( & f3 ) ( ) ; "
                                    "const C * f4 ; " // this gets simplified to a regular pointer
                                    "const C * f5 ; " // this gets simplified to a regular pointer
                                    "const C * f6 ; " // this gets simplified to a regular pointer
                                    "const C * f7 ;"; // this gets simplified to a regular pointer
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef const C func1();\n"
                                "typedef const C (* func2)();\n"
                                "typedef const C (& func3)();\n"
                                "typedef const C (C::* func4)();\n"
                                "typedef const C (C::* func5)() const;\n"
                                "typedef const C (C::* func6)() volatile;\n"
                                "typedef const C (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const char expected[] = "const C f1 ( ) ; "
                                    "const C * f2 ; " // this gets simplified to a regular pointer
                                    "const C ( & f3 ) ( ) ; "
                                    "const C * f4 ; " // this gets simplified to a regular pointer
                                    "const C * f5 ; " // this gets simplified to a regular pointer
                                    "const C * f6 ; " // this gets simplified to a regular pointer
                                    "const C * f7 ;"; // this gets simplified to a regular pointer
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef C * func1();\n"
                                "typedef C * (* func2)();\n"
                                "typedef C * (& func3)();\n"
                                "typedef C * (C::* func4)();\n"
                                "typedef C * (C::* func5)() const;\n"
                                "typedef C * (C::* func6)() volatile;\n"
                                "typedef C * (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const char expected[] = "C * f1 ( ) ; "
                                    "C * * f2 ; " // this gets simplified to a regular pointer
                                    "C * ( & f3 ) ( ) ; "
                                    "C * * f4 ; " // this gets simplified to a regular pointer
                                    "C * * f5 ; " // this gets simplified to a regular pointer
                                    "C * * f6 ; " // this gets simplified to a regular pointer
                                    "C * * f7 ;"; // this gets simplified to a regular pointer
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef const C * func1();\n"
                                "typedef const C * (* func2)();\n"
                                "typedef const C * (& func3)();\n"
                                "typedef const C * (C::* func4)();\n"
                                "typedef const C * (C::* func5)() const;\n"
                                "typedef const C * (C::* func6)() volatile;\n"
                                "typedef const C * (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const char expected[] = "const C * f1 ( ) ; "
                                    "const C * * f2 ; " // this gets simplified to a regular pointer
                                    "const C * ( & f3 ) ( ) ; "
                                    "const C * * f4 ; " // this gets simplified to a regular pointer
                                    "const C * * f5 ; " // this gets simplified to a regular pointer
                                    "const C * * f6 ; " // this gets simplified to a regular pointer
                                    "const C * * f7 ;"; // this gets simplified to a regular pointer
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef C const * func1();\n"
                                "typedef C const * (* func2)();\n"
                                "typedef C const * (& func3)();\n"
                                "typedef C const * (C::* func4)();\n"
                                "typedef C const * (C::* func5)() const;\n"
                                "typedef C const * (C::* func6)() volatile;\n"
                                "typedef C const * (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            // C const -> const C
            const char expected[] = "const C * f1 ( ) ; "
                                    "const C * * f2 ; " // this gets simplified to a regular pointer
                                    "const C * ( & f3 ) ( ) ; "
                                    "const C * * f4 ; " // this gets simplified to a regular pointer
                                    "const C * * f5 ; " // this gets simplified to a regular pointer
                                    "const C * * f6 ; " // this gets simplified to a regular pointer
                                    "const C * * f7 ;"; // this gets simplified to a regular pointer
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedefFunction4() {
        const char code[] = "typedef int ( * ( * type1 ) ( bool ) ) ( int , int ) ;\n"
                            "typedef int ( * ( type2 ) ( bool ) ) ( int , int ) ;\n"
                            "typedef int ( * type3 ( bool ) ) ( int , int ) ;\n"
                            "type1 t1;\n"
                            "type2 t2;\n"
                            "type3 t3;";

        // The expected result..
        const char expected[] = "int ( * ( * t1 ) ( bool ) ) ( int , int ) ; "
                                "int * t2 ( bool ) ; "
                                "int * t3 ( bool ) ;";
        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction5() {
        const char code[] = "typedef int ( * type1 ) ( float ) ;\n"
                            "typedef int ( * const type2 ) ( float ) ;\n"
                            "typedef int ( * volatile type3 ) ( float ) ;\n"
                            "typedef int ( * const volatile type4 ) ( float ) ;\n"
                            "typedef int ( C :: * type5 ) ( float ) ;\n"
                            "typedef int ( C :: * const type6 ) ( float ) ;\n"
                            "typedef int ( C :: * volatile type7 ) ( float ) ;\n"
                            "typedef int ( C :: * const volatile type8 ) ( float ) ;\n"
                            "typedef int ( :: C :: * type9 ) ( float ) ;\n"
                            "typedef int ( :: C :: * const type10 ) ( float ) ;\n"
                            "typedef int ( :: C :: * volatile type11 ) ( float ) ;\n"
                            "typedef int ( :: C :: * const volatile type12 ) ( float ) ;\n"
                            "type1 t1;\n"
                            "type2 t2;\n"
                            "type3 t3;\n"
                            "type4 t4;\n"
                            "type5 t5;\n"
                            "type6 t6;\n"
                            "type7 t7;\n"
                            "type8 t8;\n"
                            "type9 t9;\n"
                            "type10 t10;\n"
                            "type11 t11;\n"
                            "type12 t12;";

        // The expected result..
        const char expected[] = "int * t1 ; " // simplified to regular pointer
                                "int * const t2 ; "
                                "int * t3 ; " // volatile removed, gets simplified to regular pointer
                                "int * const t4 ; " // volatile removed
                                "int * t5 ; "
                                "int * const t6 ; "
                                "int * t7 ; " // volatile removed
                                "int * const t8 ; " // volatile removed
                                "int ( :: C :: * t9 ) ( float ) ; "
                                "int ( :: C :: * const t10 ) ( float ) ; "
                                "int ( :: C :: * t11 ) ( float ) ; " // volatile removed
                                "int ( :: C :: * const t12 ) ( float ) ;"; // volatile removed
        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction6() {
        const char code[] = "typedef void (*testfp)();\n"
                            "struct Fred\n"
                            "{\n"
                            "    testfp get1() { return 0; }\n"
                            "    void ( * get2 ( ) ) ( ) { return 0 ; }\n"
                            "    testfp get3();\n"
                            "    void ( * get4 ( ) ) ( );\n"
                            "};\n"
                            "testfp Fred::get3() { return 0; }\n"
                            "void ( * Fred::get4 ( ) ) ( ) { return 0 ; }";

        // The expected result..
        const char expected[] = "struct Fred "
                                "{ "
                                "void * get1 ( ) { return 0 ; } "
                                "void * get2 ( ) { return 0 ; } "
                                "void * get3 ( ) ; "
                                "void * get4 ( ) ; "
                                "} ; "
                                "void * Fred :: get3 ( ) { return 0 ; } "
                                "void * Fred :: get4 ( ) { return 0 ; }";

        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction7() {
        const char code[] = "typedef void ( __gnu_cxx :: _SGIAssignableConcept < _Tp > :: * _func_Tp_SGIAssignableConcept ) () ;"
                            "_func_Tp_SGIAssignableConcept X;";

        // The expected result..
        const char expected[] = "void ( __gnu_cxx :: _SGIAssignableConcept < _Tp > :: * X ) ( ) ;";

        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction8() {
        // #2376 - internal error
        const char code[] = "typedef int f_expand(const nrv_byte *);\n"
                            "void f(f_expand   *(*get_fexp(int))){}";
        checkSimplifyTypedef(code);
        TODO_ASSERT_EQUALS("", "[test.cpp:2]: (debug) Function::addArguments found argument 'int' with varid 0.\n", errout.str());  // make sure that there is no internal error
    }

    void simplifyTypedefFunction9() {
        {
            const char code[] = "typedef ::C (::C::* func1)();\n"
                                "typedef ::C (::C::* func2)() const;\n"
                                "typedef ::C (::C::* func3)() volatile;\n"
                                "typedef ::C (::C::* func4)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;";

            // The expected result..
            const char expected[] = ":: C ( :: C :: * f1 ) ( ) ; "
                                    ":: C ( :: C :: * f2 ) ( ) const ; "
                                    ":: C ( :: C :: * f3 ) ( ) ; "
                                    ":: C ( :: C :: * f4 ) ( ) const ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef B::C (B::C::* func1)();\n"
                                "typedef B::C (B::C::* func2)() const;\n"
                                "typedef B::C (B::C::* func3)() volatile;\n"
                                "typedef B::C (B::C::* func4)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;";

            // The expected result..
            const char expected[] = "B :: C * f1 ; "
                                    "B :: C * f2 ; "
                                    "B :: C * f3 ; "
                                    "B :: C * f4 ;";
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef ::B::C (::B::C::* func1)();\n"
                                "typedef ::B::C (::B::C::* func2)() const;\n"
                                "typedef ::B::C (::B::C::* func3)() volatile;\n"
                                "typedef ::B::C (::B::C::* func4)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;";

            // The expected result..
            const char expected[] = ":: B :: C ( :: B :: C :: * f1 ) ( ) ; "
                                    ":: B :: C ( :: B :: C :: * f2 ) ( ) const ; "
                                    ":: B :: C ( :: B :: C :: * f3 ) ( ) ; "
                                    ":: B :: C ( :: B :: C :: * f4 ) ( ) const ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef A::B::C (A::B::C::* func1)();\n"
                                "typedef A::B::C (A::B::C::* func2)() const;\n"
                                "typedef A::B::C (A::B::C::* func3)() volatile;\n"
                                "typedef A::B::C (A::B::C::* func4)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;";

            // The expected result..
            const char expected[] = "A :: B :: C * f1 ; "
                                    "A :: B :: C * f2 ; "
                                    "A :: B :: C * f3 ; "
                                    "A :: B :: C * f4 ;";
            ASSERT_EQUALS(expected, tok(code, true, Settings::Unspecified, false));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedefFunction10() {
        const char code[] = "enum Format_E1 { FORMAT11 FORMAT12 } Format_T1;\n"
                            "namespace MySpace {\n"
                            "   enum Format_E2 { FORMAT21 FORMAT22 } Format_T2;\n"
                            "}\n"
                            "typedef Format_E1 (**PtrToFunPtr_Type1)();\n"
                            "typedef MySpace::Format_E2 (**PtrToFunPtr_Type2)();\n"
                            "PtrToFunPtr_Type1 t1;\n"
                            "PtrToFunPtr_Type2 t2;";
        ASSERT_EQUALS("int Format_T1 ; "
                      "namespace MySpace "
                      "{ "
                      "int Format_T2 ; "
                      "} "
                      "int ( * * t1 ) ( ) ; "
                      "int ( * * t2 ) ( ) ;",
                      tok(code,false));
    }

    void simplifyTypedefShadow() { // shadow variable (#4445)
        const char code[] = "typedef struct { int x; } xyz;;\n"
                            "void f(){\n"
                            "    int abc, xyz;\n" // <- shadow variable
                            "}";
        ASSERT_EQUALS("struct xyz { int x ; } ; void f ( ) { int abc ; int xyz ; }",
                      tok(code,false));
    }
};

REGISTER_TEST(TestSimplifyTypedef)
