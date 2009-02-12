/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



#include "testsuite.h"
#include "../src/tokenize.h"
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
        TEST_CASE(cast0);
        TEST_CASE(sizeof1);
        TEST_CASE(iftruefalse);
        TEST_CASE(combine_strings);
        // TODO TEST_CASE(double_plus);
    }

    std::string tok(const char code[])
    {
        std::istringstream istr(code);
        Tokenizer tokenizer;
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();
        tokenizer.simplifyTokenList();
        std::string ret;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            ret += tok->str() + " ";
        }

        return ret;
    }

    void cast0()
    {
        const char code1[] = " if ( p == (char *)0 ) ";
        const char code2[] = " if ( p == 0 ) ";
        ASSERT_EQUALS(tok(code1), tok(code2));
    }

    void sizeof1()
    {
        const char code1[] = " struct ABC *abc = malloc(sizeof(*abc)); ";
        const char code2[] = " struct ABC *abc = malloc(100); ";
        const char code3[] = " struct ABC *abc = malloc(sizeof *abc ); ";
        ASSERT_EQUALS(tok(code1), tok(code2));
        ASSERT_EQUALS(tok(code2), tok(code3));
    }

    void iftruefalse()
    {
        {
            const char code1[] = " void f() { int a; bool use = false; if( use ) { a=0; } else {a=1;} } ";
            const char code2[] = " void f() { int a; bool use = false; {a=1;} } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = true; if( use ) { a=0; } else {a=1;} } ";
            const char code2[] = " void f() { int a; bool use = true; { a=0; } } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; int use = 5; if( use ) { a=0; } else {a=1;} } ";
            const char code2[] = " void f() { int a; int use = 5; { a=0; } } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; int use = 0; if( use ) { a=0; } else {a=1;} } ";
            const char code2[] = " void f() { int a; int use = 0; {a=1;} } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = false; if( use ) a=0; else a=1; int c=1; } ";
            const char code2[] = " void f() { int a; bool use = false; { a=1; } int c=1; } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = true; if( use ) a=0; else a=1; int c=1; } ";
            const char code2[] = " void f() { int a; bool use = true; { a=0; } int c=1; } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = false; if( use ) a=0; else if( bb ) a=1; int c=1; } ";
            const char code2[] = " void f() { int a; bool use = false; if( bb ) a=1; int c=1; } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = true; if( use ) a=0; else if( bb ) a=1; int c=1; } ";
            const char code2[] = " void f() { int a; bool use = true; { a=0;} int c=1; } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { int a; bool use = true; if( use ) a=0; else if( bb ) a=1; else if( cc ) a=33; else { gg = 0; } int c=1; } ";
            const char code2[] = " void f() { int a; bool use = true; { a=0; }int c=1; } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { if( aa ) { a=0; } else if( true ) a=1; else { a=2; } } ";
            const char code2[] = " void f() { if( aa ) { a=0; } else { a=1; } } ";
            ASSERT_EQUALS(tok(code2), tok(code1));
        }

        {
            const char code1[] = " void f() { if( aa ) { a=0; } else if( false ) a=1; else { a=2; } } ";
            const char code2[] = " void f() { if( aa ) { a=0; } else { a=2; } } ";
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
            ASSERT_EQUALS("void foo ( int a ) { a ++ ; a -- ; ++ a ; -- a ; } ", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                  "{\n"
                                  "a=a+a;\n"
                                  "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + a ; } ", tok(code1));
        }
    }
};

REGISTER_TEST(TestSimplifyTokens)
