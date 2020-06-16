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

#include "config.h"
#include "exprengine.h"
#include "library.h"
#include "platform.h"
#include "settings.h"
#include "token.h"
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
        TEST_CASE(annotation1);
        TEST_CASE(annotation2);

        TEST_CASE(expr1);
        TEST_CASE(expr2);
        TEST_CASE(expr3);
        TEST_CASE(expr4);
        TEST_CASE(expr5);
        TEST_CASE(expr6);
        TEST_CASE(expr7);
        TEST_CASE(expr8);
        TEST_CASE(exprAssign1);
        TEST_CASE(exprAssign2); // Truncation

        TEST_CASE(if1);
        TEST_CASE(if2);
        TEST_CASE(if3);
        TEST_CASE(if4);
        TEST_CASE(if5);
        TEST_CASE(ifelse1);

        TEST_CASE(switch1);
        TEST_CASE(switch2);

        TEST_CASE(while1);
        TEST_CASE(while2);
        TEST_CASE(while3);
        TEST_CASE(while4);
        TEST_CASE(while5);

        TEST_CASE(array1);
        TEST_CASE(array2);
        TEST_CASE(array3);
        TEST_CASE(array4);
        TEST_CASE(array5);
        TEST_CASE(arrayInit1);
        TEST_CASE(arrayInit2);
        TEST_CASE(arrayUninit);
        TEST_CASE(arrayInLoop);

        TEST_CASE(floatValue1);
        TEST_CASE(floatValue2);
        TEST_CASE(floatValue3);

        TEST_CASE(functionCall1);
        TEST_CASE(functionCall2);
        TEST_CASE(functionCall3);
        TEST_CASE(functionCall4);

        TEST_CASE(functionCallContract1);

        TEST_CASE(int1);

        TEST_CASE(pointer1);
        TEST_CASE(pointer2);
        TEST_CASE(pointerAlias1);
        TEST_CASE(pointerAlias2);
        TEST_CASE(pointerAlias3);
        TEST_CASE(pointerAlias4);
        TEST_CASE(pointerNull1);

        TEST_CASE(structMember1);
        TEST_CASE(structMember2);
        TEST_CASE(structMember3);

        TEST_CASE(ternaryOperator1);
