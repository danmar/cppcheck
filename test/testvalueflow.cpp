/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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

extern std::ostringstream errout;
class TestValueFlow : public TestFixture {
public:
    TestValueFlow() : TestFixture("TestValueFlow") {
    }

private:

    void run() {
        TEST_CASE(valueFlowNumber);

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

        TEST_CASE(valueFlowForLoop);
        TEST_CASE(valueFlowSubFunction);
    }

    bool testValueOfX(const char code[], unsigned int linenr, int value) {
        Settings settings;

        // strcpy cfg
        const char cfg[] = "<?xml version=\"1.0\"?>\n"
                           "<def>\n"
                           "  <function name=\"strcpy\"> <arg nr=\"1\"><not-null/></arg> </function>\n"
                           "</def>";
        settings.library.loadxmldata(cfg, sizeof(cfg));

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                std::list<ValueFlow::Value>::const_iterator it;
                for (it = tok->values.begin(); it != tok->values.end(); ++it) {
                    if (it->intvalue == value)
                        return true;
                }
            }
        }

        return false;
    }


    void bailout(const char code[]) {
        Settings settings;
        settings.debugwarnings = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");
    }

    std::list<ValueFlow::Value> tokenValues(const char code[], const char tokstr[]) {
        const Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");
        const Token *tok = Token::findmatch(tokenizer.tokens(), tokstr);
        return tok ? tok->values : std::list<ValueFlow::Value>();
    }

    ValueFlow::Value valueOfTok(const char code[], const char tokstr[]) {
        std::list<ValueFlow::Value> values = tokenValues(code, tokstr);
        return values.size() == 1U ? values.front() : ValueFlow::Value();
    }

    void valueFlowNumber() {
        const char *code;

        code  = "void f() {\n"
                "    x = 123;\n"
                "}";
        ASSERT_EQUALS(123, valueOfTok(code, "123").intvalue);
    }

    void valueFlowCalculations() {
        const char *code;
        /*
                code  = "void f() {\n"
                        "    x = 123+456;\n"
                        "}";
                ASSERT_EQUALS(579, valueOfTok(code, "+").intvalue);
        */
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

        code = "void f(int x) {\n" // loop condition, x is assigned inside loop => dont use condition
               "  a = x;\n"  // don't assume that x can be 37
               "  while (x != 37) { x++; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 37));

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

        code = "void f(int x) {\n"  // conditions inside loop, x is assigned inside do-while => dont use condition
               "    a = x;\n"
               "    do {\n"
               "        if (x!=76) { x=do_something(); }\n"
               "    } while (1);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 76));

        code = "void f(X x) {\n"  // conditions inside loop, x is assigned inside do-while => dont use condition
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
        ASSERT_EQUALS("[test.cpp:4]: (debug) ValueFlow bailout: variable x stopping on goto label\n", errout.str());

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
                      "[test.cpp:8]: (debug) ValueFlow bailout: variable abc stopping on goto label\n",
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

        code = "void f() {\n"
               "    int x = 123;\n"
               "    if (condition1) x = 456;\n"
               "    if (condition2) x = 789;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

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
               "  if(false) { x = 0; }\n"
               "  else { x->y = 1; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

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
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 4U, 32));

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
    }

    void valueFlowBitAnd() {
        const char *code;

        code = "int f(int a) {\n"
               "  int x = a & 0x80;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0));
        ASSERT_EQUALS(true, testValueOfX(code,3U,0x80));
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

        code = "void f() {\n"
               "    int x;\n"
               "    for (int i = 0; x = bar[i]; i++)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
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

        code = "void f1(int x) { a=x; }\n"
               "void f2(int y) { f1(y<123); }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 1));

        code = "void f1(int x) { a=(abc)x; }\n"
               "void f2(int y) { f1(123); }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 1U, 123));
    }
};

REGISTER_TEST(TestValueFlow)
