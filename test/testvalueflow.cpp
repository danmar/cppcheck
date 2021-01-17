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

#include "library.h"
#include "platform.h"
#include "settings.h"
#include "testsuite.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"

#include <simplecpp.h>
#include <algorithm>
#include <cmath>
#include <list>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstring>

class TestValueFlow : public TestFixture {
public:
    TestValueFlow() : TestFixture("TestValueFlow") {
    }

private:
    Settings settings;

    void run() OVERRIDE {
        // strcpy, abort cfg
        const char cfg[] = "<?xml version=\"1.0\"?>\n"
        "<def>\n"
        "  <function name=\"strcpy\"> <arg nr=\"1\"><not-null/></arg> </function>\n"
        "  <function name=\"abort\"> <noreturn>true</noreturn> </function>\n" // abort is a noreturn function
        "</def>";
        settings.library.loadxmldata(cfg, sizeof(cfg));
        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(valueFlowNumber);
        TEST_CASE(valueFlowString);
        TEST_CASE(valueFlowPointerAlias);
        TEST_CASE(valueFlowLifetime);
        TEST_CASE(valueFlowArrayElement);
        TEST_CASE(valueFlowMove);

        TEST_CASE(valueFlowBitAnd);
        TEST_CASE(valueFlowRightShift);

        TEST_CASE(valueFlowCalculations);
        TEST_CASE(valueFlowSizeof);

        TEST_CASE(valueFlowErrorPath);

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
        TEST_CASE(valueFlowBeforeConditionForward);

        TEST_CASE(valueFlowAfterAssign);
        TEST_CASE(valueFlowAfterCondition);
        TEST_CASE(valueFlowAfterConditionExpr);
        TEST_CASE(valueFlowAfterConditionSeveralNot);
        TEST_CASE(valueFlowForwardCompoundAssign);
        TEST_CASE(valueFlowForwardCorrelatedVariables);
        TEST_CASE(valueFlowForwardModifiedVariables);
        TEST_CASE(valueFlowForwardFunction);
        TEST_CASE(valueFlowForwardTernary);
        TEST_CASE(valueFlowForwardLambda);
        TEST_CASE(valueFlowForwardTryCatch);
        TEST_CASE(valueFlowForwardInconclusiveImpossible);

        TEST_CASE(valueFlowFwdAnalysis);

        TEST_CASE(valueFlowSwitchVariable);

        TEST_CASE(valueFlowForLoop);
        TEST_CASE(valueFlowSubFunction);
        TEST_CASE(valueFlowFunctionReturn);

        TEST_CASE(valueFlowFunctionDefaultParameter);

        TEST_CASE(knownValue);

        TEST_CASE(valueFlowSizeofForwardDeclaredEnum);

        TEST_CASE(valueFlowGlobalVar);

        TEST_CASE(valueFlowGlobalConstVar);

        TEST_CASE(valueFlowGlobalStaticVar);

        TEST_CASE(valueFlowInlineAssembly);

        TEST_CASE(valueFlowSameExpression);

        TEST_CASE(valueFlowUninit);

        TEST_CASE(valueFlowTerminatingCond);

        TEST_CASE(valueFlowContainerSize);

        TEST_CASE(valueFlowDynamicBufferSize);

        TEST_CASE(valueFlowSafeFunctionParameterValues);
        TEST_CASE(valueFlowUnknownFunctionReturn);

        TEST_CASE(valueFlowPointerAliasDeref);

        TEST_CASE(valueFlowCrashIncompleteCode);

        TEST_CASE(valueFlowCrash);
        TEST_CASE(valueFlowHang);
        TEST_CASE(valueFlowCrashConstructorInitialization);

        TEST_CASE(valueFlowUnknownMixedOperators);
        TEST_CASE(valueFlowIdempotent);
    }

    static bool isNotTokValue(const ValueFlow::Value &val) {
        return !val.isTokValue();
    }

    static bool isNotLifetimeValue(const ValueFlow::Value& val) {
        return !val.isLifetimeValue();
    }

