/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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

#include "exprengine.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenize.h"
#include "testsuite.h"

class TestExprEngine : public TestFixture {
public:
    TestExprEngine() : TestFixture("TestExprEngine") {
    }

private:
    void run() OVERRIDE {
        TEST_CASE(argPointer);
        TEST_CASE(argStruct);

        TEST_CASE(expr1);
        TEST_CASE(expr2);
        TEST_CASE(expr3);
        TEST_CASE(expr4);
        TEST_CASE(expr5);

        TEST_CASE(functionCall1);

        TEST_CASE(if1);
        TEST_CASE(if2);
        TEST_CASE(if3);
        TEST_CASE(if4);
        TEST_CASE(if5);

        TEST_CASE(ifelse1);

        TEST_CASE(localArray1);
        TEST_CASE(localArrayUninit);

        TEST_CASE(pointerAlias1);
        TEST_CASE(pointerAlias2);
    }

    std::string getRange(const char code[], const std::string &str) {
        Settings settings;
        settings.platform(cppcheck::Platform::Unix64);
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::string ret;
        std::function<void(const Token *, const ExprEngine::Value &)> f = [&](const Token *tok, const ExprEngine::Value &value) {
            if (tok->expressionString() == str)
                ret += value.getRange();
        };
        std::vector<ExprEngine::Callback> callbacks;
        callbacks.push_back(f);
        ExprEngine::executeAllFunctions(&tokenizer, &settings, callbacks);
        return ret;
    }

    void argPointer() {
        ASSERT_EQUALS("?", getRange("void f(unsigned char *p) { a = *p; }", "*p"));
    }

    void argStruct() {
        ASSERT_EQUALS("[0:510]",
                      getRange("struct S {\n"
                               "    unsigned char a;\n"
                               "    unsigned char b;\n"
                               "};\n"
                               "void f(struct S s) { return s.a + s.b; }", "s.a+s.b"));
    }

    void expr1() {
        ASSERT_EQUALS("[-32768:32767]", getRange("void f(short x) { a = x; }", "x"));
    }

    void expr2() {
        ASSERT_EQUALS("[-65536:65534]", getRange("void f(short x) { a = x + x; }", "x+x"));
    }

    void expr3() {
        ASSERT_EQUALS("[-65536:65534]", getRange("int f(short x) { int a = x + x; return a; }", "return a"));
    }

    void expr4() {
        ASSERT_EQUALS("[0:0]", getRange("int f(short x) { int a = x - x; return a; }", "return a"));
    }

    void expr5() {
        ASSERT_EQUALS("[-65536:65534]", getRange("void f(short a, short b, short c, short d) { if (a+b<c+d) {} }", "a+b"));
    }

    void functionCall1() {
        ASSERT_EQUALS("[-2147483648:2147483647]", getRange("int atoi(const char *p); void f() { int x = atoi(a); x = x; }", "x=x"));
    }

    void if1() {
        ASSERT_EQUALS("[7:32768]", getRange("inf f(short x) { if (x > 5) a = x + 1; }", "x+1"));
    }

    void if2() {
        ASSERT_EQUALS("[7:32768][-32767:6]", getRange("inf f(short x) { if (x > 5) {} a = x + 1; }", "x+1"));
    }

    void if3() {
        ASSERT_EQUALS("[1:1][-2147483648:2147483647][-2147483648:2147483647]", getRange("void f() { int x; if (a) { if (b) x=1; } a=x; }", "a=x"));
    }

    void if4() {
        ASSERT_EQUALS("[1:2147483647][-2147483648:-1]", getRange("int x; void f() { if (x) { a=x; }}", "a=x"));
    }

    void if5() {
        ASSERT_EQUALS("[0:0]", getRange("int x; void f() { if (x) {} else { a=x; }}", "a=x"));
    }

    void ifelse1() {
        ASSERT_EQUALS("[-32767:6]", getRange("inf f(short x) { if (x > 5) ; else a = x + 1; }", "x+1"));
    }

    void localArray1() {
        ASSERT_EQUALS("[5:5]", getRange("inf f() { int arr[10]; arr[4] = 5; return arr[4]; }", "arr[4]"));
    }

    void localArrayUninit() {
        ASSERT_EQUALS("?", getRange("inf f() { int arr[10]; return arr[4]; }", "arr[4]"));
    }

    void pointerAlias1() {
        ASSERT_EQUALS("[3:3]", getRange("inf f() { int x; int *p = &x; x = 3; return *p; }", "return*p"));
    }

    void pointerAlias2() {
        ASSERT_EQUALS("[1:1]", getRange("inf f() { int x; int *p = &x; *p = 1; return *p; }", "return*p"));
    }
};

REGISTER_TEST(TestExprEngine)
