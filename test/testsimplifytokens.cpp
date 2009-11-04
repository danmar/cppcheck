/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
#define private public
#include "tokenize.h"
#undef private
#include "token.h"
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
        TEST_CASE(cast);
        TEST_CASE(iftruefalse);
        TEST_CASE(combine_strings);
        TEST_CASE(double_plus);
        TEST_CASE(redundant_plus);
        TEST_CASE(parantheses1);
        TEST_CASE(paranthesesVar);      // Remove redundant parantheses around variable .. "( %var% )"
        TEST_CASE(declareVar);

        TEST_CASE(declareArray);

        TEST_CASE(dontRemoveIncrement);
        TEST_CASE(removePostIncrement);
        TEST_CASE(removePreIncrement);

        TEST_CASE(elseif1);

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
        TEST_CASE(casting);

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
        TEST_CASE(template_default_parameter);
        TEST_CASE(template_typename);

        TEST_CASE(namespaces);

        // Assignment in condition..
        TEST_CASE(ifassign1);
        TEST_CASE(whileAssign);

        // "if(0==x)" => "if(!x)"
        TEST_CASE(ifnot);
        TEST_CASE(combine_wstrings);

        // Simplify "not" to "!" (#345)
        TEST_CASE(not1);

        // Simplify "and" to "&&" (#620)
        TEST_CASE(and1);

        TEST_CASE(comma_keyword);
        TEST_CASE(remove_comma);

        // Simplify "?:"
        TEST_CASE(conditionOperator);

        // Simplify calculations
        TEST_CASE(calculations);

        // Simplify goto..
        TEST_CASE(goto1);

        // Simplify nested strcat() calls
        TEST_CASE(strcat1);

        // Syntax error
        TEST_CASE(argumentsWithSameName)

        TEST_CASE(simplifyAtol)
        TEST_CASE(simplifyHexInString)
        TEST_CASE(simplifyTypedef)
        TEST_CASE(simplifyTypedef2)
        TEST_CASE(simplifyTypedef3)
        TEST_CASE(simplifyTypedef4)
        TEST_CASE(simplifyTypedef5)
        TEST_CASE(reverseArraySyntax)
        TEST_CASE(simplify_numeric_condition);

        TEST_CASE(pointeralias);
    }

    std::string tok(const char code[], bool simplify = true)
    {
        std::istringstream istr(code);
        Tokenizer tokenizer;
        tokenizer.tokenize(istr, "test.cpp");
        if (simplify)
            tokenizer.simplifyTokenList();

        tokenizer.validate();
        std::string ret;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (tok != tokenizer.tokens())
                ret += " ";
            ret += tok->str();
        }

        return ret;
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
        ASSERT_EQUALS("class A { A operator * ( int ) ; } ;", tok("class A { A operator *(int); };"));
        ASSERT_EQUALS("class A { A operator * ( int ) const ; } ;", tok("class A { A operator *(int) const; };"));
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
            const char code1[] = " void f() { int a; bool use = true; if( use ) a=0; else if( bb ) a=1; else if( cc ) a=33; else { gg = 0; } int c=1; }";
            const char code2[] = " void f() { int a; bool use = true; { a=0; }int c=1; }";
            ASSERT_EQUALS(tok(code2), tok(code1));
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
    }

    void paranthesesVar()
    {
        // remove parantheses..
        ASSERT_EQUALS("= p ;", tok("= (p);"));
        ASSERT_EQUALS("if ( a < p ) { }", tok("if(a<(p)){}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p == -1 ) { } }", tok("void f(){int p; if((p)==-1){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( -1 == p ) { } }", tok("void f(){int p; if(-1==(p)){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p ) { } }", tok("void f(){int p; if((p)){}}"));
        ASSERT_EQUALS("return p ;", tok("return (p);"));
        ASSERT_EQUALS("void f ( ) { int * p ; if ( ! * p ) { } }", tok("void f(){int *p; if (*(p) == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { int * p ; if ( ! * p ) { } }", tok("void f(){int *p; if (*p == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { int p ; p = 1 ; }", tok("void f(){int p; (p) = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p [ 10 ] ; p [ 0 ] = 1 ; }", tok("void f(){int p[10]; (p)[0] = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( ! p ) { } }", tok("void f(){int p; if ((p) == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { int * p ; * p = 1 ; }", tok("void f(){int *p; *(p) = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p ) { } p = 1 ; }", tok("void f(){int p; if ( p ) { } (p) = 1;}"));

        // keep parantheses..
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
        const char expected[] = "void f ( ) { char * str ; str = \"100\" ; }";
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
        const char code[] = "void f()\n"
                            "{\n"
                            "    unsigned int c = 0;\n"
                            "    c++;\n"
                            "    if (c>0) { c++; }\n"
                            "    c++;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { int c ; c = 3 ; ; { ; } ; }", tok(code));
    }


    void removePreIncrement()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    unsigned int c = 0;\n"
                            "    ++c;\n"
                            "    if (c>0) { ++c; }\n"
                            "    ++c;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { int c ; c = 3 ; ; { ; } ; }", tok(code));
    }


    std::string elseif(const char code[])
    {
        std::istringstream istr(code);

        Tokenizer tokenizer;
        tokenizer.createTokens(istr);
        tokenizer.elseif();
        return tokenizer.tokens()->stringifyList(false);
    }

    void elseif1()
    {
        const char code[] = "else if(ab) { cd } else { ef }gh";
        ASSERT_EQUALS("\n\n##file 0\n1: else { if ( ab ) { cd } else { ef } } gh\n", elseif(code));
    }






    // Simplify 'sizeof'..
    std::string sizeof_(const char code[])
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (tok->previous())
            {
                ostr << " ";
            }
            ostr << tok->str();
        }

        return ostr.str();
    }

    unsigned int sizeofFromTokenizer(const char type[])
    {
        Tokenizer tokenizer;
        std::istringstream istr("");
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();
        Token tok;
        tok.str(type);
        return tokenizer.sizeOfType(&tok);
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
            "for (int i = 0; i < sizeof(names[0]); i++)"
            "{}";
        std::ostringstream expected;
        expected << "const char * names [ 2 ] ; for ( int i = 0 ; i < " << sizeofFromTokenizer("*") << " ; i ++ ) { }";
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
                                "  int a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << (sizeofFromTokenizer("*") * 2);
            ASSERT_EQUALS("void f ( ) { char * ptrs [ 2 ] ; int a ; a = " + oss.str() + " ; }", sizeof_(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs[55];\n"
                                "  int a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << (sizeofFromTokenizer("*") * 55);
            ASSERT_EQUALS("void f ( ) { char * ptrs [ 55 ] ; int a ; a = " + oss.str() + " ; }", sizeof_(code));
        }


        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs;\n"
                                "  int a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << sizeofFromTokenizer("*");
            ASSERT_EQUALS("void f ( ) { char * ptrs ; int a ; a = " + oss.str() + " ; }", sizeof_(code));
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
            expected << "; const char * str ; str = \"1\" ; " << sizeofFromTokenizer("char")*2 << " ;";

            ASSERT_EQUALS(expected.str(), sizeof_(code));
        }

        {
            const char code[] = "; const char str[] = {'1'}; sizeof(str);";

            const char str[] = {'1'};
            std::ostringstream expected;
            expected << "; const char * str ; str = { '1' } ; " << sizeof(str) << " ;";

            TODO_ASSERT_EQUALS(expected.str(), sizeof_(code));
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
                                " int * p ;"
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



    void template1()
    {
        const char code[] = "template <classname T> void f(T val) { T a; }\n"
                            "f<int>(10);";

        const std::string expected("; f<int> ( 10 ) ; "
                                   "void f<int> ( int val ) { int a ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template2()
    {
        const char code[] = "template <classname T> class Fred { T a; };\n"
                            "Fred<int> fred;";

        const std::string expected("; ; "
                                   "Fred<int> fred ; "
                                   "class Fred<int> { int a ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template3()
    {
        const char code[] = "template <classname T, int sz> class Fred { T data[sz]; };\n"
                            "Fred<float,4> fred;";

        const std::string expected("; ; "
                                   "Fred<float,4> fred ; "
                                   "class Fred<float,4> { float data [ 4 ] ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template4()
    {
        const char code[] = "template <classname T> class Fred { Fred(); };\n"
                            "Fred<float> fred;";

        const std::string expected("; ; "
                                   "Fred<float> fred ; "
                                   "class Fred<float> { Fred<float> ( ) ; }");

        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void template5()
    {
        const char code[] = "template <classname T> class Fred { };\n"
                            "template <classname T> Fred<T>::Fred() { }\n"
                            "Fred<float> fred;";

        const std::string expected("; ; "
                                   "; "
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

        const std::string expected("; ;"
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

            const std::string expected("; ;");

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

            const std::string expected("; ; "
                                       "int main ( ) { "
                                       "std :: vector < int > v ; "
                                       "v . push_back ( 4 ) ; "
                                       "return 0 ; "
                                       "}");

            TODO_ASSERT_EQUALS(expected, sizeof_(code));
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

            const std::string expected("; ;");

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

        ASSERT_EQUALS("; ; typedef A < int > x ; typedef B < int > y ; ; ; ;", sizeof_(code));

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
        std::string expected("; ; void f ( ) { A<int> a ; } ; ; class A<int> { }");

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
        const std::string expected("; ; "
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

        // The expected result..
        const std::string expected("; "
                                   "; "
                                   "int main ( ) { b<2> ( ) ; return 0 ; } "
                                   "void b<2> ( ) { a<2> ( ) ; } "
                                   "void a<2> ( ) { }");

        ASSERT_EQUALS(expected, sizeof_(code));
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
            const std::string expected("; ; "
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
            const std::string expected("; ; "
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

            // The expected result..
            const std::string expected("template < class T , int n >"
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
            TODO_ASSERT_EQUALS(expected, sizeof_(code));
        }
    }

    void template_typename()
    {
        const char code[] = "template <class T>\n"
                            "void foo(typename T::t *)\n"
                            "{ }";

        // The expected result..
        const std::string expected(";");
        ASSERT_EQUALS(expected, sizeof_(code));
    }

    void namespaces()
    {
        {
            const char code[] = "using namespace std; namespace a{ namespace b{ void f(){} } }";

            const std::string expected("using namespace std ; void f ( ) { }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "namespace b{ void f(){} }";

            const std::string expected("void f ( ) { }");

            ASSERT_EQUALS(expected, sizeof_(code));
        }

        {
            const char code[] = "int a; namespace b{ }";

            const std::string expected("int a ;");

            ASSERT_EQUALS(expected, sizeof_(code));
        }
    }


    std::string simplifyIfAssign(const char code[])
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyIfAssign();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << (tok->previous() ? " " : "") << tok->str();

        return ostr.str();
    }

    void ifassign1()
    {
        ASSERT_EQUALS("; a = b ; if ( a ) { ; }", simplifyIfAssign(";if(a=b);"));
        ASSERT_EQUALS("; a = b ( ) ; if ( ( a ) ) { ; }", simplifyIfAssign(";if((a=b()));"));
        ASSERT_EQUALS("; a = b ( ) ; if ( ! ( a ) ) { ; }", simplifyIfAssign(";if(!(a=b()));"));
        ASSERT_EQUALS("; a . x = b ( ) ; if ( ! ( a . x ) ) { ; }", simplifyIfAssign(";if(!(a->x=b()));"));
        ASSERT_EQUALS("A ( ) a = b ; if ( a ) { ; }", simplifyIfAssign("A() if(a=b);"));
        ASSERT_EQUALS("void foo ( int a ) { a = b ( ) ; if ( 0 <= a ) { ; } }", tok("void foo(int a) {if((a=b())>=0);}"));
        TODO_ASSERT_EQUALS("void foo ( A a ) { a . c = b ( ) ; if ( 0 <= a . c ) { ; } }", tok("void foo(A a) {if((a.c=b())>=0);}"));
    }


    void whileAssign()
    {
        ASSERT_EQUALS("; a = b ; while ( a ) { b = 0 ; a = b ; }", simplifyIfAssign(";while(a=b) { b = 0; }"));
        ASSERT_EQUALS("; a . b = c ; while ( a . b ) { c = 0 ; a . b = c ; }", simplifyIfAssign(";while(a.b=c) { c=0; }"));
    }


    std::string simplifyIfNot(const char code[])
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyIfNot();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << (tok->previous() ? " " : "") << tok->str();

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
        ASSERT_EQUALS("if ( ( ! x ) )", simplifyIfNot("if((x==0))"));
        ASSERT_EQUALS("if ( ( ! x ) && ! y )", simplifyIfNot("if((x==0) && y==0)"));
        ASSERT_EQUALS("if ( ! ( ! fclose ( fd ) ) )", simplifyIfNot("if(!(fclose(fd) == 0))"));
    }



    std::string simplifyLogicalOperators(const char code[])
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyLogicalOperators();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << (tok->previous() ? " " : "") << tok->str();

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
            const char code[] = "typedef enum { a = 0 , b = 1 , c = 2 } abc ;";
            ASSERT_EQUALS(code, sizeof_(code));
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
            ASSERT_EQUALS("void f ( ) { bool x ; x = false ; int b ; b = 3 ; }", tok(code));
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

        ASSERT_EQUALS("x = 1 + 2 * y ;", tok("x=1+2*y;"));
        ASSERT_EQUALS("x = 7 ;", tok("x=1+2*3;"));
        ASSERT_EQUALS("x = 47185 ;", tok("x=(65536*72/100);"));
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

            const char expect[] = "void foo ( ) "
                                  "{ "
                                  "if ( a ( ) ) "
                                  "{ "
                                  "c ( ) ; "
                                  "return ; "
                                  "} "
                                  "b ( ) ; "
                                  "c ( ) ; "
                                  "}";

            ASSERT_EQUALS(expect, tok(code));
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
            ASSERT_EQUALS(code, tok(code));
        }

        {
            const char code[] = "void foo ( ) { int var ; var = x < y ? y : z ; } ;";
            ASSERT_EQUALS(code, tok(code));
        }
    }


    void strcat1()
    {
        const char code[] = "; strcat(strcat(strcat(strcat(strcat(strcat(dst, \"this \"), \"\"), \"is \"), \"a \"), \"test\"), \".\");";
        const char expect[] = "; "
                              "strcat ( dst , \"this \" ) ; "
                              "strcat ( dst , \"\" ) ; "
                              "strcat ( dst , \"is \" ) ; "
                              "strcat ( dst , \"a \" ) ; "
                              "strcat ( dst , \"test\" ) ; "
                              "strcat ( dst , \".\" ) ;";

        ASSERT_EQUALS(expect, tok(code));
    }

    void argumentsWithSameName()
    {
        // This code has syntax error, two variables can not have the same name
        {
            const char code[] = "void foo(x, x)\n"
                                " int x;\n"
                                " int x;\n"
                                "{}\n";
            ASSERT_EQUALS("void foo ( x , x ) int x ; int x ; { }", tok(code));
        }

        {
            const char code[] = "void foo(x, y)\n"
                                " int x;\n"
                                " int x;\n"
                                "{}\n";
            ASSERT_EQUALS("void foo ( int x , y ) int x ; { }", tok(code));
        }
    }

    void simplifyAtol()
    {
        ASSERT_EQUALS("a = std :: atol ( x ) ;", tok("a = std::atol(x);"));
        ASSERT_EQUALS("a = atol ( \"text\" ) ;", tok("a = atol(\"text\");"));
        ASSERT_EQUALS("a = 0 ;", tok("a = std::atol(\"0\");"));
        ASSERT_EQUALS("a = 10 ;", tok("a = atol(\"0xa\");"));
    }

    void simplifyHexInString()
    {
        ASSERT_EQUALS("\"a\"", tok("\"\\x61\""));
        ASSERT_EQUALS("\"a\"", tok("\"\\141\""));

        ASSERT_EQUALS("\"\\0\"", tok("\"\\x00\""));
        ASSERT_EQUALS("\"\\0\"", tok("\"\\000\""));

        ASSERT_EQUALS("\"\\nhello\"", tok("\"\\nhello\""));

        ASSERT_EQUALS("\"aaa\"", tok("\"\\x61\\x61\\x61\""));
        ASSERT_EQUALS("\"aaa\"", tok("\"\\141\\141\\141\""));

        ASSERT_EQUALS("\"\\\\x61\"", tok("\"\\\\x61\""));

        // These tests can fail, if other characters are handled
        // more correctly. But fow now all non null characters should
        // become 'a'
        ASSERT_EQUALS("\"a\"", tok("\"\\x62\""));
        ASSERT_EQUALS("\"a\"", tok("\"\\177\""));
    }

    void simplifyTypedef()
    {
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
                            "}\n";

        const std::string expected =
            "class A "
            "{ "
            "public: "
            "typedef wchar_t duplicate ; "
            "void foo ( ) { } "
            "} ; "
            "typedef A duplicate ; "
            "int main ( ) "
            "{ "
            "A a ; "
            "a . foo ( ) ; "
            "wchar_t c ; c = 0 ; "
            "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef2()
    {
        const char code[] = "class A;\n"
                            "typedef A duplicate;\n"
                            "class A\n"
                            "{\n"
                            "public:\n"
                            "typedef wchar_t duplicate;\n"
                            "duplicate foo() { wchar_t b; return b; }\n"
                            "};\n";

        const std::string expected =
            "class A ; "
            "typedef A duplicate ; "
            "class A "
            "{ "
            "public: "
            "typedef wchar_t duplicate ; "
            "wchar_t foo ( ) { wchar_t b ; return b ; } "
            "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef3()
    {
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
                            "}\n";

        const std::string expected =
            "class A { } ; "
            "typedef A duplicate ; "
            "wchar_t foo ( ) "
            "{ "
            "typedef wchar_t duplicate ; "
            "wchar_t b ; "
            "return b ; "
            "} "
            "int main ( ) "
            "{ "
            "A b ; "
            "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef4()
    {
        const char code[] = "typedef int s32;\n"
                            "typedef unsigned int u32;\n"
                            "void f()\n"
                            "{\n"
                            "    s32 ivar = -2;\n"
                            "    u32 uvar = 2;\n"
                            "    return uvar / ivar;\n"
                            "}\n";

        const std::string expected =
            "typedef int s32 ; "
            "typedef unsigned int u32 ; "
            "void f ( ) "
            "{ "
            "int ivar ; ivar = -2 ; "
            "unsigned int uvar ; uvar = 2 ; "
            "return uvar / ivar ; "
            "}";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef5()
    {
        // ticket #780
        const char code[] =
            "typedef struct yy_buffer_state *YY_BUFFER_STATE;\n"
            "void f()\n"
            "{\n"
            "    YY_BUFFER_STATE state;\n"
            "}\n";

        const char expected[] =
            "typedef struct yy_buffer_state * YY_BUFFER_STATE ; "
            "void f ( ) "
            "{ "
            "struct yy_buffer_state * state ; "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void reverseArraySyntax()
    {
        ASSERT_EQUALS("a [ 13 ]", tok("13[a]"));
    }


    void simplify_numeric_condition()
    {
        {
            const char code[] =
                "void f()\n"
                "{\n"
                "int x = 0;\n"
                "if( !x || 0 )\n"
                "{ g();\n"
                "}\n"
                "}";

            ASSERT_EQUALS("void f ( ) { int x ; x = 0 ; { g ( ) ; } }", tok(code));
        }

        {
            const char code[] =
                "void f()\n"
                "{\n"
                "int x = 1;\n"
                "if( !x )\n"
                "{ g();\n"
                "}\n"
                "}";

            ASSERT_EQUALS("void f ( ) { int x ; x = 1 ; }", tok(code));
        }

        {
            const char code[] =
                "void f()\n"
                "{\n"
                "bool x = true;\n"
                "if( !x )\n"
                "{ g();\n"
                "}\n"
                "}";

            ASSERT_EQUALS("void f ( ) { bool x ; x = true ; }", tok(code));
        }

        {
            const char code[] =
                "void f()\n"
                "{\n"
                "bool x = false;\n"
                "if( !x )\n"
                "{ g();\n"
                "}\n"
                "}";

            ASSERT_EQUALS("void f ( ) { bool x ; x = false ; { g ( ) ; } }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    if (5==5);\n"
                                "}\n";

            ASSERT_EQUALS("void f ( ) { { ; } }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    if (4<5);\n"
                                "}\n";

            ASSERT_EQUALS("void f ( ) { { ; } }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    if (5<5);\n"
                                "}\n";

            ASSERT_EQUALS("void f ( ) { }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    if (13>12?true:false);\n"
                                "}\n";

            ASSERT_EQUALS("void f ( ) { { ; } }", tok(code));
        }
    }


    void pointeralias()
    {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    char buf[100];\n"
                                "    char *p = buf;\n"
                                "    x(p);\n"
                                "}\n";

            const char expected[] = "void f ( ) "
                                    "{ "
                                    "char buf [ 100 ] ; "
                                    "char * p ; p = buf ; "
                                    "x ( buf ) ; "
                                    "}";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f(char *p1)\n"
                                "{\n"
                                "    char *p = p1;\n"
                                "    p1 = 0;"
                                "    x(p);\n"
                                "}\n";

            const char expected[] = "void f ( char * p1 ) "
                                    "{ "
                                    "char * p ; p = p1 ; "
                                    "p1 = 0 ; "
                                    "x ( p ) ; "
                                    "}";

            ASSERT_EQUALS(expected, tok(code));
        }
    }
};

REGISTER_TEST(TestSimplifyTokens)
