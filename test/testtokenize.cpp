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
#include <cstring>

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
        TEST_CASE(tokenize6);   // array access. replace "*(p+1)" => "p[1]"
        TEST_CASE(tokenize7);
        TEST_CASE(tokenize8);
        TEST_CASE(tokenize9);
        TEST_CASE(tokenize10);
        TEST_CASE(tokenize11);
        TEST_CASE(tokenize12);
        TEST_CASE(tokenize13);  // bailout if the code contains "@" - that is not handled well.
        TEST_CASE(tokenize14);  // tokenize "0X10" => 16
        TEST_CASE(tokenize15);  // tokenize ".123"
        TEST_CASE(tokenize16);  // #2612 - segfault for "<><<"
        TEST_CASE(tokenize17);  // #2759
        TEST_CASE(tokenize18);  // tokenize "(X&&Y)" into "( X && Y )" instead of "( X & & Y )"
        TEST_CASE(tokenize19);  // #3006 (segmentation fault)
        TEST_CASE(tokenize20);  // replace C99 _Bool => bool

        // don't freak out when the syntax is wrong
        TEST_CASE(wrong_syntax);
        TEST_CASE(wrong_syntax_if_macro);  // #2518 - if MACRO()

        TEST_CASE(minus);

        TEST_CASE(longtok);

        TEST_CASE(removeCast1);
        TEST_CASE(removeCast2);
        TEST_CASE(removeCast3);
        TEST_CASE(removeCast4);
        TEST_CASE(removeCast5);
        TEST_CASE(removeCast6);

        TEST_CASE(inlineasm);

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
        TEST_CASE(ifAddBraces13);
        TEST_CASE(ifAddBraces14);	// #2610 - segfault: if()<{}
        TEST_CASE(ifAddBraces15);	// #2616 - unknown macro before if

        TEST_CASE(whileAddBraces);
        TEST_CASE(doWhileAddBraces);

        TEST_CASE(forAddBraces);

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
        TEST_CASE(simplifyKnownVariables26);
        TEST_CASE(simplifyKnownVariables27);
        TEST_CASE(simplifyKnownVariables28);
        TEST_CASE(simplifyKnownVariables29);    // ticket #1811
        TEST_CASE(simplifyKnownVariables30);
        TEST_CASE(simplifyKnownVariables31);
        TEST_CASE(simplifyKnownVariables32);    // const
        TEST_CASE(simplifyKnownVariables33);    // struct variable
        TEST_CASE(simplifyKnownVariables34);
        TEST_CASE(simplifyKnownVariables35);    // ticket #2353 - False positive: Division by zero 'if (x == 0) return 0; return 10 / x;'
        TEST_CASE(simplifyKnownVariables36);    // ticket #2304 - known value for strcpy parameter
        TEST_CASE(simplifyKnownVariables37);    // ticket #2398 - false positive caused by no simplification in for loop
        TEST_CASE(simplifyKnownVariables38);    // ticket #2399 - simplify conditions
        TEST_CASE(simplifyKnownVariables39);
        TEST_CASE(simplifyKnownVariables40);
        TEST_CASE(simplifyKnownVariables41);    // p=&x; if (p) ..
        TEST_CASE(simplifyKnownVariables42);    // ticket #2031 - known string value after strcpy
        TEST_CASE(simplifyKnownVariables43);
        TEST_CASE(simplifyKnownVariablesBailOutAssign1);
        TEST_CASE(simplifyKnownVariablesBailOutAssign2);
        TEST_CASE(simplifyKnownVariablesBailOutFor1);
        TEST_CASE(simplifyKnownVariablesBailOutFor2);
        TEST_CASE(simplifyKnownVariablesBailOutFor3);
        TEST_CASE(simplifyKnownVariablesBailOutMemberFunction);
        TEST_CASE(simplifyKnownVariablesBailOutConditionalIncrement);
        TEST_CASE(simplifyKnownVariablesBailOutSwitchBreak);	// ticket #2324
        TEST_CASE(simplifyKnownVariablesFloat);    // #2454 - float variable
        TEST_CASE(simplifyKnownVariablesClassMember);  // #2815 - value of class member may be changed by function call

        TEST_CASE(varid1);
        TEST_CASE(varid2);
        TEST_CASE(varid3);
        TEST_CASE(varid4);
        TEST_CASE(varid5);
        TEST_CASE(varid6);
        TEST_CASE(varid7);
        TEST_CASE(varidReturn1);
        TEST_CASE(varidReturn2);
        TEST_CASE(varid8);
        TEST_CASE(varid9);
        TEST_CASE(varid10);
        TEST_CASE(varid11);
        TEST_CASE(varid12);
        TEST_CASE(varid13);
        TEST_CASE(varid14);
        TEST_CASE(varid15);
        TEST_CASE(varid16);
        TEST_CASE(varid17);   // ticket #1810
        TEST_CASE(varid18);
        TEST_CASE(varid19);
        TEST_CASE(varid20);
        TEST_CASE(varid21);
        TEST_CASE(varid22);
        TEST_CASE(varid23);
        TEST_CASE(varid24);
        TEST_CASE(varid25);
        TEST_CASE(varid26);   // ticket #1967 (list of function pointers)
        TEST_CASE(varid27);	// Ticket #2280 (same name for namespace and variable)
        TEST_CASE(varid28);   // ticket #2630
        TEST_CASE(varid29);   // ticket #1974
        TEST_CASE(varid30);   // ticket #2614
        TEST_CASE(varid31);   // ticket #2831 (segmentation fault)
        TEST_CASE(varid32);   // ticket #2835 (segmentation fault)
        TEST_CASE(varid33);   // ticket #2875 (segmentation fault)
        TEST_CASE(varid34);   // ticket #2825
        TEST_CASE(varid35);   // ticket #2937
        TEST_CASE(varid36);   // ticket #2980 (segmentation fault)
        TEST_CASE(varidFunctionCall1);
        TEST_CASE(varidFunctionCall2);
        TEST_CASE(varidFunctionCall3);
        TEST_CASE(varidStl);
        TEST_CASE(varid_delete);
        TEST_CASE(varid_functions);
        TEST_CASE(varid_reference_to_containers);
        TEST_CASE(varid_in_class1);
        TEST_CASE(varid_in_class2);
        TEST_CASE(varid_operator);
        TEST_CASE(varid_throw);
        TEST_CASE(varid_unknown_macro);     // #2638 - unknown macro is not type

        TEST_CASE(varidclass1);
        TEST_CASE(varidclass2);
        TEST_CASE(varidclass3);
        TEST_CASE(varidclass4);
        TEST_CASE(varidclass5);
        TEST_CASE(varidclass6);
        TEST_CASE(varidclass7);
        TEST_CASE(varidclass8);
        TEST_CASE(varidclass9);
        TEST_CASE(varidclass10);  // variable declaration below usage
        TEST_CASE(varidclass11);  // variable declaration below usage
        TEST_CASE(varidclass12);
        TEST_CASE(varidclass13);

        TEST_CASE(file1);
        TEST_CASE(file2);
        TEST_CASE(file3);

        TEST_CASE(doublesharp);

        TEST_CASE(macrodoublesharp);

        TEST_CASE(simplify_function_parameters);

        TEST_CASE(removeParentheses1);       // Ticket #61
        TEST_CASE(removeParentheses2);
        TEST_CASE(removeParentheses3);
        TEST_CASE(removeParentheses4);       // Ticket #390
        TEST_CASE(removeParentheses5);       // Ticket #392
        TEST_CASE(removeParentheses6);
        TEST_CASE(removeParentheses7);
        TEST_CASE(removeParentheses8);       // Ticket #1865
        TEST_CASE(removeParentheses9);       // Ticket #1962
        TEST_CASE(removeParentheses10);      // Ticket #2320
        TEST_CASE(removeParentheses11);      // Ticket #2505
        TEST_CASE(removeParentheses12);      // Ticket #2760 ',(b)='

        TEST_CASE(tokenize_double);
        TEST_CASE(tokenize_strings);
        TEST_CASE(simplify_constants);
        TEST_CASE(simplify_constants2);
        TEST_CASE(simplify_constants3);
        TEST_CASE(simplify_null);
        TEST_CASE(simplifyMulAnd);          // #2784

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
        TEST_CASE(vardecl11);
        TEST_CASE(vardecl12);
        TEST_CASE(vardecl13);
        TEST_CASE(vardecl14);
        TEST_CASE(vardecl_stl_1);
        TEST_CASE(vardecl_stl_2);
        TEST_CASE(vardecl_template);
        TEST_CASE(vardecl_union);
        TEST_CASE(vardecl_par);     // #2743 - set links if variable type contains parentheses
        TEST_CASE(volatile_variables);
        TEST_CASE(syntax_error);
        TEST_CASE(syntax_error_templates_1);
        TEST_CASE(syntax_error_templates_2);

        TEST_CASE(removeKeywords);

        // unsigned i; => unsigned int i;
        TEST_CASE(unsigned1);
        TEST_CASE(unsigned2);
        TEST_CASE(unsigned3);   // template arguments

        TEST_CASE(createLinks);
        TEST_CASE(signed1);

        TEST_CASE(removeExceptionSpecification1);
        TEST_CASE(removeExceptionSpecification2);
        TEST_CASE(removeExceptionSpecification3);

        TEST_CASE(gt);      // use "<" comparisons instead of ">"

        TEST_CASE(simplifyString);
        TEST_CASE(simplifyConst);
        TEST_CASE(switchCase);

        TEST_CASE(functionpointer1);
        TEST_CASE(functionpointer2);
        TEST_CASE(functionpointer3);

        TEST_CASE(removeRedundantAssignment);

        TEST_CASE(removedeclspec);
        TEST_CASE(removeattribute);
        TEST_CASE(cpp0xtemplate1);
        TEST_CASE(cpp0xtemplate2);
        TEST_CASE(cpp0xtemplate3);
        TEST_CASE(cpp0xdefault);

        TEST_CASE(arraySize);

        TEST_CASE(labels);
        TEST_CASE(simplifyInitVar);

        TEST_CASE(bitfields1);
        TEST_CASE(bitfields2);
        TEST_CASE(bitfields3);
        TEST_CASE(bitfields4); // ticket #1956
        TEST_CASE(bitfields5); // ticket #1956
        TEST_CASE(bitfields6); // ticket #2595
        TEST_CASE(bitfields7); // ticket #1987
        TEST_CASE(bitfields8);
        TEST_CASE(bitfields9); // ticket #2706
        TEST_CASE(bitfields10);
        TEST_CASE(bitfields11); // ticket #2845 (segmentation fault)

        TEST_CASE(microsoftMFC);

        TEST_CASE(borland);

        TEST_CASE(Qt);

        TEST_CASE(sql);

        TEST_CASE(simplifyLogicalOperators);

        TEST_CASE(simplifyCalculations); // ticket #2870

        // foo(p = new char[10]);  =>  p = new char[10]; foo(p);
        TEST_CASE(simplifyAssignmentInFunctionCall);

        // "x += .." => "x = x + .."
        TEST_CASE(simplifyCompoundAssignment);

        // Tokenize C#
        TEST_CASE(cs);

        // Tokenize JAVA
        TEST_CASE(java);

        TEST_CASE(simplifyOperatorName1);
        TEST_CASE(simplifyOperatorName2);
        TEST_CASE(simplifyOperatorName3);
        TEST_CASE(simplifyOperatorName4);
        TEST_CASE(simplifyOperatorName5);

        // Some simple cleanups of unhandled macros in the global scope
        TEST_CASE(removeMacrosInGlobalScope);

        // a = b = 0;
        TEST_CASE(multipleAssignment);

        TEST_CASE(simplifyIfAddBraces); // ticket # 2739 (segmentation fault)

        //remove redundant code after the 'return ;' statement
        TEST_CASE(removeRedundantCodeAfterReturn);

        TEST_CASE(platformWin32);
        TEST_CASE(platformWin64);
        TEST_CASE(platformUnix32);
        TEST_CASE(platformUnix64);
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


    std::string tokenizeAndStringify(const char code[], bool simplify = false, bool expand = true, Settings::PlatformType platform = Settings::Host)
    {
        errout.str("");

        Settings settings;
        settings.debugwarnings = true;
        settings.platform(platform);

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        if (simplify)
            tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            if (expand)
            {
                if (tok->isUnsigned())
                    ostr << "unsigned ";
                else if (tok->isSigned())
                    ostr << "signed ";

                if (tok->isLong())
                    ostr << "long ";
            }

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

    void tokenize6()
    {
        // "*(p+1)" => "p[1]"
        ASSERT_EQUALS("; x = p [ 1 ] ;", tokenizeAndStringify("; x = * ( p + 1 ) ;", true));
        ASSERT_EQUALS("; x = p [ n ] ;", tokenizeAndStringify("; x = * ( p + n ) ;", true));
    }

    void tokenize7()
    {
        const std::string code = "void f() {\n"
                                 "    int x1 = 1;\n"
                                 "    int x2(x1);\n"
                                 "}\n";
        ASSERT_EQUALS("void f ( ) {\nint x1 ; x1 = 1 ;\nint x2 ; x2 = x1 ;\n}",
                      tokenizeAndStringify(code.c_str(), false));
    }

    void tokenize8()
    {
        const std::string code = "void f() {\n"
                                 "    int x1(g());\n"
                                 "    int x2(x1);\n"
                                 "}\n";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void f ( ) {\n"
                      "2: int x1@1 ; x1@1 = g ( ) ;\n"
                      "3: int x2@2 ; x2@2 = x1@1 ;\n"
                      "4: }\n",
                      tokenizeDebugListing(code.c_str(), false));
    }

    void tokenize9()
    {
        errout.str("");
        const char code[] = "typedef void (*fp)();\n"
                            "typedef fp (*fpp)();\n"
                            "void f() {\n"
                            "    fpp x = (fpp)f();\n"
                            "}";
        tokenizeAndStringify(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void tokenize10()
    {
        ASSERT_EQUALS("private:", tokenizeAndStringify("private:", false));
        ASSERT_EQUALS("protected:", tokenizeAndStringify("protected:", false));
        ASSERT_EQUALS("public:", tokenizeAndStringify("public:", false));
        ASSERT_EQUALS("__published:", tokenizeAndStringify("__published:", false));
    }

    void tokenize11()
    {
        ASSERT_EQUALS("X * sizeof ( Y ( ) ) ;", tokenizeAndStringify("X * sizeof(Y());", false));
    }

    // ticket #2118 - invalid syntax error
    void tokenize12()
    {
        tokenizeAndStringify("Q_GLOBAL_STATIC_WITH_INITIALIZER(Qt4NodeStaticData, qt4NodeStaticData, {\n"
                             "    for (unsigned i = 0 ; i < count; i++) {\n"
                             "    }\n"
                             "});");
        ASSERT_EQUALS("", errout.str());
    }

    // bailout if there is "@" - it is not handled well
    void tokenize13()
    {
        const char code[] = "@implementation\n"
                            "-(Foo *)foo: (Bar *)bar\n"
                            "{ }\n"
                            "@end\n";
        ASSERT_EQUALS("", tokenizeAndStringify(code));
    }

    // Ticket #2361: 0X10 => 16
    void tokenize14()
    {
        ASSERT_EQUALS("; 16 ;", tokenizeAndStringify(";0x10;"));
        ASSERT_EQUALS("; 16 ;", tokenizeAndStringify(";0X10;"));
    }

    // Ticket #2429: 0.125
    void tokenize15()
    {
        ASSERT_EQUALS("0.125", tokenizeAndStringify(".125"));
    }

    // #2612 - segfault for "<><<"
    void tokenize16()
    {
        tokenizeAndStringify("<><<");
    }

    void tokenize17() // #2759
    {
        ASSERT_EQUALS("class B : private :: A { } ;", tokenizeAndStringify("class B : private ::A { };"));
    }

    void tokenize18() // tokenize "(X&&Y)" into "( X && Y )" instead of "( X & & Y )"
    {
        ASSERT_EQUALS("( X && Y )", tokenizeAndStringify("(X&&Y)"));
    }

    void tokenize19() // #3006 (segmentation fault)
    {
        tokenizeAndStringify("x < () <");
    }

    void tokenize20() // replace C99 _Bool => bool
    {
        ASSERT_EQUALS("bool a ; a = true ;", tokenizeAndStringify("_Bool a = true;"));
    }

    void wrong_syntax()
    {
        {
            errout.str("");
            const std::string code("TR(kvmpio, PROTO(int rw), ARGS(rw), TP_(aa->rw;))");
            ASSERT_EQUALS("TR ( kvmpio , PROTO ( int rw ) , ARGS ( rw ) , TP_ ( aa . rw ; ) )", tokenizeAndStringify(code.c_str(), true));
            ASSERT_EQUALS("", errout.str());
        }

        {
            errout.str("");
            const std::string code("struct A { template<int> struct { }; };");
            ASSERT_EQUALS("", tokenizeAndStringify(code.c_str(), true));
            ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
        }

        {
            errout.str("");
            const std::string code("enum ABC { A,B, typedef enum { C } };");
            tokenizeAndStringify(code.c_str(), true);
            ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
        }
    }

    void wrong_syntax_if_macro()
    {
        // #2518
        errout.str("");
        const std::string code("void f() { if MACRO(); }");
        tokenizeAndStringify(code.c_str(), false);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(filedata);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        ASSERT_EQUALS(std::string(10000, 'a'), tokenizer.tokens()->str());
    }



    // Don’t remove "(int *)"..
    void removeCast1()
    {
        const char code[] = "int *f(int *);";

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyCasts();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" t = & p ;", ostr.str());
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
        const char expected[] = "if ( a >= ( unsigned int ) b ) { }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void removeCast5()
    {
        // ticket #1817
        ASSERT_EQUALS("a . data = f ;", tokenizeAndStringify("a->data = reinterpret_cast<void*>(static_cast<intptr_t>(f));", true));
    }

    void removeCast6()
    {
        // ticket #2103
        ASSERT_EQUALS("if ( ! x )", tokenizeAndStringify("if (x == (char *) ((void *)0))", true));
    }

    void inlineasm()
    {
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";asm { mov ax,bx };"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";_asm { mov ax,bx };"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm { mov ax,bx };"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm__ __volatile__ ( \"mov ax,bx\" );"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm _emit 12h ;"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm mov a, b ;"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";asm volatile (\"fnstcw %0\" : \"= m\" (old_cw));"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify("; __asm__ (\"fnstcw %0\" : \"= m\" (old_cw));"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify("; __asm __volatile__ (\"ddd\") ;"));
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(";__asm__ volatile ( \"mov ax,bx\" );"));
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
        ASSERT_EQUALS("a & & b", tokenizeAndStringify("a & &b", true));

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
            "void f() {"
            "    for (int k=0; k<VectorSize; k++)"
            "        LOG_OUT(ID_Vector[k])"
            "}";
        const char expected[] =
            "void f ( ) { "
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

    void ifAddBraces13()
    {
        // ticket #1809
        const char code[] = "{ if (x) if (y) { } else { } else { } }";
        const char expected[] = "{ if ( x ) { if ( y ) { } else { } } else { } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));

        // ticket #1809
        const char code2[] = "{ if (x) while (y) { } else { } }";
        const char expected2[] = "{ if ( x ) { while ( y ) { } } else { } }";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2, true));
    }

    void ifAddBraces14()
    {
        // ticket #2610 (segfault)
        tokenizeAndStringify("if()<{}", false);
    }

    void ifAddBraces15()
    {
        // ticket #2616 - unknown macro before if
        ASSERT_EQUALS("{ A if ( x ) { y ( ) ; } }", tokenizeAndStringify("{A if(x)y();}", false));
    }


    void whileAddBraces()
    {
        const char code[] = ";while(a);";
        ASSERT_EQUALS("; while ( a ) { ; }", tokenizeAndStringify(code, true));
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

    void forAddBraces()
    {
        {
            const char code[] = "void f() {\n"
                                "     for(;;)\n"
                                "         if (a) { }\n"
                                "         else { }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "for ( ; ; ) {\n"
                                    "if ( a ) { }\n"
                                    "else { } }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void f() {\n"
                                "     for(;;)\n"
                                "         if (a) { }\n"
                                "         else if (b) { }\n"
                                "         else { }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "for ( ; ; ) {\n"
                                    "if ( a ) { }\n"
                                    "else { if ( b ) { }\n"
                                    "else { } } }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }
    }


    std::string simplifyKnownVariables(const char code[])
    {
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
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

            const std::string expected1("void f ( ) {"
                                        " bool b ; b = false ;"
                                        " { b = true ; }");

            TODO_ASSERT_EQUALS(
                expected1 + " if ( true ) { a ( ) ; } }",
                expected1 + " if ( b ) { a ( ) ; } }",
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
                "void f ( ) { bool b ; b = false ; { b = false ; } { b = true ; } if ( b ) { a ( ) ; } }",
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
            "; int main ( ) { int foo ; foo = 0 ; }",
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
                "void foo ( ) { int n ; n = 10 ; i = 10 >> 1 ; }",
                simplifyKnownVariables(code));
        }
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int n = 10;\n"
                                "    i = (n << 1);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) { int n ; n = 10 ; i = 10 << 1 ; }",
                simplifyKnownVariables(code));
        }
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int n = 10;\n"
                                "    i = (1 << n);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) { int n ; n = 10 ; i = 1 << 10 ; }",
                simplifyKnownVariables(code));
        }
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int n = 10;\n"
                                "    i = (1 >> n);\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) { int n ; n = 10 ; i = 1 >> 10 ; }",
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

        TODO_ASSERT_EQUALS(
            "void foo ( int x ) "
            "{"
            " int a [ 10 ] ; int c ; c = 0 ;"
            " if ( x ) { a [ 0 ] = 0 ; c = 1 ; }"
            " else { a [ 0 ] = 0 ; } "
            "}",

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
        {
            // This testcase is related to ticket #1596
            const char code[] = "void foo()\n"
                                "{\n"
                                "    int c;\n"
                                "    for (c=0;c<10;++c) { }\n"
                                "    a[c] = 0;\n"
                                "}\n";

            ASSERT_EQUALS(
                "void foo ( ) "
                "{"
                " int c ;"
                " for ( c = 0 ; c < 10 ; ++ c ) { }"
                " a [ 10 ] = 0 ; "
                "}",
                simplifyKnownVariables(code));
        }

        {
            // #1692 - unknown counter value after for loop
            const char code[] = "void foo(const char s[])\n"
                                "{\n"
                                "    int x[3];\n"
                                "    int i;\n"
                                "    for (i = 0; i < 3; ++i) {\n"
                                "        if (s[i]) break;\n"
                                "    }"
                                "    if (i < 3) x[i] = 0;\n"
                                "}\n";
            ASSERT_EQUALS(
                "void foo ( const char s [ ] ) "
                "{"
                " int x [ 3 ] ;"
                " int i ;"
                " for ( i = 0 ; i < 3 ; ++ i ) {"
                " if ( s [ i ] ) { break ; }"
                " }"
                " if ( i < 3 ) { x [ i ] = 0 ; } "
                "}",
                simplifyKnownVariables(code));
        }
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
                " if ( ! * str ) { goto label ; }"
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
                " return ; "
                "}",
                simplifyKnownVariables(code));
        }
    }

    void simplifyKnownVariables26()
    {
        // This testcase is related to ticket #887
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i;\n"
                            "    for (i=0;i<10;++i) { }\n"
                            "    int k = i++;\n"
                            "}\n";
        ASSERT_EQUALS(
            "void foo ( ) "
            "{"
            " int i ;"
            " for ( i = 0 ; i < 10 ; ++ i ) { }"
            " int k ; k = 10 ; "
            "}",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables27()
    {
        // This testcase is related to ticket #1633
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i1 = 1;\n"
                            "    int i2 = 2;\n"
                            "    int i3 = (i1 + i2) * 3;\n"
                            "}\n";
        ASSERT_EQUALS(
            "void foo ( ) "
            "{"
            " int i1 ; i1 = 1 ;"
            " int i2 ; i2 = 2 ;"
            " int i3 ; i3 = ( 1 + 2 ) * 3 ; "
            "}",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables28()
    {
        const char code[] = "void foo(int g)\n"
                            "{\n"
                            "  int i = 2;\n"
                            "  if (g) {\n"
                            "  }\n"
                            "  if (i > 0) {\n"
                            "  }\n"
                            "}\n";
        ASSERT_EQUALS(
            "void foo ( int g ) "
            "{"
            " int i ; i = 2 ;"
            " if ( g ) { }"
            " if ( 0 < 2 ) { } "
            "}",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables29() // ticket #1811
    {
        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h + i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 + v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h - i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 - v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h * i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 * v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h / i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 / v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h & i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 & v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h | i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 | v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h ^ i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 ^ v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h % i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 % v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h >> i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 >> v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h << i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 << v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h == i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 == v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h != i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 != v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h > i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 > v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h >= i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 >= v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h < i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 < v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h <= i;\n"
                                "}\n";
            const char expected[] = "\n\n##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3: ;\n"
                                    "4:\n"
                                    "5: return u@1 <= v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h && i;\n"
                                "}\n";
            const char wanted[] = "\n\n##file 0\n"
                                  "1: bool foo ( int u@1 , int v@2 )\n"
                                  "2: {\n"
                                  "3: ;\n"
                                  "4: ;\n"
                                  "5: return u@1 && v@2 ;\n"
                                  "6: }\n";
            const char current[] =  "\n\n##file 0\n1: bool foo ( int u@1 , int v@2 )\n2: {\n3: ;\n4: int i@4 ; i@4 = v@2 ;\n5: return u@1 && i@4 ;\n6: }\n";
            TODO_ASSERT_EQUALS(wanted, current, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h || i;\n"
                                "}\n";
            const char wanted[] = "\n\n##file 0\n"
                                  "1: bool foo ( int u@1 , int v@2 )\n"
                                  "2: {\n"
                                  "3: ;\n"
                                  "4: ;\n"
                                  "5: return u@1 || v@2 ;\n"
                                  "6: }\n";
            const char current[] =  "\n\n##file 0\n1: bool foo ( int u@1 , int v@2 )\n2: {\n3: ;\n4: int i@4 ; i@4 = v@2 ;\n5: return u@1 || i@4 ;\n6: }\n";
            TODO_ASSERT_EQUALS(wanted, current, tokenizeDebugListing(code, true));
        }
    }

    void simplifyKnownVariables30()
    {
        const char code[] = "int foo() {\n"
                            "  iterator it1 = ints.begin();\n"
                            "  iterator it2 = it1;\n"
                            "  for (++it2;it2!=ints.end();++it2);\n"
                            "}\n";
        const char expected[] = "int foo ( ) {\n"
                                "iterator it1 ; it1 = ints . begin ( ) ;\n"
                                "iterator it2 ; it2 = it1 ;\n"
                                "for ( ++ it2 ; it2 != ints . end ( ) ; ++ it2 ) { ; }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables31()
    {
        const char code[] = "void foo(const char str[]) {\n"
                            "    const char *p = str;\n"
                            "    if (p[0] == 0) {\n"
                            "    }\n"
                            "}\n";
        const char expected[] = "void foo ( const char str [ ] ) {\n"
                                "const char * p ; p = str ;\n"
                                "if ( str [ 0 ] == 0 ) {\n"
                                "}\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables32()
    {
        {
            const char code[] = "void foo() {\n"
                                "    const int x = 0;\n"
                                "    bar(0,x);\n"
                                "}\n";
            const char expected[] = "void foo ( ) {\n;\nbar ( 0 , 0 ) ;\n}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "static int const SZ = 22; char str[SZ];\n";
            ASSERT_EQUALS("; char str [ 22 ] ;", tokenizeAndStringify(code,true));
        }
    }

    void simplifyKnownVariables33()
    {
        const char code[] = "static void foo(struct Foo *foo) {\n"
                            "    foo->a = 23;\n"
                            "    x[foo->a] = 0;\n"
                            "}\n";
        const char expected[] = "static void foo ( struct Foo * foo ) {\n"
                                "foo . a = 23 ;\n"
                                "x [ 23 ] = 0 ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables34()
    {
        const char code[] = "void f() {\n"
                            "    int x = 10;\n"
                            "    do { cin >> x; } while (x > 5);\n"
                            "    a[x] = 0;\n"
                            "}\n";
        const char expected[] = "void f ( ) {\n"
                                "int x ; x = 10 ;\n"
                                "do { cin >> x ; } while ( 5 < x ) ;\n"
                                "a [ x ] = 0 ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables35()
    {
        // Ticket #2353
        const char code[] = "int f() {"
                            "    int x = 0;"
                            "    if (x == 0) {"
                            "        return 0;"
                            "    }"
                            "    return 10 / x;"
                            "}";
        const char expected[] = "int f ( ) { int x ; x = 0 ; { return 0 ; } return 10 / x ; }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables36()
    {
        // Ticket #2304
        const char code[] = "void f() {"
                            "    const char *q = \"hello\";"
                            "    strcpy(p, q);"
                            "}";
        const char expected[] = "void f ( ) { const char * q ; q = \"hello\" ; strcpy ( p , \"hello\" ) ; }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables37()
    {
        // Ticket #2398 - no simplication in for loop
        const char code[] = "void f() {\n"
                            "    double x = 0;\n"
                            "    for (int iter=0; iter<42; iter++) {\n"
                            "        int EvaldF = 1;\n"
                            "        if (EvaldF)\n"
                            "            Eval (x);\n"
                            "    }\n"
                            "}";
        const char expected[] = "void f ( ) {\n"
                                "double x ; x = 0 ;\n"
                                "for ( int iter = 0 ; iter < 42 ; iter ++ ) {\n"
                                ";\n"
                                "{\n"
                                "Eval ( x ) ; }\n"
                                "}\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables38()
    {
        // Ticket #2399 - simplify conditions
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    int y = 1;\n"
                            "    if (x || y);\n"
                            "}";
        const char expected[] = "void f ( ) {\n"
                                ";\n"
                                "\n"
                                "\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables39()
    {
        // Ticket #2296 - simplify pointer alias 'delete p;'
        {
            const char code[] = "void f() {\n"
                                "    int *x;\n"
                                "    int *y = x;\n"
                                "    delete y;\n"
                                "}";
            ASSERT_EQUALS("void f ( ) {\nint * x ;\n\ndelete x ;\n}", tokenizeAndStringify(code, true));
        }
        {
            const char code[] = "void f() {\n"
                                "    int *x;\n"
                                "    int *y = x;\n"
                                "    delete [] y;\n"
                                "}";
            ASSERT_EQUALS("void f ( ) {\nint * x ;\n\ndelete [ ] x ;\n}", tokenizeAndStringify(code, true));
        }
    }


    void simplifyKnownVariables40()
    {
        const char code[] = "void f() {\n"
                            "    char c1 = 'a';\n"
                            "    char c2 = { c1 };\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n;\nchar c2 ; c2 = { 'a' } ;\n}", tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables41()
    {
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    const int *p; p = &x;\n"
                            "    if (p) { return 0; }\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\nint x ; x = 0 ;\nconst int * p ; p = & x ;\nif ( & x ) { return 0 ; }\n}", tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables42()
    {
        {
            const char code[] = "void f() {\n"
                                "    char str1[10], str2[10];\n"
                                "    strcpy(str1, \"abc\");\n"
                                "    strcpy(str2, str1);\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char str1 [ 10 ] ; char str2 [ 10 ] ;\n"
                                    "strcpy ( str1 , \"abc\" ) ;\n"
                                    "strcpy ( str2 , \"abc\" ) ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void f() {"
                                "    char *s = malloc(10);"
                                "    strcpy(s, \"\");"
                                "    free(s);"
                                "}";
            const char expected[] = "void f ( ) {"
                                    " char * s ; s = malloc ( 10 ) ;"
                                    " strcpy ( s , \"\" ) ;"
                                    " free ( s ) ; "
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void f(char *p, char *q) {"
                                "    strcpy(p, \"abc\");"
                                "    q = p;"
                                "}";
            const char expected[] = "void f ( char * p , char * q ) {"
                                    " strcpy ( p , \"abc\" ) ;"
                                    " q = p ; "
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }
    }

    void simplifyKnownVariables43()
    {
        {
            const char code[] = "void f() {\n"
                                "    int a, *p; p = &a;\n"
                                "    { int a = *p; }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "int a ; int * p ; p = & a ;\n"
                                    "{ int a ; a = * p ; }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void f() {\n"
                                "    int *a, **p; p = &a;\n"
                                "    { int *a = *p; }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "int * a ; int * * p ; p = & a ;\n"
                                    "{ int * a ; a = * p ; }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }
    }

    void simplifyKnownVariablesBailOutAssign1()
    {
        const char code[] = "int foo() {\n"
                            "    int i; i = 0;\n"
                            "    if (x) { i = 10; }\n"
                            "    return i;\n"
                            "}\n";
        const char expected[] = "int foo ( ) {\n"
                                "int i ; i = 0 ;\n"
                                "if ( x ) { i = 10 ; }\n"
                                "return i ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariablesBailOutAssign2()
    {
        // ticket #3032 - assignment in condition
        const char code[] = "void f(struct ABC *list) {\n"
                            "    struct ABC *last = NULL;\n"
                            "    nr = (last = list->prev)->nr;\n"  // <- don't replace "last" with 0
                            "}\n";
        const char expected[] = "void f ( struct ABC * list ) {\n"
                                "struct ABC * last ; last = 0 ;\n"
                                "nr = ( last = list . prev ) . nr ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariablesBailOutFor1()
    {
        const char code[] = "void foo() {\n"
                            "    for (int i = 0; i < 10; ++i) { }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "for ( int i = 0 ; i < 10 ; ++ i ) { }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        ASSERT_EQUALS("", errout.str());	// debug warnings
    }

    void simplifyKnownVariablesBailOutFor2()
    {
        const char code[] = "void foo() {\n"
                            "    int i = 0;\n"
                            "    while (i < 10) { ++i; }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "int i ; i = 0 ;\n"
                                "while ( i < 10 ) { ++ i ; }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        ASSERT_EQUALS("", errout.str());	// debug warnings
    }

    void simplifyKnownVariablesBailOutFor3()
    {
        const char code[] = "void foo() {\n"
                            "    for (std::string::size_type pos = 0; pos < 10; ++pos)\n"
                            "    { }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "for ( std :: string :: size_type pos = 0 ; pos < 10 ; ++ pos )\n"
                                "{ }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        ASSERT_EQUALS("", errout.str());	// debug warnings
    }

    void simplifyKnownVariablesBailOutMemberFunction()
    {
        const char code[] = "void foo(obj a) {\n"
                            "    obj b = a;\n"
                            "    b.f();\n"
                            "}\n";
        const char expected[] = "void foo ( obj a ) {\n"
                                "obj b ; b = a ;\n"
                                "b . f ( ) ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariablesBailOutConditionalIncrement()
    {
        const char code[] = "int f() {\n"
                            "    int a = 0;\n"
                            "    if (x) {\n"
                            "        ++a;\n"	// conditional increment
                            "    }\n"
                            "    return a;\n"
                            "}\n";
        tokenizeAndStringify(code,true);
        ASSERT_EQUALS("", errout.str());	// no debug warnings
    }

    void simplifyKnownVariablesBailOutSwitchBreak()
    {
        // Ticket #2324
        const char code[] = "int f(char *x) {\n"
                            "    char *p;\n"
                            "    char *q;\n"
                            "\n"
                            "    switch (x & 0x3)\n"
                            "    {\n"
                            "        case 1:\n"
                            "            p = x;\n"
                            "            x = p;\n"
                            "            break;\n"
                            "        case 2:\n"
                            "            q = x;\n"	// x is not equal with p
                            "            x = q;\n"
                            "            break;\n"
                            "    }\n"
                            "}\n";

        const char expected[] = "int f ( char * x ) {\n"
                                "char * p ;\n"
                                "char * q ;\n"
                                "\n"
                                "switch ( x & 3 )\n"
                                "{\n"
                                "case 1 : ;\n"
                                "p = x ;\n"
                                "x = p ;\n"
                                "break ;\n"
                                "case 2 : ;\n"
                                "q = x ;\n"
                                "x = q ;\n"
                                "break ;\n"
                                "}\n"
                                "}";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));
    }

    void simplifyKnownVariablesFloat()
    {
        // Ticket #2454
        const char code[] = "void f() {\n"
                            "    float a = 40;\n"
                            "    x(10 / a);\n"
                            "}\n";

        const char expected[] = "void f ( ) {\n;\nx ( 0.25 ) ;\n}";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));
    }

    void simplifyKnownVariablesClassMember()
    {
        // Ticket #2815
        {
            const char code[] = "char *a;\n"
                                "void f(const char *s) {\n"
                                "    a = NULL;\n"
                                "    x();\n"
                                "    memcpy(a, s, 10);\n"   // <- don't simplify "a" here
                                "}\n";

            const std::string s(tokenizeAndStringify(code, true));
            ASSERT_EQUALS(true, s.find("memcpy ( a , s , 10 ) ;") != std::string::npos);
        }

        // If the variable is local then perform simplification..
        {
            const char code[] = "void f(const char *s) {\n"
                                "    char *a = NULL;\n"
                                "    x();\n"
                                "    memcpy(a, s, 10);\n"   // <- simplify "a"
                                "}\n";

            const std::string s(tokenizeAndStringify(code, true));
            TODO_ASSERT_EQUALS(true, false, s.find("memcpy ( 0 , s , 10 ) ;") != std::string::npos);
        }
    }



    std::string tokenizeDebugListing(const std::string &code, bool simplify = false)
    {
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
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
                                   "3: ;\n"
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

    void varidReturn1()
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

    void varidReturn2()
    {
        const std::string actual = tokenizeDebugListing(
                                       "void foo()\n"
                                       "{\n"
                                       "    unsigned long mask = (1UL << size_) - 1;\n"
                                       "    return (abits_val_ & mask);\n"
                                       "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: long mask@1 ; mask@1 = ( 1UL << size_ ) - 1 ;\n"
                                   "4: return ( abits_val_ & mask@1 ) ;\n"
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
                                   "3: int a@1 ;\n"
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
                                   "4: y = z * x@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid17() // ticket #1810
    {
        const std::string code("char foo()\n"
                               "{\n"
                               "    char c('c');\n"
                               "    return c;\n"
                               "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: char foo ( )\n"
                                   "2: {\n"
                                   "3: char c@1 ( 'c' ) ;\n"
                                   "4: return c@1 ;\n"
                                   "5: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid18()
    {
        const std::string code("char foo(char c)\n"
                               "{\n"
                               "    bar::c = c;\n"
                               "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: char foo ( char c@1 )\n"
                                   "2: {\n"
                                   "3: bar :: c = c@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid19()
    {
        const std::string code("void foo()\n"
                               "{\n"
                               "    std::pair<std::vector<double>, int> x;\n"
                               "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: std :: pair < std :: vector < double > , int > x@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid20()
    {
        const std::string code("void foo()\n"
                               "{\n"
                               "    pair<vector<int>, vector<double> > x;\n"
                               "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: pair < vector < int > , vector < double > > x@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid21()
    {
        const std::string code("void foo()\n"
                               "{\n"
                               "    vector<const std::string &> x;\n"
                               "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: void foo ( )\n"
                                   "2: {\n"
                                   "3: vector < const std :: string & > x@1 ;\n"
                                   "4: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid22()
    {
        const std::string code("class foo()\n"
                               "{\n"
                               "public:\n"
                               "    vector<const std::string &> x;\n"
                               "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class foo ( )\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: vector < const std :: string & > x@1 ;\n"
                                   "5: } ;\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid23()
    {
        const std::string code("class foo()\n"
                               "{\n"
                               "public:\n"
                               "    static vector<const std::string &> x;\n"
                               "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class foo ( )\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: static vector < const std :: string & > x@1 ;\n"
                                   "5: } ;\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid24()
    {
        const std::string code("class foo()\n"
                               "{\n"
                               "public:\n"
                               "    ;\n"
                               "private:\n"
                               "    static int i;\n"
                               "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class foo ( )\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: ;\n"
                                   "5: private:\n"
                                   "6: static int i@1 ;\n"
                                   "7: } ;\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid25()
    {
        const std::string code("class foo()\n"
                               "{\n"
                               "public:\n"
                               "    ;\n"
                               "private:\n"
                               "    mutable int i;\n"
                               "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class foo ( )\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: ;\n"
                                   "5: private:\n"
                                   "6: mutable int i@1 ;\n"
                                   "7: } ;\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid26()
    {
        const std::string code("list<int (*)()> functions;\n");
        const std::string expected("\n\n##file 0\n"
                                   "1: list < int ( * ) ( ) > functions@1 ;\n");
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid27()
    {
        const std::string code("int fooled_ya;\n"
                               "fooled_ya::iterator iter;\n");
        const std::string expected("\n\n##file 0\n"
                                   "1: int fooled_ya@1 ;\n"
                                   "2: fooled_ya :: iterator iter@2 ;\n");
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid28() // ticket #2630 (segmentation fault)
    {
        tokenizeDebugListing("template <typedef A>\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varid29()
    {
        const std::string code("class A {\n"
                               "    B<C<1>,1> b;\n"
                               "};\n");
        const std::string expected("\n\n##file 0\n"
                                   "1: class A {\n"
                                   "2: B < C < 1 > , 1 > b@1 ;\n"
                                   "3: } ;\n");
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varid30() // ticket #2614
    {
        const std::string code1("void f(EventPtr *eventP, ActionPtr **actionsP)\n"
                                "{\n"
                                "    EventPtr event = *eventP;\n"
                                "    *actionsP = &event->actions;\n"
                                "}\n");
        const std::string expected1("\n\n##file 0\n"
                                    "1: void f ( EventPtr * eventP@1 , ActionPtr * * actionsP@2 )\n"
                                    "2: {\n"
                                    "3: EventPtr event@3 ; event@3 = * eventP@1 ;\n"
                                    "4: * actionsP@2 = & event@3 . actions@4 ;\n"
                                    "5: }\n");
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1));

        const std::string code2("void f(int b, int c) {\n"
                                "    x(a*b*c,10);\n"
                                "}\n");
        const std::string expected2("\n\n##file 0\n"
                                    "1: void f ( int b@1 , int c@2 ) {\n"
                                    "2: x ( a@3 * b@1 * c@2 , 10 ) ;\n"
                                    "3: }\n");
        const std::string actual2("\n\n##file 0\n"
                                  "1: void f ( int b@1 , int c@2 ) {\n"
                                  "2: x ( a * b@1 * c@3 , 10 ) ;\n"
                                  "3: }\n");
        TODO_ASSERT_EQUALS(expected2, actual2, tokenizeDebugListing(code2));

        const std::string code3("class Nullpointer : public ExecutionPath\n"
                                " {\n"
                                "    Nullpointer(Check *c, const unsigned int id, const std::string &name)\n"
                                "        : ExecutionPath(c, id)\n"
                                "    {\n"
                                "    }\n"
                                "}\n");
        const std::string expected3("\n\n##file 0\n"
                                    "1: class Nullpointer : public ExecutionPath\n"
                                    "2: {\n"
                                    "3: Nullpointer ( Check * c@1 , const int id@2 , const std :: string & name@3 )\n"
                                    "4: : ExecutionPath ( c@1 , id@2 )\n"
                                    "5: {\n"
                                    "6: }\n"
                                    "7: }\n");
        ASSERT_EQUALS(expected3, tokenizeDebugListing(code3));
    }

    void varid31()   // ticket #2831 (segmentation fault)
    {
        const std::string code("z<y<x>");
        ASSERT_EQUALS("", errout.str());
    }

    void varid32()   // ticket #2835 (segmentation fault)
    {
        const std::string code("><,f<i,");
        ASSERT_EQUALS("", errout.str());
    }

    void varid33()   // ticket #2875 (segmentation fault)
    {
        const std::string code("0; (a) < (a)");
        ASSERT_EQUALS("", errout.str());
    }

    void varid34()   // ticket #2825
    {
        const std::string code("class Fred : public B1, public B2\n"
                               "{\n"
                               "public:\n"
                               "    Fred() { a = 0; }\n"
                               "private:\n"
                               "    int a;\n"
                               "};\n");
        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred : public B1 , public B2\n"
                                   "2: {\n"
                                   "3: public:\n"
                                   "4: Fred ( ) { a@1 = 0 ; }\n"
                                   "5: private:\n"
                                   "6: int a@1 ;\n"
                                   "7: } ;\n");
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        ASSERT_EQUALS("", errout.str());
    }

    void varid35()   // ticket #2937
    {
        const std::string code("int foo() {\n"
                               "    int f(x);\n"
                               "    return f;\n"
                               "}\n");
        const std::string expected("\n\n##file 0\n"
                                   "1: int foo ( ) {\n"
                                   "2: int f@1 ( x ) ;\n"
                                   "3: return f@1 ;\n"
                                   "4: }\n");
        const std::string actual("\n\n##file 0\n"
                                 "1: int foo ( ) {\n"
                                 "2: int f ( x ) ;\n"
                                 "3: return f ;\n"
                                 "4: }\n");
        TODO_ASSERT_EQUALS(expected, actual, tokenizeDebugListing(code));
    }

    void varid36()   // ticket #2980 (segmentation fault)
    {
        const std::string code("#elif A\n"
                               "A,a<b<x0;\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varidFunctionCall1()
    {
        const std::string code("void f() {\n"
                               "    int x;\n"
                               "    x = a(y*x,10);\n"
                               "}");
        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( ) {\n"
                                   "2: int x@1 ;\n"
                                   "3: x@1 = a ( y * x@1 , 10 ) ;\n"
                                   "4: }\n");
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varidFunctionCall2()
    {
        // #2491
        const std::string code("void f(int b) {\n"
                               "    x(a*b,10);\n"
                               "}");
        const std::string expected1("\n\n##file 0\n"
                                    "1: void f ( int b@1 ) {\n"
                                    "2: x ( a * b");
        const std::string expected2(" , 10 ) ;\n"
                                    "3: }\n");
        ASSERT_EQUALS(expected1+"@1"+expected2, tokenizeDebugListing(code));
    }

    void varidFunctionCall3()
    {
        // Ticket #2339
        const std::string code("void f() {\n"
                               "    int a = 0;\n"
                               "    int b = c - (foo::bar * a);\n"
                               "}");

        const std::string expected("\n\n##file 0\n"
                                   "1: void f ( ) {\n"
                                   "2: int a@1 ; a@1 = 0 ;\n"
                                   "3: int b@2 ; b@2 = c - ( foo :: bar * a@1 ) ;\n"
                                   "4: }\n");

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
                                   "5: std :: vector < int > * c@3 ; c@3 = & b@1 ;\n"
                                   "6: }\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid_in_class1()
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

    void varid_in_class2()
    {
        const std::string actual = tokenizeDebugListing(
                                       "struct Foo {\n"
                                       "    int x;\n"
                                       "};\n"
                                       "\n"
                                       "struct Bar {\n"
                                       "    Foo foo;\n"
                                       "    int x;\n"
                                       "    void f();\n"
                                       "};\n"
                                       "\n"
                                       "void Bar::f()\n"
                                       "{\n"
                                       "    foo.x = x;\n"
                                       "}\n");
        const std::string expected("\n\n##file 0\n"
                                   "1: struct Foo {\n"
                                   "2: int x@1 ;\n"
                                   "3: } ;\n"
                                   "4:\n"
                                   "5: struct Bar {\n"
                                   "6: Foo foo@2 ;\n"
                                   "7: int x@3 ;\n"
                                   "8: void f ( ) ;\n"
                                   "9: } ;\n"
                                   "10:\n"
                                   "11: void Bar :: f ( )\n"
                                   "12: {\n"
                                   "13: foo@2 . x@4 = x@3 ;\n"
                                   "14: }\n");
        ASSERT_EQUALS(expected, actual);
    }

    void varid_operator()
    {
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
                                       "4: void operator= ( const Foo & ) ;\n"
                                       "5: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }
        {
            const std::string actual = tokenizeDebugListing(
                                           "struct Foo {\n"
                                           "    void * operator new [](int);\n"
                                           "};\n");
            const std::string expected("\n\n##file 0\n"
                                       "1: struct Foo {\n"
                                       "2: void * operatornew[] ( int ) ;\n"
                                       "3: } ;\n");

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_throw()    // ticket #1723
    {
        const std::string actual = tokenizeDebugListing(
                                       "UserDefinedException* pe = new UserDefinedException();\n"
                                       "throw pe;\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: UserDefinedException * pe@1 ; pe@1 = new UserDefinedException ( ) ;\n"
                                   "2: throw pe@1 ;\n");

        ASSERT_EQUALS(expected, actual);
    }

    void varid_unknown_macro()
    {
        // #2638 - unknown macro
        const char code[] = "void f() {\n"
                            "    int a[10];\n"
                            "    AAA\n"
                            "    a[0] = 0;\n"
                            "}";
        const char expected[] = "\n\n##file 0\n"
                                "1: void f ( ) {\n"
                                "2: int a@1 [ 10 ] ;\n"
                                "3: AAA\n"
                                "4: a@1 [ 0 ] = 0 ;\n"
                                "5: }\n";
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
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

        const std::string wanted("\n\n##file 0\n"
                                 "1: class A\n"
                                 "2: {\n"
                                 "3: public:\n"
                                 "4: static char buf@1 [ 20 ] ;\n"
                                 "5: } ;\n"
                                 "6: char A :: buf@1 [ 20 ] ;\n"
                                 "7: int main ( )\n"
                                 "8: {\n"
                                 "9: char buf@2 [ 2 ] ;\n"
                                 "10: A :: buf@1 [ 10 ] = 0 ;\n"
                                 "11: }\n");

        const char current[] =  "\n\n##file 0\n1: class A\n2: {\n3: public:\n4: static char buf@1 [ 20 ] ;\n5: } ;\n6: char A :: buf [ 20 ] ;\n7: int main ( )\n8: {\n9: char buf@2 [ 2 ] ;\n10: A :: buf [ 10 ] = 0 ;\n11: }\n";
        TODO_ASSERT_EQUALS(wanted, current, actual);
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

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass8()
    {
        const std::string code("class Fred {\n"
                               "public:\n"
                               "    void foo(int d) {\n"
                               "        int i = bar(x * d);\n"
                               "    }\n"
                               "    int x;\n"
                               "}\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: public:\n"
                                   "3: void foo ( int d@1 ) {\n"
                                   "4: int i@2 ; i@2 = bar ( x@3 * d@1 ) ;\n"
                                   "5: }\n"
                                   "6: int x@3 ;\n"
                                   "7: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varidclass9()
    {
        const std::string code("typedef char Str[10];"
                               "class A {\n"
                               "public:\n"
                               "    void f(Str &cl);\n"
                               "    void g(Str cl);\n"
                               "}\n"
                               "void Fred::f(Str &cl) {\n"
                               "    sizeof(cl);\n"
                               "}");

        const std::string expected("\n\n"
                                   "##file 0\n"
                                   "1: ; class A {\n"
                                   "2: public:\n"
                                   "3: void f ( char ( & cl ) [ 10 ] ) ;\n"
                                   "4: void g ( char cl@1 [ 10 ] ) ;\n"
                                   "5: }\n"
                                   "6: void Fred :: f ( char ( & cl ) [ 10 ] ) {\n"
                                   "7: sizeof ( cl ) ;\n"
                                   "8: }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varidclass10()
    {
        const std::string code("class A {\n"
                               "    void f() {\n"
                               "        a = 3;\n"
                               "    }\n"
                               "    int a;\n"
                               "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class A {\n"
                                   "2: void f ( ) {\n"
                                   "3: a@1 = 3 ;\n"
                                   "4: }\n"
                                   "5: int a@1 ;\n"
                                   "6: } ;\n");
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varidclass11()
    {
        const std::string code("class Fred {\n"
                               "    int a;\n"
                               "    void f();\n"
                               "};\n"
                               "class Wilma {\n"
                               "    int a;\n"
                               "    void f();\n"
                               "};\n"
                               "void Fred::f() { a = 0; }\n"
                               "void Wilma::f() { a = 0; }\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: int a@1 ;\n"
                                   "3: void f ( ) ;\n"
                                   "4: } ;\n"
                                   "5: class Wilma {\n"
                                   "6: int a@2 ;\n"
                                   "7: void f ( ) ;\n"
                                   "8: } ;\n"
                                   "9: void Fred :: f ( ) { a@1 = 0 ; }\n"
                                   "10: void Wilma :: f ( ) { a@2 = 0 ; }\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varidclass12()
    {
        const std::string code("class Fred {\n"
                               "    int a;\n"
                               "    void f() { Fred::a = 0; }\n"
                               "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: int a@1 ;\n"
                                   "3: void f ( ) { Fred :: a@1 = 0 ; }\n"
                                   "4: } ;\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void varidclass13()
    {
        const std::string code("class Fred {\n"
                               "    int a;\n"
                               "    void f() { Foo::Fred::a = 0; }\n"
                               "};\n");

        const std::string expected("\n\n##file 0\n"
                                   "1: class Fred {\n"
                                   "2: int a@1 ;\n"
                                   "3: void f ( ) { Foo :: Fred :: a = 0 ; }\n"
                                   "4: } ;\n");

        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void file1()
    {
        const char code[] = "a1\n"
                            "#file \"b\"\n"
                            "b1\n"
                            "b2\n"
                            "#endfile\n"
                            "a3\n";

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "a.cpp");

        ASSERT_EQUALS("[c:\\a.h:1]", tokenizer.fileLine(tokenizer.tokens()));
    }




    void doublesharp()
    {
        const char code[] = "TEST(var,val) var##_##val = val\n";

        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "");

        // Stringify the tokens..
        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << tok->str() << " ";

        ASSERT_EQUALS("TEST ( var , val ) var_val = val ", ostr.str());
    }

    void macrodoublesharp()
    {
        const char code[] = "DBG(fmt,args...) printf(fmt, ## args)\n";

        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
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
    void removeParentheses1()
    {
        const char code[] = "void foo()"
                            "{"
                            "    free(((void*)p));"
                            "}";

        ASSERT_EQUALS("void foo ( ) { free ( p ) ; }", tokenizeAndStringify(code, true));
    }

    void removeParentheses2()
    {
        const char code[] = "void foo()"
                            "{"
                            "    if (__builtin_expect((s == NULL), 0))"
                            "        return;"
                            "}";

        ASSERT_EQUALS("void foo ( ) { if ( ! s ) { return ; } }", tokenizeAndStringify(code));
    }

    void removeParentheses3()
    {
        {
            const char code[] = "void foo()"
                                "{"
                                "    if (( true )==(true)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { { } }", tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void foo()"
                                "{"
                                "    if (( 2 )==(2)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { { } }", tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void foo()"
                                "{"
                                "    if( g(10)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { if ( g ( 10 ) ) { } }", tokenizeAndStringify(code));
        }
    }

    // Simplify "( function (..))" into "function (..)"
    void removeParentheses4()
    {
        const char code[] = "void foo()"
                            "{"
                            "    (free(p));"
                            "}";
        ASSERT_EQUALS("void foo ( ) { free ( p ) ; }", tokenizeAndStringify(code));
    }

    void removeParentheses5()
    {
        // Simplify "( delete x )" into "delete x"
        {
            const char code[] = "void foo()"
                                "{"
                                "    (delete p);"
                                "}";
            ASSERT_EQUALS("void foo ( ) { delete p ; }", tokenizeAndStringify(code));
        }

        // Simplify "( delete [] x )" into "delete [] x"
        {
            const char code[] = "void foo()"
                                "{"
                                "    (delete [] p);"
                                "}";
            ASSERT_EQUALS("void foo ( ) { delete [ ] p ; }", tokenizeAndStringify(code));
        }
    }

    // "!(abc.a)" => "!abc.a"
    void removeParentheses6()
    {
        const char code[] = "(!(abc.a))";
        ASSERT_EQUALS("( ! abc . a )", tokenizeAndStringify(code));
    }

    void removeParentheses7()
    {
        const char code[] = ";char *p; (delete(p), (p)=0);";
        ASSERT_EQUALS("; char * p ; delete p ; p = 0 ;", tokenizeAndStringify(code,true));
    }

    void removeParentheses8()
    {
        const char code[] = "struct foo {\n"
                            "    void operator delete(void *obj, size_t sz);\n"
                            "}\n";
        const std::string actual(tokenizeAndStringify(code, false, true, Settings::Win32));

        const char expected[] = "struct foo {\n"
                                "void operatordelete ( void * obj , unsigned long sz ) ;\n"
                                "}";

        ASSERT_EQUALS(expected, actual);
    }

    void removeParentheses9()
    {
        ASSERT_EQUALS("void delete ( double num ) ;", tokenizeAndStringify("void delete(double num);", false));
    }

    void removeParentheses10()
    {
        ASSERT_EQUALS("p = buf + 8 ;", tokenizeAndStringify("p = (buf + 8);", false));
    }

    void removeParentheses11()
    {
        // #2502
        ASSERT_EQUALS("{ } x ( ) ;", tokenizeAndStringify("{}(x());", false));
    }

    void removeParentheses12()
    {
        // #2760
        ASSERT_EQUALS(", x = 0 ;", tokenizeAndStringify(",(x)=0;", false));
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();
        ASSERT_EQUALS(" void f ( ) { ; { ; } } void g ( ) { ; }", ostr.str());
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

        errout.str("");

        Settings settings;

        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
            ostr << " " << tok->str();

        std::ostringstream oss;
        oss << " void f ( Foo & foo , Foo * foo2 ) { ; foo . a = 90 ; foo2 . a = 45 ; }";
        ASSERT_EQUALS(oss.str(), ostr.str());
    }

    void simplify_constants3()
    {
        const char code[] =
            "static const char str[] = \"abcd\";\n"
            "static const unsigned int SZ = sizeof(str);\n"
            "void f() {\n"
            "a = SZ;\n"
            "}\n";
        const char expected[] =
            "static const char str [ 5 ] = \"abcd\" ;\n\nvoid f ( ) {\na = 5 ;\n}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));
    }

    void simplify_null()
    {
        const char code[] =
            "int * p = NULL;\n"
            "int * q = __null;\n";
        const char expected[] =
            "int * p ; p = 0 ;\nint * q ; q = 0 ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));
    }

    void simplifyMulAnd()
    {
        // (error) Resource leak
        ASSERT_EQUALS(
            "void f ( ) { int f ; f = open ( ) ; }",
            tokenizeAndStringify(
                "void f() {int f; *&f=open(); }"
            )
        );
        ASSERT_EQUALS(
            "void f ( ) { int f ; f = open ( ) ; }",
            tokenizeAndStringify(
                "void f() {int f; *(&f)=open(); }"
            )
        );
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
        const char res1[]  = "void * p ; p = 0 ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));

        const char code2[] = "const void *p = NULL;";
        const char res2[]  = "const void * p ; p = 0 ;";
        ASSERT_EQUALS(res2, tokenizeAndStringify(code2));

        const char code3[] = "void * const p = NULL;";
        const char res3[]  = "void * const p ; p = 0 ;";
        ASSERT_EQUALS(res3, tokenizeAndStringify(code3));

        const char code4[] = "const void * const p = NULL;";
        const char res4[]  = "const void * const p ; p = 0 ;";
        ASSERT_EQUALS(res4, tokenizeAndStringify(code4));
    }

    void vardecl_stl_1()
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

    void vardecl_stl_2()
    {
        const char code1[] = "{ std::string x = \"abc\"; }";
        ASSERT_EQUALS("{ std :: string x ; x = \"abc\" ; }", tokenizeAndStringify(code1));

        const char code2[] = "{ std::vector<int> x = y; }";
        ASSERT_EQUALS("{ std :: vector < int > x ; x = y ; }", tokenizeAndStringify(code2));
    }

    void vardecl_template()
    {
        // ticket #1046
        const char code1[] = "b<(1<<24),10,24> u, v;";
        const char res1[]  = "b < ( 1 << 24 ) , 10 , 24 > u ; b < ( 1 << 24 ) , 10 , 24 > v ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));
    }

    void vardecl_union()
    {
        // ticket #1976
        const char code1[] = "class Fred { public: union { int a ; int b ; } ; } ;";
        ASSERT_EQUALS(code1, tokenizeAndStringify(code1));

        // ticket #2039
        const char code2[] = "void f() {\n"
                             "     union {\n"
                             "         int x;\n"
                             "         long y;\n"
                             "     };\n"
                             "}";
        ASSERT_EQUALS("void f ( ) {\n\nint x ;\nlong & y = x ;\n\n}", tokenizeAndStringify(code2));
    }

    void vardecl_par()
    {
        // ticket #2743 - set links if variable type contains parentheses
        const char code[] = "Fred<int(*)()> fred1=a, fred2=b;";

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp", "", false);
        ASSERT_EQUALS(true, tokenizer.validate());
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
        const char code[] = "void f() {\n"
                            "    for (int c = 0; c < 0; ++c) {}\n"
                            "    int t;\n"
                            "    D(3 > t, \"T\");\n"
                            "}";
        const char res[] = "void f ( ) {\n"
                           "for ( int c = 0 ; c < 0 ; ++ c ) { }\n"
                           "int t ;\n"
                           "D ( 3 > t , \"T\" ) ;\n"
                           "}";

        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl8()
    {
        // ticket #696
        const char code[] = "char a[10]={'\\0'}, b[10]={'\\0'};";
        const char res[]  = "char a [ 10 ] = { 0 } ; char b [ 10 ] = { 0 } ;";

        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl9()
    {
        const char code[] = "char a[2] = {'A', '\\0'}, b[2] = {'B', '\\0'};";
        const char res[]  = "char a [ 2 ] = { 'A' , 0 } ; char b [ 2 ] = { 'B' , 0 } ;";

        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl10()
    {
        // ticket #732
        const char code[] = "char a [ 2 ] = { '-' } ; memset ( a , '-' , sizeof ( a ) ) ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void vardecl11()
    {
        // ticket #1684
        const char code[] = "char a[5][8], b[5][8];";
        ASSERT_EQUALS("char a [ 5 ] [ 8 ] ; char b [ 5 ] [ 8 ] ;", tokenizeAndStringify(code));
    }

    void vardecl12()
    {
        const char code[] = "struct A { public: B a, b, c, d; };";
        ASSERT_EQUALS("struct A { public: B a ; B b ; B c ; B d ; } ;", tokenizeAndStringify(code));
    }

    void vardecl13()
    {
        const char code[] = "void f() {\n"
                            "    int a = (x < y) ? 1 : 0;\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\nint a ; a = ( x < y ) ? 1 : 0 ;\n}", tokenizeAndStringify(code));
    }

    void vardecl14()
    {
        const char code[] = "::std::tr1::shared_ptr<int> pNum1, pNum2;\n";
        ASSERT_EQUALS(":: std :: tr1 :: shared_ptr < int > pNum1 ; :: std :: tr1 :: shared_ptr < int > pNum2 ;", tokenizeAndStringify(code));
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
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(true, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("", errout.str());
        }

        {
            errout.str("");
            const char code[] = "void f() {{}";
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:1]: (error) Invalid number of character ({) when these macros are defined: ''.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "void f()) {}";
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:1]: (error) Invalid number of character (() when these macros are defined: ''.\n", errout.str());
        }

        {
            errout.str("");
            const char code[] = "namespace extract{\nB(weighted_moment)\n}\nusing extract::weighted_moment;\n";
            Settings settings;
            Tokenizer tokenizer(&settings, this);
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
            Settings settings;
            Tokenizer tokenizer(&settings, this);
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
            Settings settings;
            Tokenizer tokenizer(&settings, this);
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
            Settings settings;
            Tokenizer tokenizer(&settings, this);
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
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            std::istringstream istr(code);
            ASSERT_EQUALS(false, tokenizer.tokenize(istr, "test.cpp"));
            ASSERT_EQUALS("[test.cpp:2]: (error) Invalid number of character (() when these macros are defined: ''.\n", errout.str());
        }
    }


    void syntax_error_templates_1()
    {
        // ok code.. using ">" for a comparison
        {
            errout.str("");
            std::istringstream istr("x<y>z> xyz;\n");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            tokenizer.tokenize(istr, "test.cpp");
            ASSERT_EQUALS("", errout.str());
        }

        // ok code..
        {
            errout.str("");
            std::istringstream istr("template<class T> operator<(T a, T b) { }\n");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            tokenizer.tokenize(istr, "test.cpp");
            ASSERT_EQUALS("", errout.str());
        }

        // ok code (ticket #1984)..
        {
            errout.str("");
            std::istringstream istr("void f(a) int a;\n"
                                    "{ ;x<y; }");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            tokenizer.tokenize(istr, "test.cpp");
            ASSERT_EQUALS("", errout.str());
        }

        // ok code (ticket #1985)..
        {
            errout.str("");
            std::istringstream istr("void f()\n"
                                    "try { ;x<y; }");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            tokenizer.tokenize(istr, "test.cpp");
            ASSERT_EQUALS("", errout.str());
        }

        // bad code.. missing ">"
        {
            errout.str("");
            std::istringstream istr("x<y<int> xyz;\n");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            tokenizer.tokenize(istr, "test.cpp");
            ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
        }

        // bad code
        {
            errout.str("");
            std::istringstream istr("typedef\n"
                                    "    typename boost::mpl::if_c<\n"
                                    "          _visitableIndex < boost::mpl::size< typename _Visitables::ConcreteVisitables >::value\n"
                                    "          , ConcreteVisitable\n"
                                    "          , Dummy< _visitableIndex >\n"
                                    "    >::type ConcreteVisitableOrDummy;\n");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
            tokenizer.tokenize(istr, "test.cpp");
            ASSERT_EQUALS("[test.cpp:2]: (error) syntax error\n", errout.str());
        }
    }

    void syntax_error_templates_2()
    {
        std::istringstream istr("template<>\n");
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        tokenizer.tokenize(istr, "test.cpp");   // shouldn't segfault
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
            const char code1[] = "void f() { for (signed i=0; i<10; i++) {} }";
            const char code2[] = "void f ( ) { for ( signed int i = 0 ; i < 10 ; i ++ ) { } }";
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
            const char code1[] = "void f() { for (unsigned i=0; i<10; i++) {} }";
            const char code2[] = "void f ( ) { for ( unsigned int i = 0 ; i < 10 ; i ++ ) { } }";
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

    void createLinks()
    {
        {
            const char code[] = "class A{\n"
                                " void f() {}\n"
                                "};";
            errout.str("");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
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
            errout.str("");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
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
            errout.str("");
            Settings settings;
            Tokenizer tokenizer(&settings, this);
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

    void removeExceptionSpecification3()
    {
        const char code[] = "namespace A {\n"
                            "    struct B {\n"
                            "        B() throw ()\n"
                            "        { }\n"
                            "    };\n"
                            "};\n";

        const char expected[] = "namespace A {\n"
                                "struct B {\n"
                                "B ( )\n"
                                "{ }\n"
                                "} ;\n"
                                "} ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }


    void gt()
    {
        ASSERT_EQUALS("( i < 10 )", tokenizeAndStringify("(10>i)"));
        ASSERT_EQUALS("; i < 10 ;", tokenizeAndStringify(";10>i;"));
        ASSERT_EQUALS("void > ( ) ; void > ( )",
                      tokenizeAndStringify("void>(); void>()"));
    }


    void simplifyString()
    {
        errout.str("");
        Settings settings;
        Tokenizer tokenizer(&settings, this);
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
        errout.str("");
        Settings settings;
        Tokenizer tokenizer(&settings, this);
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

    void functionpointer1()
    {
        ASSERT_EQUALS(" void* f;", simplifyFunctionPointers("void (*f)();"));
        ASSERT_EQUALS(" void** f;", simplifyFunctionPointers("void *(*f)();"));
        ASSERT_EQUALS(" unsigned int* f;", simplifyFunctionPointers("unsigned int (*f)();"));
        ASSERT_EQUALS(" unsigned int** f;", simplifyFunctionPointers("unsigned int * (*f)();"));
    }

    void functionpointer2()
    {
        const char code[] = "typedef void (* PF)();"
                            "void f1 ( ) { }"
                            "PF pf = &f1;"
                            "PF pfs[] = { &f1, &f1 };";
        const char expected[] = "; "
                                "void f1(){} "
                                "void* pf; pf=& f1; "
                                "void* pfs[]={& f1,& f1};";
        ASSERT_EQUALS(expected, simplifyFunctionPointers(code));
    }

    void functionpointer3()
    {
        // Related with ticket #2873
        const char code[] = "void f() {\n"
                            "(void)(xy(*p)(0);)"
                            "\n}";
        const char expected[] = " void f(){"
                                "( void)( xy(* p)(0);)"
                                "}";
        ASSERT_EQUALS(expected, simplifyFunctionPointers(code));
    }

    void removeRedundantAssignment()
    {
        ASSERT_EQUALS("void f ( ) { ; int * q ; }", tokenizeAndStringify("void f() { int *p, *q; p = q; }", true));
        ASSERT_EQUALS("void f ( ) { ; int * q ; }", tokenizeAndStringify("void f() { int *p = 0, *q; p = q; }", true));
        ASSERT_EQUALS("int f ( int * x ) { return * x ; }", tokenizeAndStringify("int f(int *x) { return *x; }", true));
    }

    void removedeclspec()
    {
        ASSERT_EQUALS("a b", tokenizeAndStringify("a __declspec ( dllexport ) b"));
        ASSERT_EQUALS("int a ;", tokenizeAndStringify("__declspec(thread) __declspec(align(32)) int a;"));
        ASSERT_EQUALS("int i ;", tokenizeAndStringify("__declspec(allocate(\"mycode\")) int i;"));
        ASSERT_EQUALS("struct IUnknown ;", tokenizeAndStringify("struct __declspec(uuid(\"00000000-0000-0000-c000-000000000046\")) IUnknown;"));
        ASSERT_EQUALS("int x [ ] ;", tokenizeAndStringify("__declspec(property(get=GetX, put=PutX)) int x[];"));
    }

    void removeattribute()
    {
        ASSERT_EQUALS("short array [ 3 ] ;", tokenizeAndStringify("short array[3] __attribute__ ((aligned));"));
        ASSERT_EQUALS("int x [ 2 ] ;", tokenizeAndStringify("int x[2] __attribute__ ((packed));"));
        ASSERT_EQUALS("int vecint ;", tokenizeAndStringify("int __attribute__((mode(SI))) __attribute__((vector_size (16))) vecint;"));
    }

    void cpp0xtemplate1()
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

    void cpp0xtemplate2()
    {
        // tokenize ">>" into "> >"
        const char *code = "list<list<int>> ints;\n";
        TODO_ASSERT_EQUALS("list < list < int > > ints ;",
                           "list < list < int >> ints ;", tokenizeAndStringify(code));
    }

    void cpp0xtemplate3()
    {
        // #2549
        const char *code = "template<class T, T t = (T)0>\n"
                           "struct S\n"
                           "{};\n"
                           "S<int> s;\n";
        TODO_ASSERT_EQUALS(";\n\n\nS < int , ( int ) 0 > s ;",   // wanted result
                           ";\n\n\nS < int , ( T ) 0 > s ;",     // current result
                           tokenizeAndStringify(code));
    }

    void cpp0xdefault()
    {
        {
            const char *code = "struct foo {"
                               "    foo() = default;"
                               "}";
            ASSERT_EQUALS("struct foo { }", tokenizeAndStringify(code));
        }

        {
            const char *code = "struct A {"
                               "  void operator delete (void *) = delete;"
                               "  void operator delete[] (void *) = delete;"
                               "}";
            ASSERT_EQUALS("struct A { }", tokenizeAndStringify(code));
        }

        {
            const char *code = "struct A {"
                               "  void operator = (void *) = delete;"
                               "}";
            ASSERT_EQUALS("struct A { }", tokenizeAndStringify(code));
        }

        {
            const char *code = "struct foo {"
                               "    foo();"
                               "}"
                               "foo::foo() = delete;";
            TODO_ASSERT_EQUALS("struct foo { }",
                               "struct foo { foo ( ) ; } foo :: foo ( ) = delete ;", tokenizeAndStringify(code));
        }
    }

    std::string arraySize_(const std::string &code)
    {
        errout.str("");
        Settings settings;
        // tokenize..
        Tokenizer tokenizer(&settings, this);
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
        ASSERT_EQUALS("; int a[]={ ABC,2,3};", arraySize_(";int a[]={ABC,2,3};"));
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
        settings.addEnabled("style");
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
            ASSERT_EQUALS("int i ; int p ; p = i ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
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
            ASSERT_EQUALS("class A { } ; int foo ( A ) ;", tokenizeAndStringify(code));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class A { } ; A a; int foo(a);";
            ASSERT_EQUALS("class A { } ; A a ; int foo ; foo = a ;", tokenizeAndStringify(code, false));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int x(f());";
            ASSERT_EQUALS("int x ; x = f ( ) ;", tokenizeAndStringify(code, false));
            checkSimplifyInitVar(code);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void bitfields1()
    {
        const char code1[] = "struct A { bool x : 1; };";
        ASSERT_EQUALS("struct A { bool x ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { char x : 3; };";
        ASSERT_EQUALS("struct A { char x ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { short x : 3; };";
        ASSERT_EQUALS("struct A { short x ; } ;", tokenizeAndStringify(code3,false));

        const char code4[] = "struct A { int x : 3; };";
        ASSERT_EQUALS("struct A { int x ; } ;", tokenizeAndStringify(code4,false));

        const char code5[] = "struct A { long x : 3; };";
        ASSERT_EQUALS("struct A { long x ; } ;", tokenizeAndStringify(code5,false));

        const char code6[] = "struct A { __int8 x : 3; };";
        ASSERT_EQUALS("struct A { char x ; } ;", tokenizeAndStringify(code6,false));

        const char code7[] = "struct A { __int16 x : 3; };";
        ASSERT_EQUALS("struct A { short x ; } ;", tokenizeAndStringify(code7,false));

        const char code8[] = "struct A { __int32 x : 3; };";
        ASSERT_EQUALS("struct A { int x ; } ;", tokenizeAndStringify(code8,false));

        const char code9[] = "struct A { __int64 x : 3; };";
        ASSERT_EQUALS("struct A { long long x ; } ;", tokenizeAndStringify(code9,false));

        const char code10[] = "struct A { unsigned char x : 3; };";
        ASSERT_EQUALS("struct A { unsigned char x ; } ;", tokenizeAndStringify(code10,false));

        const char code11[] = "struct A { unsigned short x : 3; };";
        ASSERT_EQUALS("struct A { unsigned short x ; } ;", tokenizeAndStringify(code11,false));

        const char code12[] = "struct A { unsigned int x : 3; };";
        ASSERT_EQUALS("struct A { unsigned int x ; } ;", tokenizeAndStringify(code12,false));

        const char code13[] = "struct A { unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { unsigned long x ; } ;", tokenizeAndStringify(code13,false));

        const char code14[] = "struct A { unsigned __int8 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned char x ; } ;", tokenizeAndStringify(code14,false));

        const char code15[] = "struct A { unsigned __int16 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned short x ; } ;", tokenizeAndStringify(code15,false));

        const char code16[] = "struct A { unsigned __int32 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned int x ; } ;", tokenizeAndStringify(code16,false));

        const char code17[] = "struct A { unsigned __int64 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned long long x ; } ;", tokenizeAndStringify(code17,false));

        const char code18[] = "struct A { signed char x : 3; };";
        ASSERT_EQUALS("struct A { signed char x ; } ;", tokenizeAndStringify(code18,false));

        const char code19[] = "struct A { signed short x : 3; };";
        ASSERT_EQUALS("struct A { signed short x ; } ;", tokenizeAndStringify(code19,false));

        const char code20[] = "struct A { signed int x : 3; };";
        ASSERT_EQUALS("struct A { signed int x ; } ;", tokenizeAndStringify(code20,false));

        const char code21[] = "struct A { signed long x : 3; };";
        ASSERT_EQUALS("struct A { signed long x ; } ;", tokenizeAndStringify(code21,false));

        const char code22[] = "struct A { signed __int8 x : 3; };";
        ASSERT_EQUALS("struct A { signed char x ; } ;", tokenizeAndStringify(code22,false));

        const char code23[] = "struct A { signed __int16 x : 3; };";
        ASSERT_EQUALS("struct A { signed short x ; } ;", tokenizeAndStringify(code23,false));

        const char code24[] = "struct A { signed __int32 x : 3; };";
        ASSERT_EQUALS("struct A { signed int x ; } ;", tokenizeAndStringify(code24,false));

        const char code25[] = "struct A { signed __int64 x : 3; };";
        ASSERT_EQUALS("struct A { signed long long x ; } ;", tokenizeAndStringify(code25,false));
    }

    void bitfields2()
    {
        const char code1[] = "struct A { public: int x : 3; };";
        ASSERT_EQUALS("struct A { public: int x ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { public: unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { public: unsigned long x ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { protected: int x : 3; };";
        ASSERT_EQUALS("struct A { protected: int x ; } ;", tokenizeAndStringify(code3,false));

        const char code4[] = "struct A { protected: unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { protected: unsigned long x ; } ;", tokenizeAndStringify(code4,false));

        const char code5[] = "struct A { private: int x : 3; };";
        ASSERT_EQUALS("struct A { private: int x ; } ;", tokenizeAndStringify(code5,false));

        const char code6[] = "struct A { private: unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { private: unsigned long x ; } ;", tokenizeAndStringify(code6,false));
    }

    void bitfields3()
    {
        const char code1[] = "struct A { const int x : 3; };";
        ASSERT_EQUALS("struct A { const int x ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { const unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { const unsigned long x ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { public: const int x : 3; };";
        ASSERT_EQUALS("struct A { public: const int x ; } ;", tokenizeAndStringify(code3,false));

        const char code4[] = "struct A { public: const unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { public: const unsigned long x ; } ;", tokenizeAndStringify(code4,false));
    }

    void bitfields4() // ticket #1956
    {
        const char code1[] = "struct A { CHAR x : 3; };";
        ASSERT_EQUALS("struct A { CHAR x ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { UCHAR x : 3; };";
        ASSERT_EQUALS("struct A { UCHAR x ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { BYTE x : 3; };";
        ASSERT_EQUALS("struct A { BYTE x ; } ;", tokenizeAndStringify(code3,false));

        const char code4[] = "struct A { WORD x : 3; };";
        ASSERT_EQUALS("struct A { WORD x ; } ;", tokenizeAndStringify(code4,false));

        const char code5[] = "struct A { DWORD x : 3; };";
        ASSERT_EQUALS("struct A { DWORD x ; } ;", tokenizeAndStringify(code5,false));

        const char code6[] = "struct A { LONG x : 3; };";
        ASSERT_EQUALS("struct A { LONG x ; } ;", tokenizeAndStringify(code6,false));

        const char code7[] = "struct A { UINT8 x : 3; };";
        ASSERT_EQUALS("struct A { UINT8 x ; } ;", tokenizeAndStringify(code7,false));

        const char code8[] = "struct A { UINT16 x : 3; };";
        ASSERT_EQUALS("struct A { UINT16 x ; } ;", tokenizeAndStringify(code8,false));

        const char code9[] = "struct A { UINT32 x : 3; };";
        ASSERT_EQUALS("struct A { UINT32 x ; } ;", tokenizeAndStringify(code9,false));

        const char code10[] = "struct A { UINT64 x : 3; };";
        ASSERT_EQUALS("struct A { UINT64 x ; } ;", tokenizeAndStringify(code10,false));
    }

    void bitfields5() // ticket #1956
    {
        const char code1[] = "struct RGB { unsigned int r : 3, g : 3, b : 2; };";
        ASSERT_EQUALS("struct RGB { unsigned int r ; unsigned int g ; unsigned int b ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { int a : 3; int : 3; int c : 3; };";
        ASSERT_EQUALS("struct A { int a ; int c ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { virtual void f() {} int f1 : 1; };";
        ASSERT_EQUALS("struct A { virtual void f ( ) { } int f1 ; } ;", tokenizeAndStringify(code3,false));
    }

    void bitfields6() // ticket #2595
    {
        const char code1[] = "struct A { bool b : true; };";
        ASSERT_EQUALS("struct A { bool b ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { bool b : true, c : true; };";
        ASSERT_EQUALS("struct A { bool b ; bool c ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { bool : true; };";
        ASSERT_EQUALS("struct A { } ;", tokenizeAndStringify(code3,false));

        const char code4[] = "void f(int a) { switch (a) { case b: break; } }";
        ASSERT_EQUALS("void f ( int a ) { switch ( a ) { case b : ; break ; } }", tokenizeAndStringify(code4,true));

        const char code5[] = "void f(int a) { switch (a) { default: break; } }";
        ASSERT_EQUALS("void f ( int a ) { switch ( a ) { default : ; break ; } }", tokenizeAndStringify(code5,true));
    }

    void bitfields7() // ticket #1987
    {
        const char code[] = "typedef struct Descriptor {"
                            "    unsigned element_size: 8* sizeof( unsigned );"
                            "} Descriptor;";
        const char expected[] = "struct Descriptor { "
                                "unsigned int element_size ; "
                                "} ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code,false));
        ASSERT_EQUALS("", errout.str());
    }

    void bitfields8()
    {
        const char code[] = "struct A;"
                            "class B : virtual public C"
                            "{"
                            "    int f();"
                            "};";
        const char expected[] = "struct A ; "
                                "class B : virtual public C "
                                "{ "
                                "int f ( ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code,false));
        ASSERT_EQUALS("", errout.str());
    }

    void bitfields9() // ticket #2706
    {
        const char code[] = "void f() {\n"
                            "    goto half;\n"
                            "half:\n"
                            "    {\n"
                            "        ;\n"
                            "    }\n"
                            "};";
        tokenizeAndStringify(code,false);
        ASSERT_EQUALS("", errout.str());
    }

    void bitfields10() // ticket #2737
    {
        const char code[] = "{}"
                            "MACRO "
                            "default: { }"
                            ";";
        ASSERT_EQUALS("{ } MACRO default : { } ;", tokenizeAndStringify(code,false));
    }

    void bitfields11() // ticket #2845 (segmentation fault)
    {
        const char code[] = "#if b&&a\n"
                            "#ifdef y z:\n";
        tokenizeAndStringify(code,false);
        ASSERT_EQUALS("", errout.str());
    }

    void microsoftMFC()
    {
        const char code1[] = "class MyDialog : public CDialog { DECLARE_MESSAGE_MAP() private: CString text; };";
        ASSERT_EQUALS("class MyDialog : public CDialog { private: CString text ; } ;", tokenizeAndStringify(code1,false,true,Settings::Win32));

        const char code2[] = "class MyDialog : public CDialog { DECLARE_DYNAMIC(MyDialog) private: CString text; };";
        ASSERT_EQUALS("class MyDialog : public CDialog { private: CString text ; } ;", tokenizeAndStringify(code2,false,true,Settings::Win32));

        const char code3[] = "class MyDialog : public CDialog { DECLARE_DYNCREATE(MyDialog) private: CString text; };";
        ASSERT_EQUALS("class MyDialog : public CDialog { private: CString text ; } ;", tokenizeAndStringify(code3,false,true,Settings::Win32));

        const char code4[] = "class MyDialog : public CDialog { DECLARE_DYNAMIC_CLASS(MyDialog) private: CString text; };";
        ASSERT_EQUALS("class MyDialog : public CDialog { private: CString text ; } ;", tokenizeAndStringify(code4,false,true,Settings::Win32));
    }

    void borland()
    {
        // __closure
        ASSERT_EQUALS("int * a ;",
                      tokenizeAndStringify("int (__closure *a)();", false));

        // __property
        ASSERT_EQUALS("class Fred { ; __property ; } ;",
                      tokenizeAndStringify("class Fred { __property int x = { } };", false));
    }

    void Qt()
    {
        const char code1[] = "class Counter : public QObject "
                             "{ "
                             "    Q_OBJECT "
                             "public: "
                             "    Counter() { m_value = 0; } "
                             "    int value() const { return m_value; } "
                             "public slots: "
                             "    void setValue(int value); "
                             "signals: "
                             "    void valueChanged(int newValue); "
                             "private: "
                             "    int m_value; "
                             "}; "
                             "void Counter::setValue(int value) "
                             "{ "
                             "    if (value != m_value) { "
                             "        m_value = value; "
                             "        emit valueChanged(value); "
                             "    } "
                             "}";

        const char result1 [] = "class Counter : public QObject "
                                "{ "
                                "public: "
                                "Counter ( ) { m_value = 0 ; } "
                                "int value ( ) const { return m_value ; } "
                                "public: "
                                "void setValue ( int value ) ; "
                                "protected: "
                                "void valueChanged ( int newValue ) ; "
                                "private: "
                                "int m_value ; "
                                "} ; "
                                "void Counter :: setValue ( int value ) "
                                "{ "
                                "if ( value != m_value ) { "
                                "m_value = value ; "
                                "valueChanged ( value ) ; "
                                "} "
                                "}";

        ASSERT_EQUALS(result1, tokenizeAndStringify(code1,false));

        const char code2[] = "class Counter : public QObject "
                             "{ "
                             "    Q_OBJECT "
                             "public: "
                             "    Counter() { m_value = 0; } "
                             "    int value() const { return m_value; } "
                             "public Q_SLOTS: "
                             "    void setValue(int value); "
                             "Q_SIGNALS: "
                             "    void valueChanged(int newValue); "
                             "private: "
                             "    int m_value; "
                             "};"
                             "void Counter::setValue(int value) "
                             "{ "
                             "    if (value != m_value) { "
                             "        m_value = value; "
                             "        emit valueChanged(value); "
                             "    } "
                             "}";

        const char result2 [] = "class Counter : public QObject "
                                "{ "
                                "public: "
                                "Counter ( ) { m_value = 0 ; } "
                                "int value ( ) const { return m_value ; } "
                                "public: "
                                "void setValue ( int value ) ; "
                                "protected: "
                                "void valueChanged ( int newValue ) ; "
                                "private: "
                                "int m_value ; "
                                "} ; "
                                "void Counter :: setValue ( int value ) "
                                "{ "
                                "if ( value != m_value ) { "
                                "m_value = value ; "
                                "valueChanged ( value ) ; "
                                "} "
                                "}";

        ASSERT_EQUALS(result2, tokenizeAndStringify(code2,false));

        const char code3[] = "class MyObject : public QObject {"
                             "    MyObject() {}"
                             "    ~MyObject() {}"
                             "    public slots:"
                             "    signals:"
                             "        void test() {}"
                             "};";
        const char result3 [] = "class MyObject : public QObject { "
                                "MyObject ( ) { } "
                                "~ MyObject ( ) { } "
                                "public: "
                                "protected: "
                                "void test ( ) { } "
                                "} ;";

        ASSERT_EQUALS(result3, tokenizeAndStringify(code3,false));
        ASSERT_EQUALS("", errout.str());
    }

    void sql()
    {
        // Oracle PRO*C extensions for inline SQL. Just replace the SQL with "asm()" to fix wrong error messages
        // ticket: #1959
        const char code1[] = "; EXEC SQL SELECT A FROM B;";
        ASSERT_EQUALS("; asm ( ) ;", tokenizeAndStringify(code1,false));

    }

    void simplifyLogicalOperators()
    {
        ASSERT_EQUALS("if ( a && b )", tokenizeAndStringify("if (a and b)"));
        ASSERT_EQUALS("if ( a || b )", tokenizeAndStringify("if (a or b)"));
        ASSERT_EQUALS("if ( a & b )", tokenizeAndStringify("if (a bitand b)"));
        ASSERT_EQUALS("if ( a | b )", tokenizeAndStringify("if (a bitor b)"));
        ASSERT_EQUALS("if ( a ^ b )", tokenizeAndStringify("if (a xor b)"));
        ASSERT_EQUALS("if ( ~ b )", tokenizeAndStringify("if (compl b)"));
        ASSERT_EQUALS("if ( ! b )", tokenizeAndStringify("if (not b)"));
        ASSERT_EQUALS("if ( a != b )", tokenizeAndStringify("if (a not_eq b)"));
    }

    void simplifyCalculations()
    {
        ASSERT_EQUALS("void foo ( char str [ ] ) { char x ; x = * str ; }",
                      tokenizeAndStringify("void foo ( char str [ ] ) { char x = 0 | ( * str ) ; }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (b | 0) { } }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (0 | b) { } }", true));
        ASSERT_EQUALS("void foo ( int b ) { int a ; a = b ; bar ( a ) ; }",
                      tokenizeAndStringify("void foo ( int b ) { int a = b | 0 ; bar ( a ) ; }", true));
        ASSERT_EQUALS("void foo ( int b ) { int a ; a = b ; bar ( a ) ; }",
                      tokenizeAndStringify("void foo ( int b ) { int a = 0 | b ; bar ( a ) ; }", true));

        // ticket #3093
        ASSERT_EQUALS("int f ( ) { ; return 15 ; }",
                      tokenizeAndStringify("int f() { int a = 10; int b = 5; return a + b; }", true));
        ASSERT_EQUALS("int f ( ) { return a ; }",
                      tokenizeAndStringify("int f() { return a * 1; }", true));
        ASSERT_EQUALS("int f ( int a ) { return 0 ; }",
                      tokenizeAndStringify("int f(int a) { return 0 * a; }", true));
        ASSERT_EQUALS("bool f ( int i ) { switch ( i ) { case 15 : ; return true ; } }",
                      tokenizeAndStringify("bool f(int i) { switch (i) { case 10 + 5: return true; } }", true));
    }

    void simplifyCompoundAssignment()
    {
        ASSERT_EQUALS("; x = x + y ;", tokenizeAndStringify("; x += y;"));
        ASSERT_EQUALS("; x = x - y ;", tokenizeAndStringify("; x -= y;"));
        ASSERT_EQUALS("; x = x * y ;", tokenizeAndStringify("; x *= y;"));
        ASSERT_EQUALS("; x = x / y ;", tokenizeAndStringify("; x /= y;"));
        ASSERT_EQUALS("; x = x % y ;", tokenizeAndStringify("; x %= y;"));
        ASSERT_EQUALS("; x = x & y ;", tokenizeAndStringify("; x &= y;"));
        ASSERT_EQUALS("; x = x | y ;", tokenizeAndStringify("; x |= y;"));
        ASSERT_EQUALS("; x = x ^ y ;", tokenizeAndStringify("; x ^= y;"));
        ASSERT_EQUALS("; x = x << y ;", tokenizeAndStringify("; x <<= y;"));
        ASSERT_EQUALS("; x = x >> y ;", tokenizeAndStringify("; x >>= y;"));

        ASSERT_EQUALS("{ x = x + y ; }", tokenizeAndStringify("{ x += y;}"));
        ASSERT_EQUALS("{ x = x - y ; }", tokenizeAndStringify("{ x -= y;}"));
        ASSERT_EQUALS("{ x = x * y ; }", tokenizeAndStringify("{ x *= y;}"));
        ASSERT_EQUALS("{ x = x / y ; }", tokenizeAndStringify("{ x /= y;}"));
        ASSERT_EQUALS("{ x = x % y ; }", tokenizeAndStringify("{ x %= y;}"));
        ASSERT_EQUALS("{ x = x & y ; }", tokenizeAndStringify("{ x &= y;}"));
        ASSERT_EQUALS("{ x = x | y ; }", tokenizeAndStringify("{ x |= y;}"));
        ASSERT_EQUALS("{ x = x ^ y ; }", tokenizeAndStringify("{ x ^= y;}"));
        ASSERT_EQUALS("{ x = x << y ; }", tokenizeAndStringify("{ x <<= y;}"));
        ASSERT_EQUALS("{ x = x >> y ; }", tokenizeAndStringify("{ x >>= y;}"));

        ASSERT_EQUALS("; * p = * p + y ;", tokenizeAndStringify("; *p += y;"));
        ASSERT_EQUALS("; ( * p ) = ( * p ) + y ;", tokenizeAndStringify("; (*p) += y;"));
        ASSERT_EQUALS("; * ( p [ 0 ] ) = * ( p [ 0 ] ) + y ;", tokenizeAndStringify("; *(p[0]) += y;"));

        ASSERT_EQUALS("case 0 : x = x + y ; break ;", tokenizeAndStringify("case 0: x += y; break;"));

        ASSERT_EQUALS("; x . y = x . y + 1 ;", tokenizeAndStringify("; x.y += 1;"));

        ASSERT_EQUALS("; x [ 0 ] = x [ 0 ] + 1 ;", tokenizeAndStringify("; x[0] += 1;"));
        ASSERT_EQUALS("; x [ y - 1 ] = x [ y - 1 ] + 1 ;", tokenizeAndStringify("; x[y-1] += 1;"));

        ASSERT_EQUALS(";", tokenizeAndStringify(";x += 0;"));
        ASSERT_EQUALS(";", tokenizeAndStringify(";x += '\\0';"));
        ASSERT_EQUALS(";", tokenizeAndStringify(";x -= 0;"));
        ASSERT_EQUALS(";", tokenizeAndStringify(";x |= 0;"));
        ASSERT_EQUALS(";", tokenizeAndStringify(";x *= 1;"));
        ASSERT_EQUALS(";", tokenizeAndStringify(";x /= 1;"));

        ASSERT_EQUALS("; a . x ( ) = a . x ( ) + 1 ;", tokenizeAndStringify("; a.x() += 1;"));
        ASSERT_EQUALS("; x ( 1 ) = x ( 1 ) + 1 ;", tokenizeAndStringify("; x(1) += 1;"));

        // #2368
        ASSERT_EQUALS("if ( false ) { } else { j = j - i ; }", tokenizeAndStringify("if (false) {} else { j -= i; }"));

        // #2714 - wrong simplification of "a += b?c:d;"
        ASSERT_EQUALS("; a = a + ( b ? c : d ) ;", tokenizeAndStringify("; a+=b?c:d;"));
        ASSERT_EQUALS("; a = a * ( b + 1 ) ;", tokenizeAndStringify("; a*=b+1;"));
    }

    void simplifyAssignmentInFunctionCall()
    {
        ASSERT_EQUALS("; x = g ( ) ; f ( x ) ;", tokenizeAndStringify(";f(x=g());"));
    }

    void cs()
    {
        ASSERT_EQUALS("; int * i ;", tokenizeAndStringify("; int [] i;"));
    }

    std::string javatest(const char javacode[])
    {
        errout.str("");
        Settings settings;
        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(javacode);
        tokenizer.tokenize(istr, "test.java");

        std::ostringstream ostr;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        {
            ostr << tok->str();
            if (tok->next())
                ostr << " ";
        }

        return ostr.str();
    }


    void java()
    {
        ASSERT_EQUALS("void f ( ) { }", javatest("void f() throws Exception { }"));
    }

    void simplifyOperatorName1()
    {
        // make sure C code doesn't get changed
        const char code[] = "void operator () {}"
                            "int main()"
                            "{"
                            "    operator();"
                            "}";

        const char result [] = "void operator ( ) { } "
                               "int main ( ) "
                               "{ "
                               "operator ( ) ; "
                               "}";

        ASSERT_EQUALS(result, tokenizeAndStringify(code,false));
    }

    void simplifyOperatorName2()
    {
        const char code[] = "class Fred"
                            "{"
                            "    Fred(const Fred & f) { operator = (f); }"
                            "    operator = ();"
                            "}";

        const char result [] = "class Fred "
                               "{ "
                               "Fred ( const Fred & f ) { operator= ( f ) ; } "
                               "operator= ( ) ; "
                               "}";

        ASSERT_EQUALS(result, tokenizeAndStringify(code,false));
    }

    void simplifyOperatorName3()
    {
        // #2615
        const char code[] = "void f() {"
                            "static_cast<ScToken*>(xResult.operator->())->GetMatrix();"
                            "}";
        const char result[] = "void f ( ) { static_cast < ScToken * > ( xResult . operator. ( ) ) . GetMatrix ( ) ; }";
        ASSERT_EQUALS(result, tokenizeAndStringify(code,false));
    }

    void simplifyOperatorName4()
    {
        const char code[] = "void operator==() { }";
        const char result[] = "void operator== ( ) { }";
        ASSERT_EQUALS(result, tokenizeAndStringify(code,false));
    }

    void simplifyOperatorName5()
    {
        const char code1[] = "std::istream & operator >> (std::istream & s, Fred &f);";
        const char result1[] = "std :: istream & operator>> ( std :: istream & s , Fred & f ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1,false));

        const char code2[] = "std::ostream & operator << (std::ostream & s, const Fred &f);";
        const char result2[] = "std :: ostream & operator<< ( std :: ostream & s , const Fred & f ) ;";
        ASSERT_EQUALS(result2, tokenizeAndStringify(code2,false));
    }

    void removeMacrosInGlobalScope()
    {
        // remove some unhandled macros in the global scope.
        ASSERT_EQUALS("void f ( ) { }", tokenizeAndStringify("void f() NOTHROW { }"));
        ASSERT_EQUALS("struct Foo { } ;", tokenizeAndStringify("struct __declspec(dllexport) Foo {};"));
    }

    void multipleAssignment()
    {
        ASSERT_EQUALS("a = b = 0 ;", tokenizeAndStringify("a=b=0;"));
    }

    void simplifyIfAddBraces() // ticket # 2739 (segmentation fault)
    {
        tokenizeAndStringify("if()x");
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());

        // ticket #2873
        {
            const char code[] = "void f() { "
                                "( { if(*p) (*p) = x(); } ) "
                                "}";
            ASSERT_EQUALS("void f ( ) { ( { if ( * p ) { ( * p ) = x ( ) ; } } ) }",
                          tokenizeAndStringify(code));
        }
    }

    void removeRedundantCodeAfterReturn()
    {
        ASSERT_EQUALS("void f ( ) { return ; }", tokenizeAndStringify("void f() { return; foo();}"));
        ASSERT_EQUALS("void f ( int n ) { if ( n ) { return ; } foo ( ) ; }",tokenizeAndStringify("void f(int n) { if (n) return; foo();}"));

        ASSERT_EQUALS("int f ( int n ) { switch ( n ) { case 0 : return 0 ; default : ; return n ; } return -1 ; }",
                      tokenizeAndStringify("int f(int n) { switch (n) {case 0: return 0; n*=2; default: return n; n*=6;} return -1; foo();}"));

        {
            const char code[] = "void f(){ "
                                "if (k>0) goto label; "
                                "return; "
                                "if (tnt) "
                                "   { "
                                "       { "
                                "           check(); "
                                "           k=0; "
                                "       } "
                                "       label: "
                                "       bar(); "
                                "   } "
                                "}";
            ASSERT_EQUALS("void f ( ) { if ( 0 < k ) { goto label ; } return ; { label : ; bar ( ) ; } }",simplifyKnownVariables(code));
        }

        {
            const char code[] = "int f() { "
                                "switch (x) { case 1: return 1; bar(); tack; { ticak(); return; } return; "
                                "case 2: return 2; { reere(); } tack(); "
                                "switch(y) { case 1: return 0; case 2: return 7; } "
                                "return 2; } return 3; }";
            ASSERT_EQUALS("int f ( ) { switch ( x ) { case 1 : return 1 ; case 2 : return 2 ; } return 3 ; }",simplifyKnownVariables(code));
        }

        {
            const char code[] = "int f() { "
                                "switch (x) { case 1: return 1; bar(); tack; { ticak(); return; } return; "
                                "case 2: switch(y) { case 1: return 0; bar2(); foo(); case 2: return 7; } "
                                "return 2; } return 3; }";
            ASSERT_EQUALS("int f ( ) { switch ( x ) { case 1 : return 1 ; case 2 : switch ( y ) { case 1 : return 0 ; case 2 : return 7 ; } return 2 ; } return 3 ; }",simplifyKnownVariables(code));
        }
    }

    void platformWin32()
    {
        const char code[] = "unsigned int sizeof_short = sizeof(short);"
                            "unsigned int sizeof_unsigned_short = sizeof(unsigned short);"
                            "unsigned int sizeof_int = sizeof(int);"
                            "unsigned int sizeof_unsigned_int = sizeof(unsigned int);"
                            "unsigned int sizeof_long = sizeof(long);"
                            "unsigned int sizeof_unsigned_long = sizeof(unsigned long);"
                            "unsigned int sizeof_long_long = sizeof(long long);"
                            "unsigned int sizeof_unsigned_long_long = sizeof(unsigned long long);"
                            "unsigned int sizeof_float = sizeof(float);"
                            "unsigned int sizeof_double = sizeof(double);"
                            "unsigned int sizeof_long_double = sizeof(long double);"
                            "unsigned int sizeof_bool = sizeof(bool);"
                            "unsigned int sizeof_pointer = sizeof(void *);"
                            "unsigned int sizeof_size_t = sizeof(size_t);"
                            "size_t a;"
                            "ssize_t b;"
                            "ptrdiff_t c;"
                            "intptr_t d;"
                            "uintptr_t e;"
                            "BOOL f;"
                            "BOOLEAN g;"
                            "BYTE h;"
                            "CHAR i;"
                            "DWORD j;"
                            "FLOAT k;"
                            "INT l;"
                            "INT32 m;"
                            "INT64 n;"
                            "LONG o;"
                            "SHORT p;"
                            "UCHAR q;"
                            "UINT r;"
                            "ULONG s;"
                            "USHORT t;"
                            "WORD u;"
                            "VOID *v;"
                            "LPBOOL w;"
                            "PBOOL x;"
                            "LPBYTE y;"
                            "PBOOLEAN z;"
                            "PBYTE A;"
                            "LPCSTR B;"
                            "PCSTR C;"
                            "LPCVOID D;"
                            "LPDWORD E;"
                            "LPINT F;"
                            "PINT G;"
                            "LPLONG H;"
                            "PLONG I;"
                            "LPSTR J;"
                            "PSTR K;"
                            "PCHAR L;"
                            "LPVOID M;"
                            "PVOID N;";

        const char expected[] = "unsigned int sizeof_short ; sizeof_short = 2 ; "
                                "unsigned int sizeof_unsigned_short ; sizeof_unsigned_short = 2 ; "
                                "unsigned int sizeof_int ; sizeof_int = 4 ; "
                                "unsigned int sizeof_unsigned_int ; sizeof_unsigned_int = 4 ; "
                                "unsigned int sizeof_long ; sizeof_long = 4 ; "
                                "unsigned int sizeof_unsigned_long ; sizeof_unsigned_long = 4 ; "
                                "unsigned int sizeof_long_long ; sizeof_long_long = 8 ; "
                                "unsigned int sizeof_unsigned_long_long ; sizeof_unsigned_long_long = 8 ; "
                                "unsigned int sizeof_float ; sizeof_float = 4 ; "
                                "unsigned int sizeof_double ; sizeof_double = 8 ; "
                                "unsigned int sizeof_long_double ; sizeof_long_double = 8 ; "
                                "unsigned int sizeof_bool ; sizeof_bool = 1 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 4 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 4 ; "
                                "unsigned long a ; "
                                "long b ; "
                                "long c ; "
                                "long d ; "
                                "unsigned long e ; "
                                "int f ; "
                                "unsigned char g ; "
                                "unsigned char h ; "
                                "char i ; "
                                "unsigned long j ; "
                                "float k ; "
                                "int l ; "
                                "int m ; "
                                "long long n ; "
                                "long o ; "
                                "short p ; "
                                "unsigned char q ; "
                                "unsigned int r ; "
                                "unsigned long s ; "
                                "unsigned short t ; "
                                "unsigned short u ; "
                                "void * v ; "
                                "int * w ; "
                                "int * x ; "
                                "unsigned char * y ; "
                                "unsigned char * z ; "
                                "unsigned char * A ; "
                                "const char * B ; "
                                "const char * C ; "
                                "const void * D ; "
                                "unsigned long * E ; "
                                "int * F ; "
                                "int * G ; "
                                "long * H ; "
                                "long * I ; "
                                "char * J ; "
                                "char * K ; "
                                "char * L ; "
                                "void * M ; "
                                "void * N ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Win32));
    }

    void platformWin64()
    {
        const char code[] = "unsigned int sizeof_short = sizeof(short);"
                            "unsigned int sizeof_unsigned_short = sizeof(unsigned short);"
                            "unsigned int sizeof_int = sizeof(int);"
                            "unsigned int sizeof_unsigned_int = sizeof(unsigned int);"
                            "unsigned int sizeof_long = sizeof(long);"
                            "unsigned int sizeof_unsigned_long = sizeof(unsigned long);"
                            "unsigned int sizeof_long_long = sizeof(long long);"
                            "unsigned int sizeof_unsigned_long_long = sizeof(unsigned long long);"
                            "unsigned int sizeof_float = sizeof(float);"
                            "unsigned int sizeof_double = sizeof(double);"
                            "unsigned int sizeof_long_double = sizeof(long double);"
                            "unsigned int sizeof_bool = sizeof(bool);"
                            "unsigned int sizeof_pointer = sizeof(void *);"
                            "unsigned int sizeof_size_t = sizeof(size_t);"
                            "size_t a;"
                            "ssize_t b;"
                            "ptrdiff_t c;"
                            "intptr_t d;"
                            "uintptr_t e;";

        const char expected[] = "unsigned int sizeof_short ; sizeof_short = 2 ; "
                                "unsigned int sizeof_unsigned_short ; sizeof_unsigned_short = 2 ; "
                                "unsigned int sizeof_int ; sizeof_int = 4 ; "
                                "unsigned int sizeof_unsigned_int ; sizeof_unsigned_int = 4 ; "
                                "unsigned int sizeof_long ; sizeof_long = 4 ; "
                                "unsigned int sizeof_unsigned_long ; sizeof_unsigned_long = 4 ; "
                                "unsigned int sizeof_long_long ; sizeof_long_long = 8 ; "
                                "unsigned int sizeof_unsigned_long_long ; sizeof_unsigned_long_long = 8 ; "
                                "unsigned int sizeof_float ; sizeof_float = 4 ; "
                                "unsigned int sizeof_double ; sizeof_double = 8 ; "
                                "unsigned int sizeof_long_double ; sizeof_long_double = 8 ; "
                                "unsigned int sizeof_bool ; sizeof_bool = 1 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 8 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 8 ; "
                                "unsigned long long a ; "
                                "long long b ; "
                                "long long c ; "
                                "long long d ; "
                                "unsigned long long e ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Win64));
    }

    void platformUnix32()
    {
        const char code[] = "unsigned int sizeof_short = sizeof(short);"
                            "unsigned int sizeof_unsigned_short = sizeof(unsigned short);"
                            "unsigned int sizeof_int = sizeof(int);"
                            "unsigned int sizeof_unsigned_int = sizeof(unsigned int);"
                            "unsigned int sizeof_long = sizeof(long);"
                            "unsigned int sizeof_unsigned_long = sizeof(unsigned long);"
                            "unsigned int sizeof_long_long = sizeof(long long);"
                            "unsigned int sizeof_unsigned_long_long = sizeof(unsigned long long);"
                            "unsigned int sizeof_float = sizeof(float);"
                            "unsigned int sizeof_double = sizeof(double);"
                            "unsigned int sizeof_long_double = sizeof(long double);"
                            "unsigned int sizeof_bool = sizeof(bool);"
                            "unsigned int sizeof_pointer = sizeof(void *);"
                            "unsigned int sizeof_size_t = sizeof(size_t);"
                            "size_t a;"
                            "ssize_t b;"
                            "ptrdiff_t c;"
                            "intptr_t d;"
                            "uintptr_t e;";

        const char expected[] = "unsigned int sizeof_short ; sizeof_short = 2 ; "
                                "unsigned int sizeof_unsigned_short ; sizeof_unsigned_short = 2 ; "
                                "unsigned int sizeof_int ; sizeof_int = 4 ; "
                                "unsigned int sizeof_unsigned_int ; sizeof_unsigned_int = 4 ; "
                                "unsigned int sizeof_long ; sizeof_long = 4 ; "
                                "unsigned int sizeof_unsigned_long ; sizeof_unsigned_long = 4 ; "
                                "unsigned int sizeof_long_long ; sizeof_long_long = 8 ; "
                                "unsigned int sizeof_unsigned_long_long ; sizeof_unsigned_long_long = 8 ; "
                                "unsigned int sizeof_float ; sizeof_float = 4 ; "
                                "unsigned int sizeof_double ; sizeof_double = 8 ; "
                                "unsigned int sizeof_long_double ; sizeof_long_double = 12 ; "
                                "unsigned int sizeof_bool ; sizeof_bool = 1 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 4 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 4 ; "
                                "unsigned long a ; "
                                "long b ; "
                                "long c ; "
                                "long d ; "
                                "unsigned long e ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Unix32));
    }

    void platformUnix64()
    {
        const char code[] = "unsigned int sizeof_short = sizeof(short);"
                            "unsigned int sizeof_unsigned_short = sizeof(unsigned short);"
                            "unsigned int sizeof_int = sizeof(int);"
                            "unsigned int sizeof_unsigned_int = sizeof(unsigned int);"
                            "unsigned int sizeof_long = sizeof(long);"
                            "unsigned int sizeof_unsigned_long = sizeof(unsigned long);"
                            "unsigned int sizeof_long_long = sizeof(long long);"
                            "unsigned int sizeof_unsigned_long_long = sizeof(unsigned long long);"
                            "unsigned int sizeof_float = sizeof(float);"
                            "unsigned int sizeof_double = sizeof(double);"
                            "unsigned int sizeof_long_double = sizeof(long double);"
                            "unsigned int sizeof_bool = sizeof(bool);"
                            "unsigned int sizeof_pointer = sizeof(void *);"
                            "unsigned int sizeof_size_t = sizeof(size_t);"
                            "size_t a;"
                            "ssize_t b;"
                            "ptrdiff_t c;"
                            "intptr_t d;"
                            "uintptr_t e;";

        const char expected[] = "unsigned int sizeof_short ; sizeof_short = 2 ; "
                                "unsigned int sizeof_unsigned_short ; sizeof_unsigned_short = 2 ; "
                                "unsigned int sizeof_int ; sizeof_int = 4 ; "
                                "unsigned int sizeof_unsigned_int ; sizeof_unsigned_int = 4 ; "
                                "unsigned int sizeof_long ; sizeof_long = 8 ; "
                                "unsigned int sizeof_unsigned_long ; sizeof_unsigned_long = 8 ; "
                                "unsigned int sizeof_long_long ; sizeof_long_long = 8 ; "
                                "unsigned int sizeof_unsigned_long_long ; sizeof_unsigned_long_long = 8 ; "
                                "unsigned int sizeof_float ; sizeof_float = 4 ; "
                                "unsigned int sizeof_double ; sizeof_double = 8 ; "
                                "unsigned int sizeof_long_double ; sizeof_long_double = 16 ; "
                                "unsigned int sizeof_bool ; sizeof_bool = 1 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 8 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 8 ; "
                                "unsigned long long a ; "
                                "long long b ; "
                                "long long c ; "
                                "long long d ; "
                                "unsigned long long e ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Unix64));
    }
};

REGISTER_TEST(TestTokenizer)
