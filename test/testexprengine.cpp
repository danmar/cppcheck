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
        TEST_CASE(argPointer);
        TEST_CASE(argSmartPointer);
        TEST_CASE(argStruct);

        TEST_CASE(expr1);
        TEST_CASE(expr2);
        TEST_CASE(expr3);
        TEST_CASE(expr4);
        TEST_CASE(expr5);
        TEST_CASE(exprAssign1);
        TEST_CASE(exprAssign2); // Truncation

        TEST_CASE(if1);
        TEST_CASE(if2);
        TEST_CASE(if3);
        TEST_CASE(if4);
        TEST_CASE(if5);
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

        TEST_CASE(pointer1);
        TEST_CASE(pointer2);

        TEST_CASE(pointerAlias1);
        TEST_CASE(pointerAlias2);
        TEST_CASE(pointerAlias3);
        TEST_CASE(pointerAlias4);
        TEST_CASE(pointerNull1);
    }

    std::string getRange(const char code[], const std::string &str, int linenr = 0) {
        Settings settings;
        settings.platform(cppcheck::Platform::Unix64);
        settings.library.smartPointers.insert("std::shared_ptr");
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::string ret;
        std::function<void(const Token *, const ExprEngine::Value &)> f = [&](const Token *tok, const ExprEngine::Value &value) {
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

    void argPointer() {
        ASSERT_EQUALS("->$1,null,->?", getRange("void f(unsigned char *p) { a = *p; }", "p"));
    }

    void argSmartPointer() {
        ASSERT_EQUALS("->{x=$2},null", getRange("struct S { int x; }; void f(std::shared_ptr<S> ptr) { x = ptr; }", "ptr"));
    }

    void argStruct() {
        ASSERT_EQUALS("0:510",
                      getRange("struct S {\n"
                               "    unsigned char a;\n"
                               "    unsigned char b;\n"
                               "};\n"
                               "void f(struct S s) { return s.a + s.b; }", "s.a+s.b"));
    }

    void expr1() {
        ASSERT_EQUALS("-32768:32767", getRange("void f(short x) { a = x; }", "x"));
    }

    void expr2() {
        ASSERT_EQUALS("-65536:65534", getRange("void f(short x) { a = x + x; }", "x+x"));
    }

    void expr3() {
        ASSERT_EQUALS("-65536:65534", getRange("int f(short x) { int a = x + x; return a; }", "return a"));
    }

    void expr4() {
        ASSERT_EQUALS("0", getRange("int f(short x) { int a = x - x; return a; }", "return a"));
    }

    void expr5() {
        ASSERT_EQUALS("-65536:65534", getRange("void f(short a, short b, short c, short d) { if (a+b<c+d) {} }", "a+b"));
    }

    void exprAssign1() {
        ASSERT_EQUALS("1:256", getRange("void f(unsigned char a) { a += 1; }", "a+=1"));
    }

    void exprAssign2() {
        ASSERT_EQUALS("2", getRange("void f(unsigned char x) { x = 258; int a = x }", "a=x"));
    }


    void if1() {
        ASSERT_EQUALS("7:32768", getRange("inf f(short x) { if (x > 5) a = x + 1; }", "x+1"));
    }

    void if2() {
        ASSERT_EQUALS("7:32768,-32767:6", getRange("inf f(short x) { if (x > 5) {} a = x + 1; }", "x+1"));
    }

    void if3() {
        ASSERT_EQUALS("1,-2147483648:2147483647,-2147483648:2147483647", getRange("void f() { int x; if (a) { if (b) x=1; } x=x; }", "x=x"));
    }

    void if4() {
        ASSERT_EQUALS("1:2147483647,-2147483648:-1", getRange("int x; void f() { if (x) { a=x; }}", "a=x"));
    }

    void if5() {
        ASSERT_EQUALS("0", getRange("int x; void f() { if (x) {} else { a=x; }}", "a=x"));
    }

    void ifelse1() {
        ASSERT_EQUALS("-32767:6", getRange("inf f(short x) { if (x > 5) ; else a = x + 1; }", "x+1"));
    }


    void array1() {
        ASSERT_EQUALS("5", getRange("inf f() { int arr[10]; arr[4] = 5; return arr[4]; }", "arr[4]"));
    }

    void array2() {
        ASSERT_EQUALS("0:255", getRange("void dostuff(unsigned char *); int f() { unsigned char arr[10] = \"\"; dostuff(arr); return arr[4]; }", "arr[4]"));
    }

    void array3() {
        ASSERT_EQUALS("?,43", getRange("int f(unsigned char x) { int arr[10]; arr[4] = 43; int vx = arr[x]; }", "arr[x]"));
    }

    void array4() {
        const char code[] = "int buf[10];\n"
                            "void f() { int x = buf[0]; }";
        ASSERT_EQUALS("2:20: $2=-2147483648:2147483647\n"
                      "2:26: { buf=($1,size=10,[:]=$2) x=($3,{{(null),$2}})}\n",
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
        ASSERT_EQUALS("14.500000", getRange("void func() { float f = 29.0; f = f / 2.0; }", "f/2.0"));
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

    void pointer1() {
        ASSERT_EQUALS("?", getRange("int f() { int *x; x = x; }", "x=x"));
    }

    void pointer2() {
        ASSERT_EQUALS("?", getRange("int f() { sometype *x; x = x; }", "x=x"));
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
        ASSERT_EQUALS("0", getRange("void f(void *p) { p = NULL; p += 1; }", "p+=1"));
    }
};

REGISTER_TEST(TestExprEngine)
