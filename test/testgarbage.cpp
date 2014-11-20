/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
#include "check.h"

extern std::ostringstream errout;


class TestGarbage : public TestFixture {
public:
    TestGarbage() : TestFixture("TestGarbage") {
    }

private:

    void run() {
        // don't freak out when the syntax is wrong
        TEST_CASE(wrong_syntax1);
        TEST_CASE(wrong_syntax2);
        TEST_CASE(wrong_syntax3); // #3544
        TEST_CASE(wrong_syntax4); // #3618
        TEST_CASE(wrong_syntax_if_macro);  // #2518 - if MACRO()
        TEST_CASE(wrong_syntax_class_x_y); // #3585 - class x y { };
        TEST_CASE(syntax_case_default);
        TEST_CASE(garbageCode1);
        TEST_CASE(garbageCode2); // #4300
        TEST_CASE(garbageCode3); // #4869
        TEST_CASE(garbageCode4); // #4887
        TEST_CASE(garbageCode5); // #5168
        TEST_CASE(garbageCode6); // #5214
        TEST_CASE(garbageCode7);
        TEST_CASE(garbageCode8); // #5511
        TEST_CASE(garbageCode9); // #5604
        TEST_CASE(garbageCode10);  // #6127
        TEST_CASE(garbageCode11);
        TEST_CASE(garbageCode12);
        TEST_CASE(garbageCode13);  // Ticket #2607 - crash
        TEST_CASE(garbageCode14);  // TIcket #5595 - crash
        TEST_CASE(garbageCode15);  // Ticket #5203
        TEST_CASE(garbageCode16);
        TEST_CASE(garbageCode17);
        TEST_CASE(garbageCode18);
        TEST_CASE(garbageCode19);
        TEST_CASE(garbageCode20);
        TEST_CASE(garbageCode21);
        TEST_CASE(garbageCode22);
        TEST_CASE(garbageCode23);

        TEST_CASE(garbageValueFlow);
        TEST_CASE(garbageSymbolDatabase);
        TEST_CASE(garbageAST);
    }

    std::string checkCode(const char code[], const char filename[] = "test.cpp") {
        errout.str("");

        Settings settings;
        settings.debugwarnings = true;
        settings.addEnabled("style");
        settings.addEnabled("warning");
        settings.addEnabled("portability");
        settings.addEnabled("performance");
        settings.inconclusive = true;
        settings.experimental = true;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // call all "runChecks" in all registered Check classes
        for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
            (*it)->runChecks(&tokenizer, &settings, this);
        }