    bool testValueOfXKnown(const char code[], unsigned int linenr, int value) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                for (const ValueFlow::Value& val:tok->values()) {
                    if (val.isKnown() && val.intvalue == value)
                        return true;
                }
            }
        }

        return false;
    }

    bool testValueOfXImpossible(const char code[], unsigned int linenr, int value) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                for (const ValueFlow::Value& val:tok->values()) {
                    if (val.isImpossible() && val.intvalue == value)
                        return true;
                }
            }
        }

        return false;
    }

    bool testValueOfXInconclusive(const char code[], unsigned int linenr, int value) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                for (const ValueFlow::Value& val:tok->values()) {
                    if (val.isInconclusive() && val.intvalue == value)
                        return true;
                }
            }
        }

        return false;
    }

    bool testValueOfX(const char code[], unsigned int linenr, int value) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                for (const ValueFlow::Value &v : tok->values()) {
                    if (v.isIntValue() && !v.isImpossible() && v.intvalue == value)
                        return true;
                }
            }
        }

        return false;
    }

    bool testValueOfX(const char code[], unsigned int linenr, float value, float diff) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                for (const ValueFlow::Value &v : tok->values()) {
                    if (v.isFloatValue() && !v.isImpossible() && v.floatValue >= value - diff &&
                        v.floatValue <= value + diff)
                        return true;
                }
            }
        }

        return false;
    }

    std::string getErrorPathForX(const char code[], unsigned int linenr) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() != "x" || tok->linenr() != linenr)
                continue;

            std::ostringstream ostr;
            for (const ValueFlow::Value &v : tok->values()) {
                for (const ValueFlow::Value::ErrorPathItem &ep : v.errorPath) {
                    const Token *eptok = ep.first;
                    const std::string &msg = ep.second;
                    ostr << eptok->linenr() << ',' << msg << '\n';
                }
            }
            return ostr.str();
        }

        return "";
    }

    bool testValueOfX(const char code[], unsigned int linenr, const char value[], ValueFlow::Value::ValueType type) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                for (const ValueFlow::Value &v : tok->values()) {
                    if (v.valueType == type && Token::simpleMatch(v.tokvalue, value, strlen(value)))
                        return true;
                }
            }
        }

        return false;
    }

    bool testValueOfX(const char code[], unsigned int linenr, int value, ValueFlow::Value::ValueType type) {
        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                for (const ValueFlow::Value &v : tok->values()) {
                    if (v.valueType == type && v.intvalue == value)
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
                for (const ValueFlow::Value &v : tok->values()) {
                    if (v.isMovedValue() && v.moveKind == moveKind)
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
                for (const ValueFlow::Value &v : tok->values()) {
                    if (v.isIntValue() && v.intvalue == value && v.condition)
                        return true;
                }
            }
        }

        return false;
    }

    void bailout(const char code[]) {
        settings.debugwarnings = true;
        errout.str("");

        std::vector<std::string> files(1, "test.cpp");
        std::istringstream istr(code);
        const simplecpp::TokenList tokens1(istr, files, files[0]);

        simplecpp::TokenList tokens2(files);
        std::map<std::string, simplecpp::TokenList*> filedata;
        simplecpp::preprocess(tokens2, tokens1, files, filedata, simplecpp::DUI());

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        tokenizer.createTokens(std::move(tokens2));
        tokenizer.simplifyTokens1("");

        settings.debugwarnings = false;
    }

    std::list<ValueFlow::Value> tokenValues(const char code[], const char tokstr[], const Settings *s = nullptr) {
        Tokenizer tokenizer(s ? s : &settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");
        const Token *tok = Token::findmatch(tokenizer.tokens(), tokstr);
        return tok ? tok->values() : std::list<ValueFlow::Value>();
    }

    std::list<ValueFlow::Value> tokenValues(const char code[], const char tokstr[], ValueFlow::Value::ValueType vt, const Settings *s = nullptr) {
        std::list<ValueFlow::Value> values = tokenValues(code, tokstr, s);
        values.remove_if([&](const ValueFlow::Value& v) {
            return v.valueType != vt;
        });
        return values;
    }

    std::vector<std::string> lifetimeValues(const char code[], const char tokstr[], const Settings *s = nullptr) {
        std::vector<std::string> result;
        Tokenizer tokenizer(s ? s : &settings, this);
        std::istringstream istr(code);
        errout.str("");
        tokenizer.tokenize(istr, "test.cpp");
        const Token *tok = Token::findmatch(tokenizer.tokens(), tokstr);
        if (!tok)
            return result;
        for (const ValueFlow::Value& value:tok->values()) {
            if (!value.isLifetimeValue())
                continue;
            if (!value.tokvalue)
                continue;
            result.push_back(value.tokvalue->expressionString());
        }
        return result;
    }

    ValueFlow::Value valueOfTok(const char code[], const char tokstr[]) {
        std::list<ValueFlow::Value> values = tokenValues(code, tokstr);
        return values.size() == 1U && !values.front().isTokValue() ? values.front() : ValueFlow::Value();
    }

    void valueFlowNumber() {
        ASSERT_EQUALS(123, valueOfTok("x=123;", "123").intvalue);
        ASSERT_EQUALS_DOUBLE(192.0, valueOfTok("x=0x0.3p10;", "0x0.3p10").floatValue, 1e-5); // 3 * 16^-1 * 2^10 = 192
        ASSERT(std::fabs(valueOfTok("x=0.5;", "0.5").floatValue - 0.5f) < 0.1f);
        ASSERT_EQUALS(10, valueOfTok("enum {A=10,B=15}; x=A+0;", "+").intvalue);
        ASSERT_EQUALS(0, valueOfTok("x=false;", "false").intvalue);
        ASSERT_EQUALS(1, valueOfTok("x=true;", "true").intvalue);
        ASSERT_EQUALS(0, valueOfTok("x(NULL);", "NULL").intvalue);
        ASSERT_EQUALS((int)('a'), valueOfTok("x='a';", "'a'").intvalue);
        ASSERT_EQUALS((int)('\n'), valueOfTok("x='\\n';", "'\\n'").intvalue);
        ASSERT_EQUALS(0xFFFFFFFF00000000, valueOfTok("x=0xFFFFFFFF00000000;","0xFFFFFFFF00000000").intvalue); // #7701

        // scope
        {
            const char code[] = "namespace N { enum E {e0,e1}; }\n"
                                "void foo() { x = N::e1; }";
            ASSERT_EQUALS(1, valueOfTok(code, "::").intvalue);
        }
    }

    void valueFlowString() {
        const char *code;

        // valueFlowAfterAssign
        code  = "const char * f() {\n"
                "    static const char *x;\n"
                "    if (a) x = \"123\";\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "\"123\"", ValueFlow::Value::ValueType::TOK));

        // valueFlowSubFunction
        code  = "void dostuff(const char *x) {\n"
                "  f(x);\n"
                "}\n"
                "\n"
                "void test() { dostuff(\"abc\"); }";
        ASSERT_EQUALS(true, testValueOfX(code, 2, "\"abc\"", ValueFlow::Value::ValueType::TOK));
    }

    void valueFlowPointerAlias() {
        const char *code;
        std::list<ValueFlow::Value> values;

        code  = "const char * f() {\n"
                "    static const char *x;\n"
                "    static char ret[10];\n"
                "    if (a) x = &ret[0];\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5, "& ret [ 0 ]", ValueFlow::Value::ValueType::TOK));

        // dead pointer
        code  = "void f() {\n"
                "  int *x;\n"
                "  if (cond) { int i; x = &i; }\n"
                "  *x = 0;\n"  // <- x can point at i
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "& i", ValueFlow::Value::ValueType::TOK));

        code  = "void f() {\n"
                "  struct X *x;\n"
                "  x = &x[1];\n"
                "}";
        values = tokenValues(code, "&");
        values.remove_if(&isNotTokValue);
        ASSERT_EQUALS(true, values.empty());

        values = tokenValues(code, "x [");
        values.remove_if(&isNotTokValue);
        ASSERT_EQUALS(true, values.empty());
    }

    void valueFlowLifetime() {
        const char *code;
        std::vector<std::string> lifetimes;

        LOAD_LIB_2(settings.library, "std.cfg");

        code  = "void f() {\n"
                "    int a = 1;\n"
                "    auto x = [&]() { return a + 1; };\n"
                "    auto b = x;\n"
                "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "a + 1", ValueFlow::Value::ValueType::LIFETIME));

        code  = "void f() {\n"
                "    int a = 1;\n"
                "    auto x = [=]() { return a + 1; };\n"
                "    auto b = x;\n"
                "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4, "a ;", ValueFlow::Value::ValueType::LIFETIME));

        code  = "void f(int v) {\n"
                "    int a = v;\n"
                "    int * p = &a;\n"
                "    auto x = [=]() { return p + 1; };\n"
                "    auto b = x;\n"
                "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 5, "a ;", ValueFlow::Value::ValueType::LIFETIME));

        code  = "void f() {\n"
                "    std::vector<int> v;\n"
                "    auto x = v.begin();\n"
                "    auto it = x;\n"
                "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "v . begin", ValueFlow::Value::ValueType::LIFETIME));

        code  = "void f() {\n"
                "    int i = 0;\n"
                "    void* x = (void*)&i;\n"
                "}\n";
        lifetimes = lifetimeValues(code, "( void * )");
        ASSERT_EQUALS(true, lifetimes.size() == 1);
        ASSERT_EQUALS(true, lifetimes.front() == "i");
    }

    void valueFlowArrayElement() {
        const char *code;

        code  = "void f() {\n"
                "    const int x[] = {43,23,12};\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "{ 43 , 23 , 12 }", ValueFlow::Value::ValueType::TOK));

        code  = "void f() {\n"
                "    const char x[] = \"abcd\";\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "\"abcd\"", ValueFlow::Value::ValueType::TOK));

        code  = "void f() {\n"
                "    char x[32] = \"abcd\";\n"
                "    return x;\n"
                "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, "\"abcd\"", ValueFlow::Value::ValueType::TOK));

        code = "void f() {\n"
               "  int a[10];\n"
               "  int *x = a;\n" // <- a value is a
               "  *x = 0;\n"     // .. => x value is a
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "a", ValueFlow::Value::ValueType::TOK));

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
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::forward<X>(x));\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::ForwardedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x).getA());\n"   // Only parts of x might be moved out
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::forward<X>(x).getA());\n" // Only parts of x might be moved out
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::ForwardedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   x.clear();\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   y=x->y;\n"
               "   z=x->z;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    z = g(std::move(x));\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    y = g(std::move(x), \n"
               "          x.size());\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    x = g(std::move(x));\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "A f(int i) {\n"
               "    X x;\n"
               "    if (i)"
               "        return g(std::move(x));\n"
               "    return h(std::move(x));\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "struct X {\n"
               "};\n"
               "struct Data {\n"
               "  template<typename Fun>\n"
               "  void foo(Fun f) {}\n"
               "};\n"
               "Data g(X value) { return Data(); }\n"
               "void f() {\n"
               "   X x;\n"
               "   g(std::move(x)).foo([=](int value) mutable {;});\n"
               "   X y=x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 11U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int x) {\n"
               "   g(std::move(x));\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, ValueFlow::Value::MoveKind::MovedVariable));
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
        ASSERT_EQUALS(10, valueOfTok("x = static_cast<int>(10);", "( 10 )").intvalue);

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

        code  = "int f() {\n"
                "    const int i = 1;\n"
                "    int x = i < 0 ? 0 : 1;\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 1));

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

        // Logical and
        code = "void f(bool b) {\n"
               "   bool x = false && b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(bool b) {\n"
               "   bool x = b && false;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(bool b) {\n"
               "   bool x = true && b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));

        code = "void f(bool b) {\n"
               "   bool x = b && true;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));

        // Logical or
        code = "void f(bool b) {\n"
               "   bool x = true || b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f(bool b) {\n"
               "   bool x = b || true;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f(bool b) {\n"
               "   bool x = false || b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(bool b) {\n"
               "   bool x = b || false;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "bool f() {\n"
               "    bool a = (4 == 3);\n"
               "    bool b = (3 == 3);\n"
               "    return a || b;\n"
               "}\n";
        values = tokenValues(code, "%oror%");
        ASSERT_EQUALS(1, values.size());
        if (!values.empty()) {
            ASSERT_EQUALS(true, values.front().isIntValue());
            ASSERT_EQUALS(true, values.front().isKnown());
            ASSERT_EQUALS(1, values.front().intvalue);
        }

        // function call => calculation
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

    void valueFlowSizeof() {
        const char *code;
        std::list<ValueFlow::Value> values;

        // array size
        code  = "void f() {\n"
                "    char a[10];"
                "    x = sizeof(*a);\n"
                "}";
        values = tokenValues(code,"( *");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.back().intvalue);

        code = "enum testEnum : uint32_t { a };\n"
               "sizeof(testEnum);";
        values = tokenValues(code,"( testEnum");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(4, values.back().intvalue);

#define CHECK3(A, B, C)                          \
        do {                                     \
        code = "void f() {\n"                    \
               "    x = sizeof(" A ");\n"        \
               "}";                              \
        values = tokenValues(code,"( " C " )");  \
        ASSERT_EQUALS(1U, values.size());        \
        ASSERT_EQUALS(B, values.back().intvalue);\
        } while(false)
#define CHECK(A, B) CHECK3(A, B, A)

        // standard types
        CHECK("void *", settings.sizeof_pointer);
        CHECK("char", 1U);
        CHECK("short", settings.sizeof_short);
        CHECK("int", settings.sizeof_int);
        CHECK("long", settings.sizeof_long);
        CHECK3("long long", settings.sizeof_long_long, "long");
        CHECK("wchar_t", settings.sizeof_wchar_t);
        CHECK("float", settings.sizeof_float);
        CHECK("double", settings.sizeof_double);
        CHECK3("long double", settings.sizeof_long_double, "double");

        // string/char literals
        CHECK("\"asdf\"", 5);
        CHECK("L\"asdf\"", 5 * settings.sizeof_wchar_t);
        CHECK("u8\"asdf\"", 5); // char8_t
        CHECK("u\"asdf\"", 5 * 2); // char16_t
        CHECK("U\"asdf\"", 5 * 4); // char32_t
        CHECK("'a'", 1U);
        CHECK("'ab'", settings.sizeof_int);
        CHECK("L'a'", settings.sizeof_wchar_t);
        CHECK("u8'a'", 1U); // char8_t
        CHECK("u'a'", 2U); // char16_t
        CHECK("U'a'", 4U); // char32_t
#undef CHECK
#undef CHECK3

        // array size
        code  = "void f() {\n"
                "    struct S *a[10];"
                "    x = sizeof(a) / sizeof(a[0]);\n"
                "}";
        values = tokenValues(code,"/");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(10, values.back().intvalue);

#define CHECK(A, B, C, D)                         \
        do {                                      \
        code = "enum " A " E " B " { E0, E1 };\n" \
               "void f() {\n"                     \
               "    x = sizeof(" C ");\n"         \
               "}";                               \
        values = tokenValues(code,"( " C " )");   \
        ASSERT_EQUALS(1U, values.size());         \
        ASSERT_EQUALS(D, values.back().intvalue); \
        } while(false)

        // enums
        CHECK("", "", "E", settings.sizeof_int);

        // typed enums
        CHECK("", ": char", "E", 1U);
        CHECK("", ": signed char", "E", 1U);
        CHECK("", ": unsigned char", "E", 1U);
        CHECK("", ": short", "E", settings.sizeof_short);
        CHECK("", ": signed short", "E", settings.sizeof_short);
        CHECK("", ": unsigned short", "E", settings.sizeof_short);
        CHECK("", ": int", "E", settings.sizeof_int);
        CHECK("", ": signed int", "E", settings.sizeof_int);
        CHECK("", ": unsigned int", "E", settings.sizeof_int);
        CHECK("", ": long", "E", settings.sizeof_long);
        CHECK("", ": signed long", "E", settings.sizeof_long);
        CHECK("", ": unsigned long", "E", settings.sizeof_long);
        CHECK("", ": long long", "E", settings.sizeof_long_long);
        CHECK("", ": signed long long", "E", settings.sizeof_long_long);
        CHECK("", ": unsigned long long", "E", settings.sizeof_long_long);
        CHECK("", ": wchar_t", "E", settings.sizeof_wchar_t);
        CHECK("", ": size_t", "E", settings.sizeof_size_t);

        // enumerators
        CHECK("", "", "E0", settings.sizeof_int);

        // typed enumerators
        CHECK("", ": char", "E0", 1U);
        CHECK("", ": signed char", "E0", 1U);
        CHECK("", ": unsigned char", "E0", 1U);
        CHECK("", ": short", "E0", settings.sizeof_short);
        CHECK("", ": signed short", "E0", settings.sizeof_short);
        CHECK("", ": unsigned short", "E0", settings.sizeof_short);
        CHECK("", ": int", "E0", settings.sizeof_int);
        CHECK("", ": signed int", "E0", settings.sizeof_int);
        CHECK("", ": unsigned int", "E0", settings.sizeof_int);
        CHECK("", ": long", "E0", settings.sizeof_long);
        CHECK("", ": signed long", "E0", settings.sizeof_long);
        CHECK("", ": unsigned long", "E0", settings.sizeof_long);
        CHECK("", ": long long", "E0", settings.sizeof_long_long);
        CHECK("", ": signed long long", "E0", settings.sizeof_long_long);
        CHECK("", ": unsigned long long", "E0", settings.sizeof_long_long);
        CHECK("", ": wchar_t", "E0", settings.sizeof_wchar_t);
        CHECK("", ": size_t", "E0", settings.sizeof_size_t);

        // class typed enumerators
        CHECK("class", ": char", "E :: E0", 1U);
        CHECK("class", ": signed char", "E :: E0", 1U);
        CHECK("class", ": unsigned char", "E :: E0", 1U);
        CHECK("class", ": short", "E :: E0", settings.sizeof_short);
        CHECK("class", ": signed short", "E :: E0", settings.sizeof_short);
        CHECK("class", ": unsigned short", "E :: E0", settings.sizeof_short);
        CHECK("class", ": int", "E :: E0", settings.sizeof_int);
        CHECK("class", ": signed int", "E :: E0", settings.sizeof_int);
        CHECK("class", ": unsigned int", "E :: E0", settings.sizeof_int);
        CHECK("class", ": long", "E :: E0", settings.sizeof_long);
        CHECK("class", ": signed long", "E :: E0", settings.sizeof_long);
        CHECK("class", ": unsigned long", "E :: E0", settings.sizeof_long);
        CHECK("class", ": long long", "E :: E0", settings.sizeof_long_long);
        CHECK("class", ": signed long long", "E :: E0", settings.sizeof_long_long);
        CHECK("class", ": unsigned long long", "E :: E0", settings.sizeof_long_long);
        CHECK("class", ": wchar_t", "E :: E0", settings.sizeof_wchar_t);
        CHECK("class", ": size_t", "E :: E0", settings.sizeof_size_t);
#undef CHECK

#define CHECK(A, B)                                   \
        do {                                          \
        code = "enum E " A " { E0, E1 };\n"           \
               "void f() {\n"                         \
               "    E arrE[] = { E0, E1 };\n"         \
               "    x = sizeof(arrE);\n"              \
               "}";                                   \
        values = tokenValues(code,"( arrE )");        \
        ASSERT_EQUALS(1U, values.size());             \
        ASSERT_EQUALS(B * 2U, values.back().intvalue);\
        } while(false)

        // enum array
        CHECK("", settings.sizeof_int);

        // typed enum array
        CHECK(": char", 1U);
        CHECK(": signed char", 1U);
        CHECK(": unsigned char", 1U);
        CHECK(": short", settings.sizeof_short);
        CHECK(": signed short", settings.sizeof_short);
        CHECK(": unsigned short", settings.sizeof_short);
        CHECK(": int", settings.sizeof_int);
        CHECK(": signed int", settings.sizeof_int);
        CHECK(": unsigned int", settings.sizeof_int);
        CHECK(": long", settings.sizeof_long);
        CHECK(": signed long", settings.sizeof_long);
        CHECK(": unsigned long", settings.sizeof_long);
        CHECK(": long long", settings.sizeof_long_long);
        CHECK(": signed long long", settings.sizeof_long_long);
        CHECK(": unsigned long long", settings.sizeof_long_long);
        CHECK(": wchar_t", settings.sizeof_wchar_t);
        CHECK(": size_t", settings.sizeof_size_t);
#undef CHECK

#define CHECK(A, B)                                   \
        do {                                          \
        code = "enum class E " A " { E0, E1 };\n"     \
               "void f() {\n"                         \
               "    E arrE[] = { E::E0, E::E1 };\n"   \
               "    x = sizeof(arrE);\n"              \
               "}";                                   \
        values = tokenValues(code,"( arrE )");        \
        ASSERT_EQUALS(1U, values.size());             \
        ASSERT_EQUALS(B * 2U, values.back().intvalue);\
        } while(false)

        // enum array
        CHECK("", settings.sizeof_int);

        // typed enum array
        CHECK(": char", 1U);
        CHECK(": signed char", 1U);
        CHECK(": unsigned char", 1U);
        CHECK(": short", settings.sizeof_short);
        CHECK(": signed short", settings.sizeof_short);
        CHECK(": unsigned short", settings.sizeof_short);
        CHECK(": int", settings.sizeof_int);
        CHECK(": signed int", settings.sizeof_int);
        CHECK(": unsigned int", settings.sizeof_int);
        CHECK(": long", settings.sizeof_long);
        CHECK(": signed long", settings.sizeof_long);
        CHECK(": unsigned long", settings.sizeof_long);
        CHECK(": long long", settings.sizeof_long_long);
        CHECK(": signed long long", settings.sizeof_long_long);
        CHECK(": unsigned long long", settings.sizeof_long_long);
        CHECK(": wchar_t", settings.sizeof_wchar_t);
        CHECK(": size_t", settings.sizeof_size_t);
#undef CHECK

        code = "uint16_t arr[10];\n"
               "x = sizeof(arr);";
        values = tokenValues(code,"( arr )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(10 * sizeof(std::uint16_t), values.back().intvalue);
    }

    void valueFlowErrorPath() {
        const char *code;

        code = "void f() {\n"
               "  int x = 53;\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS("2,Assignment 'x=53', assigned value is 53\n",
                      getErrorPathForX(code, 3U));

        code = "void f(int y) {\n"
               "  int x = y;\n"
               "  a = x;\n"
               "  y += 12;\n"
               "  if (y == 32) {}"
               "}\n";
        ASSERT_EQUALS("5,Assuming that condition 'y==32' is not redundant\n"
                      "4,Compound assignment '+=', assigned value is 20\n"
                      "2,Assignment 'x=y', assigned value is 20\n",
                      getErrorPathForX(code, 3U));

        code = "void f1(int x) {\n"
               "  a = x;\n"
               "}\n"
               "void f2() {\n"
               "  int x = 3;\n"
               "  f1(x+1);\n"
               "}\n";
        ASSERT_EQUALS("5,Assignment 'x=3', assigned value is 3\n"
                      "6,Calling function 'f1', 1st argument 'x+1' value is 4\n",
                      getErrorPathForX(code, 2U));

        code = "void f(int a) {\n"
               "  int x;\n"
               "  for (x = a; x < 50; x++) {}\n"
               "  b = x;\n"
               "}\n";
        ASSERT_EQUALS("3,After for loop, x has value 50\n",
                      getErrorPathForX(code, 4U));
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

        // after loop
        code = "void f(struct X *x) {\n"
               "  do {\n"
               "    if (!x)\n"
               "      break;\n"
               "  } while (x->a);\n"
               "  if (x) {}\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
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

        // compound assignment += , -= , ...
        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x += 2;\n"
               "   if (x == 4);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 2));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x -= 2;\n"
               "   if (x == 4);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 6));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x *= 2;\n"
               "   if (x == 42);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 21));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x /= 5;\n"
               "   if (x == 42);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 210));

        // bailout: assignment
        bailout("void f(int x) {\n"
                "    x = y;\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:2]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable y\n",
            errout.str());
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
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:2]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable y\n",
            errout.str());

        bailout("int f(int x) {\n"
                "  int r = x ? 1 / x : 0;\n"
                "  if (x == 0) {}\n"
                "}");

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
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:2]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable b\n",
            errout.str());

        code = "void f(int x, bool abc) {\n"
               "  a = x;\n"
               "  if (abc) { x = 1; }\n" // <- condition must be false if x is 7 in next line
               "  if (x == 7) { }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 7));

        code = "void f(int x, bool abc) {\n"
               "  a = x;\n"
               "  if (abc) { x = 7; }\n" // <- condition is probably true
               "  if (x == 7) { }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 7));
    }

    void valueFlowBeforeConditionGlobalVariables() {
        const char *code;

        // handle global variables
        code = "int x;\n"
               "void f() {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3,123));

        // bailout when there is function call
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
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable a\n",
            errout.str());

        bailout("void f(int x, int y) {\n"
                "    switch (y) {\n"
                "    case 1: a=x; return 1;\n"
                "    case 2: if (x==5) {} break;\n"
                "    };\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable a\n",
            errout.str());
    }

    void valueFlowBeforeConditionMacro() {
        // bailout: condition is a expanded macro
        bailout("#define M  if (x==123) {}\n"
                "void f(int x) {\n"
                "    a = x;\n"
                "    M;\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable a\n"
            "[test.cpp:4]: (debug) valueflow.cpp:1260:(valueFlow) bailout: variable 'x', condition is defined in macro\n",
            errout.str());

        bailout("#define FREE(obj) ((obj) ? (free((char *) (obj)), (obj) = 0) : 0)\n" // #8349
                "void f(int *x) {\n"
                "    a = x;\n"
                "    FREE(x);\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable a\n"
            "[test.cpp:4]: (debug) valueflow.cpp:1260:(valueFlow) bailout: variable 'x', condition is defined in macro\n",
            errout.str());
    }

    void valueFlowBeforeConditionGoto() {
        // bailout: goto label (TODO: handle gotos more intelligently)
        bailout("void f(int x) {\n"
                "    if (x == 123) { goto out; }\n"
                "    a=x;\n"   // <- x is not 123
                "out:"
                "    if (x==123){}\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueflow.cpp::valueFlowTerminatingCondition bailout: Skipping function due to incomplete variable a\n",
            errout.str());

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
    }

    void valueFlowBeforeConditionForward() {
        const char* code;

        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    if (a == 123) {}\n"
               "    int b = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    if (a != 123) {}\n"
               "    int b = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));
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
               "    const int x(321);\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 321));

        code = "void f() {\n"
               "    int x = 9;\n"
               "    --x;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 9));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 8));
        ASSERT_EQUALS("2,Assignment 'x=9', assigned value is 9\n"
                      "3,x is decremented', new value is 8\n",
                      getErrorPathForX(code, 4U));

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

        // truncation
        code = "int f() {\n"
               "  int x = 1.5;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "int f() {\n"
               "  unsigned char x = 0x123;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0x23));

        code = "int f() {\n"
               "  signed char x = 0xfe;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, -2));

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

        code = "void f() {\n"
               "    int x = 0;\n"
               "    dostuff([&]() {\n"
               "        if (x > 0) {}\n"
               "        x++;\n"
               "    });\n"
               "    dosomething(q);\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "int f() {\n"
               "    int x = 1;\n"
               "    dostuff([&]() {\n"
               "        x = y;\n"
               "    });\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 1));

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
        ASSERT_EQUALS(false, testValueOfX(code, 4U, "\"\"", ValueFlow::Value::ValueType::TOK));

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
        ASSERT_EQUALS(true, testValueOfX(code, 6U, "\"\"", ValueFlow::Value::ValueType::TOK));
        ASSERT_EQUALS(false, testValueOfX(code, 7U, "\"\"", ValueFlow::Value::ValueType::TOK));
        ASSERT_EQUALS(false, testValueOfX(code, 8U, "\"\"", ValueFlow::Value::ValueType::TOK));
        ASSERT_EQUALS(false, testValueOfX(code, 9U, "\"\"", ValueFlow::Value::ValueType::TOK));

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
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 5U, 123));

        code = "void f(int a) {\n"
               "    int x = 123;\n"
               "    if (a > 1)\n"
               "        ++x;\n"
               "    else\n"
               "        ++x;\n"
               "    return 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

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

        code = "void f(int i) {\n"
               "    bool x = false;\n"
               "    if (i == 0) { x = true; }\n"
               "    else if (x && i == 1) {}\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "void f(int i) {\n"
               "    bool x = false;\n"
               "    while(i > 0) {\n"
               "        i++;\n"
               "        if (i == 0) { x = true; }\n"
               "        else if (x && i == 1) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 0));

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
               "  int x = 111;\n"
               "  bool found = false;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      found = true;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  if (found)\n"
               "    a = x;\n" // <- x can't be 111
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 12U, 111)); // x can not be 111 at line 9

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
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0));       // x can be 0 at line 9
        ASSERT_EQUALS(false, testValueOfXKnown(code, 9U, 0)); // x can't be known at line 9

        code = "void f(const int *buf) {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      ;\n" // <- no break
               "    } else {\n"
               "      x = 1;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 11U, 0)); // x can't be 0 at line 11

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

        code = "struct Fred {\n"
               "    static void Create(std::unique_ptr<Wilma> wilma);\n"
               "    Fred(std::unique_ptr<Wilma> wilma);\n"
               "    std::unique_ptr<Wilma> mWilma;\n"
               "};\n"
               "void Fred::Create(std::unique_ptr<Wilma> wilma) {\n"
               "    auto fred = std::make_shared<Fred>(std::move(wilma));\n"
               "    fred->mWilma.reset();\n"
               "}\n"
               "Fred::Fred(std::unique_ptr<Wilma> wilma)\n"
               "    : mWilma(std::move(wilma)) {}\n";
        ASSERT_EQUALS(0, tokenValues(code, "mWilma (").size());

        code = "void g(unknown*);\n"
               "int f() {\n"
               "    int a = 1;\n"
               "    unknown c[] = {{&a}};\n"
               "    g(c);\n"
               "    int x = a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, 1));
        ASSERT_EQUALS(true, testValueOfXInconclusive(code, 7U, 1));

        code = "void g(unknown&);\n"
               "int f() {\n"
               "    int a = 1;\n"
               "    unknown c{&a};\n"
               "    g(c);\n"
               "    int x = a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, 1));
        ASSERT_EQUALS(true, testValueOfXInconclusive(code, 7U, 1));
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

        code = "void f(int x) {\n"
               "    if (x > 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 122));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        // ----

        code = "void f(int x) {\n"
               "    if (123 < x) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (123 > x) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 122));
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

        code = "void f(int x) {\n"
               "    if (x != 123) { throw ""; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x != 123) { }\n"
               "    else { throw ""; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));
        code = "void f(int x) {\n"
               "    if (x == 123) { }\n"
               "    else { throw ""; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));


        code = "void f(int x) {\n"
               "    if (x < 123) { }\n"
               "    else { a = x; }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) { }\n"
               "    else { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 122));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        code = "void f(int x) {\n"
               "    if (x > 123) { }\n"
               "    else { a = x; }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x > 123) { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x > 123) { }\n"
               "    else { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) { return; }\n"
               "    else { return; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

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
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 2));
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
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
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

        // aliased variable
        code = "void f() {\n"
               "  int x = 1;\n"
               "  int *data = &x;\n"
               "  if (!x) {\n"
               "    calc(data);\n"
               "    a = x;\n"  // <- x might be changed by calc
               "  }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 0));

        code = "int* g();\n"
               "int f() {\n"
               "    int * x;\n"
               "    x = g();\n"
               "    if (x) { printf(\"\"); }\n"
               "    return *x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 0));

        // volatile variable
        code = "void foo(const volatile int &x) {\n"
               "    if (x==1) {\n"
               "        return x;\n"
               "    }"
               "}";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 1));

        code = "void foo(const std::atomic<int> &x) {\n"
               "    if (x==2) {\n"
               "        return x;\n"
               "    }"
               "}";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 2));

        code = "int f(int i, int j) {\n"
               "    if (i == 0) {\n"
               "        if (j < 0)\n"
               "            return 0;\n"
               "        i = j+1;\n"
               "    }\n"
               "    int x = i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 0));

        code = "int f(int i, int j) {\n"
               "    if (i == 0) {\n"
               "        if (j < 0)\n"
               "            return 0;\n"
               "        if (j < 0)\n"
               "            i = j+1;\n"
               "    }\n"
               "    int x = i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0));

        code = "void g(long& a);\n"
               "void f(long a) {\n"
               "    if (a == 0)\n"
               "        return;\n"
               "    if (a > 1)\n"
               "         g(a);\n"
               "    int x = a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 8U, 0));

        code = "int foo(int n) {\n"
               "    if( n>= 8 ) {\n"
               "        while(true) {\n"
               "            n -= 8;\n"
               "            if( n < 8 )\n"
               "                break;\n"
               "        }\n"
               "        int x = n == 0;\n"
               "        return x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 9U, 0));

        code = "bool c();\n"
               "long f() {\n"
               "    bool stop = false;\n"
               "    while (!stop) {\n"
               "        if (c())\n"
               "            stop = true;\n"
               "        break;\n"
               "    }\n"
               "    int x = !stop;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 10U, 1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 10U, 0));

        code = "int f(int a, int b) {\n"
               "  if (!a && !b)\n"
               "    return;\n"
               "  if ((!a && b) || (a && !b))\n"
               "    return;\n"
               "  int x = a;\n" // <- a is _not_ 0
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));
    }

    void valueFlowAfterConditionExpr() {
        const char* code;

        code = "void f(int* p) {\n"
               "    if (p[0] == 123) {\n"
               "        int x = p[0];\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int y) {\n"
               "    if (y+1 == 123) {\n"
               "        int x = y+1;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int y) {\n"
               "    if (y+1 == 123) {\n"
               "        int x = y+2;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 124));

        code = "void f(int y, int z) {\n"
               "    if (y+z == 123) {\n"
               "        int x = y+z;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int y, int z) {\n"
               "    if (y+z == 123) {\n"
               "        y++;\n"
               "        int x = y+z;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 123));

        code = "void f(int y) {\n"
               "    if (y++ == 123) {\n"
               "        int x = y++;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 125));

        code = "struct A {\n"
               "    bool g() const;\n"
               "};\n"
               "void f(A a) {\n"
               "    if (a.g()) {\n"
               "        bool x = a.g();\n"
               "        bool a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, 0));

        code = "struct A {\n"
               "    bool g() const;\n"
               "};\n"
               "void f(A a) {\n"
               "    if (a.g()) {\n"
               "        bool x = !a.g();\n"
               "        bool a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 0));

        code = "struct A {\n"
               "    bool g() const;\n"
               "};\n"
               "void f(A a) {\n"
               "    if (!a.g()) {\n"
               "        bool x = a.g();\n"
               "        bool a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 0));

        code = "void f(std::vector<int> v) {\n"
               "    if (v.size() == 3) {\n"
               "        if (v.size() == 1) {\n"
               "            int x = 1;\n"
               "            int a = x;\n"
               "        }\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));
    }

    void valueFlowAfterConditionSeveralNot() {
        const char *code;

        code = "int f(int x, int y) {\n"
               "    if (x!=0) {}\n"
               "      return y/x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "int f(int x, int y) {\n"
               "    if (!!(x != 0)) {\n"
               "      return y/x;\n"
               "}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "int f(int x, int y) {\n"
               "    if (!!!(x != 0)) {\n"
               "      return y/x;\n"
               "}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "int f(int x, int y) {\n"
               "    if (!!!!(x != 0)) {\n"
               "      return y/x;\n"
               "}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));
    }

    void valueFlowForwardCompoundAssign() {
        const char *code;

        code = "void f() {\n"
               "    int x = 123;\n"
               "    x += 43;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 166));
        ASSERT_EQUALS("2,Assignment 'x=123', assigned value is 123\n"
                      "3,Compound assignment '+=', assigned value is 166\n",
                      getErrorPathForX(code, 4U));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    x /= 0;\n" // don't crash when evaluating x/=0
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        code = "void f() {\n"
               "    float x = 123.45;\n"
               "    x += 67;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123.45F + 67, 0.01F));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    x >>= 1;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 61));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    x <<= 1;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 246));
    }

    void valueFlowForwardCorrelatedVariables() {
        const char *code;

        code = "void f(int x = 0) {\n"
               "  bool zero(x==0);\n"
               "  if (zero) a = x;\n"  // <- x is 0
               "  else b = x;\n"  // <- x is not 0
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
    }

    void valueFlowForwardModifiedVariables() {
        const char *code;

        code = "void f(bool b) {\n"
               "  int x = 0;\n"
               "  if (b) x = 1;\n"
               "  else b = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(int i) {\n"
               "    int x = 0;\n"
               "    if (i == 0) \n"
               "        x = 1;\n"
               "    else if (!x && i == 1) \n"
               "        int b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 0));
    }

    void valueFlowForwardFunction() {
        const char *code;

        code = "class C {\n"
               "public:\n"
               "  C(int &i);\n" // non-const argument => might be changed
               "};\n"
               "int f() {\n"
               "  int x=1;\n"
               "  C c(x);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 1));

        code = "class C {\n"
               "public:\n"
               "  C(const int &i);\n" // const argument => is not changed
               "};\n"
               "int f() {\n"
               "  int x=1;\n"
               "  C c(x);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 8U, 1));

        code = "int f(int *);\n"
               "int g() {\n"
               "  const int a = 1;\n"
               "  int x = 11;\n"
               "  c = (a && f(&x));\n"
               "  if (x == 42) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 11));
    }

    void valueFlowForwardTernary() {
        const char *code;

        code = "int f() {\n"
               "  int x=5;\n"
               "  a = b ? init1(&x) : init2(&x);\n"
               "  return 1 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 5));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 5));

        code = "int f(int *p) {\n" // #9008 - gcc ternary ?:
               "  if (p) return;\n"
               "  x = *p ? : 1;\n" // <- no explicit expr0
               "}";
        testValueOfX(code, 1U, 0); // do not crash

        code = "void f(int a) {\n" // #8784
               "    int x = 13;\n"
               "    if (a == 1) x = 26;\n"
               "    return a == 1 ? x : 0;\n"  // <- x is 26
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 13));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 26));
    }

    void valueFlowForwardLambda() {
        const char *code;

        code = "void f() {\n"
               "  int x=1;\n"
               "  auto f = [&](){ a=x; }\n"  // x is not 1
               "  x = 2;\n"
               "  f();\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, 2));

        code = "void f() {\n"
               "  int x=3;\n"
               "  auto f = [&](){ a=x; }\n"  // todo: x is 3
               "  f();\n"
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, 3));
    }

    void valueFlowForwardTryCatch() {
        const char *code;

        code = "void g1();\n"
               "void g2();\n"
               "void f()\n {"
               "    bool x = false;\n"
               "    try {\n"
               "        g1();\n"
               "        x = true;\n"
               "        g2();\n"
               "    }\n"
               "    catch (...) {\n"
               "        if (x) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 11U, 1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 11U, 1));

        code = "void g1();\n"
               "void g2();\n"
               "void f()\n {"
               "    bool x = true;\n"
               "    try {\n"
               "        g1();\n"
               "        g2();\n"
               "    }\n"
               "    catch (...) {\n"
               "        if (x) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 10U, 1));
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

    void valueFlowForwardInconclusiveImpossible() {
        const char *code;

        code = "void foo() {\n"
               "    bool valid = f1();\n"
               "    if (!valid) return;\n"
               "    std::tie(endVal, valid) = f2();\n"
               "    bool x = !valid;"
               "    bool b = x;" // <- not always true
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 1));
    }

    void valueFlowRightShift() {
        const char *code;
        /* Set some temporary fixed values to simplify testing */
        const Settings settingsTmp = settings;
        settings.int_bit = 32;
        settings.long_bit = 64;
        settings.long_long_bit = MathLib::bigint_bits * 2;

        code = "int f(int a) {\n"
               "  int x = (a & 0xff) >> 16;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0));

        code = "int f(unsigned int a) {\n"
               "  int x = (a % 123) >> 16;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0));

        code = "int f(int y) {\n"
               "  int x = (y & 0xFFFFFFF) >> 31;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0));

        code = "int f(int y) {\n"
               "  int x = (y & 0xFFFFFFF) >> 32;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0));

        code = "int f(short y) {\n"
               "  int x = (y & 0xFFFFFF) >> 31;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0));

        code = "int f(short y) {\n"
               "  int x = (y & 0xFFFFFF) >> 32;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0));

        code = "int f(long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 63;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0));

        code = "int f(long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 64;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 63;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 64;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 121;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 128;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0));

        settings = settingsTmp;
    }

    void valueFlowFwdAnalysis() {
        const char *code;
        std::list<ValueFlow::Value> values;

        code = "void f() {\n"
               "  struct Foo foo;\n"
               "  foo.x = 1;\n"
               "  x = 0 + foo.x;\n" // <- foo.x is 1
               "}";
        values = tokenValues(code, "+");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(true, values.front().isKnown());
        ASSERT_EQUALS(true, values.front().isIntValue());
        ASSERT_EQUALS(1, values.front().intvalue);

        code = "void f() {\n"
               "  S s;\n"
               "  s.x = 1;\n"
               "  int y = 10;\n"
               "  while (s.x < y)\n" // s.x does not have known value
               "    s.x++;\n"
               "}";
        values = tokenValues(code, "<");
        ASSERT_EQUALS(1, values.size());
        ASSERT(values.front().isPossible());
        ASSERT_EQUALS(true, values.front().intvalue);

        code = "void f() {\n"
               "  S s;\n"
               "  s.x = 37;\n"
               "  int y = 10;\n"
               "  while (s.x < y)\n" // s.x has a known value
               "    y--;\n"
               "}";
        values = tokenValues(code, ". x <");
        ASSERT(values.size() == 1 &&
               values.front().isKnown() &&
               values.front().isIntValue() &&
               values.front().intvalue == 37);

        code = "void f() {\n"
               "  Hints hints;\n"
               "  hints.x = 1;\n"
               "  if (foo)\n"
               "    hints.x = 2;\n"
               "  x = 0 + foo.x;\n" // <- foo.x is possible 1, possible 2
               "}";
        values = tokenValues(code, "+");
        TODO_ASSERT_EQUALS(2U, 0U, values.size()); // should be 2

        // FP: Condition '*b>0' is always true
        code = "bool dostuff(const char *x, const char *y);\n"
               "void fun(char *s, int *b) {\n"
               "  for (int i = 0; i < 42; ++i) {\n"
               "    if (dostuff(s, \"1\")) {\n"
               "      *b = 1;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  if (*b > 0) {\n" // *b does not have known value
               "  }\n"
               "}";
        values = tokenValues(code, ">");
        ASSERT_EQUALS(1, values.size());
        ASSERT(values.front().isPossible());
        ASSERT_EQUALS(true, values.front().intvalue);

        code = "void foo() {\n"
               "    struct ISO_PVD_s pvd;\n"
               "    pvd.descr_type = 0xff;\n"
               "    do {\n"
               "        if (pvd.descr_type == 0xff) {}\n"
               "        dostuff(&pvd);\n"
               "    } while (condition)\n"
               "}";
        values = tokenValues(code, "==");
        ASSERT_EQUALS(true, values.empty());

        // for loops
        code = "struct S { int x; };\n" // #9036
               "void foo(struct S s) {\n"
               "    for (s.x = 0; s.x < 127; s.x++) {}\n"
               "}";
        values = tokenValues(code, "<"); // TODO: comparison can be true or false
        ASSERT_EQUALS(true, values.empty());
    }

    void valueFlowSwitchVariable() {
        const char *code;
        code = "void f(int x) {\n"
               "    a = x - 1;\n"  // <- x can be 14
               "    switch (x) {\n"
               "    case 14: a=x+2; break;\n"  // <- x is 14
               "    };\n"
               "    a = x;\n"  // <- x can be 14
               "}";
        ASSERT_EQUALS(true, testConditionalValueOfX(code, 2U, 14));
        TODO_ASSERT_EQUALS(true, false, testConditionalValueOfX(code, 4U, 14));
        TODO_ASSERT_EQUALS(true, false, testConditionalValueOfX(code, 6U, 14));

        ValueFlow::Value value1 = valueOfTok(code, "-");
        ASSERT_EQUALS(13, value1.intvalue);
        ASSERT(!value1.isKnown());

        ValueFlow::Value value2 = valueOfTok(code, "+");
        TODO_ASSERT_EQUALS(16, 0, value2.intvalue);
        TODO_ASSERT_EQUALS(true, false, value2.isKnown());
    }

    void valueFlowForLoop() {
        const char *code;
        ValueFlow::Value value;

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
               "    for (int x = 0; x < 5; x += 2)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 4));

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
               "    abort();\n"
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

        // #7886 - valueFlowForLoop must be called after valueFlowAfterAssign
        code = "void f() {\n"
               "  int sz = 4;\n"
               "  int x,y;\n"
               "  for(x=0,y=0; x < sz && y < 10; x++)\n"
               "    a = x;\n" // <- max value is 3
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 3));

        code = "void f() {\n"
               "    int x;\n"
               "    for (x = 0; x < 10; x++)\n"
               "        x;\n"
               "}";
        std::list<ValueFlow::Value> values = tokenValues(code, "x <");
        ASSERT(std::none_of(values.begin(), values.end(), std::mem_fn(&ValueFlow::Value::isUninitValue)));

        // #9637
        code = "void f() {\n"
               "    unsigned int x = 0;\n"
               "    for (x = 0; x < 2; x++) {}\n"
               "}\n";
        value = valueOfTok(code, "x <");
        ASSERT(value.isPossible());
        ASSERT_EQUALS(value.intvalue, 0);

        code = "void f() {\n"
               "    unsigned int x = 0;\n"
               "    for (;x < 2; x++) {}\n"
               "}\n";
        value = valueOfTok(code, "x <");
        ASSERT(value.isPossible());
        ASSERT_EQUALS(0, value.intvalue);

        code = "void f() {\n"
               "    unsigned int x = 1;\n"
               "    for (x = 0; x < 2; x++) {}\n"
               "}\n";
        value = valueOfTok(code, "x <");
        ASSERT(value.isPossible());
        ASSERT_EQUALS(0, value.intvalue);
    }

    void valueFlowSubFunction() {
        const char *code;

        code = "int f(int size) {\n"
               "    int x = 0;\n"
               "    if(size>16) {\n"
               "        x = size;\n"
               "        int a = x;\n"
               "    }\n"
               "    return x;\n"
               "}\n"
               "void g(){\n"
               "    f(42);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 17));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 42));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 17));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 42));

        code = "void g(int, int) {}\n"
               "void f(int x, int y) {\n"
               "    g(x, y);\n"
               "}\n"
               "void h() {\n"
               "    f(0, 0);\n"
               "    f(1, 1);\n"
               "    f(2, 2);\n"
               "    f(3, 3);\n"
               "    f(4, 4);\n"
               "    f(5, 5);\n"
               "    f(6, 6);\n"
               "    f(7, 7);\n"
               "    f(8, 8);\n"
               "    f(9, 9);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 2));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 3));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 4));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 5));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 6));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 7));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 8));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));

        code = "int f(int i, int j) {\n"
               "    if (i == j) {\n"
               "        int x = i;\n"
               "        return x;\n"
               "    }\n"
               "    return 0;\n"
               "}\n"
               "int g(int x) {\n"
               "    f(x, -1);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, -1));
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
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

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
        for (const ValueFlow::Value &v : tokenValues(code, str)) {
            if (v.isKnown())
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

        code = "void f(int c) {\n"
               "  int x = 0;\n"
               "  if (c) {} else { x++; }\n"
               "  return x + 2;\n" // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "+"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(&x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(0 ? ptr : &x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(unknown ? ptr : &x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

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

        code = "int x;\n"
               "void f() {\n"
               "  x = 4;\n"
               "  while (1) {\n"
               "    a = x+2;\n"
               "    dostuff();\n"
               "  }\n"
               "}";
        ASSERT(isNotKnownValues(code, "+"));

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

        code = "void f() {\n" // #8356
               "  bool b = false;\n"
               "  for(int x = 3; !b && x < 10; x++) {\n" // <- b has known value
               "    for(int y = 4; !b && y < 20; y++) {}\n"
               "  }\n"
               "}";
        value = valueOfTok(code, "!");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

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
               "  return x + 1;\n" // <- known value
               "}\n";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

        code = "void f() {\n"
               "  int x = 0;\n"
               "a:\n"
               "  a = x + 1;\n" // <- possible value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isPossible());

        // in conditional code
        code = "void f(int x) {\n"
               "  if (!x) {\n"
               "    a = x+1;\n" // <- known value
               "  }\n"
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

        code = "void f(int x) {\n"
               "  if (a && 4==x && y) {\n"
               "    a = x+12;\n" // <- known value
               "  }\n"
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(16, value.intvalue);
        ASSERT(value.isKnown());

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

        // template parameters are not known
        code = "template <int X> void f() { a = X; }\n"
               "f<1>();";
        value = valueOfTok(code, "1");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT_EQUALS(false, value.isKnown());
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

    void valueFlowGlobalConstVar() {
        const char* code;

        code = "const int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 321));

        code = "void f(const int x = 1) {\n"
               "    int a = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 2U, 1));

        code = "volatile const int x = 42;\n"
               "void f(){ int a = x; }\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 2U, 42));

        code = "static const int x = 42;\n"
               "void f(){ int a = x; }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 42));
    }

    void valueFlowGlobalStaticVar() {
        const char *code;

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { x=a; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { p = &x; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { x++; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { foo(x); }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 1;\n" // compound assignment
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { x += b; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));
    }

    void valueFlowInlineAssembly() {
        const char* code = "void f() {\n"
                           "    int x = 42;\n"
                           "    asm(\"\");\n"
                           "    a = x;\n"
                           "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 42));
    }

    void valueFlowSameExpression() {
        const char* code;

        code = "void f(int a) {\n"
               "    bool x = a == a;\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f(int a) {\n"
               "    bool x = a != a;\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(int a) {\n"
               "    int x = a - a;\n"
               "    int b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(float a) {\n"
               "    bool x = a == a;\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));
    }

    void valueFlowUninit() {
        const char* code;
        std::list<ValueFlow::Value> values;

        code = "void f() {\n"
               "    int x;\n"
               "    switch (x) {}\n"
               "}";
        values = tokenValues(code, "x )");
        ASSERT_EQUALS(true, values.size()==1U && values.front().isUninitValue());

        code = "void f() {\n"
               "    const C *c;\n"
               "    if (c->x() == 4) {}\n"
               "}";
        values = tokenValues(code, "c .");
        ASSERT_EQUALS(true, values.size()==1U && values.front().isUninitValue());

        code = "void f() {\n"
               "    C *c;\n"
               "    if (c->x() == 4) {}\n"
               "}";
        values = tokenValues(code, "c .");
        ASSERT_EQUALS(true, values.size()==1U && values.front().isUninitValue());

        code = "void f() {\n"
               "    int **x;\n"
               "    y += 10;\n"
               "    x = dostuff(sizeof(*x)*y);\n"
               "}";
        ASSERT_EQUALS(0U, tokenValues(code, "x )").size());

        // #8036
        code = "void foo() {\n"
               "    int x;\n"
               "    f(x=3), return x+3;\n"
               "}";
        values = tokenValues(code, "x +");
        ASSERT_EQUALS(true, values.empty());
        // ASSERT_EQUALS(1U, values.size());
        // ASSERT(values.front().isIntValue());
        // ASSERT_EQUALS(3, values.front().intvalue);

        // #8195
        code = "void foo(std::istream &is) {\n"
               "  int x;\n"
               "  if (is >> x) {\n"
               "    a = x;\n"
               "  }\n"
               "}";
        values = tokenValues(code, "x ; }");
        ASSERT_EQUALS(true, values.empty());

        // return (#8173)
        code = "int repeat() {\n"
               "  const char *n;\n"
               "  return((n=42) && *n == 'A');\n"
               "}";
        values = tokenValues(code, "n ==");
        ASSERT_EQUALS(true, values.empty());

        // #8233
        code = "void foo() {\n"
               "  int x;\n"
               "  int y = 1;\n"
               "  if (y>1)\n"
               "    x = 1;\n"
               "  else\n"
               "    x = 1;\n"
               "  if (x>1) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 8U, 1));

        // #8348 - noreturn else
        code = "int test_input_int(int a, int b) {\n"
               "    int x;\n"
               "    if (a == 1)\n"
               "        x = b;\n"
               "    else\n"
               "        abort();\n"
               "    a = x + 1;\n"
               "}\n";
        values = tokenValues(code, "x +");
        ASSERT_EQUALS(true, values.empty());

        // #8494 - overloaded operator &
        code = "void f() {\n"
               "    int x;\n"
               "    a & x;\n"
               "}";
        values = tokenValues(code, "x ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  if (e)\n"
               "    goto;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  if (e)\n"
               "    return;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  if (e)\n"
               "    exit();\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               " else if (e)\n"
               "   c = 0;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        TODO_ASSERT_EQUALS(true, false, values.size() == 2);
        // ASSERT_EQUALS(true, values.front().isUninitValue() || values.back().isUninitValue());
        // ASSERT_EQUALS(true, values.front().isPossible() || values.back().isPossible());
        // ASSERT_EQUALS(true, values.front().intvalue == 0 || values.back().intvalue == 0);

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               " else if (!d)\n"
               "   c = 0;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.size() == 1);
        // TODO: Value should be known
        ASSERT_EQUALS(true, values.back().isPossible());
        ASSERT_EQUALS(true, values.back().intvalue == 0);

        code = "void f() {\n" // sqlite
               "  int szHdr;\n"
               "  idx = (A<0x80) ? (szHdr = 0) : dostuff(A, (int *)&(szHdr));\n"
               "  d = szHdr;\n" // szHdr can be 0.
               "}";
        values = tokenValues(code, "szHdr ; }");
        TODO_ASSERT_EQUALS(1, 0, values.size());
        if (values.size() == 1) {
            ASSERT_EQUALS(false, values.front().isUninitValue());
        }

        code = "void f () {\n"
               "  int szHdr;\n"
               "  idx = ((aKey<0x80) ? ((szHdr)=aKey), 1 : sqlite3GetVarint32(&(szHdr)));\n"
               "  d = szHdr;\n"
               "}";
        values = tokenValues(code, "szHdr ; }");
        ASSERT_EQUALS(0, values.size());

        // #9933
        code = "struct MyStruct { size_t value; }\n"
               "\n"
               "void foo() {\n"
               "    MyStruct x;\n"
               "    fread(((char *)&x) + 0, sizeof(x), f);\n"
               "    if (x.value < 432) {}\n"
               "}";
        values = tokenValues(code, "x . value");
        ASSERT_EQUALS(0, values.size());
    }

    void valueFlowTerminatingCond() {
        const char* code;

        // opposite condition
        code = "void f(int i, int j) {\n"
               "    if (i == j) return;\n"
               "    if(i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(int i, int j) {\n"
               "    if (i == j) return;\n"
               "    i++;\n"
               "    if (i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(false, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(int i, int j, bool a) {\n"
               "    if (a) {\n"
               "        if (i == j) return;\n"
               "    }\n"
               "    if (i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 1);
        ASSERT_EQUALS(false, valueOfTok(code, "!=").isKnown());

        code = "void f(int i, int j, bool a) {\n"
               "    if (i != j) {}\n"
               "    if (i == j) return; \n"
               "}\n";
        ASSERT_EQUALS(false, valueOfTok(code, "!=").intvalue == 1);

        // same expression
        code = "void f(int i, int j) {\n"
               "    if (i != j) return;\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(int i, int j) {\n"
               "    if (i != j) return;\n"
               "    i++;\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 0));

        code = "void f(int i, int j, bool a) {\n"
               "    if (a) {\n"
               "        if (i != j) return;\n"
               "    }\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 0));

        code = "void f(int i, int j, bool a) {\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "    if (i != j) return; \n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 0));

        code = "void f(int i, int j, bool b) {\n"
               "    if (i == j) { if(b) return; }\n"
               "    if(i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(false, valueOfTok(code, "!=").intvalue == 1);

        code = "void foo()\n" // #8924
               "{\n"
               "    if ( this->FileIndex >= 0 )\n"
               "        return;\n"
               "\n"
               "    this->FileIndex = 1 ;\n"
               "    if ( this->FileIndex < 0 ) {}\n"
               "}";
        ASSERT_EQUALS(false, valueOfTok(code, "<").intvalue == 1);

        code = "int f(int p) {\n"
               "    int v = 0;\n"
               "    for (int i = 0; i < 1; ++i) {\n"
               "        if (p == 0)\n"
               "            v = 1;\n"
               "        if (v == 1)\n"
               "            break;\n"
               "    }\n"
               "    int x = v;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 10U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 10U, 1));

        code = "void f() {\n"
               "    const int size = arrayInfo.num(0);\n"
               "    if (size <= 0)\n"
               "        return;\n"
               "    for (;;)\n"
               "        if (size > 0) {}\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "> 0").isKnown());
        ASSERT_EQUALS(true, valueOfTok(code, "> 0").intvalue == 1);
    }

    static std::string isPossibleContainerSizeValue(const std::list<ValueFlow::Value> &values, MathLib::bigint i) {
        if (values.size() != 1)
            return "values.size():" + std::to_string(values.size());
        if (!values.front().isContainerSizeValue())
            return "ContainerSizeValue";
        if (!values.front().isPossible())
            return "Possible";
        if (values.front().intvalue != i)
            return "intvalue:" + std::to_string(values.front().intvalue);
        return "";
    }

    static std::string isImpossibleContainerSizeValue(const std::list<ValueFlow::Value>& values, MathLib::bigint i) {
        if (values.size() != 1)
            return "values.size():" + std::to_string(values.size());
        if (!values.front().isContainerSizeValue())
            return "ContainerSizeValue";
        if (!values.front().isImpossible())
            return "Impossible";
        if (values.front().intvalue != i)
            return "intvalue:" + std::to_string(values.front().intvalue);
        return "";
    }

    static std::string isKnownContainerSizeValue(const std::list<ValueFlow::Value> &values, MathLib::bigint i) {
        if (values.size() != 1)
            return "values.size():" + std::to_string(values.size());
        if (!values.front().isContainerSizeValue())
            return "ContainerSizeValue";
        if (!values.front().isKnown())
            return "Known";
        if (values.front().intvalue != i)
            return "intvalue:" + std::to_string(values.front().intvalue);
        return "";
    }

    void valueFlowContainerSize() {
        const char *code;

        LOAD_LIB_2(settings.library, "std.cfg");

        // condition
        code = "void f(const std::list<int> &ints) {\n"
               "  if (!static_cast<bool>(ints.empty()))\n"
               "    ints.front();\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        // valueFlowContainerReverse
        code = "void f(const std::list<int> &ints) {\n"
               "  ints.front();\n" // <- container can be empty
               "  if (ints.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  ints.front();\n" // <- container can be empty
               "  if (ints.size()==0) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(std::list<int> ints) {\n"
               "  ints.front();\n" // <- no container size
               "  ints.pop_back();\n"
               "  if (ints.empty()) {}\n"
               "}";
        ASSERT(tokenValues(code, "ints . front").empty());

        code = "void f(std::vector<int> v) {\n"
               "  v[10] = 0;\n" // <- container size can be 10
               "  if (v.size() == 10) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "v ["), 10));

        code = "void f(std::vector<std::string> params) {\n"
               "  switch(x) {\n"
               "  case CMD_RESPONSE:\n"
               "    if(y) { break; }\n"
               "    params[2];\n" // <- container use
               "    break;\n"
               "  case CMD_DELETE:\n"
               "    if (params.size() < 2) { }\n" // <- condition
               "    break;\n"
               "  }\n"
               "}";
        ASSERT(tokenValues(code, "params [ 2 ]").empty());

        // valueFlowAfterCondition
        code = "void f(const std::vector<std::string>& v) {\n"
               "    if(v.empty()) {\n"
               "        v.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "v . front"), 0));

        code = "void f(const std::vector<std::string>& v) {\n"
               "    if(!v.empty()) {\n"
               "        v.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "v . front"), 0));

        code = "void f(const std::vector<std::string>& v) {\n"
               "    if(!v.empty() && v[0] != \"\") {\n"
               "        v.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "v . front"), 0));

        // valueFlowContainerForward
        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) {}\n"
               "  ints.front();\n" // <- container can be empty
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) { continue; }\n"
               "  ints.front();\n" // <- no container size
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) { ints.push_back(0); }\n"
               "  ints.front();\n" // <- container is not empty
               "}";
        ASSERT(tokenValues(code, "ints . front").empty());

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) {\n"
               "    ints.front();\n" // <- container is empty
               "  }\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() == 3) {\n"
               "    ints.front();\n" // <- container size is 3
               "  }\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 3));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() <= 3) {\n"
               "    ints.front();\n" // <- container size is 3
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 3));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() >= 3) {\n"
               "    ints.front();\n" // <- container size is 3
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 3));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() < 3) {\n"
               "    ints.front();\n" // <- container size is 2
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 2));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() > 3) {\n"
               "    ints.front();\n" // <- container size is 4
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 4));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty() == false) {\n"
               "    ints.front();\n" // <- container is not empty
               "  }\n"
               "}";
        ASSERT(tokenValues(code, "ints . front").empty());

        code = "void f(const std::vector<int> &v) {\n"
               "  if (v.empty()) {}\n"
               "  if (!v.empty() && v[10]==0) {}\n" // <- no container size for 'v[10]'
               "}";
        ASSERT(tokenValues(code, "v [").empty());

        code = "void f() {\n"
               "  std::list<int> ints;\n"  // No value => ints is empty
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f() {\n"
               "  std::array<int,10> ints;\n" // Array size is 10
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 10));

        code = "void f() {\n"
               "  std::string s;\n"
               "  cin >> s;\n"
               "  s[0];\n"
               "}";
        ASSERT(tokenValues(code, "s [").empty());

        code = "void f() {\n"
               "  std::string s = \"abc\";\n" // size of s is 3
               "  s.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "s . size"), 3));

        code = "void f() {\n"
               "  std::string s=\"abc\";\n" // size of s is 3
               "  s += unknown;\n"
               "  s.size();\n"
               "}";
        ASSERT(tokenValues(code, "s . size").empty());

        code = "void f() {\n"
               "  std::string s=\"abc\";\n" // size of s is 3
               "  s += \"def\";\n" // size of s => 6
               "  s.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "s . size"), 6));

        code = "void f(std::string s) {\n"
               "    if (s == \"hello\")\n"
               "        s[40] = c;\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "s ["), 5));

        code = "void f(std::string s) {\n"
               "    s[40] = c;\n"
               "    if (s == \"hello\") {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "s ["), 5));

        code = "void f(std::string s) {\n"
               "    if (s != \"hello\") {}\n"
               "    s[40] = c;\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "s ["), 5));

        code = "void f(std::string s) {\n"
               "    if (s != \"hello\")\n"
               "        s[40] = c;\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "s ["), 5));

        code = "void f() {\n"
               "    static std::string s;\n"
               "    if (s.size() == 0)\n"
               "        s = x;\n"
               "}";
        ASSERT(tokenValues(code, "s . size").empty());

        // valueFlowContainerForward, loop
        code = "void f() {\n"
               "    std::stack<Token *> links;\n"
               "    while (!links.empty() || indentlevel)\n"
               "        links.push(tok);\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "links . empty"), 0));

        // valueFlowContainerForward, function call
        code = "void f() {\n"
               "  std::list<int> x;\n"
               "  f(x);\n"
               "  x.front();\n" // <- unknown container size
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void f() {\n" // #8689
               "  std::list<int> x;\n"
               "  f<ns::a>(x);\n"
               "  x.front();\n" // <- unknown container size
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>*);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>* const);\n" // #9434
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(const std::list<int>&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(int&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x[0]);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(int&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x.back());\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>&) {}\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>& y) { y.push_back(1); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>*) {}\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>* y) { y->push_back(1); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void h(std::list<int>&);\n"
               "void g(std::list<int>& y) { h(y); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void h(const std::list<int>&);\n"
               "void g(std::list<int>& y) { h(y); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void h(const std::list<int>&);\n"
               "void g(std::list<int>& y) { h(y); y.push_back(1); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void f(std::vector<int> ints) {\n" // #8697
               "  if (ints.empty())\n"
               "    abort() << 123;\n"
               "  ints[0] = 0;\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints ["), 0));

        code = "struct A {\n"  // forward, nested function call, #9424
               "    double getMessage( std::vector<unsigned char> *message );\n"
               "};\n"
               "\n"
               "struct B {\n"
               "    A *a;\n"
               "    double getMessage( std::vector<unsigned char> *message ) { return a->getMessage( message ); }\n"
               "};\n"
               "\n"
               "void foo(B *ptr) {\n"
               "    std::vector<unsigned char> v;\n"
               "    ptr->getMessage (&v);\n"
               "    if (v.size () > 0) {}\n" // <- v has unknown size!
               "}";
        ASSERT_EQUALS(0U, tokenValues(code, "v . size ( )").size());

        // if
        code = "bool f(std::vector<int>&) {\n" // #9532
               "  return false;\n"
               "}\n"
               "int g() {\n"
               "    std::vector<int> v;\n"
               "    if (f(v) || v.empty())\n"
               "        return 0;\n"
               "    return v[0];\n"
               "}\n";
        ASSERT_EQUALS(0U, tokenValues(code, "v [ 0 ]").size());

        // container size => yields
        code = "void f() {\n"
               "  std::string s = \"abcd\";\n"
               "  s.size();\n"
               "}";
        ASSERT_EQUALS(4, tokenValues(code, "( ) ;").front().intvalue);

        code = "void f() {\n"
               "  std::string s;\n"
               "  s.empty();\n"
               "}";
        ASSERT_EQUALS(1, tokenValues(code, "( ) ;").front().intvalue);

        // Calculations
        code = "void f() {\n"
               "  std::string s = \"abcd\";\n"
               "  x = s + s;\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "+"), 8));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.clear();\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.resize(3);\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 3));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.resize(3);\n"
               "  ints.push_back(3);\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 4));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.resize(3);\n"
               "  ints.pop_back();\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 2));

        code = "int f(bool b) {\n"
               "    std::map<int, int> m;\n"
               "    if (b)\n"
               "        m[0] = 1;\n"
               "    return m.at(0);\n"
               "}\n";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "m . at", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "struct Base {\n"
               "    virtual bool GetString(std::string &) const { return false; }\n"
               "};\n"
               "int f() {\n"
               "    std::string str;\n"
               "    Base *b = GetClass();\n"
               "    if (!b->GetString(str)) {\n"
               "        return -2;\n"
               "    }\n"
               "    else {\n"
               "        return str.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(0U, tokenValues(code, "str . front").size());

        code = "void f() {\n"
               "  std::vector<int> ints{};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f() {\n"
               "  std::vector<int> ints{1};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 1));

        code = "void f() {\n"
               "  std::vector<int> ints{1};\n"
               "  std::vector<int> ints2{ints.begin(), ints.end()};\n"
               "  ints2.front();\n"
               "}";
        ASSERT_EQUALS(
            "", isKnownContainerSizeValue(tokenValues(code, "ints2 . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 1));

        code = "void f() {\n"
               "  std::vector<int> ints = {};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f() {\n"
               "  std::vector<int> ints = {1};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 1));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\")\n"
               "        bool x = str.empty();\n"
               "}\n";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "str . empty", ValueFlow::Value::ValueType::CONTAINER_SIZE), 3));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str == \"\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str != \"\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str == \"321\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 1));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str != \"321\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 0));

        code = "void f(std::string str) {\n"
               "    if (str.size() == 1) {\n"
               "        bool x = (str == \"123\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));
    }

    void valueFlowDynamicBufferSize() {
        const char *code;

        LOAD_LIB_2(settings.library, "std.cfg");
        LOAD_LIB_2(settings.library, "posix.cfg");

        code = "void* f() {\n"
               "  void* x = malloc(10);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 10,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  void* x = calloc(4, 5);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 20,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  const char* y = \"abcd\";\n"
               "  const char* x = strdup(y);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 5,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  void* y = malloc(10);\n"
               "  void* x = realloc(y, 20);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 20,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  void* y = calloc(10, 4);\n"
               "  void* x = reallocarray(y, 20, 5);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 100,  ValueFlow::Value::ValueType::BUFFER_SIZE));
    }

    void valueFlowSafeFunctionParameterValues() {
        const char *code;
        std::list<ValueFlow::Value> values;
        Settings s;
        LOAD_LIB_2(s.library, "std.cfg");
        s.safeChecks.classes = s.safeChecks.externalFunctions = s.safeChecks.internalFunctions = true;

        code = "short f(short x) {\n"
               "  return x + 0;\n"
               "}";
        values = tokenValues(code, "+", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(-0x8000, values.front().intvalue);
        ASSERT_EQUALS(0x7fff, values.back().intvalue);

        code = "short f(std::string x) {\n"
               "  return x[10];\n"
               "}";
        values = tokenValues(code, "x [", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);
        ASSERT_EQUALS(1000000, values.back().intvalue);

        code = "int f(float x) {\n"
               "  return x;\n"
               "}";
        values = tokenValues(code, "x ;", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT(values.front().floatValue < -1E20);
        ASSERT(values.back().floatValue > 1E20);

        code = "short f(__cppcheck_low__(0) __cppcheck_high__(100) short x) {\n"
               "  return x + 0;\n"
               "}";
        values = tokenValues(code, "+", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);
        ASSERT_EQUALS(100, values.back().intvalue);

        code = "unsigned short f(unsigned short x) [[expects: x <= 100]] {\n"
               "  return x + 0;\n"
               "}";
        values = tokenValues(code, "+", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);
        ASSERT_EQUALS(100, values.back().intvalue);
    }


    void valueFlowUnknownFunctionReturn() {
        const char *code;
        std::list<ValueFlow::Value> values;
        Settings s;
        LOAD_LIB_2(s.library, "std.cfg");
        s.checkUnknownFunctionReturn.insert("rand");

        code = "x = rand();";
        values = tokenValues(code, "(", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(INT_MIN, values.front().intvalue);
        ASSERT_EQUALS(INT_MAX, values.back().intvalue);
    }

    void valueFlowPointerAliasDeref() {
        const char* code;

        code = "int f() {\n"
               "  int a = 123;\n"
               "  int *p = &a;\n"
               "  int x = *p;\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 123));
    }

    void valueFlowCrashIncompleteCode() {
        const char* code;

        code = "void SlopeFloor::setAttr(const Value &val) {\n"
               "    int x = val;\n"
               "    if (x >= -1)\n"
               "        state = x;\n"
               "}\n";
        valueOfTok(code, "=");

        code = "void a() {\n"
               "  auto b = [b = 0] {\n"
               "    if (b) {\n"
               "    }\n"
               "  };\n"
               "}\n";
        valueOfTok(code, "0");
    }

    void valueFlowCrash() {
        const char* code;

        code = "void f(int x) {\n"
               "    if (0 * (x > 2)) {}\n"
               "}\n";
        valueOfTok(code, "x");

        code = "const int& f(int, const int& y = 0);\n"
               "const int& f(int, const int& y) {\n"
               "    return y;\n"
               "}\n"
               "const int& g(int x) {\n"
               "    const int& r = f(x);\n"
               "    return r;\n"
               "}\n";
        valueOfTok(code, "0");

        code = "void fa(int &colors) {\n"
               "  for (int i = 0; i != 6; ++i) {}\n"
               "}\n"
               "void fb(not_null<int*> parent, int &&colors2) {\n"
               "  dostuff(1);\n"
               "}\n";
        valueOfTok(code, "x");

        code = "void a() {\n"
               "  static int x = 0;\n"
               "  struct c {\n"
               "    c(c &&) { ++x; }\n"
               "  };\n"
               "}\n";
        valueOfTok(code, "x");

        code = "void f(){\n"
               "      struct dwarf_data **pp;\n"
               "      for (pp = (struct dwarf_data **) (void *) &state->fileline_data;\n"
               "       *pp != NULL;\n"
               "       pp = &(*pp)->next)\n"
               "    ;\n"
               "}\n";
        valueOfTok(code, "x");

        code = "void *foo(void *x);\n"
               "void *foo(void *x)\n"
               "{\n"
               "    if (!x)\n"
               "yes:\n"
               "        return &&yes;\n"
               "    return x;\n"
               "}\n";
        valueOfTok(code, "x");

        code = "void f() {\n"
               "    std::string a = b[c->d()];\n"
               "    if(a.empty()) {\n"
               "        INFO(std::string{\"a\"} + c->d());\n"
               "        INFO(std::string{\"b\"} + a);\n"
               "    }\n"
               "}\n";
        valueOfTok(code, "a");

        code = "class A{\n"
               "  void f() {\n"
               "    std::string c{s()};\n"
               "  }\n"
               "  std::string s() {\n"
               "    return \"\";\n"
               "  }\n"
               "};\n";
        valueOfTok(code, "c");

        code = "void f() {\n"
               "   char* p = 0;\n"
               "   int pi =\n"
               "     p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : 0;\n"
               "   int *i2 = 0;\n"
               "   if (i2) { }\n"
               "}\n";
        valueOfTok(code, "p");

        code = "struct a;\n"
               "namespace e {\n"
               "struct f {\n"
               "  struct g {\n"
               "    enum {} h;\n"
               "    int arg;\n"
               "  };\n"
               "  std::vector<g> i;\n"
               "};\n"
               "} // namespace e\n"
               "void fn1() {\n"
               "  std::vector<a *> arguments;\n"
               "  e::f b;\n"
               "  for (e::f::g c : b.i)\n"
               "    if (c.h)\n"
               "      a *const d = arguments[c.arg];\n"
               "}\n";
        valueOfTok(code, "c");
    }

    void valueFlowHang() {
        const char* code;
        // #9659
        code = "float arr1[4][4] = {0.0};\n"
               "float arr2[4][4] = {0.0};\n"
               "void f() {\n"
               "    if(arr1[0][0] == 0.0 &&\n"
               "       arr1[0][1] == 0.0 &&\n"
               "       arr1[0][2] == 0.0 &&\n"
               "       arr1[0][3] == 0.0 &&\n"
               "       arr1[1][0] == 0.0 &&\n"
               "       arr1[1][1] == 0.0 &&\n"
               "       arr1[1][2] == 0.0 &&\n"
               "       arr1[1][3] == 0.0 &&\n"
               "       arr1[2][0] == 0.0 &&\n"
               "       arr1[2][1] == 0.0 &&\n"
               "       arr1[2][2] == 0.0 &&\n"
               "       arr1[2][3] == 0.0 &&\n"
               "       arr1[3][0] == 0.0 &&\n"
               "       arr1[3][1] == 0.0 &&\n"
               "       arr1[3][2] == 0.0 &&\n"
               "       arr1[3][3] == 0.0 &&\n"
               "       arr2[0][0] == 0.0 &&\n"
               "       arr2[0][1] == 0.0 &&\n"
               "       arr2[0][2] == 0.0 &&\n"
               "       arr2[0][3] == 0.0 &&\n"
               "       arr2[1][0] == 0.0 &&\n"
               "       arr2[1][1] == 0.0 &&\n"
               "       arr2[1][2] == 0.0 &&\n"
               "       arr2[1][3] == 0.0 &&\n"
               "       arr2[2][0] == 0.0 &&\n"
               "       arr2[2][1] == 0.0 &&\n"
               "       arr2[2][2] == 0.0 &&\n"
               "       arr2[2][3] == 0.0 &&\n"
               "       arr2[3][0] == 0.0 &&\n"
               "       arr2[3][1] == 0.0 &&\n"
               "       arr2[3][2] == 0.0 &&\n"
               "       arr2[3][3] == 0.0\n"
               "       ) {}\n"
               "}\n";
        valueOfTok(code, "x");

        code = "namespace {\n"
               "struct a {\n"
               "  a(...) {}\n"
               "  a(std::initializer_list<std::pair<int, std::vector<std::vector<a>>>>) {}\n"
               "} b{{0, {{&b, &b, &b, &b}}},\n"
               "    {0,\n"
               "     {{&b, &b, &b, &b, &b, &b, &b, &b, &b, &b},\n"
               "      {{&b, &b, &b, &b, &b, &b, &b}}}},\n"
               "    {0,\n"
               "     {{&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b},\n"
               "      {&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b}}}};\n"
               "}\n";
        valueOfTok(code, "x");

        code = "namespace {\n"
               "struct a {\n"
               "  a(...) {}\n"
               "  a(std::initializer_list<std::pair<int, std::vector<std::vector<a>>>>) {}\n"
               "} b{{0, {{&b}}},\n"
               "    {0, {{&b}}},\n"
               "    {0, {{&b}}},\n"
               "    {0, {{&b}}},\n"
               "    {0, {{&b}, {&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, {&b}}}},\n"
               "    {0,\n"
               "     {{&b},\n"
               "      {&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b,\n"
               "       &b}}}};\n"
               "}\n";
        valueOfTok(code, "x");
    }

    void valueFlowCrashConstructorInitialization() { // #9577
        const char* code;
        code = "void Error()\n"
               "{\n"
               "    VfsPath path(\"\");\n"
               "    path = path / amtype;\n"
               "    size_t base = 0;\n"
               "    VfsPath standard(\"standard\");\n"
               "    if (path != standard)\n"
               "    {\n"
               "    }\n"
               "}";
        valueOfTok(code, "path");

        code = "void Error()\n"
               "{\n"
               "    VfsPath path;\n"
               "    path = path / amtype;\n"
               "    size_t base = 0;\n"
               "    VfsPath standard(\"standard\");\n"
               "    if (path != standard)\n"
               "    {\n"
               "    }\n"
               "}";
        valueOfTok(code, "path");
    }

    void valueFlowUnknownMixedOperators() {
        const char *code= "int f(int a, int b, bool x) {\n"
                          "  if (a == 1 && (!(b == 2 && x))) {\n"
                          "  } else {\n"
                          "    if (x) {\n"
                          "    }\n"
                          "  }\n"
                          "\n"
                          "  return 0;\n"
                          "}" ;

        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 1));
    }

    void valueFlowIdempotent() {
        const char *code;

        code = "void f(bool a, bool b) {\n"
               "    bool x = true;\n"
               "    if (a)\n"
               "        x = x && b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 1));

        code = "void f(bool a, bool b) {\n"
               "    bool x = false;\n"
               "    if (a)\n"
               "        x = x && b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));

        code = "void f(bool a, bool b) {\n"
               "    bool x = true;\n"
               "    if (a)\n"
               "        x = x || b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "void f(bool a, bool b) {\n"
               "    bool x = false;\n"
               "    if (a)\n"
               "        x = x || b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 0));
    }
};

REGISTER_TEST(TestValueFlow)
