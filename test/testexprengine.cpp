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

#include <limits>
#include <string>

class TestExprEngine : public TestFixture {
public:
    TestExprEngine() : TestFixture("TestExprEngine") {
    }

private:
    void run() OVERRIDE {
#ifdef USE_Z3
        TEST_CASE(expr1);
        TEST_CASE(expr2);
        TEST_CASE(expr3);
        TEST_CASE(expr4);
        TEST_CASE(expr5);
        TEST_CASE(exprAssign1);
        TEST_CASE(exprAssign2); // Truncation

        TEST_CASE(if1);
        TEST_CASE(ifelse1);

        TEST_CASE(array1);
        TEST_CASE(array2);
        TEST_CASE(array3);
        TEST_CASE(array4);
        TEST_CASE(arrayInit1);
        TEST_CASE(arrayInit2);
        TEST_CASE(arrayUninit);

        TEST_CASE(floatValue1);
        TEST_CASE(floatValue2);

        TEST_CASE(functionCall1);
        TEST_CASE(functionCall2);
        TEST_CASE(functionCall3);

        TEST_CASE(int1);

        TEST_CASE(pointer1);
        TEST_CASE(pointerAlias1);
        TEST_CASE(pointerAlias2);
        TEST_CASE(pointerAlias3);
        TEST_CASE(pointerAlias4);
        TEST_CASE(pointerNull1);

        TEST_CASE(structMember);
#endif
    }

    std::string expr(const char code[], const std::string &binop) {
        Settings settings;
        settings.platform(cppcheck::Platform::Unix64);
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::string ret;
        std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> f = [&](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
            if (tok->str() != binop)
                return;
            auto b = dynamic_cast<const ExprEngine::BinOpResult *>(&value);
            if (!b)
                return;
            ret = b->getExpr(dataBase);
        };
        std::vector<ExprEngine::Callback> callbacks;
        callbacks.push_back(f);
        std::ostringstream dummy;
        ExprEngine::executeAllFunctions(&tokenizer, &settings, callbacks, dummy);
        return ret;
    }

    std::string getRange(const char code[], const std::string &str, int linenr = 0) {
        Settings settings;
        settings.platform(cppcheck::Platform::Unix64);
        settings.library.smartPointers.insert("std::shared_ptr");
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::string ret;
        std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> f = [&](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
            (void)dataBase;
            if ((linenr == 0 || linenr == tok->linenr()) && tok->expressionString() == str) {
                if (!ret.empty())
                    ret += ",";
                ret += value.getRange();
            }
        };
        std::vector<ExprEngine::Callback> callbacks;
        callbacks.push_back(f);
        std::ostringstream dummy;
        ExprEngine::executeAllFunctions(&tokenizer, &settings, callbacks, dummy);
        return ret;
    }

    std::string trackExecution(const char code[]) {
        Settings settings;
        settings.debugVerification = true;
        settings.platform(cppcheck::Platform::Unix64);
        settings.library.smartPointers.insert("std::shared_ptr");
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::vector<ExprEngine::Callback> dummy;
        std::ostringstream ret;
        ExprEngine::executeAllFunctions(&tokenizer, &settings, dummy, ret);
        return ret.str();
    }

    void expr1() {
        ASSERT_EQUALS("-32768:32767", getRange("void f(short x) { a = x; }", "x"));
    }

    void expr2() {
        ASSERT_EQUALS("($1)+($1)", getRange("void f(short x) { a = x + x; }", "x+x"));
    }

    void expr3() {
        ASSERT_EQUALS("($1)+($1)", getRange("int f(short x) { int a = x + x; return a; }", "return a"));
    }

    void expr4() {
        ASSERT_EQUALS("($1)-($1)", getRange("int f(short x) { int a = x - x; return a; }", "return a"));
    }

    void expr5() {
        ASSERT_EQUALS("($1)+($2)", getRange("void f(short a, short b, short c, short d) { if (a+b<c+d) {} }", "a+b"));
    }

    void exprAssign1() {
        ASSERT_EQUALS("($1)+(1)", getRange("void f(unsigned char a) { a += 1; }", "a+=1"));
    }

    void exprAssign2() {
        ASSERT_EQUALS("2", getRange("void f(unsigned char x) { x = 258; int a = x }", "a=x"));
    }

    void if1() {
        ASSERT_EQUALS("(declare-fun v0 () Int)\n"
                      "(declare-fun v1 () Int)\n"
                      "(assert (<= v0 2147483647))\n"
                      "(assert (>= v0 (- 2147483648)))\n"
                      "(assert (<= v1 2147483647))\n"
                      "(assert (>= v1 (- 2147483648)))\n"
                      "(assert (< v0 v1))\n"
                      "(assert (= v0 v1))\n",
                      expr("void f(int x, int y) { if (x < y) return x == y; }", "=="));
    }

    void ifelse1() {
        ASSERT_EQUALS("(declare-fun v0 () Int)\n"
                      "(assert (<= v0 32767))\n"
                      "(assert (>= v0 (- 32768)))\n"
                      "(assert (<= v0 5))\n"
                      "(assert (= (+ v0 2) 40))\n",
                      expr("void f(short x) { if (x > 5) ; else if (x+2==40); }", "=="));
    }


