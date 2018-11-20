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


#include "astutils.h"
#include "settings.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"


class TestAstUtils : public TestFixture {
public:
    TestAstUtils() : TestFixture("TestAstUtils") {
    }

private:

    void run() override {
        TEST_CASE(findLambdaEndToken);
        TEST_CASE(isReturnScope);
        TEST_CASE(isVariableChanged);
        TEST_CASE(isVariableChangedByFunctionCall);
        TEST_CASE(nextAfterAstRightmostLeaf);
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
        ASSERT(nullptr == ::findLambdaEndToken(nullptr));
        ASSERT_EQUALS(false, findLambdaEndToken("void f() { }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[]{ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[]{ return 0 }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[&](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[&, i](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) { return -1 }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) { return a + b }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) mutable { return a + b }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) constexpr { return a + b }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) -> int { return -1 }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) mutable -> int { return -1 }"));
        ASSERT_EQUALS(false, findLambdaEndToken("[](void) foo -> int { return -1 }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> int { return -1 }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> int* { return x }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const * int { return x }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) mutable -> const * int { return x }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const ** int { return x }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const * const* int { return x }"));
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
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return (ab){0}; } }", -2)); // #7103
        ASSERT_EQUALS(false, isReturnScope("void f() { if (a) { return (ab){0}; } }", -4)); // #7103
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -4)); // #7144
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -2)); // #7144
    }

    bool isVariableChanged(const char code[], const char startPattern[], const char endPattern[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const tok1 = Token::findsimplematch(tokenizer.tokens(), startPattern);
        const Token * const tok2 = Token::findsimplematch(tokenizer.tokens(), endPattern);
        return ::isVariableChanged(tok1,tok2,1,false,&settings,true);
    }

    void isVariableChanged() {
        // #8211 - no lhs for >> , do not crash
        isVariableChanged("void f() {\n"
                          "  int b;\n"
                          "  if (b) { (int)((INTOF(8))result >> b); }\n"
                          "}", "if", "}");
    }

    bool isVariableChangedByFunctionCall(const char code[], const char pattern[], bool *inconclusive) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        const Token * const argtok = Token::findmatch(tokenizer.tokens(), pattern);
        return ::isVariableChangedByFunctionCall(argtok, &settings, inconclusive);
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
        const Token * tok = Token::findsimplematch(tokenizer.tokens(), parentPattern);
        return Token::simpleMatch(::nextAfterAstRightmostLeaf(tok), rightPattern);
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
};

REGISTER_TEST(TestAstUtils)
