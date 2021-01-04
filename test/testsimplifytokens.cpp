/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "platform.h"
#include "settings.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <ostream>
#include <string>


class TestSimplifyTokens : public TestFixture {
public:
    TestSimplifyTokens() : TestFixture("TestSimplifyTokens") {
    }


private:
    Settings settings0;
    Settings settings1;
    Settings settings_std;
    Settings settings_windows;

    void run() OVERRIDE {
        LOAD_LIB_2(settings_std.library, "std.cfg");
        LOAD_LIB_2(settings_windows.library, "windows.cfg");
        settings0.addEnabled("portability");
        settings1.addEnabled("style");
        settings_windows.addEnabled("portability");

        // If there are unused templates, keep those
        settings0.checkUnusedTemplates = true;
        settings1.checkUnusedTemplates = true;
        settings_std.checkUnusedTemplates = true;
        settings_windows.checkUnusedTemplates = true;

        // Make sure the Tokenizer::simplifyTokenList works.
        // The order of the simplifications is important. So this test
        // case shall make sure the simplifications are done in the
        // correct order
        TEST_CASE(simplifyTokenList1);

        TEST_CASE(simplifyMathFunctions_sqrt);
        TEST_CASE(simplifyMathFunctions_cbrt);
        TEST_CASE(simplifyMathFunctions_exp);
        TEST_CASE(simplifyMathFunctions_exp2);
        TEST_CASE(simplifyMathFunctions_logb);
        TEST_CASE(simplifyMathFunctions_log1p);
        TEST_CASE(simplifyMathFunctions_ilogb);
        TEST_CASE(simplifyMathFunctions_log10);
        TEST_CASE(simplifyMathFunctions_log);
        TEST_CASE(simplifyMathFunctions_log2);
        TEST_CASE(simplifyMathFunctions_pow);
        TEST_CASE(simplifyMathFunctions_fmin);
        TEST_CASE(simplifyMathFunctions_fmax);
        TEST_CASE(simplifyMathFunctions_acosh);
        TEST_CASE(simplifyMathFunctions_acos);
        TEST_CASE(simplifyMathFunctions_cosh);
        TEST_CASE(simplifyMathFunctions_cos);
        TEST_CASE(simplifyMathFunctions_erfc);
        TEST_CASE(simplifyMathFunctions_erf);
        TEST_CASE(simplifyMathFunctions_sin);
        TEST_CASE(simplifyMathFunctions_sinh);
        TEST_CASE(simplifyMathFunctions_asin);
        TEST_CASE(simplifyMathFunctions_asinh);
        TEST_CASE(simplifyMathFunctions_tan);
        TEST_CASE(simplifyMathFunctions_tanh);
        TEST_CASE(simplifyMathFunctions_atan);
        TEST_CASE(simplifyMathFunctions_atanh);
        TEST_CASE(simplifyMathFunctions_expm1);
        TEST_CASE(simplifyMathExpressions); //ticket #1620

        // foo(p = new char[10]);  =>  p = new char[10]; foo(p);
        TEST_CASE(simplifyAssignmentInFunctionCall);

        // ";a+=b;" => ";a=a+b;"
        TEST_CASE(simplifyCompoundAssignment);

        TEST_CASE(cast);
        TEST_CASE(iftruefalse);

        TEST_CASE(combine_strings);
        TEST_CASE(combine_wstrings);
        TEST_CASE(combine_ustrings);
        TEST_CASE(combine_Ustrings);
        TEST_CASE(combine_u8strings);
        TEST_CASE(combine_mixedstrings);

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
        TEST_CASE(sizeof22);
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

        TEST_CASE(simplifyAtol);

        TEST_CASE(simplifyOperator1);
        TEST_CASE(simplifyOperator2);

        TEST_CASE(simplifyArrayAccessSyntax);
        TEST_CASE(simplify_numeric_condition);
        TEST_CASE(simplify_condition);

        TEST_CASE(pointeralias1);
        TEST_CASE(pointeralias2);
        TEST_CASE(pointeralias3);
        TEST_CASE(pointeralias4);

        // simplify "while (0)"
        TEST_CASE(while0);
        // ticket #3140
        TEST_CASE(while0for);

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

        // struct ABC { } abc; => struct ABC { }; ABC abc;
        TEST_CASE(simplifyStructDecl1);
        TEST_CASE(simplifyStructDecl2); // ticket #2579
        TEST_CASE(simplifyStructDecl3);
        TEST_CASE(simplifyStructDecl4);
        TEST_CASE(simplifyStructDecl6); // ticket #3732
        TEST_CASE(simplifyStructDecl7); // ticket #476 (static anonymous struct array)
        TEST_CASE(simplifyStructDecl8); // ticket #7698

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
        TEST_CASE(simplifyNestedNamespace);
        TEST_CASE(simplifyNamespaceAliases);
    }

