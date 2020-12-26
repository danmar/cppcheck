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
        TEST_CASE(expr9);
        TEST_CASE(exprAssign1);
        TEST_CASE(exprAssign2); // Truncation
        TEST_CASE(exprNot);

        TEST_CASE(getValueConst1);

        TEST_CASE(inc1);
        TEST_CASE(inc2);

        TEST_CASE(if1);
        TEST_CASE(if2);
        TEST_CASE(if3);
        TEST_CASE(if4);
        TEST_CASE(if5);
        TEST_CASE(ifAlwaysTrue1);
        TEST_CASE(ifAlwaysTrue2);
        TEST_CASE(ifAlwaysTrue3);
        TEST_CASE(ifAlwaysFalse1);
        TEST_CASE(ifAlwaysFalse2);
        TEST_CASE(ifelse1);
        TEST_CASE(ifif);
        TEST_CASE(ifreturn);
        TEST_CASE(ifIntRangeAlwaysFalse);
        TEST_CASE(ifIntRangeAlwaysTrue);

        TEST_CASE(istream);

        TEST_CASE(switch1);
        TEST_CASE(switch2);

        TEST_CASE(for1);
        TEST_CASE(forAlwaysFalse1);

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
        TEST_CASE(array6);
        TEST_CASE(array7);
        TEST_CASE(arrayInit1);
        TEST_CASE(arrayInit2);
        TEST_CASE(arrayInit3);
        TEST_CASE(arrayUninit);
        TEST_CASE(arrayInLoop);

        TEST_CASE(floatValue1);
        TEST_CASE(floatValue2);
        TEST_CASE(floatValue3);
        TEST_CASE(floatValue4);
        TEST_CASE(floatValue5);

        TEST_CASE(functionCall1);
        TEST_CASE(functionCall2);
        TEST_CASE(functionCall3);
        TEST_CASE(functionCall4);
        TEST_CASE(functionCall5);

        TEST_CASE(functionCallContract1);
        TEST_CASE(functionCallContract2);

        TEST_CASE(int1);

        TEST_CASE(pointer1);
        TEST_CASE(pointer2);
        TEST_CASE(pointer3);
        TEST_CASE(pointerAlias1);
        TEST_CASE(pointerAlias2);
        TEST_CASE(pointerAlias3);
        TEST_CASE(pointerAlias4);
        TEST_CASE(pointerNull1);

        TEST_CASE(structMember1);
        TEST_CASE(structMember2);
        TEST_CASE(structMember3);

        TEST_CASE(pointerToStructInLoop);

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
            replace(line, "(_ +zero 11 53)", "0.0");
            replace(line, "(fp #b0 #b10000000010 #x899999999999a)", "12.3");
            replace(line, "(/ 123.0 10.0)", "12.3");
            int par = 0;
            for (char pos : line) {
                if (pos == '(')
                    par++;
                else if (pos == ')')
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
        ExprEngine::Callback f = [&](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
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
        ExprEngine::Callback f = [&](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
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

    std::string trackExecution(const char code[], Settings *settings = nullptr) {
        Settings s;
        if (!settings)
            settings = &s;
        settings->bugHunting = true;
        settings->debugBugHunting = true;
        settings->platform(cppcheck::Platform::Unix64);
        settings->library.smartPointers.insert("std::shared_ptr");
        Tokenizer tokenizer(settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        std::vector<ExprEngine::Callback> callbacks;
        std::ostringstream ret;
        ExprEngine::executeAllFunctions(this, &tokenizer, settings, callbacks, ret);
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

    void expr9() {
        Settings settings;
        LOAD_LIB_2(settings.library, "std.cfg");

        ASSERT_EQUALS("1:26: $4=ArrayValue([$3],[:]=$2)\n"
                      "1:26: $3=IntRange(0:2147483647)\n"
                      "1:26: $2=IntRange(-128:127)\n"
                      "1:27: 0:memory:{s=($4,[$3],[:]=$2)}\n",
                      trackExecution("void foo() { std::string s; }", &settings));


        ASSERT_EQUALS("1:52: $4=ArrayValue([$3],[:]=$2)\n"
                      "1:52: $3=IntRange(0:2147483647)\n"
                      "1:52: $2=IntRange(-128:127)\n"
                      "1:66: 0:memory:{s=($4,[$3],[:]=$2)}\n",
                      trackExecution("std::string getName(int); void foo() { std::string s = getName(1); }", &settings));
    }

    void exprAssign1() {
        ASSERT_EQUALS("($1)+(1)", getRange("void f(unsigned char a) { a += 1; }", "a+=1"));
    }

    void exprAssign2() {
        ASSERT_EQUALS("2", getRange("void f(unsigned char x) { x = 258; int a = x }", "a=x"));
    }

    void exprNot() {
        ASSERT_EQUALS("($1)==(0)", getRange("void f(unsigned char a) { return !a; }", "!a"));
    }

    void getValueConst1() { // Data::getValue
        ASSERT_EQUALS("512", getRange("const int x=512; void func() { x=x }", "x=x"));
    }


    void inc1() {
        ASSERT_EQUALS("(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                      "(= (+ $1 1) $1)\n"
                      "z3::unsat\n",
                      expr("void f(int x) { int y = x++; x == y; }", "=="));

        ASSERT_EQUALS("(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                      "(= (+ $1 1) (+ $1 1))\n"
                      "z3::sat\n",
                      expr("void f(int x) { int y = ++x; x == y; }", "=="));
    }

    void inc2() {
        ASSERT_EQUALS("(= 2 2)\n"
                      "z3::sat\n",
                      expr("void f() { unsigned char a[2]; a[0] = 1; a[0]++; a[0] == a[0]; }", "=="));
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

    void ifAlwaysTrue1() {
        const char code[] = "int foo() {\n"
                            "  int a = 42;\n"
                            "  if (1) {\n"
                            "    a = 0;\n"
                            "  }\n"
                            "  return a == 0;\n"
                            "}";
        const char expected[] = "(= 0 0)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void ifAlwaysTrue2() {
        const char code[] = "int foo() {\n"
                            "  int a = 42;\n"
                            "  if (12.3) {\n"
                            "    a = 0;\n"
                            "  }\n"
                            "  return a == 0;\n"
                            "}";
        const char expected[] = "(= 0 0)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void ifAlwaysTrue3() {
        const char code[] = "int foo() {\n"
                            "  int a = 42;\n"
                            "  if (\"test\") {\n"
                            "    a = 0;\n"
                            "  }\n"
                            "  return a == 0;\n"
                            "}";
        // String literals are always true. z3 will not be involved.
        ASSERT_EQUALS("(= 0 0)\n"
                      "z3::sat\n",
                      expr(code, "=="));
    }

    void ifAlwaysFalse1() {
        const char code[] = "int foo() {\n"
                            "  int a = 42;\n"
                            "  if (0) {\n"
                            "    a = 0;\n"
                            "  }\n"
                            "  return a == 0;\n"
                            "}";
        const char expected[] = "(= 42 0)\n"
                                "z3::unsat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void ifAlwaysFalse2() {
        const char code[] = "int foo() {\n"
                            "  int a = 42;\n"
                            "  if (0.0) {\n"
                            "    a = 0;\n"
                            "  }\n"
                            "  return a == 0;\n"
                            "}";
        const char expected[] = "(= 42 0)\n"
                                "z3::unsat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void ifelse1() {
        ASSERT_EQUALS("(<= $1 5)\n"
                      "(and (>= $1 (- 32768)) (<= $1 32767))\n"
                      "(= (+ $1 2) 40)\n"
                      "z3::unsat\n",
                      expr("void f(short x) { if (x > 5) ; else if (x+2==40); }", "=="));
    }


    void ifif() {
        const char code[] = "void foo(unsigned char x) {\n"
                            "    if (x > 5) {}\n"
                            "    if (x > 5) {}\n"
                            "    return x == 13;\n"
                            "}";

        ASSERT_EQUALS("(> $1 5)\n"
                      "(and (>= $1 0) (<= $1 255))\n"
                      "(= $1 13)\n"
                      "z3::sat\n"
                      "(<= $1 5)\n"
                      "(and (>= $1 0) (<= $1 255))\n"
                      "(= $1 13)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }


    void ifreturn() { // Early return
        const char code[] = "void foo(unsigned char x) {\n"
                            "    if (x > 5) { return; }\n"
                            "    return x == 13;\n"
                            "}";

        ASSERT_EQUALS("(<= $1 5)\n"
                      "(and (>= $1 0) (<= $1 255))\n"
                      "(= $1 13)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }

    void ifIntRangeAlwaysFalse() {
        const char code[] = "void foo(unsigned char x) {\n"
                            "  if (x > 0)\n"
                            "      return;\n"
                            "  if (x) {\n"  // <-- condition should be "always false".
                            "      x++;\n"
                            "  }\n"
                            "  return x == 0;\n" // <- sat
                            "}";
        ASSERT_EQUALS("(<= $1 0)\n"
                      "(and (>= $1 0) (<= $1 255))\n"
                      "(= $1 0)\n"
                      "z3::sat\n",
                      expr(code, "=="));
    }

    void ifIntRangeAlwaysTrue() {
        const char code[] = "void foo(unsigned char x) {\n"
                            "  if (x < 1)\n"
                            "      return;\n"
                            "  if (x) {\n"  // <-- condition should be "always true".
                            "      x++;\n"
                            "  }\n"
                            "  return x == 0;\n" // <- unsat
                            "}";
        ASSERT_EQUALS("(>= $1 1)\n"
                      "(and (>= $1 0) (<= $1 255))\n"
                      "(= (+ $1 1) 0)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }

    void istream() {
        const char code[] = "void foo(const std::string& in) {\n"
                            "    std::istringstream istr(in);\n"
                            "    unsigned short x=5;\n"
                            "    istr >> x;\n"
                            "    x==3;\n"
                            "}";

        ASSERT_EQUALS("(and (>= $1 0) (<= $1 65535))\n"
                      "(= $1 3)\n"
                      "z3::sat\n",
                      expr(code, "=="));
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


    void for1() {
        const char code[] = "void f() {\n"
                            "  int x[10];\n"
                            "  for (int i = 0; i < 10; i++) x[i] = 0;\n"
                            "  x[4] == 67;\n"
                            "}";
        ASSERT_EQUALS("(= 0 67)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }

    void forAlwaysFalse1() {
        const char code[] = "int f() {\n"
                            "  int a = 19;\n"
                            "  for (int i = 0; i < 0; i++)\n"
                            "    a += 8;\n"
                            "  for (int i = 0; i < 1; i++)\n"
                            "    a += 23;\n"
                            "  for (int i = 100; i >= 1; i--)\n"
                            "    a += 23;\n"
                            "  return a == 42;\n"
                            "}";
        const char expected[] = "(and (>= $4 (- 2147483648)) (<= $4 2147483647))\n"
                                "(= (+ $4 23) 42)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void while1() {
        const char code[] = "void f(int y) {\n"
                            "  int x = 0;\n"
                            "  while (x < y)\n"
                            "    x = x + 34;\n"
                            "  x == 340;\n"
                            "}";
        const char expected[] = "(< 0 $1)\n"
                                "(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                                "(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                                "(= (+ $2 34) 340)\n"
                                "z3::sat\n"
                                "(= 0 340)\n"
                                "z3::unsat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
    }

    void while2() {
        const char code[] = "void f(int y) {\n"
                            "  int x = 0;\n"
                            "  while (x < y)\n"
                            "    x++;\n"
                            "  x == 1;\n"
                            "}";
        const char expected[] = "(< 0 $1)\n"
                                "(and (>= $2 (- 2147483648)) (<= $2 2147483647))\n"
                                "(and (>= $1 (- 2147483648)) (<= $1 2147483647))\n"
                                "(= (+ $2 1) 1)\n"
                                "z3::sat\n"
                                "(= 0 1)\n"
                                "z3::unsat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
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
        const char expected[] = "(distinct |$2:0| 0)\n"
                                "(and (>= |$2:0| (- 128)) (<= |$2:0| 127))\n"
                                "(= 0 0)\n"
                                "z3::sat\n"
                                "(and (>= $8 (- 2147483648)) (<= $8 2147483647))\n"
                                "(= $8 0)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, "=="));
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
        ASSERT_EQUALS("(and (>= |$4:4| 0) (<= |$4:4| 255))\n"
                      "(= |$4:4| 365)\n"
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
        ASSERT_EQUALS("2:16: $2:0=IntRange(-2147483648:2147483647)\n"
                      "2:20: $1=ArrayValue([10],[:]=$2)\n"
                      "2:20: $2=IntRange(-2147483648:2147483647)\n"
                      "2:26: 0:memory:{buf=($1,[10],[:]=$2) x=$2:0}\n",
                      trackExecution(code));
    }

    void array5() {
        const char code[] = "int f(int x) {\n"
                            "  int buf[3][4][5];\n"
                            "  buf[x][1][2] = 10;\n"
                            "  return buf[0][1][2];\n"
                            "}";
        ASSERT_EQUALS("1:14: $1=IntRange(-2147483648:2147483647)\n"
                      "1:14: 0:memory:{x=$1}\n"
                      "2:7: $2=ArrayValue([3][4][5],[:]=?)\n"
                      "2:19: 0:memory:{x=$1 buf=($2,[3][4][5],[:]=?)}\n"
                      "3:20: 0:memory:{x=$1 buf=($2,[3][4][5],[:]=?,[((20)*($1))+(7)]=10)}\n",
                      trackExecution(code));
    }

    void array6() {
        const char code[] = "void foo(int *x) {\n"
                            "  *x = 2;\n"
                            "  if (*x == 21) {}"
                            "}";
        ASSERT_EQUALS("(= 2 21)\n"
                      "z3::unsat\n",
                      expr(code, "=="));
    }

    void array7() {
        const char code[] = "void foo(unsigned char *x) {\n"
                            "  *x = 2;\n"
                            "  *x = 1;\n"
                            "}";
        ASSERT_EQUALS("1:28: $2=ArrayValue([$1],[:]=?,null)\n"
                      "1:28: $1=IntRange(1:ffffffffffffffff)\n"
                      "1:28: 0:memory:{x=($2,[$1],[:]=?)}\n"
                      "2:9: 0:memory:{x=($2,[$1],[:]=?,[0]=2)}\n"
                      "3:9: 0:memory:{x=($2,[$1],[:]=?,[0]=1)}\n",
                      trackExecution(code));
    }

    void arrayInit1() {
        ASSERT_EQUALS("0", getRange("inf f() { char arr[10] = \"\"; return arr[4]; }", "arr[4]"));
    }

    void arrayInit2() {
        ASSERT_EQUALS("66", getRange("void f() { char str[] = \"hello\"; str[0] = \'B\'; }", "str[0]=\'B\'"));
    }

    void arrayInit3() {
        ASSERT_EQUALS("-32768:32767", getRange("void f() { short buf[5] = {2, 1, 0, 3, 4}; ret = buf[2]; }", "buf[2]"));
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
        const char code[] = "void foo(float f) { return f > 12.3; }";
        const char expected[] = "(> $1 12.3)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, ">"));
    }

    void floatValue4() {
        const char code[] = "void foo(float f) { return f > 12.3f; }";
        const char expected[] = "(> $1 12.3)\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, ">"));
    }

    void floatValue5() { // float < int
        const char code[] = "void foo(float f) { if (f < 1){} }";
        const char expected[] = "(< $1 (to_real 1))\n"
                                "z3::sat\n";
        ASSERT_EQUALS(expected, expr(code, "<"));
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

    void functionCall5() { // unknown result from function, pointer type..
        ASSERT_EQUALS("1:36: $3=ArrayValue([$2],[:]=bailout,null)\n"
                      "1:36: $2=IntRange(1:2147483647)\n"
                      "1:36: bailout=BailoutValue(bailout)\n"
                      "1:46: 0:memory:{p=($3,[$2],[:]=bailout)}\n",
                      trackExecution("char *foo(int); void bar() { char *p = foo(1); }"));
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

    void functionCallContract2() {
        const char code[] = "void foo(float x);\n"
                            "void bar(float x) { foo(x); }";

        Settings s;
        s.functionContracts["foo(x)"] = "x < 12.3";

        ASSERT_EQUALS("checkContract:{\n"
                      "(ite (< $2 12.3) false true)\n"
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

    void pointer3() {
        const char code[] = "void f(void *p) {\n"
                            "    double *data = (double *)p;\n"
                            "    return *data;"
                            "}";
        ASSERT_EQUALS("[$1],[:]=?,null", getRange(code, "p"));
        ASSERT_EQUALS("[$4],[:]=?,null", getRange(code, "data"));
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

    void pointerToStructInLoop() {
        const char code[] = "struct S { int x; };\n"
                            "void foo(struct S *s) {\n"
                            "  while (1)\n"
                            "    s->x = 42; \n"
                            "}";

        const char expected[] = "(and (>= $3 (- 2147483648)) (<= $3 2147483647))\n"
                                "(= $3 42)\n"
                                "z3::sat\n";

        TODO_ASSERT_EQUALS(expected, "", expr(code, "=="));
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
