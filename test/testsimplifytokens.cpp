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



#include "testsuite.h"
#include "tokenize.h"
#include "token.h"
#include "settings.h"

#include <sstream>

extern std::ostringstream errout;


class TestSimplifyTokens : public TestFixture
{
public:
    TestSimplifyTokens() : TestFixture("TestSimplifyTokens")
    { }


private:

    void run()
    {
        // Make sure the Tokenizer::simplifyTokenList works.
        // The order of the simplifications is important. So this test
        // case shall make sure the simplifications are done in the
        // correct order
        TEST_CASE(simplifyTokenList1);

        TEST_CASE(cast);
        TEST_CASE(iftruefalse);
        TEST_CASE(combine_strings);
        TEST_CASE(double_plus);
        TEST_CASE(redundant_plus);
        TEST_CASE(parantheses1);
        TEST_CASE(paranthesesVar);      // Remove redundant parentheses around variable .. "( %var% )"
        TEST_CASE(declareVar);

        TEST_CASE(declareArray);

        TEST_CASE(dontRemoveIncrement);
        TEST_CASE(removePostIncrement);
        TEST_CASE(removePreIncrement);

        TEST_CASE(elseif1);
        TEST_CASE(ifa_ifa);     // "if (a) { if (a) .." => "if (a) { if (1) .."

        TEST_CASE(sizeof1);
        TEST_CASE(sizeof2);
        TEST_CASE(sizeof3);
        TEST_CASE(sizeof4);
        TEST_CASE(sizeof5);
        TEST_CASE(sizeof6);
        TEST_CASE(sizeof7);
        TEST_CASE(sizeof8);
        TEST_CASE(sizeof9);
        TEST_CASE(sizeof10);
        TEST_CASE(sizeof11);
        TEST_CASE(sizeof12);
        TEST_CASE(sizeof13);
        TEST_CASE(sizeof14);
        TEST_CASE(sizeof15);
        TEST_CASE(sizeof16);
        TEST_CASE(sizeof17);
        TEST_CASE(sizeof18);
        TEST_CASE(sizeof19);    // #1891 - sizeof 'x'
        TEST_CASE(sizeof20);    // #2024 - sizeof a)
        TEST_CASE(sizeof21);    // #2232 - sizeof...(Args)
        TEST_CASE(sizeof22);    // #2599
        TEST_CASE(sizeof23);    // #2604
        TEST_CASE(sizeofsizeof);
        TEST_CASE(casting);

        TEST_CASE(strlen1);

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
        TEST_CASE(template15);
        TEST_CASE(template16);
        TEST_CASE(template17);
        TEST_CASE(template18);
        TEST_CASE(template19);
        TEST_CASE(template20);
        TEST_CASE(template21);
        TEST_CASE(template22);
        TEST_CASE(template23);
        TEST_CASE(template_unhandled);
        TEST_CASE(template_default_parameter);
        TEST_CASE(template_default_type);
        TEST_CASE(template_typename);

        TEST_CASE(namespaces);

        // Assignment in condition..
        TEST_CASE(ifassign1);
        TEST_CASE(ifAssignWithCast);
        TEST_CASE(whileAssign1);
        TEST_CASE(whileAssign2);

        // "if(0==x)" => "if(!x)"
        TEST_CASE(ifnot);
        TEST_CASE(combine_wstrings);

        // Simplify "not" to "!" (#345)
        TEST_CASE(not1);

        // Simplify "and" to "&&" (#620)
        TEST_CASE(and1);

        // Simplify "or" to "||"
        TEST_CASE(or1);

        TEST_CASE(comma_keyword);
        TEST_CASE(remove_comma);

        // Simplify "?:"
        TEST_CASE(conditionOperator);

        // Simplify calculations
        TEST_CASE(calculations);

        // Simplify goto..
        TEST_CASE(goto1);
        TEST_CASE(goto2);

        // Simplify nested strcat() calls
        TEST_CASE(strcat1);
        TEST_CASE(strcat2);

        // Syntax error
        TEST_CASE(argumentsWithSameName)

        TEST_CASE(simplifyAtol)
        TEST_CASE(simplifyHexInString)
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

        TEST_CASE(simplifyTypedefFunction1);
        TEST_CASE(simplifyTypedefFunction2); // ticket #1685
        TEST_CASE(simplifyTypedefFunction3);
        TEST_CASE(simplifyTypedefFunction4);
        TEST_CASE(simplifyTypedefFunction5);
        TEST_CASE(simplifyTypedefFunction6);
        TEST_CASE(simplifyTypedefFunction7);
        TEST_CASE(simplifyTypedefFunction8);

        TEST_CASE(reverseArraySyntax)
        TEST_CASE(simplify_numeric_condition)
        TEST_CASE(simplify_condition);

        TEST_CASE(pointeralias1);
        TEST_CASE(pointeralias2);
        TEST_CASE(pointeralias3);
        TEST_CASE(pointeralias4);
        TEST_CASE(pointeralias5);

        TEST_CASE(reduceConstness);

        // simplify "while (0)"
        TEST_CASE(while0);
        TEST_CASE(while1);

        TEST_CASE(enum1);
        TEST_CASE(enum2);
        TEST_CASE(enum3);
        TEST_CASE(enum4);
        TEST_CASE(enum5);
        TEST_CASE(enum6);
        TEST_CASE(enum7);
        TEST_CASE(enum8);
        TEST_CASE(enum9); // ticket 1404
        TEST_CASE(enum10); // ticket 1445
        TEST_CASE(enum11);
        TEST_CASE(enum12);
        TEST_CASE(enum13);
        TEST_CASE(enum14);
        TEST_CASE(enum15);
        TEST_CASE(enum16); // ticket #1988
        TEST_CASE(enum17); // ticket #2381 (duplicate enums)
        TEST_CASE(enum18); // #2466 (array with same name as enum constant)
        TEST_CASE(enum19); // ticket #2536
        TEST_CASE(enum20); // ticket #2600

        // remove "std::" on some standard functions
        TEST_CASE(removestd);

        // Tokenizer::simplifyInitVar
        TEST_CASE(simplifyInitVar);

        // Tokenizer::simplifyReference
        TEST_CASE(simplifyReference);

        // x = realloc(y,0);  =>  free(y);x=0;
        TEST_CASE(simplifyRealloc);

        // while(f() && errno==EINTR) { } => while (f()) { }
        TEST_CASE(simplifyErrNoInWhile);

        // while(fclose(f)); => r = fclose(f); while(r){r=fclose(f);}
        TEST_CASE(simplifyFuncInWhile);

        // struct ABC abc = { .a = 3 };  =>  struct ABC abc; abc.a = 3;
        TEST_CASE(initstruct);

        // struct ABC { } abc; => struct ABC { }; ABC abc;
        TEST_CASE(simplifyStructDecl1);
        TEST_CASE(simplifyStructDecl2); // ticket #2579

        // register int var; => int var;
        // inline int foo() {} => int foo() {}
        TEST_CASE(removeUnwantedKeywords);

        // remove calling convention __cdecl, __stdcall, ...
        TEST_CASE(simplifyCallingConvention);

        TEST_CASE(simplifyFunctorCall);

        TEST_CASE(redundant_semicolon);

        TEST_CASE(simplifyFunctionReturn);

        TEST_CASE(removeUnnecessaryQualification1);
        TEST_CASE(removeUnnecessaryQualification2);

        TEST_CASE(simplifyIfNotNull);
    }

