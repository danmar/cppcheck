/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "testutils.h"
#include "valueflow.h"
#include "tokenize.h"
#include "token.h"

#include <vector>
#include <string>
#include <cmath>

class TestValueFlow : public TestFixture {
public:
    TestValueFlow() : TestFixture("TestValueFlow") {
    }

private:
    Settings settings;

    void run() {
        // strcpy cfg
        const char cfg[] = "<?xml version=\"1.0\"?>\n"
                           "<def>\n"
                           "  <function name=\"strcpy\"> <arg nr=\"1\"><not-null/></arg> </function>\n"
                           "</def>";
        settings.library.loadxmldata(cfg, sizeof(cfg));

        TEST_CASE(valueFlowNumber);
        TEST_CASE(valueFlowString);
        TEST_CASE(valueFlowPointerAlias);
        TEST_CASE(valueFlowArrayElement);
        TEST_CASE(valueFlowMove);

        TEST_CASE(valueFlowBitAnd);

        TEST_CASE(valueFlowCalculations);

        TEST_CASE(valueFlowBeforeCondition);
        TEST_CASE(valueFlowBeforeConditionAndAndOrOrGuard);
        TEST_CASE(valueFlowBeforeConditionAssignIncDec);
        TEST_CASE(valueFlowBeforeConditionFunctionCall);
        TEST_CASE(valueFlowBeforeConditionGlobalVariables);
        TEST_CASE(valueFlowBeforeConditionGoto);
        TEST_CASE(valueFlowBeforeConditionIfElse);
        TEST_CASE(valueFlowBeforeConditionLoop);
        TEST_CASE(valueFlowBeforeConditionMacro);
        TEST_CASE(valueFlowBeforeConditionSizeof);
        TEST_CASE(valueFlowBeforeConditionSwitch);
        TEST_CASE(valueFlowBeforeConditionTernaryOp);

        TEST_CASE(valueFlowAfterAssign);

        TEST_CASE(valueFlowAfterCondition);

        TEST_CASE(valueFlowSwitchVariable);

        TEST_CASE(valueFlowForLoop);
        TEST_CASE(valueFlowSubFunction);
        TEST_CASE(valueFlowFunctionReturn);

        TEST_CASE(valueFlowFunctionDefaultParameter);

        TEST_CASE(knownValue);

        TEST_CASE(valueFlowSizeofForwardDeclaredEnum);

        TEST_CASE(valueFlowGlobalVar);
    }

