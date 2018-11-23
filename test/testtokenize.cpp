/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "config.h"
#include "platform.h"
#include "preprocessor.h" // usually tests here should not use preprocessor...
#include "settings.h"
#include "standards.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <list>
#include <set>
#include <sstream>
#include <string>

struct InternalError;

class TestTokenizer : public TestFixture {
public:
    TestTokenizer() : TestFixture("TestTokenizer") {
    }

private:
    Settings settings0;
    Settings settings1;
    Settings settings2;
    Settings settings_windows;

    void run() override {
        LOAD_LIB_2(settings_windows.library, "windows.cfg");

        TEST_CASE(tokenize1);
        TEST_CASE(tokenize2);
        TEST_CASE(tokenize3);
        TEST_CASE(tokenize4);
        TEST_CASE(tokenize5);
        TEST_CASE(tokenize6);   // array access. replace "*(p+1)" => "p[1]"
        TEST_CASE(tokenize7);
        TEST_CASE(tokenize8);
        TEST_CASE(tokenize9);
        TEST_CASE(tokenize11);
        TEST_CASE(tokenize13);  // bailout if the code contains "@" - that is not handled well.
        TEST_CASE(tokenize14);  // tokenize "0X10" => 16
        TEST_CASE(tokenizeHexWithSuffix);  // tokenize 0xFFFFFFul
        TEST_CASE(tokenize15);  // tokenize ".123"
        TEST_CASE(tokenize17);  // #2759
        TEST_CASE(tokenize18);  // tokenize "(X&&Y)" into "( X && Y )" instead of "( X & & Y )"
        TEST_CASE(tokenize19);  // #3006 (segmentation fault)
        TEST_CASE(tokenize21);  // tokenize 0x0E-7
        TEST_CASE(tokenize22);  // special marker $ from preprocessor
        TEST_CASE(tokenize25);  // #4239 (segmentation fault)
        TEST_CASE(tokenize26);  // #4245 (segmentation fault)
        TEST_CASE(tokenize27);  // #4525 (segmentation fault)
        TEST_CASE(tokenize31);  // #3503 (Wrong handling of member function taking function pointer as argument)
        TEST_CASE(tokenize32);  // #5884 (fsanitize=undefined: left shift of negative value -10000 in lib/templatesimplifier.cpp:852:46)
        TEST_CASE(tokenize33);  // #5780 Various crashes on valid template code
        TEST_CASE(tokenize34);  // #8031
        TEST_CASE(tokenize35);  // #8361
        TEST_CASE(tokenize36);  // #8436
        TEST_CASE(tokenize37);  // #8550

        TEST_CASE(validate);

        TEST_CASE(objectiveC); // Syntax error should be written for objective C/C++ code.

        TEST_CASE(syntax_case_default);

        TEST_CASE(foreach);     // #3690
        TEST_CASE(ifconstexpr);

        TEST_CASE(combineOperators);

        TEST_CASE(concatenateNegativeNumber);

        TEST_CASE(longtok);

        TEST_CASE(simplifyCasts1);
        TEST_CASE(simplifyCasts2);
        TEST_CASE(simplifyCasts3);
        TEST_CASE(simplifyCasts4);
        TEST_CASE(simplifyCasts5);
        TEST_CASE(simplifyCasts7);
        TEST_CASE(simplifyCasts8);
        TEST_CASE(simplifyCasts9);
        TEST_CASE(simplifyCasts10);
        TEST_CASE(simplifyCasts11);
        TEST_CASE(simplifyCasts12);
        TEST_CASE(simplifyCasts13);
        TEST_CASE(simplifyCasts14);
        TEST_CASE(simplifyCasts15); // #5996 - don't remove cast in 'a+static_cast<int>(b?60:0)'
        TEST_CASE(simplifyCasts16); // #6278
        TEST_CASE(simplifyCasts17); // #6110 - don't remove any parentheses in 'a(b)(c)'

        TEST_CASE(inlineasm);
        TEST_CASE(simplifyAsm2);  // #4725 (writing asm() around "^{}")

        TEST_CASE(ifAddBraces1);
        TEST_CASE(ifAddBraces2);
        TEST_CASE(ifAddBraces3);
        TEST_CASE(ifAddBraces4);
        TEST_CASE(ifAddBraces5);
        TEST_CASE(ifAddBraces7);
        TEST_CASE(ifAddBraces9);
        TEST_CASE(ifAddBraces10);
        TEST_CASE(ifAddBraces11);
        TEST_CASE(ifAddBraces12);
        TEST_CASE(ifAddBraces13);
        TEST_CASE(ifAddBraces15); // #2616 - unknown macro before if
        TEST_CASE(ifAddBraces16);
        TEST_CASE(ifAddBraces17); // '} else' should be in the same line
        TEST_CASE(ifAddBraces18); // #3424 - if if { } else else
        TEST_CASE(ifAddBraces19); // #3928 - if for if else
        TEST_CASE(ifAddBraces20); // #5012 - syntax error 'else }'
        TEST_CASE(ifAddBraces21); // #5332 - if (x) label: {} ..

        TEST_CASE(whileAddBraces);
        TEST_CASE(doWhileAddBraces);

        TEST_CASE(forAddBraces1);
        TEST_CASE(forAddBraces2); // #5088

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
        TEST_CASE(simplifyKnownVariables25);
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
        TEST_CASE(simplifyKnownVariables44);    // ticket #3117 - don't simplify static variables
        TEST_CASE(simplifyKnownVariables45);    // ticket #3281 - static constant variable not simplified
        TEST_CASE(simplifyKnownVariables46);    // ticket #3587 - >>
        TEST_CASE(simplifyKnownVariables47);    // ticket #3627 - >>
        TEST_CASE(simplifyKnownVariables48);    // ticket #3754 - wrong simplification in for loop header
        TEST_CASE(simplifyKnownVariables49);    // #3691 - continue in switch
        TEST_CASE(simplifyKnownVariables50);    // #4066 sprintf changes
        TEST_CASE(simplifyKnownVariables51);    // #4409 hang
        TEST_CASE(simplifyKnownVariables52);    // #4728 "= x %cop%"
        TEST_CASE(simplifyKnownVariables53);    // references
        TEST_CASE(simplifyKnownVariables54);    // #4913 'x' is not 0 after *--x=0;
        TEST_CASE(simplifyKnownVariables55);    // pointer alias
        TEST_CASE(simplifyKnownVariables56);    // ticket #5301 - >>
        TEST_CASE(simplifyKnownVariables57);    // ticket #4724
        TEST_CASE(simplifyKnownVariables58);    // ticket #5268
        TEST_CASE(simplifyKnownVariables59);    // skip for header
        TEST_CASE(simplifyKnownVariables60);    // #6829
        TEST_CASE(simplifyKnownVariables61);    // #7805
        TEST_CASE(simplifyKnownVariables62);    // #5666 - p=&str[0]
        TEST_CASE(simplifyKnownVariablesBailOutAssign1);
        TEST_CASE(simplifyKnownVariablesBailOutAssign2);
        TEST_CASE(simplifyKnownVariablesBailOutAssign3); // #4395 - nested assignments
        TEST_CASE(simplifyKnownVariablesBailOutFor1);
        TEST_CASE(simplifyKnownVariablesBailOutFor2);
        TEST_CASE(simplifyKnownVariablesBailOutFor3);
        TEST_CASE(simplifyKnownVariablesBailOutMemberFunction);
        TEST_CASE(simplifyKnownVariablesBailOutConditionalIncrement);
        TEST_CASE(simplifyKnownVariablesBailOutSwitchBreak); // ticket #2324
        TEST_CASE(simplifyKnownVariablesFloat);    // #2454 - float variable
        TEST_CASE(simplifyKnownVariablesClassMember);  // #2815 - value of class member may be changed by function call
        TEST_CASE(simplifyKnownVariablesFunctionCalls); // Function calls (don't assume pass by reference)
        TEST_CASE(simplifyKnownVariablesGlobalVars);
        TEST_CASE(simplifyKnownVariablesReturn);   // 3500 - return
        TEST_CASE(simplifyKnownVariablesPointerAliasFunctionCall); // #7440
        TEST_CASE(simplifyExternC);
        TEST_CASE(simplifyKeyword); // #5842 - remove C99 static keyword between []

        TEST_CASE(isZeroNumber);
        TEST_CASE(isOneNumber);
        TEST_CASE(isTwoNumber);

        TEST_CASE(simplifyFunctionParameters);
        TEST_CASE(simplifyFunctionParameters1); // #3721
        TEST_CASE(simplifyFunctionParameters2); // #4430
        TEST_CASE(simplifyFunctionParameters3); // #4436
        TEST_CASE(simplifyFunctionParametersMultiTemplate);
        TEST_CASE(simplifyFunctionParametersErrors);

        TEST_CASE(removeParentheses1);       // Ticket #61
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
        TEST_CASE(removeParentheses13);
        TEST_CASE(removeParentheses14);      // Ticket #3309
        TEST_CASE(removeParentheses15);      // Ticket #4142
        TEST_CASE(removeParentheses16);      // Ticket #4423 '*(x.y)='
        TEST_CASE(removeParentheses17);      // Don't remove parentheses in 'a ? b : (c>0 ? d : e);'
        TEST_CASE(removeParentheses18);      // 'float(*a)[2]' => 'float *a[2]'
        TEST_CASE(removeParentheses19);      // ((typeof(x) *)0)
        TEST_CASE(removeParentheses20);      // Ticket #5479: a<b<int>>(2);
        TEST_CASE(removeParentheses21);      // Don't "simplify" casts
        TEST_CASE(removeParentheses22);
        TEST_CASE(removeParentheses23);      // Ticket #6103 - Infinite loop upon valid input
        TEST_CASE(removeParentheses24);      // Ticket #7040

        TEST_CASE(tokenize_double);
        TEST_CASE(tokenize_strings);
        TEST_CASE(simplify_constants);
        TEST_CASE(simplify_constants2);
        TEST_CASE(simplify_constants3);
        TEST_CASE(simplify_constants4);
        TEST_CASE(simplify_constants5);
        TEST_CASE(simplify_constants6);     // Ticket #5625: Ternary operator as template parameter
        TEST_CASE(simplifyMulAndParens);    // Ticket #2784 + #3184

        TEST_CASE(simplifyStructDecl);

        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(vardecl4);
        TEST_CASE(vardecl5);  // #7048
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
        TEST_CASE(vardecl15);
        TEST_CASE(vardecl16);
        TEST_CASE(vardecl17);
        TEST_CASE(vardecl18);
        TEST_CASE(vardecl19);
        TEST_CASE(vardecl20);  // #3700 - register const int H = 0;
        TEST_CASE(vardecl21);  // #4042 - a::b const *p = 0;
        TEST_CASE(vardecl22);  // #4211 - segmentation fault
        TEST_CASE(vardecl23);  // #4276 - segmentation fault
        TEST_CASE(vardecl24);  // #4187 - variable declaration within lambda function
        TEST_CASE(vardecl25);  // #4799 - segmentation fault
        TEST_CASE(vardecl26);  // #5907 - incorrect handling of extern declarations
        TEST_CASE(vardecl27);  // #7850 - crash on valid C code
        TEST_CASE(vardecl_stl_1);
        TEST_CASE(vardecl_stl_2);
        TEST_CASE(vardecl_template_1);
        TEST_CASE(vardecl_template_2);
        TEST_CASE(vardecl_union);
        TEST_CASE(vardecl_par);     // #2743 - set links if variable type contains parentheses
        TEST_CASE(vardecl_par2);    // #3912 - set correct links
        TEST_CASE(vardecl_par3);    // #6556 - Fred x1(a), x2(b);
        TEST_CASE(vardecl_class_ref);
        TEST_CASE(volatile_variables);

        // unsigned i; => unsigned int i;
        TEST_CASE(unsigned1);
        TEST_CASE(unsigned2);
        TEST_CASE(unsigned3);   // template arguments

        TEST_CASE(simplifyStdType); // #4947, #4950, #4951

        TEST_CASE(createLinks);
        TEST_CASE(createLinks2);
        TEST_CASE(signed1);

        TEST_CASE(simplifyString);
        TEST_CASE(simplifyConst);
        TEST_CASE(switchCase);

        TEST_CASE(simplifyPointerToStandardType);
        TEST_CASE(functionpointer1);
        TEST_CASE(functionpointer2);
        TEST_CASE(functionpointer3);
        TEST_CASE(functionpointer4);
        TEST_CASE(functionpointer5);
        TEST_CASE(functionpointer6);
        TEST_CASE(functionpointer7);
        TEST_CASE(functionpointer8); // #7410 - throw
        TEST_CASE(functionpointer9); // #6113 - function call with function pointer

        TEST_CASE(removeRedundantAssignment);

        TEST_CASE(removedeclspec);
        TEST_CASE(removeattribute);
        TEST_CASE(functionAttributeBefore);
        TEST_CASE(functionAttributeAfter);

        TEST_CASE(cpp03template1);
        TEST_CASE(cpp0xtemplate1);
        TEST_CASE(cpp0xtemplate2);
        TEST_CASE(cpp0xtemplate3);
        TEST_CASE(cpp0xtemplate4); // Ticket #6181: Mishandled C++11 syntax
        TEST_CASE(cpp14template); // Ticket #6708

        TEST_CASE(arraySize);

        TEST_CASE(labels);
        TEST_CASE(simplifyInitVar);
        TEST_CASE(simplifyInitVar2);
        TEST_CASE(simplifyInitVar3);

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
        TEST_CASE(bitfields12); // ticket #3485 (segmentation fault)
        TEST_CASE(bitfields13); // ticket #3502 (segmentation fault)
        TEST_CASE(bitfields14); // ticket #4561 (segfault for 'class a { signals: };')
        TEST_CASE(bitfields15); // ticket #7747 (enum Foo {A,B}:4;)
        TEST_CASE(bitfields16); // Save bitfield bit count

        TEST_CASE(simplifyNamespaceStd);

        TEST_CASE(microsoftMemory);
        TEST_CASE(microsoftString);

        TEST_CASE(borland);

        TEST_CASE(Qt);

        TEST_CASE(simplifySQL);

        TEST_CASE(simplifyCAlternativeTokens);

        TEST_CASE(simplifyCalculations);

        // x = ({ 123; });  =>  { x = 123; }
        TEST_CASE(simplifyRoundCurlyParentheses);

        TEST_CASE(simplifyOperatorName1);
        TEST_CASE(simplifyOperatorName2);
        TEST_CASE(simplifyOperatorName3);
        TEST_CASE(simplifyOperatorName4);
        TEST_CASE(simplifyOperatorName5);
        TEST_CASE(simplifyOperatorName6); // ticket #3194
        TEST_CASE(simplifyOperatorName7); // ticket #4619
        TEST_CASE(simplifyOperatorName8); // ticket #5706
        TEST_CASE(simplifyOperatorName9); // ticket #5709 - comma operator not properly tokenized
        TEST_CASE(simplifyOperatorName10); // #8746 - using a::operator=

        TEST_CASE(simplifyNullArray);

        // Some simple cleanups of unhandled macros in the global scope
        TEST_CASE(removeMacrosInGlobalScope);
        TEST_CASE(removeMacroInVarDecl);

        // a = b = 0;
        TEST_CASE(multipleAssignment);

        TEST_CASE(sizeOfCharLiteral);
        TEST_CASE(platformWin);
        TEST_CASE(platformWin32);
        TEST_CASE(platformWin32A);
        TEST_CASE(platformWin32W);
        TEST_CASE(platformWin64);
        TEST_CASE(platformUnix32);
        TEST_CASE(platformUnix64);
        TEST_CASE(platformWin32AStringCat); // ticket #5015
        TEST_CASE(platformWin32WStringCat); // ticket #5015
        TEST_CASE(platformWinWithNamespace);

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
        TEST_CASE(simplifyStaticConst);

        TEST_CASE(simplifyCPPAttribute);

        TEST_CASE(simplifyCaseRange);

        TEST_CASE(compileLimits); // #5592 crash: gcc: testsuit: gcc.c-torture/compile/limits-declparen.c

        TEST_CASE(prepareTernaryOpForAST);

        // AST data
        TEST_CASE(astexpr);
        TEST_CASE(astexpr2); // limit large expressions
        TEST_CASE(astpar);
        TEST_CASE(astnewdelete);
        TEST_CASE(astbrackets);
        TEST_CASE(astunaryop);
        TEST_CASE(astfunction);
        TEST_CASE(asttemplate);
        TEST_CASE(astcast);
        TEST_CASE(astlambda);
        TEST_CASE(astcase);

        TEST_CASE(startOfExecutableScope);

        TEST_CASE(removeMacroInClassDef); // #6058

        TEST_CASE(sizeofAddParentheses);

        // Make sure the Tokenizer::findGarbageCode() does not have false positives
        // The TestGarbage ensures that there are true positives
        TEST_CASE(findGarbageCode);
        TEST_CASE(checkEnableIf);

        // --check-config
        TEST_CASE(checkConfiguration);
    }

    std::string tokenizeAndStringify(const char code[], bool simplify = false, bool expand = true, Settings::PlatformType platform = Settings::Native, const char* filename = "test.cpp", bool cpp11 = true) {
        errout.str("");

        settings1.debugwarnings = true;
        settings1.platform(platform);
        settings1.standards.cpp = cpp11 ? Standards::CPP11 : Standards::CPP03;

        // tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        if (simplify)
            tokenizer.simplifyTokenList2();

        // filter out ValueFlow messages..
        const std::string debugwarnings = errout.str();
        errout.str("");
        std::istringstream istr2(debugwarnings.c_str());
        std::string line;
        while (std::getline(istr2,line)) {
            if (line.find("valueflow.cpp") == std::string::npos)
                errout << line << "\n";
        }

        if (tokenizer.tokens())
            return tokenizer.tokens()->stringifyList(false, expand, false, true, false, 0, 0);
        else
            return "";
    }