    void array1() {
        ASSERT_EQUALS("(assert (= 5 0))\n",
                      expr("int f() { int arr[10]; arr[4] = 5; return arr[4]==0; }", "=="));
    }

    void array2() {
        ASSERT_EQUALS("(declare-fun v0 () Int)\n"
                      "(assert (<= v0 255))\n"
                      "(assert (>= v0 0))\n"
                      "(assert (= v0 365))\n",
                      expr("void dostuff(unsigned char *); int f() { unsigned char arr[10] = \"\"; dostuff(arr); return arr[4] == 365; }", "=="));
    }

    void array3() {
        const char code[] = "void f(unsigned char x) { int arr[10]; arr[4] = 43; return arr[x] == 12; }";
        ASSERT_EQUALS("?,43", getRange(code, "arr[x]"));
        ASSERT_EQUALS("(declare-fun v0 () Int)\n"
                      "(assert (<= v0 255))\n"
                      "(assert (>= v0 0))\n"
                      "(assert (= (ite (= v0 4) 43 0) 12))\n",
                      expr(code, "=="));
    }

    void array4() {
        const char code[] = "int buf[10];\n"
                            "void f() { int x = buf[0]; }";
        ASSERT_EQUALS("2:16: $2:0=-2147483648:2147483647\n"
                      "2:20: $2=-2147483648:2147483647\n"
                      "2:26: { buf=($1,size=10,[:]=$2) x=$2:0}\n",
                      trackExecution(code));
    }

    void arrayInit1() {
        ASSERT_EQUALS("0", getRange("inf f() { char arr[10] = \"\"; return arr[4]; }", "arr[4]"));
    }

    void arrayInit2() {
        ASSERT_EQUALS("66", getRange("void f() { char str[] = \"hello\"; str[0] = \'B\'; }", "str[0]=\'B\'"));
    }

    void arrayUninit() {
        ASSERT_EQUALS("?", getRange("int f() { int arr[10]; return arr[4]; }", "arr[4]"));
    }


    void floatValue1() {
        ASSERT_EQUALS(std::to_string(std::numeric_limits<float>::min()) + ":" + std::to_string(std::numeric_limits<float>::max()), getRange("float f; void func() { f=f; }", "f=f"));
    }

    void floatValue2() {
        ASSERT_EQUALS("(29.0)/(2.0)", getRange("void func() { float f = 29.0; f = f / 2.0; }", "f/2.0"));
    }


    void functionCall1() {
        ASSERT_EQUALS("-2147483648:2147483647", getRange("int atoi(const char *p); void f() { int x = atoi(a); x = x; }", "x=x"));
    }

    void functionCall2() {
        const char code[] = "namespace NS {\n"
                            "    short getValue();\n"
                            "}"
                            "void f() {\n"
                            "    short value = NS::getValue();\n"
                            "    value = value;\n"
                            "}";
        ASSERT_EQUALS("-32768:32767", getRange(code, "value=value"));
    }

    void functionCall3() {
        ASSERT_EQUALS("-2147483648:2147483647", getRange("int fgets(int, const char *, void *); void f() { int x = -1; fgets(stdin, \"%d\", &x); x=x; }", "x=x"));
    }


    void int1() {
        ASSERT_EQUALS("(declare-fun v0 () Int)\n"
                      "(assert (<= v0 2147483647))\n"
                      "(assert (>= v0 (- 2147483648)))\n"
                      "(assert (= (+ 2 v0) 3))\n",
                      expr("void f(int x) { return 2+x==3; }", "=="));
    }


    void pointer1() {
        const char code[] = "void f(unsigned char *p) { a = *p; }";
        ASSERT_EQUALS("->$1,null,->?", getRange(code, "p"));
        // TODO expr
    }

    void pointerAlias1() {
        ASSERT_EQUALS("3", getRange("int f() { int x; int *p = &x; x = 3; return *p; }", "return*p"));
    }

    void pointerAlias2() {
        ASSERT_EQUALS("1", getRange("int f() { int x; int *p = &x; *p = 1; return *p; }", "return*p"));
    }

    void pointerAlias3() {
        ASSERT_EQUALS("7", getRange("int f() {\n"
                                    "  int x = 18;\n"
                                    "  int *p = &x;\n"
                                    "  *p = 7;\n"
                                    "  return x;\n"
                                    "}", "x", 5));
    }

    void pointerAlias4() {
        ASSERT_EQUALS("71", getRange("int f() { int x[10]; int *p = x+3; *p = 71; return x[3]; }", "x[3]"));
    }

    void pointerNull1() {
        ASSERT_EQUALS("1", getRange("void f(void *p) { p = NULL; p += 1; }", "p+=1"));
    }


    void structMember() {
        ASSERT_EQUALS("(declare-fun v0 () Int)\n"
                      "(declare-fun v1 () Int)\n"
                      "(assert (<= v0 255))\n"
                      "(assert (>= v0 0))\n"
                      "(assert (<= v1 255))\n"
                      "(assert (>= v1 0))\n"
                      "(assert (= (+ v0 v1) 0))\n",
                      expr("struct S {\n"
                           "    unsigned char a;\n"
                           "    unsigned char b;\n"
                           "};\n"
                           "void f(struct S s) { return s.a + s.b == 0; }", "=="));
    }

};

REGISTER_TEST(TestExprEngine)
