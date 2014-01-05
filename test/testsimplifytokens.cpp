/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "templatesimplifier.h"
#include "path.h"

#include <sstream>
#include <list>

extern std::ostringstream errout;


class TestSimplifyTokens : public TestFixture {
public:
    TestSimplifyTokens() : TestFixture("TestSimplifyTokens") {
    }


private:

    void run() {
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
        TEST_CASE(parenthesesVar);      // Remove redundant parentheses around variable .. "( %var% )"
        TEST_CASE(declareVar);

        TEST_CASE(declareArray);

        TEST_CASE(dontRemoveIncrement);
        TEST_CASE(removePostIncrement);
        TEST_CASE(removePreIncrement);

        TEST_CASE(elseif1);
        TEST_CASE(ifa_ifa);     // "if (a) { if (a) .." => "if (a) { if (1) .."

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
        TEST_CASE(strlen2);

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
        TEST_CASE(template24);  // #2648 - using sizeof in template parameter
        TEST_CASE(template25);  // #2648 - another test for sizeof template parameter
        TEST_CASE(template26);  // #2721 - passing 'char[2]' as template parameter
        TEST_CASE(template27);  // #3350 - removing unused template in macro call
        TEST_CASE(template28);
        TEST_CASE(template29);  // #3449 - don't crash for garbage code
        TEST_CASE(template30);  // #3529 - template < template < ..
        TEST_CASE(template31);  // #4010 - reference type
        TEST_CASE(template32);  // #3818 - mismatching template not handled well
        TEST_CASE(template33);  // #3818,#4544 - inner templates in template instantiation not handled well
        TEST_CASE(template34);  // #3706 - namespace => hang
        TEST_CASE(template35);  // #4074 - A<'x'> a;
        TEST_CASE(template36);  // #4310 - passing unknown template instantiation as template argument
        TEST_CASE(template37);  // #4544 - A<class B> a;
        TEST_CASE(template38);  // #4832 - crash on C++11 right angle brackets
        TEST_CASE(template39);  // #4742 - freeze
        TEST_CASE(template40);  // #5055 - template specialization outside struct
        TEST_CASE(template41);  // #4710 - const in instantiation not handled perfectly
        TEST_CASE(template42);  // #4878 - variadic templates
        TEST_CASE(template43);  // #5097 - assert due to '>>' not treated as end of template instantiation
        TEST_CASE(template_unhandled);
        TEST_CASE(template_default_parameter);
        TEST_CASE(template_default_type);
        TEST_CASE(template_typename);
        TEST_CASE(template_constructor);    // #3152 - template constructor is removed

        // Test TemplateSimplifier::templateParameters
        TEST_CASE(templateParameters);
        TEST_CASE(templateParameters1);  // #4169 - segmentation fault

        TEST_CASE(namespaces);

        // Assignment in condition..
        TEST_CASE(ifassign1);
        TEST_CASE(ifAssignWithCast);
        TEST_CASE(whileAssign1);
        TEST_CASE(whileAssign2);
        TEST_CASE(whileAssign3); // varid
        TEST_CASE(doWhileAssign); // varid
        TEST_CASE(test_4881); // similar to doWhileAssign (#4911), taken from #4881 with full code

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
        TEST_CASE(simplifyConditionOperator);

        // Simplify calculations
        TEST_CASE(calculations);
        TEST_CASE(comparisons);

        // Simplify goto..
        TEST_CASE(goto1);
        TEST_CASE(goto2);
        TEST_CASE(goto3);  // #3138
        TEST_CASE(goto4);  // #3459
        TEST_CASE(goto5);  // #3705 - return ({asm("");});
        TEST_CASE(goto6);

        //remove dead code after flow control statements
        TEST_CASE(flowControl);

        // Simplify nested strcat() calls
        TEST_CASE(strcat1);
        TEST_CASE(strcat2);

        TEST_CASE(simplifyAtol)

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
        TEST_CASE(simplifyTypedef84); // ticket #2630
        TEST_CASE(simplifyTypedef85); // ticket #2651
        TEST_CASE(simplifyTypedef86); // ticket #2581
        TEST_CASE(simplifyTypedef87); // ticket #2651
        TEST_CASE(simplifyTypedef88); // ticket #2675
        TEST_CASE(simplifyTypedef89); // ticket #2717
        TEST_CASE(simplifyTypedef90); // ticket #2718
        TEST_CASE(simplifyTypedef91); // ticket #2716
        TEST_CASE(simplifyTypedef92); // ticket #2736
        TEST_CASE(simplifyTypedef93); // ticket #2738
        TEST_CASE(simplifyTypedef94); // ticket #1982
        TEST_CASE(simplifyTypedef95); // ticket #2844
        TEST_CASE(simplifyTypedef96); // ticket #2886
        TEST_CASE(simplifyTypedef97); // ticket #2983 (segmentation fault)
        TEST_CASE(simplifyTypedef98); // ticket #2963
        TEST_CASE(simplifyTypedef99); // ticket #2999
        TEST_CASE(simplifyTypedef100); // ticket #3000
        TEST_CASE(simplifyTypedef101); // ticket #3003 (segmentation fault)
        TEST_CASE(simplifyTypedef102); // ticket #3004
        TEST_CASE(simplifyTypedef103); // ticket #3007
        TEST_CASE(simplifyTypedef104); // ticket #3070
        TEST_CASE(simplifyTypedef105); // ticket #3616
        TEST_CASE(simplifyTypedef106); // ticket #3619
        TEST_CASE(simplifyTypedef107); // ticket #3963 - bad code => segmentation fault

        TEST_CASE(simplifyTypedefFunction1);
        TEST_CASE(simplifyTypedefFunction2); // ticket #1685
        TEST_CASE(simplifyTypedefFunction3);
        TEST_CASE(simplifyTypedefFunction4);
        TEST_CASE(simplifyTypedefFunction5);
        TEST_CASE(simplifyTypedefFunction6);
        TEST_CASE(simplifyTypedefFunction7);
        TEST_CASE(simplifyTypedefFunction8);

        TEST_CASE(simplifyTypedefShadow);  // #4445 - shadow variable

        TEST_CASE(simplifyOperator1);

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
        TEST_CASE(enumscope1); // ticket #3949
        TEST_CASE(duplicateDefinition); // ticket #3565

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

        TEST_CASE(simplifyIfNotNull);
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

        TEST_CASE(simplifyFlowControl);
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
    std::string tok(const std::string& code, bool simplify = true, Settings::PlatformType type = Settings::Unspecified) {
        return tok(code.c_str(), simplify, type);
    }
    std::string tok(const char code[], const char filename[]) {
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(0, false);
    }

    std::string tokenizeDebugListing(const std::string &code, bool simplify = false, const char filename[] = "test.cpp") {
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

        {
            const char code[] = "static void crash()\n"
                                "{\n"
                                "    goto err_exit;\n"
                                "err_exit:\n"
                                "    (void)foo();\n"
                                "}\n";

            const char expected[] = "static void crash ( ) "
                                    "{ foo ( ) ; return ; }";

            ASSERT_EQUALS(expected, tok(code));
        }

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
        ASSERT_EQUALS("<= 110 ;", tok("<= (10+100);"));
        ASSERT_EQUALS("while ( x ( ) == -1 ) { }", tok("while((x()) == -1){ }"));
    }

    void parenthesesVar() {
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



    void sizeof2() {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i[4];\n"
                            "    sizeof(i);\n"
                            "    sizeof(*i);\n"
                            "}\n";
        ASSERT_EQUALS("void foo ( ) { int i [ 4 ] ; 16 ; 4 ; }", tok(code));
    }

    void sizeof3() {
        const char code[] = "static int i[4];\n"
                            "void f()\n"
                            "{\n"
                            "    int i[10];\n"
                            "    sizeof(i);\n"
                            "}\n";
        ASSERT_EQUALS("static int i [ 4 ] ; void f ( ) { int i [ 10 ] ; 40 ; }", tok(code));
    }

    void sizeof4() {
        {
            const char code[] = "int i[10];\n"
                                "sizeof(i[0]);\n";
            ASSERT_EQUALS("int i [ 10 ] ; 4 ;", tok(code));
        }

        {
            const char code[] = "int i[10];\n"
                                "sizeof i[0];\n";
            ASSERT_EQUALS("int i [ 10 ] ; 4 ;", tok(code));
        }
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
        ASSERT_EQUALS("; int i [ 10 ] ; 4 ;", tok(code, true, Settings::Win32A));
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

    void sizeof22() {
        // ticket #2599 segmentation fault
        const char code[] = "sizeof\n";

        // don't segfault
        tok(code);
    }

    void sizeof23() {
        // ticket #2604 segmentation fault
        const char code[] = "sizeof <= A\n";

        // don't segfault
        tok(code);
    }


    void sizeofsizeof() {
        // ticket #1682
        const char code[] = "void f()\n"
                            "{\n"
                            "    sizeof sizeof 1;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) { sizeof sizeof ( 1 ) ; }", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void casting() {
        {
            const char code[] = "void f()\n"
                                "{\n"
                                "for (int i = 0; i < static_cast<int>(3); ++i) {}\n"
                                "}\n";

            const std::string expected("void f ( ) { for ( int i = 0 ; i < 3 ; ++ i ) { } }");

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    p = const_cast<char *> qtu ();\n"
                                "}\n";

            const std::string expected("void f ( ) { p = const_cast < char * > qtu ( ) ; }");

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            // ticket #645
            const char code[] = "void f()\n"
                                "{\n"
                                "    return dynamic_cast<Foo *>((bar()));\n"
                                "}\n";
            const std::string expected("void f ( ) { return bar ( ) ; }");

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



    void template1() {
        const char code[] = "template <classname T> void f(T val) { T a; }\n"
                            "f<int>(10);";

        const std::string expected("f<int> ( 10 ) ; "
                                   "void f<int> ( int val ) { }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template2() {
        const char code[] = "template <classname T> class Fred { T a; };\n"
                            "Fred<int> fred;";

        const std::string expected("Fred<int> fred ; "
                                   "class Fred<int> { int a ; }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template3() {
        const char code[] = "template <classname T, int sz> class Fred { T data[sz]; };\n"
                            "Fred<float,4> fred;";

        const std::string expected("Fred<float,4> fred ; "
                                   "class Fred<float,4> { float data [ 4 ] ; }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template4() {
        const char code[] = "template <classname T> class Fred { Fred(); };\n"
                            "Fred<float> fred;";

        const std::string expected("Fred<float> fred ; "
                                   "class Fred<float> { Fred<float> ( ) ; }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template5() {
        const char code[] = "template <classname T> class Fred { };\n"
                            "template <classname T> Fred<T>::Fred() { }\n"
                            "Fred<float> fred;";

        const std::string expected("template < classname T > Fred < T > :: Fred ( ) { } " // <- TODO: this should be removed
                                   "Fred<float> fred ; "
                                   "class Fred<float> { } "
                                   "Fred<float> :: Fred<float> ( ) { }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template6() {
        const char code[] = "template <classname T> class Fred { };\n"
                            "Fred<float> fred1;\n"
                            "Fred<float> fred2;";

        const std::string expected("Fred<float> fred1 ; "
                                   "Fred<float> fred2 ; "
                                   "class Fred<float> { }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template7() {
        // A template class that is not used => no simplification
        {
            const char code[] = "template <class T>\n"
                                "class ABC\n"
                                "{\n"
                                "public:\n"
                                "    typedef ABC<T> m;\n"
                                "};\n";

            const std::string expected("template < class T > class ABC { public: } ;");

            ASSERT_EQUALS(expected, tok(code));
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

            const std::string wanted("template < typename T > class ABC { public: } ; "
                                     "int main ( ) { "
                                     "std :: vector < int > v ; "
                                     "v . push_back ( 4 ) ; "
                                     "return 0 ; "
                                     "}");

            const std::string current("template < typename T > class ABC { public: } ; "
                                      "int main ( ) { "
                                      "ABC < int > :: type v ; "
                                      "v . push_back ( 4 ) ; "
                                      "return 0 ; "
                                      "}");

            TODO_ASSERT_EQUALS(wanted, current, tok(code));
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

            const std::string expected("template < typename T > class ABC { "
                                       "public: void f ( ) { "
                                       "ABC < int > :: type v ; "
                                       "v . push_back ( 4 ) ; "
                                       "} "
                                       "} ;");

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    // Template definitions but no usage => no expansion
    void template8() {
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

        ASSERT_EQUALS("template < typename T > class A ; "
                      "template < typename T > class B ; "
                      "template < typename T > class A { void f ( ) { B < T > a ; a = B < T > :: g ( ) ; T b ; b = 0 ; } } ; "
                      "template < typename T > B < T > h ( ) { return B < T > ( ) ; }", tok(code));

        ASSERT_EQUALS("class A { template < typename T > int foo ( T d ) ; } ;", tok("class A{ template<typename T> int foo(T d);};"));
    }

    void template9() {
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
        std::string expected("void f ( ) { A<int> a ; } "
                             "template < typename T > class B { void g ( ) { A<T> b ; b = A<T> :: h ( ) ; } } ; "
                             "class A<int> { } class A<T> { }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template10() {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    foo<3,int>();\n"
                            "}\n";

        // The expected result..
        const std::string expected("void f ( ) "
                                   "{"
                                   " foo<3,int> ( ) ; "
                                   "} "
                                   "int * foo<3,int> ( ) { return new int [ 3 ] ; }");
        ASSERT_EQUALS(expected, tok(code));
    }

    void template11() {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char * p = foo<3,char>();\n"
                            "}\n";

        // The expected result..
        const std::string expected("void f ( ) "
                                   "{"
                                   " char * p ; p = foo<3,char> ( ) ; "
                                   "} "
                                   "char * foo<3,char> ( ) { return new char [ 3 ] ; }");
        ASSERT_EQUALS(expected, tok(code));
    }

    void template12() {
        const char code[] = "template <int x, int y, int z>\n"
                            "class A : public B<x, y, (x - y) ? ((y < z) ? 1 : -1) : 0>\n"
                            "{ };\n"
                            "\n"
                            "void f()\n"
                            "{\n"
                            "    A<12,12,11> a;\n"
                            "}\n";

        // The expected result..
        const std::string expected("void f ( ) "
                                   "{"
                                   " A<12,12,11> a ; "
                                   "} "
                                   "class A<12,12,11> : public B < 12 , 12 , 0 > "
                                   "{ }");
        ASSERT_EQUALS(expected, tok(code));
    }

    void template13() {
        const char code[] = "class BB {};\n"
                            "\n"
                            "template <class T>\n"
                            "class AA\n"
                            "{\n"
                            "public:\n"
                            "    static AA<T> create(T* newObject);\n"
                            "    static int size();\n"
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
                            "    {}\n"
                            "\n"
                            "int yy[AA<CC>::size()];";

        // Just run it and check that there are not assertions.
        tok(code);
    }

    void template14() {
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

        ASSERT_EQUALS(expected, tok(code));
    }

    void template15() {
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
        const std::string expected("void a<0> ( ) { } "
                                   "int main ( ) "
                                   "{ a<2> ( ) ; return 0 ; } "
                                   "void a<2> ( ) { a<1> ( ) ; } "
                                   "void a<1> ( ) { a<0> ( ) ; }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template16() {
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

        const std::string expected("int main ( ) { b<2> ( ) ; return 0 ; } "
                                   "void b<2> ( ) { a<2> ( ) ; } "
                                   "void a<i> ( ) { } "
                                   "void a<2> ( ) { }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template17() {
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
        tok(code);
    }

    void template18() {
        const char code[] = "template <class T> class foo { T a; };\n"
                            "foo<int> *f;";

        const std::string expected("foo<int> * f ; "
                                   "class foo<int> { int a ; }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template19() {
        const char code[] = "template <typename T> T & foo()\n"
                            "{ static T temp; return temp; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char p = foo<char>();\n"
                            "}\n";

        // The expected result..
        const std::string expected("void f ( ) "
                                   "{"
                                   " char p ; p = foo<char> ( ) ; "
                                   "} "
                                   "char & foo<char> ( ) { static char temp ; return temp ; }");
        ASSERT_EQUALS(expected, tok(code));
    }

    void template20() {
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
        const std::string expected("template < class T > A < T > :: ~ A ( ) { } "  // <- TODO: this should be removed
                                   "A<int> a ; "
                                   "class A<int> { public: ~ A<int> ( ) ; } "
                                   "A<int> :: ~ A<int> ( ) { }");
        ASSERT_EQUALS(expected, tok(code));
    }

    void template21() {
        {
            const char code[] = "template <classname T> struct Fred { T a; };\n"
                                "Fred<int> fred;";

            const std::string expected("Fred<int> fred ; "
                                       "struct Fred<int> { int a ; }");

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <classname T, int sz> struct Fred { T data[sz]; };\n"
                                "Fred<float,4> fred;";

            const std::string expected("Fred<float,4> fred ; "
                                       "struct Fred<float,4> { float data [ 4 ] ; }");

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <classname T> struct Fred { Fred(); };\n"
                                "Fred<float> fred;";

            const std::string expected("Fred<float> fred ; "
                                       "struct Fred<float> { Fred<float> ( ) ; }");

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <classname T> struct Fred { };\n"
                                "Fred<float> fred1;\n"
                                "Fred<float> fred2;";

            const std::string expected("Fred<float> fred1 ; "
                                       "Fred<float> fred2 ; "
                                       "struct Fred<float> { }");

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void template22() {
        const char code[] = "template <classname T> struct Fred { T a; };\n"
                            "Fred<std::string> fred;";

        const std::string expected("Fred<std::string> fred ; "
                                   "struct Fred<std::string> { std :: string a ; }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template23() {
        const char code[] = "template <classname T> void foo() { }\n"
                            "void bar() {\n"
                            "    std::cout << (foo<double>());\n"
                            "}";

        const std::string expected("void bar ( ) {"
                                   " std :: cout << ( foo<double> ( ) ) ; "
                                   "} "
                                   "void foo<double> ( ) { }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void template24() {
        // #2648
        const char code[] = "template<int n> struct B\n"
                            "{\n"
                            "  int a[n];\n"
                            "};\n"
                            "\n"
                            "template<int x> class bitset: B<sizeof(int)>\n"
                            "{};\n"
                            "\n"
                            "bitset<1> z;";
        const char expected[] = "bitset<1> z ; "
                                "class bitset<1> : B<4> { } "
                                "struct B<4> { int a [ 4 ] ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template25() {
        const char code[] = "template<int n> struct B\n"
                            "{\n"
                            "  int a[n];\n"
                            "};\n"
                            "\n"
                            "template<int x> class bitset: B<((sizeof(int)) ? : 1)>\n"
                            "{};\n"
                            "\n"
                            "bitset<1> z;";

        const char actual[] = "template < int n > struct B { int a [ n ] ; } ; "
                              "bitset<1> z ; "
                              "class bitset<1> : B < 4 > { }";

        const char expected[] = "bitset<1> z ; "
                                "class bitset<1> : B<4> { } "
                                "struct B<4> { int a [ 4 ] ; }";

        TODO_ASSERT_EQUALS(expected, actual, tok(code));

    }

    void template26() {
        // #2721
        const char code[] = "template<class T>\n"
                            "class A { public: T x; };\n"
                            "\n"
                            "template<class M>\n"
                            "class C: public A<char[M]> {};\n"
                            "\n"
                            "C<2> a;\n";
        // TODO: expand A also
        ASSERT_EQUALS("template < class T > class A { public: T x ; } ; C<2> a ; class C<2> : public A < char [ 2 ] > { }", tok(code));
    }

    void template27() {
        // #3350 - template inside macro call
        const char code[] = "X(template<class T> class Fred);";
        ASSERT_EQUALS("X ( template < class T > class Fred ) ;", tok(code));
    }

    void template28() {
        // #3226 - inner template
        const char code[] = "template<class A, class B> class Fred {};\n"
                            "Fred<int,Fred<int,int> > x;\n";
        ASSERT_EQUALS("Fred<int,Fred<int,int>> x ; class Fred<int,int> { } class Fred<int,Fred<int,int>> { }", tok(code));
    }

    void template29() {
        // #3449 - garbage code (don't segfault)
        const char code[] = "template<typename T> struct A;\n"
                            "struct B { template<typename T> struct C };\n"
                            "{};";
        ASSERT_EQUALS("template < typename T > struct A ; struct B { template < typename T > struct C } ; { } ;", tok(code));
    }

    void template30() {
        // #3529 - template < template < ..
        const char code[] = "template<template<class> class A, class B> void f(){}";
        ASSERT_EQUALS("template < template < class > class A , class B > void f ( ) { }", tok(code));
    }

    void template31() {
        // #4010 - template reference type
        const char code[] = "template<class T> struct A{}; A<int&> a;";
        ASSERT_EQUALS("A<int&> a ; struct A<int&> { }", tok(code));
    }

    void template32() {
        // #3818 - mismatching template not handled well
        const char code[] = "template <class T1, class T2, class T3, class T4 > struct A { };\n"
                            "\n"
                            "template <class T>\n"
                            "struct B\n"
                            "{\n"
                            "    public:\n"
                            "        A < int, Pair<T, int>, int > a;\n"  // mismatching parameters => don't instantiate
                            "};\n"
                            "\n"
                            "B<int> b;\n";
        ASSERT_EQUALS("template < class T1 , class T2 , class T3 , class T4 > struct A { } ; "
                      "B<int> b ; "
                      "struct B<int> { public: A < int , Pair < int , int > , int > a ; }", tok(code));
    }

    void template33() {
        {
            // #3818 - inner templates in template instantiation not handled well
            const char code[] = "template<class T> struct A { };\n"
                                "template<class T> struct B { };\n"
                                "template<class T> struct C { A<B<X<T> > > ab; };\n"
                                "C<int> c;";
            ASSERT_EQUALS("C<int> c ; "
                          "struct C<int> { A<B<X<int>>> ab ; } "
                          "struct B<X<int>> { } "  // <- redundant.. but nevermind
                          "struct A<B<X<T>>> { } "  // <- redundant.. but nevermind
                          "struct A<B<X<int>>> { }", tok(code));
        }

        {
            // #4544
            const char code[] = "struct A { };\n"
                                "template<class T> struct B { };\n"
                                "template<class T> struct C { };\n"
                                "C< B<A> > c;";
            ASSERT_EQUALS("struct A { } ; "
                          "template < class T > struct B { } ; "  // <- redundant.. but nevermind
                          "C<B<A>> c ; struct C<B<A>> { }",
                          tok(code));
        }
    }

    void template34() {
        // #3706 - namespace => hang
        const char code[] = "namespace abc {\n"
                            "template <typename T> struct X { void f(X<T> &x) {} };\n"
                            "}\n"
                            "template <> int X<int>::Y(0);";
        ASSERT_EQUALS("namespace abc { "
                      "template < typename T > struct X { void f ( X < T > & x ) { } } ; "
                      "} "
                      "template < > int X < int > :: Y ( 0 ) ;", tok(code));
    }

    void template35() { // #4074 - "A<'x'> a;" is not recognized as template instantiation
        const char code[] = "template <char c> class A {};\n"
                            "A<'x'> a;";
        ASSERT_EQUALS("A<'x'> a ; class A<'x'> { }", tok(code));
    }

    void template36() { // #4310 - Passing unknown template instantiation as template argument
        const char code[] = "template <class T> struct X { T t; };\n"
                            "template <class C> struct Y { Foo < X< Bar<C> > > _foo; };\n" // <- Bar is unknown
                            "Y<int> bar;";
        ASSERT_EQUALS("Y<int> bar ; "
                      "struct Y<int> { Foo < X<Bar<int>> > _foo ; } "
                      "struct X<Bar<int>> { Bar < int > t ; }",
                      tok(code));
    }

    void template37() { // #4544 - A<class B> a;
        const char code[] = "class A { };\n"
                            "template<class T> class B {};\n"
                            "B<class A> b1;\n"
                            "B<A> b2;";
        ASSERT_EQUALS("class A { } ; B<A> b1 ; B<A> b2 ; class B<A> { }",
                      tok(code));
    }

    void template_unhandled() {
        // An unhandled template usage should be simplified..
        ASSERT_EQUALS("x<int> ( ) ;", tok("x<int>();"));
    }

    void template38() { // #4832 - Crash on C++11 right angle brackets
        const char code[] = "template <class T> class A {\n"
                            "  T mT;\n"
                            "public:\n"
                            "  void foo() {}\n"
                            "};\n"
                            "\n"
                            "int main() {\n"
                            "    A<A<BLA>>   gna1;\n"
                            "    A<BLA>      gna2;\n"
                            "}\n";
        tok(code); // Don't crash or freeze
    }

    void template39() { // #4742 - Used to freeze in 1.60
        const char code[] = "template<typename T> struct vector {"
                            "  operator T() const;"
                            "};"
                            "void f() {"
                            "  vector<vector<int>> v;"
                            "  const vector<int> vi = static_cast<vector<int>>(v);"
                            "}";
        tok(code);
    }

    void template40() { // #5055 - false negatives when there is template specialization outside struct
        const char code[] = "struct A {"
                            "  template<typename T> struct X { T t; };"
                            "};"
                            "template<> struct A::X<int> { int *t; };";
        ASSERT_EQUALS("struct A { template < typename T > struct X { T t ; } ; } ;", tok(code));
    }

    void template41() { // #4710 - const in template instantiation not handled perfectly
        const char code1[] = "template<class T> struct X { };\n"
                             "void f(const X<int> x) { }";
        ASSERT_EQUALS("void f ( const X<int> x ) { } struct X<int> { }", tok(code1));

        const char code2[] = "template<class T> T f(T t) { return t; }\n"
                             "int x() { return f<int>(123); }";
        ASSERT_EQUALS("int x ( ) { return f<int> ( 123 ) ; } int f<int> ( int t ) { return t ; }", tok(code2));
    }

    void template42() { // #4878 cpcheck aborts in ext-blocks.cpp (clang testcode)
        const char code[] = "template<typename ...Args>\n"
                            "int f0(Args ...args) {\n"
                            "  return ^ {\n"
                            "    return sizeof...(Args);\n"
                            "  }() + ^ {\n"
                            "    return sizeof...(args);\n"
                            "  }();\n"
                            "}";
        tok(code);
    }

    void template43() { // #5097 - Assert due to '>>' in 'B<A<C>>' not being treated as end of template instantation
        const char code[] = "template <typename T> struct C { };"
                            "template <typename T> struct D { static int f() { return C<T>::f(); } };"
                            "template <typename T> inline int f2() { return D<T>::f(); }"
                            "template <typename T> int f1(int x, T *) { int id = f2<T>(); return id; }"
                            "template <> struct C < B < A >> {"
                            "  static int f() {"
                            "    return f1 < B < A >> (0, reinterpret_cast< B<A> *>(E<void *>::Int(-1)));"
                            "  }"
                            "};";
        tok(code); // Don't assert
    }


    void template_default_parameter() {
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
            const std::string expected("void f ( ) "
                                       "{"
                                       " A<int,2> a1 ;"
                                       " A<int,3> a2 ; "
                                       "} "
                                       "class A<int,2> "
                                       "{ int ar [ 2 ] ; } "
                                       "class A<int,3> "
                                       "{ int ar [ 3 ] ; }");
            ASSERT_EQUALS(expected, tok(code));
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
            const std::string expected("void f ( ) "
                                       "{"
                                       " A<int,3,2> a1 ;"
                                       " A<int,3,2> a2 ; "
                                       "} "
                                       "class A<int,3,2> "
                                       "{ int ar [ 5 ] ; }");
            ASSERT_EQUALS(expected, tok(code));
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

            const std::string current("void f ( ) "
                                      "{ "
                                      "A < int , ( int ) 2 > a1 ; "
                                      "A<int,3> a2 ; "
                                      "} "
                                      "class A<int,3> "
                                      "{ int ar [ 3 ] ; }"
                                     );
            TODO_ASSERT_EQUALS(wanted, current, tok(code));
        }
        {
            const char code[] = "template<class T, class T2 = A<T>> class B {};\n"
                                "template<class B = A, typename C = C<B>> class C;\n"
                                "template<class B, typename C> class D { };\n";
            ASSERT_EQUALS("template < class T , class T2 > class B { } ; "
                          "template < class B , typename C > class C ; "
                          "template < class B , typename C > class D { } ;", tok(code));
        }
    }

    void template_default_type() {
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

        tok(code);

        //ASSERT_EQUALS("[file1.cpp:15]: (error) Internal error: failed to instantiate template. The checking continues anyway.\n", errout.str());
        ASSERT_EQUALS("", errout.str());
    }

    void template_typename() {
        {
            const char code[] = "template <class T>\n"
                                "void foo(typename T::t *)\n"
                                "{ }";

            // The expected result..
            const std::string expected("template < class T > void foo ( T :: t * ) { }");
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f() {\n"
                                "    x(sizeof typename);\n"
                                "    type = 0;\n"
                                "}";

            ASSERT_EQUALS("void f ( ) { x ( sizeof ( typename ) ) ; type = 0 ; }", tok(code));
        }
    }

    void template_constructor() {
        // #3152 - if template constructor is removed then there might be
        //         "no constructor" false positives
        const char code[] = "class Fred {\n"
                            "    template<class T> explicit Fred(T t) { }\n"
                            "}";
        ASSERT_EQUALS("class Fred { template < class T > explicit Fred ( T t ) { } }", tok(code));

        // #3532
        const char code2[] = "class Fred {\n"
                             "    template<class T> Fred(T t) { }\n"
                             "}";
        ASSERT_EQUALS("class Fred { template < class T > Fred ( T t ) { } }", tok(code2));
    }

    unsigned int templateParameters(const char code[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        return TemplateSimplifier::templateParameters(tokenizer.tokens());
    }

    void templateParameters() {
        // Test that the function TemplateSimplifier::templateParameters works
        ASSERT_EQUALS(1U, templateParameters("<struct C> x;"));
        ASSERT_EQUALS(1U, templateParameters("<union C> x;"));
        ASSERT_EQUALS(1U, templateParameters("<const int> x;"));
        ASSERT_EQUALS(1U, templateParameters("<int const *> x;"));
        ASSERT_EQUALS(1U, templateParameters("<const struct C> x;"));
    }

    void templateParameters1() {
        // #4169 - segmentation fault (invalid code)
        const char code[] = "volatile true , test < test < #ifdef __ppc__ true ,";
        // do not crash on invalid code
        ASSERT_EQUALS(0, templateParameters(code));
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

            const std::string expected("namespace a { namespace b { void f ( ) { } } }");

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "namespace b{ void f(){} }";

            const std::string expected("namespace b { void f ( ) { } }");

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f(int namespace) { }";

            const std::string expected("void f ( int namespace ) { }");

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
        ASSERT_EQUALS("void foo ( int a ) { a = b ( ) ; if ( 0 <= a ) { ; } }", tok("void foo(int a) {if((a=b())>=0);}"));
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
        ASSERT_EQUALS("; do { current = f ( ) ; } while ( current ) ;", simplifyIfAndWhileAssign(";do { } while((current=f()) != NULL);"));
    }

    void ifnot() {
        ASSERT_EQUALS("if ( ! x ) { ; }", tok("if(0==x);", false));
        ASSERT_EQUALS("if ( ! x ) { ; }", tok("if(x==0);", false));
        ASSERT_EQUALS("if ( ! ( a = b ) ) { ; }", tok("if(0==(a=b));", false));
        ASSERT_EQUALS("if ( ! a && b ( ) ) { ; }", tok("if( 0 == a && b() );", false));
        ASSERT_EQUALS("if ( b ( ) && ! a ) { ; }", tok("if( b() && 0 == a );", false));
        ASSERT_EQUALS("if ( ! ( a = b ) ) { ; }", tok("if((a=b)==0);", false));
        ASSERT_EQUALS("if ( ! x . y ) { ; }", tok("if(x.y==0);", false));
        ASSERT_EQUALS("if ( ! x ) { ; }", tok("if((x==0));", false));
        ASSERT_EQUALS("if ( ( ! x ) && ! y ) { ; }", tok("if((x==0) && y==0);", false));
        ASSERT_EQUALS("if ( ! ( ! fclose ( fd ) ) ) { ; }", tok("if(!(fclose(fd) == 0));", false));
    }


    void not1() {
        ASSERT_EQUALS("if ( ! p ) { ; }", tok("if (not p);", false));
        ASSERT_EQUALS("if ( p && ! q ) { ; }", tok("if (p && not q);", false));
        ASSERT_EQUALS("void foo ( not i )", tok("void foo ( not i )", false));
    }

    void and1() {
        ASSERT_EQUALS("if ( p && q ) { ; }",
                      tok("if (p and q) ;", false));

        ASSERT_EQUALS("if ( foo ( ) && q ) { ; }",
                      tok("if (foo() and q) ;", false));

        ASSERT_EQUALS("if ( foo ( ) && bar ( ) ) { ; }",
                      tok("if (foo() and bar()) ;", false));

        ASSERT_EQUALS("if ( p && bar ( ) ) { ; }",
                      tok("if (p and bar()) ;", false));

        ASSERT_EQUALS("if ( p && ! q ) { ; }",
                      tok("if (p and not q) ;", false));
    }

    void or1() {
        ASSERT_EQUALS("if ( p || q ) { ; }",
                      tok("if (p or q) ;", false));

        ASSERT_EQUALS("if ( foo ( ) || q ) { ; }",
                      tok("if (foo() or q) ;", false));

        ASSERT_EQUALS("if ( foo ( ) || bar ( ) ) { ; }",
                      tok("if (foo() or bar()) ;", false));

        ASSERT_EQUALS("if ( p || bar ( ) ) { ; }",
                      tok("if (p or bar()) ;", false));

        ASSERT_EQUALS("if ( p || ! q ) { ; }",
                      tok("if (p or not q) ;", false));
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
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a ; delete b ; }", tok(code));
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
    }

    void simplifyConditionOperator() {
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
            const char code[] = "= 1 ? 0 : ({ 0; });";
            ASSERT_EQUALS("= 0 ;", tok(code));
        }

        //GNU extension: "x ?: y" <-> "x ? x : y"
        {
            const char code[] = "; a = 1 ? : x; b = 0 ? : 2;";
            ASSERT_EQUALS("; a = 1 ; b = 2 ;", tok(code));
        }

        {
            const char code[] = "int f(int b, int d)\n"
                                "{\n"
                                "  d = b ? b : 10;\n"
                                "  return d;\n"
                                "}\n";
            ASSERT_EQUALS("int f ( int b , int d ) { if ( b ) { d = b ; } else { d = 10 ; } return d ; }", tok(code));
        }

        {
            const char code[] = "int f(int b, int *d)\n"
                                "{\n"
                                "  *d = b ? b : 10;\n"
                                "  return *d;\n"
                                "}\n";
            ASSERT_EQUALS("int f ( int b , int * d ) { if ( b ) { * d = b ; } else { * d = 10 ; } return * d ; }", tok(code));
        }

        {
            const char code[] = "int f(int b, int *d)\n"
                                "{\n"
                                "  if(b) {b++;}"
                                "  *d = b ? b : 10;\n"
                                "  return *d;\n"
                                "}\n";
            ASSERT_EQUALS("int f ( int b , int * d ) { if ( b ) { b ++ ; } if ( b ) { * d = b ; } else { * d = 10 ; } return * d ; }", tok(code));
        }

        {
            // Ticket #2885
            const char code[] = "; const char *cx16 = has_cmpxchg16b ? \" -mcx16\" : \" -mno-cx16\";";
            const char expected[] = "; const char * cx16 ; if ( has_cmpxchg16b ) { cx16 = \" -mcx16\" ; } else { cx16 = \" -mno-cx16\" ; }";
            ASSERT_EQUALS(expected, tok(code));
        }
        // Ticket #3572 (segmentation fault)
        ASSERT_EQUALS("0 ; x = { ? y : z ; }", tok("0; x = { ? y : z; }"));

        {
            // #4019 - varid
            const char code[] = "; char *p; *p = a ? 1 : 0;";
            const char expected[] = "\n\n##file 0\n"
                                    "1: ; char * p@1 ; if ( a ) { * p@1 = 1 ; } else { * p@1 = 0 ; }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code, true));
        }

        {
            // #3922 - (true)
            ASSERT_EQUALS("; x = 2 ;", tok("; x = (true)?2:4;"));
            ASSERT_EQUALS("; x = 4 ;", tok("; x = (false)?2:4;"));
            ASSERT_EQUALS("; x = * a ;", tok("; x = (true)?*a:*b;"));
            ASSERT_EQUALS("; x = * b ;", tok("; x = (false)?*a:*b;"));
            ASSERT_EQUALS("void f ( ) { return 1 ; }", tok("void f() { char *p=0; return (p==0)?1:2; }"));
        }

        // 4225 - varid gets lost
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int a@1 ; int b@2 ; int c@3 ; int d@4 ; if ( b@2 ) { a@1 = c@3 ; } else { a@1 = d@4 ; }\n",
                      tokenizeDebugListing("int a, b, c, d; a = b ? (int *)c : d;", true));
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
    }

    void comparisons() {
        ASSERT_EQUALS("( 1 )", tok("( 1 < 2 )"));
        ASSERT_EQUALS("( x )", tok("( x && 1 < 2 )"));
        ASSERT_EQUALS("( 5 )", tok("( 1 < 2 && 3 < 4 ? 5 : 6 )"));
        ASSERT_EQUALS("( 6 )", tok("( 1 > 2 && 3 > 4 ? 5 : 6 )"));
    }

    void goto1() {
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
            tokenizer.simplifyTokenList2();

            const char expect[] = "\n\n##file 0\n"
                                  "1: void foo ( )\n"
                                  "2: {\n"
                                  "3: if ( a ( ) )\n"
                                  "4: {\n"
                                  "5:\n|\n8:\n"
                                  "9: c ( ) ; return ; }\n"
                                  "7: b ( ) ;\n"
                                  "8:\n"
                                  "9: c ( ) ;\n"
                                  "10: }\n";

            ASSERT_EQUALS(expect, tokenizer.tokens()->stringifyList());
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
            tokenizer.simplifyTokenList2();

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

            ASSERT_EQUALS(expect, tokenizer.tokens()->stringifyList());
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
    }

    void goto2() {
        // Don't simplify goto inside function call (macro)
        const char code[] = "void f ( ) { slist_iter ( if ( a ) { goto dont_write ; } dont_write : ; x ( ) ; ) ; }";
        ASSERT_EQUALS(code, tok(code));

        //ticket #3229 (segmentation fault)
        ASSERT_EQUALS("void f ( ) { MACRO ( return ; ) return ; }",tok("void f ( ) {goto label; label: MACRO(return;)}"));
    }

    void goto3() {
        // Simplify goto inside the namespace|struct|class|union block
        {
            const char code[] = "namespace A1"
                                "{"
                                "    void foo()"
                                "    {"
                                "        goto source ;"
                                "        bleeh;"
                                "        source:"
                                "        boo();"
                                "    }"
                                "}";
            const char expected[] = "namespace A1 "
                                    "{"
                                    " void foo ( )"
                                    " {"
                                    " boo ( ) ; return ;"
                                    " } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class A"
                                "{"
                                "    int n,m;"
                                "    A()"
                                "    {"
                                "        goto source ;"
                                "        bleeh;"
                                "        source:"
                                "        boo();"
                                "    }"
                                "    void boo();"
                                "}";
            const char expected[] = "class A "
                                    "{"
                                    " int n ; int m ;"
                                    " A ( )"
                                    " {"
                                    " boo ( ) ; return ;"
                                    " }"
                                    " void boo ( ) ; "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct A"
                                "{"
                                "    int n,m;"
                                "    A() : m(0)"
                                "    {"
                                "        goto source;"
                                "        bleeh;"
                                "        source:"
                                "        n=10;"
                                "    }"
                                "}";
            const char expected[] = "struct A "
                                    "{"
                                    " int n ; int m ;"
                                    " A ( ) : m ( 0 )"
                                    " {"
                                    " n = 10 ; return ;"
                                    " } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "namespace A1"
                                "{"
                                "    class A"
                                "    {"
                                "        int n,m;"
                                "        A()"
                                "        {"
                                "            goto source ;"
                                "            bleeh;"
                                "            source:"
                                "            boo();"
                                "        }"
                                "        void boo();"
                                "    }"
                                "}";
            const char expected[] = "namespace A1 "
                                    "{"
                                    " class A"
                                    " {"
                                    " int n ; int m ;"
                                    " A ( )"
                                    " {"
                                    " boo ( ) ; return ;"
                                    " }"
                                    " void boo ( ) ;"
                                    " } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "namespace A1"
                                "{"
                                "    namespace AA1"
                                "    {"
                                "        void foo1()"
                                "        {"
                                "            goto source1 ;"
                                "            bleeh;"
                                "            source1:"
                                "            boo1();"
                                "        }"
                                "    }"
                                "    namespace AA2"
                                "    {"
                                "        void foo2()"
                                "        {"
                                "            goto source2 ;"
                                "            bleeh;"
                                "            source2:"
                                "            boo2();"
                                "        }"
                                "    }"
                                "}";
            const char expected[] = "namespace A1 "
                                    "{"
                                    " namespace AA1"
                                    " {"
                                    " void foo1 ( )"
                                    " {"
                                    " boo1 ( ) ; return ;"
                                    " }"
                                    " }"
                                    " namespace AA2"
                                    " {"
                                    " void foo2 ( )"
                                    " {"
                                    " boo2 ( ) ; return ;"
                                    " }"
                                    " } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "union A1"
                                "{"
                                " int a; "
                                " double b; "
                                "    A1() : b(3.22)"
                                "    {"
                                "        goto source ;"
                                "        bleeh;"
                                "        source:"
                                "        a = 322;"
                                "    }"
                                "}";
            const char expected[] = "union A1 "
                                    "{"
                                    " int a ;"
                                    " double b ;"
                                    " A1 ( ) : b ( 3.22 )"
                                    " {"
                                    " a = 322 ; return ;"
                                    " } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class A"
                                "{"
                                " int a; "
                                " double b; "
                                "    A() : b(3.22)"
                                "    {"
                                "        goto source ;"
                                "        bleeh;"
                                "        source:"
                                "        a = 322;"
                                "    }"
                                "}"
                                "class A1 : public A"
                                "{"
                                " int a1; "
                                " double b1; "
                                "    A1() : b1(3.22)"
                                "    {"
                                "        goto source1 ;"
                                "        bleeh1;"
                                "        source1:"
                                "        a = 322;"
                                "    }"
                                "}";
            const char expected[] = "class A "
                                    "{"
                                    " int a ;"
                                    " double b ;"
                                    " A ( ) : b ( 3.22 )"
                                    " {"
                                    " a = 322 ;"
                                    " return ;"
                                    " } "
                                    "} "
                                    "class A1 : public A "
                                    "{"
                                    " int a1 ;"
                                    " double b1 ;"
                                    " A1 ( ) : b1 ( 3.22 )"
                                    " {"
                                    " a = 322 ;"
                                    " return ;"
                                    " } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void goto4() {
        const char code[] = "int main()\n"
                            "{\n"
                            "   goto SkipIncr;\n"
                            "   do {\n"
                            "       f();\n"
                            "       SkipIncr:\n"
                            "       printf(\".\");\n"
                            "   } while (bar());\n"
                            "}\n";

        const char expected[] = "int main ( ) "
                                "{"
                                " goto SkipIncr ;"
                                " do {"
                                " f ( ) ;"
                                " SkipIncr : ;"
                                " printf ( \".\" ) ;"
                                " } while ( bar ( ) ) ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void goto5() {
        const char code[] = "int foo() {\n"
                            "    goto err;\n"
                            "err:\n"
                            "    return ( { __asm__(X); } ); "
                            "}\n";
        ASSERT_EQUALS("int foo ( ) {"
                      " return { asm ( \"X\" ) ; } ;"
                      " return { asm ( \"X\" ) ; } ; "
                      "}", tok(code));
    }

    void goto6() { // from code in linux that wasn't handled well
        const char code1[] = "static void a() {\n"
                             "unlock:\n"
                             "}\n"
                             "\n"
                             "static void b() {\n"
                             "  if (c)\n"
                             "    goto defer;\n"
                             "defer:\n"
                             "}\n";
        ASSERT_EQUALS("static void a ( ) { } "
                      "static void b ( ) { if ( c ) { return ; } }", tok(code1));

        const char code2[] = "void a()\n"
                             "{\n"
                             "  if (x) {}\n"
                             "unlock:\n"
                             "}\n"
                             "\n"
                             "void b()\n"
                             "{\n"
                             "  { goto defer; }\n"
                             "defer:\n"
                             "}";
        ASSERT_EQUALS("void a ( ) { if ( x ) { } } void b ( ) { return ; }", tok(code2));
    }

    void flowControl() {
        std::list<std::string> beforedead;
        //beforedead.push_back("return");
        //beforedead.push_back("throw ( 10 )");
        beforedead.push_back("exit ( 0 )");
        //beforedead.push_back("abort ( )");
        //beforedead.push_back("goto labels");
        //beforedead.push_back("break");
        //beforedead.push_back("continue");

        for (std::list<std::string>::iterator it = beforedead.begin(); it != beforedead.end(); ++it) {
            {
                ASSERT_EQUALS("void f ( ) { " + *it + " ; }", tok("void f() { " + *it + "; foo(); }"));
                ASSERT_EQUALS("void f ( ) { " + *it + " ; }", tok("void f() { " + *it + "; if (m) foo(); }"));
                ASSERT_EQUALS("void f ( int n ) { if ( n ) { " + *it + " ; } foo ( ) ; }",tok("void f(int n) { if (n) { " + *it + "; } foo(); }"));
                ASSERT_EQUALS("void f ( ) { " + *it + " ; }", tok("void f() { " + *it + "; dead(); switch (n) { case 1: deadcode () ; default: deadcode (); } }"));

                ASSERT_EQUALS("int f ( int n ) { switch ( n ) { case 0 : ; " + *it + " ; default : ; " + *it + " ; } " + *it + " ; }",
                              tok("int f(int n) { switch (n) {case 0: " + *it + "; n*=2; default: " + *it + "; n*=6;} " + *it + "; foo();}"));
                //ticket #3132
                ASSERT_EQUALS("void f ( int i ) { goto label ; { label : ; " + *it + " ; } }",tok("void f (int i) { goto label; switch(i) { label: " + *it + "; } }"));
                //ticket #3148
                ASSERT_EQUALS("void f ( ) { MACRO ( " + *it + " ) }",tok("void f() { MACRO(" + *it + ") }"));
                ASSERT_EQUALS("void f ( ) { MACRO ( " + *it + " ; , NULL ) }",tok("void f() { MACRO(" + *it + ";, NULL) }"));
                ASSERT_EQUALS("void f ( ) { MACRO ( bar1 , " + *it + " ) }",tok("void f() { MACRO(bar1, " + *it + ") }"));
                ASSERT_EQUALS("void f ( ) { MACRO ( " + *it + " ; bar2 , foo ) }",tok("void f() { MACRO(" + *it + "; bar2, foo) }"));
            }

            {
                std::string code = "void f(){ "
                                   "   if (k>0) goto label; "
                                   "   " + *it + "; "
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
                ASSERT_EQUALS("void f ( ) { if ( 0 < k ) { goto label ; } " + *it + " ; { label : ; bar ( ) ; } }",tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    {"
                                   "        boo();"
                                   "        while (n) { --n; }"
                                   "        {"
                                   "            label:"
                                   "            ok();"
                                   "        }"
                                   "    }"
                                   "}";
                ASSERT_EQUALS("void foo ( ) { " + *it + " ; { label : ; ok ( ) ; } }", tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    switch (n) {"
                                   "        case 1:"
                                   "            label:"
                                   "            foo(); break;"
                                   "        default:"
                                   "            break;"
                                   "    }"
                                   "}";
                std::string expected = "void foo ( ) { " + *it + " ; { label : ; foo ( ) ; break ; } }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    switch (n) {"
                                   "        case 1:"
                                   "            {"
                                   "                foo();"
                                   "            }"
                                   "            label:"
                                   "            bar();"
                                   "    }"
                                   "}";
                std::string expected = "void foo ( ) { " + *it + " ; { label : ; bar ( ) ; } }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    switch (n) {"
                                   "        case a:"
                                   "            {"
                                   "                foo();"
                                   "            }"
                                   "        case b|c:"
                                   "            bar();"
                                   "    }"
                                   "}";
                std::string expected = "void foo ( ) { " + *it + " ; }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    switch (n) {"
                                   "        case 1:"
                                   "            label:"
                                   "            foo(); break;"
                                   "        default:"
                                   "            break; break;"
                                   "    }"
                                   "}";
                std::string expected = "void foo ( ) { " + *it + " ; { label : ; foo ( ) ; break ; } }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    switch (n) {"
                                   "        case 1:"
                                   "            label:"
                                   "            foo(); break; break;"
                                   "        default:"
                                   "            break;"
                                   "    }"
                                   "}";
                std::string expected = "void foo ( ) { " + *it + " ; { label : ; foo ( ) ; break ; } }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    switch (n) {"
                                   "        case 1:"
                                   "            label:"
                                   "            foo(); break; break;"
                                   "        default:"
                                   "            break; break;"
                                   "    }"
                                   "}";
                std::string expected = "void foo ( ) { " + *it + " ; { label : ; foo ( ) ; break ; } }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "int f() { "
                                   "switch (x) { case 1: " + *it + "; bar(); tack; { ticak(); " + *it + " } " + *it + ";"
                                   "case 2: " + *it + "; { random(); } tack(); "
                                   "switch(y) { case 1: " + *it + "; case 2: " + *it + "; } "
                                   "" + *it + "; } " + *it + "; }";
                ASSERT_EQUALS("int f ( ) { switch ( x ) { case 1 : ; " + *it + " ; case 2 : ; " + *it + " ; } " + *it + " ; }",tok(code));
            }

            {
                std::string code = "int f() {"
                                   "switch (x) { case 1: " + *it + "; bar(); tack; { ticak(); " + *it + "; } " + *it + ";"
                                   "case 2: switch(y) { case 1: " + *it + "; bar2(); foo(); case 2: " + *it + "; }"
                                   "" + *it + "; } " + *it + "; }";
                std::string expected = "int f ( ) {"
                                       " switch ( x ) { case 1 : ; " + *it + " ;"
                                       " case 2 : ; switch ( y ) { case 1 : ; " + *it + " ; case 2 : ; " + *it + " ; }"
                                       " " + *it + " ; } " + *it + " ; }";
                ASSERT_EQUALS(expected,tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    switch (i) { case 0: switch (j) { case 0: " + *it + "; }"
                                   "        case 1: switch (j) { case -1: " + *it + "; }"
                                   "        case 2: switch (j) { case -2: " + *it + "; }"
                                   "        case 3: if (blah6) {" + *it + ";} break; } }";
                std::string expected = "void foo ( ) {"
                                       " switch ( i ) { case 0 : ; switch ( j ) { case 0 : ; " + *it + " ; }"
                                       " case 1 : ; switch ( j ) { case -1 : ; " + *it + " ; }"
                                       " case 2 : ; switch ( j ) { case -2 : ; " + *it + " ; }"
                                       " case 3 : ; if ( blah6 ) { " + *it + " ; } break ; } }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "void foo () {"
                                   "    " + *it + ";"
                                   "    switch (i) { case 0: switch (j) { case 0: foo(); }"
                                   "        case 1: switch (j) { case -1: bar(); label:; ok(); }"
                                   "        case 3: if (blah6) { boo(); break; } } }";
                std::string expected = "void foo ( ) { " + *it + " ; { { label : ; ok ( ) ; } case 3 : ; if ( blah6 ) { boo ( ) ; break ; } } }";
                ASSERT_EQUALS(expected, tok(code));
            }

            {
                std::string code = "void foo() {"
                                   "     switch ( t ) {"
                                   "     case 0:"
                                   "          if ( t ) switch ( b ) {}"
                                   "          break;"
                                   "     case 1:"
                                   "          " + *it + ";"
                                   "          return 0;"
                                   "     }"
                                   "     return 0;"
                                   "}";
                std::string expected = "void foo ( ) {"
                                       " switch ( t ) {"
                                       " case 0 : ;"
                                       " if ( t ) { switch ( b ) { } }"
                                       " break ;"
                                       " case 1 : ;"
                                       " " + *it + " ;"
                                       " }"
                                       " return 0 ; "
                                       "}";
                ASSERT_EQUALS(expected, tok(code));
            }
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
            ASSERT_EQUALS(expected, tok(code));
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
            ASSERT_EQUALS(expected, tok(code));
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
            ASSERT_EQUALS(expected, tok(code));
        }
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

    std::string simplifyTypedef(const char code[]) {
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);

        std::istringstream istr(code);
        tokenizer.list.createTokens(istr);
        tokenizer.createLinks();
        tokenizer.simplifyTypedef();

        return tokenizer.tokens()->stringifyList(0, false);
    }



    void simplifyTypedef1() {
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
            ""
            "void foo ( ) { } "
            "} ; "
            "int main ( ) "
            "{ "
            "A a ; "
            "a . foo ( ) ; "
            "wchar_t c ; c = 0 ; "
            "}";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef2() {
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
            "class A "
            "{ "
            "public: "
            ""
            "wchar_t foo ( ) { wchar_t b ; return b ; } "
            "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef3() {
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
            "wchar_t foo ( ) "
            "{ "
            ""
            "wchar_t b ; "
            "return b ; "
            "} "
            "int main ( ) "
            "{ "
            "A b ; "
            "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef4() {
        const char code[] = "typedef int s32;\n"
                            "typedef unsigned int u32;\n"
                            "void f()\n"
                            "{\n"
                            "    s32 ivar = -2;\n"
                            "    u32 uvar = 2;\n"
                            "    return uvar / ivar;\n"
                            "}\n";

        const std::string expected =
            "void f ( ) "
            "{ "
            "int ivar ; ivar = -2 ; "
            "unsigned int uvar ; uvar = 2 ; "
            "return uvar / ivar ; "
            "}";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef5() {
        // ticket #780
        const char code[] =
            "typedef struct yy_buffer_state *YY_BUFFER_STATE;\n"
            "void f()\n"
            "{\n"
            "    YY_BUFFER_STATE state;\n"
            "}\n";

        const char expected[] =
            "void f ( ) "
            "{ "
            "struct yy_buffer_state * state ; "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef6() {
        // ticket #983
        const char code[] =
            "namespace VL {\n"
            "    typedef float float_t ;\n"
            "    inline VL::float_t fast_atan2(VL::float_t y, VL::float_t x){}\n"
            "}\n";

        const char expected[] =
            "namespace VL { "
            ""
            "float fast_atan2 ( float y , float x ) { } "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef7() {
        const char code[] = "typedef int abc ; "
                            "Fred :: abc f ;";
        const char expected[] = "Fred :: abc f ;";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef8() {
        const char code[] = "typedef int INT;\n"
                            "typedef unsigned int UINT;\n"
                            "typedef int * PINT;\n"
                            "typedef unsigned int * PUINT;\n"
                            "typedef int & RINT;\n"
                            "typedef unsigned int & RUINT;\n"
                            "typedef const int & RCINT;\n"
                            "typedef const unsigned int & RCUINT;\n"
                            "INT ti;\n"
                            "UINT tui;\n"
                            "PINT tpi;\n"
                            "PUINT tpui;\n"
                            "RINT tri;\n"
                            "RUINT trui;\n"
                            "RCINT trci;\n"
                            "RCUINT trcui;";

        const char expected[] =
            "int ti ; "
            "unsigned int tui ; "
            "int * tpi ; "
            "unsigned int * tpui ; "
            "int & tri ; "
            "unsigned int & trui ; "
            "const int & trci ; "
            "const unsigned int & trcui ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef9() {
        const char code[] = "typedef struct s S, * PS;\n"
                            "typedef struct t { int a; } T, *TP;\n"
                            "typedef struct { int a; } U;\n"
                            "typedef struct { int a; } * V;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;";

        const char expected[] =
            "struct t { int a ; } ; "
            "struct U { int a ; } ; "
            "struct Unnamed0 { int a ; } ; "
            "struct s s ; "
            "struct s * ps ; "
            "struct t t ; "
            "struct t * tp ; "
            "struct U u ; "
            "struct Unnamed0 * v ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef10() {
        const char code[] = "typedef union s S, * PS;\n"
                            "typedef union t { int a; float b ; } T, *TP;\n"
                            "typedef union { int a; float b; } U;\n"
                            "typedef union { int a; float b; } * V;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;";

        const char expected[] =
            "union t { int a ; float b ; } ; "
            "union U { int a ; float b ; } ; "
            "union Unnamed1 { int a ; float b ; } ; "
            "union s s ; "
            "union s * ps ; "
            "union t t ; "
            "union t * tp ; "
            "union U u ; "
            "union Unnamed1 * v ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef11() {
        const char code[] = "typedef enum { a = 0 , b = 1 , c = 2 } abc;\n"
                            "typedef enum xyz { x = 0 , y = 1 , z = 2 } XYZ;\n"
                            "abc e1;\n"
                            "XYZ e2;";

        const char expected[] = "int e1 ; "
                                "int e2 ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef12() {
        const char code[] = "typedef vector<int> V1;\n"
                            "typedef std::vector<int> V2;\n"
                            "typedef std::vector<std::vector<int> > V3;\n"
                            "typedef std::list<int>::iterator IntListIterator;\n"
                            "V1 v1;\n"
                            "V2 v2;\n"
                            "V3 v3;\n"
                            "IntListIterator iter;";

        const char expected[] =
            "vector < int > v1 ; "
            "std :: vector < int > v2 ; "
            "std :: vector < std :: vector < int > > v3 ; "
            "std :: list < int > :: iterator iter ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef13() {
        // ticket # 1167
        const char code[] = "typedef std::pair<int(*)(void*), void*> Func;"
                            "typedef std::vector<Func> CallQueue;"
                            "int main() {}";

        // Tokenize and check output..
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef14() {
        // ticket # 1232
        const char code[] = "template <typename F, unsigned int N> struct E"
                            "{"
                            "    typedef E<F,(N>0)?(N-1):0> v;"
                            "    typedef typename add<v,v>::val val;"
                            "    FP_M(val);"
                            "};"
                            "template <typename F> struct E <F,0>"
                            "{"
                            "    typedef typename D<1>::val val;"
                            "    FP_M(val);"
                            "};";

        // Tokenize and check output..
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef15() {
        {
            const char code[] = "typedef char frame[10];\n"
                                "frame f;";

            const char expected[] = "char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef unsigned char frame[10];\n"
                                "frame f;";

            const char expected[] = "unsigned char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef16() {
        // ticket # 1252
        const char code[] = "typedef char MOT8;\n"
                            "typedef  MOT8 CHFOO[4096];\n"
                            "typedef struct {\n"
                            "   CHFOO freem;\n"
                            "} STRFOO;";

        // Tokenize and check output..
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef17() {
        const char code[] = "typedef char * PCHAR, CHAR;\n"
                            "PCHAR pc;\n"
                            "CHAR c;";

        const char expected[] =
            "char * pc ; "
            "char c ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef18() {
        const char code[] = "typedef vector<int[4]> a;\n"
                            "a b;\n";

        // Clear the error buffer..
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList2();

        ASSERT_EQUALS(true, tokenizer.validate());
    }

    void simplifyTypedef19() {
        {
            // ticket #1275
            const char code[] = "typedef struct {} A, *B, **C;\n"
                                "A a;\n"
                                "B b;\n"
                                "C c;";

            const char expected[] =
                "struct A { } ; "
                "struct A a ; "
                "struct A * b ; "
                "struct A * * c ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef struct {} A, *********B;\n"
                                "A a;\n"
                                "B b;";

            const char expected[] =
                "struct A { } ; "
                "struct A a ; "
                "struct A * * * * * * * * * b ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef struct {} **********A, *B, C;\n"
                                "A a;\n"
                                "B b;\n"
                                "C c;";

            const char expected[] =
                "struct Unnamed2 { } ; "
                "struct Unnamed2 * * * * * * * * * * a ; "
                "struct Unnamed2 * b ; "
                "struct Unnamed2 c ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef20() {
        // ticket #1284
        const char code[] = "typedef jobject invoke_t (jobject, Proxy *, Method *, JArray< jobject > *);";

        // Clear the error buffer..
        errout.str("");

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyTokenList2();

        ASSERT_EQUALS(true, tokenizer.validate());
    }

    void simplifyTypedef21() {
        const char code[] = "typedef void (* PF)();\n"
                            "typedef void * (* PFV)(void *);\n"
                            "PF pf;\n"
                            "PFV pfv;";

        const char expected[] =
            ""
            ""
            "void ( * pf ) ( ) ; "
            "void * ( * pfv ) ( void * ) ;";

        ASSERT_EQUALS(expected, simplifyTypedef(code));
    }

    void simplifyTypedef22() {
        {
            const char code[] = "class Fred {\n"
                                "    typedef void (*testfp)();\n"
                                "    testfp get() { return test; }\n"
                                "    static void test() { }\n"
                                "};";

            const char expected[] =
                "class Fred { "
                ""
                "void ( * get ( ) ) ( ) { return test ; } "
                "static void test ( ) { } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef void * (*testfp)(void *);\n"
                                "    testfp get() { return test; }\n"
                                "    static void * test(void * p) { return p; }\n"
                                "};\n";

            const char expected[] =
                "class Fred { "
                ""
                "void * ( * get ( ) ) ( void * ) { return test ; } "
                "static void * test ( void * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef unsigned int * (*testfp)(unsigned int *);\n"
                                "    testfp get() { return test; }\n"
                                "    static unsigned int * test(unsigned int * p) { return p; }\n"
                                "};\n";

            const char expected[] =
                "class Fred { "
                ""
                "unsigned int * ( * get ( ) ) ( unsigned int * ) { return test ; } "
                "static unsigned int * test ( unsigned int * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef const unsigned int * (*testfp)(const unsigned int *);\n"
                                "    testfp get() { return test; }\n"
                                "    static const unsigned int * test(const unsigned int * p) { return p; }\n"
                                "};\n";

            // static const gets changed to const static
            const char expected[] =
                "class Fred { "
                ""
                "const unsigned int * ( * get ( ) ) ( const unsigned int * ) { return test ; } "
                "const static unsigned int * test ( const unsigned int * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "class Fred {\n"
                                "    typedef void * (*testfp)(void *);\n"
                                "    testfp get(int i) { return test; }\n"
                                "    static void * test(void * p) { return p; }\n"
                                "};\n";

            const char expected[] =
                "class Fred { "
                ""
                "void * ( * get ( int i ) ) ( void * ) { return test ; } "
                "static void * test ( void * p ) { return p ; } "
                "} ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef23() {
        const char code[] = "typedef bool (*Callback) (int i);\n"
                            "void    addCallback(Callback callback) { }\n"
                            "void    addCallback1(Callback callback, int j) { }";

        const char expected[] =
            "void addCallback ( bool * callback ) { } "
            "void addCallback1 ( bool * callback , int j ) { }";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef24() {
        {
            const char code[] = "typedef int (*fp)();\n"
                                "void g( fp f )\n"
                                "{\n"
                                "  fp f2 = (fp)f;\n"
                                "}";

            const char expected[] =
                "void g ( int * f ) "
                "{ "
                "int * f2 ; f2 = ( int * ) f ; "
                "}";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "typedef int (*fp)();\n"
                                "void g( fp f )\n"
                                "{\n"
                                "  fp f2 = static_cast<fp>(f);\n"
                                "}";

            const char expected[] =
                "void g ( int * f ) "
                "{ "
                "int * f2 ; f2 = static_cast < int * > ( f ) ; "
                "}";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef25() {
        {
            // ticket #1298
            const char code[] = "typedef void (*fill_names_f) (const char *);\n"
                                "struct vfs_class {\n"
                                "    void (*fill_names) (struct vfs_class *me, fill_names_f);\n"
                                "}";

            const char expected[] =
                "struct vfs_class { "
                "void ( * fill_names ) ( struct vfs_class * me , void ( * ) ( const char * ) ) ; "
                "}";

            ASSERT_EQUALS(expected, simplifyTypedef(code));
        }

        {
            const char code[] = "typedef void (*fill_names_f) (const char *);\n"
                                "struct vfs_class {\n"
                                "    void (*fill_names) (fill_names_f, struct vfs_class *me);\n"
                                "}";

            const char expected[] =
                "struct vfs_class { "
                "void ( * fill_names ) ( void ( * ) ( const char * ) , struct vfs_class * me ) ; "
                "}";

            ASSERT_EQUALS(expected, simplifyTypedef(code));
        }
    }

    void simplifyTypedef26() {
        {
            const char code[] = "typedef void (*Callback) ();\n"
                                "void    addCallback(Callback (*callback)());";

            const char expected[] = "void addCallback ( void ( * ( * callback ) ( ) ) ( ) ) ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            // ticket # 1307
            const char code[] = "typedef void (*pc_video_update_proc)(bitmap_t *bitmap,\n"
                                "struct mscrtc6845 *crtc);\n"
                                "\n"
                                "struct mscrtc6845 *pc_video_start(pc_video_update_proc (*choosevideomode)(running_machine *machine, int *width, int *height, struct mscrtc6845 *crtc));";

            const char expected[] = "struct mscrtc6845 * pc_video_start ( void ( * ( * choosevideomode ) ( running_machine * machine , int * width , int * height , struct mscrtc6845 * crtc ) ) ( bitmap_t * bitmap , struct mscrtc6845 * crtc ) ) ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef27() {
        // ticket #1316
        const char code[] = "int main()\n"
                            "{\n"
                            "    typedef int (*func_ptr)(float, double);\n"
                            "    VERIFY((is_same<result_of<func_ptr(char, float)>::type, int>::value));\n"
                            "}";

        const char expected[] =
            "int main ( ) "
            "{ "
            ""
            "VERIFY ( is_same < result_of < int ( * ( char , float ) ) ( float , double ) > :: type , int > :: value ) ; "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef28() {
        const char code[] = "typedef std::pair<double, double> (*F)(double);\n"
                            "F f;";

        const char expected[] = "std :: pair < double , double > ( * f ) ( double ) ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef29() {
        const char code[] = "typedef int array [ice_or<is_int<int>::value, is_int<UDT>::value>::value ? 1 : -1];\n"
                            "typedef int array1 [N];\n"
                            "typedef int array2 [N][M];\n"
                            "typedef int int_t, int_array[N];\n"
                            "array a;\n"
                            "array1 a1;\n"
                            "array2 a2;\n"
                            "int_t t;\n"
                            "int_array ia;";

        const char expected[] =
            "int a [ ice_or < is_int < int > :: value , is_int < UDT > :: value > :: value ? 1 : -1 ] ; "
            "int a1 [ N ] ; "
            "int a2 [ N ] [ M ] ; "
            "int t ; "
            "int ia [ N ] ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef30() {
        const char code[] = "typedef ::std::list<int> int_list;\n"
                            "typedef ::std::list<int>::iterator int_list_iterator;\n"
                            "typedef ::std::list<int> int_list_array[10];\n"
                            "int_list il;\n"
                            "int_list_iterator ili;\n"
                            "int_list_array ila;";

        const char expected[] =
            ":: std :: list < int > il ; "
            ":: std :: list < int > :: iterator ili ; "
            ":: std :: list < int > ila [ 10 ] ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef31() {
        {
            const char code[] = "class A {\n"
                                "public:\n"
                                "    typedef int INT;\n"
                                "    INT get() const;\n"
                                "    void put(INT x) { a = x; }\n"
                                "    INT a;\n"
                                "};\n"
                                "A::INT A::get() const { return a; }\n"
                                "A::INT i = A::a;";

            const char expected[] = "class A { "
                                    "public: "
                                    ""
                                    "int get ( ) const ; "
                                    "void put ( int x ) { a = x ; } "
                                    "int a ; "
                                    "} ; "
                                    "int A :: get ( ) const { return a ; } "
                                    "int i ; i = A :: a ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "struct A {\n"
                                "    typedef int INT;\n"
                                "    INT get() const;\n"
                                "    void put(INT x) { a = x; }\n"
                                "    INT a;\n"
                                "};\n"
                                "A::INT A::get() const { return a; }\n"
                                "A::INT i = A::a;";

            const char expected[] = "struct A { "
                                    ""
                                    "int get ( ) const ; "
                                    "void put ( int x ) { a = x ; } "
                                    "int a ; "
                                    "} ; "
                                    "int A :: get ( ) const { return a ; } "
                                    "int i ; i = A :: a ;";

            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef32() {
        const char code[] = "typedef char CHAR;\n"
                            "typedef CHAR * LPSTR;\n"
                            "typedef const CHAR * LPCSTR;\n"
                            "CHAR c;\n"
                            "LPSTR cp;\n"
                            "LPCSTR ccp;";

        const char expected[] =
            "char c ; "
            "char * cp ; "
            "const char * ccp ;";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef33() {
        const char code[] = "class A {\n"
                            "public:\n"
                            "    typedef char CHAR_A;\n"
                            "    CHAR_A funA();\n"
                            "    class B {\n"
                            "    public:\n"
                            "        typedef short SHRT_B;\n"
                            "        SHRT_B funB();\n"
                            "        class C {\n"
                            "        public:\n"
                            "            typedef int INT_C;\n"
                            "            INT_C funC();\n"
                            "            struct D {\n"
                            "                typedef long LONG_D;\n"
                            "                LONG_D funD();\n"
                            "                LONG_D d;\n"
                            "            };\n"
                            "            INT_C c;\n"
                            "        };\n"
                            "        SHRT_B b;\n"
                            "    };\n"
                            "    CHAR_A a;\n"
                            "};\n"
                            "A::CHAR_A A::funA() { return a; }\n"
                            "A::B::SHRT_B A::B::funB() { return b; }\n"
                            "A::B::C::INT_C A::B::C::funC() { return c; }"
                            "A::B::C::D::LONG_D A::B::C::D::funD() { return d; }";

        const char expected[] =
            "class A { "
            "public: "
            ""
            "char funA ( ) ; "
            "class B { "
            "public: "
            ""
            "short funB ( ) ; "
            "class C { "
            "public: "
            ""
            "int funC ( ) ; "
            "struct D { "
            ""
            "long funD ( ) ; "
            "long d ; "
            "} ; "
            "int c ; "
            "} ; "
            "short b ; "
            "} ; "
            "char a ; "
            "} ; "
            "char A :: funA ( ) { return a ; } "
            "short A :: B :: funB ( ) { return b ; } "
            "int A :: B :: C :: funC ( ) { return c ; } "
            "long A :: B :: C :: D :: funD ( ) { return d ; }";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef34() {
        // ticket #1411
        const char code[] = "class X { };\n"
                            "typedef X (*foofunc)(const X&);\n"
                            "int main()\n"
                            "{\n"
                            "    foofunc *Foo = new foofunc[2];\n"
                            "}";
        const char expected[] =
            "class X { } ; "
            "int main ( ) "
            "{ "
            "X ( * * Foo ) ( const X & ) = new X ( * ) ( const X & ) [ 2 ] ; "
            "}";

        ASSERT_EQUALS(expected, tok(code, false));
    }

    // Check simplifyTypedef
    void checkSimplifyTypedef(const char code[]) {
        errout.str("");
        // Tokenize..
        Settings settings;
        settings.inconclusive = true;
        settings.addEnabled("style");
        settings.debugwarnings = true;   // show warnings about unhandled typedef
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
    }

    void simplifyTypedef35() {
        const char code[] = "typedef int A;\n"
                            "class S\n"
                            "{\n"
                            "public:\n"
                            "    typedef float A;\n"
                            "    A a;\n"
                            "    virtual void fun(A x);\n"
                            "};\n"
                            "void S::fun(S::A) { };\n"
                            "class S1 : public S\n"
                            "{\n"
                            "public:\n"
                            "    void fun(S::A) { }\n"
                            "};\n"
                            "struct T\n"
                            "{\n"
                            "    typedef A B;\n"
                            "    B b;\n"
                            "};\n"
                            "float fun1(float A) { return A; }\n"
                            "float fun2(float a) { float A = a++; return A; }\n"
                            "float fun3(int a)\n"
                            "{\n"
                            "    typedef struct { int a; } A;\n"
                            "    A s; s.a = a;\n"
                            "    return s.a;\n"
                            "}\n"
                            "int main()\n"
                            "{\n"
                            "    A a = 0;\n"
                            "    S::A s = fun1(a) + fun2(a) - fun3(a);\n"
                            "    return a + s;\n"
                            "}";

        const char expected[] = "class S "
                                "{ "
                                "public: "
                                ""
                                "float a ; "
                                "virtual void fun ( float x ) ; "
                                "} ; "
                                "void S :: fun ( float ) { } ; "
                                "class S1 : public S "
                                "{ "
                                "public: "
                                "void fun ( float ) { } "
                                "} ; "
                                "struct T "
                                "{ "
                                ""
                                "int b ; "
                                "} ; "
                                "float fun1 ( float A ) { return A ; } "
                                "float fun2 ( float a ) { float A ; A = a ++ ; return A ; } "
                                "float fun3 ( int a ) "
                                "{ "
                                "struct A { int a ; } ; "
                                "struct A s ; s . a = a ; "
                                "return s . a ; "
                                "} "
                                "int main ( ) "
                                "{ "
                                "int a ; a = 0 ; "
                                "float s ; s = fun1 ( a ) + fun2 ( a ) - fun3 ( a ) ; "
                                "return a + s ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:1]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n"
                      "[test.cpp:20] -> [test.cpp:1]: (style, inconclusive) The function parameter 'A' hides a typedef with the same name.\n"
                      "[test.cpp:21] -> [test.cpp:1]: (style, inconclusive) The variable 'A' hides a typedef with the same name.\n"
                      "[test.cpp:24] -> [test.cpp:1]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n", errout.str());
    }

    void simplifyTypedef36() {
        // ticket #1434
        const char code[] = "typedef void (*TIFFFaxFillFunc)();\n"
                            "void f(va_list ap)\n"
                            "{\n"
                            "    *va_arg(ap, TIFFFaxFillFunc*) = 0;\n"
                            "}";
        const char expected[] = "void f ( va_list ap ) "
                                "{ "
                                "* va_arg ( ap , void ( * * ) ( ) ) = 0 ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code, false));
    }

    void simplifyTypedef37() {
        {
            // ticket #1449
            const char code[] = "template <class T> class V {};\n"
                                "typedef V<int> A;\n"
                                "typedef int B;\n"
                                "typedef V<int> A;\n"
                                "typedef int B;";

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n"
                          "[test.cpp:5] -> [test.cpp:3]: (style, inconclusive) The typedef 'B' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef int INT;\n"
                                "void f()\n"
                                "{\n"
                                "    INT i; { }\n"
                                "}";
            const char expected[] = "void f ( ) "
                                    "{ "
                                    "int i ; { } "
                                    "}";
            ASSERT_EQUALS(expected, tok(code, false));
        }
    }

    void simplifyTypedef38() {
        const char code[] = "typedef C A;\n"
                            "struct AB : public A, public B { };";
        const char expected[] = "struct AB : public C , public B { } ;";
        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef39() {
        const char code[] = "typedef int A;\n"
                            "template <const A, volatile A>::value;";
        const char expected[] = "template < const int , int > :: value ;";
        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef40() {
        const char code[] = "typedef int A;\n"
                            "typedef int B;\n"
                            "template <class A, class B> class C { };";
        const char expected[] = "template < class A , class B > class C { } ;";
        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (style, inconclusive) The template parameter 'A' hides a typedef with the same name.\n"
                      "[test.cpp:3] -> [test.cpp:2]: (style, inconclusive) The template parameter 'B' hides a typedef with the same name.\n", errout.str());

        checkSimplifyTypedef("typedef tuple<double&, const double&, const double, double*, const double*> t2;\n"
                             "void ordering_test()\n"
                             "{\n"
                             "  tuple<short, float> t2(5, 3.3f);\n"
                             "  BOOST_CHECK(t3 > t2);\n"
                             "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:1]: (style, inconclusive) The template instantiation 't2' hides a typedef with the same name.\n", errout.str());

        checkSimplifyTypedef("class MyOverflowingUnsigned\n"
                             "{\n"
                             "public:\n"
                             "    typedef unsigned self_type::*  bool_type;\n"
                             "    operator bool_type() const  { return this->v_ ? &self_type::v_ : 0; }\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        checkSimplifyTypedef("typedef int (*fptr_type)(int, int);\n"
                             "struct which_one {\n"
                             "  typedef fptr_type (*result_type)(bool x);\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        checkSimplifyTypedef("class my_configuration\n"
                             "{\n"
                             "public:\n"
                             "    template < typename T >\n"
                             "    class hook\n"
                             "    {\n"
                             "    public:\n"
                             "        typedef ::boost::rational<T>  rational_type;\n"
                             "    public:\n"
                             "        rational_type  ( &r_ )[ 9 ];\n"
                             "    };\n"
                             "}");
        ASSERT_EQUALS("", errout.str());

        checkSimplifyTypedef("class A\n"
                             "{\n"
                             "    typedef B b;\n"
                             "    friend b;\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef41() {
        // ticket #1488
        checkSimplifyTypedef("class Y;\n"
                             "class X\n"
                             "{\n"
                             "    typedef Y type;\n"
                             "    friend class type;\n"
                             "};");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef42() {
        // ticket #1506
        checkSimplifyTypedef("typedef struct A { } A;\n"
                             "struct A;");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style) The struct 'A' forward declaration is unnecessary. Type struct is already declared earlier.\n", errout.str());

        checkSimplifyTypedef("typedef union A { int i; float f; } A;\n"
                             "union A;");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style) The union 'A' forward declaration is unnecessary. Type union is already declared earlier.\n", errout.str());

        checkSimplifyTypedef("typedef std::map<std::string, int> A;\n"
                             "class A;");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style) The class 'A' forward declaration is unnecessary. Type class is already declared earlier.\n", errout.str());
    }

    void simplifyTypedef43() {
        // ticket #1588
        {
            const char code[] = "typedef struct foo A;\n"
                                "struct A\n"
                                "{\n"
                                "    int alloclen;\n"
                                "};\n";

            // The expected result..
            const std::string expected("struct A "
                                       "{ "
                                       "int alloclen ; "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The struct 'A' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef union foo A;\n"
                                "union A\n"
                                "{\n"
                                "    int alloclen;\n"
                                "};\n";

            // The expected result..
            const std::string expected("union A "
                                       "{ "
                                       "int alloclen ; "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The union 'A' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef class foo A;\n"
                                "class A\n"
                                "{\n"
                                "    int alloclen;\n"
                                "};\n";

            // The expected result..
            const std::string expected("class A "
                                       "{ "
                                       "int alloclen ; "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The class 'A' hides a typedef with the same name.\n", errout.str());
        }
    }

    void simplifyTypedef44() {
        {
            const char code[] = "typedef std::map<std::string, int> Map;\n"
                                "class MyMap : public Map\n"
                                "{\n"
                                "};\n";

            // The expected result..
            const std::string expected("class MyMap : public std :: map < std :: string , int > "
                                       "{ "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef std::map<std::string, int> Map;\n"
                                "class MyMap : protected Map\n"
                                "{\n"
                                "};\n";

            // The expected result..
            const std::string expected("class MyMap : protected std :: map < std :: string , int > "
                                       "{ "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef std::map<std::string, int> Map;\n"
                                "class MyMap : private Map\n"
                                "{\n"
                                "};\n";

            // The expected result..
            const std::string expected("class MyMap : private std :: map < std :: string , int > "
                                       "{ "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef struct foo { } A;\n"
                                "struct MyA : public A\n"
                                "{\n"
                                "};\n";

            // The expected result..
            const std::string expected("struct foo { } ; "
                                       "struct MyA : public foo "
                                       "{ "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef class foo { } A;\n"
                                "class MyA : public A\n"
                                "{\n"
                                "};\n";

            // The expected result..
            const std::string expected("class foo { } ; "
                                       "class MyA : public foo "
                                       "{ "
                                       "} ;");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef45() {
        // ticket # 1613
        const char code[] = "void fn() {\n"
                            "    typedef foo<> bar;\n"
                            "    while (0 > bar(1)) {}\n"
                            "}";

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef46() {
        const char code[] = "typedef const struct A { int a; } * AP;\n"
                            "AP ap;\n";

        // The expected result..
        const std::string expected("struct A { int a ; } ; "
                                   "const struct A * ap ;");
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef47() {
        {
            const char code[] = "typedef std::pair<int, int> const I;\n"
                                "I i;";

            // The expected result..
            const std::string expected("std :: pair < int , int > const i ;");
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "typedef void (X:: *F)();\n"
                                "F f;";

            // The expected result..
            const std::string expected("void ( X :: * f ) ( ) ;");
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void simplifyTypedef48() { // ticket #1673
        const char code[] = "typedef struct string { } string;\n"
                            "void foo (LIST *module_name)\n"
                            "{\n"
                            "    bar(module_name ? module_name->string : 0);\n"
                            "}\n";

        // The expected result..
        const std::string expected("struct string { } ; "
                                   "void foo ( LIST * module_name ) "
                                   "{ "
                                   "bar ( module_name ? module_name . string : 0 ) ; "
                                   "}");
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef49() { // ticket #1691
        const char code[] = "class Class2 {\n"
                            "typedef const Class & Const_Reference;\n"
                            "void some_method (Const_Reference x) const {}\n"
                            "void another_method (Const_Reference x) const {}\n"
                            "}";

        // The expected result..
        const std::string expected("class Class2 { "
                                   ""
                                   "void some_method ( const Class & x ) const { } "
                                   "void another_method ( const Class & x ) const { } "
                                   "}");
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef50() {
        const char code[] = "typedef char (* type1)[10];\n"
                            "typedef char (& type2)[10];\n"
                            "typedef char (& type3)[x];\n"
                            "typedef char (& type4)[x + 2];\n"
                            "type1 t1;\n"
                            "type1 (*tp1)[2];\n"
                            "type2 t2;\n"
                            "type3 t3;\n"
                            "type4 t4;";

        // The expected result..
        const std::string expected("char ( * t1 ) [ 10 ] ; "
                                   "char ( * ( * tp1 ) [ 2 ] ) [ 10 ] ; "
                                   "char ( & t2 ) [ 10 ] ; "
                                   "char ( & t3 ) [ x ] ; "
                                   "char ( & t4 ) [ x + 2 ] ;");
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef51() {
        const char code[] = "class A { public: int i; };\n"
                            "typedef const char (A :: * type1);\n"
                            "type1 t1 = &A::i;";

        // The expected result..
        const std::string expected("class A { public: int i ; } ; "
                                   "const char ( A :: * t1 ) = & A :: i ;");
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef52() { // ticket #1782
        {
            const char code[] = "typedef char (* type1)[10];\n"
                                "type1 foo() { }";

            // The expected result..
            const std::string expected("char ( * foo ( ) ) [ 10 ] { }");
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef char (* type1)[10];\n"
                                "LOCAL(type1) foo() { }";

            // this is invalid C so just make sure it doesn't generate an internal error
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef53() { // ticket #1801
        {
            const char code[] = "typedef int ( * int ( * ) ( ) ) ( ) ;";

            // this is invalid C so just make sure it doesn't crash
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:1]: (debug) Failed to parse 'typedef int ( * int ( * ) ( ) ) ( ) ;'. The checking continues anyway.\n", errout.str());
        }

        {
            const char code[] = "typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);\n"
                                "typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);\n";

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The typedef 'PPDMarkOption' hides a typedef with the same name.\n", errout.str());
        }

        {
            const char code[] = "typedef int * A;\n"
                                "typedef int * A;\n";
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:1]: (style, inconclusive) The typedef 'A' hides a typedef with the same name.\n", errout.str());
        }
    }

    void simplifyTypedef54() { // ticket #1814
        const char code[] = "void foo()\n"
                            "{\n"
                            "    typedef std::basic_string<char, traits_type, allocator_type> string_type;\n"
                            "    try\n"
                            "    {\n"
                            "        throw string_type(\"leak\");\n"
                            "    }\n"
                            "    catch (const string_type&)\n"
                            "    {\n"
                            "        pthread_exit (0);\n"
                            "    }\n"
                            "}";

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef55() {
        const char code[] = "typedef volatile unsigned long * const hwreg_t ;\n"
                            "typedef void *const t1[2];\n"
                            "typedef int*const *_Iterator;\n"
                            "hwreg_t v1;\n"
                            "t1 v2;\n"
                            "_Iterator v3;\n";

        // The expected result..
        const std::string expected("long * v1 ; "
                                   "void * v2 [ 2 ] ; "
                                   "int * * v3 ;");
        ASSERT_EQUALS(expected, tok(code));

        // Check for output..
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef56() { // ticket #1829
        const char code[] = "struct C {\n"
                            "    typedef void (*fptr)();\n"
                            "    const fptr pr;\n"
                            "    operator const fptr& () { return pr; }\n"
                            "};\n";

        // The expected result..
        const std::string expected("struct C { "
                                   ""
                                   "const void * pr ; " // this gets simplified to a regular pointer
                                   "operatorconstvoid(*)()& ( ) { return pr ; } "
                                   "} ;");
        ASSERT_EQUALS(expected, tok(code));

        // Check for output..
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef57() { // ticket #1846
        const char code[] = "void foo {\n"
                            "    typedef int A;\n"
                            "    A a = A(1) * A(2);\n"
                            "};\n";

        // The expected result..
        const std::string expected("void foo { "
                                   ""
                                   "int a ; a = int ( 1 ) * int ( 2 ) ; "
                                   "} ;");
        ASSERT_EQUALS(expected, tok(code));

        // Check for output..
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef58() { // ticket #1963
        {
            const char code[] = "typedef int vec2_t[2];\n"
                                "vec2_t coords[4] = {1,2,3,4,5,6,7,8};\n";

            // The expected result..
            const std::string expected("int coords [ 4 ] [ 2 ] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 } ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef int vec2_t[2];\n"
                                "vec2_t coords[4][5][6+1] = {1,2,3,4,5,6,7,8};\n";

            // The expected result..
            const std::string expected("int coords [ 4 ] [ 5 ] [ 7 ] [ 2 ] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 } ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef59() { // ticket #2011
        const char code[] = "template<typename DISPATCHER> class SomeTemplateClass {\n"
                            "    typedef void (SomeTemplateClass<DISPATCHER>::*MessageDispatcherFunc)(SerialInputMessage&);\n"
                            "};\n";
        // The expected result..
        const std::string expected("template < typename DISPATCHER > class SomeTemplateClass { } ;");
        ASSERT_EQUALS(expected, tok(code));

        // Check for output..
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef60() { // ticket #2035
        const char code[] = "typedef enum {qfalse, qtrue} qboolean;\n"
                            "typedef qboolean (*localEntitiyAddFunc_t) (struct le_s * le, entity_t * ent);\n"
                            "void f()\n"
                            "{\n"
                            "    qboolean b;\n"
                            "    localEntitiyAddFunc_t f;\n"
                            "}\n";
        // The expected result..
        const std::string expected("void f ( ) { int b ; int * f ; }");
        ASSERT_EQUALS(expected, tok(code, false));

        // Check for output..
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef61() { // ticket #2074 and 2075
        const char code1[] = "typedef       unsigned char   (*Mf_GetIndexByte_Func)          (void);\n"
                             "typedef const unsigned char * (*Mf_GetPointerToCurrentPos_Func)(void);\n";

        // Check for output..
        checkSimplifyTypedef(code1);
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "typedef unsigned long uint32_t;\n"
                             "typedef uint32_t (*write_type_t) (uint32_t);\n";

        // Check for output..
        checkSimplifyTypedef(code2);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef62() { // ticket #2082
        const char code1[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a, b;\n"
                             "}";

        // The expected tokens..
        const std::string expected1("void f ( ) { char a [ 256 ] ; char b [ 256 ] ; }");
        ASSERT_EQUALS(expected1, tok(code1, false));

        // Check for output..
        checkSimplifyTypedef(code1);
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a = { 0 }, b = { 0 };\n"
                             "}";

        // The expected tokens..
        const std::string expected2("void f ( ) { char a [ 256 ] = { 0 } ; char b [ 256 ] = { 0 } ; }");
        ASSERT_EQUALS(expected2, tok(code2, false));

        // Check for output..
        checkSimplifyTypedef(code2);
        ASSERT_EQUALS("", errout.str());

        const char code3[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a = \"\", b = \"\";\n"
                             "}";

        // The expected tokens..
        const std::string expected3("void f ( ) { char a [ 256 ] = \"\" ; char b [ 256 ] = \"\" ; }");
        ASSERT_EQUALS(expected3, tok(code3, false));

        // Check for output..
        checkSimplifyTypedef(code3);
        ASSERT_EQUALS("", errout.str());

        const char code4[] = "typedef char TString[256];\n"
                             "void f()\n"
                             "{\n"
                             "    TString a = \"1234\", b = \"5678\";\n"
                             "}";

        // The expected tokens..
        const std::string expected4("void f ( ) { char a [ 256 ] = \"1234\" ; char b [ 256 ] = \"5678\" ; }");
        ASSERT_EQUALS(expected4, tok(code4, false));

        // Check for output..
        checkSimplifyTypedef(code4);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef63() { // ticket #2175 'typedef float x[3];'
        const char code[] = "typedef float x[3];\n"
                            "x a,b,c;\n";
        const std::string actual(tok(code));
        ASSERT_EQUALS("float a [ 3 ] ; float b [ 3 ] ; float c [ 3 ] ;", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef64() {
        const char code[] = "typedef __typeof__(__type1() + __type2()) __type;"
                            "__type t;\n";
        const std::string actual(tok(code));
        ASSERT_EQUALS("__typeof__ ( __type1 ( ) + __type2 ( ) ) t ;", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef65() { // ticket #2314
        const char code[] = "typedef BAR<int> Foo;\n"
                            "int main() {\n"
                            "    Foo b(0);\n"
                            "    return b > Foo(10);\n"
                            "}";
        const std::string actual(tok(code));
        ASSERT_EQUALS("int main ( ) { BAR < int > b ( 0 ) ; return b > BAR < int > ( 10 ) ; }", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef66() { // ticket #2341
        const char code[] = "typedef long* GEN;\n"
                            "extern GEN (*foo)(long);";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef67() { // ticket #2354
        const char code[] = "typedef int ( * Function ) ( ) ;\n"
                            "void f ( ) {\n"
                            "    ((Function * (*) (char *, char *, int, int)) global[6]) ( \"assoc\", \"eggdrop\", 106, 0);\n"
                            "}\n";
        const std::string expected = "void f ( ) { "
                                     "( ( int ( * * ( * ) ( char * , char * , int , int ) ) ( ) ) global [ 6 ] ) ( \"assoc\" , \"eggdrop\" , 106 , 0 ) ; "
                                     "}";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef68() { // ticket #2355
        const char code[] = "typedef FMAC1 void (* a) ();\n"
                            "void *(*b) ();\n";
        const std::string actual(tok(code));
        ASSERT_EQUALS("void * * b ;", actual);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef69() { // ticket #2348
        const char code[] = "typedef int (*CompilerHook)();\n"
                            "typedef struct VirtualMachine\n"
                            "{\n"
                            "    CompilerHook *(*compilerHookVector)(void);\n"
                            "}VirtualMachine;\n";
        const std::string expected = "struct VirtualMachine "
                                     "{ "
                                     "int ( * * ( * compilerHookVector ) ( void ) ) ( ) ; "
                                     "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef70() { // ticket #2348
        const char code[] = "typedef int pread_f ( int ) ;\n"
                            "pread_f *(*test_func)(char *filename);\n";
        const std::string expected = "int ( * ( * test_func ) ( char * filename ) ) ( int ) ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef71() { // ticket #2348
        {
            const char code[] = "typedef int RexxFunctionHandler();\n"
                                "RexxFunctionHandler *(efuncs[1]);\n";
            const std::string expected = "int ( * ( efuncs [ 1 ] ) ) ( ) ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        {
            const char code[] = "typedef int RexxFunctionHandler();\n"
                                "RexxFunctionHandler *(efuncs[]) = { NULL, NULL };\n";
            const std::string expected = "int ( * ( efuncs [ ] ) ) ( ) = { 0 , 0 } ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef72() { // ticket #2374
        // inline operator
        {
            const char code[] = "class Fred {\n"
                                "    typedef int* (Fred::*F);\n"
                                "    operator F() const { }\n"
                                "};\n";
            const std::string expected = "class Fred { "
                                         ""
                                         "operatorint** ( ) const { } "
                                         "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        // inline local variable
        {
            const char code[] = "class Fred {\n"
                                "    typedef int INT;\n"
                                "    void f1() const { INT i; }\n"
                                "};\n";
            const std::string expected = "class Fred { "
                                         ""
                                         "void f1 ( ) const { int i ; } "
                                         "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        // out of line member variable
        {
            const char code[] = "class Fred {\n"
                                "    typedef int INT;\n"
                                "    void f1() const;\n"
                                "};\n"
                                "void Fred::f1() const { INT i; f(i); }\n";
            const std::string expected = "class Fred { "
                                         ""
                                         "void f1 ( ) const ; "
                                         "} ; "
                                         "void Fred :: f1 ( ) const { int i ; f ( i ) ; }";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
        // out of line operator
        {
            const char code[] = "class Fred {\n"
                                "    typedef int* (Fred::*F);\n"
                                "    operator F() const;\n"
                                "};\n"
                                "Fred::operator F() const { }\n";
            const std::string expected = "class Fred { "
                                         ""
                                         "operatorint** ( ) const ; "
                                         "} ; "
                                         "Fred :: operatorint** ( ) const { }";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedef73() { // ticket #2412
        const char code[] = "struct B {};\n"
                            "typedef struct A : public B {\n"
                            "    void f();\n"
                            "} a, *aPtr;\n";
        const std::string expected = "struct B { } ; "
                                     "struct A : public B { "
                                     "void f ( ) ; "
                                     "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef74() { // ticket #2414
        const char code[] = "typedef long (*state_func_t)(void);\n"
                            "typedef state_func_t (*state_t)(void);\n"
                            "state_t current_state = death;\n"
                            "static char get_runlevel(const state_t);\n";
        const std::string expected = "long ( * ( * current_state ) ( void ) ) ( void ) = death ; "
                                     "static char get_runlevel ( const long ( * ( * ) ( void ) ) ( void ) ) ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef75() { // ticket #2426
        const char code[] = "typedef _Packed struct S { long l; };\n";
        const std::string expected = "";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef76() { // ticket #2453 segmentation fault
        const char code[] = "void f1(typedef int x) {}\n";
        const std::string expected = "void f1 ( typedef int x ) { }";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef77() { // ticket #2554
        const char code[] = "typedef char Str[10]; int x = sizeof(Str);\n";
        const std::string expected = "int x ; x = 10 ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef78() { // ticket #2568
        const char code[] = "typedef struct A A_t;\n"
                            "A_t a;\n"
                            "typedef struct A { } A_t;\n"
                            "A_t a1;\n";
        const std::string expected = "struct A a ; struct A { } ; struct A a1 ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef79() { // ticket #2348
        const char code[] = "typedef int (Tcl_ObjCmdProc) (int x);\n"
                            "typedef struct LangVtab\n"
                            "{\n"
                            "    Tcl_ObjCmdProc * (*V_LangOptionCommand);\n"
                            "} LangVtab;\n";
        const std::string expected = "struct LangVtab "
                                     "{ "
                                     "int ( * ( * V_LangOptionCommand ) ) ( int x ) ; "
                                     "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef80() { // ticket #2587
        const char code[] = "typedef struct s { };\n"
                            "void f() {\n"
                            "    sizeof(struct s);\n"
                            "};\n";
        const std::string expected = "struct s { } ; "
                                     "void f ( ) { "
                                     "sizeof ( struct s ) ; "
                                     "} ;";
        ASSERT_EQUALS(expected, tok(code));

        // Check for output..
        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef81() { // ticket #2603 segmentation fault
        checkSimplifyTypedef("typedef\n");
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());

        checkSimplifyTypedef("typedef constexpr\n");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef82() { // ticket #2403
        checkSimplifyTypedef("class A {\n"
                             "public:\n"
                             "  typedef int F(int idx);\n"
                             "};\n"
                             "class B {\n"
                             "public:\n"
                             "  A::F ** f;\n"
                             "};\n"
                             "int main()\n"
                             "{\n"
                             "  B * b = new B;\n"
                             "  b->f = new A::F * [ 10 ];\n"
                             "}");
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef83() { // ticket #2620
        const char code[] = "typedef char Str[10];\n"
                            "void f(Str &cl) { }\n";

        // The expected result..
        const std::string expected("void f ( char ( & cl ) [ 10 ] ) { }");

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedef84() { // ticket #2630 (segmentation fault)
        const char code1[] = "typedef y x () x\n";
        checkSimplifyTypedef(code1);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());

        const char code2[] = "typedef struct template <>\n";
        checkSimplifyTypedef(code2);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());

        const char code3[] = "typedef ::<>\n";
        checkSimplifyTypedef(code3);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
    }

    void simplifyTypedef85() { // ticket #2651
        const char code[] = "typedef FOO ((BAR)(void, int, const int, int*));\n";
        const char expected[] = ";";
        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef86() { // ticket #2581
        const char code[] = "class relational {\n"
                            "    typedef void (safe_bool_helper::*safe_bool)();\n"
                            "public:\n"
                            "    operator safe_bool() const;\n"
                            "    safe_bool operator!() const;\n"
                            "};\n";
        const char expected[] = "class relational { "
                                ""
                                "public: "
                                "operatorsafe_bool ( ) const ; "
                                "safe_bool operator! ( ) const ; "
                                "} ;";
        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef87() { // ticket #2651
        const char code[] = "typedef FOO (*(*BAR)(void, int, const int, int*));\n";
        const char expected[] = ";";
        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef88() { // ticket #2675
        const char code[] = "typedef short int (*x)(...);\n";
        const char expected[] = ";";
        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef89() { // ticket #2717
        const char code[] = "class Fred {\n"
                            "    typedef void f(int) const;\n"
                            "    f func;\n"
                            "};\n";
        const char expected[] = "class Fred { void func ( int ) const ; } ;";
        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef90() { // ticket #2718
        const char code[] = "typedef int IA[2];\n"
                            "void f(const IA&) {};\n";
        const char expected[] = "void f ( const int ( & ) [ 2 ] ) { } ;";
        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef91() { // ticket #2716
        const char code1[] = "namespace NS {\n"
                             "    typedef int (*T)();\n"
                             "    class A {\n"
                             "        T f();\n"
                             "    };\n"
                             "}\n"
                             "namespace NS {\n"
                             "    T A::f() {}\n"
                             "}\n";
        const char expected1[] = "namespace NS { "
                                 ""
                                 "class A { "
                                 "int ( * f ( ) ) ( ) ; "
                                 "} ; "
                                 "} "
                                 "namespace NS { "
                                 "int ( * A :: f ( ) ) ( ) { } "
                                 "}";
        checkSimplifyTypedef(code1);
        ASSERT_EQUALS(expected1, tok(code1));
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "namespace NS {\n"
                             "    typedef int (*T)();\n"
                             "    class A {\n"
                             "        T f();\n"
                             "    };\n"
                             "}\n"
                             "NS::T NS::A::f() {}\n";
        const char expected2[] = "namespace NS { "
                                 ""
                                 "class A { "
                                 "int ( * f ( ) ) ( ) ; "
                                 "} ; "
                                 "} "
                                 "int ( * NS :: A :: f ( ) ) ( ) { }";
        checkSimplifyTypedef(code2);
        ASSERT_EQUALS(expected2, tok(code2));
        ASSERT_EQUALS("", errout.str());

        const char code3[] = "namespace NS1 {\n"
                             "    namespace NS2 {\n"
                             "        typedef int (*T)();\n"
                             "        class A {\n"
                             "            T f();\n"
                             "        };\n"
                             "    }\n"
                             "}\n"
                             "namespace NS1 {\n"
                             "    namespace NS2 {\n"
                             "        T A::f() {}\n"
                             "    }\n"
                             "}\n";
        const char expected3[] = "namespace NS1 { "
                                 "namespace NS2 { "
                                 ""
                                 "class A { "
                                 "int ( * f ( ) ) ( ) ; "
                                 "} ; "
                                 "} "
                                 "} "
                                 "namespace NS1 { "
                                 "namespace NS2 { "
                                 "int ( * A :: f ( ) ) ( ) { } "
                                 "} "
                                 "}";
        checkSimplifyTypedef(code3);
        ASSERT_EQUALS(expected3, tok(code3));
        ASSERT_EQUALS("", errout.str());

        const char code4[] = "namespace NS1 {\n"
                             "    namespace NS2 {\n"
                             "        typedef int (*T)();\n"
                             "        class A {\n"
                             "            T f();\n"
                             "        };\n"
                             "    }\n"
                             "}\n"
                             "namespace NS1 {\n"
                             "    NS2::T NS2::A::f() {}\n"
                             "}\n";
        const char expected4[] = "namespace NS1 { "
                                 "namespace NS2 { "
                                 ""
                                 "class A { "
                                 "int ( * f ( ) ) ( ) ; "
                                 "} ; "
                                 "} "
                                 "} "
                                 "namespace NS1 { "
                                 "int ( * NS2 :: A :: f ( ) ) ( ) { } "
                                 "}";
        checkSimplifyTypedef(code4);
        ASSERT_EQUALS(expected4, tok(code4));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef92() { // ticket #2736 (segmentation fault)
        const char code[] = "typedef long Long;\n"
                            "namespace NS {\n"
                            "}\n";

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef93() { // ticket #2738 (syntax error)
        const char code[] = "struct s { double x; };\n"
                            "typedef struct s (*binop) (struct s, struct s);\n";
        const char expected[] = "struct s { double x ; } ;";

        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef94() { // ticket #1982
        const char code1[] = "class A {\n"
                             "public:\n"
                             "  typedef struct {\n"
                             "    int a[4];\n"
                             "  } data;\n"
                             "};\n"
                             "A::data d;\n";
        const char expected1[] = "class A { "
                                 "public: "
                                 "struct data { "
                                 "int a [ 4 ] ; "
                                 "} ; "
                                 "} ; "
                                 "struct A :: data d ;";

        checkSimplifyTypedef(code1);
        ASSERT_EQUALS(expected1, tok(code1));
        TODO_ASSERT_EQUALS("[test.cpp:7]: (debug) Scope::checkVariable found variable 'd' with varid 0.\n", "", errout.str());

        const char code2[] = "class A {\n"
                             "public:\n"
                             "  typedef struct {\n"
                             "    int a[4];\n"
                             "  } data;\n"
                             "};\n"
                             "::A::data d;\n";
        const char expected2[] = "class A { "
                                 "public: "
                                 "struct data { "
                                 "int a [ 4 ] ; "
                                 "} ; "
                                 "} ; "
                                 "struct :: A :: data d ;";

        checkSimplifyTypedef(code2);
        ASSERT_EQUALS(expected2, tok(code2));
        TODO_ASSERT_EQUALS("[test.cpp:7]: (debug) Scope::checkVariable found variable 'd' with varid 0.\n", "", errout.str());

        const char code3[] = "class A {\n"
                             "public:\n"
                             "  typedef struct {\n"
                             "    int a[4];\n"
                             "  } data;\n"
                             "};\n"
                             "class B : public ::A::data { };\n";
        const char expected3[] = "class A { "
                                 "public: "
                                 "struct data { "
                                 "int a [ 4 ] ; "
                                 "} ; "
                                 "} ; "
                                 "class B : public :: A :: data { } ;";

        checkSimplifyTypedef(code3);
        ASSERT_EQUALS(expected3, tok(code3));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef95() { // ticket #2844
        const char code[] = "class symbol_table {\n"
                            "public:\n"
                            "  typedef expression_error::error_code (*valid_func)(void *cbparam, const char *name, expression_space space);\n"
                            "  valid_func f;\n"
                            "};\n";
        const char expected[] = "class symbol_table { "
                                "public: "
                                ""
                                "expression_error :: error_code ( * f ) ( void * cbparam , const char * name , expression_space space ) ; "
                                "} ;";

        checkSimplifyTypedef(code);
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef96() { // ticket #2886 (segmentation fault)
        const char code[] = "typedef struct x { }\n";
        tok(code);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
    }

    void simplifyTypedef97() { // ticket #2983 (segmentation fault)
        const char code[] = "typedef x y\n"
                            "(A); y\n";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef98() { // ticket #2963
        const char code[] = "typedef int type ## __LINE__;\n"
                            "typedef int type ## __LINE__;\n"
                            "type1 x;\n"
                            "type2 y;";
        ASSERT_EQUALS("int x ; int y ;", tok(code));
    }

    void simplifyTypedef99() { // ticket #2999
        const char code[] = "typedef struct Fred Fred;\n"
                            "struct Fred { };\n";
        tok(code);
        ASSERT_EQUALS("", errout.str());

        const char code1[] = "struct Fred { };\n"
                             "typedef struct Fred Fred;\n";
        tok(code1);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef100() { // ticket #3000
        const char code[] = "typedef struct Fred { } Fred;\n"
                            "Fred * foo() {\n"
                            "    Fred *fred;\n"
                            "    fred = se_alloc(sizeof(struct Fred));\n"
                            "    return fred;\n"
                            "}\n";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef101() { // ticket #3003 (segmentation fault)
        const char code[] = "typedef a x[];\n"
                            "y = x\n";
        tok(code);
        ASSERT_EQUALS("[test.cpp:2]: (error) syntax error\n", errout.str());
    }

    void simplifyTypedef102() { // ticket #3004
        const char code[] = "typedef struct { } Fred;\n"
                            "void foo()\n"
                            "{\n"
                            "    Fred * Fred;\n"
                            "}\n";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef103() { // ticket #3007
        const char code[] = "typedef struct { } Fred;\n"
                            "void foo()\n"
                            "{\n"
                            "    Fred Fred;\n"
                            "}\n";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef104() { // ticket #3070
        const char code[] = "typedef int (*in_func) (void FAR *, unsigned char FAR * FAR *);\n";
        ASSERT_EQUALS(";", tok(code));
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef105() { // ticket #3616 (segmentation fault)
        const char code[] = "( int typedef char x; ){}\n";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef106() { // ticket #3619 (segmentation fault)
        const char code[] = "typedef void f ();\ntypedef { f }";
        tok(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedef107() { // ticket #3963 (bad code => segmentation fault)
        const char code[] = "typedef int x[]; int main() { return x }";
        tok(code);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
    }

    void simplifyTypedefFunction1() {
        {
            const char code[] = "typedef void (*my_func)();\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (*my_func)(void);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( void ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (*my_func)(int);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (*my_func)(int*);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            // ticket # 1615
            const char code[] = "typedef void (*my_func)(arg_class*);\n"
                                "std::queue<my_func> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( arg_class * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }


        {
            const char code[] = "typedef void (my_func)();\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(void);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( void ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(int);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(int*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func)(arg_class*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( arg_class * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }


        {
            const char code[] = "typedef void my_func();\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(void);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( void ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(int);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(int*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void my_func(arg_class*);\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( arg_class * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }


        {
            const char code[] = "typedef void (my_func());\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(void));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( void ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(int));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(int*));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( int * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef void (my_func(arg_class*));\n"
                                "std::queue<my_func *> func_queue;";

            // The expected result..
            const std::string expected("std :: queue < void ( * ) ( arg_class * ) > func_queue ;");
            ASSERT_EQUALS(expected, tok(code));

            // Check for output..
            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedefFunction2() { // ticket #1685
        const char code[] = "typedef void voidfn (int);\n"
                            "voidfn xxx;";

        // The expected result..
        const std::string expected("void xxx ( int ) ;");
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTypedefFunction3() {
        {
            const char code[] = "typedef C func1();\n"
                                "typedef C (* func2)();\n"
                                "typedef C (& func3)();\n"
                                "typedef C (C::* func4)();\n"
                                "typedef C (C::* func5)() const;\n"
                                "typedef C (C::* func6)() volatile;\n"
                                "typedef C (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const std::string expected("C f1 ( ) ; "
                                       "C * f2 ; " // this gets simplified to a regular pointer
                                       "C ( & f3 ) ( ) ; "
                                       "C ( C :: * f4 ) ( ) ; "
                                       "C ( C :: * f5 ) ( ) const ; "
                                       "C ( C :: * f6 ) ( ) ; " // volatile is removed
                                       "C ( C :: * f7 ) ( ) const ;"); // volatile is removed
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef C const func1();\n"
                                "typedef C const (* func2)();\n"
                                "typedef C const (& func3)();\n"
                                "typedef C const (C::* func4)();\n"
                                "typedef C const (C::* func5)() const;\n"
                                "typedef C const (C::* func6)() volatile;\n"
                                "typedef C const (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            // C const -> const C
            const std::string expected("const C f1 ( ) ; "
                                       "const C * f2 ; " // this gets simplified to a regular pointer
                                       "const C ( & f3 ) ( ) ; "
                                       "const C ( C :: * f4 ) ( ) ; "
                                       "const C ( C :: * f5 ) ( ) const ; "
                                       "const C ( C :: * f6 ) ( ) ; " // volatile is removed
                                       "const C ( C :: * f7 ) ( ) const ;"); // volatile is removed
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef const C func1();\n"
                                "typedef const C (* func2)();\n"
                                "typedef const C (& func3)();\n"
                                "typedef const C (C::* func4)();\n"
                                "typedef const C (C::* func5)() const;\n"
                                "typedef const C (C::* func6)() volatile;\n"
                                "typedef const C (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const std::string expected("const C f1 ( ) ; "
                                       "const C * f2 ; " // this gets simplified to a regular pointer
                                       "const C ( & f3 ) ( ) ; "
                                       "const C ( C :: * f4 ) ( ) ; "
                                       "const C ( C :: * f5 ) ( ) const ; "
                                       "const C ( C :: * f6 ) ( ) ; " // volatile is removed
                                       "const C ( C :: * f7 ) ( ) const ;"); // volatile is removed
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef C * func1();\n"
                                "typedef C * (* func2)();\n"
                                "typedef C * (& func3)();\n"
                                "typedef C * (C::* func4)();\n"
                                "typedef C * (C::* func5)() const;\n"
                                "typedef C * (C::* func6)() volatile;\n"
                                "typedef C * (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const std::string expected("C * f1 ( ) ; "
                                       "C * * f2 ; " // this gets simplified to a regular pointer
                                       "C * ( & f3 ) ( ) ; "
                                       "C * ( C :: * f4 ) ( ) ; "
                                       "C * ( C :: * f5 ) ( ) const ; "
                                       "C * ( C :: * f6 ) ( ) ; " // volatile is removed
                                       "C * ( C :: * f7 ) ( ) const ;"); // volatile is removed
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef const C * func1();\n"
                                "typedef const C * (* func2)();\n"
                                "typedef const C * (& func3)();\n"
                                "typedef const C * (C::* func4)();\n"
                                "typedef const C * (C::* func5)() const;\n"
                                "typedef const C * (C::* func6)() volatile;\n"
                                "typedef const C * (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            const std::string expected("const C * f1 ( ) ; "
                                       "const C * * f2 ; " // this gets simplified to a regular pointer
                                       "const C * ( & f3 ) ( ) ; "
                                       "const C * ( C :: * f4 ) ( ) ; "
                                       "const C * ( C :: * f5 ) ( ) const ; "
                                       "const C * ( C :: * f6 ) ( ) ; " // volatile is removed
                                       "const C * ( C :: * f7 ) ( ) const ;"); // volatile is removed
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "typedef C const * func1();\n"
                                "typedef C const * (* func2)();\n"
                                "typedef C const * (& func3)();\n"
                                "typedef C const * (C::* func4)();\n"
                                "typedef C const * (C::* func5)() const;\n"
                                "typedef C const * (C::* func6)() volatile;\n"
                                "typedef C const * (C::* func7)() const volatile;\n"
                                "func1 f1;\n"
                                "func2 f2;\n"
                                "func3 f3;\n"
                                "func4 f4;\n"
                                "func5 f5;\n"
                                "func6 f6;\n"
                                "func7 f7;";

            // The expected result..
            // C const -> const C
            const std::string expected("const C * f1 ( ) ; "
                                       "const C * * f2 ; " // this gets simplified to a regular pointer
                                       "const C * ( & f3 ) ( ) ; "
                                       "const C * ( C :: * f4 ) ( ) ; "
                                       "const C * ( C :: * f5 ) ( ) const ; "
                                       "const C * ( C :: * f6 ) ( ) ; " // volatile is removed
                                       "const C * ( C :: * f7 ) ( ) const ;"); // volatile is removed
            ASSERT_EQUALS(expected, tok(code));

            checkSimplifyTypedef(code);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyTypedefFunction4() {
        const char code[] = "typedef int ( * ( * type1 ) ( bool ) ) ( int , int ) ;\n"
                            "typedef int ( * ( type2 ) ( bool ) ) ( int , int ) ;\n"
                            "typedef int ( * type3 ( bool ) ) ( int , int ) ;\n"
                            "type1 t1;\n"
                            "type2 t2;\n"
                            "type3 t3;";

        // The expected result..
        const std::string expected("int ( * ( * t1 ) ( bool ) ) ( int , int ) ; "
                                   "int ( * t2 ( bool ) ) ( int , int ) ; "
                                   "int ( * t3 ( bool ) ) ( int , int ) ;");
        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction5() {
        const char code[] = "typedef int ( * type1 ) ( float ) ;\n"
                            "typedef int ( * const type2 ) ( float ) ;\n"
                            "typedef int ( * volatile type3 ) ( float ) ;\n"
                            "typedef int ( * const volatile type4 ) ( float ) ;\n"
                            "typedef int ( C :: * type5 ) ( float ) ;\n"
                            "typedef int ( C :: * const type6 ) ( float ) ;\n"
                            "typedef int ( C :: * volatile type7 ) ( float ) ;\n"
                            "typedef int ( C :: * const volatile type8 ) ( float ) ;\n"
                            "typedef int ( :: C :: * type9 ) ( float ) ;\n"
                            "typedef int ( :: C :: * const type10 ) ( float ) ;\n"
                            "typedef int ( :: C :: * volatile type11 ) ( float ) ;\n"
                            "typedef int ( :: C :: * const volatile type12 ) ( float ) ;\n"
                            "type1 t1;\n"
                            "type2 t2;\n"
                            "type3 t3;\n"
                            "type4 t4;\n"
                            "type5 t5;\n"
                            "type6 t6;\n"
                            "type7 t7;\n"
                            "type8 t8;\n"
                            "type9 t9;\n"
                            "type10 t10;\n"
                            "type11 t11;\n"
                            "type12 t12;";

        // The expected result..
        const std::string expected("int * t1 ; " // simplified to regular pointer
                                   "int ( * const t2 ) ( float ) ; "
                                   "int * t3 ; " // volatile removed, gets simplified to regular pointer
                                   "int ( * const t4 ) ( float ) ; " // volatile removed
                                   "int ( C :: * t5 ) ( float ) ; "
                                   "int ( C :: * const t6 ) ( float ) ; "
                                   "int ( C :: * t7 ) ( float ) ; " // volatile removed
                                   "int ( C :: * const t8 ) ( float ) ; " // volatile removed
                                   "int ( :: C :: * t9 ) ( float ) ; "
                                   "int ( :: C :: * const t10 ) ( float ) ; "
                                   "int ( :: C :: * t11 ) ( float ) ; " // volatile removed
                                   "int ( :: C :: * const t12 ) ( float ) ;"); // volatile removed
        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction6() {
        const char code[] = "typedef void (*testfp)();\n"
                            "struct Fred\n"
                            "{\n"
                            "    testfp get1() { return 0; }\n"
                            "    void ( * get2 ( ) ) ( ) { return 0 ; }\n"
                            "    testfp get3();\n"
                            "    void ( * get4 ( ) ) ( );\n"
                            "};\n"
                            "testfp Fred::get3() { return 0; }\n"
                            "void ( * Fred::get4 ( ) ) ( ) { return 0 ; }\n";

        // The expected result..
        const std::string expected("struct Fred "
                                   "{ "
                                   "void ( * get1 ( ) ) ( ) { return 0 ; } "
                                   "void ( * get2 ( ) ) ( ) { return 0 ; } "
                                   "void ( * get3 ( ) ) ( ) ; "
                                   "void ( * get4 ( ) ) ( ) ; "
                                   "} ; "
                                   "void ( * Fred :: get3 ( ) ) ( ) { return 0 ; } "
                                   "void ( * Fred :: get4 ( ) ) ( ) { return 0 ; }");

        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction7() {
        const char code[] = "typedef void ( __gnu_cxx :: _SGIAssignableConcept < _Tp > :: * _func_Tp_SGIAssignableConcept ) () ;"
                            "_func_Tp_SGIAssignableConcept X;\n";

        // The expected result..
        const std::string expected("void ( __gnu_cxx :: _SGIAssignableConcept < _Tp > :: * X ) ( ) ;");

        ASSERT_EQUALS(expected, tok(code, false));

        checkSimplifyTypedef(code);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyTypedefFunction8() {
        // #2376 - internal error
        const char code[] = "typedef int f_expand(const nrv_byte *);\n"
                            "void f(f_expand   *(*get_fexp(int))){}\n";
        checkSimplifyTypedef(code);
        TODO_ASSERT_EQUALS("", "[test.cpp:2]: (debug) Function::addArguments found argument 'int' with varid 0.\n", errout.str());  // make sure that there is no internal error
    }

    void simplifyTypedefShadow() { // shadow variable (#4445)
        const char code[] = "typedef struct { int x; } xyz;;\n"
                            "void f(){\n"
                            "    int abc, xyz;\n" // <- shadow variable
                            "}\n";
        ASSERT_EQUALS("struct xyz { int x ; } ; void f ( ) { int abc ; int xyz ; }",
                      tok(code,false));
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
    std::string checkSimplifyEnum(const char code[]) {
        errout.str("");
        // Tokenize..
        Settings settings;
        settings.addEnabled("style");
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
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
                                "Enum1 e1 = a;";
            const char expected[] = "int e1 ; e1 = 0 ;";
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
                                "Enum1 e1 = a;";
            const char expected[] = "unsigned char e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class Enum1 : unsigned int { a };\n"
                                "Enum1 e1 = a;";
            const char expected[] = "unsigned int e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }

        {
            const char code[] = "enum class Enum1 : unsigned long long int { a };\n"
                                "Enum1 e1 = a;";
            const char expected[] = "unsigned long long e1 ; e1 = 0 ;";
            ASSERT_EQUALS(expected, checkSimplifyEnum(code));
        }
    }

    void enum16() { // ticket #1988
        const char code[] = "enum D : auto * { FF = 0 };";
        checkSimplifyEnum(code);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
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
        checkSimplifyEnum(code);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
    }

    void enum26() { // ticket #2975 (segmentation fault)
        const char code[] = "enum E {} e enum\n";
        checkSimplifyEnum(code);
        ASSERT_EQUALS("", errout.str());
    }

    void enum27() { // ticket #3005 (segmentation fault)
        const char code[] = "enum : x\n";
        checkSimplifyEnum(code);
        ASSERT_EQUALS("[test.cpp:1]: (error) syntax error\n", errout.str());
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

        const char code[] = "class TestIf\n"
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
                            "};\n";
        checkSimplifyEnum(code);
        TODO_ASSERT_EQUALS("","[test.cpp:12] -> [test.cpp:7]: (style) Variable 'two' hides enumerator with same name\n", errout.str());
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

    void initstruct() {
        ASSERT_EQUALS("; struct A a ; a . buf = 3 ;", tok("; struct A a = { .buf = 3 };"));
        ASSERT_EQUALS("; struct A a ; a . buf = x ;", tok("; struct A a = { .buf = x };"));
        ASSERT_EQUALS("; struct A a ; a . buf = & key ;", tok("; struct A a = { .buf = &key };"));
        ASSERT_EQUALS("; struct ABC abc ; abc . a = 3 ; abc . b = x ; abc . c = & key ;", tok("; struct ABC abc = { .a = 3, .b = x, .c = &key };"));
        TODO_ASSERT_EQUALS("; struct A a ; a . buf = { 0 } ;",
                           "; struct A a ; a = { . buf = { 0 } } ;",
                           tok("; struct A a = { .buf = {0} };"));
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

    void redundant_semicolon() {
        ASSERT_EQUALS("void f ( ) { ; }", tok("void f() { ; }", false));
        ASSERT_EQUALS("void f ( ) { ; }", tok("void f() { do { ; } while (0); }", true));
    }

    void simplifyFunctionReturn() {
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
                                "void ( * get1 ( ) ) ( ) { return 0 ; } "
                                "void ( * get2 ( ) ) ( ) { return 0 ; } "
                                "void ( * get3 ( ) ) ( ) ; "
                                "void ( * get4 ( ) ) ( ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code, false));
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
        ASSERT_EQUALS("[test.cpp:11]: (portability) The extra qualification 'two::c::' is unnecessary and is considered an error by many compilers.\n", errout.str());
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

    void simplifyIfNotNull() {
        {
            // ticket # 2601 segmentation fault
            const char code[] = "|| #if #define <=";
            tok(code, false);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "void f(int x) {\n"
                                "    x = (x != 0);\n"
                                "}";
            ASSERT_EQUALS("void f ( int x ) { }", tok(code, false));
        }
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
        ASSERT_EQUALS("int evallex ( ) { int c ; int t ; do { c = macroid ( c ) ; if ( c == EOF_CHAR || c == '\n' ) { } t = type [ c ] ; } while ( t == LET && catenate ( ) ) ; }",
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

    void simplifyFlowControl() {
        const char code1[] = "void f() {\n"
                             "  return;\n"
                             "  y();\n"
                             "}";
        ASSERT_EQUALS("void f ( ) { return ; }", tok(code1,true));

        const char code2[] = "void f() {\n"
                             "  exit();\n"
                             "  y();\n"
                             "}";
        ASSERT_EQUALS("void f ( ) { exit ( ) ; }", tok(code2,true));

        const char code3[] = "void f() {\n"
                             "  x.abort();\n"
                             "  y();\n"
                             "}";
        ASSERT_EQUALS("void f ( ) { x . abort ( ) ; y ( ) ; }", tok(code3,true));
    }
};

REGISTER_TEST(TestSimplifyTokens)
