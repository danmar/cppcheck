/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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


class TestValueFlow : public TestFixture {
public:
    TestValueFlow() : TestFixture("TestValueFlow") {
    }

private:

    void run() {
        TEST_CASE(valueFlowNumber);
        TEST_CASE(valueFlowString);
        TEST_CASE(valueFlowPointerAlias);
        TEST_CASE(valueFlowArrayElement);

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

        TEST_CASE(valueFlowForLoop);
        TEST_CASE(valueFlowSubFunction);
        TEST_CASE(valueFlowFunctionReturn);

        TEST_CASE(valueFlowFunctionDefaultParameter);
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
                    if (it->intvalue == value && !it->tokvalue)
                        return true;
                }
            }
        }

        return false;
    }


    bool testValueOfX(const char code[], unsigned int linenr, const char value[]) {
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
                    if (Token::simpleMatch(it->tokvalue, value))
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
        return values.size() == 1U && !values.front().tokvalue ? values.front() : ValueFlow::Value();
    }

    void valueFlowNumber() {
        const char *code;

        code  = "void f() {\n"
                "    x = 123;\n"
                "}";
        ASSERT_EQUALS(123, valueOfTok(code, "123").intvalue);
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
    }

    void valueFlowArrayElement() {
        const char *code;

        code  = "void f() {\n"
                "    const int a[] = {43,23,12};\n"
                "    int x = a[0];\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 43));

        code  = "void f() {\n"
                "    const char abcd[] = \"abcd\";\n"
                "    int x = abcd[2];\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 'c'));
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
               "    y = 42 / x;\n" // <- x can't be 0
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));

        code = "void f() {\n" // #6118 - FN
               "    int x = 0;\n"
               "    x = x & 0x1;\n"
               "    if (x == 0) { x += 2; }\n"
               "    y = 42 / x;\n" // <- x can be 2
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

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
               "  a = x;\n" // <- x cant be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 0)); // x cant be 0 at line 9

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
    }

    void valueFlowAfterCondition() {
        const char *code;

        // if
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

        // else
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
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

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
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 4U, 20));

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

        // hang
        code = "void f() {\n"
               "  for(int i = 0; i < 20; i++)\n"
               "    n = (int)(i < 10 || abs(negWander) < abs(negTravel));\n"
               "}";
        testValueOfX(code,0,0); // <- dont hang
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
    }

    void valueFlowFunctionReturn() {
        const char *code;

        code = "void f1(int x) {\n"
               "  return x+1;\n"
               "}\n"
               "void f2() {\n"
               "    x = 10 - f1(2);\n"
               "}";
        ASSERT_EQUALS(7, valueOfTok(code, "-").intvalue);

        code = "void add(int x, int y) {\n"
               "  return x+y;\n"
               "}\n"
               "void f2() {\n"
               "    x = 1 * add(10+1,4);\n"
               "}";
        ASSERT_EQUALS(15, valueOfTok(code, "*").intvalue);
    }

    void valueFlowFunctionDefaultParameter() {
        const char *code;

        code = "class continuous_src_time {\n"
               "    continuous_src_time(std::complex<double> f, double st = 0.0, double et = infinity) {}\n"
               "};";
        testValueOfX(code, 2U, 2); // Don't crash (#6494)
    }
};

REGISTER_TEST(TestValueFlow)
