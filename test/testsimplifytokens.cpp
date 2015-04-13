/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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


class TestSimplifyTokens : public TestFixture {
public:
    TestSimplifyTokens() : TestFixture("TestSimplifyTokens") {
    }


private:
    Settings settings_std;
    Settings settings_windows;

    void run() {
        LOAD_LIB_2(settings_std.library, "std.cfg");
        LOAD_LIB_2(settings_windows.library, "windows.cfg");

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
        TEST_CASE(redundant_plus_numbers);
        TEST_CASE(parentheses1);
        TEST_CASE(parenthesesVar);      // Remove redundant parentheses around variable .. "( %name% )"
        TEST_CASE(declareVar);

        TEST_CASE(declareArray);

        TEST_CASE(dontRemoveIncrement);
        TEST_CASE(removePostIncrement);
        TEST_CASE(removePreIncrement);

        TEST_CASE(elseif1);
        TEST_CASE(ifa_ifa);     // "if (a) { if (a) .." => "if (a) { if (1) .."

        TEST_CASE(sizeof_array);
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
        TEST_CASE(sizeofsizeof);
        TEST_CASE(casting);

        TEST_CASE(strlen1);
        TEST_CASE(strlen2);

        TEST_CASE(namespaces);

        // Assignment in condition..
        TEST_CASE(ifassign1);
        TEST_CASE(ifAssignWithCast);
        TEST_CASE(whileAssign1);
        TEST_CASE(whileAssign2);
        TEST_CASE(whileAssign3); // varid
        TEST_CASE(whileAssign4); // links
        TEST_CASE(doWhileAssign); // varid
        TEST_CASE(test_4881); // similar to doWhileAssign (#4911), taken from #4881 with full code

        // "if(0==x)" => "if(!x)"
        TEST_CASE(simplifyIfNot);
        TEST_CASE(simplifyIfNotNull);

        TEST_CASE(combine_wstrings);

        // Simplify "not" to "!" (#345)
        TEST_CASE(not1);

        // Simplify "and" to "&&" (#620)
        TEST_CASE(and1);

        // Simplify "or" to "||"
        TEST_CASE(or1);

        TEST_CASE(cAlternativeTokens);

        TEST_CASE(comma_keyword);
        TEST_CASE(remove_comma);

        // Simplify "?:"
        TEST_CASE(simplifyConditionOperator);

        // Simplify calculations
        TEST_CASE(calculations);
        TEST_CASE(comparisons);

        //remove dead code after flow control statements
        TEST_CASE(simplifyFlowControl);
        TEST_CASE(flowControl);

        // Simplify nested strcat() calls
        TEST_CASE(strcat1);
        TEST_CASE(strcat2);

        TEST_CASE(simplifyAtol)

        TEST_CASE(simplifyOperator1);
        TEST_CASE(simplifyOperator2);

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
        // ticket #3140
        TEST_CASE(while0for);
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
        TEST_CASE(enum21); // ticket #2720
        TEST_CASE(enum22); // ticket #2745
        TEST_CASE(enum23); // ticket #2804
        TEST_CASE(enum24); // ticket #2828
        TEST_CASE(enum25); // ticket #2966
        TEST_CASE(enum26); // ticket #2975 (segmentation fault)
        TEST_CASE(enum27); // ticket #3005 (segmentation fault)
        TEST_CASE(enum28);
        TEST_CASE(enum29); // ticket #3747 (bitwise or value)
        TEST_CASE(enum30); // ticket #3852 (false positive)
        TEST_CASE(enum31); // ticket #3934 (calculation in first item)
        TEST_CASE(enum32); // ticket #3998 (access violation)
        TEST_CASE(enum33); // ticket #4015 (segmentation fault)
        TEST_CASE(enum34); // ticket #4141 (division by zero)
        TEST_CASE(enum35); // ticket #3953 (avoid simplification of type)
        TEST_CASE(enum36); // ticket #4378
        TEST_CASE(enum37); // ticket #4280 (shadow variable)
        TEST_CASE(enum38); // ticket #4463 (when throwing enum id, don't warn about shadow variable)
        TEST_CASE(enum39); // ticket #5145 (fp variable hides enum)
        TEST_CASE(enum40);
        TEST_CASE(enum41); // ticket #5212 (valgrind errors during enum simplification)
        TEST_CASE(enum42); // ticket #5182 (template function call in enum value)
        TEST_CASE(enum43); // lhs in assignment
        TEST_CASE(enumscope1); // ticket #3949
        TEST_CASE(duplicateDefinition); // ticket #3565
        TEST_CASE(invalid_enum); // #5600

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

        // struct ABC { } abc; => struct ABC { }; ABC abc;
        TEST_CASE(simplifyStructDecl1);
        TEST_CASE(simplifyStructDecl2); // ticket #2579
        TEST_CASE(simplifyStructDecl3);
        TEST_CASE(simplifyStructDecl4);
        TEST_CASE(simplifyStructDecl5); // ticket #3533 (segmentation fault)
        TEST_CASE(simplifyStructDecl6); // ticket #3732
        TEST_CASE(simplifyStructDecl7); // ticket #476 (static anonymous struct array)

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

        TEST_CASE(removeUnnecessaryQualification1);
        TEST_CASE(removeUnnecessaryQualification2);
        TEST_CASE(removeUnnecessaryQualification3);
        TEST_CASE(removeUnnecessaryQualification4);
        TEST_CASE(removeUnnecessaryQualification5);
        TEST_CASE(removeUnnecessaryQualification6);  // ticket #2859
        TEST_CASE(removeUnnecessaryQualification7);  // ticket #2970
        TEST_CASE(removeUnnecessaryQualification8);
        TEST_CASE(removeUnnecessaryQualification9);  // ticket #3151
        TEST_CASE(removeUnnecessaryQualification10); // ticket #3310 segmentation fault
        TEST_CASE(removeUnnecessaryQualification11);
        TEST_CASE(removeUnnecessaryQualification12);
        TEST_CASE(removeUnnecessaryQualification13);
        TEST_CASE(removeUnnecessaryQualification14);
        TEST_CASE(removeUnnecessaryQualification15);
        TEST_CASE(removeUnnecessaryQualification16);

        TEST_CASE(simplifyVarDecl1); // ticket # 2682 segmentation fault
        TEST_CASE(simplifyVarDecl2); // ticket # 2834 segmentation fault
        TEST_CASE(return_strncat); // ticket # 2860 Returning value of strncat() reported as memory leak

        // #3069 : for loop with 1 iteration
        // for (x=0;x<1;x++) { .. }
        // The for is redundant
        TEST_CASE(removeRedundantFor);

        TEST_CASE(consecutiveBraces);

        TEST_CASE(undefinedSizeArray);

        TEST_CASE(simplifyArrayAddress);  // Replace "&str[num]" => "(str + num)"
        TEST_CASE(simplifyCharAt);
        TEST_CASE(simplifyOverride); // ticket #5069
    }

