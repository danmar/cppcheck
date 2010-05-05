/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include <cstring>
#include "testsuite.h"
#include "tokenize.h"
#include "token.h"

extern std::ostringstream errout;
class TestTokenizer : public TestFixture
{
public:
    TestTokenizer() : TestFixture("TestTokenizer")
    { }

private:

    void run()
    {
        TEST_CASE(tokenize1);
        TEST_CASE(tokenize2);
        TEST_CASE(tokenize3);
        TEST_CASE(tokenize4);
        TEST_CASE(tokenize5);

        // don't freak out when the syntax is wrong
        TEST_CASE(wrong_syntax);

        TEST_CASE(minus);

        TEST_CASE(longtok);

        TEST_CASE(removeCast1);
        TEST_CASE(removeCast2);
        TEST_CASE(removeCast3);
        TEST_CASE(removeCast4);

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
        TEST_CASE(ifAddBraces9);
        TEST_CASE(ifAddBraces10);
        TEST_CASE(ifAddBraces11);
        TEST_CASE(ifAddBraces12);

        TEST_CASE(whileAddBraces);
        TEST_CASE(doWhileAddBraces);

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
        TEST_CASE(simplifyKnownVariables14);
        TEST_CASE(simplifyKnownVariables15);
        TEST_CASE(simplifyKnownVariables16);
        TEST_CASE(simplifyKnownVariables17);
        TEST_CASE(simplifyKnownVariables18);
        TEST_CASE(simplifyKnownVariables19);
        TEST_CASE(simplifyKnownVariables20);
        TEST_CASE(simplifyKnownVariables21);
        TEST_CASE(simplifyKnownVariables22);
        TEST_CASE(simplifyKnownVariables23);
        TEST_CASE(simplifyKnownVariables24);
        TEST_CASE(simplifyKnownVariables25);

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
        TEST_CASE(varid12);
        TEST_CASE(varid13);
        TEST_CASE(varid14);
        TEST_CASE(varid15);
        TEST_CASE(varid16);
        TEST_CASE(varidStl);
        TEST_CASE(varid_delete);
        TEST_CASE(varid_functions);
        TEST_CASE(varid_reference_to_containers);
        TEST_CASE(varid_in_class);
        TEST_CASE(varid_operator);

        TEST_CASE(varidclass1);
        TEST_CASE(varidclass2);
        TEST_CASE(varidclass3);
        TEST_CASE(varidclass4);
        TEST_CASE(varidclass5);
        TEST_CASE(varidclass6);
        TEST_CASE(varidclass7);

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
        TEST_CASE(removeParantheses6);

        TEST_CASE(tokenize_double);
        TEST_CASE(tokenize_strings);
        TEST_CASE(simplify_constants);
        TEST_CASE(simplify_constants2);

        TEST_CASE(findClassFunction1);
        TEST_CASE(findClassFunction2);

        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(vardecl4);
        TEST_CASE(vardec_static);
        TEST_CASE(vardecl6);
        TEST_CASE(vardecl7);
        TEST_CASE(vardecl8);
        TEST_CASE(vardecl9);
        TEST_CASE(vardecl10);
        TEST_CASE(vardecl_stl);
        TEST_CASE(vardecl_template);
        TEST_CASE(volatile_variables);
        TEST_CASE(syntax_error);

        TEST_CASE(removeKeywords);

        // unsigned i; => unsigned int i;
        TEST_CASE(unsigned1);
        TEST_CASE(unsigned2);
        TEST_CASE(unsigned3);	// template arguments

        TEST_CASE(testUpdateClassList);
        TEST_CASE(createLinks);
        TEST_CASE(signed1);

        TEST_CASE(removeExceptionSpecification1);
        TEST_CASE(removeExceptionSpecification2);

        TEST_CASE(gt);      // use "<" comparisons instead of ">"

        TEST_CASE(simplifyString);
        TEST_CASE(simplifyConst);
        TEST_CASE(switchCase);

        TEST_CASE(functionpointer);

        TEST_CASE(removeRedundantAssignment);

        TEST_CASE(removedeclspec);
        TEST_CASE(cpp0xtemplate);

        TEST_CASE(arraySize);

        TEST_CASE(labels);
        TEST_CASE(simplifyInitVar);
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
            if (!simplify)
            {
                if (tok->isUnsigned())
                    ostr << "unsigned ";
                else if (tok->isSigned())
                    ostr << "signed ";
            }
            if (tok->isLong())
                ostr << "long ";
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


    void tokenize1()
    {
        const std::string code("void f ( )\n"
                               "{ if ( p . y ( ) > yof ) { } }");
        ASSERT_EQUALS(code, tokenizeAndStringify(code.c_str()));
    }

    void tokenize2()
    {
        const std::string code("{ sizeof a, sizeof b }");
        ASSERT_EQUALS("{ sizeof a , sizeof b }", tokenizeAndStringify(code.c_str()));
    }

    void tokenize3()
    {
        errout.str("");
        const std::string code("void foo()\n"
                               "{\n"
                               "    int i;\n"
                               "    ABC(for(i=0;i<10;i++) x());\n"
                               "}");
        ASSERT_EQUALS("void foo ( )\n"
                      "{\n"
                      "int i ;\n"
                      "ABC ( for ( i = 0 ; i < 10 ; i ++ ) x ( ) ) ;\n"
                      "}", tokenizeAndStringify(code.c_str()));
        ASSERT_EQUALS("", errout.str());
    }

    void tokenize4()
    {
        errout.str("");
        const std::string code("class foo\n"
                               "{\n"
                               "public:\n"
                               "    const int i;\n"
                               "}");
        ASSERT_EQUALS("class foo\n"
                      "{\n"
                      "public:\n"
                      "const int i ;\n"
                      "}", tokenizeAndStringify(code.c_str()));
        ASSERT_EQUALS("", errout.str());
    }

    void tokenize5()
    {
        // Tokenize values
        ASSERT_EQUALS("; + 1E3 ;", tokenizeAndStringify("; +1E3 ;"));
        ASSERT_EQUALS("; 1E-2 ;", tokenizeAndStringify("; 1E-2 ;"));
    }

    void wrong_syntax()
    {
        errout.str("");
        const std::string code("TR(kvmpio, PROTO(int rw), ARGS(rw), TP_(aa->rw;))");
        ASSERT_EQUALS("TR ( kvmpio , PROTO ( int rw ) , ARGS ( rw ) , TP_ ( aa . rw ; ) )", tokenizeAndStringify(code.c_str(), true));
        ASSERT_EQUALS("", errout.str());
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

    void removeCast3()
    {
        // ticket #961
        const char code[] = "assert (iplen >= (unsigned) ipv4->ip_hl * 4 + 20);";
        const char expected[] = "assert ( iplen >= ipv4 . ip_hl * 4 + 20 ) ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void removeCast4()
    {
        // ticket #970
        const char code[] = "if (a >= (unsigned)(b)) {}";
        const char expected[] = "if ( a >= ( int ) b ) { }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void inlineasm()
    {
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";asm { mov ax,bx };"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";_asm { mov ax,bx };"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm { mov ax,bx };"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm__ __volatile__ ( \"mov ax,bx\" );"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm _emit 12h ;"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm mov a, b ;"));
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

    void pointers_condition()
    {
        ASSERT_EQUALS("( p )", tokenizeAndStringify("( p != NULL )", true));
        ASSERT_EQUALS("( p )", tokenizeAndStringify("( NULL != p )", true));
        ASSERT_EQUALS("( this . p )", tokenizeAndStringify("( this->p != NULL )", true));
        ASSERT_EQUALS("( this . p )", tokenizeAndStringify("( NULL != this->p )", true));
        ASSERT_EQUALS("( Foo :: p )", tokenizeAndStringify("( Foo::p != NULL )", true));
        ASSERT_EQUALS("( Foo :: p )", tokenizeAndStringify("( NULL != Foo::p )", true));

        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == NULL )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( NULL == p )", true));
        ASSERT_EQUALS("( ! this . p )", tokenizeAndStringify("( this->p == NULL )", true));
        ASSERT_EQUALS("( ! this . p )", tokenizeAndStringify("( NULL == this->p )", true));
        ASSERT_EQUALS("( ! Foo :: p )", tokenizeAndStringify("( Foo::p == NULL )", true));
        ASSERT_EQUALS("( ! Foo :: p )", tokenizeAndStringify("( NULL == Foo::p )", true));

        ASSERT_EQUALS("( p1 || ! p2 )", tokenizeAndStringify("( p1 != NULL || p2 == NULL )", true));
        ASSERT_EQUALS("( p1 && ! p2 )", tokenizeAndStringify("( p1 != NULL && p2 == NULL )", true));

        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == false )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == 0 )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == '\\0' )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == 0L )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == 0UL )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == 0ul )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( p == 0l )", true));

        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( false == p )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( 0 == p )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( '\\0' == p )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( 0L == p )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( 0UL == p )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( 0ul == p )", true));
        ASSERT_EQUALS("( ! p )", tokenizeAndStringify("( 0l == p )", true));
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

    void ifAddBraces9()
    {
        // ticket #990
        const char code[] =
            "{"
            "    for (int k=0; k<VectorSize; k++)"
            "        LOG_OUT(ID_Vector[k])"
            "}";
        const char expected[] =
            "{ "
            "for ( int k = 0 ; k < VectorSize ; k ++ ) { "
            "LOG_OUT ( ID_Vector [ k ] ) "
            "} "
            "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void ifAddBraces10()
    {
        // ticket #1361
        const char code[] = "{ DEBUG(if (x) y; else z); }";
        const char expected[] = "{ DEBUG ( if ( x ) y ; else z ) ; }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void ifAddBraces11()
    {
        const char code[] = "{ if (x) if (y) ; else ; }";
        const char expected[] = "{ if ( x ) { if ( y ) { ; } else { ; } } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void ifAddBraces12()
    {
        // ticket #1424
        const char code[] = "{ if (x) do { } while(x); }";
        const char expected[] = "{ if ( x ) { do { } while ( x ) ; } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
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
        {
            const char code[] = "do ; while (0);";
            const char result[] = "do { ; } while ( 0 ) ;";

            ASSERT_EQUALS(result, tokenizeAndStringify(code, false));
        }

        {
            const char code[] = "UNKNOWN_MACRO ( do ) ; while ( a -- ) ;";
            const char result[] = "UNKNOWN_MACRO ( do ) ; while ( a -- ) { ; }";

            ASSERT_EQUALS(result, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "UNKNOWN_MACRO ( do , foo ) ; while ( a -- ) ;";
            const char result[] = "UNKNOWN_MACRO ( do , foo ) ; while ( a -- ) { ; }";

            ASSERT_EQUALS(result, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void foo ( int c , int d ) {\n"
                                " do\n"
                                "  if ( c ) {\n"
                                "   while ( c ) { c -- ; }\n"
                                "  }\n"
                                " while ( -- d > 0 ) ;\n"
                                " return 0 ;\n"
                                "}\n";
            const char result[] =   "void foo ( int c , int d ) {\n"
                                    "do {\n"
                                    "if ( c ) {\n"
                                    "while ( c ) { c -- ; }\n"
                                    "} }\n"
                                    "while ( -- d > 0 ) ;\n"
                                    "return 0 ;\n"
                                    "}";
            ASSERT_EQUALS(result, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void foo ( int c , int d ) {\n"
                                " do\n"
                                "   do c -- ; while ( c ) ;\n"
                                " while ( -- d > 0 ) ;\n"
                                " return 0 ;\n"
                                "}\n";
            const char result[] =   "void foo ( int c , int d ) {\n"
                                    "do {\n"
                                    "do { c -- ; } while ( c ) ; }\n"
                                    "while ( -- d > 0 ) ;\n"
                                    "return 0 ;\n"
                                    "}";
            ASSERT_EQUALS(result, tokenizeAndStringify(code, true));
        }
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
                "void f ( ) { int a ; a = 10 ; if ( 10 ) { ; } }",
                simplifyKnownVariables(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    int a = 10;\n"
                                "    if (!a);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void f ( ) { int a ; a = 10 ; if ( ! 10 ) { ; } }",
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
            "void f ( ) { int a ; a = 10 ; a = g ( ) ; if ( a ) { ; } }",
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
            "void f ( ) { int a ; a = 4 ; while ( true ) { break ; a = 10 ; } if ( a ) { ; } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables4()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( g(a));\n"
                            "}\n";

        // TODO: if a is passed by value is is ok to simplify..
        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; if ( g ( a ) ) { ; } }",
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
            "void f ( ) { int a ; a = 4 ; if ( a = 5 ) { ; } }",
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
            "void foo ( ) { int a ; a = 1 ; int b ; b = 2 ; if ( 1 < 2 ) { ; } }",
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
                                "  return b;\n"
                                "}\n";

            ASSERT_EQUALS(
                "void f ( ) { int b ; b = 0 ; b = 1 ; for ( int i = 0 ; i < 10 ; i ++ ) { } return 1 ; }",
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

    void simplifyKnownVariables14()
    {
        // ticket #753
        const char code[] = "void f ( ) { int n ; n = 1 ; do { ++ n ; } while ( n < 10 ) ; }";
        ASSERT_EQUALS(code, simplifyKnownVariables(code));
    }

    void simplifyKnownVariables15()
    {
        {
            const char code[] = "int main()\n"
                                "{\n"
                                "  int x=5;\n"
                                "  std::cout << 10 / x << std::endl;\n"
                                "}\n";

            ASSERT_EQUALS(
                "int main ( ) { int x ; x = 5 ; std :: cout << 10 / 5 << std :: endl ; }",
                simplifyKnownVariables(code));
        }

        {
            const char code[] = "int main()\n"
                                "{\n"
                                "  int x=5;\n"
                                "  std::cout << x / ( x == 1 ) << std::endl;\n"
                                "}\n";

            ASSERT_EQUALS(
                "int main ( ) { int x ; x = 5 ; std :: cout << 5 / ( 5 == 1 ) << std :: endl ; }",
                simplifyKnownVariables(code));
        }
    }

    void simplifyKnownVariables16()
    {
        // ticket #807 - segmentation fault when macro isn't found
        const char code[] = "void f ( ) { int n = 1; DISPATCH(while); }";
        simplifyKnownVariables(code);
    }

    void simplifyKnownVariables17()
    {
        // ticket #807 - segmentation fault when macro isn't found
        const char code[] = "void f ( ) { char *s = malloc(100);mp_ptr p = s; p++; }";
        ASSERT_EQUALS(
            "void f ( ) { char * s ; s = malloc ( 100 ) ; mp_ptr p ; p = s ; p ++ ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables18()
    {
        const char code[] = "void f ( ) { char *s = malloc(100);mp_ptr p = s; ++p; }";
        ASSERT_EQUALS(
            "void f ( ) { char * s ; s = malloc ( 100 ) ; mp_ptr p ; p = s ; ++ p ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables19()
    {
        const char code[] = "void f ( ) { int i=0; do { if (i>0) { a(); } i=b(); } while (i != 12); }";
        ASSERT_EQUALS(
            "void f ( ) { int i ; i = 0 ; do { if ( 0 < i ) { a ( ) ; } i = b ( ) ; } while ( i != 12 ) ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables20()
    {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int i = 0;\n"
                            "    if (x) {\n"
                            "        if (i) i=0;\n"
                            "    }\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int i ; i = 0 ; if ( x ) { if ( 0 ) { i = 0 ; } } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables21()
    {
        const char code[] = "void foo() { int n = 10; for (int i = 0; i < n; ++i) { } }";

        ASSERT_EQUALS(
            "void foo ( ) { int n ; n = 10 ; for ( int i = 0 ; i < 10 ; ++ i ) { } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables22()
    {
        // This testcase is related to ticket #1169
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int n = 10;\n"
                                "    i = (n >> 1);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) { int n ; n = 10 ; i = ( 10 >> 1 ) ; }",
                simplifyKnownVariables(code));
        }
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int n = 10;\n"
                                "    i = (n << 1);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) { int n ; n = 10 ; i = ( 10 << 1 ) ; }",
                simplifyKnownVariables(code));
        }
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int n = 10;\n"
                                "    i = (1 << n);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) { int n ; n = 10 ; i = ( 1 << 10 ) ; }",
                simplifyKnownVariables(code));
        }
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int n = 10;\n"
                                "    i = (1 >> n);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) { int n ; n = 10 ; i = ( 1 >> 10 ) ; }",
                simplifyKnownVariables(code));
        }
    }

    void simplifyKnownVariables23()
    {
        // This testcase is related to ticket #1596
        const char code[] = "void foo(int x)\n"
                            "{\n"
                            "    int a[10], c = 0;\n"
                            "    if (x) {\n"
                            "        a[c] = 0;\n"
                            "        c++;\n"
                            "    } else {\n"
                            "        a[c] = 0;\n"
                            "    }\n"
                            "}\n";

        // wanted result
        TODO_ASSERT_EQUALS(
            "void foo ( int x ) "
            "{"
            " int a [ 10 ] ; int c ; c = 0 ;"
            " if ( x ) { a [ 0 ] = 0 ; c = 1 ; }"
            " else { a [ 0 ] = 0 ; } "
            "}",
            simplifyKnownVariables(code));

        // Current result
        ASSERT_EQUALS(
            "void foo ( int x ) "
            "{"
            " int a [ 10 ] ; int c ; c = 0 ;"
            " if ( x ) { a [ 0 ] = 0 ; c ++ ; }"
            " else { a [ c ] = 0 ; } "
            "}",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables24()
    {
        // This testcase is related to ticket #1596
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int c;\n"
                            "    for (c=0;c<10;++c) { }\n"
                            "    a[c] = 0;\n"
                            "}\n";

        // Current result
        ASSERT_EQUALS(
            "void foo ( ) "
            "{"
            " int c ;"
            " for ( c = 0 ; c < 10 ; ++ c ) { }"
            " a [ 10 ] = 0 ; "
            "}",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables25()
    {
        {
            // This testcase is related to ticket #1646
            const char code[] = "void foo(char *str)\n"
                                "{\n"
                                "    int i;\n"
                                "    for (i=0;i<10;++i) {\n"
                                "        if (*str == 0) goto label;\n"
                                "    }\n"
                                "    return;\n"
                                "label:\n"
                                "    str[i] = 0;\n"
                                "}\n";

            // Current result
            ASSERT_EQUALS(
                "void foo ( char * str ) "
                "{"
                " int i ;"
                " for ( i = 0 ; i < 10 ; ++ i ) {"
                " if ( * str == 0 ) { goto label ; }"
                " }"
                " return ;"
                " label : ;"
                " str [ i ] = 0 ; "
                "}",
                simplifyKnownVariables(code));
        }

        {
            // This testcase is related to ticket #1646
            const char code[] = "void foo(char *str)\n"
                                "{\n"
                                "    int i;\n"
                                "    for (i=0;i<10;++i) { }\n"
                                "    return;\n"
                                "    str[i] = 0;\n"
                                "}\n";

            // Current result
            ASSERT_EQUALS(
                "void foo ( char * str ) "
                "{"
                " int i ;"
                " for ( i = 0 ; i < 10 ; ++ i ) { }"
                " return ;"
                " str [ i ] = 0 ; "
                "}",
                simplifyKnownVariables(code));
        }
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

    std::string tokenizeDebugListing(const std::string &code, bool simplify = false)
    {
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList();

        // result..
        return tokenizer.tokens()->stringifyList(true);
    }

    void varid1()
    {
        {
            const std::string actual = tokenizeDebugListing(
                                           "static int i = 1;\n"
                                           "void f()\n"
                                           "{\n"
                                           "    int i = 2;\n"
                                           "    for (int i = 0; i < 10; ++i)\n"
                                           "        i = 3;\n"
                                           "    i = 4;\n"
                                           "}\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: static int i@1 = 1 ;\n"
                                       "2: void f ( )\n"
                                       "3: {\n"
                                       "4: int i@2 ; i@2 = 2 ;\n"
                                       "5: for ( int i@3 = 0 ; i@3 < 10 ; ++ i@3 ) {\n"
                                       "6: i@3 = 3 ; }\n"
                                       "7: i@2 = 4 ;\n"
                                       "8: }\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenizeDebugListing(
                                           "static int i = 1;\n"
                                           "void f()\n"
                                           "{\n"
                                           "    int i = 2;\n"
                                           "    for (int i = 0; i < 10; ++i)\n"
                                           "    {\n"
                                           "      i = 3;\n"
                                           "    }\n"
                                           "    i = 4;\n"
                                           "}\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: static int i@1 = 1 ;\n"
                                       "2: void f ( )\n"
                                       "3: {\n"
                                       "4: int i@2 ; i@2 = 2 ;\n"
                                       "5: for ( int i@3 = 0 ; i@3 < 10 ; ++ i@3 )\n"
                                       "6: {\n"
                                       "7: i@3 = 3 ;\n"
                                       "8: }\n"
                                       "9: i@2 = 4 ;\n"
                                       "10: }\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid2()
    {
        const std::string actual = tokenizeDebugListing(
                                       "void f()\n"
                                       "{\n"
                                       "    struct ABC abc;\n"
                                       "    abc.a = 3;\n"
                                       "    i = abc.a;\n"
                                       "}\n");

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
        const std::string actual = tokenizeDebugListing(
                                       "static char str[4];\n"
                                       "void f()\n"
                                       "{\n"
                                       "    char str[10];\n"
                                       "    str[0] = 0;\n"
                                       "}\n");

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
        const std::string actual = tokenizeDebugListing(
                                       "void f(const unsigned int a[])\n"
                                       "{\n"
                                       "    int i = *(a+10);\n"
                                       "}\n", true);

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( const int a@1 [ ] )\n"
                                   "2: {\n"
                                   "3: int i@2 ; i@2 = a@1 [ 10 ] ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid5()
    {
        const std::string actual = tokenizeDebugListing(
                                       "void f()\n"
                                       "{\n"
                                       "    int a,b;\n"
                                       "}\n", true);

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: ; ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varid6()
    {
        const std::string actual = tokenizeDebugListing(
                                       "int f(int a, int b)\n"
                                       "{\n"
                                       "    return a+b;\n"
                                       "}\n", true);

        const std::string expected("\n\n##file 0\n"
                                   "1: int f ( int a@1 , int b@2 )\n"
                                   "2: {\n"
                                   "3: return a@1 + b@2 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }


    void varid7()
    {
        const std::string actual = tokenizeDebugListing(
                                       "void func()\n"
                                       "{\n"
                                       "char a[256] = \"test\";\n"
                                       "{\n"
                                       "char b[256] = \"test\";\n"
                                       "}\n"
                                       "}\n");

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
        const std::string actual = tokenizeDebugListing(
                                       "int f()\n"
                                       "{\n"
                                       "    int a;\n"
                                       "    return a;\n"
                                       "}\n");

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
        const std::string actual = tokenizeDebugListing(
                                       "void func()\n"
                                       "{\n"
                                       "    std::string str(\"test\");\n"
                                       "    str.clear();\n"
                                       "}\n");

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
        const std::string actual = tokenizeDebugListing(
                                       "typedef int INT32;\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid10()
    {
        const std::string actual = tokenizeDebugListing(
                                       "void foo()\n"
                                       "{\n"
                                       "    int abc;\n"
                                       "    struct abc abc1;\n"
                                       "}");

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
        const std::string actual = tokenizeDebugListing(
                                       "class Foo;\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Foo ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid12()
    {
        const std::string actual = tokenizeDebugListing(
                                       "static void a()\n"
                                       "{\n"
                                       "    class Foo *foo;\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: static void a ( )\n"
                                   "2: {\n"
                                   "3: class Foo * foo@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid13()
    {
        const std::string actual = tokenizeDebugListing(
                                       "void f()\n"
                                       "{\n"
                                       "    int a; int b;\n"
                                       "    a = a;\n"
                                       "}\n", true);

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( )\n"
                                   "2: {\n"
                                   "3: int a@1 ; ;\n"
                                   "4: a@1 = a@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid14()
    {
        // Overloaded operator*
        const std::string actual = tokenizeDebugListing(
                                       "void foo()\n"
                                       "{\n"
                                       "A a;\n"
                                       "B b;\n"
                                       "b * a;\n"
                                       "}");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: A a@1 ;\n"
                                   "4: B b@2 ;\n"
                                   "5: b@2 * a@1 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid15()
    {
        {
            const std::string actual = tokenizeDebugListing(
                                           "struct S {\n"
                                           "    struct T {\n"
                                           "    } t;\n"
                                           "} s;");

            const std::string expected("\n\n##file 0\n"
                                       "1: struct S {\n"
                                       "2: struct T {\n"
                                       "3: } ; T t@1 ;\n"
                                       "4: } ; S s@2 ;\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenizeDebugListing(
                                           "struct S {\n"
                                           "    struct T {\n"
                                           "    } t;\n"
                                           "};");

            const std::string expected("\n\n##file 0\n"
                                       "1: struct S {\n"
                                       "2: struct T {\n"
                                       "3: } ; T t@1 ;\n"
                                       "4: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid16()
    {
        const std::string code("void foo()\n"
                               "{\n"
                               "    int x = 1;\n"
                               "    y = (z * x);\n"
                               "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: int x@1 ; x@1 = 1 ;\n"
                                   "4: y = ( z * x@1 ) ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }


    void varidStl()
    {
        const std::string actual = tokenizeDebugListing(
                                       "list<int> ints;\n"
                                       "list<int>::iterator it;\n"
                                       "std::vector<std::string> dirs;\n"
                                       "std::map<int, int> coords;\n"
                                       "std::tr1::unordered_map<int, int> xy;\n"
                                       "std::list<boost::wave::token_id> tokens;\n"
                                       "static std::vector<CvsProcess*> ex1;\n"
                                       "extern std::vector<CvsProcess*> ex2;\n"
                                       "std::map<int, 1> m;\n"
                                   );

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
        const std::string actual = tokenizeDebugListing(
                                       "void f()\n"
                                       "{\n"
                                       "  int *a;\n"
                                       "  delete a;\n"
                                       "}\n");

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
            const std::string actual = tokenizeDebugListing(
                                           "void f();\n"
                                           "void f(){}\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: void f ( ) ;\n"
                                       "2: void f ( ) { }\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenizeDebugListing(
                                           "A f(3);\n"
                                           "A f2(true);\n"
                                           "A g();\n"
                                           "A e(int c);\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: A f@1 ( 3 ) ;\n"
                                       "2: A f2@2 ( true ) ;\n"
                                       "3: A g ( ) ;\n"
                                       "4: A e ( int c@3 ) ;\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenizeDebugListing(
                                           "void f1(int &p)\n"
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
        const std::string actual = tokenizeDebugListing(
                                       "void f()\n"
                                       "{\n"
                                       "    std::vector<int> b;\n"
                                       "    std::vector<int> &a = b;\n"
                                       "    std::vector<int> *c = &b;\n"
                                       "}\n");

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
        {
            const std::string actual = tokenizeDebugListing(
                                           "class Foo\n"
                                           "{\n"
                                           "public:\n"
                                           "    std::string name1;\n"
                                           "    std::string name2;\n"
                                           "};\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: class Foo\n"
                                       "2: {\n"
                                       "3: public:\n"
                                       "4: std :: string name1@1 ;\n"
                                       "5: std :: string name2@2 ;\n"
                                       "6: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenizeDebugListing(
                                           "class foo\n"
                                           "{\n"
                                           "public:\n"
                                           "    void do_something(const int x, const int y);\n"
                                           "    void bar();\n"
                                           "};\n"
                                           "\n"
                                           "void foo::bar()\n"
                                           "{\n"
                                           "    POINT pOutput = { 0 , 0 };\n"
                                           "    int x = pOutput.x;\n"
                                           "    int y = pOutput.y;\n"
                                           "}\n");

            const std::string expected("\n\n##file 0\n"
                                       "1: class foo\n"
                                       "2: {\n"
                                       "3: public:\n"
                                       "4: void do_something ( const int x@1 , const int y@2 ) ;\n"
                                       "5: void bar ( ) ;\n"
                                       "6: } ;\n"
                                       "7:\n"
                                       "8: void foo :: bar ( )\n"
                                       "9: {\n"
                                       "10: POINT pOutput@3 ; pOutput@3 = { 0 , 0 } ;\n"
                                       "11: int x@4 ; x@4 = pOutput@3 . x@6 ;\n"
                                       "12: int y@5 ; y@5 = pOutput@3 . y@7 ;\n"
                                       "13: }\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_operator()
    {
        const std::string actual = tokenizeDebugListing(
                                       "class Foo\n"
                                       "{\n"
                                       "public:\n"
                                       "    void operator=(const Foo &);\n"
                                       "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Foo\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: void operator = ( const Foo & ) ;\n"
                                   "5: } ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass1()
    {
        const std::string actual = tokenizeDebugListing(
                                       "class Fred\n"
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
        const std::string actual = tokenizeDebugListing(
                                       "class Fred\n"
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
        const std::string actual = tokenizeDebugListing(
                                       "class Fred\n"
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
        const std::string actual = tokenizeDebugListing(
                                       "class Fred\n"
                                       "{ int i; void f(); };\n"
                                       "\n"
                                       "void Fred::f()\n"
                                       "{\n"
                                       "    if (i) { }\n"
                                       "    i = 0;\n"
                                       "}\n");

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

    void varidclass5()
    {
        const std::string actual = tokenizeDebugListing(
                                       "class A { };\n"
                                       "class B\n"
                                       "{\n"
                                       "    A *a;\n"
                                       "    B() : a(new A)\n"
                                       "    { }\n"
                                       "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class A { } ;\n"
                                   "2: class B\n"
                                   "3: {\n"
                                   "4: A * a@1 ;\n"
                                   "5: B ( ) : a@1 ( new A )\n"
                                   "6: { }\n"
                                   "7: } ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass6()
    {
        const std::string actual = tokenizeDebugListing(
                                       "class A\n"
                                       "{\n"
                                       "  public:\n"
                                       "  static char buf[20];\n"
                                       "};\n"
                                       "char A::buf[20];\n"
                                       "int main()\n"
                                       "{\n"
                                       "  char buf[2];\n"
                                       "  A::buf[10] = 0;\n"
                                       "}");

        const std::string expected("\n\n##file 0\n"
                                   "1: class A\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: static char buf@1 [ 20 ] ;\n"
                                   "5: } ;\n"
                                   "6: char A :: buf [ 20 ] ;\n"
                                   "7: int main ( )\n"
                                   "8: {\n"
                                   "9: char buf@2 [ 2 ] ;\n"
                                   "10: A :: buf@1 [ 10 ] = 0 ;\n"
                                   "11: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass7()
    {
        const std::string actual = tokenizeDebugListing(
                                       "int main()\n"
                                       "{\n"
                                       "  char buf[2];\n"
                                       "  A::buf[10] = 0;\n"
                                       "}");

        const std::string expected("\n\n##file 0\n"
                                   "1: int main ( )\n"
                                   "2: {\n"
                                   "3: char buf@1 [ 2 ] ;\n"
                                   "4: A :: buf [ 10 ] = 0 ;\n"
                                   "5: }\n");

        TODO_ASSERT_EQUALS(expected, actual);
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
            const char code[] = "char a [ ABC ( DEF ) ] ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "module ( a , a , sizeof ( a ) , 0444 ) ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code, true));
        }

        ASSERT_EQUALS("void f ( int x ) { }", tokenizeAndStringify("void f(x) int x; { }", true));
        ASSERT_EQUALS("void f ( int x , char y ) { }", tokenizeAndStringify("void f(x,y) int x; char y; { }", true));

        // #1067 - Not simplified. Feel free to fix so it is simplified correctly but this syntax is obsolete.
        ASSERT_EQUALS("int ( * d ( a , b , c ) ) ( ) int a ; int b ; int c ; { }", tokenizeAndStringify("int (*d(a,b,c))()int a,b,c; { }", true));

        {
            // This is not a function but the pattern is similar..
            const char code[] = "void foo()"
                                "{"
                                "    if (x)"
                                "        int x;"
                                "    { }"
                                "}";
            ASSERT_EQUALS("void foo ( ) { if ( x ) { ; } { } }", tokenizeAndStringify(code, true));
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

    // "!(abc.a)" => "!abc.a"
    void removeParantheses6()
    {
        const char code[] = "(!(abc.a))";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" ( ! abc . a )", ostr.str());
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
        ASSERT_EQUALS(" void f ( ) { const int a = 45 ; { ; ; } } void g ( ) { ; ; }", ostr.str());
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
        const Token *tok = tokenizer.findClassFunction(tokenizer.tokens(), "Fred", "%var%", i);
        ASSERT_EQUALS(true, Token::simpleMatch(tok, "Fred ( ) {"));
        tok = tokenizer.findClassFunction(tok->next(), "Fred", "%var%", i);
        ASSERT_EQUALS(0, tok ? 1 : 0);
    }

    void findClassFunction2()
    {
        const char code[] =
            "struct Fred"
            "{\n"
            "    Fred()\n"
            "    { }\n"
            "};\n";

        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        int i;

        i = 0;
        const Token *tok = tokenizer.findClassFunction(tokenizer.tokens(), "Fred", "%var%", i, true);
        ASSERT_EQUALS(true, Token::simpleMatch(tok, "Fred ( ) {"));
        tok = tokenizer.findClassFunction(tok->next(), "Fred", "%var%", i, false);
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

    void vardecl_template()
    {
        // ticket #1046
        const char code1[] = "b<(1<<24),10,24> u, v;";
        const char res1[]  = "b < ( 1 << 24 ) , 10 , 24 > u ; b < ( 1 << 24 ) , 10 , 24 > v ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));
    }

    void vardec_static()
    {
        {
            // don't simplify declarations of static variables
            // "static int i = 0;" is not the same as "static int i; i = 0;"
            const char code[] = "static int i = 0 ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
        }

        {
            const char code[] = "static int a, b;";
            ASSERT_EQUALS("static int a ; static int b ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static unsigned int a, b;";
            ASSERT_EQUALS("static unsigned int a ; static unsigned int b ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static int a=1, b=1;";
            ASSERT_EQUALS("static int a = 1 ; static int b = 1 ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static int *a, *b;";
            ASSERT_EQUALS("static int * a ; static int * b ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static unsigned int *a=0, *b=0;";
            ASSERT_EQUALS("static unsigned int * a = 0 ; static unsigned int * b = 0 ;", tokenizeAndStringify(code));
        }
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

    void vardecl10()
    {
        // ticket #732
        const char code[] = "char a [ 2 ] = { '-' } ; memset ( a , '-' , sizeof ( a ) ) ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
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
            ASSERT_EQUALS("[test.cpp:1]: (error) Invalid number of character ({) when these macros are defined: ''.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "void f()) {}";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:1]: (error) Invalid number of character (() when these macros are defined: ''.\n", errout.str());
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

        {
            errout.str("");
            const char code[] = "void f()\n"
                                "{\n"
                                " foo(;\n"
                                "}\n";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp", "ABC"));
            ASSERT_EQUALS("[test.cpp:3]: (error) Invalid number of character (() when these macros are defined: 'ABC'.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "void f()\n"
                                "{\n"
                                " for(;;){ foo();\n"
                                "}\n";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:2]: (error) Invalid number of character ({) when these macros are defined: ''.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "void f()\n"
                                "{\n"
                                " a[10;\n"
                                "}\n";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:3]: (error) Invalid number of character ([) when these macros are defined: ''.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "{\n"
                                "   a(\n"
                                "}\n"
                                "{\n"
                                "   b());\n"
                                "}\n";
            Tokenizer tokenizer(0, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:2]: (error) Invalid number of character (() when these macros are defined: ''.\n", errout.str());
        }
    }

    void removeKeywords()
    {
        const char code[] = "if (__builtin_expect(!!(x), 1));";

        const std::string actual(tokenizeAndStringify(code, true));

        ASSERT_EQUALS("if ( ! ! x ) { ; }", actual);
    }


    /**
     * tokenize "signed i" => "signed int i"
     */
    void signed1()
    {
        {
            const char code1[] = "void foo ( signed int , float ) ;";
            ASSERT_EQUALS(code1, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "signed i ;";
            const char code2[] = "signed int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "signed int i ;";
            ASSERT_EQUALS(code1, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "int signed i ;";
            const char code2[] = "signed int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "for (signed i=0; i<10; i++)";
            const char code2[] = "for ( signed int i = 0 ; i < 10 ; i ++ )";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }
    }

    /**
     * tokenize "unsigned i" => "unsigned int i"
     * tokenize "unsigned" => "unsigned int"
     */
    void unsigned1()
    {
        // No changes..
        {
            const char code[] = "void foo ( unsigned int , float ) ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
        }

        // insert "int" after "unsigned"..
        {
            const char code1[] = "unsigned i ;";
            const char code2[] = "unsigned int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "int unsigned i ;";
            const char code2[] = "unsigned int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        // insert "int" after "unsigned"..
        {
            const char code1[] = "for (unsigned i=0; i<10; i++)";
            const char code2[] = "for ( unsigned int i = 0 ; i < 10 ; i ++ )";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        // "extern unsigned x;" => "extern int x;"
        {
            const char code1[] = "; extern unsigned x;";
            const char code2[] = "; extern unsigned int x ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }
    }

    void unsigned2()
    {
        const char code[] = "i = (unsigned)j;";
        const char expected[] = "i = ( unsigned int ) j ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    // simplify "unsigned" when using templates..
    void unsigned3()
    {
        {
            const char code[] = "; foo<unsigned>();";
            const char expected[] = "; foo<int> ( ) ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        {
            const char code[] = "; foo<unsigned int>();";
            const char expected[] = "; foo<int> ( ) ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
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

    void removeExceptionSpecification1()
    {
        const char code[] = "class A\n"
                            "{\n"
                            "private:\n"
                            "    void f() throw (std::runtime_error);\n"
                            "};\n"
                            "void A::f() throw (std::runtime_error)\n"
                            "{ }";

        const char expected[] = "class A\n"
                                "{\n"
                                "private:\n"
                                "void f ( ) ;\n"
                                "} ;\n"
                                "void A :: f ( )\n"
                                "{ }";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void removeExceptionSpecification2()
    {
        const char code[] = "class A\n"
                            "{\n"
                            "private:\n"
                            "    int value;\n"
                            "public:\n"
                            "    A::A() throw ()\n"
                            "      : value(0)\n"
                            "    { }\n"
                            "};\n";

        const char expected[] = "class A\n"
                                "{\n"
                                "private:\n"
                                "int value ;\n"
                                "public:\n"
                                "A :: A ( )\n"
                                ": value ( 0 )\n"
                                "{ }\n"
                                "} ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }


    void gt()
    {
        ASSERT_EQUALS("( i < 10 )", tokenizeAndStringify("(10>i)"));
        ASSERT_EQUALS("; i < 10 ;", tokenizeAndStringify(";10>i;"));
    }


    void simplifyString()
    {
        Tokenizer tokenizer;
        ASSERT_EQUALS("\"abc\"", tokenizer.simplifyString("\"abc\""));
        ASSERT_EQUALS("\"a\"", tokenizer.simplifyString("\"\\x3\""));
        ASSERT_EQUALS("\"a\"", tokenizer.simplifyString("\"\\x33\""));
        ASSERT_EQUALS("\"a3\"", tokenizer.simplifyString("\"\\x333\""));
    }

    void simplifyConst()
    {
        ASSERT_EQUALS("void foo ( ) { const int x ; }",
                      tokenizeAndStringify("void foo(){ int const x;}"));

        ASSERT_EQUALS("void foo ( ) { { } const long x ; }",
                      tokenizeAndStringify("void foo(){ {} long const x;}"));

        ASSERT_EQUALS("void foo ( ) { bar ( ) ; const char x ; }",
                      tokenizeAndStringify("void foo(){ bar(); char const x;}"));

        ASSERT_EQUALS("void foo ( const char x ) { }",
                      tokenizeAndStringify("void foo(char const x){}"));

        ASSERT_EQUALS("void foo ( int b , const char x ) { }",
                      tokenizeAndStringify("void foo(int b,char const x){}"));

        ASSERT_EQUALS("void foo ( ) { int * const x ; }",
                      tokenizeAndStringify("void foo(){ int * const x;}"));

    }

    void switchCase()
    {
        ASSERT_EQUALS("void foo ( int i ) { switch ( i ) { case -1 : break ; } }",
                      tokenizeAndStringify("void foo (int i) { switch(i) { case -1: break; } }"));
    }

    std::string simplifyFunctionPointers(const char code[])
    {
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyFunctionPointers();
        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (tok->isUnsigned())
                ostr << " unsigned";
            else if (tok->isSigned())
                ostr << " signed";
            if (tok->isLong())
                ostr << " long";
            ostr << (tok->isName() ? " " : "") << tok->str();
        }
        return ostr.str();
    }

    void functionpointer()
    {
        ASSERT_EQUALS(" void* f;", simplifyFunctionPointers("void (*f)();"));
        ASSERT_EQUALS(" void** f;", simplifyFunctionPointers("void *(*f)();"));
        ASSERT_EQUALS(" unsigned int* f;", simplifyFunctionPointers("unsigned int (*f)();"));
        ASSERT_EQUALS(" unsigned int** f;", simplifyFunctionPointers("unsigned int * (*f)();"));
    }

    void removeRedundantAssignment()
    {
        ASSERT_EQUALS("void f ( ) { ; int * q ; ; }", tokenizeAndStringify("void f() { int *p, *q; p = q; }", true));
        ASSERT_EQUALS("void f ( ) { ; ; int * q ; ; }", tokenizeAndStringify("void f() { int *p = 0, *q; p = q; }", true));
    }

    void removedeclspec()
    {
        ASSERT_EQUALS("a b", tokenizeAndStringify("a __declspec ( dllexport ) b"));
        ASSERT_EQUALS("int a ;", tokenizeAndStringify("__declspec(thread) __declspec(align(32)) int a;"));
        ASSERT_EQUALS("int i ;", tokenizeAndStringify("__declspec(allocate(\"mycode\")) int i;"));
        ASSERT_EQUALS("struct IUnknown ;", tokenizeAndStringify("struct __declspec(uuid(\"00000000-0000-0000-c000-000000000046\")) IUnknown;"));
        ASSERT_EQUALS("int x [ ] ;", tokenizeAndStringify("__declspec(property(get=GetX, put=PutX)) int x[];"));
    }

    void cpp0xtemplate()
    {
        const char *code = "template <class T>\n"
                           "void fn2 (T t = []{return 1;}())\n"
                           "{}\n"
                           "int main()\n"
                           "{\n"
                           "  fn2<int>();\n"
                           "}\n";
        ASSERT_EQUALS(";\n\n\nint main ( )\n{\nfn2<int> ( ) ;\n}void fn2<int> ( int t = [ ] { return 1 ; } ( ) )\n{ }", tokenizeAndStringify(code));
    }

    std::string arraySize_(const std::string &code)
    {
        // tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code.c_str());
        tokenizer.tokenize(istr, "test.cpp");

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (tok->isName())
                ostr << " ";
            ostr << tok->str();
        }

        return ostr.str();
    }

    void arraySize()
    {
        ASSERT_EQUALS("; int a[3]={1,2,3};", arraySize_(";int a[]={1,2,3};"));
    }

    std::string labels_(const std::string &code)
    {
        // the arraySize_ does what we want currently..
        return arraySize_(code);
    }

    void labels()
    {
        ASSERT_EQUALS(" void f(){ ab:; a=0;}", labels_("void f() { ab: a=0; }"));
    }

    // Check simplifyInitVar
    void checkSimplifyInitVar(const char code[], bool simplify = false)
    {
        // Tokenize..
        Settings settings;
        settings.inconclusive = true;
        settings._checkCodingStyle = true;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList();

        tokenizer.validate();
    }

    void simplifyInitVar()
    {
        {
            const char code[] = "int i ; int p(0);";
            ASSERT_EQUALS("int i ; int p ; p = 0 ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; int *p(0);";
            ASSERT_EQUALS("int i ; int * p ; p = 0 ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int p(0);";
            ASSERT_EQUALS("int p ; p = 0 ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int *p(0);";
            ASSERT_EQUALS("int * p ; p = 0 ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i ; int p(i);";
            // this can't be simplified because i doesn't have a varid yet
            ASSERT_EQUALS("int i ; int p ( i ) ;", tokenizeAndStringify(code, false));
            checkSimplifyInitVar(code, false);
            ASSERT_EQUALS("", errout.str());
            // this can be simplified because i shold have a varid
            ASSERT_EQUALS("int i ; int p ; p = i ;", tokenizeAndStringify(code, true));
            checkSimplifyInitVar(code, true);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; int *p(&i);";
            ASSERT_EQUALS("int i ; int * p ; p = & i ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; void *p(&i);";
            ASSERT_EQUALS("int i ; void * p ; p = & i ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S *p(&s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S * p ; p = & s ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; S s; S *p(&s);";
            ASSERT_EQUALS("struct S { } ; S s ; S * p ; p = & s ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "union S { int i; float f; }; union S s; union S *p(&s);";
            ASSERT_EQUALS("union S { int i ; float f ; } ; union S s ; union S * p ; p = & s ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "union S { int i; float f; }; S s; S *p(&s);";
            ASSERT_EQUALS("union S { int i ; float f ; } ; S s ; S * p ; p = & s ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class C { }; class C c; class C *p(&c);";
            ASSERT_EQUALS("class C { } ; class C c ; class C * p ; p = & c ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class C { }; C c; C *p(&c);";
            ASSERT_EQUALS("class C { } ; C c ; C * p ; p = & c ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S s1(s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S s1 ( s ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; S s; S s1(s);";
            ASSERT_EQUALS("struct S { } ; S s ; S s1 ( s ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S s1(&s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S s1 ( & s ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; S s; S s1(&s);";
            ASSERT_EQUALS("struct S { } ; S s ; S s1 ( & s ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class S { int function(); };";
            ASSERT_EQUALS("class S { int function ( ) ; } ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class S { int function(void); };";
            ASSERT_EQUALS("class S { int function ( void ) ; } ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class S { int function(int); };";
            ASSERT_EQUALS("class S { int function ( int ) ; } ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function(void);";
            ASSERT_EQUALS("int function ( void ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function(int);";
            ASSERT_EQUALS("int function ( int ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "extern int function(void);";
            ASSERT_EQUALS("extern int function ( void ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function1(void); int function2(void);";
            ASSERT_EQUALS("int function1 ( void ) ; int function2 ( void ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function(A);";
            // We can't tell if this a function prototype or a variable without knowing
            // what A is. Since A is undefined, just leave it alone.
            ASSERT_EQUALS("int function ( A ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; int function(A);";
            ASSERT_EQUALS("int i ; int function ( A ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class A { } ; int foo(A);";
            // don't know what A is yet so leave it alone
            ASSERT_EQUALS("class A { } ; int foo ( A ) ;", tokenizeAndStringify(code, false));
            checkSimplifyInitVar(code, false);
            ASSERT_EQUALS("", errout.str());
            // we know A is not a variable here so leave it alone
            ASSERT_EQUALS("class A { } ; int foo ( A ) ;", tokenizeAndStringify(code, true));
            checkSimplifyInitVar(code, true);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class A { } ; A a; int foo(a);";
            // don't know what a is yet so leave it alone
            ASSERT_EQUALS("class A { } ; A a ; int foo ( a ) ;", tokenizeAndStringify(code, false));
            checkSimplifyInitVar(code, false);
            ASSERT_EQUALS("", errout.str());
            // we know a is a variable here so simplify it
            ASSERT_EQUALS("class A { } ; A a ; int foo ; foo = a ;", tokenizeAndStringify(code, true));
            checkSimplifyInitVar(code, true);
            ASSERT_EQUALS("", errout.str());
        }
    }
};

REGISTER_TEST(TestTokenizer)
