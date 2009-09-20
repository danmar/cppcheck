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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include <cstring>
#include "testsuite.h"
#define private public
#include "../src/tokenize.h"
#undef private
#include "../src/token.h"

extern std::ostringstream errout;
class TestTokenizer : public TestFixture
{
public:
    TestTokenizer() : TestFixture("TestTokenizer")
    { }

private:

    void run()
    {
        TEST_CASE(minus);

        TEST_CASE(longtok);

        TEST_CASE(removeCast1);
        TEST_CASE(removeCast2);

        TEST_CASE(inlineasm);

        TEST_CASE(dupfuncname);

        TEST_CASE(const_and_volatile_functions);

        TEST_CASE(ifAddBraces1);
        TEST_CASE(ifAddBraces2);
        TEST_CASE(ifAddBraces3);
        TEST_CASE(ifAddBraces4);
        TEST_CASE(ifAddBraces5);
        TEST_CASE(ifAddBraces6);
        TEST_CASE(ifAddBraces7);
        TEST_CASE(ifAddBraces8);

        TEST_CASE(whileAddBraces);
        TEST_CASE(doWhileAddBraces);

        TEST_CASE(numeric_true_condition);
        TEST_CASE(pointers_condition);

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
        TEST_CASE(simplifyKnownVariables11);
        TEST_CASE(simplifyKnownVariables12);
        TEST_CASE(simplifyKnownVariables13);

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
        TEST_CASE(varid10);
        TEST_CASE(varid11);
        TEST_CASE(varidStl);
        TEST_CASE(varid_delete);
        TEST_CASE(varid_functions);
        TEST_CASE(varid_reference_to_containers);
        TEST_CASE(varid_in_class);

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

        TEST_CASE(removeParantheses1);        // Ticket #61
        TEST_CASE(removeParantheses2);
        TEST_CASE(removeParantheses3);
        TEST_CASE(removeParantheses4);       // Ticket #390
        TEST_CASE(removeParantheses5);       // Ticket #392

        TEST_CASE(simplify_numeric_condition);
        TEST_CASE(tokenize_double);
        TEST_CASE(tokenize_strings);
        TEST_CASE(simplify_constants);
        TEST_CASE(simplify_constants2);

        TEST_CASE(findClassFunction1);

        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(vardecl4);
        TEST_CASE(vardecl5);
        TEST_CASE(vardecl6);
        TEST_CASE(vardecl7);
        TEST_CASE(vardecl8);
        TEST_CASE(vardecl9);
        TEST_CASE(vardecl_stl);
        TEST_CASE(volatile_variables);
        TEST_CASE(syntax_error);

        TEST_CASE(removeKeywords);

        // unsigned i; => unsigned int i;
        TEST_CASE(unsigned1);
        TEST_CASE(testUpdateClassList);
        TEST_CASE(createLinks);
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


    std::string tokenizeAndStringify(const char code[], bool simplify = false)
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        if (simplify)
            tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            ostr << tok->str();

            // Append newlines
            if (tok->next())
            {
                if (tok->linenr() != tok->next()->linenr())
                {
                    for (unsigned int i = tok->linenr(); i < tok->next()->linenr(); ++i)
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
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyCasts();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" int * f ( int * ) ;", ostr.str());
    }

    // remove static_cast..
    void removeCast2()
    {
        const char code[] = "t = (static_cast<std::vector<int> *>(&p));\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyCasts();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" t = ( & p ) ;", ostr.str());
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
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.fillFunctionList();

        ASSERT_EQUALS(1, static_cast<unsigned int>(tokenizer._functionList.size()));
        ASSERT_EQUALS("b", tokenizer._functionList[0]->str());
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

        ASSERT_EQUALS(3, static_cast<unsigned int>(tokenizer._functionList.size()));
        if (tokenizer._functionList.size() == 3)
        {
            ASSERT_EQUALS("a", tokenizer._functionList[0]->str());
            ASSERT_EQUALS("b", tokenizer._functionList[1]->str());
            ASSERT_EQUALS("c", tokenizer._functionList[2]->str());
        }
    }


    void numeric_true_condition()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (5==5);\n"
                            "}\n";

        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "{ ; }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void pointers_condition()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (p != NULL);\n"
                            "    if (NULL != p);\n"
                            "    if (this->p != NULL);\n"
                            "    if (NULL != this->p);\n"
                            "    if (Foo::p != NULL);\n"
                            "    if (NULL != Foo::p);\n"
                            "    while (p != NULL);\n"
                            "    while (NULL != p);\n"
                            "    while (this->p != NULL);\n"
                            "    while (NULL != this->p);\n"
                            "    while (Foo::p != NULL);\n"
                            "    while (NULL != Foo::p);\n"
                            "    if (p == NULL);\n"
                            "    if (NULL == p);\n"
                            "    if (this->p == NULL);\n"
                            "    if (NULL == this->p);\n"
                            "    if (Foo::p == NULL);\n"
                            "    if (NULL == Foo::p);\n"
                            "    while (p == NULL);\n"
                            "    while (NULL == p);\n"
                            "    while (this->p == NULL);\n"
                            "    while (NULL == this->p);\n"
                            "    while (Foo::p == NULL);\n"
                            "    while (NULL == Foo::p);\n"
                            "    if (p1 != NULL || p2 == NULL) { ; }\n"
                            "    if (p1 != NULL && p2 == NULL) { ; }\n"
                            "    if (p == '\\0');\n"
                            "}\n";

        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( p ) { ; }\n"
                      "if ( p ) { ; }\n"
                      "if ( this . p ) { ; }\n"
                      "if ( this . p ) { ; }\n"
                      "if ( Foo :: p ) { ; }\n"
                      "if ( Foo :: p ) { ; }\n"
                      "while ( p ) { ; }\n"
                      "while ( p ) { ; }\n"
                      "while ( this . p ) { ; }\n"
                      "while ( this . p ) { ; }\n"
                      "while ( Foo :: p ) { ; }\n"
                      "while ( Foo :: p ) { ; }\n"
                      "if ( ! p ) { ; }\n"
                      "if ( ! p ) { ; }\n"
                      "if ( ! this . p ) { ; }\n"
                      "if ( ! this . p ) { ; }\n"
                      "if ( ! Foo :: p ) { ; }\n"
                      "if ( ! Foo :: p ) { ; }\n"
                      "while ( ! p ) { ; }\n"
                      "while ( ! p ) { ; }\n"
                      "while ( ! this . p ) { ; }\n"
                      "while ( ! this . p ) { ; }\n"
                      "while ( ! Foo :: p ) { ; }\n"
                      "while ( ! Foo :: p ) { ; }\n"
                      "if ( p1 || ! p2 ) { ; }\n"
                      "if ( p1 && ! p2 ) { ; }\n"
                      "if ( ! p ) { ; }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces1()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a);\n"
                            "    else ;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { ; }\n"
                      "else { ; }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces2()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) if (b) { }\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { if ( b ) { } }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces3()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) for (;;) { }\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { for ( ; ; ) { } }\n"
                      "}", tokenizeAndStringify(code, true));
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
        ASSERT_EQUALS("char * foo ( )\n"
                      "{\n"
                      "char * str ; str = malloc ( 10 ) ;\n"
                      "if ( somecondition ) {\n"
                      "for ( ; ; )\n"
                      "{ } }\n"
                      "return str ;\n"
                      "}", tokenizeAndStringify(code, true));
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

        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "for ( int i = 0 ; i < 2 ; i ++ ) {\n"
                      "{\n"
                      "return ; } }\n\n"
                      "return ;\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces6()
    {
        const char code[] = "if()";
        ASSERT_EQUALS("if ( )", tokenizeAndStringify(code, true));
    }