    std::string tok(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Native) {
        errout.str("");

        settings0.platform(type);
        Tokenizer tokenizer(&settings0, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(nullptr, !simplify);
    }

    std::string tokWithWindows(const char code[], bool simplify = true, Settings::PlatformType type = Settings::Native) {
        errout.str("");

        settings_windows.platform(type);
        Tokenizer tokenizer(&settings_windows, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(nullptr, !simplify);
    }

    std::string tok(const char code[], const char filename[], bool simplify = true) {
        errout.str("");

        Tokenizer tokenizer(&settings0, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        if (simplify)
            tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(nullptr, false);
    }

    std::string tokWithNewlines(const char code[]) {
        errout.str("");

        Tokenizer tokenizer(&settings0, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(false, false, false, true, false);
    }

    std::string tokWithStdLib(const char code[]) {
        errout.str("");

        Tokenizer tokenizer(&settings_std, this);

        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        return tokenizer.tokens()->stringifyList(nullptr, false);
    }

    std::string tokenizeDebugListing(const char code[], bool simplify = false, const char filename[] = "test.cpp") {
        errout.str("");

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        if (simplify)
            tokenizer.simplifyTokenList2();

        // result..
        return tokenizer.tokens()->stringifyList(true);
    }

    void simplifyTokenList1() {
        // #1717 : The simplifyErrNoInWhile needs to be used before simplifyIfAndWhileAssign..
        ASSERT_EQUALS("{ x = f ( ) ; while ( x == -1 ) { x = f ( ) ; } }",
                      tok("{ while((x=f())==-1 && errno==EINTR){}}",true));
    }


    void simplifyMathFunctions_erfc() {
        // verify erfc(), erfcf(), erfcl() - simplifcation
        const char code_erfc[] ="void f(int x) {\n"
                                " std::cout << erfc(x);\n" // do not simplify
                                " std::cout << erfc(0L);\n" // simplify to 1
                                "}";
        const char expected_erfc[] = "void f ( int x ) {\n"
                                     "std :: cout << erfc ( x ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_erfc, tokWithNewlines(code_erfc));

        const char code_erfcf[] ="void f(float x) {\n"
                                 " std::cout << erfcf(x);\n" // do not simplify
                                 " std::cout << erfcf(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_erfcf[] = "void f ( float x ) {\n"
                                      "std :: cout << erfcf ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_erfcf, tokWithNewlines(code_erfcf));

        const char code_erfcl[] ="void f(long double x) {\n"
                                 " std::cout << erfcl(x);\n" // do not simplify
                                 " std::cout << erfcl(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_erfcl[] = "void f ( double x ) {\n"
                                      "std :: cout << erfcl ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_erfcl, tokWithNewlines(code_erfcl));
    }

    void simplifyMathFunctions_cos() {
        // verify cos(), cosf(), cosl() - simplifcation
        const char code_cos[] ="void f(int x) {\n"
                               " std::cout << cos(x);\n" // do not simplify
                               " std::cout << cos(0L);\n" // simplify to 1
                               "}";
        const char expected_cos[] = "void f ( int x ) {\n"
                                    "std :: cout << cos ( x ) ;\n"
                                    "std :: cout << 1 ;\n"
                                    "}";
        ASSERT_EQUALS(expected_cos, tokWithNewlines(code_cos));

        const char code_cosf[] ="void f(float x) {\n"
                                " std::cout << cosf(x);\n" // do not simplify
                                " std::cout << cosf(0.0f);\n" // simplify to 1
                                "}";
        const char expected_cosf[] = "void f ( float x ) {\n"
                                     "std :: cout << cosf ( x ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_cosf, tokWithNewlines(code_cosf));

        const char code_cosl[] ="void f(long double x) {\n"
                                " std::cout << cosl(x);\n" // do not simplify
                                " std::cout << cosl(0.0f);\n" // simplify to 1
                                "}";
        const char expected_cosl[] = "void f ( double x ) {\n"
                                     "std :: cout << cosl ( x ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_cosl, tokWithNewlines(code_cosl));
    }

    void simplifyMathFunctions_cosh() {
        // verify cosh(), coshf(), coshl() - simplifcation
        const char code_cosh[] ="void f(int x) {\n"
                                " std::cout << cosh(x);\n" // do not simplify
                                " std::cout << cosh(0L);\n" // simplify to 1
                                "}";
        const char expected_cosh[] = "void f ( int x ) {\n"
                                     "std :: cout << cosh ( x ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_cosh, tokWithNewlines(code_cosh));

        const char code_coshf[] ="void f(float x) {\n"
                                 " std::cout << coshf(x);\n" // do not simplify
                                 " std::cout << coshf(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_coshf[] = "void f ( float x ) {\n"
                                      "std :: cout << coshf ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_coshf, tokWithNewlines(code_coshf));

        const char code_coshl[] ="void f(long double x) {\n"
                                 " std::cout << coshl(x);\n" // do not simplify
                                 " std::cout << coshl(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_coshl[] = "void f ( double x ) {\n"
                                      "std :: cout << coshl ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_coshl, tokWithNewlines(code_coshl));
    }

    void simplifyMathFunctions_acos() {
        // verify acos(), acosf(), acosl() - simplifcation
        const char code_acos[] ="void f(int x) {\n"
                                " std::cout << acos(x);\n" // do not simplify
                                " std::cout << acos(1L);\n" // simplify to 0
                                "}";
        const char expected_acos[] = "void f ( int x ) {\n"
                                     "std :: cout << acos ( x ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_acos, tokWithNewlines(code_acos));

        const char code_acosf[] ="void f(float x) {\n"
                                 " std::cout << acosf(x);\n" // do not simplify
                                 " std::cout << acosf(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_acosf[] = "void f ( float x ) {\n"
                                      "std :: cout << acosf ( x ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_acosf, tokWithNewlines(code_acosf));

        const char code_acosl[] ="void f(long double x) {\n"
                                 " std::cout << acosl(x);\n" // do not simplify
                                 " std::cout << acosl(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_acosl[] = "void f ( double x ) {\n"
                                      "std :: cout << acosl ( x ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_acosl, tokWithNewlines(code_acosl));
    }

    void simplifyMathFunctions_acosh() {
        // verify acosh(), acoshf(), acoshl() - simplifcation
        const char code_acosh[] ="void f(int x) {\n"
                                 " std::cout << acosh(x);\n" // do not simplify
                                 " std::cout << acosh(1L);\n" // simplify to 0
                                 "}";
        const char expected_acosh[] = "void f ( int x ) {\n"
                                      "std :: cout << acosh ( x ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_acosh, tokWithNewlines(code_acosh));

        const char code_acoshf[] ="void f(float x) {\n"
                                  " std::cout << acoshf(x);\n" // do not simplify
                                  " std::cout << acoshf(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_acoshf[] = "void f ( float x ) {\n"
                                       "std :: cout << acoshf ( x ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_acoshf, tokWithNewlines(code_acoshf));

        const char code_acoshl[] ="void f(long double x) {\n"
                                  " std::cout << acoshl(x);\n" // do not simplify
                                  " std::cout << acoshl(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_acoshl[] = "void f ( double x ) {\n"
                                       "std :: cout << acoshl ( x ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_acoshl, tokWithNewlines(code_acoshl));
    }

    void simplifyMathFunctions_sqrt() {
        // verify sqrt(), sqrtf(), sqrtl() - simplifcation
        const char code_sqrt[] ="void f(int x) {\n"
                                " std::cout << sqrt(x);\n" // do not simplify
                                " std::cout << sqrt(-1);\n" // do not simplify
                                " std::cout << sqrt(0L);\n" // simplify to 0
                                " std::cout << sqrt(1L);\n" // simplify to 1
                                "}";
        const char expected_sqrt[] = "void f ( int x ) {\n"
                                     "std :: cout << sqrt ( x ) ;\n"
                                     "std :: cout << sqrt ( -1 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_sqrt, tokWithNewlines(code_sqrt));

        const char code_sqrtf[] ="void f(float x) {\n"
                                 " std::cout << sqrtf(x);\n" // do not simplify
                                 " std::cout << sqrtf(-1.0f);\n" // do not simplify
                                 " std::cout << sqrtf(0.0f);\n" // simplify to 0
                                 " std::cout << sqrtf(1.0);\n" // simplify to 1
                                 "}";
        const char expected_sqrtf[] = "void f ( float x ) {\n"
                                      "std :: cout << sqrtf ( x ) ;\n"
                                      "std :: cout << sqrtf ( -1.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_sqrtf, tokWithNewlines(code_sqrtf));

        const char code_sqrtl[] ="void f(long double x) {\n"
                                 " std::cout << sqrtf(x);\n" // do not simplify
                                 " std::cout << sqrtf(-1.0);\n" // do not simplify
                                 " std::cout << sqrtf(0.0);\n" // simplify to 0
                                 " std::cout << sqrtf(1.0);\n" // simplify to 1
                                 "}";
        const char expected_sqrtl[] = "void f ( double x ) {\n"
                                      "std :: cout << sqrtf ( x ) ;\n"
                                      "std :: cout << sqrtf ( -1.0 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_sqrtl, tokWithNewlines(code_sqrtl));
    }

    void simplifyMathFunctions_cbrt() {
        // verify cbrt(), cbrtf(), cbrtl() - simplifcation
        const char code_cbrt[] ="void f(int x) {\n"
                                " std::cout << cbrt(x);\n" // do not simplify
                                " std::cout << cbrt(-1);\n" // do not simplify
                                " std::cout << cbrt(0L);\n" // simplify to 0
                                " std::cout << cbrt(1L);\n" // simplify to 1
                                "}";
        const char expected_cbrt[] = "void f ( int x ) {\n"
                                     "std :: cout << cbrt ( x ) ;\n"
                                     "std :: cout << cbrt ( -1 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_cbrt, tokWithNewlines(code_cbrt));

        const char code_cbrtf[] ="void f(float x) {\n"
                                 " std::cout << cbrtf(x);\n" // do not simplify
                                 " std::cout << cbrtf(-1.0f);\n" // do not simplify
                                 " std::cout << cbrtf(0.0f);\n" // simplify to 0
                                 " std::cout << cbrtf(1.0);\n" // simplify to 1
                                 "}";
        const char expected_cbrtf[] = "void f ( float x ) {\n"
                                      "std :: cout << cbrtf ( x ) ;\n"
                                      "std :: cout << cbrtf ( -1.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_cbrtf, tokWithNewlines(code_cbrtf));

        const char code_cbrtl[] ="void f(long double x) {\n"
                                 " std::cout << cbrtl(x);\n" // do not simplify
                                 " std::cout << cbrtl(-1.0);\n" // do not simplify
                                 " std::cout << cbrtl(0.0);\n" // simplify to 0
                                 " std::cout << cbrtl(1.0);\n" // simplify to 1
                                 "}";
        const char expected_cbrtl[] = "void f ( double x ) {\n"
                                      "std :: cout << cbrtl ( x ) ;\n"
                                      "std :: cout << cbrtl ( -1.0 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_cbrtl, tokWithNewlines(code_cbrtl));
    }

    void simplifyMathFunctions_exp2() {
        // verify exp2(), exp2f(), exp2l() - simplifcation
        const char code_exp2[] ="void f(int x) {\n"
                                " std::cout << exp2(x);\n" // do not simplify
                                " std::cout << exp2(-1);\n" // do not simplify
                                " std::cout << exp2(0L);\n" // simplify to 0
                                " std::cout << exp2(1L);\n" // do not simplify
                                "}";
        const char expected_exp2[] = "void f ( int x ) {\n"
                                     "std :: cout << exp2 ( x ) ;\n"
                                     "std :: cout << exp2 ( -1 ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "std :: cout << exp2 ( 1L ) ;\n"
                                     "}";
        ASSERT_EQUALS(expected_exp2, tokWithNewlines(code_exp2));

        const char code_exp2f[] ="void f(float x) {\n"
                                 " std::cout << exp2f(x);\n" // do not simplify
                                 " std::cout << exp2f(-1.0);\n" // do not simplify
                                 " std::cout << exp2f(0.0);\n" // simplify to 1
                                 " std::cout << exp2f(1.0);\n" // do not simplify
                                 "}";
        const char expected_exp2f[] = "void f ( float x ) {\n"
                                      "std :: cout << exp2f ( x ) ;\n"
                                      "std :: cout << exp2f ( -1.0 ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "std :: cout << exp2f ( 1.0 ) ;\n"
                                      "}";
        ASSERT_EQUALS(expected_exp2f, tokWithNewlines(code_exp2f));

        const char code_exp2l[] ="void f(long double x) {\n"
                                 " std::cout << exp2l(x);\n" // do not simplify
                                 " std::cout << exp2l(-1.0);\n" // do not simplify
                                 " std::cout << exp2l(0.0);\n" // simplify to 1
                                 " std::cout << exp2l(1.0);\n" // do not simplify
                                 "}";
        const char expected_exp2l[] = "void f ( double x ) {\n"
                                      "std :: cout << exp2l ( x ) ;\n"
                                      "std :: cout << exp2l ( -1.0 ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "std :: cout << exp2l ( 1.0 ) ;\n"
                                      "}";
        ASSERT_EQUALS(expected_exp2l, tokWithNewlines(code_exp2l));
    }

    void simplifyMathFunctions_exp() {
        // verify exp(), expf(), expl() - simplifcation
        const char code_exp[] ="void f(int x) {\n"
                               " std::cout << exp(x);\n" // do not simplify
                               " std::cout << exp(-1);\n" // do not simplify
                               " std::cout << exp(0L);\n" // simplify to 1
                               " std::cout << exp(1L);\n" // do not simplify
                               "}";
        const char expected_exp[] = "void f ( int x ) {\n"
                                    "std :: cout << exp ( x ) ;\n"
                                    "std :: cout << exp ( -1 ) ;\n"
                                    "std :: cout << 1 ;\n"
                                    "std :: cout << exp ( 1L ) ;\n"
                                    "}";
        ASSERT_EQUALS(expected_exp, tokWithNewlines(code_exp));

        const char code_expf[] ="void f(float x) {\n"
                                " std::cout << expf(x);\n" // do not simplify
                                " std::cout << expf(-1.0);\n" // do not simplify
                                " std::cout << expf(0.0);\n" // simplify to 1
                                " std::cout << expf(1.0);\n" // do not simplify
                                "}";
        const char expected_expf[] = "void f ( float x ) {\n"
                                     "std :: cout << expf ( x ) ;\n"
                                     "std :: cout << expf ( -1.0 ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "std :: cout << expf ( 1.0 ) ;\n"
                                     "}";
        ASSERT_EQUALS(expected_expf, tokWithNewlines(code_expf));

        const char code_expl[] ="void f(long double x) {\n"
                                " std::cout << expl(x);\n" // do not simplify
                                " std::cout << expl(-1.0);\n" // do not simplify
                                " std::cout << expl(0.0);\n" // simplify to 1
                                " std::cout << expl(1.0);\n" // do not simplify
                                "}";
        const char expected_expl[] = "void f ( double x ) {\n"
                                     "std :: cout << expl ( x ) ;\n"
                                     "std :: cout << expl ( -1.0 ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "std :: cout << expl ( 1.0 ) ;\n"
                                     "}";
        ASSERT_EQUALS(expected_expl, tokWithNewlines(code_expl));
    }

    void simplifyMathFunctions_erf() {
        // verify erf(), erff(), erfl() - simplifcation
        const char code_erf[] ="void f(int x) {\n"
                               " std::cout << erf(x);\n" // do not simplify
                               " std::cout << erf(10);\n" // do not simplify
                               " std::cout << erf(0L);\n" // simplify to 0
                               "}";
        const char expected_erf[] = "void f ( int x ) {\n"
                                    "std :: cout << erf ( x ) ;\n"
                                    "std :: cout << erf ( 10 ) ;\n"
                                    "std :: cout << 0 ;\n"
                                    "}";
        ASSERT_EQUALS(expected_erf, tokWithNewlines(code_erf));

        const char code_erff[] ="void f(float x) {\n"
                                " std::cout << erff(x);\n" // do not simplify
                                " std::cout << erff(10);\n" // do not simplify
                                " std::cout << erff(0.0f);\n" // simplify to 0
                                "}";
        const char expected_erff[] = "void f ( float x ) {\n"
                                     "std :: cout << erff ( x ) ;\n"
                                     "std :: cout << erff ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_erff, tokWithNewlines(code_erff));

        const char code_erfl[] ="void f(long double x) {\n"
                                " std::cout << erfl(x);\n" // do not simplify
                                " std::cout << erfl(10.0f);\n" // do not simplify
                                " std::cout << erfl(0.0f);\n" // simplify to 0
                                "}";
        const char expected_erfl[] = "void f ( double x ) {\n"
                                     "std :: cout << erfl ( x ) ;\n"
                                     "std :: cout << erfl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_erfl, tokWithNewlines(code_erfl));
    }

    void simplifyMathFunctions_atanh() {
        // verify atanh(), atanhf(), atanhl() - simplifcation
        const char code_atanh[] ="void f(int x) {\n"
                                 " std::cout << atanh(x);\n" // do not simplify
                                 " std::cout << atanh(10);\n" // do not simplify
                                 " std::cout << atanh(0L);\n" // simplify to 0
                                 "}";
        const char expected_atanh[] = "void f ( int x ) {\n"
                                      "std :: cout << atanh ( x ) ;\n"
                                      "std :: cout << atanh ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_atanh, tokWithNewlines(code_atanh));

        const char code_atanhf[] ="void f(float x) {\n"
                                  " std::cout << atanhf(x);\n" // do not simplify
                                  " std::cout << atanhf(10);\n" // do not simplify
                                  " std::cout << atanhf(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_atanhf[] = "void f ( float x ) {\n"
                                       "std :: cout << atanhf ( x ) ;\n"
                                       "std :: cout << atanhf ( 10 ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_atanhf, tokWithNewlines(code_atanhf));

        const char code_atanhl[] ="void f(long double x) {\n"
                                  " std::cout << atanhl(x);\n" // do not simplify
                                  " std::cout << atanhl(10.0f);\n" // do not simplify
                                  " std::cout << atanhl(0.0d);\n" // do not simplify - invalid number!
                                  " std::cout << atanhl(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_atanhl[] = "void f ( double x ) {\n"
                                       "std :: cout << atanhl ( x ) ;\n"
                                       "std :: cout << atanhl ( 10.0f ) ;\n"
                                       "std :: cout << atanhl ( 0.0d ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_atanhl, tokWithNewlines(code_atanhl));
    }

    void simplifyMathFunctions_atan() {
        // verify atan(), atanf(), atanl() - simplifcation
        const char code_atan[] ="void f(int x) {\n"
                                " std::cout << atan(x);\n" // do not simplify
                                " std::cout << atan(10);\n" // do not simplify
                                " std::cout << atan(0L);\n" // simplify to 0
                                "}";
        const char expected_atan[] = "void f ( int x ) {\n"
                                     "std :: cout << atan ( x ) ;\n"
                                     "std :: cout << atan ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_atan, tokWithNewlines(code_atan));

        const char code_atanf[] ="void f(float x) {\n"
                                 " std::cout << atanf(x);\n" // do not simplify
                                 " std::cout << atanf(10);\n" // do not simplify
                                 " std::cout << atanf(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_atanf[] = "void f ( float x ) {\n"
                                      "std :: cout << atanf ( x ) ;\n"
                                      "std :: cout << atanf ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_atanf, tokWithNewlines(code_atanf));

        const char code_atanl[] ="void f(long double x) {\n"
                                 " std::cout << atanl(x);\n" // do not simplify
                                 " std::cout << atanl(10.0f);\n" // do not simplify
                                 " std::cout << atanl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_atanl[] = "void f ( double x ) {\n"
                                      "std :: cout << atanl ( x ) ;\n"
                                      "std :: cout << atanl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_atanl, tokWithNewlines(code_atanl));
    }

    void simplifyMathFunctions_tanh() {
        // verify tanh(), tanhf(), tanhl() - simplifcation
        const char code_tanh[] ="void f(int x) {\n"
                                " std::cout << tanh(x);\n" // do not simplify
                                " std::cout << tanh(10);\n" // do not simplify
                                " std::cout << tanh(0L);\n" // simplify to 0
                                "}";
        const char expected_tanh[] = "void f ( int x ) {\n"
                                     "std :: cout << tanh ( x ) ;\n"
                                     "std :: cout << tanh ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_tanh, tokWithNewlines(code_tanh));

        const char code_tanhf[] ="void f(float x) {\n"
                                 " std::cout << tanhf(x);\n" // do not simplify
                                 " std::cout << tanhf(10);\n" // do not simplify
                                 " std::cout << tanhf(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_tanhf[] = "void f ( float x ) {\n"
                                      "std :: cout << tanhf ( x ) ;\n"
                                      "std :: cout << tanhf ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_tanhf, tokWithNewlines(code_tanhf));

        const char code_tanhl[] ="void f(long double x) {\n"
                                 " std::cout << tanhl(x);\n" // do not simplify
                                 " std::cout << tanhl(10.0f);\n" // do not simplify
                                 " std::cout << tanhl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_tanhl[] = "void f ( double x ) {\n"
                                      "std :: cout << tanhl ( x ) ;\n"
                                      "std :: cout << tanhl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_tanhl, tokWithNewlines(code_tanhl));
    }

    void simplifyMathFunctions_tan() {
        // verify tan(), tanf(), tanl() - simplifcation
        const char code_tan[] ="void f(int x) {\n"
                               " std::cout << tan(x);\n" // do not simplify
                               " std::cout << tan(10);\n" // do not simplify
                               " std::cout << tan(0L);\n" // simplify to 0
                               "}";
        const char expected_tan[] = "void f ( int x ) {\n"
                                    "std :: cout << tan ( x ) ;\n"
                                    "std :: cout << tan ( 10 ) ;\n"
                                    "std :: cout << 0 ;\n"
                                    "}";
        ASSERT_EQUALS(expected_tan, tokWithNewlines(code_tan));

        const char code_tanf[] ="void f(float x) {\n"
                                " std::cout << tanf(x);\n" // do not simplify
                                " std::cout << tanf(10);\n" // do not simplify
                                " std::cout << tanf(0.0f);\n" // simplify to 0
                                "}";
        const char expected_tanf[] = "void f ( float x ) {\n"
                                     "std :: cout << tanf ( x ) ;\n"
                                     "std :: cout << tanf ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_tanf, tokWithNewlines(code_tanf));

        const char code_tanl[] ="void f(long double x) {\n"
                                " std::cout << tanl(x);\n" // do not simplify
                                " std::cout << tanl(10.0f);\n" // do not simplify
                                " std::cout << tanl(0.0f);\n" // simplify to 0
                                "}";
        const char expected_tanl[] = "void f ( double x ) {\n"
                                     "std :: cout << tanl ( x ) ;\n"
                                     "std :: cout << tanl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_tanl, tokWithNewlines(code_tanl));
    }

    void simplifyMathFunctions_expm1() {
        // verify expm1(), expm1f(), expm1l() - simplifcation
        const char code_expm1[] ="void f(int x) {\n"
                                 " std::cout << expm1(x);\n" // do not simplify
                                 " std::cout << expm1(10);\n" // do not simplify
                                 " std::cout << expm1(0L);\n" // simplify to 0
                                 "}";
        const char expected_expm1[] = "void f ( int x ) {\n"
                                      "std :: cout << expm1 ( x ) ;\n"
                                      "std :: cout << expm1 ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_expm1, tokWithNewlines(code_expm1));

        const char code_expm1f[] ="void f(float x) {\n"
                                  " std::cout << expm1f(x);\n" // do not simplify
                                  " std::cout << expm1f(10);\n" // do not simplify
                                  " std::cout << expm1f(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_expm1f[] = "void f ( float x ) {\n"
                                       "std :: cout << expm1f ( x ) ;\n"
                                       "std :: cout << expm1f ( 10 ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_expm1f, tokWithNewlines(code_expm1f));

        const char code_expm1l[] ="void f(long double x) {\n"
                                  " std::cout << expm1l(x);\n" // do not simplify
                                  " std::cout << expm1l(10.0f);\n" // do not simplify
                                  " std::cout << expm1l(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_expm1l[] = "void f ( double x ) {\n"
                                       "std :: cout << expm1l ( x ) ;\n"
                                       "std :: cout << expm1l ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_expm1l, tokWithNewlines(code_expm1l));
    }

    void simplifyMathFunctions_asinh() {
        // verify asinh(), asinhf(), asinhl() - simplifcation
        const char code_asinh[] ="void f(int x) {\n"
                                 " std::cout << asinh(x);\n" // do not simplify
                                 " std::cout << asinh(10);\n" // do not simplify
                                 " std::cout << asinh(0L);\n" // simplify to 0
                                 "}";
        const char expected_asinh[] = "void f ( int x ) {\n"
                                      "std :: cout << asinh ( x ) ;\n"
                                      "std :: cout << asinh ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_asinh, tokWithNewlines(code_asinh));

        const char code_asinhf[] ="void f(float x) {\n"
                                  " std::cout << asinhf(x);\n" // do not simplify
                                  " std::cout << asinhf(10);\n" // do not simplify
                                  " std::cout << asinhf(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_asinhf[] = "void f ( float x ) {\n"
                                       "std :: cout << asinhf ( x ) ;\n"
                                       "std :: cout << asinhf ( 10 ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_asinhf, tokWithNewlines(code_asinhf));

        const char code_asinhl[] ="void f(long double x) {\n"
                                  " std::cout << asinhl(x);\n" // do not simplify
                                  " std::cout << asinhl(10.0f);\n" // do not simplify
                                  " std::cout << asinhl(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_asinhl[] = "void f ( double x ) {\n"
                                       "std :: cout << asinhl ( x ) ;\n"
                                       "std :: cout << asinhl ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_asinhl, tokWithNewlines(code_asinhl));
    }

    void simplifyMathFunctions_asin() {
        // verify asin(), asinf(), asinl() - simplifcation
        const char code_asin[] ="void f(int x) {\n"
                                " std::cout << asin(x);\n" // do not simplify
                                " std::cout << asin(10);\n" // do not simplify
                                " std::cout << asin(0L);\n" // simplify to 0
                                "}";
        const char expected_asin[] = "void f ( int x ) {\n"
                                     "std :: cout << asin ( x ) ;\n"
                                     "std :: cout << asin ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_asin, tokWithNewlines(code_asin));

        const char code_asinf[] ="void f(float x) {\n"
                                 " std::cout << asinf(x);\n" // do not simplify
                                 " std::cout << asinf(10);\n" // do not simplify
                                 " std::cout << asinf(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_asinf[] = "void f ( float x ) {\n"
                                      "std :: cout << asinf ( x ) ;\n"
                                      "std :: cout << asinf ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_asinf, tokWithNewlines(code_asinf));

        const char code_asinl[] ="void f(long double x) {\n"
                                 " std::cout << asinl(x);\n" // do not simplify
                                 " std::cout << asinl(10.0f);\n" // do not simplify
                                 " std::cout << asinl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_asinl[] = "void f ( double x ) {\n"
                                      "std :: cout << asinl ( x ) ;\n"
                                      "std :: cout << asinl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_asinl, tokWithNewlines(code_asinl));
    }

    void simplifyMathFunctions_sinh() {
        // verify sinh(), sinhf(), sinhl() - simplifcation
        const char code_sinh[] ="void f(int x) {\n"
                                " std::cout << sinh(x);\n" // do not simplify
                                " std::cout << sinh(10);\n" // do not simplify
                                " std::cout << sinh(0L);\n" // simplify to 0
                                "}";
        const char expected_sinh[] = "void f ( int x ) {\n"
                                     "std :: cout << sinh ( x ) ;\n"
                                     "std :: cout << sinh ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_sinh, tokWithNewlines(code_sinh));

        const char code_sinhf[] ="void f(float x) {\n"
                                 " std::cout << sinhf(x);\n" // do not simplify
                                 " std::cout << sinhf(10);\n" // do not simplify
                                 " std::cout << sinhf(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_sinhf[] = "void f ( float x ) {\n"
                                      "std :: cout << sinhf ( x ) ;\n"
                                      "std :: cout << sinhf ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_sinhf, tokWithNewlines(code_sinhf));

        const char code_sinhl[] ="void f(long double x) {\n"
                                 " std::cout << sinhl(x);\n" // do not simplify
                                 " std::cout << sinhl(10.0f);\n" // do not simplify
                                 " std::cout << sinhl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_sinhl[] = "void f ( double x ) {\n"
                                      "std :: cout << sinhl ( x ) ;\n"
                                      "std :: cout << sinhl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_sinhl, tokWithNewlines(code_sinhl));
    }

    void simplifyMathFunctions_sin() {
        // verify sin(), sinf(), sinl() - simplifcation
        const char code_sin[] ="void f(int x) {\n"
                               " std::cout << sin(x);\n" // do not simplify
                               " std::cout << sin(10);\n" // do not simplify
                               " std::cout << sin(0L);\n" // simplify to 0
                               "}";
        const char expected_sin[] = "void f ( int x ) {\n"
                                    "std :: cout << sin ( x ) ;\n"
                                    "std :: cout << sin ( 10 ) ;\n"
                                    "std :: cout << 0 ;\n"
                                    "}";
        ASSERT_EQUALS(expected_sin, tokWithNewlines(code_sin));

        const char code_sinf[] ="void f(float x) {\n"
                                " std::cout << sinf(x);\n" // do not simplify
                                " std::cout << sinf(10);\n" // do not simplify
                                " std::cout << sinf(0.0f);\n" // simplify to 0
                                "}";
        const char expected_sinf[] = "void f ( float x ) {\n"
                                     "std :: cout << sinf ( x ) ;\n"
                                     "std :: cout << sinf ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_sinf, tokWithNewlines(code_sinf));

        const char code_sinl[] ="void f(long double x) {\n"
                                " std::cout << sinl(x);\n" // do not simplify
                                " std::cout << sinl(10.0f);\n" // do not simplify
                                " std::cout << sinl(0.0f);\n" // simplify to 0
                                "}";
        const char expected_sinl[] = "void f ( double x ) {\n"
                                     "std :: cout << sinl ( x ) ;\n"
                                     "std :: cout << sinl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_sinl, tokWithNewlines(code_sinl));

        // #6629
        const char code[] = "class Foo { int sinf; Foo() : sinf(0) {} };";
        const char expected[] = "class Foo { int sinf ; Foo ( ) : sinf ( 0 ) { } } ;";
        ASSERT_EQUALS(expected, tokWithNewlines(code));
    }

    void simplifyMathFunctions_ilogb() {
        // verify ilogb(), ilogbf(), ilogbl() - simplifcation
        const char code_ilogb[] ="void f(int x) {\n"
                                 " std::cout << ilogb(x);\n" // do not simplify
                                 " std::cout << ilogb(10);\n" // do not simplify
                                 " std::cout << ilogb(1L);\n" // simplify to 0
                                 "}";
        const char expected_ilogb[] = "void f ( int x ) {\n"
                                      "std :: cout << ilogb ( x ) ;\n"
                                      "std :: cout << ilogb ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_ilogb, tokWithNewlines(code_ilogb));

        const char code_ilogbf[] ="void f(float x) {\n"
                                  " std::cout << ilogbf(x);\n" // do not simplify
                                  " std::cout << ilogbf(10);\n" // do not simplify
                                  " std::cout << ilogbf(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_ilogbf[] = "void f ( float x ) {\n"
                                       "std :: cout << ilogbf ( x ) ;\n"
                                       "std :: cout << ilogbf ( 10 ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_ilogbf, tokWithNewlines(code_ilogbf));

        const char code_ilogbl[] ="void f(long double x) {\n"
                                  " std::cout << ilogbl(x);\n" // do not simplify
                                  " std::cout << ilogbl(10.0f);\n" // do not simplify
                                  " std::cout << ilogbl(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_ilogbl[] = "void f ( double x ) {\n"
                                       "std :: cout << ilogbl ( x ) ;\n"
                                       "std :: cout << ilogbl ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_ilogbl, tokWithNewlines(code_ilogbl));
    }

    void simplifyMathFunctions_logb() {
        // verify logb(), logbf(), logbl() - simplifcation
        const char code_logb[] ="void f(int x) {\n"
                                " std::cout << logb(x);\n" // do not simplify
                                " std::cout << logb(10);\n" // do not simplify
                                " std::cout << logb(1L);\n" // simplify to 0
                                "}";
        const char expected_logb[] = "void f ( int x ) {\n"
                                     "std :: cout << logb ( x ) ;\n"
                                     "std :: cout << logb ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_logb, tokWithNewlines(code_logb));

        const char code_logbf[] ="void f(float x) {\n"
                                 " std::cout << logbf(x);\n" // do not simplify
                                 " std::cout << logbf(10);\n" // do not simplify
                                 " std::cout << logbf(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_logbf[] = "void f ( float x ) {\n"
                                      "std :: cout << logbf ( x ) ;\n"
                                      "std :: cout << logbf ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_logbf, tokWithNewlines(code_logbf));

        const char code_logbl[] ="void f(long double x) {\n"
                                 " std::cout << logbl(x);\n" // do not simplify
                                 " std::cout << logbl(10.0f);\n" // do not simplify
                                 " std::cout << logbl(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_logbl[] = "void f ( double x ) {\n"
                                      "std :: cout << logbl ( x ) ;\n"
                                      "std :: cout << logbl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_logbl, tokWithNewlines(code_logbl));
    }

    void simplifyMathFunctions_log1p() {
        // verify log1p(), log1pf(), log1pl() - simplifcation
        const char code_log1p[] ="void f(int x) {\n"
                                 " std::cout << log1p(x);\n" // do not simplify
                                 " std::cout << log1p(10);\n" // do not simplify
                                 " std::cout << log1p(0L);\n" // simplify to 0
                                 "}";
        const char expected_log1p[] = "void f ( int x ) {\n"
                                      "std :: cout << log1p ( x ) ;\n"
                                      "std :: cout << log1p ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_log1p, tokWithNewlines(code_log1p));

        const char code_log1pf[] ="void f(float x) {\n"
                                  " std::cout << log1pf(x);\n" // do not simplify
                                  " std::cout << log1pf(10);\n" // do not simplify
                                  " std::cout << log1pf(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_log1pf[] = "void f ( float x ) {\n"
                                       "std :: cout << log1pf ( x ) ;\n"
                                       "std :: cout << log1pf ( 10 ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_log1pf, tokWithNewlines(code_log1pf));

        const char code_log1pl[] ="void f(long double x) {\n"
                                  " std::cout << log1pl(x);\n" // do not simplify
                                  " std::cout << log1pl(10.0f);\n" // do not simplify
                                  " std::cout << log1pl(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_log1pl[] = "void f ( double x ) {\n"
                                       "std :: cout << log1pl ( x ) ;\n"
                                       "std :: cout << log1pl ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_log1pl, tokWithNewlines(code_log1pl));
    }

    void simplifyMathFunctions_log10() {
        // verify log10(), log10f(), log10l() - simplifcation
        const char code_log10[] ="void f(int x) {\n"
                                 " std::cout << log10(x);\n" // do not simplify
                                 " std::cout << log10(10);\n" // do not simplify
                                 " std::cout << log10(1L);\n" // simplify to 0
                                 "}";
        const char expected_log10[] = "void f ( int x ) {\n"
                                      "std :: cout << log10 ( x ) ;\n"
                                      "std :: cout << log10 ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_log10, tokWithNewlines(code_log10));

        const char code_log10f[] ="void f(float x) {\n"
                                  " std::cout << log10f(x);\n" // do not simplify
                                  " std::cout << log10f(10);\n" // do not simplify
                                  " std::cout << log10f(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_log10f[] = "void f ( float x ) {\n"
                                       "std :: cout << log10f ( x ) ;\n"
                                       "std :: cout << log10f ( 10 ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_log10f, tokWithNewlines(code_log10f));

        const char code_log10l[] ="void f(long double x) {\n"
                                  " std::cout << log10l(x);\n" // do not simplify
                                  " std::cout << log10l(10.0f);\n" // do not simplify
                                  " std::cout << log10l(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_log10l[] = "void f ( double x ) {\n"
                                       "std :: cout << log10l ( x ) ;\n"
                                       "std :: cout << log10l ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_log10l, tokWithNewlines(code_log10l));

    }
    void simplifyMathFunctions_log() {
        // verify log(), logf(), logl() - simplifcation
        const char code_log[] ="void f(int x) {\n"
                               " std::cout << log(x);\n" // do not simplify
                               " std::cout << log(10);\n" // do not simplify
                               " std::cout << log(1L);\n" // simplify to 0
                               "}";
        const char expected_log[] = "void f ( int x ) {\n"
                                    "std :: cout << log ( x ) ;\n"
                                    "std :: cout << log ( 10 ) ;\n"
                                    "std :: cout << 0 ;\n"
                                    "}";
        ASSERT_EQUALS(expected_log, tokWithNewlines(code_log));

        const char code_logf[] ="void f(float x) {\n"
                                " std::cout << logf(x);\n" // do not simplify
                                " std::cout << logf(10);\n" // do not simplify
                                " std::cout << logf(1.0f);\n" // simplify to 0
                                "}";
        const char expected_logf[] = "void f ( float x ) {\n"
                                     "std :: cout << logf ( x ) ;\n"
                                     "std :: cout << logf ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_logf, tokWithNewlines(code_logf));

        const char code_logl[] ="void f(long double x) {\n"
                                " std::cout << logl(x);\n" // do not simplify
                                " std::cout << logl(10.0f);\n" // do not simplify
                                " std::cout << logl(1.0f);\n" // simplify to 0
                                "}";
        const char expected_logl[] = "void f ( double x ) {\n"
                                     "std :: cout << logl ( x ) ;\n"
                                     "std :: cout << logl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_logl, tokWithNewlines(code_logl));
    }

    void simplifyMathFunctions_log2() {
        // verify log2(), log2f(), log2l() - simplifcation
        const char code_log2[] ="void f(int x) {\n"
                                " std::cout << log2(x);\n" // do not simplify
                                " std::cout << log2(10);\n" // do not simplify
                                " std::cout << log2(1L);\n" // simplify to 0
                                "}";
        const char expected_log2[] = "void f ( int x ) {\n"
                                     "std :: cout << log2 ( x ) ;\n"
                                     "std :: cout << log2 ( 10 ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_log2, tokWithNewlines(code_log2));

        const char code_log2f[] ="void f(float x) {\n"
                                 " std::cout << log2f(x);\n" // do not simplify
                                 " std::cout << log2f(10);\n" // do not simplify
                                 " std::cout << log2f(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_log2f[] = "void f ( float x ) {\n"
                                      "std :: cout << log2f ( x ) ;\n"
                                      "std :: cout << log2f ( 10 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_log2f, tokWithNewlines(code_log2f));

        const char code_log2l[] ="void f(long double x) {\n"
                                 " std::cout << log2l(x);\n" // do not simplify
                                 " std::cout << log2l(10.0f);\n" // do not simplify
                                 " std::cout << log2l(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_log2l[] = "void f ( double x ) {\n"
                                      "std :: cout << log2l ( x ) ;\n"
                                      "std :: cout << log2l ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_log2l, tokWithNewlines(code_log2l));
    }

    void simplifyMathFunctions_pow() {
        // verify pow(),pow(),powl() - simplifcation
        const char code_pow[] ="void f() {\n"
                               " std::cout << pow(-1.0,1);\n"
                               " std::cout << pow(1.0,1);\n"
                               " std::cout << pow(0,1);\n"
                               " std::cout << pow(1,-6);\n"
                               " std::cout << powf(-1.0,1.0f);\n"
                               " std::cout << powf(1.0,1.0f);\n"
                               " std::cout << powf(0,1.0f);\n"
                               " std::cout << powf(1.0,-6.0f);\n"
                               " std::cout << powl(-1.0,1.0);\n"
                               " std::cout << powl(1.0,1.0);\n"
                               " std::cout << powl(0,1.0);\n"
                               " std::cout << powl(1.0,-6.0d);\n"
                               "}";

        const char expected_pow[] = "void f ( ) {\n"
                                    "std :: cout << -1.0 ;\n"
                                    "std :: cout << 1 ;\n"
                                    "std :: cout << 0 ;\n"
                                    "std :: cout << 1 ;\n"
                                    "std :: cout << -1.0 ;\n"
                                    "std :: cout << 1 ;\n"
                                    "std :: cout << 0 ;\n"
                                    "std :: cout << 1 ;\n"
                                    "std :: cout << -1.0 ;\n"
                                    "std :: cout << 1 ;\n"
                                    "std :: cout << 0 ;\n"
                                    "std :: cout << 1 ;\n"
                                    "}";
        ASSERT_EQUALS(expected_pow, tokWithNewlines(code_pow));

        // verify if code is simplified correctly.
        // Do not simplify class members.
        const char code_pow1[] = "int f(const Fred &fred) {return fred.pow(12,3);}";
        const char expected_pow1[] = "int f ( const Fred & fred ) { return fred . pow ( 12 , 3 ) ; }";
        ASSERT_EQUALS(expected_pow1, tokWithNewlines(code_pow1));

        const char code_pow2[] = "int f() {return pow(0,0);}";
        const char expected_pow2[] = "int f ( ) { return 1 ; }";
        ASSERT_EQUALS(expected_pow2, tokWithNewlines(code_pow2));

        const char code_pow3[] = "int f() {return pow(0,1);}";
        const char expected_pow3[] = "int f ( ) { return 0 ; }";
        ASSERT_EQUALS(expected_pow3, tokWithNewlines(code_pow3));

        const char code_pow4[] = "int f() {return pow(1,0);}";
        const char expected_pow4[] = "int f ( ) { return 1 ; }";
        ASSERT_EQUALS(expected_pow4, tokWithNewlines(code_pow4));
    }

    void simplifyMathFunctions_fmin() {
        // verify fmin,fminl,fminl simplifcation
        const char code_fmin[] ="void f() {\n"
                                " std::cout << fmin(-1.0,0);\n"
                                " std::cout << fmin(1.0,0);\n"
                                " std::cout << fmin(0,0);\n"
                                " std::cout << fminf(-1.0,0);\n"
                                " std::cout << fminf(1.0,0);\n"
                                " std::cout << fminf(0,0);\n"
                                " std::cout << fminl(-1.0,0);\n"
                                " std::cout << fminl(1.0,0);\n"
                                " std::cout << fminl(0,0);\n"
                                "}";

        const char expected_fmin[] = "void f ( ) {\n"
                                     "std :: cout << -1.0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << -1.0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << -1.0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_fmin, tokWithNewlines(code_fmin));

        // do not simplify this case
        const char code_fmin1[] = "float f(float f) { return fmin(f,0);}";
        const char expected_fmin1[] = "float f ( float f ) { return fmin ( f , 0 ) ; }";
        ASSERT_EQUALS(expected_fmin1, tokWithNewlines(code_fmin1));
    }

    void simplifyMathFunctions_fmax() {
        // verify fmax(),fmax(),fmaxl() simplifcation
        const char code_fmax[] ="void f() {\n"
                                " std::cout << fmax(-1.0,0);\n"
                                " std::cout << fmax(1.0,0);\n"
                                " std::cout << fmax(0,0);\n"
                                " std::cout << fmaxf(-1.0,0);\n"
                                " std::cout << fmaxf(1.0,0);\n"
                                " std::cout << fmaxf(0,0);\n"
                                " std::cout << fmaxl(-1.0,0);\n"
                                " std::cout << fmaxl(1.0,0);\n"
                                " std::cout << fmaxl(0,0);\n"
                                "}";

        const char expected_fmax[] = "void f ( ) {\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 1.0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 1.0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "std :: cout << 1.0 ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_fmax, tokWithNewlines(code_fmax));

        // do not simplify this case
        const char code_fmax1[] = "float f(float f) { return fmax(f,0);}";
        const char expected_fmax1[] = "float f ( float f ) { return fmax ( f , 0 ) ; }";
        ASSERT_EQUALS(expected_fmax1, tokWithNewlines(code_fmax1));
    }

    void simplifyMathExpressions() { //#1620
        const char code1[] = "void foo() {\n"
                             "    std::cout<<pow(sin(x),2)+pow(cos(x),2);\n"
                             "    std::cout<<pow(sin(pow(sin(y),2)+pow(cos(y),2)),2)+pow(cos(pow(sin(y),2)+pow(cos(y),2)),2);\n"
                             "    std::cout<<pow(sin(x),2.0)+pow(cos(x),2.0);\n"
                             "    std::cout<<pow(sin(x*y+z),2.0)+pow(cos(x*y+z),2.0);\n"
                             "    std::cout<<pow(sin(x*y+z),2)+pow(cos(x*y+z),2);\n"
                             "    std::cout<<pow(cos(x),2)+pow(sin(x),2);\n"
                             "    std::cout<<pow(cos(x),2.0)+pow(sin(x),2.0);\n"
                             "    std::cout<<pow(cos(x*y+z),2.0)+pow(sin(x*y+z),2.0);\n"
                             "    std::cout<<pow(cos(x*y+z),2)+pow(sin(x*y+z),2);\n"
                             "    std::cout<<pow(sinh(x*y+z),2)-pow(cosh(x*y+z),2);\n"
                             "    std::cout<<pow(sinh(x),2)-pow(cosh(x),2);\n"
                             "    std::cout<<pow(sinh(x*y+z),2.0)-pow(cosh(x*y+z),2.0);\n"
                             "    std::cout<<pow(sinh(x),2.0)-pow(cosh(x),2.0);\n"
                             "    std::cout<<pow(cosh(x*y+z),2)-pow(sinh(x*y+z),2);\n"
                             "    std::cout<<pow(cosh(x),2)-pow(sinh(x),2);\n"
                             "    std::cout<<pow(cosh(x*y+z),2.0)-pow(sinh(x*y+z),2.0);\n"
                             "    std::cout<<pow(cosh(x),2.0)-pow(sinh(x),2.0);\n"
                             "    std::cout<<pow(cosh(pow(x,1)),2.0)-pow(sinh(pow(x,1)),2.0);\n"
                             "}";

        const char expected1[] = "void foo ( ) {\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "}";
        ASSERT_EQUALS(expected1, tokWithNewlines(code1));

        const char code2[] = "void f ( ) {\n"
                             "a = pow ( sin ( x ) , 2 ) + pow ( cos ( y ) , 2 ) ;\n"
                             "b = pow ( sinh ( x ) , 2 ) - pow ( cosh ( y ) , 2 ) ;\n"
                             "c = pow ( sin ( x ) , 2.0 ) + pow ( cos ( y ) , 2.0 ) ;\n"
                             "d = pow ( sinh ( x ) , 2.0 ) - pow ( cosh ( y ) , 2.0 ) ;\n"
                             "e = pow ( cos ( x ) , 2 ) + pow ( sin ( y ) , 2 ) ;\n"
                             "f = pow ( cosh ( x ) , 2 ) - pow ( sinh ( y ) , 2 ) ;\n"
                             "g = pow ( cos ( x ) , 2.0 ) + pow ( sin ( y ) , 2.0 ) ;\n"
                             "h = pow ( cosh ( x ) , 2.0 ) - pow ( sinh ( y ) , 2.0 ) ;\n"
                             "}";
        ASSERT_EQUALS(code2, tokWithNewlines(code2));

        const char code3[] = "void foo() {\n"
                             "    std::cout<<powf(sinf(x),2)+powf(cosf(x),2);\n"
                             "    std::cout<<powf(sinf(powf(sinf(y),2)+powf(cosf(y),2)),2)+powf(cosf(powf(sinf(y),2)+powf(cosf(y),2)),2);\n"
                             "    std::cout<<powf(sinf(x),2.0)+powf(cosf(x),2.0);\n"
                             "    std::cout<<powf(sinf(x*y+z),2.0)+powf(cosf(x*y+z),2.0);\n"
                             "    std::cout<<powf(sinf(x*y+z),2)+powf(cosf(x*y+z),2);\n"
                             "    std::cout<<powf(cosf(x),2)+powf(sinf(x),2);\n"
                             "    std::cout<<powf(cosf(x),2.0)+powf(sinf(x),2.0);\n"
                             "    std::cout<<powf(cosf(x*y+z),2.0)+powf(sinf(x*y+z),2.0);\n"
                             "    std::cout<<powf(cosf(x*y+z),2)+powf(sinf(x*y+z),2);\n"
                             "    std::cout<<powf(sinhf(x*y+z),2)-powf(coshf(x*y+z),2);\n"
                             "    std::cout<<powf(sinhf(x),2)-powf(coshf(x),2);\n"
                             "    std::cout<<powf(sinhf(x*y+z),2.0)-powf(coshf(x*y+z),2.0);\n"
                             "    std::cout<<powf(sinhf(x),2.0)-powf(coshf(x),2.0);\n"
                             "    std::cout<<powf(coshf(x*y+z),2)-powf(sinhf(x*y+z),2);\n"
                             "    std::cout<<powf(coshf(x),2)-powf(sinhf(x),2);\n"
                             "    std::cout<<powf(coshf(x*y+z),2.0)-powf(sinhf(x*y+z),2.0);\n"
                             "    std::cout<<powf(coshf(x),2.0)-powf(sinhf(x),2.0);\n"
                             "    std::cout<<powf(coshf(powf(x,1)),2.0)-powf(sinhf(powf(x,1)),2.0);\n"
                             "}";

        const char expected3[] = "void foo ( ) {\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "}";
        ASSERT_EQUALS(expected3, tokWithNewlines(code3));

        const char code4[] = "void f ( ) {\n"
                             "a = powf ( sinf ( x ) , 2 ) + powf ( cosf ( y ) , 2 ) ;\n"
                             "b = powf ( sinhf ( x ) , 2 ) - powf ( coshf ( y ) , 2 ) ;\n"
                             "c = powf ( sinf ( x ) , 2.0 ) + powf ( cosf ( y ) , 2.0 ) ;\n"
                             "d = powf ( sinhf ( x ) , 2.0 ) - powf ( coshf ( y ) , 2.0 ) ;\n"
                             "e = powf ( cosf ( x ) , 2 ) + powf ( sinf ( y ) , 2 ) ;\n"
                             "f = powf ( coshf ( x ) , 2 ) - powf ( sinhf ( y ) , 2 ) ;\n"
                             "g = powf ( cosf ( x ) , 2.0 ) + powf ( sinf ( y ) , 2.0 ) ;\n"
                             "h = powf ( coshf ( x ) , 2.0 ) - powf ( sinhf ( y ) , 2.0 ) ;\n"
                             "}";
        ASSERT_EQUALS(code4, tokWithNewlines(code4));

        const char code5[] = "void foo() {\n"
                             "    std::cout<<powf(sinl(x),2)+powl(cosl(x),2);\n"
                             "    std::cout<<pow(sinl(powl(sinl(y),2)+powl(cosl(y),2)),2)+powl(cosl(powl(sinl(y),2)+powl(cosl(y),2)),2);\n"
                             "    std::cout<<powl(sinl(x),2.0)+powl(cosl(x),2.0);\n"
                             "    std::cout<<powl(sinl(x*y+z),2.0)+powl(cosl(x*y+z),2.0);\n"
                             "    std::cout<<powl(sinl(x*y+z),2)+powl(cosl(x*y+z),2);\n"
                             "    std::cout<<powl(cosl(x),2)+powl(sinl(x),2);\n"
                             "    std::cout<<powl(cosl(x),2.0)+powl(sinl(x),2.0);\n"
                             "    std::cout<<powl(cosl(x*y+z),2.0)+powl(sinl(x*y+z),2.0);\n"
                             "    std::cout<<powl(cosl(x*y+z),2)+powl(sinl(x*y+z),2);\n"
                             "    std::cout<<powl(sinhl(x*y+z),2)-powl(coshl(x*y+z),2);\n"
                             "    std::cout<<powl(sinhl(x),2)-powl(coshl(x),2);\n"
                             "    std::cout<<powl(sinhl(x*y+z),2.0)-powl(coshl(x*y+z),2.0);\n"
                             "    std::cout<<powl(sinhl(x),2.0)-powl(coshl(x),2.0);\n"
                             "    std::cout<<powl(coshl(x*y+z),2)-powl(sinhl(x*y+z),2);\n"
                             "    std::cout<<powl(coshl(x),2)-powl(sinhl(x),2);\n"
                             "    std::cout<<powl(coshl(x*y+z),2.0)-powl(sinhl(x*y+z),2.0);\n"
                             "    std::cout<<powl(coshl(x),2.0)-powl(sinhl(x),2.0);\n"
                             "    std::cout<<powl(coshl(powl(x,1)),2.0)-powl(sinhl(powl(x,1)),2.0);\n"
                             "}";

        const char expected5[] = "void foo ( ) {\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << 1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "std :: cout << -1 ;\n"
                                 "}";
        ASSERT_EQUALS(expected5, tokWithNewlines(code5));


        const char code6[] = "void f ( ) {\n"
                             "a = powl ( sinl ( x ) , 2 ) + powl ( cosl ( y ) , 2 ) ;\n"
                             "b = powl ( sinhl ( x ) , 2 ) - powl ( coshl ( y ) , 2 ) ;\n"
                             "c = powl ( sinl ( x ) , 2.0 ) + powl ( cosl ( y ) , 2.0 ) ;\n"
                             "d = powl ( sinhl ( x ) , 2.0 ) - powl ( coshl ( y ) , 2.0 ) ;\n"
                             "e = powl ( cosl ( x ) , 2 ) + powl ( sinl ( y ) , 2 ) ;\n"
                             "f = powl ( coshl ( x ) , 2 ) - powl ( sinhl ( y ) , 2 ) ;\n"
                             "g = powl ( cosl ( x ) , 2.0 ) + powl ( sinl ( y ) , 2.0 ) ;\n"
                             "h = powl ( coshl ( x ) , 2.0 ) - powl ( sinhl ( y ) , 2.0 ) ;\n"
                             "}";
        ASSERT_EQUALS(code6, tokWithNewlines(code6));
    }


    void simplifyAssignmentInFunctionCall() {
        ASSERT_EQUALS("; x = g ( ) ; f ( x ) ;", tok(";f(x=g());"));
        ASSERT_EQUALS("; hs = ( xyz_t ) { h . centerX , h . centerY , 1 + index } ; putInput ( hs , 1 ) ;", tok(";putInput(hs = (xyz_t) { h->centerX, h->centerY, 1 + index }, 1);"));
    }

    void simplifyCompoundAssignment() {
        ASSERT_EQUALS("; x = x + y ;", tok("; x += y;"));
        ASSERT_EQUALS("; x = x - y ;", tok("; x -= y;"));
        ASSERT_EQUALS("; x = x * y ;", tok("; x *= y;"));
        ASSERT_EQUALS("; x = x / y ;", tok("; x /= y;"));
        ASSERT_EQUALS("; x = x % y ;", tok("; x %= y;"));
        ASSERT_EQUALS("; x = x & y ;", tok("; x &= y;"));
        ASSERT_EQUALS("; x = x | y ;", tok("; x |= y;"));
        ASSERT_EQUALS("; x = x ^ y ;", tok("; x ^= y;"));
        ASSERT_EQUALS("; x = x << y ;", tok("; x <<= y;"));
        ASSERT_EQUALS("; x = x >> y ;", tok("; x >>= y;"));

        ASSERT_EQUALS("{ x = x + y ; }", tok("{ x += y;}"));
        ASSERT_EQUALS("{ x = x - y ; }", tok("{ x -= y;}"));
        ASSERT_EQUALS("{ x = x * y ; }", tok("{ x *= y;}"));
        ASSERT_EQUALS("{ x = x / y ; }", tok("{ x /= y;}"));
        ASSERT_EQUALS("{ x = x % y ; }", tok("{ x %= y;}"));
        ASSERT_EQUALS("{ x = x & y ; }", tok("{ x &= y;}"));
        ASSERT_EQUALS("{ x = x | y ; }", tok("{ x |= y;}"));
        ASSERT_EQUALS("{ x = x ^ y ; }", tok("{ x ^= y;}"));
        ASSERT_EQUALS("{ x = x << y ; }", tok("{ x <<= y;}"));
        ASSERT_EQUALS("{ x = x >> y ; }", tok("{ x >>= y;}"));

        ASSERT_EQUALS("; * p = * p + y ;", tok("; *p += y;"));
        ASSERT_EQUALS("; ( * p ) = ( * p ) + y ;", tok("; (*p) += y;"));
        ASSERT_EQUALS("; * ( p [ 0 ] ) = * ( p [ 0 ] ) + y ;", tok("; *(p[0]) += y;"));
        ASSERT_EQUALS("; p [ { 1 , 2 } ] = p [ { 1 , 2 } ] + y ;", tok("; p[{1,2}] += y;"));

        ASSERT_EQUALS("void foo ( ) { switch ( n ) { case 0 : ; x = x + y ; break ; } }", tok("void foo() { switch (n) { case 0: x += y; break; } }"));

        ASSERT_EQUALS("; x . y = x . y + 1 ;", tok("; x.y += 1;"));

        ASSERT_EQUALS("; x [ 0 ] = x [ 0 ] + 1 ;", tok("; x[0] += 1;"));
        ASSERT_EQUALS("; x [ y - 1 ] = x [ y - 1 ] + 1 ;", tok("; x[y-1] += 1;"));
        ASSERT_EQUALS("; x [ y ] = x [ y ++ ] + 1 ;", tok("; x[y++] += 1;"));
        ASSERT_EQUALS("; x [ ++ y ] = x [ y ] + 1 ;", tok("; x[++y] += 1;"));

        ASSERT_EQUALS(";", tok(";x += 0;"));
        TODO_ASSERT_EQUALS(";", "; x = x + '\\0' ;", tok("; x += '\\0'; "));
        ASSERT_EQUALS(";", tok(";x -= 0;"));
        ASSERT_EQUALS(";", tok(";x |= 0;"));
        ASSERT_EQUALS(";", tok(";x *= 1;"));
        ASSERT_EQUALS(";", tok(";x /= 1;"));

        ASSERT_EQUALS("; a . x ( ) = a . x ( ) + 1 ;", tok("; a.x() += 1;"));
        ASSERT_EQUALS("; x ( 1 ) = x ( 1 ) + 1 ;", tok("; x(1) += 1;"));

        // #2368
        ASSERT_EQUALS("{ j = j - i ; }", tok("{if (false) {} else { j -= i; }}"));

        // #2714 - wrong simplification of "a += b?c:d;"
        ASSERT_EQUALS("; a = a + ( b ? c : d ) ;", tok("; a+=b?c:d;"));
        ASSERT_EQUALS("; a = a * ( b + 1 ) ;", tok("; a*=b+1;"));

        ASSERT_EQUALS("; a = a + ( b && c ) ;", tok("; a+=b&&c;"));
        ASSERT_EQUALS("; a = a * ( b || c ) ;", tok("; a*=b||c;"));
        ASSERT_EQUALS("; a = a | ( b == c ) ;", tok("; a|=b==c;"));

        // #3469
        ASSERT_EQUALS("; a = a + ( b = 1 ) ;", tok("; a += b = 1;"));

        // #7571
        ASSERT_EQUALS("; foo = foo + [ & ] ( ) { } ;", tok("; foo += [&]() {int i;};"));

        // #8796
        ASSERT_EQUALS("{ return ( a = b ) += c ; }", tok("{ return (a = b) += c; }"));
    }


    void cast() {
        ASSERT_EQUALS("{ if ( p == 0 ) { ; } }", tok("{if (p == (char *)0);}"));
        ASSERT_EQUALS("{ return str ; }", tok("{return (char *)str;}"));

        ASSERT_EQUALS("{ if ( * a ) }", tok("{if ((char)*a)}"));
        ASSERT_EQUALS("{ if ( & a ) }", tok("{if ((int)&a)}"));
        ASSERT_EQUALS("{ if ( * a ) }", tok("{if ((unsigned int)(unsigned char)*a)}"));
        ASSERT_EQUALS("class A { A operator* ( int ) ; } ;", tok("class A { A operator *(int); };"));
        ASSERT_EQUALS("class A { A operator* ( int ) const ; } ;", tok("class A { A operator *(int) const; };"));
        ASSERT_EQUALS("{ if ( p == 0 ) { ; } }", tok("{ if (p == (char *)(char *)0); }"));
        ASSERT_EQUALS("{ if ( p == 0 ) { ; } }", tok("{ if (p == (char **)0); }"));

        // no simplification as the cast may be important here. see #2897 for example
        ASSERT_EQUALS("; * ( ( char * ) p + 1 ) = 0 ;", tok("; *((char *)p + 1) = 0;"));

        ASSERT_EQUALS("{ if ( true ) }", tok("{ if ((unsigned char)1) }")); // #4164
        ASSERT_EQUALS("f ( 200 )", tok("f((unsigned char)200)"));
        ASSERT_EQUALS("f ( ( char ) 1234 )", tok("f((char)1234)")); // don't simplify downcast
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

        const char code3[] = "x = L\"1\" TEXT(\"2\") L\"3\";";
        ASSERT_EQUALS("x = L\"123\" ;", tok(code3, false, Settings::Win64));

        const char code4[] = "x = TEXT(\"1\") L\"2\";";
        ASSERT_EQUALS("x = L\"1\" L\"2\" ;", tok(code4, false, Settings::Win64));
    }

    void combine_wstrings() {
        const char code[] =  "a = L\"hello \"  L\"world\" ;\n";

        const char expected[] =  "a = L\"hello world\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_ustrings() {
        const char code[] =  "abcd = u\"ab\" u\"cd\";";

        const char expected[] =  "abcd = u\"abcd\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_Ustrings() {
        const char code[] =  "abcd = U\"ab\" U\"cd\";";

        const char expected[] =  "abcd = U\"abcd\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_u8strings() {
        const char code[] =  "abcd = u8\"ab\" u8\"cd\";";

        const char expected[] =  "abcd = u8\"abcd\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_mixedstrings() {
        const char code[] = "abcdef = \"ab\" L\"cd\" \"ef\";";

        const char expected[] =  "abcdef = L\"abcdef\" ;";

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
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
        ASSERT_EQUALS("{ while ( x ( ) == -1 ) { } }", tok("{while((x()) == -1){ }}"));
    }

    void parenthesesVar() {
        // remove parentheses..
        ASSERT_EQUALS("a = p ;", tok("a = (p);"));
        ASSERT_EQUALS("void f ( ) { if ( a < p ) { } }", tok("void f(){if(a<(p)){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p == -1 ) { } }", tok("void f(){int p; if((p)==-1){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( -1 == p ) { } }", tok("void f(){int p; if(-1==(p)){}}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p ) { } }", tok("void f(){int p; if((p)){}}"));
        ASSERT_EQUALS("void f ( ) { return p ; }", tok("void f(){return (p);}"));
        ASSERT_EQUALS("void f ( ) { int * p ; if ( * p == 0 ) { } }", tok("void f(){int *p; if (*(p) == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { int * p ; if ( * p == 0 ) { } }", tok("void f(){int *p; if (*p == 0) {}}"));
        ASSERT_EQUALS("void f ( int & p ) { p = 1 ; }", tok("void f(int &p) {(p) = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p [ 10 ] ; p [ 0 ] = 1 ; }", tok("void f(){int p[10]; (p)[0] = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p == 0 ) { } }", tok("void f(){int p; if ((p) == 0) {}}"));
        ASSERT_EQUALS("void f ( ) { int * p ; * p = 1 ; }", tok("void f(){int *p; *(p) = 1;}"));
        ASSERT_EQUALS("void f ( ) { int p ; if ( p ) { } p = 1 ; }", tok("void f(){int p; if ( p ) { } (p) = 1;}"));
        ASSERT_EQUALS("void f ( ) { a . b ; }", tok("void f ( ) { ( & a ) -> b ; }")); // Ticket #5776

        // keep parentheses..
        ASSERT_EQUALS("b = a ;", tok("b = (char)a;"));
        ASSERT_EQUALS("cast < char * > ( p ) ;", tok("cast<char *>(p);"));
        ASSERT_EQUALS("void f ( ) { return ( a + b ) * c ; }", tok("void f(){return (a+b)*c;}"));
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

    void elseif1() {
        const char code[] = "void f(){ if(x) {} else if(ab) { cd } else { ef }gh; }";
        ASSERT_EQUALS("\n\n##file 0\n1: void f ( ) { if ( x ) { } else { if ( ab ) { cd } else { ef } } gh ; }\n", tokenizeDebugListing(code));

        // syntax error: assert there is no segmentation fault
        ASSERT_EQUALS("\n\n##file 0\n1: void f ( ) { if ( x ) { } else { if ( x ) { } } }\n", tokenizeDebugListing("void f(){ if(x) {} else if (x) { } }"));

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
                                    "if ( f == 0 ) { coo ( ) ; } "
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

        // Ticket #6860 - lambdas
        {
            const char src[] = "( []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() );";
            const char expected[] = "\n\n##file 0\n1: ( [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) ) ;\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
        {
            const char src[] = "[ []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() ];";
            const char expected[] = "\n\n##file 0\n1: [ [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) ] ;\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
        {
            const char src[] = "= { []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() }";
            const char expected[] = "\n\n##file 0\n1: = { [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
    }


    unsigned int sizeofFromTokenizer(const char type[]) {
        Tokenizer tokenizer(&settings0, this);
        tokenizer.fillTypeSizes();
        Token tok1;
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

        code = "char i[2][20][30];\n"
               "sizeof(i[1][4][2]);\n"
               "sizeof(***i);\n"
               "sizeof(i[1][4]);\n"
               "sizeof(**i);\n"
               "sizeof(i[1]);\n"
               "sizeof(*i);\n"
               "sizeof(i);";
        ASSERT_EQUALS("char i [ 2 ] [ 20 ] [ 30 ] ; 1 ; 1 ; 30 ; 30 ; 600 ; 600 ; 1200 ;", tok(code));

        code = "sizeof(char[20]);\n"
               "sizeof(char[20][3]);\n"
               "sizeof(char[unknown][3]);";
        ASSERT_EQUALS("20 ; 60 ; sizeof ( char [ unknown ] [ 3 ] ) ;", tok(code));
    }

    void sizeof5() {
        const char code[] =
            "{"
            "const char * names[2];"
            "for (int i = 0; i != sizeof(names[0]); i++)"
            "{}"
            "}";
        std::ostringstream expected;
        expected << "{ const char * names [ 2 ] ; for ( int i = 0 ; i != " << sizeofFromTokenizer("*") << " ; i ++ ) { } }";
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
        ASSERT_EQUALS("; INT32 i [ 10 ] ; sizeof ( i [ 0 ] ) ;", tok(code, true, Settings::Native));
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
            expected << "; " << (sizeof "123") << " ;";

            ASSERT_EQUALS(expected.str(), tok("; sizeof \"123\";"));
            ASSERT_EQUALS(expected.str(), tok("; sizeof(\"123\");"));
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
        // sizeof from library
        const char code[] = "foo(sizeof(uint32_t), sizeof(std::uint32_t));";
        TODO_ASSERT_EQUALS("foo ( 4 , 4 ) ;", "foo ( 4 , sizeof ( std :: uint32_t ) ) ;", tokWithStdLib(code));
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

            ASSERT_EQUALS(";", tok(code));
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
        // tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyIfAndWhileAssign();

        return tokenizer.tokens()->stringifyList(nullptr, false);
    }

    void ifassign1() {
        ASSERT_EQUALS("{ a = b ; if ( a ) { ; } }", simplifyIfAndWhileAssign("{if(a=b);}"));
        ASSERT_EQUALS("{ a = b ( ) ; if ( a ) { ; } }", simplifyIfAndWhileAssign("{if((a=b()));}"));
        ASSERT_EQUALS("{ a = b ( ) ; if ( ! ( a ) ) { ; } }", simplifyIfAndWhileAssign("{if(!(a=b()));}"));
        ASSERT_EQUALS("{ a . x = b ( ) ; if ( ! ( a . x ) ) { ; } }", simplifyIfAndWhileAssign("{if(!(a->x=b()));}"));
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
                               "if ( f == NULL ) "
                               "{ "
                               "return -1 ; "
                               "} "
                               "fclose ( f ) ; "
                               "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void whileAssign1() {
        ASSERT_EQUALS("{ a = b ; while ( a ) { b = 0 ; a = b ; } }", simplifyIfAndWhileAssign("{while(a=b) { b = 0; }}"));
        ASSERT_EQUALS("{ a . b = c ; while ( a . b ) { c = 0 ; a . b = c ; } }", simplifyIfAndWhileAssign("{while(a.b=c) { c=0; }}"));
        ASSERT_EQUALS("{ "
                      "struct hfs_bnode * node ; "
                      "struct hfs_btree * tree ; "
                      "node = tree . node_hash [ i ++ ] ; "
                      "while ( node ) { node = tree . node_hash [ i ++ ] ; } "
                      "}",
                      tok("{"
                          "struct hfs_bnode *node;"
                          "struct hfs_btree *tree;"
                          "while ((node = tree->node_hash[i++])) { }"
                          "}"));
        ASSERT_EQUALS("{ char * s ; s = new char [ 10 ] ; while ( ! s ) { s = new char [ 10 ] ; } }",
                      tok("{ char *s; while (0 == (s=new char[10])) { } }"));
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

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr("{ while (!(m = q->push<Message>(x))) {} }");
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        ASSERT_EQUALS("{ m = q . push < Message > ( x ) ; while ( ! m ) { m = q . push < Message > ( x ) ; } }", tokenizer.tokens()->stringifyList(nullptr, false));
        ASSERT(tokenizer.tokens()->tokAt(26) != nullptr);
        if (tokenizer.tokens()->tokAt(26)) {
            ASSERT(tokenizer.tokens()->linkAt(6) == tokenizer.tokens()->tokAt(8));
            ASSERT(tokenizer.tokens()->linkAt(24) == tokenizer.tokens()->tokAt(26));
        }
    }

    void doWhileAssign() {
        ASSERT_EQUALS("{ do { a = b ; } while ( a ) ; }", simplifyIfAndWhileAssign("{ do { } while(a=b); }"));
        ASSERT_EQUALS("{ do { a . a = 0 ; a . b = c ; } while ( a . b ) ; }", simplifyIfAndWhileAssign("{ do { a.a = 0; } while(a.b=c); }"));
        ASSERT_EQUALS("{ "
                      "struct hfs_bnode * node ; "
                      "struct hfs_btree * tree ; "
                      "do { node = tree . node_hash [ i ++ ] ; } while ( node ) ; "
                      "}",
                      tok("{"
                          "struct hfs_bnode *node;"
                          "struct hfs_btree *tree;"
                          "do { } while((node = tree->node_hash[i++]));"
                          "}"));
        ASSERT_EQUALS("void foo ( ) { char * s ; do { s = new char [ 10 ] ; } while ( ! s ) ; }",
                      tok("void foo() { char *s; do { } while (0 == (s=new char[10])); }"));
        // #4911
        ASSERT_EQUALS("void foo ( ) { do { current = f ( ) ; } while ( ( current ) != NULL ) ; }", simplifyIfAndWhileAssign("void foo() { do { } while((current=f()) != NULL); }"));
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
        ASSERT_EQUALS("void f ( ) { err |= ( ( r & s ) && ! t ) ; }",
                      tok("void f() { err or_eq ((r bitand s) and not t); }", "test.c", false));
        ASSERT_EQUALS("void f ( ) const { r = f ( a [ 4 ] | 0x0F , ~ c , ! d ) ; }",
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
            ASSERT_EQUALS("a ? ( b = c , d ) : e ;", tok("a ? b = c , d : e ;")); // Keep comma
        }

        {
            ASSERT_EQUALS("{ return a ? ( b = c , d ) : e ; }", tok("{ return a ? b = c , d : e ; }")); // Keep comma
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

        {
            const char code[]     = "tr = (struct reg){ .a = (1), .c = (2) };";
            const char expected[] = "tr = ( struct reg ) { . a = 1 , . c = 2 } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void simplifyConditionOperator() {
        {
            const char code[] = "(0?(false?1:2):3);";
            ASSERT_EQUALS("( 3 ) ;", tok(code));
        }

        {
            const char code[] = "(1?(false?1:2):3);";
            ASSERT_EQUALS("( 2 ) ;", tok(code));
        }

        {
            const char code[] = "int a = (1?0:1 == 1?0:1);";
            ASSERT_EQUALS("int a ; a = 0 ;", tok(code));
        }

        {
            const char code[] = "(1?0:foo());";
            ASSERT_EQUALS("( 0 ) ;", tok(code));
        }

        {
            const char code[] = "void f () { switch(n) { case 1?0:foo(): break; }}";
            // TODO Do not throw AST validation exception
            TODO_ASSERT_THROW(tok(code), InternalError);
            //ASSERT_EQUALS("void f ( ) { switch ( n ) { case 0 : ; break ; } }", tok(code));
        }

        {
            const char code[] = "void f () { switch(n) { case 1?0?1:0:foo(): break; }}";
            // TODO Do not throw AST validation exception
            TODO_ASSERT_THROW(tok(code), InternalError);
        }

        {
            const char code[] = "void f () { switch(n) { case 0?foo():1: break; }}";
            // TODO Do not throw AST validation exception
            TODO_ASSERT_THROW(tok(code), InternalError);
        }

        {
            const char code[] = "( true ? a ( ) : b ( ) );";
            ASSERT_EQUALS("( a ( ) ) ;", tok(code));
        }

        {
            const char code[] = "( true ? abc . a : abc . b );";
            ASSERT_EQUALS("( abc . a ) ;", tok(code));
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
            ASSERT_EQUALS("int vals [ 2 ] = { 0x13 , 0x01 } ;", tok(code));
        }

        {
            const char code[] = "int vals[] = { 0x13, 0?0x01:0x00 };";
            ASSERT_EQUALS("int vals [ 2 ] = { 0x13 , 0x00 } ;", tok(code));
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

        {
            // TODO Do not throw AST validation exception
            TODO_ASSERT_THROW(tok("; type = decay_t<decltype(true ? declval<T>() : declval<U>())>;"), InternalError);
            TODO_ASSERT_THROW(tok("; type = decay_t<decltype(false ? declval<T>() : declval<U>())>;"), InternalError);
        }
    }

    void calculations() {
        {
            const char code[] = "a[i+8+2];";
            ASSERT_EQUALS("a [ i + 10 ] ;", tok(code));
        }
        {
            const char code[] = "a[8+2+i];";
            ASSERT_EQUALS("a [ 10 + i ] ;", tok(code));
        }
        {
            const char code[] = "a[i + 2 * (2 * 4)];";
            ASSERT_EQUALS("a [ i + 16 ] ;", tok(code));
        }
        {
            const char code[] = "a[i + 100 - 90];";
            ASSERT_EQUALS("a [ i + 10 ] ;", tok(code));
        }
        {
            const char code[] = "a[1+1+1+1+1+1+1+1+1+1-2+5-3];";
            ASSERT_EQUALS("a [ 10 ] ;", tok(code));
        }
        {
            const char code[] = "a[10+10-10-10];";
            ASSERT_EQUALS("a [ 0 ] ;", tok(code));
        }

        ASSERT_EQUALS("a [ 4 ] ;", tok("a[1+3|4];"));
        ASSERT_EQUALS("a [ 4U ] ;", tok("a[1+3|4U];"));
        ASSERT_EQUALS("a [ 3 ] ;", tok("a[1+2&3];"));
        ASSERT_EQUALS("a [ 3U ] ;", tok("a[1+2&3U];"));
        ASSERT_EQUALS("a [ 5 ] ;", tok("a[1-0^4];"));
        ASSERT_EQUALS("a [ 5U ] ;", tok("a[1-0^4U];"));

        ASSERT_EQUALS("x = 1 + 2 * y ;", tok("x=1+2*y;"));
        ASSERT_EQUALS("x = 7 ;", tok("x=1+2*3;"));
        ASSERT_EQUALS("x = 47185 ;", tok("x=(65536*72/100);"));
        ASSERT_EQUALS("x = 900 ;", tok("x = 1500000 / ((145000 - 55000) * 1000 / 54000);"));
        ASSERT_EQUALS("int a [ 8 ] ;", tok("int a[5+6/2];"));
        ASSERT_EQUALS("int a [ 4 ] ;", tok("int a[(10)-1-5];"));
        ASSERT_EQUALS("int a [ i - 9 ] ;", tok("int a[i - 10 + 1];"));
        ASSERT_EQUALS("int a [ i - 11 ] ;", tok("int a[i - 10 - 1];"));

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

        ASSERT_EQUALS("{ if ( a == 2 ) { } }", tok("{if (a==1+1){}}"));
        ASSERT_EQUALS("{ if ( a + 2 != 6 ) { } }", tok("{if (a+1+1!=1+2+3){}}"));
        ASSERT_EQUALS("{ if ( 4 < a ) { } }", tok("{if (14-2*5<a*4/(2*2)){}}"));

        ASSERT_EQUALS("( y / 2 - 2 ) ;", tok("(y / 2 - 2);"));
        ASSERT_EQUALS("( y % 2 - 2 ) ;", tok("(y % 2 - 2);"));

        ASSERT_EQUALS("( 4 ) ;", tok("(1 * 2 / 1 * 2);")); // #3722

        ASSERT_EQUALS("x ( 60129542144 ) ;", tok("x(14<<4+17<<300%17);")); // #4931
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(8|5&6+0 && 7);")); // #6104
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(2 && 4<<4<<5 && 4);")); // #4933
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(9&&8%5%4/3);")); // #4931
        ASSERT_EQUALS("x ( 1 ) ;", tok("x(2 && 2|5<<2%4);")); // #4931
        ASSERT_EQUALS("x ( -2 << 6 | 1 ) ;", tok("x(1-3<<6|5/3);")); // #4931
        ASSERT_EQUALS("x ( 2 ) ;", tok("x(2|0*0&2>>1+0%2*1);")); // #4931
        ASSERT_EQUALS("x ( 0 & 4 != 1 ) ;", tok("x(4%1<<1&4!=1);")); // #4931 (can be simplified further but it's not a problem)
        ASSERT_EQUALS("x ( true ) ;", tok("x(0&&4>0==2||4);")); // #4931

        // don't remove these spaces..
        ASSERT_EQUALS("new ( auto ) ( 4 ) ;", tok("new (auto)(4);"));
    }

    void comparisons() {
        ASSERT_EQUALS("( 1 ) ;", tok("( 1 < 2 );"));
        ASSERT_EQUALS("( x && true ) ;", tok("( x && 1 < 2 );"));
        ASSERT_EQUALS("( 5 ) ;", tok("( 1 < 2 && 3 < 4 ? 5 : 6 );"));
        ASSERT_EQUALS("( 6 ) ;", tok("( 1 > 2 && 3 > 4 ? 5 : 6 );"));
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
            ASSERT_EQUALS("void f ( ) { MACRO ( bar1 , exit ( 0 ) ) }", tokWithStdLib("void f() { MACRO(bar1, exit(0)) }"));
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
        ASSERT_EQUALS("template < class T > class SharedPtr { "
                      "SharedPtr & operator= ( SharedPtr < Y > const & r ) ; "
                      "} ; "
                      "class TClass { "
                      "public: TClass & operator= ( const TClass & rhs ) ; "
                      "} ; "
                      "TClass :: TClass ( const TClass & other ) { operator= ( other ) ; }",
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

    void simplifyArrayAccessSyntax() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int a@1 ; a@1 [ 13 ] ;\n", tokenizeDebugListing("int a; 13[a];"));
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

        {
            // #7849
            const char code[] =
                "void f() {\n"
                "if (-1e-2 == -0.01) \n"
                "    g();\n"
                "else\n"
                "    h();\n"
                "}";
            ASSERT_EQUALS("void f ( ) { if ( -1e-2 == -0.01 ) { g ( ) ; } else { h ( ) ; } }",
                          tok(code));
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

        {
            // #4931
            const char code[] =
                "void f() {\n"
                "if (12 && 7) g();\n"
                "}";
            ASSERT_EQUALS("void f ( ) { g ( ) ; }", tok(code));
        }
    }


    void pointeralias1() {
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

    void while0() {
        ASSERT_EQUALS("void foo ( ) { x = 1 ; }", tok("void foo() { do { x = 1 ; } while (0);}"));
        ASSERT_EQUALS("void foo ( ) { return 0 ; }", tok("void foo() { do { return 0; } while (0);}"));
        ASSERT_EQUALS("void foo ( ) { goto label ; }", tok("void foo() { do { goto label; } while (0); }"));
        ASSERT_EQUALS("void foo ( ) { continue ; }", tok("void foo() { do { continue ; } while (0); }"));
        ASSERT_EQUALS("void foo ( ) { break ; }", tok("void foo() { do { break; } while (0); }"));
        ASSERT_EQUALS("void foo ( ) { }", tok("void foo() { while (false) { a; } }"));
        ASSERT_EQUALS("void foo ( ) { }", tok("void foo() { while (false) { switch (n) { case 0: return; default: break; } n*=1; } }"));
    }

    void while0for() {
        // for (condition is always false)
        ASSERT_EQUALS("void f ( ) { int i ; for ( i = 0 ; i < 0 ; i ++ ) { } }", tok("void f() { int i; for (i = 0; i < 0; i++) { a; } }"));
        //ticket #3140
        ASSERT_EQUALS("void f ( ) { int i ; for ( i = 0 ; i < 0 ; i ++ ) { } }", tok("void f() { int i; for (i = 0; i < 0; i++) { foo(); break; } }"));
        ASSERT_EQUALS("void f ( ) { int i ; for ( i = 0 ; i < 0 ; i ++ ) { } }", tok("void f() { int i; for (i = 0; i < 0; i++) { foo(); continue; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (int i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (unsigned int i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (long long i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { for (signed long long i = 0; i < 0; i++) { a; } }"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f() { int n = 0; for (signed long long i = 0; i < n; i++) { a; } }"));
        // #8059
        ASSERT_EQUALS("void f ( ) { int i ; for ( i = 0 ; i < 0 ; ++ i ) { } return i ; }", tok("void f() { int i; for (i=0;i<0;++i){ dostuff(); } return i; }"));
    }

    void duplicateDefinition() { // #3565 - wrongly detects duplicate definition
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr("{ x ; return a not_eq x; }");
        tokenizer.tokenize(istr, "test.c");
        Token *x_token = tokenizer.list.front()->tokAt(5);
        ASSERT_EQUALS(false, tokenizer.duplicateDefinition(&x_token));
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
        ASSERT_EQUALS("{ while ( f ( ) ) { } }",
                      tok("{ while (f() && errno == EINTR) { } }"));
        ASSERT_EQUALS("{ while ( f ( ) ) { } }",
                      tok("{ while (f() && (errno == EINTR)) { } }"));
    }

    void simplifyFuncInWhile() {
        ASSERT_EQUALS("{ "
                      "int cppcheck:r1 = fclose ( f ) ; "
                      "while ( cppcheck:r1 ) "
                      "{ "
                      "foo ( ) ; "
                      "cppcheck:r1 = fclose ( f ) ; "
                      "} "
                      "}",
                      tok("{while(fclose(f))foo();}"));

        ASSERT_EQUALS("{ "
                      "int cppcheck:r1 = fclose ( f ) ; "
                      "while ( cppcheck:r1 ) "
                      "{ "
                      "; cppcheck:r1 = fclose ( f ) ; "
                      "} "
                      "}",
                      tok("{while(fclose(f));}"));

        ASSERT_EQUALS("{ "
                      "int cppcheck:r1 = fclose ( f ) ; "
                      "while ( cppcheck:r1 ) "
                      "{ "
                      "; cppcheck:r1 = fclose ( f ) ; "
                      "} "
                      "int cppcheck:r2 = fclose ( g ) ; "
                      "while ( cppcheck:r2 ) "
                      "{ "
                      "; cppcheck:r2 = fclose ( g ) ; "
                      "} "
                      "}",
                      tok("{while(fclose(f)); while(fclose(g));}"));
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
            const char code[] = "struct {int a;} const array[3] = {0};";
            const char expected[] = "struct Anonymous0 { int a ; } ; struct Anonymous0 const array [ 3 ] = { 0 } ;";
            ASSERT_EQUALS(expected, tok(code, false));
        }

        {
            const char code[] = "static struct {int a;} const array[3] = {0};";
            const char expected[] = "struct Anonymous0 { int a ; } ; static struct Anonymous0 const array [ 3 ] = { 0 } ;";
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

        // ticket #8284
        {
            const char code[] = "void f() { class : foo<int> { } abc; }";
            const char expected[] = "void f ( ) { class Anonymous0 : foo < int > { } ; Anonymous0 abc ; }";
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

    void simplifyStructDecl8() {
        ASSERT_EQUALS("enum A { x , y , z } ; enum A a ; a = x ;", tok("enum A { x, y, z } a(x);", false));
        ASSERT_EQUALS("enum B { x , y , z } ; enum B b ; b = x ;", tok("enum B { x , y, z } b{x};", false));
        ASSERT_EQUALS("struct C { int i ; } ; struct C c ; c = { 0 } ;", tok("struct C { int i; } c{0};", false));
        ASSERT_EQUALS("enum Anonymous0 { x , y , z } ; enum Anonymous0 d ; d = x ;", tok("enum { x, y, z } d(x);", false));
        ASSERT_EQUALS("enum Anonymous0 { x , y , z } ; enum Anonymous0 e ; e = x ;", tok("enum { x, y, z } e{x};", false));
        ASSERT_EQUALS("struct Anonymous0 { int i ; } ; struct Anonymous0 f ; f = { 0 } ;", tok("struct { int i; } f{0};", false));
        ASSERT_EQUALS("struct Anonymous0 { } ; struct Anonymous0 x ; x = { 0 } ;", tok("struct {} x = {0};", false));
        ASSERT_EQUALS("enum G : short { x , y , z } ; enum G g ; g = x ;", tok("enum G : short { x, y, z } g(x);", false));
        ASSERT_EQUALS("enum H : short { x , y , z } ; enum H h ; h = x ;", tok("enum H : short { x, y, z } h{x};", false));
        ASSERT_EQUALS("enum class I : short { x , y , z } ; enum I i ; i = x ;", tok("enum class I : short { x, y, z } i(x);", false));
        ASSERT_EQUALS("enum class J : short { x , y , z } ; enum J j ; j = x ;", tok("enum class J : short { x, y, z } j{x};", false));
    }

    void removeUnwantedKeywords() {
        ASSERT_EQUALS("int var ;", tok("register int var ;", true));
        ASSERT_EQUALS("short var ;", tok("register short int var ;", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("inline int foo ( ) { }", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("__inline int foo ( ) { }", true));
        ASSERT_EQUALS("int foo ( ) { }", tok("__forceinline int foo ( ) { }", true));
        ASSERT_EQUALS("const int foo ( ) { }", tok("constexpr int foo() { }", true));
        ASSERT_EQUALS("void f ( ) { int final [ 10 ] ; }", tok("void f() { int final[10]; }", true));
        ASSERT_EQUALS("int * p ;", tok("int * __restrict p;", "test.c"));
        ASSERT_EQUALS("int * * p ;", tok("int * __restrict__ * p;", "test.c"));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * __restrict__ a, float * __restrict__ b);", "test.c"));
        ASSERT_EQUALS("int * p ;", tok("int * restrict p;", "test.c"));
        ASSERT_EQUALS("int * * p ;", tok("int * restrict * p;", "test.c"));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * restrict a, float * restrict b);", "test.c"));
        ASSERT_EQUALS("void foo ( int restrict ) ;", tok("void foo(int restrict);"));
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
        ASSERT_EQUALS("int f ( ) ;", tok("int WINAPI f();", true, Settings::Win32A));
        ASSERT_EQUALS("int f ( ) ;", tok("int APIENTRY f();", true, Settings::Win32A));
        ASSERT_EQUALS("int f ( ) ;", tok("int CALLBACK f();", true, Settings::Win32A));

        // don't simplify Microsoft defines in unix code (#7554)
        ASSERT_EQUALS("enum E { CALLBACK } ;", tok("enum E { CALLBACK } ;", true, Settings::Unix32));
    }

    void simplifyFunctorCall() {
        ASSERT_EQUALS("IncrementFunctor ( ) ( a ) ;", tok("IncrementFunctor()(a);", true));
    }

    // #ticket #5339 (simplify function pointer after comma)
    void simplifyFunctionPointer() {
        ASSERT_EQUALS("f ( double x , double ( * y ) ( ) ) ;", tok("f (double x, double (*y) ());", true));
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
        {
            // Ticket #7916
            // Tokenization would include "int fact < 2 > ( ) { return 2 > ( ) ; }" and generate
            // a parse error (and use after free)
            const char code[] = "extern \"C\" void abort ();\n"
                                "template <int a> inline int fact2 ();\n"
                                "template <int a> inline int fact () {\n"
                                "  return a * fact2<a-1> ();\n"
                                "}\n"
                                "template <> inline int fact<1> () {\n"
                                "  return 1;\n"
                                "}\n"
                                "template <int a> inline int fact2 () {\n"
                                "  return a * fact<a-1>();\n"
                                "}\n"
                                "template <> inline int fact2<1> () {\n"
                                "  return 1;\n"
                                "}\n"
                                "int main() {\n"
                                "  fact2<3> ();\n"
                                "  fact2<2> ();\n"
                                "}";
            tok(code);
        }
    }

    void removeVoidFromFunction() {
        ASSERT_EQUALS("void foo ( ) ;", tok("void foo(void);"));
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
    }

    void simplifyCharAt() { // ticket #4481
        ASSERT_EQUALS("'h' ;", tok("\"hello\"[0] ;"));
        ASSERT_EQUALS("'\\n' ;", tok("\"\\n\"[0] ;"));
        ASSERT_EQUALS("'\\0' ;", tok("\"hello\"[5] ;"));
        ASSERT_EQUALS("'\\0' ;", tok("\"\"[0] ;"));
        ASSERT_EQUALS("'\\0' ;", tok("\"\\0\"[0] ;"));
        ASSERT_EQUALS("'\\n' ;", tok("\"hello\\nworld\"[5] ;"));
        ASSERT_EQUALS("'w' ;", tok("\"hello world\"[6] ;"));
        ASSERT_EQUALS("\"hello\" [ 7 ] ;", tok("\"hello\"[7] ;"));
        ASSERT_EQUALS("\"hello\" [ -1 ] ;", tok("\"hello\"[-1] ;"));
    }

    void test_4881() {
        const char code[] = "int evallex() {\n"
                            "  int c, t;\n"
                            "again:\n"
                            "   do {\n"
                            "      if ((c = macroid(c)) == EOF_CHAR || c == '\\n') {\n"
                            "      }\n"
                            "   } while ((t = type[c]) == LET && catenate());\n"
                            "}\n";
        ASSERT_EQUALS("int evallex ( ) { int c ; int t ; again : ; do { c = macroid ( c ) ; if ( c == EOF_CHAR || c == '\\n' ) { } t = type [ c ] ; } while ( t == LET && catenate ( ) ) ; }",
                      tok(code, true));
    }

    void simplifyOverride() { // ticket #5069
        const char code[] = "void fun() {\n"
                            "    unsigned char override[] = {0x01, 0x02};\n"
                            "    doSomething(override, sizeof(override));\n"
                            "}\n";
        ASSERT_EQUALS("void fun ( ) { char override [ 2 ] = { 0x01 , 0x02 } ; doSomething ( override , 2 ) ; }",
                      tok(code, true));
    }

    void simplifyNestedNamespace() {
        ASSERT_EQUALS("namespace A { namespace B { namespace C { int i ; } } }", tok("namespace A::B::C { int i; }"));
    }

    void simplifyNamespaceAliases() {
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;"));
        ASSERT_EQUALS("boost :: iostreams :: istream foo ( \"foo\" ) ;",
                      tok("namespace ios = boost::iostreams; ios::istream foo(\"foo\");"));
        ASSERT_EQUALS("boost :: iostreams :: istream foo ( \"foo\" ) ;",
                      tok("using namespace std; namespace ios = boost::iostreams; ios::istream foo(\"foo\");"));
        ASSERT_EQUALS(";",
                      tok("using namespace std; namespace ios = boost::iostreams;"));
        ASSERT_EQUALS("namespace NS { boost :: iostreams :: istream foo ( \"foo\" ) ; }",
                      tok("namespace NS { using namespace std; namespace ios = boost::iostreams; ios::istream foo(\"foo\"); }"));

        // duplicate namespace aliases
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;"));
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;"));
        ASSERT_EQUALS("namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; } } }",
                      tok("namespace A::B {"
                          "namespace AB = A::B;"
                          "void foo() {"
                          "    namespace AB = A::B;" // duplicate declaration
                          "    bar(AB::ab());"
                          "}"
                          "namespace AB = A::B;" //duplicate declaration
                          "}"));

        // redeclared nested namespace aliases
        TODO_ASSERT_EQUALS("namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; { baz ( A :: a ( ) ) ; } bar ( A :: B :: ab ( ) ) ; } } }",
                           "namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; { baz ( A :: B :: a ( ) ) ; } bar ( A :: B :: ab ( ) ) ; } } }",
                           tok("namespace A::B {"
                               "namespace AB = A::B;"
                               "void foo() {"
                               "    namespace AB = A::B;" // duplicate declaration
                               "    bar(AB::ab());"
                               "    {"
                               "         namespace AB = A;"
                               "         baz(AB::a());" // redeclaration OK
                               "    }"
                               "    bar(AB::ab());"
                               "}"
                               "namespace AB = A::B;" //duplicate declaration
                               "}"));
    }
};

REGISTER_TEST(TestSimplifyTokens)