        tokenizer.simplifyTokenList2();
        // call all "runSimplifiedChecks" in all registered Check classes
        for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
            (*it)->runSimplifiedChecks(&tokenizer, &settings, this);
        }

        return tokenizer.tokens()->stringifyList(false, false, false, true, false, 0, 0);
    }

    void wrong_syntax1() {
        {
            const char code[] ="TR(kvmpio, PROTO(int rw), ARGS(rw), TP_(aa->rw;))";
            ASSERT_EQUALS("TR ( kvmpio , PROTO ( int rw ) , ARGS ( rw ) , TP_ ( aa . rw ; ) )", checkCode(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] ="struct A { template<int> struct { }; };";
            ASSERT_THROW(checkCode(code), InternalError);
        }

        {
            const char code[] ="enum ABC { A,B, typedef enum { C } };";
            ASSERT_THROW(checkCode(code), InternalError);
        }

        {
            // #3314 - don't report syntax error.
            const char code[] ="struct A { typedef B::C (A::*f)(); };";
            checkCode(code);
            ASSERT_EQUALS("[test.cpp:1]: (debug) Failed to parse 'typedef B :: C ( A :: * f ) ( ) ;'. The checking continues anyway.\n", errout.str());
        }
    }

    void wrong_syntax2() {   // #3504
        const char code[] = "void f() {\n"
                            "    X<int> x;\n"
                            "    Y<int, int, int, int, int, char> y;\n"
                            "}\n"
                            "\n"
                            "void G( template <typename T> class (j) ) {}";

        // don't segfault..
        checkCode(code);
    }

    void wrong_syntax3() {   // #3544
        const char code[] = "X #define\n"
                            "{\n"
                            " (\n"
                            "  for(  #endif typedef typedef cb[N] )\n"
                            "        ca[N]; =  cb[i]\n"
                            " )\n"
                            "}";

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        try {
            tokenizer.tokenize(istr, "test.cpp");
            assertThrowFail(__FILE__, __LINE__);
        } catch (InternalError& e) {
            ASSERT_EQUALS("Analysis failed. If the code is valid then please report this failure.", e.errorMessage);
            ASSERT_EQUALS("cppcheckError", e.id);
            ASSERT_EQUALS(5, e.token->linenr());
        }
    }

    void wrong_syntax4() {   // #3618
        const char code[] = "typedef void (x) (int);    return x&";

        ASSERT_THROW(checkCode(code), InternalError);
    }

    void wrong_syntax_if_macro() {
        // #2518 #4171
        ASSERT_THROW(checkCode("void f() { if MACRO(); }"), InternalError);

        // #4668 - note there is no semicolon after MACRO()
        ASSERT_THROW(checkCode("void f() { if (x) MACRO() {} }"), InternalError);

        // #4810 - note there is no semicolon after MACRO()
        ASSERT_THROW(checkCode("void f() { if (x) MACRO() else ; }"), InternalError);
    }

    void wrong_syntax_class_x_y() {
        // #3585
        const char code[] = "class x y { };";

        errout.str("");

        Settings settings;
        settings.addEnabled("information");
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.c");
        tokenizer.simplifyTokenList2();

        ASSERT_EQUALS("[test.c:1]: (information) The code 'class x y {' is not handled. You can use -I or --include to add handling of this code.\n", errout.str());
    }

    void syntax_case_default() {
        ASSERT_THROW(checkCode("void f() {switch (n) { case: z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case;: z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case {}: z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case 0?{1}:{2} : z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case 0?1;:{2} : z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case 0?(1?{3:4}):2 : z(); break;}}"), InternalError);

        //ticket #4234
        ASSERT_THROW(checkCode("( ) { switch break ; { switch ( x ) { case } y break ; : } }"), InternalError);

        //ticket #4267
        ASSERT_THROW(checkCode("f ( ) { switch break; { switch ( x ) { case } case break; -6: ( ) ; } }"), InternalError);
    }

    void garbageCode1() {
        checkCode("struct x foo_t; foo_t typedef y;");
    }

    void garbageCode2() { //#4300 (segmentation fault)
        ASSERT_THROW(checkCode("enum { D = 1  struct  { } ; }  s.b = D;"), InternalError);
    }

    void garbageCode3() { //#4849 (segmentation fault in Tokenizer::simplifyStructDecl (invalid code))
        ASSERT_THROW(checkCode("enum {  D = 2 s ; struct y  { x } ; } { s.a = C ; s.b = D ; }"), InternalError);
    }

    void garbageCode4() { // #4887
        ASSERT_THROW(checkCode("void f ( ) { = a ; if ( 1 ) if = ( 0 ) ; }"), InternalError);
    }

    void garbageCode5() { // #5168
        checkCode("( asm : ; void : );");
    }

    void garbageCode6() { // #5214
        checkCode("int b = ( 0 ? ? ) 1 : 0 ;");
        checkCode("int a = int b = ( 0 ? ? ) 1 : 0 ;");
    }

    void garbageCode7() {
        ASSERT_THROW(checkCode("1 (int j) { return return (c) * sizeof } y[1];"), InternalError);
        checkCode("foo(Args&&...) fn void = { } auto template<typename... bar(Args&&...)");
    }

    void garbageCode8() { // #5604
        ASSERT_THROW(checkCode("{ enum struct : };"), InternalError);
        ASSERT_THROW(checkCode("int ScopedEnum{ template<typename T> { { e = T::error }; };\n"
                               "ScopedEnum1<int> se1; { enum class E : T { e = 0 = e ScopedEnum2<void*> struct UnscopedEnum3 { T{ e = 4 }; };\n"
                               "arr[(int) E::e]; }; UnscopedEnum3<int> e2 = f()\n"
                               "{ { e = e1; T::error } int test1 ue2; g() { enum class E { e = T::error }; return E::e; } int test2 = } \n"
                               "namespace UnscopedEnum { template<typename T> struct UnscopedEnum1 { E{ e = T::error }; }; UnscopedEnum1<int> { enum E : { e = 0 }; };\n"
                               "UnscopedEnum2<void*> ue3; template<typename T> struct UnscopedEnum3 { enum { }; }; int arr[E::e]; };\n"
                               "UnscopedEnum3<int> namespace template<typename T> int f() { enum E { e }; T::error }; return (int) E(); } int test1 int g() { enum E { e = E };\n"
                               "E::e; } int test2 = g<int>(); }"), InternalError);
    }

    void garbageCode9() {
        ASSERT_THROW(checkCode("enum { e = { } } ( ) { { enum { } } } { e } "), InternalError);
    }

    void garbageCode10() { // #6127
        checkCode("for( rl=reslist; rl!=NULL; rl=rl->next )");
    }

    void garbageCode11() { // do not crash
        checkCode("( ) &");
    }

    void garbageCode12() { // do not crash
        checkCode("{ g; S (void) { struct } { } int &g; }");
    }

    void garbageCode13() {
        checkCode("struct C {} {} x");
    }

    void garbageCode14() {
        checkCode("static f() { int i; int source[1] = { 1 }; for (i = 0; i < 4; i++) (u, if (y u.x e)) }"); // Garbage code
    }

    void garbageCode15() { // Ticket #5203
        checkCode("int f ( int* r ) { {  int s[2] ; f ( s ) ; if ( ) } }");
    }

    void garbageCode16() {
        checkCode("{ } A() { delete }"); // #6080
    }

    void garbageCode17() {
        ASSERT_THROW(checkCode("void h(int l) {\n"
                               "    while\n" // Don't crash (#3870)
                               "}"), InternalError);
    }

    void garbageCode18() {
        ASSERT_THROW(checkCode("switch(){case}"), InternalError);
    }

    void garbageCode19() {
        // ticket #3512 - Don't crash on garbage code
        ASSERT_EQUALS("p = const", checkCode("1 *p = const"));
    }

    void garbageCode20() {
        // #3953 (valgrind errors on garbage code)
        ASSERT_EQUALS("void f ( 0 * ) ;", checkCode("void f ( 0 * ) ;"));
    }

    void garbageCode21() {
        // Ticket #3486 - Don't crash garbage code
        checkCode("void f()\n"
                  "{\n"
                  "  (\n"
                  "    x;\n"
                  "    int a, a2, a2*x; if () ;\n"
                  "  )\n"
                  "}");
    }

    void garbageCode22() {
        // Ticket #3480 - Don't crash garbage code
        ASSERT_THROW(checkCode("int f()\n"
                               "{\n"
                               "    return if\n"
                               "}"), InternalError);
    }

    void garbageCode23() {
        //garbage code : don't crash (#3481)
        checkCode("{\n"
                  "    if (1) = x\n"
                  "    else abort s[2]\n"
                  "}");
        ASSERT_EQUALS("", errout.str());
    }

    void garbageValueFlow() {
        // #6089
        const char* code = "{} int foo(struct, x1, struct x2, x3, int, x5, x6, x7)\n"
                           "{\n"
                           "    (foo(s, , 2, , , 5, , 7)) abort()\n"
                           "}\n";
        ASSERT_THROW(checkCode(code), InternalError);

        // #6106
        code = " f { int i ; b2 , [ ] ( for ( i = 0 ; ; ) ) }";
        checkCode(code);

        // 6122 survive garbage code
        code = "; { int i ; for ( i = 0 ; = 123 ; ) - ; }";
        checkCode(code);
    }

    void garbageSymbolDatabase() {
        checkCode("void f( { u = 1 ; } ) { }");

        checkCode("{ }; void namespace A::f; { g() { int } }");

        ASSERT_THROW(checkCode("class Foo {}; class Bar : public Foo"), InternalError);

        ASSERT_THROW(checkCode("YY_DECL { switch (yy_act) {\n"
                               "    case 65: YY_BREAK\n"
                               "    case YY_STATE_EOF(block):\n"
                               "        yyterminate(); \n"
                               "} }"), InternalError); // #5663
    }

    void garbageAST() {
        checkCode("--"); // don't crash

        checkCode("N 1024 float a[N], b[N + 3], c[N]; void N; (void) i;\n"
                  "int #define for (i = avx_test i < c[i]; i++)\n"
                  "b[i + 3] = a[i] * {}"); // Don't hang (#5787)

        checkCode("START_SECTION([EXTRA](bool isValid(const String &filename)))"); // Don't crash (#5991)
    }
};

REGISTER_TEST(TestGarbage)