    std::string tok(const char code[], bool simplify = true)
    {
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList();

        tokenizer.validate();
        std::string ret;
        for (const Token *tok1 = tokenizer.tokens(); tok1; tok1 = tok1->next())
        {
            if (tok1 != tokenizer.tokens())
                ret += " ";
            if (!simplify)
            {
                if (tok1->isUnsigned())
                    ret += "unsigned ";
                else if (tok1->isSigned())
                    ret += "signed ";
            }
            if (tok1->isLong())
                ret += "long ";
            ret += tok1->str();
        }

        return ret;
    }


    void simplifyTokenList1()
    {
        // #1717 : The simplifyErrNoInWhile needs to be used before simplifyIfAssign..
        ASSERT_EQUALS("; x = f ( ) ; while ( ( x ) == -1 ) { x = f ( ) ; }",
                      tok(";while((x=f())==-1 && errno==EINTR){}",true));
    }


    void cast()
    {
        ASSERT_EQUALS("if ( ! p )", tok("if (p == (char *)0)"));
        ASSERT_EQUALS("return str ;", tok("return (char *)str;"));

        {
            const char code[] = "static void crash()\n"
                                "{\n"
                                "    goto err_exit;\n"
                                "err_exit:\n"
                                "    (void)foo();\n"
                                "}\n";

            const char expected[] = "static void crash ( ) "
                                    "{ foo ( ) ; return ; foo ( ) ; }";

            ASSERT_EQUALS(expected, tok(code));
        }

        ASSERT_EQUALS("if ( * a )", tok("if ((char)*a)"));
        ASSERT_EQUALS("if ( & a )", tok("if ((int)&a)"));
        ASSERT_EQUALS("if ( * a )", tok("if ((unsigned int)(unsigned char)*a)"));
        ASSERT_EQUALS("class A { A operator* ( int ) ; } ;", tok("class A { A operator *(int); };"));
        ASSERT_EQUALS("class A { A operator* ( int ) const ; } ;", tok("class A { A operator *(int) const; };"));
        ASSERT_EQUALS("if ( ! p )", tok("if (p == (char *)(char *)0)"));
    }


    void iftruefalse()
    {
        {
            const char code1[] = " void f() { int a; bool use = false; if( use ) { a=0; } else {a=1;} }";
            const char code2[] = " void f() { int a; bool use = false; {a=1;} }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = true; if( use ) { a=0; } else {a=1;} }";
            const char code2[] = " void f() { int a; bool use = true; { a=0; } }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; int use = 5; if( use ) { a=0; } else {a=1;} }";
            const char code2[] = " void f() { int a; int use = 5; { a=0; } }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; int use = 0; if( use ) { a=0; } else {a=1;} }";
            const char code2[] = " void f() { int a; int use = 0; {a=1;} }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = false; if( use ) a=0; else a=1; int c=1; }";
            const char code2[] = " void f() { int a; bool use = false; { a=1; } int c=1; }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = true; if( use ) a=0; else a=1; int c=1; }";
            const char code2[] = " void f() { int a; bool use = true; { a=0; } int c=1; }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = false; if( use ) a=0; else if( bb ) a=1; int c=1; }";
            const char code2[] = " void f ( ) { int a ; bool use ; use = false ; { if ( bb ) { a = 1 ; } } int c ; c = 1 ; }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = true; if( use ) a=0; else if( bb ) a=1; int c=1; }";
            const char code2[] = " void f() { int a; bool use = true; { a=0;} int c=1; }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = "void f() { int a; bool use = true; if( use ) a=0; else if( bb ) a=1; else if( cc ) a=33; else { gg = 0; } int c=1; }";
            const char code2[] = "void f ( ) { ; { ; } ; }";
            ASSERT_EQUALS(code2, tok(code1));
        }

        {
            const char code1[] = " void f() { if( aa ) { a=0; } else if( true ) a=1; else { a=2; } }";
            const char code2[] = " void f ( ) { if ( aa ) { a = 0 ; } else { { a = 1 ; } } }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { if( aa ) { a=0; } else if( false ) a=1; else { a=2; } }";
            const char code2[] = " void f ( ) { if ( aa ) { a = 0 ; } else { { a = 2 ; } } }";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }
    }

    void combine_strings()
    {
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
    }

    void combine_wstrings()
    {
        const char code1[] =  "void foo()\n"
                              "{\n"
                              "const wchar_t *a =\n"
                              "{\n"
                              "L\"hello \"\n"
                              "L\"world\"\n"
                              "};\n"
                              "}\n";

        const char code2[] =  "void foo()\n"
                              "{\n"
                              "const wchar_t *a =\n"
                              "{\n"
                              "\"hello world\"\n"
                              "};\n"
                              "}\n";
        ASSERT_EQUALS(tok(code2), tok(code1));
    }

