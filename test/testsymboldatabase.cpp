/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include <sstream>

#define GET_SYMBOL_DB(code) \
    errout.str(""); \
    Settings settings; \
    Tokenizer tokenizer(&settings, this); \
    std::istringstream istr(code); \
    tokenizer.tokenize(istr, "test.cpp"); \
    const SymbolDatabase *db = tokenizer.getSymbolDatabase();

#define GET_SYMBOL_DB_C(code) \
    errout.str(""); \
    Settings settings; \
    Tokenizer tokenizer(&settings, this); \
    std::istringstream istr(code); \
    tokenizer.tokenize(istr, "test.c"); \
    const SymbolDatabase *db = tokenizer.getSymbolDatabase();

class TestSymbolDatabase: public TestFixture {
public:
    TestSymbolDatabase()
        :TestFixture("TestSymbolDatabase")
        ,si(NULL, NULL, NULL)
        ,vartok(NULL)
        ,typetok(NULL)
        ,t(NULL)
        ,found(false) {
    }

private:
    const Scope si;
    const Token* vartok;
    const Token* typetok;
    const Token* t;
    bool found;

    void reset() {
        vartok = NULL;
        typetok = NULL;
        t = NULL;
        found = false;
    }

    static const Scope *findFunctionScopeByToken(const SymbolDatabase * db, const Token *tok) {
        std::list<Scope>::const_iterator scope;

        for (scope = db->scopeList.begin(); scope != db->scopeList.end(); ++scope) {
            if (scope->type == Scope::eFunction) {
                if (scope->classDef == tok)
                    return &(*scope);
            }
        }
        return 0;
    }

    static const Function *findFunctionByName(const std::string& str, const Scope* startScope) {
        const Scope* currScope = startScope;
        while (currScope && currScope->isExecutable()) {
            if (currScope->functionOf)
                currScope = currScope->functionOf;
            else
                currScope = currScope->nestedIn;
        }
        while (currScope) {
            for (std::list<Function>::const_iterator i = currScope->functionList.begin(); i != currScope->functionList.end(); ++i) {
                if (i->tokenDef->str() == str)
                    return &*i;
            }
            currScope = currScope->nestedIn;
        }
        return 0;
    }

    void run() {
        TEST_CASE(array);

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
        TEST_CASE(isVariableDeclarationPointerConst);
        TEST_CASE(isVariableDeclarationRValueRef);
        TEST_CASE(isVariableStlType);

        TEST_CASE(arrayMemberVar1);
        TEST_CASE(arrayMemberVar2);
        TEST_CASE(arrayMemberVar3);
        TEST_CASE(staticMemberVar);

        TEST_CASE(hasRegularFunction);
        TEST_CASE(hasInlineClassFunction);
        TEST_CASE(hasMissingInlineClassFunction);
        TEST_CASE(hasClassFunction);

        TEST_CASE(hasRegularFunctionReturningFunctionPointer);
        TEST_CASE(hasInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasMissingInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasClassFunctionReturningFunctionPointer);
        TEST_CASE(hasSubClassConstructor);
        TEST_CASE(testConstructors);
        TEST_CASE(functionDeclarationTemplate);
        TEST_CASE(functionDeclarations);

        TEST_CASE(classWithFriend);

        TEST_CASE(parseFunctionCorrect);
        TEST_CASE(parseFunctionDeclarationCorrect);
        TEST_CASE(Cpp11InitInInitList);

        TEST_CASE(hasGlobalVariables1);
        TEST_CASE(hasGlobalVariables2);
        TEST_CASE(hasGlobalVariables3);

        TEST_CASE(checkTypeStartEndToken1);
        TEST_CASE(checkTypeStartEndToken2); // handling for unknown macro: 'void f() MACRO {..'

        TEST_CASE(functionArgs1);
        TEST_CASE(functionArgs2);
        TEST_CASE(functionArgs3);
        TEST_CASE(functionArgs4);

        TEST_CASE(namespaces1);
        TEST_CASE(namespaces2);
        TEST_CASE(namespaces3);  // #3854 - unknown macro

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
        TEST_CASE(symboldatabase25); // ticket #3561 (throw C++)
        TEST_CASE(symboldatabase26); // ticket #3561 (throw C)
        TEST_CASE(symboldatabase27); // ticket #3543 (segmentation fault)
        TEST_CASE(symboldatabase28);
        TEST_CASE(symboldatabase29); // ticket #4442 (segmentation fault)
        TEST_CASE(symboldatabase30);
        TEST_CASE(symboldatabase31);
        TEST_CASE(symboldatabase32);
        TEST_CASE(symboldatabase33); // ticket #4682 (false negatives)
        TEST_CASE(symboldatabase34); // ticket #4694 (segmentation fault)
        TEST_CASE(symboldatabase35); // ticket #4806 (segmentation fault)
        TEST_CASE(symboldatabase36); // ticket #4892 (segmentation fault)
        TEST_CASE(symboldatabase37);
        TEST_CASE(symboldatabase38); // ticket #5125 (infinite recursion)
        TEST_CASE(symboldatabase39); // ticket #5120 (infinite recursion)
        TEST_CASE(symboldatabase40); // ticket #5153
        TEST_CASE(symboldatabase41); // ticket #5197 (unknown macro)
        TEST_CASE(symboldatabase42); // only put variables in variable list

        TEST_CASE(isImplicitlyVirtual);

        TEST_CASE(garbage);

        TEST_CASE(findFunction1);
    }

