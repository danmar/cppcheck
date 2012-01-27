/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include "symboldatabase.h"

#define GET_SYMBOL_DB(code) \
    errout.str(""); \
    Settings settings; \
    Tokenizer tokenizer(&settings, this); \
    std::istringstream istr(code); \
    tokenizer.tokenize(istr, "test.cpp"); \
    const SymbolDatabase *db = tokenizer.getSymbolDatabase();

class TestSymbolDatabase: public TestFixture {
public:
    TestSymbolDatabase()
        :TestFixture("TestSymbolDatabase")
        ,si(NULL, NULL, NULL)
        ,vartok(NULL)
        ,typetok(NULL)
        ,t(NULL)
        ,found(false)
        ,isArray(false)
        ,isPointer(false)
        ,isReference(false)
    {}

    virtual void reportOut(const std::string &outmsg) {
        errout << outmsg << std::endl;
    }

private:
    const Scope si;
    const Token* vartok;
    const Token* typetok;
    const Token* t;
    bool found;
    bool isArray;
    bool isPointer;
    bool isReference;

    void reset() {
        vartok = NULL;
        typetok = NULL;
        t = NULL;
        found = false;
        isArray = false;
        isPointer = false;
        isReference = false;
    }

    void run() {
        TEST_CASE(test_isVariableDeclarationCanHandleNull);
        TEST_CASE(test_isVariableDeclarationIdentifiesSimpleDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesStdDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedStdDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesManyScopes);
        TEST_CASE(test_isVariableDeclarationIdentifiesPointers);
        TEST_CASE(test_isVariableDeclarationDoesNotIdentifyConstness);
        TEST_CASE(test_isVariableDeclarationIdentifiesFirstOfManyVariables);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedPointerDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesDeclarationWithIndirection);
        TEST_CASE(test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection);
        TEST_CASE(test_isVariableDeclarationIdentifiesArray);
        TEST_CASE(test_isVariableDeclarationIdentifiesOfArrayPointers);
        TEST_CASE(isVariableDeclarationIdentifiesTemplatedPointerVariable);
        TEST_CASE(isVariableDeclarationIdentifiesTemplatedPointerToPointerVariable);
        TEST_CASE(isVariableDeclarationIdentifiesTemplatedArrayVariable);
        TEST_CASE(isVariableDeclarationIdentifiesTemplatedVariable);
        TEST_CASE(isVariableDeclarationIdentifiesTemplatedVariableIterator);
        TEST_CASE(isVariableDeclarationIdentifiesNestedTemplateVariable);
        TEST_CASE(isVariableDeclarationIdentifiesReference);
        TEST_CASE(isVariableDeclarationDoesNotIdentifyTemplateClass);
        TEST_CASE(canFindMatchingBracketsNeedsOpen);
        TEST_CASE(canFindMatchingBracketsInnerPair);
        TEST_CASE(canFindMatchingBracketsOuterPair);
        TEST_CASE(canFindMatchingBracketsWithTooManyClosing);
        TEST_CASE(canFindMatchingBracketsWithTooManyOpening);

        TEST_CASE(hasRegularFunction);
        TEST_CASE(hasInlineClassFunction);
        TEST_CASE(hasMissingInlineClassFunction);
        TEST_CASE(hasClassFunction);

        TEST_CASE(hasRegularFunctionReturningFunctionPointer);
        TEST_CASE(hasInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasMissingInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasClassFunctionReturningFunctionPointer);

        TEST_CASE(parseFunctionCorrect);

        TEST_CASE(hasGlobalVariables1);
        TEST_CASE(hasGlobalVariables2);
        TEST_CASE(hasGlobalVariables3);

        TEST_CASE(functionArgs1);
        TEST_CASE(functionArgs2);

        TEST_CASE(namespaces1);
        TEST_CASE(namespaces2);

        TEST_CASE(tryCatch1);