    void double_plus()
    {
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

    void redundant_plus()
    {
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


    void parantheses1()
    {
        ASSERT_EQUALS("<= 110 ;", tok("<= (10+100);"));
        ASSERT_EQUALS("while ( x ( ) == -1 ) { }", tok("while((x()) == -1){ }"));
    }

    void paranthesesVar()
    {
        // remove parentheses..
        ASSERT_EQUALS("= p ;", tok("= (p);"));
        ASSERT_EQUALS("if ( a < p ) { }", tok("if(a<(p)){}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p == -1 ) { } }", tok("void f(){int p; if((p)==-1){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p == -1 ) { } }", tok("void f(){int p; if(-1==(p)){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p ) { } }", tok("void f(){int p; if((p)){}}"));
        ASSERT_EQUALS("return p ;", tok("return (p);"));
        ASSERT_EQUALS("void f ( ) { int * p ; if ( ! * p ) { } }", tok("void f(){int *p; if (*(p) == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { int * p ; if ( ! * p ) { } }", tok("void f(){int *p; if (*p == 0) {}}"));
        ASSERT_EQUALS("void f ( int & p ) { p = 1 ; }", tok("void f(int &p) {(p) = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p [ 10 ] ; p [ 0 ] = 1 ; }", tok("void f(){int p[10]; (p)[0] = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( ! p ) { } }", tok("void f(){int p; if ((p) == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { int * p ; * p = 1 ; }", tok("void f(){int *p; *(p) = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p ) { } p = 1 ; }", tok("void f(){int p; if ( p ) { } (p) = 1;}"));

        // keep parentheses..
        ASSERT_EQUALS("= a ;", tok("= (char)a;"));
        ASSERT_EQUALS("cast < char * > ( p )", tok("cast<char *>(p)"));
        ASSERT_EQUALS("return ( a + b ) * c ;", tok("return (a+b)*c;"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( 2 * p == 0 ) { } }", tok("void f(){int p; if (2*p == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { DIR * f ; f = opendir ( dirname ) ; if ( closedir ( f ) ) { } }", tok("void f(){DIR * f = opendir(dirname);if (closedir(f)){}}"));
        ASSERT_EQUALS("void foo ( int p ) { if ( 0 <= p ) { ; } }", tok("void foo(int p){if((p)>=0);}"));
    }

    void declareVar()
    {
        const char code[] = "void f ( ) { char str [ 100 ] = \"100\" ; }";
        ASSERT_EQUALS(code, tok(code));
    }

    void declareArray()
    {
        const char code[] = "void f ( ) { char str [ ] = \"100\" ; }";
        const char expected[] = "void f ( ) { char str [ 4 ] = \"100\" ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void dontRemoveIncrement()
    {
        {
            const char code[] = "void f(int a)\n"
                                "{\n"
                                "    if (a > 10)\n"
                                "        a = 5;\n"
                                "    else\n"
                                "        a = 10;\n"
                                "    a++;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( int a ) { if ( 10 < a ) { a = 5 ; } else { a = 10 ; } a ++ ; }", tok(code));
        }

        {
            const char code[] = "void f(int a)\n"
                                "{\n"
                                "    if (a > 10)\n"
                                "        a = 5;\n"
                                "    else\n"
                                "        a = 10;\n"
                                "    ++a;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( int a ) { if ( 10 < a ) { a = 5 ; } else { a = 10 ; } ++ a ; }", tok(code));
        }
    }

    void removePostIncrement()
    {
        const char code[] = "void f(int &c)\n"
                            "{\n"
                            "    c = 0;\n"
                            "    c++;\n"
                            "    if (c>0) { c++; }\n"
                            "    c++;\n"
                            "}\n";
        TODO_ASSERT_EQUALS("void f ( int & c ) { c = 3 ; { ; } ; }",
                           "void f ( int & c ) { c = 1 ; { c ++ ; } c ++ ; }", tok(code));
    }


    void removePreIncrement()
    {
        {
            const char code[] = "void f(int &c)\n"
                                "{\n"
                                "    c = 0;\n"
                                "    ++c;\n"
                                "    if (c>0) { ++c; }\n"
                                "    ++c;\n"
                                "}\n";
            TODO_ASSERT_EQUALS("void f ( int & c ) { c = 3 ; { ; } ; }",
                               "void f ( int & c ) { c = 1 ; { ++ c ; } ++ c ; }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                " char a[] = \"p\";\n"
                                " ++a[0];\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { char a [ 2 ] = \"p\" ; ++ a [ 0 ] ; }", tok(code));
        }
    }


    std::string elseif(const char code[])
    {
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.createTokens(istr);
        tokenizer.elseif();
        return tokenizer.tokens()->stringifyList(false);
    }

    void elseif1()
    {
        const char code[] = "else if(ab) { cd } else { ef }gh";
        ASSERT_EQUALS("\n\n##file 0\n1: else { if ( ab ) { cd } else { ef } } gh\n", elseif(code));

        // syntax error: assert there is no segmentation fault
        ASSERT_EQUALS("\n\n##file 0\n1: else if ( x ) { }\n", elseif("else if (x) { }"));

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
                                    "if ( ! f ) { coo ( ) ; } "
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
    }


    void ifa_ifa()
    {
        ASSERT_EQUALS("int a ; if ( a ) { { ab } cd }", tok("int a ; if (a) { if (a) { ab } cd }", true));
        ASSERT_EQUALS("int a ; if ( a ) { { ab } cd }", tok("int a ; if (unlikely(a)) { if (a) { ab } cd }", true));
    }





    // Simplify 'sizeof'..
    std::string sizeof_(const char code[], bool simplify = true)
    {
        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        if (simplify)
            tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok1 = tokenizer.tokens(); tok1; tok1 = tok1->next())
        {
            if (tok1->previous())
            {
                ostr << " ";
            }
            ostr << tok1->str();
        }

        return ostr.str();
    }

    unsigned int sizeofFromTokenizer(const char type[])
    {
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr("");
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();
        Token tok1(0);
        tok1.str(type);
        return tokenizer.sizeOfType(&tok1);
    }

    void sizeof1()
    {
        ASSERT_EQUALS("struct ABC * abc ; abc = malloc ( 100 ) ;", tok("struct ABC *abc = malloc(sizeof(*abc));"));
        ASSERT_EQUALS("struct ABC * abc ; abc = malloc ( 100 ) ;", tok("struct ABC *abc = malloc(sizeof *abc );"));
    }


    void sizeof2()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i[4];\n"
                            "    sizeof(i);\n"
                            "    sizeof(*i);\n"
                            "}\n";
        ASSERT_EQUALS("void foo ( ) { int i [ 4 ] ; 16 ; 4 ; }", sizeof_(code));
    }

    void sizeof3()
    {
        const char code[] = "static int i[4];\n"
                            "void f()\n"
                            "{\n"
                            "    int i[10];\n"
                            "    sizeof(i);\n"
                            "}\n";
        ASSERT_EQUALS("static int i [ 4 ] ; void f ( ) { int i [ 10 ] ; 40 ; }", sizeof_(code));
    }

    void sizeof4()
    {
        {
            const char code[] = "int i[10];\n"
                                "sizeof(i[0]);\n";
            ASSERT_EQUALS("int i [ 10 ] ; 4 ;", sizeof_(code));
        }

        {
            const char code[] = "int i[10];\n"
                                "sizeof i[0];\n";
            ASSERT_EQUALS("int i [ 10 ] ; 4 ;", sizeof_(code));
        }
    }

    void sizeof5()
    {
        const char code[] =
            "const char * names[2];"
            "for (int i = 0; i != sizeof(names[0]); i++)"
            "{}";
        std::ostringstream expected;
        expected << "const char * names [ 2 ] ; for ( int i = 0 ; i != " << sizeofFromTokenizer("*") << " ; i ++ ) { }";
        ASSERT_EQUALS(expected.str(), sizeof_(code));
    }

    void sizeof6()
    {
        const char code[] = ";int i;\n"
                            "sizeof(i);\n";

        std::ostringstream expected;
        expected << "; int i ; " << sizeof(int) << " ;";

        ASSERT_EQUALS(expected.str(), sizeof_(code));
    }

    void sizeof7()
    {
        const char code[] = ";INT32 i[10];\n"
                            "sizeof(i[0]);\n";
        ASSERT_EQUALS("; INT32 i [ 10 ] ; sizeof ( i [ 0 ] ) ;", sizeof_(code));
    }

    void sizeof8()
    {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs[2];\n"
                                "  a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << (sizeofFromTokenizer("*") * 2);
            ASSERT_EQUALS("void f ( ) { char * ptrs [ 2 ] ; a = " + oss.str() + " ; }", sizeof_(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs[55];\n"
                                "  a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << (sizeofFromTokenizer("*") * 55);
            ASSERT_EQUALS("void f ( ) { char * ptrs [ 55 ] ; a = " + oss.str() + " ; }", sizeof_(code));
        }


        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs;\n"
                                "  a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << sizeofFromTokenizer("*");
            ASSERT_EQUALS("void f ( ) { ; a = " + oss.str() + " ; }", sizeof_(code));
        }
    }

    void sizeof9()
    {
        // ticket #487
        {
            const char code[] = "; const char *str = \"1\"; sizeof(str);";

            std::ostringstream expected;
            expected << "; const char * str ; str = \"1\" ; " << sizeofFromTokenizer("*") << " ;";

            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "; const char str[] = \"1\"; sizeof(str);";

            std::ostringstream expected;
            expected << "; const char str [ 2 ] = \"1\" ; " << sizeofFromTokenizer("char")*2 << " ;";

            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            // Ticket #799
            const char code[] = "; const char str[] = {'1'}; sizeof(str);";
            ASSERT_EQUALS("; const char str [ 1 ] = { '1' } ; 1 ;", sizeof_(code));
        }

        {
            // Ticket #2087
            const char code[] = "; const char str[] = {\"abc\"}; sizeof(str);";
            ASSERT_EQUALS("; const char str [ 4 ] = \"abc\" ; 4 ;", sizeof_(code));
        }

        // ticket #716 - sizeof string
        {
            std::ostringstream expected;
            expected << "; " << (sizeof "123");

            ASSERT_EQUALS(expected.str(), sizeof_("; sizeof \"123\""));
            ASSERT_EQUALS(expected.str(), sizeof_("; sizeof(\"123\")"));
        }

        {
            const char code[] = "void f(char *a,char *b, char *c)"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char * a , char * b , char * c ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "void f(char a,char b, char c)"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char a , char b , char c ) { g ( " <<
                     sizeofFromTokenizer("char") << " , " << sizeofFromTokenizer("char") << " , " << sizeofFromTokenizer("char") << " ) ; }";
            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "void f(const char *a,const char *b, const char *c)"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( const char * a , const char * b , const char * c ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "void f(char a[10],char b[10], char c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char a [ 10 ] , char b [ 10 ] , char c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "void f(const char a[10],const char b[10], const char c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( const char a [ 10 ] , "
                     "const char b [ 10 ] , "
                     "const char c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "void f(const char *a[10],const char *b[10], const char *c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( const char * a [ 10 ] , "
                     "const char * b [ 10 ] , "
                     "const char * c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "void f(char *a[10],char *b[10], char *c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char * a [ 10 ] , char * b [ 10 ] , char * c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            std::ostringstream expected;
            expected << "; " << sizeof("\"quote\"");
            ASSERT_EQUALS(expected.str(), sizeof_("; sizeof(\"\\\"quote\\\"\")"));
        }

        {
            std::ostringstream expected;
            expected << "void f ( ) { char str [ 100 ] = \"100\" ; " << sizeofFromTokenizer("char")*100 << " }";
            ASSERT_EQUALS(expected.str(), tok("void f ( ) { char str [ 100 ] = \"100\" ; sizeof ( str ) }"));
        }
    }

    void sizeof10()
    {
        // ticket #809
        const char code[] = "int m ; "
                            "compat_ulong_t um ; "
                            "size_t size ; size = sizeof ( m ) / sizeof ( um ) ;";

        ASSERT_EQUALS(code, tok(code));
    }

    void sizeof11()
    {
        // ticket #827
        const char code[] = "void f()\n"
                            "{\n"
                            "    char buf2[4];\n"
                            "    sizeof buf2;\n"
                            "}\n"
                            "\n"
                            "void g()\n"
                            "{\n"
                            "    struct A a[2];\n"
                            "    char buf[32];\n"
                            "    sizeof buf;\n"
                            "}";

        const char expected[] = "void f ( ) "
                                "{"
                                " char buf2 [ 4 ] ;"
                                " 4 ; "
                                "} "
                                ""
                                "void g ( ) "
                                "{"
                                " struct A a [ 2 ] ;"
                                " char buf [ 32 ] ;"
                                " 32 ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code));
    }

    void sizeof12()
    {
        // ticket #827
        const char code[] = "void f()\n"
                            "{\n"
                            "    int *p;\n"
                            "    (sizeof *p);\n"
                            "}";

        const char expected[] = "void f ( ) "
                                "{"
                                " ;"
                                " 4 ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code));
    }

    void sizeof13()
    {
        // ticket #851
        const char code[] = "int main()\n"
                            "{\n"
                            "    char *a;\n"
                            "    a = malloc(sizeof(*a));\n"
                            "}\n"
                            "\n"
                            "struct B\n"
                            "{\n"
                            "    char * b[2];\n"
                            "};";
        const char expected[] = "int main ( ) "
                                "{"
                                " char * a ;"
                                " a = malloc ( 1 ) ; "
                                "} "
                                "struct B "
                                "{"
                                " char * b [ 2 ] ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void sizeof14()
    {
        // ticket #954
        const char code[] = "void f()\n"
                            "{\n"
                            "    A **a;\n"
                            "    int aa = sizeof *(*a)->b;\n"
                            "}\n";
        const char expected[] = "void f ( ) "
                                "{"
                                " A * * a ;"
                                " int aa ; aa = sizeof ( * ( * a ) . b ) ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void sizeof15()
    {
        // ticket #1020
        tok("void f()\n"
            "{\n"
            "    int *n;\n"
            "    sizeof *(n);\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof16()
    {
        // ticket #1027
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a;\n"
                            "    printf(\"%i\", sizeof a++);\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { int a ; printf ( \"%i\" , sizeof ( a ++ ) ) ; }", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof17()
    {
        // ticket #1050
        const char code[] = "void f()\n"
                            "{\n"
                            "    sizeof 1;\n"
                            "    while (0);\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { sizeof ( 1 ) ; }", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof18()
    {
        {
            std::ostringstream expected;
            expected << sizeof(short int);

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(short int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(unsigned short int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(short unsigned int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(signed short int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }
        }

        {
            std::ostringstream expected;
            expected << sizeof(long long);

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(long long);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(signed long long);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(unsigned long long);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(long unsigned long);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(long long int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(signed long long int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(unsigned long long int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }

            {
                const char code[] = "void f()\n"
                                    "{\n"
                                    "    sizeof(long unsigned long int);\n"
                                    "}\n";
                ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
                ASSERT_EQUALS("", errout.str());
            }
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    sizeof(char*);\n"
                                "}\n";
            std::ostringstream expected;
            expected << sizeof(int*);
            ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    sizeof(unsigned int*);\n"
                                "}\n";
            std::ostringstream expected;
            expected << sizeof(int*);
            ASSERT_EQUALS("void f ( ) { " + expected.str() + " ; }", tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void sizeof19()
    {
        // ticket #1891 - sizeof 'x'
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    sizeof 'x';\n"
                                "}\n";
            std::ostringstream sz;
            sz << sizeof('x');
            ASSERT_EQUALS("void f ( ) { " + sz.str() + " ; }", tok(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    sizeof('x');\n"
                                "}\n";
            std::ostringstream sz;
            sz << sizeof('x');
            ASSERT_EQUALS("void f ( ) { " + sz.str() + " ; }", tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void sizeof20()
    {
        // ticket #2024 - sizeof a)
        const char code[] = "struct struct_a {\n"
                            "  char a[20];\n"
                            "};\n"
                            "\n"
                            "void foo() {\n"
                            "  struct_a a;\n"
                            "  append(sizeof a).append();\n"
                            "}\n";
        ASSERT_EQUALS("struct struct_a { char a [ 20 ] ; } ; "
                      "void foo ( ) {"
                      " struct_a a ;"
                      " append ( 100 ) . append ( ) ; "
                      "}", tok(code));
    }

    void sizeof21()
    {
        // ticket #2232 - sizeof...(Args)
        const char code[] = "struct Internal {\n"
                            "    int operator()(const Args&... args) const {\n"
                            "        int n = sizeof...(Args);\n"
                            "        return n;\n"
                            "    }\n"
                            "};\n"
                            "\n"
                            "int main() {\n"
                            "    Internal internal;\n"
                            "    int n = 0; n = internal(1);\n"
                            "    return 0;\n"
                            "}\n";

        // don't segfault
        tok(code);
    }

    void sizeof22()
    {
        // ticket #2599 segmentation fault
        const char code[] = "sizeof\n";

        // don't segfault
        tok(code);
    }

    void sizeof23()
    {
        // ticket #2604 segmentation fault
        const char code[] = "sizeof <= A\n";

        // don't segfault
        tok(code);
    }


    void sizeofsizeof()
    {
        // ticket #1682
        const char code[] = "void f()\n"
                            "{\n"
                            "    sizeof sizeof 1;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { sizeof sizeof ( 1 ) ; }", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void casting()
    {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "for (int i = 0; i < static_cast<int>(3); ++i) {}\n"
                                "}\n";

            const std::string expected("void f ( ) { for ( int i = 0 ; i < 3 ; ++ i ) { } }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    p = const_cast<char *> qtu ();\n"
                                "}\n";

            const std::string expected("void f ( ) { p = const_cast < char * > qtu ( ) ; }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            // ticket #645
            const char code[] = "void f()\n"
                                "{\n"
                                "    return dynamic_cast<Foo *>((bar()));\n"
                                "}\n";
            const std::string expected("void f ( ) { return bar ( ) ; }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }
    }


    void strlen1()
    {
        ASSERT_EQUALS("4", tok("strlen(\"abcd\")"));

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    const char *s = \"abcd\";\n"
                                "    strlen(s);\n"
                                "}\n";
            const char expected[] = "void f ( ) "
                                    "{"
                                    " const char * s ;"
                                    " s = \"abcd\" ;"
                                    " 4 ; "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    const char s [ ] = \"abcd\";\n"
                                "    strlen(s);\n"
                                "}\n";
            const char expected[] = "void f ( ) "
                                    "{"
                                    " const char s [ 5 ] = \"abcd\" ;"
                                    " 4 ; "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

    }





    void template1()
    {
        const char code[] = "template <classname T> void f(T val) { T a; }\n"
                            "f<int>(10);";

        const std::string expected("; f<int> ( 10 ) ; "
                                   "void f<int> ( int val ) { ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template2()
    {
        const char code[] = "template <classname T> class Fred { T a; };\n"
                            "Fred<int> fred;";

        const std::string expected("; "
                                   "Fred<int> fred ; "
                                   "class Fred<int> { int a ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template3()
    {
        const char code[] = "template <classname T, int sz> class Fred { T data[sz]; };\n"
                            "Fred<float,4> fred;";

        const std::string expected("; "
                                   "Fred<float,4> fred ; "
                                   "class Fred<float,4> { float data [ 4 ] ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template4()
    {
        const char code[] = "template <classname T> class Fred { Fred(); };\n"
                            "Fred<float> fred;";

        const std::string expected("; "
                                   "Fred<float> fred ; "
                                   "class Fred<float> { Fred<float> ( ) ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template5()
    {
        const char code[] = "template <classname T> class Fred { };\n"
                            "template <classname T> Fred<T>::Fred() { }\n"
                            "Fred<float> fred;";

        const std::string expected("; "
                                   "Fred<float> fred ; "
                                   "class Fred<float> { } "
                                   "Fred<float> :: Fred<float> ( ) { }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template6()
    {
        const char code[] = "template <classname T> class Fred { };\n"
                            "Fred<float> fred1;\n"
                            "Fred<float> fred2;";

        const std::string expected(";"
                                   " Fred<float> fred1 ;"
                                   " Fred<float> fred2 ;"
                                   " class Fred<float> { }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template7()
    {
        // A template class that is not used => no simplification
        {
            const char code[] = "template <class T>\n"
                                "class ABC\n"
                                "{\n"
                                "public:\n"
                                "    typedef ABC<T> m;\n"
                                "\n"
                                "};\n";

            const std::string expected(";");

            ASSERT_EQUALS(expected, sizeof_(code));
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

            const std::string wanted("; "
                                     "int main ( ) { "
                                     "std :: vector < int > v ; "
                                     "v . push_back ( 4 ) ; "
                                     "return 0 ; "
                                     "}");

            const std::string current("; "
                                      "int main ( ) { "
                                      "ABC < int > :: type v ; "
                                      "v . push_back ( 4 ) ; "
                                      "return 0 ; "
                                      "}"
                                     );

            TODO_ASSERT_EQUALS(wanted, current, sizeof_(code));
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

            const std::string expected(";");

            ASSERT_EQUALS(expected, sizeof_(code));
        }
    }

    // Template definitions but no usage => no expansion
    void template8()
    {
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

        ASSERT_EQUALS(";", sizeof_(code));

        ASSERT_EQUALS("class A { ; } ;", sizeof_("class A{ template<typename T> int foo(T d);};"));
    }

    void template9()
    {
        const char code[] = "template < typename T > class A { } ;\n"
                            "\n"
                            "void f ( ) {\n"
                            "    A<int> a ;\n"
                            "}\n"
                            "\n"
                            "template < typename T >\n"
                            "class B {\n"
                            "    void g ( ) {\n"
                            "        A < T > b = A < T > :: h ( ) ;\n"
                            "    }\n"
                            "} ;\n";

        // The expected result..
        std::string expected("; void f ( ) { A<int> a ; } ; class A<int> { }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template10()
    {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    foo<3,int>();\n"
                            "}\n";

        // The expected result..
        const std::string expected("; "
                                   "void f ( ) "
                                   "{"
                                   " foo<3,int> ( ) ; "
                                   "} "
                                   "int * foo<3,int> ( ) { return new int [ 3 ] ; }");
        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template11()
    {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char * p = foo<3,char>();\n"
                            "}\n";

        // The expected result..
        const std::string expected("; "
                                   "void f ( ) "
                                   "{"
                                   " char * p ; p = foo<3,char> ( ) ; "
                                   "} "
                                   "char * foo<3,char> ( ) { return new char [ 3 ] ; }");
        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template12()
    {
        const char code[] = "template <int x, int y, int z>\n"
                            "class A : public B<x, y, (x - y) ? ((y < z) ? 1 : -1) : 0>\n"
                            "{ };\n"
                            "\n"
                            "void f()\n"
                            "{\n"
                            "    A<12,12,11> a;\n"
                            "}\n";

        // The expected result..
        const std::string expected("; "
                                   "void f ( ) "
                                   "{"
                                   " A<12,12,11> a ; "
                                   "} "
                                   "class A<12,12,11> : public B < 12 , 12 , 0 > "
                                   "{ }");
        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template13()
    {
        const char code[] = "class BB {};\n"
                            "\n"
                            "template <class T>\n"
                            "class AA\n"
                            "{\n"
                            "public:\n"
                            "    static AA<T> create(T* newObject);\n"
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
                            "    {}\n";

        // Just run it and check that there are not assertions.
        sizeof_(code);
    }

    void template14()
    {
        const char code[] = "template <> void foo<int *>()\n"
                            "{ x(); }\n"
                            "\n"
                            "int main()\n"
                            "{\n"
                            "foo<int*>();\n"
                            "}\n";

        // The expected result..
        const std::string expected("void foo<int*> ( ) "
                                   "{ x ( ) ; } "
                                   "int main ( ) "
                                   "{ foo<int*> ( ) ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template15()
    {
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
        const std::string expected("; "
                                   "void a<0> ( ) { } "
                                   "int main ( ) "
                                   "{ a<2> ( ) ; return 0 ; } "
                                   "void a<2> ( ) { a<1> ( ) ; } "
                                   "void a<1> ( ) { a<0> ( ) ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template16()
    {
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

        const std::string wanted("; "
                                 "; "
                                 "int main ( ) { b<2> ( ) ; return 0 ; } "
                                 "void b<2> ( ) { a<2> ( ) ; } "
                                 "void a<2> ( ) { }");

        const std::string current("; "
                                  "int main ( ) { b<2> ( ) ; return 0 ; } "
                                  "void b<2> ( ) { a < 2 > ( ) ; }"
                                 );

        TODO_ASSERT_EQUALS(wanted, current, sizeof_(code));
    }

    void template17()
    {
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
        sizeof_(code);
    }

    void template18()
    {
        const char code[] = "template <class T> class foo { T a; };\n"
                            "foo<int> *f;";

        const std::string expected("; "
                                   "foo<int> * f ; "
                                   "class foo<int> { int a ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template19()
    {
        const char code[] = "template <typename T> T & foo()\n"
                            "{ static T temp; return temp; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char p = foo<char>();\n"
                            "}\n";

        // The expected result..
        const std::string expected("; "
                                   "void f ( ) "
                                   "{"
                                   " char p ; p = foo<char> ( ) ; "
                                   "} "
                                   "char & foo<char> ( ) { static char temp ; return temp ; }");
        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template20()
    {
        // Ticket #1788 - the destructor implementation is lost
        const char code[] = "template <class T> class A\n"
                            "{\n"
                            "public:\n"
                            "    ~A();\n"
                            "};\n"
                            "\n"
                            "template <class T> A<T>::~A()\n"
                            "{\n"
                            "}\n"
                            "\n"
                            "A<int> a;\n";

        // The expected result..
        const std::string expected("; A<int> a ; "
                                   "class A<int> { public: ~ A<int> ( ) ; } "
                                   "A<int> :: ~ A<int> ( ) { }");
        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template21()
    {
        {
            const char code[] = "template <classname T> struct Fred { T a; };\n"
                                "Fred<int> fred;";

            const std::string expected("; "
                                       "Fred<int> fred ; "
                                       "struct Fred<int> { int a ; }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "template <classname T, int sz> struct Fred { T data[sz]; };\n"
                                "Fred<float,4> fred;";

            const std::string expected("; "
                                       "Fred<float,4> fred ; "
                                       "struct Fred<float,4> { float data [ 4 ] ; }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "template <classname T> struct Fred { Fred(); };\n"
                                "Fred<float> fred;";

            const std::string expected("; "
                                       "Fred<float> fred ; "
                                       "struct Fred<float> { Fred<float> ( ) ; }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "template <classname T> struct Fred { };\n"
                                "template <classname T> Fred<T>::Fred() { }\n"
                                "Fred<float> fred;";

            const std::string expected("; "
                                       "Fred<float> fred ; "
                                       "struct Fred<float> { } "
                                       "Fred<float> :: Fred<float> ( ) { }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "template <classname T> struct Fred { };\n"
                                "Fred<float> fred1;\n"
                                "Fred<float> fred2;";

            const std::string expected(";"
                                       " Fred<float> fred1 ;"
                                       " Fred<float> fred2 ;"
                                       " struct Fred<float> { }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }
    }

    void template22()
    {
        const char code[] = "template <classname T> struct Fred { T a; };\n"
                            "Fred<std::string> fred;";

        const std::string expected("; "
                                   "Fred<std::string> fred ; "
                                   "struct Fred<std::string> { std :: string a ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template23()
    {
        const char code[] = "template <typename T> void foo(double &ac) {}\n"
                            "\n"
                            "void bar() {\n"
                            "    std::cout << (foo<double>(r));\n"
                            "}\n";

        const char expected[] = "; "
                                "void bar ( ) {"
                                " std :: cout << ( foo<double> ( r ) ) ; "
                                "} "
                                "void foo<double> ( double & ac ) { }";

        ASSERT_EQUALS(expected, sizeof_(code));
    }


    void template_unhandled()
    {
        // An unhandled template usage should be simplified..
        ASSERT_EQUALS("; x<int> ( ) ;", sizeof_(";x<int>();"));
    }

    void template_default_parameter()
    {
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
            const std::string expected("; "
                                       "void f ( ) "
                                       "{"
                                       " A<int,2> a1 ;"
                                       " A<int,3> a2 ; "
                                       "} "
                                       "class A<int,2> "
                                       "{ int ar [ 2 ] ; } "
                                       "class A<int,3> "
                                       "{ int ar [ 3 ] ; }");
            ASSERT_EQUALS(expected, sizeof_(code));
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
            const std::string expected("; "
                                       "void f ( ) "
                                       "{"
                                       " A<int,3,2> a1 ;"
                                       " A<int,3,2> a2 ; "
                                       "} "
                                       "class A<int,3,2> "
                                       "{ int ar [ 5 ] ; }");
            ASSERT_EQUALS(expected, sizeof_(code));
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

            const std::string wanted("template < class T , int n >"
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
                                     " { int ar [ 3 ] ; }");

            const std::string current("; "
                                      "void f ( ) "
                                      "{ "
                                      "A < int , ( int ) 2 > a1 ; "
                                      "A<int,3> a2 ; "
                                      "} "
                                      "class A<int,3> "
                                      "{ int ar [ 3 ] ; }"
                                     );
            TODO_ASSERT_EQUALS(wanted, current, sizeof_(code));
        }
    }

    void template_default_type()
    {
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

        errout.str("");
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "file1.cpp");
        tokenizer.simplifyTokenList();

        //ASSERT_EQUALS("[file1.cpp:15]: (error) Internal error: failed to instantiate template. The checking continues anyway.\n", errout.str());
        ASSERT_EQUALS("", errout.str());
    }

    void template_typename()
    {
        {
            const char code[] = "template <class T>\n"
                                "void foo(typename T::t *)\n"
                                "{ }";

            // The expected result..
            const std::string expected(";");
            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "void f() {\n"
                                "    x(sizeof typename);\n"
                                "    type = 0;\n"
                                "}";
            errout.str("");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.c", "", false);
            std::ostringstream ostr;
            for (const Token *tok1 = tokenizer.tokens(); tok1; tok1 = tok1->next())
            {
                ostr << tok1->str();
                if (Token::Match(tok1, "%var% %var%"))
                    ostr << " ";
            }
            ASSERT_EQUALS("void f(){x(sizeof typename);type=0;}", ostr.str());
        }
    }

    void namespaces()
    {
        {
            const char code[] = "using namespace std; namespace a{ namespace b{ void f(){} } }";

            const std::string expected("using namespace std ; namespace a { namespace b { void f ( ) { } } }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "namespace b{ void f(){} }";

            const std::string expected("namespace b { void f ( ) { } }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "int a; namespace b{ }";

            const std::string expected("int a ; namespace b { }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "void f(int namespace) { }";

            const std::string expected("void f ( int namespace ) { }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }
    }


    std::string simplifyIfAssign(const char code[])
    {
        errout.str("");
        Settings settings;
        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyIfAssign();

        std::ostringstream ostr;
        for (const Token *tok1 = tokenizer.tokens(); tok1; tok1 = tok1->next())
            ostr << (tok1->previous() ? " " : "") << tok1->str();

        return ostr.str();
    }

    void ifassign1()
    {
        ASSERT_EQUALS("; a = b ; if ( a ) { ; }", simplifyIfAssign(";if(a=b);"));
        ASSERT_EQUALS("; a = b ( ) ; if ( a ) { ; }", simplifyIfAssign(";if((a=b()));"));
        ASSERT_EQUALS("; a = b ( ) ; if ( ! ( a ) ) { ; }", simplifyIfAssign(";if(!(a=b()));"));
        ASSERT_EQUALS("; a . x = b ( ) ; if ( ! ( a . x ) ) { ; }", simplifyIfAssign(";if(!(a->x=b()));"));
        ASSERT_EQUALS("A ( ) a = b ; if ( a ) { ; }", simplifyIfAssign("A() if(a=b);"));
        ASSERT_EQUALS("void foo ( int a ) { a = b ( ) ; if ( 0 <= a ) { ; } }", tok("void foo(int a) {if((a=b())>=0);}"));
        TODO_ASSERT_EQUALS("void foo ( A a ) { a . c = b ( ) ; if ( 0 <= a . c ) { ; } }",
                           "void foo ( A a ) { a . c = b ( ) ; if ( a . c >= 0 ) { ; } }",
                           tok("void foo(A a) {if((a.c=b())>=0);}"));
    }

    void ifAssignWithCast()
    {
        const char *code =  "void foo()\n"
                            "{\n"
                            "FILE *f;\n"
                            "if( (f = fopen(\"foo\", \"r\")) == ((FILE*)NULL) )\n"
                            "return(-1);\n"
                            "fclose(f);\n"
                            "}\n";
        const char *exptected = "void foo ( ) "
                                "{ "
                                "FILE * f ; "
                                "f = fopen ( \"foo\" , \"r\" ) ; "
                                "if ( ! f ) "
                                "{ "
                                "return -1 ; "
                                "} "
                                "fclose ( f ) ; "
                                "}";
        ASSERT_EQUALS(exptected, tok(code));
    }

    void whileAssign1()
    {
        ASSERT_EQUALS("; a = b ; while ( a ) { b = 0 ; a = b ; }", simplifyIfAssign(";while(a=b) { b = 0; }"));
        ASSERT_EQUALS("; a . b = c ; while ( a . b ) { c = 0 ; a . b = c ; }", simplifyIfAssign(";while(a.b=c) { c=0; }"));
        ASSERT_EQUALS("struct hfs_bnode * node ; "
                      "struct hfs_btree * tree ; "
                      "node = tree . node_hash [ i ++ ] ; "
                      "while ( node ) { node = tree . node_hash [ i ++ ] ; }",
                      tok("struct hfs_bnode *node;"
                          "struct hfs_btree *tree;"
                          "while ((node = tree->node_hash[i++])) { }"));
        ASSERT_EQUALS("char * s ; s = new char [ 10 ] ; while ( ! s ) { s = new char [ 10 ] ; }",
                      tok("char *s; while (0 == (s=new char[10])) { }"));
    }

    void whileAssign2()
    {
        // #1909 - Internal error
        errout.str("");
        tok("void f()\n"
            "{\n"
            "  int b;\n"
            "  while (b = sizeof (struct foo { int i0;}))\n"
            "    ;\n"
            "  if (!(0 <= b ))\n"
            "    ;\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    std::string simplifyIfNot(const char code[])
    {
        errout.str("");
        Settings settings;
        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyIfNot();

        std::ostringstream ostr;
        for (const Token *tok1 = tokenizer.tokens(); tok1; tok1 = tok1->next())
            ostr << (tok1->previous() ? " " : "") << tok1->str();

        return ostr.str();
    }

    void ifnot()
    {
        ASSERT_EQUALS("if ( ! x )", simplifyIfNot("if(0==x)"));
        ASSERT_EQUALS("if ( ! x )", simplifyIfNot("if(x==0)"));
        ASSERT_EQUALS("if ( ! ( a = b ) )", simplifyIfNot("if(0==(a=b))"));
        ASSERT_EQUALS("if ( ! x )", simplifyIfNot("if(x==0)"));
        ASSERT_EQUALS("if ( ! a && b ( ) )", simplifyIfNot("if( 0 == a && b() )"));
        ASSERT_EQUALS("if ( b ( ) && ! a )", simplifyIfNot("if( b() && 0 == a )"));
        ASSERT_EQUALS("if ( ! ( a = b ) )", simplifyIfNot("if((a=b)==0)"));
        ASSERT_EQUALS("if ( ! x . y )", simplifyIfNot("if(x.y==0)"));
        ASSERT_EQUALS("if ( ! x )", simplifyIfNot("if((x==0))"));
        ASSERT_EQUALS("if ( ( ! x ) && ! y )", simplifyIfNot("if((x==0) && y==0)"));
        ASSERT_EQUALS("if ( ! ( ! fclose ( fd ) ) )", simplifyIfNot("if(!(fclose(fd) == 0))"));
    }



    std::string simplifyLogicalOperators(const char code[])
    {
        errout.str("");
        Settings settings;
        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        std::ostringstream ostr;
        for (const Token *tok1 = tokenizer.tokens(); tok1; tok1 = tok1->next())
            ostr << (tok1->previous() ? " " : "") << tok1->str();

        return ostr.str();
    }

    void not1()
    {
        ASSERT_EQUALS("if ( ! p )", simplifyLogicalOperators("if (not p)"));
        ASSERT_EQUALS("if ( p && ! q )", simplifyLogicalOperators("if (p && not q)"));
        ASSERT_EQUALS("void foo ( not i )", simplifyLogicalOperators("void foo ( not i )"));
    }

    void and1()
    {
        ASSERT_EQUALS("if ( p && q ) { ; }",
                      simplifyLogicalOperators("if (p and q) ;"));

        ASSERT_EQUALS("if ( foo ( ) && q ) { ; }",
                      simplifyLogicalOperators("if (foo() and q) ;"));

        ASSERT_EQUALS("if ( foo ( ) && bar ( ) ) { ; }",
                      simplifyLogicalOperators("if (foo() and bar()) ;"));

        ASSERT_EQUALS("if ( p && bar ( ) ) { ; }",
                      simplifyLogicalOperators("if (p and bar()) ;"));

        ASSERT_EQUALS("if ( p && ! q )",
                      simplifyLogicalOperators("if (p and not q)"));
    }

    void or1()
    {
        ASSERT_EQUALS("if ( p || q ) { ; }",
                      simplifyLogicalOperators("if (p or q) ;"));

        ASSERT_EQUALS("if ( foo ( ) || q ) { ; }",
                      simplifyLogicalOperators("if (foo() or q) ;"));

        ASSERT_EQUALS("if ( foo ( ) || bar ( ) ) { ; }",
                      simplifyLogicalOperators("if (foo() or bar()) ;"));

        ASSERT_EQUALS("if ( p || bar ( ) ) { ; }",
                      simplifyLogicalOperators("if (p or bar()) ;"));

        ASSERT_EQUALS("if ( p || ! q )",
                      simplifyLogicalOperators("if (p or not q)"));
    }

    void comma_keyword()
    {
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    delete a, delete b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a ; delete b ; }", sizeof_(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    struct A *a, *b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { struct A * a ; struct A * b ; }", sizeof_(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    struct A **a, **b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { struct A * * a ; struct A * * b ; }", sizeof_(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    delete a, b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a ; delete b ; }", sizeof_(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char **a, **b, **c;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * * a ; char * * b ; char * * c ; }", sizeof_(code));
        }

        {
            const char code[] = "int f()\n"
                                "{\n"
                                "    if (something)\n"
                                "        return a(2, c(3, 4)), b(3), 10;\n"
                                "    return a(), b(0, 0, 0), 10;\n"
                                "}\n";
            ASSERT_EQUALS("int f ( )"
                          " {"
                          " if ( something )"
                          " {"
                          " a ( 2 , c ( 3 , 4 ) ) ;"
                          " b ( 3 ) ;"
                          " return 10 ;"
                          " }"
                          " a ( ) ;"
                          " b ( 0 , 0 , 0 ) ;"
                          " return 10 ; "
                          "}", sizeof_(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    delete [] a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { delete [ ] a ; a = 0 ; }", sizeof_(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    delete a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { delete a ; a = 0 ; }", sizeof_(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    for(int a,b; a < 10; a = a + 1, b = b + 1);\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { for ( int a , b ; a < 10 ; a = a + 1 , b = b + 1 ) { ; } }", sizeof_(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    char buf[BUFSIZ], **p;\n"
                                "    char *ptrs[BUFSIZ], **pp;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { char buf [ BUFSIZ ] ; char * * p ; char * ptrs [ BUFSIZ ] ; char * * pp ; }", sizeof_(code));
        }
    }

    void remove_comma()
    {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  int a,b;\n"
                                "  if( a )\n"
                                "  a=0,\n"
                                "  b=0;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { int a ; int b ; if ( a ) { a = 0 ; b = 0 ; } }", sizeof_(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  A a,b;\n"
                                "  if( a.f )\n"
                                "  a.f=b.f,\n"
                                "  a.g=b.g;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { A a ; A b ; if ( a . f ) { a . f = b . f ; a . g = b . g ; } }", sizeof_(code));
        }

        // keep the comma in template specifiers..
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  int a = b<T<char,3>, int>();\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { int a ; a = b < T < char , 3 > , int > ( ) ; }", sizeof_(code));
        }

        {
            // ticket #1327
            const char code[] = "const C<1,2,3> foo ()\n"
                                "{\n"
                                "    return C<1,2,3>(x,y);\n"
                                "}\n";
            const char expected[]  = "const C < 1 , 2 , 3 > foo ( ) "
                                     "{"
                                     " return C < 1 , 2 , 3 > ( x , y ) ; "
                                     "}";
            ASSERT_EQUALS(expected, sizeof_(code));
        }
    }

    void conditionOperator()
    {
        {
            const char code[] = "; x = a ? b : c;";
            ASSERT_EQUALS("; if ( a ) { x = b ; } else { x = c ; }", tok(code));
        }

        {
            const char code[] = "(0?(false?1:2):3)";
            ASSERT_EQUALS("( 3 )", tok(code));
        }

        {
            const char code[] = "(1?(false?1:2):3)";
            ASSERT_EQUALS("( 2 )", tok(code));
        }

        {
            const char code[] = "int a = (1?0:1 == 1?0:1);";
            ASSERT_EQUALS("int a ; a = 0 ;", tok(code));
        }

        {
            const char code[] = "(1?0:foo())";
            ASSERT_EQUALS("( 0 )", tok(code));
        }

        {
            const char code[] = "( true ? a ( ) : b ( ) )";
            ASSERT_EQUALS("( a ( ) )", tok(code));
        }

        {
            const char code[] = "( true ? abc . a : abc . b )";
            ASSERT_EQUALS("( abc . a )", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  bool x = false;\n"
                                "  int b = x ? 44 : 3;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { ; }", tok(code));
        }

        {
            const char code[] = "int vals[] = { 0x13, 1?0x01:0x00 };";
            ASSERT_EQUALS("int * vals ; vals = { 19 , 1 } ;", tok(code));
        }

        {
            const char code[] = "int vals[] = { 0x13, 0?0x01:0x00 };";
            ASSERT_EQUALS("int * vals ; vals = { 19 , 0 } ;", tok(code));
        }

        {
            const char code[] = "= 1 ? 0 : ({ 0; });";
            ASSERT_EQUALS("= 0 ;", tok(code));
        }
    }

    void calculations()
    {
        {
            const char code[] = "a[i+8+2]";
            ASSERT_EQUALS("a [ i + 10 ]", tok(code));
        }
        {
            const char code[] = "a[8+2+i]";
            ASSERT_EQUALS("a [ 10 + i ]", tok(code));
        }
        {
            const char code[] = "a[i + 2 * (2 * 4)]";
            ASSERT_EQUALS("a [ i + 16 ]", tok(code));
        }
        {
            const char code[] = "a[i + 100 - 90]";
            ASSERT_EQUALS("a [ i + 10 ]", tok(code));
        }
        {
            const char code[] = "a[1+1+1+1+1+1+1+1+1+1-2+5-3]";
            ASSERT_EQUALS("a [ 10 ]", tok(code));
        }
        {
            const char code[] = "a[10+10-10-10]";
            ASSERT_EQUALS("a [ 0 ]", tok(code));
        }

        ASSERT_EQUALS("a [ 4 ] ;", tok("a[1+3|4];"));

        ASSERT_EQUALS("x = 1 + 2 * y ;", tok("x=1+2*y;"));
        ASSERT_EQUALS("x = 7 ;", tok("x=1+2*3;"));
        ASSERT_EQUALS("x = 47185 ;", tok("x=(65536*72/100);"));
        ASSERT_EQUALS("x = 900 ;", tok("x = 1500000 / ((145000 - 55000) * 1000 / 54000);"));
        ASSERT_EQUALS("int a [ 8 ] ;", tok("int a[5+6/2];"));
        ASSERT_EQUALS("int a [ 4 ] ;", tok("int a[(10)-1-5];"));
        ASSERT_EQUALS("int a [ i - 9 ] ;", tok("int a[i - 10 + 1];"));

        ASSERT_EQUALS("x = y ;", tok("x=0+y+0-0;"));
        ASSERT_EQUALS("x = 0 ;", tok("x=0*y;"));

        ASSERT_EQUALS("x = 501 ;", tok("x = 1000 + 2 >> 1;"));
        ASSERT_EQUALS("x = 125 ;", tok("x = 1000 / 2 >> 2;"));

        {
            // Ticket #1997
            const char code[] = "void * operator new[](size_t);";
            ASSERT_EQUALS("void * operatornew[] ( size_t ) ;", tok(code));
        }

        ASSERT_EQUALS("; a [ 0 ] ;", tok(";a[0*(*p)];"));

        ASSERT_EQUALS(";", tok("; x = x + 0;"));
    }


    void goto1()
    {
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if (a())\n"
                                "    {\n"
                                "        goto out;\n"
                                "    }\n"
                                "    b();\n"
                                "out:\n"
                                "    c();\n"
                                "}";

            errout.str("");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            tokenizer.simplifyTokenList();
            tokenizer.validate();

            const char expect[] = "\n\n##file 0\n"
                                  "1: void foo ( )\n"
                                  "2: {\n"
                                  "3: if ( a ( ) )\n"
                                  "4: {\n"
                                  "5:\n6:\n7:\n8:\n"
                                  "9: c ( ) ; return ; }\n"
                                  "7: b ( ) ;\n"
                                  "8:\n"
                                  "9: c ( ) ;\n"
                                  "10: }\n";

            ASSERT_EQUALS(expect, tokenizer.tokens()->stringifyList(""));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if (a())\n"
                                "        goto out;\n"
                                "    b();\n"
                                "out:\n"
                                "    if (c())\n"
                                "        d();\n"
                                "}";


            errout.str("");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            tokenizer.simplifyTokenList();
            tokenizer.validate();

            const char expect[] = "\n\n##file 0\n"
                                  "1: void foo ( )\n"
                                  "2: {\n"
                                  "3: if ( a ( ) ) {\n"
                                  "4:\n5:\n6:\n"
                                  "7: if ( c ( ) ) {\n"
                                  "8: d ( ) ; } return ; }\n"
                                  "5: b ( ) ;\n"
                                  "6:\n"
                                  "7: if ( c ( ) ) {\n"
                                  "8: d ( ) ; }\n"
                                  "9: }\n";

            ASSERT_EQUALS(expect, tokenizer.tokens()->stringifyList(""));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if (a())\n"
                                "        goto out;\n"
                                "    b();\n"
                                "out:\n"
                                "    if (c())\n"
                                "    {\n"
                                "      d();\n"
                                "    }\n"
                                "}";

            const char expect[] = "void foo ( ) "
                                  "{ "
                                  "if ( a ( ) ) "
                                  "{ "
                                  "if ( c ( ) ) "
                                  "{ d ( ) ; } "
                                  "return ; "
                                  "} "
                                  "b ( ) ; "
                                  "if ( c ( ) ) "
                                  "{ d ( ) ; } "
                                  "}";

            ASSERT_EQUALS(expect, tok(code));
        }

        {
            const char code[] = "void foo(int x)\n"
                                "{\n"
                                "    if (a())\n"
                                "        goto out;\n"
                                "    b();\n"
                                "out:\n"
                                "    if (x)\n"
                                "    {\n"
                                "      x++; b[0]=x;\n"
                                "    }\n"
                                "}";

            const char expect[] = "void foo ( int x ) "
                                  "{ "
                                  "if ( a ( ) ) "
                                  "{ "
                                  "if ( x ) "
                                  "{ x ++ ; b [ 0 ] = x ; } "
                                  "return ; "
                                  "} "
                                  "b ( ) ; "
                                  "if ( x ) "
                                  "{ x ++ ; b [ 0 ] = x ; } "
                                  "}";

            ASSERT_EQUALS(expect, tok(code));
        }

        {
            const char code[] = "class NoLabels { bool varOne : 1 ; bool varTwo : 1 ; } ;";
            const char expect[] = "class NoLabels { bool varOne ; bool varTwo ; } ;";
            ASSERT_EQUALS(expect, tok(code));
        }

        {
            const char code[] = "void foo ( ) { int var ; var = x < y ? y : z ; } ;";
            ASSERT_EQUALS(code, tok(code));
        }

        {
            const char code[] = "void foo(int x)\n"
       