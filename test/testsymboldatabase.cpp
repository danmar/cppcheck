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
#include "symboldatabase.h"
#include <sstream>
#include <stdexcept>

#define GET_SYMBOL_DB(code) \
    errout.str(""); \
    Tokenizer tokenizer(&settings, this); \
    std::istringstream istr(code); \
    tokenizer.tokenize(istr, "test.cpp"); \
    const SymbolDatabase *db = tokenizer.getSymbolDatabase();

#define GET_SYMBOL_DB_C(code) \
    errout.str(""); \
    Tokenizer tokenizer(&settings, this); \
    std::istringstream istr(code); \
    tokenizer.tokenize(istr, "test.c"); \
    const SymbolDatabase *db = tokenizer.getSymbolDatabase();

class TestSymbolDatabase: public TestFixture {
public:
    TestSymbolDatabase()
        :TestFixture("TestSymbolDatabase")
        ,si(nullptr, nullptr, nullptr)
        ,vartok(nullptr)
        ,typetok(nullptr)
        ,t(nullptr)
        ,found(false) {
    }

private:
    const Scope si;
    const Token* vartok;
    const Token* typetok;
    const Token* t;
    bool found;
    Settings settings;

    void reset() {
        vartok = nullptr;
        typetok = nullptr;
        t = nullptr;
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

    static const Function *findFunctionByName(const char str[], const Scope* startScope) {
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
        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(array);
        TEST_CASE(stlarray);

        TEST_CASE(test_isVariableDeclarationCanHandleNull);
        TEST_CASE(test_isVariableDeclarationIdentifiesSimpleDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesInitialization);
        TEST_CASE(test_isVariableDeclarationIdentifiesCpp11Initialization);
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
        TEST_CASE(isVariableDeclarationDoesNotIdentifyCppCast);
        TEST_CASE(isVariableDeclarationPointerConst);
        TEST_CASE(isVariableDeclarationRValueRef);
        TEST_CASE(isVariableStlType);

        TEST_CASE(arrayMemberVar1);
        TEST_CASE(arrayMemberVar2);
        TEST_CASE(arrayMemberVar3);
        TEST_CASE(staticMemberVar);
        TEST_CASE(getVariableFromVarIdBoundsCheck);

        TEST_CASE(hasRegularFunction);
        TEST_CASE(hasInlineClassFunction);
        TEST_CASE(hasMissingInlineClassFunction);
        TEST_CASE(hasClassFunction);

        TEST_CASE(hasRegularFunctionReturningFunctionPointer);
        TEST_CASE(hasInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasMissingInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasClassFunctionReturningFunctionPointer);
        TEST_CASE(complexFunctionArrayPtr);
        TEST_CASE(pointerToMemberFunction);
        TEST_CASE(hasSubClassConstructor);
        TEST_CASE(testConstructors);
        TEST_CASE(functionDeclarationTemplate);
        TEST_CASE(functionDeclarations);
        TEST_CASE(constructorInitialization);
        TEST_CASE(memberFunctionOfUnknownClassMacro1);
        TEST_CASE(memberFunctionOfUnknownClassMacro2);
        TEST_CASE(memberFunctionOfUnknownClassMacro3);
        TEST_CASE(functionLinkage);

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
        TEST_CASE(symboldatabase43); // #4738
        TEST_CASE(symboldatabase44);
        TEST_CASE(symboldatabase45); // #6125
        TEST_CASE(symboldatabase46); // #6171 (anonymous namespace)
        TEST_CASE(symboldatabase47); // #6308
        TEST_CASE(symboldatabase48); // #6417
        TEST_CASE(symboldatabase49); // #6424
        TEST_CASE(symboldatabase50); // #6432
        TEST_CASE(symboldatabase51); // #6538

        TEST_CASE(isImplicitlyVirtual);

        TEST_CASE(isFunction); // UNKNOWN_MACRO(a,b) { .. }

        TEST_CASE(findFunction1);
        TEST_CASE(findFunction2); // mismatch: parameter passed by address => reference argument
        TEST_CASE(findFunction3);
        TEST_CASE(findFunction4);
        TEST_CASE(findFunction5); // #6230
        TEST_CASE(findFunction6);

        TEST_CASE(noexceptFunction1);
        TEST_CASE(noexceptFunction2);
        TEST_CASE(noexceptFunction3);
        TEST_CASE(noexceptFunction4);

        TEST_CASE(throwFunction1);
        TEST_CASE(throwFunction2);

        TEST_CASE(nothrowAttributeFunction);
        TEST_CASE(nothrowDeclspecFunction);

        TEST_CASE(varTypesIntegral); // known integral
        TEST_CASE(varTypesFloating); // known floating
        TEST_CASE(varTypesOther);    // (un)known

        TEST_CASE(functionPrototype); // ticket #5867

        TEST_CASE(lambda); // ticket #5867
    }

    void array() const {
        std::istringstream code("int a[10+2];");
        TokenList list(nullptr);
        list.createTokens(code, "test.c");
        list.front()->tokAt(2)->link(list.front()->tokAt(6));
        Variable v(list.front()->next(), list.front(), list.back(), 0, Public, nullptr, nullptr, &settings.library);
        ASSERT(v.isArray());
        ASSERT_EQUALS(1U, v.dimensions().size());
        ASSERT_EQUALS(0U, v.dimension(0));
    }

    void stlarray() const {
        std::istringstream code("std::array<int, 20> arr;");
        TokenList list(nullptr);
        list.createTokens(code, "test.c");
        list.front()->tokAt(3)->link(list.front()->tokAt(7));
        Variable v(list.front()->next(), list.front(), list.back(), 0, Public, nullptr, nullptr, &settings.library);
        ASSERT(v.isArray());
        ASSERT_EQUALS(1U, v.dimensions().size());
        ASSERT_EQUALS(20U, v.dimension(0));
    }

    void test_isVariableDeclarationCanHandleNull() {
        reset();
        bool result = si.isVariableDeclaration(nullptr, vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(nullptr == vartok);
        ASSERT(nullptr == typetok);
        Variable v(nullptr, nullptr, nullptr, 0, Public, 0, 0, &settings.library);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x;");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesInitialization() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x (1);");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
        ASSERT(true == v.isIntegralType());
    }

    void test_isVariableDeclarationIdentifiesCpp11Initialization() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x {1};");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
        ASSERT(true == v.isIntegralType());
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration() {
        reset();
        givenACodeSampleToTokenize ScopedDeclaration("::int x;");
        bool result = si.isVariableDeclaration(ScopedDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v1(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v1.isArray());
        ASSERT(true == v1.isPointer());
        ASSERT(false == v1.isReference());

        reset();
        givenACodeSampleToTokenize constpointer("const int* p;");
        Variable v2(constpointer.tokens()->tokAt(3), constpointer.tokens()->next(), constpointer.tokens()->tokAt(2), 0, Public, 0, 0, &settings.library);
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
        Variable v3(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        ASSERT(nullptr == vartok);
        ASSERT(nullptr == typetok);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables() {
        reset();
        givenACodeSampleToTokenize multipleDeclaration("int first, second;");
        bool result = si.isVariableDeclaration(multipleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesReference() {
        reset();
        givenACodeSampleToTokenize var1("int& foo;");
        bool result1 = si.isVariableDeclaration(var1.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result1);
        Variable v1(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v1.isArray());
        ASSERT(false == v1.isPointer());
        ASSERT(true == v1.isReference());

        reset();
        givenACodeSampleToTokenize var2("foo*& bar;");
        bool result2 = si.isVariableDeclaration(var2.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result2);
        Variable v2(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v2.isArray());
        ASSERT(true == v2.isPointer());
        ASSERT(true == v2.isReference());

        reset();
        givenACodeSampleToTokenize var3("std::vector<int>& foo;");
        bool result3 = si.isVariableDeclaration(var3.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result3);
        Variable v3(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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

    void isVariableDeclarationDoesNotIdentifyCppCast() {
        reset();
        givenACodeSampleToTokenize var("reinterpret_cast <char *> (code)[0] = 0;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
    }

    void isVariableDeclarationPointerConst() {
        reset();
        givenACodeSampleToTokenize var("std::string const* s;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationRValueRef() {
        reset();
        givenACodeSampleToTokenize var("int&& i;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings.library);
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
            TokenList list(nullptr);
            list.createTokens(code, "test.cpp");
            bool result = si.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0, &settings.library);
            const char* types[] = { "string", "wstring" };
            const char* no_types[] = { "set" };
            ASSERT_EQUALS(true, v.isStlType());
            ASSERT_EQUALS(true, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlType(no_types));
            ASSERT_EQUALS(true, v.isStlStringType());
        }
        {
            reset();
            std::istringstream code("std::vector<int> v;");
            TokenList list(nullptr);
            list.createTokens(code, "test.cpp");
            list.front()->tokAt(3)->link(list.front()->tokAt(5));
            bool result = si.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0, &settings.library);
            const char* types[] = { "bitset", "set", "vector", "wstring" };
            const char* no_types[] = { "bitset", "map", "set" };
            ASSERT_EQUALS(true, v.isStlType());
            ASSERT_EQUALS(true, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlType(no_types));
            ASSERT_EQUALS(false, v.isStlStringType());
        }
        {
            reset();
            std::istringstream code("SomeClass s;");
            TokenList list(nullptr);
            list.createTokens(code, "test.cpp");
            bool result = si.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0, &settings.library);
            const char* types[] = { "bitset", "set", "vector" };
            ASSERT_EQUALS(false, v.isStlType());
            ASSERT_EQUALS(false, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlStringType());
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

        Settings localsettings;
        Tokenizer tokenizer(&localsettings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : nullptr;
        ASSERT(tok && tok->variable() && Token::simpleMatch(tok->variable()->typeStartToken(), "int x ;"));
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

        Settings localsettings;
        Tokenizer tokenizer(&localsettings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : nullptr;
        ASSERT(tok && tok->variable() && Token::simpleMatch(tok->variable()->typeStartToken(), "int x ;"));
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

        Settings localsettings;
        Tokenizer tokenizer(&localsettings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : nullptr;
        ASSERT(tok && tok->variable() && Token::simpleMatch(tok->variable()->typeStartToken(), "int x ;"));
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

    void getVariableFromVarIdBoundsCheck() {
        GET_SYMBOL_DB("int x;\n"
                      "int y;\n");

        const Variable* v = db->getVariableFromVarId(2);
        // three elements: varId 0 also counts via a fake-entry
        ASSERT(v && db->getVariableListSize() == 3);

        ASSERT_THROW(db->getVariableFromVarId(3), std::out_of_range);
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
            ASSERT(function && function->hasBody());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn != scope);
            ASSERT(function && function->retDef == tokenizer.tokens());
        }
    }

    void hasInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func() { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "func");
            ASSERT(scope && scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && function->isInline());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
            ASSERT(function && function->retDef == functionToken->previous());

            ASSERT(db && db->findScopeByName("Fred") && db->findScopeByName("Fred")->definedType->getFunction("func") == function);
        }
    }

    void hasMissingInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope == nullptr);

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && !function->hasBody());
        }
    }

    void hasClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); }; void Fred::func() { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens()->linkAt(2), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "func");
            ASSERT(scope && scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && !function->isInline());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
        }
    }

