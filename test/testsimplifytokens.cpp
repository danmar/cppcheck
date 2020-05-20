/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include <ostream>
#include <string>


class TestSimplifyTokens : public TestFixture {
public:
    TestSimplifyTokens() : TestFixture("TestSimplifyTokens") {
    }


private:
    Settings settings0;
    Settings settings_std;
    Settings settings_windows;

    void run() OVERRIDE {
        LOAD_LIB_2(settings_std.library, "std.cfg");
        LOAD_LIB_2(settings_windows.library, "windows.cfg");
        settings0.addEnabled("portability");
        settings_windows.addEnabled("portability");

        TEST_CASE(combine_strings);
        TEST_CASE(combine_wstrings);
        TEST_CASE(combine_ustrings);
        TEST_CASE(combine_Ustrings);
        TEST_CASE(combine_u8strings);
        TEST_CASE(combine_mixedstrings);

        TEST_CASE(double_plus);
        TEST_CASE(redundant_plus);
        TEST_CASE(redundant_plus_numbers);
        TEST_CASE(declareVar);

        TEST_CASE(declareArray);

        TEST_CASE(elseif1);

        TEST_CASE(namespaces);

        // Simplify "not" to "!" (#345)
        TEST_CASE(not1);

        // Simplify "and" to "&&" (#620)
        TEST_CASE(and1);

        // Simplify "or" to "||"
        TEST_CASE(or1);

        TEST_CASE(cAlternativeTokens);

        TEST_CASE(comma_keyword);

        // Simplify calculations
        TEST_CASE(calculations);

        TEST_CASE(simplifyOperator1);
        TEST_CASE(simplifyOperator2);

        TEST_CASE(simplifyArrayAccessSyntax);

        TEST_CASE(duplicateDefinition); // ticket #3565

        // Tokenizer::simplifyInitVar
        TEST_CASE(simplifyInitVar);

        // struct ABC { } abc; => struct ABC { }; ABC abc;
        TEST_CASE(simplifyStructDecl1);
        TEST_CASE(simplifyStructDecl2); // ticket #2579
        TEST_CASE(simplifyStructDecl3);
        TEST_CASE(simplifyStructDecl4);
        TEST_CASE(simplifyStructDecl6); // ticket #3732
        TEST_CASE(simplifyStructDecl7); // ticket #476 (static anonymous struct array)
        TEST_CASE(simplifyStructDecl8); // ticket #7698

        // register int var; => int var;
        // inline int foo() {} => int foo() {}
        TEST_CASE(removeUnwantedKeywords);

        // remove calling convention __cdecl, __stdcall, ...
        TEST_CASE(simplifyCallingConvention);

        TEST_CASE(simplifyFunctorCall);

        TEST_CASE(simplifyFunctionPointer); // ticket #5339 (simplify function pointer after comma)

        TEST_CASE(redundant_semicolon);

        TEST_CASE(simplifyFunctionReturn);

        // void foo(void) -> void foo()
        TEST_CASE(removeVoidFromFunction);

        TEST_CASE(consecutiveBraces);

        TEST_CASE(simplifyOverride); // ticket #5069
        TEST_CASE(simplifyNestedNamespace);
        TEST_CASE(simplifyNamespaceAliases);
    }