    void array() const {
        std::istringstream code("int a[10+2];");
        TokenList list(NULL);
        list.createTokens(code, "test.c");
        list.front()->tokAt(2)->link(list.front()->tokAt(6));
        Variable v(list.front()->next(), list.front(), list.back(), 0, Public, NULL, NULL);
        ASSERT(v.isArray());
        ASSERT_EQUALS(1U, v.dimensions().size());
        ASSERT_EQUALS(0U, v.dimension(0));
    }

    void test_isVariableDeclarationCanHandleNull() {
        reset();
        bool result = si.isVariableDeclaration(NULL, vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
        Variable v(NULL, NULL, NULL, 0, Public, 0, 0);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x;");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration() {
        reset();
        givenACodeSampleToTokenize ScopedDeclaration("::int x;");
        bool result = si.isVariableDeclaration(ScopedDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesStdDeclaration() {
        reset();
        givenACodeSampleToTokenize StdDeclaration("std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedStdDeclaration() {
        reset();
        givenACodeSampleToTokenize StdDeclaration("::std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesManyScopes() {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE x;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesPointers() {
        reset();
        givenACodeSampleToTokenize pointer("int* p;");
        bool result1 = si.isVariableDeclaration(pointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result1);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v1(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v1.isArray());
        ASSERT(true == v1.isPointer());
        ASSERT(false == v1.isReference());

        reset();
        givenACodeSampleToTokenize constpointer("const int* p;");
        Variable v2(constpointer.tokens()->tokAt(3), constpointer.tokens()->next(), constpointer.tokens()->tokAt(2), 0, Public, 0, 0);
        ASSERT(false == v2.isArray());
        ASSERT(true == v2.isPointer());
        ASSERT(false == v2.isConst());
        ASSERT(false == v2.isReference());

        reset();
        givenACodeSampleToTokenize pointerconst("int* const p;");
        bool result2 = si.isVariableDeclaration(pointerconst.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result2);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v3(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v3.isArray());
        ASSERT(true == v3.isPointer());
        ASSERT(true == v3.isConst());
        ASSERT(false == v3.isReference());
    }

    void test_isVariableDeclarationDoesNotIdentifyConstness() {
        reset();
        givenACodeSampleToTokenize constness("const int* cp;");
        bool result = si.isVariableDeclaration(constness.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables() {
        reset();
        givenACodeSampleToTokenize multipleDeclaration("int first, second;");
        bool result = si.isVariableDeclaration(multipleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedPointerDeclaration() {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE* p;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithIndirection() {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int** pp;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("pp", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection() {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int***** p;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesArray() {
        reset();
        givenACodeSampleToTokenize arr("::std::string v[3];");
        bool result = si.isVariableDeclaration(arr.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("v", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesOfArrayPointers() {
        reset();
        givenACodeSampleToTokenize arr("A *a[5];");
        bool result = si.isVariableDeclaration(arr.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(true == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerVariable() {
        reset();
        givenACodeSampleToTokenize var("std::set<char>* chars;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("chars", vartok->str());
        ASSERT_EQUALS("set", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerToPointerVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<int>*** ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedArrayVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<int> ints[3];");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedVariable() {
        reset();
        givenACodeSampleToTokenize var("std::vector<int> ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("vector", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedVariableIterator() {
        reset();
        givenACodeSampleToTokenize var("std::list<int>::const_iterator floats;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("floats", vartok->str());
        ASSERT_EQUALS("const_iterator", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesNestedTemplateVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("intsets", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesReference() {
        reset();
        givenACodeSampleToTokenize var1("int& foo;");
        bool result1 = si.isVariableDeclaration(var1.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result1);
        Variable v1(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v1.isArray());
        ASSERT(false == v1.isPointer());
        ASSERT(true == v1.isReference());

        reset();
        givenACodeSampleToTokenize var2("foo*& bar;");
        bool result2 = si.isVariableDeclaration(var2.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result2);
        Variable v2(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v2.isArray());
        ASSERT(true == v2.isPointer());
        ASSERT(true == v2.isReference());

        reset();
        givenACodeSampleToTokenize var3("std::vector<int>& foo;");
        bool result3 = si.isVariableDeclaration(var3.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result3);
        Variable v3(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v3.isArray());
        ASSERT(false == v3.isPointer());
        ASSERT(true == v3.isReference());
    }

    void isVariableDeclarationDoesNotIdentifyTemplateClass() {
        reset();
        givenACodeSampleToTokenize var("template <class T> class SomeClass{};");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
    }

    void isVariableDeclarationPointerConst() {
        reset();
        givenACodeSampleToTokenize var("std::string const* s;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationRValueRef() {
        reset();
        givenACodeSampleToTokenize var("int&& i;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(true == v.isReference());
        ASSERT(true == v.isRValueReference());
        ASSERT(var.tokens()->tokAt(2)->scope() != 0);
    }

    void isVariableStlType() {
        {
            reset();
            std::istringstream code("std::string s;");
            TokenList list(NULL);
            list.createTokens(code, "test.cpp");
            bool result = si.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0);
            const char* types[] = { "string", "wstring" };
            const char* no_types[] = { "set" };
            ASSERT_EQUALS(true, v.isStlType());
            ASSERT_EQUALS(true, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlType(no_types));
        }
        {
            reset();
            std::istringstream code("std::vector<int> v;");
            TokenList list(NULL);
            list.createTokens(code, "test.cpp");
            list.front()->tokAt(3)->link(list.front()->tokAt(5));
            bool result = si.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0);
            const char* types[] = { "bitset", "set", "vector", "wstring" };
            const char* no_types[] = { "bitset", "map", "set" };
            ASSERT_EQUALS(true, v.isStlType());
            ASSERT_EQUALS(true, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlType(no_types));
        }
        {
            reset();
            std::istringstream code("SomeClass s;");
            TokenList list(NULL);
            list.createTokens(code, "test.cpp");
            bool result = si.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0);
            const char* types[] = { "bitset", "set", "vector" };
            ASSERT_EQUALS(false, v.isStlType());
            ASSERT_EQUALS(false, v.isStlType(types));
        }
    }

    void arrayMemberVar1() {
        const char code[] = "struct Foo {\n"
                            "    int x;\n"
                            "};\n"
                            "void f() {\n"
                            "    struct Foo foo[10];\n"
                            "    foo[1].x = 123;\n"  // <- x should get a variable() pointer
                            "}";

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        const Token *tok = Token::findmatch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : NULL;
        ASSERT(tok && tok->variable() && Token::Match(tok->variable()->typeStartToken(), "int x ;"));
        ASSERT(tok && tok->varId() == 0U); // It's possible to set a varId
    }

    void arrayMemberVar2() {
        const char code[] = "struct Foo {\n"
                            "    int x;\n"
                            "};\n"
                            "void f() {\n"
                            "    struct Foo foo[10][10];\n"
                            "    foo[1][2].x = 123;\n"  // <- x should get a variable() pointer
                            "}";

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        const Token *tok = Token::findmatch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : NULL;
        ASSERT(tok && tok->variable() && Token::Match(tok->variable()->typeStartToken(), "int x ;"));
        ASSERT(tok && tok->varId() == 0U); // It's possible to set a varId
    }

    void arrayMemberVar3() {
        const char code[] = "struct Foo {\n"
                            "    int x;\n"
                            "};\n"
                            "void f() {\n"
                            "    struct Foo foo[10];\n"
                            "    (foo[1]).x = 123;\n"  // <- x should get a variable() pointer
                            "}";

        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        const Token *tok = Token::findmatch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : NULL;
        ASSERT(tok && tok->variable() && Token::Match(tok->variable()->typeStartToken(), "int x ;"));
        ASSERT(tok && tok->varId() == 0U); // It's possible to set a varId
    }

    void staticMemberVar() {
        GET_SYMBOL_DB("class Foo {\n"
                      "    static const double d;\n"
                      "};\n"
                      "const double Foo::d = 5.0;");

        const Variable* v = db->getVariableFromVarId(1);
        ASSERT(v && db->getVariableListSize() == 2);
        ASSERT(v && v->isStatic() && v->isConst() && v->isPrivate());
    }

    void hasRegularFunction() {
        GET_SYMBOL_DB("void func() { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->next());

            ASSERT(scope && scope->className == "func");
            ASSERT(scope && scope->functionOf == 0);

            const Function *function = findFunctionByName("func", &db->scopeList.front());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->next());
            ASSERT(function && function->hasBody);
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn != scope);
            ASSERT(function && function->retDef == tokenizer.tokens());
        }
    }

    void hasInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func() { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->tokAt(4));

            ASSERT(scope && scope->className == "func");
            ASSERT(scope && scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(4));
            ASSERT(function && function->hasBody && function->isInline);
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
            ASSERT(function && function->retDef == tokenizer.tokens()->tokAt(3));

            ASSERT(db && db->findScopeByName("Fred") && db->findScopeByName("Fred")->definedType->getFunction("func") == function);
        }
    }

    void hasMissingInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->tokAt(4));

            ASSERT(scope == NULL);

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(4));
            ASSERT(function && !function->hasBody);
        }
    }

    void hasClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); }; Fred::func() { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->tokAt(12));

            ASSERT(scope && scope->className == "func");
            ASSERT(scope && scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(12));
            ASSERT(function && function->hasBody && !function->isInline);
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
        }
    }

    void hasRegularFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("void (*func(int f))(char) { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->tokAt(3));

            ASSERT(scope && scope->className == "func");

            const Function *function = findFunctionByName("func", &db->scopeList.front());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(3));
            ASSERT(function && function->hasBody && function->retFuncPtr);
        }
    }

    void hasInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char) { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->tokAt(6));

            ASSERT(scope && scope->className == "func");

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(6));
            ASSERT(function && function->hasBody && function->isInline && function->retFuncPtr);
        }
    }

    void hasMissingInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->tokAt(6));

            ASSERT(scope == NULL);

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(6));
            ASSERT(function && !function->hasBody && function->retFuncPtr);
        }
    }

    void hasClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); }; void (*Fred::func(int f))(char) { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->tokAt(23));

            ASSERT(scope && scope->className == "func");

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(23));
            ASSERT(function && function->hasBody && !function->isInline && function->retFuncPtr);
        }
    }

    void hasSubClassConstructor() {
        GET_SYMBOL_DB("class Foo { class Sub; }; class Foo::Sub { Sub() {} };");
        ASSERT(db != 0);

        if (db) {
            bool seen_something = false;
            for (std::list<Scope>::const_iterator scope = db->scopeList.begin(); scope != db->scopeList.end(); ++scope) {
                for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                    ASSERT_EQUALS("Sub", func->token->str());
                    ASSERT_EQUALS(true, func->hasBody);
                    ASSERT_EQUALS(Function::eConstructor, func->type);
                    seen_something = true;
                }
            }
            ASSERT_EQUALS(true, seen_something);
        }
    }

    void testConstructors() {
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor && !ctor->isExplicit);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { explicit Foo(Foo f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(4)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor && ctor->isExplicit);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo&& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eMoveConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
    }

    void functionDeclarationTemplate() {
        GET_SYMBOL_DB("std::map<int, string> foo() {}")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && findFunctionByName("foo", &db->scopeList.back()));

        if (db) {
            const Scope *scope = &db->scopeList.front();

            ASSERT(scope && scope->functionList.size() == 1);

            const Function *foo = &scope->functionList.front();

            ASSERT(foo && foo->token->str() == "foo");
            ASSERT(foo && foo->hasBody);
        }
    }

    void functionDeclarations() {
        GET_SYMBOL_DB("void foo();\nvoid foo();\nint foo(int i);\nvoid foo() {}")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && findFunctionByName("foo", &db->scopeList.back()));

        if (db) {
            const Scope *scope = &db->scopeList.front();

            ASSERT(scope && scope->functionList.size() == 2);

            const Function *foo = &scope->functionList.front();
            const Function *foo_int = &scope->functionList.back();

            ASSERT(foo && foo->token->str() == "foo");
            ASSERT(foo && foo->hasBody);
            ASSERT(foo && foo->token->strAt(2) == ")");

            ASSERT(foo_int && !foo_int->token);
            ASSERT(foo_int && foo_int->tokenDef->str() == "foo");
            ASSERT(foo_int && !foo_int->hasBody);
            ASSERT(foo_int && foo_int->tokenDef->strAt(2) == "int");

            ASSERT(&foo_int->argumentList.front() == db->getVariableFromVarId(1));
        }
    }

    void classWithFriend() {
        GET_SYMBOL_DB("class Foo {}; class Bar1 { friend class Foo; }; class Bar2 { friend Foo; };")
        // 3 scopes: Global, 3 classes
        ASSERT(db && db->scopeList.size() == 4);
        if (db) {
            const Scope* foo = db->findScopeByName("Foo");
            ASSERT(foo != 0);
            const Scope* bar1 = db->findScopeByName("Bar1");
            ASSERT(bar1 != 0);
            const Scope* bar2 = db->findScopeByName("Bar2");
            ASSERT(bar2 != 0);

            if (foo && bar1 && bar2) {
                ASSERT(bar1->definedType->friendList.size() == 1 && bar1->definedType->friendList.front().name == "Foo" && bar1->definedType->friendList.front().type == foo->definedType);
                ASSERT(bar2->definedType->friendList.size() == 1 && bar2->definedType->friendList.front().name == "Foo" && bar2->definedType->friendList.front().type == foo->definedType);
            }
        }
    }

    void parseFunctionCorrect() {
        // ticket 3188 - "if" statement parsed as function
        GET_SYMBOL_DB("void func(i) int i; { if (i == 1) return; }\n")
        ASSERT(db != NULL);

        // 3 scopes: Global, function, if
        ASSERT_EQUALS(3, db->scopeList.size());

        ASSERT(findFunctionByName("func", &db->scopeList.back()) != NULL);
        ASSERT(findFunctionByName("if", &db->scopeList.back()) == NULL);
    }

    void parseFunctionDeclarationCorrect() {
        GET_SYMBOL_DB("void func();\n"
                      "int bar() {}\n"
                      "void func() {}")
        ASSERT_EQUALS(3, db->findScopeByName("func")->classStart->linenr());
    }

    void Cpp11InitInInitList() {
        GET_SYMBOL_DB("class Foo {\n"
                      "    std::vector<std::string> bar;\n"
                      "    Foo() : bar({\"a\", \"b\"})\n"
                      "    {}\n"
                      "};");
        ASSERT_EQUALS(4, db->scopeList.front().nestedList.front()->nestedList.front()->classStart->linenr());
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

    void checkTypeStartEndToken1() {
        GET_SYMBOL_DB("static std::string i;\n"
                      "static const std::string j;\n"
                      "const std::string* k;\n"
                      "const char m[];\n"
                      "void f(const char* const l;) {}");

        ASSERT(db && db->getVariableListSize() == 6 && db->getVariableFromVarId(1) && db->getVariableFromVarId(2) && db->getVariableFromVarId(3) && db->getVariableFromVarId(4) && db->getVariableFromVarId(5));
        if (db && db->getVariableFromVarId(1) && db->getVariableFromVarId(2) && db->getVariableFromVarId(3) && db->getVariableFromVarId(4) && db->getVariableFromVarId(5)) {
            ASSERT_EQUALS("std", db->getVariableFromVarId(1)->typeStartToken()->str());
            ASSERT_EQUALS("std", db->getVariableFromVarId(2)->typeStartToken()->str());
            ASSERT_EQUALS("std", db->getVariableFromVarId(3)->typeStartToken()->str());
            ASSERT_EQUALS("char", db->getVariableFromVarId(4)->typeStartToken()->str());
            ASSERT_EQUALS("char", db->getVariableFromVarId(5)->typeStartToken()->str());

            ASSERT_EQUALS("string", db->getVariableFromVarId(1)->typeEndToken()->str());
            ASSERT_EQUALS("string", db->getVariableFromVarId(2)->typeEndToken()->str());
            ASSERT_EQUALS("*", db->getVariableFromVarId(3)->typeEndToken()->str());
            ASSERT_EQUALS("char", db->getVariableFromVarId(4)->typeEndToken()->str());
            ASSERT_EQUALS("*", db->getVariableFromVarId(5)->typeEndToken()->str());
        }
    }

    void checkTypeStartEndToken2() {
        GET_SYMBOL_DB("class CodeGenerator {\n"
                      "  DiagnosticsEngine Diags;\n"
                      "public:\n"
                      "  void Initialize() {\n"
                      "    Builder.reset(Diags);\n"
                      "  }\n"
                      "\n"
                      "  void HandleTagDeclRequiredDefinition() LLVM_OVERRIDE {\n"
                      "    if (Diags.hasErrorOccurred())\n"
                      "      return;\n"
                      "  }\n"
                      "};");
        ASSERT_EQUALS("DiagnosticsEngine", db->getVariableFromVarId(1)->typeStartToken()->str());
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
        tokenizer.simplifyTokenList2();

        // force symbol database creation
        tokenizer.getSymbolDatabase();
    }

    void functionArgs1() {
        {
            GET_SYMBOL_DB("void f(std::vector<std::string>, const std::vector<int> & v) { }");
            ASSERT_EQUALS(1+1, db->getVariableListSize());
            const Variable* v = db->getVariableFromVarId(1);
            ASSERT(v && v->isReference() && v->isConst() && v->isArgument());
            const Scope* f = db->findScopeByName("f");
            ASSERT(f && f->type == Scope::eFunction && f->function);
            if (f && f->function)
                ASSERT(f->function->argumentList.size() == 2 && f->function->argumentList.front().index() == 0 && f->function->argumentList.front().name() == "" && f->function->argumentList.back().index() == 1);
            ASSERT_EQUALS("", errout.str());
        }
        {
            GET_SYMBOL_DB("void g(std::map<std::string, std::vector<int> > m) { }");
            ASSERT_EQUALS(1+1, db->getVariableListSize());
            const Variable* m = db->getVariableFromVarId(1);
            ASSERT(m && !m->isReference() && !m->isConst() && m->isArgument() && m->isClass());
            const Scope* g = db->findScopeByName("g");
            ASSERT(g && g->type == Scope::eFunction && g->function && g->function->argumentList.size() == 1 && g->function->argumentList.front().index() == 0);
            ASSERT_EQUALS("", errout.str());
        }
        {
            GET_SYMBOL_DB("void g(std::map<int, int> m = std::map<int, int>()) { }");
            const Scope* g = db->findScopeByName("g");
            ASSERT(g && g->type == Scope::eFunction && g->function && g->function->argumentList.size() == 1 && g->function->argumentList.front().index() == 0 && g->function->initializedArgCount() == 1);
            ASSERT_EQUALS("", errout.str());
        }
        {
            GET_SYMBOL_DB("void g(int = 0) { }");
            const Scope* g = db->findScopeByName("g");
            ASSERT(g && g->type == Scope::eFunction && g->function && g->function->argumentList.size() == 1 && g->function->argumentList.front().hasDefault());
            ASSERT_EQUALS("", errout.str());
        }
    }

    void functionArgs2() {
        GET_SYMBOL_DB("void f(int a[][4]) { }");
        const Variable *a = db->getVariableFromVarId(1);
        ASSERT_EQUALS("a", a->nameToken()->str());
        ASSERT_EQUALS(2UL, a->dimensions().size());
        ASSERT_EQUALS(0UL, a->dimension(0));
        ASSERT_EQUALS(false, a->dimensions()[0].known);
        ASSERT_EQUALS(4UL, a->dimension(1));
        ASSERT_EQUALS(true, a->dimensions()[1].known);
    }

    void functionArgs3() {
        GET_SYMBOL_DB("void f(int i,) { }"); // Don't crash
        const Variable *a = db->getVariableFromVarId(1);
        ASSERT_EQUALS("i", a->nameToken()->str());
    }

    void functionArgs4() {
        GET_SYMBOL_DB("void f1(char [10], struct foo [10]);");
        ASSERT_EQUALS(true, db->scopeList.front().functionList.size() == 1UL);
        const Function *func = &db->scopeList.front().functionList.front();
        ASSERT_EQUALS(true, func && func->argumentList.size() == 2UL);
        if (func && func->argumentList.size() == 2UL) {
            const Variable *first = &func->argumentList.front();
            ASSERT_EQUALS(0UL, first->name().size());
            ASSERT_EQUALS(1UL, first->dimensions().size());
            ASSERT_EQUALS(10UL, first->dimension(0));
            const Variable *second = &func->argumentList.back();
            ASSERT_EQUALS(0UL, second->name().size());
            ASSERT_EQUALS(1UL, second->dimensions().size());
            ASSERT_EQUALS(10UL, second->dimension(0));
        }
    }

    void namespaces1() {
        GET_SYMBOL_DB("namespace fred {\n"
                      "    namespace barney {\n"
                      "        class X { X(int); };\n"
                      "    }\n"
                      "}\n"
                      "namespace barney { X::X(int) { } }");

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
                      "}");

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

    void namespaces3() { // #3854 - namespace with unknown macro
        GET_SYMBOL_DB("namespace fred UNKNOWN_MACRO(default) {\n"
                      "}");
        ASSERT_EQUALS(2U, db->scopeList.size());
        ASSERT_EQUALS(Scope::eGlobal, db->scopeList.front().type);
        ASSERT_EQUALS(Scope::eNamespace, db->scopeList.back().type);
    }

    void tryCatch1() {
        const std::string str("void foo() {\n"
                              "    try { }\n"
                              "    catch (const Error1 & x) { }\n"
                              "    catch (const X::Error2 & x) { }\n"
                              "    catch (Error3 x) { }\n"
                              "    catch (X::Error4 x) { }\n"
                              "}");
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
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase4() {
        check("static void function_declaration_before(void) __attribute__((__used__));\n"
              "static void function_declaration_before(void) {}\n"
              "static void function_declaration_after(void) {}\n"
              "static void function_declaration_after(void) __attribute__((__used__));\n");
        ASSERT_EQUALS("", errout.str());

        check("main(int argc, char *argv[]) { }");
        ASSERT_EQUALS("", errout.str());

        check("namespace boost {\n"
              "    std::locale generate_locale()\n"
              "    {\n"
              "        return std::locale();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("namespace X {\n"
              "    static void function_declaration_before(void) __attribute__((__used__));\n"
              "    static void function_declaration_before(void) {}\n"
              "    static void function_declaration_after(void) {}\n"
              "    static void function_declaration_after(void) __attribute__((__used__));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("testing::testing()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase5() {
        // ticket #2178 - segmentation fault
        check("int CL_INLINE_DECL(integer_decode_float) (int x) {\n"
              "    return (sign ? cl_I() : 0);\n"
              "}");
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
              "};");
        ASSERT_EQUALS("[test.cpp:1]: (debug) simplifyTemplates: bailing out\n", errout.str());
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
              "};");

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
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase11() {
        // ticket #2539 - segmentation fault
        check("int g ();\n"
              "struct S {\n"
              "  int i : (false ? g () : 1);\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase12() {
        // ticket #2547 - segmentation fault
        check("class foo {\n"
              "    void bar2 () = __null;\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase13() {
        // ticket #2577 - segmentation fault
        check("class foo {\n"
              "    void bar2 () = A::f;\n"
              "};");

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
        check("return f(){}");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase18() {
        // ticket #2865 - segmentation fault
        check("char a[1]\n");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase19() {
        // ticket #2991 - segmentation fault
        check("::y(){x}");

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
              "}");

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
        GET_SYMBOL_DB("class A { std::vector<int*> ints; };");
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

    // #ticket #3561 (throw C++)
    void symboldatabase25() {
        const std::string str("int main() {\n"
                              "    foo bar;\n"
                              "    throw bar;\n"
                              "}");
        GET_SYMBOL_DB(str.c_str());
        check(str.c_str(), false);
        ASSERT_EQUALS("", errout.str());
        ASSERT(db && db->getVariableListSize() == 2); // index 0 + 1 variable
    }

    // #ticket #3561 (throw C)
    void symboldatabase26() {
        const std::string str("int main() {\n"
                              "    throw bar;\n"
                              "}");
        GET_SYMBOL_DB_C(str.c_str());
        check(str.c_str(), false);
        ASSERT_EQUALS("", errout.str());
        ASSERT(db && db->getVariableListSize() == 2); // index 0 + 1 variable
    }

    // #ticket #3543 (segmentation fault)
    void symboldatabase27() {
        check("class C : public B1\n"
              "{\n"
              "    B1()\n"
              "    {} C(int) : B1() class\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase28() {
        GET_SYMBOL_DB("struct S {};\n"
                      "void foo(struct S s) {}");
        ASSERT(db && db->getVariableFromVarId(1) && db->getVariableFromVarId(1)->typeScope() && db->getVariableFromVarId(1)->typeScope()->className == "S");
    }

    // #ticket #4442 (segmentation fault)
    void symboldatabase29() {
        check("struct B : A {\n"
              "    B() : A {}\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase30() {
        GET_SYMBOL_DB("struct A { void foo(const int a); };\n"
                      "void A::foo(int a) { }");
        ASSERT(db && db->functionScopes.size() == 1 && db->functionScopes[0]->functionOf);
    }

    void symboldatabase31() {
        GET_SYMBOL_DB("class Foo;\n"
                      "class Bar;\n"
                      "class Sub;\n"
                      "class Foo { class Sub; };\n"
                      "class Bar { class Sub; };\n"
                      "class Bar::Sub {\n"
                      "    int b;\n"
                      "public:\n"
                      "    Sub() { }\n"
                      "    Sub(int);\n"
                      "};\n"
                      "Bar::Sub::Sub(int) { };\n"
                      "class ::Foo::Sub {\n"
                      "    int f;\n"
                      "public:\n"
                      "    ~Sub();\n"
                      "    Sub();\n"
                      "};\n"
                      "::Foo::Sub::~Sub() { }\n"
                      "::Foo::Sub::Sub() { }\n"
                      "class Foo;\n"
                      "class Bar;\n"
                      "class Sub;\n");
        ASSERT(db && db->typeList.size() == 5);
        ASSERT(db && db->isClassOrStruct("Foo"));
        ASSERT(db && db->isClassOrStruct("Bar"));
        ASSERT(db && db->isClassOrStruct("Sub"));
        if (!db || db->typeList.size() < 5)
            return;
        std::list<Type>::const_iterator i = db->typeList.begin();
        const Type* Foo = &(*i++);
        const Type* Bar = &(*i++);
        const Type* Sub = &(*i++);
        const Type* Foo_Sub = &(*i++);
        const Type* Bar_Sub = &(*i);
        ASSERT(Foo && Foo->classDef && Foo->classScope && Foo->enclosingScope && Foo->name() == "Foo");
        ASSERT(Bar && Bar->classDef && Bar->classScope && Bar->enclosingScope && Bar->name() == "Bar");
        ASSERT(Sub && Sub->classDef && !Sub->classScope && Sub->enclosingScope && Sub->name() == "Sub");
        ASSERT(Foo_Sub && Foo_Sub->classDef && Foo_Sub->classScope && Foo_Sub->enclosingScope == Foo->classScope && Foo_Sub->name() == "Sub");
        ASSERT(Bar_Sub && Bar_Sub->classDef && Bar_Sub->classScope && Bar_Sub->enclosingScope == Bar->classScope && Bar_Sub->name() == "Sub");
        ASSERT(Foo_Sub && Foo_Sub->classScope && Foo_Sub->classScope->numConstructors == 1 && Foo_Sub->classScope->className == "Sub");
        ASSERT(Bar_Sub && Bar_Sub->classScope && Bar_Sub->classScope->numConstructors == 2 && Bar_Sub->classScope->className == "Sub");
    }

    void symboldatabase32() {
        GET_SYMBOL_DB("struct Base {\n"
                      "    void foo() {}\n"
                      "};\n"
                      "class Deri : Base {\n"
                      "};");
        ASSERT(db && db->findScopeByName("Deri") && db->findScopeByName("Deri")->definedType->getFunction("foo"));
    }

    void symboldatabase33() { // ticket #4682
        GET_SYMBOL_DB("static struct A::B s;\n"
                      "static struct A::B t = { 0 };\n"
                      "static struct A::B u(0);\n"
                      "static struct A::B v{0};\n"
                      "static struct A::B w({0});\n"
                      "void foo() { }");
        ASSERT(db && db->functionScopes.size() == 1);
    }

    void symboldatabase34() { // ticket #4694
        check("typedef _Atomic(int(A::*)) atomic_mem_ptr_to_int;\n"
              "typedef _Atomic(int)&atomic_int_ref;\n"
              "struct S {\n"
              "  _Atomic union { int n; };\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase35() { // ticket #4806 and #4841
        check("class FragmentQueue : public CL_NS(util)::PriorityQueue<CL_NS(util)::Deletor::Object<TextFragment> >\n"
              "{};");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase36() { // ticket #4892
        check("void struct ( ) { if ( 1 ) } int main ( ) { }");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase37() {
        GET_SYMBOL_DB("class Fred {\n"
                      "public:\n"
                      "    struct Wilma { };\n"
                      "    struct Barney {\n"
                      "        bool operator == (const struct Barney & b) const { return true; }\n"
                      "        bool operator == (const struct Wilma & w) const { return true; }\n"
                      "    };\n"
                      "    Fred(const struct Barney & b) { barney = b; }\n"
                      "private:\n"
                      "    struct Barney barney;\n"
                      "};\n");
        ASSERT(db && db->typeList.size() == 3);
        ASSERT(db && db->isClassOrStruct("Fred"));
        ASSERT(db && db->isClassOrStruct("Wilma"));
        ASSERT(db && db->isClassOrStruct("Barney"));
        if (!db || db->typeList.size() != 3)
            return;
        std::list<Type>::const_iterator i = db->typeList.begin();
        const Type* Fred = &(*i++);
        const Type* Wilma = &(*i++);
        const Type* Barney = &(*i++);
        ASSERT(Fred && Fred->classDef && Fred->classScope && Fred->enclosingScope && Fred->name() == "Fred");
        ASSERT(Wilma && Wilma->classDef && Wilma->classScope && Wilma->enclosingScope && Wilma->name() == "Wilma");
        ASSERT(Barney && Barney->classDef && Barney->classScope && Barney->enclosingScope && Barney->name() == "Barney");
        ASSERT(db->getVariableListSize() == 5);
        if (db->getVariableListSize() != 5)
            return;
        ASSERT(db->getVariableFromVarId(1) && db->getVariableFromVarId(1)->type() && db->getVariableFromVarId(1)->type()->name() == "Barney");
        ASSERT(db->getVariableFromVarId(2) && db->getVariableFromVarId(2)->type() && db->getVariableFromVarId(2)->type()->name() == "Wilma");
        ASSERT(db->getVariableFromVarId(3) && db->getVariableFromVarId(3)->type() && db->getVariableFromVarId(3)->type()->name() == "Barney");
    }

    void symboldatabase38() { // ticket #5125
        check("template <typename T = class service> struct scoped_service;\n"
              "struct service {};\n"
              "template <> struct scoped_service<service> {};\n"
              "template <typename T>\n"
              "struct scoped_service : scoped_service<service>\n"
              "{\n"
              "  scoped_service( T* ptr ) : scoped_service<service>(ptr), m_ptr(ptr) {}\n"
              "  T* const m_ptr;\n"
              "};");
    }

    void symboldatabase39() { // ticket #5120
        check("struct V : { public case {} ; struct U : U  void { V *f (int x) (x) } }");
    }

    void symboldatabase40() { // ticket #5153
        check("void f() {\n"
              "    try {  }\n"
              "    catch (std::bad_alloc) {  }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase41() { // ticket #5197 (unknown macro)
        GET_SYMBOL_DB("struct X1 { MACRO1 f(int spd) MACRO2; };\n");
        ASSERT(db && db->findScopeByName("X1") && db->findScopeByName("X1")->functionList.size() == 1 && !db->findScopeByName("X1")->functionList.front().hasBody);
    }

    void symboldatabase42() { // only put variables in variable list
        GET_SYMBOL_DB("void f() { extern int x(); }\n");
        ASSERT(!!db);
        const Scope * const fscope = db ? db->findScopeByName("f") : NULL;
        ASSERT(!!fscope);
        ASSERT_EQUALS(0U, fscope ? fscope->varlist.size() : ~0U);  // "x" is not a variable
    }

    void isImplicitlyVirtual() {
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    virtual void foo() {}\n"
                          "};\n"
                          "class Deri : Base {\n"
                          "    void foo() {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri") && db->findScopeByName("Deri")->functionList.front().isImplicitlyVirtual());
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    virtual void foo() {}\n"
                          "};\n"
                          "class Deri1 : Base {\n"
                          "    void foo() {}\n"
                          "};\n"
                          "class Deri2 : Deri1 {\n"
                          "    void foo() {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri2") && db->findScopeByName("Deri2")->functionList.front().isImplicitlyVirtual());
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    void foo() {}\n"
                          "};\n"
                          "class Deri : Base {\n"
                          "    void foo() {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri") && !db->findScopeByName("Deri")->functionList.front().isImplicitlyVirtual(true));
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    virtual void foo() {}\n"
                          "};\n"
                          "class Deri : Base {\n"
                          "    void foo(std::string& s) {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri") && !db->findScopeByName("Deri")->functionList.front().isImplicitlyVirtual(true));
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    virtual void foo() {}\n"
                          "};\n"
                          "class Deri1 : Base {\n"
                          "    void foo(int i) {}\n"
                          "};\n"
                          "class Deri2 : Deri1 {\n"
                          "    void foo() {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri2") && db->findScopeByName("Deri2")->functionList.front().isImplicitlyVirtual());
        }
        {
            GET_SYMBOL_DB("class Base : Base2 {\n" // We don't know Base2
                          "    void foo() {}\n"
                          "};\n"
                          "class Deri : Base {\n"
                          "    void foo() {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri") && db->findScopeByName("Deri")->functionList.front().isImplicitlyVirtual(true)); // Default true -> true
        }
        {
            GET_SYMBOL_DB("class Base : Base2 {\n" // We don't know Base2
                          "    void foo() {}\n"
                          "};\n"
                          "class Deri : Base {\n"
                          "    void foo() {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri") && !db->findScopeByName("Deri")->functionList.front().isImplicitlyVirtual(false)); // Default false -> false
        }
        {
            GET_SYMBOL_DB("class Base : Base2 {\n" // We don't know Base2
                          "    virtual void foo() {}\n"
                          "};\n"
                          "class Deri : Base {\n"
                          "    void foo() {}\n"
                          "};");
            ASSERT(db && db->findScopeByName("Deri") && db->findScopeByName("Deri")->functionList.front().isImplicitlyVirtual(false)); // Default false, but we saw "virtual" -> true
        }
        // #5289
        {
            GET_SYMBOL_DB("template<>\n"
                          "class Bar<void, void> {\n"
                          "};\n"
                          "template<typename K, typename V, int KeySize>\n"
                          "class Bar : private Bar<void, void> {\n"
                          "   void foo() {\n"
                          "   }\n"
                          "};");
            ASSERT(db && db->findScopeByName("Bar") && !db->findScopeByName("Bar")->functionList.front().isImplicitlyVirtual(false));
        }

    }

    void garbage() {
        GET_SYMBOL_DB("void f( { u = 1 ; } ) { }");
        (void)db;
    }

    void findFunction1() {
        GET_SYMBOL_DB("int foo(int x);\n" /* 1 */
                      "void foo();\n"     /* 2 */
                      "void bar() {\n"    /* 3 */
                      "    foo();\n"      /* 4 */
                      "    foo(1);\n"     /* 5 */
                      "}");               /* 6 */
        ASSERT_EQUALS("", errout.str());
        if (db) {
            const Scope * bar = db->findScopeByName("bar");
            ASSERT(bar != 0);
            if (bar) {
                unsigned int linenrs[] = { 2, 1 };
                unsigned int index = 0;
                for (const Token * tok = bar->classStart->next(); tok != bar->classEnd; tok = tok->next()) {
                    if (Token::Match(tok, "%var% (") && !tok->varId() && Token::simpleMatch(tok->linkAt(1), ") ;")) {
                        const Function * function = db->findFunction(tok);
                        ASSERT(function != 0);
                        if (function) {
                            std::stringstream expected;
                            expected << "Function call on line " << tok->linenr() << " calls function on line " << linenrs[index] << std::endl;
                            std::stringstream actual;
                            actual << "Function call on line " << tok->linenr() << " calls function on line " << function->tokenDef->linenr() << std::endl;
                            ASSERT_EQUALS(expected.str().c_str(), actual.str().c_str());
                        }
                        index++;
                    }
                }
            }
        }
    }
};

REGISTER_TEST(TestSymbolDatabase)