    void hasRegularFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("void (*func(int f))(char) { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "func");

            const Function *function = findFunctionByName("func", &db->scopeList.front());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody());
        }
    }

    void hasInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char) { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "func");

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && function->isInline());
        }
    }

    void hasMissingInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope == nullptr);

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && !function->hasBody());
        }
    }

    void hasClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); }; void (*Fred::func(int f))(char) { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens()->linkAt(2), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "func");

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && !function->isInline());
        }
    }

    void complexFunctionArrayPtr() {
        GET_SYMBOL_DB("int(*p1)[10]; \n"                            // pointer to array 10 of int
                      "void(*p2)(char); \n"                         // pointer to function (char) returning void
                      "int(*(*p3)(char))[10];\n"                    // pointer to function (char) returning pointer to array 10 of int
                      "float(*(*p4)(char))(long); \n"               // pointer to function (char) returning pointer to function (long) returning float
                      "short(*(*(p5) (char))(long))(double); \n"    // pointer to function (char) returning pointer to function (long) returning pointer to function (double) returning short
                      "int(*a1[10])(void); \n"                      // array 10 of pointer to function (void) returning int
                      "float(*(*a2[10])(char))(long);\n"            // array 10 of pointer to func (char) returning pointer to func (long) returning float
                      "short(*(*(*a3[10])(char))(long))(double);\n" // array 10 of pointer to function (char) returning pointer to function (long) returning pointer to function (double) returning short
                      "::boost::rational(&r_)[9];\n"                // reference to array of ::boost::rational
                      "::boost::rational<T>(&r_)[9];");             // reference to array of ::boost::rational<T> (template!)

        ASSERT(db != nullptr);

        if (db) {
            ASSERT_EQUALS(10, db->getVariableListSize() - 1);
            ASSERT_EQUALS(true, db->getVariableFromVarId(1) && db->getVariableFromVarId(1)->dimensions().size() == 1);
            ASSERT_EQUALS(true, db->getVariableFromVarId(2) != nullptr);
            ASSERT_EQUALS(true, db->getVariableFromVarId(3) && db->getVariableFromVarId(3)->dimensions().size() == 0);
            ASSERT_EQUALS(true, db->getVariableFromVarId(4) != nullptr);
            ASSERT_EQUALS(true, db->getVariableFromVarId(5) != nullptr);
            ASSERT_EQUALS(true, db->getVariableFromVarId(6) && db->getVariableFromVarId(6)->dimensions().size() == 1);
            ASSERT_EQUALS(true, db->getVariableFromVarId(7) && db->getVariableFromVarId(7)->dimensions().size() == 1);
            ASSERT_EQUALS(true, db->getVariableFromVarId(8) && db->getVariableFromVarId(8)->dimensions().size() == 1);
            ASSERT_EQUALS(true, db->getVariableFromVarId(9) && db->getVariableFromVarId(9)->dimensions().size() == 1);
            ASSERT_EQUALS(true, db->getVariableFromVarId(10) && db->getVariableFromVarId(10)->dimensions().size() == 1);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void pointerToMemberFunction() {
        GET_SYMBOL_DB("bool (A::*pFun)();"); // Pointer to member function of A, returning bool and taking no parameters

        ASSERT(db != nullptr);

        if (db) {
            ASSERT_EQUALS(1, db->getVariableListSize() - 1);
            ASSERT_EQUALS(true, db->getVariableFromVarId(1) != nullptr);
            if (db->getVariableFromVarId(1))
                ASSERT_EQUALS("pFun", db->getVariableFromVarId(1)->name());
            ASSERT_EQUALS("", errout.str());
        }
    }

    void hasSubClassConstructor() {
        GET_SYMBOL_DB("class Foo { class Sub; }; class Foo::Sub { Sub() {} };");
        ASSERT(db != nullptr);

        if (db) {
            bool seen_something = false;
            for (std::list<Scope>::const_iterator scope = db->scopeList.begin(); scope != db->scopeList.end(); ++scope) {
                for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                    ASSERT_EQUALS("Sub", func->token->str());
                    ASSERT_EQUALS(true, func->hasBody());
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
            ASSERT(db && ctor && ctor->type == Function::eConstructor && !ctor->isExplicit());
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { explicit Foo(Foo f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(4)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor && ctor->isExplicit());
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
            ASSERT(foo && foo->hasBody());
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
            ASSERT(foo && foo->hasBody());
            ASSERT(foo && foo->token->strAt(2) == ")");

            ASSERT(foo_int && !foo_int->token);
            ASSERT(foo_int && foo_int->tokenDef->str() == "foo");
            ASSERT(foo_int && !foo_int->hasBody());
            ASSERT(foo_int && foo_int->tokenDef->strAt(2) == "int");

            ASSERT(&foo_int->argumentList.front() == db->getVariableFromVarId(1));
        }
    }

    void constructorInitialization() {
        GET_SYMBOL_DB("std::string logfile;\n"
                      "std::ofstream log(logfile.c_str(), std::ios::out);");

        // 1 scope: Global
        ASSERT(db && db->scopeList.size() == 1);

        if (db && !db->scopeList.empty()) {
            // No functions
            ASSERT(db->scopeList.front().functionList.empty());
        }
    }

    void memberFunctionOfUnknownClassMacro1() {
        GET_SYMBOL_DB("class ScVbaFormatCondition { OUString getServiceImplName() SAL_OVERRIDE; };\n"
                      "void ScVbaValidation::getFormula1() {\n"
                      "    sal_uInt16 nFlags = 0;\n"
                      "    if (pDocSh && !getCellRangesForAddress(nFlags)) ;\n"
                      "}");

        ASSERT(db && errout.str() == "");

        if (db) {
            const Scope *scope = db->findScopeByName("getFormula1");
            ASSERT(scope != nullptr);
            ASSERT(scope && scope->nestedIn == &db->scopeList.front());
        }
    }

    void memberFunctionOfUnknownClassMacro2() {
        GET_SYMBOL_DB("class ScVbaFormatCondition { OUString getServiceImplName() SAL_OVERRIDE {} };\n"
                      "void getFormula1() {\n"
                      "    sal_uInt16 nFlags = 0;\n"
                      "    if (pDocSh && !getCellRangesForAddress(nFlags)) ;\n"
                      "}");

        ASSERT(db && errout.str() == "");

        if (db) {
            const Scope *scope = db->findScopeByName("getFormula1");
            ASSERT(scope != nullptr);
            ASSERT(scope && scope->nestedIn == &db->scopeList.front());

            scope = db->findScopeByName("getServiceImplName");
            ASSERT(scope != nullptr);
            ASSERT(scope && scope->nestedIn && scope->nestedIn->className == "ScVbaFormatCondition");
        }
    }

    void memberFunctionOfUnknownClassMacro3() {
        GET_SYMBOL_DB("class ScVbaFormatCondition { OUString getServiceImplName() THROW(whatever); };\n"
                      "void ScVbaValidation::getFormula1() {\n"
                      "    sal_uInt16 nFlags = 0;\n"
                      "    if (pDocSh && !getCellRangesForAddress(nFlags)) ;\n"
                      "}");

        ASSERT(db && errout.str() == "");

        if (db) {
            const Scope *scope = db->findScopeByName("getFormula1");
            ASSERT(scope != nullptr);
            ASSERT(scope && scope->nestedIn == &db->scopeList.front());
        }
    }

    void functionLinkage() {
        GET_SYMBOL_DB("static void f1() { }\n"
                      "void f2();\n"
                      "extern void f3();\n"
                      "void f4();\n"
                      "extern void f5() { };\n"
                      "void f6() { }");

        ASSERT(db && errout.str() == "");

        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "f1");
            ASSERT(f && f->function() && f->function()->isStaticLocal());

            f = Token::findsimplematch(tokenizer.tokens(), "f2");
            ASSERT(f && f->function() && !f->function()->isStaticLocal());

            f = Token::findsimplematch(tokenizer.tokens(), "f3");
            ASSERT(f && f->function() && f->function()->isExtern());

            f = Token::findsimplematch(tokenizer.tokens(), "f4");
            ASSERT(f && f->function() && !f->function()->isExtern());

            f = Token::findsimplematch(tokenizer.tokens(), "f5");
            ASSERT(f && f->function() && f->function()->isExtern());

            f = Token::findsimplematch(tokenizer.tokens(), "f6");
            ASSERT(f && f->function() && !f->function()->isExtern());
        }
    }

    void classWithFriend() {
        GET_SYMBOL_DB("class Foo {}; class Bar1 { friend class Foo; }; class Bar2 { friend Foo; };")
        // 3 scopes: Global, 3 classes
        ASSERT(db && db->scopeList.size() == 4);
        if (db) {
            const Scope* foo = db->findScopeByName("Foo");
            ASSERT(foo != nullptr);
            const Scope* bar1 = db->findScopeByName("Bar1");
            ASSERT(bar1 != nullptr);
            const Scope* bar2 = db->findScopeByName("Bar2");
            ASSERT(bar2 != nullptr);

            if (foo && bar1 && bar2) {
                ASSERT(bar1->definedType->friendList.size() == 1 && bar1->definedType->friendList.front().name == "Foo" && bar1->definedType->friendList.front().type == foo->definedType);
                ASSERT(bar2->definedType->friendList.size() == 1 && bar2->definedType->friendList.front().name == "Foo" && bar2->definedType->friendList.front().type == foo->definedType);
            }
        }
    }

    void parseFunctionCorrect() {
        // ticket 3188 - "if" statement parsed as function
        GET_SYMBOL_DB("void func(i) int i; { if (i == 1) return; }\n")
        ASSERT(db != nullptr);

        // 3 scopes: Global, function, if
        ASSERT_EQUALS(3, db->scopeList.size());

        ASSERT(findFunctionByName("func", &db->scopeList.back()) != nullptr);
        ASSERT(findFunctionByName("if", &db->scopeList.back()) == nullptr);
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
        Settings localsettings;
        localsettings.debugwarnings = debug;

        // Tokenize..
        Tokenizer tokenizer(&localsettings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // force symbol database creation
        tokenizer.createSymbolDatabase();
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
        {
            GET_SYMBOL_DB("void g(int* const) { }"); // 'const' is not the name of the variable - #5882
            const Scope* g = db->findScopeByName("g");
            ASSERT(g && g->type == Scope::eFunction && g->function && g->function->argumentList.size() == 1 && g->function->argumentList.front().nameToken() == nullptr);
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
        const Scope *scope = nullptr;
        for (std::list<Scope>::const_iterator it = db->scopeList.begin(); it != db->scopeList.end(); ++it) {
            if (it->isClassOrStruct()) {
                scope = &(*it);
                break;
            }
        }

        ASSERT(scope != nullptr);
        if (!scope)
            return;

        ASSERT_EQUALS("X", scope->className);

        // The class has a constructor but the implementation _is not_ seen
        ASSERT_EQUALS(1U, scope->functionList.size());
        const Function *function = &(scope->functionList.front());
        ASSERT_EQUALS(false, function->hasBody());
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
        const Scope *scope = nullptr;
        for (std::list<Scope>::const_iterator it = db->scopeList.begin(); it != db->scopeList.end(); ++it) {
            if (it->isClassOrStruct()) {
                scope = &(*it);
                break;
            }
        }

        ASSERT(scope != nullptr);
        if (!scope)
            return;

        ASSERT_EQUALS("X", scope->className);

        // The class has a constructor and the implementation _is_ seen
        ASSERT_EQUALS(1U, scope->functionList.size());
        const Function *function = &(scope->functionList.front());
        ASSERT_EQUALS("X", function->tokenDef->str());
        ASSERT_EQUALS(true, function->hasBody());
    }

    void namespaces3() { // #3854 - namespace with unknown macro
        GET_SYMBOL_DB("namespace fred UNKNOWN_MACRO(default) {\n"
                      "}");
        ASSERT_EQUALS(2U, db->scopeList.size());
        ASSERT_EQUALS(Scope::eGlobal, db->scopeList.front().type);
        ASSERT_EQUALS(Scope::eNamespace, db->scopeList.back().type);
    }

    void tryCatch1() {
        const char str[] = "void foo() {\n"
                           "    try { }\n"
                           "    catch (const Error1 & x) { }\n"
                           "    catch (const X::Error2 & x) { }\n"
                           "    catch (Error3 x) { }\n"
                           "    catch (X::Error4 x) { }\n"
                           "}";
        GET_SYMBOL_DB(str);
        check(str, false);
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
        ASSERT_THROW(check("struct B : A\n"), InternalError);
    }

    void symboldatabase15() {
        // ticket #2591 - segmentation fault
        ASSERT_THROW(check("struct A :\n"), InternalError);
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
        ASSERT_THROW(check("struct x : virtual y\n"), InternalError);
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
        const Scope *fredScope = nullptr;
        for (std::list<Scope>::const_iterator scope = db->scopeList.begin(); scope != db->scopeList.end(); ++scope) {
            if (scope->isClassOrStruct() && scope->className == "Fred")
                fredScope = &(*scope);
        }
        ASSERT(fredScope != nullptr);
        if (fredScope == nullptr)
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
        const char str[] = "int main() {\n"
                           "    foo bar;\n"
                           "    throw bar;\n"
                           "}";
        GET_SYMBOL_DB(str);
        check(str, false);
        ASSERT_EQUALS("", errout.str());
        ASSERT(db && db->getVariableListSize() == 2); // index 0 + 1 variable
    }

    // #ticket #3561 (throw C)
    void symboldatabase26() {
        const char str[] = "int main() {\n"
                           "    throw bar;\n"
                           "}";
        GET_SYMBOL_DB_C(str);
        check(str, false);
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
        ASSERT(db && db->findScopeByName("X1") && db->findScopeByName("X1")->functionList.size() == 1 && !db->findScopeByName("X1")->functionList.front().hasBody());
    }

    void symboldatabase42() { // only put variables in variable list
        GET_SYMBOL_DB("void f() { extern int x(); }\n");
        ASSERT(!!db);
        const Scope * const fscope = db ? db->findScopeByName("f") : nullptr;
        ASSERT(!!fscope);
        ASSERT_EQUALS(0U, fscope ? fscope->varlist.size() : ~0U);  // "x" is not a variable
    }

    void symboldatabase43() { // ticket #4738
        check("void f() {\n"
              "    new int;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase44() {
        GET_SYMBOL_DB("int i { 1 };\n"
                      "int j ( i );\n"
                      "void foo() {\n"
                      "    int k { 1 };\n"
                      "    int l ( 1 );\n"
                      "}");
        ASSERT(db != nullptr);
        ASSERT_EQUALS(4U, db->getVariableListSize() - 1);
        ASSERT_EQUALS(2U, db->scopeList.size());
        for (std::size_t i = 1U; i < db->getVariableListSize(); i++)
            ASSERT(db->getVariableFromVarId(i) != nullptr);
    }

    void symboldatabase45() {
        GET_SYMBOL_DB("typedef struct {\n"
                      "    unsigned long bits;\n"
                      "} S;\n"
                      "struct T {\n"
                      "    S span;\n"
                      "    int flags;\n"
                      "};\n"
                      "struct T f(int x) {\n"
                      "    return (struct T) {\n"
                      "        .span = (S) { 0UL },\n"
                      "        .flags = (x ? 256 : 0),\n"
                      "    };\n"
                      "}");

        ASSERT(db != nullptr);
        ASSERT_EQUALS(4U, db->getVariableListSize() - 1);
        for (std::size_t i = 1U; i < db->getVariableListSize(); i++)
            ASSERT(db->getVariableFromVarId(i) != nullptr);

        ASSERT_EQUALS(4U, db->scopeList.size());
        std::list<Scope>::const_iterator scope = db->scopeList.begin();
        ASSERT_EQUALS(Scope::eGlobal, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eStruct, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eStruct, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eFunction, scope->type);
    }

    void symboldatabase46() { // #6171 (anonymous namespace)
        GET_SYMBOL_DB("struct S { };\n"
                      "namespace {\n"
                      "    struct S { };\n"
                      "}");

        ASSERT(db != nullptr);
        ASSERT_EQUALS(4U, db->scopeList.size());
        std::list<Scope>::const_iterator scope = db->scopeList.begin();
        ASSERT_EQUALS(Scope::eGlobal, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eStruct, scope->type);
        ASSERT_EQUALS(scope->className, "S");
        ++scope;
        ASSERT_EQUALS(Scope::eNamespace, scope->type);
        ASSERT_EQUALS(scope->className, "");
        ++scope;
        ASSERT_EQUALS(Scope::eStruct, scope->type);
        ASSERT_EQUALS(scope->className, "S");
    }

    void symboldatabase47() { // #6308 - associate Function and Scope for destructors
        GET_SYMBOL_DB("namespace NS {\n"
                      "    class MyClass {\n"
                      "        ~MyClass();\n"
                      "    };\n"
                      "}\n"
                      "using namespace NS;\n"
                      "MyClass::~MyClass() {\n"
                      "    delete Example;\n"
                      "}");
        ASSERT(db && !db->functionScopes.empty() && db->functionScopes.front()->function && db->functionScopes.front()->function->functionScope == db->functionScopes.front());
    }

    void symboldatabase48() { // #6417
        GET_SYMBOL_DB("namespace NS {\n"
                      "    class MyClass {\n"
                      "        MyClass();\n"
                      "        ~MyClass();\n"
                      "    };\n"
                      "}\n"
                      "using namespace NS;\n"
                      "MyClass::~MyClass() { }\n"
                      "MyClass::MyClass() { }\n");
        ASSERT(db && !db->functionScopes.empty() && db->functionScopes.front()->function && db->functionScopes.front()->function->functionScope == db->functionScopes.front());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "MyClass ( ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3  && f->function()->token->linenr() == 9);

        f = Token::findsimplematch(tokenizer.tokens(), "~ MyClass ( ) ;");
        f = f->next();
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4  && f->function()->token->linenr() == 8);
    }

    void symboldatabase49() { // #6424
        GET_SYMBOL_DB("namespace Ns { class C; }\n"
                      "void f1() { char *p; *p = 0; }\n"
                      "class Ns::C* p;\n"
                      "void f2() { char *p; *p = 0; }\n");
        ASSERT(db != nullptr);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "p ; void f2");
        ASSERT_EQUALS(true, db && f && f->variable());
        f = Token::findsimplematch(tokenizer.tokens(), "f2");
        ASSERT_EQUALS(true, db && f && f->function());
    }

    void symboldatabase50() { // #6432
        GET_SYMBOL_DB("template <bool del, class T>\n"
                      "class _ConstTessMemberResultCallback_0_0<del, void, T>\n"
                      "   {\n"
                      " public:\n"
                      "  typedef void (T::*MemberSignature)() const;\n"
                      "\n"
                      " private:\n"
                      "  const T* object_;\n"
                      "  MemberSignature member_;\n"
                      "\n"
                      " public:\n"
                      "  inline _ConstTessMemberResultCallback_0_0(\n"
                      "      const T* object, MemberSignature member)\n"
                      "    : object_(object),\n"
                      "      member_(member) {\n"
                      "  }\n"
                      "};");
        ASSERT(db != nullptr);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "_ConstTessMemberResultCallback_0_0 (");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->isConstructor());
    }

    void symboldatabase51() { // #6538
        GET_SYMBOL_DB("static const float f1 = 2 * foo1(a, b);\n"
                      "static const float f2 = 2 * ::foo2(a, b);\n"
                      "static const float f3 = 2 * std::foo3(a, b);\n"
                      "static const float f4 = c * foo4(a, b);\n"
                      "static const int i1 = 2 & foo5(a, b);\n"
                      "static const bool b1 = 2 > foo6(a, b);\n");
        ASSERT(db != nullptr);
        if (db) {
            ASSERT(findFunctionByName("foo1", &db->scopeList.front()) == nullptr);
            ASSERT(findFunctionByName("foo2", &db->scopeList.front()) == nullptr);
            ASSERT(findFunctionByName("foo3", &db->scopeList.front()) == nullptr);
            ASSERT(findFunctionByName("foo4", &db->scopeList.front()) == nullptr);
            ASSERT(findFunctionByName("foo5", &db->scopeList.front()) == nullptr);
            ASSERT(findFunctionByName("foo6", &db->scopeList.front()) == nullptr);
        }
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
            if (db)
                ASSERT_EQUALS(1, db->findScopeByName("Bar")->functionList.size());
        }

        // #5590
        {
            GET_SYMBOL_DB("class InfiniteB : InfiniteA {\n"
                          "    class D {\n"
                          "    };\n"
                          "};\n"
                          "namespace N {\n"
                          "    class InfiniteA : InfiniteB {\n"
                          "    };\n"
                          "}\n"
                          "class InfiniteA : InfiniteB {\n"
                          "    void foo();\n"
                          "};\n"
                          "void InfiniteA::foo() {\n"
                          "    C a;\n"
                          "}");
            //ASSERT(db && db->findScopeByName("InfiniteA") && !db->findScopeByName("InfiniteA")->functionList.front().isImplicitlyVirtual());
            TODO_ASSERT_EQUALS(1, 0, db->findScopeByName("InfiniteA")->functionList.size());
        }
    }

    void isFunction() { // #5602 - UNKNOWN_MACRO(a,b) { .. }
        GET_SYMBOL_DB("TEST(a,b) {\n"
                      "  std::vector<int> messages;\n"
                      "  foo(messages[2].size());\n"
                      "}");
        const Variable * const var = db ? db->getVariableFromVarId(1U) : nullptr;
        ASSERT(db &&
               db->findScopeByName("TEST") &&
               var &&
               var->typeStartToken() &&
               var->typeStartToken()->str() == "std");
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
            ASSERT(bar != nullptr);
            if (bar) {
                unsigned int linenrs[] = { 2, 1 };
                unsigned int index = 0;
                for (const Token * tok = bar->classStart->next(); tok != bar->classEnd; tok = tok->next()) {
                    if (Token::Match(tok, "%name% (") && !tok->varId() && Token::simpleMatch(tok->linkAt(1), ") ;")) {
                        const Function * function = db->findFunction(tok);
                        ASSERT(function != nullptr);
                        if (function) {
                            std::stringstream expected;
                            expected << "Function call on line " << tok->linenr() << " calls function on line " << linenrs[index] << std::endl;
                            std::stringstream actual;
                            actual << "Function call on line " << tok->linenr() << " calls function on line " << function->tokenDef->linenr() << std::endl;
                            ASSERT_EQUALS(expected.str(), actual.str());
                        }
                        index++;
                    }
                }
            }
        }
    }

    void findFunction2() {
        // The function does not match the function call.
        GET_SYMBOL_DB("void func(const int x, const Fred &fred);\n"
                      "void otherfunc() {\n"
                      "    float t;\n"
                      "    func(x, &t);\n"
                      "}");
        const Token *callfunc = Token::findsimplematch(tokenizer.tokens(), "func ( x , & t ) ;");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null
        ASSERT_EQUALS(true,  callfunc != nullptr); // not null
        ASSERT_EQUALS(false, (callfunc && callfunc->function())); // callfunc->function() should be null
    }

    void findFunction3() {
        GET_SYMBOL_DB("struct base { void foo() { } };\n"
                      "struct derived : public base { void foo() { } };\n"
                      "void foo() {\n"
                      "    derived d;\n"
                      "    d.foo();\n"
                      "}");

        const Token *callfunc = Token::findsimplematch(tokenizer.tokens(), "d . foo ( ) ;");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true, db != nullptr); // not null
        ASSERT_EQUALS(true, callfunc != nullptr); // not null
        ASSERT_EQUALS(true, callfunc && callfunc->tokAt(2)->function() && callfunc->tokAt(2)->function()->tokenDef->linenr() == 2); // should find function on line 2
    }

    void findFunction4() {
        GET_SYMBOL_DB("void foo(UNKNOWN) { }\n"
                      "void foo(int a) { }\n"
                      "void foo(unsigned int a) { }\n"
                      "void foo(unsigned long a) { }\n"
                      "void foo(unsigned long long a) { }\n"
                      "void foo(float a) { }\n"
                      "void foo(double a) { }\n"
                      "void foo(long double a) { }\n"
                      "int i;\n"
                      "unsigned int ui;\n"
                      "unsigned long ul;\n"
                      "unsigned long long ull;\n"
                      "float f;\n"
                      "double d;\n"
                      "long double ld;\n"
                      "int & ri = i;\n"
                      "unsigned int & rui = ui;\n"
                      "unsigned long & rul = ul;\n"
                      "unsigned long long & rull = ull;\n"
                      "float & rf = f;\n"
                      "double & rd = d;\n"
                      "long double & rld = ld;\n"
                      "const int & cri = i;\n"
                      "const unsigned int & crui = ui;\n"
                      "const unsigned long & crul = ul;\n"
                      "const unsigned long long & crull = ull;\n"
                      "const float & crf = f;\n"
                      "const double & crd = d;\n"
                      "const long double & crld = ld;\n"
                      "void foo() {\n"
                      "    foo(1);\n"
                      "    foo(1U);\n"
                      "    foo(1UL);\n"
                      "    foo(1ULL);\n"
                      "    foo(1.0F);\n"
                      "    foo(1.0);\n"
                      "    foo(1.0L);\n"
                      "    foo(i);\n"
                      "    foo(ui);\n"
                      "    foo(ul);\n"
                      "    foo(ull);\n"
                      "    foo(f);\n"
                      "    foo(d);\n"
                      "    foo(ld);\n"
                      "    foo(ri);\n"
                      "    foo(rui);\n"
                      "    foo(rul);\n"
                      "    foo(rull);\n"
                      "    foo(rf);\n"
                      "    foo(rd);\n"
                      "    foo(rld);\n"
                      "    foo(cri);\n"
                      "    foo(crui);\n"
                      "    foo(crul);\n"
                      "    foo(crull);\n"
                      "    foo(crf);\n"
                      "    foo(crd);\n"
                      "    foo(crld);\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( 1 ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 1U ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 1UL ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 1ULL ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 1.0F ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 1.0 ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 1.0L ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( i ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ui ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ul ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ull ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( f ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( d ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ld ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ri ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( rui ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( rul ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( rull ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( rf ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( rd ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( rld ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( cri ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( crui ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( crul ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( crull ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( crf ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( crd ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( crld ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8);
    }

    void findFunction5() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    void Sync(dsmp_t& type, int& len, int limit = 123);\n"
                      "    void Sync(int& syncpos, dsmp_t& type, int& len, int limit = 123);\n"
                      "    void FindSyncPoint();\n"
                      "};\n"
                      "void Fred::FindSyncPoint() {\n"
                      "    dsmp_t type;\n"
                      "    int syncpos, len;\n"
                      "    Sync(syncpos, type, len);\n"
                      "    Sync(type, len);\n"
                      "}");
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "Sync ( syncpos");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "Sync ( type");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);
    }

    void findFunction6() { // avoid null pointer access
        GET_SYMBOL_DB("void addtoken(Token** rettail, const Token *tok);\n"
                      "void CheckMemoryLeakInFunction::getcode(const Token *tok ) {\n"
                      "   addtoken(&rettail, tok);\n"
                      "}");
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "void addtoken ( Token * *");
        ASSERT_EQUALS(true, db && f && !f->function()); // regression value only
    }