    void ifAddBraces7()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "int a;\n"
                            "if( a )\n"
                            "  ({a=4;}),({a=5;});\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "int a ;\n"
                      "if ( a ) {\n"
                      "( { a = 4 ; } ) , ( { a = 5 ; } ) ; }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces8()
    {
        const char code[] = "do { ; } while(0);";
        ASSERT_EQUALS("do { ; } while ( false ) ;", tokenizeAndStringify(code, true));
    }

    void whileAddBraces()
    {
        {
            const char code[] = ";while(a);";
            ASSERT_EQUALS("; while ( a ) { ; }", tokenizeAndStringify(code, true));
        }
    }

    void doWhileAddBraces()
    {
        const char code[] = "do ; while (0);";
        const char result[] = "do { ; } while ( false ) ;";

        ASSERT_EQUALS(result, tokenizeAndStringify(code, true));
    }

    std::string simplifyKnownVariables(const char code[])
    {
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyKnownVariables();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (tok->previous())
                ostr << " ";
            ostr << tok->str();
        }

        return ostr.str();
    }

    void simplifyKnownVariables1()
    {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    int a = 10;\n"
                                "    if (a);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void f ( ) { int a ; a = 10 ; if ( 10 ) ; }",
                simplifyKnownVariables(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    int a = 10;\n"
                                "    if (!a);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void f ( ) { int a ; a = 10 ; if ( ! 10 ) ; }",
                simplifyKnownVariables(code));
        }
    }

    void simplifyKnownVariables2()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 10;\n"
                            "    a = g();\n"
                            "    if (a);\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 10 ; a = g ( ) ; if ( a ) ; }",
            simplifyKnownVariables(code));
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

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; while ( true ) { break ; a = 10 ; } if ( a ) ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables4()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( g(a));\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; if ( g ( 4 ) ) ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables5()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( a = 5 );\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; if ( a = 5 ) ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables6()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    char str[2];"
                            "    int a = 4;\n"
                            "    str[a] = 0;\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { char str [ 2 ] ; int a ; a = 4 ; str [ 4 ] = 0 ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables7()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i = 22;\n"
                            "    abc[i++] = 1;\n"
                            "    abc[++i] = 2;\n"
                            "}\n";

        ASSERT_EQUALS(
            "void foo ( ) { int i ; i = 24 ; abc [ 22 ] = 1 ; abc [ 24 ] = 2 ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables8()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i = 22;\n"
                            "    i++;\n"
                            "    abc[i] = 0;\n"
                            "}\n";

        ASSERT_EQUALS(
            "void foo ( ) { int i ; i = 23 ; ; abc [ 23 ] = 0 ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables9()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int a = 1, b = 2;\n"
                            "    if (a < b)\n"
                            "        ;\n"
                            "}\n";

        ASSERT_EQUALS(
            "void foo ( ) { int a ; a = 1 ; int b ; b = 2 ; if ( 1 < 2 ) ; }",
            simplifyKnownVariables(code));
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

            TODO_ASSERT_EQUALS(
                "void f ( ) { bool b ; b = false ; { b = true ; } if ( true ) { a ( ) ; } }",
                simplifyKnownVariables(code));
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

            TODO_ASSERT_EQUALS(
                "void f ( ) { bool b ; b = false ; { b = false ; } { b = true ; } if ( true ) { a ( ) ; } }",
                simplifyKnownVariables(code));
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

            ASSERT_EQUALS(
                "void f ( ) { int b ; b = 0 ; b = 1 ; for ( int i = 0 ; i < 10 ; i ++ ) { } a ( 1 ) ; }",
                simplifyKnownVariables(code));
        }
    }

    void simplifyKnownVariables11()
    {
        const char code[] = "const int foo = 0;\n"
                            "int main()\n"
                            "{\n"
                            "  int foo=0;\n"
                            "}\n";

        ASSERT_EQUALS(
            "const int foo = 0 ; int main ( ) { int foo ; foo = 0 ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables12()
    {
        const char code[] = "ENTER_NAMESPACE(project_namespace)\n"
                            "const double pi = 3.14;\n"
                            "int main(){}\n";
        ASSERT_EQUALS(
            "ENTER_NAMESPACE ( project_namespace ) const double pi = 3.14 ; int main ( ) { }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables13()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int i = 10;\n"
                            "    while(--i) {}\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int i ; i = 10 ; while ( -- i ) { } }",
            simplifyKnownVariables(code));
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
                                   "1: static int i@1 = 1 ;\n"
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
                                   "4: str@1 . clear ( ) ;\n"
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

    void varid10()
    {
        const std::string code("void foo()\n"
                               "{\n"
                               "    int abc;\n"
                               "    struct abc abc1;\n"
                               "}");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: int abc@1 ;\n"
                                   "4: struct abc abc1@2 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid11()
    {
        const std::string code("class Foo;\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: class Foo ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidStl()
    {
        const std::string code("list<int> ints;\n"
                               "list<int>::iterator it;\n"
                               "std::vector<std::string> dirs;\n"
                               "std::map<int, int> coords;\n"
                               "std::tr1::unordered_map<int, int> xy;\n"
                               "std::list<boost::wave::token_id> tokens;\n"
                               "static std::vector<CvsProcess*> ex1;\n"
                               "extern std::vector<CvsProcess*> ex2;\n"
                               "std::map<int, 1> m;\n"
                              );

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: list < int > ints@1 ;\n"
                                   "2: list < int > :: iterator it@2 ;\n"
                                   "3: std :: vector < std :: string > dirs@3 ;\n"
                                   "4: std :: map < int , int > coords@4 ;\n"
                                   "5: std :: tr1 :: unordered_map < int , int > xy@5 ;\n"
                                   "6: std :: list < boost :: wave :: token_id > tokens@6 ;\n"
                                   "7: static std :: vector < CvsProcess * > ex1@7 ;\n"
                                   "8: extern std :: vector < CvsProcess * > ex2@8 ;\n"
                                   "9: std :: map < int , 1 > m@9 ;\n"
                                  );

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

    void varid_functions()
    {
        {
            const std::string code("void f();\n"
                                   "void f(){}\n");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            tokenizer.setVarId();

            // result..
            const std::string actual(tokenizer.tokens()->stringifyList(true));
            const std::string expected("\n\n##file 0\n"
                                       "1: void f ( ) ;\n"
                                       "2: void f ( ) { }\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string code("A f(3);\n"
                                   "A f2(true);\n"
                                   "A g();\n"
                                   "A e(int c);\n");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            tokenizer.setVarId();

            // result..
            const std::string actual(tokenizer.tokens()->stringifyList(true));
            const std::string expected("\n\n##file 0\n"
                                       "1: A f@1 ( 3 ) ;\n"
                                       "2: A f2@2 ( true ) ;\n"
                                       "3: A g ( ) ;\n"
                                       "4: A e ( int c@3 ) ;\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string code("void f1(int &p)\n"
                                   "{\n"
                                   "    p = 0;\n"
                                   "}\n"
                                   "void f2(std::string &str)\n"
                                   "{\n"
                                   "   str.clear();\n"
                                   "}\n"
                                   "void f3(const std::string &s)\n"
                                   "{\n"
                                   "    s.size();\n"
                                   "}\n");

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            tokenizer.setVarId();

            // result..
            const std::string actual(tokenizer.tokens()->stringifyList(true));
            const std::string expected("\n\n##file 0\n"
                                       "1: void f1 ( int & p@1 )\n"
                                       "2: {\n"
                                       "3: p@1 = 0 ;\n"
                                       "4: }\n"
                                       "5: void f2 ( std :: string & str@2 )\n"
                                       "6: {\n"
                                       "7: str@2 . clear ( ) ;\n"
                                       "8: }\n"
                                       "9: void f3 ( const std :: string & s@3 )\n"
                                       "10: {\n"
                                       "11: s@3 . size ( ) ;\n"
                                       "12: }\n");

            ASSERT_EQUALS(expected, actual);
        }

    }

    void varid_reference_to_containers()
    {
        const std::string code("void f()\n"
                               "{\n"
                               "    std::vector<int> b;\n"
                               "    std::vector<int> &a = b;\n"
                               "    std::vector<int> *c = &b;\n"
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
                                   "3: std :: vector < int > b@1 ;\n"
                                   "4: std :: vector < int > & a@2 = b@1 ;\n"
                                   "5: std :: vector < int > * c@3 = & b@1 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid_in_class()
    {
        const std::string code("class Foo\n"
                               "{\n"
                               "public:\n"
                               "    std::string name1;\n"
                               "    std::string name2;\n"
                               "};\n");

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // result..
        const std::string actual(tokenizer.tokens()->stringifyList(true));
        const std::string expected("\n\n##file 0\n"
                                   "1: class Foo\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: std :: string name1@1 ;\n"
                                   "5: std :: string name2@2 ;\n"
                                   "6: } ;\n");

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
                                   "6: void foo1 ( ) ;\n"
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
                                   "2: { void f ( ) ; } ;\n"
                                   "3:\n"
                                   "4: void A :: foo1 ( )\n"
                                   "5: {\n"
                                   "6: int i@1 ; i@1 = 0 ;\n"
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
                                   "2: { int i@1 ; void f ( ) ; } ;\n"
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
                                   "2: { int i@1 ; void f ( ) ; } ;\n"
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
            ASSERT_EQUALS(" void f ( int x ) { }", ostr.str());
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
            ASSERT_EQUALS(" void f ( int x , char y ) { }", ostr.str());
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
            ASSERT_EQUALS(" void foo ( ) { if ( x ) { int x ; } { } }", ostr.str());
        }
    }


    // Simplify "((..))" into "(..)"
    void removeParantheses1()
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
        ASSERT_EQUALS(" void foo ( ) { free ( p ) ; }", ostr.str());
    }

    void removeParantheses2()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    if (__builtin_expect((s == NULL), 0))\n"
                            "        return;\n"
                            "}";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" void foo ( ) { if ( ! s ) { return ; } }", ostr.str());
    }

    void removeParantheses3()
    {
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if (( true )==(true)){}\n"
                                "}";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void foo ( ) { { } }", ostr.str());
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if (( 2 )==(2)){}\n"
                                "}";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void foo ( ) { { } }", ostr.str());
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if( g(10)){}\n"
                                "}";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void foo ( ) { if ( g ( 10 ) ) { } }", ostr.str());
        }
    }

    // Simplify "( function (..))" into "function (..)"
    void removeParantheses4()
    {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    (free(p));\n"
                            "}";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" void foo ( ) { free ( p ) ; }", ostr.str());
    }

    void removeParantheses5()
    {
        // Simplify "( delete x )" into "delete x"
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    (delete p);\n"
                                "}";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void foo ( ) { delete p ; }", ostr.str());
        }

        // Simplify "( delete [] x )" into "delete [] x"
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    (delete [] p);\n"
                                "}";

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void foo ( ) { delete [ ] p ; }", ostr.str());
        }
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

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.setVarId();
            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void f ( ) { int x ; x = 0 ; { g ( ) ; } }", ostr.str());
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

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.setVarId();
            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void f ( ) { int x ; x = 1 ; }", ostr.str());
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

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.setVarId();
            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void f ( ) { bool x ; x = true ; }", ostr.str());
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

            // tokenize..
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");

            tokenizer.setVarId();
            tokenizer.simplifyTokenList();

            std::ostringstream ostr;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            ASSERT_EQUALS(" void f ( ) { bool x ; x = false ; { g ( ) ; } }", ostr.str());
        }
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
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" void f ( ) { double a ; a = 4.2 ; float b ; b = 4.2f ; double c ; c = 4.2e+10 ; double d ; d = 4.2e-10 ; int e ; e = 4 + 2 ; }", ostr.str());
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
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" void f ( ) { const char * a ; a = { \"hello more world\" } ; }", ostr.str());
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
        ASSERT_EQUALS(" void f ( ) { const int a = 45 ; { int b ; b = 45 ; } } void g ( ) { int a ; a = 2 ; }", ostr.str());
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
        const Token *tok = Tokenizer::findClassFunction(tokenizer.tokens(), "Fred", "%var%", i);
        ASSERT_EQUALS(true, Token::simpleMatch(tok, "Fred ( ) {"));
        tok = Tokenizer::findClassFunction(tok->next(), "Fred", "%var%", i);
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

    void vardecl3()
    {
        const char code[] = "void f() { char * p = foo<10,char>(); }";
        const std::string actual(tokenizeAndStringify(code));
        ASSERT_EQUALS("void f ( ) { char * p ; p = foo < 10 , char > ( ) ; }", actual);
    }

    void vardecl4()
    {
        // ticket #346

        const char code1[] = "void *p = NULL;";
        const char res1[]  = "void * p ; p = NULL ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));

        const char code2[] = "const void *p = NULL;";
        const char res2[]  = "const void * p ; p = NULL ;";
        ASSERT_EQUALS(res2, tokenizeAndStringify(code2));

        const char code3[] = "void * const p = NULL;";
        const char res3[]  = "void * const p ; p = NULL ;";
        ASSERT_EQUALS(res3, tokenizeAndStringify(code3));

        const char code4[] = "const void * const p = NULL;";
        const char res4[]  = "const void * const p ; p = NULL ;";
        ASSERT_EQUALS(res4, tokenizeAndStringify(code4));
    }

    void vardecl_stl()
    {
        // ticket #520

        const char code1[] = "std::vector<std::string>a, b;";
        const char res1[]  = "std :: vector < std :: string > a ; std :: vector < std :: string > b ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));

        const char code2[] = "std::vector<std::string>::const_iterator it, cit;";
        const char res2[]  = "std :: vector < std :: string > :: const_iterator it ; std :: vector < std :: string > :: const_iterator cit ;";
        ASSERT_EQUALS(res2, tokenizeAndStringify(code2));

        const char code3[] = "std::vector<std::pair<std::string, std::string > > *c, d;";
        const char res3[]  = "std :: vector < std :: pair < std :: string , std :: string > > * c ; std :: vector < std :: pair < std :: string , std :: string > > d ;";
        ASSERT_EQUALS(res3, tokenizeAndStringify(code3));
    }

    void vardecl5()
    {
        // don't simplify declarations of static variables
        // "static int i = 0;" is not the same as "static int i; i = 0;"
        const char code[] = "static int i = 0 ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void vardecl6()
    {
        // ticket #565

        const char code1[] = "int z = x >> 16;";
        const char res1[]  = "int z ; z = x >> 16 ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));
    }

    void vardecl7()
    {
        // ticket #603
        const char code[] = "for (int c = 0; c < 0; ++c) {}\n"
                            "int t;\n"
                            "D(3 > t, \"T\");";
        const char res[] = "for ( int c = 0 ; c < 0 ; ++ c ) { }\n"
                           "int t ;\n"
                           "D ( 3 > t , \"T\" ) ;";

        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl8()
    {
        // ticket #696
        const char code[] = "char a[10]={'\\0'}, b[10]={'\\0'};";
        const char res[]  = "char a [ 10 ] = { '\\0' } ; char b [ 10 ] = { '\\0' } ;";

        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl9()
    {
        const char code[] = "char a[2] = {'A', '\\0'}, b[2] = {'B', '\\0'};";
        const char res[]  = "char a [ 2 ] = { 'A' , '\\0' } ; char b [ 2 ] = { 'B' , '\\0' } ;";

        ASSERT_EQUALS(res, tokenizeAndStringify(code));
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
            errout.str("");
            const char code[] = "void f() {}";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(true, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("", errout.str());
        }

        {
            errout.str("");
            const char code[] = "void f() {{}";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:1]: (error) Invalid number of character ({). Can't process file.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "void f()) {}";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:1]: (error) Invalid number of character ((). Can't process file.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "namespace extract{\nB(weighted_moment)\n}\nusing extract::weighted_moment;\n";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(true, tokenizer.tokenize(istr, "test.cpp"));
            tokenizer.simplifyTokenList();
            ASSERT_EQUALS("", errout.str());
        }
    }

    void removeKeywords()
    {
        const char code[] = "if (__builtin_expect(!!(x), 1));";

        const std::string actual(tokenizeAndStringify(code, true));

        ASSERT_EQUALS("if ( ! ! x ) { ; }", actual);
    }


    /**
     * tokenize "unsigned i" => "unsigned int i"
     * tokenize "unsigned int" => "unsigned int"
     */
    void unsigned1()
    {
        // No changes..
        {
            const char code[] = "void foo ( unsigned int , unsigned float ) ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
        }

        // insert "int" after "unsigned"..
        {
            const char code1[] = "unsigned i ;";
            const char code2[] = "unsigned int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        // insert "int" after "unsigned"..
        {
            const char code1[] = "for (unsigned i=0; i<10; i++)";
            const char code2[] = "for ( unsigned int i = 0 ; i < 10 ; i ++ )";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

    }

    void tokenizeAndUpdateClassList(const char code[])
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.updateClassList();
    }

    void testUpdateClassList()
    {
        const char code[] = "class A{\n"
                            " void f() {}\n"
                            " public:\n"
                            " void g() {}\n"
                            "};";
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.updateClassList();
        ASSERT_EQUALS(2, tokenizer._classInfoList["A"]._memberFunctions.size());
        if (tokenizer._classInfoList["A"]._memberFunctions.size() > 1)
        {
            ASSERT_EQUALS(std::string("f"), tokenizer._classInfoList["A"]._memberFunctions[0]._name);
            ASSERT_EQUALS(std::string("f"), tokenizer._classInfoList["A"]._memberFunctions[0]._declaration->str());
            ASSERT_EQUALS(ClassInfo::PRIVATE, tokenizer._classInfoList["A"]._memberFunctions[0]._type);
            ASSERT_EQUALS(std::string("g"), tokenizer._classInfoList["A"]._memberFunctions[1]._name);
            ASSERT_EQUALS(std::string("g"), tokenizer._classInfoList["A"]._memberFunctions[1]._declaration->str());
            ASSERT_EQUALS(ClassInfo::PUBLIC, tokenizer._classInfoList["A"]._memberFunctions[1]._type);
        }
    }

    void createLinks()
    {
        {
            const char code[] = "class A{\n"
                                " void f() {}\n"
                                "};";
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            // A body {}
            ASSERT_EQUALS(true, tok->tokAt(2)->link() == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->tokAt(9)->link() == tok->tokAt(2));

            // f body {}
            ASSERT_EQUALS(true, tok->tokAt(7)->link() == tok->tokAt(8));
            ASSERT_EQUALS(true, tok->tokAt(8)->link() == tok->tokAt(7));

            // f ()
            ASSERT_EQUALS(true, tok->tokAt(5)->link() == tok->tokAt(6));
            ASSERT_EQUALS(true, tok->tokAt(6)->link() == tok->tokAt(5));
        }

        {
            const char code[] = "void f(){\n"
                                " char a[10];\n"
                                " char *b ; b = new char[a[0]];\n"
                                "};";
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            // a[10]
            ASSERT_EQUALS(true, tok->tokAt(7)->link() == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->tokAt(9)->link() == tok->tokAt(7));

            // new char[]
            ASSERT_EQUALS(true, tok->tokAt(19)->link() == tok->tokAt(24));
            ASSERT_EQUALS(true, tok->tokAt(24)->link() == tok->tokAt(19));

            // a[0]
            ASSERT_EQUALS(true, tok->tokAt(21)->link() == tok->tokAt(23));
            ASSERT_EQUALS(true, tok->tokAt(23)->link() == tok->tokAt(21));
        }

        {
            const char code[] = "void f(){\n"
                                " foo(g());\n"
                                "};";
            Tokenizer tokenizer;
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            // foo(
            ASSERT_EQUALS(true, tok->tokAt(6)->link() == tok->tokAt(10));
            ASSERT_EQUALS(true, tok->tokAt(10)->link() == tok->tokAt(6));

            // g(
            ASSERT_EQUALS(true, tok->tokAt(8)->link() == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->tokAt(9)->link() == tok->tokAt(8));
        }
    }
};

REGISTER_TEST(TestTokenizer)
