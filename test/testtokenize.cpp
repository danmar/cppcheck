/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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


// The preprocessor that c++check uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include <cstring>
#include "testsuite.h"
#include "../src/tokenize.h"

extern std::ostringstream errout;
class TestTokenizer : public TestFixture
{
public:
    TestTokenizer() : TestFixture("TestTokenizer")
    { }

    class OurTokenizer : public Tokenizer
    {
    public:
        void simplifyCasts()
        {
            Tokenizer::simplifyCasts();
        }

        bool simplifyIfAddBraces()
        {
            return Tokenizer::simplifyIfAddBraces();
        }

        bool simplifyKnownVariables()
        {
            return Tokenizer::simplifyKnownVariables();
        }

        std::vector<const Token *> &getFunctionList()
        {
            return _functionList;
        }
    };

private:

    void run()
    {
        TEST_CASE(minus);

        TEST_CASE(longtok);

        TEST_CASE(removeCast1);

        TEST_CASE(inlineasm);

        TEST_CASE(dupfuncname);

        TEST_CASE(const_and_volatile_functions);

        TEST_CASE(ifAddBraces1);
        TEST_CASE(ifAddBraces2);
        TEST_CASE(ifAddBraces3);
        TEST_CASE(ifAddBraces4);
        TEST_CASE(ifAddBraces5);
        TEST_CASE(ifAddBraces6);

        TEST_CASE(numeric_true_condition);

        TEST_CASE(simplifyKnownVariables1);
        TEST_CASE(simplifyKnownVariables2);
        TEST_CASE(simplifyKnownVariables3);
        TEST_CASE(simplifyKnownVariables4);
        TEST_CASE(simplifyKnownVariables5);
        TEST_CASE(simplifyKnownVariables6);
        TEST_CASE(simplifyKnownVariables7);
        TEST_CASE(simplifyKnownVariables8);
        TEST_CASE(simplifyKnownVariables9);
        TEST_CASE(simplifyKnownVariables10);

        TEST_CASE(multiCompare);

        TEST_CASE(match1);

        TEST_CASE(match2);

        TEST_CASE(varid1);
        TEST_CASE(varid2);
        TEST_CASE(varid3);
        TEST_CASE(varid4);
        TEST_CASE(varid5);
        TEST_CASE(varid6);
        TEST_CASE(varid7);
        TEST_CASE(varidReturn);
        TEST_CASE(varid8);
        TEST_CASE(varid9);
        TEST_CASE(varidStl);
        TEST_CASE(varid_delete);

        TEST_CASE(varidclass1);
        TEST_CASE(varidclass2);
        TEST_CASE(varidclass3);
        TEST_CASE(varidclass4);

        TEST_CASE(file1);
        TEST_CASE(file2);
        TEST_CASE(file3);

        TEST_CASE(doublesharp);

        TEST_CASE(macrodoublesharp);

        TEST_CASE(simplify_function_parameters);

        TEST_CASE(reduce_redundant_paranthesis);        // Ticket #61

        TEST_CASE(simplify_numeric_condition);
        TEST_CASE(tokenize_double);
        TEST_CASE(tokenize_strings);
        TEST_CASE(simplify_constants);
        TEST_CASE(simplify_constants2);

        TEST_CASE(findClassFunction1);

        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(volatile_variables);
        TEST_CASE(syntax_error);
    }


    bool cmptok(const char *expected[], const Token *actual)
    {
        unsigned int i = 0;
        for (; expected[i] && actual; ++i, actual = actual->next())
        {
            if (strcmp(expected[i], actual->str().c_str()) != 0)
                return false;
        }
        return (expected[i] == NULL && actual == NULL);
    }


    std::string tokenizeAndStringify(const char code[])
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            ostr << tok->str();