#define FUNC(x) const Function *x = findFunctionByName(#x, &db->scopeList.front()); \
                ASSERT_EQUALS(true, x != nullptr);                                  \
                if (x) ASSERT_EQUALS(true, x->isNoExcept());

    void noexceptFunction1() {
        GET_SYMBOL_DB("void func1() noexcept;\n"
                      "void func2() noexcept { }\n"
                      "void func3() noexcept(true);\n"
                      "void func4() noexcept(true) { }\n");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            FUNC(func1);
            FUNC(func2);
            FUNC(func3);
            FUNC(func4);
        }
    }

    void noexceptFunction2() {
        GET_SYMBOL_DB("template <class T> void self_assign(T& t) noexcept(noexcept(t = t)) {t = t; }\n");

        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            FUNC(self_assign);
        }
    }

#define CLASS_FUNC(x, y, z) const Function *x = findFunctionByName(#x, y); \
                         ASSERT_EQUALS(true, x != nullptr);             \
                         if (x) ASSERT_EQUALS(z, x->isNoExcept());

    void noexceptFunction3() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    void func1() noexcept;\n"
                      "    void func2() noexcept { }\n"
                      "    void func3() noexcept(true);\n"
                      "    void func4() noexcept(true) { }\n"
                      "    void func5() const noexcept;\n"
                      "    void func6() const noexcept { }\n"
                      "    void func7() const noexcept(true);\n"
                      "    void func8() const noexcept(true) { }\n"
                      "    void func9() noexcept = 0;\n"
                      "    void func10() noexcept = 0;\n"
                      "    void func11() const noexcept(true) = 0;\n"
                      "    void func12() const noexcept(true) = 0;\n"
                      "    void func13() const noexcept(false) = 0;\n"
                      "};");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            const Scope *fred = db->findScopeByName("Fred");
            ASSERT_EQUALS(true, fred != nullptr);
            if (fred) {
                CLASS_FUNC(func1, fred, true);
                CLASS_FUNC(func2, fred, true);
                CLASS_FUNC(func3, fred, true);
                CLASS_FUNC(func4, fred, true);
                CLASS_FUNC(func5, fred, true);
                CLASS_FUNC(func6, fred, true);
                CLASS_FUNC(func7, fred, true);
                CLASS_FUNC(func8, fred, true);
                CLASS_FUNC(func9, fred, true);
                CLASS_FUNC(func10, fred, true);
                CLASS_FUNC(func11, fred, true);
                CLASS_FUNC(func12, fred, true);
                CLASS_FUNC(func13, fred, false);
            }
        }
    }

    void noexceptFunction4() {
        GET_SYMBOL_DB("class A {\n"
                      "public:\n"
                      "   A(A&& a) {\n"
                      "      throw std::runtime_error(\"err\");\n"
                      "   }\n"
                      "};\n"
                      "class B {\n"
                      "   A a;\n"
                      "   B(B&& b) noexcept\n"
                      "   :a(std::move(b.a)) { }\n"
                      "};\n");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            const Scope *b = db->findScopeByName("B");
            ASSERT_EQUALS(true, b != nullptr);
            if (b) {
                CLASS_FUNC(B, b, true);
            }
        }
    }

