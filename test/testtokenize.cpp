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
        TEST_CASE(longtok);

        TEST_CASE(removeCast1);

        TEST_CASE(inlineasm);

        TEST_CASE(dupfuncname);

        TEST_CASE(const_and_volatile_functions);

        TEST_CASE(ifAddBraces1);
        TEST_CASE(ifAddBraces2);
        TEST_CASE(ifAddBraces3);
        TEST_CASE(ifAddBraces4);

        TEST_CASE(numeric_true_condition);

        TEST_CASE(simplifyKnownVariables1);
        TEST_CASE(simplifyKnownVariables2);
        TEST_CASE(simplifyKnownVariables3);
        TEST_CASE(simplifyKnownVariables4);
        TEST_CASE(simplifyKnownVariables5);

        TEST_CASE(multiCompare);

        TEST_CASE(match1);

        TEST_CASE(match2);

        TEST_CASE(varid1);
        TEST_CASE(varid2);
        TEST_CASE(varid3);

        TEST_CASE(file1);
        TEST_CASE(file2);

        TEST_CASE(doublesharp);

        TEST_CASE(simplify_function_parameters);

        TEST_CASE(reduce_redundant_paranthesis);        // Ticket #61

        TEST_CASE(sizeof1);
        TEST_CASE(sizeof2);
        TEST_CASE(sizeof3);
        TEST_CASE(sizeof4);
        TEST_CASE(simplify_numeric_condition);
        TEST_CASE(tokenize_double);
        TEST_CASE(tokenize_strings);
        TEST_CASE(simplify_constants);
    }


    bool cmptok(const char *expected[], const Token *actual)
    {
        unsigned int i = 0;
        for (; expected[i] && actual; ++i, actual = actual->next())
        {
            if (strcmp(expected[i], actual->aaaa()) != 0)
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



    void longtok()
    {
        std::string filedata(10000, 'a');

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(filedata);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        ASSERT_EQUALS(std::string(10000, 'a'), std::string(tokenizer.tokens()->aaaa()));
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
        const char filedata[] = "void foo()\n"
                                "{\n"
                                "    __asm\n"
                                "    {\n"
                                "        jmp $jump1\n"
                                "        $jump1:\n"
                                "    }\n"
                                "}\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(filedata);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        const char *expected[] =
        {
            "void",
            "foo",
            "(",
            ")",
            "{",
            "}",
            0
        };

        // Compare..
        ASSERT_EQUALS(true, cmptok(expected, tokenizer.tokens()));
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
        ASSERT_EQUALS(std::string("b"), tokenizer.getFunctionList()[0]->aaaa());
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
        ASSERT_EQUALS(std::string(" char * foo ( ) { char * str = malloc ( 10 ) ; if ( somecondition ) { for ( ; ; ) { } } return str ; }"), ostr.str());
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
        ASSERT_EQUALS(std::string(" void f ( ) { int a = 10 ; if ( 10 ) ; }"), ostr.str());
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
        ASSERT_EQUALS(std::string(" void f ( ) { int a = 10 ; a = g ( ) ; if ( a ) ; }"), ostr.str());
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
        ASSERT_EQUALS(std::string(" void f ( ) { int a = 4 ; while ( true ) { break ; a = 10 ; } if ( a ) ; }"), ostr.str());
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
        ASSERT_EQUALS(std::string(" void f ( ) { int a = 4 ; if ( g ( a ) ) ; }"), ostr.str());
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
        ASSERT_EQUALS(std::string(" void f ( ) { int a = 4 ; if ( a = 5 ) ; }"), ostr.str());
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
        const std::string code(";static int i = 1;\n"
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

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (tok->str() != "i")
                ASSERT_EQUALS(0, tok->varId());
            else if (Token::Match(tok, "i = 1"))
                ASSERT_EQUALS(1, tok->varId());
            else if (Token::Match(tok, "i = 2"))
                ASSERT_EQUALS(2, tok->varId());
            else if (Token::Match(tok, "i = 3"))
                ASSERT_EQUALS(3, tok->varId());
            else if (Token::Match(tok, "i = 4"))
                ASSERT_EQUALS(2, tok->varId());
        }
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

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (tok->str() == "abc")
                ASSERT_EQUALS(1, tok->varId());
            else if (tok->str() == "a")
                ASSERT_EQUALS(2, tok->varId());
            else
                ASSERT_EQUALS(0, tok->varId());
        }
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

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (Token::Match(tok, "str [ 4"))
                ASSERT_EQUALS(1, tok->varId());
            else if (tok->str() == "str")
                ASSERT_EQUALS(2, tok->varId());
        }
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




    void sizeof1()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i[4];\n"
                            "    sizeof(i);\n"
                            "    sizeof(*i);\n"
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
        ASSERT_EQUALS(std::string(" void foo ( ) { int i [ 4 ] ; 16 ; 4 ; }"), ostr.str());
    }

    void sizeof2()
    {
        const char code[] = "static int i[4];\n"
                            "void f()\n"
                            "{\n"
                            "    int i[10];\n"
                            "    sizeof(i);\n"
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
        ASSERT_EQUALS(std::string(" static int i [ 4 ] ; void f ( ) { int i [ 10 ] ; 40 ; }"), ostr.str());
    }

    void sizeof3()
    {
        const char code[] = "int i[10];\n"
                            "sizeof(i[0]);\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" int i [ 10 ] ; 4 ;"), ostr.str());
    }


    void sizeof4()
    {
        const char code[] =
            "for (int i = 0; i < sizeof(g_ReservedNames[0]); i++)"
            "{}";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(std::string(" for ( int i = 0 ; i < 100 ; i + + ) { }"), ostr.str());
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
        ASSERT_EQUALS(std::string(" void f ( ) { double a = 4.2 ; float b = 4.2f ; double c = 4.2e+10 ; double d = 4.2e-10 ; int e = 4 + 2 ; }"), ostr.str());
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
        ASSERT_EQUALS(std::string(" void f ( ) { const char * a = { \"hello more world\" } ; }"), ostr.str());
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
};

REGISTER_TEST(TestTokenizer)