    bool testValueOfX(const char code[], unsigned int linenr, int value) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                std::list<ValueFlow::Value>::const_iterator it;
                for (it = tok->values.begin(); it != tok->values.end(); ++it) {
                    if (it->isIntValue() && it->intvalue == value)
                        return true;
                }
            }
        }

        return false;
    }


    bool testValueOfX(const char code[], unsigned int linenr, const char value[]) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                std::list<ValueFlow::Value>::const_iterator it;
                for (it = tok->values.begin(); it != tok->values.end(); ++it) {
                    if (it->isTokValue() && Token::simpleMatch(it->tokvalue, value))
                        return true;
                }
            }
        }

        return false;
    }

    bool testValueOfX(const char code[], unsigned int linenr, ValueFlow::Value::MoveKind moveKind) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                std::list<ValueFlow::Value>::const_iterator it;
                for (it = tok->values.begin(); it != tok->values.end(); ++it) {
                    if (it->isMovedValue() && it->moveKind == moveKind)
                        return true;
                }
            }
        }

        return false;
    }

    bool testConditionalValueOfX(const char code[], unsigned int linenr, int value) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                std::list<ValueFlow::Value>::const_iterator it;
                for (it = tok->values.begin(); it != tok->values.end(); ++it) {
                    if (it->isIntValue() && it->intvalue == value && it->condition)
                        return true;
                }
            }
        }

        return false;
    }

    void bailout(const char code[]) {
        settings.debugwarnings = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");

        settings.debugwarnings = false;
    }

    std::list<ValueFlow::Value> tokenValues(const char code[], const char tokstr[]) {
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");
        const Token *tok = Token::findmatch(tokenizer.tokens(), tokstr);
        return tok ? tok->values : std::list<ValueFlow::Value>();
    }

    ValueFlow::Value valueOfTok(const char code[], const char tokstr[]) {
        std::list<ValueFlow::Value> values = tokenValues(code, tokstr);
        return values.size() == 1U && !values.front().isTokValue() ? values.front() : ValueFlow::Value();
    }

    void valueFlowNumber() {
        ASSERT_EQUALS(123, valueOfTok("x=123;", "123").intvalue);
        ASSERT(std::fabs(valueOfTok("x=0.5;", "0.5").floatValue - 0.5f) < 0.1f);
        ASSERT_EQUALS(10, valueOfTok("enum {A=10,B=15}; x=A+0;", "+").intvalue);
        ASSERT_EQUALS(0, valueOfTok("x=false;", "false").intvalue);
        ASSERT_EQUALS(1, valueOfTok("x=true;", "true").intvalue);
        ASSERT_EQUALS(0, valueOfTok("x(NULL);", "NULL").intvalue);
        ASSERT_EQUALS((int)('a'), valueOfTok("x='a';", "'a'").intvalue);
        ASSERT_EQUALS((int)('\n'), valueOfTok("x='\\n';", "'\\n'").intvalue);
    }

    void valueFlowString() {
        const char *code;

        // valueFlowAfterAssign
        code  = "const char * f() {\n"
                "    static const char *x;\n"
                "    if (a) x = \"123\";\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "\"123\""));

        // valueFlowSubFunction
        code  = "void dostuff(const char *x) {\n"
                "  f(x);\n"
                "}\n"
                "\n"
                "void test() { dostuff(\"abc\"); }";
        ASSERT_EQUALS(true, testValueOfX(code, 2, "\"abc\""));
    }

    void valueFlowPointerAlias() {
        const char *code;

        code  = "const char * f() {\n"
                "    static const char *x;\n"
                "    static char ret[10];\n"
                "    if (a) x = &ret[0];\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5, "& ret [ 0 ]"));

        // dead pointer
        code  = "void f() {\n"
                "  int *x;\n"
                "  if (cond) { int i; x = &i; }\n"
                "  *x = 0;\n"  // <- x can point at i
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "& i"));

        code  = "void f() {\n"
                "  struct X *x;\n"
                "  x = &x[1];\n"
                "}";
        ASSERT_EQUALS(true, tokenValues(code, "&").empty());
        ASSERT_EQUALS(true, tokenValues(code, "x [").empty());
    }

    void valueFlowArrayElement() {
        const char *code;

        code  = "void f() {\n"
                "    const int x[] = {43,23,12};\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "{ 43 , 23 , 12 }"));

        code  = "void f() {\n"
                "    const char x[] = \"abcd\";\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "\"abcd\""));

        code  = "void f() {\n"
                "    char x[32] = \"abcd\";\n"
                "    return x;\n"
                "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, "\"abcd\""));

        code = "void f() {\n"
               "  int a[10];\n"
               "  int *x = a;\n" // <- a value is a
               "  *x = 0;\n"     // .. => x value is a
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "a"));

        code  = "char f() {\n"
                "    const char *x = \"abcd\";\n"
                "    return x[0];\n"
                "}";
        ASSERT_EQUALS((int)('a'), valueOfTok(code, "[").intvalue);

        code  = "char f() {\n"
                "    const char *x = \"\";\n"
                "    return x[0];\n"
                "}";
        ASSERT_EQUALS(0, valueOfTok(code, "[").intvalue);
    }

    void valueFlowMove() {
        const char *code;

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::forward<X>(x));\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::ForwardedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x).getA());\n"   // Only parts of x might be moved out
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::forward<X>(x).getA());\n" // Only parts of x might be moved out
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::ForwardedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   x.clear();\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, ValueFlow::Value::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   y=x->y;\n"
               "   z=x->z;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, ValueFlow::Value::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    z = g(std::move(x));\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    y = g(std::move(x), \n"
               "          x.size());\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    x = g(std::move(x));\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MovedVariable));

        code = "A f(int i) {\n"
               "    X x;\n"
               "    if (i)"
               "        return g(std::move(x));\n"
               "    return h(std::move(x));\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, ValueFlow::Value::MovedVariable));
    }

    void valueFlowCalculations() {
        const char *code;

        // Different operators
        ASSERT_EQUALS(5, valueOfTok("3 +  (a ? b : 2);", "+").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 -  (a ? b : 2);", "-").intvalue);
        ASSERT_EQUALS(6, valueOfTok("3 *  (a ? b : 2);", "*").intvalue);
        ASSERT_EQUALS(6, valueOfTok("13 / (a ? b : 2);", "/").intvalue);
        ASSERT_EQUALS(1, valueOfTok("13 % (a ? b : 2);", "%").intvalue);
        ASSERT_EQUALS(0, valueOfTok("3 == (a ? b : 2);", "==").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 != (a ? b : 2);", "!=").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 >  (a ? b : 2);", ">").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 >= (a ? b : 2);", ">=").intvalue);
        ASSERT_EQUALS(0, valueOfTok("3 <  (a ? b : 2);", "<").intvalue);
        ASSERT_EQUALS(0, valueOfTok("3 <= (a ? b : 2);", "<=").intvalue);

        ASSERT_EQUALS(1, valueOfTok("(UNKNOWN_TYPE)1;","(").intvalue);
        ASSERT(tokenValues("(UNKNOWN_TYPE)1000;","(").empty()); // don't know if there is truncation, sign extension
        ASSERT_EQUALS(255, valueOfTok("(unsigned char)~0;", "(").intvalue);
        ASSERT_EQUALS(0, valueOfTok("(int)0;", "(").intvalue);
        ASSERT_EQUALS(3, valueOfTok("(int)(1+2);", "(").intvalue);
        ASSERT_EQUALS(0, valueOfTok("(UNKNOWN_TYPE*)0;","(").intvalue);
        ASSERT_EQUALS(100, valueOfTok("(int)100.0;", "(").intvalue);

        // Don't calculate if there is UB
        ASSERT(tokenValues(";-1<<10;","<<").empty());
        ASSERT(tokenValues(";10<<-1;","<<").empty());
        ASSERT(tokenValues(";10<<64;","<<").empty());
        ASSERT(tokenValues(";-1>>10;",">>").empty());
        ASSERT(tokenValues(";10>>-1;",">>").empty());
        ASSERT(tokenValues(";10>>64;",">>").empty());

        // calculation using 1,2 variables/values
        code  = "void f(int x) {\n"
                "    a = x+456;\n"
                "    if (x==123) {}"
                "}";
        ASSERT_EQUALS(579, valueOfTok(code, "+").intvalue);

        code  = "void f(int x, int y) {\n"
                "    a = x+y;\n"
                "    if (x==123 || y==456) {}"
                "}";
        ASSERT_EQUALS(0, valueOfTok(code, "+").intvalue);

        code  = "void f(int x) {\n"
                "    a = x+x;\n"
                "    if (x==123) {}"
                "}";
        ASSERT_EQUALS(246, valueOfTok(code, "+").intvalue);

        code  = "void f(int x, int y) {\n"
                "    a = x*x;\n"
                "    if (x==2) {}\n"
                "    if (x==4) {}\n"
                "}";
        std::list<ValueFlow::Value> values = tokenValues(code,"*");
        ASSERT_EQUALS(2U, values.size());
        ASSERT_EQUALS(4, values.front().intvalue);
        ASSERT_EQUALS(16, values.back().intvalue);

        code  = "void f(int x) {\n"
                "    if (x == 3) {}\n"
                "    a = x * (1 - x - 1);\n"
                "}";
        ASSERT_EQUALS(-9, valueOfTok(code, "*").intvalue);

        // addition of different variables with known values
        code = "int f(int x) {\n"
               "  int a = 1;\n"
               "  while (x!=3) { x+=a; }\n"
               "  return x/a;\n"
               "}\n";
        ASSERT_EQUALS(3, valueOfTok(code, "/").intvalue);

        // ? :
        code = "x = y ? 2 : 3;\n";
        values = tokenValues(code,"?");
        ASSERT_EQUALS(2U, values.size());
        ASSERT_EQUALS(2, values.front().intvalue);
        ASSERT_EQUALS(3, values.back().intvalue);

        code = "void f(int a) { x = a ? 2 : 3; }\n";
        values = tokenValues(code,"?");
        ASSERT_EQUALS(2U, values.size());
        ASSERT_EQUALS(2, values.front().intvalue);
        ASSERT_EQUALS(3, values.back().intvalue);

        code = "x = (2<5) ? 2 : 3;\n";
        values = tokenValues(code, "?");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(2, values.front().intvalue);

        code = "x = 123 ? : 456;\n";
        values = tokenValues(code, "?");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(123, values.empty() ? 0 : values.front().intvalue);

        // ~
        code  = "x = ~0U;";
        settings.platform(cppcheck::Platform::Native); // ensure platform is native
        values = tokenValues(code,"~");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(~0U, values.back().intvalue);

        // !
        code  = "void f(int x) {\n"
                "    a = !x;\n"
                "    if (x==0) {}\n"
                "}";
        values = tokenValues(code,"!");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.back().intvalue);

        // unary minus
        code  = "void f(int x) {\n"
                "    a = -x;\n"
                "    if (x==10) {}\n"
                "}";
        values = tokenValues(code,"-");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(-10, values.back().intvalue);

        // sizeof
        code  = "void f() {\n"
                "    x = sizeof(int);\n"
                "}";
        values = tokenValues(code,"( int )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(settings.sizeof_int, values.back().intvalue);

        code  = "void f() {\n"
                "    struct S *a[10];"
                "    x = sizeof(a) / sizeof(a[0]);\n"
                "}";
        values = tokenValues(code,"/");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(10, values.back().intvalue);

        // function call => calculation
        code  = "void f(int x) {\n"
                "    a = x + 8;\n"
                "}\n"
                "void callf() {\n"
                "    f(7);\n"
                "}";
        ASSERT_EQUALS(15, valueOfTok(code, "+").intvalue);

        code  = "void f(int x, int y) {\n"
                "    a = x + y;\n"
                "}\n"
                "void callf() {\n"
                "    f(1,1);\n"
                "    f(10,10);\n"
                "}";
        values = tokenValues(code, "+");
        ASSERT_EQUALS(true, values.empty());
        if (!values.empty()) {
            /* todo.. */
            ASSERT_EQUALS(2U, values.size());
            ASSERT_EQUALS(2, values.front().intvalue);
            ASSERT_EQUALS(22, values.back().intvalue);
        }

        // Comparison of string
        values = tokenValues("f(\"xyz\" == \"xyz\");", "=="); // implementation defined
        ASSERT_EQUALS(0U, values.size()); // <- no value

        values = tokenValues("f(\"xyz\" == 0);", "==");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);

        values = tokenValues("f(0 == \"xyz\");", "==");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);

        values = tokenValues("f(\"xyz\" != 0);", "!=");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.front().intvalue);

        values = tokenValues("f(0 != \"xyz\");", "!=");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.front().intvalue);
    }

    void valueFlowBeforeCondition() {
        const char *code;

        code = "void f(int x) {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));

        code = "void f(unsigned int x) {\n"
               "    int a = x;\n"
               "    if (x >= 1) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(unsigned int x) {\n"
               "    int a = x;\n"
               "    if (x > 0) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(unsigned int x) {\n"
               "    int a = x;\n"
               "    if (x > 1) {}\n" // not zero => don't consider > condition
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 1));

        code = "void f(int x) {\n" // not unsigned => don't consider > condition
               "    int a = x;\n"
               "    if (x > 0) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));

        code = "void f(int *x) {\n"
               "    *x = 100;\n"
               "    if (x) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "extern const int x;\n"
               "void f() {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));
    }

    void valueFlowBeforeConditionAssignIncDec() {  // assignment / increment
        const char *code;

        code = "void f(int x) {\n"
               "   x = 2 + x;\n"
               "   if (x == 65);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 65));

        code = "void f(int x) {\n"
               "   x = y = 2 + x;\n"
               "   if (x == 65);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 65));

        code = "void f(int x) {\n"
               "   a[x++] = 0;\n"
               "   if (x == 5);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 5));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x++;\n"
               "   if (x == 4);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 3));

        // bailout: assignment
        bailout("void f(int x) {\n"
                "    x = y;\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: assignment of x\n", errout.str());
    }

    void valueFlowBeforeConditionAndAndOrOrGuard() { // guarding by &&
        const char *code;

        code = "void f(int x) {\n"
               "    if (!x || \n"  // <- x can be 0
               "        a/x) {}\n" // <- x can't be 0
               "    if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int *x) {\n"
               "  ((x=ret())&&\n"
               "   (*x==0));\n"  // <- x is not 0
               "  if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int *x) {\n"
               "  int a = (x && *x == '1');\n"
               "  int b = a ? atoi(x) : 0;\n"  // <- x is not 0
               "  if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));
    }

    void valueFlowBeforeConditionFunctionCall() { // function calls
        const char *code;

        code = "void f(int x) {\n"
               "  a = x;\n"
               "  setx(x);\n"
               "  if (x == 1) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX((std::string("void setx(int x);")+code).c_str(), 2U, 1));
        ASSERT_EQUALS(false, testValueOfX((std::string("void setx(int &x);")+code).c_str(), 2U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 1));

        code = "void f(char* x) {\n"
               "  strcpy(x,\"abc\");\n"
               "  if (x) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void addNewFunction(Scope**scope, const Token**tok);\n"
               "void f(Scope *x) {\n"
               "  x->functionList.back();\n"
               "  addNewFunction(&x,&tok);\n" // address-of, x can be changed by subfunction
               "  if (x) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));
    }

    void valueFlowBeforeConditionLoop() { // while, for, do-while
        const char *code;

        code = "void f(int x) {\n" // loop condition, x is not assigned inside loop => use condition
               "  a = x;\n"  // x can be 37
               "  while (x == 37) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 37));

        code = "void f(int x) {\n" // loop condition, x is assigned inside loop => don't use condition
               "  a = x;\n"  // don't assume that x can be 37
               "  while (x != 37) { x++; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 37));

        code = "void f(int x) {\n"
               "  a = x;\n"
               "  for (; x!=1; x++) { }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 1));

        code = "void f(menu *x) {\n"
               "  a = x->parent;\n"
               "  for (i=0;(i<10) && (x!=0); i++) { x = x->next; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));

        code = "void f(int x) {\n"  // condition inside loop, x is NOT assigned inside loop => use condition
               "    a = x;\n"
               "    do {\n"
               "        if (x==76) {}\n"
               "    } while (1);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 76));

        code = "void f(int x) {\n"  // conditions inside loop, x is assigned inside do-while => don't use condition
               "    a = x;\n"
               "    do {\n"
               "        if (x!=76) { x=do_something(); }\n"
               "    } while (1);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 76));

        code = "void f(X x) {\n"  // conditions inside loop, x is assigned inside do-while => don't use condition
               "    a = x;\n"
               "    for (i=1;i<=count;i++) {\n"
               "        BUGON(x==0)\n"
               "        x = x.next;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));
    }

    void valueFlowBeforeConditionTernaryOp() { // bailout: ?:
        const char *code;

        bailout("void f(int x) {\n"
                "    y = ((x<0) ? x : ((x==2)?3:4));\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: no simplification of x within ?: expression\n", errout.str());

        bailout("int f(int x) {\n"
                "  int r = x ? 1 / x : 0;\n"
                "  if (x == 0) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: no simplification of x within ?: expression\n", errout.str());

        code = "void f(int x) {\n"
               "    int a =v x;\n"
               "    a = b ? x/2 : 20/x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));

        code = "void f(const s *x) {\n"
               "  x->a = 0;\n"
               "  if (x ? x->a : 0) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(int x, int y) {\n"
               "    a = x;\n"
               "    if (y){}\n"
               "    if (x==123){}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));
    }

    void valueFlowBeforeConditionSizeof() { // skip sizeof
        const char *code;

        code = "void f(int *x) {\n"
               "    sizeof(x[0]);\n"
               "    if (x==63){}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 63));

        code = "void f(int *x) {\n"
               "    char a[sizeof x.y];\n"
               "    if (x==0){}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));
    }

    void valueFlowBeforeConditionIfElse() { // bailout: if/else/etc
        const char *code;

        code = "void f(X * x) {\n"
               "  a = x;\n"
               "  if ((x != NULL) &&\n"
               "      (a(x->name, html)) &&\n"
               "      (a(x->name, body))) {}\n"
               "  if (x != NULL) { }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));

        bailout("void f(int x) {\n"
                "    if (x != 123) { b = x; }\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: variable x stopping on }\n", errout.str());

        code = "void f(int x) {\n"
               "  a = x;\n"
               "  if (abc) { x = 1; }\n"  // <- condition must be false if x is 7 in next line
               "  if (x == 7) { }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 7));
    }

    void valueFlowBeforeConditionGlobalVariables() {
        // bailout: global variables
        bailout("int x;\n"
                "void f() {\n"
                "    int a = x;\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:4]: (debug) ValueFlow bailout: global variable x\n", errout.str());

        // class variable
        const char *code;
        code = "class Fred { int x; void clear(); void f(); };\n"
               "void Fred::f() {\n"
               "    int a = x;\n"
               "    clear();\n"  // <- x might be assigned
               "    if (x == 234) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code,3,234));
    }

    void valueFlowBeforeConditionSwitch() {
        // bailout: switch
        // TODO : handle switch/goto more intelligently
        bailout("void f(int x, int y) {\n"
                "    switch (y) {\n"
                "    case 1: a=x; break;\n"
                "    case 2: if (x==5) {} break;\n"
                "    };\n"
                "}");
        ASSERT_EQUALS("[test.cpp:3]: (debug) ValueFlow bailout: variable x stopping on break\n", errout.str());

        bailout("void f(int x, int y) {\n"
                "    switch (y) {\n"
                "    case 1: a=x; return 1;\n"
                "    case 2: if (x==5) {} break;\n"
                "    };\n"
                "}");
        ASSERT_EQUALS("[test.cpp:3]: (debug) ValueFlow bailout: variable x stopping on return\n", errout.str());
    }

    void valueFlowBeforeConditionMacro() {
        // bailout: condition is a expanded macro
        bailout("void f(int x) {\n"
                "    a = x;\n"
                "    $if ($x==$123){}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:3]: (debug) ValueFlow bailout: variable x, condition is defined in macro\n", errout.str());
    }

    void valueFlowBeforeConditionGoto() {
        // bailout: goto label (TODO: handle gotos more intelligently)
        bailout("void f(int x) {\n"
                "    if (x == 123) { goto out; }\n"
                "    a=x;\n"   // <- x is not 123
                "out:"
                "    if (x==123){}\n"
                "}");
        ASSERT_EQUALS("[test.cpp:4]: (debug) ValueFlow bailout: variable x stopping on goto label\n"
                      "[test.cpp:2]: (debug) ValueFlow bailout: variable x. noreturn conditional scope.\n"
                      , errout.str());

        // #5721 - FP
        bailout("static void f(int rc) {\n"
                "    ABC* abc = getabc();\n"
                "    if (!abc) { goto out };\n"
                "\n"
                "    abc->majortype = 0;\n"
                "    if (FAILED(rc)) {}\n"
                "\n"
                "out:\n"
                "    if (abc) {}\n"
                "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (debug) ValueFlow bailout: assignment of abc\n"
                      "[test.cpp:8]: (debug) ValueFlow bailout: variable abc stopping on goto label\n"
                      "[test.cpp:3]: (debug) ValueFlow bailout: variable abc. noreturn conditional scope.\n",
                      errout.str());
    }

    void valueFlowAfterAssign() {
        const char *code;

        code = "void f() {\n"
               "    int x = 123;\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f() {\n"
               "    bool x = 32;\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    a = sizeof(x);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f() {\n"
               "    int x = 9;\n"
               "    --x;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 9));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 8));

        code = "void x() {\n"
               "    int x = value ? 6 : 0;\n"
               "    x =\n"
               "        1 + x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 7));

        code = "void f() {\n"
               "    int x = 0;\n"
               "    y = x += z;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    static int x = 2;\n"
               "    x++;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));

        code = "void f() {\n"
               "    static int x = 2;\n"
               "    a >> x;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));

        code = "void f() {\n"
               "    static int x = 0;\n"
               "    if (x==0) x = getX();\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // function
        code = "void f() {\n"
               "    char *x = 0;\n"
               "    int success = getx((char**)&x);\n"
               "    if (success) x[0] = 0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    char *x = 0;\n"
               "    getx(reinterpret_cast<void **>(&x));\n"
               "    *x = 0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // lambda
        code = "void f() {\n"
               "    int x = 0;\n"
               "    Q q = [&]() {\n"
               "        if (x > 0) {}\n"
               "        x++;\n"
               "    };\n"
               "    dosomething(q);\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // ?:
        code = "void f() {\n"
               "    int x = 8;\n"
               "    a = ((x > 10) ?\n"
               "        x : 0);\n" // <- x is not 8
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 8));

        code = "void f() {\n" // #6973
               "    char *x = \"\";\n"
               "    a = ((x[0] == 'U') ?\n"
               "        x[1] : 0);\n" // <- x is not ""
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, "\"\""));

        code = "void f() {\n" // #6973
               "    char *x = getenv (\"LC_ALL\");\n"
               "    if (x == NULL)\n"
               "        x = \"\";\n"
               "\n"
               "    if ( (x[0] == 'U') &&\n"  // x can be ""
               "         (x[1] ?\n"           // x can't be ""
               "          x[3] :\n"           // x can't be ""
               "          x[2] ))\n"          // x can't be ""
               "    {}\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, "\"\""));
        ASSERT_EQUALS(false, testValueOfX(code, 7U, "\"\""));
        ASSERT_EQUALS(false, testValueOfX(code, 8U, "\"\""));
        ASSERT_EQUALS(false, testValueOfX(code, 9U, "\"\""));

        code = "void f() {\n" // #7599
               "  t *x = 0;\n"
               "  y = (a ? 1 : x\n" // <- x is 0
               "       && x->y ? 1 : 2);" // <- x is not 0
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #7599
               "  t *x = 0;\n"
               "  y = (a ? 1 : !x\n" // <- x is 0
               "       || x->y ? 1 : 2);" // <- x is not 0
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // if/else
        code = "void f() {\n"
               "    int x = 123;\n"
               "    if (condition) return;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f() {\n"
               "    int x = 1;\n"
               "    if (condition) x = 2;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 2));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    if (condition1) x = 456;\n"
               "    if (condition2) x = 789;\n"
               "    a = 2 + x;\n" // <- either assignment "x=123" is redundant or x can be 123 here.
               "}";
        ASSERT_EQUALS(true,  testValueOfX(code, 5U, 123));

        code = "void f(int a) {\n"
               "    int x = 123;\n"
               "    if (a > 1)\n"
               "        ++x;\n"
               "    else\n"
               "        ++x;\n"
               "    return 2 + x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        code = "void f() {\n"
               "    int x = 1;\n"
               "    if (condition1) x = 2;\n"
               "    else return;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 1));

        code = "void f(){\n"
               "    int x = 0;\n"
               "    if (a>=0) { x = getx(); }\n"
               "    if (x==0) { return; }\n"
               "    return 123 / x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));

        code = "void f() {\n"
               "  X *x = getx();\n"
               "  if(0) { x = 0; }\n"
               "  else { x->y = 1; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #6239
               "  int x = 4;\n"
               "  if(1) { x = 0; }\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 4));

        code = "void f() {\n"
               "    int x = 32;\n"
               "    if (x>=32) return;\n"
               "    a[x]=0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 32));

        code = "void f() {\n"
               "    int x = 32;\n"
               "    if (x>=32) {\n"
               "        a[x] = 0;\n"  // <- should have possible value 32
               "        return;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 32));

        code = "void f() {\n"
               "    int x = 33;\n"
               "    if (x==33) goto fail;\n"
               "    a[x]=0;\n"
               "fail:\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 33));

        code = "void f() {\n"
               "    int x = 32;\n"
               "    if (a==1) { z=x+12; }\n"
               "    if (a==2) { z=x+32; }\n"
               "    z = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 32));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 32));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 32));

        code = "void f() {\n" // #5656 - FP
               "    int x = 0;\n"
               "    if (!x) {\n"
               "        x = getx();\n"
               "    }\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 0));

        code = "void f(int y) {\n" // alias
               "  int x = y;\n"
               "  if (y == 54) {}\n"
               "  else { a = x; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 54));

        code = "void f () {\n"
               "    ST * x =  g_pST;\n"
               "    if (x->y == 0) {\n"
               "        x = NULL;\n"
               "        return 1;\n"
               "    }\n"
               "    a = x->y;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));

        code = "void f () {\n"
               "    ST * x =  g_pST;\n"
               "    if (x->y == 0) {\n"
               "        x = NULL;\n"
               "        goto label;\n"
               "    }\n"
               "    a = x->y;\n"
               "label:\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));

        code = "void f() {\n" // #5752 - FP
               "    int *x = 0;\n"
               "    if (x && *x == 123) {\n"
               "        getx(*x);\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    int x = 0;\n"
               "    if (!x) {}\n"
               "    else { y = x; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #6118 - FP
               "    int x = 0;\n"
               "    x = x & 0x1;\n"
               "    if (x == 0) { x = 2; }\n"
               "    y = 42 / x;\n" // <- x is 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

        code = "void f() {\n" // #6118 - FN
               "    int x = 0;\n"
               "    x = x & 0x1;\n"
               "    if (x == 0) { x += 2; }\n"
               "    y = 42 / x;\n" // <- x is 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 5U, 2));

        code = "void f(int mode) {\n"
               "    struct ABC *x;\n"
               "\n"
               "    if (mode) { x = &y; }\n"
               "    else { x = NULL; }\n"
               "\n"
               "    if (!x) exit(1);\n"
               "\n"
               "    a = x->a;\n" // <- x can't be 0
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 0));

        // multivariables
        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    if (a!=132) { b = x; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 132));

        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    b = x;\n" // <- line 3
               "    if (a!=132) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 132));

        code = "void f() {\n"
               "    int a;\n"
               "    if (n) { a = n; }\n"
               "    else { a = 0; }\n"
               "    int x = a;\n"
               "    if (a > 0) { a = b / x; }\n" // <- line 6
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 0)); // x is not 0 at line 6

        code = "void f(int x1) {\n" // #6086
               "  int x = x1;\n"
               "  if (x1 >= 3) {\n"
               "    return;\n"
               "  }\n"
               "  a = x;\n"  // <- x is not 3
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 3));

        code = "int f(int *x) {\n" // #5980
               "  if (!x) {\n"
               "    switch (i) {\n"
               "      default:\n"
               "        throw std::runtime_error(msg);\n"
               "    };\n"
               "  }\n"
               "  return *x;\n"  // <- x is not 0
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 0));

        code = "void f(int a) {\n" // #6826
               "    int x = a ? a : 87;\n"
               "    if (a && x) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 87));

        code = "void f() {\n"
               "  int first=-1, x=0;\n"
               "  do {\n"
               "    if (first >= 0) { a = x; }\n" // <- x is not 0
               "    first++; x=3;\n"
               "  } while (1);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // pointer/reference to x
        code = "int f(void) {\n"
               "  int x = 2;\n"
               "  int *px = &x;\n"
               "  for (int i = 0; i < 1; i++) {\n"
               "    *px = 1;\n"
               "  }\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 2));

        code = "int f(void) {\n"
               "  int x = 5;\n"
               "  int &rx = x;\n"
               "  for (int i = 0; i < 1; i++) {\n"
               "    rx = 1;\n"
               "  }\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 5));

        // break
        code = "void f() {\n"
               "  for (;;) {\n"
               "    int x = 1;\n"
               "    if (!abc()) {\n"
               "      x = 2;\n"
               "      break;\n"
               "    }\n"
               "    a = x;\n" // <- line 8
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 2)); // x is not 2 at line 8

        code = "void f() {\n"
               "  int x;\n"
               "  switch (ab) {\n"
               "    case A: x = 12; break;\n"
               "    case B: x = 34; break;\n"
               "  }\n"
               "  v = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 12));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 34));

        code = "void f() {\n" // #5981
               "  int x;\n"
               "  switch (ab) {\n"
               "    case A: x = 12; break;\n"
               "    case B: x = 34; break;\n"
               "  }\n"
               "  switch (ab) {\n"
               "    case A: v = x; break;\n" // <- x is not 34
               "  }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 34));

        // while/for
        code = "void f() {\n" // #6138
               "  ENTRY *x = 0;\n"
               "  while (x = get()) {\n"
               "    set(x->value);\n"  // <- x is not 0
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f(const int *buf) {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can be 0
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0)); // x can be 0 at line 9

        code = "void f(const int *buf) {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      ;\n" // <- no break
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 0)); // x can't be 0 at line 9

        code = "void f(const int *buf) {\n"
               "  int x = 0;\n"
               "  while (++i < 10) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can be 0
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0)); // x can be 0 at line 9

        code = "void f(const int a[]) {\n" // #6616
               "  const int *x = 0;\n"
               "  for (int i = 0; i < 10; i = *x) {\n" // <- x is not 0
               "    x = a[i];\n"
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // alias
        code = "void f() {\n" // #7778
               "  int x = 0;\n"
               "  int *p = &x;\n"
               "  x = 3;\n"
               "  *p = 2;\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 3));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 6U, 2));
    }

    void valueFlowAfterCondition() {
        const char *code;

        // in if
        code = "void f(int x) {\n"
               "    if (x == 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x != 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        // in else
        code = "void f(int x) {\n"
               "    if (x == 123) {}\n"
               "    else a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x != 123) {}\n"
               "    else a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        // after if
        code = "void f(int x) {\n"
               "    if (x == 10) {\n"
               "        x++;\n"
               "    }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 10));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 5U, 11));

        // !
        code = "void f(int x) {\n"
               "    if (!x) { a = x; }\n"
               "    else a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(int x, int y) {\n"
               "    if (!(x&&y)) { return; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int x) {\n"
               "    if (!x) { { throw new string(); }; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // if (var)
        code = "void f(int x) {\n"
               "    if (x) { a = x; }\n"  // <- x is not 0
               "    else { b = x; }\n"    // <- x is 0
               "    c = x;\n"             // <- x might be 0
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(true,  testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 0));

        // After while
        code = "void f(int x) {\n"
               "    while (x != 3) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 3));

        code = "void f(int x) {\n"
               "    while (11 != (x = dostuff())) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 11));

        code = "void f(int x) {\n"
               "    while (11 != (x = dostuff()) && y) {}\n"
               "    a = x;\n"
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, 11));

        code = "void f(int x) {\n"
               "    while (x = dostuff()) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(const Token *x) {\n" // #5866
               "    x = x->next();\n"
               "    while (x) { x = x->next(); }\n"
               "    if (x->str()) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "void f(const Token *x) {\n"
               "  while (0 != (x = x->next)) {}\n"
               "  x->ab = 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(const Token* x) {\n"
               "  while (0 != (x = x->next)) {}\n"
               "  if (x->str) {\n" // <- possible value 0
               "    x = y;\n" // <- this caused some problem
               "  }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        // conditional code after if/else/while
        code = "void f(int x) {\n"
               "  if (x == 2) {}\n"
               "  if (x > 0)\n"
               "    a = x;\n"  // <- TODO, x can be 2
               "  else\n"
               "    b = x;\n"
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 4U, 2));
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 2));

        // condition with 2nd variable
        code = "void f(int x) {\n"
               "  int y = 0;\n"
               "  if (x == 7) { y = 1; }\n"
               "  if (!y)\n"
               "    a = x;\n" // <- x can not be 7 here
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 7));

        code = "void f(struct X *x) {\n"
               "  bool b = TRUE;\n"
               "  if(x) { }\n"
               "  else\n"
               "    b = FALSE;\n"
               "  if (b)\n"
               "    abc(x->value);\n" // <- x is not 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));

        // In condition, after && and ||
        code = "void f(int x) {\n"
               "  a = (x != 3 ||\n"
               "       x);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 3));

        code = "void f(int x) {\n"
               "  a = (x == 4 &&\n"
               "       x);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 4));

        // protected usage with &&
        code = "void f(const Token* x) {\n"
               "    if (x) {}\n"
               "    for (; x && \n"
               "         x->str() != y; x = x->next()) {}\n"
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f(const Token* x) {\n"
               "    if (x) {}\n"
               "    if (x && \n"
               "        x->str() != y) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // return
        code = "void f(int x) {\n" // #6024
               "  if (x == 5) {\n"
               "    if (z) return; else return;\n"
               "  }\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 5));

        code = "void f(int x) {\n" // #6730
               "  if (x == 5) {\n"
               "    if (z) continue; else throw e;\n"
               "  }\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 5));

        // TODO: float
        code = "void f(float x) {\n"
               "  if (x == 0.5) {}\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));
    }

    void valueFlowBitAnd() {
        const char *code;

        code = "int f(int a) {\n"
               "  int x = a & 0x80;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0));
        ASSERT_EQUALS(true, testValueOfX(code,3U,0x80));

        code = "int f(int a) {\n"
               "  int x = a & 0x80 ? 1 : 2;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code,3U,0));
        ASSERT_EQUALS(false, testValueOfX(code,3U,0x80));

        code = "int f() {\n"
               "  int x = (19 - 3) & 15;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0));
        ASSERT_EQUALS(false, testValueOfX(code,3U,16));
    }

    void valueFlowSwitchVariable() {
        const char *code;
        code = "void f(int x) {\n"
               "    a = x;\n"  // <- x can be 14
               "    switch (x) {\n"
               "    case 14: a=x; break;\n"  // <- x is 14
               "    };\n"
               "    a = x;\n"  // <- x can be 14
               "}";
        ASSERT_EQUALS(true, testConditionalValueOfX(code, 2U, 14));
        ASSERT_EQUALS(true, testConditionalValueOfX(code, 4U, 14));
        ASSERT_EQUALS(true, testConditionalValueOfX(code, 6U, 14));
    }

    void valueFlowForLoop() {
        const char *code;

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x++)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 10));

        code = "void f() {\n"
               "    int x;\n"
               "    for (x = 2; x < 1; x++)\n"
               "        a[x] = 0;\n" // <- not 2
               "    b = x;\n" // 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

        code = "void f() {\n"
               "    int x;\n"
               "    for (x = 2; x < 1; ++x)\n"
               "        a[x] = 0;\n" // <- not 2
               "    b = x;\n" // 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

        code = "enum AB {A,B};\n" // enum => handled by valueForLoop2
               "void f() {\n"
               "    int x;\n"
               "    for (x = 1; x < B; ++x)\n"
               "        a[x] = 0;\n" // <- not 1
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 1));

        code = "void f(int a) {\n"
               "    for (int x = a; x < 10; x++)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x = x + 2)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 8));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 10));

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x = x / 0)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0)); // don't crash

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x++)\n"
               "        x<4 ?\n"
               "        a[x] : 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 9));

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x++)\n"
               "        x==0 ?\n"
               "        0 : a[x];\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #5223
               "    for (int x = 0; x < 300 && x < 18; x++)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 17));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 299));

        code = "void f() {\n"
               "    int x;\n"
               "    for (int i = 0; x = bar[i]; i++)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    const char abc[] = \"abc\";\n"
               "    int x;\n"
               "    for (x = 0; abc[x] != '\\0'; x++) {}\n"
               "    a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 3));

        code = "void f() {\n" // #5939
               "    int x;\n"
               "    for (int x = 0; (x = do_something()) != 0;)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    int x;\n"
               "    for (int x = 0; x < 10 && y = do_something();)\n"
               "        x;\n"
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    int x,y;\n"
               "    for (x = 0, y = 0; x < 10, y < 10; x++, y++)\n" // usage of ,
               "        x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "void foo(double recoveredX) {\n"
               "  for (double x = 1e-18; x < 1e40; x *= 1.9) {\n"
               "    double relativeError = (x - recoveredX) / x;\n"
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // &&
        code = "void foo() {\n"
               "  for (int x = 0; x < 10; x++) {\n"
               "    if (x > 1\n"
               "        && x) {}" // <- x is not 0
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 9));

        code = "void foo() {\n"
               "  for (int x = 0; x < 10; x++) {\n"
               "    if (x < value\n"
               "        && x) {}" // <- maybe x is not 9
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 9));

        // ||
        code = "void foo() {\n"
               "  for (int x = 0; x < 10; x++) {\n"
               "    if (x == 0\n"
               "        || x) {}" // <- x is not 0
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 9));

        // After loop
        code = "void foo() {\n"
               "  int x;\n"
               "  for (x = 0; x < 10; x++) {}\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 10));

        code = "void foo() {\n"
               "  int x;\n"
               "  for (x = 0; 2 * x < 20; x++) {}\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 10));

        code = "void foo() {\n" // related with #887
               "  int x;\n"
               "  for (x = 0; x < 20; x++) {}\n"
               "  a = x++;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 20));

        code = "void f() {\n"
               "  int x;\n"
               "  for (x = 0; x < 5; x++) {}\n"
               "  if (x == 5) {\n"
               "    panic();\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 5
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 5));

        code = "void f() {\n"
               "  int x;\n"
               "  for (x = 0; x < 5; x++) {}\n"
               "  if (x < 5) {}\n"
               "  else return;\n"
               "  a = x;\n" // <- x can't be 5
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 5));

        // assert after for loop..
        code = "static void f() {\n"
               "  int x;\n"
               "  int ctls[10];\n"
               "  for (x = 0; x <= 10; x++) {\n"
               "    if (cond)\n"
               "      break;\n"
               "  }\n"
               "  assert(x <= 10);\n"
               "  ctls[x] = 123;\n" // <- x can't be 11
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 11));


        // hang
        code = "void f() {\n"
               "  for(int i = 0; i < 20; i++)\n"
               "    n = (int)(i < 10 || abs(negWander) < abs(negTravel));\n"
               "}";
        testValueOfX(code,0,0); // <- don't hang

        // conditional code in loop
        code = "void f(int mask) {\n" // #6000
               "  for (int x = 10; x < 14; x++) {\n"
               "    int bit = mask & (1 << i);\n"
               "    if (bit) {\n"
               "      if (bit == (1 << 10)) {}\n"
               "      else { a = x; }\n" // <- x is not 10
               "    }\n"
               "  }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 10));

    }

    void valueFlowSubFunction() {
        const char *code;

        code = "void f1(int x) { return x; }\n"
               "void f2(int x) {\n"
               "    f1(123);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 123));

        code = "void f1(int x) { return x; }\n"
               "void f2(int x) {\n"
               "    f1(x);\n"
               "    if (x==0){}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 0));

        code = "void f1(int x) {\n"
               "    if (x == 0) return;\n"
               "    int y = 1234 / x;\n"
               "}\n"
               "\n"
               "void f2() {\n"
               "    f1(0);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f1(int x) {\n"
               "    if (x == 0) return;\n"
               "    int y = x;\n"
               "}\n"
               "\n"
               "void f2() {\n"
               "    f1(x&4);\n" // possible {0,4}
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 4));

        code = "void f1(int x) { a=x; }\n"
               "void f2(int y) { f1(y<123); }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 1));

        code = "void f1(int x) { a=(abc)x; }\n"
               "void f2(int y) { f1(123); }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 123));

        code = "void f1(int x) {\n"
               "  x ?\n"
               "  1024 / x :\n"
               "  0; }\n"
               "void f2() { f1(0); }";
        ASSERT_EQUALS(true,  testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f1(int *x) {\n"
               "  if (x &&\n"
               "      *x) {}\n"
               "}\n"
               "void f2() { f1(0); }";
        ASSERT_EQUALS(true,  testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // #5861 - fp with float
        code = "void f1(float x) {\n"
               "  return 1.0 / x;\n"
               "}\n"
               "void f2() { f1(0.5); }";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));

        code = "void dostuff(int x) {\n"
               "  return x/x;\n"
               "}\n"
               "\n"
               "void test(int x) {\n"
               "  if(x==1) {}\n"
               "  dostuff(x+1);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 2));

        code = "void leaveNotifyEvent(const XCrossingEvent * const) { }\n"
               "void motionNotifyEvent() {\n"
               "    leaveNotifyEvent(0);\n"
               "}";
        testValueOfX(code, 2U, 2); // No complaint about Token::Match called with varid 0. (#6443)

        // #6560 - multivariables
        code = "void f1(int x) {\n"
               "  int a = x && y;\n"
               "  int b = a ? x : 0;\n"
               "}\n"
               "void f2() {\n"
               "  f1(0);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "class A\n"
               "{\n"
               "    void f1(int x) { return x; }\n"
               "    void f2(int x) {\n"
               "        f1(123);\n"
               "    }\n"
               "};";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "class A\n"
               "{\n"
               "    virtual void f1(int x) { return x; }\n"
               "    void f2(int x) {\n"
               "        f1(123);\n"
               "    }\n"
               "};";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void foo(int x, int y) {\n"
               "  if (y == 1) {\n"
               "    a = x;\n"  // <- x is not 1
               "  }\n"
               "}\n"
               "\n"
               "void bar() {\n"
               "  foo(1, 10);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));
    }

    void valueFlowFunctionReturn() {
        const char *code;

        code = "int f1(int x) {\n"
               "  return x+1;\n"
               "}\n"
               "void f2() {\n"
               "    x = 10 - f1(2);\n"
               "}";
        ASSERT_EQUALS(7, valueOfTok(code, "-").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "-").isKnown());

        code = "int add(int x, int y) {\n"
               "  return x+y;\n"
               "}\n"
               "void f2() {\n"
               "    x = 1 * add(10+1,4);\n"
               "}";
        ASSERT_EQUALS(15, valueOfTok(code, "*").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "*").isKnown());

        code = "int one() { return 1; }\n"
               "void f() { x = 1 * one(); }";
        ASSERT_EQUALS(1, valueOfTok(code, "*").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "*").isKnown());

        code = "int add(int x, int y) {\n"
               "  return x+y;\n"
               "}\n"
               "void f2() {\n"
               "    x = 1 * add(1,add(2,3));\n"
               "}";
        ASSERT_EQUALS(6, valueOfTok(code, "*").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "*").isKnown());

        code = "int f(int i, X x) {\n"
               "    if (i)\n"
               "        return g(std::move(x));\n"
               "    g(x);\n"
               "    return 0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MovedVariable));

        code = "class A\n"
               "{\n"
               "    int f1(int x) {\n"
               "        return x+1;\n"
               "    }\n"
               "    void f2() {\n"
               "        x = 10 - f1(2);\n"
               "    }\n"
               "};";
        ASSERT_EQUALS(7, valueOfTok(code, "-").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "-").isKnown());

        code = "class A\n"
               "{\n"
               "    virtual int f1(int x) {\n"
               "        return x+1;\n"
               "    }\n"
               "    void f2() {\n"
               "        x = 10 - f1(2);\n"
               "    }\n"
               "};";
        ASSERT_EQUALS(7, valueOfTok(code, "-").intvalue);
        ASSERT_EQUALS(false, valueOfTok(code, "-").isKnown());
    }

    void valueFlowFunctionDefaultParameter() {
        const char *code;

        code = "class continuous_src_time {\n"
               "    continuous_src_time(std::complex<double> f, double st = 0.0, double et = infinity) {}\n"
               "};";
        testValueOfX(code, 2U, 2); // Don't crash (#6494)
    }

    bool isNotKnownValues(const char code[], const char str[]) {
        const std::list<ValueFlow::Value> values = tokenValues(code, str);
        for (std::list<ValueFlow::Value>::const_iterator it = values.begin(); it != values.end(); ++it) {
            if (it->isKnown())
                return false;
        }
        return true;
    }

    void knownValue() {
        const char *code;
        ValueFlow::Value value;

        ASSERT(valueOfTok("x = 1;", "1").isKnown());

        // after assignment
        code = "void f() {\n"
               "  int x = 1;\n"
               "  return x + 2;\n" // <- known value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(3, value.intvalue);
        ASSERT(value.isKnown());

        {
            code = "void f() {\n"
                   "  int x = 15;\n"
                   "  if (x == 15) { x += 7; }\n" // <- condition is true
                   "}";
            value = valueOfTok(code, "==");
            ASSERT_EQUALS(1, value.intvalue);
            ASSERT(value.isKnown());

            code = "int f() {\n"
                   "    int a = 0, x = 0;\n"
                   "    a = index();\n"
                   "    if (a != 0)\n"
                   "        x = next();\n"
                   "    return x + 1;\n"
                   "}\n";
            value = valueOfTok(code, "+");
            ASSERT(value.isPossible());
        }

        code = "void f() {\n"
               "  int x;\n"
               "  if (ab) { x = 7; }\n"
               "  return x + 2;\n" // <- possible value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(9, value.intvalue);
        ASSERT(value.isPossible());

        code = "void f() {\n"
               "  int x = 0;\n"
               "  fred.dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void dostuff(int x);\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        value = valueOfTok(code, "<");
        ASSERT_EQUALS(0, value.intvalue);
        ASSERT(value.isKnown());

        code = "void dostuff(int & x);\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void dostuff(const int & x);\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        value = valueOfTok(code, "<");
        ASSERT_EQUALS(0, value.intvalue);
        ASSERT(value.isKnown());

        code = "void f() {\n"
               "  int x = 0;\n"
               "  do {\n"
               "    if (x < 0) {}\n"
               "    fred.dostuff(x);\n"
               "  } while (abc);\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  if (y) { dostuff(x); }\n"
               "  if (!x) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  MACRO( v, { if (y) { x++; } } );\n"
               "  if (!x) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (cond) {\n"
               "      x = 1;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  if (!x) {}\n"  // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  switch (state) {\n"
               "  case 1:\n"
               "    x = 1;\n"
               "    break;\n"
               "  }\n"
               "  if (!x) {}\n"  // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n" // #7049
               "  int x = 0;\n"
               "  switch (a) {\n"
               "  case 1:\n"
               "    x = 1;\n"
               "  case 2:\n"
               "    if (!x) {}\n" // <- possible value
               "  }\n"
               "}";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  while (!x) {\n" // <- possible value
               "    scanf(\"%d\", &x);\n"
               "  }\n"
               "}";
        value = valueOfTok(code, "!");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isPossible());

        code = "void f() {\n"
               "  int x = 0;\n"
               "  do { } while (++x < 12);\n" // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  static int x = 0;\n"
               "  return x + 1;\n" // <- possible value
               "}\n";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isPossible());

        code = "void f() {\n"
               "  int x = 0;\n"
               "a:\n"
               "  a = x + 1;\n" // <- possible value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isPossible());

        // after condition
        code = "int f(int x) {\n"
               "  if (x == 4) {}\n"
               "  return x + 1;\n" // <- possible value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(5, value.intvalue);
        ASSERT(value.isPossible());

        code = "int f(int x) {\n"
               "  if (x < 2) {}\n"
               "  else if (x >= 2) {}\n" // <- known value
               "}";
        value = valueOfTok(code, ">=");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

        code = "int f(int x) {\n"
               "  if (x < 2) {}\n"
               "  else if (x > 2) {}\n" // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, ">"));

        // function
        code = "int f(int x) { return x + 1; }\n" // <- possible value
               "void a() { f(12); }";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(13, value.intvalue);
        ASSERT(value.isPossible());

        // known and possible value
        code = "void f() {\n"
               "    int x = 1;\n"
               "    int y = 2 + x;\n" // <- known value, don't care about condition
               "    if (x == 2) {}\n"
               "}";
        ASSERT_EQUALS(true,  testValueOfX(code, 3U, 1)); // value of x can be 1
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 2)); // value of x can't be 2

        // calculation with known result
        code = "int f(int x) { a = x & 0; }"; // <- & is 0
        value = valueOfTok(code, "&");
        ASSERT_EQUALS(0, value.intvalue);
        ASSERT(value.isKnown());

        // Ticket #7139
        // "<<" in third expression of for
        code = "void f(void) {\n"
               "    int bit, x;\n"
               "    for (bit = 1, x = 0; bit < 128; bit = bit << 1, x++) {\n"
               "        z = x;\n"       // <- known value [0..6]
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 6));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 7));
        ASSERT(value.isKnown());
    }

    void valueFlowSizeofForwardDeclaredEnum() {
        const char *code = "enum E; sz=sizeof(E);";
        valueOfTok(code, "="); // Don't crash (#7775)
    }

    void valueFlowGlobalVar() {
        const char *code;

        code = "int x;\n"
               "void f() {\n"
               "    x = 4;\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 4));

        code = "int x;\n"
               "void f() {\n"
               "    if (x == 4) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 4));

        code = "int x;\n"
               "void f() {\n"
               "    x = 42;\n"
               "    unknownFunction();\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 42));
    }
};

REGISTER_TEST(TestValueFlow)