    std::string tok(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Native) {
        errout.str("");

        settings0.platform(type);
        Tokenizer tokenizer(&settings0, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        bool modified = true;
        while (modified) {
            modified = false;
            modified |= tokenizer.simplifyCalculations();
            modified |= tokenizer.simplifyRedundantParentheses();
        }

        return tokenizer.tokens()->stringifyList(nullptr, !simplify);
    }

    std::string tokWithWindows(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Native) {
        errout.str("");

        settings_windows.platform(type);
        Tokenizer tokenizer(&settings_windows, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        return tokenizer.tokens()->stringifyList(nullptr, !simplify);
    }

    std::string tok(const char code[], const char filename[]) {
        errout.str("");

        Tokenizer tokenizer(&settings0, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        return tokenizer.tokens()->stringifyList(nullptr, false);
    }

    std::string tokWithNewlines(const char code[]) {
        errout.str("");

        Tokenizer tokenizer(&settings0, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        return tokenizer.tokens()->stringifyList(false, false, false, true, false);
    }

    std::string tokWithStdLib(const char code[]) {
        errout.str("");

        Tokenizer tokenizer(&settings_std, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        return tokenizer.tokens()->stringifyList(nullptr, false);
    }

    std::string tokenizeDebugListing(const char code[], const char filename[] = "test.cpp") {
        errout.str("");

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // result..
        return tokenizer.tokens()->stringifyList(true);
    }


    void combine_strings() {
        const char code1[] =  "void foo()\n"
                              "{\n"
                              "const char *a =\n"
                              "{\n"
                              "\"hello \"\n"
                              "\"world\"\n"
                              "};\n"
                              "}\n";

        const char code2[] =  "void foo()\n"
                              "{\n"
                              "const char *a =\n"
                              "{\n"
                              "\"hello world\"\n"
                              "};\n"
                              "}\n";
        ASSERT_EQUALS(tok(code2), tok(code1));

        const char code3[] = "x = L\"1\" TEXT(\"2\") L\"3\";";
        ASSERT_EQUALS("x = L\"123\" ;", tok(code3, false, Settings::Win64));

        const char code4[] = "x = TEXT(\"1\") L\"2\";";
        ASSERT_EQUALS("x = L\"1\" L\"2\" ;", tok(code4, false, Settings::Win64));
    }

    void combine_wstrings() {
        const char code[] =  "a = L\"hello \"  L\"world\" ;\n";

        const char expected[] =  "a = L\"hello world\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_ustrings() {
        const char code[] =  "abcd = u\"ab\" u\"cd\";";

        const char expected[] =  "abcd = u\"abcd\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_Ustrings() {
        const char code[] =  "abcd = U\"ab\" U\"cd\";";

        const char expected[] =  "abcd = U\"abcd\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_u8strings() {
        const char code[] =  "abcd = u8\"ab\" u8\"cd\";";

        const char expected[] =  "abcd = u8\"abcd\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_mixedstrings() {
        const char code[] = "abcdef = \"ab\" L\"cd\" \"ef\";";

        const char expected[] =  "abcdef = L\"abcdef\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void double_plus() {
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a++;\n"
                                  "a--;\n"
                                  "++a;\n"
                                  "--a;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a ++ ; a -- ; ++ a ; -- a ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a+a;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + a ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a+++b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a ++ + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a---b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a -- - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a--+b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a -- + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a++-b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a ++ - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a+--b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + -- b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a-++b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - ++ b ; }", tok(code1));
        }
    }

    void redundant_plus() {
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a + + b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a + + + b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a + - b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a - + b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a - - b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a - + - b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                  "{\n"
                                  "a=a - - - b;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - b ; }", tok(code1));
        }
    }

    void redundant_plus_numbers() {
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a + + 1;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a + + + 1;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a + - 1;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a - 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a - + 1;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a - 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a - - 1;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a - + - 1;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a - - - 1;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a - 1 ; }", tok(code1));
        }
    }

    void declareVar() {
        const char code[] = "void f ( ) { char str [ 100 ] = \"100\" ; }";
        ASSERT_EQUALS(code, tok(code));
    }

    void declareArray() {
        const char code1[] = "void f ( ) { char str [ ] = \"100\" ; }";
        const char expected1[] = "void f ( ) { char str [ 4 ] = \"100\" ; }";
        ASSERT_EQUALS(expected1, tok(code1));

        const char code2[] = "char str [ ] = \"\\x00\";";
        const char expected2[] = "char str [ 2 ] = \"\\0\" ;";
        ASSERT_EQUALS(expected2, tok(code2));

        const char code3[] = "char str [ ] = \"\\0\";";
        const char expected3[] = "char str [ 2 ] = \"\\0\" ;";
        ASSERT_EQUALS(expected3, tok(code3));

        const char code4[] = "char str [ ] = \"\\n\\n\";";
        const char expected4[] = "char str [ 3 ] = \"\\n\\n\" ;";
        ASSERT_EQUALS(expected4, tok(code4));
    }

    void elseif1() {
        const char code[] = "void f(){ if(x) {} else if(ab) { cd } else { ef }gh; }";
        ASSERT_EQUALS("\n\n##file 0\n1: void f ( ) { if ( x ) { } else { if ( ab ) { cd } else { ef } } gh ; }\n", tokenizeDebugListing(code));

        // syntax error: assert there is no segmentation fault
        ASSERT_EQUALS("\n\n##file 0\n1: void f ( ) { if ( x ) { } else { if ( x ) { } } }\n", tokenizeDebugListing("void f(){ if(x) {} else if (x) { } }"));

        {
            const char src[] =  "void f(int g,int f) {\n"
                                "if(g==1) {poo();}\n"
                                "else if( g == 2 )\n"
                                "{\n"
                                " if( f == 0 ){coo();}\n"
                                " else if( f==1)\n"
                                "  goo();\n"
                                "}\n"
                                "}";

            const char expected[] = "void f ( int g , int f ) "
                                    "{ "
                                    "if ( g == 1 ) { poo ( ) ; } "
                                    "else { "
                                    "if ( g == 2 ) "
                                    "{ "
                                    "if ( f == 0 ) { coo ( ) ; } "
                                    "else { "
                                    "if ( f == 1 ) "
                                    "{ "
                                    "goo ( ) ; "
                                    "} "
                                    "} "
                                    "} "
                                    "} "
                                    "}";
            ASSERT_EQUALS(tok(expected), tok(src));
        }

        // Ticket #6860 - lambdas
        {
            const char src[] = "( []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() );";
            const char expected[] = "\n\n##file 0\n1: ( [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) ) ;\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
        {
            const char src[] = "[ []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() ];";
            const char expected[] = "\n\n##file 0\n1: [ [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) ] ;\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
        {
            const char src[] = "= { []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() }";
            const char expected[] = "\n\n##file 0\n1: = { [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
    }



    void namespaces() {
        {
            const char code[] = "namespace std { }";

            ASSERT_EQUALS("", tok(code));
        }

        {
            const char code[] = "; namespace std { }";

            ASSERT_EQUALS(";", tok(code));
        }

        {
            const char code[] = "using namespace std; namespace a{ namespace b{ void f(){} } }";

            const char expected[] = "namespace a { namespace b { void f ( ) { } } }";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "namespace b{ void f(){} }";

            const char expected[] = "namespace b { void f ( ) { } }";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f(int namespace) { }";

            const char expected[] = "void f ( int namespace ) { }";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void not1() {
        ASSERT_EQUALS("void f ( ) { if ( ! p ) { ; } }", tok("void f() { if (not p); }", "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( p && ! q ) { ; } }", tok("void f() { if (p && not q); }", "test.c"));
        ASSERT_EQUALS("void f ( ) { a = ! ( p && q ) ; }", tok("void f() { a = not(p && q); }", "test.c"));
        // Don't simplify 'not' or 'compl' if they are defined as a type;
        // in variable declaration and in function declaration/definition
        ASSERT_EQUALS("struct not { int x ; } ;", tok("struct not { int x; };", "test.c"));
        ASSERT_EQUALS("void f ( ) { not p ; compl c ; }", tok(" void f() { not p; compl c; }", "test.c"));
        ASSERT_EQUALS("void foo ( not i ) ;", tok("void foo(not i);", "test.c"));
        ASSERT_EQUALS("int foo ( not i ) { return g ( i ) ; }", tok("int foo(not i) { return g(i); }", "test.c"));
    }

    void and1() {
        ASSERT_EQUALS("void f ( ) { if ( p && q ) { ; } }",
                      tok("void f() { if (p and q) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) && q ) { ; } }",
                      tok("void f() { if (foo() and q) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) && bar ( ) ) { ; } }",
                      tok("void f() { if (foo() and bar()) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( p && bar ( ) ) { ; } }",
                      tok("void f() { if (p and bar()) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( p && ! q ) { ; } }",
                      tok("void f() { if (p and not q) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { r = a && b ; }",
                      tok("void f() { r = a and b; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { r = ( a || b ) && ( c || d ) ; }",
                      tok("void f() { r = (a || b) and (c || d); }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( test1 [ i ] == 'A' && test2 [ i ] == 'C' ) { } }",
                      tok("void f() { if (test1[i] == 'A' and test2[i] == 'C') {} }", "test.c"));
    }

    void or1() {
        ASSERT_EQUALS("void f ( ) { if ( p || q ) { ; } }",
                      tok("void f() { if (p or q) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) || q ) { ; } }",
                      tok("void f() { if (foo() or q) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) || bar ( ) ) { ; } }",
                      tok("void f() { if (foo() or bar()) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( p || bar ( ) ) { ; } }",
                      tok("void f() { if (p or bar()) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { if ( p || ! q ) { ; } }",
                      tok("void f() { if (p or not q) ; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { r = a || b ; }",
                      tok("void f() { r = a or b; }", "test.c"));

        ASSERT_EQUALS("void f ( ) { r = ( a && b ) || ( c && d ) ; }",
                      tok("void f() { r = (a && b) or (c && d); }", "test.c"));
    }

    void cAlternativeTokens() {
        ASSERT_EQUALS("void f ( ) { err |= ( ( r & s ) && ! t ) ; }",
                      tok("void f() { err or_eq ((r bitand s) and not t); }", "test.c"));
        ASSERT_EQUALS("void f ( ) const { r = f ( a [ 4 ] | 0x0F , ~ c , ! d ) ; }",
                      tok("void f() const { r = f(a[4] bitor 0x0F, compl c, not d) ; }", "test.c"));

    }

    void comma_keyword() {
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    struct A *a, *b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { struct A * a ; struct A * b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    struct A **a, **b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { struct A * * a ; struct A * * b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    delete a, b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a , b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b, *c;\n"
                                "    delete a, b, c;\n"
                                "}\n";
            // delete a; b; c; would be better but this will do too
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; char * c ; delete a , b , c ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    if (x) \n"
                                "        delete a, b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; if ( x ) { delete a , b ; } }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char **a, **b, **c;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * * a ; char * * b ; char * * c ; }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    for(int a,b; a < 10; a = a + 1, b = b + 1);\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { for ( int a , b ; a < 10 ; a = a + 1 , b = b + 1 ) { ; } }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    char buf[BUFSIZ], **p;\n"
                                "    char *ptrs[BUFSIZ], **pp;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { char buf [ BUFSIZ ] ; char * * p ; char * ptrs [ BUFSIZ ] ; char * * pp ; }", tok(code));
        }

        {
            // #4786 - don't replace , with ; in ".. : public B, C .." code
            const char code[] = "template < class T = X > class A : public B , C { } ;";
            ASSERT_EQUALS(code, tok(code));
        }
    }

    void calculations() {
        {
            const char code[] = "a[i+8+2];";
            ASSERT_EQUALS("a [ i + 10 ] ;", tok(code));
        }
        {
            const char code[] = "a[8+2+i];";
            ASSERT_EQUALS("a [ 10 + i ] ;", tok(code));
        }
        {
            const char code[] = "a[i + 2 * (2 * 4)];";
            ASSERT_EQUALS("a [ i + 16 ] ;", tok(code));
        }
        {
            const char code[] = "a[i + 100 - 90];";
            ASSERT_EQUALS("a [ i + 10 ] ;", tok(code));
        }
        {
            const char code[] = "a[1+1+1+1+1+1+1+1+1+1-2+5-3];";
            ASSERT_EQUALS("a [ 10 ] ;", tok(code));
        }
        {
            const char code[] = "a[10+10-10-10];";
            ASSERT_EQUALS("a [ 0 ] ;", tok(code));
        }

        ASSERT_EQUALS("a [ 4 ] ;", tok("a[1+3|4];"));
        ASSERT_EQUALS("a [ 4U ] ;", tok("a[1+3|4U];"));
        ASSERT_EQUALS("a [ 3 ] ;", tok("a[1+2&3];"));
        ASSERT_EQUALS("a [ 3U ] ;", tok("a[1+2&3U];"));
        ASSERT_EQUALS("a [ 5 ] ;", tok("a[1-0^4];"));
        ASSERT_EQUALS("a [ 5U ] ;", tok("a[1-0^4U];"));

        ASSERT_EQUALS("x = 1 + 2 * y ;", tok("x=1+2*y;"));
        ASSERT_EQUALS("x = 7 ;", tok("x=1+2*3;"));
        ASSERT_EQUALS("x = 47185 ;", tok("x=(65536*72/100);"));
        ASSERT_EQUALS("x = 900 ;", tok("x = 1500000 / ((145000 - 55000) * 1000 / 54000);"));
        ASSERT_EQUALS("int a [ 8 ] ;", tok("int a[5+6/2];"));
        ASSERT_EQUALS("int a [ 4 ] ;", tok("int a[(10)-1-5];"));
        ASSERT_EQUALS("int a [ i - 9 ] ;", tok("int a[i - 10 + 1];"));
        ASSERT_EQUALS("int a [ i - 11 ] ;", tok("int a[i - 10 - 1];"));

        ASSERT_EQUALS("x = y ;", tok("x=0+y+0-0;"));
        ASSERT_EQUALS("x = 0 ;", tok("x=0*y;"));

        ASSERT_EQUALS("x = 501 ;", tok("x = 1000 + 2 >> 1;"));
        ASSERT_EQUALS("x = 125 ;", tok("x = 1000 / 2 >> 2;"));

        {
            // Ticket #1997
            const char code[] = "void * operator new[](size_t);";
            ASSERT_EQUALS("void * operatornew[] ( long ) ;", tok(code, true, Settings::Win32A));
        }

        ASSERT_EQUALS("; a [ 0 ] ;", tok(";a[0*(*p)];"));

        ASSERT_EQUALS("; ;", tok("; x = x + 0;"));

        ASSERT_EQUALS("{ if ( a == 2 ) { } }", tok("{if (a==1+1){}}"));
        ASSERT_EQUALS("{ if ( a + 2 != 6 ) { } }", tok("{if (a+1+1!=1+2+3){}}"));
        ASSERT_EQUALS("{ if ( 4 < a ) { } }", tok("{if (14-2*5<a*4/(2*2)){}}"));

        ASSERT_EQUALS("( y / 2 - 2 ) ;", tok("(y / 2 - 2);"));
        ASSERT_EQUALS("( y % 2 - 2 ) ;", tok("(y % 2 - 2);"));

        ASSERT_EQUALS("( 4 ) ;", tok("(1 * 2 / 1 * 2);")); // #3722

        ASSERT_EQUALS("x ( 60129542144 ) ;", tok("x(14<<4+17<<300%17);")); // #4931
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(8|5&6+0 && 7);")); // #6104
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(2 && 4<<4<<5 && 4);")); // #4933
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(9&&8%5%4/3);")); // #4931
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(2 && 2|5<<2%4);")); // #4931
        ASSERT_EQUALS("x ( -2 << 6 | 1 ) ;", tok("x(1-3<<6|5/3);")); // #4931
        ASSERT_EQUALS("x ( 2 ) ;", tok("x(2|0*0&2>>1+0%2*1);")); // #4931
        ASSERT_EQUALS("x ( 0 & 4 != 1 ) ;", tok("x(4%1<<1&4!=1);")); // #4931 (can be simplified further but it's not a problem)
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(0&&4>0==2||4);")); // #4931

        // don't remove these spaces..
        ASSERT_EQUALS("new ( auto ) ( 4 ) ;", tok("new (auto)(4);"));
    }

    void simplifyOperator1() {
        // #3237 - error merging namespaces with operators
        const char code[] = "class c {\n"
                            "public:\n"
                            "    operator std::string() const;\n"
                            "    operator string() const;\n"
                            "};\n";
        const char expected[] = "class c { "
                                "public: "
                                "operatorstd::string ( ) const ; "
                                "operatorstring ( ) const ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyOperator2() {
        // #6576
        ASSERT_EQUALS("template < class T > class SharedPtr { "
                      "SharedPtr & operator= ( SharedPtr < Y > const & r ) ; "
                      "} ; "
                      "class TClass { "
                      "public: TClass & operator= ( const TClass & rhs ) ; "
                      "} ; "
                      "TClass :: TClass ( const TClass & other ) { operator= ( other ) ; }",
                      tok("template<class T>\n"
                          "    class SharedPtr {\n"
                          "    SharedPtr& operator=(SharedPtr<Y> const & r);\n"
                          "};\n"
                          "class TClass {\n"
                          "public:\n"
                          "    TClass& operator=(const TClass& rhs);\n"
                          "};\n"
                          "TClass::TClass(const TClass &other) {\n"
                          "    operator=(other);\n"
                          "}"));
    }

    void simplifyArrayAccessSyntax() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int a@1 ; a@1 [ 13 ] ;\n", tokenizeDebugListing("int a; 13[a];"));
    }

    void duplicateDefinition() { // #3565 - wrongly detects duplicate definition
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr("{ x ; return a not_eq x; }");
        tokenizer.tokenize(istr, "test.c");
        Token *x_token = tokenizer.list.front()->tokAt(5);
        ASSERT_EQUALS(false, tokenizer.duplicateDefinition(&x_token));
    }

    void simplifyInitVar() {
        // ticket #1005 - int *p(0); => int *p = 0;
        {
            const char code[] = "void foo() { int *p(0); }";
            ASSERT_EQUALS("void foo ( ) { int * p ; p = 0 ; }", tok(code));
        }

        {
            const char code[] = "void foo() { int p(0); }";
            ASSERT_EQUALS("void foo ( ) { int p ; p = 0 ; }", tok(code));
        }

        {
            const char code[] = "void a() { foo *p(0); }";
            ASSERT_EQUALS("void a ( ) { foo * p ; p = 0 ; }", tok(code));
        }
    }

    void simplifyStructDecl1() {
        {
            const char code[] = "struct ABC { } abc;";
            const char expected[] = "struct ABC { } ; struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC { } * pabc;";
            const char expected[] = "struct ABC { } ; struct ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC { } abc[4];";
            const char expected[] = "struct ABC { } ; struct ABC abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC { } abc, def;";
            const char expected[] = "struct ABC { } ; struct ABC abc ; struct ABC def ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC { } abc, * pabc;";
            const char expected[] = "struct ABC { } ; struct ABC abc ; struct ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC { struct DEF {} def; } abc;";
            const char expected[] = "struct ABC { struct DEF { } ; struct DEF def ; } ; struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { } abc;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { } * pabc;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { } abc[4];";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct {int a;} const array[3] = {0};";
            const char expected[] = "struct Anonymous0 { int a ; } ; struct Anonymous0 const array [ 3 ] = { 0 } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "static struct {int a;} const array[3] = {0};";
            const char expected[] = "struct Anonymous0 { int a ; } ; static struct Anonymous0 const array [ 3 ] = { 0 } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { } abc, def;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc ; struct Anonymous0 def ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { } abc, * pabc;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc ; struct Anonymous0 * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { struct DEF {} def; } abc;";
            const char expected[] = "struct Anonymous0 { struct DEF { } ; struct DEF def ; } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC { struct {} def; } abc;";
            const char expected[] = "struct ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ; struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { struct {} def; } abc;";
            const char expected[] = "struct Anonymous0 { struct Anonymous1 { } ; struct Anonymous1 def ; } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "union ABC { int i; float f; } abc;";
            const char expected[] = "union ABC { int i ; float f ; } ; union ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC { struct {} def; };";
            const char expected[] = "struct ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct ABC : public XYZ { struct {} def; };";
            const char expected[] = "struct ABC : public XYZ { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { int x; }; int y;";
            const char expected[] = "int x ; int y ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { int x; };";
            const char expected[] = "int x ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { };";
            const char expected[] = ";";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { struct { struct { } ; } ; };";
            const char expected[] = ";";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        // ticket 2464
        {
            const char code[] = "static struct ABC { } abc ;";
            const char expected[] = "struct ABC { } ; static struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        // ticket #980
        {
            const char code[] = "void f() { int A(1),B(2),C=3,D,E(5),F=6; }";
            const char expected[] = "void f ( ) { int A ; A = 1 ; int B ; B = 2 ; int C ; C = 3 ; int D ; int E ; E = 5 ; int F ; F = 6 ; }";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        // ticket #8284
        {
            const char code[] = "void f() { class : foo<int> { } abc; }";
            const char expected[] = "void f ( ) { class Anonymous0 : foo < int > { } ; Anonymous0 abc ; }";
            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyStructDecl2() { // ticket #2479 (segmentation fault)
        const char code[] = "struct { char c; }";
        const char expected[] = "struct { char c ; }";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyStructDecl3() {
        {
            const char code[] = "class ABC { } abc;";
            const char expected[] = "class ABC { } ; ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC { } * pabc;";
            const char expected[] = "class ABC { } ; ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC { } abc[4];";
            const char expected[] = "class ABC { } ; ABC abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC { } abc, def;";
            const char expected[] = "class ABC { } ; ABC abc ; ABC def ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC { } abc, * pabc;";
            const char expected[] = "class ABC { } ; ABC abc ; ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC { class DEF {} def; } abc;";
            const char expected[] = "class ABC { class DEF { } ; DEF def ; } ; ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { } abc;";
            const char expected[] = "class { } abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { } * pabc;";
            const char expected[] = "class { } * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { } abc[4];";
            const char expected[] = "class { } abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { } abc, def;";
            const char expected[] = "class { } abc , def ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { } abc, * pabc;";
            const char expected[] = "class { } abc , * pabc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct { class DEF {} def; } abc;";
            const char expected[] = "struct Anonymous0 { class DEF { } ; DEF def ; } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC { struct {} def; } abc;";
            const char expected[] = "class ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ; ABC abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { class {} def; } abc;";
            const char expected[] = "class { class { } def ; } abc ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC { struct {} def; };";
            const char expected[] = "class ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class ABC : public XYZ { struct {} def; };";
            const char expected[] = "class ABC : public XYZ { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { int x; }; int y;";
            const char expected[] = "class { int x ; } ; int y ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { int x; };";
            const char expected[] = "class { int x ; } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { };";
            const char expected[] = "class { } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class { struct { struct { } ; } ; };";
            const char expected[] = "class { } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyStructDecl4() {
        const char code[] = "class ABC {\n"
                            "    void foo() {\n"
                            "        union {\n"
                            "            int i;\n"
                            "            float f;\n"
                            "        };\n"
                            "        struct Fee { } fee;\n"
                            "    }\n"
                            "    union {\n"
                            "        long long ll;\n"
                            "        double d;\n"
                            "    };\n"
                            "} abc;\n";
        const char expected[] = "class ABC { "
                                "void foo ( ) { "
                                "int i ; "
                                "float & f = i ; "
                                "struct Fee { } ; struct Fee fee ; "
                                "} "
                                "union { "
                                "long long ll ; "
                                "double d ; "
                                "} ; "
                                "} ; ABC abc ;";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyStructDecl6() {
        ASSERT_EQUALS("struct A { "
                      "char integers [ X ] ; "
                      "} ; struct A arrays ; arrays = { { 0 } } ;",
                      tok("struct A {\n"
                          "    char integers[X];\n"
                          "} arrays = {{0}};", false));
    }

    void simplifyStructDecl7() {
        ASSERT_EQUALS("struct Anonymous0 { char x ; } ; struct Anonymous0 a [ 2 ] ;",
                      tok("struct { char x; } a[2];", false));
        ASSERT_EQUALS("struct Anonymous0 { char x ; } ; static struct Anonymous0 a [ 2 ] ;",
                      tok("static struct { char x; } a[2];", false));
    }

    void simplifyStructDecl8() {
        ASSERT_EQUALS("enum A { x , y , z } ; enum A a ; a = x ;", tok("enum A { x, y, z } a(x);", false));
        ASSERT_EQUALS("enum B { x , y , z } ; enum B b ; b = x ;", tok("enum B { x , y, z } b{x};", false));
        ASSERT_EQUALS("struct C { int i ; } ; struct C c ; c = { 0 } ;", tok("struct C { int i; } c{0};", false));
        ASSERT_EQUALS("enum Anonymous0 { x , y , z } ; enum Anonymous0 d ; d = x ;", tok("enum { x, y, z } d(x);", false));
        ASSERT_EQUALS("enum Anonymous0 { x , y , z } ; enum Anonymous0 e ; e = x ;", tok("enum { x, y, z } e{x};", false));
        ASSERT_EQUALS("struct Anonymous0 { int i ; } ; struct Anonymous0 f ; f = { 0 } ;", tok("struct { int i; } f{0};", false));
        ASSERT_EQUALS("struct Anonymous0 { } ; struct Anonymous0 x ; x = { 0 } ;", tok("struct {} x = {0};", false));
        ASSERT_EQUALS("enum G : short { x , y , z } ; enum G g ; g = x ;", tok("enum G : short { x, y, z } g(x);", false));
        ASSERT_EQUALS("enum H : short { x , y , z } ; enum H h ; h = x ;", tok("enum H : short { x, y, z } h{x};", false));
        ASSERT_EQUALS("enum class I : short { x , y , z } ; enum I i ; i = x ;", tok("enum class I : short { x, y, z } i(x);", false));
        ASSERT_EQUALS("enum class J : short { x , y , z } ; enum J j ; j = x ;", tok("enum class J : short { x, y, z } j{x};", false));
    }

    void removeUnwantedKeywords() {
        ASSERT_EQUALS("int var ;", tok("register int var ;", true));
        ASSERT_EQUALS("short var ;", tok("register short int var ;", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("inline int foo ( ) { }", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("__inline int foo ( ) { }", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("__forceinline int foo ( ) { }", true));
        ASSERT_EQUALS("const int foo ( ) { }", tok("constexpr int foo() { }", true));
        ASSERT_EQUALS("void f ( ) { int final [ 10 ] ; }", tok("void f() { int final[10]; }", true));
        ASSERT_EQUALS("int * p ;", tok("int * __restrict p;", "test.c"));
        ASSERT_EQUALS("int * * p ;", tok("int * __restrict__ * p;", "test.c"));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * __restrict__ a, float * __restrict__ b);", "test.c"));
        ASSERT_EQUALS("int * p ;", tok("int * restrict p;", "test.c"));
        ASSERT_EQUALS("int * * p ;", tok("int * restrict * p;", "test.c"));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * restrict a, float * restrict b);", "test.c"));
        ASSERT_EQUALS("void foo ( int restrict ) ;", tok("void foo(int restrict);"));
        ASSERT_EQUALS("int * p ;", tok("typedef int * __restrict__ rint; rint p;", "test.c"));

        // don't remove struct members:
        ASSERT_EQUALS("a = b . _inline ;", tok("a = b._inline;", true));

        ASSERT_EQUALS("int i ; i = 0 ;", tok("auto int i = 0;", "test.c"));
        ASSERT_EQUALS("auto i ; i = 0 ;", tok("auto i = 0;", "test.cpp"));
    }

    void simplifyCallingConvention() {
        ASSERT_EQUALS("int f ( ) ;", tok("int __cdecl f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __stdcall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __fastcall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __clrcall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __thiscall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __syscall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __pascal f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __fortran f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __cdecl f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __stdcall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __fastcall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __clrcall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __thiscall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __syscall f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __pascal f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __fortran f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int WINAPI f();", true, Settings::Win32A));
        ASSERT_EQUALS("int f ( ) ;", tok("int APIENTRY f();", true, Settings::Win32A));
        ASSERT_EQUALS("int f ( ) ;", tok("int CALLBACK f();", true, Settings::Win32A));

        // don't simplify Microsoft defines in unix code (#7554)
        ASSERT_EQUALS("enum E { CALLBACK } ;", tok("enum E { CALLBACK } ;", true, Settings::Unix32));
    }

    void simplifyFunctorCall() {
        ASSERT_EQUALS("IncrementFunctor ( ) ( a ) ;", tok("IncrementFunctor()(a);", true));
    }

    // #ticket #5339 (simplify function pointer after comma)
    void simplifyFunctionPointer() {
        ASSERT_EQUALS("f ( double x , double ( * y ) ( ) ) ;", tok("f (double x, double (*y) ());", true));
    }

    void redundant_semicolon() {
        ASSERT_EQUALS("void f ( ) { ; }", tok("void f() { ; }"));
    }

    void simplifyFunctionReturn() {
        {
            const char code[] = "typedef void (*testfp)();\n"
                                "struct Fred\n"
                                "{\n"
                                "    testfp get1() { return 0; }\n"
                                "    void ( * get2 ( ) ) ( ) { return 0 ; }\n"
                                "    testfp get3();\n"
                                "    void ( * get4 ( ) ) ( );\n"
                                "};";
            const char expected[] = "struct Fred "
                                    "{ "
                                    "void * get1 ( ) { return 0 ; } "
                                    "void * get2 ( ) { return 0 ; } "
                                    "void * get3 ( ) ; "
                                    "void * get4 ( ) ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }
        {
            const char code[] = "class Fred {\n"
                                "    std::string s;\n"
                                "    const std::string & foo();\n"
                                "};\n"
                                "const std::string & Fred::foo() { return \"\"; }";
            const char expected[] = "class Fred { "
                                    "std :: string s ; "
                                    "const std :: string & foo ( ) ; "
                                    "} ; "
                                    "const std :: string & Fred :: foo ( ) { return \"\" ; }";
            ASSERT_EQUALS(expected, tok(code, false));
        }
        {
            // Ticket #7916
            // Tokenization would include "int fact < 2 > ( ) { return 2 > ( ) ; }" and generate
            // a parse error (and use after free)
            const char code[] = "extern \"C\" void abort ();\n"
                                "template <int a> inline int fact2 ();\n"
                                "template <int a> inline int fact () {\n"
                                "  return a * fact2<a-1> ();\n"
                                "}\n"
                                "template <> inline int fact<1> () {\n"
                                "  return 1;\n"
                                "}\n"
                                "template <int a> inline int fact2 () {\n"
                                "  return a * fact<a-1>();\n"
                                "}\n"
                                "template <> inline int fact2<1> () {\n"
                                "  return 1;\n"
                                "}\n"
                                "int main() {\n"
                                "  fact2<3> ();\n"
                                "  fact2<2> ();\n"
                                "}";
            tok(code);
        }
    }

    void removeVoidFromFunction() {
        ASSERT_EQUALS("void foo ( ) ;", tok("void foo(void);"));
    }

    void consecutiveBraces() {
        ASSERT_EQUALS("void f ( ) { }", tok("void f(){{}}", true));
        ASSERT_EQUALS("void f ( ) { }", tok("void f(){{{}}}", true));
        ASSERT_EQUALS("void f ( ) { for ( ; ; ) { } }", tok("void f () { for(;;){} }", true));
        ASSERT_EQUALS("void f ( ) { { scope_lock lock ; foo ( ) ; } { scope_lock lock ; bar ( ) ; } }", tok("void f () { {scope_lock lock; foo();} {scope_lock lock; bar();} }", true));
    }

    void simplifyOverride() { // ticket #5069
        const char code[] = "void fun() {\n"
                            "    unsigned char override[] = {0x01, 0x02};\n"
                            "    doSomething(override, sizeof(override));\n"
                            "}\n";
        ASSERT_EQUALS("void fun ( ) { char override [ 2 ] = { 0x01 , 0x02 } ; doSomething ( override , sizeof ( override ) ) ; }",
                      tok(code, true));
    }

    void simplifyNestedNamespace() {
        ASSERT_EQUALS("namespace A { namespace B { namespace C { int i ; } } }", tok("namespace A::B::C { int i; }"));
    }

    void simplifyNamespaceAliases() {
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;"));
        ASSERT_EQUALS("boost :: iostreams :: istream foo ( \"foo\" ) ;",
                      tok("namespace ios = boost::iostreams; ios::istream foo(\"foo\");"));
        ASSERT_EQUALS("boost :: iostreams :: istream foo ( \"foo\" ) ;",
                      tok("using namespace std; namespace ios = boost::iostreams; ios::istream foo(\"foo\");"));
        ASSERT_EQUALS(";",
                      tok("using namespace std; namespace ios = boost::iostreams;"));
        ASSERT_EQUALS("namespace NS { boost :: iostreams :: istream foo ( \"foo\" ) ; }",
                      tok("namespace NS { using namespace std; namespace ios = boost::iostreams; ios::istream foo(\"foo\"); }"));

        // duplicate namespace aliases
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;"));
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;"));
        ASSERT_EQUALS("namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; } } }",
                      tok("namespace A::B {"
                          "namespace AB = A::B;"
                          "void foo() {"
                          "    namespace AB = A::B;" // duplicate declaration
                          "    bar(AB::ab());"
                          "}"
                          "namespace AB = A::B;" //duplicate declaration
                          "}"));

        // redeclared nested namespace aliases
        TODO_ASSERT_EQUALS("namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; { baz ( A :: a ( ) ) ; } bar ( A :: B :: ab ( ) ) ; } } }",
                           "namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; { baz ( A :: B :: a ( ) ) ; } bar ( A :: B :: ab ( ) ) ; } } }",
                           tok("namespace A::B {"
                               "namespace AB = A::B;"
                               "void foo() {"
                               "    namespace AB = A::B;" // duplicate declaration
                               "    bar(AB::ab());"
                               "    {"
                               "         namespace AB = A;"
                               "         baz(AB::a());" // redeclaration OK
                               "    }"
                               "    bar(AB::ab());"
                               "}"
                               "namespace AB = A::B;" //duplicate declaration
                               "}"));
    }
};

REGISTER_TEST(TestSimplifyTokens)