    std::string tokenizeAndStringifyWindows(const char code[], bool simplify = false, bool expand = true, Settings::PlatformType platform = Settings::Native, const char* filename = "test.cpp", bool cpp11 = true) {
        errout.str("");

        settings_windows.debugwarnings = true;
        settings_windows.platform(platform);
        settings_windows.standards.cpp = cpp11 ? Standards::CPP11 : Standards::CPP03;

        // tokenize..
        Tokenizer tokenizer(&settings_windows, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        if (simplify)
            tokenizer.simplifyTokenList2();

        // filter out ValueFlow messages..
        const std::string debugwarnings = errout.str();
        errout.str("");
        std::istringstream istr2(debugwarnings.c_str());
        std::string line;
        while (std::getline(istr2,line)) {
            if (line.find("valueflow.cpp") == std::string::npos)
                errout << line << "\n";
        }

        if (tokenizer.tokens())
            return tokenizer.tokens()->stringifyList(false, expand, false, true, false, 0, 0);
        else
            return "";
    }

    std::string tokenizeDebugListing(const char code[], bool simplify = false, const char filename[] = "test.cpp") {
        errout.str("");

        settings2.standards.c = Standards::C89;
        settings2.standards.cpp = Standards::CPP03;

        Tokenizer tokenizer(&settings2, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        if (simplify)
            tokenizer.simplifyTokenList2();

        // result..
        return tokenizer.tokens()->stringifyList(true,true,true,true,false);
    }

    void tokenize1() {
        const char code[] = "void f ( )\n"
                            "{ if ( p . y ( ) > yof ) { } }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void tokenize2() {
        const char code[] = "{ sizeof a, sizeof b }";
        ASSERT_EQUALS("{ sizeof ( a ) , sizeof ( b ) }", tokenizeAndStringify(code));
    }

    void tokenize3() {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i;\n"
                            "    ABC(for(i=0;i<10;i++) x());\n"
                            "}";
        ASSERT_EQUALS("void foo ( )\n"
                      "{\n"
                      "int i ;\n"
                      "ABC ( for ( i = 0 ; i < 10 ; i ++ ) x ( ) ) ;\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout.str());
    }

    void tokenize4() {
        const char code[] = "class foo\n"
                            "{\n"
                            "public:\n"
                            "    const int i;\n"
                            "}";
        ASSERT_EQUALS("class foo\n"
                      "{\n"
                      "public:\n"
                      "const int i ;\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout.str());
    }

    void tokenize5() {
        // Tokenize values
        ASSERT_EQUALS("; + 1E3 ;", tokenizeAndStringify("; +1E3 ;"));
        ASSERT_EQUALS("; 1E-2 ;", tokenizeAndStringify("; 1E-2 ;"));
    }

    void tokenize6() {
        // "&p[1]" => "p+1"
        /*
        ASSERT_EQUALS("; x = p + n ;", tokenizeAndStringify("; x = & p [ n ] ;", true));
        ASSERT_EQUALS("; x = ( p + n ) [ m ] ;", tokenizeAndStringify("; x = & p [ n ] [ m ] ;", true));
        ASSERT_EQUALS("; x = y & p [ n ] ;", tokenizeAndStringify("; x = y & p [ n ] ;", true));
        ASSERT_EQUALS("; x = 10 & p [ n ] ;", tokenizeAndStringify(";  x = 10 & p [ n ] ;", true));
        ASSERT_EQUALS("; x = y [ 10 ] & p [ n ] ;", tokenizeAndStringify("; x = y [ 10 ] & p [ n ] ;", true));
        ASSERT_EQUALS("; x = ( a + m ) & p [ n ] ;", tokenizeAndStringify("; x = ( a + m ) & p [ n ] ;", true));*/
        // "*(p+1)" => "p[1]"
        ASSERT_EQUALS("; x = p [ 1 ] ;", tokenizeAndStringify("; x = * ( p + 1 ) ;", true));
        ASSERT_EQUALS("; x = p [ 10 ] ;", tokenizeAndStringify("; x = * ( p + 0xA ) ;", true));
        ASSERT_EQUALS("; x = p [ n ] ;", tokenizeAndStringify("; x = * ( p + n ) ;", true));
        ASSERT_EQUALS("; x = y * ( p + n ) ;", tokenizeAndStringify("; x = y * ( p + n ) ;", true));
        ASSERT_EQUALS("; x = 10 * ( p + n ) ;", tokenizeAndStringify("; x = 10 * ( p + n ) ;", true));
        ASSERT_EQUALS("; x = y [ 10 ] * ( p + n ) ;", tokenizeAndStringify("; x = y [ 10 ] * ( p + n ) ;", true));
        ASSERT_EQUALS("; x = ( a + m ) * ( p + n ) ;", tokenizeAndStringify("; x = ( a + m ) * ( p + n ) ;", true));

        // "*(p-1)" => "p[-1]" and "*(p-n)" => "p[-n]"
        ASSERT_EQUALS("; x = p [ -1 ] ;", tokenizeAndStringify("; x = *(p - 1);", true));
        ASSERT_EQUALS("; x = p [ -10 ] ;", tokenizeAndStringify("; x = *(p - 0xA);", true));
        ASSERT_EQUALS("; x = p [ - n ] ;", tokenizeAndStringify("; x = *(p - n);", true));
        ASSERT_EQUALS("; x = y * ( p - 1 ) ;", tokenizeAndStringify("; x = y * (p - 1);", true));
        ASSERT_EQUALS("; x = 10 * ( p - 1 ) ;", tokenizeAndStringify("; x = 10 * (p - 1);", true));
        ASSERT_EQUALS("; x = y [ 10 ] * ( p - 1 ) ;", tokenizeAndStringify("; x = y[10] * (p - 1);", true));
        ASSERT_EQUALS("; x = ( a - m ) * ( p - n ) ;", tokenizeAndStringify("; x = (a - m) * (p - n);", true));

        // Test that the array-index simplification is not applied when there's no dereference:
        // "(x-y)" => "(x-y)" and "(x+y)" => "(x+y)"
        ASSERT_EQUALS("; a = b * ( x - y ) ;", tokenizeAndStringify("; a = b * (x - y);", true));
        ASSERT_EQUALS("; a = b * x [ - y ] ;", tokenizeAndStringify("; a = b * *(x - y);", true));
        ASSERT_EQUALS("; a = a * ( x - y ) ;", tokenizeAndStringify("; a *= (x - y);", true));
        ASSERT_EQUALS("; z = a ++ * ( x - y ) ;", tokenizeAndStringify("; z = a++ * (x - y);", true));
        ASSERT_EQUALS("; z = a ++ * ( x + y ) ;", tokenizeAndStringify("; z = a++ * (x + y);", true));
        ASSERT_EQUALS("; z = a -- * ( x - y ) ;", tokenizeAndStringify("; z = a-- * (x - y);", true));
        ASSERT_EQUALS("; z = a -- * ( x + y ) ;", tokenizeAndStringify("; z = a-- * (x + y);", true));
        ASSERT_EQUALS("; z = 'a' * ( x - y ) ;", tokenizeAndStringify("; z = 'a' * (x - y);", true));
        ASSERT_EQUALS("; z = \"a\" * ( x - y ) ;", tokenizeAndStringify("; z = \"a\" * (x - y);", true));
        ASSERT_EQUALS("; z = 'a' * ( x + y ) ;", tokenizeAndStringify("; z = 'a' * (x + y);", true));
        ASSERT_EQUALS("; z = \"a\" * ( x + y ) ;", tokenizeAndStringify("; z = \"a\" * (x + y);", true));
        ASSERT_EQUALS("; z = foo ( ) * ( x + y ) ;", tokenizeAndStringify("; z = foo() * (x + y);", true));
    }

    void tokenize7() {
        const char code[] = "void f() {\n"
                            "    int x1 = 1;\n"
                            "    int x2(x1);\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) {\nint x1 ; x1 = 1 ;\nint x2 ; x2 = x1 ;\n}",
                      tokenizeAndStringify(code, false));
    }

    void tokenize8() {
        const char code[] = "void f() {\n"
                            "    int x1(g());\n"
                            "    int x2(x1);\n"
                            "}\n";
        ASSERT_EQUALS("1: void f ( ) {\n"
                      "2: int x1@1 ; x1@1 = g ( ) ;\n"
                      "3: int x2@2 ; x2@2 = x1@1 ;\n"
                      "4: }\n",
                      tokenizeDebugListing(code, false));
    }

    void tokenize9() {
        const char code[] = "typedef void (*fp)();\n"
                            "typedef fp (*fpp)();\n"
                            "void f() {\n"
                            "    fpp x = (fpp)f();\n"
                            "}";
        tokenizeAndStringify(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void tokenize11() {
        ASSERT_EQUALS("X * sizeof ( Y ( ) ) ;", tokenizeAndStringify("X * sizeof(Y());", false));
    }

    // bailout if there is "@" - it is not handled well
    void tokenize13() {
        const char code[] = "@implementation\n"
                            "-(Foo *)foo: (Bar *)bar\n"
                            "{ }\n"
                            "@end\n";
        ASSERT_THROW(tokenizeAndStringify(code), InternalError);
    }

    // Ticket #2361: 0X10 => 16
    void tokenize14() {
        ASSERT_EQUALS("; 16 ;", tokenizeAndStringify(";0x10;"));
        ASSERT_EQUALS("; 16 ;", tokenizeAndStringify(";0X10;"));
        ASSERT_EQUALS("; 292 ;", tokenizeAndStringify(";0444;"));
    }

    // Ticket #8050
    void tokenizeHexWithSuffix() {
        ASSERT_EQUALS("; 16777215 ;", tokenizeAndStringify(";0xFFFFFF;"));
        ASSERT_EQUALS("; 16777215U ;", tokenizeAndStringify(";0xFFFFFFu;"));
        ASSERT_EQUALS("; 16777215UL ;", tokenizeAndStringify(";0xFFFFFFul;"));

        // Number of digits decides about internal representation...
        ASSERT_EQUALS("; 4294967295U ;", tokenizeAndStringify(";0xFFFFFFFF;"));
        ASSERT_EQUALS("; 4294967295U ;", tokenizeAndStringify(";0xFFFFFFFFu;"));
        ASSERT_EQUALS("; 4294967295UL ;", tokenizeAndStringify(";0xFFFFFFFFul;"));
    }

    // Ticket #2429: 0.125
    void tokenize15() {
        ASSERT_EQUALS("0.125 ;", tokenizeAndStringify(".125;"));
        ASSERT_EQUALS("005.125 ;", tokenizeAndStringify("005.125;")); // Don't confuse with octal values
    }

    void tokenize17() { // #2759
        ASSERT_EQUALS("class B : private :: A { } ;", tokenizeAndStringify("class B : private ::A { };"));
    }

    void tokenize18() { // tokenize "(X&&Y)" into "( X && Y )" instead of "( X & & Y )"
        ASSERT_EQUALS("( X && Y ) ;", tokenizeAndStringify("(X&&Y);"));
    }

    void tokenize19() {
        // #3006 - added hasComplicatedSyntaxErrorsInTemplates to avoid segmentation fault
        ASSERT_THROW(tokenizeAndStringify("x < () <"), InternalError);

        // #3496 - make sure hasComplicatedSyntaxErrorsInTemplates works
        ASSERT_EQUALS("void a ( Fred * f ) { for ( ; n < f . x ( ) ; ) { } }",
                      tokenizeAndStringify("void a(Fred* f) MACRO { for (;n < f->x();) {} }"));

        // #6216 - make sure hasComplicatedSyntaxErrorsInTemplates works
        ASSERT_EQUALS("C :: C ( )\n"
                      ": v { }\n"
                      "{\n"
                      "for ( int dim = 0 ; dim < v . size ( ) ; ++ dim ) {\n"
                      "v [ dim ] . f ( ) ;\n"
                      "}\n"
                      "} ;",
                      tokenizeAndStringify("C::C()\n"
                                           ":v{}\n"
                                           "{\n"
                                           "    for (int dim = 0; dim < v.size(); ++dim) {\n"
                                           "        v[dim]->f();\n"
                                           "    }\n"
                                           "};"));
    }

    void tokenize21() { // tokenize 0x0E-7
        ASSERT_EQUALS("14 - 7 ;", tokenizeAndStringify("0x0E-7;"));
    }

    void tokenize22() { // tokenize special marker $ from preprocessor
        ASSERT_EQUALS("a$b", tokenizeAndStringify("a$b"));
        ASSERT_EQUALS("a $b\nc", tokenizeAndStringify("a $b\nc"));
        ASSERT_EQUALS("a = $0 ;", tokenizeAndStringify("a = $0;"));
        ASSERT_EQUALS("a$ ++ ;", tokenizeAndStringify("a$++;"));
        ASSERT_EQUALS("$if ( ! p )", tokenizeAndStringify("$if(!p)"));
    }

    // #4239 - segfault for "f ( struct { int typedef T x ; } ) { }"
    void tokenize25() {
        ASSERT_THROW(tokenizeAndStringify("f ( struct { int typedef T x ; } ) { }"), InternalError);
    }

    // #4245 - segfault
    void tokenize26() {
        ASSERT_THROW(tokenizeAndStringify("class x { protected : template < int y = } ;"), InternalError); // Garbage code
    }

    void tokenize27() {
        // #4525 - segfault
        tokenizeAndStringify("struct except_spec_d_good : except_spec_a, except_spec_b {\n"
                             "~except_spec_d_good();\n"
                             "};\n"
                             "struct S { S(); };\n"
                             "S::S() __attribute((pure)) = default;"
                            );

        // original code: glibc-2.18/posix/bug-regex20.c
        tokenizeAndStringify("static unsigned int re_string_context_at (const re_string_t *input, int idx, int eflags) internal_function __attribute__ ((pure));");
    }

    // #3503 - don't "simplify" SetFunction member function to a variable
    void tokenize31() {
        ASSERT_EQUALS("struct TTestClass { TTestClass ( ) { }\n"
                      "void SetFunction ( Other * m_f ) { }\n"
                      "} ;",
                      tokenizeAndStringify("struct TTestClass { TTestClass() { }\n"
                                           "    void SetFunction(Other(*m_f)()) { }\n"
                                           "};"));

        ASSERT_EQUALS("struct TTestClass { TTestClass ( ) { }\n"
                      "void SetFunction ( Other * m_f ) ;\n"
                      "} ;",
                      tokenizeAndStringify("struct TTestClass { TTestClass() { }\n"
                                           "    void SetFunction(Other(*m_f)());\n"
                                           "};"));
    }

    // #5884 - Avoid left shift of negative integer value.
    void tokenize32() {
        // Do not simplify negative integer left shifts.
        const char * code = "void f ( ) { int max_x ; max_x = -10000 << 16 ; }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    // #5780 Various crashes on valid template code in Tokenizer::setVarId()
    void tokenize33() {
        const char * code = "template<typename T, typename A = Alloc<T>> struct vector {};\n"
                            "void z() {\n"
                            "    vector<int> VI;\n"
                            "}\n";
        tokenizeAndStringify(code, true);
    }

    void tokenize34() { // #8031
        {
            const char code[] = "struct Containter {\n"
                                "  Containter();\n"
                                "  int* mElements;\n"
                                "};\n"
                                "Containter::Containter() : mElements(nullptr) {}\n"
                                "Containter intContainer;";
            const char exp [] = "1: struct Containter {\n"
                                "2: Containter ( ) ;\n"
                                "3: int * mElements@1 ;\n"
                                "4: } ;\n"
                                "5: Containter :: Containter ( ) : mElements@1 ( nullptr ) { }\n"
                                "6: Containter intContainer@2 ;\n";
            ASSERT_EQUALS(exp, tokenizeDebugListing(code, /*simplify=*/true));
        }
        {
            const char code[] = "template<class T> struct Containter {\n"
                                "  Containter();\n"
                                "  int* mElements;\n"
                                "};\n"
                                "template <class T> Containter<T>::Containter() : mElements(nullptr) {}\n"
                                "Containter<int> intContainer;";
            const char exp [] = "1: struct Containter<int> ;\n"
                                "2:\n"
                                "|\n"
                                "5:\n"
                                "6: Containter<int> intContainer@1 ; struct Containter<int> {\n"
                                "2: Containter<int> ( ) ;\n"
                                "3: int * mElements@2 ;\n"
                                "4: } ;\n"
                                "5: Containter<int> :: Containter<int> ( ) : mElements@2 ( nullptr ) { }\n";
            ASSERT_EQUALS(exp, tokenizeDebugListing(code, /*simplify=*/true));
        }
    }

    void tokenize35() { // #8361
        tokenizeAndStringify("typedef int CRCWord; "
                             "template<typename T> ::CRCWord const Compute(T const t) { return 0; }");
    }

    void tokenize36() { // #8436
        const char code[] = "int foo ( int i ) { return i ? * new int { 5 } : int { i ? 0 : 1 } ; }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void tokenize37() { // #8550
        const char codeC[] = "class name { public: static void init ( ) {} } ; "
                             "typedef class name N; "
                             "void foo ( ) { return N :: init ( ) ; }";
        const char expC [] = "class name { public: static void init ( ) { } } ; "
                             "void foo ( ) { return name :: init ( ) ; }";
        ASSERT_EQUALS(expC, tokenizeAndStringify(codeC));
        const char codeS[] = "class name { public: static void init ( ) {} } ; "
                             "typedef struct name N; "
                             "void foo ( ) { return N :: init ( ) ; }";
        const char expS [] = "class name { public: static void init ( ) { } } ; "
                             "void foo ( ) { return name :: init ( ) ; }";
        ASSERT_EQUALS(expS, tokenizeAndStringify(codeS));
    }

    void validate() {
        // C++ code in C file
        ASSERT_THROW(tokenizeAndStringify(";using namespace std;",false,false,Settings::Native,"test.c"), InternalError);
        ASSERT_THROW(tokenizeAndStringify(";std::map<int,int> m;",false,false,Settings::Native,"test.c"), InternalError);
        ASSERT_THROW(tokenizeAndStringify(";template<class T> class X { };",false,false,Settings::Native,"test.c"), InternalError);
        ASSERT_THROW(tokenizeAndStringify("int X<Y>() {};",false,false,Settings::Native,"test.c"), InternalError);
        ASSERT_THROW(tokenizeAndStringify("void foo(int i) { reinterpret_cast<char>(i) };",false,false,Settings::Native,"test.h"), InternalError);
    }

    void objectiveC() {
        ASSERT_THROW(tokenizeAndStringify("void f() { [foo bar]; }"), InternalError);
    }

    void syntax_case_default() { // correct syntax
        tokenizeAndStringify("void f() {switch (n) { case 0: z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        tokenizeAndStringify("void f() {switch (n) { case 0:; break;}}");
        ASSERT_EQUALS("", errout.str());

        tokenizeAndStringify("void f() {switch (n) { case 0?1:2 : z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        tokenizeAndStringify("void f() {switch (n) { case 0?(1?3:4):2 : z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        //allow GCC '({ %name%|%num%|%bool% ; })' statement expression extension
        tokenizeAndStringify("void f() {switch (n) { case 0?({0;}):1: z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        //'b' can be or a macro or an undefined enum
        tokenizeAndStringify("void f() {switch (n) { case b: z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        //valid, when there's this declaration: 'constexpr int g() { return 2; }'
        tokenizeAndStringify("void f() {switch (n) { case g(): z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        //valid, when there's also this declaration: 'constexpr int g[1] = {0};'
        tokenizeAndStringify("void f() {switch (n) { case g[0]: z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        //valid, similar to above case
        tokenizeAndStringify("void f() {switch (n) { case *g: z(); break;}}");
        ASSERT_EQUALS("", errout.str());

        //valid, when 'x' and 'y' are constexpr.
        tokenizeAndStringify("void f() {switch (n) { case sqrt(x+y): z(); break;}}");
        ASSERT_EQUALS("", errout.str());
    }

    void foreach () {
        // #3690,#5154
        const char code[] ="void f() { for each ( char c in MyString ) { Console::Write(c); } }";
        ASSERT_EQUALS("void f ( ) { asm ( \"char c in MyString\" ) { Console :: Write ( c ) ; } }", tokenizeAndStringify(code));
    }

    void ifconstexpr() {
        ASSERT_EQUALS("void f ( ) { if ( FOO ) { bar ( c ) ; } }", tokenizeAndStringify("void f() { if constexpr ( FOO ) { bar(c); } }"));
    }

    void combineOperators() {
        ASSERT_EQUALS("; private: ;", tokenizeAndStringify(";private:;", false));
        ASSERT_EQUALS("; protected: ;", tokenizeAndStringify(";protected:;", false));
        ASSERT_EQUALS("; public: ;", tokenizeAndStringify(";public:;", false));
        ASSERT_EQUALS("; __published: ;", tokenizeAndStringify(";__published:;", false));
        ASSERT_EQUALS("a . public : ;", tokenizeAndStringify("a.public:;", false));
        ASSERT_EQUALS("void f ( x & = 2 ) ;", tokenizeAndStringify("void f(x &= 2);", false));
    }

    void concatenateNegativeNumber() {
        ASSERT_EQUALS("i = -12 ;", tokenizeAndStringify("i = -12;"));
        ASSERT_EQUALS("1 - 2 ;", tokenizeAndStringify("1-2;"));
        ASSERT_EQUALS("foo ( -1 ) - 2 ;", tokenizeAndStringify("foo(-1)-2;"));
        ASSERT_EQUALS("int f ( ) { return -2 ; }", tokenizeAndStringify("int f(){return -2;}"));
        ASSERT_EQUALS("int x [ 2 ] = { -2 , 1 }", tokenizeAndStringify("int x[2] = {-2,1}"));

        ASSERT_EQUALS("f ( 123 )", tokenizeAndStringify("f(+123)"));
    }



    void longtok() {
        const std::string filedata(10000, 'a');
        ASSERT_EQUALS(filedata, tokenizeAndStringify(filedata.c_str(), true));
    }



    // Don’t remove "(int *)"..
    void simplifyCasts1() {
        const char code[] = "int *f(int *);";
        ASSERT_EQUALS("int * f ( int * ) ;", tokenizeAndStringify(code, true));
    }

    // remove static_cast..
    void simplifyCasts2() {
        const char code[] = "t = (static_cast<std::vector<int> *>(&p));\n";
        ASSERT_EQUALS("t = & p ;", tokenizeAndStringify(code, true));
    }

    void simplifyCasts3() {
        // ticket #961
        const char code[] = "assert (iplen >= (unsigned) ipv4->ip_hl * 4 + 20);";
        const char expected[] = "assert ( iplen >= ipv4 . ip_hl * 4 + 20 ) ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyCasts4() {
        // ticket #970
        const char code[] = "if (a >= (unsigned)(b)) {}";
        const char expected[] = "if ( a >= ( unsigned int ) ( b ) ) { }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyCasts5() {
        // ticket #1817
        ASSERT_EQUALS("a . data = f ;", tokenizeAndStringify("a->data = reinterpret_cast<void*>(static_cast<intptr_t>(f));", true));
    }

    void simplifyCasts7() {
        ASSERT_EQUALS("str = malloc ( 3 )", tokenizeAndStringify("str=(char **)malloc(3)", true));
    }

    void simplifyCasts8() {
        ASSERT_EQUALS("ptr1 = ptr2", tokenizeAndStringify("ptr1=(int *   **)ptr2", true));
    }

    void simplifyCasts9() {
        ASSERT_EQUALS("f ( ( double ) ( v1 ) * v2 )", tokenizeAndStringify("f((double)(v1)*v2)", true));
        ASSERT_EQUALS("int v1 ; f ( ( double ) ( v1 ) * v2 )", tokenizeAndStringify("int v1; f((double)(v1)*v2)", true));
        ASSERT_EQUALS("f ( ( A ) ( B ) & x )", tokenizeAndStringify("f((A)(B)&x)", true)); // #4439
    }

    void simplifyCasts10() {
        ASSERT_EQUALS("; ( * f ) ( p ) ;", tokenizeAndStringify("; (*(void (*)(char *))f)(p);", true));
    }

    void simplifyCasts11() {
        ASSERT_EQUALS("; x = 0 ;", tokenizeAndStringify("; *(int *)&x = 0;", true));
    }

    void simplifyCasts12() {
        // #3935 - don't remove this cast
        ASSERT_EQUALS("; ( ( short * ) data ) [ 5 ] = 0 ;", tokenizeAndStringify("; ((short*)data)[5] = 0;", true));
    }

    void simplifyCasts13() {
        // casting deref / address of
        ASSERT_EQUALS("; int x ; x = * y ;", tokenizeAndStringify(";int x=(int)*y;",true));
        ASSERT_EQUALS("; int x ; x = & y ;", tokenizeAndStringify(";int x=(int)&y;",true));
        TODO_ASSERT_EQUALS("; int x ; x = ( INT ) * y ;",
                           "; int x ; x = * y ;",
                           tokenizeAndStringify(";int x=(INT)*y;",true)); // INT might be a variable
        TODO_ASSERT_EQUALS("; int x ; x = ( INT ) & y ;",
                           "; int x ; x = & y ;",
                           tokenizeAndStringify(";int x=(INT)&y;",true)); // INT might be a variable

        // #4899 - False positive on unused variable
        ASSERT_EQUALS("; float angle ; angle = tilt ;", tokenizeAndStringify("; float angle = (float) tilt;", true)); // status quo
        ASSERT_EQUALS("; float angle ; angle = ( float ) - tilt ;", tokenizeAndStringify("; float angle = (float) -tilt;", true));
        ASSERT_EQUALS("; float angle ; angle = ( float ) + tilt ;", tokenizeAndStringify("; float angle = (float) +tilt;", true));
        ASSERT_EQUALS("; int a ; a = ( int ) ~ c ;", tokenizeAndStringify("; int a = (int)~c;", true));
    }

    void simplifyCasts14() { // const
        // #5081
        ASSERT_EQUALS("( ! ( & s ) . a ) ;", tokenizeAndStringify("(! ( (struct S const *) &s)->a);", true));
        // #5244
        ASSERT_EQUALS("bar ( & ptr ) ;", tokenizeAndStringify("bar((const X**)&ptr);",true));
    }

    void simplifyCasts15() { // #5996 - don't remove cast in 'a+static_cast<int>(b?60:0)'
        ASSERT_EQUALS("a + ( b ? 60 : 0 ) ;",
                      tokenizeAndStringify("a + static_cast<int>(b ? 60 : 0);", true));
    }

    void simplifyCasts16() { // #6278
        ASSERT_EQUALS("Get ( pArray ) ;",
                      tokenizeAndStringify("Get((CObject*&)pArray);", true));
    }

    void simplifyCasts17() { // #6110 - don't remove any parentheses in 'a(b)(c)'
        ASSERT_EQUALS("if ( a ( b ) ( c ) >= 3 )",
                      tokenizeAndStringify("if (a(b)(c) >= 3)", true));
    }

    void inlineasm() {
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("asm { mov ax,bx };"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("_asm { mov ax,bx };"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("_asm mov ax,bx"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("__asm { mov ax,bx };"));
        ASSERT_EQUALS("asm ( \"\"mov ax,bx\"\" ) ;", tokenizeAndStringify("__asm__ __volatile__ ( \"mov ax,bx\" );"));
        ASSERT_EQUALS("asm ( \"_emit 12h\" ) ;", tokenizeAndStringify("__asm _emit 12h ;"));
        ASSERT_EQUALS("asm ( \"mov a , b\" ) ;", tokenizeAndStringify("__asm mov a, b ;"));
        ASSERT_EQUALS("asm ( \"\"fnstcw %0\" : \"= m\" ( old_cw )\" ) ;", tokenizeAndStringify("asm volatile (\"fnstcw %0\" : \"= m\" (old_cw));"));
        ASSERT_EQUALS("asm ( \"\"fnstcw %0\" : \"= m\" ( old_cw )\" ) ;", tokenizeAndStringify(" __asm__ (\"fnstcw %0\" : \"= m\" (old_cw));"));
        ASSERT_EQUALS("asm ( \"\"ddd\"\" ) ;", tokenizeAndStringify(" __asm __volatile__ (\"ddd\") ;"));
        ASSERT_EQUALS("asm ( \"\"ddd\"\" ) ;", tokenizeAndStringify(" __asm __volatile (\"ddd\") ;"));
        ASSERT_EQUALS("asm ( \"\"mov ax,bx\"\" ) ;", tokenizeAndStringify("__asm__ volatile ( \"mov ax,bx\" );"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ; int a ;", tokenizeAndStringify("asm { mov ax,bx } int a;"));
        ASSERT_EQUALS("asm\n\n( \"mov ax , bx\" ) ;", tokenizeAndStringify("__asm\nmov ax,bx\n__endasm;"));
        ASSERT_EQUALS("asm\n\n( \"push b ; for if\" ) ;", tokenizeAndStringify("__asm\npush b ; for if\n__endasm;"));

        // 'asm ( ) ;' should be in the same line
        ASSERT_EQUALS(";\n\nasm ( \"\"mov ax,bx\"\" ) ;", tokenizeAndStringify(";\n\n__asm__ volatile ( \"mov ax,bx\" );", true));
    }

    // #4725 - ^{}
    void simplifyAsm2() {
        ASSERT_EQUALS("void f ( ) { asm ( \"^{}\" ) ; }", tokenizeAndStringify("void f() { ^{} }"));
        ASSERT_EQUALS("void f ( ) { x ( asm ( \"^{}\" ) ) ; }", tokenizeAndStringify("void f() { x(^{}); }"));
        ASSERT_EQUALS("void f ( ) { foo ( A ( ) , asm ( \"^{bar();}\" ) ) ; }", tokenizeAndStringify("void f() { foo(A(), ^{ bar(); }); }"));
        ASSERT_EQUALS("int f0 ( Args args ) { asm ( \"asm(\"return^{returnsizeof...(Args);}()\")+^{returnsizeof...(args);}()\" )\n"
                      "2:\n"
                      "|\n"
                      "5:\n"
                      "6: ;\n"
                      "} ;", tokenizeAndStringify("int f0(Args args) {\n"
                              "    return ^{\n"
                              "        return sizeof...(Args);\n"
                              "    }() + ^ {\n"
                              "        return sizeof...(args);\n"
                              "    }();\n"
                              "};"));
        ASSERT_EQUALS("int ( ^ block ) ( void ) = asm ( \"^{staticinttest=0;returntest;}\" )\n\n\n;",
                      tokenizeAndStringify("int(^block)(void) = ^{\n"
                                           "    static int test = 0;\n"
                                           "    return test;\n"
                                           "};"));

        ASSERT_EQUALS("; return f ( a [ b = c ] , asm ( \"^{}\" ) ) ;",
                      tokenizeAndStringify("; return f(a[b=c],^{});")); // #7185
        ASSERT_EQUALS("; return f ( asm ( \"^(void){somecode}\" ) ) ;",
                      tokenizeAndStringify("; return f(^(void){somecode});"));
        ASSERT_EQUALS("; asm ( \"a?(b?(c,asm(\"^{}\")):0):^{}\" ) ;",
                      tokenizeAndStringify(";a?(b?(c,^{}):0):^{};"));
    }

    void ifAddBraces1() {
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

    void ifAddBraces2() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) if (b) { }\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { if ( b ) { } }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces3() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) for (;;) { }\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { for ( ; ; ) { } }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces4() {
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

    void ifAddBraces5() {
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
                      "\n"
                      "return ; }\n\n"
                      "return ;\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces7() {
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

    void ifAddBraces9() {
        // ticket #990
        const char code[] =
            "void f() {"
            "    for (int k=0; k<VectorSize; k++)"
            "        LOG_OUT(ID_Vector[k])"
            "}";
        const char expected[] =
            "void f ( ) { "
            "for ( int k = 0 ; k < VectorSize ; k ++ ) "
            "LOG_OUT ( ID_Vector [ k ] ) "
            "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void ifAddBraces10() {
        // ticket #1361
        const char code[] = "{ DEBUG(if (x) y; else z); }";
        const char expected[] = "{ DEBUG ( if ( x ) { y ; } else z ) ; }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void ifAddBraces11() {
        const char code[] = "{ if (x) if (y) ; else ; }";
        const char expected[] = "{ if ( x ) { if ( y ) { ; } else { ; } } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void ifAddBraces12() {
        // ticket #1424
        const char code[] = "{ if (x) do { } while(x); }";
        const char expected[] = "{ if ( x ) { do { } while ( x ) ; } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void ifAddBraces13() {
        // ticket #1809
        const char code[] = "{ if (x) if (y) { } else { } else { } }";
        const char expected[] = "{ if ( x ) { if ( y ) { } else { } } else { } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));

        // ticket #1809
        const char code2[] = "{ if (x) while (y) { } else { } }";
        const char expected2[] = "{ if ( x ) { while ( y ) { } } else { } }";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2, true));
    }

    void ifAddBraces15() {
        // ticket #2616 - unknown macro before if
        // TODO: Remove "A" or change it to ";A;". Then cleanup Tokenizer::ifAddBraces().
        ASSERT_EQUALS("{ A if ( x ) { y ( ) ; } }", tokenizeAndStringify("{A if(x)y();}", false));
    }

    void ifAddBraces16() {
        // ticket #2873 - the fix is not needed anymore.
        {
            const char code[] = "void f() { "
                                "(void) ( { if(*p) (*p) = x(); } ) "
                                "}";
            ASSERT_EQUALS("void f ( ) { ( void ) ( { if ( * p ) { ( * p ) = x ( ) ; } } ) }",
                          tokenizeAndStringify(code));
        }
    }

    void ifAddBraces17() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a)\n"
                            "        bar1 ();\n"
                            "\n"
                            "    else\n"
                            "        bar2 ();\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) {\n"
                      "bar1 ( ) ; }\n"
                      "\n"
                      "else {\n"
                      "bar2 ( ) ; }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces18() {
        // ticket #3424 - if if { } else else
        ASSERT_EQUALS("{ if ( x ) { if ( y ) { } else { ; } } else { ; } }",
                      tokenizeAndStringify("{ if(x) if(y){}else;else;}", false));

        ASSERT_EQUALS("{ if ( x ) { if ( y ) { if ( z ) { } else { ; } } else { ; } } else { ; } }",
                      tokenizeAndStringify("{ if(x) if(y) if(z){}else;else;else;}", false));
    }

    void ifAddBraces19() {
        // #3928 - if for if else
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a)\n"
                            "        for (;;)\n"
                            "            if (b)\n"
                            "                bar1();\n"
                            "            else\n"
                            "                bar2();\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) {\n"
                      "for ( ; ; ) {\n"
                      "if ( b ) {\n"
                      "bar1 ( ) ; }\n"
                      "else {\n"
                      "bar2 ( ) ; } } }\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void ifAddBraces20() { // #5012 - syntax error 'else }'
        const char code[] = "void f() { if(x) {} else }";
        ASSERT_THROW(tokenizeAndStringify(code, true), InternalError);
    }

    void ifAddBraces21() { // #5332 - if (x) label: {} ...
        const char code[] = "void f() { if(x) label: {} a=1; }";
        ASSERT_EQUALS("void f ( ) { if ( x ) { label : ; { } } a = 1 ; }", tokenizeAndStringify(code, false));
    }

    void whileAddBraces() {
        const char code[] = ";while(a);";
        ASSERT_EQUALS("; while ( a ) { ; }", tokenizeAndStringify(code, true));
    }

    void doWhileAddBraces() {
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

        {
            // #8148 - while inside the do-while body
            const char code[] = "void foo() {\n"
                                "    do { while (x) f(); } while (y);\n"
                                "}";
            const char result[] = "void foo ( ) {\n"
                                  "do { while ( x ) { f ( ) ; } } while ( y ) ;\n"
                                  "}";
            ASSERT_EQUALS(result, tokenizeAndStringify(code, true));
        }
    }

    void forAddBraces1() {
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

    void forAddBraces2() { // #5088
        const char code[] = "void f() {\n"
                            "    for(;;) try { } catch (...) { }\n"
                            "}";
        const char expected[] = "void f ( ) {\n"
                                "for ( ; ; ) { try { } catch ( . . . ) { } }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    std::string simplifyKnownVariables(const char code[]) {
        errout.str("");

        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        tokenizer.simplifyKnownVariables();

        return tokenizer.tokens()->stringifyList(0, false);
    }

    void simplifyKnownVariables1() {
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

    void simplifyKnownVariables2() {
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

    void simplifyKnownVariables3() {
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

    void simplifyKnownVariables4() {
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

    void simplifyKnownVariables5() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( a = 5 );\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; if ( a = 5 ) { ; } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables6() {
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

    void simplifyKnownVariables7() {
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

    void simplifyKnownVariables8() {
        const char code[] = "void foo()\n"
                            "{\n"
                            "    int i = 22;\n"
                            "    i++;\n"
                            "    abc[i] = 0;\n"
                            "}\n";

        ASSERT_EQUALS(
            "void foo ( ) { int i ; i = 23 ; abc [ 23 ] = 0 ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables9() {
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

    void simplifyKnownVariables10() {
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

    void simplifyKnownVariables11() {
        const char code[] = "const int foo = 0;\n"
                            "int main()\n"
                            "{\n"
                            "  int foo=0;\n"
                            "}\n";

        ASSERT_EQUALS(
            "int main ( ) { int foo ; foo = 0 ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables12() {
        const char code[] = "ENTER_NAMESPACE(project_namespace)\n"
                            "const double pi = 3.14;\n"
                            "int main(){}\n";
        ASSERT_EQUALS(
            "ENTER_NAMESPACE ( project_namespace ) const double pi = 3.14 ; int main ( ) { }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables13() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int i = 10;\n"
                            "    while(--i) {}\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int i ; i = 10 ; while ( -- i ) { } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables14() {
        // ticket #753
        const char code[] = "void f ( ) { int n ; n = 1 ; do { ++ n ; } while ( n < 10 ) ; }";
        ASSERT_EQUALS(code, simplifyKnownVariables(code));
    }

    void simplifyKnownVariables15() {
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

    void simplifyKnownVariables16() {
        // ticket #807 - segmentation fault when macro isn't found
        const char code[] = "void f ( ) { int n = 1; DISPATCH(while); }";
        ASSERT_THROW(simplifyKnownVariables(code), InternalError);
    }

    void simplifyKnownVariables17() {
        // ticket #807 - segmentation fault when macro isn't found
        const char code[] = "void f ( ) { char *s = malloc(100);mp_ptr p = s; p++; }";
        ASSERT_EQUALS(
            "void f ( ) { char * s ; s = malloc ( 100 ) ; mp_ptr p ; p = s ; p ++ ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables18() {
        const char code[] = "void f ( ) { char *s = malloc(100);mp_ptr p = s; ++p; }";
        ASSERT_EQUALS(
            "void f ( ) { char * s ; s = malloc ( 100 ) ; mp_ptr p ; p = s ; ++ p ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables19() {
        const char code[] = "void f ( ) { int i=0; do { if (i>0) { a(); } i=b(); } while (i != 12); }";
        ASSERT_EQUALS(
            "void f ( ) { int i ; i = 0 ; do { if ( i > 0 ) { a ( ) ; } i = b ( ) ; } while ( i != 12 ) ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables20() {
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

    void simplifyKnownVariables21() {
        const char code[] = "void foo() { int n = 10; for (int i = 0; i < n; ++i) { } }";

        ASSERT_EQUALS(
            "void foo ( ) { int n ; n = 10 ; for ( int i = 0 ; i < 10 ; ++ i ) { } }",
            simplifyKnownVariables(code));

        ASSERT_EQUALS(
            "void foo ( int i ) { int n ; n = i ; for ( i = 0 ; i < n ; ++ i ) { } }",
            simplifyKnownVariables("void foo(int i) { int n = i; for (i = 0; i < n; ++i) { } }"));
    }

    void simplifyKnownVariables22() {
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

    void simplifyKnownVariables23() {
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

    void simplifyKnownVariables25() {
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

    void simplifyKnownVariables27() {
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

    void simplifyKnownVariables28() {
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
            " if ( 2 > 0 ) { } "
            "}",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables29() { // ticket #1811
        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h + i;\n"
                                "}\n";
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char expected[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
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
            const char wanted[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                  "2: {\n"
                                  "3:\n"
                                  "4:\n"
                                  "5: return u@1 && v@2 ;\n"
                                  "6: }\n";
            ASSERT_EQUALS(wanted, tokenizeDebugListing(code, true));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h || i;\n"
                                "}\n";
            const char wanted[] = "1: bool foo ( int u@1 , int v@2 )\n"
                                  "2: {\n"
                                  "3:\n"
                                  "4:\n"
                                  "5: return u@1 || v@2 ;\n"
                                  "6: }\n";
            ASSERT_EQUALS(wanted, tokenizeDebugListing(code, true));
        }
    }

    void simplifyKnownVariables30() {
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

    void simplifyKnownVariables31() {
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

    void simplifyKnownVariables32() {
        {
            const char code[] = "void foo() {\n"
                                "    const int x = 0;\n"
                                "    bar(0,x);\n"
                                "}\n";
            const char expected[] = "void foo ( ) {\n\nbar ( 0 , 0 ) ;\n}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "static int const SZ = 22; char str[SZ];\n";
            ASSERT_EQUALS("char str [ 22 ] ;", tokenizeAndStringify(code,true));
        }
    }

    void simplifyKnownVariables33() {
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

    void simplifyKnownVariables34() {
        const char code[] = "void f() {\n"
                            "    int x = 10;\n"
                            "    do { cin >> x; } while (x > 5);\n"
                            "    a[x] = 0;\n"
                            "}\n";
        const char expected[] = "void f ( ) {\n"
                                "int x ; x = 10 ;\n"
                                "do { cin >> x ; } while ( x > 5 ) ;\n"
                                "a [ x ] = 0 ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables35() {
        // Ticket #2353
        const char code[] = "int f() {"
                            "    int x = 0;"
                            "    if (x == 0) {"
                            "        return 0;"
                            "    }"
                            "    return 10 / x;"
                            "}";
        const char expected[] = "int f ( ) { int x ; x = 0 ; { return 0 ; } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables36() {
        // Ticket #2304
        const char code[] = "void f() {"
                            "    const char *q = \"hello\";"
                            "    strcpy(p, q);"
                            "}";
        const char expected[] = "void f ( ) { const char * q ; q = \"hello\" ; strcpy ( p , \"hello\" ) ; }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));

        // Ticket #5972
        const char code2[] = "void f() {"
                             "  char buf[10] = \"ab\";"
                             "    memset(buf, 0, 10);"
                             "}";
        const char expected2[] = "void f ( ) { char buf [ 10 ] = \"ab\" ; memset ( buf , 0 , 10 ) ; }";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2, true));
    }

    void simplifyKnownVariables37() {
        // Ticket #2398 - no simplification in for loop
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
                                "\n"
                                "\n"
                                "Eval ( x ) ;\n"
                                "}\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables38() {
        // Ticket #2399 - simplify conditions
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    int y = 1;\n"
                            "    if (x || y);\n"
                            "}";
        const char expected[] = "void f ( ) {\n"
                                "\n"
                                "\n"
                                ";\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables39() {
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


    void simplifyKnownVariables40() {
        const char code[] = "void f() {\n"
                            "    char c1 = 'a';\n"
                            "    char c2 = { c1 };\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n\nchar c2 ; c2 = { 'a' } ;\n}", tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables41() {
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    const int *p; p = &x;\n"
                            "    if (p) { return 0; }\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\nint x ; x = 0 ;\nconst int * p ; p = & x ;\nif ( & x ) { return 0 ; }\n}", tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables42() {
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
            const char code[] = "void f() {\n"
                                "   char a[10];\n"
                                "   strcpy(a, \"hello\");\n"
                                "   strcat(a, \"!\");\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char a [ 10 ] ;\n"
                                    "strcpy ( a , \"hello\" ) ;\n"
                                    "strcat ( a , \"!\" ) ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Native, "test.c"));
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

        // 3538
        {
            const char code[] = "void f() {\n"
                                "    char s[10];\n"
                                "    strcpy(s, \"123\");\n"
                                "    if (s[6] == ' ');\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char s [ 10 ] ;\n"
                                    "strcpy ( s , \"123\" ) ;\n"
                                    "if ( s [ 6 ] == ' ' ) { ; }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));
        }
    }

    void simplifyKnownVariables43() {
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

    void simplifyKnownVariables44() {
        const char code[] = "void a() {\n"
                            "    static int i = 10;\n"
                            "    b(i++);\n"
                            "}";
        const char expected[] = "void a ( ) {\n"
                                "static int i = 10 ;\n"
                                "b ( i ++ ) ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables45() {
        const char code[] = "class Fred {\n"
                            "private:\n"
                            "    const static int NUM = 2;\n"
                            "    int array[NUM];\n"
                            "}";
        const char expected[] = "class Fred {\n"
                                "private:\n"
                                "\n"
                                "int array [ 2 ] ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables46() {
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    cin >> x;\n"
                            "    return x;\n"
                            "}";

        {
            const char expected[] = "void f ( ) {\n"
                                    "int x ; x = 0 ;\n"
                                    "cin >> x ;\n"
                                    "return x ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Native, "test.cpp"));
        }

        {
            const char expected[] = "void f ( ) {\n"
                                    "\n"
                                    "cin >> 0 ;\n"
                                    "return 0 ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Native, "test.c"));
        }
    }

    void simplifyKnownVariables47() {
        // #3621
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    cin >> std::hex >> x;\n"
                            "}";
        const char expected[] = "void f ( ) {\n"
                                "int x ; x = 0 ;\n"
                                "cin >> std :: hex >> x ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Native, "test.cpp"));
    }

    void simplifyKnownVariables48() {
        // #3754
        const char code[] = "void f(int sz) {\n"
                            "    int i;\n"
                            "    for (i = 0; ((i<sz) && (sz>3)); ++i) { }\n"
                            "}";
        const char expected[] = "void f ( int sz ) {\n"
                                "int i ;\n"
                                "for ( i = 0 ; ( i < sz ) && ( sz > 3 ) ; ++ i ) { }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Native, "test.c"));
    }

    void simplifyKnownVariables49() { // #3691
        const char code[] = "void f(int sz) {\n"
                            "    switch (x) {\n"
                            "    case 1: sz = 2; continue;\n"
                            "    case 2: x = sz; break;\n"
                            "    }\n"
                            "}";
        const char expected[] = "void f ( int sz ) {\n"
                                "switch ( x ) {\n"
                                "case 1 : ; sz = 2 ; continue ;\n"
                                "case 2 : ; x = sz ; break ;\n"
                                "}\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Native, "test.c"));
    }

    void simplifyKnownVariables50() { // #4066
        {
            const char code[] = "void f() {\n"
                                "    char str1[10], str2[10];\n"
                                "    sprintf(str1, \"%%\");\n"
                                "    strcpy(str2, str1);\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char str1 [ 10 ] ; char str2 [ 10 ] ;\n"
                                    "sprintf ( str1 , \"%%\" ) ;\n"
                                    "strcpy ( str2 , \"%\" ) ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }
        {
            const char code[] = "void f() {\n"
                                "    char str1[25], str2[25];\n"
                                "    sprintf(str1, \"abcdef%%%% and %% and %\");\n"
                                "    strcpy(str2, str1);\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char str1 [ 25 ] ; char str2 [ 25 ] ;\n"
                                    "sprintf ( str1 , \"abcdef%%%% and %% and %\" ) ;\n"
                                    "strcpy ( str2 , \"abcdef%% and % and %\" ) ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }
        {
            const char code[] = "void f() {\n"
                                "    char str1[10], str2[10];\n"
                                "    sprintf(str1, \"abc\");\n"
                                "    strcpy(str2, str1);\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char str1 [ 10 ] ; char str2 [ 10 ] ;\n"
                                    "sprintf ( str1 , \"abc\" ) ;\n"
                                    "strcpy ( str2 , \"abc\" ) ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        }
        {
            //don't simplify '&x'!
            const char code[] = "const char * foo ( ) {\n"
                                "const char x1 = 'b' ;\n"
                                "f ( & x1 ) ;\n"
                                "const char x2 = 'b' ;\n"
                                "f ( y , & x2 ) ;\n"
                                "const char x3 = 'b' ;\n"
                                "t = & x3 ;\n"
                                "const char x4 = 'b' ;\n"
                                "t = y + & x4 ;\n"
                                "const char x5 = 'b' ;\n"
                                "z [ & x5 ] = y ;\n"
                                "const char x6 = 'b' ;\n"
                                "v = { & x6 } ;\n"
                                "const char x7 = 'b' ;\n"
                                "return & x7 ;\n"
                                "}";
            ASSERT_EQUALS(code, tokenizeAndStringify(code, true));
        }
        {
            //don't simplify '&x'!
            const char code[] = "const int * foo ( ) {\n"
                                "const int x1 = 1 ;\n"
                                "f ( & x1 ) ;\n"
                                "const int x2 = 1 ;\n"
                                "f ( y , & x2 ) ;\n"
                                "const int x3 = 1 ;\n"
                                "t = & x3 ;\n"
                                "const int x4 = 1 ;\n"
                                "t = y + & x4 ;\n"
                                "const int x5 = 1 ;\n"
                                "z [ & x5 ] = y ;\n"
                                "const int x6 = 1 ;\n"
                                "v = { & x6 } ;\n"
                                "const int x7 = 1 ;\n"
                                "return & x7 ;\n"
                                "}";
            ASSERT_EQUALS(code, tokenizeAndStringify(code, true));
        }
    }

    void simplifyKnownVariables51() { // #4409 hang
        const char code[] = "void mhz_M(int enough) {\n"
                            "  TYPE *x=&x, **p=x, **q = NULL;\n"
                            "  BENCH1(q = _mhz_M(n); n = 1;)\n"
                            "  use_pointer(q);\n"
                            "}";
        tokenizeAndStringify(code, true); // don't hang
    }

    void simplifyKnownVariables52() { // #4728 "= x %op%"
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 + z ; }", tokenizeAndStringify("void f() { int x=34; int y=x+z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 - z ; }", tokenizeAndStringify("void f() { int x=34; int y=x-z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 * z ; }", tokenizeAndStringify("void f() { int x=34; int y=x*z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 / z ; }", tokenizeAndStringify("void f() { int x=34; int y=x/z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 % z ; }", tokenizeAndStringify("void f() { int x=34; int y=x%z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 & z ; }", tokenizeAndStringify("void f() { int x=34; int y=x&z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 | z ; }", tokenizeAndStringify("void f() { int x=34; int y=x|z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 ^ z ; }", tokenizeAndStringify("void f() { int x=34; int y=x^z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 << z ; }", tokenizeAndStringify("void f() { int x=34; int y=x<<z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 >> z ; }", tokenizeAndStringify("void f() { int x=34; int y=x>>z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 && z ; }", tokenizeAndStringify("void f() { int x=34; int y=x&&z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 || z ; }", tokenizeAndStringify("void f() { int x=34; int y=x||z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 > z ; }", tokenizeAndStringify("void f() { int x=34; int y=x>z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 >= z ; }", tokenizeAndStringify("void f() { int x=34; int y=x>=z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 < z ; }", tokenizeAndStringify("void f() { int x=34; int y=x<z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 <= z ; }", tokenizeAndStringify("void f() { int x=34; int y=x<=z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 == z ; }", tokenizeAndStringify("void f() { int x=34; int y=x==z; }", true));
        ASSERT_EQUALS("void f ( ) { int y ; y = 34 != z ; }", tokenizeAndStringify("void f() { int x=34; int y=x!=z; }", true));

        // #4007
        ASSERT_EQUALS("void f ( ) { }", tokenizeAndStringify("void f() { char *p = 0; int result = p && (!*p); }", true));
        ASSERT_EQUALS("void f ( ) { }", tokenizeAndStringify("void f() { Foo *p = 0; bool b = (p && (p->type() == 1)); }", true));
    }

    void simplifyKnownVariables53() { // references
        ASSERT_EQUALS("void f ( ) { int x ; x = abc ( ) ; }", tokenizeAndStringify("void f() { int x; int &ref=x; ref=abc(); }", true));
        ASSERT_EQUALS("void f ( ) { int * p ; p = abc ( ) ; }", tokenizeAndStringify("void f() { int *p; int *&ref=p; ref=abc(); }", true));
    }

    void simplifyKnownVariables54() { // #4913
        ASSERT_EQUALS("void f ( int * p ) { * -- p = 0 ; * p = 0 ; }", tokenizeAndStringify("void f(int*p) { *--p=0; *p=0; }", true));
    }

    void simplifyKnownVariables55() { // pointer alias
        ASSERT_EQUALS("void f ( ) { int a ; if ( a > 0 ) { } }", tokenizeAndStringify("void f() { int a; int *p=&a; if (*p>0) {} }", true));
        ASSERT_EQUALS("void f ( ) { int a ; struct AB ab ; ab . a = & a ; if ( a > 0 ) { } }", tokenizeAndStringify("void f() { int a; struct AB ab; ab.a = &a; if (*ab.a>0) {} }", true));
        ASSERT_EQUALS("void f ( ) { int a ; if ( x > a ) { } }", tokenizeAndStringify("void f() { int a; int *p=&a; if (x>*p) {} }", true));
    }

    void simplifyKnownVariables56() { // ticket #5301 - >>
        ASSERT_EQUALS("void f ( ) { int a ; a = 0 ; int b ; b = 0 ; * p >> a >> b ; return a / b ; }",
                      tokenizeAndStringify("void f() { int a=0,b=0; *p>>a>>b; return a/b; }", true));
    }

    void simplifyKnownVariables57() { // #4724
        ASSERT_EQUALS("unsigned long long x ; x = 9223372036854775808UL ;", tokenizeAndStringify("unsigned long long x = 1UL << 63 ;", true));
        ASSERT_EQUALS("long long x ; x = -9223372036854775808L ;", tokenizeAndStringify("long long x = 1L << 63 ;", true));
    }

    void simplifyKnownVariables58() { // #5268
        const char code[] = "enum e { VAL1 = 1, VAL2 }; "
                            "typedef char arr_t[VAL2]; "
                            "int foo(int) ; "
                            "void bar () { "
                            "  throw foo (VAL1); "
                            "} "
                            "int baz() { "
                            "  return sizeof(arr_t); "
                            "}";
        ASSERT_EQUALS("enum e { VAL1 = 1 , VAL2 } ; "
                      "int foo ( int ) ; "
                      "void bar ( ) { "
                      "throw foo ( VAL1 ) ; "
                      "} "
                      "int baz ( ) { "
                      "return sizeof ( char [ VAL2 ] ) ; "
                      "}", tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables59() { // #5062 - for head
        const char code[] = "void f() {\n"
                            "  int a[3], i, j;\n"
                            "  for(i = 0, j = 1; i < 3, j < 12; i++,j++) {\n"
                            "    a[i] = 0;\n"
                            "  }\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "int a [ 3 ] ; int i ; int j ;\n"
                      "for ( i = 0 , j = 1 ; i < 3 , j < 12 ; i ++ , j ++ ) {\n"
                      "a [ i ] = 0 ;\n"
                      "}\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables60() { // #6829
        const char code[] = "void f() {\n"
                            "  int i = 1;\n"
                            "  const int * const constPtrToConst = &i;\n"
                            "  std::cout << *constPtrToConst << std::endl;\n"
                            "  std::cout << constPtrToConst << std::endl;\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "int i ; i = 1 ;\n"
                      "const int * const constPtrToConst ; constPtrToConst = & i ;\n"
                      "std :: cout << i << std :: endl ;\n"
                      "std :: cout << & i << std :: endl ;\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariables61() { // #7805
        tokenizeAndStringify("static const int XX = 0;\n"
                             "enum E { XX };\n"
                             "struct s {\n"
                             "  enum Bar {\n"
                             "    XX,\n"
                             "    Other\n"
                             "  };\n"
                             "  enum { XX };\n"
                             "};", /*simplify=*/true);
        ASSERT_EQUALS("", errout.str());
    }

    void simplifyKnownVariables62() { // #5666
        ASSERT_EQUALS("void foo ( std :: string str ) {\n"
                      "char * p ; p = & str [ 0 ] ;\n"
                      "* p = 0 ;\n"
                      "}",
                      tokenizeAndStringify("void foo(std::string str) {\n"
                                           "  char *p = &str[0];\n"
                                           "  *p = 0;\n"
                                           "}", /*simplify=*/true));
    }

    void simplifyKnownVariablesBailOutAssign1() {
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

    void simplifyKnownVariablesBailOutAssign2() {
        // ticket #3032 - assignment in condition
        const char code[] = "void f(struct ABC *list) {\n"
                            "    struct ABC *last = NULL;\n"
                            "    nr = (last = list->prev)->nr;\n"  // <- don't replace "last" with 0
                            "}\n";
        const char expected[] = "void f ( struct ABC * list ) {\n"
                                "struct ABC * last ; last = NULL ;\n"
                                "nr = ( last = list . prev ) . nr ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariablesBailOutAssign3() { // #4395 - nested assignments
        const char code[] = "void f() {\n"
                            "    int *p = 0;\n"
                            "    a = p = (VdbeCursor*)pMem->z;\n"
                            "    return p ;\n"
                            "}\n";
        const char expected[] = "void f ( ) {\n"
                                "int * p ; p = 0 ;\n"
                                "a = p = pMem . z ;\n"
                                "return p ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
    }

    void simplifyKnownVariablesBailOutFor1() {
        const char code[] = "void foo() {\n"
                            "    for (int i = 0; i < 10; ++i) { }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "for ( int i = 0 ; i < 10 ; ++ i ) { }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        ASSERT_EQUALS("", errout.str());    // debug warnings
    }

    void simplifyKnownVariablesBailOutFor2() {
        const char code[] = "void foo() {\n"
                            "    int i = 0;\n"
                            "    while (i < 10) { ++i; }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "int i ; i = 0 ;\n"
                                "while ( i < 10 ) { ++ i ; }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        ASSERT_EQUALS("", errout.str());    // debug warnings
    }

    void simplifyKnownVariablesBailOutFor3() {
        const char code[] = "void foo() {\n"
                            "    for (std::string::size_type pos = 0; pos < 10; ++pos)\n"
                            "    { }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "for ( std :: string :: size_type pos = 0 ; pos < 10 ; ++ pos )\n"
                                "{ }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true));
        ASSERT_EQUALS("", errout.str());    // debug warnings
    }

    void simplifyKnownVariablesBailOutMemberFunction() {
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

    void simplifyKnownVariablesBailOutConditionalIncrement() {
        const char code[] = "int f() {\n"
                            "    int a = 0;\n"
                            "    if (x) {\n"
                            "        ++a;\n" // conditional increment
                            "    }\n"
                            "    return a;\n"
                            "}\n";
        tokenizeAndStringify(code,true);
        ASSERT_EQUALS("", errout.str());     // no debug warnings
    }

    void simplifyKnownVariablesBailOutSwitchBreak() {
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
                            "            q = x;\n" // x is not equal with p
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

    void simplifyKnownVariablesFloat() {
        // Ticket #2454
        const char code[] = "void f() {\n"
                            "    float a = 40;\n"
                            "    x(10 / a);\n"
                            "}\n";

        const char expected[] = "void f ( ) {\n\nx ( 0.25 ) ;\n}";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));

        // Ticket #4227
        const char code2[] = "double f() {"
                             "    double a = false;"
                             "    return a;"
                             "}";
        ASSERT_EQUALS("double f ( ) { return 0.0 ; }", tokenizeAndStringify(code2,true));

        // Ticket #5485
        const char code3[] = "void f() {"
                             "    double a = 1e+007;\n"
                             "    std::cout << a;\n"
                             "}";
        ASSERT_EQUALS("void f ( ) {\nstd :: cout << 1e+007 ;\n}", tokenizeAndStringify(code3,true));

        const char code4[] = "void f() {"
                             "    double a = 1;\n"
                             "    std::cout << a;\n"
                             "}";
        ASSERT_EQUALS("void f ( ) {\nstd :: cout << 1.0 ;\n}", tokenizeAndStringify(code4,true));
    }

    void simplifyKnownVariablesFunctionCalls() {
        {
            const char code[] = "void a(int x);"  // <- x is passed by value
                                "void b() {"
                                "    int x = 123;"
                                "    a(x);"       // <- replace with a(123);
                                "}";
            const char expected[] = "void a ( int x ) ; void b ( ) { a ( 123 ) ; }";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));
        }

        {
            const char code[] = "void a(int &x);" // <- x is passed by reference
                                "void b() {"
                                "    int x = 123;"
                                "    a(x);"       // <- don't replace with a(123);
                                "}";
            const char expected[] = "void a ( int & x ) ; void b ( ) { int x ; x = 123 ; a ( x ) ; }";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code,true));
        }
    }

    void simplifyKnownVariablesGlobalVars() {
        // #8054
        const char code[] = "static int x;"
                            "void f() {"
                            "    x = 123;"
                            "    while (!x) { dostuff(); }"
                            "}";
        ASSERT_EQUALS("static int x ; void f ( ) { x = 123 ; while ( ! x ) { dostuff ( ) ; } }", tokenizeAndStringify(code,true));
    }

    void simplifyKnownVariablesReturn() {
        const char code[] = "int a() {"
                            "    int x = 123;"
                            "    return (x);"
                            "}";
        ASSERT_EQUALS("int a ( ) { return 123 ; }", tokenizeAndStringify(code,true));
    }

    void simplifyKnownVariablesPointerAliasFunctionCall() { // #7440
        const char code[] = "int main() {\n"
                            "  char* data = new char[100];\n"
                            "  char** dataPtr = &data;\n"
                            "  printf(\"test\");\n"
                            "  delete [] *dataPtr;\n"
                            "}";
        const char exp[]  = "int main ( ) {\n"
                            "char * data ; data = new char [ 100 ] ;\n"
                            "char * * dataPtr ; dataPtr = & data ;\n"
                            "printf ( \"test\" ) ;\n"
                            "delete [ ] data ;\n"
                            "}";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code, /*simplify=*/true));
    }

    void simplifyKnownVariablesClassMember() {
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

    void simplifyExternC() {
        ASSERT_EQUALS("int foo ( ) ;", tokenizeAndStringify("extern \"C\" int foo();"));
        ASSERT_EQUALS("int foo ( ) ;", tokenizeAndStringify("extern \"C\" { int foo(); }"));
    }

    void simplifyFunctionParameters() {
        {
            const char code[] = "char a [ ABC ( DEF ) ] ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
        }

        {
            const char code[] = "module ( a , a , sizeof ( a ) , 0444 ) ;";
            ASSERT_EQUALS("module ( a , a , sizeof ( a ) , 292 ) ;", tokenizeAndStringify(code));
        }

        ASSERT_EQUALS("void f ( int x ) { }", tokenizeAndStringify("void f(x) int x; { }"));
        ASSERT_EQUALS("void f ( int x , char y ) { }", tokenizeAndStringify("void f(x,y) int x; char y; { }"));
        ASSERT_EQUALS("int main ( int argc , char * argv [ ] ) { }", tokenizeAndStringify("int main(argc,argv) int argc; char *argv[]; { }"));
        ASSERT_EQUALS("int f ( int p , int w , float d ) { }", tokenizeAndStringify("int f(p,w,d) float d; { }"));

        // #1067 - Not simplified. Feel free to fix so it is simplified correctly but this syntax is obsolescent.
        ASSERT_EQUALS("int ( * d ( a , b , c ) ) ( ) int a ; int b ; int c ; { }", tokenizeAndStringify("int (*d(a,b,c))()int a,b,c; { }"));

        {
            // This is not a function but the pattern is similar..
            const char code[] = "void foo()"
                                "{"
                                "    if (x)"
                                "        int x;"
                                "    { }"
                                "}";
            ASSERT_EQUALS("void foo ( ) { if ( x ) { } { } }", tokenizeAndStringify(code, true));
        }

        // #3770 - Don't segfault and don't change macro argument as if it's a K&R function argument
        {
            const char code[] = "MACRO(a)"
                                ""
                                "void f()"
                                "{"
                                "    SetLanguage();"
                                "    {"
                                "    }"
                                "}";
            ASSERT_EQUALS("MACRO ( a ) void f ( ) { SetLanguage ( ) ; { } }", tokenizeAndStringify(code));
        }
    }

    void simplifyFunctionParameters1() { // ticket #3721

        const char code[] = "typedef float ufloat;\n"
                            "typedef short ftnlen;\n"
                            "int f(p,w,d,e,len) ufloat *p; ftnlen len;\n"
                            "{\n"
                            "}\n";
        ASSERT_EQUALS("int f ( float * p , int w , int d , int e , short len )\n"
                      "{\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyFunctionParameters2() { // #4430
        const char code[] = "class Item { "
                            "int i ; "
                            "public: "
                            "Item ( int i ) ; "
                            "} ; "
                            "Item :: Item ( int i ) : i ( i ) { }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyFunctionParameters3() { // #4436
        const char code[] = "class Item { "
                            "int i ; "
                            "int j ; "
                            "public: "
                            "Item ( int i , int j ) ; "
                            "} ; "
                            "Item :: Item ( int i , int j ) : i ( i ) , j ( j ) { }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyFunctionParametersMultiTemplate() {
        const char code[] = "template < typename T1 > template < typename T2 > "
                            "void A < T1 > :: foo ( T2 ) { }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyFunctionParametersErrors() {
        //same parameters...
        ASSERT_THROW(tokenizeAndStringify("void foo(x, x)\n"
                                          " int x;\n"
                                          " int x;\n"
                                          "{}\n"), InternalError);

        ASSERT_THROW(tokenizeAndStringify("void foo(x, y)\n"
                                          " int x;\n"
                                          " int x;\n"
                                          "{}\n"), InternalError);

        tokenizeAndStringify("void foo(int, int)\n"
                             "{}\n");
        ASSERT_EQUALS("", errout.str());

        // #3848 - Don't hang
        tokenizeAndStringify("sal_Bool ShapeHasText(sal_uLong, sal_uLong) const {\n"
                             "    return sal_True;\n"
                             "}\n"
                             "void CreateSdrOLEFromStorage() {\n"
                             "    comphelper::EmbeddedObjectContainer aCnt( xDestStorage );\n"
                             "    { }\n"
                             "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Simplify "((..))" into "(..)"
    void removeParentheses1() {
        const char code[] = "void foo()"
                            "{"
                            "    free(((void*)p));"
                            "}";

        ASSERT_EQUALS("void foo ( ) { free ( p ) ; }", tokenizeAndStringify(code, true));
    }

    void removeParentheses3() {
        {
            const char code[] = "void foo()"
                                "{"
                                "    if (( true )==(true)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { }", tokenizeAndStringify(code, true));
        }

        {
            const char code[] = "void foo()"
                                "{"
                                "    if (( 2 )==(2)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { }", tokenizeAndStringify(code, true));
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
    void removeParentheses4() {
        const char code[] = "void foo()"
                            "{"
                            "    (free(p));"
                            "}";
        ASSERT_EQUALS("void foo ( ) { free ( p ) ; }", tokenizeAndStringify(code));
    }

    void removeParentheses5() {
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
    void removeParentheses6() {
        {
            const char code[] = "(!(abc.a));";
            ASSERT_EQUALS("( ! abc . a ) ;", tokenizeAndStringify(code));
        }
        //handle more complex member selections
        {
            const char code[] = "(!(a.b.c.d));";
            ASSERT_EQUALS("( ! a . b . c . d ) ;", tokenizeAndStringify(code));
        }
    }

    void removeParentheses7() {
        const char code[] = ";char *p; (delete(p), (p)=0);";
        ASSERT_EQUALS("; char * p ; delete p ; p = 0 ;", tokenizeAndStringify(code,true));
    }

    void removeParentheses8() {
        const char code[] = "struct foo {\n"
                            "    void operator delete(void *obj, size_t sz);\n"
                            "}\n";
        const std::string actual(tokenizeAndStringify(code, false, true, Settings::Win32A));

        const char expected[] = "struct foo {\n"
                                "void operatordelete ( void * obj , unsigned long sz ) ;\n"
                                "}";

        ASSERT_EQUALS(expected, actual);
    }

    void removeParentheses9() {
        ASSERT_EQUALS("void delete ( double num ) ;", tokenizeAndStringify("void delete(double num);", false));
    }

    void removeParentheses10() {
        ASSERT_EQUALS("p = buf + 8 ;", tokenizeAndStringify("p = (buf + 8);", false));
    }

    void removeParentheses11() {
        // #2502
        ASSERT_EQUALS("{ } x ( ) ;", tokenizeAndStringify("{}(x());", false));
    }

    void removeParentheses12() {
        // #2760
        ASSERT_EQUALS(", x = 0 ;", tokenizeAndStringify(",(x)=0;", false));
    }

    void removeParentheses13() {
        ASSERT_EQUALS("; f ( a + b , c ) ;", tokenizeAndStringify(";f((a+b),c);", false));
        ASSERT_EQUALS("; x = y [ a + b ] ;", tokenizeAndStringify(";x=y[(a+b)];", false));
    }

    void removeParentheses14() {
        ASSERT_EQUALS("; if ( ( i & 1 ) == 0 ) { ; } ;", tokenizeAndStringify("; if ( (i & 1) == 0 ); ;", false));
    }

    void removeParentheses15() {
        ASSERT_EQUALS("a = b ? c : 123 ;", tokenizeAndStringify("a = b ? c : (123);", false));
        ASSERT_EQUALS("a = b ? c : ( 123 + 456 ) ;", tokenizeAndStringify("a = b ? c : ((123)+(456));", false));
        ASSERT_EQUALS("a = b ? 123 : c ;", tokenizeAndStringify("a = b ? (123) : c;", false));

        // #4316
        ASSERT_EQUALS("a = b ? c : ( d = 1 , 0 ) ;", tokenizeAndStringify("a = b ? c : (d=1,0);", false));
    }

    void removeParentheses16() { // *(x.y)=
        // #4423
        ASSERT_EQUALS("; * x = 0 ;", tokenizeAndStringify(";*(x)=0;", false));
        ASSERT_EQUALS("; * x . y = 0 ;", tokenizeAndStringify(";*(x.y)=0;", false));
    }

    void removeParentheses17() { // a ? b : (c > 0 ? d : e)
        ASSERT_EQUALS("a ? b : ( c > 0 ? d : e ) ;", tokenizeAndStringify("a?b:(c>0?d:e);", false));
    }

    void removeParentheses18() {
        ASSERT_EQUALS("float ( * a ) [ 2 ] ;", tokenizeAndStringify("float(*a)[2];", false));
    }

    void removeParentheses19() {
        ASSERT_EQUALS("( ( ( typeof ( X ) ) * ) 0 ) ;", tokenizeAndStringify("(((typeof(X))*)0);", false));
    }

    void removeParentheses20() {
        ASSERT_EQUALS("a < b < int > > ( 2 ) ;", tokenizeAndStringify("a<b<int>>(2);", false));
    }

    void removeParentheses21() {
        ASSERT_EQUALS("a = ( int ) - b ;", tokenizeAndStringify("a = ((int)-b);", false));
    }

    void removeParentheses22() {
        static char code[] = "struct S { "
                             "char *(a); "
                             "char &(b); "
                             "const static char *(c); "
                             "} ;";
        static char  exp[] = "struct S { "
                             "char * a ; "
                             "char & b ; "
                             "static const char * c ; "
                             "} ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParentheses23() { // Ticket #6103
        // Reported case
        {
            static char code[] = "; * * p f ( ) int = { new int ( * [ 2 ] ) ; void }";
            static char  exp[] = "; * * p f ( ) int = { new int ( * [ 2 ] ) ; void }";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        // Various valid cases
        {
            static char code[] = "int * f [ 1 ] = { new ( int ) } ;";
            static char  exp[] = "int * f [ 1 ] = { new int } ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        {
            static char code[] = "int * * f [ 1 ] = { new ( int ) [ 1 ] } ;";
            static char  exp[] = "int * * f [ 1 ] = { new int [ 1 ] } ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        {
            static char code[] = "list < int > * f [ 1 ] = { new ( list < int > ) } ;";
            static char  exp[] = "list < int > * f [ 1 ] = { new list < int > } ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        // don't remove parentheses in operator new overload
        {
            static char code[] = "void *operator new(__SIZE_TYPE__, int);";
            static char  exp[] = "void * operatornew ( __SIZE_TYPE__ , int ) ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
    }

    void removeParentheses24() { // Ticket #7040
        static char code[] = "std::hash<decltype(t._data)>()(t._data);";
        static char  exp[] = "std :: hash < decltype ( t . _data ) > ( ) ( t . _data ) ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void tokenize_double() {
        const char code[] = "void f() {\n"
                            "    double a = 4.2;\n"
                            "    float b = 4.2f;\n"
                            "    double c = 4.2e+10;\n"
                            "    double d = 4.2e-10;\n"
                            "    int e = 4+2;\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "double a ; a = 4.2 ;\n"
                      "float b ; b = 4.2f ;\n"
                      "double c ; c = 4.2e+10 ;\n"
                      "double d ; d = 4.2e-10 ;\n"
                      "int e ; e = 4 + 2 ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void tokenize_strings() {
        const char code[] =   "void f() {\n"
                              "const char *a =\n"
                              "{\n"
                              "\"hello \"\n"
                              "\"more \"\n"
                              "\"world\"\n"
                              "};\n"
                              "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "const char * a ; a =\n"
                      "{\n"
                      "\"hello more world\"\n"
                      "\n"
                      "\n"
                      "} ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplify_constants() {
        const char code[] =
            "void f() {\n"
            "const int a = 45;\n"
            "if( a )\n"
            "{ int b = a; }\n"
            "}\n"
            "void g() {\n"
            "int a = 2;\n"
            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "\n"
                      "\n"
                      "\n"
                      "}\n"
                      "void g ( ) {\n"
                      "\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void simplify_constants2() {
        const char code[] =
            "void f( Foo &foo, Foo *foo2 ) {\n"
            "const int a = 45;\n"
            "foo.a=a+a;\n"
            "foo2->a=a;\n"
            "}";
        ASSERT_EQUALS("void f ( Foo & foo , Foo * foo2 ) {\n"
                      "\n"
                      "foo . a = 90 ;\n"
                      "foo2 . a = 45 ;\n"
                      "}", tokenizeAndStringify(code, true));
    }

    void simplify_constants3() {
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

    void simplify_constants4() {
        const char code[] = "static const int bSize = 4;\n"
                            "static const int aSize = 50;\n"
                            "x = bSize;\n"
                            "y = aSize;\n";
        ASSERT_EQUALS("x = 4 ;\ny = 50 ;", tokenizeAndStringify(code,true));
    }

    void simplify_constants5() {
        const char code[] = "int buffer[10];\n"
                            "static const int NELEMS = sizeof(buffer)/sizeof(int);\n"
                            "static const int NELEMS2(sizeof(buffer)/sizeof(int));\n"
                            "x = NELEMS;\n"
                            "y = NELEMS2;\n";
        ASSERT_EQUALS("int buffer [ 10 ] ;\n\n\nx = 10 ;\ny = 10 ;", tokenizeAndStringify(code,true));
    }

    void simplify_constants6() { // Ticket #5625
        {
            const char code[] = "template < class T > struct foo ;\n"
                                "void bar ( ) {\n"
                                "foo < 1 ? 0 ? 1 : 6 : 2 > x ;\n"
                                "foo < 1 ? 0 : 2 > y ;\n"
                                "}";
            const char exp [] = "template < class T > struct foo ;\n"
                                "void bar ( ) {\n"
                                "foo < 6 > x ;\n"
                                "foo < 0 > y ;\n"
                                "}";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code, true));
        }
        {
            const char code[] = "bool b = true ? false : 1 > 2 ;";
            const char exp [] = "bool b ; b = false ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code, true));
        }
    }

    void simplifyMulAndParens() {
        // (error) Resource leak
        const char code[] = "void f() {"
                            "   *&n1=open();"
                            "   *&(n2)=open();"
                            "   *(&n3)=open();"
                            "   *&*&n4=open();"
                            "   *&*&*&(n5)=open();"
                            "   *&*&(*&n6)=open();"
                            "   *&*(&*&n7)=open();"
                            "   *(&*&n8)=open();"
                            "   *&(*&*&(*&n9))=open();"
                            "   (n10) = open();"
                            "   ((n11)) = open();"
                            "   ((*&n12))=open();"
                            "   *(&(*&n13))=open();"
                            "   ((*&(*&n14)))=open();"
                            "   ((*&(*&n15)))+=10;"
                            "}";
        const char expected[] = "void f ( ) {"
                                " n1 = open ( ) ;"
                                " n2 = open ( ) ;"
                                " n3 = open ( ) ;"
                                " n4 = open ( ) ;"
                                " n5 = open ( ) ;"
                                " n6 = open ( ) ;"
                                " n7 = open ( ) ;"
                                " n8 = open ( ) ;"
                                " n9 = open ( ) ;"
                                " n10 = open ( ) ;"
                                " n11 = open ( ) ;"
                                " n12 = open ( ) ;"
                                " n13 = open ( ) ;"
                                " n14 = open ( ) ;"
                                " n15 += 10 ; "
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void simplifyStructDecl() {
        const char code[] = "const struct A { int a; int b; } a;";
        ASSERT_EQUALS("struct A { int a ; int b ; } ; const struct A a ;", tokenizeAndStringify(code));
    }

    void vardecl1() {
        const char code[] = "unsigned int a, b;";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("unsigned int a ; unsigned int b ;", actual);
    }

    void vardecl2() {
        const char code[] = "void foo(a,b) unsigned int a, b; { }";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("void foo ( unsigned int a , unsigned int b ) { }", actual);
    }

    void vardecl3() {
        const char code[] = "void f() { char * p = foo<10,char>(); }";
        const std::string actual(tokenizeAndStringify(code));
        ASSERT_EQUALS("void f ( ) { char * p ; p = foo < 10 , char > ( ) ; }", actual);
    }

    void vardecl4() {
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

    void vardecl5() {
        ASSERT_EQUALS("void foo ( int nX ) {\n"
                      "int addI ; addI = frontPoint == 2 || frontPoint == 1 ? ( i = 0 , 1 ) : ( i = nX - 2 , -1 ) ;\n"
                      "}", tokenizeAndStringify("void foo(int nX) {\n"
                                                "    int addI = frontPoint == 2 || frontPoint == 1 ? i = 0, 1 : (i = nX - 2, -1);\n"
                                                "}"));
    }

    void vardecl_stl_1() {
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

    void vardecl_stl_2() {
        const char code1[] = "{ std::string x = \"abc\"; }";
        ASSERT_EQUALS("{ std :: string x ; x = \"abc\" ; }", tokenizeAndStringify(code1));

        const char code2[] = "{ std::vector<int> x = y; }";
        ASSERT_EQUALS("{ std :: vector < int > x ; x = y ; }", tokenizeAndStringify(code2));
    }

    void vardecl_template_1() {
        // ticket #1046
        const char code1[] = "b<(1<<24),10,24> u, v;";
        const char res1[]  = "b < 16777216 , 10 , 24 > u ; b < 16777216 , 10 , 24 > v ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));
        // ticket #3571 (segmentation fault)
        tokenizeAndStringify("template <int i = (3>4) > class X4 {};");
    }

    void vardecl_template_2() {
        // ticket #3650
        const char code[] = "const string str = x<8,int>();";
        const char expected[]  = "const string str = x < 8 , int > ( ) ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void vardecl_union() {
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

        // ticket #3927
        const char code3[] = "union xy *p = NULL;";
        ASSERT_EQUALS("union xy * p ; p = NULL ;", tokenizeAndStringify(code3));
    }

    void vardecl_par() {
        // ticket #2743 - set links if variable type contains parentheses
        const char code[] = "Fred<int(*)()> fred1=a, fred2=b;";
        ASSERT_EQUALS("Fred < int ( * ) ( ) > fred1 ; fred1 = a ; Fred < int ( * ) ( ) > fred2 ; fred2 = b ;", tokenizeAndStringify(code));
    }

    void vardecl_par2() {
        // ticket #3912 - set correct links
        const char code[] = "function<void (shared_ptr<MyClass>)> v;";
        ASSERT_EQUALS("function < void ( shared_ptr < MyClass > ) > v ;", tokenizeAndStringify(code));
    }

    void vardecl_par3() {
        // ticket #6556- Fred x1(a), x2(b);
        const char code[] = "Fred x1(a), x2(b);";
        ASSERT_EQUALS("Fred x1 ( a ) ; Fred x2 ( b ) ;", tokenizeAndStringify(code));
    }

    void vardecl_class_ref() {
        const char code[] = "class A { B &b1,&b2; };";
        ASSERT_EQUALS("class A { B & b1 ; B & b2 ; } ;", tokenizeAndStringify(code));
    }

    void vardec_static() {
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

        {
            // Ticket #4450
            const char code[] = "static int large_eeprom_type = (13 | (5)), "
                                "default_flash_type = 42;";
            ASSERT_EQUALS("static int large_eeprom_type = 13 | 5 ; static int default_flash_type = 42 ;",
                          tokenizeAndStringify(code));
        }

        {
            // Ticket #5121
            const char code[] = "unsigned int x;"
                                "static const unsigned int A = 1, B = A, C = 0, D = (A), E = 0;"
                                "void f() {"
                                "  unsigned int *foo = &x;"
                                "}";
            ASSERT_EQUALS("unsigned int x ; "
                          "static const unsigned int A = 1 ; "
                          "static const unsigned int B = A ; "
                          "static const unsigned int C = 0 ; "
                          "static const unsigned int D = A ; "
                          "static const unsigned int E = 0 ; "
                          "void f ( ) { "
                          "unsigned int * foo ; "
                          "foo = & x ; "
                          "}",
                          tokenizeAndStringify(code));
        }

        {
            // Ticket #5266
            const char code[] = "class Machine {\n"
                                "  static int const STACK_ORDER = 10, STACK_MAX = 1 << STACK_ORDER,"
                                "                   STACK_GUARD = 2;\n"
                                "};";
            ASSERT_EQUALS("class Machine {\n"
                          "static const int STACK_ORDER = 10 ; static const int STACK_MAX = 1 << STACK_ORDER ; "
                          "static const int STACK_GUARD = 2 ;\n"
                          "} ;",
                          tokenizeAndStringify(code));
        }
    }

    void vardecl6() {
        // ticket #565

        const char code1[] = "int z = x >> 16;";
        const char res1[]  = "int z ; z = x >> 16 ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));
    }

    void vardecl7() {
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

    void vardecl8() {
        // ticket #696
        const char code[] = "char a[10]={'\\0'}, b[10]={'\\0'};";
        const char res[]  = "char a [ 10 ] = { '\\0' } ; char b [ 10 ] = { '\\0' } ;";
        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl9() {
        const char code[] = "char a[2] = {'A', '\\0'}, b[2] = {'B', '\\0'};";
        const char res[]  = "char a [ 2 ] = { 'A' , '\\0' } ; char b [ 2 ] = { 'B' , '\\0' } ;";
        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl10() {
        // ticket #732
        const char code[] = "char a [ 2 ] = { '-' } ; memset ( a , '-' , sizeof ( a ) ) ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void vardecl11() {
        // ticket #1684
        const char code[] = "char a[5][8], b[5][8];";
        ASSERT_EQUALS("char a [ 5 ] [ 8 ] ; char b [ 5 ] [ 8 ] ;", tokenizeAndStringify(code));
    }

    void vardecl12() {
        const char code[] = "struct A { public: B a, b, c, d; };";
        ASSERT_EQUALS("struct A { public: B a ; B b ; B c ; B d ; } ;", tokenizeAndStringify(code));
    }

    void vardecl13() {
        const char code[] = "void f() {\n"
                            "    int a = (x < y) ? 1 : 0;\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\nint a ; a = ( x < y ) ? 1 : 0 ;\n}", tokenizeAndStringify(code));
    }

    void vardecl14() {
        const char code[] = "::std::tr1::shared_ptr<int> pNum1, pNum2;\n";
        ASSERT_EQUALS(":: std :: tr1 :: shared_ptr < int > pNum1 ; :: std :: tr1 :: shared_ptr < int > pNum2 ;", tokenizeAndStringify(code, false, false, Settings::Native, "test.cpp", false));
    }

    void vardecl15() {
        const char code[] = "const char x[] = \"foo\", y[] = \"bar\";\n";
        ASSERT_EQUALS("const char x [ 4 ] = \"foo\" ; const char y [ 4 ] = \"bar\" ;", tokenizeAndStringify(code));
    }

    void vardecl16() {
        {
            const char code[] = "const a::b<c,d(e),f>::g::h<i>::l *x [] = foo(),y [][] = bar();\n";
            ASSERT_EQUALS("const a :: b < c , d ( e ) , f > :: g :: h < i > :: l * x [ ] = foo ( ) ; "
                          "const a :: b < c , d ( e ) , f > :: g :: h < i > :: l y [ ] [ ] = bar ( ) ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "const ::b<c,d(e),f>::g::h<i>::l *x [] = foo(),y [][] = bar();\n";
            ASSERT_EQUALS("const :: b < c , d ( e ) , f > :: g :: h < i > :: l * x [ ] = foo ( ) ; "
                          "const :: b < c , d ( e ) , f > :: g :: h < i > :: l y [ ] [ ] = bar ( ) ;", tokenizeAndStringify(code));
        }
    }

    void vardecl17() {
        const char code[] = "a < b > :: c :: d :: e < f > x = foo(), y = bar();\n";
        ASSERT_EQUALS("a < b > :: c :: d :: e < f > x ; x = foo ( ) ; "
                      "a < b > :: c :: d :: e < f > y ; y = bar ( ) ;", tokenizeAndStringify(code));
    }

    void vardecl18() {
        const char code[] = "void f() {\n"
                            "    g((double)v1*v2, v3, v4);\n"
                            "}\n";

        ASSERT_EQUALS("void f ( ) {\n"
                      "g ( ( double ) v1 * v2 , v3 , v4 ) ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void vardecl19() {
        {
            const char code[] = "void func(in, r, m)\n"
                                "int in;"
                                "int r,m;"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void func (\n"
                          "int in ,\n"
                          "int r ,\n"
                          "int m )\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(r,f)\n"
                                "char *r;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r )\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(f)\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f ( )\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(f,r)\n"
                                "char *r;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r )\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(r,f,s)\n"
                                "char *r;\n"
                                "char *s;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r ,\n"
                          "\n"
                          "char * s )\n"
                          "\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(r,s,t)\n"
                                "char *r,*s,*t;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r ,\n"
                          "char * s ,\n"
                          "char * t )\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(a, b) register char *a, *b;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f ( char * a , char * b )\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
    }

    void vardecl20() {
        // #3700
        const char code[] = "void a::b() const\n"
                            "{\n"
                            "    register const int X = 0;\n"
                            "}\n";
        ASSERT_EQUALS("void a :: b ( ) const\n"
                      "{\n"
                      "const int X = 0 ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void vardecl21() { // type in namespace
        // #4042 - a::b const *p = 0;
        const char code1[] = "void f() {\n"
                             "    a::b const *p = 0;\n"
                             "}\n";
        ASSERT_EQUALS("void f ( ) {\n"
                      "const a :: b * p ; p = 0 ;\n"
                      "}"
                      , tokenizeAndStringify(code1));

        // #4226 - ::a::b const *p = 0;
        const char code2[] = "void f() {\n"
                             "    ::a::b const *p = 0;\n"
                             "}\n";
        ASSERT_EQUALS("void f ( ) {\n"
                      "const :: a :: b * p ; p = 0 ;\n"
                      "}"
                      , tokenizeAndStringify(code2));
    }

    void vardecl22() {  // #4211 - segmentation fault
        tokenizeAndStringify("A<B<C<int>> >* p = 0;");
    }

    void vardecl23() {  // #4276 - segmentation fault
        ASSERT_THROW(tokenizeAndStringify("class a { protected : template < class int x = 1 ; public : int f ( ) ; }"), InternalError);
    }

    void vardecl24() {  // #4187 - variable declaration within lambda function
        const char code1[] = "void f() {\n"
                             "    std::for_each(ints.begin(), ints.end(), [](int val)\n"
                             "    {\n"
                             "        int temp = 0;\n"
                             "    });\n"
                             "}";

        const char expected1[] = "void f ( ) {\n"
                                 "std :: for_each ( ints . begin ( ) , ints . end ( ) , [ ] ( int val )\n"
                                 "{\n"
                                 "int temp ; temp = 0 ;\n"
                                 "} ) ;\n"
                                 "}";

        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1));

        const char code2[] = "void f(int j) {\n"
                             "    g( [](){int temp = 0;} , j );\n"
                             "}";

        const char expected2[] = "void f ( int j ) {\n"
                                 "g ( [ ] ( ) { int temp ; temp = 0 ; } , j ) ;\n"
                                 "}";

        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));
    }

    void vardecl25() {  // #4799 - segmentation fault
        tokenizeAndStringify("void A::func(P g) const {}\n"
                             "void A::a() {\n"
                             "   b = new d(  [this]( const P & p) -> double { return this->func(p);}  );\n"
                             "}");
    }

    void vardecl26() { // #5907
        const char code[] = "extern int *new, obj, player;";
        const char expected[] = "extern int * new ; extern int obj ; extern int player ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void vardecl27() { // #7850
        const char code[] = "extern int foo(char);\n"
                            "void* class(char c) {\n"
                            "  if (foo(c))\n"
                            "    return 0;\n"
                            "  return 0;\n"
                            "}";
        tokenizeAndStringify(code, /*simplify=*/false, /*expand=*/true, Settings::Native, "test.c");
    }

    void volatile_variables() {
        const char code[] = "volatile int a=0;\n"
                            "volatile int b=0;\n"
                            "volatile int c=0;\n";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("volatile int a ; a = 0 ;\nvolatile int b ; b = 0 ;\nvolatile int c ; c = 0 ;", actual);
    }


    void simplifyKeyword() {
        {
            const char code[] = "void f (int a [ static 5] );";
            ASSERT_EQUALS("void f ( int a [ 5 ] ) ;", tokenizeAndStringify(code));
        }
        {
            const char in4 [] = "struct B final : A { void foo(); };";
            const char out4 [] = "struct B : A { void foo ( ) ; } ;";
            ASSERT_EQUALS(out4, tokenizeAndStringify(in4));

            const char in5 [] = "struct ArrayItemsValidator final {\n"
                                "    SchemaError validate() const override {\n"
                                "        for (; pos < value.size(); ++pos) {\n"
                                "        }\n"
                                "        return none;\n"
                                "    }\n"
                                "};\n";
            const char out5 [] =
                "struct ArrayItemsValidator {\n"
                "SchemaError validate ( ) const override {\n"
                "for ( ; pos < value . size ( ) ; ++ pos ) {\n"
                "}\n"
                "return none ;\n"
                "}\n"
                "} ;";

            ASSERT_EQUALS(out5, tokenizeAndStringify(in5));
        }
        {
            // Ticket #8679
            const char code[] = "thread_local void *thread_local_var; "
                                "__thread void *thread_local_var_2;";
            ASSERT_EQUALS("void * thread_local_var ; "
                          "void * thread_local_var_2 ;", tokenizeAndStringify(code));
        }
    }

    /**
     * tokenize "signed i" => "signed int i"
     */
    void signed1() {
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
    void unsigned1() {
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

    void unsigned2() {
        const char code[] = "i = (unsigned)j;";
        const char expected[] = "i = ( unsigned int ) j ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    // simplify "unsigned" when using templates..
    void unsigned3() {
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

    void simplifyStdType() { // #4947, #4950, #4951
        // usigned long long
        {
            const char code[] = "long long unsigned int x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "long long int unsigned x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned long long int x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned int long long x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int unsigned long long x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int long long unsigned x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        // signed long long
        {
            const char code[] = "long long signed int x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "long long int signed x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed long long int x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed int long long x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int signed long long x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int long long signed x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        // usigned short
        {
            const char code[] = "short unsigned int x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "short int unsigned x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned short int x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned int short x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int unsigned short x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int short unsigned x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        // signed short
        {
            const char code[] = "short signed int x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "short int signed x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed short int x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed int short x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int signed short x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int short signed x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned static short const int i;";
            const char expected[] = "static const unsigned short i ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "float complex x;";
            const char expected[] = "_Complex float x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "complex float x;";
            const char expected[] = "_Complex float x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "complex long double x;";
            const char expected[] = "_Complex long double x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "long double complex x;";
            const char expected[] = "_Complex long double x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "double complex;";
            const char expected[] = "double complex ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
    }

    void createLinks() {
        {
            const char code[] = "class A{\n"
                                " void f() {}\n"
                                "};";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            // A body {}
            ASSERT_EQUALS(true, tok->linkAt(2) == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(2));

            // f body {}
            ASSERT_EQUALS(true, tok->linkAt(7) == tok->tokAt(8));
            ASSERT_EQUALS(true, tok->linkAt(8) == tok->tokAt(7));

            // f ()
            ASSERT_EQUALS(true, tok->linkAt(5) == tok->tokAt(6));
            ASSERT_EQUALS(true, tok->linkAt(6) == tok->tokAt(5));

            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "void f(){\n"
                                " char a[10];\n"
                                " char *b ; b = new char[a[0]];\n"
                                "};";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            // a[10]
            ASSERT_EQUALS(true, tok->linkAt(7) == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(7));

            // new char[]
            ASSERT_EQUALS(true, tok->linkAt(19) == tok->tokAt(24));
            ASSERT_EQUALS(true, tok->linkAt(24) == tok->tokAt(19));

            // a[0]
            ASSERT_EQUALS(true, tok->linkAt(21) == tok->tokAt(23));
            ASSERT_EQUALS(true, tok->linkAt(23) == tok->tokAt(21));

            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "void f(){\n"
                                " foo(g());\n"
                                "};";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            // foo(
            ASSERT_EQUALS(true, tok->linkAt(6) == tok->tokAt(10));
            ASSERT_EQUALS(true, tok->linkAt(10) == tok->tokAt(6));

            // g(
            ASSERT_EQUALS(true, tok->linkAt(8) == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(8));

            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "bool foo(C<z> a, bar<int, x<float>>& f, int b) {\n"
                                "    return(a<b && b>f);\n"
                                "}";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            // template<
            ASSERT_EQUALS(true, tok->tokAt(6) == tok->linkAt(4));
            ASSERT_EQUALS(true, tok->tokAt(4) == tok->linkAt(6));

            // bar<
            ASSERT_EQUALS(true, tok->tokAt(17) == tok->linkAt(10));
            ASSERT_EQUALS(true, tok->tokAt(10) == tok->linkAt(17));

            // x<
            ASSERT_EQUALS(true, tok->tokAt(16) == tok->linkAt(14));
            ASSERT_EQUALS(true, tok->tokAt(14) == tok->linkAt(16));

            // a<b && b>f
            ASSERT_EQUALS(true, 0 == tok->linkAt(28));
            ASSERT_EQUALS(true, 0 == tok->linkAt(32));

            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "void foo() {\n"
                                "    return static_cast<bar>(a);\n"
                                "}";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            // static_cast<
            ASSERT_EQUALS(true, tok->tokAt(9) == tok->linkAt(7));
            ASSERT_EQUALS(true, tok->tokAt(7) == tok->linkAt(9));

            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "void foo() {\n"
                                "    nvwa<(x > y)> ERROR_nnn;\n"
                                "}";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            // nvwa<(x > y)>
            ASSERT_EQUALS(true, tok->tokAt(12) == tok->linkAt(6));
            ASSERT_EQUALS(true, tok->tokAt(6) == tok->linkAt(12));

            ASSERT_EQUALS("", errout.str());
        }

        {
            // #4860
            const char code[] = "class A : public B<int> {};";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            // B<..>
            ASSERT_EQUALS(true, tok->tokAt(5) == tok->linkAt(7));
            ASSERT_EQUALS(true, tok->linkAt(5) == tok->tokAt(7));

            ASSERT_EQUALS("", errout.str());
        }

        {
            // #4860
            const char code[] = "Bar<Typelist< int, Typelist< int, Typelist< int, FooNullType>>>>::set(1, 2, 3);";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->tokAt(1) == tok->linkAt(18));
            ASSERT_EQUALS(true, tok->tokAt(3) == tok->linkAt(17));
            ASSERT_EQUALS(true, tok->tokAt(7) == tok->linkAt(16));
            ASSERT_EQUALS(true, tok->tokAt(11) == tok->linkAt(15));

            ASSERT_EQUALS("", errout.str());
        }

        {
            // #5627
            const char code[] = "new Foo<Bar>[10];";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->tokAt(2) == tok->linkAt(4));
            ASSERT_EQUALS(true, tok->tokAt(4) == tok->linkAt(2));
            ASSERT_EQUALS(true, tok->tokAt(5) == tok->linkAt(7));
            ASSERT_EQUALS(true, tok->tokAt(7) == tok->linkAt(5));

            ASSERT_EQUALS("", errout.str());
        }
        {
            // #6242
            const char code[] = "func = integral_<uchar, int, double>;";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->tokAt(3) == tok->linkAt(9));
            ASSERT_EQUALS(true, tok->linkAt(3) == tok->tokAt(9));

            ASSERT_EQUALS("", errout.str());
        }

        {
            // if (a < b || c > d) { }
            const char code[] = "if (a < b || c > d);";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(3) == nullptr);
        }

        {
            // if (a < ... > d) { }
            const char code[] = "if (a < b || c == 3 || d > e);";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(3) == nullptr);
        }

        {
            // template
            const char code[] = "a<b==3 || c> d;";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(7));
        }

        {
            // template
            const char code[] = "a<b || c==4> d;";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(7));
        }

        {
            const char code[] = "template < f = b || c > struct S;";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(7));
            ASSERT_EQUALS(true, tok->tokAt(1) == tok->linkAt(7));
        }

        {
            const char code[] = "struct A : B<c&&d> {};";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(4) == tok->tokAt(8));
            ASSERT_EQUALS(true, tok->tokAt(4) == tok->linkAt(8));
        }

        {
            const char code[] = "Data<T&&>;";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(4));
            ASSERT_EQUALS(true, tok->tokAt(1) == tok->linkAt(4));
        }

        {
            // #6601
            const char code[] = "template<class R> struct FuncType<R(&)()> : FuncType<R()> { };";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(4)); // <class R>
            ASSERT_EQUALS(true, tok->linkAt(7) == tok->tokAt(14)); // <R(&)()>
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(11)); // (&)
            ASSERT_EQUALS(true, tok->linkAt(12) == tok->tokAt(13)); // ()
            ASSERT_EQUALS(true, tok->linkAt(17) == tok->tokAt(21)); // <R()>
            ASSERT_EQUALS(true, tok->linkAt(19) == tok->tokAt(20)); // ()
            ASSERT_EQUALS(true, tok->linkAt(22) == tok->tokAt(23)); // {}
        }
    }

    void createLinks2() {
        {
            // #7158
            const char code[] = "enum { value = boost::mpl::at_c<B, C> };";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok = Token::findsimplematch(tokenizer.tokens(), "<");
            ASSERT_EQUALS(true, tok->link() == tok->tokAt(4));
            ASSERT_EQUALS(true, tok->linkAt(4) == tok);
        }

        {
            // #7865
            const char code[] = "template <typename T, typename U>\n"
                                "struct CheckedDivOp< T, U, typename std::enable_if<std::is_floating_point<T>::value || std::is_floating_point<U>::value>::type> {\n"
                                "};\n";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "struct")->tokAt(2);
            const Token *tok2 = Token::findsimplematch(tokenizer.tokens(), "{")->previous();
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #7975
            const char code[] = "template <class C> X<Y&&Z, C*> copy() {};\n";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "< Y");
            const Token *tok2 = Token::findsimplematch(tok1, "> copy");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #8006
            const char code[] = "C<int> && a = b;";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok1 = tokenizer.tokens()->next();
            const Token *tok2 = tok1->tokAt(2);
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #8115
            const char code[] = "void Test(C<int> && c);";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "<");
            const Token *tok2 = tok1->tokAt(2);
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }
        {
            // #8654
            const char code[] = "template<int N> struct A {}; "
                                "template<int... Ns> struct foo : A<Ns>... {};";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            const Token *A = Token::findsimplematch(tokenizer.tokens(), "A <");
            ASSERT_EQUALS(true, A->next()->link() == A->tokAt(3));
        }
        {
            // #8851
            const char code[] = "template<typename std::enable_if<!(std::value1) && std::value2>::type>"
                                "void basic_json() {}";
            errout.str("");
            Tokenizer tokenizer(&settings0, this);
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
            ASSERT_EQUALS(true, Token::simpleMatch(tokenizer.tokens()->next()->link(), "> void"));
        }
    }

    void simplifyString() {
        errout.str("");
        Tokenizer tokenizer(&settings0, this);
        ASSERT_EQUALS("\"abc\"", tokenizer.simplifyString("\"abc\""));
        ASSERT_EQUALS("\"\n\"", tokenizer.simplifyString("\"\\xa\""));
        ASSERT_EQUALS("\"3\"", tokenizer.simplifyString("\"\\x33\""));
        ASSERT_EQUALS("\"33\"", tokenizer.simplifyString("\"\\x333\""));

        ASSERT_EQUALS("\"a\"", tokenizer.simplifyString("\"\\x61\""));
        ASSERT_EQUALS("\"\n1\"", tokenizer.simplifyString("\"\\0121\""));

        ASSERT_EQUALS("\"3\"", tokenizer.simplifyString("\"\\x33\""));
        ASSERT_EQUALS("\" 0\"", tokenizer.simplifyString("\"\\0400\""));

        ASSERT_EQUALS("\"\\nhello\"", tokenizer.simplifyString("\"\\nhello\""));

        ASSERT_EQUALS("\"aaa\"", tokenizer.simplifyString("\"\\x61\\x61\\x61\""));
        ASSERT_EQUALS("\"\n1\n1\n1\"", tokenizer.simplifyString("\"\\0121\\0121\\0121\""));

        ASSERT_EQUALS("\"\\\\x61\"", tokenizer.simplifyString("\"\\\\x61\""));
        ASSERT_EQUALS("\"b\"", tokenizer.simplifyString("\"\\x62\""));
        ASSERT_EQUALS("\" 7\"", tokenizer.simplifyString("\"\\0407\""));

        // terminate a string at null character.
        ASSERT_EQUALS(std::string("\"a") + '\0' + "\"", tokenizer.simplifyString("\"a\\0\""));
    }

    void simplifyConst() {
        ASSERT_EQUALS("void foo ( ) { const int x ; }",
                      tokenizeAndStringify("void foo(){ int const x;}"));

        ASSERT_EQUALS("void foo ( ) { { } const long x ; }",
                      tokenizeAndStringify("void foo(){ {} long const x;}"));

        ASSERT_EQUALS("void foo ( int b , const unsigned int x ) { }",
                      tokenizeAndStringify("void foo(int b,unsigned const x){}"));

        ASSERT_EQUALS("void foo ( ) { bar ( ) ; const char x ; }",
                      tokenizeAndStringify("void foo(){ bar(); char const x;}"));

        ASSERT_EQUALS("void foo ( const char x ) { }",
                      tokenizeAndStringify("void foo(char const x){}"));

        ASSERT_EQUALS("void foo ( int b , const char x ) { }",
                      tokenizeAndStringify("void foo(int b,char const x){}"));

        ASSERT_EQUALS("void foo ( ) { int * const x ; }",
                      tokenizeAndStringify("void foo(){ int * const x;}"));

        ASSERT_EQUALS("const int foo ( ) ;", tokenizeAndStringify("int const foo ();"));

        ASSERT_EQUALS("const int x ;", tokenizeAndStringify("int const x;"));
        ASSERT_EQUALS("const unsigned int x ;", tokenizeAndStringify("unsigned const x;"));
        ASSERT_EQUALS("const struct X x ;", tokenizeAndStringify("struct X const x;"));
    }

    void switchCase() {
        ASSERT_EQUALS("void foo ( int i ) { switch ( i ) { case -1 : ; break ; } }",
                      tokenizeAndStringify("void foo (int i) { switch(i) { case -1: break; } }"));
        //ticket #3227
        ASSERT_EQUALS("void foo ( ) { switch ( n ) { label : ; case 1 : ; label1 : ; label2 : ; break ; } }",
                      tokenizeAndStringify("void foo(){ switch (n){ label: case 1: label1: label2: break; }}"));
        //ticket #8345
        ASSERT_EQUALS("void foo ( ) { switch ( 0 ) { case 0 : ; default : ; } }",
                      tokenizeAndStringify("void foo () { switch(0) case 0 : default : ; }"));
        //ticket #8477
        ASSERT_EQUALS("void foo ( ) { enum Anonymous0 : int { Six = 6 } ; return Six ; }",
                      tokenizeAndStringify("void foo () { enum : int { Six = 6 } ; return Six ; }"));
        // ticket #8281
        tokenizeAndStringify("void lzma_decode(int i) { "
                             "  bool state; "
                             "  switch (i) "
                             "  while (true) { "
                             "     state=false; "
                             "   case 1: "
                             "      ; "
                             "  }"
                             "}");
        // ticket #8417
        tokenizeAndStringify("void printOwnedAttributes(int mode) { "
                             "  switch(mode) case 0: { break; } "
                             "}");
        ASSERT_THROW(tokenizeAndStringify("void printOwnedAttributes(int mode) { "
                                          "  switch(mode) case 0: { break; } case 1: ; "
                                          "}"),
                     InternalError);
    }

    void simplifyPointerToStandardType() {
        // Pointer to standard type
        ASSERT_EQUALS("char buf [ 100 ] ; readlink ( path , buf , 99 ) ;",
                      tokenizeAndStringify("char buf[100] ; readlink(path, &buf[0], 99);",
                                           false, true, Settings::Native, "test.c"));

        ASSERT_EQUALS("void foo ( char * c ) { if ( 1 == ( 1 & c [ 0 ] ) ) { } }",
                      tokenizeAndStringify("void foo(char *c) { if (1==(1 & c[0])) {} }",
                                           false, true, Settings::Native, "test.c"));

        // Simplification of unknown type - C only
        ASSERT_EQUALS("foo data [ 100 ] ; something ( foo ) ;",
                      tokenizeAndStringify("foo data[100]; something(&foo[0]);", false, true, Settings::Native, "test.c"));

        // C++: No pointer simplification
        ASSERT_EQUALS("foo data [ 100 ] ; something ( & foo [ 0 ] ) ;",
                      tokenizeAndStringify("foo data[100]; something(&foo[0]);"));
    }

    void functionpointer1() {
        ASSERT_EQUALS("void * f ;", tokenizeAndStringify("void (*f)();"));
        ASSERT_EQUALS("void * * f ;", tokenizeAndStringify("void *(*f)();"));
        ASSERT_EQUALS("unsigned int * f ;", tokenizeAndStringify("unsigned int (*f)();"));
        ASSERT_EQUALS("unsigned int * * f ;", tokenizeAndStringify("unsigned int * (*f)();"));
    }

    void functionpointer2() {
        const char code[] = "typedef void (* PF)();"
                            "void f1 ( ) { }"
                            "PF pf = &f1;"
                            "PF pfs[] = { &f1, &f1 };";
        const char expected[] = "void f1 ( ) { } "
                                "void * pf ; pf = & f1 ; "
                                "void * pfs [ 2 ] = { & f1 , & f1 } ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void functionpointer3() {
        // Related with ticket #2873
        const char code[] = "void f() {\n"
                            "(void)(xy(*p)(0);)"
                            "\n}";
        const char expected[] = "void f ( ) {\n"
                                "( void ) ( xy ( * p ) ( 0 ) ; )\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void functionpointer4() {
        const char code[] = "struct S\n"
                            "{\n"
                            "    typedef void (*FP)();\n"
                            "    virtual FP getFP();\n"
                            "    virtual void execute();\n"
                            "};\n"
                            "void f() {\n"
                            "  int a[9];\n"
                            "}\n";
        const char expected[] = "1: struct S\n"
                                "2: {\n"
                                "3:\n"
                                "4: virtual void * getFP ( ) ;\n"
                                "5: virtual void execute ( ) ;\n"
                                "6: } ;\n"
                                "7: void f ( ) {\n"
                                "8: int a@1 [ 9 ] ;\n"
                                "9: }\n";
        ASSERT_EQUALS(expected, tokenizeDebugListing(code, false));
    }

    void functionpointer5() {
        const char code[] = ";void (*fp[])(int a) = {0,0,0};";
        const char expected[] = "1: ; void * fp@1 [ 3 ] = { 0 , 0 , 0 } ;\n";
        ASSERT_EQUALS(expected, tokenizeDebugListing(code, false));
    }

    void functionpointer6() {
        const char code1[] = ";void (*fp(f))(int);";
        const char expected1[] = "1: ; void * fp ( f ) ;\n"; // No varId - it could be a function
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1, false));

        const char code2[] = ";std::string (*fp(f))(int);";
        const char expected2[] = "1: ; std :: string * fp ( f ) ;\n";
        ASSERT_EQUALS(expected2, tokenizeDebugListing(code2, false));
    }

    void functionpointer7() {
        const char code1[] = "void (X::*y)();";
        const char expected1[] = "1: void * y@1 ;\n";
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1, false));
    }

    void functionpointer8() {
        const char code1[] = "int (*f)() throw(int);";
        const char expected1[] = "1: int * f@1 ;\n";
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1, false));
    }

    void functionpointer9() { // function call with function pointer
        const char code1[] = "int f() { (*f)(); }";
        const char expected1[] = "1: int f ( ) { ( * f ) ( ) ; }\n";
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1, false));

        const char code2[] = "int f() { return (*f)(); }";
        const char expected2[] = "1: int f ( ) { return ( * f ) ( ) ; }\n";
        ASSERT_EQUALS(expected2, tokenizeDebugListing(code2, false));

        const char code3[] = "int f() { throw (*f)(); }";
        const char expected3[] = "1: int f ( ) { throw ( * f ) ( ) ; }\n";
        ASSERT_EQUALS(expected3, tokenizeDebugListing(code3, false));
    }

    void removeRedundantAssignment() {
        ASSERT_EQUALS("void f ( ) { }", tokenizeAndStringify("void f() { int *p, *q; p = q; }", true));
        ASSERT_EQUALS("void f ( ) { }", tokenizeAndStringify("void f() { int *p = 0, *q; p = q; }", true));
        ASSERT_EQUALS("int f ( int * x ) { return * x ; }", tokenizeAndStringify("int f(int *x) { return *x; }", true));
    }

    void removedeclspec() {
        ASSERT_EQUALS("a b", tokenizeAndStringify("a __declspec ( dllexport ) b"));
        ASSERT_EQUALS("a b", tokenizeAndStringify("a _declspec ( dllexport ) b"));
        ASSERT_EQUALS("int a ;", tokenizeAndStringify("__declspec(thread) __declspec(align(32)) int a;"));
        ASSERT_EQUALS("int i ;", tokenizeAndStringify("__declspec(allocate(\"mycode\")) int i;"));
        ASSERT_EQUALS("struct IUnknown ;", tokenizeAndStringify("struct __declspec(uuid(\"00000000-0000-0000-c000-000000000046\")) IUnknown;"));
        ASSERT_EQUALS("__property int x [ ] ;", tokenizeAndStringify("__declspec(property(get=GetX, put=PutX)) int x[];"));
    }

    void removeattribute() {
        ASSERT_EQUALS("short array [ 3 ] ;", tokenizeAndStringify("short array[3] __attribute__ ((aligned));"));
        ASSERT_EQUALS("int x [ 2 ] ;", tokenizeAndStringify("int x[2] __attribute__ ((packed));"));
        ASSERT_EQUALS("int vecint ;", tokenizeAndStringify("int __attribute__((mode(SI))) __attribute__((vector_size (16))) vecint;"));

        // alternate spelling #5328
        ASSERT_EQUALS("short array [ 3 ] ;", tokenizeAndStringify("short array[3] __attribute ((aligned));"));
        ASSERT_EQUALS("int x [ 2 ] ;", tokenizeAndStringify("int x[2] __attribute ((packed));"));
        ASSERT_EQUALS("int vecint ;", tokenizeAndStringify("int __attribute((mode(SI))) __attribute((vector_size (16))) vecint;"));

        ASSERT_EQUALS("struct Payload_IR_config { uint8_t tap [ 16 ] ; } ;", tokenizeAndStringify("struct __attribute__((packed, gcc_struct)) Payload_IR_config { uint8_t tap[16]; };"));
    }

    void functionAttributeBefore() {
        const char code[] = "void __attribute__((pure)) __attribute__((nothrow)) __attribute__((const)) func1();\n"
                            "void __attribute__((__pure__)) __attribute__((__nothrow__)) __attribute__((__const__)) func2();\n"
                            "void __attribute__((nothrow)) __attribute__((pure)) __attribute__((const)) func3();\n"
                            "void __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__const__)) func4();\n"
                            "void __attribute__((noreturn)) func5();";
        const char expected[] = "void func1 ( ) ; void func2 ( ) ; void func3 ( ) ; void func4 ( ) ; void func5 ( ) ;";

        errout.str("");

        // tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(0, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");
        const Token * func2 = Token::findsimplematch(tokenizer.tokens(), "func2");
        const Token * func3 = Token::findsimplematch(tokenizer.tokens(), "func3");
        const Token * func4 = Token::findsimplematch(tokenizer.tokens(), "func4");
        const Token * func5 = Token::findsimplematch(tokenizer.tokens(), "func5");

        ASSERT(func1 && func1->isAttributePure() && func1->isAttributeNothrow() && func1->isAttributeConst());
        ASSERT(func2 && func2->isAttributePure() && func2->isAttributeNothrow() && func2->isAttributeConst());
        ASSERT(func3 && func3->isAttributePure() && func3->isAttributeNothrow() && func3->isAttributeConst());
        ASSERT(func4 && func4->isAttributePure() && func4->isAttributeNothrow() && func4->isAttributeConst());
        ASSERT(func5 && func5->isAttributeNoreturn());
    }

    void functionAttributeAfter() {
        const char code[] = "void func1() __attribute__((pure)) __attribute__((nothrow)) __attribute__((const));\n"
                            "void func2() __attribute__((__pure__)) __attribute__((__nothrow__)) __attribute__((__const__));\n"
                            "void func3() __attribute__((nothrow)) __attribute__((pure)) __attribute__((const));\n"
                            "void func4() __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__const__));"
                            "void func5() __attribute__((noreturn));";
        const char expected[] = "void func1 ( ) ; void func2 ( ) ; void func3 ( ) ; void func4 ( ) ; void func5 ( ) ;";

        errout.str("");

        // tokenize..
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(0, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");
        const Token * func2 = Token::findsimplematch(tokenizer.tokens(), "func2");
        const Token * func3 = Token::findsimplematch(tokenizer.tokens(), "func3");
        const Token * func4 = Token::findsimplematch(tokenizer.tokens(), "func4");
        const Token * func5 = Token::findsimplematch(tokenizer.tokens(), "func5");

        ASSERT(func1 && func1->isAttributePure() && func1->isAttributeNothrow() && func1->isAttributeConst());
        ASSERT(func2 && func2->isAttributePure() && func2->isAttributeNothrow() && func2->isAttributeConst());
        ASSERT(func3 && func3->isAttributePure() && func3->isAttributeNothrow() && func3->isAttributeConst());
        ASSERT(func4 && func4->isAttributePure() && func4->isAttributeNothrow() && func4->isAttributeConst());
        ASSERT(func5 && func5->isAttributeNoreturn());
    }

    void cpp03template1() {
        {
            const char *code = "template<typename> struct extent {};";
            ASSERT_EQUALS("template < typename > struct extent { } ;", tokenizeAndStringify(code));
        }
        {
            const char *code = "template<typename> struct extent;";
            ASSERT_EQUALS("template < typename > struct extent ;", tokenizeAndStringify(code));
        }
        {
            const char *code = "template<typename, unsigned = 0> struct extent;";
            ASSERT_EQUALS("template < typename , unsigned int = 0 > struct extent ;", tokenizeAndStringify(code));
        }
    }

    void cpp0xtemplate1() {
        const char *code = "template <class T>\n"
                           "void fn2 (T t = []{return 1;}())\n"
                           "{}\n"
                           "int main()\n"
                           "{\n"
                           "  fn2<int>();\n"
                           "}\n";
        ASSERT_EQUALS("void fn2<int> ( int t = [ ] { return 1 ; } ( ) ) ;\n"
                      "\n"
                      "\n"
                      "int main ( )\n"
                      "{\n"
                      "fn2<int> ( ) ;\n"
                      "} void fn2<int> ( int t = [ ] { return 1 ; } ( ) )\n"
                      "{ }", tokenizeAndStringify(code));
    }

    void cpp0xtemplate2() {
        // tokenize ">>" into "> >"
        const char *code = "list<list<int>> ints;\n";
        ASSERT_EQUALS("list < list < int > > ints ;", tokenizeAndStringify(code));
    }

    void cpp0xtemplate3() {
        // #2549
        const char *code = "template<class T, T t = (T)0>\n"
                           "struct S\n"
                           "{};\n"
                           "S<int> s;\n";
        TODO_ASSERT_EQUALS("S<int,(int)0> s ; struct S<int,(int)0> { } ;",   // wanted result
                           "template < class T , T t >\n"
                           "struct S\n"
                           "{ } ;\n"
                           "S < int , ( T ) 0 > s ;",     // current result
                           tokenizeAndStringify(code));
    }

    void cpp0xtemplate4() { // #6181, #6354, #6414
        tokenizeAndStringify("class A; "
                             "template <class T> class Disposer; "
                             "template <typename T, class D = Disposer<T>> class Shim {}; "
                             "class B : public Shim<A> {};");
        tokenizeAndStringify("template <class ELFT> class ELFObjectImage {}; "
                             "ObjectImage *createObjectImage() { "
                             "  return new ELFObjectImage<ELFType<little>>(Obj); "
                             "} "
                             "void resolveX86_64Relocation() { "
                             "  reinterpret_cast<int>(0); "
                             "}");
        tokenizeAndStringify("template<typename value_type, typename function_type> "
                             "value_type Base(const value_type x, const value_type dx, function_type func, int type_deriv) { "
                             "   return 0.0; "
                             "}; "
                             "namespace { "
                             "  template<class DC> class C { "
                             "    void Fun(int G, const double x); "
                             "  }; "
                             "  template<class DC> void C<DC>::Fun(int G, const double x) {"
                             "    Base<double, CDFFunctor<DC>>(2, 2, f, 0); "
                             "  }; "
                             "  template<class DC> class C2 {}; "
                             "}");
    }

    void cpp14template() { // Ticket #6708
        tokenizeAndStringify("template <typename T> "
                             "decltype(auto) forward(T& t) { return 0; }");
    }

    void arraySize() {
        ASSERT_EQUALS("; int a [ 3 ] = { 1 , 2 , 3 } ;", tokenizeAndStringify(";int a[]={1,2,3};"));
        ASSERT_EQUALS("; int a [ 3 ] = { 1 , 2 , 3 } ;", tokenizeAndStringify(";int a[]={1,2,3,};"));
        ASSERT_EQUALS("; foo a [ 3 ] = { { 1 , 2 } , { 3 , 4 } , { 5 , 6 } } ;", tokenizeAndStringify(";foo a[]={{1,2},{3,4},{5,6}};"));
        ASSERT_EQUALS("; int a [ 1 ] = { foo < bar1 , bar2 > ( 123 , 4 ) } ;", tokenizeAndStringify(";int a[]={foo<bar1,bar2>(123,4)};"));
        ASSERT_EQUALS("; int a [ 2 ] = { b > c ? 1 : 2 , 3 } ;", tokenizeAndStringify(";int a[]={ b>c?1:2,3};"));
        ASSERT_EQUALS("int main ( ) { int a [ 2 ] = { b < c ? 1 : 2 , 3 } }", tokenizeAndStringify("int main(){int a[]={b<c?1:2,3}}"));
        ASSERT_EQUALS("; int a [ 3 ] = { ABC , 2 , 3 } ;", tokenizeAndStringify(";int a[]={ABC,2,3};"));
        ASSERT_EQUALS("; int a [ 3 ] = { [ 2 ] = 5 } ;", tokenizeAndStringify(";int a[]={ [2] = 5 };"));
        ASSERT_EQUALS("; int a [ 5 ] = { 1 , 2 , [ 2 ] = 5 , 3 , 4 } ;", tokenizeAndStringify(";int a[]={ 1, 2, [2] = 5, 3, 4 };"));
        ASSERT_EQUALS("; int a [ ] = { 1 , 2 , [ x ] = 5 , 3 , 4 } ;", tokenizeAndStringify(";int a[]={ 1, 2, [x] = 5, 3, 4 };"));
    }

    void labels() {
        ASSERT_EQUALS("void f ( ) { ab : ; a = 0 ; }", tokenizeAndStringify("void f() { ab: a=0; }"));
        //ticket #3176
        ASSERT_EQUALS("void f ( ) { ab : ; ( * func ) ( ) ; }", tokenizeAndStringify("void f() { ab: (*func)(); }"));
        //with '*' operator
        ASSERT_EQUALS("void f ( ) { ab : ; * b = 0 ; }", tokenizeAndStringify("void f() { ab: *b=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; * * b = 0 ; }", tokenizeAndStringify("void f() { ab: **b=0; }"));
        //with '&' operator
        ASSERT_EQUALS("void f ( ) { ab : ; & b = 0 ; }", tokenizeAndStringify("void f() { ab: &b=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; & ( b . x ) = 0 ; }", tokenizeAndStringify("void f() { ab: &(b->x)=0; }"));
        //with '(' parentheses
        ASSERT_EQUALS("void f ( ) { ab : ; * ( * b ) . x = 0 ; }", tokenizeAndStringify("void f() { ab: *(* b)->x=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; ( * * b ) . x = 0 ; }", tokenizeAndStringify("void f() { ab: (** b).x=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; & ( * b . x ) = 0 ; }", tokenizeAndStringify("void f() { ab: &(*b.x)=0; }"));
        //with '{' parentheses
        ASSERT_EQUALS("void f ( ) { ab : ; { b = 0 ; } }", tokenizeAndStringify("void f() { ab: {b=0;} }"));
        ASSERT_EQUALS("void f ( ) { ab : ; { * b = 0 ; } }", tokenizeAndStringify("void f() { ab: { *b=0;} }"));
        ASSERT_EQUALS("void f ( ) { ab : ; { & b = 0 ; } }", tokenizeAndStringify("void f() { ab: { &b=0;} }"));
        ASSERT_EQUALS("void f ( ) { ab : ; { & ( * b . x ) = 0 ; } }", tokenizeAndStringify("void f() { ab: {&(*b.x)=0;} }"));
        //with unhandled MACRO() code
        ASSERT_EQUALS("void f ( ) { MACRO ( ab : b = 0 ; , foo ) }", tokenizeAndStringify("void f() { MACRO(ab: b=0;, foo)}"));
        ASSERT_EQUALS("void f ( ) { MACRO ( bar , ab : { & ( * b . x ) = 0 ; } ) }", tokenizeAndStringify("void f() { MACRO(bar, ab: {&(*b.x)=0;})}"));
    }

    void simplifyInitVar() {
        {
            const char code[] = "int i ; int p(0);";
            ASSERT_EQUALS("int i ; int p ; p = 0 ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; int *p(0);";
            ASSERT_EQUALS("int i ; int * p ; p = 0 ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int p(0);";
            ASSERT_EQUALS("int p ; p = 0 ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int *p(0);";
            ASSERT_EQUALS("int * p ; p = 0 ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i ; int p(i);";
            ASSERT_EQUALS("int i ; int p ; p = i ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; int *p(&i);";
            ASSERT_EQUALS("int i ; int * p ; p = & i ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; void *p(&i);";
            ASSERT_EQUALS("int i ; void * p ; p = & i ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S *p(&s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S * p ; p = & s ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; S s; S *p(&s);";
            ASSERT_EQUALS("struct S { } ; S s ; S * p ; p = & s ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "union S { int i; float f; }; union S s; union S *p(&s);";
            ASSERT_EQUALS("union S { int i ; float f ; } ; union S s ; union S * p ; p = & s ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "union S { int i; float f; }; S s; S *p(&s);";
            ASSERT_EQUALS("union S { int i ; float f ; } ; S s ; S * p ; p = & s ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class C { }; class C c; class C *p(&c);";
            ASSERT_EQUALS("class C { } ; class C c ; class C * p ; p = & c ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class C { }; C c; C *p(&c);";
            ASSERT_EQUALS("class C { } ; C c ; C * p ; p = & c ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S s1(s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S s1 ( s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; S s; S s1(s);";
            ASSERT_EQUALS("struct S { } ; S s ; S s1 ( s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S s1(&s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S s1 ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "struct S { }; S s; S s1(&s);";
            ASSERT_EQUALS("struct S { } ; S s ; S s1 ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class S { int function(); };";
            ASSERT_EQUALS("class S { int function ( ) ; } ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class S { int function(void); };";
            ASSERT_EQUALS("class S { int function ( ) ; } ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class S { int function(int); };";
            ASSERT_EQUALS("class S { int function ( int ) ; } ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function(void);";
            ASSERT_EQUALS("int function ( ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function(int);";
            ASSERT_EQUALS("int function ( int ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "extern int function(void);";
            ASSERT_EQUALS("extern int function ( ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function1(void); int function2(void);";
            ASSERT_EQUALS("int function1 ( ) ; int function2 ( ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int function(A);";
            // We can't tell if this a function prototype or a variable without knowing
            // what A is. Since A is undefined, just leave it alone.
            ASSERT_EQUALS("int function ( A ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int i; int function(A);";
            ASSERT_EQUALS("int i ; int function ( A ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class A { } ; int foo(A);";
            ASSERT_EQUALS("class A { } ; int foo ( A ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "class A { } ; A a; int foo(a);";
            ASSERT_EQUALS("class A { } ; A a ; int foo ; foo = a ;", tokenizeAndStringify(code, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "int x(f());";
            ASSERT_EQUALS("int x ; x = f ( ) ;", tokenizeAndStringify(code, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "return doSomething(X), 0;";
            ASSERT_EQUALS("return doSomething ( X ) , 0 ;", tokenizeAndStringify(code, false));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "const int x(1);"
                                "const int y(2);"
                                "const int z((x+1)*y);"
                                "f(z);";
            ASSERT_EQUALS("f ( 4 ) ;", tokenizeAndStringify(code, true));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "const int x(1);"
                                "const int y(2);"
                                "const int z((x+1)*y);"
                                "f(&z);";
            ASSERT_EQUALS("const int z ( 4 ) ; f ( & z ) ;", tokenizeAndStringify(code, true));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "const bool x(true);"
                                "const bool y(!x);"
                                "f(y);";
            ASSERT_EQUALS("f ( false ) ;", tokenizeAndStringify(code, true));
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char code[] = "const bool x(true);"
                                "const bool y(!x);"
                                "f(&y);";
            ASSERT_EQUALS("const bool y ( false ) ; f ( & y ) ;", tokenizeAndStringify(code, true));
            ASSERT_EQUALS("", errout.str());
        }
    }

    void simplifyInitVar2() {
        // ticket #5131 - unsigned
        const char code[] = "void f() {\n"
                            "    unsigned int a(0),b(0);\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "unsigned int a ; a = 0 ; unsigned int b ; b = 0 ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyInitVar3() {
        const char code[] = "void f() {\n"
                            "    int *a(0),b(0);\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "int * a ; a = 0 ; int b ; b = 0 ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void bitfields1() {
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
        ASSERT_EQUALS("struct A { char x ; } ;", tokenizeAndStringifyWindows(code6,false, true, Settings::Win32A));

        const char code7[] = "struct A { __int16 x : 3; };";
        ASSERT_EQUALS("struct A { short x ; } ;", tokenizeAndStringifyWindows(code7,false, true, Settings::Win32A));

        const char code8[] = "struct A { __int32 x : 3; };";
        ASSERT_EQUALS("struct A { int x ; } ;", tokenizeAndStringifyWindows(code8,false, true, Settings::Win32A));

        const char code9[] = "struct A { __int64 x : 3; };";
        ASSERT_EQUALS("struct A { long long x ; } ;", tokenizeAndStringifyWindows(code9,false, true, Settings::Win32A));

        const char code10[] = "struct A { unsigned char x : 3; };";
        ASSERT_EQUALS("struct A { unsigned char x ; } ;", tokenizeAndStringify(code10,false));

        const char code11[] = "struct A { unsigned short x : 3; };";
        ASSERT_EQUALS("struct A { unsigned short x ; } ;", tokenizeAndStringify(code11,false));

        const char code12[] = "struct A { unsigned int x : 3; };";
        ASSERT_EQUALS("struct A { unsigned int x ; } ;", tokenizeAndStringify(code12,false));

        const char code13[] = "struct A { unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { unsigned long x ; } ;", tokenizeAndStringify(code13,false));

        const char code14[] = "struct A { unsigned __int8 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned char x ; } ;", tokenizeAndStringifyWindows(code14,false, true, Settings::Win32A));

        const char code15[] = "struct A { unsigned __int16 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned short x ; } ;", tokenizeAndStringifyWindows(code15,false, true, Settings::Win32A));

        const char code16[] = "struct A { unsigned __int32 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned int x ; } ;", tokenizeAndStringifyWindows(code16,false, true, Settings::Win32A));

        const char code17[] = "struct A { unsigned __int64 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned long long x ; } ;", tokenizeAndStringifyWindows(code17,false, true, Settings::Win32A));

        const char code18[] = "struct A { signed char x : 3; };";
        ASSERT_EQUALS("struct A { signed char x ; } ;", tokenizeAndStringify(code18,false));

        const char code19[] = "struct A { signed short x : 3; };";
        ASSERT_EQUALS("struct A { signed short x ; } ;", tokenizeAndStringify(code19,false));

        const char code20[] = "struct A { signed int x : 3; };";
        ASSERT_EQUALS("struct A { signed int x ; } ;", tokenizeAndStringify(code20,false));

        const char code21[] = "struct A { signed long x : 3; };";
        ASSERT_EQUALS("struct A { signed long x ; } ;", tokenizeAndStringifyWindows(code21,false));

        const char code22[] = "struct A { signed __int8 x : 3; };";
        ASSERT_EQUALS("struct A { signed char x ; } ;", tokenizeAndStringifyWindows(code22,false, true, Settings::Win32A));

        const char code23[] = "struct A { signed __int16 x : 3; };";
        ASSERT_EQUALS("struct A { signed short x ; } ;", tokenizeAndStringifyWindows(code23,false, true, Settings::Win32A));

        const char code24[] = "struct A { signed __int32 x : 3; };";
        ASSERT_EQUALS("struct A { signed int x ; } ;", tokenizeAndStringifyWindows(code24,false, true, Settings::Win32A));

        const char code25[] = "struct A { signed __int64 x : 3; };";
        ASSERT_EQUALS("struct A { signed long long x ; } ;", tokenizeAndStringifyWindows(code25,false, true, Settings::Win32A));
    }

    void bitfields2() {
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

    void bitfields3() {
        const char code1[] = "struct A { const int x : 3; };";
        ASSERT_EQUALS("struct A { const int x ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { const unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { const unsigned long x ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { public: const int x : 3; };";
        ASSERT_EQUALS("struct A { public: const int x ; } ;", tokenizeAndStringify(code3,false));

        const char code4[] = "struct A { public: const unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { public: const unsigned long x ; } ;", tokenizeAndStringify(code4,false));
    }

    void bitfields4() { // ticket #1956
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

    void bitfields5() { // ticket #1956
        const char code1[] = "struct RGB { unsigned int r : 3, g : 3, b : 2; };";
        ASSERT_EQUALS("struct RGB { unsigned int r ; unsigned int g ; unsigned int b ; } ;", tokenizeAndStringify(code1,false));

        const char code2[] = "struct A { int a : 3; int : 3; int c : 3; };";
        ASSERT_EQUALS("struct A { int a ; int c ; } ;", tokenizeAndStringify(code2,false));

        const char code3[] = "struct A { virtual void f() {} int f1 : 1; };";
        ASSERT_EQUALS("struct A { virtual void f ( ) { } int f1 ; } ;", tokenizeAndStringify(code3,false));
    }

    void bitfields6() { // ticket #2595
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

    void bitfields7() { // ticket #1987
        const char code[] = "typedef struct Descriptor {"
                            "    unsigned element_size: 8* sizeof( unsigned );"
                            "} Descriptor;";
        const char expected[] = "struct Descriptor { "
                                "unsigned int element_size ; "
                                "} ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code,false));
        ASSERT_EQUALS("", errout.str());
    }

    void bitfields8() {
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

    void bitfields9() { // ticket #2706
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

    void bitfields10() { // ticket #2737
        const char code[] = "{}"
                            "MACRO "
                            "default: { }"
                            ";";
        ASSERT_EQUALS("{ } MACRO default : { } ;", tokenizeAndStringify(code,false));
    }

    void bitfields12() { // ticket #3485 (segmentation fault)
        const char code[] = "{a:1;};\n";
        ASSERT_EQUALS("{ } ;", tokenizeAndStringify(code,false));
    }

    void bitfields13() { // ticket #3502 (segmentation fault)
        ASSERT_EQUALS("x y ;", tokenizeAndStringify("struct{x y:};\n",false));
    }

    void bitfields14() { // #4561 - crash for 'signals:'
        ASSERT_EQUALS("class x { signals : } ;", tokenizeAndStringify("class x { signals: };\n",false));
    }

    void bitfields15() { // #7747 - enum Foo {A,B}:4;
        ASSERT_EQUALS("struct AB {\n"
                      "enum Foo { A , B } ; enum Foo Anonymous ;\n"
                      "} ;",
                      tokenizeAndStringify("struct AB {\n"
                                           "  enum Foo {A,B} : 4;\n"
                                           "};"));
        ASSERT_EQUALS("struct AB {\n"
                      "enum Foo { A , B } ; enum Foo foo ;\n"
                      "} ;",
                      tokenizeAndStringify("struct AB {\n"
                                           "  enum Foo {A,B} foo : 4;\n"
                                           "};"));
    }

    void bitfields16() {
        const char code[] = "struct A { unsigned int x : 1; };";

        errout.str("");
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token *x = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT_EQUALS(1, x->bits());
    }

    void simplifyNamespaceStd() {
        const char *code, *expected;

        code = "map<foo, bar> m;"; // namespace std is not used
        ASSERT_EQUALS("map < foo , bar > m ;", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "map<foo, bar> m;";
        ASSERT_EQUALS("std :: map < foo , bar > m ;", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "string s;";
        ASSERT_EQUALS("std :: string s ;", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "void foo() {swap(a, b); }";
        ASSERT_EQUALS("void foo ( ) { std :: swap ( a , b ) ; }", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "void search() {}";
        ASSERT_EQUALS("void search ( ) { }", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "void search();\n"
               "void dostuff() { search(); }";
        ASSERT_EQUALS("void search ( ) ;\nvoid dostuff ( ) { search ( ) ; }", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "void foo() {map(a, b); }"; // That's obviously not std::map<>
        ASSERT_EQUALS("void foo ( ) { map ( a , b ) ; }", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "string<wchar_t> s;"; // That's obviously not std::string
        ASSERT_EQUALS("string < wchar_t > s ;", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "swap s;"; // That's obviously not std::swap
        ASSERT_EQUALS("swap s ;", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "std::string s;";
        ASSERT_EQUALS("std :: string s ;", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "tr1::function <void(int)> f;";
        ASSERT_EQUALS("tr1 :: function < void ( int ) > f ;", tokenizeAndStringify(code, false, true, Settings::Native, "test.cpp", false));
        ASSERT_EQUALS("std :: function < void ( int ) > f ;", tokenizeAndStringify(code, false, true, Settings::Native, "test.cpp", true));

        code = "std::tr1::function <void(int)> f;";
        ASSERT_EQUALS("std :: tr1 :: function < void ( int ) > f ;", tokenizeAndStringify(code, false, true, Settings::Native, "test.cpp", false));
        ASSERT_EQUALS("std :: function < void ( int ) > f ;", tokenizeAndStringify(code, false, true, Settings::Native, "test.cpp", true));

        // #4042 (Do not add 'std ::' to variables)
        code = "using namespace std;\n"
               "const char * string = \"Hi\";";
        ASSERT_EQUALS("const char * string ; string = \"Hi\" ;", tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "string f(const char * string) {\n"
               "    cout << string << endl;\n"
               "    return string;\n"
               "}";
        expected = "std :: string f ( const char * string ) {\n"
                   "std :: cout << string << std :: endl ;\n"
                   "return string ;\n"
                   "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, false));

        code = "using namespace std;\n"
               "try { }\n"
               "catch(std::exception &exception) { }";
        expected = "try { }\n"
                   "catch ( std :: exception & exception ) { }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, false));

        // #5773 (Don't prepend 'std ::' to function definitions)
        code = "using namespace std;\n"
               "class C {\n"
               "    void search() {}\n"
               "    void search() const {}\n"
               "    void search() THROW_MACRO {}\n"
               "};";
        expected = "class C {\n"
                   "void search ( ) { }\n"
                   "void search ( ) const { }\n"
                   "void search ( ) { }\n"
                   "} ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, false));

        // Ticket #8091
        ASSERT_EQUALS("enum Anonymous0 { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum { string };"));
        ASSERT_EQUALS("enum Type { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum Type { string } ;"));
        ASSERT_EQUALS("enum class Type { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum class Type { string } ;"));
        ASSERT_EQUALS("enum struct Type { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum struct Type { string } ;"));
        ASSERT_EQUALS("enum struct Type : int { f = 0 , string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum struct Type : int { f = 0 , string } ;"));
        ASSERT_EQUALS("enum Type { a , b } ; void foo ( enum Type , std :: string ) { }",
                      tokenizeAndStringify("using namespace std; "
                                           "enum Type { a , b } ; void foo ( enum Type , string) {}"));
        ASSERT_EQUALS("struct T { } ; enum struct Type : int { f = 0 , string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "struct T { typedef int type; } ; "
                                           "enum struct Type : T :: type { f = 0 , string } ;"));
        // Handle garbage enum code "well"
        ASSERT_EQUALS("enum E : int ; void foo ( ) { std :: string s ; }",
                      tokenizeAndStringify("using namespace std; enum E : int ; void foo ( ) { string s ; }"));
    }

    void microsoftMemory() {
        const char code1a[] = "void foo() { int a[10], b[10]; CopyMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcpy ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code1a,false,true,Settings::Win32A));

        const char code1b[] = "void foo() { int a[10], b[10]; RtlCopyMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcpy ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code1b,false,true,Settings::Win32A));

        const char code1c[] = "void foo() { int a[10], b[10]; RtlCopyBytes(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcpy ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code1c,false,true,Settings::Win32A));

        const char code2a[] = "void foo() { int a[10]; FillMemory(a, sizeof(a), 255); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 255 , sizeof ( a ) ) ; }", tokenizeAndStringify(code2a,false,true,Settings::Win32A));
        const char code2b[] = "void foo() { int a[10]; RtlFillMemory(a, sizeof(a), 255); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 255 , sizeof ( a ) ) ; }", tokenizeAndStringify(code2b,false,true,Settings::Win32A));
        const char code2c[] = "void foo() { int a[10]; RtlFillBytes(a, sizeof(a), 255); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 255 , sizeof ( a ) ) ; }", tokenizeAndStringify(code2c,false,true,Settings::Win32A));

        const char code3a[] = "void foo() { int a[10], b[10]; MoveMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memmove ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code3a,false,true,Settings::Win32A));
        const char code3b[] = "void foo() { int a[10], b[10]; RtlMoveMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memmove ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code3b,false,true,Settings::Win32A));

        const char code4a[] = "void foo() { int a[10]; ZeroMemory(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4a,false,true,Settings::Win32A));
        const char code4b[] = "void foo() { int a[10]; RtlZeroMemory(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4b,false,true,Settings::Win32A));
        const char code4c[] = "void foo() { int a[10]; RtlZeroBytes(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4c,false,true,Settings::Win32A));
        const char code4d[] = "void foo() { int a[10]; RtlSecureZeroMemory(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4d,false,true,Settings::Win32A));

        const char code5[] = "void foo() { int a[10], b[10]; RtlCompareMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcmp ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code5,false,true,Settings::Win32A));

        const char code6[] = "void foo() { ZeroMemory(f(1, g(a, b)), h(i, j(0, 1))); }";
        ASSERT_EQUALS("void foo ( ) { memset ( f ( 1 , g ( a , b ) ) , 0 , h ( i , j ( 0 , 1 ) ) ) ; }", tokenizeAndStringify(code6,false,true,Settings::Win32A));

        const char code7[] = "void foo() { FillMemory(f(1, g(a, b)), h(i, j(0, 1)), 255); }";
        ASSERT_EQUALS("void foo ( ) { memset ( f ( 1 , g ( a , b ) ) , 255 , h ( i , j ( 0 , 1 ) ) ) ; }", tokenizeAndStringify(code7,false,true,Settings::Win32A));
    }

    void microsoftString() {
        const char code1a[] = "void foo() { _tprintf (_T(\"test\") _T(\"1\")); }";
        ASSERT_EQUALS("void foo ( ) { printf ( \"test1\" ) ; }", tokenizeAndStringify(code1a, false, true, Settings::Win32A));
        const char code1b[] = "void foo() { _tprintf (_TEXT(\"test\") _TEXT(\"2\")); }";
        ASSERT_EQUALS("void foo ( ) { printf ( \"test2\" ) ; }", tokenizeAndStringify(code1b, false, true, Settings::Win32A));
        const char code1c[] = "void foo() { _tprintf (TEXT(\"test\") TEXT(\"3\")); }";
        ASSERT_EQUALS("void foo ( ) { printf ( \"test3\" ) ; }", tokenizeAndStringify(code1c, false, true, Settings::Win32A));

        const char code2a[] = "void foo() { _tprintf (_T(\"test\") _T(\"1\")); }";
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test1\" ) ; }", tokenizeAndStringify(code2a, false, true, Settings::Win32W));
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test1\" ) ; }", tokenizeAndStringify(code2a, false, true, Settings::Win64));
        const char code2b[] = "void foo() { _tprintf (_TEXT(\"test\") _TEXT(\"2\")); }";
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test2\" ) ; }", tokenizeAndStringify(code2b, false, true, Settings::Win32W));
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test2\" ) ; }", tokenizeAndStringify(code2b, false, true, Settings::Win64));
        const char code2c[] = "void foo() { _tprintf (TEXT(\"test\") TEXT(\"3\")); }";
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test3\" ) ; }", tokenizeAndStringify(code2c, false, true, Settings::Win32W));
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test3\" ) ; }", tokenizeAndStringify(code2c, false, true, Settings::Win64));
    }

    void borland() {
        // __closure
        ASSERT_EQUALS("int * a ;",
                      tokenizeAndStringify("int (__closure *a)();", false, true, Settings::Win32A));

        // __property
        ASSERT_EQUALS("class Fred { ; __property ; } ;",
                      tokenizeAndStringify("class Fred { __property int x = { } };", false, true, Settings::Win32A));
    }

    void Qt() {
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

        const char code4[] = "class MyObject : public QObject {"
                             "    Q_OBJECT "
                             "public slots:"
                             "};";
        const char result4[] = "class MyObject : public QObject { "
                               "public: "
                               "} ;";

        ASSERT_EQUALS(result4, tokenizeAndStringify(code4,false));
    }

    void simplifySQL() {
        // Oracle PRO*C extensions for inline SQL. Just replace the SQL with "asm()" to fix wrong error messages
        // ticket: #1959
        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL SELECT A FROM B\"\" ) ;", tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL SELECT A FROM B;",false));
        ASSERT_THROW(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL",false), InternalError);

        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1 ( A ) ; END ; END - __CPPCHECK_EMBEDDED_SQL_EXEC__\"\" ) ; asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT\"\" ) ;",
                      tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1(A); END; END-__CPPCHECK_EMBEDDED_SQL_EXEC__; __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT;",false));
        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL UPDATE A SET B = C\"\" ) ; asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT\"\" ) ;",
                      tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL UPDATE A SET B = C; __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT;",false));
        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT\"\" ) ; asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1 ( A ) ; END ; END - __CPPCHECK_EMBEDDED_SQL_EXEC__\"\" ) ;",
                      tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT; __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1(A); END; END-__CPPCHECK_EMBEDDED_SQL_EXEC__;",false));

        ASSERT_THROW(tokenizeAndStringify("int f(){ __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL } int a;",false), InternalError);
        ASSERT_THROW(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL int f(){",false), InternalError);
        ASSERT_THROW(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL END-__CPPCHECK_EMBEDDED_SQL_EXEC__ int a;",false), InternalError);
        ASSERT_NO_THROW(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL UPDATE A SET B = :&b->b1, C = :c::c1;",false));
    }

    void simplifyCAlternativeTokens() {
        ASSERT_EQUALS("void f ( ) { if ( a && b ) { ; } }", tokenizeAndStringify("void f() { if (a and b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( a && b ) { ; } }", tokenizeAndStringify("void f() { if (a and b); }", false, true, Settings::Native, "test.cpp"));
        ASSERT_EQUALS("void f ( ) { if ( a || b ) { ; } }", tokenizeAndStringify("void f() { if (a or b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( a || b ) { ; } }", tokenizeAndStringify("void f() { if (a or b); }", false, true, Settings::Native, "test.cpp"));
        ASSERT_EQUALS("void f ( ) { if ( a & b ) { ; } }", tokenizeAndStringify("void f() { if (a bitand b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( a & b ) { ; } }", tokenizeAndStringify("void f() { if (a bitand b); }", false, true, Settings::Native, "test.cpp"));
        ASSERT_EQUALS("void f ( ) { if ( a | b ) { ; } }", tokenizeAndStringify("void f() { if (a bitor b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( a | b ) { ; } }", tokenizeAndStringify("void f() { if (a bitor b); }", false, true, Settings::Native, "test.cpp"));
        ASSERT_EQUALS("void f ( ) { if ( a ^ b ) { ; } }", tokenizeAndStringify("void f() { if (a xor b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( a ^ b ) { ; } }", tokenizeAndStringify("void f() { if (a xor b); }", false, true, Settings::Native, "test.cpp"));
        ASSERT_EQUALS("void f ( ) { if ( ~ b ) { ; } }", tokenizeAndStringify("void f() { if (compl b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( ~ b ) { ; } }", tokenizeAndStringify("void f() { if (compl b); }", false, true, Settings::Native, "test.cpp"));
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { ; } }", tokenizeAndStringify("void f() { if (not b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { ; } }", tokenizeAndStringify("void f() { if (not b); }", false, true, Settings::Native, "test.cpp"));
        ASSERT_EQUALS("void f ( ) { if ( a != b ) { ; } }", tokenizeAndStringify("void f() { if (a not_eq b); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( a != b ) { ; } }", tokenizeAndStringify("void f() { if (a not_eq b); }", false, true, Settings::Native, "test.cpp"));
        // #6201
        ASSERT_EQUALS("void f ( ) { if ( ! c || ! memcmp ( a , b , s ) ) { ; } }", tokenizeAndStringify("void f() { if (!c or !memcmp(a, b, s)); }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( ! c || ! memcmp ( a , b , s ) ) { ; } }", tokenizeAndStringify("void f() { if (!c or !memcmp(a, b, s)); }", false, true, Settings::Native, "test.cpp"));
        // #6029
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { } }", tokenizeAndStringify("void f() { if (not b){} }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { } }", tokenizeAndStringify("void f() { if (not b){} }", false, true, Settings::Native, "test.cpp"));
        // #6207
        ASSERT_EQUALS("void f ( ) { if ( not = x ) { } }", tokenizeAndStringify("void f() { if (not=x){} }", false, true, Settings::Native, "test.c"));
        ASSERT_EQUALS("void f ( ) { if ( not = x ) { } }", tokenizeAndStringify("void f() { if (not=x){} }", false, true, Settings::Native, "test.cpp"));
        // #8029
        ASSERT_EQUALS("void f ( struct S * s ) { x = s . and + 1 ; }", tokenizeAndStringify("void f(struct S *s) { x = s->and + 1; }", false, true, Settings::Native, "test.c"));
        // #8745
        ASSERT_EQUALS("void f ( ) { if ( x ) { or = 0 ; } }", tokenizeAndStringify("void f() { if (x) or = 0; }"));
    }

    void simplifyCalculations() {
        ASSERT_EQUALS("void foo ( char str [ ] ) { char x ; x = * str ; }",
                      tokenizeAndStringify("void foo ( char str [ ] ) { char x = 0 | ( * str ) ; }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (b + 0) { } }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (0 + b) { } }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (b - 0) { } }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (b * 1) { } }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (1 * b) { } }", true));
        //ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
        //              tokenizeAndStringify("void foo ( ) { if (b / 1) { } }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (b | 0) { } }", true));
        ASSERT_EQUALS("void foo ( ) { if ( b ) { } }",
                      tokenizeAndStringify("void foo ( ) { if (0 | b) { } }", true));
        ASSERT_EQUALS("void foo ( int b ) { int a ; a = b ; bar ( a ) ; }",
                      tokenizeAndStringify("void foo ( int b ) { int a = b | 0 ; bar ( a ) ; }", true));
        ASSERT_EQUALS("void foo ( int b ) { int a ; a = b ; bar ( a ) ; }",
                      tokenizeAndStringify("void foo ( int b ) { int a = 0 | b ; bar ( a ) ; }", true));

        // ticket #3093
        ASSERT_EQUALS("int f ( ) { return 15 ; }",
                      tokenizeAndStringify("int f() { int a = 10; int b = 5; return a + b; }", true));
        ASSERT_EQUALS("int f ( ) { return a ; }",
                      tokenizeAndStringify("int f() { return a * 1; }", true));
        ASSERT_EQUALS("int f ( int a ) { return 0 ; }",
                      tokenizeAndStringify("int f(int a) { return 0 * a; }", true));
        ASSERT_EQUALS("bool f ( int i ) { switch ( i ) { case 15 : ; return true ; } }",
                      tokenizeAndStringify("bool f(int i) { switch (i) { case 10 + 5: return true; } }", true));

        // ticket #3576 - False positives in boolean expressions
        ASSERT_EQUALS("int foo ( ) { return 1 ; }",
                      tokenizeAndStringify("int foo ( ) { int i; int j; i = 1 || j; return i; }", true));

        ASSERT_EQUALS("int foo ( ) { return 0 ; }",
                      tokenizeAndStringify("int foo ( ) { int i; int j; i = 0 && j; return i; }", true));        // ticket #3576 - False positives in boolean expressions

        // ticket #3723 - Simplify condition (0 && a < 123)
        ASSERT_EQUALS("( 0 ) ;",
                      tokenizeAndStringify("( 0 && a < 123 );", true));
        ASSERT_EQUALS("( 0 ) ;",
                      tokenizeAndStringify("( 0 && a[123] );", true));

        // ticket #4931
        ASSERT_EQUALS("dostuff ( 1 ) ;", tokenizeAndStringify("dostuff(9&&8);", true));
    }

    void simplifyRoundCurlyParentheses() {
        ASSERT_EQUALS("; x = 123 ;", tokenizeAndStringify(";x=({123;});"));
        ASSERT_EQUALS("; x = y ;", tokenizeAndStringify(";x=({y;});"));
    }

    void simplifyOperatorName1() {
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

        ASSERT_EQUALS(result, tokenizeAndStringify(code, /*simplify=*/false, /*expand=*/true, /*platform=*/Settings::Native, "test.c"));
    }

    void simplifyOperatorName2() {
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

    void simplifyOperatorName3() {
        // #2615
        const char code[] = "void f() {"
                            "static_cast<ScToken*>(xResult.operator->())->GetMatrix();"
                            "}";
        const char result[] = "void f ( ) { static_cast < ScToken * > ( xResult . operator. ( ) ) . GetMatrix ( ) ; }";
        ASSERT_EQUALS(result, tokenizeAndStringify(code,false));
    }

    void simplifyOperatorName4() {
        const char code[] = "void operator==() { }";
        const char result[] = "void operator== ( ) { }";
        ASSERT_EQUALS(result, tokenizeAndStringify(code,false));
    }

    void simplifyOperatorName5() {
        const char code1[] = "std::istream & operator >> (std::istream & s, Fred &f);";
        const char result1[] = "std :: istream & operator>> ( std :: istream & s , Fred & f ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1,false));

        const char code2[] = "std::ostream & operator << (std::ostream & s, const Fred &f);";
        const char result2[] = "std :: ostream & operator<< ( std :: ostream & s , const Fred & f ) ;";
        ASSERT_EQUALS(result2, tokenizeAndStringify(code2,false));
    }

    void simplifyOperatorName6() { // ticket #3195
        const char code1[] = "value_type * operator ++ (int);";
        const char result1[] = "value_type * operator++ ( int ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1,false));

        const char code2[] = "value_type * operator -- (int);";
        const char result2[] = "value_type * operator-- ( int ) ;";
        ASSERT_EQUALS(result2, tokenizeAndStringify(code2,false));
    }

    void simplifyOperatorName7() { // ticket #4619
        const char code1[] = "value_type * operator += (int);";
        const char result1[] = "value_type * operator+= ( int ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1,false));
    }

    void simplifyOperatorName8() { // ticket #5706
        const char code1[] = "value_type * operator += (int) noexcept ;";
        const char result1[] = "value_type * operator+= ( int ) noexcept ( true ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1,false));

        const char code2[] = "value_type * operator += (int) noexcept ( true ) ;";
        const char result2[] = "value_type * operator+= ( int ) noexcept ( true ) ;";
        ASSERT_EQUALS(result2, tokenizeAndStringify(code2,false));

        const char code3[] = "value_type * operator += (int) throw ( ) ;";
        const char result3[] = "value_type * operator+= ( int ) throw ( ) ;";
        ASSERT_EQUALS(result3, tokenizeAndStringify(code3,false));

        const char code4[] = "value_type * operator += (int) const noexcept ;";
        const char result4[] = "value_type * operator+= ( int ) const noexcept ;";
        ASSERT_EQUALS(result4, tokenizeAndStringify(code4,false));

        const char code5[] = "value_type * operator += (int) const noexcept ( true ) ;";
        const char result5[] = "value_type * operator+= ( int ) const noexcept ( true ) ;";
        ASSERT_EQUALS(result5, tokenizeAndStringify(code5,false));

        const char code6[] = "value_type * operator += (int) const throw ( ) ;";
        const char result6[] = "value_type * operator+= ( int ) const throw ( ) ;";
        ASSERT_EQUALS(result6, tokenizeAndStringify(code6,false));

        const char code7[] = "value_type * operator += (int) const noexcept ( false ) ;";
        const char result7[] = "value_type * operator+= ( int ) const noexcept ( false ) ;";
        ASSERT_EQUALS(result7, tokenizeAndStringify(code7,false));

    }

    void simplifyOperatorName9() { // Ticket #5709
        const char code[] = "struct R { R operator, ( R b ) ; } ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyOperatorName10() { // #8746
        const char code[] = "using a::operator=;";
        ASSERT_EQUALS("using a :: operator= ;", tokenizeAndStringify(code));
    }

    void simplifyNullArray() {
        ASSERT_EQUALS("* ( foo . bar [ 5 ] ) = x ;", tokenizeAndStringify("0[foo.bar[5]] = x;"));
    }

    void removeMacrosInGlobalScope() {
        // remove some unhandled macros in the global scope.
        ASSERT_EQUALS("void f ( ) { }", tokenizeAndStringify("void f() NOTHROW { }"));
        ASSERT_EQUALS("struct Foo { } ;", tokenizeAndStringify("struct __declspec(dllexport) Foo {};"));
        ASSERT_EQUALS("namespace { }", tokenizeAndStringify("ABA() namespace { }"));

        // #3750
        ASSERT_EQUALS("; foo :: foo ( ) { }",
                      tokenizeAndStringify("; AB(foo*) foo::foo() { }"));

        // #4834 - syntax error
        ASSERT_THROW(tokenizeAndStringify("A(B) foo() {}"), InternalError);

        // #3855
        ASSERT_EQUALS("; class foo { }",
                      tokenizeAndStringify("; AB class foo { }"));
        ASSERT_EQUALS("; CONST struct ABC abc ;",
                      tokenizeAndStringify("; CONST struct ABC abc ;"));
    }

    void removeMacroInVarDecl() { // #4304
        // only remove macros with parentheses (those hurt most)
        ASSERT_EQUALS("void f ( ) { PROGMEM int x ; }", tokenizeAndStringify("void f() { PROGMEM int x ; }"));
        ASSERT_EQUALS("void f ( ) { int x ; }", tokenizeAndStringify("void f() { SECTION(\".data.ro\") int x ; }"));

        // various variable declarations
        ASSERT_EQUALS("void f ( ) { CONST int x ; }", tokenizeAndStringify("void f() { SECTION(\".data.ro\") CONST int x ; }"));
        ASSERT_EQUALS("void f ( ) { char a [ 4 ] ; }", tokenizeAndStringify("void f() { SECTION(\".data.ro\") char a[4]; }"));
        ASSERT_EQUALS("void f ( ) { const char a [ 4 ] ; }", tokenizeAndStringify("void f() { SECTION(\".data.ro\") const char a[4]; }"));
        ASSERT_EQUALS("void f ( ) { struct ABC abc ; }", tokenizeAndStringify("void f() { SECTION(\".data.ro\") struct ABC abc; }"));
        ASSERT_EQUALS("void f ( ) { CONST struct ABC abc ; }", tokenizeAndStringify("void f() { SECTION(\".data.ro\") CONST struct ABC abc; }"));
    }

    void multipleAssignment() {
        ASSERT_EQUALS("a = b = 0 ;", tokenizeAndStringify("a=b=0;"));
    }

    void sizeOfCharLiteral() { // #7490 sizeof('a') should be 4 in C mode
        std::stringstream expected;
        expected << "unsigned long a ; a = " << settings1.sizeof_int << " ;";
        ASSERT_EQUALS(expected.str(),
                      tokenizeAndStringify("unsigned long a = sizeof('x');", true, true, Settings::Native, "test.c", false));
        ASSERT_EQUALS("unsigned long a ; a = 1 ;",
                      tokenizeAndStringify("unsigned long a = sizeof('x');", true, true, Settings::Native, "test.cpp", true));
    }

    void platformWin() {
        const char code[] = "BOOL f;"
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
                            "PVOID N;"
                            "BOOL _bool;"
                            "HFILE hfile;"
                            "LONG32 long32;"
                            "LCID lcid;"
                            "LCTYPE lctype;"
                            "LGRPID lgrpid;"
                            "LONG64 long64;"
                            "PUCHAR puchar;"
                            "LPCOLORREF lpcolorref;"
                            "PDWORD pdword;"
                            "PULONG pulong;"
                            "SERVICE_STATUS_HANDLE service_status_hanlde;"
                            "SC_LOCK sc_lock;"
                            "SC_HANDLE sc_handle;"
                            "HACCEL haccel;"
                            "HCONV hconv;"
                            "HCONVLIST hconvlist;"
                            "HDDEDATA hddedata;"
                            "HDESK hdesk;"
                            "HDROP hdrop;"
                            "HDWP hdwp;"
                            "HENHMETAFILE henhmetafile;"
                            "HHOOK hhook;"
                            "HKL hkl;"
                            "HMONITOR hmonitor;"
                            "HSZ hsz;"
                            "HWINSTA hwinsta;"
                            "PWCHAR pwchar;"
                            "PUSHORT pushort;"
                            "LANGID langid;"
                            "DWORD64 dword64;"
                            "ULONG64 ulong64;"
                            "LPWSTR lpcwstr;"
                            "LPCWSTR lpcwstr;"
                            "LPHANDLE lpHandle;"
                            "PCWSTR pcwStr;"
                            "PDWORDLONG pdWordLong;"
                            "PDWORD_PTR pdWordPtr;"
                            "PDWORD32 pdWord32;"
                            "PDWORD64 pdWord64;"
                            "LONGLONG ll;"
                            "USN usn;"
                            "PULONG64 puLong64;"
                            "PULONG32 puLong32;"
                            "PFLOAT ptrToFloat;";

        const char expected[] = "int f ; "
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
                                "void * N ; "
                                "int _bool ; "
                                "int hfile ; "
                                "int long32 ; "
                                "unsigned long lcid ; "
                                "unsigned long lctype ; "
                                "unsigned long lgrpid ; "
                                "long long long64 ; "
                                "unsigned char * puchar ; "
                                "unsigned long * lpcolorref ; "
                                "unsigned long * pdword ; "
                                "unsigned long * pulong ; "
                                "void * service_status_hanlde ; "
                                "void * sc_lock ; "
                                "void * sc_handle ; "
                                "void * haccel ; "
                                "void * hconv ; "
                                "void * hconvlist ; "
                                "void * hddedata ; "
                                "void * hdesk ; "
                                "void * hdrop ; "
                                "void * hdwp ; "
                                "void * henhmetafile ; "
                                "void * hhook ; "
                                "void * hkl ; "
                                "void * hmonitor ; "
                                "void * hsz ; "
                                "void * hwinsta ; "
                                "wchar_t * pwchar ; "
                                "unsigned short * pushort ; "
                                "unsigned short langid ; "
                                "unsigned long long dword64 ; "
                                "unsigned long long ulong64 ; "
                                "wchar_t * lpcwstr ; "
                                "const wchar_t * lpcwstr ; "
                                "void * lpHandle ; "
                                "const wchar_t * pcwStr ; "
                                "long * pdWordLong ; "
                                "long * pdWordPtr ; "
                                "unsigned int * pdWord32 ; "
                                "unsigned long * pdWord64 ; "
                                "long long ll ; "
                                "long long usn ; "
                                "unsigned long long * puLong64 ; "
                                "unsigned int * puLong32 ; "
                                "float * ptrToFloat ;";

        // These types should be defined the same on all Windows platforms
        const std::string win32A = tokenizeAndStringifyWindows(code, true, true, Settings::Win32A);
        ASSERT_EQUALS(expected, win32A);
        ASSERT_EQUALS(win32A, tokenizeAndStringifyWindows(code, true, true, Settings::Win32W));
        ASSERT_EQUALS(win32A, tokenizeAndStringifyWindows(code, true, true, Settings::Win64));
    }

    void platformWin32() {
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
                            "unsigned int sizeof_wchar_t = sizeof(wchar_t);"
                            "unsigned int sizeof_pointer = sizeof(void *);"
                            "unsigned int sizeof_size_t = sizeof(size_t);"
                            "size_t a;"
                            "ssize_t b;"
                            "ptrdiff_t c;"
                            "intptr_t d;"
                            "uintptr_t e;"
                            "DWORD_PTR O;"
                            "ULONG_PTR P;"
                            "SIZE_T Q;"
                            "HRESULT R;"
                            "LONG_PTR S;"
                            "HANDLE T;"
                            "PHANDLE U;"
                            "SSIZE_T _ssize_t;"
                            "UINT_PTR uint_ptr;"
                            "WPARAM wparam;"
                            "HALF_PTR half_ptr;"
                            "INT_PTR int_ptr;";

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
                                "unsigned int sizeof_wchar_t ; sizeof_wchar_t = 2 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 4 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 4 ; "
                                "unsigned long a ; "
                                "long b ; "
                                "long c ; "
                                "long d ; "
                                "unsigned long e ; "
                                "unsigned long O ; "
                                "unsigned long P ; "
                                "unsigned long Q ; "
                                "long R ; "
                                "long S ; "
                                "void * T ; "
                                "void * * U ; "
                                "long _ssize_t ; "
                                "unsigned int uint_ptr ; "
                                "unsigned int wparam ; "
                                "short half_ptr ; "
                                "int int_ptr ;";

        // These types should be defined the same on all Win32 platforms
        std::string win32A = tokenizeAndStringifyWindows(code, true, true, Settings::Win32A);
        ASSERT_EQUALS(expected, win32A);
        ASSERT_EQUALS(win32A, tokenizeAndStringifyWindows(code, true, true, Settings::Win32W));
    }

    void platformWin32A() {
        const char code[] = "wchar_t wc;"
                            "TCHAR c;"
                            "PTSTR ptstr;"
                            "LPTSTR lptstr;"
                            "PCTSTR pctstr;"
                            "LPCTSTR lpctstr;"
                            "void foo() {"
                            "    TCHAR tc = _T(\'c\'); "
                            "    TCHAR src[10] = _T(\"123456789\");"
                            "    TCHAR dst[10];"
                            "    _tcscpy(dst, src);"
                            "    dst[0] = 0;"
                            "    _tcscat(dst, src);"
                            "    LPTSTR d = _tcsdup(str);"
                            "    _tprintf(_T(\"Hello world!\"));"
                            "    _stprintf(dst, _T(\"Hello!\"));"
                            "    _sntprintf(dst, sizeof(dst) / sizeof(TCHAR), _T(\"Hello world!\"));"
                            "    _tscanf(_T(\"%s\"), dst);"
                            "    _stscanf(dst, _T(\"%s\"), dst);"
                            "}"
                            "TBYTE tbyte;";
        const char expected[] = "wchar_t wc ; "
                                "char c ; "
                                "char * ptstr ; "
                                "char * lptstr ; "
                                "const char * pctstr ; "
                                "const char * lpctstr ; "
                                "void foo ( ) { "
                                "char tc ; tc = \'c\' ; "
                                "char src [ 10 ] = \"123456789\" ; "
                                "char dst [ 10 ] ; "
                                "strcpy ( dst , src ) ; "
                                "dst [ 0 ] = 0 ; "
                                "strcat ( dst , src ) ; "
                                "char * d ; d = strdup ( str ) ; "
                                "printf ( \"Hello world!\" ) ; "
                                "sprintf ( dst , \"Hello!\" ) ; "
                                "_snprintf ( dst , sizeof ( dst ) / sizeof ( char ) , \"Hello world!\" ) ; "
                                "scanf ( \"%s\" , dst ) ; "
                                "sscanf ( dst , \"%s\" , dst ) ; "
                                "} "
                                "unsigned char tbyte ;";
        ASSERT_EQUALS(expected, tokenizeAndStringifyWindows(code, false, true, Settings::Win32A));
    }

    void platformWin32W() {
        const char code[] = "wchar_t wc;"
                            "TCHAR c;"
                            "PTSTR ptstr;"
                            "LPTSTR lptstr;"
                            "PCTSTR pctstr;"
                            "LPCTSTR lpctstr;"
                            "TBYTE tbyte;"
                            "void foo() {"
                            "    TCHAR tc = _T(\'c\');"
                            "    TCHAR src[10] = _T(\"123456789\");"
                            "    TCHAR dst[10];"
                            "    _tcscpy(dst, src);"
                            "    dst[0] = 0;"
                            "    _tcscat(dst, src);"
                            "    LPTSTR d = _tcsdup(str);"
                            "    _tprintf(_T(\"Hello world!\"));"
                            "    _stprintf(dst, _T(\"Hello!\"));"
                            "    _sntprintf(dst, sizeof(dst) / sizeof(TCHAR), _T(\"Hello world!\"));"
                            "    _tscanf(_T(\"%s\"), dst);"
                            "    _stscanf(dst, _T(\"%s\"), dst);"
                            "}";
        const char expected[] = "wchar_t wc ; "
                                "wchar_t c ; "
                                "wchar_t * ptstr ; "
                                "wchar_t * lptstr ; "
                                "const wchar_t * pctstr ; "
                                "const wchar_t * lpctstr ; "
                                "unsigned wchar_t tbyte ; "
                                "void foo ( ) { "
                                "wchar_t tc ; tc = L\'c\' ; "
                                "wchar_t src [ 10 ] = L\"123456789\" ; "
                                "wchar_t dst [ 10 ] ; "
                                "wcscpy ( dst , src ) ; "
                                "dst [ 0 ] = 0 ; "
                                "wcscat ( dst , src ) ; "
                                "wchar_t * d ; d = wcsdup ( str ) ; "
                                "wprintf ( L\"Hello world!\" ) ; "
                                "swprintf ( dst , L\"Hello!\" ) ; "
                                "_snwprintf ( dst , sizeof ( dst ) / sizeof ( wchar_t ) , L\"Hello world!\" ) ; "
                                "wscanf ( L\"%s\" , dst ) ; "
                                "swscanf ( dst , L\"%s\" , dst ) ; "
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringifyWindows(code, false, true, Settings::Win32W));
    }

    void platformWin64() {
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
                            "unsigned int sizeof_wchar_t = sizeof(wchar_t);"
                            "unsigned int sizeof_pointer = sizeof(void *);"
                            "unsigned int sizeof_size_t = sizeof(size_t);"
                            "size_t a;"
                            "ssize_t b;"
                            "ptrdiff_t c;"
                            "intptr_t d;"
                            "uintptr_t e;"
                            "DWORD_PTR O;"
                            "ULONG_PTR P;"
                            "SIZE_T Q;"
                            "HRESULT R;"
                            "LONG_PTR S;"
                            "HANDLE T;"
                            "PHANDLE U;"
                            "SSIZE_T _ssize_t;"
                            "UINT_PTR uint_ptr;"
                            "WPARAM wparam;"
                            "HALF_PTR half_ptr;"
                            "INT_PTR int_ptr;";

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
                                "unsigned int sizeof_wchar_t ; sizeof_wchar_t = 2 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 8 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 8 ; "
                                "unsigned long long a ; "
                                "long long b ; "
                                "long long c ; "
                                "long long d ; "
                                "unsigned long long e ; "
                                "unsigned long long O ; "
                                "unsigned long long P ; "
                                "unsigned long long Q ; "
                                "long R ; "
                                "long long S ; "
                                "void * T ; "
                                "void * * U ; "
                                "long long _ssize_t ; "
                                "unsigned long long uint_ptr ; "
                                "unsigned long long wparam ; "
                                "int half_ptr ; "
                                "long long int_ptr ;";

        ASSERT_EQUALS(expected, tokenizeAndStringifyWindows(code, true, true, Settings::Win64));
    }

    void platformUnix32() {
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
                            "unsigned int sizeof_wchar_t = sizeof(wchar_t);"
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
                                "unsigned int sizeof_wchar_t ; sizeof_wchar_t = 4 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 4 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 4 ; "
                                "unsigned long a ; "
                                "long b ; "
                                "long c ; "
                                "long d ; "
                                "unsigned long e ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Unix32));
    }

    void platformUnix64() {
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
                            "unsigned int sizeof_wchar_t = sizeof(wchar_t);"
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
                                "unsigned int sizeof_wchar_t ; sizeof_wchar_t = 4 ; "
                                "unsigned int sizeof_pointer ; sizeof_pointer = 8 ; "
                                "unsigned int sizeof_size_t ; sizeof_size_t = 8 ; "
                                "unsigned long a ; "
                                "long b ; "
                                "long c ; "
                                "long d ; "
                                "unsigned long e ;";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code, true, true, Settings::Unix64));
    }

    void platformWin32AStringCat() { //#5150
        const char code[] = "TCHAR text[] = _T(\"123\") _T(\"456\") _T(\"789\");";
        const char expected[] = "char text [ 10 ] = \"123456789\" ;";
        ASSERT_EQUALS(expected, tokenizeAndStringifyWindows(code, true, true, Settings::Win32A));
    }

    void platformWin32WStringCat() { //#5150
        const char code[] = "TCHAR text[] = _T(\"123\") _T(\"456\") _T(\"789\");";
        const char expected[] = "wchar_t text [ 10 ] = L\"123456789\" ;";
        ASSERT_EQUALS(expected, tokenizeAndStringifyWindows(code, true, true, Settings::Win32W));
    }

    void platformWinWithNamespace() {
        const char code1[] = "UINT32 a; ::UINT32 b; foo::UINT32 c;";
        const char expected1[] = "unsigned int a ; unsigned int b ; foo :: UINT32 c ;";
        ASSERT_EQUALS(expected1, tokenizeAndStringifyWindows(code1, true, true, Settings::Win32A));

        const char code2[] = "LPCVOID a; ::LPCVOID b; foo::LPCVOID c;";
        const char expected2[] = "const void * a ; const void * b ; foo :: LPCVOID c ;";
        ASSERT_EQUALS(expected2, tokenizeAndStringifyWindows(code2, true, true, Settings::Win32A));
    }

    void isZeroNumber() const {
        ASSERT_EQUALS(true, Tokenizer::isZeroNumber("0.0"));
        ASSERT_EQUALS(true, Tokenizer::isZeroNumber("+0.0"));
        ASSERT_EQUALS(true, Tokenizer::isZeroNumber("-0.0"));
        ASSERT_EQUALS(true, Tokenizer::isZeroNumber("+0L"));
        ASSERT_EQUALS(true, Tokenizer::isZeroNumber("+0"));
        ASSERT_EQUALS(true, Tokenizer::isZeroNumber("-0"));
        ASSERT_EQUALS(true, Tokenizer::isZeroNumber("-0E+0"));

        ASSERT_EQUALS(false, Tokenizer::isZeroNumber("1.0"));
        ASSERT_EQUALS(false, Tokenizer::isZeroNumber("+1.0"));
        ASSERT_EQUALS(false, Tokenizer::isZeroNumber("-1"));
        ASSERT_EQUALS(false, Tokenizer::isZeroNumber(""));
        ASSERT_EQUALS(false, Tokenizer::isZeroNumber("garbage"));
        ASSERT_EQUALS(false, Tokenizer::isZeroNumber("E2"));
        ASSERT_EQUALS(false, Tokenizer::isZeroNumber("2e"));
    }

    void isOneNumber() const {
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("1.0"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1.0"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("1.0e+0"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1L"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("1"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1E+0"));

        ASSERT_EQUALS(false, Tokenizer::isOneNumber("0.0"));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber("+0.0"));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber("-0"));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber(""));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber("garbage"));
    }

    void isTwoNumber() const {
        ASSERT_EQUALS(true, Tokenizer::isTwoNumber("2.0"));
        ASSERT_EQUALS(true, Tokenizer::isTwoNumber("+2.0"));
        ASSERT_EQUALS(true, Tokenizer::isTwoNumber("2.0e+0"));
        ASSERT_EQUALS(true, Tokenizer::isTwoNumber("+2L"));
        ASSERT_EQUALS(true, Tokenizer::isTwoNumber("+2"));
        ASSERT_EQUALS(true, Tokenizer::isTwoNumber("2"));
        ASSERT_EQUALS(true, Tokenizer::isTwoNumber("+2E+0"));

        ASSERT_EQUALS(false, Tokenizer::isTwoNumber("0.0"));
        ASSERT_EQUALS(false, Tokenizer::isTwoNumber("+0.0"));
        ASSERT_EQUALS(false, Tokenizer::isTwoNumber("-0"));
        ASSERT_EQUALS(false, Tokenizer::isTwoNumber(""));
        ASSERT_EQUALS(false, Tokenizer::isTwoNumber("garbage"));
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
        ASSERT_EQUALS(expected_erfc, tokenizeAndStringify(code_erfc));

        const char code_erfcf[] ="void f(float x) {\n"
                                 " std::cout << erfcf(x);\n" // do not simplify
                                 " std::cout << erfcf(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_erfcf[] = "void f ( float x ) {\n"
                                      "std :: cout << erfcf ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_erfcf, tokenizeAndStringify(code_erfcf));

        const char code_erfcl[] ="void f(long double x) {\n"
                                 " std::cout << erfcl(x);\n" // do not simplify
                                 " std::cout << erfcl(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_erfcl[] = "void f ( long double x ) {\n"
                                      "std :: cout << erfcl ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_erfcl, tokenizeAndStringify(code_erfcl));
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
        ASSERT_EQUALS(expected_cos, tokenizeAndStringify(code_cos));

        const char code_cosf[] ="void f(float x) {\n"
                                " std::cout << cosf(x);\n" // do not simplify
                                " std::cout << cosf(0.0f);\n" // simplify to 1
                                "}";
        const char expected_cosf[] = "void f ( float x ) {\n"
                                     "std :: cout << cosf ( x ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_cosf, tokenizeAndStringify(code_cosf));

        const char code_cosl[] ="void f(long double x) {\n"
                                " std::cout << cosl(x);\n" // do not simplify
                                " std::cout << cosl(0.0f);\n" // simplify to 1
                                "}";
        const char expected_cosl[] = "void f ( long double x ) {\n"
                                     "std :: cout << cosl ( x ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_cosl, tokenizeAndStringify(code_cosl));
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
        ASSERT_EQUALS(expected_cosh, tokenizeAndStringify(code_cosh));

        const char code_coshf[] ="void f(float x) {\n"
                                 " std::cout << coshf(x);\n" // do not simplify
                                 " std::cout << coshf(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_coshf[] = "void f ( float x ) {\n"
                                      "std :: cout << coshf ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_coshf, tokenizeAndStringify(code_coshf));

        const char code_coshl[] ="void f(long double x) {\n"
                                 " std::cout << coshl(x);\n" // do not simplify
                                 " std::cout << coshl(0.0f);\n" // simplify to 1
                                 "}";
        const char expected_coshl[] = "void f ( long double x ) {\n"
                                      "std :: cout << coshl ( x ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_coshl, tokenizeAndStringify(code_coshl));
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
        ASSERT_EQUALS(expected_acos, tokenizeAndStringify(code_acos));

        const char code_acosf[] ="void f(float x) {\n"
                                 " std::cout << acosf(x);\n" // do not simplify
                                 " std::cout << acosf(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_acosf[] = "void f ( float x ) {\n"
                                      "std :: cout << acosf ( x ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_acosf, tokenizeAndStringify(code_acosf));

        const char code_acosl[] ="void f(long double x) {\n"
                                 " std::cout << acosl(x);\n" // do not simplify
                                 " std::cout << acosl(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_acosl[] = "void f ( long double x ) {\n"
                                      "std :: cout << acosl ( x ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_acosl, tokenizeAndStringify(code_acosl));
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
        ASSERT_EQUALS(expected_acosh, tokenizeAndStringify(code_acosh));

        const char code_acoshf[] ="void f(float x) {\n"
                                  " std::cout << acoshf(x);\n" // do not simplify
                                  " std::cout << acoshf(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_acoshf[] = "void f ( float x ) {\n"
                                       "std :: cout << acoshf ( x ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_acoshf, tokenizeAndStringify(code_acoshf));

        const char code_acoshl[] ="void f(long double x) {\n"
                                  " std::cout << acoshl(x);\n" // do not simplify
                                  " std::cout << acoshl(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_acoshl[] = "void f ( long double x ) {\n"
                                       "std :: cout << acoshl ( x ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_acoshl, tokenizeAndStringify(code_acoshl));
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
        ASSERT_EQUALS(expected_sqrt, tokenizeAndStringify(code_sqrt));

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
        ASSERT_EQUALS(expected_sqrtf, tokenizeAndStringify(code_sqrtf));

        const char code_sqrtl[] ="void f(long double x) {\n"
                                 " std::cout << sqrtf(x);\n" // do not simplify
                                 " std::cout << sqrtf(-1.0);\n" // do not simplify
                                 " std::cout << sqrtf(0.0);\n" // simplify to 0
                                 " std::cout << sqrtf(1.0);\n" // simplify to 1
                                 "}";
        const char expected_sqrtl[] = "void f ( long double x ) {\n"
                                      "std :: cout << sqrtf ( x ) ;\n"
                                      "std :: cout << sqrtf ( -1.0 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_sqrtl, tokenizeAndStringify(code_sqrtl));
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
        ASSERT_EQUALS(expected_cbrt, tokenizeAndStringify(code_cbrt));

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
        ASSERT_EQUALS(expected_cbrtf, tokenizeAndStringify(code_cbrtf));

        const char code_cbrtl[] ="void f(long double x) {\n"
                                 " std::cout << cbrtl(x);\n" // do not simplify
                                 " std::cout << cbrtl(-1.0);\n" // do not simplify
                                 " std::cout << cbrtl(0.0);\n" // simplify to 0
                                 " std::cout << cbrtl(1.0);\n" // simplify to 1
                                 "}";
        const char expected_cbrtl[] = "void f ( long double x ) {\n"
                                      "std :: cout << cbrtl ( x ) ;\n"
                                      "std :: cout << cbrtl ( -1.0 ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "std :: cout << 1 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_cbrtl, tokenizeAndStringify(code_cbrtl));
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
        ASSERT_EQUALS(expected_exp2, tokenizeAndStringify(code_exp2));

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
        ASSERT_EQUALS(expected_exp2f, tokenizeAndStringify(code_exp2f));

        const char code_exp2l[] ="void f(long double x) {\n"
                                 " std::cout << exp2l(x);\n" // do not simplify
                                 " std::cout << exp2l(-1.0);\n" // do not simplify
                                 " std::cout << exp2l(0.0);\n" // simplify to 1
                                 " std::cout << exp2l(1.0);\n" // do not simplify
                                 "}";
        const char expected_exp2l[] = "void f ( long double x ) {\n"
                                      "std :: cout << exp2l ( x ) ;\n"
                                      "std :: cout << exp2l ( -1.0 ) ;\n"
                                      "std :: cout << 1 ;\n"
                                      "std :: cout << exp2l ( 1.0 ) ;\n"
                                      "}";
        ASSERT_EQUALS(expected_exp2l, tokenizeAndStringify(code_exp2l));
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
        ASSERT_EQUALS(expected_exp, tokenizeAndStringify(code_exp));

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
        ASSERT_EQUALS(expected_expf, tokenizeAndStringify(code_expf));

        const char code_expl[] ="void f(long double x) {\n"
                                " std::cout << expl(x);\n" // do not simplify
                                " std::cout << expl(-1.0);\n" // do not simplify
                                " std::cout << expl(0.0);\n" // simplify to 1
                                " std::cout << expl(1.0);\n" // do not simplify
                                "}";
        const char expected_expl[] = "void f ( long double x ) {\n"
                                     "std :: cout << expl ( x ) ;\n"
                                     "std :: cout << expl ( -1.0 ) ;\n"
                                     "std :: cout << 1 ;\n"
                                     "std :: cout << expl ( 1.0 ) ;\n"
                                     "}";
        ASSERT_EQUALS(expected_expl, tokenizeAndStringify(code_expl));
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
        ASSERT_EQUALS(expected_erf, tokenizeAndStringify(code_erf));

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
        ASSERT_EQUALS(expected_erff, tokenizeAndStringify(code_erff));

        const char code_erfl[] ="void f(long double x) {\n"
                                " std::cout << erfl(x);\n" // do not simplify
                                " std::cout << erfl(10.0f);\n" // do not simplify
                                " std::cout << erfl(0.0f);\n" // simplify to 0
                                "}";
        const char expected_erfl[] = "void f ( long double x ) {\n"
                                     "std :: cout << erfl ( x ) ;\n"
                                     "std :: cout << erfl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_erfl, tokenizeAndStringify(code_erfl));
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
        ASSERT_EQUALS(expected_atanh, tokenizeAndStringify(code_atanh));

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
        ASSERT_EQUALS(expected_atanhf, tokenizeAndStringify(code_atanhf));

        const char code_atanhl[] ="void f(long double x) {\n"
                                  " std::cout << atanhl(x);\n" // do not simplify
                                  " std::cout << atanhl(10.0f);\n" // do not simplify
                                  " std::cout << atanhl(0.0d);\n" // do not simplify - invalid number!
                                  " std::cout << atanhl(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_atanhl[] = "void f ( long double x ) {\n"
                                       "std :: cout << atanhl ( x ) ;\n"
                                       "std :: cout << atanhl ( 10.0f ) ;\n"
                                       "std :: cout << atanhl ( 0.0d ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_atanhl, tokenizeAndStringify(code_atanhl));
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
        ASSERT_EQUALS(expected_atan, tokenizeAndStringify(code_atan));

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
        ASSERT_EQUALS(expected_atanf, tokenizeAndStringify(code_atanf));

        const char code_atanl[] ="void f(long double x) {\n"
                                 " std::cout << atanl(x);\n" // do not simplify
                                 " std::cout << atanl(10.0f);\n" // do not simplify
                                 " std::cout << atanl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_atanl[] = "void f ( long double x ) {\n"
                                      "std :: cout << atanl ( x ) ;\n"
                                      "std :: cout << atanl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_atanl, tokenizeAndStringify(code_atanl));
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
        ASSERT_EQUALS(expected_tanh, tokenizeAndStringify(code_tanh));

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
        ASSERT_EQUALS(expected_tanhf, tokenizeAndStringify(code_tanhf));

        const char code_tanhl[] ="void f(long double x) {\n"
                                 " std::cout << tanhl(x);\n" // do not simplify
                                 " std::cout << tanhl(10.0f);\n" // do not simplify
                                 " std::cout << tanhl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_tanhl[] = "void f ( long double x ) {\n"
                                      "std :: cout << tanhl ( x ) ;\n"
                                      "std :: cout << tanhl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_tanhl, tokenizeAndStringify(code_tanhl));
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
        ASSERT_EQUALS(expected_tan, tokenizeAndStringify(code_tan));

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
        ASSERT_EQUALS(expected_tanf, tokenizeAndStringify(code_tanf));

        const char code_tanl[] ="void f(long double x) {\n"
                                " std::cout << tanl(x);\n" // do not simplify
                                " std::cout << tanl(10.0f);\n" // do not simplify
                                " std::cout << tanl(0.0f);\n" // simplify to 0
                                "}";
        const char expected_tanl[] = "void f ( long double x ) {\n"
                                     "std :: cout << tanl ( x ) ;\n"
                                     "std :: cout << tanl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_tanl, tokenizeAndStringify(code_tanl));
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
        ASSERT_EQUALS(expected_expm1, tokenizeAndStringify(code_expm1));

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
        ASSERT_EQUALS(expected_expm1f, tokenizeAndStringify(code_expm1f));

        const char code_expm1l[] ="void f(long double x) {\n"
                                  " std::cout << expm1l(x);\n" // do not simplify
                                  " std::cout << expm1l(10.0f);\n" // do not simplify
                                  " std::cout << expm1l(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_expm1l[] = "void f ( long double x ) {\n"
                                       "std :: cout << expm1l ( x ) ;\n"
                                       "std :: cout << expm1l ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_expm1l, tokenizeAndStringify(code_expm1l));
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
        ASSERT_EQUALS(expected_asinh, tokenizeAndStringify(code_asinh));

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
        ASSERT_EQUALS(expected_asinhf, tokenizeAndStringify(code_asinhf));

        const char code_asinhl[] ="void f(long double x) {\n"
                                  " std::cout << asinhl(x);\n" // do not simplify
                                  " std::cout << asinhl(10.0f);\n" // do not simplify
                                  " std::cout << asinhl(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_asinhl[] = "void f ( long double x ) {\n"
                                       "std :: cout << asinhl ( x ) ;\n"
                                       "std :: cout << asinhl ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_asinhl, tokenizeAndStringify(code_asinhl));
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
        ASSERT_EQUALS(expected_asin, tokenizeAndStringify(code_asin));

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
        ASSERT_EQUALS(expected_asinf, tokenizeAndStringify(code_asinf));

        const char code_asinl[] ="void f(long double x) {\n"
                                 " std::cout << asinl(x);\n" // do not simplify
                                 " std::cout << asinl(10.0f);\n" // do not simplify
                                 " std::cout << asinl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_asinl[] = "void f ( long double x ) {\n"
                                      "std :: cout << asinl ( x ) ;\n"
                                      "std :: cout << asinl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_asinl, tokenizeAndStringify(code_asinl));
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
        ASSERT_EQUALS(expected_sinh, tokenizeAndStringify(code_sinh));

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
        ASSERT_EQUALS(expected_sinhf, tokenizeAndStringify(code_sinhf));

        const char code_sinhl[] ="void f(long double x) {\n"
                                 " std::cout << sinhl(x);\n" // do not simplify
                                 " std::cout << sinhl(10.0f);\n" // do not simplify
                                 " std::cout << sinhl(0.0f);\n" // simplify to 0
                                 "}";
        const char expected_sinhl[] = "void f ( long double x ) {\n"
                                      "std :: cout << sinhl ( x ) ;\n"
                                      "std :: cout << sinhl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_sinhl, tokenizeAndStringify(code_sinhl));
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
        ASSERT_EQUALS(expected_sin, tokenizeAndStringify(code_sin));

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
        ASSERT_EQUALS(expected_sinf, tokenizeAndStringify(code_sinf));

        const char code_sinl[] ="void f(long double x) {\n"
                                " std::cout << sinl(x);\n" // do not simplify
                                " std::cout << sinl(10.0f);\n" // do not simplify
                                " std::cout << sinl(0.0f);\n" // simplify to 0
                                "}";
        const char expected_sinl[] = "void f ( long double x ) {\n"
                                     "std :: cout << sinl ( x ) ;\n"
                                     "std :: cout << sinl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_sinl, tokenizeAndStringify(code_sinl));

        // #6629
        const char code[] = "class Foo { int sinf; Foo() : sinf(0) {} };";
        const char expected[] = "class Foo { int sinf ; Foo ( ) : sinf ( 0 ) { } } ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
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
        ASSERT_EQUALS(expected_ilogb, tokenizeAndStringify(code_ilogb));

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
        ASSERT_EQUALS(expected_ilogbf, tokenizeAndStringify(code_ilogbf));

        const char code_ilogbl[] ="void f(long double x) {\n"
                                  " std::cout << ilogbl(x);\n" // do not simplify
                                  " std::cout << ilogbl(10.0f);\n" // do not simplify
                                  " std::cout << ilogbl(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_ilogbl[] = "void f ( long double x ) {\n"
                                       "std :: cout << ilogbl ( x ) ;\n"
                                       "std :: cout << ilogbl ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_ilogbl, tokenizeAndStringify(code_ilogbl));
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
        ASSERT_EQUALS(expected_logb, tokenizeAndStringify(code_logb));

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
        ASSERT_EQUALS(expected_logbf, tokenizeAndStringify(code_logbf));

        const char code_logbl[] ="void f(long double x) {\n"
                                 " std::cout << logbl(x);\n" // do not simplify
                                 " std::cout << logbl(10.0f);\n" // do not simplify
                                 " std::cout << logbl(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_logbl[] = "void f ( long double x ) {\n"
                                      "std :: cout << logbl ( x ) ;\n"
                                      "std :: cout << logbl ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_logbl, tokenizeAndStringify(code_logbl));
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
        ASSERT_EQUALS(expected_log1p, tokenizeAndStringify(code_log1p));

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
        ASSERT_EQUALS(expected_log1pf, tokenizeAndStringify(code_log1pf));

        const char code_log1pl[] ="void f(long double x) {\n"
                                  " std::cout << log1pl(x);\n" // do not simplify
                                  " std::cout << log1pl(10.0f);\n" // do not simplify
                                  " std::cout << log1pl(0.0f);\n" // simplify to 0
                                  "}";
        const char expected_log1pl[] = "void f ( long double x ) {\n"
                                       "std :: cout << log1pl ( x ) ;\n"
                                       "std :: cout << log1pl ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_log1pl, tokenizeAndStringify(code_log1pl));
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
        ASSERT_EQUALS(expected_log10, tokenizeAndStringify(code_log10));

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
        ASSERT_EQUALS(expected_log10f, tokenizeAndStringify(code_log10f));

        const char code_log10l[] ="void f(long double x) {\n"
                                  " std::cout << log10l(x);\n" // do not simplify
                                  " std::cout << log10l(10.0f);\n" // do not simplify
                                  " std::cout << log10l(1.0f);\n" // simplify to 0
                                  "}";
        const char expected_log10l[] = "void f ( long double x ) {\n"
                                       "std :: cout << log10l ( x ) ;\n"
                                       "std :: cout << log10l ( 10.0f ) ;\n"
                                       "std :: cout << 0 ;\n"
                                       "}";
        ASSERT_EQUALS(expected_log10l, tokenizeAndStringify(code_log10l));

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
        ASSERT_EQUALS(expected_log, tokenizeAndStringify(code_log));

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
        ASSERT_EQUALS(expected_logf, tokenizeAndStringify(code_logf));

        const char code_logl[] ="void f(long double x) {\n"
                                " std::cout << logl(x);\n" // do not simplify
                                " std::cout << logl(10.0f);\n" // do not simplify
                                " std::cout << logl(1.0f);\n" // simplify to 0
                                "}";
        const char expected_logl[] = "void f ( long double x ) {\n"
                                     "std :: cout << logl ( x ) ;\n"
                                     "std :: cout << logl ( 10.0f ) ;\n"
                                     "std :: cout << 0 ;\n"
                                     "}";
        ASSERT_EQUALS(expected_logl, tokenizeAndStringify(code_logl));
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
        ASSERT_EQUALS(expected_log2, tokenizeAndStringify(code_log2));

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
        ASSERT_EQUALS(expected_log2f, tokenizeAndStringify(code_log2f));

        const char code_log2l[] ="void f(long double x) {\n"
                                 " std::cout << log2l(x);\n" // do not simplify
                                 " std::cout << log2l(10.0f);\n" // do not simplify
                                 " std::cout << log2l(1.0f);\n" // simplify to 0
                                 "}";
        const char expected_log2l[] = "void f ( long double x ) {\n"
                                      "std :: cout << log2l ( x ) ;\n"
                                      "std :: cout << log2l ( 10.0f ) ;\n"
                                      "std :: cout << 0 ;\n"
                                      "}";
        ASSERT_EQUALS(expected_log2l, tokenizeAndStringify(code_log2l));
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
        ASSERT_EQUALS(expected_pow, tokenizeAndStringify(code_pow));

        // verify if code is simplified correctly.
        // Do not simplify class members.
        const char code_pow1[] = "int f(const Fred &fred) {return fred.pow(12,3);}";
        const char expected_pow1[] = "int f ( const Fred & fred ) { return fred . pow ( 12 , 3 ) ; }";
        ASSERT_EQUALS(expected_pow1, tokenizeAndStringify(code_pow1));

        const char code_pow2[] = "int f() {return pow(0,0);}";
        const char expected_pow2[] = "int f ( ) { return 1 ; }";
        ASSERT_EQUALS(expected_pow2, tokenizeAndStringify(code_pow2));

        const char code_pow3[] = "int f() {return pow(0,1);}";
        const char expected_pow3[] = "int f ( ) { return 0 ; }";
        ASSERT_EQUALS(expected_pow3, tokenizeAndStringify(code_pow3));

        const char code_pow4[] = "int f() {return pow(1,0);}";
        const char expected_pow4[] = "int f ( ) { return 1 ; }";
        ASSERT_EQUALS(expected_pow4, tokenizeAndStringify(code_pow4));
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
        ASSERT_EQUALS(expected_fmin, tokenizeAndStringify(code_fmin));

        // do not simplify this case
        const char code_fmin1[] = "float f(float f) { return fmin(f,0);}";
        const char expected_fmin1[] = "float f ( float f ) { return fmin ( f , 0 ) ; }";
        ASSERT_EQUALS(expected_fmin1, tokenizeAndStringify(code_fmin1));
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
        ASSERT_EQUALS(expected_fmax, tokenizeAndStringify(code_fmax));

        // do not simplify this case
        const char code_fmax1[] = "float f(float f) { return fmax(f,0);}";
        const char expected_fmax1[] = "float f ( float f ) { return fmax ( f , 0 ) ; }";
        ASSERT_EQUALS(expected_fmax1, tokenizeAndStringify(code_fmax1));
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
        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1));

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
        ASSERT_EQUALS(code2, tokenizeAndStringify(code2));

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
        ASSERT_EQUALS(expected3, tokenizeAndStringify(code3));

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
        ASSERT_EQUALS(code4, tokenizeAndStringify(code4));

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
        ASSERT_EQUALS(expected5, tokenizeAndStringify(code5));


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
        ASSERT_EQUALS(code6, tokenizeAndStringify(code6));
    }

    void simplifyStaticConst() {
        const char code1[]     = "class foo { public: bool const static c ; }";
        const char expected1[] = "class foo { public: static const bool c ; }";
        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1, true));

        const char code2[] =
            "int long long f()\n"
            "{\n"
            "static const long long signed int i1;\n"
            "static const long long int signed i2;\n"
            "static const signed long long int i3;\n"
            "static const signed int long long i4;\n"
            "static const int signed long long i5;\n"
            "static const int long long signed i6;\n"
            "long long static const signed int i7;\n"
            "long long static const int signed i8;\n"
            "signed static const long long int i9;\n"
            "signed static const int long long i10;\n"
            "int static const signed long long i11;\n"
            "int static const long long signed i12;\n"
            "long long signed int static const i13;\n"
            "long long int signed static const i14;\n"
            "signed long long int static const i15;\n"
            "signed int long long static const i16;\n"
            "int signed long long static const i17;\n"
            "int long long signed static const i18;\n"
            "return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11 + i12\n"
            "+ i13 + i14 + i15 + i16 + i17 + i18;\n"
            "}";
        const char expected2[] =
            "long long f ( )\n"
            "{\n"
            "static const signed long long i1 ;\n"
            "static const signed long long i2 ;\n"
            "static const signed long long i3 ;\n"
            "static const signed long long i4 ;\n"
            "static const signed long long i5 ;\n"
            "static const signed long long i6 ;\n"
            "static const signed long long i7 ;\n"
            "static const signed long long i8 ;\n"
            "static const signed long long i9 ;\n"
            "static const signed long long i10 ;\n"
            "static const signed long long i11 ;\n"
            "static const signed long long i12 ;\n"
            "static const signed long long i13 ;\n"
            "static const signed long long i14 ;\n"
            "static const signed long long i15 ;\n"
            "static const signed long long i16 ;\n"
            "static const signed long long i17 ;\n"
            "static const signed long long i18 ;\n"
            "return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11 + i12\n"
            "+ i13 + i14 + i15 + i16 + i17 + i18 ;\n"
            "}";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2, true));

        const char code3[] = "const unsigned long extern int i;";
        const char expected3[] = "extern const unsigned long i ;";
        ASSERT_EQUALS(expected3, tokenizeAndStringify(code3, true));
    }

    void simplifyCPPAttribute() {
        ASSERT_EQUALS("int f ( ) ;",
                      tokenizeAndStringify("[[deprecated]] int f();", false, true, Settings::Native, "test.cpp", true));

        ASSERT_EQUALS("[ [ deprecated ] ] int f ( ) ;",
                      tokenizeAndStringify("[[deprecated]] int f();", false, true, Settings::Native, "test.cpp", false));

        ASSERT_EQUALS("[ [ deprecated ] ] int f ( ) ;",
                      tokenizeAndStringify("[[deprecated]] int f();", false, true, Settings::Native, "test.c", true));

        ASSERT_EQUALS("template < class T > int f ( ) { }",
                      tokenizeAndStringify("template <class T> [[noreturn]] int f(){}", false, true, Settings::Native, "test.cpp", true));

        ASSERT_EQUALS("template < class T > [ [ noreturn ] ] int f ( ) { }",
                      tokenizeAndStringify("template <class T> [[noreturn]] int f(){}", false, true, Settings::Native, "test.cpp", false));
        ASSERT_EQUALS("int f ( int i ) ;",
                      tokenizeAndStringify("[[maybe_unused]] int f([[maybe_unused]] int i);", false, true, Settings::Native, "test.cpp", true));

        ASSERT_EQUALS("[ [ maybe_unused ] ] int f ( [ [ maybe_unused ] ] int i ) ;",
                      tokenizeAndStringify("[[maybe_unused]] int f([[maybe_unused]] int i);", false, true, Settings::Native, "test.cpp", false));

    }

    void simplifyCaseRange() {
        ASSERT_EQUALS("void f ( ) { switch ( x ) { case 1 : case 2 : case 3 : case 4 : ; } }", tokenizeAndStringify("void f() { switch(x) { case 1 ... 4: } }"));
        ASSERT_EQUALS("void f ( ) { switch ( x ) { case 4 . . . 1 : ; } }", tokenizeAndStringify("void f() { switch(x) { case 4 ... 1: } }"));
        tokenizeAndStringify("void f() { switch(x) { case 1 ... 1000000: } }"); // Do not run out of memory

        ASSERT_EQUALS("void f ( ) { switch ( x ) { case 'a' : case 'b' : case 'c' : ; } }", tokenizeAndStringify("void f() { switch(x) { case 'a' ... 'c': } }"));
        ASSERT_EQUALS("void f ( ) { switch ( x ) { case 'c' . . . 'a' : ; } }", tokenizeAndStringify("void f() { switch(x) { case 'c' ... 'a': } }"));
    }

    void prepareTernaryOpForAST() {
        ASSERT_EQUALS("a ? b : c ;", tokenizeAndStringify("a ? b : c;"));

        ASSERT_EQUALS("a ? ( b , c ) : d ;", tokenizeAndStringify("a ? b , c : d;"));
        ASSERT_EQUALS("a ? ( b , c ) : d ;", tokenizeAndStringify("a ? (b , c) : d;"));

        ASSERT_EQUALS("a ? ( 1 ? ( a , b ) : 3 ) : d ;", tokenizeAndStringify("a ? 1 ? a, b : 3 : d;"));

        ASSERT_EQUALS("a ? ( std :: map < int , int > ( ) ) : 0 ;", tokenizeAndStringify("typedef std::map<int,int> mymap; a ? mymap() : 0;"));

        ASSERT_EQUALS("a ? ( b < c ) : d > e", tokenizeAndStringify("a ? b < c : d > e"));
    }

    std::string testAst(const char code[],bool verbose=false) {
        // tokenize given code..
        Tokenizer tokenList(&settings0, nullptr);
        std::istringstream istr(code);
        if (!tokenList.list.createTokens(istr,"test.cpp"))
            return "ERROR";

        tokenList.combineOperators();
        tokenList.createLinks();
        tokenList.createLinks2();

        // Create AST..
        tokenList.prepareTernaryOpForAST();
        tokenList.list.createAst();

        tokenList.list.validateAst();

        // Basic AST validation
        for (const Token *tok = tokenList.list.front(); tok; tok = tok->next()) {
            if (tok->astOperand2() && !tok->astOperand1() && tok->str() != ";" && tok->str() != ":")
                return "Op2 but no Op1 for token: " + tok->str();
        }

        // Return stringified AST
        if (verbose)
            return tokenList.list.front()->astTop()->astStringVerbose(0, 0);

        std::string ret;
        std::set<const Token *> astTop;
        for (const Token *tok = tokenList.list.front(); tok; tok = tok->next()) {
            if (tok->astOperand1() && astTop.find(tok->astTop()) == astTop.end()) {
                astTop.insert(tok->astTop());
                if (!ret.empty())
                    ret = ret + " ";
                ret += tok->astTop()->astString();
            }
        }
        return ret;
    }

    void astexpr() { // simple expressions with arithmetical ops
        ASSERT_EQUALS("12+3+", testAst("1+2+3"));
        ASSERT_EQUALS("12*3+", testAst("1*2+3"));
        ASSERT_EQUALS("123*+", testAst("1+2*3"));
        ASSERT_EQUALS("12*34*+", testAst("1*2+3*4"));
        ASSERT_EQUALS("12*34*5*+", testAst("1*2+3*4*5"));
        ASSERT_EQUALS("0(r.&", testAst("(&((typeof(x))0).r);"));
        ASSERT_EQUALS("0(r.&", testAst("&((typeof(x))0).r;"));

        // Various tests of precedence
        ASSERT_EQUALS("ab::c+", testAst("a::b+c"));
        ASSERT_EQUALS("abc+=", testAst("a=b+c"));
        ASSERT_EQUALS("abc=,", testAst("a,b=c"));
        ASSERT_EQUALS("a-1+", testAst("-a+1"));
        ASSERT_EQUALS("ab++-c-", testAst("a-b++-c"));

        // sizeof
        ASSERT_EQUALS("ab.sizeof", testAst("sizeof a.b"));

        // assignment operators
        ASSERT_EQUALS("ab>>=", testAst("a>>=b;"));
        ASSERT_EQUALS("ab<<=", testAst("a<<=b;"));
        ASSERT_EQUALS("ab+=",  testAst("a+=b;"));
        ASSERT_EQUALS("ab-=",  testAst("a-=b;"));
        ASSERT_EQUALS("ab*=",  testAst("a*=b;"));
        ASSERT_EQUALS("ab/=",  testAst("a/=b;"));
        ASSERT_EQUALS("ab%=",  testAst("a%=b;"));
        ASSERT_EQUALS("ab&=",  testAst("a&=b;"));
        ASSERT_EQUALS("ab|=",  testAst("a|=b;"));
        ASSERT_EQUALS("ab^=",  testAst("a^=b;"));

        ASSERT_EQUALS("ab*c*.(+return", testAst("return a + ((*b).*c)();"));

        // assignments are executed from right to left
        ASSERT_EQUALS("abc==", testAst("a=b=c;"));

        // ternary operator
        ASSERT_EQUALS("ab0=c1=:?", testAst("a?b=0:c=1;"));
        ASSERT_EQUALS("fabc,d:?=e,", testAst("f = a ? b, c : d, e;"));
        ASSERT_EQUALS("fabc,de,:?=", testAst("f = (a ? (b, c) : (d, e));"));
        ASSERT_EQUALS("fabc,de,:?=", testAst("f = (a ? b, c : (d, e));"));
        ASSERT_EQUALS("ab35,4:?foo(:?return", testAst("return (a ? b ? (3,5) : 4 : foo());"));
        ASSERT_EQUALS("check(result_type00,{invalid:?return", testAst("return check() ? result_type {0, 0} : invalid;"));

        ASSERT_EQUALS("a\"\"=", testAst("a=\"\""));
        ASSERT_EQUALS("a\'\'=", testAst("a=\'\'"));
        ASSERT_EQUALS("a1[\"\"=", testAst("char a[1]=\"\";"));

        ASSERT_EQUALS("'X''a'>", testAst("('X' > 'a')"));
        ASSERT_EQUALS("'X''a'>", testAst("(L'X' > L'a')"));

        ASSERT_EQUALS("a0>bc/d:?", testAst("(a>0) ? (b/(c)) : d;"));
        ASSERT_EQUALS("abc/+d+", testAst("a + (b/(c)) + d;"));
        ASSERT_EQUALS("f( x1024x/0:?", testAst("void f() { x ? 1024 / x : 0; }"));

        ASSERT_EQUALS("absizeofd(ef.+(=", testAst("a = b(sizeof(c d) + e.f)"));

        ASSERT_EQUALS("a*b***", testAst("*a * **b;")); // Correctly distinguish between unary and binary operator*

        // strings
        ASSERT_EQUALS("f\"A\"1,(",testAst("f(\"A\" B, 1);"));
        ASSERT_EQUALS("fA1,(",testAst("f(A \"B\", 1);"));

        // C++ : type()
        ASSERT_EQUALS("fint(0,(", testAst("f(int(),0);"));
        ASSERT_EQUALS("f(0,(", testAst("f(int *(),0);"));  // typedef int* X; f(X(),0);
        ASSERT_EQUALS("f((0,(", testAst("f((intp)int *(),0);"));
        ASSERT_EQUALS("zx1(&y2(&|=", testAst("z = (x & (unsigned)1) | (y & (unsigned)2);")); // not type()

        // for
        ASSERT_EQUALS("for;;(", testAst("for(;;)"));
        ASSERT_EQUALS("fora0=a8<a++;;(", testAst("for(a=0;a<8;a++)"));
        ASSERT_EQUALS("fori1=current0=,iNUM<=i++;;(", testAst("for(i = (1), current = 0; i <= (NUM); ++i)"));
        ASSERT_EQUALS("foreachxy,((", testAst("for(each(x,y)){}"));  // it's not well-defined what this ast should be
        ASSERT_EQUALS("forab:(", testAst("for (int a : b);"));
        ASSERT_EQUALS("forab:(", testAst("for (int *a : b);"));
        ASSERT_EQUALS("forcd:(", testAst("for (a<b> c : d);"));
        ASSERT_EQUALS("forde:(", testAst("for (a::b<c> d : e);"));
        ASSERT_EQUALS("forx*0=yz;;(", testAst("for(*x=0;y;z)"));
        ASSERT_EQUALS("forx0=y(8<z;;(", testAst("for (x=0;(int)y<8;z);"));

        // problems with multiple expressions
        ASSERT_EQUALS("ax( whilex(", testAst("a(x) while (x)"));
        ASSERT_EQUALS("ifx( i0= whilei(", testAst("if (x) { ({ int i = 0; while(i); }) };"));
        ASSERT_EQUALS("ifx( BUG_ON{!( i0= whilei(", testAst("if (x) { BUG_ON(!({int i=0; while(i);})); }"));
        ASSERT_EQUALS("v0= while{0!=( v0= while{0!=( v0=", testAst("({ v = 0; }); while (({ v = 0; }) != 0); while (({ v = 0; }) != 0);"));


        ASSERT_EQUALS("abc.1:?1+bd.1:?+=", testAst("a =(b.c ? : 1) + 1 + (b.d ? : 1);"));

        ASSERT_EQUALS("try{ catch.(", testAst("try {} catch (...) {}"));

        ASSERT_EQUALS("FooBar(", testAst("void Foo(Bar&);"));
        ASSERT_EQUALS("FooBar(", testAst("void Foo(Bar& &);")); // Rvalue reference - simplified from && to & & by real tokenizer
        ASSERT_EQUALS("DerivedDerived::(", testAst("Derived::~Derived() {}"));

        ASSERT_EQUALS("ifCA_FarReadfilenew(,sizeofobjtype(,(!(", testAst("if (!CA_FarRead(file, (void far *)new, sizeof(objtype)))")); // #5910 - don't hang if C code is parsed as C++

        // Variable declaration
        ASSERT_EQUALS("charp*(3[char5[3[new=", testAst("char (*p)[3] = new char[5][3];"));
    }

    void astexpr2() { // limit for large expressions
        // #7724 - wrong AST causes hang
        // Ideally a proper AST is created for this code.
        const char code[] = "const char * a(int type) {\n"
                            "  return (\n"
                            "   (type == 1) ? \"\"\n"
                            " : (type == 2) ? \"\"\n"
                            " : (type == 3) ? \"\"\n"
                            " : (type == 4) ? \"\"\n"
                            " : (type == 5) ? \"\"\n"
                            " : (type == 6) ? \"\"\n"
                            " : (type == 7) ? \"\"\n"
                            " : (type == 8) ? \"\"\n"
                            " : (type == 9) ? \"\"\n"
                            " : (type == 10) ? \"\"\n"
                            " : (type == 11) ? \"\"\n"
                            " : (type == 12) ? \"\"\n"
                            " : (type == 13) ? \"\"\n"
                            " : (type == 14) ? \"\"\n"
                            " : (type == 15) ? \"\"\n"
                            " : (type == 16) ? \"\"\n"
                            " : (type == 17) ? \"\"\n"
                            " : (type == 18) ? \"\"\n"
                            " : (type == 19) ? \"\"\n"
                            " : (type == 20) ? \"\"\n"
                            " : (type == 21) ? \"\"\n"
                            " : (type == 22) ? \"\"\n"
                            " : (type == 23) ? \"\"\n"
                            " : (type == 24) ? \"\"\n"
                            " : (type == 25) ? \"\"\n"
                            " : (type == 26) ? \"\"\n"
                            " : (type == 27) ? \"\"\n"
                            " : (type == 28) ? \"\"\n"
                            " : (type == 29) ? \"\"\n"
                            " : (type == 30) ? \"\"\n"
                            " : (type == 31) ? \"\"\n"
                            " : (type == 32) ? \"\"\n"
                            " : (type == 33) ? \"\"\n"
                            " : (type == 34) ? \"\"\n"
                            " : (type == 35) ? \"\"\n"
                            " : (type == 36) ? \"\"\n"
                            " : (type == 37) ? \"\"\n"
                            " : (type == 38) ? \"\"\n"
                            " : (type == 39) ? \"\"\n"
                            " : (type == 40) ? \"\"\n"
                            " : (type == 41) ? \"\"\n"
                            " : (type == 42) ? \"\"\n"
                            " : (type == 43) ? \"\"\n"
                            " : (type == 44) ? \"\"\n"
                            " : (type == 45) ? \"\"\n"
                            " : (type == 46) ? \"\"\n"
                            " : (type == 47) ? \"\"\n"
                            " : (type == 48) ? \"\"\n"
                            " : (type == 49) ? \"\"\n"
                            " : (type == 50) ? \"\"\n"
                            " : (type == 51) ? \"\"\n"
                            " : \"\");\n"
                            "}\n";
        // Ensure that the AST is validated for the simplified token list
        tokenizeAndStringify(code); // this does not crash/hang
        ASSERT_THROW(tokenizeAndStringify(code,true), InternalError); // when parentheses are simplified the AST will be wrong
    }

    void astnewdelete() {
        ASSERT_EQUALS("aintnew=", testAst("a = new int;"));
        ASSERT_EQUALS("aint4[new=", testAst("a = new int[4];"));
        ASSERT_EQUALS("aFoobar(new=", testAst("a = new Foo(bar);"));
        ASSERT_EQUALS("aFoobar(new=", testAst("a = new Foo(bar);"));
        ASSERT_EQUALS("aFoo(new=", testAst("a = new Foo<bar>();"));
        ASSERT_EQUALS("X12,3,(new", testAst("new (a,b,c) X(1,2,3);"));
        ASSERT_EQUALS("aXnew(", testAst("a (new (X));"));
        ASSERT_EQUALS("aXnew5,(", testAst("a (new (X), 5);"));
        ASSERT_EQUALS("adelete", testAst("delete a;"));
        ASSERT_EQUALS("adelete", testAst("delete (a);"));
        ASSERT_EQUALS("adelete", testAst("delete[] a;"));
        ASSERT_EQUALS("ab.3c-(delete", testAst("delete[] a.b(3 - c);"));
        ASSERT_EQUALS("a::new=", testAst("a = new (b) ::X;"));
        ASSERT_EQUALS("aA1(new(bB2(new(,", testAst("a(new A(1)), b(new B(2))"));
        ASSERT_EQUALS("Fred10[new", testAst(";new Fred[10];"));
        ASSERT_EQUALS("f( adelete", testAst("void f() { delete a; }"));

        // invalid code (libreoffice), don't hang
        // #define SlideSorterViewShell
        // SfxViewFrame* pFrame;
        // new SlideSorterViewShell(pFrame,rViewShellBase,pParentWindow,pFrameViewArgument);
        ASSERT_EQUALS("fxnewy,z,(", testAst("f(new (x,y,z));"));

        // clang testsuite..
        ASSERT_EQUALS("const0(new", testAst("new const auto (0);"));
        ASSERT_EQUALS("autonew", testAst("new (auto) (0.0);"));
        ASSERT_EQUALS("int3[4[5[new", testAst("new (int S::*[3][4][5]) ();"));
        ASSERT_EQUALS("pSnew=", testAst("p=new (x)(S)(1,2);"));
        ASSERT_EQUALS("inti[new(", testAst("(void)new (int[i]);"));
        ASSERT_EQUALS("intp* pnew malloc4(", testAst("int*p; new (p) (malloc(4));"));
        ASSERT_EQUALS("intnew", testAst("new (&w.x)(int*)(0);"));
        ASSERT_EQUALS("&new", testAst("new (&w.x)(0);")); // <- the "(int*)" has been simplified

        // gcc testsuite..
        ASSERT_EQUALS("char10[new(", testAst("(void)new(char*)[10];"));
    }

    void astpar() { // parentheses
        ASSERT_EQUALS("12+3*", testAst("(1+2)*3"));
        ASSERT_EQUALS("123+*", testAst("1*(2+3)"));
        ASSERT_EQUALS("123+*4*", testAst("1*(2+3)*4"));
        ASSERT_EQUALS("ifab.c&d==(", testAst("if((a.b&c)==d){}"));

        ASSERT_EQUALS("pf.pf.12,(&&", testAst("((p.f) && (p.f)(1,2))"));

        // problems with: if (x[y]==z)
        ASSERT_EQUALS("ifa(0[1==(", testAst("if(a()[0]==1){}"));
        ASSERT_EQUALS("ifbuff0[&(*1==(", testAst("if (*((DWORD*)&buff[0])==1){}"));
        ASSERT_EQUALS("ifp*0[1==(", testAst("if((*p)[0]==1)"));
        ASSERT_EQUALS("ifab.cd.[e==(", testAst("if(a.b[c.d]==e){}"));

        ASSERT_EQUALS("iftpnote.i1-[note.0==tpnote.i1-[type.4>||(", testAst("if ((tp.note[i - 1].note == 0) || (tp.note[i - 1].type > 4)) {}"));
        ASSERT_EQUALS("ab.i[j1+[", testAst("a.b[i][j+1]"));

        // problems with: x=expr
        ASSERT_EQUALS("=\n"
                      "|-x\n"
                      "`-(\n"
                      "  `-.\n"
                      "    |-[\n"
                      "    | |-a\n"
                      "    | `-i\n"
                      "    `-f\n",
                      testAst("x = ((a[i]).f)();", true));
        ASSERT_EQUALS("abc.de.++[=", testAst("a = b.c[++(d.e)];"));
        ASSERT_EQUALS("abc(1+=", testAst("a = b(c**)+1;"));
        ASSERT_EQUALS("abc.=", testAst("a = (b).c;"));

        // casts
        ASSERT_EQUALS("a1(2(+=",testAst("a=(t)1+(t)2;"));
        ASSERT_EQUALS("a1(2+=",testAst("a=(t)1+2;"));
        ASSERT_EQUALS("a1(2+=",testAst("a=(t*)1+2;"));
        ASSERT_EQUALS("a1(2+=",testAst("a=(t&)1+2;"));
        ASSERT_EQUALS("ab::r&c(=", testAst("a::b& r = (a::b&)c;")); // #5261
        ASSERT_EQUALS("ab10:?=", testAst("a=(b)?1:0;"));
        ASSERT_EQUALS("ac5[new(=", testAst("a = (b*)(new c[5]);")); // #8786
        ASSERT_EQUALS("a(4+", testAst("(int)(a) + 4;"));

        // TODO: This AST is incomplete however it's very weird syntax (taken from clang test suite)
        ASSERT_EQUALS("a&(", testAst("(int (**)[i]){&a}[0][1][5] = 0;"));
        ASSERT_EQUALS("n0=", testAst("TrivialDefCtor{[2][2]}[1][1].n = 0;"));
        ASSERT_EQUALS("aT12,3,{1[=", testAst("a = T{1, 2, 3}[1];"));

        // ({..})
        ASSERT_EQUALS("a{+d+ bc+", testAst("a+({b+c;})+d"));
        ASSERT_EQUALS("a{d*+ bc+", testAst("a+({b+c;})*d"));
        ASSERT_EQUALS("xa{((= bc( yd{((= ef(",
                      testAst("x=(int)(a({b(c);}));" // don't hang
                              "y=(int)(d({e(f);}));"));
        ASSERT_EQUALS("QT_WA{{,( x0= QT_WA{{,( x1= x2=",
                      testAst("QT_WA({},{x=0;});" // don't hang
                              "QT_WA({x=1;},{x=2;});"));
        ASSERT_EQUALS("xMACROtype.T=value.1=,{({=",
                      testAst("x = { MACRO( { .type=T, .value=1 } ) }")); // don't hang: MACRO({..})
        ASSERT_EQUALS("fori10=i{;;( i--", testAst("for (i=10;i;({i--;}) ) {}"));

        // function pointer
        TODO_ASSERT_EQUALS("todo", "va_argapvoid((,(*0=", testAst("*va_arg(ap, void(**) ()) = 0;"));

        // struct initialization
        ASSERT_EQUALS("name_bytes[bits~unusedBits>>unusedBits<<{=", testAst("const uint8_t name_bytes[] = { (~bits >> unusedBits) << unusedBits };"));
        ASSERT_EQUALS("abuf.0{={=", testAst("a = { .buf = { 0 } };"));
        ASSERT_EQUALS("ab2[a.0=b.0=,{a.0=b.0=,{,{=", testAst("struct AB ab[2] = { { .a=0, .b=0 }, { .a=0, .b=0 } };"));
        ASSERT_EQUALS("tset{=", testAst("struct cgroup_taskset tset = {};"));
        ASSERT_EQUALS("s1a&,{2b&,{,{=", testAst("s = { {1, &a}, {2, &b} };"));
        ASSERT_EQUALS("s0[L.2[x={=", testAst("s = { [0].L[2] = x};"));
        ASSERT_EQUALS("ac.0={(=", testAst("a = (b){.c=0,};")); // <- useless comma
        ASSERT_EQUALS("xB[1y.z.1={(&=,{={=", testAst("x = { [B] = {1, .y = &(struct s) { .z=1 } } };"));
        ASSERT_EQUALS("xab,c,{=", testAst("x={a,b,(c)};"));

        // struct initialization hang
        ASSERT_EQUALS("sbar.1{,{(={= fcmd( forfieldfield++;;(",
                      testAst("struct S s = {.bar = (struct foo) { 1, { } } };\n"
                              "void f(struct cmd *) { for (; field; field++) {} }"));

        // template parentheses: <>
        ASSERT_EQUALS("stdfabs::m_similarity(numeric_limitsepsilon::(<=return", testAst("return std::fabs(m_similarity) <= numeric_limits<double>::epsilon();")); // #6195

        // C++ initializer
        ASSERT_EQUALS("Class{", testAst("Class{};"));
        ASSERT_EQUALS("Class12,{", testAst("Class{1,2};"));
        ASSERT_EQUALS("Class12,{", testAst("Class<X>{1,2};"));
        ASSERT_EQUALS("abc{d:?=", testAst("a=b?c{}:d;"));
        ASSERT_EQUALS("abc12,{d:?=", testAst("a=b?c{1,2}:d;"));
        ASSERT_EQUALS("abc{d:?=", testAst("a=b?c<X>{}:d;"));
        ASSERT_EQUALS("abc12,{d:?=", testAst("a=b?c<X>{1,2}:d;"));
        ASSERT_EQUALS("a::12,{", testAst("::a{1,2};")); // operator precedence
        ASSERT_EQUALS("Abc({newreturn", testAst("return new A {b(c)};"));
        ASSERT_EQUALS("a{{return", testAst("return{{a}};"));
        ASSERT_EQUALS("a{b{,{return", testAst("return{{a},{b}};"));
    }

    void astbrackets() { // []
        ASSERT_EQUALS("a23+[4+", testAst("a[2+3]+4"));
        ASSERT_EQUALS("a1[0[", testAst("a[1][0]"));
        ASSERT_EQUALS("ab0[=", testAst("a=(b)[0];"));
        ASSERT_EQUALS("abc.0[=", testAst("a=b.c[0];"));
        ASSERT_EQUALS("ab0[1[=", testAst("a=b[0][1];"));
    }

    void astunaryop() { // unary operators
        ASSERT_EQUALS("1a--+", testAst("1 + --a"));
        ASSERT_EQUALS("1a--+", testAst("1 + a--"));
        ASSERT_EQUALS("ab+!", testAst("!(a+b)"));
        ASSERT_EQUALS("ab.++", testAst("++a.b;"));
        ASSERT_EQUALS("ab.++", testAst("a.b++;"));
        ASSERT_EQUALS("ab::++", testAst("a::b++;"));
        ASSERT_EQUALS("c5[--*", testAst("*c[5]--;"));
        ASSERT_EQUALS("xreturn", testAst("return x;"));
        ASSERT_EQUALS("x(throw", testAst(";throw x();"));
        ASSERT_EQUALS("a*bc:?return", testAst("return *a ? b : c;"));
        ASSERT_EQUALS("xy*--=", testAst("x = -- * y;"));

        // Unary :: operator
        ASSERT_EQUALS("abcd::12,(e/:?=", testAst("a = b ? c : ::d(1,2) / e;"));

        // how is "--" handled here:
        ASSERT_EQUALS("ab4<<c--+1:?", testAst("a ? (b << 4) + --c : 1"));
        ASSERT_EQUALS("ab4<<c--+1:?", testAst("a ? (b << 4) + c-- : 1"));
        ASSERT_EQUALS("ai[i= i--", testAst("a[i]=i; --i;"));
    }

    void astfunction() { // function calls
        ASSERT_EQUALS("1f(+2+", testAst("1+f()+2"));
        ASSERT_EQUALS("1f2(+3+", testAst("1+f(2)+3"));
        ASSERT_EQUALS("1f23,(+4+", testAst("1+f(2,3)+4"));
        ASSERT_EQUALS("1f2a&,(+", testAst("1+f(2,&a)"));
        ASSERT_EQUALS("fargv[(", testAst("int f(char argv[]);"));
        ASSERT_EQUALS("fchar(", testAst("extern unsigned f(const char *);"));
        ASSERT_EQUALS("fcharformat*.,(", testAst("extern void f(const char *format, ...);"));
        ASSERT_EQUALS("for_each_commit_graftint((void,(", testAst("extern int for_each_commit_graft(int (*)(int*), void *);"));
        ASSERT_EQUALS("for;;(", testAst("for (;;) {}"));
        ASSERT_EQUALS("xsizeofvoid(=", testAst("x=sizeof(void*)"));
    }

    void asttemplate() { // uninstantiated templates will have <,>,etc..
        ASSERT_EQUALS("a(3==", testAst("a<int>()==3"));
        ASSERT_EQUALS("ab(== f(", testAst("a == b<c>(); f();"));
        ASSERT_EQUALS("static_casta(i[", testAst("; static_cast<char*>(a)[i];")); // #6203
        ASSERT_EQUALS("reinterpret_castreinterpret_castptr(123&(",
                      testAst(";reinterpret_cast<void*>(reinterpret_cast<unsigned>(ptr) & 123);")); // #7253
        ASSERT_EQUALS("bcd.(=", testAst(";a<int> && b = c->d();"));

        // This two unit tests were added to avoid a crash. The actual correct AST result for non-executable code has not been determined so far.
        ASSERT_EQUALS("Cpublica::b:::", testAst("class C : public ::a::b<bool> { };"));
        ASSERT_EQUALS("AB: f( abc+=", testAst("struct A : public B<C*> { void f() { a=b+c; } };"));
    }

    void astcast() {
        ASSERT_EQUALS("ac&(=", testAst("a = (long)&c;"));
        ASSERT_EQUALS("ac*(=", testAst("a = (Foo*)*c;"));
        ASSERT_EQUALS("ac-(=", testAst("a = (long)-c;"));
        ASSERT_EQUALS("ac~(=", testAst("a = (b)~c;"));
        ASSERT_EQUALS("ac(=", testAst("a = (some<strange, type>)c;"));
        ASSERT_EQUALS("afoveon_avgimage((foveon_avgimage((+=", testAst("a = foveon_avg(((short(*)[4]) image)) + foveon_avg(((short(*)[4]) image));"));
        ASSERT_EQUALS("c(40<<return", testAst("return (long long)c << 40;"));
        ASSERT_EQUALS("ab-(=", testAst("a = ((int)-b)")); // Multiple subsequent unary operators (cast and -)
        ASSERT_EQUALS("xdouble123(i*(=", testAst("x = (int)(double(123)*i);"));
        ASSERT_EQUALS("ac(=", testAst("a = (::b)c;"));
        ASSERT_EQUALS("abcd,({(=", testAst("a = (s){b(c, d)};"));
        ASSERT_EQUALS("xatoistr({(=", testAst("x = (struct X){atoi(str)};"));
        ASSERT_EQUALS("xa.0=b.0=,c.0=,{(=", testAst("x = (struct abc) { .a=0, .b=0, .c=0 };"));

        ASSERT_EQUALS("yz.(return", testAst("return (x)(y).z;"));

        // not cast
        ASSERT_EQUALS("AB||", testAst("(A)||(B)"));
        ASSERT_EQUALS("abc[1&=", testAst("a = (b[c]) & 1;"));
        ASSERT_EQUALS("abc::(=", testAst("a = (b::c)();"));
    }

    void astlambda() {
        // a lambda expression '[x](y){}' is compiled as:
        // [
        // `-(
        //   `-{
        ASSERT_EQUALS("x{([( ai=", testAst("x([&a](int i){a=i;});"));

        ASSERT_EQUALS("{([(return 0return", testAst("return [](){ return 0; }();"));
        ASSERT_EQUALS("{([(return 0return", testAst("return []() -> int { return 0; }();"));
        ASSERT_EQUALS("{([(return 0return", testAst("return [something]() -> int { return 0; }();"));
        ASSERT_EQUALS("{([cd,(return 0return", testAst("return [](int a, int b) -> int { return 0; }(c, d);"));
        ASSERT_EQUALS("x{([=", testAst("x = [&]()->std::string const & {};"));
        ASSERT_EQUALS("f{([=", testAst("f = []() -> foo* {};"));
        ASSERT_EQUALS("f{([=", testAst("f = [](void) mutable -> foo* {};"));
        ASSERT_EQUALS("f{([=", testAst("f = []() mutable {};"));

        ASSERT_EQUALS("x{([= 0return", testAst("x = [](){return 0; };"));

        ASSERT_EQUALS("ab{[(= cd=", testAst("a = b([&]{c=d;});"));

        // 8628
        ASSERT_EQUALS("f{([( switchx( 1case y++", testAst("f([](){switch(x){case 1:{++y;}}});"));
    }

    void astcase() {
        ASSERT_EQUALS("0case", testAst("case 0:"));
        ASSERT_EQUALS("12+case", testAst("case 1+2:"));
        ASSERT_EQUALS("xyz:?case", testAst("case (x?y:z):"));
    }

    void compileLimits() {
        const char raw_code[] = "#define PTR1 (* (* (* (* (* (* (* (* (* (*\n"
                                "#define PTR2 PTR1 PTR1 PTR1 PTR1 PTR1 PTR1 PTR1 PTR1 PTR1 PTR1\n"
                                "#define PTR3 PTR2 PTR2 PTR2 PTR2 PTR2 PTR2 PTR2 PTR2 PTR2 PTR2\n"
                                "#define PTR4 PTR3 PTR3 PTR3 PTR3 PTR3 PTR3 PTR3 PTR3 PTR3 PTR3\n"
                                "#define PTR5 PTR4 PTR4 PTR4 PTR4 PTR4 PTR4 PTR4 PTR4 PTR4 PTR4\n"
                                "#define PTR6 PTR5 PTR5 PTR5 PTR5 PTR5 PTR5 PTR5 PTR5 PTR5 PTR5\n"
                                "\n"
                                "#define RBR1 ) ) ) ) ) ) ) ) ) )\n"
                                "#define RBR2 RBR1 RBR1 RBR1 RBR1 RBR1 RBR1 RBR1 RBR1 RBR1 RBR1\n"
                                "#define RBR3 RBR2 RBR2 RBR2 RBR2 RBR2 RBR2 RBR2 RBR2 RBR2 RBR2\n"
                                "#define RBR4 RBR3 RBR3 RBR3 RBR3 RBR3 RBR3 RBR3 RBR3 RBR3 RBR3\n"
                                "#define RBR5 RBR4 RBR4 RBR4 RBR4 RBR4 RBR4 RBR4 RBR4 RBR4 RBR4\n"
                                "#define RBR6 RBR5 RBR5 RBR5 RBR5 RBR5 RBR5 RBR5 RBR5 RBR5 RBR5\n"
                                "\n"
                                "int PTR4 q4_var RBR4 = 0;\n";

        // Preprocess file..
        Preprocessor preprocessor(settings0);
        std::list<std::string> configurations;
        std::string filedata;
        std::istringstream fin(raw_code);
        preprocessor.preprocess(fin, filedata, configurations, emptyString, settings0.includePaths);
        const std::string code = preprocessor.getcode(filedata, emptyString, emptyString);

        tokenizeAndStringify(code.c_str()); // just survive...
    }

    bool isStartOfExecutableScope(int offset, const char code[]) {
        Tokenizer tokenizer(&settings0, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        return Tokenizer::startOfExecutableScope(tokenizer.tokens()->tokAt(offset)) != nullptr;
    }

    void startOfExecutableScope() {
        ASSERT(isStartOfExecutableScope(3, "void foo() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() volatile { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() override { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() noexcept { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() NOEXCEPT { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() CONST NOEXCEPT { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const noexcept { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() noexcept(true) { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const noexcept(true) { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() throw() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() THROW() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() CONST THROW() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const throw() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() throw(int) { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const throw(int) { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a(1) { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a(1), b(2) { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a{1} { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a{1}, b{2} { }"));
    }

    void removeMacroInClassDef() { // #6058
        ASSERT_EQUALS("class Fred { } ;", tokenizeAndStringify("class DLLEXPORT Fred { } ;"));
        ASSERT_EQUALS("class Fred : Base { } ;", tokenizeAndStringify("class Fred FINAL : Base { } ;"));
        // Regression for C code:
        ASSERT_EQUALS("struct Fred { } ;", tokenizeAndStringify("struct DLLEXPORT Fred { } ;", false, true, Settings::Native, "test.c"));
    }

    void sizeofAddParentheses() {
        ASSERT_EQUALS("sizeof ( sizeof ( 1 ) ) ;", tokenizeAndStringify("sizeof sizeof 1;"));
        ASSERT_EQUALS("sizeof ( a . b ) + 3 ;", tokenizeAndStringify("sizeof a.b+3;"));
        ASSERT_EQUALS("sizeof ( a [ 2 ] . b ) + 3 ;", tokenizeAndStringify("sizeof a[2].b+3;"));
        ASSERT_EQUALS("f ( 0 , sizeof ( ptr . bar ) ) ;", tokenizeAndStringify("f(0, sizeof ptr->bar );"));
    }

    void findGarbageCode() { // Make sure the Tokenizer::findGarbageCode() does not have FPs
        // before if|for|while|switch
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { do switch (a) {} while (1); }"))
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { label: switch (a) {} }"));
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { UNKNOWN_MACRO if (a) {} }"))
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { []() -> int * {}; }"));
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { const char* var = \"1\" \"2\"; }"));
        // TODO ASSERT_NO_THROW(tokenizeAndStringify("void f() { MACRO(switch); }"));
        // TODO ASSERT_NO_THROW(tokenizeAndStringify("void f() { MACRO(x,switch); }"));

        // after (expr)
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { switch (a) int b; }"));
    }


    void checkEnableIf() {
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template<\n"
                            "    typename U,\n"
                            "    typename std::enable_if<\n"
                            "        std::is_convertible<U, T>{}>::type* = nullptr>\n"
                            "void foo(U x);\n"))

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template<class t>\n"
                            "T f(const T a, const T b) {\n"
                            "    return a < b ? b : a;\n"
                            "}\n"))

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template<class T>\n"
                            "struct A {\n"
                            "    T f(const T a, const T b) {\n"
                            "        return a < b ? b : a;\n"
                            "    }\n"
                            "};\n"))

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "const int a = 1;\n"
                            "const int b = 2;\n"
                            "template<class T>\n"
                            "struct A {\n"
                            "    int x = a < b ? b : a;"
                            "};\n"))

    }

    void checkConfig(const char code[]) {
        errout.str("");

        Settings s;
        s.checkConfiguration = true;

        // tokenize..
        Tokenizer tokenizer(&s, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
    }

    void checkConfiguration() {
        checkConfig("void f() { DEBUG(x();y()); }");
        ASSERT_EQUALS("[test.cpp:1]: (information) Ensure that 'DEBUG' is defined either using -I, --include or -D.\n", errout.str());
    }
};

REGISTER_TEST(TestTokenizer)