        TEST_CASE(symboldatabase1);
        TEST_CASE(symboldatabase2);
        TEST_CASE(symboldatabase3); // ticket #2000
        TEST_CASE(symboldatabase4);
        TEST_CASE(symboldatabase5); // ticket #2178
        TEST_CASE(symboldatabase6); // ticket #2221
        TEST_CASE(symboldatabase7); // ticket #2230
        TEST_CASE(symboldatabase8); // ticket #2252
        TEST_CASE(symboldatabase9); // ticket #2525
        TEST_CASE(symboldatabase10); // ticket #2537
        TEST_CASE(symboldatabase11); // ticket #2539
        TEST_CASE(symboldatabase12); // ticket #2547
        TEST_CASE(symboldatabase13); // ticket #2577
        TEST_CASE(symboldatabase14); // ticket #2589
        TEST_CASE(symboldatabase15); // ticket #2591
        TEST_CASE(symboldatabase16); // ticket #2637
        TEST_CASE(symboldatabase17); // ticket #2657
        TEST_CASE(symboldatabase18); // ticket #2865
        TEST_CASE(symboldatabase19); // ticket #2991 (segmentation fault)
        TEST_CASE(symboldatabase20); // ticket #3013 (segmentation fault)
        TEST_CASE(symboldatabase21);
        TEST_CASE(symboldatabase22); // ticket #3437 (segmentation fault)
        TEST_CASE(symboldatabase23); // ticket #3435
        TEST_CASE(symboldatabase24); // ticket #3508 (constructor, destructor)
    }

    void test_isVariableDeclarationCanHandleNull() {
        reset();
        bool result = si.isVariableDeclaration(NULL, vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x;");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration() {
        reset();
        givenACodeSampleToTokenize ScopedDeclaration("::int x;");
        bool result = si.isVariableDeclaration(ScopedDeclaration.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesStdDeclaration() {
        reset();
        givenACodeSampleToTokenize StdDeclaration("std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesScopedStdDeclaration() {
        reset();
        givenACodeSampleToTokenize StdDeclaration("::std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesManyScopes() {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE x;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesPointers() {
        reset();
        givenACodeSampleToTokenize pointer("int* p;");
        bool result = si.isVariableDeclaration(pointer.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
        ASSERT(true == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationDoesNotIdentifyConstness() {
        reset();
        givenACodeSampleToTokenize constness("const int* cp;");
        bool result = si.isVariableDeclaration(constness.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables() {
        reset();
        givenACodeSampleToTokenize multipleDeclaration("int first, second;");
        bool result = si.isVariableDeclaration(multipleDeclaration.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesScopedPointerDeclaration() {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE* p;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        ASSERT(false == isArray);
        ASSERT(true == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithIndirection() {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int** pp;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("pp", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
        ASSERT(true == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection() {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int***** p;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
        ASSERT(true == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesArray() {
        reset();
        givenACodeSampleToTokenize array("::std::string v[3];");
        bool result = si.isVariableDeclaration(array.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("v", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        ASSERT(true == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void test_isVariableDeclarationIdentifiesOfArrayPointers() {
        reset();
        givenACodeSampleToTokenize array("A *a[5];");
        bool result = si.isVariableDeclaration(array.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
        ASSERT(true == isArray);
        ASSERT(true == isPointer);
        ASSERT(false == isReference);
    }

    void isVariableDeclarationIdentifiesTemplatedPointerVariable() {
        reset();
        givenACodeSampleToTokenize var("std::set<char>* chars;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("chars", vartok->str());
        ASSERT_EQUALS("set", typetok->str());
        ASSERT(false == isArray);
        ASSERT(true == isPointer);
        ASSERT(false == isReference);
    }

    void isVariableDeclarationIdentifiesTemplatedPointerToPointerVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<int>*** ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        ASSERT(false == isArray);
        ASSERT(true == isPointer);
        ASSERT(false == isReference);
    }

    void isVariableDeclarationIdentifiesTemplatedArrayVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<int> ints[3];");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        ASSERT(true == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void isVariableDeclarationIdentifiesTemplatedVariable() {
        reset();
        givenACodeSampleToTokenize var("std::vector<int> ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("vector", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void isVariableDeclarationIdentifiesTemplatedVariableIterator() {
        reset();
        givenACodeSampleToTokenize var("std::list<int>::const_iterator floats;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("floats", vartok->str());
        ASSERT_EQUALS("const_iterator", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void isVariableDeclarationIdentifiesNestedTemplateVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("intsets", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void isVariableDeclarationIdentifiesReference() {
        reset();
        givenACodeSampleToTokenize var1("int& foo;");
        bool result1 = si.isVariableDeclaration(var1.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result1);
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(true == isReference);

        reset();
        givenACodeSampleToTokenize var2("foo*& bar;");
        bool result2 = si.isVariableDeclaration(var2.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result2);
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(true == isReference);

        reset();
        givenACodeSampleToTokenize var3("std::vector<int>& foo;");
        bool result3 = si.isVariableDeclaration(var3.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(true, result3);
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(true == isReference);
    }

    void isVariableDeclarationDoesNotIdentifyTemplateClass() {
        reset();
        givenACodeSampleToTokenize var("template <class T> class SomeClass{};");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray, isPointer, isReference);
        ASSERT_EQUALS(false, result);
        ASSERT(false == isArray);
        ASSERT(false == isPointer);
        ASSERT(false == isReference);
    }

    void canFindMatchingBracketsNeedsOpen() {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        found = si.findClosingBracket(var.tokens(), t);
        ASSERT(! found);
        ASSERT(! t);
    }

    void canFindMatchingBracketsInnerPair() {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        found = si.findClosingBracket(var.tokens()->tokAt(7), t);
        ASSERT(found);
        ASSERT_EQUALS(">", t->str());
        ASSERT_EQUALS(var.tokens()->strAt(9), t->str());
    }

    void canFindMatchingBracketsOuterPair() {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        found = si.findClosingBracket(var.tokens()->tokAt(3), t);
        ASSERT(found);
        ASSERT_EQUALS(">", t->str());
        ASSERT_EQUALS(var.tokens()->strAt(10), t->str());

    }

    void canFindMatchingBracketsWithTooManyClosing() {
        reset();
        givenACodeSampleToTokenize var("X< 1>2 > x1;\n");

        found = si.findClosingBracket(var.tokens()->next(), t);
        ASSERT(found);
        ASSERT_EQUALS(">", t->str());
        ASSERT_EQUALS(var.tokens()->strAt(3), t->str());
    }

    void canFindMatchingBracketsWithTooManyOpening() {
        reset();
        givenACodeSampleToTokenize var("X < (2 < 1) > x1;\n");

        found = si.findClosingBracket(var.tokens()->next(), t);
        ASSERT(!found);
    }

    void hasRegularFunction() {
        GET_SYMBOL_DB("void func() { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->next());

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->next());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->next());
            ASSERT(function && function->hasBody);
        }
    }

    void hasInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func() { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(4));
            ASSERT(function && function->hasBody && function->isInline);
        }
    }

    void hasMissingInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2 && !tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(scope == NULL);

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(4));
            ASSERT(function && !function->hasBody);
        }
    }

    void hasClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); }; Fred::func() { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(12));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(12));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(12));
            ASSERT(function && function->hasBody && !function->isInline);
        }
    }

    void hasRegularFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("void (*func(int f))(char) { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(3));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(3));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(3));
            ASSERT(function && function->hasBody && function->retFuncPtr);
        }
    }

    void hasInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char) { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(6));
            ASSERT(function && function->hasBody && function->isInline && function->retFuncPtr);
        }
    }

    void hasMissingInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2 && !tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(scope == NULL);

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(6));
            ASSERT(function && !function->hasBody && function->retFuncPtr);
        }
    }

    void hasClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); }; void (*Fred::func(int f))(char) { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db) {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(23));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(23));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(23));
            ASSERT(function && function->hasBody && !function->isInline && function->retFuncPtr);
        }
    }

    void parseFunctionCorrect() {
        // ticket 3188 - "if" statement parsed as function
        GET_SYMBOL_DB("void func(i) int i; { if (i == 1) return; }\n")
        ASSERT(db != NULL);

        // 3 scopes: Global, function, if
        ASSERT_EQUALS(3, db->scopeList.size());

        ASSERT(tokenizer.getFunctionTokenByName("func") != NULL);
        ASSERT(tokenizer.getFunctionTokenByName("if") == NULL);
    }

    void hasGlobalVariables1() {
        GET_SYMBOL_DB("int i;\n")

        ASSERT(db && db->scopeList.size() == 1);
        if (db && db->scopeList.size() == 1) {
            std::list<Scope>::const_iterator it = db->scopeList.begin();
            ASSERT(it->varlist.size() == 1);
            if (it->varlist.size() == 1) {
                std::list<Variable>::const_iterator var = it->varlist.begin();
                ASSERT(var->name() == "i");
                ASSERT(var->typeStartToken()->str() == "int");
            }
        }
    }

    void hasGlobalVariables2() {
        GET_SYMBOL_DB("int array[2][2];\n")

        ASSERT(db && db->scopeList.size() == 1);
        if (db && db->scopeList.size() == 1) {
            std::list<Scope>::const_iterator it = db->scopeList.begin();
            ASSERT(it->varlist.size() == 1);
            if (it->varlist.size() == 1) {
                std::list<Variable>::const_iterator var = it->varlist.begin();
                ASSERT(var->name() == "array");
                ASSERT(var->typeStartToken()->str() == "int");
            }
        }
    }

    void hasGlobalVariables3() {
        GET_SYMBOL_DB("int array[2][2] = { { 0, 0 }, { 0, 0 } };\n")

        ASSERT(db && db->scopeList.size() == 1);
        if (db && db->scopeList.size() == 1) {
            std::list<Scope>::const_iterator it = db->scopeList.begin();
            ASSERT(it->varlist.size() == 1);
            if (it->varlist.size() == 1) {
                std::list<Variable>::const_iterator var = it->varlist.begin();
                ASSERT(var->name() == "array");
                ASSERT(var->typeStartToken()->str() == "int");
            }
        }
    }

    void check(const char code[], bool debug = true) {
        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings.debugwarnings = debug;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // force symbol database creation
        tokenizer.getSymbolDatabase();
    }

    void functionArgs1() {
        check("void f(std::vector<std::string s>, const std::vector<int> & v) { }\n");

        ASSERT_EQUALS("", errout.str());

        check("void f(std::map<std::string, std::vector<int> > m) { }\n");

        ASSERT_EQUALS("", errout.str());
    }

    void functionArgs2() {
        GET_SYMBOL_DB("void f(int a[][4]) { }");
        const Variable *a = db->getVariableFromVarId(1);
        ASSERT_EQUALS("a", a->nameToken()->str());
        ASSERT_EQUALS(2UL, a->dimensions().size());
        ASSERT_EQUALS(0UL, a->dimension(0));
        ASSERT_EQUALS(4UL, a->dimension(1));
    }

    void namespaces1() {
        GET_SYMBOL_DB("namespace fred {\n"
                      "    namespace barney {\n"
                      "        class X { X(int); };\n"
                      "    }\n"
                      "}\n"
                      "namespace barney { X::X(int) { } }\n");

        // Locate the scope for the class..
        const Scope *scope = NULL;
        for (std::list<Scope>::const_iterator it = db->scopeList.begin(); it != db->scopeList.end(); ++it) {
            if (it->isClassOrStruct()) {
                scope = &(*it);
                break;
            }
        }

        ASSERT(scope != 0);
        if (!scope)
            return;

        ASSERT_EQUALS("X", scope->className);

        // The class has a constructor but the implementation _is not_ seen
        ASSERT_EQUALS(1U, scope->functionList.size());
        const Function *function = &(scope->functionList.front());
        ASSERT_EQUALS(false, function->hasBody);
    }

    // based on namespaces1 but here the namespaces match
    void namespaces2() {
        GET_SYMBOL_DB("namespace fred {\n"
                      "    namespace barney {\n"
                      "        class X { X(int); };\n"
                      "    }\n"
                      "}\n"
                      "namespace fred {\n"
                      "    namespace barney {\n"
                      "        X::X(int) { }\n"
                      "    }\n"
                      "}\n");

        // Locate the scope for the class..
        const Scope *scope = NULL;
        for (std::list<Scope>::const_iterator it = db->scopeList.begin(); it != db->scopeList.end(); ++it) {
            if (it->isClassOrStruct()) {
                scope = &(*it);
                break;
            }
        }

        ASSERT(scope != 0);
        if (!scope)
            return;

        ASSERT_EQUALS("X", scope->className);

        // The class has a constructor and the implementation _is_ seen
        ASSERT_EQUALS(1U, scope->functionList.size());
        const Function *function = &(scope->functionList.front());
        ASSERT_EQUALS("X", function->tokenDef->str());
        ASSERT_EQUALS(true, function->hasBody);
    }


    void tryCatch1() {
        const std::string str("void foo() {\n"
                              "    try { }\n"
                              "    catch (const Error1 & x) { }\n"
                              "    catch (const X::Error2 & x) { }\n"
                              "    catch (Error3 x) { }\n"
                              "    catch (X::Error4 x) { }\n"
                              "}\n");
        GET_SYMBOL_DB(str.c_str())
        check(str.c_str(), false);
        ASSERT_EQUALS("", errout.str());
        ASSERT(db && db->getVariableListSize() == 5); // index 0 + 4 variables
        ASSERT(db && db->scopeList.size() == 7); // global + function + try + 4 catch
    }


    void symboldatabase1() {
        check("namespace foo {\n"
              "    class bar;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class foo : public bar < int, int> {\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase2() {
        check("class foo {\n"
              "public slots :\n"
              "foo() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class foo {\n"
              "class bar;\n"
              "foo() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase3() {
        check("typedef void (func_type)();\n"
              "struct A {\n"
              "    friend func_type f : 2;\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase4() {
        check("static void function_declaration_before(void) __attribute__((__used__));\n"
              "static void function_declaration_before(void) {}\n"
              "static void function_declaration_after(void) {}\n"
              "static void function_declaration_after(void) __attribute__((__used__));\n");
        ASSERT_EQUALS("", errout.str());

        check("main(int argc, char *argv[]) { }\n");
        ASSERT_EQUALS("", errout.str());

        check("namespace boost {\n"
              "    std::locale generate_locale()\n"
              "    {\n"
              "        return std::locale();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("namespace X {\n"
              "    static void function_declaration_before(void) __attribute__((__used__));\n"
              "    static void function_declaration_before(void) {}\n"
              "    static void function_declaration_after(void) {}\n"
              "    static void function_declaration_after(void) __attribute__((__used__));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("testing::testing()\n"
              "{\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase5() {
        // ticket #2178 - segmentation fault
        check("int CL_INLINE_DECL(integer_decode_float) (int x) {\n"
              "    return (sign ? cl_I() : 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase6() {
        // ticket #2221 - segmentation fault
        check("template<int i> class X { };\n"
              "X< 1>2 > x1;\n"
              "X<(1>2)> x2;\n"
              "template<class T> class Y { };\n"
              "Y<X<1>> x3;\n"
              "Y<X<6>>1>> x4;\n"
              "Y<X<(6>>1)>> x5;\n", false);
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase7() {
        // ticket #2230 - segmentation fault
        check("template<template<class> class E,class D> class C : E<D>\n"
              "{\n"
              "public:\n"
              "    int f();\n"
              "};\n"
              "class E : C<D,int>\n"
              "{\n"
              "public:\n"
              "    int f() { return C< ::D,int>::f(); }\n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase8() {
        // ticket #2252 - segmentation fault
        check("struct PaletteColorSpaceHolder: public rtl::StaticWithInit<uno::Reference<rendering::XColorSpace>,\n"
              "                                                           PaletteColorSpaceHolder>\n"
              "{\n"
              "    uno::Reference<rendering::XColorSpace> operator()()\n"
              "    {\n"
              "        return vcl::unotools::createStandardColorSpace();\n"
              "    }\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase9() {
        // ticket #2425 - segmentation fault
        check("class CHyperlink : public CString\n"
              "{\n"
              "public:\n"
              "    const CHyperlink& operator=(LPCTSTR lpsz) {\n"
              "        CString::operator=(lpsz);\n"
              "        return *this;\n"
              "    }\n"
              "};\n", false);

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase10() {
        // ticket #2537 - segmentation fault
        check("class A {\n"
              "private:\n"
              "  void f();\n"
              "};\n"
              "class B {\n"
              "  friend void A::f();\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase11() {
        // ticket #2539 - segmentation fault
        check("int g ();\n"
              "struct S {\n"
              "  int i : (false ? g () : 1);\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase12() {
        // ticket #2547 - segmentation fault
        check("class foo {\n"
              "    void bar2 () = __null;\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase13() {
        // ticket #2577 - segmentation fault
        check("class foo {\n"
              "    void bar2 () = A::f;\n"
              "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase14() {
        // ticket #2589 - segmentation fault
        check("struct B : A\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase15() {
        // ticket #2591 - segmentation fault
        check("struct A :\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase16() {
        // ticket #2637 - segmentation fault
        check("{} const const\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase17() {
        // ticket #2657 - segmentation fault
        check("return f(){}\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase18() {
        // ticket #2865 - segmentation fault
        check("char a[1]\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase19() {
        // ticket #2991 - segmentation fault
        check("::y(){x}\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase20() {
        // ticket #3013 - segmentation fault
        check("struct x : virtual y\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase21() {
        check("class Fred {\n"
              "    class Foo { };\n"
              "    void func() const;\n"
              "};\n"
              "Fred::func() const {\n"
              "    Foo foo;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
    }

    // #ticket 3437 (segmentation fault)
    void symboldatabase22() {
        check("template <class C> struct A {};\n"
              "A<int> a;\n");
        ASSERT_EQUALS("", errout.str());
    }

    // #ticket 3435 (std::vector)
    void symboldatabase23() {
        GET_SYMBOL_DB("class A { std::vector<int*> ints; };\n");
        ASSERT_EQUALS(2U, db->scopeList.size());
        const Scope &scope = db->scopeList.back();
        ASSERT_EQUALS(1U, scope.varlist.size());
        const Variable &var = scope.varlist.front();
        ASSERT_EQUALS(std::string("ints"), var.name());
        ASSERT_EQUALS(true, var.isClass());
    }

    // #ticket 3508 (constructor, destructor)
    void symboldatabase24() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    ~Fred();\n"
                      "    Fred();\n"
                      "};\n"
                      "Fred::Fred() { }\n"
                      "Fred::~Fred() { }");
        // Global scope, Fred, Fred::Fred, Fred::~Fred
        ASSERT_EQUALS(4U, db->scopeList.size());

        // Find the scope for the Fred struct..
        const Scope *fredScope = NULL;
        for (std::list<Scope>::const_iterator scope = db->scopeList.begin(); scope != db->scopeList.end(); ++scope) {
            if (scope->isClassOrStruct() && scope->className == "Fred")
                fredScope = &(*scope);
        }
        ASSERT(fredScope != NULL);
        if (fredScope == NULL)
            return;

        // The struct Fred has two functions, a constructor and a destructor
        ASSERT_EQUALS(2U, fredScope->functionList.size());

        // Get linenumbers where the bodies for the constructor and destructor are..
        unsigned int constructor = 0;
        unsigned int destructor = 0;
        for (std::list<Function>::const_iterator it = fredScope->functionList.begin(); it != fredScope->functionList.end(); ++it) {
            if (it->type == Function::eConstructor)
                constructor = it->token->linenr();  // line number for constructor body
            if (it->type == Function::eDestructor)
                destructor = it->token->linenr();  // line number for destructor body
        }

        // The body for the constructor is located at line 5..
        ASSERT_EQUALS(5U, constructor);

        // The body for the destructor is located at line 6..
        ASSERT_EQUALS(6U, destructor);

    }


};

REGISTER_TEST(TestSymbolDatabase)

