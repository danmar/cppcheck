
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
#include "check.h"


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
        TEST_CASE(garbageCode10); // #6127
        TEST_CASE(garbageCode11);
        TEST_CASE(garbageCode12);
        TEST_CASE(garbageCode13); // #2607
        TEST_CASE(garbageCode14); // #5595
        TEST_CASE(garbageCode15); // #5203
        TEST_CASE(garbageCode16);
        TEST_CASE(garbageCode17);
        TEST_CASE(garbageCode18);
        TEST_CASE(garbageCode19);
        TEST_CASE(garbageCode20);
        TEST_CASE(garbageCode21);
        TEST_CASE(garbageCode22);
        TEST_CASE(garbageCode23);
        TEST_CASE(garbageCode24); // #6361
        TEST_CASE(garbageCode25);
        TEST_CASE(garbageCode26);
        TEST_CASE(garbageCode27);
        TEST_CASE(garbageCode28);
        TEST_CASE(garbageCode29);
        TEST_CASE(garbageCode30); // #5867
        TEST_CASE(garbageCode31); // #6539
        TEST_CASE(garbageCode32); // #6135
        TEST_CASE(garbageCode33); // #6613
        TEST_CASE(garbageCode34); // #6626
        TEST_CASE(garbageCode35); // #2599, #2604
        TEST_CASE(garbageCode36); // #6334
        TEST_CASE(garbageCode37); // #5166
        TEST_CASE(garbageCode38); // #6666
        TEST_CASE(garbageCode39); // #6686
        TEST_CASE(garbageCode40); // #6620
        TEST_CASE(garbageCode41); // #6685
        TEST_CASE(garbageCode42); // #5760
        TEST_CASE(garbageCode43); // #6703
        TEST_CASE(garbageCode44); // #6704
        TEST_CASE(garbageCode45); // #6608
        TEST_CASE(garbageCode46); // #6705
        TEST_CASE(garbageCode47); // #6706
        TEST_CASE(garbageCode48); // #6712
        TEST_CASE(garbageCode49); // #6715
        TEST_CASE(garbageCode50); // #6718
        TEST_CASE(garbageCode51); // #6719
        TEST_CASE(garbageCode52); // #6720
        TEST_CASE(garbageCode53); // #6721
        TEST_CASE(garbageCode54); // #6722
        TEST_CASE(garbageCode55); // #6724
        TEST_CASE(garbageCode56); // #6713
        TEST_CASE(garbageCode57); // #6733
        TEST_CASE(garbageCode58); // #6732
        TEST_CASE(garbageCode59); // #6735
        TEST_CASE(garbageCode60); // #6736
        TEST_CASE(garbageCode61);
        TEST_CASE(garbageCode62);
        TEST_CASE(garbageCode63);
        TEST_CASE(garbageCode64);
        TEST_CASE(garbageCode65);
        TEST_CASE(garbageCode66);
        TEST_CASE(garbageCode67);
        TEST_CASE(garbageCode68);
        TEST_CASE(garbageCode69);
        TEST_CASE(garbageCode70);
        TEST_CASE(garbageCode71);
        TEST_CASE(garbageCode72);
        TEST_CASE(garbageCode73);
        TEST_CASE(garbageCode74);
        TEST_CASE(garbageCode75);
        TEST_CASE(garbageCode76);
        TEST_CASE(garbageCode77);
        TEST_CASE(garbageCode78);
        TEST_CASE(garbageCode79);
        TEST_CASE(garbageCode80);
        TEST_CASE(garbageCode81);
        TEST_CASE(garbageCode82);
        TEST_CASE(garbageCode83);
        TEST_CASE(garbageCode84);
        TEST_CASE(garbageCode85);
        TEST_CASE(garbageCode86);
        TEST_CASE(garbageCode87);
        TEST_CASE(garbageCode88);
        TEST_CASE(garbageCode89);
        TEST_CASE(garbageCode90);
        TEST_CASE(garbageCode91);
        TEST_CASE(garbageCode92);
        TEST_CASE(garbageCode93);
        TEST_CASE(garbageCode94);
        TEST_CASE(garbageCode95);
        TEST_CASE(garbageCode96);
        TEST_CASE(garbageCode97);
        TEST_CASE(garbageCode98);
        TEST_CASE(garbageCode99);
        TEST_CASE(garbageCode100);
        TEST_CASE(garbageCode101); // #6835
        TEST_CASE(garbageCode102); // #6846
        TEST_CASE(garbageCode103); // #6824
        TEST_CASE(garbageCode104); // #6847
        TEST_CASE(garbageCode105); // #6859
        TEST_CASE(garbageCode106);
        TEST_CASE(garbageCode107);
        TEST_CASE(garbageCode108);
        TEST_CASE(garbageCode109);
        TEST_CASE(garbageCode110);
        TEST_CASE(garbageCode111);
        TEST_CASE(garbageCode112);
        TEST_CASE(garbageCode113);
        TEST_CASE(garbageCode114);
        TEST_CASE(garbageCode115); // #5506
        TEST_CASE(garbageCode116); // #5356
        TEST_CASE(garbageCode117); // #6121
        TEST_CASE(garbageCode118); // #5600
        TEST_CASE(garbageCode119); // #5598
        TEST_CASE(garbageCode120); // #4927
        TEST_CASE(garbageCode121); // #2585
        TEST_CASE(garbageCode122); // #6303
        TEST_CASE(garbageCode123);
        TEST_CASE(garbageCode124); // 6948

        TEST_CASE(garbageValueFlow);
        TEST_CASE(garbageSymbolDatabase);
        TEST_CASE(garbageAST);
        TEST_CASE(templateSimplifierCrashes);
    }

    std::string checkCode(const char code[], bool cpp = true) {
        // double the tests - run each example as C as well as C++
        const char* filename = cpp ? "test.cpp" : "test.c";
        const char* alternatefilename = cpp ? "test.c" : "test.cpp";

        // run alternate check first. It should only ensure stability
        try {
            checkCodeInternal(code, alternatefilename);
        } catch (InternalError&) {
        }

        return checkCodeInternal(code, filename);
    }

    std::string checkCodeInternal(const char code[], const char* filename) {
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

        Settings settings;
        settings.addEnabled("information");

        {
            errout.str("");
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.c");
            tokenizer.simplifyTokenList2();
            ASSERT_EQUALS("", errout.str());
        }
        {
            errout.str("");
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            tokenizer.simplifyTokenList2();
            ASSERT_EQUALS("[test.cpp:1]: (information) The code 'class x y {' is not handled. You can use -I or --include to add handling of this code.\n", errout.str());
        }
    }

    void syntax_case_default() {
        ASSERT_THROW(checkCode("void f() {switch (n) { case: z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case;: z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case {}: z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case 0?{1}:{2} : z(); break;}}"), InternalError);

        ASSERT_THROW(checkCode("void f() {switch (n) { case 0?1;:{2} : z(); break;}}"), InternalError);

        checkCode("void f() {switch (n) { case 0?(1?{3:4}):2 : z(); break;}}");

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
        checkCode("1 (int j) { return return (c) * sizeof } y[1];");
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

    void garbageCode24() {
        // don't crash (example from #6361)
        checkCode("float buffer[64];\n"
                  "main (void)\n"
                  "{\n"
                  "  char *cptr;\n"
                  "  cptr = (char *)buffer;\n"
                  "  cptr += (-(long int) buffer & (16 * sizeof (float) - 1));\n"
                  "}\n");
    }

    void garbageCode25() {
        // Ticket #2386 - Segmentation fault upon strange syntax
        ASSERT_THROW(checkCode("void f() {\n"
                               "    switch ( x ) {\n"
                               "        case struct Tree : break;\n"
                               "    }\n"
                               "}"), InternalError);
    }

    void garbageCode26() {
        // See tickets #2518 #2555 #4171
        ASSERT_THROW(checkCode("void f() {\n"
                               "    switch MAKEWORD(1)\n"
                               "    {\n"
                               "    case 0:\n"
                               "        return;\n"
                               "    }\n"
                               "}"), InternalError);
    }

    void garbageCode27() {
        ASSERT_THROW(checkCode("int f() {\n"
                               "    return if\n"
                               "}"), InternalError);
    }

    void garbageCode28() {
        // 5702
        checkCode("struct R1 {\n"
                  "  int a;\n"
                  "  R1 () : a { }\n"
                  "};\n");
    }

    void garbageCode29() {
        // ticket #2601 segmentation fault
        checkCode("|| #if #define <=");
    }

    void garbageCode30() {
        // simply survive - a syntax error would be even better (#5867)
        checkCode("void f(int x) {\n"
                  " x = 42\n"
                  "}");
    }

    void garbageCode31() {
        ASSERT_THROW(checkCode("typedef struct{}x[([],)]typedef e y;(y,x 0){}"), InternalError);
    }

    void garbageCode32() { // #6135
        checkCode(" ( * const ( size_t ) ; foo )");
    }

    void garbageCode33() { // #6613
        ASSERT_THROW(checkCode("main(()B{});"), InternalError);

        checkCode("f::y:y : <x::");

        checkCode("\xe2u.");
    }

    // Bug #6626 crash: Token::astOperand2() const ( do while )
    void garbageCode34() {
        checkCode("void foo(void) {\n"
                  " do\n"
                  " while (0);\n"
                  "}");
        ASSERT_EQUALS("", errout.str());
    }

    void garbageCode35() {
        // ticket #2599 segmentation fault
        checkCode("sizeof");

        // ticket #2604 segmentation fault
        checkCode("sizeof <= A");
    }

    void garbageCode36() { // #6334
        checkCode("{ } < class template < > , { = } ; class... >\n"
                  "struct Y { }\n"
                  "class Types { }\n"
                  "( X < int > \"uses template\" ) ( < ( ) \"uses ; \n"
                  "( int int ::primary \"uses template\" ) int double \"uses )\n"
                  "::primary , \"uses template\" ;\n");
    }

    void garbageCode37() {
        // #5166 segmentation fault (invalid code) in lib/checkother.cpp:329 ( void * f { } void b ( ) { * f } )
        checkCode("void * f { } void b ( ) { * f }");
    }

    void garbageCode38() { // Ticket #6666
        checkCode("{ f2 { } } void f3 () { delete[] } { }");
    }

    void garbageCode39() { // #6686
        checkCode("({ (); strcat(strcat(() ()) ()) })");
    }

    void garbageCode40() { // #6620
        checkCode("{ ( ) () { virtual } ; { } E } A { : { } ( ) } * const ( ) const { }");
        // test doesn't seem to work on any platform: ASSERT_THROW(checkCode("{ ( ) () { virtual } ; { } E } A { : { } ( ) } * const ( ) const { }", "test.c"), InternalError);
    }

    void garbageCode41() { // #6685
        checkCode(" { } { return } *malloc(__SIZE_TYPE__ size); *memcpy(void n); static * const () { memcpy (*slot, 3); } { (); } { }");
    }

    void garbageCode42() { // #5760
        checkCode("{  } * const ( ) { }");
    }

    void garbageCode43() { // #6703
        checkCode("int { }; struct A<void> a = { }");
    }

    void garbageCode44() { // #6704
        ASSERT_THROW(checkCode("{ { }; }; { class A : }; public typedef b;"), InternalError);
    }

    void garbageCode45() { // #6608
        checkCode("struct true template < > { = } > struct Types \"s\" ; static_assert < int > ;");
    }

    void garbageCode46() { // #6705
        checkCode(" { bar(char *x); void foo (int ...) { struct } va_list ap; va_start(ap, size); va_arg(ap, (d)); }");
    }

    void garbageCode47() { // #6706
        checkCode(" { { }; }; * new private: B: B;");
    }

    void garbageCode48() { // #6712
        checkCode(" { d\n\" ) d ...\n\" } int main ( ) { ( ) catch ( A a ) { { } catch ( ) \"\" } }");
    }

    void garbageCode49() { // #6715
        checkCode(" ( ( ) ) { } ( { ( __builtin_va_arg_pack ( ) ) ; } ) { ( int { ( ) ( ( ) ) } ( ) { } ( ) ) += ( ) }");
    }

    void garbageCode50() { // #6718
        checkCode(" enum struct");
    }

    void garbageCode51() { // #6719
        checkCode(" (const \"C\" ...); struct base { int f2; base (int arg1, int arg2); }; global_base(0x55, 0xff); { ((global_base.f1 0x55) (global_base.f2 0xff)) { } } base::base(int arg1, int arg2) { f2 = }");
    }

    void garbageCode52() { // #6720
        checkCode("a \"b\" not_eq \"c\"");
    }

    void garbageCode53() { // #6721
        checkCode("{ { } }; void foo (struct int i) { x->b[i] = = }");
    }

    void garbageCode54() { // #6722
        ASSERT_THROW(checkCode("{ typedef long ((pf) p) (); }"), InternalError);
    }

    void garbageCode55() { // #6724
        checkCode("() __attribute__((constructor)); { } { }");
    }

    void garbageCode56() { // #6713
        checkCode("void foo() { int a = 0; int b = ???; }");
    }

    void garbageCode57() { // #6731
        ASSERT_THROW(checkCode("{ } if () try { } catch (...) B::~B { }"), InternalError);
    }

    void garbageCode58() { // #6732, #6762
        ASSERT_THROW(checkCode("{ }> {= ~A()^{} }P { }"), InternalError);
        ASSERT_THROW(checkCode("{= ~A()^{} }P { } { }> is"), InternalError);
    }

    void garbageCode59() { // #6735
        ASSERT_THROW(checkCode("{ { } }; char font8x8[256][8]"), InternalError);
    }

    void garbageCode60() { // #6736
        ASSERT_THROW(checkCode("{ } { } typedef int int_array[]; int_array &right ="), InternalError);
    }

    void garbageCode61() { // #6737
        ASSERT_THROW(checkCode("{ (const U&) }; { }; { }; struct U : virtual public"), InternalError);
    }

    void garbageCode62() { // #6738
        checkCode("(int arg2) { } { } typedef void (func_type) (int, int); typedef func_type&");
    }

    void garbageCode63() { // #6739
        ASSERT_THROW(checkCode("{ } { } typedef int u_array[]; typedef u_array &u_array_ref; (u_array_ref arg) { } u_array_ref u_array_ref_gbl_obj0"), InternalError);
    }

    void garbageCode64() { // #6740
        ASSERT_THROW(checkCode("{ } foo(void (*bar)(void))"), InternalError);
    }

    void garbageCode65() { // #6741
        ASSERT_THROW(checkCode("{ } { } typedef int u_array[]; typedef u_array &u_array_ref; (u_array_ref arg) { } u_array_ref"), InternalError);
    }

    void garbageCode66() { // #6742
        ASSERT_THROW(checkCode("{ { } }; { { } }; { }; class bar : public virtual"), InternalError);
    }

    void garbageCode67() { // #6744
        checkCode("&g[0]; { (g[0] 0) } =", false);
    }

    void garbageCode68() { // #6745
        checkCode("(int a[3]); typedef void (*fp) (void); fp");
    }

    void garbageCode69() { // #6746
        ASSERT_THROW(checkCode("{ (make_mess, aux); } typedef void F(void); aux(void (*x)()) { } (void (*y)()) { } F*"), InternalError);
    }

    void garbageCode70() { // #6747
        ASSERT_THROW(checkCode("{ } __attribute__((constructor)) void"), InternalError);
    }

    void garbageCode71() { // #6748
        ASSERT_THROW(checkCode("( ) { } typedef void noattr_t ( ) ; noattr_t __attribute__ ( )"), InternalError);
    }

    void garbageCode72() { // #6749
        ASSERT_THROW(checkCode("{ } { } typedef void voidfn(void); <voidfn&"), InternalError);
    }

    void garbageCode73() { // #6750
        ASSERT_THROW(checkCode("typedef int IRT[2]; IRT&"), InternalError);
    }

    void garbageCode74() { // #6751
        checkCode("_lenraw(const char* digits) { } typedef decltype(sizeof(0)) { } operator");
    }

    void garbageCode75() { // #6753
        checkCode("{ { void foo() { struct }; { }; } }; struct S { } f =", false);
    }

    void garbageCode76() { // #6754
        checkCode(" ( ) ( ) { ( ) [ ] } TEST ( ) { ( _broadcast_f32x4 ) ( ) ( ) ( ) ( ) if ( ) ( ) ; } E mask = ( ) [ ] ( ) res1.x =");
    }

    void garbageCode77() { // #6755
        checkCode("void foo (int **p) { { { };>= } } unsigned *d = (b b--) --*d");
    }

    void garbageCode78() { // #6756
        checkCode("( ) { [ ] } ( ) { } const_array_of_int ( ) { } typedef int A [ ] [ ] ; A a = { { } { } }");
    }

    void garbageCode79() { // #6757
        ASSERT_THROW(checkCode("{ } { } typedef void ( func_type ) ( ) ; func_type & ( )"), InternalError);
    }

    void garbageCode80() { // #6759
        ASSERT_THROW(checkCode("( ) { ; ( ) ; ( * ) [ ] ; [ ] = ( ( ) ( ) h ) ! ( ( ) ) } { ; } { } head heads [ ] = ; = & heads [ 2 ]"), InternalError);
    }

    void garbageCode81() { // #6760
        ASSERT_THROW(checkCode("{ } [ ] { ( ) } { } typedef void ( *fptr1 ) ( ) const"), InternalError);
    }

    void garbageCode82() { // #6761
        ASSERT_THROW(checkCode("p(\"Hello \" 14) _yn(const size_t) typedef bool pfunk (*pfunk)(const size_t)"), InternalError);
    }

    void garbageCode83() { // #6771
        ASSERT_THROW(checkCode("namespace A { class } class A { friend C ; } { } ;"), InternalError);
    }

    void garbageCode84() { // #6780
        checkCode("int main ( [ ] ) { " " [ ] ; int i = 0 ; do { } ; } ( [ ] ) { }"); // do not crash
    }

    void garbageCode85() { // #6784
        ASSERT_THROW(checkCode("{ } { } typedef void ( *VoidFunc() ) ( ) ; VoidFunc"), InternalError); // do not crash
    }

    void garbageCode86() { // #6785
        ASSERT_THROW(checkCode("{ } typedef char ( *( X ) ( void) , char ) ;"), InternalError); // do not crash
    }

    void garbageCode87() { // #6788
        checkCode("((X (128))) (int a) { v[ = {} (x 42) a] += }"); // do not crash
    }

    void garbageCode88() { // #6786
        ASSERT_THROW(checkCode("( ) { ( 0 ) { ( ) } } g ( ) { i( ( false ?) ( ) : 1 ) ; } ;"), InternalError); // do not crash
    }

    void garbageCode89() { // #6772
        ASSERT_THROW(checkCode("{ { ( ) } P ( ) ^ { } { } { } ( ) } 0"), InternalError); // do not crash
    }

    void garbageCode90() { // #6790
        ASSERT_THROW(checkCode("{ } { } typedef int u_array [[ ] ; typedef u_array & u_array_ref] ( ) { } u_array_ref_gbl_obj0"), InternalError); // do not crash
    }

    void garbageCode91() { // #6791
        checkCode("typedef __attribute__((vector_size (16))) { return[ (v2df){ } ;] }"); // do not crash
    }

    void garbageCode92() { // #6792
        ASSERT_THROW(checkCode("template < typename _Tp ( ( ) ; _Tp ) , decltype > { } { ( ) ( ) }"), InternalError); // do not crash
    }

    void garbageCode93() { // #6800
        checkCode(" namespace A { } class A{ { }} class A : T ;", false); // do not crash
    }

    void garbageCode94() { // #6803
        //checkCode("typedef long __m256i __attribute__ ( ( ( ) ) )[ ; ( ) { } typedef __m256i __attribute__ ( ( ( ) ) ) < ] ( ) { ; }");
        ASSERT_THROW(checkCode("typedef long __m256i __attribute__ ( ( ( ) ) )[ ; ( ) { } typedef __m256i __attribute__ ( ( ( ) ) ) < ] ( ) { ; }"), InternalError);
    }

    void garbageCode95() { // #6804
        checkCode("{ } x x ; { } h h [ ] ( ) ( ) { struct x ( x ) ; int __attribute__ ( ) f ( ) { h - > first = & x ; struct x * n = h - > first ; ( ) n > } }");    // do not crash
    }

    void garbageCode96() { // #6807
        ASSERT_THROW(checkCode("typedef J J[ ; typedef ( ) ( ) { ; } typedef J J ;] ( ) ( J cx ) { n } ;"), InternalError);
    }

    void garbageCode97() { // #6808
        ASSERT_THROW(checkCode("namespace A {> } class A{ { }} class A : T< ;"), InternalError);
    }

    void garbageCode98() { // #6838
        ASSERT_THROW(checkCode("for (cocon To::ta@Taaaaaforconst oken aaaaaaaaaaaa5Dl()\n"
                               "const unsiged in;\n"
                               "fon *tok = f);.s(Token i = d-)L;"), InternalError);
    }

    void garbageCode99() { // #6726
        ASSERT_THROW(checkCode("{ xs :: i(:) ! ! x/5 ! !\n"
                               "i, :: a :: b integer, } foo2(x) :: j(:) \n"
                               "b type(*), d(:), a x :: end d(..), foo end\n"
                               "foo4 b d(..), a a x type(*), b foo2 b"), InternalError);
    }

    void garbageCode100() { // #6840
        checkCode("( ) { ( i< ) } int foo ( ) { int i ; ( for ( i => 1 ) ; ) }");
    }

    void garbageCode101() { // #6835
        // Reported case
        checkCode("template < class , =( , int) X = 1 > struct A { } ( ) { = } [ { } ] ( ) { A < void > 0 }");
        // Reduced case
        checkCode("template < class =( , ) X = 1> struct A {}; A<void> a;");
    }

    void garbageCode102() { // #6846
        checkCode("struct Object { ( ) ; Object & operator= ( Object ) { ( ) { } if ( this != & b ) } }");
    }

    void garbageCode103() { // #6824
        ASSERT_THROW(checkCode("a f(r) int * r; { { int s[2]; [f(s); if () ]  } }"), InternalError);
    }

    void garbageCode104() { // #6847
        ASSERT_THROW(checkCode("template < Types > struct S {> ( S < ) S >} { ( ) { } } ( ) { return S < void > ( ) } { ( )> >} { ( ) { } } ( ) { ( ) }"), InternalError);
    }

    void garbageCode105() { // #6859
        checkCode("void foo (int i) { int a , for (a 1; a( < 4; a++) if (a) (b b++) (b);) n++; }");
    }

    void garbageCode106() { // #6880
        ASSERT_THROW(checkCode("[ ] typedef typedef b_array b_array_ref [ ; ] ( ) b_array_ref b_array_ref_gbl_obj0 { ; { b_array_ref b_array_ref_gbl_obj0 } }"), InternalError);
    }

    void garbageCode107() { // #6881
        ASSERT_THROW(checkCode("enum { val = 1{ }; { const} }; { } Bar { const int A = val const } ;"), InternalError);
    }

    void garbageCode108() { //  #6895 "segmentation fault (invalid code) in CheckCondition::isOppositeCond"
        checkCode("A( ) { } bool f( ) { ( ) F; ( ) { ( == ) if ( !=< || ( !A( ) && r[2] ) ) ( !A( ) ) ( ) } }");
    }

    void garbageCode109() { //  #6900 "segmentation fault (invalid code) in CheckStl::runSimplifiedChecks"
        checkCode("( *const<> (( ) ) { } ( *const ( ) ( ) ) { } ( * const<> ( size_t )) ) { } ( * const ( ) ( ) ) { }");
    }

    void garbageCode110() { //  #6902 "segmentation fault (invalid code) in CheckStl::string_c_str"
        checkCode("( *const<> ( size_t ) ; foo ) { } * ( *const ( size_t ) ( ) ;> foo )< { }");
    }

    void garbageCode111() { //  #6907
        ASSERT_THROW(checkCode("enum { FOO = 1( ,) } {{ FOO }} ;"), InternalError);
    }

    void garbageCode112() { //  #6909
        ASSERT_THROW(checkCode("enum { FOO = ( , ) } {{ }}>> enum { FOO< = ( ) } { { } } ;"), InternalError);
    }

    void garbageCode113() { //  #6858
        checkCode("*(*const<> (size_t); foo) { } *(*const (size_t)() ; foo) { }");
    }

    void garbageCode114() { // #2118
        ASSERT_THROW(checkCode("Q_GLOBAL_STATIC_WITH_INITIALIZER(Qt4NodeStaticData, qt4NodeStaticData, {\n"
                               "    for (unsigned i = 0 ; i < count; i++) {\n"
                               "    }\n"
                               "});"), InternalError);
    }

    void garbageCode115() { // #5506
        checkCode("A template < int { int = -1 ; } template < int N > struct B { int [ A < N > :: zero ] ;  } ; B < 0 > b ;");
    }

    void garbageCode116() { // #5356
        checkCode("struct template<int { = }; > struct B { }; B < 0 > b;");
    }

    void garbageCode117() { // #6121
        ASSERT_THROW(checkCode("enum E { f = {} };\n"
                               "int a = f;"), InternalError);
    }

    void garbageCode118() { // #5600 - missing include causes invalid enum
        ASSERT_THROW(checkCode("enum {\n"
                               "    NUM_OPCODES = \n"
                               // #include "definition"
                               "};\n"
                               "struct bytecode {};\n"
                               "jv jq_next() { opcode = ((opcode) +NUM_OPCODES);\n"
                               "}"), InternalError);
    }

    void garbageCode119() { // #5598
        checkCode("{ { void foo() { struct }; template <typename> struct S { Used x; void bar() } auto f = [this] { }; } };");
    }

    void garbageCode120() { // #4927
        checkCode("int main() {\n"
                  "   return 0\n"
                  "}");
        ASSERT_EQUALS("", errout.str());
    }

    void garbageCode121() { // #2585
        checkCode("abcdef?""?<"
                  "123456?""?>"
                  "+?""?=");
        ASSERT_EQUALS("", errout.str());
    }

    void garbageCode122() { // #6303
        checkCode("void foo() {\n"
                  "char *a = malloc(10);\n"
                  "a[0]\n"
                  "}");
    }

    void garbageCode123() {
        checkCode("namespace pr16989 {\n"
                  "    class C {\n"
                  "        C tpl_mem(T *) { return }\n"
                  "    };\n"
                  "}");
    }

    void garbageCode124() {
        checkCode("+---+");
    }

    void garbageValueFlow() {
        // #6089
        const char* code = "{} int foo(struct, x1, struct x2, x3, int, x5, x6, x7)\n"
                           "{\n"
                           "    (foo(s, , 2, , , 5, , 7)) abort()\n"
                           "}\n";
        checkCode(code);

        // #6106
        code = " f { int i ; b2 , [ ] ( for ( i = 0 ; ; ) ) }";
        checkCode(code);

        // 6122 survive garbage code
        code = "; { int i ; for ( i = 0 ; = 123 ; ) - ; }";
        checkCode(code);

        code = "void f1() { for (int n = 0 n < 10 n++); }";
        checkCode(code);
    }

    void garbageSymbolDatabase() {
        checkCode("void f( { u = 1 ; } ) { }");

        checkCode("{ }; void namespace A::f; { g() { int } }");

        ASSERT_THROW(checkCode("class Foo {}; class Bar : public Foo"), InternalError);

        checkCode("YY_DECL { switch (yy_act) {\n"
                  "    case 65: YY_BREAK\n"
                  "    case YY_STATE_EOF(block):\n"
                  "        yyterminate(); \n"
                  "} }"); // #5663
    }

    void garbageAST() {
        checkCode("--"); // don't crash

        checkCode("N 1024 float a[N], b[N + 3], c[N]; void N; (void) i;\n"
                  "int #define for (i = avx_test i < c[i]; i++)\n"
                  "b[i + 3] = a[i] * {}"); // Don't hang (#5787)

        checkCode("START_SECTION([EXTRA](bool isValid(const String &filename)))"); // Don't crash (#5991)
    }

    void templateSimplifierCrashes() {
        checkCode( // #5950
            "struct A { \n"
            "  template <class T> operator T*();\n"
            "}; \n"
            "\n"
            "template <> A::operator char*(){ return 0; } // specialization\n"
            "\n"
            "int main() { \n"
            "  A a;\n"
            "  int *ip = a.operator int*();\n"
            "}\n"
            "\n"
            "namespace PR5742 {\n"
            "  template <class T> struct A { };\n"
            "  struct S {\n"
            "    template <class T> operator T();\n"
            "  } s;\n"
            "  void f() {\n"
            "    s.operator A<A<int> >();\n"
            "  }\n"
            "}\n");

        checkCode( // #6034
            "template<template<typename...> class T, typename... Args>\n"
            "struct foo<T<Args...> > {\n"
            "    const bool value = true;\n"
            "};\n"
            "\n"
            "template<int I>\n"
            "struct int_\n"
            "{};\n"
            "\n"
            "int main() {\n"
            "  foo<int_<0> >::value;\n"
            "}\n"
        );

        checkCode(" > template < . > struct Y < T > { = } ;\n"); // #6108

        checkCode( // #6117
            "template <typename ...> struct something_like_tuple\n"
            "{};\n"
            "template <typename, typename> struct is_last {\n"
            "  static const bool value = false;\n"
            "};\n"
            "template <typename T, template <typename ...> class Tuple, typename ... Head>\n"
            "struct is_last<T, Tuple<Head ..., T>>\n"
            "{\n"
            "  static const bool value = true;\n"
            "};\n"
            "\n"
            "#define SA(X) static_assert (X, #X)\n"
            "\n"
            "typedef something_like_tuple<char, int, float> something_like_tuple_t;\n"
            "SA ((is_last<float, something_like_tuple_t>::value == false));\n"
            "SA ((is_last<int, something_like_tuple_t>::value == false));\n"
        );

        checkCode( // #6225
            "template <typename...>\n"
            "void templ_fun_with_ty_pack() {}\n"
            " \n"
            "namespace PR20047 {\n"
            "        template <typename T>\n"
            "        struct A {};\n"
            "        using AliasA = A<T>;\n"
            "}\n"
        );

        // #3449
        ASSERT_EQUALS("template < typename T > struct A ;\n"
                      "struct B { template < typename T > struct C } ;\n"
                      "{ } ;",
                      checkCode("template<typename T> struct A;\n"
                                "struct B { template<typename T> struct C };\n"
                                "{};"));

        // #4169
        checkCode("volatile true , test < test < #ifdef __ppc__ true ,");
    }
};

REGISTER_TEST(TestGarbage)
