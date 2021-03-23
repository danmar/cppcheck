/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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


#include "astutils.h"
#include "settings.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstring>

class TestAstUtils : public TestFixture {
public:
    TestAstUtils() : TestFixture("TestAstUtils") {
    }

private:

    void run() OVERRIDE {
        TEST_CASE(findLambdaEndToken);
        TEST_CASE(findLambdaStartToken);
        TEST_CASE(isNullOperand);
        TEST_CASE(isReturnScope);
        TEST_CASE(isSameExpression);
        TEST_CASE(isVariableChanged);
        TEST_CASE(isVariableChangedByFunctionCall);
        TEST_CASE(nextAfterAstRightmostLeaf);
        TEST_CASE(isUsedAsBool);
    }

    bool findLambdaEndToken(const char code[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const tokEnd = ::findLambdaEndToken(tokenizer.tokens());
        return tokEnd && tokEnd->next() == nullptr;
    }

    void findLambdaEndToken() {
        const Token* nullTok = nullptr;
        ASSERT(nullptr == ::findLambdaEndToken(nullTok));
        ASSERT_EQUALS(false, findLambdaEndToken("void f() { }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[]{ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[]{ return 0; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[&](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[&, i](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) mutable { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) constexpr { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) mutable -> int { return -1; }"));
        ASSERT_EQUALS(false, findLambdaEndToken("[](void) foo -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> int* { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) mutable -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const ** int { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const * const* int { return x; }"));
    }

    bool findLambdaStartToken(const char code[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const tokStart = ::findLambdaStartToken(tokenizer.list.back());
        return tokStart && tokStart == tokenizer.list.front();
    }

    void findLambdaStartToken() {
        ASSERT(nullptr == ::findLambdaStartToken(nullptr));
        ASSERT_EQUALS(false, findLambdaStartToken("void f() { }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[]{ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[]{ return 0; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](){ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[&](){ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[&, i](){ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](int a, int b) { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](int a, int b) mutable { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](int a, int b) constexpr { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) mutable -> int { return -1; }"));
        ASSERT_EQUALS(false, findLambdaStartToken("[](void) foo -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> int* { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) mutable -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> const ** int { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> const * const* int { return x; }"));
    }

    bool isNullOperand(const char code[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        return ::isNullOperand(tokenizer.tokens());
    }

    void isNullOperand() {
        ASSERT_EQUALS(true, isNullOperand("(void*)0;"));
        ASSERT_EQUALS(true, isNullOperand("(void*)0U;"));
        ASSERT_EQUALS(true, isNullOperand("(void*)0x0LL;"));
        ASSERT_EQUALS(true, isNullOperand("NULL;"));
        ASSERT_EQUALS(true, isNullOperand("nullptr;"));
        ASSERT_EQUALS(true, isNullOperand("(void*)NULL;"));
        ASSERT_EQUALS(true, isNullOperand("static_cast<int*>(0);"));
        ASSERT_EQUALS(false, isNullOperand("0;"));
        ASSERT_EQUALS(false, isNullOperand("(void*)0.0;"));
        ASSERT_EQUALS(false, isNullOperand("(void*)1;"));
    }

    bool isReturnScope(const char code[], int offset) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const tok = (offset < 0)
                                  ? tokenizer.list.back()->tokAt(1+offset)
                                  : tokenizer.tokens()->tokAt(offset);
        return ::isReturnScope(tok);
    }

    void isReturnScope() {
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return; } }", -2));
        ASSERT_EQUALS(true, isReturnScope("int f() { if (a) { return {}; } }", -2));                        // #8891
        ASSERT_EQUALS(true, isReturnScope("std::string f() { if (a) { return std::string{}; } }", -2));     // #8891
        ASSERT_EQUALS(true, isReturnScope("std::string f() { if (a) { return std::string{\"\"}; } }", -2)); // #8891
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return (ab){0}; } }", -2)); // #7103
        ASSERT_EQUALS(false, isReturnScope("void f() { if (a) { return (ab){0}; } }", -4)); // #7103
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -4)); // #7144
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -2)); // #7144
        ASSERT_EQUALS(false, isReturnScope("void f() { [=]() { return data; }; }", -1));
        ASSERT_EQUALS(true, isReturnScope("auto f() { return [=]() { return data; }; }", -1));
        ASSERT_EQUALS(true, isReturnScope("auto f() { return [=]() { return data; }(); }", -1));
        ASSERT_EQUALS(false, isReturnScope("auto f() { [=]() { return data; }(); }", -1));

        ASSERT_EQUALS(true, isReturnScope("void negativeTokenOffset() { return; }", -1));
        ASSERT_EQUALS(false, isReturnScope("void zeroTokenOffset() { return; }", 0));
        ASSERT_EQUALS(true, isReturnScope("void positiveTokenOffset() { return; }", 7));
    }

    bool isSameExpression(const char code[], const char tokStr1[], const char tokStr2[]) {
        Settings settings;
        Library library;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokens1("");
        const Token * const tok1 = Token::findsimplematch(tokenizer.tokens(), tokStr1, strlen(tokStr1));
        const Token * const tok2 = Token::findsimplematch(tok1->next(), tokStr2, strlen(tokStr2));
        return ::isSameExpression(false, false, tok1, tok2, library, false, true, nullptr);
    }

    void isSameExpression() {
        ASSERT_EQUALS(true,  isSameExpression("x = 1 + 1;", "1", "1"));
        ASSERT_EQUALS(false, isSameExpression("x = 1 + 1u;", "1", "1u"));
        ASSERT_EQUALS(true,  isSameExpression("x = 1.0 + 1.0;", "1.0", "1.0"));
        ASSERT_EQUALS(false, isSameExpression("x = 1.0f + 1.0;", "1.0f", "1.0"));
        ASSERT_EQUALS(false, isSameExpression("x = 1L + 1;", "1L", "1"));
        ASSERT_EQUALS(true,  isSameExpression("x = 0.0f + 0x0p+0f;", "0.0f", "0x0p+0f"));
        ASSERT_EQUALS(true,  isSameExpression("x < x;", "x", "x"));
        ASSERT_EQUALS(false, isSameExpression("x < y;", "x", "y"));
        ASSERT_EQUALS(true,  isSameExpression("(x + 1) < (x + 1);", "+", "+"));
        ASSERT_EQUALS(false, isSameExpression("(x + 1) < (x + 1L);", "+", "+"));
        ASSERT_EQUALS(true,  isSameExpression("(1 + x) < (x + 1);", "+", "+"));
        ASSERT_EQUALS(false, isSameExpression("(1.0l + x) < (1.0 + x);", "+", "+"));
        ASSERT_EQUALS(true,  isSameExpression("(0.0 + x) < (x + 0x0p+0);", "+", "+"));
        ASSERT_EQUALS(true,  isSameExpression("void f() {double y = 1e1; (x + y) < (x + 10.0); } ", "+", "+"));
        ASSERT_EQUALS(true,  isSameExpression("void f() {double y = 1e1; (x + 10.0) < (y + x); } ", "+", "+"));
        ASSERT_EQUALS(true,  isSameExpression("void f() {double y = 1e1; double z = 10.0; (x + y) < (x + z); } ", "+", "+"));
        ASSERT_EQUALS(true,  isSameExpression("A + A", "A", "A"));

        //https://trac.cppcheck.net/ticket/9700
        ASSERT_EQUALS(true, isSameExpression("A::B + A::B;", "::", "::"));
        ASSERT_EQUALS(false, isSameExpression("A::B + A::C;", "::", "::"));
        ASSERT_EQUALS(true, isSameExpression("A::B* get() { if(x) return new A::B(true); else return new A::B(true); }", "new", "new"));
        ASSERT_EQUALS(false, isSameExpression("A::B* get() { if(x) return new A::B(true); else return new A::C(true); }", "new", "new"));
        ASSERT_EQUALS(true, true);
    }

    bool isVariableChanged(const char code[], const char startPattern[], const char endPattern[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const tok1 = Token::findsimplematch(tokenizer.tokens(), startPattern, strlen(startPattern));
        const Token * const tok2 = Token::findsimplematch(tokenizer.tokens(), endPattern, strlen(endPattern));
        return ::isVariableChanged(tok1,tok2,1,false,&settings,true);
    }

    void isVariableChanged() {
        // #8211 - no lhs for >> , do not crash
        isVariableChanged("void f() {\n"
                          "  int b;\n"
                          "  if (b) { (int)((INTOF(8))result >> b); }\n"
                          "}", "if", "}");
        // #9235
        ASSERT_EQUALS(true, isVariableChanged("void f() {\n"
                                              "    int &a = a;\n"
                                              "}\n", "= a", "}"));
    }

    bool isVariableChangedByFunctionCall(const char code[], const char pattern[], bool *inconclusive) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const argtok = Token::findmatch(tokenizer.tokens(), pattern);
        return ::isVariableChangedByFunctionCall(argtok, 0, &settings, inconclusive);
    }

    void isVariableChangedByFunctionCall() {
        const char *code;
        bool inconclusive;

        // #8271 - template method
        code = "void f(int x) {\n"
               "  a<int>(x);\n"
               "}";
        inconclusive = false;
        ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "x ) ;", &inconclusive));
        ASSERT_EQUALS(true, inconclusive);
    }

    bool nextAfterAstRightmostLeaf(const char code[], const char parentPattern[], const char rightPattern[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * tok = Token::findsimplematch(tokenizer.tokens(), parentPattern, strlen(parentPattern));
        return Token::simpleMatch(::nextAfterAstRightmostLeaf(tok), rightPattern, strlen(rightPattern));
    }

    void nextAfterAstRightmostLeaf() {
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("void f(int a, int b) { int x = a + b; }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a); }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a)[b]; }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(g(a)[b]); }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(g(a)[b] + a); }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a)[b + 1]; }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("void f() { int a; int b; int x = [](int a){}; }", "=", "; }"));

        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = a + b; }", "+", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a)[b + 1]; }", "+", "] ; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a + 1)[b]; }", "+", ") ["));
    }

    enum class Result {False, True, Fail};

    Result isUsedAsBool(const char code[], const char pattern[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        if (!tokenizer.tokenize(istr, "test.cpp"))
            return Result::Fail;
        const Token * const argtok = Token::findmatch(tokenizer.tokens(), pattern);
        if (!argtok)
            return Result::Fail;
        return ::isUsedAsBool(argtok) ? Result::True : Result::False;
    }

    void isUsedAsBool() {
        ASSERT(Result::True == isUsedAsBool("void f() { bool b = true; }", "b"));
        ASSERT(Result::False ==isUsedAsBool("void f() { int i = true; }", "i"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; while (i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; for (;i;) {} }", "i ; )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; for (;;i) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; for (i;;) {} }", "i ; ; )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; for (int j=0; i; ++j) {} }", "i ; ++"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (i == 2) {} }", "i =="));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (i == true) {} }", "i =="));
        ASSERT(Result::True == isUsedAsBool("void f() { int i,j; if (i == (j&&f())) {} }", "i =="));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (!i == 0) {} }", "i =="));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (!i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (!!i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (i && f()) {} }", "i &&"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; int j = i && f(); }", "i &&"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (i & f()) {} }", "i &"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (static_cast<bool>(i)) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if ((bool)i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (1+static_cast<bool>(i)) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (1+(bool)i) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (1+static_cast<int>(i)) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (1+!static_cast<int>(i)) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (1+(int)i) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (i + 2) {} }", "i +"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; bool b = i; }", "i ; }"));
        ASSERT(Result::True == isUsedAsBool("void f(bool b); void f() { int i; f(i); }","i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int *i; if (*i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int *i; if (*i) {} }", "* i )"));
    }
};

REGISTER_TEST(TestAstUtils)