            // Append newlines
            if (tok->next())
            {
                if (tok->linenr() != tok->next()->linenr())
                {
                    for (unsigned int i = tok->linenr();i < tok->next()->linenr();++i)
                        ostr << "\n";
                }
                else
                {
                    ostr << " ";
                }
            }
        }

        return ostr.str();
    }


    void minus()
    {
        ASSERT_EQUALS("i = -12", tokenizeAndStringify("i = -12"));
        ASSERT_EQUALS("1 - 2", tokenizeAndStringify("1-2"));
        ASSERT_EQUALS("foo ( -1 ) - 2", tokenizeAndStringify("foo(-1)-2"));
        ASSERT_EQUALS("int f ( ) { return -2 ; }", tokenizeAndStringify("int f(){return -2;}"));
    }



    void longtok()
    {
        std::string filedata(10000, 'a');

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(filedata);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        ASSERT_EQUALS(std::string(10000, 'a'), tokenizer.tokens()->str());
    }



    // Dont remove "(int *)"..
    void removeCast1()
    {
        const char code[] = "int *f(int *);";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyCasts();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" int * f ( int * ) ;"), ostr.str());
    }


    void inlineasm()
    {
        {
            const char code[] = "abc asm { mov ax,bx } def";
            ASSERT_EQUALS("abc def", tokenizeAndStringify(code));
        }

        {
            const char code[] = "abc _asm { mov ax,bx } def";
            ASSERT_EQUALS("abc def", tokenizeAndStringify(code));
        }

        {
            const char code[] = "abc __asm { mov ax,bx } def";
            ASSERT_EQUALS("abc def", tokenizeAndStringify(code));
        }
    }


    void dupfuncname()
    {
        const char code[] = "void a()\n"
                            "{ }\n"
                            "void a(int i)\n"
                            "{ }\n"
                            "void b()\n"
                            "{ }\n";
        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.fillFunctionList();

        ASSERT_EQUALS(1, static_cast<unsigned int>(tokenizer.getFunctionList().size()));
        ASSERT_EQUALS(std::string("b"), tokenizer.getFunctionList()[0]->str());
    }

    void const_and_volatile_functions()
    {
        const char code[] = "class B\n\
                            {\n\
                            public:\n\
                            void a();\n\
                            void b() const;\n\
                            void c() volatile;\n\
                            };\n\
                            \n\
                            void B::a()\n\
                            {}\n\
                            \n\
                            void B::b() const\n\
                            {}\n\
                            \n\
                            void B::c() volatile\n\
                            {}\n";


        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.fillFunctionList();

        ASSERT_EQUALS(3, static_cast<unsigned int>(tokenizer.getFunctionList().size()));
        if (tokenizer.getFunctionList().size() == 3)
        {
            ASSERT_EQUALS(std::string("a"), tokenizer.getFunctionList()[0]->str());
            ASSERT_EQUALS(std::string("b"), tokenizer.getFunctionList()[1]->str());
            ASSERT_EQUALS(std::string("c"), tokenizer.getFunctionList()[2]->str());
        }
    }


    void numeric_true_condition()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (5==5);\n"
                            "}\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { { ; } }"), ostr.str());
    }

    void ifAddBraces1()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a);\n"
                            "    else ;\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(true, tokenizer.simplifyIfAddBraces());

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { if ( a ) { ; } else { ; } }"), ostr.str());
    }

    void ifAddBraces2()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) if (b) { }\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(true, tokenizer.simplifyIfAddBraces());

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { if ( a ) { if ( b ) { } } }"), ostr.str());
    }

    void ifAddBraces3()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) for (;;) { }\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(true, tokenizer.simplifyIfAddBraces());

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { if ( a ) { for ( ; ; ) { } } }"), ostr.str());
    }

    void ifAddBraces4()
    {
        const char code[] = "char * foo ()\n"
                            "{\n"
                            "    char *str = malloc(10);\n"
                            "    if (somecondition)\n"
                            "        for ( ; ; )\n"
                            "        { }\n"
                            "    return str;\n"
                            "}\n";


        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(true, tokenizer.simplifyIfAddBraces());

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" char * foo ( ) { char * str ; str = malloc ( 10 ) ; if ( somecondition ) { for ( ; ; ) { } } return str ; }"), ostr.str());
    }

    void ifAddBraces5()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "for(int i = 0; i < 2; i++)\n"
                            "if(true)\n"
                            "return;\n"
                            "\n"
                            "return;\n"
                            "}\n";


        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(true, tokenizer.simplifyIfAddBraces());

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { for ( int i = 0 ; i < 2 ; i ++ ) { if ( true ) { return ; } } return ; }"), ostr.str());
    }

    void ifAddBraces6()
    {
        const char code[] = "if()";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(false, tokenizer.simplifyIfAddBraces());

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" if ( )"), ostr.str());
    }

    void simplifyKnownVariables1()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 10;\n"
                            "    if (a);\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { int a ; a = 10 ; if ( 10 ) ; }"), ostr.str());
    }

    void simplifyKnownVariables2()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 10;\n"
                            "    a = g();\n"
                            "    if (a);\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { int a ; a = 10 ; a = g ( ) ; if ( a ) ; }"), ostr.str());
    }

    void simplifyKnownVariables3()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    while(true){\n"
                            "    break;\n"
                            "    a = 10;\n"
                            "    }\n"
                            "    if (a);\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { int a ; a = 4 ; while ( true ) { break ; a = 10 ; } if ( a ) ; }"), ostr.str());
    }

    void simplifyKnownVariables4()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( g(a));\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { int a ; a = 4 ; if ( g ( 4 ) ) ; }"), ostr.str());
    }

    void simplifyKnownVariables5()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( a = 5 );\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { int a ; a = 4 ; if ( a = 5 ) ; }"), ostr.str());
    }

    void simplifyKnownVariables6()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    char str[2];"
                            "    int a = 4;\n"
                            "    str[a] = 0;\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { char str [ 2 ] ; int a ; a = 4 ; str [ 4 ] = 0 ; }"), ostr.str());
    }

    void simplifyKnownVariables7()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i = 22;\n"
                            "    abc[i++] = 1;\n"
                            "    abc[++i] = 2;\n"
                            "}\n";
        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void foo ( ) { int i ; i = 24 ; abc [ 22 ] = 1 ; abc [ 24 ] = 2 ; }"), ostr.str());
    }

    void simplifyKnownVariables8()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i = 22;\n"
                            "    i++;\n"
                            "    abc[i] = 0;\n"
                            "}\n";
        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void foo ( ) { int i ; i = 23 ; ; abc [ 23 ] = 0 ; }"), ostr.str());
    }

    void simplifyKnownVariables9()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int a = 1, b = 2;\n"
                            "    if (a < b)\n"
                            "        ;\n"
                            "}\n";
        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void foo ( ) { int a ; a = 1 ; int b ; b = 2 ; if ( 1 < 2 ) ; }"), ostr.str());
    }

    void simplifyKnownVariables10()
    {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  bool b=false;\n"
                                "\n"
                                "  {\n"
                                "    b = true;\n"
                                "  }\n"
                                "\n"
                                "  if( b )\n"
                                "  {\n"
                                "    a();\n"
                                "  }\n"
                                "}\n";
            // tokenize..
            OurTokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.setVarId();
            tokenizer.simplifyKnownVariables();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(std::string(" void f ( ) { bool b ; b = false ; { b = true ; } if ( true ) { a ( ) ; } }"), ostr.str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  bool b=false;\n"
                                "  { b = false; }\n"
                                "  {\n"
                                "    b = true;\n"
                                "  }\n"
                                "\n"
                                "  if( b )\n"
                                "  {\n"
                                "    a();\n"
                                "  }\n"
                                "}\n";
            // tokenize..
            OurTokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.setVarId();
            tokenizer.simplifyKnownVariables();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(std::string(" void f ( ) { bool b ; b = false ; { b = false ; } { b = true ; } if ( true ) { a ( ) ; } }"), ostr.str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  int b=0;\n"
                                "  b = 1;\n"
                                "  for( int i = 0; i < 10; i++ )"
                                "  {\n"
                                "  }\n"
                                "\n"
                                "  a(b);\n"
                                "}\n";
            // tokenize..
            OurTokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.setVarId();
            tokenizer.simplifyKnownVariables();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(std::string(" void f ( ) { int b ; b = 0 ; b = 1 ; for ( int i = 0 ; i < 10 ; i ++ ) { } a ( 1 ) ; }"), ostr.str());
        }
    }

    void multiCompare()
    {
        // Test for found
        ASSERT_EQUALS(1, Token::multiCompare("one|two", "one"));
        ASSERT_EQUALS(1, Token::multiCompare("one|two", "two"));
        ASSERT_EQUALS(1, Token::multiCompare("verybig|two|", "two"));

        // Test for empty string found
        ASSERT_EQUALS(0, Token::multiCompare("|one|two", "notfound"));
        ASSERT_EQUALS(0, Token::multiCompare("one||two", "notfound"));
        ASSERT_EQUALS(0, Token::multiCompare("one|two|", "notfound"));

        // Test for not found
        ASSERT_EQUALS(-1, Token::multiCompare("one|two", "notfound"));
        ASSERT_EQUALS(-1, Token::multiCompare("verybig|two", "s"));
        ASSERT_EQUALS(-1, Token::multiCompare("one|two", "ne"));
        ASSERT_EQUALS(-1, Token::multiCompare("abc|def", "a"));
        ASSERT_EQUALS(-1, Token::multiCompare("abc|def", "abcd"));
        ASSERT_EQUALS(-1, Token::multiCompare("abc|def", "default"));
    }

    void match1()
    {
        // Match "%var% | %var%"
        {
            const std::string code("abc|def");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(true, Token::Match(tokenizer.tokens(), "%var% | %var%"));
        }

        // Match "%var% || %var%"
        {
            const std::string code("abc||def");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(true, Token::Match(tokenizer.tokens(), "%var% || %var%"));
        }
    }

    void match2()
    {
        {
            const std::string code("");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(true, Token::Match(tokenizer.tokens(), "!!else"));
        }

        {
            const std::string code("");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(false, Token::Match(tokenizer.tokens(), "!!else something"));
        }

        {
            const std::string code("if ;");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(true, Token::Match(tokenizer.tokens(), "!!return if"));
        }

        {
            const std::string code("if ;");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(true, Token::Match(tokenizer.tokens(), "if ; !!else"));
        }

        {
            const std::string code("if ; something");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(true, Token::Match(tokenizer.tokens(), "if ; !!else"));
        }

        {
            const std::string code("else");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(false, Token::Match(tokenizer.tokens(), "!!else"));
        }

        {
            const std::string code("if ; else");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS(false, Token::Match(tokenizer.tokens(), "if ; !!else"));
        }
    }

    void varid1()
    {
        const std::string code("static int i = 1;\n"
                               "void f()\n"
                               "{\n"
                               "    int i = 2;\n"
                               "    for (int i = 0; i < 10; ++i)\n"
                               "        i = 3;\n"
                               "    i = 4;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: static int i@1 ; i@1 = 1 ;\n"
                                   "2: void f ( )\n"
                                   "3: {\n"
                                   "4: int i@2 ; i@2 = 2 ;\n"
                                   "5: for ( int i@3 = 0 ; i@3 < 10 ; ++ i@3 )\n"
                                   "6: i@3 = 3 ;\n"
                                   "7: i@2 = 4 ;\n"
                                   "8: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid2()
    {
        const std::string code("void f()\n"
                               "{\n"
                               "    struct ABC abc;\n"
                               "    abc.a = 3;\n"
                               "    i = abc.a;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: struct ABC abc@1 ;\n"
                                   "4: abc@1 . a@2 = 3 ;\n"
                                   "5: i = abc@1 . a@2 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid3()
    {
        const std::string code("static char str[4];\n"
                               "void f()\n"
                               "{\n"
                               "    char str[10];\n"
                               "    str[0] = 0;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: static char str@1 [ 4 ] ;\n"
                                   "2: void f ( )\n"
                                   "3: {\n"
                                   "4: char str@2 [ 10 ] ;\n"
                                   "5: str@2 [ 0 ] = 0 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid4()
    {
        const std::string code("void f(const unsigned int a[])\n"
                               "{\n"
                               "    int i = *(a+10);\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( const int a@1 [ ] )\n"
                                   "2: {\n"
                                   "3: int i@2 ; i@2 = a@1 [ 10 ] ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid5()
    {
        const std::string code("void f()\n"
                               "{\n"
                               "    int a,b;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: int a@1 ; int b@2 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varid6()
    {
        const std::string code("int f(int a, int b)\n"
                               "{\n"
                               "    return a+b;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: int f ( int a@1 , int b@2 )\n"
                                   "2: {\n"
                                   "3: return a@1 + b@2 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varid7()
    {
        const std::string code("void func()\n"
                               "{\n"
                               "char a[256] = \"test\";\n"
                               "{\n"
                               "char b[256] = \"test\";\n"
                               "}\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: void func ( )\n"
                                   "2: {\n"
                                   "3: char a@1 [ 256 ] = \"test\" ;\n"
                                   "4: {\n"
                                   "5: char b@2 [ 256 ] = \"test\" ;\n"
                                   "6: }\n"
                                   "7: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidReturn()
    {
        const std::string code("int f()\n"
                               "{\n"
                               "    int a;\n"
                               "    return a;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: int f ( )\n"
                                   "2: {\n"
                                   "3: int a@1 ;\n"
                                   "4: return a@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid8()
    {
        const std::string code("void func()\n"
                               "{\n"
                               "    std::string str(\"test\");\n"
                               "    str.clear();\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: void func ( )\n"
                                   "2: {\n"
                                   "3: std :: string str@1 ( \"test\" ) ;\n"
                                   "4: str@1 . clear@2 ( ) ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid9()
    {
        const std::string code("typedef int INT32;\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: typedef int INT32 ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidStl()
    {
        const std::string code("list<int> ints;\n"
                               "list<int>::iterator it;\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: list < int > ints@1 ;\n"
                                   "2: list < int > :: iterator it@2 ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid_delete()
    {
        const std::string code("void f()\n"
                               "{\n"
                               "  int *a;\n"
                               "  delete a;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: int * a@1 ;\n"
                                   "4: delete a@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass1()
    {
        const std::string code("class Fred\n"
                               "{\n"
                               "private:\n"
                               "    int i;\n"
                               "\n"
                               "    void foo1();\n"
                               "    void foo2()\n"
                               "    {\n"
                               "        ++i;\n"
                               "    }\n"
                               "}\n"
                               "\n"
                               "Fred::foo1()\n"
                               "{\n"
                               "    i = 0;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: {\n"
                                   "3: private:\n"
                                   "4: int i@1 ;\n"
                                   "5:\n"
                                   "6: void foo1@2 ( ) ;\n"
                                   "7: void foo2 ( )\n"
                                   "8: {\n"
                                   "9: ++ i@1 ;\n"
                                   "10: }\n"
                                   "11: }\n"
                                   "12:\n"
                                   "13: Fred :: foo1 ( )\n"
                                   "14: {\n"
                                   "15: i@1 = 0 ;\n"
                                   "16: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass2()
    {
        const std::string code("class Fred\n"
                               "{ void f(); };\n"
                               "\n"
                               "void A::foo1()\n"
                               "{\n"
                               "    int i = 0;\n"
                               "}\n"
                               "\n"
                               "void Fred::f()\n"
                               "{\n"
                               "    i = 0;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: { void f@1 ( ) ; } ;\n"
                                   "3:\n"
                                   "4: void A :: foo1 ( )\n"
                                   "5: {\n"
                                   "6: int i@2 ; i@2 = 0 ;\n"
                                   "7: }\n"
                                   "8:\n"
                                   "9: void Fred :: f ( )\n"
                                   "10: {\n"
                                   "11: i = 0 ;\n"
                                   "12: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass3()
    {
        const std::string code("class Fred\n"
                               "{ int i; void f(); };\n"
                               "\n"
                               "void Fred::f()\n"
                               "{\n"
                               "    i = 0;\n"
                               "}\n"
                               "\n"
                               "void A::f()\n"
                               "{\n"
                               "    i = 0;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: { int i@1 ; void f@2 ( ) ; } ;\n"
                                   "3:\n"
                                   "4: void Fred :: f ( )\n"
                                   "5: {\n"
                                   "6: i@1 = 0 ;\n"
                                   "7: }\n"
                                   "8:\n"
                                   "9: void A :: f ( )\n"
                                   "10: {\n"
                                   "11: i = 0 ;\n"
                                   "12: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass4()
    {
        const std::string code("class Fred\n"
                               "{ int i; void f(); };\n"
                               "\n"
                               "void Fred::f()\n"
                               "{\n"
                               "    if (i) { }\n"
                               "    i = 0;\n"
                               "}\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred\n"
                                   "2: { int i@1 ; void f@2 ( ) ; } ;\n"
                                   "3:\n"
                                   "4: void Fred :: f ( )\n"
                                   "5: {\n"
                                   "6: if ( i@1 ) { }\n"
                                   "7: i@1 = 0 ;\n"
                                   "8: }\n");

        ASSERT_EQUALS(expected, actual);
    }



    void file1()
    {
        const char code[] = "a1\n"
                            "#file \"b\"\n"
                            "b1\n"
                            "b2\n"
                            "#endfile\n"
                            "a3\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "a");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            std::ostringstream ostr;
            ostr << char('a' + tok->fileIndex()) << tok->linenr();
            ASSERT_EQUALS(tok->str(), ostr.str());
        }
    }


    void file2()
    {
        const char code[] = "a1\n"
                            "#file \"b\"\n"
                            "b1\n"
                            "b2\n"
                            "#file \"c\"\n"
                            "c1\n"
                            "c2\n"
                            "#endfile\n"
                            "b4\n"
                            "#endfile\n"
                            "a3\n"
                            "#file \"d\"\n"
                            "d1\n"
                            "#endfile\n"
                            "a5\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "a");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            std::ostringstream ostr;
            ostr << char('a' + tok->fileIndex()) << tok->linenr();
            ASSERT_EQUALS(tok->str(), ostr.str());
        }
    }



    void file3()
    {
        const char code[] = "#file \"c:\\a.h\"\n"
                            "123\n"
                            "#endfile\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "a.cpp");


        ASSERT_EQUALS("[c:\\a.h:1]", tokenizer.fileLine(tokenizer.tokens()));
    }




    void doublesharp()
    {
        const char code[] = "TEST(var,val) var##_##val = val\n";

        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "");

        // Stringify the tokens..
        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << tok->str() << " ";

        ASSERT_EQUALS("TEST ( var , val ) var ## _ ## val = val ", ostr.str());
    }

    void macrodoublesharp()
    {
        const char code[] = "DBG(fmt,args...) printf(fmt, ## args)\n";

        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "");

        // Stringify the tokens..
        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << tok->str() << " ";

        ASSERT_EQUALS("DBG ( fmt , args . . . ) printf ( fmt , ## args ) ", ostr.str());
    }

    void simplify_function_parameters()
    {
        {
            const char code[] = "void f(x) int x;\n"
                                "{\n"
                                "}\n";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(std::string(" void f ( int x ) { }"), ostr.str());
        }

        {
            const char code[] = "void f(x,y) int x; char y;\n"
                                "{\n"
                                "}\n";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(std::string(" void f ( int x , char y ) { }"), ostr.str());
        }

        {
            // This is not a function but the pattern is similar..
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if (x)\n"
                                "        int x;\n"
                                "    { }\n"
                                "}\n";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(std::string(" void foo ( ) { if ( x ) { int x ; } { } }"), ostr.str());
        }
    }


    // Simplify "((..))" into "(..)"
    void reduce_redundant_paranthesis()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    free(((void*)p));\n"
                            "}";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void foo ( ) { free ( p ) ; }"), ostr.str());
    }



    void simplify_numeric_condition()
    {
        const char code[] =
            "void f()\n"
            "{\n"
            "int x = 0;\n"
            "if( !x || 0 )\n"
            "{\n"
            "}\n"
            "}";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { int x ; x = 0 ; if ( ! x ) { } }"), ostr.str());
    }

    void tokenize_double()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    double a = 4.2;\n"
                            "    float b = 4.2f;\n"
                            "    double c = 4.2e+10;\n"
                            "    double d = 4.2e-10;\n"
                            "    int e = 4+2;\n"
                            "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { double a ; a = 4.2 ; float b ; b = 4.2f ; double c ; c = 4.2e+10 ; double d ; d = 4.2e-10 ; int e ; e = 4 + 2 ; }"), ostr.str());
    }

    void tokenize_strings()
    {
        const char code[] =   "void f()\n"
                              "{\n"
                              "const char *a =\n"
                              "{\n"
                              "\"hello \"\n"
                              "\"more \"\n"
                              "\"world\"\n"
                              "};\n"
                              "}\n";

        // tokenize..
        OurTokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { const char * a ; a = { \"hello more world\" } ; }"), ostr.str());
    }

    void simplify_constants()
    {
        const char code[] =
            "void f()\n"
            "{\n"
            "const int a = 45;\n"
            "if( a )\n"
            "{ int b = a; }\n"
            "}\n"
            "void g()\n"
            "{\n"
            "int a = 2;\n"
            "}\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" void f ( ) { const int a = 45 ; { int b ; b = 45 ; } } void g ( ) { int a ; a = 2 ; }"), ostr.str());
    }

    void simplify_constants2()
    {
        const char code[] =
            "void f( Foo &foo, Foo *foo2 )\n"
            "{\n"
            "const int a = 45;\n"
            "foo.a=a+a;\n"
            "foo2->a=a;\n"
            "}\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();

        std::ostringstream oss;
        oss << " void f ( Foo & foo , Foo * foo2 ) { const int a = 45 ; foo . a = 90 ; foo2 . a = 45 ; }";
        ASSERT_EQUALS(oss.str(), ostr.str());
    }


    void findClassFunction1()
    {
        const char code[] =
            "class Fred"
            "{\n"
            "public:\n"
            "    Fred()\n"
            "    { }\n"
            "};\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        int i;

        i = 0;
        const Token *tok = Tokenizer::FindClassFunction(tokenizer.tokens(), "Fred", "%var%", i);
        ASSERT_EQUALS(true, Token::simpleMatch(tok, "Fred ( ) {"));
        tok = Tokenizer::FindClassFunction(tok->next(), "Fred", "%var%", i);
        ASSERT_EQUALS(0, tok ? 1 : 0);
    }

    void vardecl1()
    {
        const char code[] = "unsigned int a, b;";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("unsigned int a ; unsigned int b ;", actual);
    }

    void vardecl2()
    {
        const char code[] = "void foo(a,b) unsigned int a, b; { }";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("void foo ( a , b ) unsigned int a ; unsigned int b ; { }", actual);
    }

    void volatile_variables()
    {
        const char code[] = "volatile int a=0;\n"
                            "volatile int b=0;\n"
                            "volatile int c=0;\n";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("int a ; a = 0 ;\nint b ; b = 0 ;\nint c ; c = 0 ;", actual);
    }

    void syntax_error()
    {
        {
            const char code[] = "void f() {}";
            Tokenizer tokenizer;
            std::istringstream istr(code);
            ASSERT_EQUALS(true, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS(std::string(""), errout.str());
        }

        {
            const char code[] = "void f() {{}";
            Tokenizer tokenizer;
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            TODO_ASSERT_EQUALS(std::string("correct error message here"), errout.str());
        }

        {
            const char code[] = "void f()) {}";
            Tokenizer tokenizer;
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            TODO_ASSERT_EQUALS(std::string("correct error message here"), errout.str());
        }
    }
};

REGISTER_TEST(TestTokenizer)