    std::string tok(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Unspecified) {
        errout.str("");

        Settings settings;
        settings.addEnabled("portability");
        settings.platform(type);
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, !simplify);
    }

    std::string tokWithWindows(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Unspecified) {
        errout.str("");

        settings_windows.addEnabled("portability");
        settings_windows.platform(type);
        Tokenizer tokenizer(&settings_windows, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, !simplify);
    }

    std::string tok(const char code[], const char filename[], bool simplify = true) {
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, false);
    }

    std::string tokWithStdLib(const char code[]) {
        errout.str("");

        Tokenizer tokenizer(&settings_std, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, false);
    }

    std::string tokenizeDebugListing(const char code[], bool simplify = false, const char filename[] = "test.cpp") {
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        if (simplify)
            tokenizer.simplifyTokenList2();

        // result..
        return tokenizer.tokens()->stringifyList(true);
    }

    void simplifyTokenList1() {
        // #1717 : The simplifyErrNoInWhile needs to be used before simplifyIfAndWhileAssign..
        ASSERT_EQUALS("; x = f ( ) ; while ( x == -1 ) { x = f ( ) ; }",
                      tok(";while((x=f())==-1 && errno==EINTR){}",true));
    }


    void cast() {
        ASSERT_EQUALS("if ( ! p ) { ; }", tok("if (p == (char *)0);"));
        ASSERT_EQUALS("return str ;", tok("return (char *)str;"));

        ASSERT_EQUALS("if ( * a )", tok("if ((char)*a)"));
        ASSERT_EQUALS("if ( & a )", tok("if ((int)&a)"));
        ASSERT_EQUALS("if ( * a )", tok("if ((unsigned int)(unsigned char)*a)"));
        ASSERT_EQUALS("class A { A operator* ( int ) ; } ;", tok("class A { A operator *(int); };"));
        ASSERT_EQUALS("class A { A operator* ( int ) const ; } ;", tok("class A { A operator *(int) const; };"));
        ASSERT_EQUALS("if ( ! p ) { ; }", tok("if (p == (char *)(char *)0);"));

        // no simplification as the cast may be important here. see #2897 for example
        ASSERT_EQUALS("; * ( ( char * ) p + 1 ) = 0 ;", tok("; *((char *)p + 1) = 0;"));

        ASSERT_EQUALS("if ( true )", tok("if ((unsigned char)1)")); // #4164
        ASSERT_EQUALS("f ( 200 )", tok("f((unsigned char)200)"));
        ASSERT_EQUALS("f ( ( char ) 1234 )", tok("f((char)1234)")); // dont simplify downcast
    }


    void iftruefalse() {
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
            const char code2[] = "void f ( ) { }";
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

        {
            const char code1[] = "static const int x=1; void f() { if(x) { a=0; } }";
            ASSERT_EQUALS("void f ( ) { a = 0 ; }", tok(code1));
        }
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
    }

    void combine_wstrings() {
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

        Settings settings;
        settings.platform(Settings::Unspecified);
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code1);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(tok(code2), tokenizer.tokens()->stringifyList(0, false));
        ASSERT_EQUALS(true, tokenizer.tokens()->tokAt(13) && tokenizer.tokens()->tokAt(13)->isLong());
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


    void parentheses1() {
        ASSERT_EQUALS("a <= 110 ;", tok("a <= (10+100);"));
        ASSERT_EQUALS("while ( x ( ) == -1 ) { }", tok("while((x()) == -1){ }"));
    }

    void parenthesesVar() {
        // remove parentheses..
        ASSERT_EQUALS("a = p ;", tok("a = (p);"));
        ASSERT_EQUALS("if ( a < p ) { }", tok("if(a<(p)){}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p == -1 ) { } }", tok("void f(){int p; if((p)==-1){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( -1 == p ) { } }", tok("void f(){int p; if(-1==(p)){}}"));
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
        ASSERT_EQUALS("b = a ;", tok("b = (char)a;"));
        ASSERT_EQUALS("cast < char * > ( p )", tok("cast<char *>(p)"));
        ASSERT_EQUALS("return ( a + b ) * c ;", tok("return (a+b)*c;"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( 2 * p == 0 ) { } }", tok("void f(){int p; if (2*p == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { DIR * f ; f = opendir ( dirname ) ; if ( closedir ( f ) ) { } }", tok("void f(){DIR * f = opendir(dirname);if (closedir(f)){}}"));
        ASSERT_EQUALS("void foo ( int p ) { if ( p >= 0 ) { ; } }", tok("void foo(int p){if((p)>=0);}"));
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

    void dontRemoveIncrement() {
        {
            const char code[] = "void f(int a)\n"
                                "{\n"
                                "    if (a > 10)\n"
                                "        a = 5;\n"
                                "    else\n"
                                "        a = 10;\n"
                                "    a++;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( int a ) { if ( a > 10 ) { a = 5 ; } else { a = 10 ; } a ++ ; }", tok(code));
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
            ASSERT_EQUALS("void f ( int a ) { if ( a > 10 ) { a = 5 ; } else { a = 10 ; } ++ a ; }", tok(code));
        }
    }

    void removePostIncrement() {
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


    void removePreIncrement() {
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


    std::string elseif(const char code[]) {
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.elseif();
        return tokenizer.tokens()->stringifyList(false);
    }

    void elseif1() {
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


    void ifa_ifa() {
        ASSERT_EQUALS("int a ; if ( a ) { { ab } cd }", tok("int a ; if (a) { if (a) { ab } cd }", true));
        ASSERT_EQUALS("int a ; if ( a ) { { ab } cd }", tok("int a ; if (unlikely(a)) { if (a) { ab } cd }", true));
    }




    unsigned int sizeofFromTokenizer(const char type[]) {
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr("");
        tokenizer.tokenize(istr, "test.cpp");
        Token tok1(0);
        tok1.str(type);
        return tokenizer.sizeOfType(&tok1);
    }



    void sizeof_array() {
        const char *code;

        code = "void foo()\n"
               "{\n"
               "    int i[4];\n"
               "    sizeof(i);\n"
               "    sizeof(*i);\n"
               "}\n";
        ASSERT_EQUALS("void foo ( ) { int i [ 4 ] ; 16 ; 4 ; }", tok(code));

        code = "static int i[4];\n"
               "void f()\n"
               "{\n"
               "    int i[10];\n"
               "    sizeof(i);\n"
               "}\n";
        ASSERT_EQUALS("static int i [ 4 ] ; void f ( ) { int i [ 10 ] ; 40 ; }", tok(code));
        {
            code = "int i[10];\n"
                   "sizeof(i[0]);\n";
            ASSERT_EQUALS("int i [ 10 ] ; 4 ;", tok(code));

            code = "int i[10];\n"
                   "sizeof i[0];\n";
            ASSERT_EQUALS("int i [ 10 ] ; 4 ;", tok(code));
        }

        code = "char i[2][20];\n"
               "sizeof(i[1]);\n"
               "sizeof(i);";
        ASSERT_EQUALS("char i [ 2 ] [ 20 ] ; 20 ; 40 ;", tok(code));
    }

    void sizeof5() {
        const char code[] =
            "const char * names[2];"
            "for (int i = 0; i != sizeof(names[0]); i++)"
            "{}";
        std::ostringstream expected;
        expected << "const char * names [ 2 ] ; for ( int i = 0 ; i != " << sizeofFromTokenizer("*") << " ; i ++ ) { }";
        ASSERT_EQUALS(expected.str(), tok(code));
    }

    void sizeof6() {
        const char code[] = ";int i;\n"
                            "sizeof(i);\n";

        std::ostringstream expected;
        expected << "; int i ; " << sizeof(int) << " ;";

        ASSERT_EQUALS(expected.str(), tok(code));
    }

    void sizeof7() {
        const char code[] = ";INT32 i[10];\n"
                            "sizeof(i[0]);\n";
        ASSERT_EQUALS("; INT32 i [ 10 ] ; sizeof ( i [ 0 ] ) ;", tok(code, true, Settings::Unspecified));
        ASSERT_EQUALS("; int i [ 10 ] ; 4 ;", tokWithWindows(code, true, Settings::Win32A));
    }

    void sizeof8() {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs[2];\n"
                                "  a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << (sizeofFromTokenizer("*") * 2);
            ASSERT_EQUALS("void f ( ) { char * ptrs [ 2 ] ; a = " + oss.str() + " ; }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs[55];\n"
                                "  a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << (sizeofFromTokenizer("*") * 55);
            ASSERT_EQUALS("void f ( ) { char * ptrs [ 55 ] ; a = " + oss.str() + " ; }", tok(code));
        }


        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  char* ptrs;\n"
                                "  a = sizeof( ptrs );\n"
                                "}\n";
            std::ostringstream oss;
            oss << sizeofFromTokenizer("*");
            ASSERT_EQUALS("void f ( ) { a = " + oss.str() + " ; }", tok(code));
        }
    }

    void sizeof9() {
        // ticket #487
        {
            const char code[] = "; const char *str = \"1\"; sizeof(str);";

            std::ostringstream expected;
            expected << "; const char * str ; str = \"1\" ; " << sizeofFromTokenizer("*") << " ;";

            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            const char code[] = "; const char str[] = \"1\"; sizeof(str);";

            std::ostringstream expected;
            expected << "; const char str [ 2 ] = \"1\" ; " << sizeofFromTokenizer("char")*2 << " ;";

            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            // Ticket #799
            const char code[] = "; const char str[] = {'1'}; sizeof(str);";
            ASSERT_EQUALS("; const char str [ 1 ] = { '1' } ; 1 ;", tok(code));
        }

        {
            // Ticket #2087
            const char code[] = "; const char str[] = {\"abc\"}; sizeof(str);";
            ASSERT_EQUALS("; const char str [ 4 ] = \"abc\" ; 4 ;", tok(code));
        }

        // ticket #716 - sizeof string
        {
            std::ostringstream expected;
            expected << "; " << (sizeof "123");

            ASSERT_EQUALS(expected.str(), tok("; sizeof \"123\""));
            ASSERT_EQUALS(expected.str(), tok("; sizeof(\"123\")"));
        }

        {
            const char code[] = "void f(char *a,char *b, char *c)"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char * a , char * b , char * c ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            const char code[] = "void f(char a,char b, char c)"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char a , char b , char c ) { g ( " <<
                     sizeofFromTokenizer("char") << " , " << sizeofFromTokenizer("char") << " , " << sizeofFromTokenizer("char") << " ) ; }";
            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            const char code[] = "void f(const char *a,const char *b, const char *c)"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( const char * a , const char * b , const char * c ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            const char code[] = "void f(char a[10],char b[10], char c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char a [ 10 ] , char b [ 10 ] , char c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            const char code[] = "void f(const char a[10],const char b[10], const char c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( const char a [ 10 ] , "
                     "const char b [ 10 ] , "
                     "const char c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            const char code[] = "void f(const char *a[10],const char *b[10], const char *c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( const char * a [ 10 ] , "
                     "const char * b [ 10 ] , "
                     "const char * c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            const char code[] = "void f(char *a[10],char *b[10], char *c[10])"
                                "{g(sizeof(a),sizeof(b),sizeof(c));}";
            std::ostringstream expected;
            expected << "void f ( char * a [ 10 ] , char * b [ 10 ] , char * c [ 10 ] ) { g ( " <<
                     sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " , " << sizeofFromTokenizer("*") << " ) ; }";
            ASSERT_EQUALS(expected.str(), tok(code));
        }

        {
            std::ostringstream expected;
            expected << "; " << sizeof("\"quote\"");
            ASSERT_EQUALS(expected.str(), tok("; sizeof(\"\\\"quote\\\"\")"));
        }

        {
            std::ostringstream expected;
            expected << "void f ( ) { char str [ 100 ] = \"100\" ; " << sizeofFromTokenizer("char")*100 << " }";
            ASSERT_EQUALS(expected.str(), tok("void f ( ) { char str [ 100 ] = \"100\" ; sizeof ( str ) }"));
        }
    }

    void sizeof10() {
        // ticket #809
        const char code[] = "int m ; "
                            "compat_ulong_t um ; "
                            "long size ; size = sizeof ( m ) / sizeof ( um ) ;";

        ASSERT_EQUALS(code, tok(code, true, Settings::Win32A));
    }

    void sizeof11() {
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

    void sizeof12() {
        // ticket #827
        const char code[] = "void f()\n"
                            "{\n"
                            "    int *p;\n"
                            "    (sizeof *p);\n"
                            "}";

        const char expected[] = "void f ( ) "
                                "{"
                                ""
                                " 4 ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code));
    }

    void sizeof13() {
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

    void sizeof14() {
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

        // #5064 - sizeof !! (a == 1);
        ASSERT_EQUALS("sizeof ( ! ! ( a == 1 ) ) ;", tok("sizeof !!(a==1);"));
    }

    void sizeof15() {
        // ticket #1020
        tok("void f()\n"
            "{\n"
            "    int *n;\n"
            "    sizeof *(n);\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof16() {
        // ticket #1027
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a;\n"
                            "    printf(\"%i\", sizeof a++);\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { int a ; printf ( \"%i\" , sizeof ( a ++ ) ) ; }", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof17() {
        // ticket #1050
        const char code[] = "void f()\n"
                            "{\n"
                            "    sizeof 1;\n"
                            "    while (0);\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { sizeof ( 1 ) ; }", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof18() {
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

    void sizeof19() {
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

    void sizeof20() {
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
                      " append ( sizeof ( a ) ) . append ( ) ; "
                      "}", tok(code));
    }

    void sizeof21() {
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

    void sizeofsizeof() {
        // ticket #1682
        const char code[] = "void f()\n"
                            "{\n"
                            "    sizeof sizeof 1;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { sizeof ( sizeof ( 1 ) ) ; }", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void casting() {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "for (int i = 0; i < static_cast<int>(3); ++i) {}\n"
                                "}\n";

            const char expected[] = "void f ( ) { for ( int i = 0 ; i < 3 ; ++ i ) { } }";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    p = const_cast<char *> qtu ();\n"
                                "}\n";

            const char expected[] = "void f ( ) { p = const_cast < char * > qtu ( ) ; }";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            // ticket #645
            const char code[] = "void f()\n"
                                "{\n"
                                "    return dynamic_cast<Foo *>((bar()));\n"
                                "}\n";
            const char expected[] = "void f ( ) { return bar ( ) ; }";

            ASSERT_EQUALS(expected, tok(code));
        }
    }


    void strlen1() {
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

    void strlen2() {
        // #4530 - make sure calculation with strlen is simplified
        ASSERT_EQUALS("i = -4 ;",
                      tok("i = (strlen(\"abcd\") - 8);"));
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


    std::string simplifyIfAndWhileAssign(const char code[]) {
        errout.str("");
        Settings settings;
        // tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyIfAndWhileAssign();

        return tokenizer.tokens()->stringifyList(0, false);
    }

    void ifassign1() {
        ASSERT_EQUALS("; a = b ; if ( a ) { ; }", simplifyIfAndWhileAssign(";if(a=b);"));
        ASSERT_EQUALS("; a = b ( ) ; if ( a ) { ; }", simplifyIfAndWhileAssign(";if((a=b()));"));
        ASSERT_EQUALS("; a = b ( ) ; if ( ! ( a ) ) { ; }", simplifyIfAndWhileAssign(";if(!(a=b()));"));
        ASSERT_EQUALS("; a . x = b ( ) ; if ( ! ( a . x ) ) { ; }", simplifyIfAndWhileAssign(";if(!(a->x=b()));"));
        ASSERT_EQUALS("void f ( ) { A ( ) a = b ; if ( a ) { ; } }", simplifyIfAndWhileAssign("void f() { A() if(a=b); }"));
        ASSERT_EQUALS("void foo ( int a ) { a = b ( ) ; if ( a >= 0 ) { ; } }", tok("void foo(int a) {if((a=b())>=0);}"));
        TODO_ASSERT_EQUALS("void foo ( A a ) { a . c = b ( ) ; if ( 0 <= a . c ) { ; } }",
                           "void foo ( A a ) { a . c = b ( ) ; if ( a . c >= 0 ) { ; } }",
                           tok("void foo(A a) {if((a.c=b())>=0);}"));
    }

    void ifAssignWithCast() {
        const char *code =  "void foo()\n"
                            "{\n"
                            "FILE *f;\n"
                            "if( (f = fopen(\"foo\", \"r\")) == ((FILE*)NULL) )\n"
                            "return(-1);\n"
                            "fclose(f);\n"
                            "}\n";
        const char *expected = "void foo ( ) "
                               "{ "
                               "FILE * f ; "
                               "f = fopen ( \"foo\" , \"r\" ) ; "
                               "if ( ! f ) "
                               "{ "
                               "return -1 ; "
                               "} "
                               "fclose ( f ) ; "
                               "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void whileAssign1() {
        ASSERT_EQUALS("; a = b ; while ( a ) { b = 0 ; a = b ; }", simplifyIfAndWhileAssign(";while(a=b) { b = 0; }"));
        ASSERT_EQUALS("; a . b = c ; while ( a . b ) { c = 0 ; a . b = c ; }", simplifyIfAndWhileAssign(";while(a.b=c) { c=0; }"));
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

    void whileAssign2() {
        // #1909 - Internal error
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

    void whileAssign3() {
        // #4254 - Variable id
        const char code[] = "void f() {\n"
                            "  int a;\n"
                            "  while (a = x());\n"
                            "}";
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: void f ( ) {\n"
                      "2: int a@1 ;\n"
                      "3: a@1 = x ( ) ; while ( a@1 ) { ; a@1 = x ( ) ; }\n"
                      "4: }\n", tokenizeDebugListing(code, true, "test.c"));
    }

    void whileAssign4() {
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr("; while (!(m = q->push<Message>(x))) {}");
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        ASSERT_EQUALS("; m = q . push < Message > ( x ) ; while ( ! m ) { m = q . push < Message > ( x ) ; }", tokenizer.tokens()->stringifyList(0, false));
        ASSERT(tokenizer.tokens()->tokAt(26) != nullptr);
        if (tokenizer.tokens()->tokAt(26)) {
            ASSERT(tokenizer.tokens()->linkAt(6) == tokenizer.tokens()->tokAt(8));
            ASSERT(tokenizer.tokens()->linkAt(24) == tokenizer.tokens()->tokAt(26));
        }
    }

    void doWhileAssign() {
        ASSERT_EQUALS("; do { a = b ; } while ( a ) ;", simplifyIfAndWhileAssign(";do { } while(a=b);"));
        ASSERT_EQUALS("; do { a . a = 0 ; a . b = c ; } while ( a . b ) ;", simplifyIfAndWhileAssign(";do { a.a = 0; } while(a.b=c);"));
        ASSERT_EQUALS("struct hfs_bnode * node ; "
                      "struct hfs_btree * tree ; "
                      "do { node = tree . node_hash [ i ++ ] ; } while ( node ) ;",
                      tok("struct hfs_bnode *node;"
                          "struct hfs_btree *tree;"
                          "do { } while((node = tree->node_hash[i++]));"));
        ASSERT_EQUALS("char * s ; do { s = new char [ 10 ] ; } while ( ! s ) ;",
                      tok("char *s; do { } while (0 == (s=new char[10]));"));
        // #4911
        ASSERT_EQUALS("; do { current = f ( ) ; } while ( ( current ) != 0 ) ;", simplifyIfAndWhileAssign(";do { } while((current=f()) != NULL);"));
    }

    void simplifyIfNot() {
        ASSERT_EQUALS("if ( ! x ) { ; }", tok("if(0==x);"));
        ASSERT_EQUALS("if ( ! x ) { ; }", tok("if(x==0);"));
        ASSERT_EQUALS("if ( ! ( a = b ) ) { ; }", tok("if(0==(a=b));"));
        ASSERT_EQUALS("if ( ! a && b ( ) ) { ; }", tok("if( 0 == a && b() );"));
        ASSERT_EQUALS("if ( b ( ) && ! a ) { ; }", tok("if( b() && 0 == a );"));
        ASSERT_EQUALS("if ( ! ( a = b ) ) { ; }", tok("if((a=b)==0);"));
        ASSERT_EQUALS("if ( ! x . y ) { ; }", tok("if(x.y==0);"));
        ASSERT_EQUALS("if ( ! x ) { ; }", tok("if((x==0));"));
        ASSERT_EQUALS("if ( ( ! x ) && ! y ) { ; }", tok("if((x==0) && y==0);"));
        ASSERT_EQUALS("if ( ! ( ! fclose ( fd ) ) ) { ; }", tok("if(!(fclose(fd) == 0));"));
    }

    void simplifyIfNotNull() {
        const char code[] = "void f(int x) {\n"
                            "    x = (x != 0);\n"
                            "}";
        ASSERT_EQUALS("void f ( int x ) { }", tok(code, true));
    }

    void not1() {
        ASSERT_EQUALS("void f ( ) { if ( ! p ) { ; } }", tok("void f() { if (not p); }", "test.c", false));
        ASSERT_EQUALS("void f ( ) { if ( p && ! q ) { ; } }", tok("void f() { if (p && not q); }", "test.c", false));
        ASSERT_EQUALS("void f ( ) { a = ! ( p && q ) ; }", tok("void f() { a = not(p && q); }", "test.c", false));
        // Don't simplify 'not' or 'compl' if they are defined as a type;
        // in variable declaration and in function declaration/definition
        ASSERT_EQUALS("struct not { int x ; } ;", tok("struct not { int x; };", "test.c", false));
        ASSERT_EQUALS("void f ( ) { not p ; compl c ; }", tok(" void f() { not p; compl c; }", "test.c", false));
        ASSERT_EQUALS("void foo ( not i ) ;", tok("void foo(not i);", "test.c", false));
        ASSERT_EQUALS("int foo ( not i ) { return g ( i ) ; }", tok("int foo(not i) { return g(i); }", "test.c", false));
    }

    void and1() {
        ASSERT_EQUALS("void f ( ) { if ( p && q ) { ; } }",
                      tok("void f() { if (p and q) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) && q ) { ; } }",
                      tok("void f() { if (foo() and q) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) && bar ( ) ) { ; } }",
                      tok("void f() { if (foo() and bar()) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( p && bar ( ) ) { ; } }",
                      tok("void f() { if (p and bar()) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( p && ! q ) { ; } }",
                      tok("void f() { if (p and not q) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { r = a && b ; }",
                      tok("void f() { r = a and b; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { r = ( a || b ) && ( c || d ) ; }",
                      tok("void f() { r = (a || b) and (c || d); }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( test1 [ i ] == 'A' && test2 [ i ] == 'C' ) { } }",
                      tok("void f() { if (test1[i] == 'A' and test2[i] == 'C') {} }", "test.c", false));
    }

    void or1() {
        ASSERT_EQUALS("void f ( ) { if ( p || q ) { ; } }",
                      tok("void f() { if (p or q) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) || q ) { ; } }",
                      tok("void f() { if (foo() or q) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) || bar ( ) ) { ; } }",
                      tok("void f() { if (foo() or bar()) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( p || bar ( ) ) { ; } }",
                      tok("void f() { if (p or bar()) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { if ( p || ! q ) { ; } }",
                      tok("void f() { if (p or not q) ; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { r = a || b ; }",
                      tok("void f() { r = a or b; }", "test.c", false));

        ASSERT_EQUALS("void f ( ) { r = ( a && b ) || ( c && d ) ; }",
                      tok("void f() { r = (a && b) or (c && d); }", "test.c", false));
    }

    void cAlternativeTokens() {
        ASSERT_EQUALS("void f ( ) { err = err | ( ( r & s ) && ! t ) ; }",
                      tok("void f() { err or_eq ((r bitand s) and not t); }", "test.c", false));
        ASSERT_EQUALS("void f ( ) const { r = f ( a [ 4 ] | 15 , ~ c , ! d ) ; }",
                      tok("void f() const { r = f(a[4] bitor 0x0F, compl c, not d) ; }", "test.c", false));

    }

    void comma_keyword() {
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    delete a, delete b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a ; delete b ; }", tok(code));
        }

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
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a ; b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b, *c;\n"
                                "    delete a, b, c;\n"
                                "}\n";
            // delete a; b; c; would be better but this will do too
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; char * c ; delete a ; b , c ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    if (x) \n"
                                "        delete a, b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; if ( x ) { delete a ; b ; } }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b, *c;\n"
                                "    if (x) \n"
                                "        delete a, b, c;\n"
                                "}\n";
            // delete a; b; c; would be better but this will do too
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; char * c ; if ( x ) { delete a ; b , c ; } }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char **a, **b, **c;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * * a ; char * * b ; char * * c ; }", tok(code));
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
                          "}", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    delete [] a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { delete [ ] a ; a = 0 ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    delete a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { delete a ; a = 0 ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if( x ) delete a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { if ( x ) { delete a ; a = 0 ; } }", tok(code));
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

    void remove_comma() {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  int a,b;\n"
                                "  if( a )\n"
                                "  a=0,\n"
                                "  b=0;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { int a ; int b ; if ( a ) { a = 0 ; b = 0 ; } }", tok(code));
        }

        {
            const char code[] = "a ? b = c , d : e ;"; // do nothing
            ASSERT_EQUALS(code, tok(code));
        }

        {
            const char code[] = "; return a ? b = c , d : e ;"; // do nothing
            ASSERT_EQUALS(code, tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  A a,b;\n"
                                "  if( a.f )\n"
                                "  a.f=b.f,\n"
                                "  a.g=b.g;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { A a ; A b ; if ( a . f ) { a . f = b . f ; a . g = b . g ; } }", tok(code));
        }

        // keep the comma in template specifiers..
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "  int a = b<T<char,3>, int>();\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { int a ; a = b < T < char , 3 > , int > ( ) ; }", tok(code));
        }

        {
            const char code[] = "void f() {\n"
                                "  a = new std::map<std::string, std::string>;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { a = new std :: map < std :: string , std :: string > ; }", tok(code));
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
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "int foo ()\n"
                                "{\n"
                                "    return doSomething(), 0;\n"
                                "}\n";
            const char expected[]  = "int foo ( ) "
                                     "{"
                                     " doSomething ( ) ; return 0 ; "
                                     "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "int foo ()\n"
                                "{\n"
                                "    return a=1, b=2;\n"
                                "}\n";
            const char expected[]  = "int foo ( ) "
                                     "{"
                                     " a = 1 ; return b = 2 ; "
                                     "}";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void simplifyConditionOperator() {
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
            const char code[] = "void f () { switch(n) { case 1?0:foo(): break; }}";
            ASSERT_EQUALS("void f ( ) { switch ( n ) { case 0 : ; break ; } }", tok(code));
        }

        {
            const char code[] = "void f () { switch(n) { case 1?0?1:0:foo(): break; }}";
            ASSERT_EQUALS("void f ( ) { switch ( n ) { case 0 : ; break ; } }", tok(code));
        }

        {
            const char code[] = "void f () { switch(n) { case 0?foo():1: break; }}";
            ASSERT_EQUALS("void f ( ) { switch ( n ) { case 1 : ; break ; } }", tok(code));
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
            ASSERT_EQUALS("void f ( ) { }", tok(code));
        }

        {
            const char code[] = "int vals[] = { 0x13, 1?0x01:0x00 };";
            ASSERT_EQUALS("int vals [ 2 ] = { 19 , 1 } ;", tok(code));
        }

        {
            const char code[] = "int vals[] = { 0x13, 0?0x01:0x00 };";
            ASSERT_EQUALS("int vals [ 2 ] = { 19 , 0 } ;", tok(code));
        }

        {
            const char code[] = "a = 1 ? 0 : ({ 0; });";
            ASSERT_EQUALS("a = 0 ;", tok(code));
        }

        //GNU extension: "x ?: y" <-> "x ? x : y"
        {
            const char code[] = "; a = 1 ? : x; b = 0 ? : 2;";
            ASSERT_EQUALS("; a = 1 ; b = 2 ;", tok(code));
        }

        // Ticket #3572 (segmentation fault)
        ASSERT_EQUALS("0 ; x = { ? y : z ; }", tok("0; x = { ? y : z; }"));

        {
            // #3922 - (true)
            ASSERT_EQUALS("; x = 2 ;", tok("; x = (true)?2:4;"));
            ASSERT_EQUALS("; x = 4 ;", tok("; x = (false)?2:4;"));
            ASSERT_EQUALS("; x = * a ;", tok("; x = (true)?*a:*b;"));
            ASSERT_EQUALS("; x = * b ;", tok("; x = (false)?*a:*b;"));
            ASSERT_EQUALS("void f ( ) { return 1 ; }", tok("void f() { char *p=0; return (p==0)?1:2; }"));
        }
    }

    void calculations() {
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
            ASSERT_EQUALS("void * operatornew[] ( long ) ;", tok(code, true, Settings::Win32A));
        }

        ASSERT_EQUALS("; a [ 0 ] ;", tok(";a[0*(*p)];"));

        ASSERT_EQUALS(";", tok("; x = x + 0;"));

        ASSERT_EQUALS("if ( a == 2 ) { ; }", tok("if (a==1+1);"));
        ASSERT_EQUALS("if ( a + 2 != 6 ) { ; }", tok("if (a+1+1!=1+2+3);"));
        ASSERT_EQUALS("if ( 4 < a ) { ; }", tok("if (14-2*5<a*4/(2*2));"));

        ASSERT_EQUALS("( y / 2 - 2 )", tok("(y / 2 - 2)"));
        ASSERT_EQUALS("( y % 2 - 2 )", tok("(y % 2 - 2)"));

        ASSERT_EQUALS("( 4 )", tok("(1 * 2 / 1 * 2)")); // #3722

        // don't remove these spaces..
        ASSERT_EQUALS("new ( auto ) ( 4 ) ;", tok("new (auto)(4);"));
    }

    void comparisons() {
        ASSERT_EQUALS("( 1 )", tok("( 1 < 2 )"));
        ASSERT_EQUALS("( x )", tok("( x && 1 < 2 )"));
        ASSERT_EQUALS("( 5 )", tok("( 1 < 2 && 3 < 4 ? 5 : 6 )"));
        ASSERT_EQUALS("( 6 )", tok("( 1 > 2 && 3 > 4 ? 5 : 6 )"));
    }


    void simplifyFlowControl() {
        const char code1[] = "void f() {\n"
                             "  return;\n"
                             "  y();\n"
                             "}";
        ASSERT_EQUALS("void f ( ) { return ; }", tokWithStdLib(code1));

        const char code2[] = "void f() {\n"
                             "  exit(0);\n"
                             "  y();\n"
                             "}";
        ASSERT_EQUALS("void f ( ) { exit ( 0 ) ; }", tokWithStdLib(code2));

        const char code3[] = "void f() {\n"
                             "  x.abort();\n"
                             "  y();\n"
                             "}";
        ASSERT_EQUALS("void f ( ) { x . abort ( ) ; y ( ) ; }", tokWithStdLib(code3));
    }

    void flowControl() {
        {
            ASSERT_EQUALS("void f ( ) { exit ( 0 ) ; }", tokWithStdLib("void f() { exit(0); foo(); }"));
            ASSERT_EQUALS("void f ( ) { exit ( 0 ) ; }", tokWithStdLib("void f() { exit(0); if (m) foo(); }"));
            ASSERT_EQUALS("void f ( int n ) { if ( n ) { exit ( 0 ) ; } foo ( ) ; }", tokWithStdLib("void f(int n) { if (n) { exit(0); } foo(); }"));
            ASSERT_EQUALS("void f ( ) { exit ( 0 ) ; }", tokWithStdLib("void f() { exit(0); dead(); switch (n) { case 1: deadcode () ; default: deadcode (); } }"));

            ASSERT_EQUALS("int f ( int n ) { switch ( n ) { case 0 : ; exit ( 0 ) ; default : ; exit ( 0 ) ; } exit ( 0 ) ; }",
                          tokWithStdLib("int f(int n) { switch (n) {case 0: exit(0); n*=2; default: exit(0); n*=6;} exit(0); foo();}"));
            //ticket #3132
            ASSERT_EQUALS("void f ( int i ) { goto label ; { label : ; exit ( 0 ) ; } }", tokWithStdLib("void f (int i) { goto label; switch(i) { label: exit(0); } }"));
            //ticket #3148
            ASSERT_EQUALS("void f ( ) { MACRO ( exit ( 0 ) ) }", tokWithStdLib("void f() { MACRO(exit(0)) }"));
            ASSERT_EQUALS("void f ( ) { MACRO ( exit ( 0 ) ; , NULL ) }", tokWithStdLib("void f() { MACRO(exit(0);, NULL) }"));
            ASSERT_EQUALS("void f ( ) { MACRO ( bar1 , exit ( 0 ) ) }", tokWithStdLib("void f() { MACRO(bar1, exit(0)) }"));
            ASSERT_EQUALS("void f ( ) { MACRO ( exit ( 0 ) ; bar2 , foo ) }", tokWithStdLib("void f() { MACRO(exit(0); bar2, foo) }"));
        }

        {
            const char* code = "void f(){ "
                               "   if (k>0) goto label; "
                               "   exit(0); "
                               "   if (tnt) "
                               "   { "
                               "       { "
                               "           check(); "
                               "           k=0; "
                               "       } "
                               "       label: "
                               "       bar(); "
                               "   } "
                               "}";
            ASSERT_EQUALS("void f ( ) { if ( k > 0 ) { goto label ; } exit ( 0 ) ; { label : ; bar ( ) ; } }", tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    {"
                               "        boo();"
                               "        while (n) { --n; }"
                               "        {"
                               "            label:"
                               "            ok();"
                               "        }"
                               "    }"
                               "}";
            ASSERT_EQUALS("void foo ( ) { exit ( 0 ) ; { label : ; ok ( ) ; } }", tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    switch (n) {"
                               "        case 1:"
                               "            label:"
                               "            foo(); break;"
                               "        default:"
                               "            break;"
                               "    }"
                               "}";
            const char* expected = "void foo ( ) { exit ( 0 ) ; { label : ; foo ( ) ; break ; } }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    switch (n) {"
                               "        case 1:"
                               "            {"
                               "                foo();"
                               "            }"
                               "            label:"
                               "            bar();"
                               "    }"
                               "}";
            const char* expected = "void foo ( ) { exit ( 0 ) ; { label : ; bar ( ) ; } }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    switch (n) {"
                               "        case a:"
                               "            {"
                               "                foo();"
                               "            }"
                               "        case b|c:"
                               "            bar();"
                               "    }"
                               "}";
            const char* expected = "void foo ( ) { exit ( 0 ) ; }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    switch (n) {"
                               "        case 1:"
                               "            label:"
                               "            foo(); break;"
                               "        default:"
                               "            break; break;"
                               "    }"
                               "}";
            const char* expected = "void foo ( ) { exit ( 0 ) ; { label : ; foo ( ) ; break ; } }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    switch (n) {"
                               "        case 1:"
                               "            label:"
                               "            foo(); break; break;"
                               "        default:"
                               "            break;"
                               "    }"
                               "}";
            const char* expected = "void foo ( ) { exit ( 0 ) ; { label : ; foo ( ) ; break ; } }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    switch (n) {"
                               "        case 1:"
                               "            label:"
                               "            foo(); break; break;"
                               "        default:"
                               "            break; break;"
                               "    }"
                               "}";
            const char* expected = "void foo ( ) { exit ( 0 ) ; { label : ; foo ( ) ; break ; } }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "int f() { "
                               "switch (x) { case 1: exit(0); bar(); tack; { ticak(); exit(0) } exit(0);"
                               "case 2: exit(0); { random(); } tack(); "
                               "switch(y) { case 1: exit(0); case 2: exit(0); } "
                               "exit(0); } exit(0); }";
            ASSERT_EQUALS("int f ( ) { switch ( x ) { case 1 : ; exit ( 0 ) ; case 2 : ; exit ( 0 ) ; } exit ( 0 ) ; }",tokWithStdLib(code));
        }

        {
            const char* code = "int f() {"
                               "switch (x) { case 1: exit(0); bar(); tack; { ticak(); exit(0); } exit(0);"
                               "case 2: switch(y) { case 1: exit(0); bar2(); foo(); case 2: exit(0); }"
                               "exit(0); } exit(0); }";
            const char* expected = "int f ( ) {"
                                   " switch ( x ) { case 1 : ; exit ( 0 ) ;"
                                   " case 2 : ; switch ( y ) { case 1 : ; exit ( 0 ) ; case 2 : ; exit ( 0 ) ; }"
                                   " exit ( 0 ) ; } exit ( 0 ) ; }";
            ASSERT_EQUALS(expected,tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    switch (i) { case 0: switch (j) { case 0: exit(0); }"
                               "        case 1: switch (j) { case -1: exit(0); }"
                               "        case 2: switch (j) { case -2: exit(0); }"
                               "        case 3: if (blah6) {exit(0);} break; } }";
            const char* expected = "void foo ( ) {"
                                   " switch ( i ) { case 0 : ; switch ( j ) { case 0 : ; exit ( 0 ) ; }"
                                   " case 1 : ; switch ( j ) { case -1 : ; exit ( 0 ) ; }"
                                   " case 2 : ; switch ( j ) { case -2 : ; exit ( 0 ) ; }"
                                   " case 3 : ; if ( blah6 ) { exit ( 0 ) ; } break ; } }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "void foo () {"
                               "    exit(0);"
                               "    switch (i) { case 0: switch (j) { case 0: foo(); }"
                               "        case 1: switch (j) { case -1: bar(); label:; ok(); }"
                               "        case 3: if (blah6) { boo(); break; } } }";
            const char* expected = "void foo ( ) { exit ( 0 ) ; { { label : ; ok ( ) ; } case 3 : ; if ( blah6 ) { boo ( ) ; break ; } } }";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char* code = "void foo() {"
                               "     switch ( t ) {"
                               "     case 0:"
                               "          if ( t ) switch ( b ) {}"
                               "          break;"
                               "     case 1:"
                               "          exit(0);"
                               "          return 0;"
                               "     }"
                               "     return 0;"
                               "}";
            const char* expected = "void foo ( ) {"
                                   " switch ( t ) {"
                                   " case 0 : ;"
                                   " if ( t ) { switch ( b ) { } }"
                                   " break ;"
                                   " case 1 : ;"
                                   " exit ( 0 ) ;"
                                   " }"
                                   " return 0 ; "
                                   "}";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    A *a = 0;\n"
                                "    if (!a) {\n"
                                "        nondeadcode;\n"
                                "        return;\n"
                                "        dead;\n"
                                "    }\n"
                                "    stilldead;\n"
                                "    a->_a;\n"
                                "}\n";
            const char expected[] = "void foo ( ) "
                                    "{"
                                    " A * a ; a = 0 ; {"
                                    " nondeadcode ;"
                                    " return ;"
                                    " } "
                                    "}";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char code[] = "class Fred\n"
                                "{\n"
                                "public:\n"
                                "    bool foo() const { return f; }\n"
                                "    bool exit();\n"
                                "\n"
                                "private:\n"
                                "   bool f;\n"
                                "};\n";
            const char expected[] = "class Fred "
                                    "{"
                                    " public:"
                                    " bool foo ( ) const { return f ; }"
                                    " bool exit ( ) ;"
                                    ""
                                    " private:"
                                    " bool f ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        {
            const char code[] = "class abort { };\n"
                                "\n"
                                "class Fred\n"
                                "{\n"
                                "    public:\n"
                                "    bool foo() const { return f; }\n"
                                "    abort exit();\n"
                                "\n"
                                "    private:\n"
                                "bool f;\n"
                                "};\n";
            const char expected[] = "class abort { } ; "
                                    "class Fred "
                                    "{"
                                    " public:"
                                    " bool foo ( ) const { return f ; }"
                                    " abort exit ( ) ;"
                                    ""
                                    " private:"
                                    " bool f ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tokWithStdLib(code));
        }

        ASSERT_EQUALS("void foo ( ) { exit ( 0 ) ; }",
                      tokWithStdLib("void foo() { do { exit(0); } while (true); }"));

        // #6187
        tokWithStdLib("void foo() {\n"
                      "  goto label;\n"
                      "  for (int i = 0; i < 0; ++i) {\n"
                      "    ;\n"
                      "label:\n"
                      "    ;\n"
                      "  }\n"
                      "}");
    }

    void strcat1() {
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
    void strcat2() {
        const char code[] = "; strcat(strcat(dst, foo[0]), \" \");";
        const char expect[] = "; "
                              "strcat ( dst , foo [ 0 ] ) ; "
                              "strcat ( dst , \" \" ) ;";

        ASSERT_EQUALS(expect, tok(code));
    }

    void simplifyAtol() {
        ASSERT_EQUALS("a = std :: atol ( x ) ;", tok("a = std::atol(x);"));
        ASSERT_EQUALS("a = atol ( \"text\" ) ;", tok("a = atol(\"text\");"));
        ASSERT_EQUALS("a = 0 ;", tok("a = std::atol(\"0\");"));
        ASSERT_EQUALS("a = 10 ;", tok("a = atol(\"0xa\");"));
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
        ASSERT_EQUALS("class TClass { "
                      "public: "
                      "TClass & operator= ( const TClass & rhs ) ; "
                      "} ; "
                      "TClass :: TClass ( const TClass & other ) "
                      "{ "
                      "operator= ( other ) ; "
                      "} class SharedPtr<Y> { "
                      "SharedPtr<Y> & operator= ( SharedPtr<Y> const & r ) ; "
                      "} ;",
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

    void reverseArraySyntax() {
        ASSERT_EQUALS("a [ 13 ]", tok("13[a]"));
    }

    void simplify_numeric_condition() {
        {
            const char code[] =
                "void f()\n"
                "{\n"
                "int x = 0;\n"
                "if( !x || 0 )\n"
                "{ g();\n"
                "}\n"
                "}";

            ASSERT_EQUALS("void f ( ) { g ( ) ; }", tok(code));
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

            ASSERT_EQUALS("void f ( ) { }", tok(code));
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

            ASSERT_EQUALS("void f ( ) { }", tok(code));
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

            ASSERT_EQUALS("void f ( ) { g ( ) ; }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    if (5==5);\n"
                                "}\n";

            ASSERT_EQUALS("void f ( ) { ; }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    if (4<5);\n"
                                "}\n";

            ASSERT_EQUALS("void f ( ) { ; }", tok(code));
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

            ASSERT_EQUALS("void f ( ) { ; }", tok(code));
        }
    }

    void simplify_condition() {
        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if (a && false) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { }", tok(code));
        }

        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if (false && a) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { }", tok(code));
        }

        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if (true || a) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { g ( ) ; }", tok(code));
        }

        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if (a || true) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { g ( ) ; }", tok(code));
        }

        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if (a || true || b) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { g ( ) ; }", tok(code));
        }

        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if (a && false && b) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { }", tok(code));
        }

        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if (a || (b && false && c) || d) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { if ( a || d ) { g ( ) ; } }", tok(code));
        }

        {
            const char code[] =
                "void f(int a)\n"
                "{\n"
                "if ((a && b) || true || (c && d)) g();\n"
                "}";
            ASSERT_EQUALS("void f ( int a ) { g ( ) ; }", tok(code));
        }
    }


    void pointeralias1() {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    char buf[100];\n"
                                "    char *p = buf;\n"
                                "    free(p);\n"
                                "}\n";

            const char expected[] = "void f ( ) "
                                    "{ "
                                    "char buf [ 100 ] ; "
                                    "free ( buf ) ; "
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

        {
            const char code[] = "void foo(Result* ptr)\n"
                                "{\n"
                                "    Result* obj = ptr;\n"
                                "    ++obj->total;\n"
                                "}\n";

            const char expected[] = "void foo ( Result * ptr ) "
                                    "{ "
                                    "Result * obj ; obj = ptr ; "
                                    "++ obj . total ; "
                                    "}";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "int *foo()\n"
                                "{\n"
                                "    int a[10];\n"
                                "    int *b = a;\n"
                                "    return b;\n"
                                "}\n";

            const char expected[] = "int * foo ( ) "
                                    "{ "
                                    "int a [ 10 ] ; "
                                    "return a ; "
                                    "}";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f() {\n"
                                "    int a[10];\n"
                                "    int *b = a;\n"
                                "    memset(b,0,sizeof(a));\n"
                                "}";

            const char expected[] = "void f ( ) {"
                                    " int a [ 10 ] ;"
                                    " memset ( a , 0 , 40 ) ; "
                                    "}";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void pointeralias2() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int i;\n"
                            "    int *p = &i;\n"
                            "    return *p;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { int i ; return i ; }", tok(code));
    }

    void pointeralias3() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int i, j, *p;\n"
                            "    if (ab) p = &i;\n"
                            "    else p = &j;\n"
                            "    *p = 0;\n"
                            "}\n";
        const char expected[] = "void f ( ) "
                                "{"
                                " int i ; int j ; int * p ;"
                                " if ( ab ) { p = & i ; }"
                                " else { p = & j ; }"
                                " * p = 0 ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void pointeralias4() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a[10];\n"
                            "    int *p = &a[0];\n"
                            "    *p = 0;\n"
                            "}\n";
        const char expected[] = "void f ( ) "
                                "{"
                                " int a [ 10 ] ;"
                                " * a = 0 ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void pointeralias5() {
        const char code[] = "int f()\n"
                            "{\n"
                            "    int i;\n"
                            "    int *p = &i;\n"
                            "    *p = 5;\n"
                            "    return i;\n"
                            "}\n";
        const char expected[] = "int f ( ) "
                                "{"
                                " return 5 ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void reduceConstness() {
        ASSERT_EQUALS("char * p ;", tok("char * const p;"));
    }

    void while0() {
        ASSERT_EQUALS("; x = 1 ;", tok("; do { x = 1 ; } while (0);"));
        ASSERT_EQUALS("; return 0 ;", tok("; do { return 0; } while (0);"));
        ASSERT_EQUALS("void foo ( ) { goto label ; }", tok("void foo() { do { goto label; } while (0); }"));
        ASSERT_EQUALS("; { continue ; }", tok("; do { continue ; } while (0);"));
        ASSERT_EQUALS("; { break ; }", tok("; do { break; } while (0);"));
        ASSERT_EQUALS(";", tok("; while (false) { a; }"));
        ASSERT_EQUALS(";", tok("; while (false) { switch (n) { case 0: return; default: break; } n*=1; }"));
    }

    void while0for() {
        // for (condition is always false)
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { int i; for (i = 0; i < 0; i++) { a; } }"));
        //ticket #3140
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { int i; for (i = 0; i < 0; i++) { foo(); break; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { int i; for (i = 0; i < 0; i++) { foo(); continue; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (int i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (unsigned int i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (long long i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (signed long long i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { int n = 0; for (signed long long i = 0; i < n; i++) { a; } }"));
    }

    void while1() {
        // ticket #1197
        const char code[] = "void do {} while (0) { }";
        const char expected[] = "void { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void enum1() {
        const char code[] = "enum A { a, b, c }; A c1 = c;";
        const char expected[] = "int c1 ; c1 = 2 ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void enum2() {
        const char code[] = "enum A { a, }; int array[a];";
        const char expected[] = "int array [ 0 ] ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void enum3() {
        const char code[] = "enum { a, }; int array[a];";
        const char expected[] = "int array [ 0 ] ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void enum4() {
        {
            const char code[] = "class A {\n"
                                "public:\n"
                                "    enum EA { a1, a2, a3 };\n"
                                "    EA get() const;\n"
                                "    void put(EA a) { ea = a; ea = a1; }\n"
                                "private:\n"
                                "    EA ea;\n"
                                "};\n"
                                "A::EA A::get() const { return ea; }\n"
                                "A::EA e = A::a1;";

            const char expected[] = "class A { "
                                    "public: "
                                    ""
                                    "int get ( ) const ; "
                                    "void put ( int a ) { ea = a ; ea = 0 ; } "
                                    "private: "
                                    "int ea ; "
                                    "} ; "
                                    "int A :: get ( ) const { return ea ; } "
                                    "int e ; e = 0 ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct A {\n"
                                "    enum EA { a1, a2, a3 };\n"
                                "    EA get() const;\n"
                                "    void put(EA a) { ea = a; ea = a1; }\n"
                                "    EA ea;\n"
                                "};\n"
                                "A::EA A::get() const { return ea; }\n"
                                "A::EA e = A::a1;";

            const char expected[] = "struct A { "
                                    ""
                                    "int get ( ) const ; "
                                    "void put ( int a ) { ea = a ; ea = 0 ; } "
                                    "int ea ; "
                                    "} ; "
                                    "int A :: get ( ) const { return ea ; } "
                                    "int e ; e = 0 ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void enum5() {
        const char code[] = "enum ABC {\n"
                            "    a = sizeof(int),\n"
                            "    b = 1 + a,\n"
                            "    c = b + 100,\n"
                            "    d,\n"
                            "    e,\n"
                            "    f = 90,\n"
                            "    g\n"
                            "};\n"
                            "int sum =  a + b + c + d + e + f + g;";
        const char expected[] = "int sum ; sum = "
                                "sizeof ( int ) + "
                                "( 1 + sizeof ( int ) ) + "
                                "( 1 + sizeof ( int ) + 100 ) + " // 101 = 100 + 1
                                "( 1 + sizeof ( int ) + 101 ) + " // 102 = 100 + 1 + 1
                                "( 1 + sizeof ( int ) + 102 ) + 181 " // 283 = 100+2+90+91
                                ";";

        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("int sum ; sum = 508 ;", tok(code, true));
    }

    void enum6() {
        const char code[] = "enum { a = MAC(A, B, C) }; void f(a) { }";
        const char expected[] = "void f ( a ) { }";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void enum7() {
        {
            // ticket 1388
            const char code[] = "enum FOO {A,B,C};\n"
                                "int main()\n"
                                "{\n"
                                "  int A = B;\n"
                                "  { float A = C; }\n"
                                "}";
            const char expected[] = "int main ( ) "
                                    "{ "
                                    "int A ; A = 1 ; "
                                    "{ float A ; A = 2 ; } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "enum FOO {A,B,C};\n"
                                "void f(int A, float B, char C) { }";
            const char expected[] = "void f ( int A , float B , char C ) { }";
            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    // Check simplifyEnum
    std::string checkSimplifyEnum(const char code[], bool cpp = true) {
        errout.str("");
        // Tokenize..
        Settings settings;
        settings.addEnabled("style");
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, cpp?"test.cpp":"test.c");
        return tokenizer.tokens()->stringifyList(0, true);
    }

    void enum8() {
        // ticket 1388
        checkSimplifyEnum("enum Direction {N=100,E,S,W,ALL};\n"
                          "template<class T,int S> class EF_Vector{\n"
                          "  T v_v[S];\n"
                          "\n"
                          "public:\n"
                          "  EF_Vector();\n"
                          "  explicit EF_Vector(const T &);\n"
                          "  explicit EF_Vector(const T arr[S]);\n"
                          "};\n"
                          "\n"
                          "template<class T,int S>\n"
                          "EF_Vector<T,S>::EF_Vector()\n"
                          "{\n"
                          "}\n"
                          "\n"
                          "template<class T,int S>\n"
                          "EF_Vector<T,S>::EF_Vector(const T &t)\n"
                          "{\n"
                          "  for(int i=0;i<S;i++)\n"
                          "    v_v[i]=t;\n"
                          "}\n"
                          "\n"
                          "template<class T,int S>\n"
                          "EF_Vector<T,S>::EF_Vector(const T arr[S])\n"
                          "{\n"
                          "  for(int i=0;i<S;i++)\n"
                          "    v_v[i]=arr[i];\n"
                          "}\n"
                          "\n"
                          "void initialize()\n"
                          "{\n"
                          "   EF_Vector<float,6> d;\n"
                          "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style) Template parameter 'S' hides enumerator with same name\n"
                      "[test.cpp:11] -> [test.cpp:1]: (style) Template parameter 'S' hides enumerator with same name\n"
                      "[test.cpp:16] -> [test.cpp:1]: (style) Template parameter 'S' hides enumerator with same name\n"
                      "[test.cpp:23] -> [test.cpp:1]: (style) Template parameter 'S' hides enumerator with same name\n", errout.str());
    }

    void enum9() {
        // ticket 1404
        checkSimplifyEnum("class XX {\n"
                          "public:\n"
                          "static void Set(const int &p){m_p=p;}\n"
                          "static int m_p;\n"
                          "};\n"
                          "int XX::m_p=0;\n"
                          "int main() {\n"
                          "  enum { XX };\n"
                          "  XX::Set(std::numeric_limits<X>::digits());\n"
                          "}");
        ASSERT_EQUALS("", errout.str());
    }

    void enum10() {
        // ticket 1445
        const char code[] = "enum {\n"
                            "SHELL_SIZE = sizeof(union { int i; char *cp; double d; }) - 1,\n"
                            "} e = SHELL_SIZE;";
        const char expected[] = "int e ; e = sizeof ( union { int i ; char * cp ; double d ; } ) - 1 ;";
        ASSERT_EQUALS(expected, checkSimplifyEnum(code));

        ASSERT_EQUALS("", errout.str());
    }

    void enum11() {
        const char code[] = "int main()\n"
                            "{\n"
                            "  enum { u, v };\n"
                            "  A u = 1, v = 2;\n"
                            "}";
        const char expected[] = "int main ( ) "
                                "{ "
                                ""
                                "A u ; u = 1 ; A v ; v = 2 ; "
                                "}";
        ASSERT_EQUALS(expected, checkSimplifyEnum(code));

        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3]: (style) Variable 'u' hides enumerator with same name\n"
                      "[test.cpp:4] -> [test.cpp:3]: (style) Variable 'v' hides enumerator with same name\n", errout.str());
    }

    void enum12() {
        const char code[] = "enum fred { a, b };\n"
                            "void foo()\n"
                            "{\n"
                            "    unsigned int fred = 0;\n"
                            "}";
        const char expected[] = "void foo ( ) { unsigned int fred ; fred = 0 ; }";
        ASSERT_EQUALS(expected, checkSimplifyEnum(code));
    }

    void enum13() {
        const char code[] = "enum ab { ENTRY(1, a = 0), ENTRY(2, b) };\n"
                            "void foo()\n"
                            "{\n"
                            "    unsigned int fred = a;\n"
                            "}";
        const char expected[] = "void foo ( ) { unsigned int fred ; fred = a ; }";
        ASSERT_EQUALS(expected, checkSimplifyEnum(code));
    }

    void enum14() {
        const char code[] = "enum ab { a };\n"
                            "ab";
        const char expected[] = "ab";
        ASSERT_EQUALS(expected, checkSimplifyEnum(code));
    }

    void enum15() { // C++0x features
        {
            const char code[] = "enum : char { a = 99 };\n"
                                "char c1 = a;";
            const char expected[] = "char c1 ; c1 = 99 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class Enum1 { a };\n"
                                "Enum1 e1 = Enum1::a;";
            const char expected[] = "int e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class Enum1 { a };\n"
                                "Enum1 e1 = a;";
            const char expected[] = "int e1 ; e1 = a ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum Enum1 : char { a };\n"
                                "Enum1 e1 = a;";
            const char expected[] = "char e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class Enum1 : unsigned char { a };\n"
                                "Enum1 e1 = Enum1::a;";
            const char expected[] = "unsigned char e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class Enum1 : unsigned int { a };\n"
                                "Enum1 e1 = Enum1::a;";
            const char expected[] = "unsigned int e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class Enum1 : unsigned long long int { a };\n"
                                "Enum1 e1 = Enum1::a;";
            const char expected[] = "unsigned long long e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class { A };\n"
                                "int i = A;";
            const char expected [] = "int i ; i = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code, false)); // Compile as C code: enum has name 'class'
            checkSimplifyEnum(code, true); // Compile as C++ code: Don't crash
        }
    }

    void enum16() { // ticket #1988
        const char code[] = "enum D : auto * { FF = 0 };";
        ASSERT_THROW(checkSimplifyEnum(code), InternalError);
    }

    void enum17() { // ticket #2381
        // if header is included twice its enums will be duplicated
        const char code[] = "enum ab { a=0, b };"
                            "enum ab { a=0, b };\n";
        ASSERT_EQUALS(";", checkSimplifyEnum(code));
        ASSERT_EQUALS("", errout.str());
    }

    void enum18() { // ticket #2466 - array with same name as enum constant
        const char code[] = "enum ab { a=0, b };\n"
                            "void f() { a[0]; }\n";
        ASSERT_EQUALS("void f ( ) { a [ 0 ] ; }", checkSimplifyEnum(code));
    }

    void enum19() { // ticket #2536
        const char code[] = "enum class E1;\n"
                            "enum class E2 : int;\n";
        ASSERT_EQUALS(";", checkSimplifyEnum(code));
    }

    void enum20() { // ticket #2600 segmentation fault
        const char code[] = "enum { const }\n";
        ASSERT_EQUALS("", checkSimplifyEnum(code));
    }

    void enum21() { // ticket #2720 syntax error
        const char code[] = "enum E2 : signed const short { };\n";
        ASSERT_EQUALS(";", checkSimplifyEnum(code));
        ASSERT_EQUALS("", errout.str());
    }

    void enum22() { // ticket #2745
        const char code[] = "enum en { x = 0 };\n"
                            "void f() {\n"
                            "    int x = 0;\n"
                            "    g(x);\n"
                            "}\n"
                            "void f2(int &x) {\n"
                            "    x+=1;\n"
                            "}\n";
        checkSimplifyEnum(code);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (style) Variable 'x' hides enumerator with same name\n"
                      "[test.cpp:6] -> [test.cpp:1]: (style) Function argument 'x' hides enumerator with same name\n",
                      errout.str());

        // avoid false positive: in other scope
        const char code2[] = "class C1 { enum en { x = 0 }; };\n"
                             "class C2 { bool x; };\n";
        checkSimplifyEnum(code2);
        ASSERT_EQUALS("", errout.str());

        // avoid false positive: inner if-scope
        const char code3[] = "enum en { x = 0 };\n"
                             "void f() { if (aa) ; else if (bb==x) df; }\n";
        checkSimplifyEnum(code3);
        ASSERT_EQUALS("", errout.str());
    }

    void enum23() { // ticket #2804
        const char code[] = "enum Enumerator : std::uint8_t { ITEM1, ITEM2, ITEM3 };\n"
                            "Enumerator e = ITEM3;\n";
        const char expected[] = "std :: uint8_t e ; e = 2 ;";
        ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        ASSERT_EQUALS("", errout.str());
    }

    void enum24() { // ticket #2828
        const char code[] = "enum EnumName { STYLE = 0x0001 };\n"
                            "void f(long style) {\n"
                            "    if (style & STYLE) { }\n"
                            "}\n";
        const char expected[] = "void f ( long style ) { if ( style & 1 ) { } }";
        ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        ASSERT_EQUALS("", errout.str());
    }

    void enum25() { // ticket #2966 (segmentation fault)
        const char code[] = "enum x :\n";
        ASSERT_THROW(checkSimplifyEnum(code), InternalError);
    }

    void enum26() { // ticket #2975 (segmentation fault)
        const char code[] = "enum E {} e enum\n";
        checkSimplifyEnum(code);
        ASSERT_EQUALS("", errout.str());
    }

    void enum27() { // ticket #3005 (segmentation fault)
        const char code[] = "enum : x\n";
        ASSERT_THROW(checkSimplifyEnum(code), InternalError);
    }

    void enum28() {
        const char code[] = "enum { x=0 };\n"
                            "void f() { char x[4];  memset(x, 0, 4);\n"
                            "{ x } };\n"
                            "void g() { x; }";
        ASSERT_EQUALS("void f ( ) { char x [ 4 ] ; memset ( x , 0 , 4 ) ; { x } } ; void g ( ) { 0 ; }", checkSimplifyEnum(code));
    }

    void enum29() {  // #3747 - bitwise or value
        const char code[] = "enum { x=1, y=x|2 }; i = (3==y);";
        ASSERT_EQUALS("i = 3 == 3 ;", checkSimplifyEnum(code));
    }

    void enum30() { // #3852 - false positive
        const char code [] = "class TestIf\n"
                             "{\n"
                             "public:\n"
                             "    enum class Foo\n"
                             "    {\n"
                             "      one = 0,\n"
                             "      two = 1\n"
                             "    };\n"
                             "    enum class Bar\n"
                             "    {\n"
                             "      one = 0,\n"
                             "      two = 1\n"
                             "    };\n"
                             "};\n"
                             "int main() {"
                             "    return TestIf::Bar::two;\n"
                             "}";
        ASSERT_EQUALS("class TestIf { public: } ; int main ( ) { return 1 ; }", checkSimplifyEnum(code));
        ASSERT_EQUALS("", errout.str());
    }

    void enum31() {  // #3934 - calculation in first item
        const char code[] = "enum { x=2*32, y }; i = y;";
        ASSERT_EQUALS("i = 65 ;", checkSimplifyEnum(code));
    }

    void enum32() {  // #3998 - wrong enum simplification => access violation
        const char code[] = "enum { x=(32), y=x, z }; { a, z }";
        ASSERT_EQUALS("{ a , ( 33 ) }", checkSimplifyEnum(code));
    }

    void enum33() {  // #4015 - segmentation fault
        const char code[] = "enum { A=SOME_VALUE, B=A };";
        ASSERT_EQUALS(";", checkSimplifyEnum(code));
    }

    void enum34() {  // #4141 - division by zero
        const char code[] = "enum { A=1/0 };";
        ASSERT_EQUALS(";", checkSimplifyEnum(code));
    }

    void enum35() {  // #3953 - avoid simplification of type
        ASSERT_EQUALS("void f ( A * a ) ;", checkSimplifyEnum("enum { A }; void f(A * a) ;"));
        ASSERT_EQUALS("void f ( A * a ) { }", checkSimplifyEnum("enum { A }; void f(A * a) { }"));
    }

    void enum36() {  // #4378
        const char code[] = "struct X { enum Y { a, b }; X(Y) { Y y = (Y)1; } };";
        ASSERT_EQUALS("struct X { X ( int ) { int y ; y = ( int ) 1 ; } } ;", checkSimplifyEnum(code));
    }

    void enum37() {  // #4280 - shadow variables
        const char code1[] = "enum { a, b }; void f(int a) { return a + 1; }";
        ASSERT_EQUALS("void f ( int a ) { return a + 1 ; }", checkSimplifyEnum(code1));

        const char code2[] = "enum { a, b }; void f() { int a; }";
        ASSERT_EQUALS("void f ( ) { int a ; }", checkSimplifyEnum(code2));

        const char code3[] = "enum { a, b }; void f() { int *a=do_something(); }";
        ASSERT_EQUALS("void f ( ) { int * a ; a = do_something ( ) ; }", checkSimplifyEnum(code3));

        const char code4[] = "enum { a, b }; void f() { int &a=x; }";
        ASSERT_EQUALS("void f ( ) { int & a = x ; }", checkSimplifyEnum(code4));

        // #4857 - not shadow variable
        checkSimplifyEnum("enum { a,b }; void f() { if (x) { } else if ( x & a ) {} }");
        ASSERT_EQUALS("", errout.str());
    }

    void enum38() { // #4463
        const char code[] = "enum { a,b }; void f() { throw a; }";
        checkSimplifyEnum(code);
        ASSERT_EQUALS("", errout.str());
    }

    void enum39() { // #5145 - fp variable hides enum
        const char code[] = "enum { A }; void f() { int a = 1 * A; }";
        checkSimplifyEnum(code);
        ASSERT_EQUALS("", errout.str());
    }

    void enum40() {
        const char code[] = "enum { A=(1<<0)|(1<<1) }; void f() { x = y + A; }";
        ASSERT_EQUALS("void f ( ) { x = y + ( 3 ) ; }", checkSimplifyEnum(code));
    }

    void enum41() { // ticket #5212 (valgrind errors during enum simplification)
        const char code[] = "namespace Foo {\n"
                            "  enum BarConfig {\n"
                            "    eBitOne = (1 << 0),\n"
                            "    eBitTwo = (1 << 1),\n"
                            "    eAll        = eBitOne|eBitTwo\n"
                            "  };\n"
                            "}\n"
                            "int x = Foo::eAll;";
        ASSERT_EQUALS("int x ; x = ( 1 ) | 2 ;", checkSimplifyEnum(code));
    }

    void enum42() { // ticket #5182 (template function call in template value)
        const char code[] = "enum { A = f<int,2>() };\n"
                            "a = A;";
        ASSERT_EQUALS("a = f < int , 2 > ( ) ;", checkSimplifyEnum(code));
    }

    void enum43() { // lhs in assignment
        const char code[] = "enum { A, B };\n"
                            "A = 1;";
        ASSERT_EQUALS("A = 1 ;", checkSimplifyEnum(code));
    }

    void enumscope1() { // #3949 - don't simplify enum from one function in another function
        const char code[] = "void foo() { enum { A = 0, B = 1 }; }\n"
                            "void bar() { int a = A; }";
        ASSERT_EQUALS("void foo ( ) { } void bar ( ) { int a ; a = A ; }", checkSimplifyEnum(code));
    }

    void duplicateDefinition() { // #3565 - wrongly detects duplicate definition
        const Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr("x ; return a not_eq x;");
        tokenizer.tokenize(istr, "test.c");
        Token *x_token = tokenizer.list.front()->tokAt(5);
        ASSERT_EQUALS(false, tokenizer.duplicateDefinition(&x_token, tokenizer.tokens()));
    }

    void invalid_enum() { // #5600: missing include causes invalid enum
        const char code [] = "enum {\n"
                             "    NUM_OPCODES = \n"
                             // #include "definition"
                             "};\n"
                             "struct bytecode {};\n"
                             "jv jq_next() { opcode = ((opcode) +NUM_OPCODES);\n"
                             "}";
        ASSERT_THROW(checkSimplifyEnum(code), InternalError);
    }

    void removestd() {
        ASSERT_EQUALS("; strcpy ( a , b ) ;", tok("; std::strcpy(a,b);"));
        ASSERT_EQUALS("; strcat ( a , b ) ;", tok("; std::strcat(a,b);"));
        ASSERT_EQUALS("; strncpy ( a , b , 10 ) ;", tok("; std::strncpy(a,b,10);"));
        ASSERT_EQUALS("; strncat ( a , b , 10 ) ;", tok("; std::strncat(a,b,10);"));
        ASSERT_EQUALS("; free ( p ) ;", tok("; std::free(p);"));
        ASSERT_EQUALS("; malloc ( 10 ) ;", tok("; std::malloc(10);"));
    }

    void simplifyInitVar() {
        // ticket #1005 - int *p(0); => int *p = 0;
        {
            const char code[] = "void foo() { int *p(0); }";
            ASSERT_EQUALS("void foo ( ) { }", tok(code));
        }

        {
            const char code[] = "void foo() { int p(0); }";
            ASSERT_EQUALS("void foo ( ) { }", tok(code));
        }

        {
            const char code[] = "void a() { foo *p(0); }";
            ASSERT_EQUALS("void a ( ) { }", tok(code));
        }
    }

    void simplifyReference() {
        ASSERT_EQUALS("void f ( ) { int a ; a ++ ; }",
                      tok("void f() { int a; int &b(a); b++; }"));
        ASSERT_EQUALS("void f ( ) { int a ; a ++ ; }",
                      tok("void f() { int a; int &b = a; b++; }"));

        ASSERT_EQUALS("void test ( ) { c . f ( 7 ) ; }",
                      tok("void test() { c.f(7); T3 &t3 = c; }")); // #6133
    }

    void simplifyRealloc() {
        ASSERT_EQUALS("; free ( p ) ; p = 0 ;", tok("; p = realloc(p, 0);"));
        ASSERT_EQUALS("; p = malloc ( 100 ) ;", tok("; p = realloc(0, 100);"));
        ASSERT_EQUALS("; p = malloc ( 0 ) ;", tok("; p = realloc(0, 0);"));
        ASSERT_EQUALS("; free ( q ) ; p = 0 ;", tok("; p = realloc(q, 0);"));
        ASSERT_EQUALS("; free ( * q ) ; p = 0 ;", tok("; p = realloc(*q, 0);"));
        ASSERT_EQUALS("; free ( f ( z ) ) ; p = 0 ;", tok("; p = realloc(f(z), 0);"));
        ASSERT_EQUALS("; p = malloc ( n * m ) ;", tok("; p = realloc(0, n*m);"));
        ASSERT_EQUALS("; p = malloc ( f ( 1 ) ) ;", tok("; p = realloc(0, f(1));"));
    }

    void simplifyErrNoInWhile() {
        ASSERT_EQUALS("; while ( f ( ) ) { }",
                      tok("; while (f() && errno == EINTR) { }"));
        ASSERT_EQUALS("; while ( f ( ) ) { }",
                      tok("; while (f() && (errno == EINTR)) { }"));
    }

    void simplifyFuncInWhile() {
        ASSERT_EQUALS("int cppcheck:r1 = fclose ( f ) ; "
                      "while ( cppcheck:r1 ) "
                      "{ "
                      "foo ( ) ; "
                      "cppcheck:r1 = fclose ( f ) ; "
                      "}",
                      tok("while(fclose(f))foo();"));

        ASSERT_EQUALS("int cppcheck:r1 = fclose ( f ) ; "
                      "while ( cppcheck:r1 ) "
                      "{ "
                      "; cppcheck:r1 = fclose ( f ) ; "
                      "}",
                      tok("while(fclose(f));"));

        ASSERT_EQUALS("int cppcheck:r1 = fclose ( f ) ; "
                      "while ( cppcheck:r1 ) "
                      "{ "
                      "; cppcheck:r1 = fclose ( f ) ; "
                      "} "
                      "int cppcheck:r2 = fclose ( g ) ; "
                      "while ( cppcheck:r2 ) "
                      "{ "
                      "; cppcheck:r2 = fclose ( g ) ; "
                      "}",
                      tok("while(fclose(f)); while(fclose(g));"));
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

    void simplifyStructDecl5() {
        const char code[] = "<class T>\n"
                            "{\n"
                            "    struct {\n"
                            "        typename D4:typename Base<T*>\n"
                            "    };\n"
                            "};\n";
        //don't crash
        tok(code, false);
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

    void removeUnwantedKeywords() {
        ASSERT_EQUALS("int var ;", tok("register int var ;", true));
        ASSERT_EQUALS("short var ;", tok("register short int var ;", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("inline int foo ( ) { }", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("__inline int foo ( ) { }", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("__forceinline int foo ( ) { }", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("constexpr int foo() { }", true));
        ASSERT_EQUALS("class C { int f ( ) ; } ;", tok("class C { int f() override ; };", true));
        ASSERT_EQUALS("class C { int f ( ) ; } ;", tok("class C { int f() final ; };", true));
        ASSERT_EQUALS("void f ( ) { int final [ 10 ] ; }", tok("void f() { int final[10]; }", true));
        ASSERT_EQUALS("if ( a ) { }", tok("if ( likely ( a ) ) { }", true));
        ASSERT_EQUALS("if ( a ) { }", tok("if ( unlikely ( a ) ) { }", true));
        ASSERT_EQUALS("int * p ;", tok("int * __restrict p;", "test.c"));
        ASSERT_EQUALS("int * * p ;", tok("int * __restrict__ * p;", "test.c"));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * __restrict__ a, float * __restrict__ b);", "test.c"));
        ASSERT_EQUALS("int * p ;", tok("int * restrict p;", "test.c"));
        ASSERT_EQUALS("int * * p ;", tok("int * restrict * p;", "test.c"));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * restrict a, float * restrict b);", "test.c"));
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
        ASSERT_EQUALS("int f ( ) ;", tok("int WINAPI f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int APIENTRY f();", true));
        ASSERT_EQUALS("int f ( ) ;", tok("int CALLBACK f();", true));
    }

    void simplifyFunctorCall() {
        ASSERT_EQUALS("IncrementFunctor ( ) ( a ) ;", tok("IncrementFunctor()(a);", true));
    }

    // #ticket #5339 (simplify function pointer after comma)
    void simplifyFunctionPointer() {
        ASSERT_EQUALS("f ( double x , double * y ) ;", tok("f (double x, double (*y) ());", true));
    }

    void redundant_semicolon() {
        ASSERT_EQUALS("void f ( ) { ; }", tok("void f() { ; }", false));
        ASSERT_EQUALS("void f ( ) { ; }", tok("void f() { do { ; } while (0); }", true));
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
    }

    void removeVoidFromFunction() {
        ASSERT_EQUALS("void foo ( ) ;", tok("void foo(void);"));
    }

    void removeUnnecessaryQualification1() {
        const char code[] = "class Fred { Fred::Fred() {} };";
        const char expected[] = "class Fred { Fred ( ) { } } ;";
        ASSERT_EQUALS(expected, tok(code, false));
        ASSERT_EQUALS("[test.cpp:1]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification2() {
        const char code[] = "template<typename Iter, typename Skip>\n"
                            "struct grammar : qi::grammar<Iter, int(), Skip> {\n"
                            "    grammar() : grammar::base_type(start) { }\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void removeUnnecessaryQualification3() {
        const char code[] = "namespace one {\n"
                            "   class c {\n"
                            "   public:\n"
                            "      void   function() {}\n"
                            "   };\n"
                            "}\n"
                            "namespace two {\n"
                            "   class c : public one::c {\n"
                            "   public:\n"
                            "      void   function() {\n"
                            "         one::c::function();\n"
                            "      }\n"
                            "   };\n"
                            "}\n";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void removeUnnecessaryQualification4() {
        const char code[] = "namespace one {\n"
                            "   class c {\n"
                            "   public:\n"
                            "      void   function() {}\n"
                            "   };\n"
                            "}\n"
                            "class c : public one::c {\n"
                            "public:\n"
                            "   void   function() {\n"
                            "      one::c::function();\n"
                            "   }\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void removeUnnecessaryQualification5() {
        const char code[] = "namespace one {\n"
                            "   class c {\n"
                            "   public:\n"
                            "      void   function() {}\n"
                            "   };\n"
                            "}\n"
                            "namespace two {\n"
                            "   class c : public one::c {\n"
                            "   public:\n"
                            "      void   function() {\n"
                            "         two::c::function();\n"
                            "      }\n"
                            "   };\n"
                            "}\n";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void removeUnnecessaryQualification6() {
        const char code[] = "namespace NS {\n"
                            "    int HRDF_bit() { return 1; }\n"
                            "    void HRDF_bit_set() { }\n"
                            "    void func(int var) {\n"
                            "        if (!NS::HRDF_bit())\n"
                            "            return;\n"
                            "        else\n"
                            "            NS::HRDF_bit_set();\n"
                            "    }\n"
                            "}\n";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void removeUnnecessaryQualification7() { // ticket #2970
        const char code[] = "class TProcedure {\n"
                            "public:\n"
                            "    TProcedure::TProcedure(long endAddress) : m_lEndAddr(endAddress){}\n"
                            "private:\n"
                            "    long m_lEndAddr;\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'TProcedure::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification8() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    Fred & Fred::operator = (const Fred &);\n"
                            "    void Fred::operator () (void);\n"
                            "    void Fred::operator delete[](void* x);\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n"
                      "[test.cpp:4]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n"
                      "[test.cpp:5]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification9() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    Fred::~Fred();\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification10() {
        const char code[] = "template<typename T> class A\n"
                            "{\n"
                            "    operator T();\n"
                            "    A() { T (A::*f)() = &A::operator T; }\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void removeUnnecessaryQualification11() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    Fred& Fred::Magic();\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification12() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    Fred* Fred::Magic();\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification13() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    Fred** Fred::Magic();\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification14() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    Fred*& Fred::Magic();\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void removeUnnecessaryQualification15() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    Fred*& Magic() {\n"
                            "        Fred::Magic(param);\n"
                            "    }\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void removeUnnecessaryQualification16() {
        const char code[] = "class Fred {\n"
                            "public:\n"
                            "    void Fred::Magic();\n"
                            "};\n";
        tok(code, false);
        ASSERT_EQUALS("[test.cpp:3]: (portability) The extra qualification 'Fred::' is unnecessary and is considered an error by many compilers.\n", errout.str());
    }

    void simplifyVarDecl1() { // ticket # 2682 segmentation fault
        const char code[] = "x a[0] =";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyVarDecl2() { // ticket # 2834 segmentation fault
        const char code[] = "std::vector<int>::iterator";
        tok(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void return_strncat() {
        {
            const char code[] = "char *f()\n"
                                "{\n"
                                "    char *temp=malloc(2);\n"
                                "    strcpy(temp,\"\");\n"
                                "    return (strncat(temp,\"a\",1));\n"
                                "}";
            ASSERT_EQUALS("char * f ( ) {"
                          " char * temp ;"
                          " temp = malloc ( 2 ) ;"
                          " strcpy ( temp , \"\" ) ;"
                          " strncat ( temp , \"a\" , 1 ) ;"
                          " return temp ; "
                          "}", tok(code, true));
        }
        {
            const char code[] = "char *f()\n"
                                "{\n"
                                "    char **temp=malloc(8);\n"
                                "    *temp = malloc(2);\n"
                                "    strcpy(*temp,\"\");\n"
                                "    return (strncat(*temp,\"a\",1));\n"
                                "}";
            ASSERT_EQUALS("char * f ( ) {"
                          " char * * temp ;"
                          " temp = malloc ( 8 ) ;"
                          " * temp = malloc ( 2 ) ;"
                          " strcpy ( * temp , \"\" ) ;"
                          " strncat ( * temp , \"a\" , 1 ) ;"
                          " return * temp ; "
                          "}", tok(code, true));
        }
        {
            const char code[] = "char *f()\n"
                                "{\n"
                                "    char **temp=malloc(8);\n"
                                "    *temp = malloc(2);\n"
                                "    strcpy(*temp,\"\");\n"
                                "    return (strncat(temp[0],foo(b),calc(c-d)));\n"
                                "}";
            ASSERT_EQUALS("char * f ( ) {"
                          " char * * temp ;"
                          " temp = malloc ( 8 ) ;"
                          " * temp = malloc ( 2 ) ;"
                          " strcpy ( * temp , \"\" ) ;"
                          " strncat ( temp [ 0 ] , foo ( b ) , calc ( c - d ) ) ;"
                          " return temp [ 0 ] ; "
                          "}", tok(code, true));
        }
    }

    void removeRedundantFor() { // ticket #3069
        {
            const char code[] = "void f() {"
                                "    for(x=0;x<1;x++) {"
                                "        y = 1;"
                                "    }"
                                "}";
            ASSERT_EQUALS("void f ( ) { { y = 1 ; } x = 1 ; }", tok(code, true));
        }

        {
            const char code[] = "void f() {"
                                "    for(x=0;x<1;x++) {"
                                "        y = 1 + x;"
                                "    }"
                                "}";
            ASSERT_EQUALS("void f ( ) { x = 0 ; { y = 1 + x ; } x = 1 ; }", tok(code, true));
        }

        {
            const char code[] = "void f() {"
                                "    foo();"
                                "    for(int x=0;x<1;x++) {"
                                "        y = 1 + x;"
                                "    }"
                                "}";
            ASSERT_EQUALS("void f ( ) { foo ( ) ; { int x = 0 ; y = 1 + x ; } }", tok(code, true));
        }
    }

    void consecutiveBraces() {
        ASSERT_EQUALS("void f ( ) { }", tok("void f(){{}}", true));
        ASSERT_EQUALS("void f ( ) { }", tok("void f(){{{}}}", true));
        ASSERT_EQUALS("void f ( ) { for ( ; ; ) { } }", tok("void f () { for(;;){} }", true));
        ASSERT_EQUALS("void f ( ) { { scope_lock lock ; foo ( ) ; } { scope_lock lock ; bar ( ) ; } }", tok("void f () { {scope_lock lock; foo();} {scope_lock lock; bar();} }", true));
    }

    void undefinedSizeArray() {
        ASSERT_EQUALS("int * x ;", tok("int x [];"));
        ASSERT_EQUALS("int * * x ;", tok("int x [][];"));
        ASSERT_EQUALS("int * * x ;", tok("int * x [];"));
        ASSERT_EQUALS("int * * * x ;", tok("int * x [][];"));
        ASSERT_EQUALS("int * * * * x ;", tok("int * * x [][];"));
        ASSERT_EQUALS("void f ( int x [ ] , double y [ ] ) { }", tok("void f(int x[], double y[]) { }"));
        ASSERT_EQUALS("int x [ 13 ] = { [ 11 ] = 2 , [ 12 ] = 3 } ;", tok("int x[] = {[11]=2, [12]=3};"));
    }

    void simplifyArrayAddress() { // ticket #3304
        const char code[] = "void foo() {\n"
                            "    int a[10];\n"
                            "    memset(&a[4], 0, 20*sizeof(int));\n"
                            "}";
        ASSERT_EQUALS("void foo ( ) {"
                      " int a [ 10 ] ;"
                      " memset ( a + 4 , 0 , 80 ) ;"
                      " }", tok(code, true));

        // Don't crash
        tok("int", true);
    }

    void simplifyCharAt() { // ticket #4481
        ASSERT_EQUALS("'h' ;", tok("\"hello\"[0] ;"));
        ASSERT_EQUALS("'\n' ;", tok("\"\n\"[0] ;"));
        ASSERT_EQUALS("'\\0' ;", tok("\"hello\"[5] ;"));
        ASSERT_EQUALS("'\\0' ;", tok("\"\"[0] ;"));
        ASSERT_EQUALS("'\\0' ;", tok("\"\\0\"[0] ;"));
        ASSERT_EQUALS("'\\n' ;", tok("\"hello\\nworld\"[5] ;"));
        ASSERT_EQUALS("'w' ;", tok("\"hello\nworld\"[6] ;"));
        ASSERT_EQUALS("\"hello\" [ 7 ] ;", tok("\"hello\"[7] ;"));
        ASSERT_EQUALS("\"hello\" [ -1 ] ;", tok("\"hello\"[-1] ;"));
    }

    void test_4881() {
        const char code[] = "int evallex() {\n"
                            "  int c, t;\n"
                            "again:\n"
                            "   do {\n"
                            "      if ((c = macroid(c)) == EOF_CHAR || c == '\n') {\n"
                            "      }\n"
                            "   } while ((t = type[c]) == LET && catenate());\n"
                            "}\n";
        ASSERT_EQUALS("int evallex ( ) { int c ; int t ; again : ; do { c = macroid ( c ) ; if ( c == EOF_CHAR || c == '\n' ) { } t = type [ c ] ; } while ( t == LET && catenate ( ) ) ; }",
                      tok(code, true));
    }

    void simplifyOverride() { // ticket #5069
        const char code[] = "void fun() {\n"
                            "    unsigned char override[] = {0x01, 0x02};\n"
                            "    doSomething(override, sizeof(override));\n"
                            "}\n";
        ASSERT_EQUALS("void fun ( ) { char override [ 2 ] = { 1 , 2 } ; doSomething ( override , 2 ) ; }",
                      tok(code, true));
    }
};

REGISTER_TEST(TestSimplifyTokens)
