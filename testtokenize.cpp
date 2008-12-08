/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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
#define UNIT_TESTING        // Get access to "private" data in Tokenizer
#include "tokenize.h"

extern std::ostringstream errout;
class TestTokenizer : public TestFixture
{
public:
    TestTokenizer() : TestFixture("TestTokenizer")
    { }

private:

    void run()
    {
        TEST_CASE( multiline );
        TEST_CASE( longtok );

        TEST_CASE( inlineasm );

        TEST_CASE( dupfuncname );

        TEST_CASE( const_and_volatile_functions );

        TEST_CASE( numeric_true_condition );

        TEST_CASE( multi_compare );

        TEST_CASE( match1 );

        TEST_CASE( varid1 );
    }


    bool cmptok(const char *expected[], const TOKEN *actual)
    {
        unsigned int i = 0;
        for (; expected[i] && actual; ++i, actual = actual->next())
        {
            if ( strcmp( expected[i], actual->aaaa() ) != 0)
                return false;
        }
        return (expected[i] == NULL && actual == NULL);
    }


    void multiline()
    {
        const char filedata[] = "#define str \"abc\" \\\n"
                                "            \"def\"\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(filedata);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        const char *expected[] =
        {
            "def",
            "str",
            ";",
            0
        };

        // Compare..
        ASSERT_EQUALS( true, cmptok(expected, tokenizer.tokens()) );
    }


    void longtok()
    {
        std::string filedata(10000,'a');

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(filedata);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        ASSERT_EQUALS( std::string(10000,'a'), std::string(tokenizer.tokens()->aaaa()) );
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
        ASSERT_EQUALS( true, cmptok(expected, tokenizer.tokens()) );
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
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.fillFunctionList();

        ASSERT_EQUALS( 1, static_cast<unsigned int>(tokenizer._functionList.size()) );
        ASSERT_EQUALS( std::string("b"), tokenizer._functionList[0]->aaaa() );
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
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.fillFunctionList();

        ASSERT_EQUALS( 3, static_cast<unsigned int>(tokenizer._functionList.size()) );
        if( tokenizer._functionList.size() == 3 )
        {
            ASSERT_EQUALS( std::string("a"), tokenizer._functionList[0]->str() );
            ASSERT_EQUALS( std::string("b"), tokenizer._functionList[1]->str() );
            ASSERT_EQUALS( std::string("c"), tokenizer._functionList[2]->str() );
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
        for (const TOKEN *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS( std::string(" void f ( ) { if ( true ) ; }"), ostr.str() );
    }

    void multi_compare()
    {
        // Test for found
        ASSERT_EQUALS( 1, TOKEN::multiCompare( "one|two", "one" ) );
        ASSERT_EQUALS( 1, TOKEN::multiCompare( "one|two", "two" ) );
        ASSERT_EQUALS( 1, TOKEN::multiCompare( "verybig|two|", "two" ) );

        // Test for empty string found
        ASSERT_EQUALS( 0, TOKEN::multiCompare( "|one|two", "notfound" ) );
        ASSERT_EQUALS( 0, TOKEN::multiCompare( "one||two", "notfound" ) );
        ASSERT_EQUALS( 0, TOKEN::multiCompare( "one|two|", "notfound" ) );

        // Test for not found
        ASSERT_EQUALS( -1, TOKEN::multiCompare( "one|two", "notfound" ) );
        ASSERT_EQUALS( -1, TOKEN::multiCompare( "verybig|two", "s" ) );
        ASSERT_EQUALS( -1, TOKEN::multiCompare( "one|two", "ne" ) );
        ASSERT_EQUALS( -1, TOKEN::multiCompare( "abc|def", "a" ) );
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
            ASSERT_EQUALS( true, TOKEN::Match(tokenizer.tokens(), "%var% | %var%") );
        }

        // Match "%var% || %var%"
        {
            const std::string code("abc||def");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            // Match..
            ASSERT_EQUALS( true, TOKEN::Match(tokenizer.tokens(), "%var% || %var%") );
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
                               "}\n" );

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        for ( const TOKEN *tok = tokenizer.tokens(); tok; tok = tok->next() )
        {
            if ( tok->str() != "i" )
                ASSERT_EQUALS( 0, tok->varId() );
            else if ( TOKEN::Match(tok, "i = 1") )
                ASSERT_EQUALS( 1, tok->varId() );
            else if ( TOKEN::Match(tok, "i = 2") )
                ASSERT_EQUALS( 2, tok->varId() );
            else if ( TOKEN::Match(tok, "i = 3") )
                ASSERT_EQUALS( 3, tok->varId() );
            else if ( TOKEN::Match(tok, "i = 4") )
                ASSERT_EQUALS( 2, tok->varId() );
        }
    }
};

REGISTER_TEST( TestTokenizer )