#endif
    }

    static void replace(std::string& str, const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::string::npos)
            str.replace(pos, from.length(), to);
    }

    static std::string cleanupExpr(std::string rawexpr) {
        std::string ret;
        std::istringstream istr(rawexpr);
        std::string line;
        while (std::getline(istr, line)) {
            if (line.empty())
                continue;
            line = line.substr(line.find_first_not_of(" "));
            if (line.compare(0,13,"(declare-fun ") == 0)
                continue;
            if (line == "(solver")
                continue;
            if (line.compare(0,9,"(assert (") == 0) {
                line.erase(0,8);
                line.erase(line.size()-1);
            }
            replace(line, "(fp.gt ", "(> ");
            replace(line, "(fp.lt ", "(< ");
            int par = 0;
            for (int pos = 0; pos < line.size(); ++pos) {
                if (line[pos] == '(')
                    par++;
                else if (line[pos] == ')')
                    --par;
            }
            if (par < 0)
                line.erase(line.size() - 1);
            ret += line + "\n";
        }
        return ret;
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
            const auto *b = dynamic_cast<const ExprEngine::BinOpResult *>(&value);
            if (!b)
                return;
            ret += TestExprEngine::cleanupExpr(b->getExpr(dataBase));
        };
        std::vector<ExprEngine::Callback> callbacks;
        callbacks.push_back(f);
        std::ostringstream trace;
        ExprEngine::executeAllFunctions(this, &tokenizer, &settings, callbacks, trace);
        return ret;
    }

    std::string functionCallContractExpr(const char code[], const Settings &s) {
        Settings settings;
        settings.bugHunting = true;
        settings.debugBugHunting = true;
        settings.functionContracts = s.functionContracts;
        settings.platform(cppcheck::Platform::Unix64);
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::vector<ExprEngine::Callback> callbacks;
        std::ostringstream trace;
        ExprEngine::executeAllFunctions(this, &tokenizer, &settings, callbacks, trace);
        std::string ret = trace.str();
        std::string::size_type pos1 = ret.find("checkContract:{");
        std::string::size_type pos2 = ret.find("}", pos1);
        if (pos2 == std::string::npos)
            return "Error:" + ret;
        return TestExprEngine::cleanupExpr(ret.substr(pos1, pos2+1-pos1));
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
        std::ostringstream trace;
        ExprEngine::executeAllFunctions(this, &tokenizer, &settings, callbacks, trace);
        return ret;
    }

    std::string trackExecution(const char code[]) {
        Settings settings;
        settings.bugHunting = true;
        settings.debugBugHunting = true;
        settings.platform(cppcheck::Platform::Unix64);
        settings.library.smartPointers.insert("std::shared_ptr");
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::vector<ExprEngine::Callback> callbacks;
        std::ostringstream ret;
        ExprEngine::executeAllFunctions(this, &tokenizer, &settings, callbacks, ret);
        return ret.str();
    }

    void annotation1() {
        const char code[] = "void f(__cppcheck_low__(100) short x) {\n"
                            "    return x < 10;\n"
                            "}";

        const char expected[] = "(>= $1 100)\n" // <- annotation
                                "(and (>= $1 (- 32768)) (<= $1 32767))\n"
                                "(< $1 10)\n"
                                "z3::unsat\n";

        ASSERT_EQUALS(expected, expr(code, "<"));
    }

    void annotation2() {
        const char code[] = "__cppcheck_low__(100) short x;\n"
                            " void f() {\n"
                            "    return x < 10;\n"
                            "}";

        const char expected[] = "(>= $1 100)\n" // <- annotation
                                "(and (>= $1 (- 32768)) (<= $1 32767))\n"
                                "(< $1 10)\n"
                                "z3::unsat\n";

        ASSERT_EQUALS(expected, expr(code, "<"));
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

    void expr6() {
        const char code[] = "void f(unsigned char x) {\n"
                            "    unsigned char result = 8 - x;\n"
                            "    result > 1000;"
                            "}";

        ASSERT_EQUALS("(8)-($1)", getRange(code, "8-x"));

        ASSERT_EQUALS("(and (>= $1 0) (<= $1 255))\n"
                      "(> (- 8 $1) 1000)\n"
                      "z3::unsat\n",
                      expr(code, ">"));
    }

    void expr7() {
        const char code[] = "void f(bool a, bool b, int c) {\n"
                            "    if (a||b) {}\n"
                            "    c > 1000;"
                            "}";

        ASSERT_EQUALS("(or (distinct $1 0) (distinct $2 0))\n"
                      "(and (>= $3 (- 2147483648)) (<= $3 2147483647))\n"
                      "(and (>= $1 0) (<= $1 1))\n"
                      "(and (>= $2 0) (<= $2 1))\n"
                      "(> $3 1000)\n"
                      "z3::sat\n"
                      "(= (ite (or (distinct $1 0) (distinct $2 0)) 1 0) 0)\n"
                      "(and (>= $3 (- 2147483648)) (<= $3 2147483647))\n"
                      "(and (>= $1 0) (<= $1 1))\n"
                      "(and (>= $2 0) (<= $2 1))\n"
                      "(> $3 1000)\n"
                      "z3::sat\n",
                      expr(code, ">"));
    }

    void expr8() {
        const char code[] = "void foo(int x, int y) {\n"
                            "    if (x % 32) {}\n"
                            "    y==3;\n"
                            "}";
        // Do not crash
        expr(code, "==");
    }

    void exprAssign1() {
        ASSERT_EQUALS("($1)+(1)", getRange("void f(unsigned char a) { a += 1; }", "a+=1"));
    }

    void exprAssign2() {
        ASSERT_EQUALS("2", getRange("void f(unsigned char x) { x = 258; int a = x }", "a=x"));
    }

    void if1() {
        ASSERT_EQUALS("(< $1 $2)\n"
                      "(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                      "(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                      "(= $1 $2)\n"
                      "z3::unsat\n",
                      expr("void f(int x, int y) { if (x < y) return x == y; }", "=="));
    }

    void if2() {
        const char code[] = "void foo(int x) {\n"
                            "  if (x > 0 && x == 20) {}\n"
                            "}";
        // In expression "x + x < 20", "x" is greater than 0
        const char expected[] = "(> $1 0)\n"
                                "(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                                "(= $1 20)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void if3() {
        const char code[] = "void foo(int x) {\n"
                            "  if (x > 0 || x == 20) {}\n"
                            "}";
        // In expression "x + x < 20", "x" is greater than 0
        const char expected[] = "(<= $1 0)\n"
                                "(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                                "(= $1 20)\n"
                                "z3::unsat\n"; // "x == 20" is unsat
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void if4() {
        const char code[] = "void foo(unsigned int x, unsigned int y) {\n"
                            "    unsigned int z = y;"
                            "    if (x < z) { return z == 0; }\n"
                            "}";
        const char expected[] = "(< $1 $2)\n"
                                "(>= $2 0)\n"
                                "(>= $1 0)\n"
                                "(= $2 0)\n"
                                "z3::unsat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void if5() {
        ASSERT_EQUALS("(> |$2:0| 12)\n"
                      "(and (>= |$2:0| (- 2147483648)) (<= |$2:0| 2147483647))\n"
                      "(= |$2:0| 5)\n"
                      "z3::unsat\n",
                      expr("void foo(const int *x) { if (f1() && *x > 12) dostuff(*x == 5); }", "=="));
    }


    void ifelse1() {
        ASSERT_EQUALS("(<= $1 5)\n"
                      "(and (>= $1 (- 32768)) (<= $1 32767))\n"
                      "(= (+ $1 2) 40)\n"
                      "z3::unsat\n",
                      expr("void f(short x) { if (x > 5) ; else if (x+2==40); }", "=="));
    }


    void switch1() {
        const char code[] = "void f(int x) {\n"
                            "    switch (x) {\n"
                            "    case 1: x==3; break;\n"
                            "    case 2: x>0; break;\n"
                            "    };\n"
                            "    x<=4;\n"
                            "}";
        ASSERT_EQUALS("(= $1 1)\n"
                      "(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                      "(= $1 3)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }

    void switch2() {
        const char code[] = "void foo(char type, int mcc) {\n"
                            "    switch (type) {\n"
                            "        case '1':\n"
                            "        case '3':\n"
                            "            break;\n"
                            "        default:\n"
                            "            return false;\n"
                            "    }\n"
                            "    p[0] = mcc == 0;\n"
                            "}";
        ASSERT_EQUALS("(= $1 49)\n"
                      "(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                      "(and (>= $1 (- 128)) (<= $1 127))\n"
                      "(= $2 0)\n"
                      "z3::sat\n"
                      "(= $1 51)\n"
                      "(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                      "(and (>= $1 (- 128)) (<= $1 127))\n"
                      "(= $2 0)\n"
                      "z3::sat\n",
                      expr(code, "=="));
    }

    void while1() {
        const char code[] = "void f(int y) {\n"
                            "  int x = 0;\n"
                            "  while (x < y)\n"
                            "    x = x + 34;\n"
                            "  x == 340;\n"
                            "}";
        ASSERT_EQUALS("(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                      "(= (+ $2 34) 340)\n"
                      "z3::sat\n",
                      expr(code, "=="));
    }

    void while2() {
        const char code[] = "void f(int y) {\n"
                            "  int x = 0;\n"
                            "  while (x < y)\n"
                            "    x++;\n"
                            "  x == 1;\n"
                            "}";
        ASSERT_EQUALS("(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                      "(= $2 1)\n"
                      "z3::sat\n",
                      expr(code, "=="));
    }

    void while3() {
        const char code[] = "struct AB {int a; int b;};\n"
                            "void f() {\n"
                            "  struct AB ab;\n"
                            "  while (1)\n"
                            "    ab.a = 3;\n"
                            "  ab.a == 0;\n"
                            "}";
        ASSERT_EQUALS("(= 3 0)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }

    void while4() {
        const char code[] = "void f(const char *host, int *len) {\n"
                            "  while (*host)\n"
                            "    *len = 0;\n"
                            "  *len == 0;\n"
                            "}";
        // Currently the *len gets a BailoutValue in the loop
        ASSERT_EQUALS("", expr(code, "=="));
    }

    void while5() {
        const char code[] = "void f() {\n"
                            "  int x;\n"
                            "  while (cond)\n"
                            "    x += 4;\n"
                            "}";
        ASSERT(getRange(code, "x", 4).find("?") != std::string::npos);
    }


    void array1() {
        ASSERT_EQUALS("(= 5 0)\nz3::unsat\n",
                      expr("int f() { int arr[10]; arr[4] = 5; return arr[4]==0; }", "=="));
    }

    void array2() {
        ASSERT_EQUALS("(and (>= |$3:4| 0) (<= |$3:4| 255))\n"
                      "(= |$3:4| 365)\n"
                      "z3::unsat\n",
                      expr("void dostuff(unsigned char *); int f() { unsigned char arr[10] = \"\"; dostuff(arr); return arr[4] == 365; }", "=="));
    }

    void array3() {
        const char code[] = "void f(unsigned char x) { int arr[10]; arr[4] = 43; return arr[x] == 12; }";
        ASSERT_EQUALS("?,43", getRange(code, "arr[x]"));
        ASSERT_EQUALS("(and (>= $1 0) (<= $1 255))\n"
                      "(= (ite (= $1 4) 43 0) 12)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }

    void array4() {
        const char code[] = "int buf[10];\n"
                            "void f() { int x = buf[0]; }";
        ASSERT_EQUALS("2:16: $2:0=-2147483648:2147483647\n"
                      "2:20: $2=-2147483648:2147483647\n"
                      "2:26: { buf=($1,[10],[:]=$2) x=$2:0}\n",
                      trackExecution(code));
    }

    void array5() {
        const char code[] = "int f(int x) {\n"
                            "  int buf[3][4][5];\n"
                            "  buf[x][1][2] = 10;\n"
                            "  return buf[0][1][2];\n"
                            "}";
        ASSERT_EQUALS("1:14: $1=-2147483648:2147483647\n"
                      "1:14: { x=$1}\n"
                      "2:19: { x=$1 buf=($2,[3][4][5],[:]=?)}\n"
                      "3:20: { x=$1 buf=($2,[3][4][5],[:]=?,[((20)*($1))+(7)]=10)}\n",
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

    void arrayInLoop() {
        const char code[] = "void f() {\n"
                            "  int arr[3][3];\n"
                            "  for (int i = 0; i < 3; i++) arr[i][0] = arr[1][2];\n"
                            "  return arr[0][0];"
                            "}";
        ASSERT_EQUALS("?", getRange(code, "arr[1][2]"));
    }


    void floatValue1() {
        ASSERT_EQUALS("-inf:inf", getRange("float f; void func() { f=f; }", "f=f"));
    }

    void floatValue2() {
        ASSERT_EQUALS("(29.0)/(2.0)", getRange("void func() { float f = 29.0; f = f / 2.0; }", "f/2.0"));
    }

    void floatValue3() {
        const char code[] = "void foo(float f) { return f > 12.0; }";
        const char expected[] = "(> $1 |12.0|)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, ">"));
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

    void functionCall4() {
        ASSERT_EQUALS("1:2147483647", getRange("void f() { sizeof(data); }", "sizeof(data)"));
    }

    void functionCallContract1() {
        const char code[] = "void foo(int x);\n"
                            "void bar(unsigned short x) { foo(x); }";

        Settings s;
        s.functionContracts["foo(x)"] = "x < 1000";

        ASSERT_EQUALS("checkContract:{\n"
                      "(ite (< $2 1000) false true)\n"
                      "(= $2 $1)\n"
                      "(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                      "(and (>= $1 0) (<= $1 65535))\n"
                      "}\n",
                      functionCallContractExpr(code, s));
    }


    void int1() {
        ASSERT_EQUALS("(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                      "(= (+ 2 $1) 3)\n"
                      "z3::sat\n",
                      expr("void f(int x) { return 2+x==3; }", "=="));
    }


    void pointer1() {
        const char code[] = "void f(unsigned char *p) { return *p == 7; }";
        ASSERT_EQUALS("[$1],[:]=?,null", getRange(code, "p"));
        ASSERT_EQUALS("(and (>= $3 0) (<= $3 255))\n"
                      "(= $3 7)\n"
                      "z3::sat\n",
                      expr(code, "=="));
    }

    void pointer2() {
        const char code[] = "void f(unsigned char *p) { return p[2] == 7; }";
        ASSERT_EQUALS("(and (>= $3 0) (<= $3 255))\n"
                      "(= $3 7)\n"
                      "z3::sat\n",
                      expr(code, "=="));
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


    void structMember1() {
        ASSERT_EQUALS("(and (>= $2 0) (<= $2 255))\n"
                      "(and (>= $3 0) (<= $3 255))\n"
                      "(= (+ $2 $3) 0)\n"
                      "z3::sat\n",
                      expr("struct S {\n"
                           "    unsigned char a;\n"
                           "    unsigned char b;\n"
                           "};\n"
                           "void f(struct S s) { return s.a + s.b == 0; }", "=="));
    }

    void structMember2() {
        const char code[] = "struct S { int x; };\n"
                            "void foo(struct S *s) { return s->x == 123; }";

        const char expected[] = "(and (>= $3 (- 2147483648)) (<= $3 2147483647))\n"
                                "(= $3 123)\n"
                                "z3::sat\n";

        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void structMember3() {
        const char code[] = "struct S { int x; };\n"
                            "void foo(struct S *s) {\n"
                            "  s->x = iter->second.data;\n" // assign some unknown value
                            "  return s->x == 1;\n"
                            "}";

        const char expected[] = "(and (>= $3 (- 2147483648)) (<= $3 2147483647))\n"
                                "(= $3 1)\n"
                                "z3::sat\n";

        ASSERT_EQUALS(expected, expr(code, "=="));
    }


    void ternaryOperator1() {
        const char code[] = "void foo(signed char x) {\n"
                            "  x = (x > 0) ? (0==x) : 0;\n"
                            "}";

        const char expected[] = "(> $1 0)\n"
                                "(and (>= $1 (- 128)) (<= $1 127))\n"
                                "(= 0 $1)\n"
                                "z3::unsat\n";

        ASSERT_EQUALS(expected, expr(code, "=="));
    }
};

REGISTER_TEST(TestExprEngine)