#define FUNC_THROW(x) const Function *x = findFunctionByName(#x, &db->scopeList.front()); \
                      ASSERT_EQUALS(true, x != nullptr);                                  \
                      if (x) ASSERT_EQUALS(true, x->isThrow());

    void throwFunction1() {
        GET_SYMBOL_DB("void func1() throw();\n"
                      "void func2() throw() { }\n"
                      "void func3() throw(int);\n"
                      "void func4() throw(int) { }\n");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            FUNC_THROW(func1);
            FUNC_THROW(func2);
            FUNC_THROW(func3);
            FUNC_THROW(func4);
        }
    }

#define CLASS_FUNC_THROW(x, y) const Function *x = findFunctionByName(#x, y); \
                               ASSERT_EQUALS(true, x != nullptr);             \
                               if (x) ASSERT_EQUALS(true, x->isThrow());
    void throwFunction2() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    void func1() throw();\n"
                      "    void func2() throw() { }\n"
                      "    void func3() throw(int);\n"
                      "    void func4() throw(int) { }\n"
                      "    void func5() const throw();\n"
                      "    void func6() const throw() { }\n"
                      "    void func7() const throw(int);\n"
                      "    void func8() const throw(int) { }\n"
                      "    void func9() throw() = 0;\n"
                      "    void func10() throw(int) = 0;\n"
                      "    void func11() const throw() = 0;\n"
                      "    void func12() const throw(int) = 0;\n"
                      "};");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            const Scope *fred = db->findScopeByName("Fred");
            ASSERT_EQUALS(true, fred != nullptr);
            if (fred) {
                CLASS_FUNC_THROW(func1, fred);
                CLASS_FUNC_THROW(func2, fred);
                CLASS_FUNC_THROW(func3, fred);
                CLASS_FUNC_THROW(func4, fred);
                CLASS_FUNC_THROW(func5, fred);
                CLASS_FUNC_THROW(func6, fred);
                CLASS_FUNC_THROW(func7, fred);
                CLASS_FUNC_THROW(func8, fred);
                CLASS_FUNC_THROW(func9, fred);
                CLASS_FUNC_THROW(func10, fred);
                CLASS_FUNC_THROW(func11, fred);
                CLASS_FUNC_THROW(func12, fred);
            }
        }
    }

    void nothrowAttributeFunction() {
        GET_SYMBOL_DB("void func() __attribute__((nothrow));\n"
                      "void func() { }\n");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            const Function *func = findFunctionByName("func", &db->scopeList.front());
            ASSERT_EQUALS(true, func != nullptr);
            if (func)
                ASSERT_EQUALS(true, func->isAttributeNothrow());
        }
    }

    void nothrowDeclspecFunction() {
        GET_SYMBOL_DB("void __declspec(nothrow) func() { }\n");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            const Function *func = findFunctionByName("func", &db->scopeList.front());
            ASSERT_EQUALS(true, func != nullptr);
            if (func)
                ASSERT_EQUALS(true, func->isAttributeNothrow());
        }
    }

    void varTypesIntegral() {
        GET_SYMBOL_DB("void f() { bool b; char c; unsigned char uc; short s; unsigned short us; int i; unsigned u; unsigned int ui; long l; unsigned long ul; long long ll; }");
        const Variable *b = db->getVariableFromVarId(1);
        ASSERT(b != nullptr);
        if (b) {
            ASSERT_EQUALS("b", b->nameToken()->str());
            ASSERT_EQUALS(true, b->isIntegralType());
            ASSERT_EQUALS(false, b->isFloatingType());
        }
        const Variable *c = db->getVariableFromVarId(2);
        ASSERT(c != nullptr);
        if (c) {
            ASSERT_EQUALS("c", c->nameToken()->str());
            ASSERT_EQUALS(true, c->isIntegralType());
            ASSERT_EQUALS(false, c->isFloatingType());
        }
        const Variable *uc = db->getVariableFromVarId(3);
        ASSERT(uc != nullptr);
        if (uc) {
            ASSERT_EQUALS("uc", uc->nameToken()->str());
            ASSERT_EQUALS(true, uc->isIntegralType());
            ASSERT_EQUALS(false, uc->isFloatingType());
        }
        const Variable *s = db->getVariableFromVarId(4);
        ASSERT(s != nullptr);
        if (s) {
            ASSERT_EQUALS("s", s->nameToken()->str());
            ASSERT_EQUALS(true, s->isIntegralType());
            ASSERT_EQUALS(false, s->isFloatingType());
        }
        const Variable *us = db->getVariableFromVarId(5);
        ASSERT(us != nullptr);
        if (us) {
            ASSERT_EQUALS("us", us->nameToken()->str());
            ASSERT_EQUALS(true, us->isIntegralType());
            ASSERT_EQUALS(false, us->isFloatingType());
        }
        const Variable *i = db->getVariableFromVarId(6);
        ASSERT(i != nullptr);
        if (i) {
            ASSERT_EQUALS("i", i->nameToken()->str());
            ASSERT_EQUALS(true, i->isIntegralType());
            ASSERT_EQUALS(false, i->isFloatingType());
        }
        const Variable *u = db->getVariableFromVarId(7);
        ASSERT(u != nullptr);
        if (u) {
            ASSERT_EQUALS("u", u->nameToken()->str());
            ASSERT_EQUALS(true, u->isIntegralType());
            ASSERT_EQUALS(false, u->isFloatingType());
        }
        const Variable *ui = db->getVariableFromVarId(8);
        ASSERT(ui != nullptr);
        if (ui) {
            ASSERT_EQUALS("ui", ui->nameToken()->str());
            ASSERT_EQUALS(true, ui->isIntegralType());
            ASSERT_EQUALS(false, ui->isFloatingType());
        }
        const Variable *l = db->getVariableFromVarId(9);
        ASSERT(l != nullptr);
        if (l) {
            ASSERT_EQUALS("l", l->nameToken()->str());
            ASSERT_EQUALS(true, l->isIntegralType());
            ASSERT_EQUALS(false, l->isFloatingType());
        }
        const Variable *ul = db->getVariableFromVarId(10);
        ASSERT(ul != nullptr);
        if (ul) {
            ASSERT_EQUALS("ul", ul->nameToken()->str());
            ASSERT_EQUALS(true, ul->isIntegralType());
            ASSERT_EQUALS(false, ul->isFloatingType());
        }
        const Variable *ll = db->getVariableFromVarId(11);
        ASSERT(ll != nullptr);
        if (ll) {
            ASSERT_EQUALS("ll", ll->nameToken()->str());
            ASSERT_EQUALS(true, ll->isIntegralType());
            ASSERT_EQUALS(false, ll->isFloatingType());
        }
    }

    void varTypesFloating() {
        {
            GET_SYMBOL_DB("void f() { float f; double d; long double ld; }");
            const Variable *f = db->getVariableFromVarId(1);
            ASSERT(f != nullptr);
            if (f) {
                ASSERT_EQUALS("f", f->nameToken()->str());
                ASSERT_EQUALS(false, f->isIntegralType());
                ASSERT_EQUALS(true, f->isFloatingType());
            }
            const Variable *d = db->getVariableFromVarId(2);
            ASSERT(d != nullptr);
            if (d) {
                ASSERT_EQUALS("d", d->nameToken()->str());
                ASSERT_EQUALS(false, d->isIntegralType());
                ASSERT_EQUALS(true, d->isFloatingType());
            }
            const Variable *ld = db->getVariableFromVarId(3);
            ASSERT(ld != nullptr);
            if (ld) {
                ASSERT_EQUALS("ld", ld->nameToken()->str());
                ASSERT_EQUALS(false, ld->isIntegralType());
                ASSERT_EQUALS(true, ld->isFloatingType());
            }
        }
        {
            GET_SYMBOL_DB("void f() { float * f; static const float * scf; }");
            const Variable *f = db->getVariableFromVarId(1);
            ASSERT(f != nullptr);
            if (f) {
                ASSERT_EQUALS("f", f->nameToken()->str());
                ASSERT_EQUALS(false, f->isIntegralType());
                ASSERT_EQUALS(true, f->isFloatingType());
                ASSERT_EQUALS(true, f->isArrayOrPointer());
            }
            const Variable *scf = db->getVariableFromVarId(2);
            ASSERT(scf != nullptr);
            if (scf) {
                ASSERT_EQUALS("scf", scf->nameToken()->str());
                ASSERT_EQUALS(false, scf->isIntegralType());
                ASSERT_EQUALS(true, scf->isFloatingType());
                ASSERT_EQUALS(true, scf->isArrayOrPointer());
            }
        }
        {
            GET_SYMBOL_DB("void f() { float fa[42]; }");
            const Variable *fa = db->getVariableFromVarId(1);
            ASSERT(fa != nullptr);
            if (fa) {
                ASSERT_EQUALS("fa", fa->nameToken()->str());
                ASSERT_EQUALS(false, fa->isIntegralType());
                ASSERT_EQUALS(true, fa->isFloatingType());
                ASSERT_EQUALS(true, fa->isArrayOrPointer());
            }
        }
    }

    void varTypesOther() {
        GET_SYMBOL_DB("void f() { class A {} a; void *b;  }");
        const Variable *a = db->getVariableFromVarId(1);
        ASSERT(a != nullptr);
        if (a) {
            ASSERT_EQUALS("a", a->nameToken()->str());
            ASSERT_EQUALS(false, a->isIntegralType());
            ASSERT_EQUALS(false, a->isFloatingType());
        }
        const Variable *b = db->getVariableFromVarId(2);
        ASSERT(b != nullptr);
        if (b) {
            ASSERT_EQUALS("b", b->nameToken()->str());
            ASSERT_EQUALS(false, b->isIntegralType());
            ASSERT_EQUALS(false, b->isFloatingType());
        }
    }

    void functionPrototype() {
        check("int foo(int x) {\n"
              "    extern int func1();\n"
              "    extern int func2(int);\n"
              "    int func3();\n"
              "    int func4(int);\n"
              "    return func4(x);\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void lambda() {
        GET_SYMBOL_DB("void func() {\n"
                      "    float y = 0.0f;\n"
                      "    auto lambda = [&]()\n"
                      "    {\n"
                      "        float x = 1.0f;\n"
                      "        y += 10.0f - x;\n"
                      "    }\n"
                      "    lambda();\n"
                      "}");

        ASSERT(db && db->scopeList.size() == 3);
        if (db && db->scopeList.size() == 3) {
            std::list<Scope>::const_iterator scope = db->scopeList.begin();
            ASSERT_EQUALS(Scope::eGlobal, scope->type);
            ++scope;
            ASSERT_EQUALS(Scope::eFunction, scope->type);
            ++scope;
            ASSERT_EQUALS(Scope::eLambda, scope->type);
        }
    }
};

REGISTER_TEST(TestSymbolDatabase)
